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

add_subdirectory(kalman_filter)

ink_cc_library(
  NAME
  input_predictor
  HDRS
  input_predictor.h
  DEPS
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::internal_types
)

ink_cc_library(
  NAME
  kalman_predictor
  SRCS
  kalman_predictor.cc
  HDRS
  kalman_predictor.h
  DEPS
  InkStrokeModeler::input_predictor
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::internal_types
  InkStrokeModeler::utils
  InkStrokeModeler::kalman_filter
)

ink_cc_test(
  NAME
  kalman_predictor_test
  SRCS
  kalman_predictor_test.cc
  DEPS
  InkStrokeModeler::input_predictor
  InkStrokeModeler::kalman_predictor
  GTest::gmock_main
  absl::str_format
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::internal_types
  InkStrokeModeler::type_matchers
)

ink_cc_library(
  NAME
  stroke_end_predictor
  SRCS
  stroke_end_predictor.cc
  HDRS
  stroke_end_predictor.h
  DEPS
  InkStrokeModeler::input_predictor
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::internal_types
  InkStrokeModeler::position_modeler
)

ink_cc_test(
  NAME
  stroke_end_predictor_test
  SRCS
  stroke_end_predictor_test.cc
  DEPS
  InkStrokeModeler::input_predictor
  InkStrokeModeler::stroke_end_predictor
  GTest::gmock_main
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::internal_types
  InkStrokeModeler::type_matchers
)
