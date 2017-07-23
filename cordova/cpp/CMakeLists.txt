cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/../../../../thirdparty)
set(OpenCV_DIR ${THIRDPARTY_DIR}/opencv-3.2.0-android-sdk/OpenCV-android-sdk/sdk/native/jni)
message("Set OpenCV_DIR to " ${OpenCV_DIR})
find_package(OpenCV REQUIRED)

set(LIBPPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libppp)

file(GLOB LIBPPP_SRC ${LIBPPP_DIR}/src/*.cpp)
message(${LIBPPP_SRC})

include_directories(${OpenCV_INCLUDE_DIRS} ${THIRDPARTY_DIR} ${LIBPPP_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(cordova-cpp-bridge-lib SHARED /*SOURCE_FILES*/ ${LIBPPP_SRC})
target_link_libraries(cordova-cpp-bridge-lib log ${OpenCV_LIBS})