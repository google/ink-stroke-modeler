// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ink_stroke_modeler/stroke_modeler.h"

#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/position_modeler.h"
#include "ink_stroke_modeler/internal/prediction/input_predictor.h"
#include "ink_stroke_modeler/internal/prediction/kalman_predictor.h"
#include "ink_stroke_modeler/internal/prediction/stroke_end_predictor.h"
#include "ink_stroke_modeler/internal/stylus_state_modeler.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

std::vector<Result> ModelStylus(
    const std::vector<TipState> &tip_states,
    const StylusStateModeler &stylus_state_modeler) {
  std::vector<Result> result;
  result.reserve(tip_states.size());
  for (const auto &tip_state : tip_states) {
    auto stylus_state = stylus_state_modeler.Query(tip_state.position);
    result.push_back({.position = tip_state.position,
                      .velocity = tip_state.velocity,
                      .time = tip_state.time,
                      .pressure = stylus_state.pressure,
                      .tilt = stylus_state.tilt,
                      .orientation = stylus_state.orientation});
  }
  return result;
}

absl::StatusOr<int> GetNumberOfSteps(Time start_time, Time end_time,
                                     const SamplingParams &sampling_params) {
  float float_delta = (end_time - start_time).Value();
  int n_steps =
      std::min(std::ceil(float_delta * sampling_params.min_output_rate),
               static_cast<double>(INT_MAX));
  if (n_steps > sampling_params.max_outputs_per_call) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Input events are too far apart, requested $0 > $1 samples.", n_steps,
        sampling_params.max_outputs_per_call));
  }
  return n_steps;
}

template <typename>
ABSL_ATTRIBUTE_UNUSED inline constexpr bool kAlwaysFalse = false;

}  // namespace

absl::Status StrokeModeler::Reset(
    const StrokeModelParams &stroke_model_params) {
  if (auto status = ValidateStrokeModelParams(stroke_model_params);
      !status.ok()) {
    return status;
  }

  // Note that many of the sub-modelers require some knowledge about the stroke
  // (e.g. start position, input type) when resetting, and as such are reset in
  // ProcessTDown() instead.
  stroke_model_params_ = stroke_model_params;
  last_input_ = std::nullopt;

  std::visit(
      [this](auto &&params) {
        using ParamType = std::decay_t<decltype(params)>;
        if constexpr (std::is_same_v<ParamType, KalmanPredictorParams>) {
          predictor_ = std::make_unique<KalmanPredictor>(
              params, stroke_model_params_->sampling_params);
        } else if constexpr (std::is_same_v<ParamType,
                                            StrokeEndPredictorParams>) {
          predictor_ = std::make_unique<StrokeEndPredictor>(
              stroke_model_params_->position_modeler_params,
              stroke_model_params_->sampling_params);
        } else {
          static_assert(kAlwaysFalse<ParamType>,
                        "Unknown prediction parameter type");
        }
      },
      stroke_model_params_->prediction_params);
  return absl::OkStatus();
}

absl::Status StrokeModeler::Reset() {
  if (!stroke_model_params_.has_value()) {
    return absl::FailedPreconditionError(
        "Initial call to Reset must pass StrokeModelParams.");
  }
  last_input_ = absl::nullopt;
  return absl::OkStatus();
}

absl::StatusOr<std::vector<Result>> StrokeModeler::Update(const Input &input) {
  if (!stroke_model_params_.has_value()) {
    return absl::FailedPreconditionError(
        "Stroke model has not yet been initialized");
  }

  if (absl::Status status = ValidateInput(input); !status.ok()) {
    return status;
  }

  if (last_input_) {
    if (last_input_->input == input) {
      return absl::InvalidArgumentError("Received duplicate input");
    }

    if (input.time < last_input_->input.time) {
      return absl::InvalidArgumentError("Inputs travel backwards in time");
    }
  }

  switch (input.event_type) {
    case Input::EventType::kDown:
      return ProcessDownEvent(input);
    case Input::EventType::kMove:
      return ProcessMoveEvent(input);
    case Input::EventType::kUp:
      return ProcessUpEvent(input);
  }
  return absl::InvalidArgumentError("Invalid EventType.");
}

absl::StatusOr<std::vector<Result>> StrokeModeler::Predict() const {
  if (!stroke_model_params_.has_value()) {
    return absl::FailedPreconditionError(
        "Stroke model has not yet been initialized");
  }

  if (last_input_ == std::nullopt) {
    return absl::FailedPreconditionError(
        "Cannot construct prediction when no stroke is in-progress");
  }

  return ModelStylus(
      predictor_->ConstructPrediction(position_modeler_.CurrentState()),
      stylus_state_modeler_);
}

