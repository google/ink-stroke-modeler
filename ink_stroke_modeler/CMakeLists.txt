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

add_subdirectory(internal)

ink_cc_library(
  NAME
  params
  SRCS
  params.cc
  HDRS
  params.h
  DEPS
  InkStrokeModeler::types
  absl::status
  absl::strings
  InkStrokeModeler::validation
)

ink_cc_test(
  NAME
  params_test
  SRCS
  params_test.cc
  DEPS
  InkStrokeModeler::params
  InkStrokeModeler::types
  GTest::gmock_main
  absl::status
)

ink_cc_library(
  NAME
  stroke_modeler
  SRCS
  stroke_modeler.cc
  HDRS
  stroke_modeler.h
  DEPS
  InkStrokeModeler::params
  InkStrokeModeler::types
  absl::status
  absl::statusor
  InkStrokeModeler::internal_types
  InkStrokeModeler::loop_contraction_mitigation_modeler
  InkStrokeModeler::position_modeler
  InkStrokeModeler::stylus_state_modeler
  InkStrokeModeler::wobble_smoother
  InkStrokeModeler::input_predictor
  InkStrokeModeler::kalman_predictor
  InkStrokeModeler::stroke_end_predictor
  InkStrokeModeler::utils
)

ink_cc_test(
  NAME
  stroke_modeler_fuzz_test
  SRCS
  stroke_modeler_fuzz_test.cc
  DEPS
  fuzztest::fuzztest_gtest_main
  InkStrokeModeler::params
  InkStrokeModeler::stroke_modeler
  InkStrokeModeler::types
  fuzztest::fuzztest
)

ink_cc_test(
  NAME
  stroke_modeler_test
  SRCS
  stroke_modeler_test.cc
  DEPS
  InkStrokeModeler::params
  InkStrokeModeler::stroke_modeler
  InkStrokeModeler::types
  GTest::gmock_main
  absl::status
  absl::strings
  InkStrokeModeler::type_matchers
  InkStrokeModeler::utils
)

ink_cc_library(
  NAME
  types
  SRCS
  types.cc
  HDRS
  types.h
  DEPS
  absl::status
  absl::statusor
  absl::strings
  absl::str_format
  InkStrokeModeler::validation
)

ink_cc_test(
  NAME
  types_test
  SRCS
  types_test.cc
  DEPS
  InkStrokeModeler::types
  GTest::gmock_main
  absl::status
  absl::str_format
  InkStrokeModeler::type_matchers
)
