# Copyright 2024 Google LLC
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

"""Bazelmod repository extensions."""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def _non_module_dev_deps_impl(_ctx):
    maybe(
        git_repository,
        name = "com_google_fuzztest",
        remote = "https://github.com/google/fuzztest.git",
        commit = "529b2bfd547281a9829548d7293ea47fb81e30aa",
    )

non_module_dev_deps = module_extension(
    implementation = _non_module_dev_deps_impl,
)
