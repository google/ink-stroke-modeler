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

#include "ink_stroke_modeler/stroke_modeler.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatNear;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::PrintToString;

constexpr float kTol = 1e-4;

// These parameters use cm for distance and seconds for time.
const StrokeModelParams kDefaultParams{
    .wobble_smoother_params{
        .timeout = Duration(.04), .speed_floor = 1.31, .speed_ceiling = 1.44},
    .position_modeler_params{.spring_mass_constant = 11.f / 32400,
                             .drag_constant = 72.f},
    .sampling_params{.min_output_rate = 180,
                     .end_of_stroke_stopping_distance = .001,
                     .end_of_stroke_max_iterations = 20},
    .stylus_state_modeler_params{.max_input_samples = 20},
    .prediction_params = StrokeEndPredictorParams()};

MATCHER_P2(ResultNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately match"
                                 : "approximately matches",
                        " Result (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Field("position", &Result::position,
                  Vec2Near(expected.position, tolerance)),
            Field("velocity", &Result::velocity,
                  Vec2Near(expected.velocity, tolerance)),
            Field("acceleration", &Result::acceleration,
                  Vec2Near(expected.acceleration, tolerance)),
            Field("time", &Result::time, TimeNear(expected.time, tolerance)),
            Field("pressure", &Result::pressure,
                  FloatNear(expected.pressure, tolerance)),
            Field("tilt", &Result::tilt, FloatNear(expected.tilt, tolerance)),
            Field("orientation", &Result::orientation,
                  FloatNear(expected.orientation, tolerance))),
      arg, result_listener);
}

::testing::Matcher<Result> ResultNear(const Result &expected, float tolerance) {
  return ResultNearMatcher(expected, tolerance);
}

