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

# POCO Library
if (UNIX)
    add_definitions(-DPOCO_STATIC)
    find_package(Poco REQUIRED COMPONENTS Foundation)
endif()

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB LIB_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set(MODULE_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${Poco_INCLUDE_DIRS}
)

set(MODULE_LIB_DEPS
    ${OpenCV_LIBS}
    ${Poco_LIBRARIES}
    ${OPENCV_3RDPARTY_LIBS}
)

set(MODULE_LIB_DIRS
    ${OpenCV_LIB_DIR}
)

if (WIN32 AND (${BUILD_TYPE} MATCHES "debug"))
    # Build a static library for Windows when debugging
    set(LIB_MODE STATIC)
else()
    # Build a shared version of the library
    # Avoid node-gyp mechanism to resolve third party libraries
    set(LIB_MODE SHARED)
    add_definitions(-DDLLEXPORT)
endif()
add_library(${LIB_NAME} ${LIB_MODE} ${LIB_SRC_FILES} ${LIB_INC_FILES})
link_directories(${MODULE_LIB_DIRS})
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})

# Install the library
install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/share/config.json DESTINATION ${CMAKE_INSTALL_PREFIX})

#-----------------------------
# Build the module tests
#-----------------------------
file(GLOB TEST_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
file(GLOB TEST_INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h")

set(TEST_INC_DIRS
    "tests/include"
    ${GMOCK_ROOT}/gtest/include
    ${GMOCK_ROOT}/include
    ${MODULE_INC_DIRS}
)
set(TEST_LIB_DEPS
    ${LIB_NAME}
    ${MODULE_LIB_DEPS}
    gmock
)

add_executable(${TEST_NAME} ${TEST_SRC_FILES} ${TEST_INC_FILES} ${LIB_SRC_FILES})
target_include_directories(${TEST_NAME} PUBLIC ${TEST_INC_DIRS})
target_link_libraries(${TEST_NAME} ${TEST_LIB_DEPS})

install(TARGETS ${TEST_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
