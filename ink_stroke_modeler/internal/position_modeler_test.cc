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

#include "ink_stroke_modeler/internal/position_modeler.h"

#include <cmath>
#include <iterator>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::ElementsAre;

const Duration kDefaultTimeStep(1. / 180);
constexpr float kTol = .00005;

// The expected position values are taken directly from results the old
// TipDynamics class. The expected velocity values are from the same source, but
// a multiplier of 300 (i.e. dt / (1 - drag)) had to be applied to account for
// the fact that PositionModeler uses the time step correctly.

TEST(PositionModelerTest, StraightLine) {
  PositionModeler modeler;
  Time current_time(0);
  modeler.Reset({{0, 0}, {0, 0}, current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({{.0909, 0}, {16.3636, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({2, 0}, current_time),
              TipStateNear({{.319, 0}, {41.0579, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({{.6996, 0}, {68.5055, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({{1.228, 0}, {95.1099, 0}, current_time}, kTol));
}

TEST(PositionModelerTest, ZigZag) {
  PositionModeler modeler;
  Time current_time(3);
  modeler.Reset({{-1, -1}, {0, 0}, current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({-.5, -1}, current_time),
              TipStateNear({{-.9545, -1}, {8.1818, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({-.5, -.5}, current_time),
      TipStateNear({{-.886, -.9545}, {12.3471, 8.1818}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({0, -.5}, current_time),
      TipStateNear({{-.7643, -.886}, {21.9056, 12.3471}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({0, 0}, current_time),
      TipStateNear({{-.6218, -.7643}, {25.6493, 21.9056}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({.5, 0}, current_time),
      TipStateNear({{-.4343, -.6218}, {33.7456, 25.6493}, current_time}, kTol));
}

TEST(PositionModelerTest, SharpTurn) {
  PositionModeler modeler;
  Time current_time(1.6);
  modeler.Reset({{0, 0}, {0, 0}, current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({.25, .25}, current_time),
      TipStateNear({{.0227, .0227}, {4.0909, 4.0909}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({.5, .5}, current_time),
      TipStateNear({{.0798, .0798}, {10.2645, 10.2645}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({.75, .75}, current_time),
      TipStateNear({{.1749, .1749}, {17.1264, 17.1264}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({1, 1}, current_time),
      TipStateNear({{.307, .307}, {23.7775, 23.7775}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({1.25, .75}, current_time),
      TipStateNear({{.472, .4265}, {29.6975, 21.5157}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({1.5, .5}, current_time),
      TipStateNear({{.6644, .5049}, {34.6406, 14.1117}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({1.75, .25}, current_time),
      TipStateNear({{.8786, .5288}, {38.5482, 4.2955}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update({2, 0}, current_time),
      TipStateNear({{1.109, .495}, {41.4794, -6.0756}, current_time}, kTol));
}

TEST(PositionModelerTest, SmoothTurn) {
  auto point_on_circle = [](float theta) {
    return Vec2{std::cos(theta), std::sin(theta)};
  };

  PositionModeler modeler;
  Time current_time(10.1);
  modeler.Reset({point_on_circle(0), {0, 0}, current_time},
                PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update(point_on_circle(M_PI * .125), current_time),
      TipStateNear({{.9931, .0348}, {-1.2456, 6.2621}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update(point_on_circle(M_PI * .25), current_time),
      TipStateNear({{0.9629, 0.1168}, {-5.4269, 14.7588}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(M_PI * .375), current_time),
              TipStateNear(
                  {{0.8921, 0.2394}, {-12.7511, 22.0623}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(M_PI * .5), current_time),
              TipStateNear(
                  {{0.7685, 0.3820}, {-22.2485, 25.6844}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(M_PI * .625), current_time),
              TipStateNear(
                  {{0.5897, 0.5169}, {-32.1865, 24.2771}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(M_PI * .75), current_time),
              TipStateNear(
                  {{0.3645, 0.6151}, {-40.5319, 17.6785}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update(point_on_circle(M_PI * .875), current_time),
      TipStateNear({{0.1123, 0.6529}, {-45.4017, 6.8034}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(
      modeler.Update(point_on_circle(M_PI), current_time),
      TipStateNear({{-0.1402, 0.6162}, {-45.4417, -6.6022}, current_time},
                   kTol));
}

TEST(PositionModelerTest, UpdateAlongLinearPath) {
  PositionModeler modeler;
  modeler.Reset({{5, 10}, {0, 0}, Time{3}}, PositionModelerParams());

  std::vector<TipState> result;
  modeler.UpdateAlongLinearPath({5, 10}, Time{3}, {15, 10}, Time{3.05}, 5,
                                std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(
          TipStateNear({{5.5891, 10}, {58.9091, 0}, Time{3.01}}, kTol),
          TipStateNear({{6.7587, 10}, {116.9613, 0}, Time{3.02}}, kTol),
          TipStateNear({{8.3355, 10}, {157.6746, 0}, Time{3.03}}, kTol),
          TipStateNear({{10.1509, 10}, {181.5411, 0}, Time{3.04}}, kTol),
          TipStateNear({{12.0875, 10}, {193.6607, 0}, Time{3.05}}, kTol)));

  result.clear();
  modeler.UpdateAlongLinearPath({15, 10}, Time{3.05}, {15, 16}, Time{3.08}, 3,
                                std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(
          TipStateNear({{13.4876, 10.5891}, {140.0123, 58.9091}, Time{3.06}},
                       kTol),
          TipStateNear({{14.3251, 11.7587}, {83.7508, 116.9613}, Time{3.07}},
                       kTol),
          TipStateNear({{14.7584, 13.3355}, {43.3291, 157.6746}, Time{3.08}},
                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeStationary) {
  PositionModeler modeler;
  modeler.Reset({{4, -2}, {0, 0}, Time{0}}, PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({3, -1}, Duration(1. / 180), 20, .01,
                           std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(
          TipStateNear({{3.9091, -1.9091}, {-16.3636, 16.3636}, Time{0.0056}},
                       kTol),
          TipStateNear({{3.7719, -1.7719}, {-24.6942, 24.6942}, Time{0.0111}},
                       kTol),
          TipStateNear({{3.6194, -1.6194}, {-27.4476, 27.4476}, Time{0.0167}},
                       kTol),
          TipStateNear({{3.4716, -1.4716}, {-26.6045, 26.6044}, Time{0.0222}},
                       kTol),
          TipStateNear({{3.3401, -1.3401}, {-23.6799, 23.6799}, Time{0.0278}},
                       kTol),
          TipStateNear({{3.2302, -1.2302}, {-19.7725, 19.7725}, Time{0.0333}},
                       kTol),
          TipStateNear({{3.1434, -1.1434}, {-15.6306, 15.6306}, Time{0.0389}},
                       kTol),
          TipStateNear({{3.0782, -1.0782}, {-11.7244, 11.7244}, Time{0.0444}},
                       kTol),
          TipStateNear({{3.0320, -1.0320}, {-8.3149, 8.3149}, Time{0.0500}},
                       kTol),
          TipStateNear({{3.0014, -1.0014}, {-5.5133, 5.5133}, Time{0.0556}},
                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeInMotion) {
  PositionModeler modeler;
  modeler.Reset({{-1, 2}, {40, 10}, Time{1}}, PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({7, 2}, Duration(1. / 120), 20, .01,
                           std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(
          TipStateNear({{0.7697, 2.0333}, {212.3636, 4.0000}, Time{1.0083}},
                       kTol),
          TipStateNear({{2.7520, 2.0398}, {237.8711, 0.7818}, Time{1.0167}},
                       kTol),
          TipStateNear({{4.4138, 2.0343}, {199.4186, -0.6654}, Time{1.0250}},
                       kTol),
          TipStateNear({{5.6075, 2.0251}, {143.2474, -1.1081}, Time{1.0333}},
                       kTol),
          TipStateNear({{6.3698, 2.0162}, {91.4784, -1.0586}, Time{1.0417}},
                       kTol),
          TipStateNear({{6.8037, 2.0094}, {52.0592, -0.8222}, Time{1.0500}},
                       kTol),
          TipStateNear({{6.9655, 2.0065}, {38.8512, -0.6909}, Time{1.0542}},
                       kTol),
          TipStateNear({{6.9850, 2.0062}, {37.4471, -0.6750}, Time{1.0547}},
                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeMaxIterationsReached) {
  PositionModeler modeler;
  modeler.Reset({{8, -3}, {-100, -150}, Time{1}}, PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({-9, -10}, Duration(.0001), 10, .001,
                           std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(
          TipStateNear(
              {{7.9896, -3.0151}, {-104.2873, -150.9818}, Time{1.0001}}, kTol),
          TipStateNear(
              {{7.9787, -3.0303}, {-108.5406, -151.9521}, Time{1.0002}}, kTol),
          TipStateNear(
              {{7.9674, -3.0456}, {-112.7601, -152.9110}, Time{1.0003}}, kTol),
          TipStateNear(
              {{7.9557, -3.0610}, {-116.9459, -153.8584}, Time{1.0004}}, kTol),
          TipStateNear(
              {{7.9436, -3.0764}, {-121.0982, -154.7945}, Time{1.0005}}, kTol),
          TipStateNear(
              {{7.9311, -3.0920}, {-125.2169, -155.7193}, Time{1.0006}}, kTol),
          TipStateNear(
              {{7.9182, -3.1077}, {-129.3023, -156.6328}, Time{1.0007}}, kTol),
          TipStateNear(
              {{7.9048, -3.1234}, {-133.3545, -157.5351}, Time{1.0008}}, kTol),
          TipStateNear(
              {{7.8911, -3.1393}, {-137.3736, -158.4263}, Time{1.0009}}, kTol),
          TipStateNear(
              {{7.8770, -3.1552}, {-141.3597, -159.3065}, Time{1.0010}},
              kTol)));
}

TEST(PositionModelerTest, SaveAndRestore) {
  PositionModeler modeler;
  Time current_time(0);
  modeler.Reset({{0, 0}, {0, 0}, current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({{.0909, 0}, {16.3636, 0}, current_time}, kTol));

  // Save state that we will overwrite.
  modeler.Save();

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({2, 0}, current_time),
              TipStateNear({{.319, 0}, {41.0579, 0}, current_time}, kTol));

  // Set a second saved state, which should overwrite the first.
  modeler.Save();

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({{.6996, 0}, {68.5055, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({{1.228, 0}, {95.1099, 0}, current_time}, kTol));

  // Restore and repeat the updates.
  modeler.Restore();
  current_time -= kDefaultTimeStep;

  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({{.6996, 0}, {68.5055, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({{1.228, 0}, {95.1099, 0}, current_time}, kTol));

  // Restore should not have cleared the saved state, so do it one more time.
  modeler.Restore();
  current_time -= kDefaultTimeStep;

  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({{.6996, 0}, {68.5055, 0}, current_time}, kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({{1.228, 0}, {95.1099, 0}, current_time}, kTol));

  // Reset should clear the saved state.
  current_time = Time(0);
  modeler.Reset({{0, 0}, {0, 0}, current_time}, PositionModelerParams());

  modeler.Restore();
  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({{.0909, 0}, {16.3636, 0}, current_time}, kTol));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
