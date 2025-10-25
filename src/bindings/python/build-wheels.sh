#!/bin/bash

# Build wheels for multiple Python versions using pyenv
# If pyenv is not found, builds for current Python only

set -euo pipefail

cd "$(dirname "$0")"

# Python versions to build for
if [ $# -gt 0 ]; then
    PYTHON_VERSIONS=("$@")
else
    PYTHON_VERSIONS=("3.10" "3.11" "3.12" "3.13" "3.14")
fi

OS_TYPE=$(uname -s)

echo
echo "====================================="
echo "=  Building Python Wheels for PyPI  ="
echo "====================================="
echo
echo "Platform: $OS_TYPE"
echo "Python Manager: pyenv"
echo "Target versions: ${PYTHON_VERSIONS[*]}"
echo

echo "[1/4] Preparing environment..."

if [[ "$OS_TYPE" == "Linux" ]]; then
    if [ -d "$HOME/.pyenv" ]; then
        export PYENV_ROOT="$HOME/.pyenv"
        export PATH="$PYENV_ROOT/bin:$PATH"
        eval "$(pyenv init -)"
    elif command -v pyenv &> /dev/null; then
        eval "$(pyenv init -)"
    else
        echo "Error: 'pyenv' is not available" >&2
        exit 1
    fi
else
    if ! command -v pyenv &> /dev/null; then
        echo "Error: 'pyenv' is not available" >&2
        exit 1
    fi

    VCPKG_ROOT="$PWD/../../../../vcpkg"
    CMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CMAKE_TOOLCHAIN_FILE=$(echo "$CMAKE_TOOLCHAIN_FILE" | sed 's|^/c/|C:/|' | sed 's|/|\\|g')
    VCPKG_TARGET_TRIPLET="x64-windows"

    git -C "$VCPKG_ROOT" pull

    "$VCPKG_ROOT/vcpkg" install giflib jbigkit libavif[aom] libheif libjpeg-turbo \
                                libjxl libpng libwebp nanosvg openjpeg tiff zlib  \
                                --triplet x64-windows --clean-buildtrees-after-build --clean-downloads-after-build
    "$VCPKG_ROOT/vcpkg" remove openexr # compilation errors

    # Used by setup.py
    export CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET_TRIPLET -DCMAKE_C_FLAGS=-MP -DCMAKE_CXX_FLAGS=-MP"
fi

source_venv()
{
    local temp_venv="$1"

    if [ -f "$temp_venv/Scripts/activate" ]; then
        source "$temp_venv/Scripts/activate"
    else
        source "$temp_venv/bin/activate"
    fi
}

cleanup_build_files()
{
    rm -rf dist/ build/ build-python/ *.egg-info sailpy/*.so sailpy/*.dylib sailpy/*.pyd sailpy/*.dll
}

# Function to build wheel for a specific Python version
build_wheel_for_version()
{
    local python_version=$1
    local python_executable=$2

    cleanup_build_files

    local temp_venv="venv-$python_version"
    rm -rf "$temp_venv"
    "$python_executable" -m venv "$temp_venv"

    source_venv "$temp_venv"

    python -m pip install --upgrade pip
    pip install --upgrade build twine wheel setuptools pybind11 cmake numpy

    if [[ "$OS_TYPE" == "Linux" ]]; then
        pip install auditwheel patchelf
    fi

    # Set Python executable for CMake
    export CMAKE_ARGS="${CMAKE_ARGS:-} -DPYTHON_EXECUTABLE=$python_executable"

    echo "Building wheel..."
    python -m build --wheel

    # Check the wheel was built against the right Python version
    #
    local python_version_no_dots="$(echo "$python_version" | sed 's/\.//g')"
    for wheel in dist/*.whl; do
        echo "Checking: $wheel"

        local unpacked="${wheel}-unpacked"
        wheel unpack --dest "$unpacked" "$wheel"

        local found="$(find "$unpacked" -name "_libsail*${python_version_no_dots}*" -print)"

        if [ -z "$found" ]; then
            echo "Error: Expected _libsail*${python_version_no_dots}* file not found" >&2
            exit 1
        fi
    done

    if [[ "$OS_TYPE" == "Linux" ]]; then
        for wheel in dist/*.whl; do
            echo "Processing: $wheel"
            auditwheel repair "$wheel" -w wheelhouse/
        done
    else
        mkdir -p wheelhouse
        mv dist/*.whl wheelhouse/
    fi

    deactivate

    rm -rf "$temp_venv"
    cleanup_build_files

    echo
    echo "============================"
    echo " ✓ Built for Python $python_version ✓"
    echo "============================"
    echo
}

# Clean previous builds
echo "[2/4] Cleaning previous builds..."

rm -rf wheelhouse
mkdir -p wheelhouse

echo "[3/4] Building wheels..."

for version in "${PYTHON_VERSIONS[@]}"; do
    if ! pyenv versions | grep -q "$version"; then
        echo
        echo "============================"
        echo "  Building for Python $version"
        echo "============================"
        echo

        echo "Installing Python $version with pyenv..."
        pyenv install --skip-existing "$version" || {
            echo "Error: Failed to install Python $version" >&2
            exit 1
        }
    fi

    pyenv local "$version"

    python_executable="$(pyenv which python)"
    build_wheel_for_version "$version" "$python_executable"
done

echo "[4/4] Checking all wheels..."

echo "Creating temporary venv for checking..."
pyenv local
python -m venv "venv-check"
source_venv "venv-check"
pip install --upgrade pip twine > /dev/null

for wheel in wheelhouse/*.whl; do
    echo "Checking: $wheel"
    twine check --strict "$wheel"
done

deactivate
rm -rf "venv-check"

echo
echo "====================================="
echo " ✓  Wheels ready for distribution  ✓ "
echo "====================================="
echo

echo "Wheels location: wheelhouse/"
ls -1 wheelhouse/*.whl

echo
echo "Next steps:"
echo "  Upload to Test PyPI:    twine upload --repository testpypi wheelhouse/*.whl"
echo "  Install from Test PyPI: pip install --index-url https://test.pypi.org/simple sailpy"
echo
echo "  Upload to PyPI:    twine upload wheelhouse/*.whl"
echo "  Install from PyPI: pip install --index-url https://pypi.org/simple sailpy"
echo
