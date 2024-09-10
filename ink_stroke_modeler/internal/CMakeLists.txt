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

add_subdirectory(prediction)

ink_cc_library(
  NAME
  internal_types
  SRCS
  internal_types.cc
  HDRS
  internal_types.h
  DEPS
  absl::str_format
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  internal_types_test
  SRCS
  internal_types_test.cc
  DEPS
  InkStrokeModeler::internal_types
  GTest::gmock_main
  absl::str_format
  InkStrokeModeler::types
)

ink_cc_library(
  NAME
  position_modeler
  SRCS
  position_modeler.cc
  HDRS
  position_modeler.h
  DEPS
  InkStrokeModeler::internal_types
  InkStrokeModeler::utils
  absl::status
  absl::statusor
  absl::strings
  InkStrokeModeler::params
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  position_modeler_test
  SRCS
  position_modeler_test.cc
  DEPS
  InkStrokeModeler::internal_types
  InkStrokeModeler::position_modeler
  InkStrokeModeler::type_matchers
  GTest::gmock_main
  absl::status
  absl::statusor
  InkStrokeModeler::params
  InkStrokeModeler::types
)

ink_cc_library(
  NAME
  stylus_state_modeler
  SRCS
  stylus_state_modeler.cc
  HDRS
  stylus_state_modeler.h
  DEPS
  InkStrokeModeler::internal_types
  InkStrokeModeler::utils
  InkStrokeModeler::params
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  stylus_state_modeler_test
  SRCS
  stylus_state_modeler_test.cc
  DEPS
  InkStrokeModeler::internal_types
  InkStrokeModeler::stylus_state_modeler
  InkStrokeModeler::type_matchers
  InkStrokeModeler::types
  GTest::gmock_main
  InkStrokeModeler::params
)

ink_cc_library(
  NAME
  type_matchers
  TESTONLY
  SRCS
  type_matchers.cc
  HDRS
  type_matchers.h
  DEPS
  InkStrokeModeler::internal_types
  GTest::gmock_main
  absl::strings
  InkStrokeModeler::types
)

ink_cc_library(
  NAME
  utils
  SRCS
  utils.cc
  HDRS
  utils.h
  DEPS
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  utils_test
  SRCS
  utils_test.cc
  DEPS
  InkStrokeModeler::type_matchers
  InkStrokeModeler::utils
  GTest::gmock_main
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  validaiton_test
  SRCS
  validation_test.cc
  DEPS
  InkStrokeModeler::validation
  GTest::gmock_main
  absl::status
)

ink_cc_library(
  NAME
  validation
  HDRS
  validation.h
  DEPS
  absl::status
  absl::strings
)

ink_cc_library(
  NAME
  wobble_smoother
  SRCS
  wobble_smoother.cc
  HDRS
  wobble_smoother.h
  DEPS
  InkStrokeModeler::utils
  InkStrokeModeler::params
  InkStrokeModeler::types
)

ink_cc_test(
  NAME
  wobble_smoother_test
  SRCS
  wobble_smoother_test.cc
  DEPS
  InkStrokeModeler::type_matchers
  InkStrokeModeler::wobble_smoother
  GTest::gmock_main
  InkStrokeModeler::params
  InkStrokeModeler::types
)

ink_cc_library(
  NAME
  loop_contraction_mitigation_modeler
  SRCS
  loop_contraction_mitigation_modeler.cc
  HDRS
  loop_contraction_mitigation_modeler.h
  DEPS
  InkStrokeModeler::params
  InkStrokeModeler::types
  InkStrokeModeler::utils
)

ink_cc_test(
  NAME
  loop_contraction_mitigation_modeler_test
  SRCS
  loop_contraction_mitigation_modeler_test.cc
  DEPS
  InkStrokeModeler::loop_contraction_mitigation_modeler
  GTest::gmock_main
  InkStrokeModeler::params
  InkStrokeModeler::types
)
