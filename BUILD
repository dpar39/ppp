load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
  name = "refresh_compile_commands",

  # Specify the targets of interest.
  # For example, specify a dict of targets and any flags required to build.
  targets = {
    #"//:ppp-wasm": "-c opts--config=wasm",
    "//:ppp_test": "",
  },
  # No need to add flags already in .bazelrc. They're automatically picked up.
  # If you don't need flags, a list of targets is also okay, as is a single target string.
  # Wildcard patterns, like //... for everything, *are* allowed here, just like a build.
    # As are additional targets (+) and subtractions (-), like in bazel query https://docs.bazel.build/versions/main/query.html#expressions
  # And if you're working on a header-only library, specify a test or binary target that compiles it.
)

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")
load("@org_tensorflow//tensorflow/lite:build_def.bzl", "tflite_linkopts")

config_setting(
  name = "wasm_build",
  values = {"cpu": "wasm"},
)

cc_library(
  name = "ppp",
  srcs = glob(['libppp/src/**/*.cpp']),
  defines = ['RAPIDJSON_HAS_STDSTRING=1'],
  includes = ["libppp/include"],
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
    ":wasm_build": ["@tp_wasm//:tplibs"],
    "//conditions:default": ["@tp_linux//:tplibs", "@com_google_googletest//:gtest",],
  }),
  strip_include_prefix = "include",
)

cc_binary(
  name = "ppp-wasm",
  srcs = glob(["libppp/embind/*.cc"]),
  linkstatic=1,
  deps = [":ppp"],
)

wasm_cc_binary(
  name = "ppp-wasm-wrap",
  cc_target = ":ppp-wasm",
)

cc_test(
  name = "ppp_test",
  srcs = glob(["libppp/test/**/*.cpp"]) + glob(['libppp/test/**/*.h']),
  linkstatic=1,
  deps = [ ":ppp", "@com_google_googletest//:gtest" ],
)

cc_binary(
  name = "ppp_app",
  srcs = glob(["libppp/app/main.cpp"]) + glob(['libppp/app/**/*.h']),
  linkstatic=1,
  includes = ['libppp/app', 'libppp/app/tclap'],
  deps = [":ppp"]
)