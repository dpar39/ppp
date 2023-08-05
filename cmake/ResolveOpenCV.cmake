function(ResolveOpenCV PKG_VERSION)
    include(Configuration)
    include(BuildAndInstall)

    set(PKG_NAME OpenCV)

    set(DOWNLOAD_URL "https://github.com/opencv/opencv_contrib/archive/${PKG_VERSION}.tar.gz")
    set(DOWNLOAD_ARCHIVE_CONTRIB "${THIRD_PARTY_DIR}/opencv_contrib-${PKG_VERSION}.tar.gz")
    set(EXTRACT_DIR_CONTRIB "${THIRD_PARTY_DIR}/opencv_contrib-${PKG_VERSION}")
    DownloadAndUnzip(${DOWNLOAD_URL} ${DOWNLOAD_ARCHIVE_CONTRIB} ${EXTRACT_DIR_CONTRIB})

    set(DOWNLOAD_URL "https://github.com/opencv/opencv/archive/${PKG_VERSION}.tar.gz")
    set(DOWNLOAD_ARCHIVE "${THIRD_PARTY_DIR}/opencv-${PKG_VERSION}.tar.gz")
    set(EXTRACT_DIR "${THIRD_PARTY_DIR}/opencv-${PKG_VERSION}")
    DownloadAndUnzip(${DOWNLOAD_URL} ${DOWNLOAD_ARCHIVE} ${EXTRACT_DIR})

    if (NOT WIN32)
        if(EMSCRIPTEN)
            set(SED_EXPRESSION "s/add_extra_compiler_option(-pthread)/#x#x#x#x#x#x#x#x#x#x/")
        else()
            set(SED_EXPRESSION "s/#x#x#x#x#x#x#x#x#x#x/add_extra_compiler_option(-pthread)/")
        endif()
        execute_process(
            COMMAND sed -i "${SED_EXPRESSION}" cmake/OpenCVCompilerOptions.cmake
            WORKING_DIRECTORY ${EXTRACT_DIR}
            COMMAND_ECHO STDOUT
        )
    endif()

    set(SOURCE_DIR ${EXTRACT_DIR})
    set(DEFINES_COMMON
        "BUILD_SHARED_LIBS=OFF"
        "BUILD_DOCS=OFF"
        "BUILD_PERF_TESTS=OFF"
        "BUILD_ILMIMF=ON"
        "BUILD_ZLIB=ON"
        "BUILD_JASPER=ON"
        "BUILD_PNG=ON"
        "BUILD_JPEG=ON"
        "BUILD_TIFF=OFF"
        "BUILD_opencv_apps=OFF"
        "BUILD_DOCS=OFF"
        "BUILD_TESTS=OFF"
        "WITH_PROTOBUF=OFF"
        "WITH_JASPER=ON"
        "WITH_PYTHON=OFF"
        "WITH_PYTHON2=OFF"
        "WITH_JAVA=OFF"
        "WITH_ADE=OFF"
        "WITH_FFMPEG=OFF"
        "WITH_MSMF=OFF"
        "WITH_VFW=OFF"
        "WITH_OPENEXR=OFF"
        "WITH_WEBP=OFF"
        "WITH_TIFF=OFF"
        "BUILD_opencv_java=OFF"
        "BUILD_opencv_python=OFF"
        "BUILD_opencv_python2=OFF"
        "BUILD_opencv_python3=OFF"
        "BUILD_opencv_python_bindings_generator=OFF"
        "OPENCV_EXTRA_MODULES_PATH=${EXTRACT_DIR_CONTRIB}/modules"
    )
    set(DEFINES_NATIVE
        "BUILD_TBB=ON"
        "BUILD_LIST=objdetect,imgproc,imgcodecs,ximgproc,video,videoio,features2d,calib3d,highgui"
    )
    set(DEFINES_WIN32
        "BUILD_WITH_STATIC_CRT=ON"
    )
    set(DEFINES_POSIX
        ""
    )
    set(DEFINES_WASM
        "CV_ENABLE_INTRINSICS=OFF"
        "ENABLE_PIC=FALSE"
        "BUILD_IPP_IW=OFF"
        "WITH_TBB=OFF"
        "WITH_OPENMP=OFF"
        "WITH_PTHREADS_PF=OFF"
        "WITH_OPENCL=OFF"
        "WITH_IPP=OFF"
        "WITH_ITT=OFF"
        "CPU_BASELINE="
        "CPU_DISPATCH="
        "BUILD_LIST=objdetect,imgproc,imgcodecs,ximgproc,video,videoio,features2d,calib3d,highgui"
    )
    set(INSTALL_CHECKS
        "lib/cmake/opencv4"
        "lib/opencv4"
        "OpenCVConfig.cmake"
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
    set(OpenCV_STATIC ON)
    set(OpenCV_DIR "${THIRD_PARTY_INSTALL_DIR}/lib/cmake/opencv4")
    list(APPEND CMAKE_PREFIX_PATH
        "${THIRD_PARTY_INSTALL_DIR}"
        "${THIRD_PARTY_INSTALL_DIR}/lib/cmake/opencv4"
    )
    find_package(${PKG_NAME} REQUIRED)

    CleanUp(${DOWNLOAD_ARCHIVE})
    CleanUp(${DOWNLOAD_ARCHIVE_CONTRIB})
    CleanUp(${EXTRACT_DIR})
    CleanUp(${EXTRACT_DIR_CONTRIB})
endfunction()
