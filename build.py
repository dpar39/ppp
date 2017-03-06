"""
build.py
"""

import os
import sys
import glob
import shutil
import argparse
import subprocess
import zipfile
import tarfile
import multiprocessing

# Configuration
GMOCK_SRC_URL = 'https://googlemock.googlecode.com/files/gmock-1.7.0.zip'
NODEJS_SRC_URL = 'https://nodejs.org/dist/v4.4.7/node-v4.4.7.tar.gz'
OPENCV_SRC_URL = 'https://github.com/Itseez/opencv/archive/3.1.0.zip'
POCO_SRC_URL = 'http://pocoproject.org/releases/poco-1.7.4/poco-1.7.4-all.zip'

MINUS_JN = '-j%i' % min(multiprocessing.cpu_count(), 8)
IS_WINDOWS = sys.platform == 'win32'
# All thrid party libs that can be build with CMAKE are unpackaged and built
# within a 'build' directory inside their respective folder

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

class Builder(object):
    """
    Class that holds the whole building process
    """
    def detect_vs_version(self):
        """
        Detects the first available version of Visual Studio
        """
        vc_releases = [('14', '2015'), ('12', '2013'), ('11', '2012'), ('10', '2010')]
        for (vc_version, vc_release) in vc_releases:
            vcvarsbat = "C:\\Program Files (x86)\\Microsoft Visual Studio %s.0\\VC\\vcvarsall.bat" \
                % vc_version
            if os.path.exists(vcvarsbat):
                self._vc_version = vc_version
                self._vcvarsbat = vcvarsbat
                self._vc_cmake_gen = 'Visual Studio ' + vc_version + ' ' + vc_release
                if "64" in self._arch_name:
                    self._vc_cmake_gen += ' Win64'
                break

    def run_cmd(self, cmd_args):
        """
        Runs a shell command
        """
        env = os.environ.copy()
        cmd_all = []
        if IS_WINDOWS:
            # Load visual studio environmental variables first
            if not hasattr(self, '_vcvarsbat'):
                self.detect_vs_version()
            cmd_all = [self._vcvarsbat, self._arch_name, '&&', 'set', 'CL=/MP', '&&']
        else:
            env['CXXFLAGS'] = '-fPIC'
        cmd_all = cmd_all + cmd_args
        print ' '.join(cmd_args)
        process = subprocess.Popen(cmd_all, env=env)
        process.wait()
        if process.returncode != 0:
            print 'Command "%s" exited with code %d' % (' '.join(cmd_args), process.returncode)
            os.chdir(self._root_dir)
            sys.exit(process.returncode)

    def run_cmake(self, cmake_generator, cmakelists_path='.', extra_definitions=[]):
        """
        Runs CMake with the specified generator in the specified path with
        possibly some extra definitions
        """
        cmake_args = ['cmake',  \
        '-DCMAKE_INSTALL_PREFIX=' + self._install_dir,  \
        '-DCMAKE_PREFIX_PATH=' + self._install_dir,  \
        '-DCMAKE_BUILD_TYPE=' + self._build_config,  \
        '-G', cmake_generator]

        for elmt in extra_definitions:
            cmake_args.append(elmt)

        cmake_args.append(cmakelists_path)
        self.run_cmd(cmake_args)

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
            file_handle.writelines(["%s\n" % item  for item in prj_lines])
        self.run_cmd(['call', 'devenv', solution_file])

    def build_dir_name(self, prefix):
        """
        Returns a name for a build directory based on the build configuration
        """
        return os.path.join(prefix, 'build_' + self._build_config + '_' + self._arch_name)
        
    def build_nodejs(self):
        """
        Downloads, extract and builds Node JS from source (Windows ONLY)
        """
        # Download Node JS if not done yet
        node_src_pkg = self.download_third_party_lib(NODEJS_SRC_URL)
        # Get the file prefix for node js
        node_extract_dir = self.get_third_party_lib_dir('node')
        if node_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(node_src_pkg)
            node_extract_dir = self.get_third_party_lib_dir('node')
        # Build Node JS if not done yet
        node_exe_path = os.path.join(node_extract_dir, self._build_config, 'node.exe')

        if not os.path.exists(node_exe_path):
            print 'Building Node JS from sources ... please wait ...'
            os.chdir(node_extract_dir)
            build_cmd = ['vcbuild.bat', 'nosign', self._build_config]
            if "64" in self._arch_name:
                build_cmd.append('x64')
            self.run_cmd(build_cmd)
            os.chdir(self._root_dir)
        # Install built node executable into the install dir
        if self._run_install:
            shutil.copy(node_exe_path, self._install_dir)

    def build_poco_lib(self, override=False):
        """
        Downloads, extracts and builds POCO libraries from source, if not done yet
        """
        # Download POCO sources if not done yet
        poco_src_pkg = self.download_third_party_lib(POCO_SRC_URL)
        poco_extract_dir = self.get_third_party_lib_dir('poco')
        if poco_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(poco_src_pkg)
            poco_extract_dir = self.get_third_party_lib_dir('poco')

        poco_all_modules = ['XML', 'JSON', 'MONGODB', 'UTIL', \
           'NET', 'NETSSL', 'NETSSL_WIN', 'CRYPTO', \
           'DATA', 'DATA_SQLITE', 'DATA_MYSQL', 'DATA_ODBC', 'ZIP', \
           'PAGECOMPILER', 'PAGECOMPILER_FILE2PAGE']
        poco_build_modules = [] # Foundation always gets built - that's all we need for now

        lib_files = glob.glob(self._third_party_install_dir + '/lib/Poco*.lib') \
            + glob.glob(self._third_party_install_dir + '/lib/libPoco*.a')
        if len(lib_files) < len(poco_build_modules) + 1 or override:
            # Build POCO libraries
            cmake_definitions = [
                '-DPOCO_STATIC=ON', \
                '-DPOCO_MT=ON', \
                '-DCMAKE_INSTALL_PREFIX=' + self._third_party_install_dir, \
                '-DCMAKE_BUILD_TYPE=' + self._build_config]

            for module in poco_all_modules:
                on_off = '=ON' if module in poco_build_modules else '=OFF'
                cmake_def = '-DENABLE_' + module + on_off
                cmake_definitions.append(cmake_def)

            self.build_cmake_lib(poco_extract_dir, cmake_definitions, ['install'])

    def extract_gmock(self):
        """
        Extract and build GMock/GTest libraries
        """
        # Download POCO sources if not done yet
        gmock_src_pkg = self.download_third_party_lib(GMOCK_SRC_URL)
        # Get the file prefix for POCO
        gmock_extract_dir = self.get_third_party_lib_dir('gmock')

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
        # Download OpenCV sources if not done yet
        opencv_src_pkg = self.download_third_party_lib(OPENCV_SRC_URL)
        # Get the file prefix for OpenCV
        opencv_extract_dir = self.get_third_party_lib_dir('opencv')

        if opencv_extract_dir is None:
            # Extract the source files
            self.extract_third_party_lib(opencv_src_pkg)
            opencv_extract_dir = self.get_third_party_lib_dir('opencv')

        ocv_all_modules = ['core', 'flann', 'imgproc', \
            'ml', 'photo', 'video', 'imgcodecs', 'shape', \
            'videoio', 'highgui', 'objdetect', 'superres', \
            'ts', 'features2d', 'calib3d', 'stitching', \
            'videostab', 'java']
        ocv_build_modules = ['highgui', 'core', 'imgproc',\
            'objdetect', 'imgcodecs']

        # Skip building OpenCV if done already
        if IS_WINDOWS:
            if os.path.exists(os.path.join(self._third_party_install_dir, 'OpenCVConfig.cmake')):
                return
        else:
            lib_files = glob.glob(self._third_party_install_dir + '/lib/libopencv_*.a')
            if len(lib_files) == len(ocv_build_modules):
                return

        cmake_extra_defs = [ \
            '-DCMAKE_INSTALL_PREFIX=' + self._third_party_install_dir, \
            '-DBUILD_WITH_STATIC_CRT=ON', \
            '-DBUILD_SHARED_LIBS=OFF', \
            '-DBUILD_PERF_TESTS=OFF', \
            '-DBUILD_opencv_apps=OFF' \
            '-DBUILD_WITH_DEBUG_INFO=OFF', \
            '-DBUILD_DOCS=OFF', \
            '-DBUILD_TESTS=OFF', \
            '-DWITH_FFMPEG=OFF', \
            '-DWITH_MSMF=OFF', \
            '-DWITH_VFW=OFF', \
            '-DWITH_OPENEXR=OFF', \
            '-DWITH_WEBP=OFF']

        for ocv_module in ocv_all_modules:
            onoff = '=ON' if ocv_module in ocv_build_modules else '=OFF'
            cmake_def = '-DBUILD_opencv_' + ocv_module + onoff
            cmake_extra_defs.append(cmake_def)

        # Clean and create the build directory
        build_dir = self.build_dir_name(opencv_extract_dir)
        if os.path.exists(build_dir): # Remove the build directory
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir): # Create the build directory
            os.mkdir(build_dir)
        # Build
        if not IS_WINDOWS:
            self.build_cmake_lib(opencv_extract_dir, cmake_extra_defs, [], False)
        else: # Windows OS: only builds with msbuild.exe
            # Change directory to the build directory
            os.chdir(build_dir)
            cmake_cmd = ['cmake', '-G', self._vc_cmake_gen] \
                + cmake_extra_defs + [opencv_extract_dir]
            self.run_cmd(cmake_cmd)
            platform = 'x64' if '64' in self._arch_name else 'Win32'
            msbuild_conf = '/p:Configuration='+ self._build_config + ';Platform=' + platform
            self.run_cmd(['msbuild.exe', 'OpenCV.sln', \
                '/t:Build', msbuild_conf])
            self.run_cmd(['msbuild.exe', 'INSTALL.vcxproj', \
                '/t:Build', msbuild_conf])
            os.chdir(self._root_dir)

    def get_filename_from_url(self, url):
        """
        Extracts the file name from a given URL
        """
        lib_filename = url.split('/')[-1].split('#')[0].split('?')[0]
        lib_filepath = os.path.join(self._third_party_dir, lib_filename)
        return lib_filepath

    def download_third_party_lib(self, url):
        """
        Download a third party dependency from the internet if is not available offline
        """
        lib_filepath = self.get_filename_from_url(url)
        if not os.path.exists(lib_filepath):
            print 'Downloading ' + url + ' to "' + lib_filepath + '" please wait ...'
            import urllib2
            lib_file = urllib2.urlopen(url)
            with open(lib_filepath, 'wb') as output:
                output.write(lib_file.read())
        return lib_filepath

    def extract_third_party_lib(self, lib_src_pkg):
        """
        Extracts a third party lib package source file into a directory
        """
        print 'Extracting third party library "' + lib_src_pkg + '" please wait ...'
        if 'zip' in lib_src_pkg:
            zip_handle = zipfile.ZipFile(lib_src_pkg)
            for item in zip_handle.namelist():
                zip_handle.extract(item, self._third_party_dir)
            zip_handle.close()
        else: # Assume tar archive (tgz, tar.bz2, tar.gz)
            tar = tarfile.open(lib_src_pkg, 'r')
            for item in tar:
                tar.extract(item, self._third_party_dir)
            tar.close()

    def build_cmake_lib(self, cmakelists_path, extra_definitions=[], targets=[], clean_build=False):
        """
        Builds a library using cmake
        """
        build_dir = self.build_dir_name(cmakelists_path)
        # Clean and create the build directory
        if clean_build and os.path.exists(build_dir): # Remove the build directory
            shutil.rmtree(build_dir)
        if not os.path.exists(build_dir): # Create the build directory
            os.mkdir(build_dir)

        # Define CMake generator and make command
        if IS_WINDOWS:
            cmake_generator = 'NMake Makefiles'
            make_cmd = ['set', 'MAKEFLAGS=', '&&', 'nmake', 'VEBOSITY=1']
        else:
            cmake_generator = 'Unix Makefiles'
            make_cmd = ['make', MINUS_JN, 'install']
        os.chdir(build_dir)
        cmake_cmd = ['cmake',  \
            '-DCMAKE_BUILD_TYPE=' + self._build_config,  \
            '-G', cmake_generator] + extra_definitions
        cmake_cmd.append(cmakelists_path)
        # Run CMake and Make
        self.run_cmd(cmake_cmd)
        self.run_cmd(make_cmd)
        for target in targets:
            self.run_cmd(make_cmd + [target])
        os.chdir(self._root_dir)

    def parse_arguments(self):
        """
        Parses command line arguments
        """
        if IS_WINDOWS:
            default_arch_name = 'x86'
            default_build_cfg = 'debug'
        else:
            default_arch_name = 'x64'
            default_build_cfg = 'release'

        parser = argparse.ArgumentParser(description='Builds the passport photo application.')
        parser.add_argument('--arch_name', help='Platform [x86 | x64]', default=default_arch_name)
        parser.add_argument('--build_config', help='Build configuration [debug | release]', \
            default=default_build_cfg)
        parser.add_argument('--clean', help='Cleans the whole build directory', action="store_true")
        parser.add_argument('--skip_tests', help='Run existing unit tests', action="store_true")
        parser.add_argument('--skip_install', help='Runs install commands', action="store_true")
        parser.add_argument('--gen_vs_sln', help='Generates Visual Studio solution and projects', \
            action="store_true")

        args = parser.parse_args()

        self._arch_name = args.arch_name
        self._build_clean = args.clean
        self._build_config = args.build_config
        self._gen_vs_sln = args.gen_vs_sln
        self._run_tests = not args.skip_tests
        self._run_install = not args.skip_install

        # directory suffix for the build and release
        self._root_dir = os.path.dirname(os.path.realpath(__file__))
        self._build_dir = os.path.join(self._root_dir, 'build_' \
            + self._build_config + '_' + self._arch_name)
        self._install_dir = os.path.join(self._root_dir, 'install_' \
            + self._build_config + '_' + self._arch_name)
        self._third_party_dir = os.path.join(self._root_dir, 'thirdparty')
        self._third_party_install_dir = os.path.join(self._third_party_dir, 'install_' \
            + self._build_config + '_' + self._arch_name)
        if self._gen_vs_sln:
            self._build_dir = os.path.join(self._root_dir, 'visualstudio')

    def build_addon_with_nodegyp(self):
        """
        Builds the Node JS addon using node-gyp (Linux only)
        """
        addon_dir = os.path.join(self._root_dir, 'addon')
        os.chdir(addon_dir)
        self.run_cmd(['node-gyp', 'clean', 'configure', 'build'])
        os.chdir(self._root_dir)
        # Copy build output to install directory
        shutil.copy(os.path.join(addon_dir, "build", "Release", "addon.node"), self._install_dir)
        shutil.copy(os.path.join(addon_dir, "test.js"), self._install_dir)

    def extract_validation_data(self):
        """
        Extracts validation imageset with annotations from a password protected zip file
        These images were requested at http://www.scface.org/ and are copyrighted,
        so please do not share them without obatining written consent
        """
        def extract(research_dir, zip_file):
            """
            Extracts file from zip archive
            """
            zip_file = os.path.join(research_dir, zip_file)
            if os.path.exists(os.path.join(research_dir, 'mugshot_frontal_original_all')):
                return # Nothing to do, data already been extracted
            print 'Extracting validation data ...'
            zip_handle = zipfile.ZipFile(zip_file)
            for item in zip_handle.namelist():
                zip_handle.extract(item, research_dir, pwd='mugshot_frontal_original_all.zip')
            zip_handle.close()
            print 'Extracting validation data completed!'

        research_dir = os.path.join(self._root_dir, 'research')
        extract(research_dir, 'annotated_imageset0.zip')
        extract(research_dir, 'annotated_imageset1.zip')
        extract(research_dir, 'annotated_imageset2.zip')
        extract(research_dir, 'annotated_imageset3.zip')

    def build_cpp_code(self):
        """
        Builds the project from sources
        """
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
            cmake_generator = self._vc_cmake_gen
            self.run_cmake(cmake_generator, '..')
            self.set_startup_vs_prj('ppp_test')
        else:
            # Building the project code from the command line
            self.run_cmake(cmake_generator, '..')
            self.run_cmd(make_cmd)
            if self._run_tests:
                self.run_cmd(make_cmd + ['test'])
            if self._run_install:
                self.run_cmd(make_cmd + ['install'])
            os.chdir(self._root_dir)

            self.build_addon_with_nodegyp()
            # Run addon integration test
            os.chdir(self._install_dir)
            self.run_cmd(['node', './test.js'])
        os.chdir(self._root_dir)

    def deploy_addon(self):
        """
        Deploys the addon to the webapp directory as well as the shared configuration
        """
        webapp_dir = os.path.join(self._root_dir, 'webapp')
        shutil.copy(os.path.join(self._install_dir, 'addon.node'), webapp_dir)
        shutil.copy(os.path.join(self._install_dir, 'config.json'), webapp_dir)
        for shared_lib in ['liblibppp.so', 'libppp.dll']:
            shared_lib_path = os.path.join(self._install_dir, shared_lib)
            if os.path.exists(shared_lib_path):
                shutil.copy(shared_lib_path, webapp_dir)

    def __init__(self):
        # Detect OS version
        self.parse_arguments()
        if IS_WINDOWS:
            self.detect_vs_version()

        # Create install directory if it doesn't exist
        if not os.path.exists(self._install_dir):
            os.mkdir(self._install_dir)

        # Build Third party libs
        self.extract_gmock()
        if not IS_WINDOWS:
            self.build_poco_lib()
        self.build_opencv()

        if self._gen_vs_sln:
            # Build Node JS from source so the addon can be build reliably for Windows
            self.build_nodejs()

        #Extract validation data (imageset with annotations)
        if self._run_tests:
            self.extract_validation_data()

        # Build this project
        self.build_cpp_code()

        # Copy built addon and configuration to webapp
        if not self._gen_vs_sln:
            self.deploy_addon()

BUILDER = Builder()
