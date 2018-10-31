cmake_minimum_required(VERSION 2.8.0)

set(MODULE_NAME ppp)
project(${MODULE_NAME} CXX)
set(LIB_NAME lib${MODULE_NAME})
set(TEST_NAME ${MODULE_NAME}_test)

message(STATUS "-------- CMake for module ${MODULE_NAME} --------")

#----------------------------
# Third party dependencies
#----------------------------
# OpenCV Library
find_package(OpenCV REQUIRED)

# Boost library for non C++17 compiler
if (UNIX)
    find_package(Boost REQUIRED COMPONENTS system filesystem)
endif()

set(LIB_MODE SHARED)
add_definitions(-DDLLEXPORT)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB LIB_WRP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/swig/*.c")
file(GLOB LIB_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Add some source files needed for DLIB
set(LIB_SRC_FILES ${LIB_CPP_FILES} #${LIB_WRP_FILES}
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

set(MODULE_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${DLIB_ROOT}
    ${DLIB_ROOT}/dlib/base64
    ${DLIB_ROOT}/dlib/entropy_decoder
    ${DLIB_ROOT}/dlib/entropy_encoder
)

set(MODULE_LIB_DEPS
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    ${OPENCV_3RDPARTY_LIBS}
)

include_directories(${MODULE_INC_DIRS})
add_library(${LIB_NAME} ${LIB_MODE} ${LIB_SRC_FILES} ${LIB_INC_FILES})
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})

# Install the library
install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/config.json DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/sp_model.dat DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/libpppwrapper.py DESTINATION ${CMAKE_INSTALL_PREFIX})

#-----------------------------
# Build the module tests
#-----------------------------
file(GLOB TEST_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
file(GLOB TEST_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")

#if (WIN32)
#    set(DLIB_TRAIN_SRC_FILES
#        ${DLIB_ROOT}/dlib/threads/thread_pool_extension.cpp
#        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_extensions.cpp
#        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_kernel_1.cpp
#        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_kernel_2.cpp
#        ${DLIB_ROOT}/dlib/misc_api/misc_api_kernel_1.cpp
#        ${DLIB_ROOT}/dlib/data_io/image_dataset_metadata.cpp)
#endif()

set(TEST_INC_DIRS
    ${GMOCK_ROOT}/gtest/include
    ${GMOCK_ROOT}/include
    ${MODULE_INC_DIRS}
)
set(TEST_LIB_DEPS
    ${LIB_NAME}
    ${MODULE_LIB_DEPS}
    gmock
)

add_executable(${TEST_NAME} ${TEST_SRC_FILES} ${DLIB_TRAIN_SRC_FILES} ${TEST_INC_FILES})
target_include_directories(${TEST_NAME} PUBLIC ${TEST_INC_DIRS})
target_link_libraries(${TEST_NAME} ${TEST_LIB_DEPS})

install(TARGETS ${TEST_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
