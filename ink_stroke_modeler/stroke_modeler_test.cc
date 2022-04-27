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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ink_stroke_modeler/internal/type_matchers.h"
#include "ink_stroke_modeler/params.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::DoubleNear;
using ::testing::ElementsAre;
using ::testing::FloatNear;
using ::testing::IsEmpty;
using ::testing::Matches;
using ::testing::Not;

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

MATCHER_P2(ResultNearMatcher, expected, tolerance, "") {
  if (Matches(Vec2Near(expected.position, tolerance))(arg.position) &&
      Matches(Vec2Near(expected.velocity, tolerance))(arg.velocity) &&
      Matches(DoubleNear(expected.time.Value(), tolerance))(arg.time.Value()) &&
      Matches(FloatNear(expected.pressure, tolerance))(arg.pressure) &&
      Matches(FloatNear(expected.tilt, tolerance))(arg.tilt) &&
      Matches(FloatNear(expected.orientation, tolerance))(arg.orientation)) {
    return true;
  }

  return false;
}

::testing::Matcher<Result> ResultNear(const Result &expected, float tolerance) {
  return ResultNearMatcher(expected, tolerance);
}

TEST(StrokeModelerTest, NoPredictionUponInit) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());
  EXPECT_EQ(modeler.Predict().status().code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, InputRateSlowerThanMinOutputRate) {
  const Duration kDeltaTime{1. / 30};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {3, 4},
                      .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(ResultNear(
          {.position = {3, 4}, .velocity = {0, 0}, .time = Time(0)}, kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, IsEmpty());

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {3.2, 4.2},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {3.0019, 4.0019},
                                                .velocity = {0.4007, 0.4007},
                                                .time = Time(0.0048)},
                                               kTol),
                                    ResultNear({.position = {3.0069, 4.0069},
                                                .velocity = {1.0381, 1.0381},
                                                .time = Time(0.0095)},
                                               kTol),
                                    ResultNear({.position = {3.0154, 4.0154},
                                                .velocity = {1.7883, 1.7883},
                                                .time = Time(0.0143)},
                                               kTol),
                                    ResultNear({.position = {3.0276, 4.0276},
                                                .velocity = {2.5626, 2.5626},
                                                .time = Time(0.0190)},
                                               kTol),
                                    ResultNear({.position = {3.0433, 4.0433},
                                                .velocity = {3.3010, 3.3010},
                                                .time = Time(0.0238)},
                                               kTol),
                                    ResultNear({.position = {3.0622, 4.0622},
                                                .velocity = {3.9665, 3.9665},
                                                .time = Time(0.0286)},
                                               kTol),
                                    ResultNear({.position = {3.0838, 4.0838},
                                                .velocity = {4.5397, 4.5397},
                                                .time = Time(0.0333)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {3.1095, 4.1095},
                                                .velocity = {4.6253, 4.6253},
                                                .time = Time(0.0389)},
                                               kTol),
                                    ResultNear({.position = {3.1331, 4.1331},
                                                .velocity = {4.2563, 4.2563},
                                                .time = Time(0.0444)},
                                               kTol),
                                    ResultNear({.position = {3.1534, 4.1534},
                                                .velocity = {3.6479, 3.6479},
                                                .time = Time(0.0500)},
                                               kTol),
                                    ResultNear({.position = {3.1698, 4.1698},
                                                .velocity = {2.9512, 2.9512},
                                                .time = Time(0.0556)},
                                               kTol),
                                    ResultNear({.position = {3.1824, 4.1824},
                                                .velocity = {2.2649, 2.2649},
                                                .time = Time(0.0611)},
                                               kTol),
                                    ResultNear({.position = {3.1915, 4.1915},
                                                .velocity = {1.6473, 1.6473},
                                                .time = Time(0.0667)},
                                               kTol),
                                    ResultNear({.position = {3.1978, 4.1978},
                                                .velocity = {1.1269, 1.1269},
                                                .time = Time(0.0722)},
                                               kTol),
                                    ResultNear({.position = {3.1992, 4.1992},
                                                .velocity = {1.0232, 1.0232},
                                                .time = Time(0.0736)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {3.5, 4.2},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {3.1086, 4.1058},
                                                .velocity = {5.2142, 4.6131},
                                                .time = Time(0.0381)},
                                               kTol),
                                    ResultNear({.position = {3.1368, 4.1265},
                                                .velocity = {5.9103, 4.3532},
                                                .time = Time(0.0429)},
                                               kTol),
                                    ResultNear({.position = {3.1681, 4.1450},
                                                .velocity = {6.5742, 3.8917},
                                                .time = Time(0.0476)},
                                               kTol),
                                    ResultNear({.position = {3.2022, 4.1609},
                                                .velocity = {7.1724, 3.3285},
                                                .time = Time(0.0524)},
                                               kTol),
                                    ResultNear({.position = {3.2388, 4.1739},
                                                .velocity = {7.6876, 2.7361},
                                                .time = Time(0.0571)},
                                               kTol),
                                    ResultNear({.position = {3.2775, 4.1842},
                                                .velocity = {8.1138, 2.1640},
                                                .time = Time(0.0619)},
                                               kTol),
                                    ResultNear({.position = {3.3177, 4.1920},
                                                .velocity = {8.4531, 1.6436},
                                                .time = Time(0.0667)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {3.3625, 4.1982},
                                                .velocity = {8.0545, 1.1165},
                                                .time = Time(0.0722)},
                                               kTol),
                                    ResultNear({.position = {3.4018, 4.2021},
                                                .velocity = {7.0831, 0.6987},
                                                .time = Time(0.0778)},
                                               kTol),
                                    ResultNear({.position = {3.4344, 4.2043},
                                                .velocity = {5.8564, 0.3846},
                                                .time = Time(0.0833)},
                                               kTol),
                                    ResultNear({.position = {3.4598, 4.2052},
                                                .velocity = {4.5880, 0.1611},
                                                .time = Time(0.0889)},
                                               kTol),
                                    ResultNear({.position = {3.4788, 4.2052},
                                                .velocity = {3.4098, 0.0124},
                                                .time = Time(0.0944)},
                                               kTol),
                                    ResultNear({.position = {3.4921, 4.2048},
                                                .velocity = {2.3929, -0.0780},
                                                .time = Time(0.1000)},
                                               kTol),
                                    ResultNear({.position = {3.4976, 4.2045},
                                                .velocity = {1.9791, -0.1015},
                                                .time = Time(0.1028)},
                                               kTol),
                                    ResultNear({.position = {3.5001, 4.2044},
                                                .velocity = {1.7911, -0.1098},
                                                .time = Time(0.1042)},
                                               kTol)));

  time += kDeltaTime;
  // We get more results at the end of the stroke as it tries to "catch up" to
  // the raw input.
  results = modeler.Update({.event_type = Input::EventType::kUp,
                            .position = {3.7, 4.4},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {3.3583, 4.1996},
                                                .velocity = {8.5122, 1.5925},
                                                .time = Time(0.0714)},
                                               kTol),
                                    ResultNear({.position = {3.3982, 4.2084},
                                                .velocity = {8.3832, 1.8534},
                                                .time = Time(0.0762)},
                                               kTol),
                                    ResultNear({.position = {3.4369, 4.2194},
                                                .velocity = {8.1393, 2.3017},
                                                .time = Time(0.0810)},
                                               kTol),
                                    ResultNear({.position = {3.4743, 4.2329},
                                                .velocity = {7.8362, 2.8434},
                                                .time = Time(0.0857)},
                                               kTol),
                                    ResultNear({.position = {3.5100, 4.2492},
                                                .velocity = {7.5143, 3.4101},
                                                .time = Time(0.0905)},
                                               kTol),
                                    ResultNear({.position = {3.5443, 4.2680},
                                                .velocity = {7.2016, 3.9556},
                                                .time = Time(0.0952)},
                                               kTol),
                                    ResultNear({.position = {3.5773, 4.2892},
                                                .velocity = {6.9159, 4.4505},
                                                .time = Time(0.1000)},
                                               kTol),
                                    ResultNear({.position = {3.6115, 4.3141},
                                                .velocity = {6.1580, 4.4832},
                                                .time = Time(0.1056)},
                                               kTol),
                                    ResultNear({.position = {3.6400, 4.3369},
                                                .velocity = {5.1434, 4.0953},
                                                .time = Time(0.1111)},
                                               kTol),
                                    ResultNear({.position = {3.6626, 4.3563},
                                                .velocity = {4.0671, 3.4902},
                                                .time = Time(0.1167)},
                                               kTol),
                                    ResultNear({.position = {3.6796, 4.3719},
                                                .velocity = {3.0515, 2.8099},
                                                .time = Time(0.1222)},
                                               kTol),
                                    ResultNear({.position = {3.6916, 4.3838},
                                                .velocity = {2.1648, 2.1462},
                                                .time = Time(0.1278)},
                                               kTol),
                                    ResultNear({.position = {3.6996, 4.3924},
                                                .velocity = {1.4360, 1.5529},
                                                .time = Time(0.1333)},
                                               kTol),
                                    ResultNear({.position = {3.7028, 4.3960},
                                                .velocity = {1.1520, 1.3044},
                                                .time = Time(0.1361)},
                                               kTol)));

  // The stroke is finished, so there's nothing to predict anymore.
  EXPECT_EQ(modeler.Predict().status().code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, InputRateFasterThanMinOutputRate) {
  const Duration kDeltaTime{1. / 300};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{2};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {5, -3},
                      .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(ResultNear(
          {.position = {5, -3}, .velocity = {0, 0}, .time = Time(2)}, kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, IsEmpty());

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {5, -3.1},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {5, -3.0033},
                                                .velocity = {0, -0.9818},
                                                .time = Time(2.0033)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {5, -3.0153},
                                                .velocity = {0, -2.1719},
                                                .time = Time(2.0089)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0303},
                                                .velocity = {0, -2.6885},
                                                .time = Time(2.0144)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0456},
                                                .velocity = {0, -2.7541},
                                                .time = Time(2.0200)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0597},
                                                .velocity = {0, -2.5430},
                                                .time = Time(2.0256)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0718},
                                                .velocity = {0, -2.1852},
                                                .time = Time(2.0311)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0817},
                                                .velocity = {0, -1.7719},
                                                .time = Time(2.0367)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0893},
                                                .velocity = {0, -1.3628},
                                                .time = Time(2.0422)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0948},
                                                .velocity = {0, -0.9934},
                                                .time = Time(2.0478)},
                                               kTol),
                                    ResultNear({.position = {5, -3.0986},
                                                .velocity = {0, -0.6815},
                                                .time = Time(2.0533)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.975, -3.175},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9992, -3.0114},
                                                .velocity = {-0.2455, -2.4322},
                                                .time = Time(2.0067)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9962, -3.0344},
                                                .velocity = {-0.5430, -4.1368},
                                                .time = Time(2.0122)},
                                               kTol),
                                    ResultNear({.position = {4.9924, -3.0609},
                                                .velocity = {-0.6721, -4.7834},
                                                .time = Time(2.0178)},
                                               kTol),
                                    ResultNear({.position = {4.9886, -3.0873},
                                                .velocity = {-0.6885, -4.7365},
                                                .time = Time(2.0233)},
                                               kTol),
                                    ResultNear({.position = {4.9851, -3.1110},
                                                .velocity = {-0.6358, -4.2778},
                                                .time = Time(2.0289)},
                                               kTol),
                                    ResultNear({.position = {4.9820, -3.1311},
                                                .velocity = {-0.5463, -3.6137},
                                                .time = Time(2.0344)},
                                               kTol),
                                    ResultNear({.position = {4.9796, -3.1471},
                                                .velocity = {-0.4430, -2.8867},
                                                .time = Time(2.0400)},
                                               kTol),
                                    ResultNear({.position = {4.9777, -3.1593},
                                                .velocity = {-0.3407, -2.1881},
                                                .time = Time(2.0456)},
                                               kTol),
                                    ResultNear({.position = {4.9763, -3.1680},
                                                .velocity = {-0.2484, -1.5700},
                                                .time = Time(2.0511)},
                                               kTol),
                                    ResultNear({.position = {4.9754, -3.1739},
                                                .velocity = {-0.1704, -1.0564},
                                                .time = Time(2.0567)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.9, -3.2},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9953, -3.0237},
                                                .velocity = {-1.1603, -3.7004},
                                                .time = Time(2.0100)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9828, -3.0521},
                                                .velocity = {-2.2559, -5.1049},
                                                .time = Time(2.0156)},
                                               kTol),
                                    ResultNear({.position = {4.9677, -3.0825},
                                                .velocity = {-2.7081, -5.4835},
                                                .time = Time(2.0211)},
                                               kTol),
                                    ResultNear({.position = {4.9526, -3.1115},
                                                .velocity = {-2.7333, -5.2122},
                                                .time = Time(2.0267)},
                                               kTol),
                                    ResultNear({.position = {4.9387, -3.1369},
                                                .velocity = {-2.4999, -4.5756},
                                                .time = Time(2.0322)},
                                               kTol),
                                    ResultNear({.position = {4.9268, -3.1579},
                                                .velocity = {-2.1326, -3.7776},
                                                .time = Time(2.0378)},
                                               kTol),
                                    ResultNear({.position = {4.9173, -3.1743},
                                                .velocity = {-1.7184, -2.9554},
                                                .time = Time(2.0433)},
                                               kTol),
                                    ResultNear({.position = {4.9100, -3.1865},
                                                .velocity = {-1.3136, -2.1935},
                                                .time = Time(2.0489)},
                                               kTol),
                                    ResultNear({.position = {4.9047, -3.1950},
                                                .velocity = {-0.9513, -1.5369},
                                                .time = Time(2.0544)},
                                               kTol),
                                    ResultNear({.position = {4.9011, -3.2006},
                                                .velocity = {-0.6475, -1.0032},
                                                .time = Time(2.0600)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.825, -3.2},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9868, -3.0389},
                                                .velocity = {-2.5540, -4.5431},
                                                .time = Time(2.0133)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9636, -3.0687},
                                                .velocity = {-4.1801, -5.3627},
                                                .time = Time(2.0189)},
                                               kTol),
                                    ResultNear({.position = {4.9370, -3.0985},
                                                .velocity = {-4.7757, -5.3670},
                                                .time = Time(2.0244)},
                                               kTol),
                                    ResultNear({.position = {4.9109, -3.1256},
                                                .velocity = {-4.6989, -4.8816},
                                                .time = Time(2.0300)},
                                               kTol),
                                    ResultNear({.position = {4.8875, -3.1486},
                                                .velocity = {-4.2257, -4.1466},
                                                .time = Time(2.0356)},
                                               kTol),
                                    ResultNear({.position = {4.8677, -3.1671},
                                                .velocity = {-3.5576, -3.3287},
                                                .time = Time(2.0411)},
                                               kTol),
                                    ResultNear({.position = {4.8520, -3.1812},
                                                .velocity = {-2.8333, -2.5353},
                                                .time = Time(2.0467)},
                                               kTol),
                                    ResultNear({.position = {4.8401, -3.1914},
                                                .velocity = {-2.1411, -1.8288},
                                                .time = Time(2.0522)},
                                               kTol),
                                    ResultNear({.position = {4.8316, -3.1982},
                                                .velocity = {-1.5312, -1.2386},
                                                .time = Time(2.0578)},
                                               kTol),
                                    ResultNear({.position = {4.8280, -3.2010},
                                                .velocity = {-1.2786, -1.0053},
                                                .time = Time(2.0606)},
                                               kTol),
                                    ResultNear({.position = {4.8272, -3.2017},
                                                .velocity = {-1.2209, -0.9529},
                                                .time = Time(2.0613)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.75, -3.225},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9726, -3.0565},
                                                .velocity = {-4.2660, -5.2803},
                                                .time = Time(2.0167)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9381, -3.0894},
                                                .velocity = {-6.2018, -5.9261},
                                                .time = Time(2.0222)},
                                               kTol),
                                    ResultNear({.position = {4.9004, -3.1215},
                                                .velocity = {-6.7995, -5.7749},
                                                .time = Time(2.0278)},
                                               kTol),
                                    ResultNear({.position = {4.8640, -3.1501},
                                                .velocity = {-6.5400, -5.1591},
                                                .time = Time(2.0333)},
                                               kTol),
                                    ResultNear({.position = {4.8319, -3.1741},
                                                .velocity = {-5.7897, -4.3207},
                                                .time = Time(2.0389)},
                                               kTol),
                                    ResultNear({.position = {4.8051, -3.1932},
                                                .velocity = {-4.8133, -3.4248},
                                                .time = Time(2.0444)},
                                               kTol),
                                    ResultNear({.position = {4.7841, -3.2075},
                                                .velocity = {-3.7898, -2.5759},
                                                .time = Time(2.0500)},
                                               kTol),
                                    ResultNear({.position = {4.7683, -3.2176},
                                                .velocity = {-2.8312, -1.8324},
                                                .time = Time(2.0556)},
                                               kTol),
                                    ResultNear({.position = {4.7572, -3.2244},
                                                .velocity = {-1.9986, -1.2198},
                                                .time = Time(2.0611)},
                                               kTol),
                                    ResultNear({.position = {4.7526, -3.2271},
                                                .velocity = {-1.6580, -0.9805},
                                                .time = Time(2.0639)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.7, -3.3},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9529, -3.0778},
                                                .velocity = {-5.9184, -6.4042},
                                                .time = Time(2.0200)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9101, -3.1194},
                                                .velocity = {-7.6886, -7.4784},
                                                .time = Time(2.0256)},
                                               kTol),
                                    ResultNear({.position = {4.8654, -3.1607},
                                                .velocity = {-8.0518, -7.4431},
                                                .time = Time(2.0311)},
                                               kTol),
                                    ResultNear({.position = {4.8235, -3.1982},
                                                .velocity = {-7.5377, -6.7452},
                                                .time = Time(2.0367)},
                                               kTol),
                                    ResultNear({.position = {4.7872, -3.2299},
                                                .velocity = {-6.5440, -5.7133},
                                                .time = Time(2.0422)},
                                               kTol),
                                    ResultNear({.position = {4.7574, -3.2553},
                                                .velocity = {-5.3529, -4.5748},
                                                .time = Time(2.0478)},
                                               kTol),
                                    ResultNear({.position = {4.7344, -3.2746},
                                                .velocity = {-4.1516, -3.4758},
                                                .time = Time(2.0533)},
                                               kTol),
                                    ResultNear({.position = {4.7174, -3.2885},
                                                .velocity = {-3.0534, -2.5004},
                                                .time = Time(2.0589)},
                                               kTol),
                                    ResultNear({.position = {4.7056, -3.2979},
                                                .velocity = {-2.1169, -1.6879},
                                                .time = Time(2.0644)},
                                               kTol),
                                    ResultNear({.position = {4.7030, -3.3000},
                                                .velocity = {-1.9283, -1.5276},
                                                .time = Time(2.0658)},
                                               kTol),
                                    ResultNear({.position = {4.7017, -3.3010},
                                                .velocity = {-1.8380, -1.4512},
                                                .time = Time(2.0665)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.675, -3.4},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9288, -3.1046},
                                                .velocity = {-7.2260, -8.0305},
                                                .time = Time(2.0233)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.8816, -3.1582},
                                                .velocity = {-8.4881, -9.6525},
                                                .time = Time(2.0289)},
                                               kTol),
                                    ResultNear({.position = {4.8345, -3.2124},
                                                .velocity = {-8.4738, -9.7482},
                                                .time = Time(2.0344)},
                                               kTol),
                                    ResultNear({.position = {4.7918, -3.2619},
                                                .velocity = {-7.6948, -8.9195},
                                                .time = Time(2.0400)},
                                               kTol),
                                    ResultNear({.position = {4.7555, -3.3042},
                                                .velocity = {-6.5279, -7.6113},
                                                .time = Time(2.0456)},
                                               kTol),
                                    ResultNear({.position = {4.7264, -3.3383},
                                                .velocity = {-5.2343, -6.1345},
                                                .time = Time(2.0511)},
                                               kTol),
                                    ResultNear({.position = {4.7043, -3.3643},
                                                .velocity = {-3.9823, -4.6907},
                                                .time = Time(2.0567)},
                                               kTol),
                                    ResultNear({.position = {4.6884, -3.3832},
                                                .velocity = {-2.8691, -3.3980},
                                                .time = Time(2.0622)},
                                               kTol),
                                    ResultNear({.position = {4.6776, -3.3961},
                                                .velocity = {-1.9403, -2.3135},
                                                .time = Time(2.0678)},
                                               kTol),
                                    ResultNear({.position = {4.6752, -3.3990},
                                                .velocity = {-1.7569, -2.0983},
                                                .time = Time(2.0692)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {4.675, -3.525},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.9022, -3.1387},
                                                .velocity = {-7.9833, -10.2310},
                                                .time = Time(2.0267)},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.8549, -3.2079},
                                                .velocity = {-8.5070, -12.4602},
                                                .time = Time(2.0322)},
                                               kTol),
                                    ResultNear({.position = {4.8102, -3.2783},
                                                .velocity = {-8.0479, -12.6650},
                                                .time = Time(2.0378)},
                                               kTol),
                                    ResultNear({.position = {4.7711, -3.3429},
                                                .velocity = {-7.0408, -11.6365},
                                                .time = Time(2.0433)},
                                               kTol),
                                    ResultNear({.position = {4.7389, -3.3983},
                                                .velocity = {-5.7965, -9.9616},
                                                .time = Time(2.0489)},
                                               kTol),
                                    ResultNear({.position = {4.7137, -3.4430},
                                                .velocity = {-4.5230, -8.0510},
                                                .time = Time(2.0544)},
                                               kTol),
                                    ResultNear({.position = {4.6951, -3.4773},
                                                .velocity = {-3.3477, -6.1727},
                                                .time = Time(2.0600)},
                                               kTol),
                                    ResultNear({.position = {4.6821, -3.5022},
                                                .velocity = {-2.3381, -4.4846},
                                                .time = Time(2.0656)},
                                               kTol),
                                    ResultNear({.position = {4.6737, -3.5192},
                                                .velocity = {-1.5199, -3.0641},
                                                .time = Time(2.0711)},
                                               kTol),
                                    ResultNear({.position = {4.6718, -3.5231},
                                                .velocity = {-1.3626, -2.7813},
                                                .time = Time(2.0725)},
                                               kTol)));

  time += kDeltaTime;
  // We get more results at the end of the stroke as it tries to "catch up" to
  // the raw input.
  results = modeler.Update({.event_type = Input::EventType::kUp,
                            .position = {4.7, -3.6},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {4.8753, -3.1797},
                                                .velocity = {-8.0521, -12.3049},
                                                .time = Time(2.0300)},
                                               kTol),
                                    ResultNear({.position = {4.8325, -3.2589},
                                                .velocity = {-7.7000, -14.2607},
                                                .time = Time(2.0356)},
                                               kTol),
                                    ResultNear({.position = {4.7948, -3.3375},
                                                .velocity = {-6.7888, -14.1377},
                                                .time = Time(2.0411)},
                                               kTol),
                                    ResultNear({.position = {4.7636, -3.4085},
                                                .velocity = {-5.6249, -12.7787},
                                                .time = Time(2.0467)},
                                               kTol),
                                    ResultNear({.position = {4.7390, -3.4685},
                                                .velocity = {-4.4152, -10.8015},
                                                .time = Time(2.0522)},
                                               kTol),
                                    ResultNear({.position = {4.7208, -3.5164},
                                                .velocity = {-3.2880, -8.6333},
                                                .time = Time(2.0578)},
                                               kTol),
                                    ResultNear({.position = {4.7079, -3.5528},
                                                .velocity = {-2.3128, -6.5475},
                                                .time = Time(2.0633)},
                                               kTol),
                                    ResultNear({.position = {4.6995, -3.5789},
                                                .velocity = {-1.5174, -4.7008},
                                                .time = Time(2.0689)},
                                               kTol),
                                    ResultNear({.position = {4.6945, -3.5965},
                                                .velocity = {-0.9022, -3.1655},
                                                .time = Time(2.0744)},
                                               kTol),
                                    ResultNear({.position = {4.6942, -3.5976},
                                                .velocity = {-0.8740, -3.0899},
                                                .time = Time(2.0748)},
                                               kTol)));

  // The stroke is finished, so there's nothing to predict anymore.
  EXPECT_EQ(modeler.Predict().status().code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, WobbleSmoothed) {
  const Duration kDeltaTime{.0167};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{4};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {-6, -2},
                      .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(ResultNear(
          {.position = {-6, -2}, .velocity = {0, 0}, .time = Time(4)}, kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-6.02, -2},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {-6.0001, -2},
                                                .velocity = {-0.0328, 0},
                                                .time = Time(4.0042)},
                                               kTol),
                                    ResultNear({.position = {-6.0005, -2},
                                                .velocity = {-0.0869, 0},
                                                .time = Time(4.0084)},
                                               kTol),
                                    ResultNear({.position = {-6.0011, -2},
                                                .velocity = {-0.1531, 0},
                                                .time = Time(4.0125)},
                                               kTol),
                                    ResultNear({.position = {-6.0021, -2},
                                                .velocity = {-0.2244, 0},
                                                .time = Time(4.0167)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-6.02, -2.02},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {-6.0032, -2.0001},
                                                .velocity = {-0.2709, -0.0205},
                                                .time = Time(4.0209)},
                                               kTol),
                                    ResultNear({.position = {-6.0044, -2.0003},
                                                .velocity = {-0.2977, -0.0543},
                                                .time = Time(4.0251)},
                                               kTol),
                                    ResultNear({.position = {-6.0057, -2.0007},
                                                .velocity = {-0.3093, -0.0956},
                                                .time = Time(4.0292)},
                                               kTol),
                                    ResultNear({.position = {-6.0070, -2.0013},
                                                .velocity = {-0.3097, -0.1401},
                                                .time = Time(4.0334)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-6.04, -2.02},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {-6.0084, -2.0021},
                                                .velocity = {-0.3350, -0.1845},
                                                .time = Time(4.0376)},
                                               kTol),
                                    ResultNear({.position = {-6.0100, -2.0030},
                                                .velocity = {-0.3766, -0.2266},
                                                .time = Time(4.0418)},
                                               kTol),
                                    ResultNear({.position = {-6.0118, -2.0041},
                                                .velocity = {-0.4273, -0.2649},
                                                .time = Time(4.0459)},
                                               kTol),
                                    ResultNear({.position = {-6.0138, -2.0054},
                                                .velocity = {-0.4818, -0.2986},
                                                .time = Time(4.0501)},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-6.04, -2.04},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({.position = {-6.0160, -2.0068},
                                                .velocity = {-0.5157, -0.3478},
                                                .time = Time(4.0543)},
                                               kTol),
                                    ResultNear({.position = {-6.0182, -2.0085},
                                                .velocity = {-0.5334, -0.4054},
                                                .time = Time(4.0585)},
                                               kTol),
                                    ResultNear({.position = {-6.0204, -2.0105},
                                                .velocity = {-0.5389, -0.4658},
                                                .time = Time(4.0626)},
                                               kTol),
                                    ResultNear({.position = {-6.0227, -2.0126},
                                                .velocity = {-0.5356, -0.5251},
                                                .time = Time(4.0668)},
                                               kTol)));
}

