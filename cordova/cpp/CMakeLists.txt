cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(APP_CPPFLAGS "${APP_CPPFLAGS} -std=c++11 -fPIC")

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/../../../../thirdparty)
set(DLIB_ROOT ${THIRDPARTY_DIR}/dlib-19.6)

#set(OpenCV_DIR ${THIRDPARTY_DIR}/opencv-3.2.0-android-sdk/OpenCV-android-sdk/sdk/native/jni)

set(OpenCV_DIR  $ENV{OpenCV_DIR})
message("Set OpenCV_DIR to " ${OpenCV_DIR})
find_package(OpenCV REQUIRED)
add_definitions(-DOS_ANDROID)

add_definitions(-D__GXX_EXPERIMENTAL_CXX0X__)
add_definitions(-DGCC_ATOMIC_INT_LOCK_FREE=2)

set(LIBPPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libppp)
file(GLOB LIBPPP_SRC ${LIBPPP_DIR}/src/*.cpp)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${DLIB_ROOT}
    ${DLIB_ROOT}/dlib/entropy_decoder
    ${DLIB_ROOT}/dlib/entropy_encoder
    ${DLIB_ROOT}/dlib/base64
    ${DLIB_ROOT}/dlib/threads
    ${THIRDPARTY_DIR}
    ${LIBPPP_DIR}/include)

add_library(cordova-cpp-bridge-lib SHARED
    /*SOURCE_FILES*/
    ${LIBPPP_SRC}
    ${DLIB_ROOT}/dlib/base64/base64_kernel_1.cpp
    ${DLIB_ROOT}/dlib/bit_stream/bit_stream_kernel_1.cpp
    ${DLIB_ROOT}/dlib/threads/threads_kernel_1.cpp
    ${DLIB_ROOT}/dlib/threads/threads_kernel_2.cpp
    ${DLIB_ROOT}/dlib/threads/threads_kernel_shared.cpp
    ${DLIB_ROOT}/dlib/entropy_decoder/entropy_decoder_kernel_1.cpp
    ${DLIB_ROOT}/dlib/entropy_decoder/entropy_decoder_kernel_2.cpp
    ${DLIB_ROOT}/dlib/entropy_encoder/entropy_encoder_kernel_1.cpp
    ${DLIB_ROOT}/dlib/entropy_encoder/entropy_encoder_kernel_2.cpp
    ${DLIB_ROOT}/dlib/md5/md5_kernel_1.cpp
    ${DLIB_ROOT}/dlib/tokenizer/tokenizer_kernel_1.cpp
    ${DLIB_ROOT}/dlib/unicode/unicode.cpp
    ${DLIB_ROOT}/dlib/data_io/mnist.cpp
)
target_link_libraries(cordova-cpp-bridge-lib log ${OpenCV_LIBS})