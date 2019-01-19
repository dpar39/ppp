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

# Configuration
OPENCV_SRC_URL = 'https://github.com/opencv/opencv/archive/3.4.5.zip'
DLIB_SRC_URL = 'http://dlib.net/files/dlib-19.6.zip'
GMOCK_SRC_URL = 'https://github.com/google/googletest/archive/release-1.8.1.zip'

MINUS_JN = '-j%i' % min(multiprocessing.cpu_count(), 8)
IS_WINDOWS = sys.platform == 'win32'

if sys.platform == 'win32':
    PLATFORM = 'windows'
elif 'linux' in sys.platform:
    PLATFORM = 'linux'
elif sys.platform == 'darwin':
    PLATFORM = 'darwin'
ANDROID_SDK_TOOLS = 'https://dl.google.com/android/repository/sdk-tools-{}-4333796.zip'.format(PLATFORM)
ANDROID_NDK = 'https://dl.google.com/android/repository/android-ndk-r15c-{}-x86_64.zip'.format(PLATFORM)
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
    if IS_WINDOWS:
        link_cmd = 'mklink "%s" "%s"' % (dst_link, src_file_path)
    else:
        link_cmd = 'ln -sf "%s" "%s"' % (src_file_path, dst_link)
    print('Creating link for file "%s" in "%s"' % (src_file_path, dst_link))
    os.system(link_cmd)


