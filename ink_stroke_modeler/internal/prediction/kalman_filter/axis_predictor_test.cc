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

#include "ink_stroke_modeler/internal/prediction/kalman_filter/axis_predictor.h"

#include <vector>

#include "gtest/gtest.h"

namespace ink {
namespace stroke_model {
namespace {

constexpr int kStableIterNum = 4;

constexpr double kProcessNoise = 0.01;
constexpr double kMeasurementNoise = 1.0;

}  // namespace

struct DataSet {
  double initial_observation;
  std::vector<double> observation;
  std::vector<double> position;
  std::vector<double> velocity;
  std::vector<double> acceleration;
  std::vector<double> jerk;
};

void ValidateAxisPredictor(AxisPredictor* predictor, const DataSet& data) {
  predictor->Reset();
  predictor->Update(data.initial_observation);
  for (decltype(data.observation.size()) i = 0; i < data.observation.size();
       i++) {
    predictor->Update(data.observation[i]);
    EXPECT_NEAR(data.position[i], predictor->GetPosition(), 0.0001);
    EXPECT_NEAR(data.velocity[i], predictor->GetVelocity(), 0.0001);
    EXPECT_NEAR(data.acceleration[i], predictor->GetAcceleration(), 0.0001);
    EXPECT_NEAR(data.jerk[i], predictor->GetJerk(), 0.0001);
  }
}

// Test that the predictor will stable.
TEST(AxisPredictorTest, ShouldStable) {
  AxisPredictor predictor(kProcessNoise, kMeasurementNoise, kStableIterNum);
  for (int i = 0; i < kStableIterNum; i++) {
    EXPECT_FALSE(predictor.Stable());
    predictor.Update(1);
  }
  EXPECT_TRUE(predictor.Stable());
}

// Test the kalman filter behavior. The data set is generated by a "known to
// work" kalman filter.
TEST(AxisPredictorTest, PredictedValue) {
  AxisPredictor predictor(kProcessNoise, kMeasurementNoise, kStableIterNum);
  DataSet data;
  data.initial_observation = 0;
  data.observation = {1, 2, 3, 4, 5, 6};
  data.position = {0.6949411066858742, 1.8880162111305765, 3.0596776689233476,
                   4.080666568886563,  5.039574058758894,  5.990101744132957};
  data.velocity = {0.48326413015846115, 1.349212968908908,  1.5150757723942188,
                   1.2449353797925855,  0.9823147273054352, 0.831418084705206};
  data.acceleration = {0.20388102703160751, 0.6602537865634062,
                       0.46392675203046707, 0.0691864035645362,
                       -0.1571001901104591, -0.2303438651979314};
  data.jerk = {0.051351580374544535, 0.17805019978769315,
               0.06592110190532013,  -0.06063794909774803,
               -0.10198612906906362, -0.09541445938944032};

  ValidateAxisPredictor(&predictor, data);

  data.initial_observation = 0;
  data.observation = {1, 2, 4, 8, 16, 32};
  data.position = {0.6949411066858742, 1.8880162111305765, 3.9597202826804603,
                   7.9052737853848285, 15.720340533540115, 31.24662046486774};
  data.velocity = {0.48326413015846115, 1.349212968908908, 2.492271225870179,
                   4.610844489557212,   8.828231877380588, 16.987494416071463};
  data.acceleration = {0.20388102703160751, 0.6602537865634062,
                       1.090991623810185,   1.885675547541351,
                       3.4586206593783526,  6.34082285106952};
  data.jerk = {0.051351580374544535, 0.17805019978769315, 0.25373225050247916,
               0.4023497012294069,   0.6945464157568688,  1.1947316519015612};

  ValidateAxisPredictor(&predictor, data);
}

TEST(AxisPredictorTest, CopyPredictor) {
  AxisPredictor predictor(kProcessNoise, kMeasurementNoise, kStableIterNum);
  predictor.Update(0);
  predictor.Update(1);
  predictor.Update(2);
  predictor.Update(4);

  AxisPredictor predictor_copy = predictor;
  EXPECT_EQ(predictor.GetPosition(), predictor_copy.GetPosition());
  EXPECT_EQ(predictor.GetVelocity(), predictor_copy.GetVelocity());
  EXPECT_EQ(predictor.GetAcceleration(), predictor_copy.GetAcceleration());
  EXPECT_EQ(predictor.GetJerk(), predictor_copy.GetJerk());

  predictor.Update(8);
  predictor_copy.Update(8);
  EXPECT_EQ(predictor.GetPosition(), predictor_copy.GetPosition());
  EXPECT_EQ(predictor.GetVelocity(), predictor_copy.GetVelocity());
  EXPECT_EQ(predictor.GetAcceleration(), predictor_copy.GetAcceleration());
  EXPECT_EQ(predictor.GetJerk(), predictor_copy.GetJerk());
}

}  // namespace stroke_model
}  // namespace ink
