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

#include "ink_stroke_modeler/internal/prediction/kalman_predictor.h"

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/prediction/input_predictor.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::ElementsAre;
using ::testing::Matcher;
using ::testing::Optional;

constexpr float kTol = 1e-4;

const KalmanPredictorParams kDefaultKalmanParams{
    .process_noise = .00026458,
    .measurement_noise = .026458,
    .min_catchup_velocity = .01,
    .prediction_interval = Duration(1. / 60),
    .confidence_params{.max_estimation_distance = .04,
                       .min_travel_speed = 3,
                       .max_travel_speed = 15,
                       .max_linear_deviation = .2}};
constexpr SamplingParams kDefaultSamplingParams{
    .min_output_rate = 180, .end_of_stroke_stopping_distance = .001};

// Matcher for the State. Note that, because each of position,
// velocity, acceleration, and jerk are divided by increasing powers of the time
// delta, the values grow exponentially. As such, this uses a relative tolerance
// unless one of the arguments is exactly zero.
MATCHER_P5(StateNearMatcher, position, velocity, acceleration, jerk,
           relative_tol, "") {
  auto within_relative_tol = [](float lhs, float rhs, float tol) {
    if (lhs == 0 || rhs == 0) {
      return std::abs(lhs - rhs) < tol;
    }
    return std::abs(lhs / rhs - 1.f) < tol;
  };

  if (within_relative_tol(position.x, arg.position.x, relative_tol) &&
      within_relative_tol(position.y, arg.position.y, relative_tol) &&
      within_relative_tol(velocity.x, arg.velocity.x, relative_tol) &&
      within_relative_tol(velocity.y, arg.velocity.y, relative_tol) &&
      within_relative_tol(acceleration.x, arg.acceleration.x, relative_tol) &&
      within_relative_tol(acceleration.y, arg.acceleration.y, relative_tol) &&
      within_relative_tol(jerk.x, arg.jerk.x, relative_tol) &&
      within_relative_tol(jerk.y, arg.jerk.y, relative_tol)) {
    return true;
  }

  *result_listener << "\n  expected:"                   //
                   << "\n    p = " << position          //
                   << "\n    v = " << velocity          //
                   << "\n    a = " << acceleration      //
                   << "\n    j = " << jerk              //
                   << "\n  actual:"                     //
                   << "\n    p = " << arg.position      //
                   << "\n    v = " << arg.velocity      //
                   << "\n    a = " << arg.acceleration  //
                   << "\n    j = " << arg.jerk;
  return false;
}

// Wrapping the matcher in a function allows the compiler to perform template
// deduction, so we can specify arguments as initializer lists.
Matcher<KalmanPredictor::State> StateNear(Vec2 position, Vec2 velocity,
                                          Vec2 acceleration, Vec2 jerk,
                                          float tolerance) {
  return StateNearMatcher(position, velocity, acceleration, jerk, tolerance);
}

TEST(KalmanPredictorTest, EmptyPrediction) {
  KalmanPredictor predictor{kDefaultKalmanParams, kDefaultSamplingParams};
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{4, 3}, {2, -4}, Time{3}}).empty());

  predictor.Update({1, 3}, Time{4});
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{1, 3}, {0, 0}, Time{3.1}}).empty());
}

TEST(KalmanPredictorTest, TypicalCase) {
  KalmanPredictor predictor{kDefaultKalmanParams, kDefaultSamplingParams};

  predictor.Update({0, 0}, Time{0});
  predictor.Update({.1, 0}, Time{.01});
  predictor.Update({.2, 0}, Time{.02});
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{4, 3}, {2, -4}, Time{3}}).empty());

  predictor.Update({.3, 0}, Time{.03});
  EXPECT_THAT(predictor.GetEstimatedState(),
              Optional(StateNear({.30078, 0}, {13.584, 0}, {-66.806, 0},
                                 {-3382.8, 0}, kTol)));
  EXPECT_THAT(
      predictor.ConstructPrediction({{.2, 0}, {10, 0}, Time{.03}}),
      ElementsAre(TipStateNear({{.2454, 0}, {7.7094, 0}, Time{.0356}}, kTol),
                  TipStateNear({{.3008, 0}, {13.5837, 0}, Time{.0411}}, kTol),
                  TipStateNear({{.3751, 0}, {13.1604, 0}, Time{.0467}}, kTol)));

  predictor.Update({.5, .1}, Time{.04});
  EXPECT_THAT(predictor.GetEstimatedState(),
              Optional(StateNear({.49705, .097146}, {28.217, 16.732},
                                 {671.91, 813.82}, {4454.3, 6998.2}, kTol)));
  EXPECT_THAT(
      predictor.ConstructPrediction({{.3, 0}, {10, 0}, Time{.04}}),
      ElementsAre(
          TipStateNear({{.3732, .0253}, {17.047, 8.9317}, Time{.0456}}, kTol),
          TipStateNear({{.497, .0971}, {28.2172, 16.7319}, Time{.0511}}, kTol),
          TipStateNear({{.6643, .2029}, {32.0188, 21.3611}, Time{.0567}},
                       kTol)));
}

