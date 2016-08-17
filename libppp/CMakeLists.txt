cmake_minimum_required(VERSION 2.8.8)

set(MODULE_NAME ppp)
project(${MODULE_NAME} CXX)
set(LIB_NAME lib${MODULE_NAME})
set(TEST_NAME ${MODULE_NAME}_test)

message(STATUS "-------- CMake for module ${MODULE_NAME} --------")

#----------------------------
# Third party dependencies
#----------------------------
# OpenCV
find_package(OpenCV REQUIRED)

# POCO
add_definitions(-DPOCO_STATIC -DPOCO_NO_AUTOMATIC_LIBS)
find_package(Poco REQUIRED COMPONENTS Foundation)

#-----------------------------
# Build the module library
#-----------------------------
file(GLOB LIB_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(MODULE_INC_DIRS
    ${CMAKE_SOURCE_DIR}/share/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIB_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${Poco_INCLUDE_DIRS}
)

set(MODULE_LIB_DEPS
    ${OpenCV_LIBS}
    ${Poco_LIBRARIES}
)

set(MODULE_LIB_DIRS
    ${OpenCV_LIB_DIR}
)
# Build a shared library version 
add_definitions(-DDLLEXPORT)
add_library(${LIB_NAME} SHARED ${LIB_SRC_FILES})
link_directories(${MODULE_LIB_DIRS})
target_include_directories(${LIB_NAME} PUBLIC ${MODULE_INC_DIRS})
target_link_libraries(${LIB_NAME} ${MODULE_LIB_DEPS})

# Install the library
install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libppp.h DESTINATION ${CMAKE_INSTALL_PREFIX})

#-----------------------------
# Build the module tests
#-----------------------------
file(GLOB TEST_SRC_FILES  "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
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
set(TEST_LIB_DIRS
    ${MODULE_INC_DIRS}
)

add_executable(${TEST_NAME} ${TEST_SRC_FILES} ${LIB_SRC_FILES})
target_include_directories(${TEST_NAME} PUBLIC ${TEST_INC_DIRS})
link_directories(${TEST_LIB_DIRS})
target_link_libraries(${TEST_NAME} ${TEST_LIB_DEPS})

#add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} --gtest_output=xml:${TEST_NAME}.xml)