TEST(StrokeModelerTest, Reset) {
  const Duration kDeltaTime{1. / 50};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {-8, -10},
                      .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, IsEmpty());

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-10, -8},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {-11, -5},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());
  EXPECT_EQ(modeler.Predict().status().code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, IgnoreInputsBeforeTDown) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {0, 0},
                         .time = Time(0)})
                .status()
                .code(),
            absl::StatusCode::kFailedPrecondition);

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {0, 0},
                         .time = Time(1)})
                .status()
                .code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, IgnoreTDownWhileStrokeIsInProgress) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = Time(0)});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {1, 1},
                         .time = Time(1)})
                .status()
                .code(),
            absl::StatusCode::kFailedPrecondition);

  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {1, 1},
                            .time = Time(1)});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {2, 2},
                         .time = Time(2)})
                .status()
                .code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, AlternateParams) {
  const Duration kDeltaTime{1. / 50};

  StrokeModelParams stroke_model_params = kDefaultParams;
  stroke_model_params.sampling_params.min_output_rate = 70;

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(stroke_model_params).ok());

  Time time{3};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = time,
                      .pressure = .5,
                      .tilt = .2,
                      .orientation = .4});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear(
                            {{0, 0}, {0, 0}, Time{3}, .5, .2, .4}, kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, IsEmpty());

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {0, .5},
                            .time = time,
                            .pressure = .4,
                            .tilt = .3,
                            .orientation = .3});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(
          ResultNear(
              {{0, 0.0736}, {0, 7.3636}, Time{3.0100}, 0.4853, 0.2147, 0.3853},
              kTol),
          ResultNear(
              {{0, 0.2198}, {0, 14.6202}, Time{3.0200}, 0.4560, 0.2440, 0.3560},
              kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(
          ResultNear(
              {{0, 0.3823}, {0, 11.3709}, Time{3.0343}, 0.4235, 0.2765, 0.3235},
              kTol),
          ResultNear(
              {{0, 0.4484}, {0, 4.6285}, Time{3.0486}, 0.4103, 0.2897, 0.3103},
              kTol),
          ResultNear(
              {{0, 0.4775}, {0, 2.0389}, Time{3.0629}, 0.4045, 0.2955, 0.3045},
              kTol),
          ResultNear(
              {{0, 0.4902}, {0, 0.8873}, Time{3.0771}, 0.4020, 0.2980, 0.3020},
              kTol),
          ResultNear(
              {{0, 0.4957}, {0, 0.3868}, Time{3.0914}, 0.4009, 0.2991, 0.3009},
              kTol),
          ResultNear(
              {{0, 0.4981}, {0, 0.1686}, Time{3.1057}, 0.4004, 0.2996, 0.3004},
              kTol),
          ResultNear(
              {{0, 0.4992}, {0, 0.0735}, Time{3.1200}, 0.4002, 0.2998, 0.3002},
              kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {.2, 1},
                            .time = time,
                            .pressure = .3,
                            .tilt = .4,
                            .orientation = .2});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({{0.0295, 0.4169},
                                                {2.9455, 19.7093},
                                                Time{3.0300},
                                                0.4166,
                                                0.2834,
                                                0.3166},
                                               kTol),
                                    ResultNear({{0.0879, 0.6439},
                                                {5.8481, 22.6926},
                                                Time{3.0400},
                                                0.3691,
                                                0.3309,
                                                0.2691},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({{0.1529, 0.8487},
                                                {4.5484, 14.3374},
                                                Time{3.0543},
                                                0.3293,
                                                0.3707,
                                                0.2293},
                                               kTol),
                                    ResultNear({{0.1794, 0.9338},
                                                {1.8514, 5.9577},
                                                Time{3.0686},
                                                0.3128,
                                                0.3872,
                                                0.2128},
                                               kTol),
                                    ResultNear({{0.1910, 0.9712},
                                                {0.8156, 2.6159},
                                                Time{3.0829},
                                                0.3056,
                                                0.3944,
                                                0.2056},
                                               kTol),
                                    ResultNear({{0.1961, 0.9874},
                                                {0.3549, 1.1389},
                                                Time{3.0971},
                                                0.3024,
                                                0.3976,
                                                0.2024},
                                               kTol),
                                    ResultNear({{0.1983, 0.9945},
                                                {0.1547, 0.4965},
                                                Time{3.1114},
                                                0.3011,
                                                0.3989,
                                                0.2011},
                                               kTol),
                                    ResultNear({{0.1993, 0.9976},
                                                {0.0674, 0.2164},
                                                Time{3.1257},
                                                0.3005,
                                                0.3995,
                                                0.2005},
                                               kTol),
                                    ResultNear({{0.1997, 0.9990},
                                                {0.0294, 0.0943},
                                                Time{3.1400},
                                                0.3002,
                                                0.3998,
                                                0.2002},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {.4, 1.4},
                            .time = time,
                            .pressure = .2,
                            .tilt = .7,
                            .orientation = 0});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({{0.1668, 0.8712},
                                                {7.8837, 22.7349},
                                                Time{3.0500},
                                                0.3245,
                                                0.3755,
                                                0.2245},
                                               kTol),
                                    ResultNear({{0.2575, 1.0906},
                                                {9.0771, 21.9411},
                                                Time{3.0600},
                                                0.2761,
                                                0.4716,
                                                0.1522},
                                               kTol)));

  results = modeler.Predict();
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({{0.3395, 1.2676},
                                                {5.7349, 12.3913},
                                                Time{3.0743},
                                                0.2325,
                                                0.6024,
                                                0.0651},
                                               kTol),
                                    ResultNear({{0.3735, 1.3421},
                                                {2.3831, 5.2156},
                                                Time{3.0886},
                                                0.2142,
                                                0.6573,
                                                0.0284},
                                               kTol),
                                    ResultNear({{0.3885, 1.3748},
                                                {1.0463, 2.2854},
                                                Time{3.1029},
                                                0.2062,
                                                0.6814,
                                                0.0124},
                                               kTol),
                                    ResultNear({{0.3950, 1.3890},
                                                {0.4556, 0.9954},
                                                Time{3.1171},
                                                0.2027,
                                                0.6919,
                                                0.0054},
                                               kTol),
                                    ResultNear({{0.3978, 1.3952},
                                                {0.1986, 0.4339},
                                                Time{3.1314},
                                                0.2012,
                                                0.6965,
                                                0.0024},
                                               kTol),
                                    ResultNear({{0.3990, 1.3979},
                                                {0.0866, 0.1891},
                                                Time{3.1457},
                                                0.2005,
                                                0.6985,
                                                0.0010},
                                               kTol),
                                    ResultNear({{0.3996, 1.3991},
                                                {0.0377, 0.0824},
                                                Time{3.1600},
                                                0.2002,
                                                0.6993,
                                                0.0004},
                                               kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kUp,
                            .position = {.7, 1.7},
                            .time = time,
                            .pressure = .1,
                            .tilt = 1,
                            .orientation = 0});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, ElementsAre(ResultNear({{0.3691, 1.2874},
                                                {11.1558, 19.6744},
                                                Time{3.0700},
                                                0.2256,
                                                0.6231,
                                                0.0512},
                                               kTol),
                                    ResultNear({{0.4978, 1.4640},
                                                {12.8701, 17.6629},
                                                Time{3.0800},
                                                0.1730,
                                                0.7809,
                                                0},
                                               kTol),
                                    ResultNear({{0.6141, 1.5986},
                                                {8.1404, 9.4261},
                                                Time{3.0943},
                                                0.1312,
                                                0.9064,
                                                0},
                                               kTol),
                                    ResultNear({{0.6624, 1.6557},
                                                {3.3822, 3.9953},
                                                Time{3.1086},
                                                0.1136,
                                                0.9591,
                                                0},
                                               kTol),
                                    ResultNear({{0.6836, 1.6807},
                                                {1.4851, 1.7488},
                                                Time{3.1229},
                                                0.1059,
                                                0.9822,
                                                0},
                                               kTol),
                                    ResultNear({{0.6929, 1.6916},
                                                {0.6466, 0.7618},
                                                Time{3.1371},
                                                0.1026,
                                                0.9922,
                                                0},
                                               kTol),
                                    ResultNear({{0.6969, 1.6963},
                                                {0.2819, 0.3321},
                                                Time{3.1514},
                                                0.1011,
                                                0.9966,
                                                0},
                                               kTol),
                                    ResultNear({{0.6986, 1.6984},
                                                {0.1229, 0.1447},
                                                Time{3.1657},
                                                0.1005,
                                                0.9985,
                                                0},
                                               kTol),
                                    ResultNear({{0.6994, 1.6993},
                                                {0.0535, 0.0631},
                                                Time{3.1800},
                                                0.1002,
                                                0.9994,
                                                0},
                                               kTol)));

  EXPECT_EQ(modeler.Predict().status().code(),
            absl::StatusCode::kFailedPrecondition);
}

