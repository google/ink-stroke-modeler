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

#include "ink_stroke_modeler/types.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/internal/type_matchers.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::Not;

TEST(TypesTest, Vec2Equality) {
  EXPECT_EQ((Vec2{1, 2}), (Vec2{1, 2}));
  EXPECT_EQ((Vec2{-.4, 17}), (Vec2{-.4, 17}));

  EXPECT_NE((Vec2{1, 2}), (Vec2{1, 5}));
  EXPECT_NE((Vec2{3, 2}), (Vec2{.7, 2}));
  EXPECT_NE((Vec2{-4, .3}), (Vec2{.5, 12}));
}

TEST(TypesTest, Vec2EqMatcher) {
  EXPECT_THAT((Vec2{1, 2}), Vec2Eq({1, 2}));
  EXPECT_THAT((Vec2{3, 4}), Not(Vec2Eq({3, 5})));
  EXPECT_THAT((Vec2{5, 6}), Not(Vec2Eq({4, 6})));

  // Vec2Eq delegates to FloatEq, which uses a tolerance of 4 ULP.
  constexpr float kEps = std::numeric_limits<float>::epsilon();
  EXPECT_THAT((Vec2{1, 1}), Vec2Eq({1 + kEps, 1 - kEps}));
}

TEST(TypesTest, Vec2NearMatcher) {
  EXPECT_THAT((Vec2{1, 2}), Vec2Near({1.05, 1.95}, .1));
  EXPECT_THAT((Vec2{3, 4}), Not(Vec2Near({3, 5}, .5)));
  EXPECT_THAT((Vec2{5, 6}), Not(Vec2Near({4, 6}, .5)));
}

TEST(TypesTest, Vec2Magnitude) {
  EXPECT_FLOAT_EQ((Vec2{1, 1}).Magnitude(), std::sqrt(2));
  EXPECT_FLOAT_EQ((Vec2{-3, 4}).Magnitude(), 5);
  EXPECT_FLOAT_EQ((Vec2{0, 0}).Magnitude(), 0);
  EXPECT_FLOAT_EQ((Vec2{0, 17}).Magnitude(), 17);
}

TEST(TypesTest, Vec2Addition) {
  Vec2 a{3, 0};
  Vec2 b{-1, .3};
  Vec2 c{2.7, 4};

  EXPECT_THAT(a + b, Vec2Eq({2, .3}));
  EXPECT_THAT(a + c, Vec2Eq({5.7, 4}));
  EXPECT_THAT(b + c, Vec2Eq({1.7, 4.3}));
}

TEST(TypesTest, Vec2Subtraction) {
  Vec2 a{0, -2};
  Vec2 b{.5, 19};
  Vec2 c{1.1, -3.4};

  EXPECT_THAT(a - b, Vec2Eq({-.5, -21}));
  EXPECT_THAT(a - c, Vec2Eq({-1.1, 1.4}));
  EXPECT_THAT(b - c, Vec2Eq({-.6, 22.4}));
}

TEST(TypesTest, Vec2ScalarMultiplication) {
  Vec2 a{.7, -3};
  Vec2 b{3, 5};

  EXPECT_THAT(a * 2, Vec2Eq({1.4, -6}));
  EXPECT_THAT(.1 * a, Vec2Eq({.07, -.3}));
  EXPECT_THAT(b * -.3, Vec2Eq({-.9, -1.5}));
  EXPECT_THAT(4 * b, Vec2({12, 20}));
}

TEST(TypesTest, Vec2ScalarDivision) {
  Vec2 a{7, .9};
  Vec2 b{-4.5, -2};

  EXPECT_THAT(a / 2, Vec2Eq({3.5, .45}));
  EXPECT_THAT(a / -.1, Vec2Eq({-70, -9}));
  EXPECT_THAT(b / 5, Vec2Eq({-.9, -.4}));
  EXPECT_THAT(b / .2, Vec2Eq({-22.5, -10}));
}

TEST(TypesTest, Vec2AddAssign) {
  Vec2 a{1, 2};
  a += {.x = 3, .y = -1};
  EXPECT_THAT(a, Vec2Eq({4, 1}));
  a += {.x = -.5, .y = 2};
  EXPECT_THAT(a, Vec2Eq({3.5, 3}));
}

