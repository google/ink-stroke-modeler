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

#include "ink_stroke_modeler/params.h"

#include <cmath>
#include <limits>

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ink_stroke_modeler/numbers.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

const KalmanPredictorParams kGoodKalmanParams{
    .process_noise = .01,
    .measurement_noise = .1,
    .min_stable_iteration = 2,
    .max_time_samples = 10,
    .min_catchup_velocity = 1,
    .acceleration_weight = -1,
    .jerk_weight = 200,
    .prediction_interval{Duration(1)},
    .confidence_params{.desired_number_of_samples = 10,
                       .max_estimation_distance = 1,
                       .min_travel_speed = 6,
                       .max_travel_speed = 50,
                       .max_linear_deviation = 2,
                       .baseline_linearity_confidence = .5}};

const StrokeModelParams kGoodStrokeModelParams{
    .wobble_smoother_params{
        .timeout = Duration(.5), .speed_floor = 1, .speed_ceiling = 20},
    .position_modeler_params{.spring_mass_constant = .2, .drag_constant = 4},
    .sampling_params{.min_output_rate = 3,
                     .end_of_stroke_stopping_distance = 1e-6,
                     .end_of_stroke_max_iterations = 1},
    .stylus_state_modeler_params{.max_input_samples = 7},
    .prediction_params = StrokeEndPredictorParams{}};

TEST(ParamsTest, ValidatePositionModelerParams) {
  EXPECT_TRUE(ValidatePositionModelerParams(
                  {.spring_mass_constant = 1, .drag_constant = 3})
                  .ok());

  EXPECT_EQ(ValidatePositionModelerParams(
                {.spring_mass_constant = 0, .drag_constant = 1})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidatePositionModelerParams(
                {.spring_mass_constant = 1, .drag_constant = 0})
                .code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(ParamsTest, ValidateSamplingParams) {
  EXPECT_TRUE(ValidateSamplingParams({.min_output_rate = 10,
                                      .end_of_stroke_stopping_distance = .1,
                                      .end_of_stroke_max_iterations = 3})
                  .ok());

  EXPECT_EQ(ValidateSamplingParams({.min_output_rate = 0,
                                    .end_of_stroke_stopping_distance = .1,
                                    .end_of_stroke_max_iterations = 3})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidateSamplingParams({.min_output_rate = 1,
                                    .end_of_stroke_stopping_distance = 0,
                                    .end_of_stroke_max_iterations = 3})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidateSamplingParams({.min_output_rate = 1,
                                    .end_of_stroke_stopping_distance = 5,
                                    .end_of_stroke_max_iterations = 0})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidateSamplingParams({.min_output_rate = 10,
                                    .end_of_stroke_stopping_distance = .1,
                                    .end_of_stroke_max_iterations = 3,
                                    .max_outputs_per_call = 0})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      ValidateSamplingParams({.min_output_rate = 10,
                              .end_of_stroke_stopping_distance = .1,
                              .end_of_stroke_max_iterations = 3,
                              .max_estimated_angle_to_traverse_per_input = 0})
          .code(),
      absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(
      ValidateSamplingParams({.min_output_rate = 10,
                              .end_of_stroke_stopping_distance = .1,
                              .end_of_stroke_max_iterations = 3,
                              .max_estimated_angle_to_traverse_per_input = kPi})
          .code(),
      absl::StatusCode::kInvalidArgument);
  EXPECT_TRUE(
      ValidateSamplingParams({.min_output_rate = 10,
                              .end_of_stroke_stopping_distance = .1,
                              .end_of_stroke_max_iterations = 3,
                              .max_estimated_angle_to_traverse_per_input =
                                  std::nextafter(kPi, 0.0)})
          .ok());
}

TEST(ParamsTest, ValidateStylusStateModelerParams) {
  EXPECT_TRUE(ValidateStylusStateModelerParams({.max_input_samples = 1}).ok());

  EXPECT_EQ(ValidateStylusStateModelerParams({.max_input_samples = 0}).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(ParamsTest, ValidateWobbleSmootherParams) {
  EXPECT_TRUE(
      ValidateWobbleSmootherParams(
          {.timeout = Duration(1), .speed_floor = 2, .speed_ceiling = 3})
          .ok());
  EXPECT_TRUE(
      ValidateWobbleSmootherParams(
          {.timeout = Duration(0), .speed_floor = 0, .speed_ceiling = 0})
          .ok());

  EXPECT_EQ(ValidateWobbleSmootherParams(
                {.timeout = Duration(-1), .speed_floor = 2, .speed_ceiling = 5})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidateWobbleSmootherParams(
                {.timeout = Duration(1), .speed_floor = -2, .speed_ceiling = 1})
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(ValidateWobbleSmootherParams(
                {.timeout = Duration(1), .speed_floor = 7, .speed_ceiling = 4})
                .code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(ParamsTest, ValidateDisabledPredictorParams) {
  EXPECT_TRUE(ValidatePredictionParams(DisabledPredictorParams{}).ok());
}

TEST(ParamsTest, ValidateStrokeEndPredictorParams) {
  EXPECT_TRUE(ValidatePredictionParams(StrokeEndPredictorParams()).ok());
}

TEST(ParamsTest, ValidateKalmanPredictorParams) {
  EXPECT_TRUE(ValidatePredictionParams(kGoodKalmanParams).ok());
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.process_noise = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.measurement_noise = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.min_stable_iteration = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.max_time_samples = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.prediction_interval = Duration(0);
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
}

TEST(ParamsTest, ValidateKalmanPredictorConfidenceParams) {
  EXPECT_TRUE(ValidatePredictionParams(kGoodKalmanParams).ok());
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.desired_number_of_samples = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.max_estimation_distance = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.min_travel_speed = -1;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.min_travel_speed = 10;
    bad_params.confidence_params.max_travel_speed = 1;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.max_linear_deviation = 0;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.baseline_linearity_confidence = -.3;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodKalmanParams;
    bad_params.confidence_params.baseline_linearity_confidence = 1.01;
    EXPECT_EQ(ValidatePredictionParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
}

TEST(ParamsTest, ValidateStrokeModelParams) {
  EXPECT_TRUE(ValidateStrokeModelParams(kGoodStrokeModelParams).ok());
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.wobble_smoother_params.timeout = Duration(-10);
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.position_modeler_params.spring_mass_constant = -1;
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.stylus_state_modeler_params.max_input_samples = 0;
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.sampling_params.end_of_stroke_max_iterations = -3;
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.sampling_params.end_of_stroke_max_iterations = 123456789;
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
  {
    auto bad_params = kGoodStrokeModelParams;
    bad_params.prediction_params =
        KalmanPredictorParams{.prediction_interval = Duration(-1)};
    EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
              absl::StatusCode::kInvalidArgument);
  }
}

TEST(ParamsTest, NaNIsNotAValidValue) {
  auto bad_params = kGoodStrokeModelParams;
  bad_params.position_modeler_params.spring_mass_constant =
      std::numeric_limits<float>::quiet_NaN();
  EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(ParamsTest, InfinityIsNotAValidValue) {
  auto bad_params = kGoodStrokeModelParams;
  bad_params.position_modeler_params.spring_mass_constant =
      std::numeric_limits<float>::infinity();
  EXPECT_EQ(ValidateStrokeModelParams(bad_params).code(),
            absl::StatusCode::kInvalidArgument);
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