TEST(KalmanPredictorTest, AlternateParams) {
  auto kalman_params = kDefaultKalmanParams;
  auto sampling_params = kDefaultSamplingParams;
  kalman_params.prediction_interval = Duration(.025);
  sampling_params.min_output_rate = 200;
  KalmanPredictor predictor{kalman_params, sampling_params};

  predictor.Update({2, 5}, Time{1});
  predictor.Update({2.2, 4.9}, Time{1.02});
  predictor.Update({2.3, 4.7}, Time{1.04});
  predictor.Update({2.3, 4.4}, Time{1.06});
  EXPECT_THAT(
      predictor.GetEstimatedState(),
      Optional(StateNear({2.3016, 4.3992}, {-3.9981, -24.374},
                         {-338.22, -288.12}, {-1852.9, -584.31}, kTol)));
  EXPECT_THAT(
      predictor.ConstructPrediction({{2.25, 4.75}, {1, -20}, Time{1.06}}),
      ElementsAre(
          TipStateNear({{2.27, 4.6417}, {5.917, -23.0547}, Time{1.065}}, kTol),
          TipStateNear({{2.2982, 4.5221}, {4.251, -24.5126}, Time{1.07}}, kTol),
          TipStateNear({{2.3016, 4.3992}, {-3.9981, -24.3736}, Time{1.075}},
                       kTol),
          TipStateNear({{2.2773, 4.2738}, {-5.7123, -25.8215}, Time{1.08}},
                       kTol)));

  predictor.Update({2.2, 4.2}, Time{1.08});
  EXPECT_THAT(predictor.GetEstimatedState(),
              Optional(StateNear({2.1987, 4.1933}, {-11.457, -11.953},
                                 {-328.01, 185.32}, {-1133.8, 1569.8}, kTol)));
  EXPECT_THAT(
      predictor.ConstructPrediction({{2.25, 4.5}, {-1, -20}, Time{1.08}}),
      ElementsAre(
          TipStateNear({{2.2499, 4.407}, {.5082, -17.2661}, Time{1.085}}, kTol),
          TipStateNear({{2.2505, 4.3265}, {-.7319, -15.0137}, Time{1.09}},
                       kTol),
          TipStateNear({{2.238, 4.2561}, {-4.7203, -13.2427}, Time{1.095}},
                       kTol),
          TipStateNear({{2.1987, 4.1933}, {-11.4569, -11.9531}, Time{1.1}},
                       kTol),
          TipStateNear({{2.1373, 4.1359}, {-13.1112, -11.0068}, Time{1.105}},
                       kTol)));
}

TEST(KalmanPredictorTest, Reset) {
  KalmanPredictor predictor{kDefaultKalmanParams, kDefaultSamplingParams};

  predictor.Update({4, -4}, Time{6});
  predictor.Update({-6, 9}, Time{6.03});
  predictor.Update({10, 5}, Time{6.06});
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{1, 1}, {6, -3}, Time{6.06}}).empty());

  predictor.Update({2, 4}, Time{6.09});
  EXPECT_NE(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_FALSE(
      predictor.ConstructPrediction({{1, 1}, {6, -3}, Time{6.06}}).empty());

  predictor.Reset();
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{1, 1}, {6, -3}, Time{6.09}}).empty());

  predictor.Update({-9, 3}, Time{2});
  predictor.Update({-6, -1}, Time{2.1});
  predictor.Update({6, -6}, Time{2.2});
  EXPECT_EQ(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_TRUE(
      predictor.ConstructPrediction({{1, 1}, {6, -3}, Time{2.2}}).empty());

  predictor.Update({3, 6}, Time{2.3});
  EXPECT_NE(predictor.GetEstimatedState(), std::nullopt);
  EXPECT_FALSE(
      predictor.ConstructPrediction({{1, 1}, {6, -3}, Time{2.3}}).empty());
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
