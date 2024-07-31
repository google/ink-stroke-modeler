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

void StylusStateModeler::Update(double time, const StylusState &state) {
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

  Vec2 velocity = {0, 0};
  Vec2 acceleration = {0, 0};
  if (!state_.stylus_states.empty()) {
    velocity = (state.projected_position -
                state_.stylus_states.back().projected_position) /
               (time - last_time_);
    acceleration =
        (velocity - *state_.stylus_states.back().projected_velocity) /
        (time - last_time_);
  }

  state_.stylus_states.push_back(
      {.pressure = state.pressure,
       .tilt = state.tilt,
       .orientation = state.orientation,
       .projected_position = state.projected_position,
       .projected_velocity = velocity,
       .projected_acceleration = acceleration});

  if (params_.max_input_samples < 0 ||
      state_.stylus_states.size() >
          static_cast<unsigned int>(params_.max_input_samples)) {
    state_.stylus_states.pop_front();
  }
  last_time_ = time;
}

void StylusStateModeler::Reset(const StylusStateModelerParams &params) {
  state_.stylus_states.clear();
  state_.received_unknown_pressure = false;
  state_.received_unknown_tilt = false;
  state_.received_unknown_orientation = false;
  save_active_ = false;
  params_ = params;
  last_time_ = 0;
}

StylusState StylusStateModeler::Query(Vec2 position) const {
  if (state_.stylus_states.empty())
    return {.pressure = -1,
            .tilt = -1,
            .orientation = -1,
            .projected_position = {0, 0},
            .projected_velocity = std::nullopt,
            .projected_acceleration = std::nullopt};

  int closest_segment_index = -1;
  float min_distance = std::numeric_limits<float>::infinity();
  float interp_value = 0;
  for (decltype(state_.stylus_states.size()) i = 0;
       i < state_.stylus_states.size() - 1; ++i) {
    const Vec2 segment_start = state_.stylus_states[i].projected_position;
    const Vec2 segment_end = state_.stylus_states[i + 1].projected_position;
    float param = NearestPointOnSegment(segment_start, segment_end, position);
    float distance =
        Distance(position, Interp(segment_start, segment_end, param));
    if (distance <= min_distance) {
      closest_segment_index = i;
      min_distance = distance;
      interp_value = param;
    }
  }

  if (closest_segment_index < 0) {
    const auto &state = state_.stylus_states.front();
    return {.pressure = state_.received_unknown_pressure ? -1 : state.pressure,
            .tilt = state_.received_unknown_tilt ? -1 : state.tilt,
            .orientation =
                state_.received_unknown_orientation ? -1 : state.orientation,
            .projected_position = state.projected_position,
            .projected_velocity = state.projected_velocity.has_value()
                                      ? state.projected_velocity
                                      : std::nullopt,
            .projected_acceleration = state.projected_acceleration.has_value()
                                          ? state.projected_acceleration
                                          : std::nullopt};
  }

  auto from_state = state_.stylus_states[closest_segment_index];
  auto to_state = state_.stylus_states[closest_segment_index + 1];
  return StylusState{
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
      .projected_position = Interp(from_state.projected_position,
                                   to_state.projected_position, interp_value),
      .projected_velocity =
          (from_state.projected_velocity.has_value() &&
           to_state.projected_velocity.has_value())
              ? std::optional<Vec2>(Interp(*from_state.projected_velocity,
                                           *to_state.projected_velocity,
                                           interp_value))
              : std::nullopt,
      .projected_acceleration =
          (from_state.projected_acceleration.has_value() &&
           to_state.projected_acceleration.has_value())
              ? std::optional<Vec2>(Interp(*from_state.projected_acceleration,
                                           *to_state.projected_acceleration,
                                           interp_value))
              : std::nullopt};
}

void StylusStateModeler::Save() {
  saved_state_ = state_;
  save_active_ = true;
  saved_last_time_ = last_time_;
}

void StylusStateModeler::Restore() {
  if (save_active_) {
    state_ = saved_state_;
    last_time_ = saved_last_time_;
  }
}

}  // namespace stroke_model
}  // namespace ink
