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

function(ink_cc_library)
  cmake_parse_arguments(INK_CC_LIB
    "PUBLIC"
    "NAME"
    "HDRS;SRCS;DEPS"
    ${ARGN}
  )
  add_library(${INK_CC_LIB_NAME} ${INK_CC_LIB_SRCS} ${INK_CC_LIB_HDRS})
  if(NOT DEFINED INK_CC_LIB_SRCS)
    set_target_properties(${INK_CC_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
  endif()
  target_link_libraries(${INK_CC_LIB_NAME} PUBLIC ${INK_CC_LIB_DEPS})
  if(INK_CC_LIB_PUBLIC)
    install(TARGETS ${INK_CC_LIB_NAME} EXPORT ${PROJECT_NAME}Targets)
  endif()
  target_compile_features(${INK_CC_LIB_NAME} PUBLIC cxx_std_17)
endfunction()

function(ink_cc_test)
  cmake_parse_arguments(INK_CC_TEST
    ""
    "NAME"
    "SRCS;DEPS"
    ${ARGN}
  )
  add_executable(${INK_CC_TEST_NAME} ${INK_CC_TEST_SRCS})
  target_link_libraries(${INK_CC_TEST_NAME} ${INK_CC_TEST_DEPS})
  add_test(NAME ${INK_CC_TEST_NAME} COMMAND ${INK_CC_TEST_NAME})
endfunction()
