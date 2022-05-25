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

#ifndef INK_STROKE_MODELER_INTERNAL_INTERNAL_TYPES_H_
#define INK_STROKE_MODELER_INTERNAL_INTERNAL_TYPES_H_

#include <ostream>

#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

// This struct contains the position and velocity of the modeled pen tip at
// the indicated time.
struct TipState {
  Vec2 position{0};
  Vec2 velocity{0};
  Time time{0};
};

std::ostream &operator<<(std::ostream &s, const TipState &tip_state);

// This struct contains information about the state of the stylus. See the
// corresponding fields on the Input struct for more info.
struct StylusState {
  float pressure = -1;
  float tilt = -1;
  float orientation = -1;
};

bool operator==(const StylusState &lhs, const StylusState &rhs);
std::ostream &operator<<(std::ostream &s, const StylusState &stylus_state);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const StylusState &lhs, const StylusState &rhs) {
  return lhs.pressure == rhs.pressure && lhs.tilt == rhs.tilt &&
         lhs.orientation == rhs.orientation;
}

inline std::ostream &operator<<(std::ostream &s, const TipState &tip_state) {
  return s << "<TipState: pos: " << tip_state.position
           << ", velocity: " << tip_state.velocity
           << ", time: " << tip_state.time << ">";
}

inline std::ostream &operator<<(std::ostream &s,
                                const StylusState &stylus_state) {
  return s << "<Result:  pressure: " << stylus_state.pressure
           << ", tilt: " << stylus_state.tilt
           << ", orientation: " << stylus_state.orientation << ">";
}

}  // namespace stroke_model
}  // namespace ink

#endif  // INK_STROKE_MODELER_INTERNAL_INTERNAL_TYPES_H_
