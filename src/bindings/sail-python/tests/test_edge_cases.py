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
Edge cases and error handling tests for SAIL Python bindings
"""

import pytest
import sailpy
import tempfile
import os


def test_edge_nonexistent_file():
    """Test loading from nonexistent file"""
    with pytest.raises(FileNotFoundError):
        sailpy.Image.from_file("/nonexistent/path/image.png")


def test_edge_invalid_path():
    """Test loading from invalid path"""
    with pytest.raises(FileNotFoundError):
        sailpy.Image.from_file("")


def test_edge_null_bytes():
    """Test loading from empty bytes"""
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.Image.from_bytes(b"")


def test_edge_invalid_bytes():
    """Test loading from invalid image data"""
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.Image.from_bytes(b"not an image")


def test_edge_truncated_data():
    """Test loading truncated image data"""
    # Valid PNG header but truncated
    png_header = b"\x89PNG\r\n\x1a\n"
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.Image.from_bytes(png_header)


def test_edge_zero_dimensions():
    """Test creating image with zero dimensions"""
    # Zero dimensions should raise ValueError
    with pytest.raises(ValueError, match="Invalid image dimensions"):
        sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 0, 0)


def test_edge_zero_width():
    """Test creating image with zero width"""
    # Zero width should raise ValueError
    with pytest.raises(ValueError, match="Invalid image dimensions"):
        sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 0, 10)


def test_edge_zero_height():
    """Test creating image with zero height"""
    # Zero height should raise ValueError
    with pytest.raises(ValueError, match="Invalid image dimensions"):
        sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 0)


def test_edge_huge_dimensions():
    """Test creating image with unreasonably large dimensions"""
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 1000000, 1000000)


def test_edge_invalid_pixel_format():
    """Test creating image with invalid pixel format"""
    # PixelFormat.UNKNOWN should raise ValueError
    with pytest.raises(ValueError, match="Invalid image dimensions"):
        sailpy.Image(sailpy.PixelFormat.UNKNOWN, 10, 10)


def test_edge_save_with_initialized_pixels():
    """Test saving image with initialized pixels"""
    output_path = os.path.join(tempfile.gettempdir(), "test_valid.png")
    try:
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
        img.to_numpy()[:] = 128  # Initialize
        img.save(output_path)
        assert os.path.exists(output_path)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_edge_save_to_unavailable_path():
    """Test saving to unavailable location"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    # Use platform-specific unavailable path
    with pytest.raises((RuntimeError, MemoryError, PermissionError, OSError)):
        img.save("/unavailable/path/test.png")


def test_edge_save_to_invalid_extension():
    """Test saving to unknown extension"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    output_path = os.path.join(tempfile.gettempdir(), "test.unknownext")

    with pytest.raises(RuntimeError):
        img.save(output_path)

    # Cleanup if file was created
    if os.path.exists(output_path):
        os.remove(output_path)


def test_edge_input_invalid_file():
    """Test ImageInput with invalid file"""
    with pytest.raises((RuntimeError, MemoryError)):
        sailpy.ImageInput("/nonexistent.png")


def test_edge_output_to_unavailable_path():
    """Test ImageOutput with unavailable path"""
    with pytest.raises((RuntimeError, MemoryError, PermissionError, OSError)):
        sailpy.ImageOutput("/unavailable/path/test.png")


def test_edge_codec_info_invalid_extension():
    """Test getting codec info for invalid extension"""
    with pytest.raises(ValueError, match="No codec found for extension"):
        sailpy.CodecInfo.from_extension("unknownext")


def test_edge_codec_info_empty_extension():
    """Test getting codec info for empty extension"""
    with pytest.raises(ValueError, match="No codec found for extension"):
        sailpy.CodecInfo.from_extension("")


def test_edge_double_finish():
    """Test calling finish() twice on input"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    output_path = os.path.join(tempfile.gettempdir(), "test_double.png")

    try:
        img.save(output_path)
        input = sailpy.ImageInput(output_path)
        input.finish()
        input.finish()  # Second finish should not crash
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_edge_finish_then_read_fails():
    """Test that reading after finish() raises error"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    output_path = os.path.join(tempfile.gettempdir(), "test_after_finish.png")
    try:
        img.save(output_path)

        input = sailpy.ImageInput(output_path)
        frame = input.load()
        assert frame.is_valid

        input.finish()

        # Reading after finish should fail
        with pytest.raises(RuntimeError):
            input.load()
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_edge_numpy_wrong_dtype():
    """Test from_numpy with wrong dtype"""
    import numpy as np
    # Create uint32 array (unsupported)
    arr = np.zeros((10, 10, 3), dtype=np.uint32)

    with pytest.raises((RuntimeError, TypeError)):
        sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)


def test_edge_numpy_wrong_shape():
    """Test from_numpy with wrong shape"""
    import numpy as np
    # 4D array
    arr = np.zeros((10, 10, 3, 1), dtype=np.uint8)

    with pytest.raises(RuntimeError):
        sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)


def test_edge_numpy_1d_array():
    """Test from_numpy with 1D array"""
    import numpy as np
    arr = np.zeros(100, dtype=np.uint8)

    with pytest.raises(RuntimeError):
        sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)


def test_edge_conversion_to_same_format():
    """Test converting to same pixel format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)
    img.to_numpy()[:] = 0

    # Convert to same format should work
    img.convert(sailpy.PixelFormat.BPP24_RGB)
    assert img.pixel_format == sailpy.PixelFormat.BPP24_RGB


def test_edge_metadata_valid_creation():
    """Test creating metadata with valid key"""
    var = sailpy.Variant("value")
    md = sailpy.MetaData("Author", var)
    assert md.key_unknown is not None


def test_edge_variant_type_mismatch():
    """Test variant value retrieval with wrong type"""
    var = sailpy.Variant(42)

    # Check has_string returns False
    assert not var.has_string()
    assert var.has_int()

    # Variant has int, not string
    assert var.is_valid
