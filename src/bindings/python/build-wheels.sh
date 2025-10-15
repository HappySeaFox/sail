#!/bin/bash

# Build wheels for PyPI distribution

set -euo pipefail

cd "$(dirname "$0")"

# Check if we're in a virtual environment
if [ -z "${VIRTUAL_ENV:-}" ]; then
    if [ ! -d "venv" ]; then
        echo "Creating virtual environment..."
        python3 -m venv venv
    fi

    echo "Activating existing virtual environment..."
    source venv/bin/activate
    echo
fi

echo "================================="
echo " Building Python Wheels for PyPI "
echo "================================="
echo
echo "Platform: $(uname -s)"
echo

OS_TYPE=$(uname -s)

echo "[1/5] Cleaning previous builds..."
rm -rf dist/ build/ wheelhouse/ *.egg-info sailpy/*.so sailpy/*.dylib sailpy/*.dll

if [[ "$OS_TYPE" == "Linux" ]]; then
    echo "[2/5] Installing build tools (including auditwheel)..."
    pip install --upgrade build twine auditwheel patchelf
else
    echo "[2/5] Installing build tools..."
    pip install --upgrade build twine
fi

echo "[3/5] Building wheel..."
echo "NOTE: Building wheel only (sdist is not usable for this package)"
python3 -m build --wheel

if [[ "$OS_TYPE" == "Linux" ]]; then
    echo "[4/5] Converting to manylinux (Linux only)..."

    mkdir -p wheelhouse

    for wheel in dist/*.whl; do
        echo "Processing: $wheel"
        auditwheel repair "$wheel" -w wheelhouse/
        echo
    done

    WHEEL_DIR="wheelhouse"
else
    echo "[4/5] Skipping manylinux conversion (not on Linux)"

    WHEEL_DIR="dist"
fi

echo "[5/5] Checking wheel..."
twine check $WHEEL_DIR/*.whl

echo
echo "==================================="
echo " ✓ Wheels ready for distribution ✓ "
echo "==================================="
echo

echo "Wheels location: $WHEEL_DIR/"
ls -1 $WHEEL_DIR/*.whl

echo "Next steps:"
echo "  Upload to Test PyPI:    twine upload --repository testpypi $WHEEL_DIR/*.whl"
echo "  Install from Test PyPI: pip install --index-url https://test.pypi.org/simple sailpy"
echo
echo "  Upload to PyPI:    twine upload $WHEEL_DIR/*.whl"
echo "  Install from PyPI: pip install --index-url https://pypi.org/simple sailpy"
echo