TEST(StrokeModelerTest, NoPredictionUponInit) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());
  std::vector<Result> results;
  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, NoPredictionWithDisabledPredictor) {
  StrokeModeler modeler;
  StrokeModelParams params = kDefaultParams;
  params.prediction_params = DisabledPredictorParams{};
  EXPECT_TRUE(modeler.Reset(params).ok());
  std::vector<Result> results;
  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, InputRateSlowerThanMinOutputRate) {
  const Duration kDeltaTime{1. / 30};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {3, 4},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {3, 4}, .time = Time(0)}, kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, IsEmpty());

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {3.2, 4.2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {3.0019, 4.0019},
                                      .velocity = {0.4007, 0.4007},
                                      .acceleration = {84.1557, 84.1564},
                                      .time = Time(0.0048)},
                                     kTol),
                          ResultNear({.position = {3.0069, 4.0069},
                                      .velocity = {1.0381, 1.0381},
                                      .acceleration = {133.8378, 133.8369},
                                      .time = Time(0.0095)},
                                     kTol),
                          ResultNear({.position = {3.0154, 4.0154},
                                      .velocity = {1.7883, 1.7883},
                                      .acceleration = {157.5465, 157.5459},
                                      .time = Time(0.0143)},
                                     kTol),
                          ResultNear({.position = {3.0276, 4.0276},
                                      .velocity = {2.5626, 2.5626},
                                      .acceleration = {162.6039, 162.6021},
                                      .time = Time(0.0190)},
                                     kTol),
                          ResultNear({.position = {3.0433, 4.0433},
                                      .velocity = {3.3010, 3.3010},
                                      .acceleration = {155.0670, 155.0666},
                                      .time = Time(0.0238)},
                                     kTol),
                          ResultNear({.position = {3.0622, 4.0622},
                                      .velocity = {3.9665, 3.9665},
                                      .acceleration = {139.7575, 139.7564},
                                      .time = Time(0.0286)},
                                     kTol),
                          ResultNear({.position = {3.0838, 4.0838},
                                      .velocity = {4.5397, 4.5397},
                                      .acceleration = {120.3618, 120.3625},
                                      .time = Time(0.0333)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {3.1095, 4.1095},
                                      .velocity = {4.6253, 4.6253},
                                      .acceleration = {15.4218, 15.4223},
                                      .time = Time(0.0389)},
                                     kTol),
                          ResultNear({.position = {3.1331, 4.1331},
                                      .velocity = {4.2563, 4.2563},
                                      .acceleration = {-66.4341, -66.4339},
                                      .time = Time(0.0444)},
                                     kTol),
                          ResultNear({.position = {3.1534, 4.1534},
                                      .velocity = {3.6479, 3.6479},
                                      .acceleration = {-109.5083, -109.5081},
                                      .time = Time(0.0500)},
                                     kTol),
                          ResultNear({.position = {3.1698, 4.1698},
                                      .velocity = {2.9512, 2.9512},
                                      .acceleration = {-125.3978, -125.3976},
                                      .time = Time(0.0556)},
                                     kTol),
                          ResultNear({.position = {3.1824, 4.1824},
                                      .velocity = {2.2649, 2.2649},
                                      .acceleration = {-123.5318, -123.5310},
                                      .time = Time(0.0611)},
                                     kTol),
                          ResultNear({.position = {3.1915, 4.1915},
                                      .velocity = {1.6473, 1.6473},
                                      .acceleration = {-111.1818, -111.1806},
                                      .time = Time(0.0667)},
                                     kTol),
                          ResultNear({.position = {3.1978, 4.1978},
                                      .velocity = {1.1269, 1.1269},
                                      .acceleration = {-93.6643, -93.6636},
                                      .time = Time(0.0722)},
                                     kTol),
                          ResultNear({.position = {3.1992, 4.1992},
                                      .velocity = {1.0232, 1.0232},
                                      .acceleration = {-74.6390, -74.6392},
                                      .time = Time(0.0736)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {3.5, 4.2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {3.1086, 4.1058},
                                      .velocity = {5.2142, 4.6131},
                                      .acceleration = {141.6557, 15.4223},
                                      .time = Time(0.0381)},
                                     kTol),
                          ResultNear({.position = {3.1368, 4.1265},
                                      .velocity = {5.9103, 4.3532},
                                      .acceleration = {146.1873, -54.5680},
                                      .time = Time(0.0429)},
                                     kTol),
                          ResultNear({.position = {3.1681, 4.1450},
                                      .velocity = {6.5742, 3.8917},
                                      .acceleration = {139.4012, -96.9169},
                                      .time = Time(0.0476)},
                                     kTol),
                          ResultNear({.position = {3.2022, 4.1609},
                                      .velocity = {7.1724, 3.3285},
                                      .acceleration = {125.6306, -118.2742},
                                      .time = Time(0.0524)},
                                     kTol),
                          ResultNear({.position = {3.2388, 4.1739},
                                      .velocity = {7.6876, 2.7361},
                                      .acceleration = {108.1908, -124.4087},
                                      .time = Time(0.0571)},
                                     kTol),
                          ResultNear({.position = {3.2775, 4.1842},
                                      .velocity = {8.1138, 2.1640},
                                      .acceleration = {89.5049, -120.1309},
                                      .time = Time(0.0619)},
                                     kTol),
                          ResultNear({.position = {3.3177, 4.1920},
                                      .velocity = {8.4531, 1.6436},
                                      .acceleration = {71.2473, -109.2959},
                                      .time = Time(0.0667)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {3.3625, 4.1982},
                                      .velocity = {8.0545, 1.1165},
                                      .acceleration = {-71.7427, -94.8765},
                                      .time = Time(0.0722)},
                                     kTol),
                          ResultNear({.position = {3.4018, 4.2021},
                                      .velocity = {7.0831, 0.6987},
                                      .acceleration = {-174.8469, -75.1957},
                                      .time = Time(0.0778)},
                                     kTol),
                          ResultNear({.position = {3.4344, 4.2043},
                                      .velocity = {5.8564, 0.3846},
                                      .acceleration = {-220.8140, -56.5515},
                                      .time = Time(0.0833)},
                                     kTol),
                          ResultNear({.position = {3.4598, 4.2052},
                                      .velocity = {4.5880, 0.1611},
                                      .acceleration = {-228.3204, -40.2244},
                                      .time = Time(0.0889)},
                                     kTol),
                          ResultNear({.position = {3.4788, 4.2052},
                                      .velocity = {3.4098, 0.0124},
                                      .acceleration = {-212.0678, -26.7709},
                                      .time = Time(0.0944)},
                                     kTol),
                          ResultNear({.position = {3.4921, 4.2048},
                                      .velocity = {2.3929, -0.0780},
                                      .acceleration = {-183.0373, -16.2648},
                                      .time = Time(0.1000)},
                                     kTol),
                          ResultNear({.position = {3.4976, 4.2045},
                                      .velocity = {1.9791, -0.1015},
                                      .acceleration = {-148.9792, -8.4822},
                                      .time = Time(0.1028)},
                                     kTol),
                          ResultNear({.position = {3.5001, 4.2044},
                                      .velocity = {1.7911, -0.1098},
                                      .acceleration = {-135.3759, -5.9543},
                                      .time = Time(0.1042)},
                                     kTol)));

  time += kDeltaTime;
  // We get more results at the end of the stroke as it tries to "catch up" to
  // the raw input.
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {3.7, 4.4},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {3.3583, 4.1996},
                                      .velocity = {8.5122, 1.5925},
                                      .acceleration = {12.4129, -10.7201},
                                      .time = Time(0.0714)},
                                     kTol),
                          ResultNear({.position = {3.3982, 4.2084},
                                      .velocity = {8.3832, 1.8534},
                                      .acceleration = {-27.0783, 54.7731},
                                      .time = Time(0.0762)},
                                     kTol),
                          ResultNear({.position = {3.4369, 4.2194},
                                      .velocity = {8.1393, 2.3017},
                                      .acceleration = {-51.2222, 94.1542},
                                      .time = Time(0.0810)},
                                     kTol),
                          ResultNear({.position = {3.4743, 4.2329},
                                      .velocity = {7.8362, 2.8434},
                                      .acceleration = {-63.6668, 113.7452},
                                      .time = Time(0.0857)},
                                     kTol),
                          ResultNear({.position = {3.5100, 4.2492},
                                      .velocity = {7.5143, 3.4101},
                                      .acceleration = {-67.5926, 119.0224},
                                      .time = Time(0.0905)},
                                     kTol),
                          ResultNear({.position = {3.5443, 4.2680},
                                      .velocity = {7.2016, 3.9556},
                                      .acceleration = {-65.6568, 114.5394},
                                      .time = Time(0.0952)},
                                     kTol),
                          ResultNear({.position = {3.5773, 4.2892},
                                      .velocity = {6.9159, 4.4505},
                                      .acceleration = {-59.9999, 103.9444},
                                      .time = Time(0.1000)},
                                     kTol),
                          ResultNear({.position = {3.6115, 4.3141},
                                      .velocity = {6.1580, 4.4832},
                                      .acceleration = {-136.4312, 5.8833},
                                      .time = Time(0.1056)},
                                     kTol),
                          ResultNear({.position = {3.6400, 4.3369},
                                      .velocity = {5.1434, 4.0953},
                                      .acceleration = {-182.6254, -69.8314},
                                      .time = Time(0.1111)},
                                     kTol),
                          ResultNear({.position = {3.6626, 4.3563},
                                      .velocity = {4.0671, 3.4902},
                                      .acceleration = {-193.7401, -108.9119},
                                      .time = Time(0.1167)},
                                     kTol),
                          ResultNear({.position = {3.6796, 4.3719},
                                      .velocity = {3.0515, 2.8099},
                                      .acceleration = {-182.7957, -122.4598},
                                      .time = Time(0.1222)},
                                     kTol),
                          ResultNear({.position = {3.6916, 4.3838},
                                      .velocity = {2.1648, 2.1462},
                                      .acceleration = {-159.6116, -119.4551},
                                      .time = Time(0.1278)},
                                     kTol),
                          ResultNear({.position = {3.6996, 4.3924},
                                      .velocity = {1.4360, 1.5529},
                                      .acceleration = {-131.1906, -106.7926},
                                      .time = Time(0.1333)},
                                     kTol),
                          ResultNear({.position = {3.7028, 4.3960},
                                      .velocity = {1.1520, 1.3044},
                                      .acceleration = {-102.2117, -89.4872},
                                      .time = Time(0.1361)},
                                     kTol)));

  // The stroke is finished, so there's nothing to predict anymore.
  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, InputRateFasterThanMinOutputRate) {
  const Duration kDeltaTime{1. / 300};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{2};
  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {5, -3},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {5, -3}, .time = Time(2)}, kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, IsEmpty());

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {5, -3.1},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {5, -3.0033},
                                               .velocity = {0, -0.9818},
                                               .acceleration = {0, -294.5452},
                                               .time = Time(2.0033)},
                                              kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {5, -3.0153},
                                               .velocity = {0, -2.1719},
                                               .acceleration = {0, -214.2145},
                                               .time = Time(2.0089)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0303},
                                               .velocity = {0, -2.6885},
                                               .acceleration = {0, -92.9885},
                                               .time = Time(2.0144)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0456},
                                               .velocity = {0, -2.7541},
                                               .acceleration = {0, -11.7992},
                                               .time = Time(2.0200)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0597},
                                               .velocity = {0, -2.5430},
                                               .acceleration = {0, 37.9868},
                                               .time = Time(2.0256)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0718},
                                               .velocity = {0, -2.1852},
                                               .acceleration = {0, 64.4053},
                                               .time = Time(2.0311)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0817},
                                               .velocity = {0, -1.7719},
                                               .acceleration = {0, 74.4011},
                                               .time = Time(2.0367)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0893},
                                               .velocity = {0, -1.3628},
                                               .acceleration = {0, 73.6345},
                                               .time = Time(2.0422)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0948},
                                               .velocity = {0, -0.9934},
                                               .acceleration = {0, 66.4807},
                                               .time = Time(2.0478)},
                                              kTol),
                                   ResultNear({.position = {5, -3.0986},
                                               .velocity = {0, -0.6815},
                                               .acceleration = {0, 56.1448},
                                               .time = Time(2.0533)},
                                              kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.975, -3.175},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9992, -3.0114},
                                      .velocity = {-0.2455, -2.4322},
                                      .acceleration = {-73.6366, -435.1238},
                                      .time = Time(2.0067)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9962, -3.0344},
                                      .velocity = {-0.5430, -4.1368},
                                      .acceleration = {-53.5537, -306.8140},
                                      .time = Time(2.0122)},
                                     kTol),
                          ResultNear({.position = {4.9924, -3.0609},
                                      .velocity = {-0.6721, -4.7834},
                                      .acceleration = {-23.2474, -116.3963},
                                      .time = Time(2.0178)},
                                     kTol),
                          ResultNear({.position = {4.9886, -3.0873},
                                      .velocity = {-0.6885, -4.7365},
                                      .acceleration = {-2.9498, 8.4358},
                                      .time = Time(2.0233)},
                                     kTol),
                          ResultNear({.position = {4.9851, -3.1110},
                                      .velocity = {-0.6358, -4.2778},
                                      .acceleration = {9.4971, 82.5682},
                                      .time = Time(2.0289)},
                                     kTol),
                          ResultNear({.position = {4.9820, -3.1311},
                                      .velocity = {-0.5463, -3.6137},
                                      .acceleration = {16.1014, 119.5413},
                                      .time = Time(2.0344)},
                                     kTol),
                          ResultNear({.position = {4.9796, -3.1471},
                                      .velocity = {-0.4430, -2.8867},
                                      .acceleration = {18.6005, 130.8578},
                                      .time = Time(2.0400)},
                                     kTol),
                          ResultNear({.position = {4.9777, -3.1593},
                                      .velocity = {-0.3407, -2.1881},
                                      .acceleration = {18.4089, 125.7516},
                                      .time = Time(2.0456)},
                                     kTol),
                          ResultNear({.position = {4.9763, -3.1680},
                                      .velocity = {-0.2484, -1.5700},
                                      .acceleration = {16.6198, 111.2560},
                                      .time = Time(2.0511)},
                                     kTol),
                          ResultNear({.position = {4.9754, -3.1739},
                                      .velocity = {-0.1704, -1.0564},
                                      .acceleration = {14.0365, 92.4447},
                                      .time = Time(2.0567)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.9, -3.2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9953, -3.0237},
                                      .velocity = {-1.1603, -3.7004},
                                      .acceleration = {-274.4622, -380.4507},
                                      .time = Time(2.0100)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9828, -3.0521},
                                      .velocity = {-2.2559, -5.1049},
                                      .acceleration = {-197.1994, -252.8115},
                                      .time = Time(2.0156)},
                                     kTol),
                          ResultNear({.position = {4.9677, -3.0825},
                                      .velocity = {-2.7081, -5.4835},
                                      .acceleration = {-81.4051, -68.1520},
                                      .time = Time(2.0211)},
                                     kTol),
                          ResultNear({.position = {4.9526, -3.1115},
                                      .velocity = {-2.7333, -5.2122},
                                      .acceleration = {-4.5282, 48.8396},
                                      .time = Time(2.0267)},
                                     kTol),
                          ResultNear({.position = {4.9387, -3.1369},
                                      .velocity = {-2.4999, -4.5756},
                                      .acceleration = {42.0094, 114.5943},
                                      .time = Time(2.0322)},
                                     kTol),
                          ResultNear({.position = {4.9268, -3.1579},
                                      .velocity = {-2.1326, -3.7776},
                                      .acceleration = {66.1132, 143.6292},
                                      .time = Time(2.0378)},
                                     kTol),
                          ResultNear({.position = {4.9173, -3.1743},
                                      .velocity = {-1.7184, -2.9554},
                                      .acceleration = {74.5656, 147.9932},
                                      .time = Time(2.0433)},
                                     kTol),
                          ResultNear({.position = {4.9100, -3.1865},
                                      .velocity = {-1.3136, -2.1935},
                                      .acceleration = {72.8575, 137.1578},
                                      .time = Time(2.0489)},
                                     kTol),
                          ResultNear({.position = {4.9047, -3.1950},
                                      .velocity = {-0.9513, -1.5369},
                                      .acceleration = {65.2090, 118.1874},
                                      .time = Time(2.0544)},
                                     kTol),
                          ResultNear({.position = {4.9011, -3.2006},
                                      .velocity = {-0.6475, -1.0032},
                                      .acceleration = {54.6929, 96.0608},
                                      .time = Time(2.0600)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.825, -3.2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9868, -3.0389},
                                      .velocity = {-2.5540, -4.5431},
                                      .acceleration = {-418.1093, -252.8115},
                                      .time = Time(2.0133)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9636, -3.0687},
                                      .velocity = {-4.1801, -5.3627},
                                      .acceleration = {-292.6871, -147.5319},
                                      .time = Time(2.0189)},
                                     kTol),
                          ResultNear({.position = {4.9370, -3.0985},
                                      .velocity = {-4.7757, -5.3670},
                                      .acceleration = {-107.2116, -0.7651},
                                      .time = Time(2.0244)},
                                     kTol),
                          ResultNear({.position = {4.9109, -3.1256},
                                      .velocity = {-4.6989, -4.8816},
                                      .acceleration = {13.8210, 87.3644},
                                      .time = Time(2.0300)},
                                     kTol),
                          ResultNear({.position = {4.8875, -3.1486},
                                      .velocity = {-4.2257, -4.1466},
                                      .acceleration = {85.1835, 132.2997},
                                      .time = Time(2.0356)},
                                     kTol),
                          ResultNear({.position = {4.8677, -3.1671},
                                      .velocity = {-3.5576, -3.3287},
                                      .acceleration = {120.2579, 147.2335},
                                      .time = Time(2.0411)},
                                     kTol),
                          ResultNear({.position = {4.8520, -3.1812},
                                      .velocity = {-2.8333, -2.5353},
                                      .acceleration = {130.3700, 142.8088},
                                      .time = Time(2.0467)},
                                     kTol),
                          ResultNear({.position = {4.8401, -3.1914},
                                      .velocity = {-2.1411, -1.8288},
                                      .acceleration = {124.5846, 127.1714},
                                      .time = Time(2.0522)},
                                     kTol),
                          ResultNear({.position = {4.8316, -3.1982},
                                      .velocity = {-1.5312, -1.2386},
                                      .acceleration = {109.7874, 106.2279},
                                      .time = Time(2.0578)},
                                     kTol),
                          ResultNear({.position = {4.8280, -3.2010},
                                      .velocity = {-1.2786, -1.0053},
                                      .acceleration = {90.9288, 84.0051},
                                      .time = Time(2.0606)},
                                     kTol),
                          ResultNear({.position = {4.8272, -3.2017},
                                      .velocity = {-1.2209, -0.9529},
                                      .acceleration = {83.2052, 75.4288},
                                      .time = Time(2.0613)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.75, -3.225},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9726, -3.0565},
                                      .velocity = {-4.2660, -5.2803},
                                      .acceleration = {-513.5957, -221.1678},
                                      .time = Time(2.0167)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9381, -3.0894},
                                      .velocity = {-6.2018, -5.9261},
                                      .acceleration = {-348.4476, -116.2445},
                                      .time = Time(2.0222)},
                                     kTol),
                          ResultNear({.position = {4.9004, -3.1215},
                                      .velocity = {-6.7995, -5.7749},
                                      .acceleration = {-107.5834, 27.2264},
                                      .time = Time(2.0278)},
                                     kTol),
                          ResultNear({.position = {4.8640, -3.1501},
                                      .velocity = {-6.5400, -5.1591},
                                      .acceleration = {46.7146, 110.8336},
                                      .time = Time(2.0333)},
                                     kTol),
                          ResultNear({.position = {4.8319, -3.1741},
                                      .velocity = {-5.7897, -4.3207},
                                      .acceleration = {135.0462, 150.9226},
                                      .time = Time(2.0389)},
                                     kTol),
                          ResultNear({.position = {4.8051, -3.1932},
                                      .velocity = {-4.8132, -3.4248},
                                      .acceleration = {175.7684, 161.2555},
                                      .time = Time(2.0444)},
                                     kTol),
                          ResultNear({.position = {4.7841, -3.2075},
                                      .velocity = {-3.7898, -2.5759},
                                      .acceleration = {184.2227, 152.7958},
                                      .time = Time(2.0500)},
                                     kTol),
                          ResultNear({.position = {4.7683, -3.2176},
                                      .velocity = {-2.8312, -1.8324},
                                      .acceleration = {172.5480, 133.8294},
                                      .time = Time(2.0556)},
                                     kTol),
                          ResultNear({.position = {4.7572, -3.2244},
                                      .velocity = {-1.9986, -1.2198},
                                      .acceleration = {149.8577, 110.2830},
                                      .time = Time(2.0611)},
                                     kTol),
                          ResultNear({.position = {4.7526, -3.2271},
                                      .velocity = {-1.6580, -0.9805},
                                      .acceleration = {122.6198, 86.1299},
                                      .time = Time(2.0639)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.7, -3.3},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9529, -3.0778},
                                      .velocity = {-5.9184, -6.4042},
                                      .acceleration = {-495.7209, -337.1538},
                                      .time = Time(2.0200)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9101, -3.1194},
                                      .velocity = {-7.6886, -7.4784},
                                      .acceleration = {-318.6394, -193.3594},
                                      .time = Time(2.0256)},
                                     kTol),
                          ResultNear({.position = {4.8654, -3.1607},
                                      .velocity = {-8.0518, -7.4431},
                                      .acceleration = {-65.3698, 6.3579},
                                      .time = Time(2.0311)},
                                     kTol),
                          ResultNear({.position = {4.8235, -3.1982},
                                      .velocity = {-7.5377, -6.7452},
                                      .acceleration = {92.5345, 125.6104},
                                      .time = Time(2.0367)},
                                     kTol),
                          ResultNear({.position = {4.7872, -3.2299},
                                      .velocity = {-6.5440, -5.7133},
                                      .acceleration = {178.8654, 185.7426},
                                      .time = Time(2.0422)},
                                     kTol),
                          ResultNear({.position = {4.7574, -3.2553},
                                      .velocity = {-5.3529, -4.5748},
                                      .acceleration = {214.4027, 204.9362},
                                      .time = Time(2.0478)},
                                     kTol),
                          ResultNear({.position = {4.7344, -3.2746},
                                      .velocity = {-4.1516, -3.4758},
                                      .acceleration = {216.2348, 197.8224},
                                      .time = Time(2.0533)},
                                     kTol),
                          ResultNear({.position = {4.7174, -3.2885},
                                      .velocity = {-3.0534, -2.5004},
                                      .acceleration = {197.6767, 175.5702},
                                      .time = Time(2.0589)},
                                     kTol),
                          ResultNear({.position = {4.7056, -3.2979},
                                      .velocity = {-2.1169, -1.6879},
                                      .acceleration = {168.5711, 146.2573},
                                      .time = Time(2.0644)},
                                     kTol),
                          ResultNear({.position = {4.7030, -3.3000},
                                      .velocity = {-1.9283, -1.5276},
                                      .acceleration = {135.7820, 115.3739},
                                      .time = Time(2.0658)},
                                     kTol),
                          ResultNear({.position = {4.7017, -3.3010},
                                      .velocity = {-1.8380, -1.4512},
                                      .acceleration = {130.0928, 110.0859},
                                      .time = Time(2.0665)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.675, -3.4},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9288, -3.1046},
                                      .velocity = {-7.2260, -8.0305},
                                      .acceleration = {-392.2747, -487.9053},
                                      .time = Time(2.0233)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.8816, -3.1582},
                                      .velocity = {-8.4881, -9.6525},
                                      .acceleration = {-227.1831, -291.9628},
                                      .time = Time(2.0289)},
                                     kTol),
                          ResultNear({.position = {4.8345, -3.2124},
                                      .velocity = {-8.4738, -9.7482},
                                      .acceleration = {2.5870, -17.2266},
                                      .time = Time(2.0344)},
                                     kTol),
                          ResultNear({.position = {4.7918, -3.2619},
                                      .velocity = {-7.6948, -8.9195},
                                      .acceleration = {140.2131, 149.1810},
                                      .time = Time(2.0400)},
                                     kTol),
                          ResultNear({.position = {4.7555, -3.3042},
                                      .velocity = {-6.5279, -7.6113},
                                      .acceleration = {210.0428, 235.4638},
                                      .time = Time(2.0456)},
                                     kTol),
                          ResultNear({.position = {4.7264, -3.3383},
                                      .velocity = {-5.2343, -6.1345},
                                      .acceleration = {232.8451, 265.8274},
                                      .time = Time(2.0511)},
                                     kTol),
                          ResultNear({.position = {4.7043, -3.3643},
                                      .velocity = {-3.9823, -4.6907},
                                      .acceleration = {225.3593, 259.8790},
                                      .time = Time(2.0567)},
                                     kTol),
                          ResultNear({.position = {4.6884, -3.3832},
                                      .velocity = {-2.8691, -3.3980},
                                      .acceleration = {200.3802, 232.6849},
                                      .time = Time(2.0622)},
                                     kTol),
                          ResultNear({.position = {4.6776, -3.3961},
                                      .velocity = {-1.9403, -2.3135},
                                      .acceleration = {167.1764, 195.2152},
                                      .time = Time(2.0678)},
                                     kTol),
                          ResultNear({.position = {4.6752, -3.3990},
                                      .velocity = {-1.7569, -2.0983},
                                      .acceleration = {132.0560, 154.9868},
                                      .time = Time(2.0692)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {4.675, -3.525},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.9022, -3.1387},
                                      .velocity = {-7.9833, -10.2310},
                                      .acceleration = {-227.1831, -660.1446},
                                      .time = Time(2.0267)},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.8549, -3.2079},
                                      .velocity = {-8.5070, -12.4602},
                                      .acceleration = {-94.2781, -401.2599},
                                      .time = Time(2.0322)},
                                     kTol),
                          ResultNear({.position = {4.8102, -3.2783},
                                      .velocity = {-8.0479, -12.6650},
                                      .acceleration = {82.6390, -36.8616},
                                      .time = Time(2.0378)},
                                     kTol),
                          ResultNear({.position = {4.7711, -3.3429},
                                      .velocity = {-7.0408, -11.6365},
                                      .acceleration = {181.2765, 185.1286},
                                      .time = Time(2.0433)},
                                     kTol),
                          ResultNear({.position = {4.7389, -3.3983},
                                      .velocity = {-5.7965, -9.9616},
                                      .acceleration = {223.9801, 301.4933},
                                      .time = Time(2.0489)},
                                     kTol),
                          ResultNear({.position = {4.7137, -3.4430},
                                      .velocity = {-4.5230, -8.0510},
                                      .acceleration = {229.2397, 343.9032},
                                      .time = Time(2.0544)},
                                     kTol),
                          ResultNear({.position = {4.6951, -3.4773},
                                      .velocity = {-3.3477, -6.1727},
                                      .acceleration = {211.5554, 338.0856},
                                      .time = Time(2.0600)},
                                     kTol),
                          ResultNear({.position = {4.6821, -3.5022},
                                      .velocity = {-2.3381, -4.4846},
                                      .acceleration = {181.7131, 303.8597},
                                      .time = Time(2.0656)},
                                     kTol),
                          ResultNear({.position = {4.6737, -3.5192},
                                      .velocity = {-1.5199, -3.0641},
                                      .acceleration = {147.2879, 255.7003},
                                      .time = Time(2.0711)},
                                     kTol),
                          ResultNear({.position = {4.6718, -3.5231},
                                      .velocity = {-1.3626, -2.7813},
                                      .acceleration = {113.2437, 203.5595},
                                      .time = Time(2.0725)},
                                     kTol)));

  time += kDeltaTime;
  // We get more results at the end of the stroke as it tries to "catch up" to
  // the raw input.
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {4.7, -3.6},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {4.8753, -3.1797},
                                      .velocity = {-8.0521, -12.3049},
                                      .acceleration = {-20.6429, -622.1685},
                                      .time = Time(2.0300)},
                                     kTol),
                          ResultNear({.position = {4.8325, -3.2589},
                                      .velocity = {-7.7000, -14.2607},
                                      .acceleration = {63.3680, -352.0363},
                                      .time = Time(2.0356)},
                                     kTol),
                          ResultNear({.position = {4.7948, -3.3375},
                                      .velocity = {-6.7888, -14.1377},
                                      .acceleration = {164.0215, 22.1350},
                                      .time = Time(2.0411)},
                                     kTol),
                          ResultNear({.position = {4.7636, -3.4085},
                                      .velocity = {-5.6249, -12.7787},
                                      .acceleration = {209.5020, 244.6249},
                                      .time = Time(2.0467)},
                                     kTol),
                          ResultNear({.position = {4.7390, -3.4685},
                                      .velocity = {-4.4152, -10.8015},
                                      .acceleration = {217.7452, 355.8801},
                                      .time = Time(2.0522)},
                                     kTol),
                          ResultNear({.position = {4.7208, -3.5164},
                                      .velocity = {-3.2880, -8.6333},
                                      .acceleration = {202.8961, 390.2804},
                                      .time = Time(2.0578)},
                                     kTol),
                          ResultNear({.position = {4.7079, -3.5528},
                                      .velocity = {-2.3128, -6.5475},
                                      .acceleration = {175.5414, 375.4407},
                                      .time = Time(2.0633)},
                                     kTol),
                          ResultNear({.position = {4.6995, -3.5789},
                                      .velocity = {-1.5174, -4.7008},
                                      .acceleration = {143.1705, 332.4062},
                                      .time = Time(2.0689)},
                                     kTol),
                          ResultNear({.position = {4.6945, -3.5965},
                                      .velocity = {-0.9022, -3.1655},
                                      .acceleration = {110.7325, 276.3669},
                                      .time = Time(2.0744)},
                                     kTol),
                          ResultNear({.position = {4.6942, -3.5976},
                                      .velocity = {-0.8740, -3.0899},
                                      .acceleration = {81.2036, 217.6189},
                                      .time = Time(2.0748)},
                                     kTol)));

  // The stroke is finished, so there's nothing to predict anymore.
  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, WobbleSmoothed) {
  const Duration kDeltaTime{.0167};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{4};
  std::vector<Result> results;

  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {-6, -2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {-6, -2}, .time = Time(4)}, kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {-6.02, -2},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {-6.0003, -2},
                                               .velocity = {-0.0615, 0},
                                               .acceleration = {-14.7276, 0},
                                               .time = Time(4.0042)},
                                              kTol),
                                   ResultNear({.position = {-6.0009, -2},
                                               .velocity = {-0.1628, 0},
                                               .acceleration = {-24.2725, 0},
                                               .time = Time(4.0084)},
                                              kTol),
                                   ResultNear({.position = {-6.0021, -2},
                                               .velocity = {-0.2868, 0},
                                               .acceleration = {-29.6996, 0},
                                               .time = Time(4.0125)},
                                              kTol),
                                   ResultNear({.position = {-6.0039, -2},
                                               .velocity = {-0.4203, 0},
                                               .acceleration = {-31.9728, 0},
                                               .time = Time(4.0167)},
                                              kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {-6.02, -2.02},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {-6.0059, -2.0001},
                                      .velocity = {-0.4921, -0.0307},
                                      .acceleration = {-17.1932, -7.3638},
                                      .time = Time(4.0209)},
                                     kTol),
                          ResultNear({.position = {-6.0081, -2.0005},
                                      .velocity = {-0.5170, -0.0814},
                                      .acceleration = {-5.9729, -12.1355},
                                      .time = Time(4.0251)},
                                     kTol),
                          ResultNear({.position = {-6.0102, -2.0010},
                                      .velocity = {-0.5079, -0.1434},
                                      .acceleration = {2.1807, -14.8493},
                                      .time = Time(4.0292)},
                                     kTol),
                          ResultNear({.position = {-6.0122, -2.0019},
                                      .velocity = {-0.4755, -0.2101},
                                      .acceleration = {7.7710, -15.9860},
                                      .time = Time(4.0334)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {-6.04, -2.02},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {-6.0141, -2.0030},
                                      .velocity = {-0.4489, -0.2563},
                                      .acceleration = {6.3733, -11.0507},
                                      .time = Time(4.0376)},
                                     kTol),
                          ResultNear({.position = {-6.0159, -2.0042},
                                      .velocity = {-0.4277, -0.2856},
                                      .acceleration = {5.0670, -7.0315},
                                      .time = Time(4.0418)},
                                     kTol),
                          ResultNear({.position = {-6.0176, -2.0055},
                                      .velocity = {-0.4115, -0.3018},
                                      .acceleration = {3.8950, -3.8603},
                                      .time = Time(4.0459)},
                                     kTol),
                          ResultNear({.position = {-6.0193, -2.0067},
                                      .velocity = {-0.3994, -0.3078},
                                      .acceleration = {2.8758, -1.4435},
                                      .time = Time(4.0501)},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {-6.04, -2.04},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {-6.0209, -2.0082},
                                      .velocity = {-0.3910, -0.3372},
                                      .acceleration = {2.0142, -7.0427},
                                      .time = Time(4.0543)},
                                     kTol),
                          ResultNear({.position = {-6.0225, -2.0098},
                                      .velocity = {-0.3856, -0.3814},
                                      .acceleration = {1.3090, -10.5977},
                                      .time = Time(4.0585)},
                                     kTol),
                          ResultNear({.position = {-6.0241, -2.0116},
                                      .velocity = {-0.3825, -0.4338},
                                      .acceleration = {0.7470, -12.5399},
                                      .time = Time(4.0626)},
                                     kTol),
                          ResultNear({.position = {-6.0257, -2.0136},
                                      .velocity = {-0.3811, -0.4891},
                                      .acceleration = {0.3174, -13.2543},
                                      .time = Time(4.0668)},
                                     kTol)));
}

