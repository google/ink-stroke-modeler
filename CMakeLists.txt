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

cmake_minimum_required(VERSION 3.19)
Project(InkStrokeModeler VERSION 0.1 LANGUAGES CXX)

enable_testing()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(FetchContent)

option(INK_STROKE_MODELER_FIND_GTEST
  "If ON, use find_package to load an existing GoogleTest dependency."
  OFF)

option(INK_STROKE_MODELER_FIND_ABSL
  "If ON, use find_package to load an existing Abseil dependency."
  OFF)

if(INK_STROKE_MODELER_FIND_GTEST)
  find_package(gtest REQUIRED)
else()
  FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
  )
  FetchContent_MakeAvailable(gtest)
endif()

set(ABSL_PROPAGATE_CXX_STD ON)
# No reason to get two different versions of Googletest.
set(ABSL_USE_EXTERNAL_GOOGLETEST ON)
set(ABSL_FIND_GOOGLETEST ON)
if(INK_STROKE_MODELER_FIND_ABSL)
  find_package(absl REQUIRED)
else()
  FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20211102.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
  )
  FetchContent_MakeAvailable(absl)
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
