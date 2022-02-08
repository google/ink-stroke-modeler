package(
    default_visibility = ["//ink_stroke_modeler:__subpackages__"],
)

licenses(["notice"])

cc_library(
    name = "input_predictor",
    hdrs = ["input_predictor.h"],
    deps = [
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "//ink_stroke_modeler/internal:internal_types",
    ],
)

cc_library(
    name = "kalman_predictor",
    srcs = ["kalman_predictor.cc"],
    hdrs = ["kalman_predictor.h"],
    deps = [
        ":input_predictor",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "//ink_stroke_modeler/internal:internal_types",
        "//ink_stroke_modeler/internal:utils",
        "//ink_stroke_modeler/internal/prediction/kalman_filter",
    ],
)

cc_test(
    name = "kalman_predictor_test",
    srcs = ["kalman_predictor_test.cc"],
    deps = [
        ":input_predictor",
        ":kalman_predictor",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "//ink_stroke_modeler/internal:type_matchers",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "stroke_end_predictor",
    srcs = ["stroke_end_predictor.cc"],
    hdrs = ["stroke_end_predictor.h"],
    deps = [
        ":input_predictor",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler:types",
        "//ink_stroke_modeler/internal:internal_types",
        "//ink_stroke_modeler/internal:position_modeler",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_test(
    name = "stroke_end_predictor_test",
    srcs = ["stroke_end_predictor_test.cc"],
    deps = [
        ":input_predictor",
        ":stroke_end_predictor",
        "//ink_stroke_modeler:params",
        "//ink_stroke_modeler/internal:type_matchers",
        "@com_google_googletest//:gtest_main",
    ],
)
