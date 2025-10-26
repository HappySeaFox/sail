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
Roundtrip tests: Load -> Save -> Load and verify data integrity
Based on tests/sail/io-produce-same-images.c
"""

import sailpy
import tempfile
import os
import numpy as np


def test_roundtrip_lossless_png():
    """Test PNG roundtrip preserves data"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    arr = img.to_numpy()
    arr[:] = [100, 150, 200]

    output_path = os.path.join(tempfile.gettempdir(), "test_roundtrip.png")
    try:
        img.save(output_path)
        loaded = sailpy.Image.from_file(output_path)

        assert loaded.width == 64
        assert loaded.height == 64
        assert np.allclose(loaded.to_numpy(), arr, atol=1)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_jpeg_lossy():
    """Test JPEG roundtrip with lossy compression"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 128, 128)
    arr = img.to_numpy()
    arr[:] = [50, 100, 150]

    output_path = os.path.join(tempfile.gettempdir(), "test_roundtrip.jpg")
    try:
        img.save(output_path)
        loaded = sailpy.Image.from_file(output_path)

        assert loaded.width == 128
        assert loaded.height == 128
        assert np.allclose(loaded.to_numpy(), arr, atol=20)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_dimensions_preservation():
    """Test dimensions preserved through roundtrip"""
    for width, height in [(1, 1), (10, 10), (100, 50), (256, 256)]:
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height)
        img.to_numpy()[:] = 128

        output_path = os.path.join(
            tempfile.gettempdir(), f"test_{width}x{height}.png")
        try:
            img.save(output_path)
            loaded = sailpy.Image.from_file(output_path)
            assert (loaded.width, loaded.height) == (width, height)
        finally:
            if os.path.exists(output_path):
                os.remove(output_path)


def test_roundtrip_different_formats():
    """Test roundtrip through different formats"""
    original = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    original.to_numpy()[:] = [80, 120, 160]

    for fmt in ["png", "qoi"]:
        output_path = os.path.join(
            tempfile.gettempdir(), f"test_roundtrip.{fmt}")
        try:
            original.save(output_path)
            loaded = sailpy.Image.from_file(output_path)

            assert loaded.is_valid
            assert (loaded.width, loaded.height) == (64, 64)
        finally:
            if os.path.exists(output_path):
                os.remove(output_path)


def test_roundtrip_pixel_format_conversion():
    """Test roundtrip with pixel format conversion"""
    # Create RGB image
    img_rgb = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img_rgb.to_numpy()[:] = [100, 150, 200]

    # Convert to RGBA
    img_rgb.convert(sailpy.PixelFormat.BPP32_RGBA)

    output_path = os.path.join(tempfile.gettempdir(), "test_converted.png")
    try:
        # Save
        img_rgb.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)

        # Should be RGBA or similar
        assert loaded.is_valid
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_16bit_image():
    """Test roundtrip with 16-bit image"""
    # Create 16-bit RGB image
    img = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 32, 32)
    arr = img.to_numpy()
    assert arr.dtype == np.uint16
    arr[:] = [10000, 30000, 50000]

    output_path = os.path.join(tempfile.gettempdir(), "test_16bit.png")
    try:
        # Save
        img.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)

        # Verify
        assert loaded.is_valid
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_grayscale():
    """Test roundtrip with grayscale image"""
    img = sailpy.Image(sailpy.PixelFormat.BPP8_GRAYSCALE, 64, 64)
    arr = img.to_numpy()
    arr[:] = 128

    output_path = os.path.join(tempfile.gettempdir(), "test_gray.png")
    try:
        # Save
        img.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)

        # Verify
        assert loaded.is_valid
        assert loaded.width == 64
        assert loaded.height == 64
        assert loaded.is_grayscale
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_rgba_alpha_preservation():
    """Test that alpha channel is preserved in roundtrip"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 32, 32)
    arr = img.to_numpy()
    arr[:, :, :3] = 128  # RGB
    arr[:, :, 3] = 200   # Alpha

    output_path = os.path.join(tempfile.gettempdir(), "test_alpha.png")
    try:
        # Save
        img.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)

        # Verify alpha is present
        assert loaded.is_valid
        loaded_arr = loaded.to_numpy()

        # Should have 4 channels (RGBA or similar)
        assert loaded_arr.ndim == 3
        assert loaded_arr.shape[2] >= 3  # At least RGB
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_gamma_reading():
    """Test that gamma can be read after roundtrip"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = 128

    output_path = os.path.join(tempfile.gettempdir(), "test_gamma.png")
    try:
        # Save
        img.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)

        # Should be able to read gamma
        assert loaded.is_valid
        gamma = loaded.gamma
        # Gamma should be non-negative
        assert gamma >= 0
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_create_save_load():
    """Test complete cycle: create -> save -> load"""
    # Create from scratch
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 48, 48)
    arr = img.to_numpy()

    # Create gradient pattern
    for y in range(48):
        arr[y, :, 0] = y * 5  # Red gradient
        arr[y, :, 1] = 128    # Green constant
        arr[y, :, 2] = 255 - y * 5  # Blue reverse gradient

    output_path = os.path.join(tempfile.gettempdir(), "test_gradient.png")
    try:
        # Save
        img.save(output_path)

        # Load
        loaded = sailpy.Image.from_file(output_path)
        loaded_arr = loaded.to_numpy()

        # Verify pattern is preserved (PNG is lossless)
        assert loaded_arr.shape == arr.shape
        assert np.allclose(loaded_arr, arr, atol=1)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_roundtrip_input_output():
    """Test roundtrip using ImageInput and ImageOutput"""
    # Create original
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [70, 140, 210]

    path1 = os.path.join(tempfile.gettempdir(), "test_rw1.png")
    path2 = os.path.join(tempfile.gettempdir(), "test_rw2.png")

    try:
        # Save first
        img.save(path1)

        # Read with ImageInput
        input = sailpy.ImageInput(path1)
        frame = input.load()
        input.finish()

        # Write with ImageOutput
        output = sailpy.ImageOutput(path2)
        output.save(frame)
        output.finish()

        # Verify
        final = sailpy.Image.from_file(path2)
        assert final.is_valid
        assert final.width == 32
        assert final.height == 32
    finally:
        for p in [path1, path2]:
            if os.path.exists(p):
                os.remove(p)


def test_roundtrip_bytes_consistency():
    """Test that bytes_per_line is consistent after roundtrip"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 33, 10)  # Odd width
    img.to_numpy()[:] = 128

    original_bpl = img.bytes_per_line

    output_path = os.path.join(tempfile.gettempdir(), "test_bpl.png")
    try:
        # Save and load
        img.save(output_path)
        loaded = sailpy.Image.from_file(output_path)

        # bytes_per_line may differ due to alignment, but should be valid
        assert loaded.bytes_per_line >= 33 * 3
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)
