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

#include "ink_stroke_modeler/internal/wobble_smoother.h"

#include <algorithm>

#include "ink_stroke_modeler/internal/utils.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

void WobbleSmoother::Reset(const WobbleSmootherParams& params, Vec2 position,
                           Time time) {
  params_ = params;
  samples_.clear();
  // Initialize with the "fast" speed -- otherwise, we'll lag behind at the
  // start of the stroke.
  position_sum_ = position;
  speed_sum_ = params_.speed_ceiling;
  samples_.push_back(
      {.position = position, .speed = params_.speed_ceiling, .time = time});
}

Vec2 WobbleSmoother::Update(Vec2 position, Time time) {
  // The moving average acts as a low-pass signal filter, removing
  // high-frequency fluctuations in the position caused by the discrete nature
  // of the touch digitizer. To compensate for the distance between the average
  // position and the actual position, we interpolate between them, based on
  // speed, to determine the position to use for the input model.
  float distance = Distance(position, samples_.back().position);
  Duration delta_time = time - samples_.back().time;
  float speed = 0;
  if (delta_time == Duration(0)) {
    // We're going to assume that you're not actually moving infinitely fast.
    speed = std::max(params_.speed_ceiling, speed_sum_ / samples_.size());
  } else {
    speed = distance / delta_time.Value();
  }

  samples_.push_back({.position = position, .speed = speed, .time = time});
  position_sum_ += position;
  speed_sum_ += speed;
  while (samples_.front().time < time - params_.timeout) {
    position_sum_ -= samples_.front().position;
    speed_sum_ -= samples_.front().speed;
    samples_.pop_front();
  }

  Vec2 avg_position = position_sum_ / samples_.size();
  float avg_speed = speed_sum_ / samples_.size();
  return Interp(
      avg_position, position,
      Normalize01(params_.speed_floor, params_.speed_ceiling, avg_speed));
}

}  // namespace stroke_model
}  // namespace ink
