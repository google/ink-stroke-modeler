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

#include "ink_stroke_modeler/internal/stylus_state_modeler.h"

#include <cmath>
#include <limits>
#include <optional>

#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/utils.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

void StylusStateModeler::Update(Vec2 position, Time time,
                                const StylusState &state) {
  // Possibly NaN should be prohibited in ValidateInput, but due to current
  // consumers, that can't be tightened for these values currently.
  if (state.pressure < 0 || std::isnan(state.pressure)) {
    state_.received_unknown_pressure = true;
  }
  if (state.tilt < 0 || std::isnan(state.tilt)) {
    state_.received_unknown_tilt = true;
  }
  if (state.orientation < 0 || std::isnan(state.orientation)) {
    state_.received_unknown_orientation = true;
  }

  if (!params_.use_stroke_normal_projection &&
      state_.received_unknown_pressure && state_.received_unknown_tilt &&
      state_.received_unknown_orientation) {
    // We've stopped tracking all fields, so there's no need to keep updating.
    state_.raw_input_and_stylus_states.clear();
    return;
  }

  Vec2 velocity = {0, 0};
  Vec2 acceleration = {0, 0};
  if (!state_.raw_input_and_stylus_states.empty() &&
      time != state_.raw_input_and_stylus_states.back().time) {
    velocity = (position - state_.raw_input_and_stylus_states.back().position) /
               (time - state_.raw_input_and_stylus_states.back().time).Value();
    acceleration =
        (velocity - state_.raw_input_and_stylus_states.back().velocity) /
        (time - state_.raw_input_and_stylus_states.back().time).Value();
  }

  state_.raw_input_and_stylus_states.push_back({
      .position = position,
      .velocity = velocity,
      .acceleration = acceleration,
      .time = time,
      .pressure = state.pressure,
      .tilt = state.tilt,
      .orientation = state.orientation,
  });

  if (params_.max_input_samples < 0 ||
      state_.raw_input_and_stylus_states.size() >
          static_cast<unsigned int>(params_.max_input_samples)) {
    state_.raw_input_and_stylus_states.pop_front();
  }
}

void StylusStateModeler::Reset(const StylusStateModelerParams &params) {
  state_.raw_input_and_stylus_states.clear();
  state_.received_unknown_pressure = false;
  state_.received_unknown_tilt = false;
  state_.received_unknown_orientation = false;
  save_active_ = false;
  params_ = params;
}

Result StylusStateModeler::Query(Vec2 position,
                                 std::optional<Vec2> stroke_normal,
                                 Time time) const {
  if (state_.raw_input_and_stylus_states.empty())
    return {
        .position = {0, 0},
        .velocity = {0, 0},
        .acceleration = {0, 0},
        .time = Time(0),
        .pressure = -1,
        .tilt = -1,
        .orientation = -1,
    };

  int closest_segment_index = -1;
  float min_distance = std::numeric_limits<float>::infinity();
  float interp_value = 0;
  for (decltype(state_.raw_input_and_stylus_states.size()) i = 0;
       i < state_.raw_input_and_stylus_states.size() - 1; ++i) {
    const Vec2 segment_start = state_.raw_input_and_stylus_states[i].position;
    const Vec2 segment_end = state_.raw_input_and_stylus_states[i + 1].position;
    float param = 0;
    if (params_.use_stroke_normal_projection && stroke_normal.has_value()) {
      std::optional<float> temp_param = ProjectToSegmentAlongNormal(
          segment_start, segment_end, position, *stroke_normal);
      if (!temp_param.has_value()) continue;
      param = *temp_param;
    } else {
      param = NearestPointOnSegment(segment_start, segment_end, position);
    }
    float distance =
        Distance(position, Interp(segment_start, segment_end, param));
    if (distance <= min_distance) {
      closest_segment_index = i;
      min_distance = distance;
      interp_value = param;
    }
  }

  if (closest_segment_index < 0) {
    const auto &state =
        Distance(state_.raw_input_and_stylus_states.front().position,
                 position) <
                Distance(state_.raw_input_and_stylus_states.back().position,
                         position)
            ? state_.raw_input_and_stylus_states.front()
            : state_.raw_input_and_stylus_states.back();
    return {
        .position = state.position,
        .velocity = state.velocity,
        .acceleration = state.acceleration,
        .time = time,
        .pressure = state_.received_unknown_pressure ? -1 : state.pressure,
        .tilt = state_.received_unknown_tilt ? -1 : state.tilt,
        .orientation =
            state_.received_unknown_orientation ? -1 : state.orientation,
    };
  }

  auto from_state = state_.raw_input_and_stylus_states[closest_segment_index];
  auto to_state = state_.raw_input_and_stylus_states[closest_segment_index + 1];

  return Result{
      .position = Interp(from_state.position, to_state.position, interp_value),
      .velocity = Interp(from_state.velocity, to_state.velocity, interp_value),
      .acceleration =
          Interp(from_state.acceleration, to_state.acceleration, interp_value),
      .time = time,
      .pressure =
          state_.received_unknown_pressure
              ? -1
              : Interp(from_state.pressure, to_state.pressure, interp_value),
      .tilt = state_.received_unknown_tilt
                  ? -1
                  : Interp(from_state.tilt, to_state.tilt, interp_value),
      .orientation = state_.received_unknown_orientation
                         ? -1
                         : InterpAngle(from_state.orientation,
                                       to_state.orientation, interp_value),
  };
}

void StylusStateModeler::Save() {
  saved_state_ = state_;
  save_active_ = true;
}

void StylusStateModeler::Restore() {
  if (save_active_) {
    state_ = saved_state_;
  }
}

}  // namespace stroke_model
}  // namespace ink
