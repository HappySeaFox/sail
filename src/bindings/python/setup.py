#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

import os
import sys
import subprocess
import multiprocessing
from pathlib import Path
import re

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext


def get_version():
    """Get version from root CMakeLists.txt."""
    cmake = Path(__file__).resolve(
    ).parent.parent.parent.parent / "CMakeLists.txt"
    if not cmake.exists():
        raise FileNotFoundError(f"Cannot find SAIL CMakeLists.txt: {cmake}")

    match = re.search(
        r'project\s*\(\s*SAIL\s+VERSION\s+([\d.]+)', cmake.read_text(), re.I)
    if match:
        return match.group(1)
    else:
        raise ValueError(f"Cannot find SAIL version in {cmake}")

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))

        # Determine build configuration
        cfg = 'Debug' if self.debug else 'Release'

        # Find SAIL root directory FIRST (before cmake_args)
        # When building from repo: src/bindings/python/setup.py -> ../../../../
        # When building from sdist: we need to check if CMakeLists.txt exists
        sail_root = Path(__file__).parent.parent.parent.parent.resolve()

        # Check if this is a valid SAIL root
        if not (sail_root / 'CMakeLists.txt').exists():
            # Try alternative paths or raise error
            alt_sail_root = Path(__file__).parent.resolve()
            if not (alt_sail_root / 'CMakeLists.txt').exists():
                raise RuntimeError(
                    "Cannot find SAIL root directory with CMakeLists.txt.\n"
                    "Please ensure you're building from the SAIL repository, not from PyPI sdist.\n"
                    "To build from source:\n"
                    "  git clone https://github.com/HappySeaFox/sail\n"
                    "  cd sail/src/bindings/python\n"
                    "  pip install -e .\n"
                    "\n"
                    "Or install pre-built wheels from PyPI:\n"
                    "  pip install sailpy"
                )
            sail_root = alt_sail_root

        print(f"Building SAIL Python bindings from: {sail_root}")

        # Use fixed build directory instead of temporary one
        python_bindings_dir = Path(__file__).parent.resolve()
        build_dir = python_bindings_dir / 'build-python'

        cmake_args = [
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            f'-DCMAKE_BUILD_TYPE={cfg}',
            f'-DSAIL_ROOT_DIR={sail_root}',
            '-DSAIL_BUILD_TESTS=OFF',
            '-DSAIL_BUILD_EXAMPLES=OFF',
            '-DSAIL_BUILD_APPS=OFF',
            '-DSAIL_COMBINE_CODECS=ON',
            '-DSAIL_THIRD_PARTY_CODECS_PATH=OFF',
        ]

        # On Windows, force output to build_dir root (MSVC creates Debug/Release subdirs anyway)
        is_windows = sys.platform.startswith('win')
        if is_windows:
            cmake_args.extend([
                f'-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={build_dir}',
                f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={build_dir}',
                f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={build_dir}',
            ])
        else:
            cmake_args.append(f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={build_dir}')

        # Add vcpkg toolchain if available (from CMAKE_ARGS environment variable)
        # This is used on Windows via build-wheels.bat
        cmake_args_env = os.environ.get('CMAKE_ARGS', '')
        if cmake_args_env:
            cmake_args.extend(cmake_args_env.split())
            print(f"Adding CMAKE_ARGS from environment: {cmake_args_env}")

        build_args = ['--config', cfg]
        build_args += ['--parallel', f'{multiprocessing.cpu_count()}']

        env = os.environ.copy()

        if not os.path.exists(build_dir):
            os.makedirs(build_dir)

        print(f"CMake arguments: {cmake_args}")

        # Configure with CMake - pass the Python bindings directory
        subprocess.check_call(
            ['cmake', str(python_bindings_dir)] + cmake_args,
            cwd=build_dir,
            env=env
        )
        subprocess.check_call(
            ['cmake', '--build', '.', '--target', '_libsail'] + build_args,
            cwd=build_dir
        )

        # Copy Python extension module and SAIL libraries to sailpy directory
        import shutil
        import glob
        sailpy_dir = python_bindings_dir / 'sailpy'
        os.makedirs(extdir, exist_ok=True)

        # On Windows, MSVC puts files in Debug/Release subdirectories
        if is_windows:
            search_dirs = [build_dir, build_dir / cfg]
        else:
            search_dirs = [build_dir]

        # Find and copy Python extension module
        ext_pattern = '_libsail*.pyd' if is_windows else '_libsail*.so'
        ext_file = None
        for search_dir in search_dirs:
            files = list(search_dir.glob(ext_pattern))
            if files:
                ext_file = files[0]
                break

        if not ext_file:
            raise RuntimeError(f"Python extension not found in {build_dir}")

        shutil.copy2(ext_file, extdir)
        shutil.copy2(ext_file, sailpy_dir)
        print(f"Copied {ext_file.name}")

        # Copy all DLL/SO files (SAIL libs + dependencies)
        # Must copy to both extdir (for wheel) and sailpy_dir (for local development)
        lib_pattern = '*.dll' if is_windows else 'libsail*.so*'
        copied_count = 0
        for search_dir in search_dirs:
            for lib_file in search_dir.glob(lib_pattern):
                if lib_file.is_file():
                    # Copy to extdir for wheel packaging
                    if not (Path(extdir) / lib_file.name).exists():
                        shutil.copy2(lib_file, extdir)
                    # Copy to sailpy_dir for local development
                    if not (sailpy_dir / lib_file.name).exists():
                        shutil.copy2(lib_file, sailpy_dir)
                        copied_count += 1

        print(f"Copied {copied_count} library files to {sailpy_dir} and {extdir}")


setup(
    name='sailpy',
    version=get_version(),
    long_description=Path('README.md').read_text(encoding='utf-8'),
    long_description_content_type='text/markdown',
    packages=find_packages(),
    include_package_data=True,
    ext_modules=[CMakeExtension('sailpy._libsail')],
    cmdclass={'build_ext': CMakeBuild},
    zip_safe=False,
)
