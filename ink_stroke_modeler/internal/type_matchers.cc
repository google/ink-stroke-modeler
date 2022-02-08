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

#include "ink_stroke_modeler/internal/type_matchers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::DoubleNear;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::Matches;

MATCHER_P(Vec2EqMatcher, expected, "") {
  return Matches(FloatEq(expected.x))(arg.x) &&
         Matches(FloatEq(expected.y))(arg.y);
}

MATCHER_P2(Vec2NearMatcher, expected, tolerance, "") {
  return Matches(FloatNear(expected.x, tolerance))(arg.x) &&
         Matches(FloatNear(expected.y, tolerance))(arg.y);
}
MATCHER_P2(TipStateNearMatcher, expected, tolerance, "") {
  return Matches(Vec2Near(expected.position, tolerance))(arg.position) &&
         Matches(Vec2Near(expected.velocity, tolerance))(arg.velocity) &&
         Matches(DoubleNear(expected.time.Value(), tolerance))(
             arg.time.Value());
}

MATCHER_P2(StylusStateNearMatcher, expected, tolerance, "") {
  return Matches(FloatNear(expected.pressure, tolerance))(arg.pressure) &&
         Matches(FloatNear(expected.tilt, tolerance))(arg.tilt) &&
         Matches(FloatNear(expected.orientation, tolerance))(arg.orientation);
}

}  // namespace

Matcher<Vec2> Vec2Eq(const Vec2 v) { return Vec2EqMatcher(v); }
Matcher<Vec2> Vec2Near(const Vec2 v, float tolerance) {
  return Vec2NearMatcher(v, tolerance);
}
Matcher<TipState> TipStateNear(const TipState &expected, float tolerance) {
  return TipStateNearMatcher(expected, tolerance);
}
Matcher<StylusState> StylusStateNear(const StylusState &expected,
                                     float tolerance) {
  return StylusStateNearMatcher(expected, tolerance);
}

}  // namespace stroke_model
}  // namespace ink
