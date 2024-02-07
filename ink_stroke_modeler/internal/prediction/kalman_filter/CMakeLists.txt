# Copyright 2022-2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ink_cc_test(
  NAME
  axis_predictor_test
  SRCS
  axis_predictor_test.cc
  DEPS
  InkStrokeModeler::kalman_filter
  GTest::gmock_main
)

ink_cc_library(
  NAME
  kalman_filter
  SRCS
  axis_predictor.cc
  kalman_filter.cc
  HDRS
  axis_predictor.h
  kalman_filter.h
  DEPS
  InkStrokeModeler::matrix
)

ink_cc_library(
  NAME
  matrix
  HDRS
  matrix.h
)

ink_cc_test(
  NAME
  matrix_test
  SRCS
  matrix_test.cc
  DEPS
  InkStrokeModeler::matrix
  GTest::gmock_main
)
