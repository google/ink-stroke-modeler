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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

constexpr float kTol = 1e-5;
constexpr StylusState kUnknown{.pressure = -1, .tilt = -1, .orientation = -1};

TEST(StylusStateModelerTest, QueryEmpty) {
  StylusStateModeler modeler;
  EXPECT_EQ(modeler.Query({0, 0}), kUnknown);
  EXPECT_EQ(modeler.Query({-5, 3}), kUnknown);
}

TEST(StylusStateModelerTest, QuerySingleInput) {
  StylusStateModeler modeler;
  modeler.Update({0, 0}, {.pressure = 0.75, .tilt = 0.75, .orientation = 0.75});
  EXPECT_THAT(modeler.Query({0, 0}),
              StylusStateNear(
                  {.pressure = .75, .tilt = .75, .orientation = .75}, kTol));
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear(
                  {.pressure = .75, .tilt = .75, .orientation = .75}, kTol));
}

TEST(StylusStateModelerTest, QueryMultipleInputs) {
  StylusStateModeler modeler;
  modeler.Update({.5, 1.5}, {.pressure = .3, .tilt = .8, .orientation = .1});
  modeler.Update({2, 1.5}, {.pressure = .6, .tilt = .5, .orientation = .7});
  modeler.Update({3, 3.5}, {.pressure = .8, .tilt = .1, .orientation = .3});
  modeler.Update({3.5, 4}, {.pressure = .2, .tilt = .2, .orientation = .2});

  EXPECT_THAT(
      modeler.Query({0, 2}),
      StylusStateNear({.pressure = .3, .tilt = .8, .orientation = .1}, kTol));
  EXPECT_THAT(
      modeler.Query({1, 2}),
      StylusStateNear({.pressure = .4, .tilt = .7, .orientation = .3}, kTol));
  EXPECT_THAT(
      modeler.Query({2, 1.5}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .7}, kTol));
  EXPECT_THAT(
      modeler.Query({2.5, 1.875}),
      StylusStateNear({.pressure = .65, .tilt = .4, .orientation = .6}, kTol));
  EXPECT_THAT(
      modeler.Query({2.5, 3.125}),
      StylusStateNear({.pressure = .75, .tilt = .2, .orientation = .4}, kTol));
  EXPECT_THAT(
      modeler.Query({2.5, 4}),
      StylusStateNear({.pressure = .8, .tilt = .1, .orientation = .3}, kTol));
  EXPECT_THAT(
      modeler.Query({3, 4}),
      StylusStateNear({.pressure = .5, .tilt = .15, .orientation = .25}, kTol));
  EXPECT_THAT(
      modeler.Query({4, 4}),
      StylusStateNear({.pressure = .2, .tilt = .2, .orientation = .2}, kTol));
}

TEST(StylusStateModelerTest, QueryStaleInputsAreDiscarded) {
  StylusStateModeler modeler;
  modeler.Update({1, 1}, {.pressure = .6, .tilt = .5, .orientation = .4});
  modeler.Update({-1, 2}, {.pressure = .3, .tilt = .7, .orientation = .6});
  modeler.Update({-4, 0}, {.pressure = .9, .tilt = .7, .orientation = .3});
  modeler.Update({-6, -3}, {.pressure = .4, .tilt = .3, .orientation = .5});
  modeler.Update({-5, -5}, {.pressure = .3, .tilt = .3, .orientation = .1});
  modeler.Update({-3, -4}, {.pressure = .6, .tilt = .8, .orientation = .3});
  modeler.Update({-6, -7}, {.pressure = .9, .tilt = .8, .orientation = .1});
  modeler.Update({-9, -8}, {.pressure = .8, .tilt = .2, .orientation = .2});
  modeler.Update({-11, -5}, {.pressure = .2, .tilt = .4, .orientation = .7});
  modeler.Update({-10, -2}, {.pressure = .7, .tilt = .3, .orientation = .2});

  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .4}, kTol));
  EXPECT_THAT(
      modeler.Query({1, 3.5}),
      StylusStateNear({.pressure = .45, .tilt = .6, .orientation = .5}, kTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}),
      StylusStateNear({.pressure = .5, .tilt = .7, .orientation = .5}, kTol));

  // This causes the point at {1, 1} to be discarded.
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .3, .tilt = .7, .orientation = .6}, kTol));
  EXPECT_THAT(
      modeler.Query({1, 3.5}),
      StylusStateNear({.pressure = .3, .tilt = .7, .orientation = .6}, kTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}),
      StylusStateNear({.pressure = .5, .tilt = .7, .orientation = .5}, kTol));

  // This causes the point at {-1, 2} to be discarded.
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .9, .tilt = .7, .orientation = .3}, kTol));
  EXPECT_THAT(
      modeler.Query({1, 3.5}),
      StylusStateNear({.pressure = .9, .tilt = .7, .orientation = .3}, kTol));
  EXPECT_THAT(
      modeler.Query({-3, 17. / 6}),
      StylusStateNear({.pressure = .9, .tilt = .7, .orientation = .3}, kTol));
}