TEST(TypesTest, Vec2SubractAssign) {
  Vec2 a{1, 2};
  a -= {.x = 3, .y = -1};
  EXPECT_THAT(a, Vec2Eq({-2, 3}));
  a -= {.x = -.5, .y = 2};
  EXPECT_THAT(a, Vec2Eq({-1.5, 1}));
}

TEST(TypesTest, Vec2ScalarMultiplyAssign) {
  Vec2 a{1, 2};
  a *= 2;
  EXPECT_THAT(a, Vec2Eq({2, 4}));
  a *= -.4;
  EXPECT_THAT(a, Vec2Eq({-.8, -1.6}));
}

TEST(TypesTest, Vec2ScalarDivideAssign) {
  Vec2 a{1, 2};
  a /= 2;
  EXPECT_THAT(a, Vec2Eq({.5, 1}));
  a /= -.4;
  EXPECT_THAT(a, Vec2Eq({-1.25, -2.5}));
}

TEST(TypesTest, Vec2Stream) {
  std::stringstream s;
  s << Vec2{3.5, -2.7};
  EXPECT_EQ(s.str(), "(3.5, -2.7)");
}

TEST(TypesTest, DurationArithmetic) {
  EXPECT_EQ(Duration(1) + Duration(2), Duration(3));
  EXPECT_EQ(Duration(6) - Duration(1), Duration(5));
  EXPECT_EQ(Duration(3) * 2, Duration(6));
  EXPECT_EQ(.5 * Duration(7), Duration(3.5));
  EXPECT_EQ(Duration(12) / 4, Duration(3));
}

TEST(TypesTest, DurationArithmeticAssignment) {
  Duration d{5};
  d += Duration(2);
  EXPECT_EQ(d, Duration(7));
  d -= Duration(3);
  EXPECT_EQ(d, Duration(4));
  d *= 5;
  EXPECT_EQ(d, Duration(20));
  d /= 2;
  EXPECT_EQ(d, Duration(10));
}

TEST(TypesTest, DurationComparison) {
  EXPECT_EQ(Duration(0), Duration(0));
  EXPECT_NE(Duration(0), Duration(1));
  EXPECT_LT(Duration(1), Duration(2));
  EXPECT_LE(Duration(4), Duration(5));
  EXPECT_LE(Duration(2), Duration(2));
  EXPECT_GT(Duration(10), Duration(9));
  EXPECT_GE(Duration(7), Duration(5));
  EXPECT_GE(Duration(5), Duration(5));
}

TEST(TypesTest, TimeArithmetic) {
  EXPECT_EQ(Time(5) + Duration(1), Time(6));
  EXPECT_EQ(Duration(7) + Time(12), Time(19));
  EXPECT_EQ(Time(23) - Duration(5), Time(18));
  EXPECT_EQ(Time(35) - Time(7), Duration(28));
}

TEST(TypesTest, TimeArithmeticAssignment) {
  Time t{20};
  t += Duration(10);
  EXPECT_EQ(t, Time(30));
  t -= Duration(24);
  EXPECT_EQ(t, Time(6));
}

TEST(TypesTest, TimeComparison) {
  EXPECT_EQ(Time(0), Time(0));
  EXPECT_NE(Time(0), Time(1));
  EXPECT_LT(Time(1), Time(2));
  EXPECT_LE(Time(4), Time(5));
  EXPECT_LE(Time(2), Time(2));
  EXPECT_GT(Time(10), Time(9));
  EXPECT_GE(Time(7), Time(5));
  EXPECT_GE(Time(5), Time(5));
}

TEST(TypesTest, InputEquality) {
  const Input kBaseline{};
  EXPECT_EQ(kBaseline, Input());
  EXPECT_NE(kBaseline, Input{.event_type = Input::EventType::kMove});
  EXPECT_NE(kBaseline, (Input{.position = {1, -1}}));
  EXPECT_NE(kBaseline, Input{.time = Time(1)});
  EXPECT_NE(kBaseline, Input{.pressure = .5});
  EXPECT_NE(kBaseline, Input{.tilt = .2});
  EXPECT_NE(kBaseline, Input{.orientation = .7});
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
