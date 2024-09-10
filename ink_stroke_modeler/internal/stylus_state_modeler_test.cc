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

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/numbers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

constexpr float kTol = 1e-5;
constexpr float kAccelTol = 1e-3;
constexpr StylusState kUnknownState{
    .pressure = -1, .tilt = -1, .orientation = -1};
const Result kUnknownResult{.position = {0, 0},
                            .velocity = {0, 0},
                            .acceleration = {0, 0},
                            .time = Time(0),
                            .pressure = -1,
                            .tilt = -1,
                            .orientation = -1};
constexpr StylusStateModelerParams kNormalProjectionParams{
    .max_input_samples = 10,
    .use_stroke_normal_projection = true,
};

TEST(StylusStateModelerTest, QueryEmpty) {
  StylusStateModeler modeler;
  EXPECT_EQ(modeler.Query({0, 0}, std::optional<Vec2>({0, 1}), Time(0)),
            kUnknownResult);
  EXPECT_EQ(modeler.Query({-5, 3}, std::optional<Vec2>({0, 1}), Time(0.1)),
            kUnknownResult);
}

TEST(StylusStateModelerTest, QuerySingleInput) {
  StylusStateModeler modeler;
  modeler.Update({0, 0}, Time(0),
                 {.pressure = 0.75, .tilt = 0.75, .orientation = 0.75});
  EXPECT_THAT(modeler.Query({0, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .75,
                      .tilt = .75,
                      .orientation = .75,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, 1}), Time(0.1)),
              ResultNear({.position = {0, 0},
                          .velocity = {0, 0},
                          .acceleration = {0, 0},
                          .time = Time(0.1),
                          .pressure = .75,
                          .tilt = .75,
                          .orientation = .75},
                         kTol, kAccelTol));
}

TEST(StylusStateModelerTest, QuerySingleInputWithNormalProjection) {
  StylusStateModeler modeler;
  modeler.Reset(kNormalProjectionParams);
  modeler.Update({0, 0}, Time(0),
                 {.pressure = 0.75, .tilt = 0.75, .orientation = 0.75});
  EXPECT_THAT(modeler.Query({0, 0}, std::optional<Vec2>({0, 1.1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .75,
                      .tilt = .75,
                      .orientation = .75,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, 1.1}), Time(0.1)),
              ResultNear({.position = {0, 0},
                          .velocity = {0, 0},
                          .acceleration = {0, 0},
                          .time = Time(0.1),
                          .pressure = .75,
                          .tilt = .75,
                          .orientation = .75},
                         kTol, kAccelTol));
}

