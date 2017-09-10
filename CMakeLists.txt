#========================================
# Passport Photo Print application (ppp-app)
#========================================
cmake_minimum_required(VERSION 2.8.12)

project(PassportPhoto CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(ARCH_NAME "x86")
else()
    set(ARCH_NAME "x64")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(THIRD_PARTY_DIR_INSTALL "${THIRD_PARTY_DIR}/install_${BUILD_TYPE}_${ARCH_NAME}")

set(POCO_ROOT  ${THIRD_PARTY_DIR_INSTALL} CACHE INTERNAL "" FORCE)
set(OPENCV_DIR ${THIRD_PARTY_DIR_INSTALL} CACHE INTERNAL "" FORCE)
set(DLIB_DIR "${THIRD_PARTY_DIR_INSTALL}/lib/cmake/dlib" CACHE INTERNAL "" FORCE)
set(GMOCK_ROOT "${THIRD_PARTY_DIR}/gmock-1.7.0" CACHE INTERNAL "" FORCE)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${THIRD_PARTY_DIR_INSTALL}" CACHE INTERNAL "" FORCE)

# Set compilers flags globally
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem \"${GMOCK_ROOT}/include\"" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem \"${GMOCK_ROOT}/gtest/include\"" CACHE STRING "" FORCE)
endif()
if (MSVC)
    # On windows, compile with CRT only in debug mode
    set(gtest_disable_pthreads ON CACHE INTERNAL "" FORCE)
    foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${FLAG_VAR} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
        endif()
    endforeach()
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

# Add GMock/GTest as it is needed by all modules
include_directories(${GMOCK_ROOT}/gtest/include)
include_directories(${GMOCK_ROOT}/include)
include_directories(${THIRD_PARTY_DIR})

add_subdirectory(${GMOCK_ROOT})

# Add the ppp library
include_directories(libppp/include)
add_subdirectory(libppp)

# Add the C++ wrapper for node.js
if (MSVC AND (${BUILD_TYPE} MATCHES "debug"))
    include_directories(addon/include)
    add_subdirectory(addon)
endif()