class ShellRunner(object):
    def __init__(self, arch_name='x64'):
        self._env = os.environ.copy()
        self._extra_paths = []
        self._arch_name = arch_name
        if sys.platform == 'win32':
            self._detect_vs_version()

    def add_system_path(self, new_path):
        curr_path_str = self._env['PATH']
        path_elmts = set(curr_path_str.split(os.pathsep))
        if new_path in path_elmts:
            return
        self._env['PATH'] = curr_path_str + os.pathsep + new_path

    def set_env_var(self, var_name, var_value):
        assert isinstance(var_name, str), 'var_name should be a string'
        assert isinstance(
            var_value, str) or var_value is None, 'var_value should be a string or None'
        self._env[var_name] = var_value

    def get_env_var(self, var_name):
        return self._env.get(var_name, '')

    def run_cmd(self, cmd_args, cmd_print=True, cwd=None, input=None):
        """
        Runs a shell command
        """
        if isinstance(cmd_args, str):
            cmd_args = cmd_args.split()
        cmd_all = []
        if IS_WINDOWS:
            cmd_all = [self._vcvarsbat, self._arch_name,
                       '&&', 'set', 'CL=/MP', '&&']
        cmd_all = cmd_all + cmd_args

        if cmd_print:
            print(' '.join(cmd_args))

        p = subprocess.Popen(cmd_all, env=self._env, cwd=cwd,
                             stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
        if input:
            p.communicate(input=input)
        else:
            p.wait()
        if p.returncode != 0:
            print('Command "%s" exited with code %d' %
                  (' '.join(cmd_args), p.returncode))
            sys.exit(p.returncode)

    def _detect_vs_version(self):
        """
        Detects the first available version of Visual Studio
        """
        vc_releases = [('Visual Studio 15 2017', r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat'),
                       ('Visual Studio 15 2017', r'C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat'),
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

    def web_app_dir(self):
        """
        Returns the absolute path of the webapp directory (depending on wheter we using NodeJs or .Net Core server)
        """
        webapp_dir_name = 'webapp'
        return os.path.join(self._root_dir, webapp_dir_name)

    def run_cmake(self, cmake_generator, cmakelists_path='.'):
        """
        Runs CMake with the specified generator in the specified path with
        possibly some extra definitions
        """
        cmake_args = ['cmake',
                      '-DCMAKE_INSTALL_PREFIX=' + self._install_dir,
                      '-DCMAKE_PREFIX_PATH=' + self._install_dir,
                      '-DCMAKE_BUILD_TYPE=' + self._build_config,
                      '-G', cmake_generator]

        cmake_args.append(cmakelists_path)
        self.run_cmd(cmake_args)

    def run_cmd(self, cmd_args, cmd_print=True, cwd=None, input=None):
        self._shell.run_cmd(cmd_args, cmd_print=cmd_print,
                            cwd=cwd, input=input)

    def set_startup_vs_prj(self, project_name):
        """
        Rearranges the projects so that the specified project is the first
        therefore is the startup project within Visual Studio
        """
        solution_file = glob.glob(self._build_dir + '/*.sln')[0]
        sln_lines = []
        with open(solution_file) as file_handle:
            sln_lines = file_handle.read().splitlines()
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
        prj_lines = sln_lines[:2] + sln_lines[lin_prj_beg:lin_prj_end+1] \
            + sln_lines[2:lin_prj_beg] + sln_lines[lin_prj_end+1:]
        with open(solution_file, "w") as file_handle:
            file_handle.writelines(["%s\n" % item for item in prj_lines])

        # if not "devenv" in (p.name() for p in psutil.process_iter()):
        #    self.run_cmd(['call', 'devenv', solution_file])

    def build_dir_name(self, prefix):
        """
        Returns a name for a build directory based on the build configuration
        """
        return os.path.join(prefix, 'build_' + self._build_config + '_' + self._arch_name)

    def extract_gmock(self):
        """
        Extract and build GMock/GTest libraries
        """
        # Download googletest sources if not done yet
        gmock_src_pkg = self.download_third_party_lib(
            GMOCK_SRC_URL, 'googletest.zip')
        # Get the file prefix for googletest
        gmock_extract_dir = self.get_third_party_lib_dir('googletest')

        if gmock_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(gmock_src_pkg)

    def get_third_party_lib_dir(self, prefix):
        """
        Get the directory where a third party library with the specified prefix
        name was extracted, if any
        """
        third_party_dirs = next(os.walk(self._third_party_dir))[1]
        for lib_dir in third_party_dirs:
            if prefix in lib_dir:
                return os.path.join(self._third_party_dir, lib_dir)
        return None

    def build_opencv(self):
        """
        Downloads and builds OpenCV from source
        """
        ocv_build_modules = ['highgui', 'core', 'imgproc', 'objdetect', 'imgcodecs']

        # Skip building OpenCV if done already
        if IS_WINDOWS:
            if os.path.exists(os.path.join(self._third_party_install_dir, 'OpenCVConfig.cmake')):
                return
        else:
            lib_files = glob.glob(
                self._third_party_install_dir + '/lib/libopencv_*.a')
            if len(lib_files) >= len(ocv_build_modules):
                return
        # Download OpenCV sources if not done yet
        opencv_src_pkg = self.download_third_party_lib(OPENCV_SRC_URL)
        # Get the file prefix for OpenCV
        opencv_extract_dir = self.get_third_party_lib_dir('opencv-')

        if opencv_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(opencv_src_pkg)
            opencv_extract_dir = self.get_third_party_lib_dir('opencv')

        cmake_extra_defs = [
            '-DCMAKE_INSTALL_PREFIX=' + self._third_party_install_dir,
            '-DBUILD_WITH_STATIC_CRT=ON',
            '-DBUILD_SHARED_LIBS=OFF',
            '-DBUILD_PERF_TESTS=OFF',
            '-DBUILD_opencv_apps=OFF',
            '-DBUILD_WITH_DEBUG_INFO=OFF',
            '-DBUILD_DOCS=OFF',
            '-DBUILD_TESTS=OFF',
            '-DWITH_FFMPEG=OFF',
            '-DWITH_MSMF=OFF',
            '-DWITH_VFW=OFF',
            '-DWITH_OPENEXR=OFF',
            '-DWITH_WEBP=OFF',
            '-DBUILD_LIST=objdetect,imgproc,imgcodecs,highgui']  # List of libraries that need to be build

        # Clean and create the build directory
        build_dir = self.build_dir_name(opencv_extract_dir)
        if os.path.exists(build_dir):  # Remove the build directory
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir):  # Create the build directory
            os.mkdir(build_dir)
        # Build
        if not IS_WINDOWS:
            self.build_cmake_lib(opencv_extract_dir,
                                 cmake_extra_defs, [], False)
        else:  # Windows OS: only builds with msbuild.exe
            # Change directory to the build directory
            os.chdir(build_dir)
            cmake_cmd = ['cmake', '-G', self._shell.get_vc_cmake_generator()] \
                + cmake_extra_defs + [opencv_extract_dir]
            self.run_cmd(cmake_cmd)
            platform = 'x64' if '64' in self._arch_name else 'Win32'
            msbuild_conf = '/p:Configuration=' + self._build_config + ';Platform=' + platform
            self.run_cmd(['msbuild.exe', 'OpenCV.sln',
                          '/t:Build', msbuild_conf])
            self.run_cmd(['msbuild.exe', 'INSTALL.vcxproj',
                          '/t:Build', msbuild_conf])
            os.chdir(self._root_dir)

    def insert_static_crt(self, cmake_file):
        """
        Insert static CRT build on CMAKE
        """
        static_crt = """
        if (MSVC)
            # On windows, compile with CRT only in debug mode
            set(gtest_disable_pthreads ON CACHE INTERNAL "" FORCE)
            foreach(FLAG_VAR CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                if(${FLAG_VAR} MATCHES "/MD")
                    string(REGEX REPLACE "/MD" "/MT" ${FLAG_VAR} "${${FLAG_VAR}}")
                endif()
            endforeach()
            add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
        endif()
        """
        with open(cmake_file, 'r') as fp:
            cmake_content = fp.readlines()

        re_prj = re.compile(r'project\s*\(\s*dlib\s*\)', re.IGNORECASE)
        mod_content = []
        for line in cmake_content:
            mod_content.append(line)
            if re_prj.search(line):
                mod_content.append(static_crt)
        with open(cmake_file, 'w') as fp:
            fp.writelines(mod_content)

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
            print('Downloading %s to "%s" please wait ...' %
                  (url, lib_filepath))
            lib_file = urlopen(url)
            with open(lib_filepath, 'wb') as output:
                output.write(lib_file.read())
        return lib_filepath

    def extract_third_party_lib(self, lib_src_pkg, extract_dir=None):
        """
        Extracts a third party lib package source file into a directory
        """
        if not extract_dir:
            extract_dir = self._third_party_dir
        print('Extracting third party library "%s" into "%s" ... please wait ...' % (
            lib_src_pkg, extract_dir))
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

    def build_cmake_lib(self, cmakelists_path, extra_definitions, targets, clean_build=False):
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

        # Define CMake generator and make command
        if IS_WINDOWS:
            cmake_generator = 'NMake Makefiles'
            make_cmd = ['set', 'MAKEFLAGS=', '&&', 'nmake', 'VEBOSITY=1']
        else:
            cmake_generator = 'Unix Makefiles'
            make_cmd = ['make', MINUS_JN, 'install']
        os.chdir(build_dir)
        cmake_cmd = ['cmake',
                     '-DCMAKE_BUILD_TYPE=' + self._build_config,
                     '-G', cmake_generator] + extra_definitions
        cmake_cmd.append(cmakelists_path)
        # Run CMake and Make
        self.run_cmd(cmake_cmd)
        self.run_cmd(make_cmd)
        for target in targets:
            self.run_cmd(make_cmd + [target])
        os.chdir(self._root_dir)
#

    def parse_arguments(self):
        """
        Parses command line arguments
        """
        parser = argparse.ArgumentParser(
            description='Builds the passport photo application.')
        parser.add_argument('--arch_name', required=False,
                            choices=['x64', 'x86'], help='Platform architecture', default='x64')
        parser.add_argument('--build_config', required=False, choices=[
                            'debug', 'release'], help='Build configuration type', default='release')
        parser.add_argument(
            '--clean', help='Cleans the whole build directory', action="store_true")
        parser.add_argument(
            '--test', help='Runs unit tests', action="store_true")
        parser.add_argument(
            '--skip_install', help='Skips installation', action="store_true")
        parser.add_argument('--gen_vs_sln', help='Generates Visual Studio solution and projects',
                            action="store_true")
        parser.add_argument(
            '--android', help='Builds the android app', action="store_true")
        parser.add_argument(
            '--web', help='Builds the web app', action="store_true")

        args = parser.parse_args()

        self._arch_name = args.arch_name
        self._build_clean = args.clean
        self._build_config = args.build_config
        self._gen_vs_sln = args.gen_vs_sln
        self._run_tests = args.test
        self._run_install = not args.skip_install
        self._android_build = args.android
        self._web_build = args.web

        # directory suffix for the build and release
        self._root_dir = os.path.dirname(os.path.realpath(__file__))
        self._build_dir = os.path.join(self._root_dir, 'build_'
                                       + self._build_config + '_' + self._arch_name)
        self._install_dir = os.path.join(self._root_dir, 'install_'
                                         + self._build_config + '_' + self._arch_name)
        self._third_party_dir = os.path.join(self._root_dir, 'thirdparty')
        self._third_party_install_dir = os.path.join(self._third_party_dir, 'install_'
                                                     + self._build_config + '_' + self._arch_name)

        shell = ShellRunner(args.arch_name)

        # Set up some compiler flags
        if not IS_WINDOWS:
            shell.set_env_var('CXXFLAGS', '-fPIC')
            shell.set_env_var('LD_LIBRARY_PATH', self._install_dir)
        shell.set_env_var('INSTALL_DIR', self._install_dir)
        self._shell = shell
#

    def setup_android(self):
        # Download SDK tools if not present and extract it to
        android_sdk_tools_pkg = self.download_third_party_lib(
            ANDROID_SDK_TOOLS)

        # Extract the SDK if not done already
        android_sdk_tools_dirname = os.path.splitext(
            os.path.basename(android_sdk_tools_pkg))[0]
        android_sdk_tools_dir = self.get_third_party_lib_dir(
            android_sdk_tools_dirname)
        if android_sdk_tools_dir is None:
            android_sdk_tools_dir = os.path.join(
                self._third_party_dir, android_sdk_tools_dirname)
            self.extract_third_party_lib(
                android_sdk_tools_pkg, android_sdk_tools_dir)

        # Download gradle
        gradle_pkg = self.download_third_party_lib(ANDROID_GRADLE)
        gradle_pkg_dir = self.get_third_party_lib_dir('gradle')
        if gradle_pkg_dir is None:
            self.extract_third_party_lib(gradle_pkg)
            gradle_pkg_dir = self.get_third_party_lib_dir('gradle')

        # Download Android NDK if not present
        android_ndk_pkg = self.download_third_party_lib(ANDROID_NDK)

        # Extract the NDK if not done already
        android_ndk_dir = self.get_third_party_lib_dir('android-ndk')
        if android_ndk_dir is None:
            self.extract_third_party_lib(android_ndk_pkg)

        # Set up environment
        android_user_dir = os.path.join(os.path.expanduser("~"), '.android')
        if not os.path.exists(android_user_dir):
            os.mkdir(android_user_dir)
        repos_cfg = os.path.join(android_user_dir, 'repositories.cfg')
        if not os.path.exists(repos_cfg):
            with open(repos_cfg, 'w') as fp:
                fp.write('')

        # self._shell.set_env_var('JAVA_HOME', '/usr/lib/jvm/java-8-oracle')
        self._shell.set_env_var('ANDROID_HOME', android_sdk_tools_dir)
        self._shell.set_env_var('ANDROID_SDK_ROOT', android_sdk_tools_dir)
        self._shell.set_env_var('ANDROID_NDK_HOME', android_ndk_dir)
        # self._shell.set_env_var('JAVA_OPTS', '-XX:+IgnoreUnrecognizedVMOptions --add-modules java.se.ee')
        self._shell.set_env_var('JAVA_OPTS', '')

        bin_tools = os.path.normpath(
            os.path.join(android_sdk_tools_dir, 'tools/bin'))
        self._shell.add_system_path(bin_tools)
        self._shell.add_system_path(os.path.normpath(
            os.path.join(android_sdk_tools_dir, 'tools')))
        self._shell.add_system_path(os.path.normpath(
            os.path.join(gradle_pkg_dir, 'bin')))
        self._shell.add_system_path(os.path.normpath(android_ndk_dir))
        self._shell.add_system_path(os.path.normpath(
            os.path.join(self._shell.get_env_var('JAVA_HOME'), '/jre/bin')))

        # print(self._shell._env)
        if os.name == 'posix':
            self.run_cmd('chmod -R +x {}'.format(bin_tools))
            self.run_cmd('chmod -R +x {}'.format(android_ndk_dir))
            self.run_cmd('chmod -R +x {}/bin'.format(gradle_pkg_dir))
        # self.run_cmd('yes | sdkmanager --licenses')
        # self.run_cmd('sdkmanager "platform-tools" "platforms;android-25"', input='y')

    def extract_validation_data(self):
        """
        Extracts validation imageset with annotations from a password protected zip file
        These images were requested at http://www.scface.org/ and are copyrighted,
        so please do not share them without obatining written consent
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
                if key == 'file' and not node.get('data', ''):
                    file_name = node['file']
                    file_path = os.path.join(lippp_share_dir, file_name)
                    with open(file_path, 'rb') as fp:
                        content = base64.b64encode(fp.read()).decode('ascii')
                    node['data'] = content
                else:
                    expand_node(node[key])

        config_input_file = os.path.join(lippp_share_dir, 'config.json')
        with open(config_input_file) as fp:
            config_data = json.load(fp)

        for key in config_data:
            expand_node(config_data[key])

        config_bundle_file = os.path.join(
            lippp_share_dir, 'config.bundle.json')
        with open(config_bundle_file, 'w') as fp:
            json.dump(config_data, fp)

    def build_cpp_code(self):
        """
        Builds the C++ libppp project from sources
        """

        # self.run_cmd(
        #     'swig -c++ -python -Ilibppp/include -outdir libppp/python -o libppp/swig/libppp_python_wrap.cxx libppp/swig/libppp.i')

        # Build actions
        if self._build_clean and os.path.exists(self._build_dir):
            # Remove the build directory - clean
            shutil.rmtree(self._build_dir)
        if not os.path.exists(self._build_dir):
            # Create the build directory if doesn't exist
            os.mkdir(self._build_dir)

        # Configure build system
        make_cmd = ['make', MINUS_JN]
        cmake_generator = 'Unix Makefiles'
        if IS_WINDOWS:
            cmake_generator = 'NMake Makefiles'
            make_cmd = ['nmake']

        # Change directory to build directory
        os.chdir(self._build_dir)
        if self._gen_vs_sln:
            # Generating visual studio solution
            cmake_generator = self._shell.get_vc_cmake_generator()
            self.run_cmake(cmake_generator, '..')
            self.set_startup_vs_prj('ppp_test')
        else:
            # Building the project code from the command line
            self.run_cmake(cmake_generator, '..')
            # Copy binaries to the local install directory
            if self._run_install:
                self.run_cmd(make_cmd + ['install'])
            else:
                self.run_cmd(make_cmd)
            # Run unit tests for C++ code
            if self._run_tests:
                os.chdir(self._install_dir)
                test_exe = r'.\ppp_test.exe' if IS_WINDOWS else './ppp_test'
                self.run_cmd([test_exe, '--gtest_output=xml:tests.xml'])
        os.chdir(self._root_dir)

        # Copy libppp artifacts to the webapp directory
        dist_files = ['liblibppp.so', 'libppp.dll', 'liblibppp.dylib']
        for dist_file in dist_files:
            src_file_path = os.path.join(self._install_dir, dist_file)
            dst_link = os.path.join(self.web_app_dir(), dist_file)
            if os.path.exists(src_file_path):
                link_file(src_file_path, dst_link)

        # Copy libppp configuration file to assets (this is needed for Android and IOS apps)
        libpp_config_file = os.path.join(
            self._root_dir, 'libppp/share/config.bundle.json')
        dst_link = os.path.join(
            self.web_app_dir(), 'src', 'assets', os.path.basename(libpp_config_file))
        link_file(libpp_config_file, dst_link)

    def build_android(self):
        """
        Builds android app
        """
        if self._android_build:
            # Create swig code
            self.run_cmd('swig -c++ -java -package swig -Ilibppp/include -outdir webapp/android/app/src/main/java/swig -module libppp -o libppp/swig/libppp_java_wrap.cxx libppp/swig/libppp.i')
            # Build android project
            self.run_cmd('gradle build --stacktrace', cwd='webapp/android')

    def build_webapp(self, build=True):
        """
        Builds and test the web application by running shell commands
        """
        #  Copy libppp configurations and build files to the webapp directory
        dist_files = ['libppp/share/config.bundle.json',
                      'libppp/python/libpppwrapper.py']
        for dist_file in dist_files:
            src_file_path = os.path.join(self._root_dir, dist_file)
            dst_link = os.path.join(
                self.web_app_dir(), os.path.basename(dist_file))
            if os.path.exists(src_file_path):
                link_file(src_file_path, dst_link)

        # Install the angular-cli if not found in PATH
        if not which('ng'):
            self.run_cmd('npm install @angular/cli -g')
        # Install the angular-cli if not found in PATH
        if not which('npx'):
            self.run_cmd('npm install npx -g')

        # Build the web app
        if self._web_build:
            os.chdir(self.web_app_dir())
            self.run_cmd(['npm', 'install'])
            self.run_cmd(
                ['ng', 'test', '--browsers=PhantomJS', '--watch=false'])
            self.run_cmd(['ng', 'build'])
            os.chdir(self._root_dir)

    def deploy_to_heroku(self):
        """
        Deploys the webserver to azure
        """
        pass  # TODO

    def __init__(self):
        # Detect OS version
        self.parse_arguments()

        # Setup android tools
        if self._android_build:
            self.setup_android()

        # Create install directory if it doesn't exist
        if not os.path.exists(self._install_dir):
            os.mkdir(self._install_dir)

        # Extract testing dataset
        self.extract_validation_data()
        self.bundle_config()

        # Build Third Party Libs
        self.extract_gmock()
        self.build_opencv()

        # Build this project for a desktop platform (Windows or Unix-based OS)
        self.build_cpp_code()

        # Copy built addon and configuration to webapp
        self.build_webapp(False)

        # Build the android app
        self.build_android()


BUILDER = Builder()
