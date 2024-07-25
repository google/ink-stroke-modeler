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

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

# Use this when testonly libraries must depend on :gtest, that requires
# different behavior in the upstream repo.
alias(
    name = "gtest_for_library_testonly",
    testonly = 1,
    actual = "@com_google_googletest//:gtest",
    visibility = ["//:__subpackages__"],
)

# For building with clang-cl.
# https://bazel.build/configure/windows#clang
platform(
    name = "x64_windows-clang-cl",
    constraint_values = [
        "@platforms//cpu:x86_64",
        "@platforms//os:windows",
        "@bazel_tools//tools/cpp:clang-cl",
    ],
)
