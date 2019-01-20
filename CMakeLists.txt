#========================================
# Passport Photo Print application (ppp-app)
#========================================
cmake_minimum_required(VERSION 3.0.0)

project(PassportPhoto CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(GMOCK_ROOT "${THIRD_PARTY_DIR}/googletest-release-1.8.1")

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${THIRD_PARTY_DIR}" CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 17)
# Set compilers flags globally
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -fexceptions -pthread")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC -fexceptions -pthread")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(GTEST_DISABLE_PTHREADS ON CACHE INTERNAL "" FORCE)
    foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${FLAG_VAR} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
        endif()
    endforeach()
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

# Add GMock/GTest as it is needed by all modules
include_directories(${GMOCK_ROOT}/googletest/include)
include_directories(${GMOCK_ROOT}/googlemock/include)
add_subdirectory(${GMOCK_ROOT})

# Add the ppp library
include_directories(libppp/include)
add_subdirectory(libppp)

# Add the webapp app
# include_directories(server/include)
# add_subdirectory(server)
