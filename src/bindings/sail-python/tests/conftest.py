
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

"""
Pytest configuration for SAIL Python bindings tests

Provides global test image paths and fixtures to avoid hardcoded ../../../ paths.

Usage:
    def test_my_feature(test_png):
        img = sailpy.Image.from_file(str(test_png))
        assert img.is_valid

    def test_custom_path(test_images_dir):
        webp_path = test_images_dir / "webp" / "example.webp"
        img = sailpy.Image.from_file(str(webp_path))
"""

import sys
from pathlib import Path
import pytest

# Add python directory to path for importing sailpy
python_dir = Path(__file__).parent.parent
sys.path.insert(0, str(python_dir))

# Base directory for test images
# __file__ is in sail/src/bindings/python/tests/conftest.py
# We need sail/tests/images/acceptance
TEST_IMAGES_DIR = Path(__file__).parent.parent.parent.parent.parent / "tests" / "images" / "acceptance"


@pytest.fixture
def test_images_dir():
    """Fixture providing the base directory for test images"""
    return TEST_IMAGES_DIR


@pytest.fixture
def test_png():
    """Fixture providing path to test PNG image"""
    return TEST_IMAGES_DIR / "png" / "bpp32-rgba-animated.png"


@pytest.fixture
def test_jpeg():
    """Fixture providing path to test JPEG image"""
    return TEST_IMAGES_DIR / "jpeg" / "bpp24-ycbcr.comment.iccp.jpeg"
