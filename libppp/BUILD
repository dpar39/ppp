load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_linkopts")
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

config_setting(
  name = "wasm_build",
  values = {"cpu": "wasm"},
)

cc_library(
  name = "libppp",
  srcs = glob(['src/**/*.cpp']),
  copts = ["--std=c++20"],
  linkopts = tflite_linkopts() + [],
  defines = ['RAPIDJSON_HAS_STDSTRING=1'],
  includes = ['include'],
  hdrs = glob(['include/**/*.h']),
  linkstatic=1,
  deps = [
    "@org_mediapipe//mediapipe/framework:calculator_framework",
    "@org_mediapipe//mediapipe/framework/formats:image_frame",
    "@org_mediapipe//mediapipe/framework/formats:landmark_cc_proto",
    "@org_mediapipe//mediapipe/framework/formats:image_frame_opencv",
    "@org_mediapipe//mediapipe/framework/port:opencv_imgproc",
    "@org_mediapipe//mediapipe/framework/port:parse_text_proto",
    "@org_mediapipe//mediapipe/framework/port:status",
    "@org_mediapipe//mediapipe/graphs/face_mesh:desktop_live_calculators",
    "@org_mediapipe//mediapipe/graphs/selfie_segmentation:selfie_segmentation_cpu_deps",
  ] + select({
    ':wasm_build': ['@tp_wasm//:tplibs'],
    '//conditions:default': ['@tp_linux//:tplibs'],
  }),
  strip_include_prefix = "include",
  visibility = ["//visibility:public"],
)


cc_binary(
  name = "ppp-wasm",
  srcs = ['src/PublicPppWasm.cc'],
  deps = [':libppp'],
  visibility = ["//visibility:public"],
  copts = ["--std=c++20"],
)

wasm_cc_binary(
  name = "ppp-wasm-wrap",
  cc_target = ":ppp-wasm",
)

cc_binary(
  name = "ppp_app",
  srcs = glob(["app/main.cpp"]) + glob(['app/**/*.h']),
  includes = ['app', 'app/tclap'],
  deps = [
    ':libppp'
  ],
  copts = ["--std=c++20"],
)

cc_test(
  name = "ppp_test",
  srcs = glob(["test/**/*.cpp"]) + glob(['test/**/*.h']),
  linkstatic=1,
  copts = ["--std=c++20"],
  deps = [
    ':libppp',
    "@com_google_googletest//:gtest",
  ],
)