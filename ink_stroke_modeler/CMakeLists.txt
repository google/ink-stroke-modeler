# Copyright 2022 Google LLC
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
  types
  absl::status
  absl::strings
  absl::variant
  PUBLIC
)

ink_cc_test(
  NAME
  params_test
  SRCS
  params_test.cc
  DEPS
  params
  absl::status
  GTest::gmock_main
)

ink_cc_library(
  NAME
  types
  HDRS
  types.h
  PUBLIC
)

ink_cc_test(
  NAME
  types_test
  SRCS
  types_test.cc
  DEPS
  types
  type_matchers
  GTest::gmock_main
)

ink_cc_library(
  NAME
  stroke_modeler
  SRCS
  stroke_modeler.cc
  HDRS
  stroke_modeler.h
  DEPS
  params
  types
  internal_types
  position_modeler
  stylus_state_modeler
  wobble_smoother
  input_predictor
  kalman_predictor
  stroke_end_predictor
  absl::core_headers
  absl::memory
  absl::status
  absl::statusor
  absl::optional
  absl::variant
)

ink_cc_test(
  NAME
  stroke_modeler_test
  SRCS
  stroke_modeler_test.cc
  DEPS
  params
  stroke_modeler
  type_matchers
  absl::status
  GTest::gmock_main
)