TEST(StrokeModelerTest, UpdateAppendsToResults) {
  const Duration kDeltaTime{1. / 300};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{2};
  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {5, -3},
                           .time = time},
                          results)
                  .ok());
  auto first_result_matcher =
      ResultNear({.position = {5, -3}, .time = Time(2)}, kTol);
  EXPECT_THAT(results, ElementsAre(first_result_matcher));

  time += kDeltaTime;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {5, -3.1},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(first_result_matcher,
                                   ResultNear({.position = {5, -3.0033},
                                               .velocity = {0, -0.9818},
                                               .acceleration = {0, -294.5452},
                                               .time = Time(2.0033)},
                                              kTol)));
}

TEST(StrokeModelerTest, Reset) {
  const Duration kDeltaTime{1. / 50};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  std::vector<Result> results;

  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {-8, -10},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, IsEmpty());

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove, .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {-11, -5},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());
  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, IgnoreInputsBeforeTDown) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {0, 0},
                         .time = Time(0)},
                        results)
                .code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(results, IsEmpty());

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {0, 0},
                         .time = Time(1)},
                        results)
                .code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, IgnoreTDownWhileStrokeIsInProgress) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;

  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = Time(0)},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {1, 1},
                         .time = Time(1)},
                        results)
                .code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(results, IsEmpty());

  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {1, 1},
                           .time = Time(1)},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {2, 2},
                         .time = Time(2)},
                        results)
                .code(),
            absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, AlternateParams) {
  const Duration kDeltaTime{1. / 50};

  StrokeModelParams stroke_model_params = kDefaultParams;
  stroke_model_params.sampling_params.min_output_rate = 70;

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(stroke_model_params).ok());

  Time time{3};
  std::vector<Result> results;

  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = time,
                           .pressure = .5,
                           .tilt = .2,
                           .orientation = .4},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear(
                  {{0, 0}, {0, 0}, {0, 0}, Time{3}, .5, .2, .4}, kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, IsEmpty());

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {0, .5},
                           .time = time,
                           .pressure = .4,
                           .tilt = .3,
                           .orientation = .3},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {0, 0.0736},
                                               .velocity = {0, 7.3636},
                                               .acceleration = {0, 736.3636},
                                               .time = Time(3.0100),
                                               .pressure = 0.4853,
                                               .tilt = 0.2147,
                                               .orientation = 0.3853},
                                              kTol),
                                   ResultNear({.position = {0, 0.2198},
                                               .velocity = {0, 14.6202},
                                               .acceleration = {0, 725.6529},
                                               .time = Time(3.0200),
                                               .pressure = 0.4560,
                                               .tilt = 0.2440,
                                               .orientation = 0.3560},
                                              kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {0, 0.3823},
                                               .velocity = {0, 11.3709},
                                               .acceleration = {0, -227.4474},
                                               .time = Time(3.0343),
                                               .pressure = 0.4235,
                                               .tilt = 0.2765,
                                               .orientation = 0.3235},
                                              kTol),
                                   ResultNear({.position = {0, 0.4484},
                                               .velocity = {0, 4.6285},
                                               .acceleration = {0, -471.9660},
                                               .time = Time(3.0486),
                                               .pressure = 0.4103,
                                               .tilt = 0.2897,
                                               .orientation = 0.3103},
                                              kTol),
                                   ResultNear({.position = {0, 0.4775},
                                               .velocity = {0, 2.0389},
                                               .acceleration = {0, -181.2747},
                                               .time = Time(3.0629),
                                               .pressure = 0.4045,
                                               .tilt = 0.2955,
                                               .orientation = 0.3045},
                                              kTol),
                                   ResultNear({.position = {0, 0.4902},
                                               .velocity = {0, 0.8873},
                                               .acceleration = {0, -80.6136},
                                               .time = Time(3.0771),
                                               .pressure = 0.4020,
                                               .tilt = 0.2980,
                                               .orientation = 0.3020},
                                              kTol),
                                   ResultNear({.position = {0, 0.4957},
                                               .velocity = {0, 0.3868},
                                               .acceleration = {0, -35.0318},
                                               .time = Time(3.0914),
                                               .pressure = 0.4009,
                                               .tilt = 0.2991,
                                               .orientation = 0.3009},
                                              kTol),
                                   ResultNear({.position = {0, 0.4981},
                                               .velocity = {0, 0.1686},
                                               .acceleration = {0, -15.2760},
                                               .time = Time(3.1057),
                                               .pressure = 0.4004,
                                               .tilt = 0.2996,
                                               .orientation = 0.3004},
                                              kTol),
                                   ResultNear({.position = {0, 0.4992},
                                               .velocity = {0, 0.0735},
                                               .acceleration = {0, -6.6579},
                                               .time = Time(3.1200),
                                               .pressure = 0.4002,
                                               .tilt = 0.2998,
                                               .orientation = 0.3002},
                                              kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {.2, 1},
                           .time = time,
                           .pressure = .3,
                           .tilt = .4,
                           .orientation = .2},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {0.0295, 0.4169},
                                      .velocity = {2.9455, 19.7093},
                                      .acceleration = {294.5455, 508.9161},
                                      .time = Time(3.0300),
                                      .pressure = 0.4166,
                                      .tilt = 0.2834,
                                      .orientation = 0.3166},
                                     kTol),
                          ResultNear({.position = {0.0879, 0.6439},
                                      .velocity = {5.8481, 22.6926},
                                      .acceleration = {290.2612, 298.3311},
                                      .time = Time(3.0400),
                                      .pressure = 0.3691,
                                      .tilt = 0.3309,
                                      .orientation = 0.2691},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {0.1529, 0.8487},
                                      .velocity = {4.5484, 14.3374},
                                      .acceleration = {-90.9790, -584.8687},
                                      .time = Time(3.0543),
                                      .pressure = 0.3293,
                                      .tilt = 0.3707,
                                      .orientation = 0.2293},
                                     kTol),
                          ResultNear({.position = {0.1794, 0.9338},
                                      .velocity = {1.8514, 5.9577},
                                      .acceleration = {-188.7864, -586.5760},
                                      .time = Time(3.0686),
                                      .pressure = 0.3128,
                                      .tilt = 0.3872,
                                      .orientation = 0.2128},
                                     kTol),
                          ResultNear({.position = {0.1910, 0.9712},
                                      .velocity = {0.8156, 2.6159},
                                      .acceleration = {-72.5099, -233.9289},
                                      .time = Time(3.0829),
                                      .pressure = 0.3056,
                                      .tilt = 0.3944,
                                      .orientation = 0.2056},
                                     kTol),
                          ResultNear({.position = {0.1961, 0.9874},
                                      .velocity = {0.3549, 1.1389},
                                      .acceleration = {-32.2455, -103.3868},
                                      .time = Time(3.0971),
                                      .pressure = 0.3024,
                                      .tilt = 0.3976,
                                      .orientation = 0.2024},
                                     kTol),
                          ResultNear({.position = {0.1983, 0.9945},
                                      .velocity = {0.1547, 0.4965},
                                      .acceleration = {-14.0127, -44.9693},
                                      .time = Time(3.1114),
                                      .pressure = 0.3011,
                                      .tilt = 0.3989,
                                      .orientation = 0.2011},
                                     kTol),
                          ResultNear({.position = {0.1993, 0.9976},
                                      .velocity = {0.0674, 0.2164},
                                      .acceleration = {-6.1104, -19.6068},
                                      .time = Time(3.1257),
                                      .pressure = 0.3005,
                                      .tilt = 0.3995,
                                      .orientation = 0.2005},
                                     kTol),
                          ResultNear({.position = {0.1997, 0.9990},
                                      .velocity = {0.0294, 0.0943},
                                      .acceleration = {-2.6631, -8.5455},
                                      .time = Time(3.1400),
                                      .pressure = 0.3002,
                                      .tilt = 0.3998,
                                      .orientation = 0.2002},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {.4, 1.4},
                           .time = time,
                           .pressure = .2,
                           .tilt = .7,
                           .orientation = 0},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {0.1668, 0.8712},
                                      .velocity = {7.8837, 22.7349},
                                      .acceleration = {203.5665, 4.2224},
                                      .time = Time(3.0500),
                                      .pressure = 0.3245,
                                      .tilt = 0.3755,
                                      .orientation = 0.2245},
                                     kTol),
                          ResultNear({.position = {0.2575, 1.0906},
                                      .velocity = {9.0771, 21.9411},
                                      .acceleration = {119.3324, -79.3721},
                                      .time = Time(3.0600),
                                      .pressure = 0.2761,
                                      .tilt = 0.4716,
                                      .orientation = 0.1522},
                                     kTol)));

  ASSERT_TRUE(modeler.Predict(results).ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {0.3395, 1.2676},
                                      .velocity = {5.7349, 12.3913},
                                      .acceleration = {-233.9475, -668.4906},
                                      .time = Time(3.0743),
                                      .pressure = 0.2325,
                                      .tilt = 0.6024,
                                      .orientation = 0.0651},
                                     kTol),
                          ResultNear({.position = {0.3735, 1.3421},
                                      .velocity = {2.3831, 5.2156},
                                      .acceleration = {-234.6304, -502.2992},
                                      .time = Time(3.0886),
                                      .pressure = 0.2142,
                                      .tilt = 0.6573,
                                      .orientation = 0.0284},
                                     kTol),
                          ResultNear({.position = {0.3885, 1.3748},
                                      .velocity = {1.0463, 2.2854},
                                      .acceleration = {-93.5716, -205.1091},
                                      .time = Time(3.1029),
                                      .pressure = 0.2062,
                                      .tilt = 0.6814,
                                      .orientation = 0.0124},
                                     kTol),
                          ResultNear({.position = {0.3950, 1.3890},
                                      .velocity = {0.4556, 0.9954},
                                      .acceleration = {-41.3547, -90.3064},
                                      .time = Time(3.1171),
                                      .pressure = 0.2027,
                                      .tilt = 0.6919,
                                      .orientation = 0.0054},
                                     kTol),
                          ResultNear({.position = {0.3978, 1.3952},
                                      .velocity = {0.1986, 0.4339},
                                      .acceleration = {-17.9877, -39.3021},
                                      .time = Time(3.1314),
                                      .pressure = 0.2012,
                                      .tilt = 0.6965,
                                      .orientation = 0.0024},
                                     kTol),
                          ResultNear({.position = {0.3990, 1.3979},
                                      .velocity = {0.0866, 0.1891},
                                      .acceleration = {-7.8428, -17.1346},
                                      .time = Time(3.1457),
                                      .pressure = 0.2005,
                                      .tilt = 0.6985,
                                      .orientation = 0.0010},
                                     kTol),
                          ResultNear({.position = {0.3996, 1.3991},
                                      .velocity = {0.0377, 0.0824},
                                      .acceleration = {-3.4182, -7.4680},
                                      .time = Time(3.1600),
                                      .pressure = 0.2002,
                                      .tilt = 0.6993,
                                      .orientation = 0.0004},
                                     kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {.7, 1.7},
                           .time = time,
                           .pressure = .1,
                           .tilt = 1,
                           .orientation = 0},
                          results)
                  .ok());
  EXPECT_THAT(results,
              ElementsAre(ResultNear({.position = {0.3691, 1.2874},
                                      .velocity = {11.1558, 19.6744},
                                      .acceleration = {207.8707, -226.6725},
                                      .time = Time(3.0700),
                                      .pressure = 0.2256,
                                      .tilt = 0.6231,
                                      .orientation = 0.0512},
                                     kTol),
                          ResultNear({.position = {0.4978, 1.4640},
                                      .velocity = {12.8701, 17.6629},
                                      .acceleration = {171.4340, -201.1508},
                                      .time = Time(3.0800),
                                      .pressure = 0.1730,
                                      .tilt = 0.7809,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6141, 1.5986},
                                      .velocity = {8.1404, 9.4261},
                                      .acceleration = {-331.0815, -576.5752},
                                      .time = Time(3.0943),
                                      .pressure = 0.1312,
                                      .tilt = 0.9064,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6624, 1.6557},
                                      .velocity = {3.3822, 3.9953},
                                      .acceleration = {-333.0701, -380.1579},
                                      .time = Time(3.1086),
                                      .pressure = 0.1136,
                                      .tilt = 0.9591,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6836, 1.6807},
                                      .velocity = {1.4851, 1.7488},
                                      .acceleration = {-132.8005, -157.2520},
                                      .time = Time(3.1229),
                                      .pressure = 0.1059,
                                      .tilt = 0.9822,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6929, 1.6916},
                                      .velocity = {0.6466, 0.7618},
                                      .acceleration = {-58.6943, -69.0946},
                                      .time = Time(3.1371),
                                      .pressure = 0.1026,
                                      .tilt = 0.9922,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6969, 1.6963},
                                      .velocity = {0.2819, 0.3321},
                                      .acceleration = {-25.5298, -30.0794},
                                      .time = Time(3.1514),
                                      .pressure = 0.1011,
                                      .tilt = 0.9966,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6986, 1.6984},
                                      .velocity = {0.1229, 0.1447},
                                      .acceleration = {-11.1311, -13.1133},
                                      .time = Time(3.1657),
                                      .pressure = 0.1005,
                                      .tilt = 0.9985,
                                      .orientation = 0},
                                     kTol),
                          ResultNear({.position = {0.6994, 1.6993},
                                      .velocity = {0.0535, 0.0631},
                                      .acceleration = {-4.8514, -5.7153},
                                      .time = Time(3.1800),
                                      .pressure = 0.1002,
                                      .tilt = 0.9994,
                                      .orientation = 0},
                                     kTol)));

  EXPECT_EQ(modeler.Predict(results).code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, GenerateOutputOnTUpEvenIfNoTimeDelta) {
  const Duration kDeltaTime{1. / 500};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  std::vector<Result> results;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {5, 5},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {5, 5}, .time = Time(0)}, kTol)));

  time += kDeltaTime;
  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {5, 5},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {5, 5}, .time = Time(0.002)}, kTol)));

  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {5, 5},
                           .time = time},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {5, 5}, .time = Time(0.0076)}, kTol)));
}

