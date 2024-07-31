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
#include "ink_stroke_modeler/numbers.h"
#include "ink_stroke_modeler/params.h"

namespace ink {
namespace stroke_model {
namespace {

constexpr float kTol = 1e-5;
constexpr StylusState kUnknown{.pressure = -1,
                               .tilt = -1,
                               .orientation = -1,
                               .projected_position = {0, 0}};

TEST(StylusStateModelerTest, QueryEmpty) {
  StylusStateModeler modeler;
  EXPECT_EQ(modeler.Query({0, 0}), kUnknown);
  EXPECT_EQ(modeler.Query({-5, 3}), kUnknown);
}

TEST(StylusStateModelerTest, QuerySingleInput) {
  StylusStateModeler modeler;
  modeler.Update({.pressure = 0.75,
                  .tilt = 0.75,
                  .orientation = 0.75,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({0, 0}),
              StylusStateNear(
                  {.pressure = .75, .tilt = .75, .orientation = .75}, kTol));
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear(
                  {.pressure = .75, .tilt = .75, .orientation = .75}, kTol));
}

TEST(StylusStateModelerTest, QueryMultipleInputs) {
  StylusStateModeler modeler;
  modeler.Update({.pressure = .3,
                  .tilt = .8,
                  .orientation = .1,
                  .projected_position = {.5, 1.5}});
  modeler.Update({.pressure = .6,
                  .tilt = .5,
                  .orientation = .7,
                  .projected_position = {2, 1.5}});
  modeler.Update({.pressure = .8,
                  .tilt = .1,
                  .orientation = .3,
                  .projected_position = {3, 3.5}});
  modeler.Update({.pressure = .2,
                  .tilt = .2,
                  .orientation = .2,
                  .projected_position = {3.5, 4}});

  EXPECT_THAT(modeler.Query({0, 2}),
              StylusStateNear({.pressure = .3,
                               .tilt = .8,
                               .orientation = .1,
                               .projected_position = {.5, 1.5}},
                              kTol));
  EXPECT_THAT(modeler.Query({1, 2}),
              StylusStateNear({.pressure = .4,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {1, 1.5}},
                              kTol));
  EXPECT_THAT(modeler.Query({2, 1.5}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .7,
                               .projected_position = {2, 1.5}},
                              kTol));
  EXPECT_THAT(modeler.Query({2.5, 1.875}),
              StylusStateNear({.pressure = .65,
                               .tilt = .4,
                               .orientation = .6,
                               .projected_position = {2.25, 2}},
                              kTol));
  EXPECT_THAT(modeler.Query({2.5, 3.125}),
              StylusStateNear({.pressure = .75,
                               .tilt = .2,
                               .orientation = .4,
                               .projected_position = {2.75, 3}},
                              kTol));
  EXPECT_THAT(modeler.Query({2.5, 4}),
              StylusStateNear({.pressure = .8,
                               .tilt = .1,
                               .orientation = .3,
                               .projected_position = {3, 3.5}},
                              kTol));
  EXPECT_THAT(modeler.Query({3, 4}),
              StylusStateNear({.pressure = .5,
                               .tilt = .15,
                               .orientation = .25,
                               .projected_position = {3.25, 3.75}},
                              kTol));
  EXPECT_THAT(modeler.Query({4, 4}),
              StylusStateNear({.pressure = .2,
                               .tilt = .2,
                               .orientation = .2,
                               .projected_position = {3.5, 4}},
                              kTol));
}

TEST(StylusStateModelerTest, QueryStaleInputsAreDiscarded) {
  StylusStateModeler modeler;
  modeler.Update({.pressure = .6,
                  .tilt = .5,
                  .orientation = .4,
                  .projected_position = {1, 1}});
  modeler.Update({.pressure = .3,
                  .tilt = .7,
                  .orientation = .6,
                  .projected_position = {-1, 2}});
  modeler.Update({.pressure = .9,
                  .tilt = .7,
                  .orientation = .3,
                  .projected_position = {-4, 0}});
  modeler.Update({.pressure = .4,
                  .tilt = .3,
                  .orientation = .5,
                  .projected_position = {-6, -3}});
  modeler.Update({.pressure = .3,
                  .tilt = .3,
                  .orientation = .1,
                  .projected_position = {-5, -5}});
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .3,
                  .projected_position = {-3, -4}});
  modeler.Update({.pressure = .9,
                  .tilt = .8,
                  .orientation = .1,
                  .projected_position = {-6, -7}});
  modeler.Update({.pressure = .8,
                  .tilt = .2,
                  .orientation = .2,
                  .projected_position = {-9, -8}});
  modeler.Update({.pressure = .2,
                  .tilt = .4,
                  .orientation = .7,
                  .projected_position = {-11, -5}});
  modeler.Update({.pressure = .7,
                  .tilt = .3,
                  .orientation = .2,
                  .projected_position = {-10, -2}});

  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .4,
                               .projected_position = {1, 1}},
                              kTol));
  EXPECT_THAT(modeler.Query({1, 3.5}),
              StylusStateNear({.pressure = .45,
                               .tilt = .6,
                               .orientation = .5,
                               .projected_position = {0, 1.5}},
                              kTol));
  EXPECT_THAT(modeler.Query({-3, 17. / 6}),
              StylusStateNear({.pressure = .5,
                               .tilt = .7,
                               .orientation = .5,
                               .projected_position = {-2, 4. / 3.}},
                              kTol));

  // This causes the point at {1, 1} to be discarded.
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .3,
                               .tilt = .7,
                               .orientation = .6,
                               .projected_position = {-1, 2}},
                              kTol));
  EXPECT_THAT(modeler.Query({1, 3.5}),
              StylusStateNear({.pressure = .3,
                               .tilt = .7,
                               .orientation = .6,
                               .projected_position = {-1, 2}},
                              kTol));
  EXPECT_THAT(modeler.Query({-3, 17. / 6}),
              StylusStateNear({.pressure = .5,
                               .tilt = .7,
                               .orientation = .5,
                               .projected_position = {-2, 4. / 3.}},
                              kTol));

  // This causes the point at {-1, 2} to be discarded.
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .9,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {-4, 0}},
                              kTol));
  EXPECT_THAT(modeler.Query({1, 3.5}),
              StylusStateNear({.pressure = .9,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {-4, 0}},
                              kTol));
  EXPECT_THAT(modeler.Query({-3, 17. / 6}),
              StylusStateNear({.pressure = .9,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {-4, 0}},
                              kTol));
}

