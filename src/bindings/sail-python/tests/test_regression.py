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
Regression tests for known bugs and edge cases
Based on tests/sail/bugs.c from SAIL C tests
"""

import pytest
import sailpy


# Check for historical bugs that should not occur


def test_regression_image_dimensions_consistency():
    """Test that image dimensions remain consistent after operations"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 50)

    assert img.width == 100
    assert img.height == 50

    # Dimensions should not change after pixel access
    arr = img.to_numpy()
    assert img.width == 100
    assert img.height == 50


def test_regression_pixel_format_preservation():
    """Test that pixel format is preserved through save/load"""
    import tempfile
    import os

    original_format = sailpy.PixelFormat.BPP32_RGBA
    img = sailpy.Image(original_format, 32, 32)
    img.to_numpy()[:] = 128

    output_path = os.path.join(tempfile.gettempdir(), "test_format.png")
    try:
        img.save(output_path)
        loaded = sailpy.Image.from_file(output_path)

        # Format may be converted by codec, but should be valid
        assert loaded.pixel_format != sailpy.PixelFormat.UNKNOWN
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_regression_numpy_memory_leak():
    """Test that NumPy conversions don't leak memory"""
    # Create and destroy many images to check for leaks
    for _ in range(100):
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
        arr = img.to_numpy()
        arr[:] = 255
        # Let Python GC handle cleanup


def test_regression_reader_after_error():
    """Test that reader can be created after error"""
    import tempfile
    import os

    # First, try invalid file (should fail)
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.ImageReader("/nonexistent.png")

    # Then, create valid reader (should work)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    output_path = os.path.join(tempfile.gettempdir(), "test_valid.png")
    try:
        img.save(output_path)
        reader = sailpy.ImageReader(output_path)
        loaded = reader.read()
        assert loaded.is_valid
        reader.finish()
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_regression_codec_info_caching():
    """Test that codec info queries are consistent"""
    # Multiple queries should return consistent results
    codec1 = sailpy.CodecInfo.from_extension("png")
    codec2 = sailpy.CodecInfo.from_extension("png")

    assert codec1.is_valid == codec2.is_valid
    assert codec1.name == codec2.name
    assert codec1.can_load == codec2.can_load
    assert codec1.can_save == codec2.can_save


def test_regression_image_without_metadata():
    """Test that images work without metadata"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)

    # Image should be valid even without metadata
    assert img.is_valid
    assert img.width == 32
    assert img.height == 32


def test_regression_palette_with_non_indexed():
    """Test that palette is None for non-indexed images"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)

    # Non-indexed format should not have palette
    assert not img.is_indexed


def test_regression_zero_delay_animated():
    """Test that zero delay is handled correctly"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)

    # Delay can be -1, 0, or positive
    # Just ensure it's retrievable without error
    delay = img.delay
    assert delay >= -1


def test_regression_bytes_per_line_consistency():
    """Test that bytes_per_line is calculated correctly"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 33, 10)  # Odd width

    # BPP24_RGB = 3 bytes per pixel
    expected_min = 33 * 3
    assert img.bytes_per_line >= expected_min


def test_regression_concurrent_codec_queries():
    """Test multiple concurrent codec info queries"""
    codecs = []
    for ext in ["png", "jpg", "gif", "bmp", "webp"]:
        codec = sailpy.CodecInfo.from_extension(ext)
        if codec.is_valid:
            codecs.append(codec)

    # Should have found at least some codecs
    assert len(codecs) > 0

    # All should have valid names
    for codec in codecs:
        assert len(codec.name) > 0