TEST(StylusStateModelerTest, QueryCyclicOrientationInterpolation) {
  StylusStateModeler modeler;
  modeler.Update({0, 0}, {.pressure = 0, .tilt = 0, .orientation = 1.8 * M_PI});
  modeler.Update({0, 1}, {.pressure = 0, .tilt = 0, .orientation = .2 * M_PI});
  modeler.Update({0, 2}, {.pressure = 0, .tilt = 0, .orientation = 1.6 * M_PI});

  EXPECT_NEAR(modeler.Query({0, .25}).orientation, 1.9 * M_PI, 1e-5);
  EXPECT_NEAR(modeler.Query({0, .75}).orientation, .1 * M_PI, 1e-5);
  EXPECT_NEAR(modeler.Query({0, 1.25}).orientation, .05 * M_PI, 1e-5);
  EXPECT_NEAR(modeler.Query({0, 1.75}).orientation, 1.75 * M_PI, 1e-5);
}

TEST(StylusStateModelerTest, QueryAndReset) {
  StylusStateModeler modeler;

  modeler.Update({4, 5}, {.pressure = .4, .tilt = .9, .orientation = .1});
  modeler.Update({7, 8}, {.pressure = .1, .tilt = .2, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({10, 12}),
      StylusStateNear({.pressure = .1, .tilt = .2, .orientation = .5}, kTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_EQ(modeler.Query({10, 12}), kUnknown);

  modeler.Update({-1, 4}, {.pressure = .4, .tilt = .6, .orientation = .8});
  EXPECT_THAT(
      modeler.Query({6, 7}),
      StylusStateNear({.pressure = .4, .tilt = .6, .orientation = .8}, kTol));

  modeler.Update({-3, 0}, {.pressure = .7, .tilt = .2, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({-2, 2}),
      StylusStateNear({.pressure = .55, .tilt = .4, .orientation = .65}, kTol));
  EXPECT_THAT(
      modeler.Query({0, 5}),
      StylusStateNear({.pressure = .4, .tilt = .6, .orientation = .8}, kTol));
}

TEST(StylusStateModelerTest, UpdateWithUnknownState) {
  StylusStateModeler modeler;

  modeler.Update({1, 2}, {.pressure = .1, .tilt = .2, .orientation = .3});
  modeler.Update({2, 3}, {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({2, 2}),
      StylusStateNear({.pressure = .2, .tilt = .3, .orientation = .4}, kTol));

  modeler.Update({5, 5}, kUnknown);
  EXPECT_EQ(modeler.Query({5, 5}), kUnknown);

  modeler.Update({2, 3}, {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_EQ(modeler.Query({1, 2}), kUnknown);

  modeler.Update({-1, 3}, kUnknown);
  EXPECT_EQ(modeler.Query({7, 9}), kUnknown);

  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({3, 3}, {.pressure = .7, .tilt = .6, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({3, 3}),
      StylusStateNear({.pressure = .7, .tilt = .6, .orientation = .5}, kTol));
}

TEST(StylusStateModelerTest, ModelPressureOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, {.pressure = .5, .tilt = -2, .orientation = -.1});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = .5, .tilt = -1, .orientation = -1}, kTol));

  modeler.Update({2, 0}, {.pressure = .7, .tilt = -2, .orientation = -.1});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = .6, .tilt = -1, .orientation = -1}, kTol));
}

TEST(StylusStateModelerTest, ModelTiltOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, {.pressure = -2, .tilt = .5, .orientation = -.1});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = -1, .tilt = .5, .orientation = -1}, kTol));

  modeler.Update({2, 0}, {.pressure = -2, .tilt = .3, .orientation = -.1});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = -1, .tilt = .4, .orientation = -1}, kTol));
}