TEST(StylusStateModelerTest, QueryMultipleInputs) {
  StylusStateModeler modeler;
  modeler.Update({.5, 1.5}, Time(0),
                 {.pressure = .3, .tilt = .8, .orientation = .1});
  modeler.Update({2, 1.5}, Time(0.1),
                 {.pressure = .6, .tilt = .5, .orientation = .7});
  modeler.Update({3, 3.5}, Time(0.2),
                 {.pressure = .8, .tilt = .1, .orientation = .3});
  modeler.Update({3.5, 4}, Time(0.3),
                 {.pressure = .2, .tilt = .2, .orientation = .2});

  EXPECT_THAT(modeler.Query({0, 2}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {.5, 1.5},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .3,
                      .tilt = .8,
                      .orientation = .1,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 2}, std::optional<Vec2>({0, -0.5}), Time(0.1)),
              ResultNear({.position = {1, 1.5},
                          .velocity = {5, 0},
                          .acceleration = {50, 0},
                          .time = Time(0.1),
                          .pressure = .4,
                          .tilt = .7,
                          .orientation = .3},
                         kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2, 1.5}, std::optional<Vec2>({2, 2}), Time(0.1)),
              ResultNear(
                  {
                      .position = {2, 1.5},
                      .velocity = {15, 0},
                      .acceleration = {150, 0},
                      .time = Time(0.1),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .7,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2.5, 1.875}, std::optional<Vec2>({-0.25, 0.125}),
                            Time(0.2)),
              ResultNear(
                  {
                      .position = {2.25, 2},
                      .velocity = {13.75, 5},
                      .acceleration = {100, 50},
                      .time = Time(0.2),
                      .pressure = .65,
                      .tilt = .4,
                      .orientation = .6,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2.5, 3.125}, std::optional<Vec2>({0.25, -0.125}),
                            Time(0.22)),
              ResultNear(
                  {
                      .position = {2.75, 3},
                      .velocity = {11.25, 15},
                      .acceleration = {0, 150},
                      .time = Time(0.22),
                      .pressure = .75,
                      .tilt = .2,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({2.5, 4}, std::optional<Vec2>({0.5, -0.5}), Time(0.25)),
      ResultNear(
          {
              .position = {3, 3.5},
              .velocity = {10, 20},
              .acceleration = {-50, 200},
              .time = Time(0.25),
              .pressure = .8,
              .tilt = .1,
              .orientation = .3,
          },
          kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({3, 4}, std::optional<Vec2>({0.25, -0.25}), Time(0.29)),
      ResultNear(
          {
              .position = {3.25, 3.75},
              .velocity = {7.5, 12.5},
              .acceleration = {-50, 25},
              .time = Time(0.29),
              .pressure = .5,
              .tilt = .15,
              .orientation = .25,
          },
          kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({4, 4}, std::optional<Vec2>({0, 1}), Time(0.31)),
              ResultNear(
                  {
                      .position = {3.5, 4},
                      .velocity = {5, 5},
                      .acceleration = {-50, -150},
                      .time = Time(0.31),
                      .pressure = .2,
                      .tilt = .2,
                      .orientation = .2,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, QueryMultipleInputsWithNormalProjection) {
  StylusStateModeler modeler;
  modeler.Reset(kNormalProjectionParams);
  modeler.Update({.5, 1.5}, Time(0),
                 {.pressure = .3, .tilt = .8, .orientation = .1});
  modeler.Update({2, 1.5}, Time(0.1),
                 {.pressure = .6, .tilt = .5, .orientation = .7});
  modeler.Update({3, 3.5}, Time(0.2),
                 {.pressure = .8, .tilt = .1, .orientation = .3});
  modeler.Update({3.5, 4}, Time(0.3),
                 {.pressure = .2, .tilt = .2, .orientation = .2});

  EXPECT_THAT(modeler.Query({0, 2}, std::optional<Vec2>({0, 1.1}), Time(0)),
              ResultNear(
                  {
                      .position = {.5, 1.5},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .3,
                      .tilt = .8,
                      .orientation = .1,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 2}, std::optional<Vec2>({0, -0.55}), Time(0.1)),
              ResultNear({.position = {1, 1.5},
                          .velocity = {5, 0},
                          .acceleration = {50, 0},
                          .time = Time(0.1),
                          .pressure = .4,
                          .tilt = .7,
                          .orientation = .3},
                         kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2, 1.5}, std::optional<Vec2>({2, 2.1}), Time(0.1)),
              ResultNear(
                  {
                      .position = {2, 1.5},
                      .velocity = {15, 0},
                      .acceleration = {150, 0},
                      .time = Time(0.1),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .7,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2.5, 1.875}, std::optional<Vec2>({-0.3, 0.125}),
                            Time(0.2)),
              ResultNear({.position = {2.24138, 1.98276},
                          .velocity = {13.79310, 4.82759},
                          .acceleration = {101.72414, 48.27586},
                          .time = Time(0.2),
                          .pressure = 0.648276,
                          .tilt = 0.403448,
                          .orientation = 0.603448},
                         kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({2.5, 3.125}, std::optional<Vec2>({0.3, -0.125}),
                            Time(0.22)),
              ResultNear({.position = {2.75862, 3.01724},
                          .velocity = {11.20690, 15.17241},
                          .acceleration = {-1.72414, 151.72414},
                          .time = Time(0.22),
                          .pressure = 0.751724,
                          .tilt = 0.196552,
                          .orientation = 0.396552},
                         kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({2.5, 4}, std::optional<Vec2>({0.5, -0.55}), Time(0.25)),
      ResultNear({.position = {2.98387, 3.46774},
                  .velocity = {10.08064, 19.67742},
                  .acceleration = {-46.77420, 196.77420},
                  .time = Time(0.25),
                  .pressure = 0.796774,
                  .tilt = 0.106452,
                  .orientation = 0.306452},
                 kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({3, 4}, std::optional<Vec2>({0.3, -0.25}), Time(0.29)),
      ResultNear({.position = {3.27273, 3.77273},
                  .velocity = {7.27273, 11.81818},
                  .acceleration = {-50.00000, 9.09090},
                  .time = Time(0.29),
                  .pressure = 0.472727,
                  .tilt = 0.154545,
                  .orientation = 0.245455},
                 kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({4, 4}, std::optional<Vec2>({0, 1.1}), Time(0.31)),
              ResultNear(
                  {
                      .position = {3.5, 4},
                      .velocity = {5, 5},
                      .acceleration = {-50, -150},
                      .time = Time(0.31),
                      .pressure = .2,
                      .tilt = .2,
                      .orientation = .2,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, QueryStaleInputsAreDiscarded) {
  StylusStateModeler modeler;
  modeler.Update({1, 1}, Time(0),
                 {.pressure = .6, .tilt = .5, .orientation = .4});
  modeler.Update({-1, 2}, Time(0.1),
                 {.pressure = .3, .tilt = .7, .orientation = .6});
  modeler.Update({-4, 0}, Time(0.2),
                 {.pressure = .9, .tilt = .7, .orientation = .3});
  modeler.Update({-6, -3}, Time(0.3),
                 {.pressure = .4, .tilt = .3, .orientation = .5});
  modeler.Update({-5, -5}, Time(0.4),
                 {.pressure = .3, .tilt = .3, .orientation = .1});
  modeler.Update({-3, -4}, Time(0.5),
                 {.pressure = .6, .tilt = .8, .orientation = .3});
  modeler.Update({-6, -7}, Time(0.6),
                 {.pressure = .9, .tilt = .8, .orientation = .1});
  modeler.Update({-9, -8}, Time(0.7),
                 {.pressure = .8, .tilt = .2, .orientation = .2});
  modeler.Update({-11, -5}, Time(0.8),
                 {.pressure = .2, .tilt = .4, .orientation = .7});
  modeler.Update({-10, -2}, Time(0.9),
                 {.pressure = .7, .tilt = .3, .orientation = .2});

  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {1, 1},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 3.5}, std::optional<Vec2>({-1, -2}), Time(0.1)),
              ResultNear(
                  {
                      .position = {0, 1.5},
                      .velocity = {-10, 5},
                      .acceleration = {-100, 50},
                      .time = Time(0.1),
                      .pressure = .45,
                      .tilt = .6,
                      .orientation = .5,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}, std::optional<Vec2>({1, -1.5}), Time(0.2)),
      ResultNear(
          {
              .position = {-2, 4. / 3.},
              .velocity = {-70. / 3., 0},
              .acceleration = {-166.666656, -33.33334},
              .time = Time(0.2),
              .pressure = .5,
              .tilt = .7,
              .orientation = .5,
          },
          kTol, kAccelTol));

  // This causes the point at {1, 1} to be discarded.
  modeler.Update({-8, 0}, Time(1),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0.1)),
              ResultNear(
                  {
                      .position = {-1, 2},
                      .velocity = {-20, 10},
                      .acceleration = {-200, 100},
                      .time = Time(0.1),
                      .pressure = .3,
                      .tilt = .7,
                      .orientation = .6,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 3.5}, std::optional<Vec2>({0, 1}), Time(0.2)),
              ResultNear(
                  {
                      .position = {-1, 2},
                      .velocity = {-20, 10},
                      .acceleration = {-200, 100},
                      .time = Time(0.2),
                      .pressure = .3,
                      .tilt = .7,
                      .orientation = .6,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}, std::optional<Vec2>({1, -1.5}), Time(0.3)),
      ResultNear(
          {
              .position = {-2, 4. / 3.},
              .velocity = {-70. / 3., 0},
              .acceleration = {-166.666656, -33.33334},
              .time = Time(0.3),
              .pressure = .5,
              .tilt = .7,
              .orientation = .5,
          },
          kTol, kAccelTol));

  // This causes the point at {-1, 2} to be discarded.
  modeler.Update({-8, 0}, Time(1.1),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0.3)),
              ResultNear(
                  {
                      .position = {-4, 0},
                      .velocity = {-30, -20},
                      .acceleration = {-100, -300},
                      .time = Time(0.3),
                      .pressure = .9,
                      .tilt = .7,
                      .orientation = .3,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({1, 3.5}, std::optional<Vec2>({0, 1}), Time(0.4)),
              ResultNear(
                  {
                      .position = {-4, 0},
                      .velocity = {-30, -20},
                      .acceleration = {-100, -300},
                      .time = Time(0.4),
                      .pressure = .9,
                      .tilt = .7,
                      .orientation = .3,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}, std::optional<Vec2>({-6, 2}), Time(0.5)),
      ResultNear(
          {
              .position = {-4, 0},
              .velocity = {-30, -20},
              .acceleration = {-100, -300},
              .time = Time(0.5),
              .pressure = .9,
              .tilt = .7,
              .orientation = .3,
          },
          kTol, kAccelTol));
}

TEST(StylusStateModelerTest, QueryCyclicOrientationInterpolation) {
  StylusStateModeler modeler;
  modeler.Update({0, 0}, Time(0),
                 {.pressure = 0, .tilt = 0, .orientation = 1.8 * kPi});
  modeler.Update({0, 1}, Time(1),
                 {.pressure = 0, .tilt = 0, .orientation = .2 * kPi});
  modeler.Update({0, 2}, Time(2),
                 {.pressure = 0, .tilt = 0, .orientation = 1.6 * kPi});

  EXPECT_NEAR(
      modeler.Query({0, .25}, std::optional<Vec2>({1, 0}), Time(0)).orientation,
      1.9 * kPi, kTol);
  EXPECT_NEAR(
      modeler.Query({0, .75}, std::optional<Vec2>({1, 0}), Time(1)).orientation,
      .1 * kPi, kTol);
  EXPECT_NEAR(modeler.Query({0, 1.25}, std::optional<Vec2>({1, 0}), Time(1.5))
                  .orientation,
              .05 * kPi, kTol);
  EXPECT_NEAR(modeler.Query({0, 1.75}, std::optional<Vec2>({1, 0}), Time(2))
                  .orientation,
              1.75 * kPi, kTol);
}

TEST(StylusStateModelerTest, QueryAndReset) {
  StylusStateModeler modeler;

  modeler.Update({4, 5}, Time(0),
                 {.pressure = .4, .tilt = .9, .orientation = .1});
  modeler.Update({7, 8}, Time(1),
                 {.pressure = .1, .tilt = .2, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({10, 12}, std::optional<Vec2>({0.5, -0.5}), Time(0)),
      ResultNear(
          {
              .position = {7, 8},
              .velocity = {3, 3},
              .acceleration = {3, 3},
              .time = Time(0),
              .pressure = .1,
              .tilt = .2,
              .orientation = .5,
          },
          kTol, kAccelTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_EQ(modeler.Query({10, 12}, std::optional<Vec2>({0.5, -0.5}), Time(0)),
            kUnknownResult);

  modeler.Update({-1, 4}, Time(2),
                 {.pressure = .4, .tilt = .6, .orientation = .8});
  EXPECT_THAT(modeler.Query({6, 7}, std::optional<Vec2>({-7, -3}), Time(2)),
              ResultNear(
                  {
                      .position = {-1, 4},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(2),
                      .pressure = .4,
                      .tilt = .6,
                      .orientation = .8,
                  },
                  kTol, kAccelTol));

  modeler.Update({-3, 0}, Time(3),
                 {.pressure = .7, .tilt = .2, .orientation = .5});
  EXPECT_THAT(modeler.Query({-2, 2}, std::optional<Vec2>({0, 1}), Time(2.5)),
              ResultNear(
                  {
                      .position = {-2, 2},
                      .velocity = {-1, -2},
                      .acceleration = {-1, -2},
                      .time = Time(2.5),
                      .pressure = .55,
                      .tilt = .4,
                      .orientation = .65,
                  },
                  kTol, kAccelTol));
  EXPECT_THAT(modeler.Query({0, 5}, std::optional<Vec2>({-0.4, 0.2}), Time(0)),
              ResultNear(
                  {
                      .position = {-1, 4},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .4,
                      .tilt = .6,
                      .orientation = .8,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, UpdateWithUnknownState) {
  StylusStateModeler modeler;

  modeler.Update({1, 2}, Time(0),
                 {.pressure = .1, .tilt = .2, .orientation = .3});
  modeler.Update({2, 3}, Time(1),
                 {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_THAT(modeler.Query({2, 2}, std::optional<Vec2>({-0.5, 0.5}), Time(0)),
              ResultNear({.position = {1.5, 2.5},
                          .velocity = {0.5, 0.5},
                          .acceleration = {0.5, 0.5},
                          .time = Time(0),
                          .pressure = .2,
                          .tilt = .3,
                          .orientation = .4},
                         kTol, kAccelTol));

  modeler.Update({5, 5}, Time(2), kUnknownState);
  EXPECT_EQ(modeler.Query({5, 5}, std::optional<Vec2>({-0.5, 0.5}), Time(1)),
            kUnknownResult);

  modeler.Update({2, 3}, Time(3),
                 {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_EQ(modeler.Query({1, 2}, std::optional<Vec2>({-0.5, 0.5}), Time(2)),
            kUnknownResult);

  modeler.Update({-1, 3}, Time(4), kUnknownState);
  EXPECT_EQ(modeler.Query({7, 9}, std::optional<Vec2>({-0.5, 0.5}), Time(3)),
            kUnknownResult);

  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({3, 3}, Time(5),
                 {.pressure = .7, .tilt = .6, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({3, 3}, std::optional<Vec2>({-0.5, 0.5}), Time(0.2)),
      ResultNear({.position = {3, 3},
                  .velocity = {0, 0},
                  .acceleration = {0, 0},
                  .time = Time(0.2),
                  .pressure = .7,
                  .tilt = .6,
                  .orientation = .5},
                 kTol, kAccelTol));
}

TEST(StylusStateModelerTest, StrokeNormalIgnored) {
  StylusStateModeler modeler;

  modeler.Update({4, 5}, Time(0),
                 {.pressure = .4, .tilt = .9, .orientation = .1});
  modeler.Update({7, 8}, Time(1),
                 {.pressure = .1, .tilt = .2, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({5, 7}, std::optional<Vec2>({0.5, -0.5}), Time(0.2)),
      ResultNear(modeler.Query({5, 7}, std::optional<Vec2>({0, 1}), Time(0.2)),
                 kTol, kAccelTol));
}

TEST(StylusStateModelerTest, ModelPressureOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, Time(0),
                 {.pressure = .5, .tilt = -2, .orientation = -.1});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .5,
                      .tilt = -1,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Update({2, 0}, Time(1),
                 {.pressure = .7, .tilt = -2, .orientation = -.1});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, -1}), Time(1)),
              ResultNear(
                  {
                      .position = {1, 0},
                      .velocity = {1, 0},
                      .acceleration = {1, 0},
                      .time = Time(1),
                      .pressure = .6,
                      .tilt = -1,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, ModelTiltOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, Time(0),
                 {.pressure = -2, .tilt = .5, .orientation = -.1});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = -1,
                      .tilt = .5,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Update({2, 0}, Time(1),
                 {.pressure = -2, .tilt = .3, .orientation = -.1});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, -1}), Time(1)),
              ResultNear(
                  {
                      .position = {1, 0},
                      .velocity = {1, 0},
                      .acceleration = {1, 0},
                      .time = Time(1),
                      .pressure = -1,
                      .tilt = .4,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, ModelOrientationOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, Time(0),
                 {.pressure = -2, .tilt = -.1, .orientation = 1});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = -1,
                      .tilt = -1,
                      .orientation = 1,
                  },
                  kTol, kAccelTol));

  modeler.Update({2, 0}, Time(1),
                 {.pressure = -2, .tilt = -.3, .orientation = 2});
  EXPECT_THAT(modeler.Query({1, 1}, std::optional<Vec2>({0, -1}), Time(1)),
              ResultNear(
                  {
                      .position = {1, 0},
                      .velocity = {1, 0},
                      .acceleration = {1, 0},
                      .time = Time(1),
                      .pressure = -1,
                      .tilt = -1,
                      .orientation = 1.5,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, DropFieldsOneByOne) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, Time(0),
                 {.pressure = .5, .tilt = .5, .orientation = .5});
  EXPECT_THAT(modeler.Query({1, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .5,
                      .tilt = .5,
                      .orientation = .5,
                  },
                  kTol, kAccelTol));

  modeler.Update({2, 0}, Time(1),
                 {.pressure = .3, .tilt = .7, .orientation = -1});
  EXPECT_THAT(modeler.Query({1, 0}, std::optional<Vec2>({0, 1}), Time(1)),
              ResultNear(
                  {
                      .position = {1, 0},
                      .velocity = {1, 0},
                      .acceleration = {1, 0},
                      .time = Time(1),
                      .pressure = .4,
                      .tilt = .6,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Update({4, 0}, Time(2),
                 {.pressure = .1, .tilt = -1, .orientation = 1});
  EXPECT_THAT(modeler.Query({3, 0}, std::optional<Vec2>({0, 1}), Time(2)),
              ResultNear(
                  {
                      .position = {3, 0},
                      .velocity = {2, 0},
                      .acceleration = {1, 0},
                      .time = Time(2),
                      .pressure = .2,
                      .tilt = -1,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Update({6, 0}, Time(3),
                 {.pressure = -1, .tilt = .2, .orientation = 0});
  EXPECT_THAT(modeler.Query({5, 0}, std::optional<Vec2>({0, 1}), Time(3)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = -1,
                      .tilt = -1,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Update({8, 0}, Time(4),
                 {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_THAT(modeler.Query({7, 0}, std::optional<Vec2>({0, 1}), Time(4)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = -1,
                      .tilt = -1,
                      .orientation = -1,
                  },
                  kTol, kAccelTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_THAT(modeler.Query({1, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(kUnknownResult, kTol, kAccelTol));

  modeler.Update({0, 0}, Time(5),
                 {.pressure = .1, .tilt = .8, .orientation = .3});
  EXPECT_THAT(modeler.Query({1, 0}, std::optional<Vec2>({0, 1}), Time(5)),
              ResultNear(
                  {
                      .position = {0, 0},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(5),
                      .pressure = .1,
                      .tilt = .8,
                      .orientation = .3,
                  },
                  kTol, kAccelTol));
}

TEST(StylusStateModelerTest, SaveAndRestore) {
  StylusStateModeler modeler;
  modeler.Update({1, 1}, Time(0),
                 {.pressure = .6, .tilt = .5, .orientation = .4});
  modeler.Update({-1, 2}, Time(1),
                 {.pressure = .3, .tilt = .7, .orientation = .6});
  modeler.Update({-4, 0}, Time(2),
                 {.pressure = .9, .tilt = .7, .orientation = .3});
  modeler.Update({-6, -3}, Time(3),
                 {.pressure = .4, .tilt = .3, .orientation = .5});
  modeler.Update({-5, -5}, Time(4),
                 {.pressure = .3, .tilt = .3, .orientation = .1});
  modeler.Update({-3, -4}, Time(5),
                 {.pressure = .6, .tilt = .8, .orientation = .3});
  modeler.Update({-6, -7}, Time(6),
                 {.pressure = .9, .tilt = .8, .orientation = .1});
  modeler.Update({-9, -8}, Time(7),
                 {.pressure = .8, .tilt = .2, .orientation = .2});
  modeler.Update({-11, -5}, Time(8),
                 {.pressure = .2, .tilt = .4, .orientation = .7});
  modeler.Update({-10, -2}, Time(9),
                 {.pressure = .7, .tilt = .3, .orientation = .2});

  ASSERT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {1, 1},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));

  // Calling restore with no save should have no effect.
  modeler.Restore();

  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {1, 1},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));

  modeler.Save();

  // This causes the points at {1, 1} and {-1, 2} to be discarded.
  modeler.Update({-8, 0}, Time(10),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  modeler.Update({-8, 0}, Time(11),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {-4, 0},
                      .velocity = {-3, -2},
                      .acceleration = {-1, -3},
                      .time = Time(0),
                      .pressure = .9,
                      .tilt = .7,
                      .orientation = .3,
                  },
                  kTol, kAccelTol));

  // Restoring should revert the updates.
  modeler.Restore();
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {1, 1},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));

  // Restoring should not have cleared the saved state, so we can repeat the
  // action.
  modeler.Update({-8, 0}, Time(12),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  modeler.Update({-8, 0}, Time(13),
                 {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {-4, 0},
                      .velocity = {-3, -2},
                      .acceleration = {-1, -3},
                      .time = Time(0),
                      .pressure = .9,
                      .tilt = .7,
                      .orientation = .3,
                  },
                  kTol, kAccelTol));
  modeler.Restore();
  EXPECT_THAT(modeler.Query({2, 0}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {1, 1},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .6,
                      .tilt = .5,
                      .orientation = .4,
                  },
                  kTol, kAccelTol));

  // Calling Reset should clear the save point so that calling Restore should
  // have no effect.
  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({-1, 4}, Time(14),
                 {.pressure = .4, .tilt = .6, .orientation = .8});
  EXPECT_THAT(modeler.Query({6, 7}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {-1, 4},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .4,
                      .tilt = .6,
                      .orientation = .8,
                  },
                  kTol, kAccelTol));
  modeler.Restore();
  EXPECT_THAT(modeler.Query({6, 7}, std::optional<Vec2>({0, 1}), Time(0)),
              ResultNear(
                  {
                      .position = {-1, 4},
                      .velocity = {0, 0},
                      .acceleration = {0, 0},
                      .time = Time(0),
                      .pressure = .4,
                      .tilt = .6,
                      .orientation = .8,
                  },
                  kTol, kAccelTol));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
