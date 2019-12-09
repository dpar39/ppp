#!/usr/bin/env python
"""
Builds this project from the ground up, run unit tests and deploys it to Azure or Heroku, well, not just yet :)
"""
import os
import re
import sys
import glob
import json
import base64
import shutil
import zipfile
import tarfile
import argparse
import threading
import subprocess
import multiprocessing

try:  # For Python 3.0 and later
    from urllib.request import urlopen
except ImportError:   # Fall back to Python 2's urllib2
    from urllib2 import urlopen

IS_WINDOWS = sys.platform == 'win32'
if sys.platform == 'win32':
    PLATFORM = 'windows'
elif 'linux' in sys.platform:
    PLATFORM = 'linux'
elif sys.platform == 'darwin':
    PLATFORM = 'darwin'

ANDROID_SDK_TOOLS = 'https://dl.google.com/android/repository/sdk-tools-{}-4333796.zip'.format(PLATFORM)
ANDROID_GRADLE = 'https://services.gradle.org/distributions/gradle-4.10.3-bin.zip'

#  swig -c++ -java -package swig -Ilibppp/include -outdir webapp/android/app/src/main/java/swig -module libppp -o libppp/swig/libppp_java_wrap.cxx libppp/swig/libppp.i


def which(program):
    """
    Returns the full path of to a program if available in the system PATH, None otherwise
    """
    def is_exe(fpath):
        """
        Returns true if the file can be executed, false otherwise
        """
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)
    fpath, _ = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file
    return None


def link_file(src_file_path, dst_link):
    if not os.path.exists(src_file_path):
        raise FileNotFoundError(src_file_path)
    print('Creating link for file "%s" in "%s"' % (src_file_path, dst_link))
    if IS_WINDOWS:
        shutil.copyfile(src_file_path, dst_link)
        return
        link_cmd = 'mklink "%s" "%s"' % (dst_link, src_file_path)
    else:
        link_cmd = 'ln -sf "%s" "%s"' % (src_file_path, dst_link)
    os.system(link_cmd)


def read_file(file_path, mode='r'):
    with open(file_path, mode) as fp:
        return fp.read()


def write_file(file_path, content, mode='w'):
    with open(file_path, mode) as fp:
        fp.write(content)


def read_json(file_path):
    with open(file_path) as fp:
        return json.load(fp)


def write_json(file_path, data):
    with open(file_path, 'w') as fp:
        json.dump(data, fp)


