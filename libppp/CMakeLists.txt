cmake_minimum_required(VERSION 3.15.0)

set(MODULE_NAME ppp)
project(${MODULE_NAME})
set(LIB_NAME  lib${MODULE_NAME})
set(APP_NAME  ${MODULE_NAME}_app)
set(TEST_NAME ${MODULE_NAME}_test)

message(STATUS "-------- Running CMake for module ${MODULE_NAME} --------")

if (CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(ARCH_NAME "x86")
else()
    set(ARCH_NAME "x64")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
set(BUILD_NAME_SUFFIX ${SYSTEM_NAME_LOWER}_${BUILD_TYPE_LOWER}_${ARCH_NAME} )

#----------------------------
# Third party dependencies
#----------------------------
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty)

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
        set(dlib_DIR ${THIRD_PARTY_DIR}/install_emscripten)
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
        set(dlib_DIR ${THIRD_PARTY_DIR}/install_${BUILD_NAME_SUFFIX})
    endif()

    if(EXISTS ${OpenCV_DIR}/share/OpenCV/OpenCVConfig.cmake)
        set(OpenCV_DIR ${OpenCV_DIR}/share/OpenCV)
    elseif(EXISTS ${OpenCV_DIR}/lib/cmake/opencv4/OpenCVConfig.cmake)
        set(OpenCV_DIR ${OpenCV_DIR}/lib/cmake/opencv4/)
    endif()

    if(EXISTS ${dlib_DIR}/lib/cmake/dlib/dlibConfig.cmake)
        set(dlib_DIR ${dlib_DIR}/lib/cmake/dlib)
    endif()

    message(STATUS "Setting OpenCV_DIR=${OpenCV_DIR}")
    message(STATUS "Setting dlib_DIR=${dlib_DIR}")
endif()


# Find OpenCV library
find_package(OpenCV REQUIRED)
# Find Dlib library
find_package(dlib REQUIRED)

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB LIB_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
# Add all source files needed for the library or wasm executable
set(LIB_SRC_FILES ${LIB_CPP_FILES} ${LIB_WRP_FILES})

set(MODULE_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${THIRD_PARTY_DIR}
)

set(MODULE_LIB_DEPS
    ${OpenCV_LIBS}
    dlib::dlib
    ${OPENCV_3RDPARTY_LIBS}
)

include_directories(${MODULE_INC_DIRS})
if (DEFINED EMSCRIPTEN)
    add_executable(${LIB_NAME} ${LIB_SRC_FILES} ${LIB_INC_FILES})
else()
    add_library(${LIB_NAME} ${LIB_SRC_FILES} ${LIB_INC_FILES})
endif()
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})


if (NOT ANDROID AND NOT IOS AND NOT DEFINED EMSCRIPTEN)
    # Install the library as we need everything in a single place for unit testing
    install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/config.bundle.json DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/python/libpppwrapper.py DESTINATION ${CMAKE_INSTALL_PREFIX})

    #----------------------------------------------
    # Build the module unit tests
    #----------------------------------------------
    set(GTest_DIR ${THIRD_PARTY_DIR}/install_${BUILD_NAME_SUFFIX})
    set(GTest_INCLUDE_DIRS ${GTest_DIR}/include)
    set(DEBUG_SUFFIX "")
    if(${BUILD_TYPE_LOWER} STREQUAL debug)
        set(DEBUG_SUFFIX "d")
    endif()
    if (MSVC)
        set(GTest_LIBRARIES ${GTest_DIR}/lib/gtest${DEBUG_SUFFIX}.lib ${GTest_DIR}/lib/gmock${DEBUG_SUFFIX}.lib)
    else()
        set(GTest_LIBRARIES ${GTest_DIR}/lib/libgtest${DEBUG_SUFFIX}.a ${GTest_DIR}/lib/libgmock${DEBUG_SUFFIX}.a)
    endif()

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

    #----------------------------------------------
    # Build a command line app
    #----------------------------------------------
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

    #----------------------------------------------
    # Build dlib's shape prediction model trainer
    #----------------------------------------------
    add_executable(trainer
        ${CMAKE_CURRENT_SOURCE_DIR}/share/trainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/share/config.json
    )
    target_link_libraries(trainer dlib::dlib)
    install(TARGETS trainer DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
message(STATUS "-------- Finished configuring CMake for module ${MODULE_NAME} --------")