TEST(StrokeModelerTest, RejectInputIfNegativeTimeDelta) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = Time(0)},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {1, 1},
                         .time = Time(-.1)},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());

  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {1, 1},
                           .time = Time(1)},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {1, 1},
                         .time = Time(.9)},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, RejectDuplicateInput) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = Time(0),
                           .pressure = .2,
                           .tilt = .3,
                           .orientation = .4},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {0, 0},
                         .time = Time(0),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());

  results.clear();
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kMove,
                           .position = {1, 2},
                           .time = Time(1),
                           .pressure = .1,
                           .tilt = .2,
                           .orientation = .3},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {1, 2},
                         .time = Time(1),
                         .pressure = .1,
                         .tilt = .2,
                         .orientation = .3},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, FarApartTimesDoNotCrashForMove) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = Time(0),
                           .pressure = .2,
                           .tilt = .3,
                           .orientation = .4},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {0, 0},
                         .time = Time(INT_MAX),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, FarApartTimesDoNotCrashForUp) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;
  ASSERT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {0, 0},
                           .time = Time(0),
                           .pressure = .2,
                           .tilt = .3,
                           .orientation = .4},
                          results)
                  .ok());
  EXPECT_THAT(results, Not(IsEmpty()));

  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {0, 0},
                         .time = Time(INT_MAX),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4},
                        results)
                .code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(results, IsEmpty());
}