TEST(StylusStateModelerTest, QueryCyclicOrientationInterpolation) {
  StylusStateModeler modeler;
  modeler.Update({.pressure = 0,
                  .tilt = 0,
                  .orientation = 1.8 * kPi,
                  .projected_position = {0, 0}});
  modeler.Update({.pressure = 0,
                  .tilt = 0,
                  .orientation = .2 * kPi,
                  .projected_position = {0, 1}});
  modeler.Update({.pressure = 0,
                  .tilt = 0,
                  .orientation = 1.6 * kPi,
                  .projected_position = {0, 2}});

  EXPECT_NEAR(modeler.Query({0, .25}).orientation, 1.9 * kPi, 1e-5);
  EXPECT_NEAR(modeler.Query({0, .75}).orientation, .1 * kPi, 1e-5);
  EXPECT_NEAR(modeler.Query({0, 1.25}).orientation, .05 * kPi, 1e-5);
  EXPECT_NEAR(modeler.Query({0, 1.75}).orientation, 1.75 * kPi, 1e-5);
}

TEST(StylusStateModelerTest, QueryAndReset) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = .4,
                  .tilt = .9,
                  .orientation = .1,
                  .projected_position = {4, 5}});
  modeler.Update({.pressure = .1,
                  .tilt = .2,
                  .orientation = .5,
                  .projected_position = {7, 8}});
  EXPECT_THAT(modeler.Query({10, 12}),
              StylusStateNear({.pressure = .1,
                               .tilt = .2,
                               .orientation = .5,
                               .projected_position = {7, 8}},
                              kTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_EQ(modeler.Query({10, 12}), kUnknown);

  modeler.Update({.pressure = .4,
                  .tilt = .6,
                  .orientation = .8,
                  .projected_position = {-1, 4}});
  EXPECT_THAT(modeler.Query({6, 7}),
              StylusStateNear({.pressure = .4,
                               .tilt = .6,
                               .orientation = .8,
                               .projected_position = {-1, 4}},
                              kTol));

  modeler.Update({.pressure = .7,
                  .tilt = .2,
                  .orientation = .5,
                  .projected_position = {-3, 0}});
  EXPECT_THAT(modeler.Query({-2, 2}),
              StylusStateNear({.pressure = .55,
                               .tilt = .4,
                               .orientation = .65,
                               .projected_position = {-2, 2}},
                              kTol));
  EXPECT_THAT(modeler.Query({0, 5}),
              StylusStateNear({.pressure = .4,
                               .tilt = .6,
                               .orientation = .8,
                               .projected_position = {-1, 4}},
                              kTol));
}

TEST(StylusStateModelerTest, UpdateWithUnknownState) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = .1,
                  .tilt = .2,
                  .orientation = .3,
                  .projected_position = {1, 2}});
  modeler.Update({.pressure = .3,
                  .tilt = .4,
                  .orientation = .5,
                  .projected_position = {2, 3}});
  EXPECT_THAT(modeler.Query({2, 2}),
              StylusStateNear({.pressure = .2,
                               .tilt = .3,
                               .orientation = .4,
                               .projected_position = {1.5, 2.5}},
                              kTol));

  StylusState unknown = kUnknown;
  unknown.projected_position = {5, 5};
  modeler.Update(unknown);
  EXPECT_EQ(modeler.Query({5, 5}), unknown);

  modeler.Update({.pressure = .3,
                  .tilt = .4,
                  .orientation = .5,
                  .projected_position = {2, 3}});
  unknown.projected_position = {1, 2};
  EXPECT_EQ(modeler.Query({1, 2}), unknown);

  unknown.projected_position = {-1, 3};
  modeler.Update(unknown);
  unknown.projected_position = {5, 5};
  EXPECT_EQ(modeler.Query({7, 9}), unknown);

  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({.pressure = .7,
                  .tilt = .6,
                  .orientation = .5,
                  .projected_position = {3, 3}});
  EXPECT_THAT(modeler.Query({3, 3}),
              StylusStateNear({.pressure = .7,
                               .tilt = .6,
                               .orientation = .5,
                               .projected_position = {3, 3}},
                              kTol));
}

