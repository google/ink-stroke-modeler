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

#include "ink_stroke_modeler/internal/prediction/stroke_end_predictor.h"

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/prediction/input_predictor.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::Not;

constexpr float kTol = 1e-4;

constexpr SamplingParams kDefaultSamplingParams{
    .min_output_rate = 180,
    .end_of_stroke_stopping_distance = .001,
    .end_of_stroke_max_iterations = 20};

TEST(StrokeEndPredictorTest, EmptyPrediction) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  std::vector<TipState> prediction;
  predictor.ConstructPrediction(
      {.position = {4, 6}, .velocity = {-1, 1}, .time = Time{5}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Reset();
  predictor.ConstructPrediction(
      {.position = {-2, 11}, .velocity = {0, 0}, .time = Time{1}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, SingleInput) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  predictor.Update({4, 5}, Time{2});

  std::vector<TipState> prediction;
  predictor.ConstructPrediction(
      {.position = {4, 5}, .velocity = {0, 0}, .time = Time{2}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, MultipleInputs) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  std::vector<TipState> prediction;

  predictor.Update({-1, 1}, Time{1});
  predictor.ConstructPrediction({.position = {-1, 1}, .time = Time{1}},
                                prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Update({-1, 1.2}, Time{1.02});
  predictor.ConstructPrediction(
      {.position = {-1, 1.1}, .velocity = {0, 5}, .time = Time{1.02}},
      prediction);
  EXPECT_THAT(prediction,
              ElementsAre(TipStateNear({.position = {-1, 1.1258},
                                        .velocity = {0, 4.6364},
                                        .acceleration = {0, -65.4545},
                                        .time = Time{1.0256}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.1480},
                                        .velocity = {0, 3.9967},
                                        .acceleration = {0, -115.1404},
                                        .time = Time{1.0311}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.1660},
                                        .velocity = {0, 3.2496},
                                        .acceleration = {0, -134.4845},
                                        .time = Time{1.0367}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.1799},
                                        .velocity = {0, 2.5059},
                                        .acceleration = {0, -133.8652},
                                        .time = Time{1.0422}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.1901},
                                        .velocity = {0, 1.8318},
                                        .acceleration = {0, -121.3242},
                                        .time = Time{1.0478}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.1971},
                                        .velocity = {0, 1.2609},
                                        .acceleration = {0, -102.7701},
                                        .time = Time{1.0533}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.2000},
                                        .velocity = {0, 1.0323},
                                        .acceleration = {0, -82.2949},
                                        .time = Time{1.0561}},
                                       kTol)));

  predictor.Update({-1, 1.4}, Time{1.04});
  predictor.ConstructPrediction(
      {.position = {-1, 1.2}, .velocity = {0, 5}, .time = Time{1.04}},
      prediction);
  EXPECT_THAT(prediction,
              ElementsAre(TipStateNear({.position = {-1, 1.2348},
                                        .velocity = {0, 6.2727},
                                        .acceleration = {0, 229.0907},
                                        .time = Time{1.0455}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.2708},
                                        .velocity = {0, 6.4661},
                                        .acceleration = {0, 34.8099},
                                        .time = Time{1.0511}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3041},
                                        .velocity = {0, 5.9943},
                                        .acceleration = {0, -84.9232},
                                        .time = Time{1.0566}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3328},
                                        .velocity = {0, 5.1663},
                                        .acceleration = {0, -149.0426},
                                        .time = Time{1.0622}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3561},
                                        .velocity = {0, 4.1998},
                                        .acceleration = {0, -173.9650},
                                        .time = Time{1.0677}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3741},
                                        .velocity = {0, 3.2381},
                                        .acceleration = {0, -173.1034},
                                        .time = Time{1.0733}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3872},
                                        .velocity = {0, 2.3668},
                                        .acceleration = {0, -156.8500},
                                        .time = Time{1.0788}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.3963},
                                        .velocity = {0, 1.6288},
                                        .acceleration = {0, -132.8388},
                                        .time = Time{1.0844}},
                                       kTol),
                          TipStateNear({.position = {-1, 1.4000},
                                        .velocity = {0, 1.3333},
                                        .acceleration = {0, -106.3558},
                                        .time = Time{1.0872}},
                                       kTol)));
}