TEST(StrokeModelerTest, FirstResetMustPassParams) {
  StrokeModeler modeler;
  EXPECT_EQ(modeler.Reset().code(), absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, ResetKeepsParamsAndResetsStroke) {
  StrokeModeler modeler;
  // Initialize with parameters and update.
  Input pointer_down{.event_type = Input::EventType::kDown,
                     .position = {3, 4},
                     .time = Time(0)};
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());
  std::vector<Result> results;
  ASSERT_TRUE(modeler.Update(pointer_down, results).ok());

  // Reset, using the same parameters.
  ASSERT_TRUE(modeler.Reset().ok());

  // Doesn't object to seeing a duplicate input or another down event, since
  // the previous stroke in progress was aborted by the call to reset.
  results.clear();
  ASSERT_TRUE(modeler.Update(pointer_down, results).ok());
}

TEST(StrokeModelerTest, SaveAndRestore) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  std::vector<Result> results;

  // Create a save that will be overwritten.
  modeler.Save();

  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {-6, -2},
                           .time = Time(4)},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {-6, -2}, .time = Time(4)}, kTol)));

  // Save a second time and then finish the stroke.
  modeler.Save();

  results.clear();
  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {-6.02, -2},
                           .time = Time(4.0167)},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {-6.0003, -2},
                                               .velocity = {-0.0615, 0},
                                               .acceleration = {-14.7276, 0},
                                               .time = Time(4.0042)},
                                              kTol),
                                   ResultNear({.position = {-6.0009, -2},
                                               .velocity = {-0.1628, 0},
                                               .acceleration = {-24.2725, 0},
                                               .time = Time(4.0084)},
                                              kTol),
                                   ResultNear({.position = {-6.0021, -2},
                                               .velocity = {-0.2868, 0},
                                               .acceleration = {-29.6996, 0},
                                               .time = Time(4.0125)},
                                              kTol),
                                   ResultNear({.position = {-6.0039, -2},
                                               .velocity = {-0.4203, 0},
                                               .acceleration = {-31.9728, 0},
                                               .time = Time(4.0167)},
                                              kTol),
                                   ResultNear({.position = {-6.0068, -2},
                                               .velocity = {-0.5158, 0},
                                               .acceleration = {-17.1932, 0},
                                               .time = Time(4.0223)},
                                              kTol),
                                   ResultNear({.position = {-6.0097, -2},
                                               .velocity = {-0.5262, 0},
                                               .acceleration = {-1.8749, 0},
                                               .time = Time(4.0278)},
                                              kTol),
                                   ResultNear({.position = {-6.0124, -2},
                                               .velocity = {-0.4847, 0},
                                               .acceleration = {7.4861, 0},
                                               .time = Time(4.0334)},
                                              kTol),
                                   ResultNear({.position = {-6.0147, -2},
                                               .velocity = {-0.4156, 0},
                                               .acceleration = {12.4229, 0},
                                               .time = Time(4.0389)},
                                              kTol),
                                   ResultNear({.position = {-6.0165, -2},
                                               .velocity = {-0.3364, 0},
                                               .acceleration = {14.2557, 0},
                                               .time = Time(4.0445)},
                                              kTol),
                                   ResultNear({.position = {-6.0180, -2},
                                               .velocity = {-0.2583, 0},
                                               .acceleration = {14.0591, 0},
                                               .time = Time(4.0500)},
                                              kTol),
                                   ResultNear({.position = {-6.0190, -2},
                                               .velocity = {-0.1880, 0},
                                               .acceleration = {12.6630, 0},
                                               .time = Time(4.0556)},
                                              kTol)));

  // Restore and finish the stroke again.
  modeler.Restore();
  results.clear();
  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {-6.02, -2},
                           .time = Time(4.0167)},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {-6.0003, -2},
                                               .velocity = {-0.0615, 0},
                                               .acceleration = {-14.7276, 0},
                                               .time = Time(4.0042)},
                                              kTol),
                                   ResultNear({.position = {-6.0009, -2},
                                               .velocity = {-0.1628, 0},
                                               .acceleration = {-24.2725, 0},
                                               .time = Time(4.0084)},
                                              kTol),
                                   ResultNear({.position = {-6.0021, -2},
                                               .velocity = {-0.2868, 0},
                                               .acceleration = {-29.6996, 0},
                                               .time = Time(4.0125)},
                                              kTol),
                                   ResultNear({.position = {-6.0039, -2},
                                               .velocity = {-0.4203, 0},
                                               .acceleration = {-31.9728, 0},
                                               .time = Time(4.0167)},
                                              kTol),
                                   ResultNear({.position = {-6.0068, -2},
                                               .velocity = {-0.5158, 0},
                                               .acceleration = {-17.1932, 0},
                                               .time = Time(4.0223)},
                                              kTol),
                                   ResultNear({.position = {-6.0097, -2},
                                               .velocity = {-0.5262, 0},
                                               .acceleration = {-1.8749, 0},
                                               .time = Time(4.0278)},
                                              kTol),
                                   ResultNear({.position = {-6.0124, -2},
                                               .velocity = {-0.4847, 0},
                                               .acceleration = {7.4861, 0},
                                               .time = Time(4.0334)},
                                              kTol),
                                   ResultNear({.position = {-6.0147, -2},
                                               .velocity = {-0.4156, 0},
                                               .acceleration = {12.4229, 0},
                                               .time = Time(4.0389)},
                                              kTol),
                                   ResultNear({.position = {-6.0165, -2},
                                               .velocity = {-0.3364, 0},
                                               .acceleration = {14.2557, 0},
                                               .time = Time(4.0445)},
                                              kTol),
                                   ResultNear({.position = {-6.0180, -2},
                                               .velocity = {-0.2583, 0},
                                               .acceleration = {14.0591, 0},
                                               .time = Time(4.0500)},
                                              kTol),
                                   ResultNear({.position = {-6.0190, -2},
                                               .velocity = {-0.1880, 0},
                                               .acceleration = {12.6630, 0},
                                               .time = Time(4.0556)},
                                              kTol)));

  // Restoring should not have cleared the save, so repeat one more time.
  modeler.Restore();
  results.clear();
  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {-6.02, -2},
                           .time = Time{4.0167}},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear({.position = {-6.0003, -2},
                                               .velocity = {-0.0615, 0},
                                               .acceleration = {-14.7276, 0},
                                               .time = Time(4.0042)},
                                              kTol),
                                   ResultNear({.position = {-6.0009, -2},
                                               .velocity = {-0.1628, 0},
                                               .acceleration = {-24.2725, 0},
                                               .time = Time(4.0084)},
                                              kTol),
                                   ResultNear({.position = {-6.0021, -2},
                                               .velocity = {-0.2868, 0},
                                               .acceleration = {-29.6996, 0},
                                               .time = Time(4.0125)},
                                              kTol),
                                   ResultNear({.position = {-6.0039, -2},
                                               .velocity = {-0.4203, 0},
                                               .acceleration = {-31.9728, 0},
                                               .time = Time(4.0167)},
                                              kTol),
                                   ResultNear({.position = {-6.0068, -2},
                                               .velocity = {-0.5158, 0},
                                               .acceleration = {-17.1932, 0},
                                               .time = Time(4.0223)},
                                              kTol),
                                   ResultNear({.position = {-6.0097, -2},
                                               .velocity = {-0.5262, 0},
                                               .acceleration = {-1.8749, 0},
                                               .time = Time(4.0278)},
                                              kTol),
                                   ResultNear({.position = {-6.0124, -2},
                                               .velocity = {-0.4847, 0},
                                               .acceleration = {7.4861, 0},
                                               .time = Time(4.0334)},
                                              kTol),
                                   ResultNear({.position = {-6.0147, -2},
                                               .velocity = {-0.4156, 0},
                                               .acceleration = {12.4229, 0},
                                               .time = Time(4.0389)},
                                              kTol),
                                   ResultNear({.position = {-6.0165, -2},
                                               .velocity = {-0.3364, 0},
                                               .acceleration = {14.2557, 0},
                                               .time = Time(4.0445)},
                                              kTol),
                                   ResultNear({.position = {-6.0180, -2},
                                               .velocity = {-0.2583, 0},
                                               .acceleration = {14.0591, 0},
                                               .time = Time(4.0500)},
                                              kTol),
                                   ResultNear({.position = {-6.0190, -2},
                                               .velocity = {-0.1880, 0},
                                               .acceleration = {12.6630, 0},
                                               .time = Time(4.0556)},
                                              kTol)));

  // Calling Reset() should clear the save point so calling Restore() again
  // should have no effect.
  EXPECT_TRUE(modeler.Reset().ok());
  results.clear();
  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kDown,
                           .position = {-6, -2},
                           .time = Time(4)},
                          results)
                  .ok());
  EXPECT_THAT(results, ElementsAre(ResultNear(
                           {.position = {-6, -2}, .time = Time(4)}, kTol)));
  results.clear();
  EXPECT_TRUE(modeler
                  .Update({.event_type = Input::EventType::kUp,
                           .position = {-6.02, -2},
                           .time = Time(4.0167)},
                          results)
                  .ok());

  modeler.Restore();

  // Restore should have no effect so we cannot finish the line again.
  results.clear();
  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {-6.02, -2},
                         .time = Time(4.0167)},
                        results)
                .code(),
            absl::StatusCode::kFailedPrecondition);
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