TEST(StylusStateModelerTest, ModelPressureOnly) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = .5,
                  .tilt = -2,
                  .orientation = -.1,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = .5,
                               .tilt = -1,
                               .orientation = -1,
                               .projected_position = {0, 0}},
                              kTol));

  modeler.Update({.pressure = .7,
                  .tilt = -2,
                  .orientation = -.1,
                  .projected_position = {2, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = .6,
                               .tilt = -1,
                               .orientation = -1,
                               .projected_position = {1, 0}},
                              kTol));
}

TEST(StylusStateModelerTest, ModelTiltOnly) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = -2,
                  .tilt = .5,
                  .orientation = -.1,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = -1,
                               .tilt = .5,
                               .orientation = -1,
                               .projected_position = {0, 0}},
                              kTol));

  modeler.Update({.pressure = -2,
                  .tilt = .3,
                  .orientation = -.1,
                  .projected_position = {2, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = -1,
                               .tilt = .4,
                               .orientation = -1,
                               .projected_position = {1, 0}},
                              kTol));
}

TEST(StylusStateModelerTest, ModelOrientationOnly) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = -2,
                  .tilt = -.1,
                  .orientation = 1,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = -1,
                               .tilt = -1,
                               .orientation = 1,
                               .projected_position = {0, 0}},
                              kTol));

  modeler.Update({.pressure = -2,
                  .tilt = -.3,
                  .orientation = 2,
                  .projected_position = {2, 0}});
  EXPECT_THAT(modeler.Query({1, 1}),
              StylusStateNear({.pressure = -1,
                               .tilt = -1,
                               .orientation = 1.5,
                               .projected_position = {1, 0}},
                              kTol));
}

