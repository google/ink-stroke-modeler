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

#include "ink_stroke_modeler/internal/wobble_smoother.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

const WobbleSmootherParams kDefaultParams{
    .timeout = Duration(.04), .speed_floor = 1.31, .speed_ceiling = 1.44};
const float kDefaultTol = 0.0001;

TEST(WobbleSmootherTest, SlowStraightLine) {
  // The line moves at 1 cm/s, which is below the floor of 1.31 cm/s.
  WobbleSmoother filter;
  filter.Reset(kDefaultParams, {3, 4}, Time{1});
  EXPECT_THAT(filter.Update({3.016, 4}, Time{1.016}), Vec2Eq({3.016, 4}));
  EXPECT_THAT(filter.Update({3.032, 4}, Time{1.032}),
              Vec2Near({3.024, 4}, kDefaultTol));
  EXPECT_THAT(filter.Update({3.048, 4}, Time{1.048}),
              Vec2Near({3.032, 4}, kDefaultTol));
  EXPECT_THAT(filter.Update({3.064, 4}, Time{1.064}),
              Vec2Near({3.048, 4}, kDefaultTol));
}

TEST(WobbleSmootherTest, SlowStraightLineEqualFloorAndCeiling) {
  // The line moves at 1 cm/s, which is below the floor of 1.31 cm/s.
  WobbleSmootherParams equal_floor_and_ceiling_params{
      .timeout = Duration(.04), .speed_floor = 1.31, .speed_ceiling = 1.31};
  WobbleSmoother filter;
  filter.Reset(equal_floor_and_ceiling_params, {3, 4}, Time{1});
  EXPECT_THAT(filter.Update({3.016, 4}, Time{1.016}), Vec2Eq({3.016, 4}));
  EXPECT_THAT(filter.Update({3.032, 4}, Time{1.032}),
              Vec2Near({3.024, 4}, kDefaultTol));
  EXPECT_THAT(filter.Update({3.048, 4}, Time{1.048}),
              Vec2Near({3.032, 4}, kDefaultTol));
  EXPECT_THAT(filter.Update({3.064, 4}, Time{1.064}),
              Vec2Near({3.048, 4}, kDefaultTol));
}

TEST(WobbleSmootherTest, FastStraightLine) {
  // The line moves at 1.5 cm/s, which is above the ceiling of 1.44 cm/s.
  WobbleSmoother filter;
  filter.Reset(kDefaultParams, {-1, 0}, Time{0});
  EXPECT_THAT(filter.Update({-1, .024}, Time{.016}), Vec2Eq({-1, .024}));
  EXPECT_THAT(filter.Update({-1, .048}, Time{.032}), Vec2Eq({-1, .048}));
  EXPECT_THAT(filter.Update({-1, .072}, Time{.048}), Vec2Eq({-1, .072}));
}

TEST(WobbleSmootherTest, FastStraightLineEqualFloorAndCeiling) {
  // The line moves at 1.5 cm/s, which is above the ceiling of 1.44 cm/s.
  WobbleSmoother filter;
  WobbleSmootherParams equal_floor_and_ceiling_params{
      .timeout = Duration(.04), .speed_floor = 1.41, .speed_ceiling = 1.41};
  filter.Reset(equal_floor_and_ceiling_params, {-1, 0}, Time{0});
  EXPECT_THAT(filter.Update({-1, .024}, Time{.016}), Vec2Eq({-1, .024}));
  EXPECT_THAT(filter.Update({-1, .048}, Time{.032}), Vec2Eq({-1, .048}));
  EXPECT_THAT(filter.Update({-1, .072}, Time{.048}), Vec2Eq({-1, .072}));
}

TEST(WobbleSmootherTest, SlowZigZag) {
  // The line moves at 1 cm/s, which is below the floor of 1.31 cm/s.
  WobbleSmoother filter;
  filter.Reset(kDefaultParams, {1, 2}, Time{5});
  EXPECT_THAT(filter.Update({1.016, 2}, Time{5.016}), Vec2Eq({1.016, 2}));
  EXPECT_THAT(filter.Update({1.016, 2.016}, Time{5.032}),
              Vec2Near({1.016, 2.008}, kDefaultTol));
  EXPECT_THAT(filter.Update({1.032, 2.016}, Time{5.048}),
              Vec2Near({1.02133, 2.01067}, kDefaultTol));
  EXPECT_THAT(filter.Update({1.032, 2.032}, Time{5.064}),
              Vec2Near({1.0266667, 2.0213333}, kDefaultTol));
  EXPECT_THAT(filter.Update({1.048, 2.032}, Time{5.080}),
              Vec2Near({1.0373333, 2.0266667}, kDefaultTol));
  EXPECT_THAT(filter.Update({1.048, 2.048}, Time{5.096}),
              Vec2Near({1.0426667, 2.0373333}, kDefaultTol));
}

TEST(WobbleSmootherTest, FastZigZag) {
  // The line moves at 1.5 cm/s, which is above the ceiling of 1.44 cm/s.
  WobbleSmoother filter;
  filter.Reset(kDefaultParams, {7, 3}, Time{8});
  EXPECT_THAT(filter.Update({7, 3.024}, Time{8.016}), Vec2Eq({7, 3.024}));
  EXPECT_THAT(filter.Update({7.024, 3.024}, Time{8.032}),
              Vec2Eq({7.024, 3.024}));
  EXPECT_THAT(filter.Update({7.024, 3.048}, Time{8.048}),
              Vec2Eq({7.024, 3.048}));
  EXPECT_THAT(filter.Update({7.048, 3.048}, Time{8.064}),
              Vec2Eq({7.048, 3.048}));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