absl::StatusOr<std::vector<Result>> StrokeModeler::ProcessDownEvent(
    const Input &input) {
  if (last_input_) {
    return absl::FailedPreconditionError(
        "Received down event while stroke is in-progress");
  }

  // Note that many of the sub-modelers require some knowledge about the stroke
  // (e.g. start position, input type) when resetting, and as such are reset
  // here instead of in Reset().
  wobble_smoother_.Reset(stroke_model_params_->wobble_smoother_params,
                         input.position, input.time);
  position_modeler_.Reset({input.position, {0, 0}, input.time},
                          stroke_model_params_->position_modeler_params);
  stylus_state_modeler_.Reset(
      stroke_model_params_->stylus_state_modeler_params);
  stylus_state_modeler_.Update(input.position,
                               {.pressure = input.pressure,
                                .tilt = input.tilt,
                                .orientation = input.orientation});

  const TipState &tip_state = position_modeler_.CurrentState();
  predictor_->Reset();
  predictor_->Update(input.position, input.time);

  // We don't correct the position on the down event, so we set
  // corrected_position to use the input position.
  last_input_ = {.input = input, .corrected_position = input.position};
  return {{{.position = tip_state.position,
            .velocity = tip_state.velocity,
            .time = tip_state.time,
            .pressure = input.pressure,
            .tilt = input.tilt,
            .orientation = input.orientation}}};
}

absl::StatusOr<std::vector<Result>> StrokeModeler::ProcessUpEvent(
    const Input &input) {
  if (!last_input_) {
    return absl::FailedPreconditionError(
        "Received up event while no stroke is in-progress");
  }

  absl::StatusOr<int> n_steps =
      GetNumberOfSteps(last_input_->input.time, input.time,
                       stroke_model_params_->sampling_params);
  if (!n_steps.ok()) {
    return n_steps.status();
  }
  std::vector<TipState> tip_states;
  tip_states.reserve(
      *n_steps +
      stroke_model_params_->sampling_params.end_of_stroke_max_iterations);
  position_modeler_.UpdateAlongLinearPath(
      last_input_->corrected_position, last_input_->input.time, input.position,
      input.time, *n_steps, std::back_inserter(tip_states));

  position_modeler_.ModelEndOfStroke(
      input.position,
      Duration(1. / stroke_model_params_->sampling_params.min_output_rate),
      stroke_model_params_->sampling_params.end_of_stroke_max_iterations,
      stroke_model_params_->sampling_params.end_of_stroke_stopping_distance,
      std::back_inserter(tip_states));

  if (tip_states.empty()) {
    // If we haven't generated any new states, add the current state. This can
    // happen if the TUp has the same timestamp as the last in-contact input.
    tip_states.push_back(position_modeler_.CurrentState());
  }

  stylus_state_modeler_.Update(input.position,
                               {.pressure = input.pressure,
                                .tilt = input.tilt,
                                .orientation = input.orientation});

  // This indicates that we've finished the stroke.
  last_input_ = std::nullopt;

  return ModelStylus(tip_states, stylus_state_modeler_);
}

absl::StatusOr<std::vector<Result>> StrokeModeler::ProcessMoveEvent(
    const Input &input) {
  if (!last_input_) {
    return absl::FailedPreconditionError(
        "Received move event while no stroke is in-progress");
  }

  Vec2 corrected_position = wobble_smoother_.Update(input.position, input.time);
  stylus_state_modeler_.Update(corrected_position,
                               {.pressure = input.pressure,
                                .tilt = input.tilt,
                                .orientation = input.orientation});

  std::vector<TipState> tip_states;

  absl::StatusOr<int> n_steps =
      GetNumberOfSteps(last_input_->input.time, input.time,
                       stroke_model_params_->sampling_params);
  if (!n_steps.ok()) {
    return n_steps.status();
  }
  tip_states.reserve(*n_steps);
  position_modeler_.UpdateAlongLinearPath(
      last_input_->corrected_position, last_input_->input.time,
      corrected_position, input.time, *n_steps, std::back_inserter(tip_states));

  predictor_->Update(corrected_position, input.time);
  last_input_ = {.input = input, .corrected_position = corrected_position};
  return ModelStylus(tip_states, stylus_state_modeler_);
}

}  // namespace stroke_model
}  // namespace ink
