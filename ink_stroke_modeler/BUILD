package(default_visibility = ["//visibility:public"])

licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "params",
    srcs = ["params.cc"],
    hdrs = ["params.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":types",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "params_test",
    srcs = ["params_test.cc"],
    deps = [
        ":params",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "types",
    hdrs = ["types.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "types_test",
    srcs = ["types_test.cc"],
    deps = [
        ":types",
        "//ink_stroke_modeler/internal:type_matchers",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "stroke_modeler",
    srcs = ["stroke_modeler.cc"],
    hdrs = ["stroke_modeler.h"],
    deps = [
        ":params",
        ":types",
        "//ink_stroke_modeler/internal:internal_types",
        "//ink_stroke_modeler/internal:position_modeler",
        "//ink_stroke_modeler/internal:stylus_state_modeler",
        "//ink_stroke_modeler/internal:wobble_smoother",
        "//ink_stroke_modeler/internal/prediction:input_predictor",
        "//ink_stroke_modeler/internal/prediction:kalman_predictor",
        "//ink_stroke_modeler/internal/prediction:stroke_end_predictor",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "stroke_modeler_test",
    srcs = ["stroke_modeler_test.cc"],
    deps = [
        ":params",
        ":stroke_modeler",
        "//ink_stroke_modeler/internal:type_matchers",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)
