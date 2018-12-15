cmake_minimum_required(VERSION 2.8.0)

set(MODULE_NAME ppp)
project(${MODULE_NAME} CXX)
set(LIB_NAME lib${MODULE_NAME})
set(TEST_NAME ${MODULE_NAME}_test)

message(STATUS "-------- CMake for module ${MODULE_NAME} --------")

#----------------------------
# Third party dependencies
#----------------------------
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty)
set(GMOCK_ROOT "${THIRD_PARTY_DIR}/gmock-1.7.0")
set(DLIB_ROOT "${THIRD_PARTY_DIR}/dlib-19.6")
#set(JAVA_HOME $ENV{JAVA_HOME})

if( ANDROID )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fexceptions -fPIC")
    set(APP_CPPFLAGS "${APP_CPPFLAGS} -std=c++17 -fexceptions -fPIC")
    set(OpenCV_DIR ${THIRD_PARTY_DIR}/OpenCV-android-sdk/sdk/native/jni)
    file(GLOB LIB_WRP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/swig/*java_wrap.cxx")
else()
    if (CMAKE_SIZEOF_VOID_P MATCHES "4")
        set(ARCH_NAME "x86")
    else()
        set(ARCH_NAME "x64")
    endif()
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    set(OpenCV_DIR ${THIRD_PARTY_DIR}/install_${BUILD_TYPE}_${ARCH_NAME})
    if(EXISTS ${OpenCV_DIR}/share/OpenCV/OpenCVConfig.cmake)
        set(OpenCV_DIR ${OpenCV_DIR}/share/OpenCV)
    endif()
    message(STATUS "Setting OpenCV_DIR=${OpenCV_DIR}")
    add_definitions(-DDLLEXPORT)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    file(GLOB LIB_WRP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/swig/*nodejs_wrap.cxx")
endif()


# Find OpenCV Library
find_package(OpenCV REQUIRED)

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB LIB_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Add some source files needed for DLIB
set(LIB_SRC_FILES ${LIB_CPP_FILES} ${LIB_WRP_FILES}
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
    ${DLIB_ROOT}
    ${DLIB_ROOT}/dlib/base64
    ${DLIB_ROOT}/dlib/entropy_decoder
    ${DLIB_ROOT}/dlib/entropy_encoder
    ${THIRD_PARTY_DIR}
)

set(MODULE_LIB_DEPS
    ${OpenCV_LIBS}
    ${OPENCV_3RDPARTY_LIBS}
)

include_directories(${MODULE_INC_DIRS})
add_library(${LIB_NAME} SHARED ${LIB_SRC_FILES} ${LIB_INC_FILES})
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})

if (NOT ANDROID AND NOT IOS)
    # Install the library
    install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/config.bundle.json DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/libpppwrapper.py DESTINATION ${CMAKE_INSTALL_PREFIX})

    #-----------------------------
    # Build the module tests
    #-----------------------------
    #if (UNIX) # Boost library for non C++17 compiler
    #    find_package(Boost REQUIRED COMPONENTS system filesystem)
    #endif()

    file(GLOB TEST_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
    file(GLOB TEST_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")

    set(TEST_INC_DIRS
        ${MODULE_INC_DIRS}
        ${GMOCK_ROOT}/gtest/include
        ${GMOCK_ROOT}/include
        ${Boost_INCLUDE_DIRS}
    )
    set(TEST_LIB_DEPS
        ${MODULE_LIB_DEPS}
        ${LIB_NAME}
        ${Boost_LIBRARIES}
        gmock
        stdc++fs
    )

    add_executable(${TEST_NAME} ${TEST_SRC_FILES} ${DLIB_TRAIN_SRC_FILES} ${TEST_INC_FILES})
    target_include_directories(${TEST_NAME} PUBLIC ${TEST_INC_DIRS})
    target_link_libraries(${TEST_NAME} ${TEST_LIB_DEPS})

    install(TARGETS ${TEST_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()