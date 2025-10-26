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
Tests for Image.to_bytes() method using io_expanding_buffer
"""

import pytest
import sailpy
import numpy as np


def test_to_bytes_png():
    """Test saving image to PNG bytes"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [100, 150, 200]

    data = img.to_bytes("png")

    assert isinstance(data, bytes)
    assert len(data) > 0
    assert data[:8] == b'\x89PNG\r\n\x1a\n'  # PNG magic number

    # Decode back from bytes
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert decoded.width == 32
    assert decoded.height == 32
    assert np.allclose(decoded.to_numpy(), img.to_numpy(), atol=1)


def test_to_bytes_jpeg():
    """Test saving image to JPEG bytes"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = [80, 120, 160]

    data = img.to_bytes("jpg")

    assert isinstance(data, bytes)
    assert len(data) > 0
    assert data[:2] == b'\xff\xd8'  # JPEG magic number

    # Decode back from bytes (JPEG is lossy, so use larger tolerance)
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert decoded.width == 64
    assert decoded.height == 64
    assert np.allclose(decoded.to_numpy(), img.to_numpy(), atol=10)


def test_to_bytes_default_format():
    """Test to_bytes with default format (PNG)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img.to_numpy()[:] = 128

    data = img.to_bytes()  # Default to PNG

    assert isinstance(data, bytes)
    assert data[:8] == b'\x89PNG\r\n\x1a\n'

    # Decode back from bytes
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert (decoded.width, decoded.height) == (16, 16)


def test_to_bytes_roundtrip():
    """Test to_bytes -> from_bytes roundtrip"""
    original = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 48, 48)
    arr = original.to_numpy()
    arr[:] = [50, 100, 150]

    data = original.to_bytes("png")

    loaded = sailpy.Image.from_bytes(data)

    assert loaded.is_valid
    assert (loaded.width, loaded.height) == (48, 48)
    assert np.allclose(loaded.to_numpy(), arr, atol=1)


def test_to_bytes_different_formats():
    """Test to_bytes with different formats and decode them back"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [70, 140, 210]

    formats = {
        "png": b'\x89PNG',
        "qoi": b'qoif',
    }

    for fmt, magic in formats.items():
        data = img.to_bytes(fmt)
        assert isinstance(data, bytes)
        assert len(data) > 0
        assert data[:len(magic)] == magic

        # Decode back from bytes
        decoded = sailpy.Image.from_bytes(data)
        assert decoded.is_valid
        assert (decoded.width, decoded.height) == (32, 32)
        assert np.allclose(decoded.to_numpy(), img.to_numpy(), atol=1)


def test_to_bytes_rgba():
    """Test to_bytes with RGBA image"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 32, 32)
    arr = img.to_numpy()
    arr[:, :, :3] = [100, 150, 200]
    arr[:, :, 3] = 255

    data = img.to_bytes("png")

    assert isinstance(data, bytes)
    assert len(data) > 0

    # Decode back from bytes
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert (decoded.width, decoded.height) == (32, 32)
    assert np.allclose(decoded.to_numpy(), arr, atol=1)


def test_to_bytes_grayscale():
    """Test to_bytes with grayscale image"""
    img = sailpy.Image(sailpy.PixelFormat.BPP8_GRAYSCALE, 64, 64)
    img.to_numpy()[:] = 128

    data = img.to_bytes("png")

    assert isinstance(data, bytes)
    assert len(data) > 0

    # Decode back from bytes
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert (decoded.width, decoded.height) == (64, 64)
    assert np.allclose(decoded.to_numpy(), 128, atol=1)


def test_to_bytes_16bit():
    """Test to_bytes with 16-bit image"""
    img = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 32, 32)
    arr = img.to_numpy()
    assert arr.dtype == np.uint16
    arr[:] = [10000, 30000, 50000]

    data = img.to_bytes("png")

    assert isinstance(data, bytes)
    assert len(data) > 0

    # Decode back from bytes
    decoded = sailpy.Image.from_bytes(data)
    assert decoded.is_valid
    assert decoded.to_numpy().dtype == np.uint16
    assert (decoded.width, decoded.height) == (32, 32)
    assert np.allclose(decoded.to_numpy(), arr, atol=1)


def test_to_bytes_various_sizes():
    """Test to_bytes with various image sizes"""
    sizes = [(1, 1), (10, 10), (100, 100), (256, 256), (512, 512)]

    for width, height in sizes:
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height)
        img.to_numpy()[:] = 128

        data = img.to_bytes("png")

        assert isinstance(data, bytes)
        assert len(data) > 0


def test_to_bytes_invalid_format():
    """Test to_bytes with invalid format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img.to_numpy()[:] = 128

    with pytest.raises(RuntimeError, match="Unknown format"):
        img.to_bytes("unknownformat")


def test_to_bytes_expanding_buffer_efficiency():
    """Test that expanding buffer scales with image size"""
    # Small image should produce small bytes
    small = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 8, 8)
    small.to_numpy()[:] = 100
    small_data = small.to_bytes("png")

    # Large image should produce larger bytes
    large = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    large.to_numpy()[:] = 100
    large_data = large.to_bytes("png")

    # Large should be bigger (but not necessarily 10x due to compression)
    assert len(large_data) > len(small_data)