TEST(StrokeEndPredictorTest, Reset) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  std::vector<TipState> prediction;

  predictor.Update({-9, 6}, Time{5});
  predictor.ConstructPrediction(
      {.position = {-9, 6}, .velocity = {0, 0}, .time = Time{5}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
  predictor.Update({1, 4}, Time{7});
  predictor.ConstructPrediction(
      {.position = {-4, 5}, .velocity = {5, -1}, .time = Time{7}}, prediction);
  EXPECT_THAT(prediction, Not(IsEmpty()));

  predictor.Reset();
  predictor.ConstructPrediction(
      {.position = {0, 1}, .velocity = {0, 0}, .time = Time{1}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, AlternateSamplingParams) {
  StrokeEndPredictor predictor{
      PositionModelerParams{},
      SamplingParams{.min_output_rate = 200,
                     .end_of_stroke_stopping_distance = .005}};
  std::vector<TipState> prediction;

  predictor.Update({4, -7}, Time{3});
  predictor.ConstructPrediction(
      {.position = {4, -7}, .velocity = {0, 0}, .time = Time{3}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Update({4.2, -6.8}, Time{3.01});
  predictor.ConstructPrediction(
      {.position = {4.1, -6.9}, .velocity = {2, 2}, .time = Time{3.01}},
      prediction);
  EXPECT_THAT(prediction,
              ElementsAre(TipStateNear({.position = {4.1138, -6.8862},
                                        .velocity = {2.7527, 2.7527},
                                        .acceleration = {150.5452, 150.5452},
                                        .time = Time{3.015}},
                                       kTol),
                          TipStateNear({.position = {4.1289, -6.8711},
                                        .velocity = {3.0318, 3.0318},
                                        .acceleration = {55.8094, 55.8094},
                                        .time = Time{3.02}},
                                       kTol),
                          TipStateNear({.position = {4.1439, -6.8561},
                                        .velocity = {2.9871, 2.9871},
                                        .acceleration = {-8.9311, -8.9311},
                                        .time = Time{3.025}},
                                       kTol),
                          TipStateNear({.position = {4.1576, -6.8424},
                                        .velocity = {2.7386, 2.7386},
                                        .acceleration = {-49.7077, -49.7077},
                                        .time = Time{3.03}},
                                       kTol),
                          TipStateNear({.position = {4.1694, -6.8306},
                                        .velocity = {2.3779, 2.3779},
                                        .acceleration = {-72.1446, -72.1446},
                                        .time = Time{3.035}},
                                       kTol),
                          TipStateNear({.position = {4.1793, -6.8207},
                                        .velocity = {1.9719, 1.9719},
                                        .acceleration = {-81.1924, -81.1924},
                                        .time = Time{3.04}},
                                       kTol),
                          TipStateNear({.position = {4.1871, -6.8129},
                                        .velocity = {1.5669, 1.5669},
                                        .acceleration = {-81.0040, -81.0040},
                                        .time = Time{3.045}},
                                       kTol),
                          TipStateNear({.position = {4.1931, -6.8069},
                                        .velocity = {1.1923, 1.1923},
                                        .acceleration = {-74.9186, -74.9186},
                                        .time = Time{3.05}},
                                       kTol),
                          TipStateNear({.position = {4.1974, -6.8026},
                                        .velocity = {0.8647, 0.8647},
                                        .acceleration = {-65.5070, -65.5070},
                                        .time = Time{3.055}},
                                       kTol)));
}

TEST(StrokeEndPredictorTest, MakeCopy) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};

  predictor.Update({-1, 1}, Time{1});
  std::unique_ptr<InputPredictor> predictor_copy = predictor.MakeCopy();

  predictor.Update({-1, 1.2}, Time{1.02});
  predictor_copy->Update({-1, 1.2}, Time{1.02});

  std::vector<TipState> prediction;
  std::vector<TipState> prediction_from_copy;
  TipState last_tip_state = {
      .position = {-1, 1.1},
      .velocity = {0, 5},
      .time = Time{1.02},
  };

  predictor.ConstructPrediction(last_tip_state, prediction);
  predictor_copy->ConstructPrediction(last_tip_state, prediction_from_copy);

  ASSERT_EQ(prediction.size(), 7);
  EXPECT_THAT(
      prediction_from_copy,
      ElementsAre(
          TipStateNear(prediction[0], kTol), TipStateNear(prediction[1], kTol),
          TipStateNear(prediction[2], kTol), TipStateNear(prediction[3], kTol),
          TipStateNear(prediction[4], kTol), TipStateNear(prediction[5], kTol),
          TipStateNear(prediction[6], kTol)));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
