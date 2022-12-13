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

"""Set up dependencies for Ink Stroke Modeler.

To use this from a consumer, add the following to your WORKSPACE setup:

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# https://bazel.build/rules/lib/repo/git#git_repository
git_repository(
    name = "ink_stroke_modeler",
    remote = "https://github.com/google/ink-stroke-modeler.git",
    branch = "main",
)
load("@ink_stroke_modeler//:workspace.bzl", "ink_stroke_modeler_workspace")
ink_stroke_modeler_workspace()

If you want to make it possible for your consumer to be consumed by other
Bazel projects, factor that setup into a workspace.bzl file, following
the pattern below.

If you want to use a local version of this repo instead, use local_repository
instead of git_repository:

# https://bazel.build/reference/be/workspace#local_repository
local_repository(
    name = "ink_stroke_modeler",
    path = "path/to/ink-stroke-modeler",
)

Ink Stroke Modeler requires C++20. This is not currently the default for Bazel,
--cxxopt='-std=c++20' (or newer) is required. You can put the following in
.bazelrc at your project's root:

build --cxxopt='-std=c++20'
"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def ink_stroke_modeler_workspace():
    maybe(
        git_repository,
        name = "com_google_absl",
        remote = "https://github.com/abseil/abseil-cpp.git",
        # tag = "20211102.0",
        commit = "215105818dfde3174fe799600bb0f3cae233d0bf",
        shallow_since = "1635953174 -0400",
    )

    maybe(
        git_repository,
        name = "com_google_googletest",
        remote = "https://github.com/google/googletest.git",
        # tag = "release-1.11.0",
        commit = "e2239ee6043f73722e7aa812a459f54a28552929",
        shallow_since = "1623433346 -0700",
    )
