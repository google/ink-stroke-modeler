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

#include "ink_stroke_modeler/internal/utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/numbers.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

TEST(UtilsTest, Clamp01) {
  EXPECT_FLOAT_EQ(Clamp01(-2), 0);
  EXPECT_FLOAT_EQ(Clamp01(0), 0);
  EXPECT_FLOAT_EQ(Clamp01(.3), .3);
  EXPECT_FLOAT_EQ(Clamp01(.7), .7);
  EXPECT_FLOAT_EQ(Clamp01(1), 1);
  EXPECT_FLOAT_EQ(Clamp01(1.1), 1);
}

TEST(UtilsTest, Normalize01) {
  EXPECT_FLOAT_EQ(Normalize01(1, 2, 1.5), .5);
  EXPECT_FLOAT_EQ(Normalize01(7, 3, 4), .75);
  EXPECT_FLOAT_EQ(Normalize01(-1, 1, 2), 1);
  EXPECT_FLOAT_EQ(Normalize01(1, 1, 1), 0);
  EXPECT_FLOAT_EQ(Normalize01(1, 1, 0), 0);
  EXPECT_FLOAT_EQ(Normalize01(1, 1, 2), 1);
}

TEST(UtilsTest, InterpFloat) {
  EXPECT_FLOAT_EQ(Interp(5, 10, .2), 6);
  EXPECT_FLOAT_EQ(Interp(10, -2, .75), 1);
  EXPECT_FLOAT_EQ(Interp(-1, 2, -3), -1);
  EXPECT_FLOAT_EQ(Interp(5, 7, 20), 7);
}

TEST(UtilsTest, InterpVec2) {
  EXPECT_THAT(Interp(Vec2{1, 2}, {3, 5}, .5), Vec2Eq({2, 3.5}));
  EXPECT_THAT(Interp(Vec2{-5, 5}, {-15, 0}, .4), Vec2Eq({-9, 3}));
  EXPECT_THAT(Interp(Vec2{7, 9}, {25, 30}, -.1), Vec2Eq({7, 9}));
  EXPECT_THAT(Interp(Vec2{12, 5}, {13, 14}, 3.2), Vec2Eq({13, 14}));
}

TEST(UtilsTest, InterpAngle) {
  EXPECT_NEAR(InterpAngle(.25 * kPi, .5 * kPi, .4), .35 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.05 * kPi, .25 * kPi, .5), .65 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(.25 * kPi, 1.75 * kPi, .1), .2 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(.25 * kPi, 1.75 * kPi, .7), 1.9 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.6 * kPi, .4 * kPi, .25), 1.8 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.6 * kPi, .4 * kPi, .625), .1 * kPi, 1e-6);
}

TEST(UtilsTest, Distance) {
  EXPECT_FLOAT_EQ(Distance({0, 0}, {1, 0}), 1);
  EXPECT_FLOAT_EQ(Distance({1, 1}, {-2, 5}), 5);
}

TEST(UtilsTest, NearestPointOnSegment) {
  EXPECT_FLOAT_EQ(NearestPointOnSegment({0, 0}, {1, 0}, {.25, .5}), .25);
  EXPECT_FLOAT_EQ(NearestPointOnSegment({3, 4}, {5, 6}, {-1, -1}), 0);
  EXPECT_FLOAT_EQ(NearestPointOnSegment({20, 10}, {10, 5}, {2, 2}), 1);
  EXPECT_FLOAT_EQ(NearestPointOnSegment({0, 5}, {5, 0}, {3, 3}), .5);
}

TEST(UtilsTest, NearestPointOnSegmentDegenerateCase) {
  EXPECT_FLOAT_EQ(NearestPointOnSegment({0, 0}, {0, 0}, {5, 10}), 0);
  EXPECT_FLOAT_EQ(NearestPointOnSegment({3, 7}, {3, 7}, {0, -20}), 0);
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
