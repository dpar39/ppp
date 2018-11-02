#========================================
# Passport Photo Print application (ppp-app)
#========================================
cmake_minimum_required(VERSION 2.8.12)

project(PassportPhoto CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${THIRD_PARTY_DIR_INSTALL}" CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 14)
# Set compilers flags globally
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -fexceptions -pthread")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC -fexceptions -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem \"${GMOCK_ROOT}/include\"" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem \"${GMOCK_ROOT}/gtest/include\"" CACHE STRING "" FORCE)
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(GTEST_DISABLE_PTHREADS ON CACHE INTERNAL "" FORCE)
    foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${FLAG_VAR} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
        endif()
    endforeach()
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

# Add GMock/GTest as it is needed by all modules
include_directories(${GMOCK_ROOT}/gtest/include)
include_directories(${GMOCK_ROOT}/include)

add_subdirectory(${GMOCK_ROOT})

# Add the ppp library
include_directories(libppp/include)
add_subdirectory(libppp)

# Add the webapp app
# include_directories(server/include)
# add_subdirectory(server)
