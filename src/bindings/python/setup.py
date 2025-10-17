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
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={build_dir}',
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            f'-DCMAKE_BUILD_TYPE={cfg}',
            f'-DSAIL_ROOT_DIR={sail_root}',
            '-DSAIL_COMBINE_CODECS=ON',
            '-DSAIL_BUILD_TESTS=OFF',
            '-DSAIL_BUILD_EXAMPLES=OFF',
            '-DSAIL_BUILD_APPS=OFF',
        ]

        build_args = ['--config', cfg]
        build_args += ['--parallel', f'{multiprocessing.cpu_count()}']

        env = os.environ.copy()

        if not os.path.exists(build_dir):
            os.makedirs(build_dir)

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

        # Copy the Python extension module (_libsail.*.so) to both extdir and sailpy
        ext_pattern = str(build_dir / '_libsail*.so')
        ext_files = glob.glob(ext_pattern)
        if not ext_files:
            raise RuntimeError(f"Python extension not found at {ext_pattern}")

        os.makedirs(extdir, exist_ok=True)

        for ext_file in ext_files:
            # Copy to extdir for setuptools
            shutil.copy2(ext_file, extdir)
            print(f"Copied {Path(ext_file).name} to {extdir}")
            # Also copy to sailpy for runtime
            shutil.copy2(ext_file, sailpy_dir)
            print(f"Copied {Path(ext_file).name} to {sailpy_dir}")

        # Copy SAIL libraries to sailpy directory so they can be found at runtime
        # Libraries are in build_dir root due to CMAKE_LIBRARY_OUTPUT_DIRECTORY
        sail_lib_patterns = [
            'libsail*.so*',
        ]
        for pattern in sail_lib_patterns:
            for lib_file in glob.glob(str(build_dir / pattern)):
                lib_path = Path(lib_file)
                if lib_path.is_file():
                    shutil.copy2(lib_path, sailpy_dir)
                    print(f"Copied {lib_path.name} to {sailpy_dir}")


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
