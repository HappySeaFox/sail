#!/bin/bash

# Build wheels for multiple Python versions using pyenv
# If pyenv is not found, builds for current Python only

set -euo pipefail

cd "$(dirname "$0")"

# Python versions to build for
PYTHON_VERSIONS=("3.9" "3.10" "3.11" "3.12" "3.13" "3.14")

# Setup pyenv if available
MULTI_VERSION=false
if [ -d "$HOME/.pyenv" ]; then
    export PYENV_ROOT="$HOME/.pyenv"
    export PATH="$PYENV_ROOT/bin:$PATH"
    eval "$(pyenv init -)"
    MULTI_VERSION=true
elif command -v pyenv &> /dev/null; then
    eval "$(pyenv init -)"
    MULTI_VERSION=true
fi

echo "====================================="
echo "=  Building Python Wheels for PyPI  ="
echo "====================================="
echo
echo "Platform: $(uname -s)"

if [[ "$MULTI_VERSION" == "true" ]]; then
    echo "Python Manager: pyenv"
    echo "Target versions: ${PYTHON_VERSIONS[*]}"
else
    echo "Mode: Single Python version (current)"
    echo "Python: $(python3 --version)"
    echo
    echo "To build for multiple Python versions, install pyenv:"
    echo "  curl https://pyenv.run | bash"
fi
echo

OS_TYPE=$(uname -s)

# Function to build wheel for a specific Python version
build_wheel_for_version()
{
    local python_version=$1
    local python_executable=$2
    local is_temp_env=$3

    echo "========================================"
    echo "Building for Python $python_version"
    echo "========================================"

    local temp_venv="venv-$python_version"
    rm -rf "$temp_venv"
    $python_executable -m venv "$temp_venv"
    source "$temp_venv/bin/activate"

    pip install --upgrade pip
    pip install --upgrade build twine wheel setuptools pybind11 cmake numpy

    if [[ "$OS_TYPE" == "Linux" ]]; then
        pip install auditwheel patchelf
    fi

    echo "Building wheel..."
    python -m build --wheel

    if [[ "$OS_TYPE" == "Linux" ]]; then
        for wheel in dist/*.whl; do
            echo "Processing: $wheel"
            auditwheel repair "$wheel" -w wheelhouse/
        done
    else
        mkdir -p wheelhouse
        cp dist/*.whl wheelhouse/
    fi

    deactivate
    if [[ "$is_temp_env" == "true" ]]; then
        rm -rf "$temp_venv"
    fi
    rm -rf dist/ build/ *.egg-info sailpy/*.so sailpy/*.dylib sailpy/*.dll

    echo "✓ Completed Python $python_version"
    echo
}

# Clean previous builds
echo "[1/3] Cleaning previous builds..."
rm -rf dist/ build/ wheelhouse/ *.egg-info sailpy/*.so sailpy/*.dylib sailpy/*.dll

mkdir -p wheelhouse

echo "[2/3] Building wheels..."

if [[ "$MULTI_VERSION" == "true" ]]; then
    for version in "${PYTHON_VERSIONS[@]}"; do
        if ! pyenv versions | grep -q "$version"; then
            echo "Installing Python $version with pyenv..."
            pyenv install "$version" || {
                echo "Warning: Failed to install Python $version, skipping..."
                continue
            }
        fi

        python_executable=$(pyenv prefix "$version")/bin/python
        build_wheel_for_version "$version" "$python_executable" "true"
    done
else
    if [ ! -x venv/bin/activate ]; then
        echo "Creating virtual environment..."
        python3 -m venv venv
    fi

    if [ -z "${VIRTUAL_ENV:-}" ]; then
        echo "Activating existing virtual environment..."
        source venv/bin/activate
        echo
    fi

    if [[ "$OS_TYPE" == "Linux" ]]; then
        echo "Installing build tools (including auditwheel)..."
        pip install --upgrade build twine auditwheel patchelf
    else
        echo "Installing build tools..."
        pip install --upgrade build twine
    fi

    echo "Building wheel..."
    echo "NOTE: Building wheel only (sdist is not usable for this package)"
    python3 -m build --wheel

    if [[ "$OS_TYPE" == "Linux" ]]; then
        echo "Converting to manylinux (Linux only)..."
        mkdir -p wheelhouse
        for wheel in dist/*.whl; do
            echo "Processing: $wheel"
            auditwheel repair "$wheel" -w wheelhouse/
            echo
        done
    else
        echo "Skipping manylinux conversion (not on Linux)"
        mkdir -p wheelhouse
        cp dist/*.whl wheelhouse/
    fi
fi

echo "[3/3] Checking all wheels..."
# Create temporary venv for checking if not already in one
if [ -z "${VIRTUAL_ENV:-}" ]; then
    echo "Creating temporary venv for checking..."
    python3 -m venv venv
    source venv/bin/activate
    pip install --upgrade pip twine > /dev/null
fi

for wheel in wheelhouse/*.whl; do
    echo "Checking: $wheel"
    twine check "$wheel"
done

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