class ShellRunner(object):
    def __init__(self, arch_name=''):
        self._env = os.environ.copy()
        self._extra_paths = []
        self._arch_name = arch_name
        self._is_emscripten = arch_name == 'wasm' or arch_name == 'web'
        if arch_name in ['x64', 'x86']:
            if IS_WINDOWS:
                self._detect_vs_version()
            else:
                self.set_env_var('CC', 'clang')
                self.set_env_var('CXX', 'clang++')
                self.set_env_var('CXXFLAGS', '-fPIC')
                # self.set_env_var('LD_LIBRARY_PATH', self._install_dir)

        # Add tools like ninja and swig to the current PATH
        this_dir = os.path.dirname(os.path.realpath(__file__))
        tools_dir = os.path.join(this_dir, 'thirdparty', 'tools', PLATFORM)
        self.add_system_path(tools_dir)

    def add_system_path(self, new_path, at_end=True):
        curr_path_str = self._env['PATH']
        path_elmts = set(curr_path_str.split(os.pathsep))
        if new_path in path_elmts:
            return
        if at_end:
            self._env['PATH'] = curr_path_str + os.pathsep + new_path
        else:
            self._env['PATH'] = new_path + os.pathsep + curr_path_str

    def set_env_var(self, var_name, var_value):
        assert isinstance(var_name, str), 'var_name should be a string'
        assert isinstance(
            var_value, str) or var_value is None, 'var_value should be a string or None'
        self._env[var_name] = var_value

    def get_env_var(self, var_name):
        return self._env.get(var_name, '')

    def get_env(self):
        return self._env

    def run_cmd(self, cmd_args, cmd_print=True, cwd=None, input=None):
        """
        Runs a shell command
        """
        if isinstance(cmd_args, str):
            cmd_args = cmd_args.split()
        cmd_all = []
        if IS_WINDOWS and self._arch_name in ['x64', 'x86']:
            vs_arch = {'x64': 'x86_amd64', 'x86': 'x86'}
            cmd_all = [self._vcvarsbat, vs_arch[self._arch_name]] + '&& set CC=cl.exe && set CXX=cl.exe &&'.split(' ')
        cmd_all = cmd_all + cmd_args

        if cmd_print:
            print(' '.join(cmd_args))

        use_shell = os.name == 'nt'
        p = subprocess.Popen(cmd_all, env=self._env, cwd=cwd, shell=use_shell,
                             stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
        if input:
            p.communicate(input=input)
        else:
            p.wait()
        if p.returncode != 0:
            print('Command "%s" exited with code %d' % (' '.join(cmd_args), p.returncode))
            sys.exit(p.returncode)

    def _detect_vs_version(self):
        """
        Detects the first available version of Visual Studio
        """
        vc_releases = [
            ('Visual Studio 15 2017',
             r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat'),
            ('Visual Studio 15 2017',
             r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat'),
            ('Visual Studio 15 2017',
             r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat'),
            ('Visual Studio 14 2015', r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat')]
        for (vsgenerator, vcvarsbat) in vc_releases:
            if os.path.exists(vcvarsbat):
                self._vcvarsbat = vcvarsbat
                self._vc_cmake_gen = vsgenerator
                if "64" in self._arch_name:
                    self._vc_cmake_gen += ' Win64'
                break

    def get_vc_cmake_generator(self):
        return self._vc_cmake_gen


class Builder(object):
    """
    Class that holds the whole building process
    """

    def repo_path(self, rel_path=''):
        if not rel_path:
            return self._root_dir
        return os.path.join(self._root_dir, rel_path).replace('\\', '/')

    def webapp_path(self, rel_path=''):
        """
        Returns the absolute path of the webapp directory (depending on whether we using NodeJs or .Net Core server)
        """
        if not rel_path:
            return os.path.join(self._root_dir, 'webapp')
        return os.path.join(self._root_dir, 'webapp', rel_path)

    def build_name(self):
        if self._arch_name == 'wasm':
            return 'emscripten'
        return PLATFORM + '_' + self._build_config + '_' + self._arch_name

    def build_dir_name(self, prefix):
        """
        Returns a name for a build directory based on the build configuration
        """
        return os.path.join(prefix, 'build_' + self.build_name())

    def build_path(self, rel_path):
        return os.path.join(self._build_dir, rel_path).replace('\\', '/')

    def run_cmd(self, cmd_args, cmd_print=True, cwd=None, input=None):
        self._shell.run_cmd(cmd_args, cmd_print=cmd_print,
                            cwd=cwd, input=input)

    def set_startup_vs_prj(self, project_name):
        """
        Rearranges the projects so that the specified project is the first
        therefore is the startup project within Visual Studio
        """
        solution_file = glob.glob(self._build_dir + '/*.sln')[0]
        sln_lines = read_file(solution_file).splitlines()
        lnum = 0
        lin_prj_beg = 0
        lin_prj_end = 0
        for line in sln_lines:
            if project_name in line:
                lin_prj_beg = lnum
            if lin_prj_beg > 0 and line.endswith('EndProject'):
                lin_prj_end = lnum
                break
            lnum = lnum + 1
        prj_lines = sln_lines[:2] + sln_lines[lin_prj_beg:lin_prj_end + 1] \
            + sln_lines[2:lin_prj_beg] + sln_lines[lin_prj_end + 1:]
        write_file(solution_file, '\n'.join(item for item in prj_lines))

    def build_googletest(self):
        """
        Extract and build GMock/GTest libraries
        """
        if self._arch_name == 'wasm':
            return  # We don't run WebAssembly unit tests

        gtest_extract_dir = self.get_third_party_lib_dir('googletest')
        gtest_installed_dir = os.path.join(self._third_party_install_dir, 'lib/cmake/GTest')

        self.clean_thirdparty_if_needed('gtest', gtest_installed_dir, gtest_extract_dir)

        if os.path.isdir(gtest_installed_dir):
            return  # We have Gtest installed

        # Download googletest sources if not done yet
        gmock_src_pkg = self.download_third_party_lib(self.gmock_src_url, 'googletest.zip')
        # Get the file prefix for googletest
        gtest_extract_dir = self.get_third_party_lib_dir('googletest')
        if gtest_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(gmock_src_pkg)
            gtest_extract_dir = self.get_third_party_lib_dir('googletest')
        self.build_cmake_lib(gtest_extract_dir, [], ['install'], True)  # '-DGTEST_FORCE_SHARED_CRT=ON'

    def get_third_party_lib_dir(self, prefix):
        """
        Get the directory where a third party library with the specified prefix
        name was extracted, if any
        """
        third_party_dirs = next(os.walk(self._third_party_dir))[1]
        candidate = None
        for lib_dir in sorted(third_party_dirs):
            if prefix in lib_dir:
                candidate = os.path.join(self._third_party_dir, lib_dir)
        return candidate

    def clean_thirdparty_if_needed(self, libname, installed_dir, extract_dir):
        if libname in self._clean_targets:
            if os.path.isdir(installed_dir):
                shutil.rmtree(installed_dir)
            if extract_dir and os.path.isdir(extract_dir):
                shutil.rmtree(extract_dir)

    def build_opencv(self):
        """
        Downloads and builds OpenCV from source
        """
        opencv_extract_dir = self.get_third_party_lib_dir('opencv')

        opencv_installed_dir = os.path.join(self._third_party_install_dir, 'lib/cmake/opencv4')
        self.clean_thirdparty_if_needed('opencv', opencv_installed_dir, opencv_extract_dir)
        # HACK: For Windows the install directory is not quite the same thing
        cmake_ref = os.path.join(self._third_party_install_dir, 'OpenCVConfig.cmake')
        if os.path.isdir(opencv_installed_dir) or os.path.isfile(cmake_ref):
            return  # We have OpenCV installed

        # Download OpenCV sources if not done yet
        opencv_src_pkg = self.download_third_party_lib(self.opencv_src_url)
        # Get the extract directory for OpenCV
        dir_name = 'opencv-' + self.get_third_party_lib_version(self.opencv_src_url)
        opencv_extract_dir = self.get_third_party_lib_dir(dir_name)

        if opencv_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(opencv_src_pkg)
            opencv_extract_dir = self.get_third_party_lib_dir(dir_name)

        cmake_extra_defs = [
            '-DBUILD_SHARED_LIBS=OFF',
            '-DBUILD_DOCS=OFF',
            '-DBUILD_PERF_TESTS=OFF',
            '-DBUILD_ILMIMF=ON',
            '-DBUILD_ZLIB=ON',
            '-DBUILD_JASPER=ON',
            '-DBUILD_PNG=ON',
            '-DBUILD_JPEG=ON',
            '-DBUILD_TIFF=OFF',
            '-DBUILD_opencv_apps=OFF',
            '-DBUILD_DOCS=OFF',
            '-DBUILD_TESTS=OFF',
            '-DWITH_JASPER=ON',
            '-DWITH_PYTHON=OFF',
            '-DWITH_PYTHON2=OFF',
            '-DWITH_JAVA=OFF',
            '-DWITH_FFMPEG=OFF',
            '-DWITH_MSMF=OFF',
            '-DWITH_VFW=OFF',
            '-DWITH_OPENEXR=OFF',
            '-DWITH_WEBP=OFF',
            '-DWITH_TIFF=OFF',
            '-DBUILD_opencv_java=OFF',
            '-DBUILD_opencv_python=OFF',
            '-DBUILD_opencv_python2=OFF',
            '-DBUILD_opencv_python3=OFF'
            '-DBUILD_opencv_python_bindings_generator=OFF'
        ]

        if self._arch_name == 'wasm':
            cmake_extra_defs += [
                '-DCV_ENABLE_INTRINSICS=OFF',
                '-DENABLE_PIC=FALSE'
                '-DBUILD_IPP_IW=OFF',
                '-DWITH_TBB=OFF',
                '-DWITH_OPENMP=OFF',
                '-DWITH_PTHREADS_PF=OFF',
                '-DWITH_OPENCL=OFF',
                '-DWITH_IPP=OFF',
                '-DWITH_ITT=OFF',
                "-DCPU_BASELINE=''",
                "-DCPU_DISPATCH=''",
                '-DBUILD_WITH_DEBUG_INFO=OFF',
                '-DBUILD_LIST=objdetect,imgproc,imgcodecs,calib3d',
            ]
        else:
            cmake_extra_defs += [
                '-DBUILD_TBB=ON',
                '-DBUILD_LIST=objdetect,imgproc,imgcodecs,highgui'
            ]
            if IS_WINDOWS:
                cmake_extra_defs += ['-DBUILD_WITH_STATIC_CRT=ON']

        # Clean and create the build directory
        build_dir = self.build_dir_name(opencv_extract_dir)
        if os.path.exists(build_dir):  # Remove the build directory
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir):  # Create the build directory
            os.mkdir(build_dir)
        self.build_cmake_lib(opencv_extract_dir, cmake_extra_defs, ['install'], True)

    def build_dlib(self):
        # Get the file prefix for dlib
        dlib_version = self.get_third_party_lib_version(self.dlib_src_url)
        dlib_folder = 'dlib-' + dlib_version
        dlib_extract_dir = self.get_third_party_lib_dir(dlib_folder)
        dlib_installed_dir = os.path.join(self._third_party_install_dir, 'lib/cmake/dlib')

        # Clean the library build and install if needed
        self.clean_thirdparty_if_needed('dlib', dlib_installed_dir, dlib_extract_dir)

        # Skip building dlib if it is already installed
        if os.path.exists(dlib_installed_dir):
            return

        # Download dlib sources if not done yet
        dlib_src_pkg = self.download_third_party_lib(self.dlib_src_url)

        if dlib_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(dlib_src_pkg)
            dlib_extract_dir = self.get_third_party_lib_dir(dlib_folder)

        cmake_extra_defs = [
            '-DDLIB_JPEG_SUPPORT=OFF',
            '-DDLIB_PNG_SUPPORT=OFF',
            '-DDLIB_NO_GUI_SUPPORT=ON'
        ]

        # Hacks to compile dlib
        dlib_cmakelists_path = os.path.join(dlib_extract_dir, 'dlib/CMakeLists.txt')
        if IS_WINDOWS:
            cmake_extra_defs.append('-DDLIB_FORCE_MSVC_STATIC_RUNTIME=ON')
            # Hack cmakelists.txt to get option -DDLIB_FORCE_MSVC_STATIC_RUNTIME to work
            hack_line = 'include(cmake_utils/tell_visual_studio_to_use_static_runtime.cmake)'
            content = read_file(dlib_cmakelists_path)
            if hack_line not in content:
                content = hack_line + '\n' + content
            write_file(dlib_cmakelists_path, content)
        else:
            content = read_file(dlib_cmakelists_path)
            content = content.replace('cmake_minimum_required(VERSION 2.8.12)',
                                      'cmake_minimum_required(VERSION 3.1.0)')
            write_file(dlib_cmakelists_path, content)

        build_dir = self.build_dir_name(dlib_extract_dir)
        if os.path.exists(build_dir):  # Remove the build directory
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir):  # Create the build directory
            os.mkdir(build_dir)
        self.build_cmake_lib(dlib_extract_dir, cmake_extra_defs, ['install'], True)

    def get_filename_from_url(self, url):
        """
        Extracts the file name from a given URL
        """
        lib_filename = url.split('/')[-1].split('#')[0].split('?')[0]
        lib_filepath = os.path.join(self._third_party_dir, lib_filename)
        return lib_filepath

    def download_third_party_lib(self, url, package_name=None):
        """
        Download a third party dependency from the internet if is not available offline
        """
        if not package_name:
            lib_filepath = self.get_filename_from_url(url)
        else:
            lib_filepath = os.path.join(self._third_party_dir, package_name)
        if not os.path.exists(lib_filepath):
            print('Downloading %s to "%s" please wait ...' % (url, lib_filepath))
            lib_file = urlopen(url)
            write_file(lib_filepath, lib_file.read(), 'wb')
        return lib_filepath

    def get_third_party_lib_version(self, url):
        a = re.compile(r'\d+\.\d+(:?\.\d+)?')
        m = a.search(url)
        return m.group(0) if m else None

    def extract_third_party_lib(self, lib_src_pkg, extract_dir=None):
        """
        Extracts a third party lib package source file into a directory
        """
        if not extract_dir:
            extract_dir = self._third_party_dir
        print('Extracting third party library "%s" into "%s" ... please wait ...' % (lib_src_pkg, extract_dir))
        if 'zip' in lib_src_pkg:
            zip_handle = zipfile.ZipFile(lib_src_pkg)
            for item in zip_handle.namelist():
                zip_handle.extract(item, extract_dir)
            zip_handle.close()
        else:  # Assume tar archive (tgz, tar.bz2, tar.gz)
            tar = tarfile.open(lib_src_pkg, 'r')
            for item in tar:
                tar.extract(item, self._third_party_dir)
            tar.close()

    def build_cmake_lib(self, cmakelists_path, extra_definitions, targets, is_3rd_party, clean_build=False):
        """
        Builds a library using cmake
        """
        build_dir = self.build_dir_name(cmakelists_path)
        # Clean and create the build directory
        # Remove the build directory
        if clean_build and os.path.exists(build_dir):
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir):  # Create the build directory
            os.mkdir(build_dir)

        if 'install' in targets:
            install_dir = self._third_party_install_dir if is_3rd_party else self._install_dir
            extra_definitions.append('-DCMAKE_INSTALL_PREFIX=' + install_dir)

        if self._arch_name == 'wasm':
            emscripten_path = os.path.join(self._third_party_dir, 'emsdk/' +
                                           self._emsdk_backend + '/emscripten')
            if not os.path.isdir(emscripten_path):
                print(emscripten_path + ' does NOT exist, exiting ...')
                exit(1)
            cmake_module_path = os.path.join(emscripten_path, 'cmake')
            cmake_toolchain = os.path.join(cmake_module_path, 'Modules', 'Platform', 'Emscripten.cmake')

            s_conf = self.get_emscripten_config(True)
            cxx_flags = s_conf + ' -std=c++1z -O2 --llvm-lto 1'  # -msimd128'
            extra_definitions += [
                '-DEMSCRIPTEN=1',
                '-DCMAKE_TOOLCHAIN_FILE=' + cmake_toolchain.replace('\\', '/'),
                '-DCMAKE_MAKE_PROGRAM=ninja',
                '-DCMAKE_MODULE_PATH=' + cmake_module_path.replace('\\', '/'),
                '-DCMAKE_CXX_FLAGS=' + cxx_flags + '',
                '-DCMAKE_EXE_LINKER_FLAGS=' + cxx_flags + ''
            ]
        elif not IS_WINDOWS:
            extra_definitions += [
            ]

        if self._arch_name != 'wasm':
            c_compiler = self._shell.get_env_var('CC')
            if c_compiler:
                extra_definitions.append('-DCMAKE_C_COMPILER=' + c_compiler)
            cxx_compiler = self._shell.get_env_var('CXX')
            if cxx_compiler:
                extra_definitions.append('-DCMAKE_CXX_COMPILER=' + cxx_compiler)

        # Define CMake generator and make command
        os.chdir(build_dir)
        cmake_cmd = ['cmake', '-G', 'Ninja',
                     '-DCMAKE_BUILD_TYPE=' + self._build_config] + extra_definitions + [cmakelists_path.replace('\\', '/')]

        # Run CMake and Make
        self.run_cmd(cmake_cmd)
        self.run_cmd('ninja')
        for target in targets:
            self.run_cmd(['ninja', target])
        os.chdir(self._root_dir)

    def setup_android(self):
        # Download SDK tools if not present and extract it to
        android_sdk_tools_pkg = self.download_third_party_lib(ANDROID_SDK_TOOLS)

        # Extract the SDK if not done already
        android_sdk_tools_dirname = os.path.splitext(os.path.basename(android_sdk_tools_pkg))[0]
        android_sdk_tools_dir = self.get_third_party_lib_dir(android_sdk_tools_dirname)
        if android_sdk_tools_dir is None:
            android_sdk_tools_dir = os.path.join(self._third_party_dir, android_sdk_tools_dirname)
            self.extract_third_party_lib(android_sdk_tools_pkg, android_sdk_tools_dir)

        # Download gradle
        gradle_pkg = self.download_third_party_lib(ANDROID_GRADLE)
        gradle_pkg_dir = self.get_third_party_lib_dir('gradle')
        if gradle_pkg_dir is None:
            self.extract_third_party_lib(gradle_pkg)
            gradle_pkg_dir = self.get_third_party_lib_dir('gradle')

        # # Download Android NDK if not present
        # android_ndk_pkg = self.download_third_party_lib(ANDROID_NDK)

        # # Extract the NDK if not done already
        # android_ndk_dir = self.get_third_party_lib_dir('android-ndk')
        # if android_ndk_dir is None:
        #     self.extract_third_party_lib(android_ndk_pkg)

        # Set up environment
        android_user_dir = os.path.join(os.path.expanduser("~"), '.android')
        if not os.path.exists(android_user_dir):
            os.mkdir(android_user_dir)
        repos_cfg = os.path.join(android_user_dir, 'repositories.cfg')
        if not os.path.exists(repos_cfg):
            write_file(repos_cfg, '')

        # self._shell.set_env_var('JAVA_HOME', '/usr/lib/jvm/java-8-oracle')
        self._shell.set_env_var('ANDROID_HOME', android_sdk_tools_dir)
        self._shell.set_env_var('ANDROID_SDK_ROOT', android_sdk_tools_dir)
        # self._shell.set_env_var('ANDROID_NDK_HOME', android_ndk_dir)
        # self._shell.set_env_var('JAVA_OPTS', '-XX:+IgnoreUnrecognizedVMOptions --add-modules java.se.ee')
        # self._shell.set_env_var('JAVA_OPTS', '')

        bin_tools = os.path.normpath(os.path.join(android_sdk_tools_dir, 'tools/bin'))
        self._shell.add_system_path(bin_tools)
        self._shell.add_system_path(os.path.normpath(os.path.join(android_sdk_tools_dir, 'tools')))
        self._shell.add_system_path(os.path.normpath(os.path.join(gradle_pkg_dir, 'bin')))
        # self._shell.add_system_path(os.path.normpath(android_ndk_dir))
        self._shell.add_system_path(os.path.normpath(
            os.path.join(self._shell.get_env_var('JAVA_HOME'), '/jre/bin')))

        # print(self._shell._env)
        if os.name == 'posix':
            self.run_cmd('chmod -R +x {}'.format(bin_tools))
            # self.run_cmd('chmod -R +x {}'.format(android_ndk_dir))
            self.run_cmd('chmod -R +x {}/bin'.format(gradle_pkg_dir))
        self.run_cmd('yes | sdkmanager --licenses')
        # self.run_cmd('sdkmanager "platform-tools" "platforms;android-25"', input='y')

    def get_emscripten_config(self, as_compiler_args):
        config = {
            'ASSERTIONS': 2,
            'ALLOW_MEMORY_GROWTH': 1,
            'DISABLE_EXCEPTION_CATCHING': 0,
            'TOTAL_MEMORY': 268435456,  # 268MB is too much?
            'WASM': 1,
            'FETCH': 1
        }

        if as_compiler_args:
            return ' '.join('-s ' + k + '=' + str(config[k]) for k in config)
        else:
            return config

    def setup_emscripten(self):
        if which('emsdk'):
            return  # we already have emscripten in path
        emsdk_dir = os.path.join(self._third_party_dir, 'emsdk')
        emsdk_cmd = 'emsdk.bat' if IS_WINDOWS else './emsdk'

        emsdk_version_number = str(self.emsdk_version_number)
        emsdk_version_name = emsdk_version_number
        # emsdk_version_name = 'sdk-' + emsdk_version_number + '-64bit'
        # if 'upstream' in self._emsdk_backend:
        #     emsdk_version_name += '-' + self._emsdk_backend
        emsdk_backend_dir = os.path.join(emsdk_dir, self._emsdk_backend)
        if not os.path.exists(emsdk_dir):
            os.chdir(self._third_party_dir)
            self.run_cmd('git clone https://github.com/emscripten-core/emsdk.git emsdk')
        if not os.path.exists(emsdk_backend_dir):
            os.chdir(emsdk_dir)
            self.run_cmd(emsdk_cmd + ' install ' + emsdk_version_name)
        os.chdir(emsdk_dir)
        self.run_cmd(emsdk_cmd + ' activate ' + emsdk_version_name)
        process = subprocess.Popen(['python', 'emsdk.py', 'construct_env'], stdout=subprocess.PIPE)
        (output, _) = process.communicate()
        exit_code = process.wait()
        if exit_code != 0:
            exit(exit_code)
        path_re = re.compile(r'PATH \+= (.*)')
        envvar_re = re.compile(r'([A-Za-z_]+) = (.*)')
        if not isinstance(output, str):
            output = output.decode("utf-8")
        for line in output.splitlines():
            m = path_re.search(line)
            if m:
                path = m.group(1)
                path = path.replace('\\', '/')
                self._shell.add_system_path(path, at_end=True)
                continue
            m = envvar_re.search(line)
            if m:
                var_name = m.group(1)
                var_value = m.group(2).replace('\\', '/')
                self._shell.set_env_var(var_name, var_value)
                continue
        os.chdir(self._root_dir)
        self._shell.set_env_var('CC', 'emcc')
        self._shell.set_env_var('CXX', 'em++')

        # Configure settings.js as for some reason flags passed in CMAKE_CXX_FLAGS do not get really used
        config = self.get_emscripten_config(False)

        settings_file = None
        for p in ['fastcomp/emscripten', 'upstream/emscripten']:
            candidate = os.path.join(emsdk_dir, p, 'src', 'settings.js')
            if os.path.isfile(candidate):
                settings_file = candidate
                break

        if not settings_file:
            self.run_cmd(['tree', emsdk_dir])
            raise Exception('Unable to find emscripten\'s settings file')

        content = read_file(settings_file)
        new_content = content
        for name in config:
            new_content = re.sub(r'(var ' + name + r'\s?=\s?)([A-z0-9\[\]"]+);',
                                 r'\g<1>' + str(config[name]) + r';', new_content)
        if new_content != content:
            write_file(settings_file, new_content)

    def extract_validation_data(self):
        """
        Extracts validation imageset with annotations from a password protected zip file
        These images were requested at http://www.scface.org/ and are copyrighted,
        so please do not share them without obtaining written consent
        """
        research_dir = os.path.join(self._root_dir, 'research')

        def extract(research_dir, zip_file):
            """
            Extracts file from zip archive
            """
            print('Extracting "%s", please wait ...' %
                  (os.path.basename(zip_file)))
            os.chdir(research_dir)
            if IS_WINDOWS:
                self.run_cmd(['7za.exe', 'x', zip_file, '*',
                              '-pmugshot_frontal_original_all.zip', '-y'], False)
            else:
                self.run_cmd(['unzip', '-qq', '-o', '-P',
                              'mugshot_frontal_original_all.zip', zip_file], False)
        data_dir = os.path.join(research_dir, 'mugshot_frontal_original_all')
        if not os.path.exists(data_dir):
            os.mkdir(data_dir)
        if os.path.exists(os.path.join(data_dir, '130_frontal.jpg')):
            return  # Nothing to do, data already been extracted

        print('Extracting validation data ...')
        extract(research_dir, 'mugshot_frontal_original_all_1.zip')
        extract(research_dir, 'mugshot_frontal_original_all_2.zip')
        extract(research_dir, 'mugshot_frontal_original_all_3.zip')
        print('Extracting validation data completed!')

    def bundle_config(self):
        """
        Bundles all configuration files into a config.bundle.json encoding referred files as Base64
        """
        lippp_share_dir = os.path.join(self._root_dir, 'libppp/share')

        def expand_node(node):
            if not isinstance(node, dict):
                return
            for key in node:
                embed = node.get('embed', False)
                if key == 'file' and not node.get('data', '') and embed:
                    file_name = node['file']
                    file_path = os.path.join(lippp_share_dir, file_name)
                    content = ''
                    if embed == 'base64':
                        data = read_file(file_path, 'rb')
                        content = base64.b64encode(data).decode('ascii')
                    elif embed == 'text':
                        content = read_file(file_path)
                    node['data'] = content
                else:
                    expand_node(node[key])

        config_input_file = os.path.join(lippp_share_dir, 'config.json')
        config_data = read_json(config_input_file)

        for key in config_data:
            expand_node(config_data[key])

        config_bundle_file = os.path.join(lippp_share_dir, 'config.bundle.json')
        write_json(config_bundle_file, config_data)

    def build_cpp_code(self):
        """
        Builds the C++ libppp project from sources
        """
        # self.run_cmd(
        #     'swig -c++ -python -Ilibppp/include -outdir libppp/python -o libppp/swig/libppp_python_wrap.cxx libppp/swig/libppp.i')

        # Clean build and install directory first if needed
        if 'ppp' in self._clean_targets:
            for path in [self._build_dir, self._install_dir]:
                if os.path.isdir(path):
                    shutil.rmtree(self._build_dir)

        if not os.path.exists(self._build_dir):
            # Create the build directory if doesn't exist
            os.mkdir(self._build_dir)

        # Change directory to build directory
        os.chdir(self._build_dir)
        if self._gen_vs_sln:
            # Generating visual studio solution
            cmake_generator = self._shell.get_vc_cmake_generator()
            cmake_args = ['cmake',
                          '-DCMAKE_INSTALL_PREFIX=' + self._install_dir,
                          '-DCMAKE_PREFIX_PATH=' + self._install_dir,
                          '-DCMAKE_BUILD_TYPE=' + self._build_config,
                          '-G', cmake_generator, '..']
            self.run_cmd(cmake_args)
            self.set_startup_vs_prj('ppp_test')
        else:
            targets = ['install'] if self._arch_name != 'wasm' else []
            self.build_cmake_lib('..', [], targets, False)
            # Run unit tests for C++ code
            if self._arch_name in ['x64', 'x86'] and self._run_tests:
                os.chdir(self._install_dir)
                test_exe = r'.\ppp_test.exe' if IS_WINDOWS else './ppp_test'
                self.run_cmd([test_exe, '--gtest_output=xml:../tests.xml'])

        os.chdir(self._root_dir)
        if self._arch_name == 'wasm':
            shutil.copytree(self.repo_path('libppp/share'),
                            self.repo_path('webapp/src/assets'))
            shutil.copyfile(self.repo_path('libppp/share/config.bundle.json'),
                            self.repo_path('webapp/src/assets/config.bundle.json'))
            shutil.copyfile(self.build_path('libppp/libppp.js'),
                            self.repo_path('webapp/src/assets/libppp.js'))
            shutil.copyfile(self.build_path('libppp/libppp.wasm'),
                            self.repo_path('webapp/src/assets/libppp.wasm'))

    def build_android(self):
        """
        Builds android app
        """
        self.run_cmd('npx cap copy', cwd='webapp')
        self.run_cmd('gradle build --stacktrace', cwd='webapp/android')

    def build_webapp(self):
        """
        Builds and test the web application by running shell commands
        """
        #  Copy libppp configurations and build files to the webapp directory
        # dist_files = ['libppp/share/config.bundle.json',
        #               'libppp/python/libpppwrapper.py']
        # for dist_file in dist_files:
        #     src_file_path = self.repo_path(dist_file)
        #     dst_link = self.webapp_path(os.path.basename(dist_file))
        #     if os.path.exists(src_file_path):
        #         link_file(src_file_path, dst_link)

        # Build the web app
        os.chdir(self.webapp_path())
        self.run_cmd('npm run gen-app-info')
        self.run_cmd('npm run gen-pwa-icons')
        if self._run_tests:
            self.run_cmd('npx ng test --browsers=ChromeHeadless --watch=false')
        self.run_cmd('npx ionic build --prod')
        os.chdir(self._root_dir)

    def setup_webapp(self):
        if not which('npx'):
            self.run_cmd('npm install npx -g')

        # Build the web app
        if not self._no_npm:
            os.chdir(self.webapp_path())
            self.run_cmd('npm install --no-optional')
            os.chdir(self._root_dir)

    def load_third_party_config(self):
        # Configuration
        this_dir = os.path.dirname(os.path.realpath(__file__))
        third_party_config = os.path.join(this_dir, 'thirdparty/thirdparty.json')
        config = read_json(third_party_config)

        self.emsdk_version_number = config.get('EMSDK_VERSION_NUMBER', '1.38.46')
        self.emsdk_version_name = 'sdk-' + self.emsdk_version_number + '-64bit'
        self.opencv_src_url = config.get('OPENCV_SRC_URL')
        self.dlib_src_url = config.get('DLIB_SRC_URL')
        self.gmock_src_url = config.get('GMOCK_SRC_URL')

    def parse_arguments(self):
        """
        Parses command line arguments
        """
        parser = argparse.ArgumentParser(description='Builds this application and it dependencies')
        parser.add_argument('--arch_names', '-a', required=False, choices=['x64', 'wasm', 'web', 'android', 'all'],
                            help='Platform architectures to build', default=['x64'], nargs='+')
        parser.add_argument('--build_config', '-b', required=False, choices=['debug', 'release'],
                            help='Build configuration type', default='release')
        parser.add_argument('--clean_targets', '-c', nargs='+', help='Clean specified targets', default=[])
        parser.add_argument('--test', '-t', help='Runs available unit tests', action="store_true")
        parser.add_argument('--skip_install', help='Skips installation', action="store_true")
        parser.add_argument('--gen_vs_sln', help='Generates Visual Studio solution and projects',
                            action="store_true")
        parser.add_argument('--no_npm', help='Skips installing npm packages. Use only on developer workflow',
                            action="store_true")

        args = parser.parse_args()

        arch_names = args.arch_names
        if 'all' in arch_names:
            arch_names == ['x64', 'wasm', 'web']  # Add android

        build_order = {'x64': 1, 'x86': 1, 'wasm': 1, 'web': 2, 'android': 3}
        self._arch_names = sorted(arch_names, key=lambda a: build_order[a])
        self._clean_targets = args.clean_targets
        self._build_config = args.build_config
        self._gen_vs_sln = args.gen_vs_sln
        self._run_tests = args.test
        self._run_install = not args.skip_install
        self._no_npm = args.no_npm
        self._emsdk_backend = 'upstream' if 'upstream' in self.emsdk_version_number else 'fastcomp'

    def clean_all_if_needed(self):
        if 'all' in self._clean_targets:
            if os.path.isdir(self._third_party_install_dir):
                shutil.rmtree(self._third_party_install_dir)
            if os.path.isdir(self._build_dir):
                shutil.rmtree(self._build_dir)

    def __init__(self):
        self._root_dir = os.path.dirname(os.path.realpath(__file__))

        # Load 3rd party config
        self.load_third_party_config()

        # Detect OS version
        self.parse_arguments()

        # Extract testing dataset
        self._shell = ShellRunner()
        self.extract_validation_data()
        #self.bundle_config()

        for arch in self._arch_names:
            self._arch_name = arch

            # directory suffix for the build and release
            self._build_dir = os.path.join(self._root_dir, 'build_' + self.build_name())
            self._install_dir = os.path.join(self._root_dir, 'install_' + self.build_name())
            self._third_party_dir = os.path.join(self._root_dir, 'thirdparty')
            self._third_party_install_dir = os.path.join(
                self._third_party_dir, 'install_' + self.build_name()).replace('\\', '/')

            shell = ShellRunner(arch)
            shell.set_env_var('INSTALL_DIR', self._install_dir)
            self._shell = shell

            # Clean all if requested
            self.clean_all_if_needed()

            # Setup Emscripten tools
            if arch == 'wasm':
                self.setup_emscripten()

            # Create install directory if it doesn't exist
            if not os.path.exists(self._install_dir):
                os.mkdir(self._install_dir)

            # Setup tools and build web app
            if arch == 'web':
                self.setup_webapp()
                self.build_webapp()

            if arch in ['x64', 'x86', 'wasm']:
                # Build Third Party Libs
                self.build_dlib()
                self.build_googletest()
                self.build_opencv()
                # Build libppp
                self.build_cpp_code()

            if arch == 'android':
                self.setup_android()
                self.build_android()


BUILDER = Builder()