TEST(StylusStateModelerTest, DropFieldsOneByOne) {
  StylusStateModeler modeler;

  modeler.Update({.pressure = .5,
                  .tilt = .5,
                  .orientation = .5,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({1, 0}),
              StylusStateNear({.pressure = .5,
                               .tilt = .5,
                               .orientation = .5,
                               .projected_position = {0, 0}},
                              kTol));

  modeler.Update({.pressure = .3,
                  .tilt = .7,
                  .orientation = -1,
                  .projected_position = {2, 0}});
  EXPECT_THAT(modeler.Query({1, 0}),
              StylusStateNear({.pressure = .4,
                               .tilt = .6,
                               .orientation = -1,
                               .projected_position = {1, 0}},
                              kTol));

  modeler.Update({.pressure = .1,
                  .tilt = -1,
                  .orientation = 1,
                  .projected_position = {4, 0}});
  EXPECT_THAT(modeler.Query({3, 0}),
              StylusStateNear({.pressure = .2,
                               .tilt = -1,
                               .orientation = -1,
                               .projected_position = {3, 0}},
                              kTol));

  modeler.Update({.pressure = -1,
                  .tilt = .2,
                  .orientation = 0,
                  .projected_position = {6, 0}});
  StylusState unknown = kUnknown;
  unknown.projected_position = {5, 0};
  EXPECT_THAT(modeler.Query({5, 0}), StylusStateNear(unknown, kTol));

  modeler.Update({.pressure = .3,
                  .tilt = .4,
                  .orientation = .5,
                  .projected_position = {8, 0}});
  unknown.projected_position = {7, 0};
  EXPECT_THAT(modeler.Query({7, 0}), StylusStateNear(unknown, kTol));

  modeler.Reset(StylusStateModelerParams{});
  EXPECT_THAT(modeler.Query({1, 0}), StylusStateNear(kUnknown, kTol));

  modeler.Update({.pressure = .1,
                  .tilt = .8,
                  .orientation = .3,
                  .projected_position = {0, 0}});
  EXPECT_THAT(modeler.Query({1, 0}),
              StylusStateNear({.pressure = .1,
                               .tilt = .8,
                               .orientation = .3,
                               .projected_position = {0, 0}},
                              kTol));
}

TEST(StylusStateModelerTest, SaveAndRestore) {
  StylusStateModeler modeler;
  modeler.Update({.pressure = .6,
                  .tilt = .5,
                  .orientation = .4,
                  .projected_position = {1, 1}});
  modeler.Update({.pressure = .3,
                  .tilt = .7,
                  .orientation = .6,
                  .projected_position = {-1, 2}});
  modeler.Update({.pressure = .9,
                  .tilt = .7,
                  .orientation = .3,
                  .projected_position = {-4, 0}});
  modeler.Update({.pressure = .4,
                  .tilt = .3,
                  .orientation = .5,
                  .projected_position = {-6, -3}});
  modeler.Update({.pressure = .3,
                  .tilt = .3,
                  .orientation = .1,
                  .projected_position = {-5, -5}});
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .3,
                  .projected_position = {-3, -4}});
  modeler.Update({.pressure = .9,
                  .tilt = .8,
                  .orientation = .1,
                  .projected_position = {-6, -7}});
  modeler.Update({.pressure = .8,
                  .tilt = .2,
                  .orientation = .2,
                  .projected_position = {-9, -8}});
  modeler.Update({.pressure = .2,
                  .tilt = .4,
                  .orientation = .7,
                  .projected_position = {-11, -5}});
  modeler.Update({.pressure = .7,
                  .tilt = .3,
                  .orientation = .2,
                  .projected_position = {-10, -2}});

  ASSERT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .4,
                               .projected_position = {1, 1}},
                              kTol));

  // Calling restore with no save should have no effect.
  modeler.Restore();

  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .4,
                               .projected_position = {1, 1}},
                              kTol));

  modeler.Save();

  // This causes the points at {1, 1} and {-1, 2} to be discarded.
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .9,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {-4, 0}},
                              kTol));

  // Restoring should revert the updates.
  modeler.Restore();
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .4,
                               .projected_position = {1, 1}},
                              kTol));

  // Restoring should not have cleared the saved state, so we can repeat the
  // action.
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  modeler.Update({.pressure = .6,
                  .tilt = .8,
                  .orientation = .9,
                  .projected_position = {-8, 0}});
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .9,
                               .tilt = .7,
                               .orientation = .3,
                               .projected_position = {-4, 0}},
                              kTol));
  modeler.Restore();
  EXPECT_THAT(modeler.Query({2, 0}),
              StylusStateNear({.pressure = .6,
                               .tilt = .5,
                               .orientation = .4,
                               .projected_position = {1, 1}},
                              kTol));

  // Calling Reset should clear the save point so that calling Restore should
  // have no effect.
  modeler.Reset(StylusStateModelerParams{});
  modeler.Update({.pressure = .4,
                  .tilt = .6,
                  .orientation = .8,
                  .projected_position = {-1, 4}});
  EXPECT_THAT(modeler.Query({6, 7}),
              StylusStateNear({.pressure = .4,
                               .tilt = .6,
                               .orientation = .8,
                               .projected_position = {-1, 4}},
                              kTol));
  modeler.Restore();
  EXPECT_THAT(modeler.Query({6, 7}),
              StylusStateNear({.pressure = .4,
                               .tilt = .6,
                               .orientation = .8,
                               .projected_position = {-1, 4}},
                              kTol));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
