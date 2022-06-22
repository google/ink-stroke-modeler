/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INK_STROKE_MODELER_INTERNAL_WOBBLE_SMOOTHER_H_
#define INK_STROKE_MODELER_INTERNAL_WOBBLE_SMOOTHER_H_

#include <deque>

#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

// This class smooths "wobble" in input positions from high-frequency noise. It
// does so by maintaining a moving average of the positions, and interpolating
// between the given input and the moving average based on how quickly it's
// moving. When moving at a speed above the ceiling in the WobbleSmootherParams,
// the result will be the unmodified input; when moving at a speed below the
// floor, the result will be the moving average.
class WobbleSmoother {
 public:
  void Reset(const WobbleSmootherParams &params, Vec2 position, Time time);

  // Updates the average position and speed, and returns the smoothed position.
  Vec2 Update(Vec2 position, Time time);

 private:
  struct Sample {
    Vec2 position{0, 0};
    Vec2 weighted_position{0, 0};
    float distance = 0;
    Duration duration{0};
    Time time{0};
  };
  std::deque<Sample> samples_;
  Vec2 weighted_position_sum_{0, 0};
  float distance_sum_ = 0;
  float duration_sum_ = 0;

  WobbleSmootherParams params_;
};

}  // namespace stroke_model
}  // namespace ink

#endif  // INK_STROKE_MODELER_INTERNAL_WOBBLE_SMOOTHER_H_
