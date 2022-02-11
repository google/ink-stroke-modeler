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

add_subdirectory(prediction)

ink_cc_library(
  NAME
  internal_types
  HDRS
  internal_types.h
  DEPS
  types
)

ink_cc_library(
  NAME
  type_matchers
  SRCS
  type_matchers.cc
  HDRS
  type_matchers.h
  DEPS
  internal_types
  types
  GTest::gmock_main
)

ink_cc_library(
  NAME
  utils
  HDRS
  utils.h
  DEPS
  types
)

ink_cc_test(
  NAME
  utils_test
  SRCS
  utils_test.cc
  DEPS
  type_matchers
  utils
  GTest::gmock_main
)

ink_cc_library(
  NAME
  wobble_smoother
  SRCS
  wobble_smoother.cc
  HDRS
  wobble_smoother.h
  DEPS
  utils
  params
  types
)

ink_cc_test(
  NAME
  wobble_smoother_test
  SRCS
  wobble_smoother_test.cc
  DEPS
  type_matchers
  wobble_smoother
  params
  types
  GTest::gmock_main
)

ink_cc_library(
  NAME
  position_modeler
  HDRS
  position_modeler.h
  DEPS
  internal_types
  utils
  params
  types
)

ink_cc_test(
  NAME
  position_modeler_test
  SRCS
  position_modeler_test.cc
  DEPS
  internal_types
  position_modeler
  type_matchers
  params
  types
  GTest::gmock_main
)

ink_cc_library(
  NAME
  stylus_state_modeler
  SRCS
  stylus_state_modeler.cc
  HDRS
  stylus_state_modeler.h
  DEPS
  internal_types
  utils
  params
)

ink_cc_test(
  NAME
  stylus_state_modeler_test
  SRCS
  stylus_state_modeler_test.cc
  DEPS
  internal_types
  stylus_state_modeler
  type_matchers
  params
  types
  GTest::gmock_main
)
