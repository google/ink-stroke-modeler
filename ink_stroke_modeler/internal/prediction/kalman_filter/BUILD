package(
    default_visibility = ["//ink_stroke_modeler:__subpackages__"],
)

licenses(["notice"])

cc_library(
    name = "matrix",
    hdrs = ["matrix.h"],
)

cc_test(
    name = "matrix_test",
    srcs = ["matrix_test.cc"],
    deps = [
        ":matrix",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "kalman_filter",
    srcs = [
        "axis_predictor.cc",
        "kalman_filter.cc",
    ],
    hdrs = [
        "axis_predictor.h",
        "kalman_filter.h",
    ],
    deps = [
        ":matrix",
        "@com_google_absl//absl/memory",
    ],
)

cc_test(
    name = "axis_predictor_test",
    srcs = ["axis_predictor_test.cc"],
    deps = [
        ":kalman_filter",
        "@com_google_googletest//:gtest_main",
    ],
)
