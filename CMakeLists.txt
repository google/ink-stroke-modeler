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

cmake_minimum_required(VERSION 3.19)
Project(InkStrokeModeler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++20")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  option(INK_STROKE_MODELER_BUILD_TESTING "Build tests and testonly libraries" ON)
  option(INK_STROKE_MODELER_ENABLE_INSTALL "Enable install rule" ON)
else()
  option(INK_STROKE_MODELER_BUILD_TESTING "Build tests and testonly libraries" OFF)
  option(INK_STROKE_MODELER_ENABLE_INSTALL "Enable install rule" OFF)
endif()

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(FetchContent)

option(INK_STROKE_MODELER_FIND_DEPENDENCIES
  "If ON, use find_package to load all dependencies."
  OFF)

cmake_dependent_option(INK_STROKE_MODELER_FIND_GTEST
  "If ON, use find_package to load an existing GoogleTest dependency."
  ON
  "INK_STROKE_MODELER_FIND_DEPENDENCIES"
  OFF)

cmake_dependent_option(INK_STROKE_MODELER_FIND_ABSL
  "If ON, use find_package to load an existing Abseil dependency."
  ON
  "INK_STROKE_MODELER_FIND_DEPENDENCIES"
  OFF)

cmake_dependent_option(INK_STROKE_MODELER_FIND_FUZZTEST
  "If ON, use find_package to load an existing Fuzztest dependency."
  ON
  "INK_STROKE_MODELER_FIND_DEPENDENCIES"
  OFF)

set(ABSL_PROPAGATE_CXX_STD ON)

# Used by Fuzztest to fetch Abseil and Googletest deps
set(gtest_URL https://github.com/google/googletest.git)
set(gtest_TAG f8d7d77c06936315286eb55f8de22cd23c188571)  # v1.14.0
set(absl_URL https://github.com/abseil/abseil-cpp.git)
set(absl_TAG 2f9e432cce407ce0ae50676696666f33a77d42ac)  # 20240116.0

if(INK_STROKE_MODELER_ENABLE_INSTALL)
  set(ABSL_ENABLE_INSTALL ON)
endif()

if(INK_STROKE_MODELER_BUILD_TESTING)
  enable_testing()

  # No reason to get two different versions of Googletest.
  # gtest and absl fetched indirectly by fuzztest
  set(ABSL_USE_EXTERNAL_GOOGLETEST ON)

  if(INK_STROKE_MODELER_FIND_FUZZTEST)
    find_package(fuzztest REQUIRED)
  else()
    set(FUZZTEST_BUILD_TESTING OFF)
    FetchContent_Declare(
      fuzztest
      GIT_REPOSITORY https://github.com/google/fuzztest.git
      GIT_TAG        529b2bfd547281a9829548d7293ea47fb81e30aa  # head at 2024-02-14
      GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(fuzztest)
  endif()
  fuzztest_setup_fuzzing_flags()
else()
  if(INK_STROKE_MODELER_FIND_ABSL)
    find_package(absl REQUIRED)
  else()
    set(ABSL_BUILD_TESTING OFF)
    FetchContent_Declare(
      abseil-cpp
      GIT_REPOSITORY ${absl_URL}
      GIT_TAG        ${absl_TAG}
      GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(abseil-cpp)
  endif()
endif()

if(CMAKE_CXX_STANDARD LESS 20)
  message(FATAL_ERROR
      "${PROJECT_NAME} requires CMAKE_CXX_STANDARD >= 20 (got: ${CMAKE_CXX_STANDARD})")
endif()

include_directories("${CMAKE_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(InkBazelEquivalents)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(ink_stroke_modeler)

if(INK_STROKE_MODELER_ENABLE_INSTALL)
  install(EXPORT InkStrokeModelerTargets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/InkStrokeModeler"
    NAMESPACE "InkStrokeModeler::"
  )

  configure_package_config_file(
    cmake/InkStrokeModelerConfig.cmake.in
    "${PROJECT_BINARY_DIR}/InkStrokeModelerConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/InkStrokeModeler"
  )
  install(FILES "${PROJECT_BINARY_DIR}/InkStrokeModelerConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/InkStrokeModeler"
  )
  install(DIRECTORY ink_stroke_modeler
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN "*.h"
      PATTERN "CMakeFiles" EXCLUDE)
endif()
