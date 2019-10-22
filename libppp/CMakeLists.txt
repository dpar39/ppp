cmake_minimum_required(VERSION 3.2.0)

set(MODULE_NAME ppp)
project(${MODULE_NAME})
set(LIB_NAME  lib${MODULE_NAME})
set(APP_NAME  ${MODULE_NAME}_app)
set(TEST_NAME ${MODULE_NAME}_test)

message(STATUS "-------- CMake for module ${MODULE_NAME} --------")

if (CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(ARCH_NAME "x86")
else()
    set(ARCH_NAME "x64")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
set(BUILD_NAME_SUFFIX ${SYSTEM_NAME}_${BUILD_TYPE}_${ARCH_NAME} )

#----------------------------
# Third party dependencies
#----------------------------
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty)
set(DLIB_ROOT "${THIRD_PARTY_DIR}/dlib-19.18")

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fexceptions -fPIC")
    set(APP_CPPFLAGS "${APP_CPPFLAGS} -std=c++17 -fexceptions -fPIC")
    set(OpenCV_DIR ${THIRD_PARTY_DIR}/OpenCV-android-sdk/sdk/native/jni)
    file(GLOB LIB_WRP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/swig/*java_wrap.cxx")
else()
    set(OpenCV_STATIC ON)
    if (DEFINED EMSCRIPTEN)
        SET(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS} ${CMAKE_CXX_FLAGS}")
        message(STATUS "Setting EMCC_LINKER_FLAGS=${EMCC_LINKER_FLAGS}")
        set(OpenCV_DIR ${THIRD_PARTY_DIR}/install_emscripten)
    else()
        if (MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4834")
            foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                if(${FLAG_VAR} MATCHES "/MD")
                    string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
                endif()
            endforeach()
            add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
        endif()
        if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
            set(CMAKE_MACOSX_RPATH 1)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -fexceptions -pthread")
            set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC -fexceptions -pthread")
        endif()
        add_definitions(-DDLLEXPORT)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
        set(OpenCV_DIR ${THIRD_PARTY_DIR}/install_${BUILD_NAME_SUFFIX})
    endif()

    if(EXISTS ${OpenCV_DIR}/share/OpenCV/OpenCVConfig.cmake)
        set(OpenCV_DIR ${OpenCV_DIR}/share/OpenCV)
    elseif(EXISTS ${OpenCV_DIR}/lib/cmake/opencv4/OpenCVConfig.cmake)
        set(OpenCV_DIR ${OpenCV_DIR}/lib/cmake/opencv4/)
    endif()
    message(STATUS "Setting OpenCV_DIR=${OpenCV_DIR}")

    # Not used for now as we are not building node addon
    file(GLOB LIB_WRP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/swig/*nodejs_wrap.cxx")
endif()

if (APPLE)
   set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
   set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
   set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
   set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

# Find OpenCV Library
find_package(OpenCV REQUIRED)

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB LIB_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set(DLIB_SOURCES
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
)

# Add all source files needed for the library or wasm executable
set(LIB_SRC_FILES ${LIB_CPP_FILES} ${LIB_WRP_FILES} ${DLIB_SOURCES})

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
if (DEFINED EMSCRIPTEN)
    add_executable(${LIB_NAME} ${LIB_SRC_FILES} ${LIB_INC_FILES})
else()
    add_library(${LIB_NAME} ${LIB_SRC_FILES} ${LIB_INC_FILES})
endif()
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})

# Unit tests for libppp
if (NOT ANDROID AND NOT IOS AND NOT DEFINED EMSCRIPTEN)
    # Install the library as we need everything in a single place for unit testing
    install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/config.bundle.json DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/libpppwrapper.py DESTINATION ${CMAKE_INSTALL_PREFIX})

    set(GTest_DIR ${THIRD_PARTY_DIR}/install_${BUILD_NAME_SUFFIX})
    set(GTest_INCLUDE_DIRS ${GTest_DIR}/include)
    set(DEBUG_SUFFIX "")
    if(${BUILD_TYPE} STREQUAL debug)
        set(DEBUG_SUFFIX "d")
    endif()
    if (MSVC)
        set(GTest_LIBRARIES ${GTest_DIR}/lib/gtest${DEBUG_SUFFIX}.lib ${GTest_DIR}/lib/gmock${DEBUG_SUFFIX}.lib)
    else()
        set(GTest_LIBRARIES ${GTest_DIR}/lib/libgtest${DEBUG_SUFFIX}.a ${GTest_DIR}/lib/libgmock${DEBUG_SUFFIX}.a)
    endif()

    #-----------------------------
    # Build the module tests
    #-----------------------------
    if (UNIX) # Boost library for non C++17 compiler that provide filesystem
        find_package(Boost REQUIRED COMPONENTS system filesystem)
    endif()

    file(GLOB TEST_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
    file(GLOB TEST_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")

    set(TEST_INC_DIRS
        ${MODULE_INC_DIRS}
        ${GTest_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
    )
    set(TEST_LIB_DEPS
        ${MODULE_LIB_DEPS}
        ${LIB_NAME}
        ${Boost_LIBRARIES}
        ${GTest_LIBRARIES}
    )

    add_executable(${TEST_NAME} ${TEST_SRC_FILES}  ${TEST_INC_FILES})
    target_include_directories(${TEST_NAME} PUBLIC ${TEST_INC_DIRS})
    target_link_libraries(${TEST_NAME} ${TEST_LIB_DEPS})
    install(TARGETS ${TEST_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# Command line app
if (NOT ANDROID AND NOT IOS AND NOT DEFINED EMSCRIPTEN)
    file(GLOB APP_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp")
    file(GLOB APP_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/app/*.h")
    set(APP_INC_DIRS
        ${MODULE_INC_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/app
    )
    set(APP_LIB_DEPS
        ${MODULE_LIB_DEPS}
        ${LIB_NAME}
    )

    add_executable(${APP_NAME} ${APP_SRC_FILES} ${APP_INC_FILES})
    target_include_directories(${APP_NAME} PUBLIC ${APP_INC_DIRS})
    target_link_libraries(${APP_NAME} ${APP_LIB_DEPS})
    install(TARGETS ${APP_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# Dlib model trainer app
if (NOT ANDROID AND NOT IOS AND NOT DEFINED EMSCRIPTEN)
    file(GLOB LIBPNG_SRC_FILES ${DLIB_ROOT}/dlib/external/libpng/*.c)
    file(GLOB ZLIB_SRC_FILES ${DLIB_ROOT}/dlib/external/zlib/*.c)
    file(GLOB LIBJPEG_SRC_FILES ${DLIB_ROOT}/dlib/external/libjpeg/*.c)

    set(LIBJPEG_DIR ${DLIB_ROOT}/dlib/external/libjpeg)
    set(LIBJPEG_SRC_FILES
        ${LIBJPEG_DIR}/jaricom.c
        ${LIBJPEG_DIR}/jcapimin.c
        ${LIBJPEG_DIR}/jcapistd.c
        ${LIBJPEG_DIR}/jcarith.c
        ${LIBJPEG_DIR}/jccoefct.c
        ${LIBJPEG_DIR}/jccolor.c
        ${LIBJPEG_DIR}/jcdctmgr.c
        ${LIBJPEG_DIR}/jchuff.c
        ${LIBJPEG_DIR}/jcinit.c
        ${LIBJPEG_DIR}/jcmainct.c
        ${LIBJPEG_DIR}/jcmarker.c
        ${LIBJPEG_DIR}/jcmaster.c
        ${LIBJPEG_DIR}/jcomapi.c
        ${LIBJPEG_DIR}/jcparam.c
        ${LIBJPEG_DIR}/jcprepct.c
        ${LIBJPEG_DIR}/jcsample.c
        ${LIBJPEG_DIR}/jctrans.c
        ${LIBJPEG_DIR}/jdapimin.c
        ${LIBJPEG_DIR}/jdapistd.c
        ${LIBJPEG_DIR}/jdarith.c
        ${LIBJPEG_DIR}/jdatadst.c
        ${LIBJPEG_DIR}/jdatasrc.c
        ${LIBJPEG_DIR}/jdcoefct.c
        ${LIBJPEG_DIR}/jdcolor.c
        ${LIBJPEG_DIR}/jddctmgr.c
        ${LIBJPEG_DIR}/jdhuff.c
        ${LIBJPEG_DIR}/jdinput.c
        ${LIBJPEG_DIR}/jdmainct.c
        ${LIBJPEG_DIR}/jdmarker.c
        ${LIBJPEG_DIR}/jdmaster.c
        ${LIBJPEG_DIR}/jdmerge.c
        ${LIBJPEG_DIR}/jdpostct.c
        ${LIBJPEG_DIR}/jdsample.c
        ${LIBJPEG_DIR}/jdtrans.c
        ${LIBJPEG_DIR}/jerror.c
        ${LIBJPEG_DIR}/jfdctflt.c
        ${LIBJPEG_DIR}/jfdctfst.c
        ${LIBJPEG_DIR}/jfdctint.c
        ${LIBJPEG_DIR}/jidctflt.c
        ${LIBJPEG_DIR}/jidctfst.c
        ${LIBJPEG_DIR}/jidctint.c
        ${LIBJPEG_DIR}/jquant1.c
        ${LIBJPEG_DIR}/jquant2.c
        ${LIBJPEG_DIR}/jutils.c
        ${LIBJPEG_DIR}/jmemmgr.c
        ${LIBJPEG_DIR}/jmemansi.c
    )

    add_definitions(-DDLIB_JPEG_SUPPORT -DDLIB_PNG_SUPPORT -DDLIB_HAVE_SSE2 -DDLIB_NO_GUI_SUPPORT)
    add_executable(trainer
        ${CMAKE_CURRENT_SOURCE_DIR}/share/trainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/share/config.json
        ${DLIB_SOURCES}
        ${ZLIB_SRC_FILES}
        ${LIBPNG_SRC_FILES}
        ${LIBJPEG_SRC_FILES}
        ${DLIB_ROOT}/dlib/image_loader/jpeg_loader.cpp
        ${DLIB_ROOT}/dlib/image_loader/png_loader.cpp
        ${DLIB_ROOT}/dlib/data_io/image_dataset_metadata.cpp
        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_extensions.cpp
        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_kernel_1.cpp
        ${DLIB_ROOT}/dlib/dir_nav/dir_nav_kernel_2.cpp
        ${DLIB_ROOT}/dlib/misc_api/misc_api_kernel_1.cpp
        ${DLIB_ROOT}/dlib/misc_api/misc_api_kernel_2.cpp
        ${DLIB_ROOT}/dlib/threads/thread_pool_extension.cpp
    )
    target_include_directories(trainer PUBLIC
        ${DLIB_EXTERNAL}
        ${DLIB_ROOT}/dlib/external/libpng
        ${DLIB_ROOT}/dlib/external/libjpeg
        ${DLIB_ROOT}/dlib/external/zlib
    )
    target_link_libraries(trainer)
    install(TARGETS trainer DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()