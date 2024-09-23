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

#include <optional>

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

TEST(UtilsTest, InverseLerp) {
  EXPECT_FLOAT_EQ(InverseLerp(1, 2, 1), 0);
  EXPECT_FLOAT_EQ(InverseLerp(1, 2, 1.5), .5);
  EXPECT_FLOAT_EQ(InverseLerp(1, 2, 2), 1);
  EXPECT_FLOAT_EQ(InverseLerp(1, 2, 0), -1);
  EXPECT_FLOAT_EQ(InverseLerp(1, 2, 3), 2);
  EXPECT_FLOAT_EQ(InverseLerp(1, 1, 1), 0);
  EXPECT_FLOAT_EQ(InverseLerp(1, 1, 2), 0);
}

TEST(UtilsTest, InterpAngle) {
  EXPECT_NEAR(InterpAngle(.25 * kPi, .5 * kPi, .4), .35 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.05 * kPi, .25 * kPi, .5), .65 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(.25 * kPi, 1.75 * kPi, .1), .2 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(.25 * kPi, 1.75 * kPi, .7), 1.9 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.6 * kPi, .4 * kPi, .25), 1.8 * kPi, 1e-6);
  EXPECT_NEAR(InterpAngle(1.6 * kPi, .4 * kPi, .625), .1 * kPi, 1e-6);
}

TEST(UtilsTest, InterpResult) {
  Result a{.position = {1, 2},
           .velocity = {3, 4},
           .acceleration = {5, 6},
           .time = Time(1),
           .pressure = 0.1,
           .tilt = 0.2,
           .orientation = 0.3};
  Result b{.position = {7, 8},
           .velocity = {9, 10},
           .acceleration = {11, 12},
           .time = Time(2),
           .pressure = 0.4,
           .tilt = 0.5,
           .orientation = 0.6};
  EXPECT_THAT(InterpResult(a, b, 0), ResultNear(a, 1e-5, 1e-5));
  EXPECT_THAT(InterpResult(a, b, 1), ResultNear(b, 1e-5, 1e-5));

  EXPECT_THAT(InterpResult(a, b, 0.5), ResultNear({.position = {4, 5},
                                                   .velocity = {6, 7},
                                                   .acceleration = {8, 9},
                                                   .time = Time(1.5),
                                                   .pressure = 0.25,
                                                   .tilt = 0.35,
                                                   .orientation = 0.45},
                                                  1e-5, 1e-5));
  EXPECT_THAT(InterpResult(a, b, 0.25), ResultNear({.position = {2.5, 3.5},
                                                    .velocity = {4.5, 5.5},
                                                    .acceleration = {6.5, 7.5},
                                                    .time = Time(1.25),
                                                    .pressure = 0.175,
                                                    .tilt = 0.275,
                                                    .orientation = 0.375},
                                                   1e-5, 1e-5));
}

TEST(UtilsTest, InterpResultIgnoresMissingFields) {
  EXPECT_EQ(InterpResult({.pressure = 0.3}, {.pressure = -1}, 0.5).pressure,
            -1);
  EXPECT_EQ(InterpResult({.pressure = -1}, {.pressure = 0.3}, 0.5).pressure,
            -1);
  EXPECT_EQ(InterpResult({.tilt = 0.3}, {.tilt = -1}, 0.5).tilt, -1);
  EXPECT_EQ(InterpResult({.tilt = -1}, {.tilt = 0.3}, 0.5).tilt, -1);
  EXPECT_EQ(
      InterpResult({.orientation = 0.3}, {.orientation = -1}, 0.5).orientation,
      -1);
  EXPECT_EQ(
      InterpResult({.orientation = -1}, {.orientation = 0.3}, 0.5).orientation,
      -1);
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

TEST(UtilsTest, GetStrokeNormal) {
  EXPECT_EQ(GetStrokeNormal(
                {.velocity = {0, 0}, .acceleration = {0, 0}, .time = Time(0.2)},
                Time(0.1)),
            std::nullopt);
  EXPECT_THAT(
      GetStrokeNormal(
          {.velocity = {0, 0}, .acceleration = {4, 3}, .time = Time(0.2)},
          Time(0.1)),
      Optional(Vec2Eq({-3, 4})));
  EXPECT_THAT(
      GetStrokeNormal(
          {.velocity = {2, 3}, .acceleration = {0, 0}, .time = Time(0.2)},
          Time(0.1)),
      Optional(Vec2Eq({-3, 2})));
  EXPECT_THAT(
      GetStrokeNormal(
          {.velocity = {0, 1}, .acceleration = {-1, 0}, .time = Time(1)},
          Time(0)),
      Optional(Vec2Near({-1.7071, -0.7071}, 1e-4)));
  EXPECT_THAT(
      GetStrokeNormal(
          {.velocity = {1, 0}, .acceleration = {3, 4}, .time = Time(0.2)},
          Time(0.1)),
      Optional(Vec2Near({-0.2941, 1.9558}, 1e-4)));
  EXPECT_THAT(
      GetStrokeNormal(
          {.velocity = {3, 0}, .acceleration = {-3, 0}, .time = Time(0.2)},
          Time(0.1)),
      Optional(Vec2Near({0, 3}, 1e-4)));
}

TEST(UtilsTest, ProjectToSegmentAlongNormal) {
  EXPECT_EQ(ProjectToSegmentAlongNormal({1, 1}, {3, 1}, {2, 0}, {0, 1}), 0.5);
  EXPECT_EQ(ProjectToSegmentAlongNormal({1, 1}, {5, 1}, {4, 0}, {0, 1}), 0.75);
  EXPECT_EQ(ProjectToSegmentAlongNormal({1, 1}, {1, 5}, {0, 2}, {1, 0}), 0.25);

  EXPECT_EQ(ProjectToSegmentAlongNormal({1, 1}, {5, 1}, {0, 1}, {0, 0}),
            std::nullopt);
  EXPECT_EQ(ProjectToSegmentAlongNormal({1, 1}, {5, 1}, {0, 2}, {0, 1}),
            std::nullopt);
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
