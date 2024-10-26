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
#include <limits>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/numbers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::ElementsAre;

const Duration kDefaultTimeStep(1. / 180);
constexpr float kTol = .0005;

// The expected position values are taken directly from results the old
// TipDynamics class. The expected velocity values are from the same source, but
// a multiplier of 300 (i.e. dt / (1 - drag)) had to be applied to account for
// the fact that PositionModeler uses the time step correctly.

TEST(PositionModelerTest, StraightLine) {
  PositionModeler modeler;
  Time current_time(0);
  modeler.Reset({.time = current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({.position = {.0909, 0},
                            .velocity = {16.3636, 0},
                            .acceleration = {2945.4546, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({2, 0}, current_time),
              TipStateNear({.position = {.319, 0},
                            .velocity = {41.0579, 0},
                            .acceleration = {4444.9590, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({.position = {.6996, 0},
                            .velocity = {68.5055, 0},
                            .acceleration = {4940.5737, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({.position = {1.228, 0},
                            .velocity = {95.1099, 0},
                            .acceleration = {4788.8003, 0},
                            .time = current_time},
                           kTol));
}

TEST(PositionModelerTest, ZigZag) {
  PositionModeler modeler;
  Time current_time(3);
  modeler.Reset({.position = {-1, -1}, .time = current_time},
                PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({-.5, -1}, current_time),
              TipStateNear({.position = {-.9545, -1},
                            .velocity = {8.1818, 0},
                            .acceleration = {1472.7273, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({-.5, -.5}, current_time),
              TipStateNear({.position = {-.886, -.9545},
                            .velocity = {12.3471, 8.1818},
                            .acceleration = {749.7521, 1472.7273},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({0, -.5}, current_time),
              TipStateNear({.position = {-.7643, -.886},
                            .velocity = {21.9056, 12.3471},
                            .acceleration = {1720.5348, 749.7521},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({0, 0}, current_time),
              TipStateNear({.position = {-.6218, -.7643},
                            .velocity = {25.6493, 21.9056},
                            .acceleration = {673.8650, 1720.5348},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({.5, 0}, current_time),
              TipStateNear({.position = {-.4343, -.6218},
                            .velocity = {33.7456, 25.6493},
                            .acceleration = {1457.3298, 673.8650},
                            .time = current_time},
                           kTol));
}

TEST(PositionModelerTest, SharpTurn) {
  PositionModeler modeler;
  Time current_time(1.6);
  modeler.Reset({.time = current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({.25, .25}, current_time),
              TipStateNear({.position = {.0227, .0227},
                            .velocity = {4.0909, 4.0909},
                            .acceleration = {736.3636, 736.3636},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({.5, .5}, current_time),
              TipStateNear({.position = {.0798, .0798},
                            .velocity = {10.2645, 10.2645},
                            .acceleration = {1111.2397, 1111.2397},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({.75, .75}, current_time),
              TipStateNear({.position = {.1749, .1749},
                            .velocity = {17.1264, 17.1264},
                            .acceleration = {1235.1434, 1235.1434},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 1}, current_time),
              TipStateNear({.position = {.307, .307},
                            .velocity = {23.7775, 23.7775},
                            .acceleration = {1197.2001, 1197.2001},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1.25, .75}, current_time),
              TipStateNear({.position = {.472, .4265},
                            .velocity = {29.6975, 21.5157},
                            .acceleration = {1065.5977, -407.1296},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1.5, .5}, current_time),
              TipStateNear({.position = {.6644, .5049},
                            .velocity = {34.6406, 14.1117},
                            .acceleration = {889.7637, -1332.7158},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1.75, .25}, current_time),
              TipStateNear({.position = {.8786, .5288},
                            .velocity = {38.5482, 4.2955},
                            .acceleration = {703.3755, -1766.9114},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({2, 0}, current_time),
              TipStateNear({.position = {1.109, .495},
                            .velocity = {41.4794, -6.0756},
                            .acceleration = {527.5996, -1866.8005},
                            .time = current_time},
                           kTol));
}

TEST(PositionModelerTest, SmoothTurn) {
  auto point_on_circle = [](float theta) {
    return Vec2{std::cos(theta), std::sin(theta)};
  };

  PositionModeler modeler;
  Time current_time(10.1);
  modeler.Reset({.position = point_on_circle(0), .time = current_time},
                PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .125), current_time),
              TipStateNear({.position = {.9931, .0348},
                            .velocity = {-1.2456, 6.2621},
                            .acceleration = {-224.2095, 1127.1768},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .25), current_time),
              TipStateNear({.position = {0.9629, 0.1168},
                            .velocity = {-5.4269, 14.7588},
                            .acceleration = {-752.6373, 1529.4097},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .375), current_time),
              TipStateNear({.position = {0.8921, 0.2394},
                            .velocity = {-12.7511, 22.0623},
                            .acceleration = {-1318.3523, 1314.6320},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .5), current_time),
              TipStateNear({.position = {0.7685, 0.3820},
                            .velocity = {-22.2485, 25.6844},
                            .acceleration = {-1709.5339, 651.9690},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .625), current_time),
              TipStateNear({.position = {0.5897, 0.5169},
                            .velocity = {-32.1865, 24.2771},
                            .acceleration = {-1788.8300, -253.3177},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .75), current_time),
              TipStateNear({.position = {0.3645, 0.6151},
                            .velocity = {-40.5319, 17.6785},
                            .acceleration = {-1502.1846, -1187.7462},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi * .875), current_time),
              TipStateNear({.position = {0.1123, 0.6529},
                            .velocity = {-45.4017, 6.8034},
                            .acceleration = {-876.5552, -1957.5056},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update(point_on_circle(kPi), current_time),
              TipStateNear({.position = {-0.1402, 0.6162},
                            .velocity = {-45.4417, -6.6022},
                            .acceleration = {-7.2061, -2413.0093},
                            .time = current_time},
                           kTol));
}

TEST(PositionModelerTest, UpdateAlongLinearPath) {
  PositionModeler modeler;
  modeler.Reset({.position = {5, 10}, .velocity = {0, 0}, .time = Time{3}},
                PositionModelerParams());

  std::vector<TipState> result;
  modeler.UpdateAlongLinearPath({5, 10}, Time{3}, {15, 10}, Time{3.05}, 5,
                                std::back_inserter(result));
  EXPECT_THAT(result, ElementsAre(TipStateNear({.position = {5.5891, 10},
                                                .velocity = {58.9091, 0},
                                                .acceleration = {5890.9092, 0},
                                                .time = Time{3.01}},
                                               kTol),
                                  TipStateNear({.position = {6.7587, 10},
                                                .velocity = {116.9613, 0},
                                                .acceleration = {5805.2231, 0},
                                                .time = Time{3.02}},
                                               kTol),
                                  TipStateNear({.position = {8.3355, 10},
                                                .velocity = {157.6746, 0},
                                                .acceleration = {4071.3291, 0},
                                                .time = Time{3.03}},
                                               kTol),
                                  TipStateNear({.position = {10.1509, 10},
                                                .velocity = {181.5411, 0},
                                                .acceleration = {2386.6475, 0},
                                                .time = Time{3.04}},
                                               kTol),
                                  TipStateNear({.position = {12.0875, 10},
                                                .velocity = {193.6607, 0},
                                                .acceleration = {1211.9609, 0},
                                                .time = Time{3.05}},
                                               kTol)));

  result.clear();
  modeler.UpdateAlongLinearPath({15, 10}, Time{3.05}, {15, 16}, Time{3.08}, 3,
                                std::back_inserter(result));
  EXPECT_THAT(result,
              ElementsAre(TipStateNear({.position = {13.4876, 10.5891},
                                        .velocity = {140.0123, 58.9091},
                                        .acceleration = {-5364.8398, 5890.9092},
                                        .time = Time{3.06}},
                                       kTol),
                          TipStateNear({.position = {14.3251, 11.7587},
                                        .velocity = {83.7508, 116.9613},
                                        .acceleration = {-5626.1528, 5805.2217},
                                        .time = Time{3.07}},
                                       kTol),
                          TipStateNear({.position = {14.7584, 13.3355},
                                        .velocity = {43.3291, 157.6746},
                                        .acceleration = {-4042.1616, 4071.3291},
                                        .time = Time{3.08}},
                                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeStationary) {
  PositionModeler modeler;
  modeler.Reset({.position = {4, -2}}, PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({3, -1}, Duration(1. / 180), 20, .01,
                           std::back_inserter(result));
  EXPECT_THAT(result,
              ElementsAre(TipStateNear({.position = {3.9091, -1.9091},
                                        .velocity = {-16.3636, 16.3636},
                                        .acceleration = {-2945.4546, 2945.4546},
                                        .time = Time{0.0056}},
                                       kTol),
                          TipStateNear({.position = {3.7719, -1.7719},
                                        .velocity = {-24.6942, 24.6942},
                                        .acceleration = {-1499.5044, 1499.5042},
                                        .time = Time{0.0111}},
                                       kTol),
                          TipStateNear({.position = {3.6194, -1.6194},
                                        .velocity = {-27.4476, 27.4476},
                                        .acceleration = {-495.6155, 495.6150},
                                        .time = Time{0.0167}},
                                       kTol),
                          TipStateNear({.position = {3.4716, -1.4716},
                                        .velocity = {-26.6045, 26.6044},
                                        .acceleration = {151.7738, -151.7742},
                                        .time = Time{0.0222}},
                                       kTol),
                          TipStateNear({.position = {3.3401, -1.3401},
                                        .velocity = {-23.6799, 23.6799},
                                        .acceleration = {526.4102, -526.4102},
                                        .time = Time{0.0278}},
                                       kTol),
                          TipStateNear({.position = {3.2302, -1.2302},
                                        .velocity = {-19.7725, 19.7725},
                                        .acceleration = {703.3362, -703.3359},
                                        .time = Time{0.0333}},
                                       kTol),
                          TipStateNear({.position = {3.1434, -1.1434},
                                        .velocity = {-15.6306, 15.6306},
                                        .acceleration = {745.5521, -745.5518},
                                        .time = Time{0.0389}},
                                       kTol),
                          TipStateNear({.position = {3.0782, -1.0782},
                                        .velocity = {-11.7244, 11.7244},
                                        .acceleration = {703.1044, -703.1039},
                                        .time = Time{0.0444}},
                                       kTol),
                          TipStateNear({.position = {3.0320, -1.0320},
                                        .velocity = {-8.3149, 8.3149},
                                        .acceleration = {613.7169, -613.7166},
                                        .time = Time{0.0500}},
                                       kTol),
                          TipStateNear({.position = {3.0014, -1.0014},
                                        .velocity = {-5.5133, 5.5133},
                                        .acceleration = {504.2921, -504.2918},
                                        .time = Time{0.0556}},
                                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeInMotion) {
  PositionModeler modeler;
  modeler.Reset({.position = {-1, 2}, .velocity = {40, 10}, .time = Time{1}},
                PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({7, 2}, Duration(1. / 120), 20, .01,
                           std::back_inserter(result));
  EXPECT_THAT(result,
              ElementsAre(TipStateNear({.position = {0.7697, 2.0333},
                                        .velocity = {212.3636, 4.0000},
                                        .acceleration = {20683.6367, -720.0000},
                                        .time = Time{1.0083}},
                                       kTol),
                          TipStateNear({.position = {2.7520, 2.0398},
                                        .velocity = {237.8711, 0.7818},
                                        .acceleration = {3060.8916, -386.1817},
                                        .time = Time{1.0167}},
                                       kTol),
                          TipStateNear({.position = {4.4138, 2.0343},
                                        .velocity = {199.4186, -0.6654},
                                        .acceleration = {-4614.2959, -173.6631},
                                        .time = Time{1.0250}},
                                       kTol),
                          TipStateNear({.position = {5.6075, 2.0251},
                                        .velocity = {143.2474, -1.1081},
                                        .acceleration = {-6740.5410, -53.1330},
                                        .time = Time{1.0333}},
                                       kTol),
                          TipStateNear({.position = {6.3698, 2.0162},
                                        .velocity = {91.4784, -1.0586},
                                        .acceleration = {-6212.2896, 5.9471},
                                        .time = Time{1.0417}},
                                       kTol),
                          TipStateNear({.position = {6.8037, 2.0094},
                                        .velocity = {52.0592, -0.8222},
                                        .acceleration = {-4730.2935, 28.3621},
                                        .time = Time{1.0500}},
                                       kTol),
                          TipStateNear({.position = {6.9655, 2.0065},
                                        .velocity = {38.8512, -0.6909},
                                        .acceleration = {-3169.9351, 31.5268},
                                        .time = Time{1.0542}},
                                       kTol),
                          TipStateNear({.position = {6.9850, 2.0062},
                                        .velocity = {37.4471, -0.6750},
                                        .acceleration = {-2695.7649, 30.5478},
                                        .time = Time{1.0547}},
                                       kTol)));
}

TEST(PositionModelerTest, ModelEndOfStrokeMaxIterationsReached) {
  PositionModeler modeler;
  modeler.Reset(
      {.position = {8, -3}, .velocity = {-100, -150}, .time = Time{1}},
      PositionModelerParams());

  std::vector<TipState> result;
  modeler.ModelEndOfStroke({-9, -10}, Duration(.0001), 10, .001,
                           std::back_inserter(result));
  EXPECT_THAT(
      result,
      ElementsAre(TipStateNear({.position = {7.9896, -3.0151},
                                .velocity = {-104.2873, -150.9818},
                                .acceleration = {-42872.7266, -9818.1816},
                                .time = Time{1.0001}},
                               kTol),
                  TipStateNear({.position = {7.9787, -3.0303},
                                .velocity = {-108.5406, -151.9521},
                                .acceleration = {-42533.3242, -9703.0205},
                                .time = Time{1.0002}},
                               kTol),
                  TipStateNear({.position = {7.9674, -3.0456},
                                .velocity = {-112.7601, -152.9110},
                                .acceleration = {-42195.1211, -9588.4023},
                                .time = Time{1.0003}},
                               kTol),
                  TipStateNear({.position = {7.9557, -3.0610},
                                .velocity = {-116.9459, -153.8584},
                                .acceleration = {-41858.1016, -9474.3242},
                                .time = Time{1.0004}},
                               kTol),
                  TipStateNear({.position = {7.9436, -3.0764},
                                .velocity = {-121.0982, -154.7945},
                                .acceleration = {-41522.2734, -9360.7930},
                                .time = Time{1.0005}},
                               kTol),
                  TipStateNear({.position = {7.9311, -3.0920},
                                .velocity = {-125.2169, -155.7193},
                                .acceleration = {-41187.6445, -9247.7998},
                                .time = Time{1.0006}},
                               kTol),
                  TipStateNear({.position = {7.9182, -3.1077},
                                .velocity = {-129.3023, -156.6328},
                                .acceleration = {-40854.2109, -9135.3506},
                                .time = Time{1.0007}},
                               kTol),
                  TipStateNear({.position = {7.9048, -3.1234},
                                .velocity = {-133.3545, -157.5351},
                                .acceleration = {-40521.9727, -9023.4395},
                                .time = Time{1.0008}},
                               kTol),
                  TipStateNear({.position = {7.8911, -3.1393},
                                .velocity = {-137.3736, -158.4263},
                                .acceleration = {-40190.9414, -8912.0703},
                                .time = Time{1.0009}},
                               kTol),
                  TipStateNear({.position = {7.8770, -3.1552},
                                .velocity = {-141.3597, -159.3065},
                                .acceleration = {-39861.0977, -8801.2402},
                                .time = Time{1.0010}},
                               kTol)));
}

TEST(PositionModelerTest, SaveAndRestore) {
  PositionModeler modeler;
  Time current_time(0);
  modeler.Reset({.time = current_time}, PositionModelerParams());

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({.position = {.0909, 0},
                            .velocity = {16.3636, 0},
                            .acceleration = {2945.4546, 0},
                            .time = current_time},
                           kTol));

  // Save state that we will overwrite.
  modeler.Save();

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({2, 0}, current_time),
              TipStateNear({.position = {.319, 0},
                            .velocity = {41.0579, 0},
                            .acceleration = {4444.9590, 0},
                            .time = current_time},
                           kTol));

  // Set a second saved state, which should overwrite the first.
  modeler.Save();

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({.position = {.6996, 0},
                            .velocity = {68.5055, 0},
                            .acceleration = {4940.5737, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({.position = {1.228, 0},
                            .velocity = {95.1099, 0},
                            .acceleration = {4788.8003, 0},
                            .time = current_time},
                           kTol));

  // Restore and repeat the updates.
  modeler.Restore();
  current_time -= kDefaultTimeStep;

  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({.position = {.6996, 0},
                            .velocity = {68.5055, 0},
                            .acceleration = {4940.5737, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({.position = {1.228, 0},
                            .velocity = {95.1099, 0},
                            .acceleration = {4788.8003, 0},
                            .time = current_time},
                           kTol));

  // Restore should not have cleared the saved state, so do it one more time.
  modeler.Restore();
  current_time -= kDefaultTimeStep;

  EXPECT_THAT(modeler.Update({3, 0}, current_time),
              TipStateNear({.position = {.6996, 0},
                            .velocity = {68.5055, 0},
                            .acceleration = {4940.5737, 0},
                            .time = current_time},
                           kTol));

  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({4, 0}, current_time),
              TipStateNear({.position = {1.228, 0},
                            .velocity = {95.1099, 0},
                            .acceleration = {4788.8003, 0},
                            .time = current_time},
                           kTol));

  // Reset should clear the saved state.
  current_time = Time(0);
  modeler.Reset({.time = current_time}, PositionModelerParams());

  modeler.Restore();
  current_time += kDefaultTimeStep;
  EXPECT_THAT(modeler.Update({1, 0}, current_time),
              TipStateNear({.position = {.0909, 0},
                            .velocity = {16.3636, 0},
                            .acceleration = {2945.4546, 0},
                            .time = current_time},
                           kTol));
}

TEST(NumberOfStepsBetweenInputsTest, ResolutionIsSufficient) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{}, Input{.position = {0, 0}, .time = Time{0}},
      Input{.position = {1, 0}, .time = Time{1}},
      SamplingParams{.min_output_rate = 0.1}, PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 1);
}

TEST(NumberOfStepsBetweenInputsTest, TimeTooLong) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{}, Input{.position = {0, 0}, .time = Time{0}},
      Input{.position = {1, 0}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1}, PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 2);
}

TEST(NumberOfStepsBetweenInputsTest, TimeTooLongAvoidsIntOverflow) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{}, Input{.position = {0, 0}, .time = Time{0}},
      Input{.position = {1, 0}, .time = Time{1}},
      SamplingParams{.min_output_rate = 1.0 + std::numeric_limits<int>::max(),
                     .max_outputs_per_call = std::numeric_limits<int>::max()},
      PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, std::numeric_limits<int>::max());
}

TEST(NumberOfStepsBetweenInputsTest, ExactlyMaxOutputsPerCall) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{}, Input{.position = {0, 0}, .time = Time{0}},
      Input{.position = {1, 0}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 2},
      PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 2);
}

TEST(NumberOfStepsBetweenInputsTest, OverMaxOutputsPerCall) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{}, Input{.position = {0, 0}, .time = Time{0}},
      Input{.position = {1, 0}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest, NanTipPositionIsError) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {5, std::numeric_limits<float>::quiet_NaN()},
               .velocity = {1, 1},
               .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {25, 20}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest, InfiniteTipPositionIsError) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {5, std::numeric_limits<float>::infinity()},
               .velocity = {1, 1},
               .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {25, 20}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest, InfiniteTipVelocityIsError) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {5, 0},
               .velocity = {std::numeric_limits<float>::infinity(), 1},
               .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {25, 20}, .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest, InfiniteEndPositionIsError) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {5, 0}, .velocity = {1, 1}, .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {25, std::numeric_limits<float>::infinity()},
            .time = Time{20}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest,
     HugeDifferenceBetweenTipAndEndCanOverflow) {
  // If the difference between the tip and end positions is huge, dividing it by
  // the spring mass constant can overflow to an infinite value, even over a
  // very short time difference. This overflow should be handled gracefully with
  // an InvalidArgument error.
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {3.4e38, 0}, .velocity = {1, 1}, .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {5.0001, 0.0001}, .time = Time{0.0001}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{.spring_mass_constant = 0.5});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(NumberOfStepsBetweenInputsTest, CanHandleBigDifferenceBetweenTipAndEnd) {
  // Even with a very big difference between tip and end positions, if the
  // intermediate calculations don't overflow, we shouldn't get an error. This
  // test case is the same as HugeDifferenceBetweenTipAndEndCanOverflow, but
  // with the TipState x position a little smaller.
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {3.4e36, 0}, .velocity = {1, 1}, .time = Time{0}},
      Input{.position = {5, 0}, .time = Time{0}},
      Input{.position = {5.0001, 0.0001}, .time = Time{0.0001}},
      SamplingParams{.min_output_rate = 0.1, .max_outputs_per_call = 1},
      PositionModelerParams{.spring_mass_constant = 0.5});
  EXPECT_EQ(n_steps.status().code(), absl::StatusCode::kOk);
}

TEST(NumberOfStepsBetweenInputsTest, UpsampleDueToSharpTurn) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {0, 0}, .velocity = {0, 1}, .time = Time{0}},
      Input{.position = {0, 0}, .time = Time{0}},
      // This should predict basically a 90-degree turn over the interval, it
      // starts going straight up and is pulled very strongly to the right.
      Input{.position = {500, 0}, .time = Time{1}},
      SamplingParams{.min_output_rate = 0.1,
                     // Require one sample per degree of turn that would
                     // be made without upsampling.
                     .max_estimated_angle_to_traverse_per_input = kPi / 180},
      PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 90);
}

TEST(NumberOfStepsBetweenInputsTest, UpsampleDueToSharpTurnSamePosition) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {0, 0}, .velocity = {0, 1}, .time = Time{0}},
      Input{.position = {0, 0}, .time = Time{0}},
      // Here there's no acceleration from the spring because the input
      // didn't move. We're ignoring drag.
      Input{.position = {0, 0}, .time = Time{1}},
      SamplingParams{.min_output_rate = 0.1,
                     // Require one sample per degree of turn that would
                     // be made without upsampling.
                     .max_estimated_angle_to_traverse_per_input = kPi / 180},
      PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 1);
}

TEST(NumberOfStepsBetweenInputsTest, UpsampleDueToSharpTurnSmallForce) {
  absl::StatusOr<int> n_steps = NumberOfStepsBetweenInputs(
      TipState{.position = {0, 0}, .velocity = {0, 1}},
      Input{.position = {0, 0}, .time = Time{0}},
      // Here the acceleration is at a 90-deg angle but the velocity doesn't
      // change much.
      Input{.position = {0, 0.0001}, .time = Time{1}},
      SamplingParams{.min_output_rate = 0.1,
                     // Require one sample per degree of turn that would
                     // be made without upsampling.
                     .max_estimated_angle_to_traverse_per_input = kPi / 180},
      PositionModelerParams{});
  ASSERT_TRUE(n_steps.ok());
  EXPECT_EQ(*n_steps, 1);
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
