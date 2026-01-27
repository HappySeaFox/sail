
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
Basic tests for SAIL Python bindings
"""

import numpy as np
import sailpy


# Removed: version() and supported_formats() - not part of Pythonic API


def test_pixel_format_enum():
    """Test PixelFormat enumeration"""
    assert hasattr(sailpy, "PixelFormat")
    assert hasattr(sailpy.PixelFormat, "BPP24_RGB")
    assert hasattr(sailpy.PixelFormat, "BPP32_RGBA")


def test_image_creation():
    """Test creating an empty image"""
    width, height = 100, 100
    image = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height)

    assert image.is_valid
    assert image.width == width
    assert image.height == height
    assert image.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert image.bits_per_pixel == 24


def test_image_numpy_integration():
    """Test NumPy integration"""
    width, height = 50, 50
    image = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height)

    # Get NumPy array
    arr = image.to_numpy()
    assert isinstance(arr, np.ndarray)
    assert arr.shape == (height, width, 3)
    assert arr.dtype == np.uint8

    # Fill with data
    arr[:] = [255, 0, 0]  # Red

    # Create from NumPy
    new_image = sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)
    assert new_image.is_valid
    assert new_image.width == width
    assert new_image.height == height


def test_image_properties():
    """Test image property checks"""
    rgb_image = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    assert rgb_image.is_rgb_family
    assert not rgb_image.is_grayscale
    assert not rgb_image.is_indexed
    assert not rgb_image.has_alpha

    rgba_image = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 10, 10)
    assert rgba_image.is_rgb_family
    assert not rgba_image.is_grayscale
    assert not rgba_image.is_indexed
    assert rgba_image.has_alpha

    gray_image = sailpy.Image(sailpy.PixelFormat.BPP8_GRAYSCALE, 10, 10)
    assert not gray_image.is_rgb_family
    assert gray_image.is_grayscale
    assert not gray_image.is_indexed
    assert not gray_image.has_alpha

    gray_alpha_image = sailpy.Image(sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA, 10, 10)
    assert not gray_alpha_image.is_rgb_family
    assert gray_alpha_image.is_grayscale
    assert not gray_alpha_image.is_indexed
    assert gray_alpha_image.has_alpha


def test_static_methods():
    """Test static helper methods"""
    # Test get_bits_per_pixel
    assert sailpy.Image.get_bits_per_pixel(sailpy.PixelFormat.BPP24_RGB) == 24
    assert sailpy.Image.get_bits_per_pixel(sailpy.PixelFormat.BPP32_RGBA) == 32

    # Test calculate_bytes_per_line
    bpl = sailpy.Image.calculate_bytes_per_line(
        100, sailpy.PixelFormat.BPP24_RGB)
    assert bpl == 300  # 100 pixels * 3 bytes

    # Test check_conversion
    assert sailpy.Image.check_conversion(
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP32_RGBA
    )


def test_enum_serialization():
    """Test enum .name and .from_string()"""
    # PixelFormat
    pf = sailpy.PixelFormat.BPP24_RGB
    assert pf.name == "BPP24_RGB"
    assert sailpy.PixelFormat.from_string("BPP24-RGB") == sailpy.PixelFormat.BPP24_RGB

    # Compression
    comp = sailpy.Compression.JPEG
    assert comp.name == "JPEG"
    assert sailpy.Compression.from_string("JPEG") == sailpy.Compression.JPEG

    # Orientation
    orient = sailpy.Orientation.ROTATED_90
    assert orient.name == "ROTATED_90"
    assert sailpy.Orientation.from_string("ROTATED-90") == sailpy.Orientation.ROTATED_90

    # ChromaSubsampling
    cs = sailpy.ChromaSubsampling.C420
    assert cs.name == "C420"
    assert sailpy.ChromaSubsampling.from_string("420") == sailpy.ChromaSubsampling.C420


def test_enum_standard_methods():
    """Test standard Python enum methods"""
    pf = sailpy.PixelFormat.BPP32_RGBA
    assert str(pf) == "PixelFormat.BPP32_RGBA"
    assert isinstance(pf.value, int)  # Has numeric value
    assert isinstance(pf.name, str)
    assert pf.name == "BPP32_RGBA"


def test_load_png(test_png):
    """Test loading PNG image"""
    image = sailpy.Image.from_file(str(test_png))
    assert image.is_valid
    assert image.width == 16
    assert image.height == 16


def test_load_jpeg(test_jpeg):
    """Test loading JPEG image"""
    image = sailpy.Image.from_file(str(test_jpeg))
    assert image.is_valid
    assert image.width == 32
    assert image.height == 32
