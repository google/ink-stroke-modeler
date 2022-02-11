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

cmake_minimum_required(VERSION 3.15)
Project(InkStrokeModeler VERSION 0.1 LANGUAGES CXX)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20211102.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
set(ABSL_PROPAGATE_CXX_STD ON)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(ABSL_ENABLE_INSTALL ON)
endif()
# No reason to get two different versions of Googletest.
set(ABSL_USE_EXTERNAL_GOOGLETEST ON)
FetchContent_MakeAvailable(abseil-cpp)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_COMPILER clang++)
endif()

if(CMAKE_CXX_STANDARD LESS 17)
  message(FATAL_ERROR
      "${PROJECT_NAME} requires CMAKE_CXX_STANDARD >= 17 (got: ${CMAKE_CXX_STANDARD})")
endif()

include_directories("${CMAKE_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(InkBazelEquivalents)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(ink_stroke_modeler)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE InkStrokeModeler::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
endif()