TEST(StylusStateModelerTest, ModelOrientationOnly) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, {.pressure = -2, .tilt = -.1, .orientation = 1});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = -1, .tilt = -1, .orientation = 1}, kTol));

  modeler.Update({2, 0}, {.pressure = -2, .tilt = -.3, .orientation = 2});
  EXPECT_THAT(
      modeler.Query({1, 1}),
      StylusStateNear({.pressure = -1, .tilt = -1, .orientation = 1.5}, kTol));
}

TEST(StylusStateModelerTest, DropFieldsOneByOne) {
  StylusStateModeler modeler;

  modeler.Update({0, 0}, {.pressure = .5, .tilt = .5, .orientation = .5});
  EXPECT_THAT(
      modeler.Query({1, 0}),
      StylusStateNear({.pressure = .5, .tilt = .5, .orientation = .5}, kTol));

  modeler.Update({2, 0}, {.pressure = .3, .tilt = .7, .orientation = -1});
  EXPECT_THAT(
      modeler.Query({1, 0}),
      StylusStateNear({.pressure = .4, .tilt = .6, .orientation = -1}, kTol));

  modeler.Update({4, 0}, {.pressure = .1, .tilt = -1, .orientation = 1});
  EXPECT_THAT(
      modeler.Query({3, 0}),
      StylusStateNear({.pressure = .2, .tilt = -1, .orientation = -1}, kTol));

  modeler.Update({6, 0}, {.pressure = -1, .tilt = .2, .orientation = 0});
  EXPECT_THAT(modeler.Query({5, 0}), StylusStateNear(kUnknown, kTol));

  modeler.Update({8, 0}, {.pressure = .3, .tilt = .4, .orientation = .5});
  EXPECT_THAT(modeler.Query({7, 0}), StylusStateNear(kUnknown, kTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_THAT(modeler.Query({1, 0}), StylusStateNear(kUnknown, kTol));

  modeler.Update({0, 0}, {.pressure = .1, .tilt = .8, .orientation = .3});
  EXPECT_THAT(
      modeler.Query({1, 0}),
      StylusStateNear({.pressure = .1, .tilt = .8, .orientation = .3}, kTol));
}

TEST(StylusStateModelerTest, SaveAndRestore) {
  StylusStateModeler modeler;
  modeler.Update({1, 1}, {.pressure = .6, .tilt = .5, .orientation = .4});
  modeler.Update({-1, 2}, {.pressure = .3, .tilt = .7, .orientation = .6});
  modeler.Update({-4, 0}, {.pressure = .9, .tilt = .7, .orientation = .3});
  modeler.Update({-6, -3}, {.pressure = .4, .tilt = .3, .orientation = .5});
  modeler.Update({-5, -5}, {.pressure = .3, .tilt = .3, .orientation = .1});
  modeler.Update({-3, -4}, {.pressure = .6, .tilt = .8, .orientation = .3});
  modeler.Update({-6, -7}, {.pressure = .9, .tilt = .8, .orientation = .1});
  modeler.Update({-9, -8}, {.pressure = .8, .tilt = .2, .orientation = .2});
  modeler.Update({-11, -5}, {.pressure = .2, .tilt = .4, .orientation = .7});
  modeler.Update({-10, -2}, {.pressure = .7, .tilt = .3, .orientation = .2});

  ASSERT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .4}, kTol));

  // Calling restore with no save should have no effect.
  modeler.Restore();

  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .4}, kTol));

  modeler.Save();

  // This causes the points at {1, 1} and {-1, 2} to be discarded.
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .9, .tilt = .7, .orientation = .3}, kTol));

  // Restoring should revert the updates.
  modeler.Restore();
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .4}, kTol));

  // Restoring should not have cleared the saved state, so we can repeat the
  // action.
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  modeler.Update({-8, 0}, {.pressure = .6, .tilt = .8, .orientation = .9});
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .9, .tilt = .7, .orientation = .3}, kTol));
  modeler.Restore();
  EXPECT_THAT(
      modeler.Query({2, 0}),
      StylusStateNear({.pressure = .6, .tilt = .5, .orientation = .4}, kTol));

  // Calling Reset should clear the save point so that calling Restore should
  // have no effect.
  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({-1, 4}, {.pressure = .4, .tilt = .6, .orientation = .8});
  EXPECT_THAT(
      modeler.Query({6, 7}),
      StylusStateNear({.pressure = .4, .tilt = .6, .orientation = .8}, kTol));
  modeler.Restore();
  EXPECT_THAT(
      modeler.Query({6, 7}),
      StylusStateNear({.pressure = .4, .tilt = .6, .orientation = .8}, kTol));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
