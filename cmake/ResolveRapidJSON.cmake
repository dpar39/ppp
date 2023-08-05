function(ResolveRapidJSON PKG_VERSION)
    include(Configuration)
    include(BuildAndInstall)

    set(PKG_NAME RapidJSON)

    set(DOWNLOAD_URL "https://github.com/Tencent/rapidjson/archive/v${PKG_VERSION}.tar.gz")
    set(DOWNLOAD_ARCHIVE "${THIRD_PARTY_DIR}/rapidjson-${PKG_VERSION}.tar.gz")
    set(EXTRACT_DIR "${THIRD_PARTY_DIR}/rapidjson-${PKG_VERSION}")
    DownloadAndUnzip(${DOWNLOAD_URL} ${DOWNLOAD_ARCHIVE} ${EXTRACT_DIR})

    set(SOURCE_DIR ${EXTRACT_DIR})
    set(THIRD_PARTY_BUILD_DIR ${EXTRACT_DIR}/build_${BUILD_NAME_SUFFIX})

    set(DEFINES_COMMON
        "RAPIDJSON_BUILD_EXAMPLES=OFF"
        "RAPIDJSON_BUILD_TESTS=OFF"
        "RAPIDJSON_BUILD_DOC=OFF"
    )
    set(DEFINES_NATIVE "")
    set(DEFINES_WIN32 "")
    set(DEFINES_POSIX "")
    set(DEFINES_WASM "")
    set(INSTALL_CHECKS
        "cmake/RapidJSONConfig.cmake"
        "lib/cmake/RapidJSON/RapidJSONConfig.cmake"
    )

    BuildAndInstall(
        "${PKG_NAME}"
        "${DEFINES_COMMON}"
        "${DEFINES_WASM}"
        "${DEFINES_NATIVE}"
        "${DEFINES_WIN32}"
        "${DEFINES_POSIX}"
        "${INSTALL_CHECKS}"
        "${SOURCE_DIR}"
        "${THIRD_PARTY_INSTALL_DIR}"
    )

    set(${PKG_NAME}_DIR "${THIRD_PARTY_INSTALL_DIR}/lib/cmake/RapidJSON")
    find_package(${PKG_NAME} REQUIRED)

    CleanUp(${EXTRACT_DIR})
    CleanUp(${DOWNLOAD_ARCHIVE})
endfunction()