TEST(StrokeModelerTest, GenerateOutputOnTUpEvenIfNoTimeDelta) {
  const Duration kDeltaTime{1. / 500};

  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  Time time{0};
  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {5, 5},
                      .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(ResultNear(
          {.position = {5, 5}, .velocity = {0, 0}, .time = Time(0)}, kTol)));

  time += kDeltaTime;
  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {5, 5},
                            .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results,
              ElementsAre(ResultNear(
                  {.position = {5, 5}, .velocity = {0, 0}, .time = Time(0.002)},
                  kTol)));

  results = modeler.Update(
      {.event_type = Input::EventType::kUp, .position = {5, 5}, .time = time});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(
      *results,
      ElementsAre(ResultNear(
          {.position = {5, 5}, .velocity = {0, 0}, .time = Time(0.0076)},
          kTol)));
}

TEST(StrokeModelerTest, RejectInputIfNegativeTimeDelta) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = Time(0)});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {1, 1},
                         .time = Time(-.1)})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);

  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {1, 1},
                            .time = Time(1)});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {1, 1},
                         .time = Time(.9)})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(StrokeModelerTest, RejectDuplicateInput) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = Time(0),
                      .pressure = .2,
                      .tilt = .3,
                      .orientation = .4});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kDown,
                         .position = {0, 0},
                         .time = Time(0),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);

  results = modeler.Update({.event_type = Input::EventType::kMove,
                            .position = {1, 2},
                            .time = Time(1),
                            .pressure = .1,
                            .tilt = .2,
                            .orientation = .3});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {1, 2},
                         .time = Time(1),
                         .pressure = .1,
                         .tilt = .2,
                         .orientation = .3})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(StrokeModelerTest, FarApartTimesDoNotCrashForMove) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = Time(0),
                      .pressure = .2,
                      .tilt = .3,
                      .orientation = .4});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kMove,
                         .position = {0, 0},
                         .time = Time(INT_MAX),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);
}

TEST(StrokeModelerTest, FarApartTimesDoNotCrashForUp) {
  StrokeModeler modeler;
  ASSERT_TRUE(modeler.Reset(kDefaultParams).ok());

  absl::StatusOr<std::vector<Result>> results =
      modeler.Update({.event_type = Input::EventType::kDown,
                      .position = {0, 0},
                      .time = Time(0),
                      .pressure = .2,
                      .tilt = .3,
                      .orientation = .4});
  ASSERT_TRUE(results.ok());
  EXPECT_THAT(*results, Not(IsEmpty()));

  EXPECT_EQ(modeler
                .Update({.event_type = Input::EventType::kUp,
                         .position = {0, 0},
                         .time = Time(INT_MAX),
                         .pressure = .2,
                         .tilt = .3,
                         .orientation = .4})
                .status()
                .code(),
            absl::StatusCode::kInvalidArgument);
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
  ASSERT_TRUE(modeler.Update(pointer_down).ok());

  // Reset, using the same parameters.
  ASSERT_TRUE(modeler.Reset().ok());

  // Doesn't object to seeing a duplicate input or another down event, since
  // the previous stroke in progress was aborted by the call to reset.
  ASSERT_TRUE(modeler.Update(pointer_down).ok());
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
