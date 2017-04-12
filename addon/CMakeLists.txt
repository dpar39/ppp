############################################################
# CMake is only used to build the Nodejs addon for Windows
############################################################
cmake_minimum_required(VERSION 2.8.12)

if(WIN32)
    set(MODULE_NAME addon)
    project(${MODULE_NAME} CXX)
    message(STATUS "-------- CMake for module ${MODULE_NAME} --------")

    set(NODE_ROOT "${THIRD_PARTY_DIR}/node-v6.10.2")
    if(${CMAKE_BUILD_TYPE} MATCHES debug)
        set(NODE_LIB_DIR "${NODE_ROOT}/Debug")
    else()
        set(NODE_LIB_DIR "${NODE_ROOT}/Release")
    endif()

    file(GLOB LIB_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.js")
    set(MODULE_INC_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${NODE_ROOT}/src"
        "${NODE_ROOT}/deps/v8/include"
        "${NODE_ROOT}/deps/uv/include"
    )
    set(MODULE_LIB_DEPS
        libppp
        "${NODE_LIB_DIR}/node.lib"
    )
    set(MODULE_LIB_DIRS
        ${OpenCV_LIB_DIR}
    )

    add_library(${MODULE_NAME} SHARED ${LIB_SRC_FILES})
    set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
    link_directories(${MODULE_LIB_DIRS})
    target_include_directories(${MODULE_NAME} PUBLIC ${MODULE_INC_DIRS})
    target_link_libraries(${MODULE_NAME} ${MODULE_LIB_DEPS})

    # Configure visual studio to debug the addon native code
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vs/addon.vcxproj.user" addon.vcxproj.user COPYONLY)

    # Copy test script to the installation directory
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/test.js"
            ${CMAKE_INSTALL_PREFIX})

    # Copy addon to the installation directory
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${MODULE_NAME}>
            ${CMAKE_INSTALL_PREFIX})
endif()