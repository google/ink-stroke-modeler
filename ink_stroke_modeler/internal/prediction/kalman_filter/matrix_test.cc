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

#include "ink_stroke_modeler/internal/prediction/kalman_filter/matrix.h"

#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace ink {
namespace stroke_model {
namespace {

TEST(MatrixTest, Vec4Equality) {
  EXPECT_EQ(Vec4(1, 2, 3, 4), Vec4(1, 2, 3, 4));
  EXPECT_NE(Vec4(1, 2, 3, 4), Vec4(1, 2, 7, 4));
  EXPECT_NE(Vec4(.1, .7, -4, 6), Vec4(-1, 64, .3, 200));
}

TEST(MatrixTest, Vec4Addition) {
  EXPECT_EQ(Vec4(0, 1, 2, 4) + Vec4(5, -1, 6, 7), Vec4(5, 0, 8, 11));
  EXPECT_EQ(Vec4(0.25, -3, 17, 0) + Vec4(.5, -.5, -1, 2),
            Vec4(.75, -3.5, 16, 2));
}

TEST(MatrixTest, Vec4ScalarMultiplication) {
  EXPECT_EQ(Vec4(6, -12, 7, .25) * 2, Vec4(12, -24, 14, .5));
  EXPECT_EQ(Vec4(17, -3, 5.5, 0) * -.25, Vec4(-4.25, .75, -1.375, 0));
}

TEST(MatrixTest, Vec4ScalarDivision) {
  EXPECT_EQ(Vec4(13, -8, 0, 100) / -2, Vec4(-6.5, 4, 0, -50));
  EXPECT_EQ(Vec4(0, -3, 20, 1) / .2, Vec4(0, -15, 100, 5));
}

TEST(MatrixTest, Matrix4Equality) {
  EXPECT_EQ(Matrix4(0, 1, 2, 3,    //
                    4, 5, 6, 7,    //
                    8, 9, 10, 11,  //
                    12, 13, 14, 15),
            Matrix4(0, 1, 2, 3,    //
                    4, 5, 6, 7,    //
                    8, 9, 10, 11,  //
                    12, 13, 14, 15));
  EXPECT_NE(Matrix4(0, 1, 2, 3,    //
                    4, 5, 6, 7,    //
                    8, 9, 10, 11,  //
                    12, 13, 14, 15),
            Matrix4(1, 2, 0, 4,   //
                    4, 9, 6, 12,  //
                    9, 9, 9, 9,   //
                    -1, -2, 14, 99));
}

TEST(MatrixTest, Matrix4IdentityCtor) {
  EXPECT_EQ(Matrix4(), Matrix4(1, 0, 0, 0,  //
                               0, 1, 0, 0,  //
                               0, 0, 1, 0,  //
                               0, 0, 0, 1));
}

TEST(MatrixTest, Matrix4Zero) {
  EXPECT_EQ(Matrix4::Zero(), Matrix4(0, 0, 0, 0,  //
                                     0, 0, 0, 0,  //
                                     0, 0, 0, 0,  //
                                     0, 0, 0, 0));
}

TEST(MatrixTest, Matrix4Transpose) {
  Matrix4 m(0, 1, 2, 3,    //
            4, 5, 6, 7,    //
            8, 9, 10, 11,  //
            12, 13, 14, 15);
  EXPECT_EQ(m.Transpose(), Matrix4(0, 4, 8, 12,   //
                                   1, 5, 9, 13,   //
                                   2, 6, 10, 14,  //
                                   3, 7, 11, 15));
}

TEST(MatrixTest, Matrix4Multiplication) {
  Matrix4 a(-4, 4, 2, 9,   //
            -2, -5, 6, 1,  //
            -2, 7, 10, 1,  //
            -4, -5, 2, 6);
  Matrix4 b(-1, 7, 9, 3,    //
            0, 7, -3, 8,    //
            -9, 7, 7, -10,  //
            1, -1, -3, -1);
  EXPECT_EQ(a * b, Matrix4(-5, 5, -61, -9,     //
                           -51, -8, 36, -107,  //
                           -87, 104, 28, -51,  //
                           -8, -55, -25, -78));
  EXPECT_EQ(b * a, Matrix4(-40, 9, 136, 25,   //
                           -40, -96, 28, 52,  //
                           48, 28, 74, -127,  //
                           8, -7, -36, -1));
}

TEST(MatrixTest, Matrix4Addition) {
  Matrix4 a(2, 0, -10, -1,  //
            -4, -4, -7, 3,  //
            7, -1, 7, 3,    //
            -7, -4, -4, -4);
  Matrix4 b(9, -6, -10, 0,   //
            6, 1, -5, 9,     //
            -7, -4, -3, -6,  //
            7, 7, -10, -9);
  EXPECT_EQ(a + b, Matrix4(11, -6, -20, -1,  //
                           2, -3, -12, 12,   //
                           0, -5, 4, -3,     //
                           0, 3, -14, -13));
  EXPECT_EQ(b + a, Matrix4(11, -6, -20, -1,  //
                           2, -3, -12, 12,   //
                           0, -5, 4, -3,     //
                           0, 3, -14, -13));
}

TEST(MatrixTest, Matrix4Subtraction) {
  Matrix4 a(-7, -9, 9, 9,    //
            -4, 10, -3, -1,  //
            8, 9, 6, 4,      //
            9, -7, 7, 4);
  Matrix4 b(2, -1, 2, 6,     //
            -1, -8, -1, 10,  //
            3, 0, -6, -1,    //
            -6, -3, 6, 7);
  EXPECT_EQ(a - b, Matrix4(-9, -8, 7, 3,     //
                           -3, 18, -2, -11,  //
                           5, 9, 12, 5,      //
                           15, -4, 1, -3));
  EXPECT_EQ(b - a, Matrix4(9, 8, -7, -3,     //
                           3, -18, 2, 11,    //
                           -5, -9, -12, -5,  //
                           -15, 4, -1, 3));
}

TEST(MatrixTest, Matrix4ScalarMultiplication) {
  Matrix4 m(6, 8, 6, 7,    //
            2, -4, 10, 8,  //
            -1, 4, 9, -7,  //
            -2, -9, 10, 10);
  EXPECT_EQ(m * 3, Matrix4(18, 24, 18, 21,   //
                           6, -12, 30, 24,   //
                           -3, 12, 27, -21,  //
                           -6, -27, 30, 30));
  EXPECT_EQ(m * .5, Matrix4(3, 4, 3, 3.5,       //
                            1, -2, 5, 4,        //
                            -.5, 2, 4.5, -3.5,  //
                            -1, -4.5, 5, 5));
}

TEST(MatrixTest, Matrix4VectorMultiplication) {
  Matrix4 m(3, 0, 4, 3,      //
            7, -6, 7, -10,   //
            6, -2, -10, -5,  //
            -3, 9, 1, -5);
  Vec4 v(-6, 9, -7, -10);
  EXPECT_EQ(m * v, Vec4(-76, -45, 66, 142));
  EXPECT_EQ(v * m, Vec4(33, -130, 99, -23));
}

TEST(MatrixTest, DotProduct) {
  Vec4 a(0, -3, 0, -5);
  Vec4 b(6, 4, -4, 6);
  EXPECT_EQ(DotProduct(a, b), -42);
  EXPECT_EQ(DotProduct(b, a), -42);
}

TEST(MatrixTest, OuterProduct) {
  Vec4 a(-8, -3, 6, -8);
  Vec4 b(4, -9, -1, 10);
  EXPECT_EQ(OuterProduct(a, b), Matrix4(-32, 72, 8, -80,  //
                                        -12, 27, 3, -30,  //
                                        24, -54, -6, 60,  //
                                        -32, 72, 8, -80));
  EXPECT_EQ(OuterProduct(b, a), Matrix4(-32, -12, 24, -32,  //
                                        72, 27, -54, 72,    //
                                        8, 3, -6, 8,        //
                                        -80, -30, 60, -80));
}

TEST(MatrixTest, Vec4Stream) {
  std::stringstream s;
  s << Vec4(1.28, -9, .9, 2.7);
  EXPECT_EQ(s.str(), "(1.28, -9, 0.9, 2.7)");
}

TEST(MatrixTest, Matrix4Stream) {
  std::stringstream s;
  s << Matrix4(7.5, -7.7, -4.6, 8,  //
               6.4, -8.52, 0, 8.8,  //
               -3.5, -5.2, -.5, 9,  //
               -2.6, -3.4, 5.5, 8.3);
  EXPECT_EQ(s.str(),
            "\n7.5\t-7.7\t-4.6\t8"
            "\n6.4\t-8.52\t0\t8.8"
            "\n-3.5\t-5.2\t-0.5\t9"
            "\n-2.6\t-3.4\t5.5\t8.3");
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink
