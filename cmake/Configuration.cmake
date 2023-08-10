
if(EXISTS /thirdparty)
    set(THIRD_PARTY_DIR /thirdparty)
else()
    set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty)
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_CONFIG_LOWER)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
set(BUILD_NAME_SUFFIX ${SYSTEM_NAME_LOWER}_${BUILD_CONFIG_LOWER})
set(THIRD_PARTY_INSTALL_DIR ${THIRD_PARTY_DIR}/install_${BUILD_NAME_SUFFIX})

if(EMSCRIPTEN)
    message(STATUS "Compiling for Emscripten")
    #set(PRE_POST "--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/src/pre.js --post-js ${CMAKE_CURRENT_SOURCE_DIR}/src/post.js")
    set(PRE_POST "")
    set(EM_FLAGS "-std=c++17 -s DISABLE_EXCEPTION_CATCHING=0 -s USE_PTHREADS=0")
    set(CMAKE_EXE_LINKER_FLAGS "${PRE_POST} -s INITIAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 --bind -s FETCH=1" )
    if (${BUILD_CONFIG_LOWER} MATCHES "debug")
        message("Building in debug mode")
        set(EM_FLAGS "${EM_FLAGS} -g --source-map-base http://127.0.0.1:5500/example/")
    else()
        message("Building in release mode")
        set(EM_FLAGS "${EM_FLAGS} -O2")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_FLAGS}")
    foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${FLAG_VAR} MATCHES "-g")
            string(REGEX REPLACE "-g" "-g4" ${FLAG_VAR} "${${FLAG_VAR}}")
        endif()
    endforeach()
else()
    message(STATUS "Compiling native C++")
    if (MSVC)
        foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
            if(${FLAG_VAR} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
            endif()
        endforeach()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
        set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /MP /EHsc")
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_MACOSX_RPATH 1)
    endif()
endif()