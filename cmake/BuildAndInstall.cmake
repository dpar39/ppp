function (CheckProcess RC)
    if(RC EQUAL "1")
        message( FATAL_ERROR "Command failed wiht exit code ${RC}")
    endif()
endfunction()

function (DownloadAndUnzip DOWNLOAD_URL DOWNLOAD_ARCHIVE EXTRACT_DIR)
    # Download archive if it doesn't exist
    if(NOT EXISTS "${DOWNLOAD_ARCHIVE}")
        message(STATUS "Downloading ${DOWNLOAD_URL} to ${DOWNLOAD_ARCHIVE}...")
        file(DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_ARCHIVE}" SHOW_PROGRESS)
    endif()

    # Extract archive
    if(NOT EXISTS "${EXTRACT_DIR}")
        message(STATUS "Extracting ${DOWNLOAD_ARCHIVE}...")
        get_filename_component(PARENT_DIR ${EXTRACT_DIR} DIRECTORY)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_ARCHIVE}"
            WORKING_DIRECTORY ${PARENT_DIR}
        )
    endif()
endfunction()

function (CleanUp FILE_OR_DIR)
    if (IS_DIRECTORY  ${FILE_OR_DIR})
        file(REMOVE_RECURSE ${FILE_OR_DIR})
    else()
        file(REMOVE ${FILE_OR_DIR})
    endif()
endfunction()

function (BuildAndInstall PKG_NAME DEFINES_COMMON DEFINES_WASM DEFINES_NATIVE DEFINES_WIN32 DEFINES_POSIX INSTALL_CHECKS SOURCE_DIR THIRD_PARTY_INSTALL_DIR)
    set(THIRD_PARTY_BUILD_DIR ${CMAKE_BINARY_DIR}/build_${PKG_NAME}_${BUILD_NAME_SUFFIX})
    ## Generic code
    set(DEFINES_FINAL "")
    foreach(X ${DEFINES_COMMON})
        list(APPEND DEFINES_FINAL "-D${X}")
    endforeach()
    if (EMSCRIPTEN)
        foreach(X ${DEFINES_WASM})
            list(APPEND DEFINES_FINAL "-D${X}")
        endforeach()
    else()
        foreach(X ${DEFINES_NATIVE})
            list(APPEND DEFINES_FINAL "-D${X}")
        endforeach()
        if (WIN32)
            foreach(X ${DEFINES_WIN32})
                list(APPEND DEFINES_FINAL "-D${X}")
            endforeach()
        endif()
    endif()

    foreach(X ${INSTALL_CHECKS})
        if (EXISTS "${THIRD_PARTY_INSTALL_DIR}/${X}")
            return()
        endif()
    endforeach()

    # Build package
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${SOURCE_DIR}
        -G ${CMAKE_GENERATOR}
        -B ${THIRD_PARTY_BUILD_DIR}
        "-DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
        "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
        "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        ${DEFINES_FINAL}
        COMMAND_ECHO STDOUT
        RESULT_VARIABLE RC
    )
    CheckProcess(${RC})

    execute_process(COMMAND ${CMAKE_COMMAND} --build ${THIRD_PARTY_BUILD_DIR} RESULT_VARIABLE RC)
    CheckProcess(${RC})

    execute_process(COMMAND ${CMAKE_COMMAND} --install ${THIRD_PARTY_BUILD_DIR} RESULT_VARIABLE RC)
    CheckProcess(${RC})

    CleanUp(${THIRD_PARTY_BUILD_DIR})
endfunction()