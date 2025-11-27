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
import tempfile
import fnmatch
from pathlib import Path
import re

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from setuptools.command.sdist import sdist
import shutil

def find_sail_root(start_dir=None):
    """Find SAIL root directory and version by looking for CMakeLists.txt with SAIL project.
    Returns tuple (root_path, version).
    Raises FileNotFoundError if not found.
    """
    if start_dir is None:
        start_dir = Path(__file__).parent.resolve()
    else:
        start_dir = Path(start_dir).resolve()

    current = start_dir
    for _ in range(5):
        cmake_path = current / "CMakeLists.txt"
        if cmake_path.exists():
            try:
                content = cmake_path.read_text()
                match = re.search(r'project\s*\(\s*SAIL\s+VERSION\s+([\d.]+)', content, re.I)
                if match:
                    return current.resolve(), match.group(1)
            except Exception:
                pass
        current = current.parent
        if current == current.parent:  # Reached filesystem root
            break

    raise FileNotFoundError("Cannot find SAIL CMakeLists.txt with version")

def sail_version():
    """Get version from root CMakeLists.txt."""
    _, version = find_sail_root()
    return version

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def _generate_version_file(self, target_dir):
        """Generate _version.py from template in the target directory."""
        setup_dir = Path(__file__).parent.resolve()
        if (setup_dir / 'src' / 'bindings' / 'sail-python' / 'sailpy').exists():
            sailpy_dir = setup_dir / 'src' / 'bindings' / 'sail-python' / 'sailpy'
        else:
            sailpy_dir = setup_dir / 'sailpy'

        template_file = sailpy_dir / '_version.py.in'
        version_file = target_dir / '_version.py'

        version = sail_version()
        content = template_file.read_text()
        content = content.replace('@SAIL_VERSION@', version)
        version_file.write_text(content)
        print(f"Generated _version.py with version {version} in {target_dir}")

    def run(self):
        super().run()

        build_sailpy_dir = Path(self.build_lib) / 'sailpy'
        self._generate_version_file(build_sailpy_dir)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))

        cfg = 'Debug' if self.debug else 'Release'

        setup_dir = Path(__file__).parent.resolve()
        sail_root, _ = find_sail_root(setup_dir)

        if not (sail_root / 'src' / 'sail').exists():
            raise RuntimeError(
                f"SAIL root directory found at {sail_root} but src/sail directory is missing.\n"
                "Please ensure you're building from the SAIL repository or from a complete sdist.\n"
            )

        print(f"Building SAIL Python bindings from: {sail_root}")

        if (setup_dir / 'src' / 'bindings' / 'sail-python').exists():
            python_bindings_dir = setup_dir / 'src' / 'bindings' / 'sail-python'
        else:
            python_bindings_dir = setup_dir
        build_dir = python_bindings_dir / 'build-python'

        cmake_args = [
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            f'-DCMAKE_BUILD_TYPE={cfg}',
            f'-DSAIL_ROOT_DIR={sail_root}',
            '-DBUILD_TESTING=OFF',
            '-DSAIL_BUILD_EXAMPLES=OFF',
            '-DSAIL_BUILD_APPS=OFF',
            '-DSAIL_COMBINE_CODECS=ON',
            '-DSAIL_THIRD_PARTY_CODECS_PATH=OFF',
        ]

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

        cmake_source_dir = python_bindings_dir

        subprocess.check_call(
            ['cmake', str(cmake_source_dir)] + cmake_args,
            cwd=build_dir,
            env=env
        )

        subprocess.check_call(
            ['cmake', '--build', '.', '--target', '_libsail'] + build_args,
            cwd=build_dir
        )

        # Copy Python extension module and SAIL libraries to sailpy directory
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


class CustomSDist(sdist):
    """Custom sdist command that includes SAIL source files from parent directories."""

    def make_release_tree(self, base_dir, files):
        """Override to add SAIL source files to the release tree."""
        super().make_release_tree(base_dir, files)

        sail_root = Path(__file__).parent.parent.parent.parent.resolve()
        python_bindings_dir = Path(__file__).parent.resolve()
        release_dir = Path(base_dir).resolve()

        print("Including SAIL source files in sdist...")

        # Use git archive to get clean source tree without build artifacts
        with tempfile.TemporaryDirectory() as tmpdir:
            archive_dir = Path(tmpdir) / 'sail-archive'
            archive_dir.mkdir()

            # Create clean archive using git archive
            subprocess.check_call(
                ['git', 'archive', '--format=tar', 'HEAD'],
                cwd=sail_root,
                stdout=open(archive_dir / 'archive.tar', 'wb')
            )
            subprocess.check_call(
                ['tar', '-xf', 'archive.tar'],
                cwd=archive_dir
            )
            (archive_dir / 'archive.tar').unlink()

            # Ignore patterns for copying (shell patterns)
            def ignore_patterns(src, names):
                patterns = ['*.so*', 'sailpy.egg-info']
                ignored = set()
                for name in names:
                    for pattern in patterns:
                        if fnmatch.fnmatch(name, pattern):
                            ignored.add(name)
                            break
                return ignored

            # Copy setup.py to root (overwrite if exists)
            setup_py_src = python_bindings_dir / 'setup.py'
            setup_py_dest = release_dir / 'setup.py'
            if setup_py_src.exists():
                shutil.copy2(setup_py_src, setup_py_dest)

            # Copy SAIL root CMakeLists.txt (overwrite Python bindings one if exists)
            sail_cmake = archive_dir / 'CMakeLists.txt'
            if sail_cmake.exists():
                shutil.copy2(sail_cmake, release_dir / 'CMakeLists.txt')

            # Copy all SAIL source files from git archive
            sail_items = [
                (archive_dir / 'cmake', 'cmake'),
                (archive_dir / 'src', 'src'),
                (archive_dir / 'tests', 'tests'),
                (archive_dir / 'LICENSE.txt', 'LICENSE.txt'),
                (archive_dir / 'LICENSE.INIH.txt', 'LICENSE.INIH.txt'),
                (archive_dir / 'LICENSE.MUNIT.txt', 'LICENSE.MUNIT.txt'),
                (archive_dir / 'vcpkg.json', 'vcpkg.json'),
            ]

            for src_path, dest_rel in sail_items:
                if src_path.exists():
                    dest_path = release_dir / dest_rel
                    if dest_path.exists():
                        if dest_path.is_dir():
                            shutil.rmtree(dest_path)
                        else:
                            dest_path.unlink()
                    if src_path.is_file():
                        dest_path.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(src_path, dest_path)
                    else:
                        shutil.copytree(src_path, dest_path, ignore=ignore_patterns)

        print("Added SAIL source files to sdist")


setup(
    name='sailpy',
    version=sail_version(),
    long_description=Path('README.md').read_text(encoding='utf-8'),
    long_description_content_type='text/markdown',
    packages=find_packages(),
    include_package_data=True,
    ext_modules=[CMakeExtension('sailpy._libsail')],
    cmdclass={'build_ext': CMakeBuild, 'sdist': CustomSDist},
    zip_safe=False,
)
