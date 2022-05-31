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

#include <limits>

#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/utils.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

void StylusStateModeler::Update(Vec2 position, const StylusState &state) {
  // Possibly NaN should be prohibited in ValidateInput, but due to current
  // consumers, that can't be tightened for these values currently.
  if (state.pressure < 0 || std::isnan(state.pressure)) {
    received_unknown_pressure_ = true;
  }
  if (state.tilt < 0 || std::isnan(state.tilt)) {
    received_unknown_tilt_ = true;
  }
  if (state.orientation < 0 || std::isnan(state.orientation)) {
    received_unknown_orientation_ = true;
  }

  if (received_unknown_pressure_ && received_unknown_tilt_ &&
      received_unknown_orientation_) {
    // We've stopped tracking all fields, so there's no need to keep updating.
    positions_and_states_.clear();
    return;
  }

  positions_and_states_.push_back({position, state});

  if (params_.max_input_samples < 0 ||
      positions_and_states_.size() > (uint)params_.max_input_samples) {
    positions_and_states_.pop_front();
  }
}

void StylusStateModeler::Reset(const StylusStateModelerParams &params) {
  params_ = params;
  positions_and_states_.clear();
  received_unknown_pressure_ = false;
  received_unknown_tilt_ = false;
  received_unknown_orientation_ = false;
}

StylusState StylusStateModeler::Query(Vec2 position) const {
  if (positions_and_states_.empty())
    return {.pressure = -1, .tilt = -1, .orientation = -1};

  int closest_segment_index = -1;
  float min_distance = std::numeric_limits<float>::infinity();
  float interp_value = 0;
  for (decltype(positions_and_states_.size()) i = 0;
       i < positions_and_states_.size() - 1; ++i) {
    const Vec2 segment_start = positions_and_states_[i].position;
    const Vec2 segment_end = positions_and_states_[i + 1].position;
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
    const auto &state = positions_and_states_.front().state;
    return {
        .pressure = received_unknown_pressure_ ? -1 : state.pressure,
        .tilt = received_unknown_tilt_ ? -1 : state.tilt,
        .orientation = received_unknown_orientation_ ? -1 : state.orientation};
  }

  auto from_state = positions_and_states_[closest_segment_index].state;
  auto to_state = positions_and_states_[closest_segment_index + 1].state;
  return StylusState{
      .pressure =
          received_unknown_pressure_
              ? -1
              : Interp(from_state.pressure, to_state.pressure, interp_value),
      .tilt = received_unknown_tilt_
                  ? -1
                  : Interp(from_state.tilt, to_state.tilt, interp_value),
      .orientation = received_unknown_orientation_
                         ? -1
                         : InterpAngle(from_state.orientation,
                                       to_state.orientation, interp_value)};
}

}  // namespace stroke_model
}  // namespace ink
