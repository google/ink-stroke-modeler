package(
    default_visibility = ["//ink_stroke_modeler:__subpackages__"],
)

licenses(["notice"])

cc_library(
    name = "internal_types",
    hdrs = ["internal_types.h"],
    deps = ["//ink_stroke_modeler:types"],
)

cc_library(
    name = "type_matchers",
    testonly = 1,
    srcs = ["type_matchers.cc"],
    hdrs = ["type_matchers.h"],
    deps = [
        ":internal_types",
        "//ink_stroke_modeler:types",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "utils",
    hdrs = ["utils.h"],
    deps = ["//ink_stroke_modeler:types"],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cc"],
    deps = [
        ":type_matchers",
        ":utils",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "wobble_smoother",
    srcs = ["wobble_smoother.cc"],
    hdrs = ["wobble_smoother.h"],
    deps = [
        ":utils",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
    ],
)

cc_test(
    name = "wobble_smoother_test",
    srcs = ["wobble_smoother_test.cc"],
    deps = [
        ":type_matchers",
        ":wobble_smoother",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "position_modeler",
    hdrs = ["position_modeler.h"],
    deps = [
        ":internal_types",
        ":utils",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
    ],
)

cc_test(
    name = "position_modeler_test",
    srcs = ["position_modeler_test.cc"],
    deps = [
        ":internal_types",
        ":position_modeler",
        ":type_matchers",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "stylus_state_modeler",
    srcs = ["stylus_state_modeler.cc"],
    hdrs = ["stylus_state_modeler.h"],
    deps = [
        ":internal_types",
        ":utils",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
    ],
)

cc_test(
    name = "stylus_state_modeler_test",
    srcs = ["stylus_state_modeler_test.cc"],
    deps = [
        ":internal_types",
        ":stylus_state_modeler",
        ":type_matchers",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "@com_google_googletest//:gtest_main",
    ],
)
