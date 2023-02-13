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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"

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
  predictor.ConstructPrediction({{4, 6}, {-1, 1}, Time{5}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Reset();
  predictor.ConstructPrediction({{-2, 11}, {0, 0}, Time{1}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, SingleInput) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  predictor.Update({4, 5}, Time{2});

  std::vector<TipState> prediction;
  predictor.ConstructPrediction({{4, 5}, {0, 0}, Time{2}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, MultipleInputs) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  std::vector<TipState> prediction;

  predictor.Update({-1, 1}, Time{1});
  predictor.ConstructPrediction({{-1, 1}, {0, 0}, Time{1}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Update({-1, 1.2}, Time{1.02});
  predictor.ConstructPrediction({{-1, 1.1}, {0, 5}, Time{1.02}}, prediction);
  EXPECT_THAT(
      prediction,
      ElementsAre(
          TipStateNear({{-1, 1.1258}, {0, 4.6364}, Time{1.0256}}, kTol),
          TipStateNear({{-1, 1.1480}, {0, 3.9967}, Time{1.0311}}, kTol),
          TipStateNear({{-1, 1.1660}, {0, 3.2496}, Time{1.0367}}, kTol),
          TipStateNear({{-1, 1.1799}, {0, 2.5059}, Time{1.0422}}, kTol),
          TipStateNear({{-1, 1.1901}, {0, 1.8318}, Time{1.0478}}, kTol),
          TipStateNear({{-1, 1.1971}, {0, 1.2609}, Time{1.0533}}, kTol),
          TipStateNear({{-1, 1.2000}, {0, 1.0323}, Time{1.0561}}, kTol)));

  predictor.Update({-1, 1.4}, Time{1.04});
  predictor.ConstructPrediction({{-1, 1.2}, {0, 5}, Time{1.04}}, prediction);
  EXPECT_THAT(
      prediction,
      ElementsAre(
          TipStateNear({{-1, 1.2348}, {0, 6.2727}, Time{1.0455}}, kTol),
          TipStateNear({{-1, 1.2708}, {0, 6.4661}, Time{1.0511}}, kTol),
          TipStateNear({{-1, 1.3041}, {0, 5.9943}, Time{1.0566}}, kTol),
          TipStateNear({{-1, 1.3328}, {0, 5.1663}, Time{1.0622}}, kTol),
          TipStateNear({{-1, 1.3561}, {0, 4.1998}, Time{1.0677}}, kTol),
          TipStateNear({{-1, 1.3741}, {0, 3.2381}, Time{1.0733}}, kTol),
          TipStateNear({{-1, 1.3872}, {0, 2.3668}, Time{1.0788}}, kTol),
          TipStateNear({{-1, 1.3963}, {0, 1.6288}, Time{1.0844}}, kTol),
          TipStateNear({{-1, 1.4000}, {0, 1.3333}, Time{1.0872}}, kTol)));
}

TEST(StrokeEndPredictorTest, Reset) {
  StrokeEndPredictor predictor{PositionModelerParams{}, kDefaultSamplingParams};
  std::vector<TipState> prediction;

  predictor.Update({-9, 6}, Time{5});
  predictor.ConstructPrediction({{-9, 6}, {0, 0}, Time{5}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
  predictor.Update({1, 4}, Time{7});
  predictor.ConstructPrediction({{-4, 5}, {5, -1}, Time{7}}, prediction);
  EXPECT_THAT(prediction, Not(IsEmpty()));

  predictor.Reset();
  predictor.ConstructPrediction({{0, 1}, {0, 0}, Time{1}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());
}

TEST(StrokeEndPredictorTest, AlternateSamplingParams) {
  StrokeEndPredictor predictor{
      PositionModelerParams{},
      SamplingParams{.min_output_rate = 200,
                     .end_of_stroke_stopping_distance = .005}};
  std::vector<TipState> prediction;

  predictor.Update({4, -7}, Time{3});
  predictor.ConstructPrediction({{4, -7}, {0, 0}, Time{3}}, prediction);
  EXPECT_THAT(prediction, IsEmpty());

  predictor.Update({4.2, -6.8}, Time{3.01});
  predictor.ConstructPrediction({{4.1, -6.9}, {2, 2}, Time{3.01}}, prediction);
  EXPECT_THAT(
      prediction,
      ElementsAre(
          TipStateNear({{4.1138, -6.8862}, {2.7527, 2.7527}, Time{3.015}},
                       kTol),
          TipStateNear({{4.1289, -6.8711}, {3.0318, 3.0318}, Time{3.02}}, kTol),
          TipStateNear({{4.1439, -6.8561}, {2.9871, 2.9871}, Time{3.025}},
                       kTol),
          TipStateNear({{4.1576, -6.8424}, {2.7386, 2.7386}, Time{3.03}}, kTol),
          TipStateNear({{4.1694, -6.8306}, {2.3779, 2.3779}, Time{3.035}},
                       kTol),
          TipStateNear({{4.1793, -6.8207}, {1.9719, 1.9719}, Time{3.04}}, kTol),
          TipStateNear({{4.1871, -6.8129}, {1.5669, 1.5669}, Time{3.045}},
                       kTol),
          TipStateNear({{4.1931, -6.8069}, {1.1923, 1.1923}, Time{3.05}}, kTol),
          TipStateNear({{4.1974, -6.8026}, {0.8647, 0.8647}, Time{3.055}},
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
  TipState last_tip_state = {{-1, 1.1}, {0, 5}, Time{1.02}};

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
