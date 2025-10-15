
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
Tests for various image format codecs
"""

import pytest
import sailpy
import tempfile
import os


# Formats that reliably support both loading and saving
# Note: Each codec has specific supported pixel formats
ROUNDTRIP_FORMATS = [
    ("png", sailpy.PixelFormat.BPP24_RGB),
    ("png", sailpy.PixelFormat.BPP32_RGBA),
    ("qoi", sailpy.PixelFormat.BPP24_RGB),
    ("qoi", sailpy.PixelFormat.BPP32_RGBA),
]


@pytest.mark.parametrize("extension,pixel_format", ROUNDTRIP_FORMATS)
def test_format_roundtrip(extension, pixel_format):
    """Test save and load roundtrip for various formats"""
    # Get codec info
    codec = sailpy.CodecInfo.from_extension(extension)

    if not codec.is_valid or not codec.can_save:
        pytest.skip(f"Codec {extension} doesn't support saving")

    # Create image
    img = sailpy.Image(pixel_format, 32, 32)
    arr = img.to_numpy()
    arr[:] = 123

    # Save
    output_path = os.path.join(
        tempfile.gettempdir(), f"test_roundtrip.{extension}")

    try:
        # Try to save, some codecs may not support this pixel format
        try:
            img.save(output_path)
        except RuntimeError as e:
            if "unsupported" in str(e).lower() or "pixel" in str(e).lower():
                pytest.skip(f"Codec {extension} doesn't support {
                            pixel_format}")
            raise

        # Load
        loaded = sailpy.Image.from_file(output_path)

        assert loaded.is_valid
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_png_format():
    """Test PNG format specifically"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = [100, 150, 200]

    output_path = os.path.join(tempfile.gettempdir(), "test.png")

    try:
        img.save(output_path)

        # Verify codec
        codec = sailpy.CodecInfo.from_path(output_path)
        assert codec.name == "PNG"

        # Load back
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 64
        assert loaded.height == 64
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_jpeg_format():
    """Test JPEG format (lossy)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 128, 128)
    img.to_numpy()[:] = [200, 100, 50]

    output_path = os.path.join(tempfile.gettempdir(), "test.jpg")

    try:
        img.save(output_path)

        # Verify codec
        codec = sailpy.CodecInfo.from_path(output_path)
        assert codec.name == "JPEG"

        # Load back (values may differ due to compression)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 128
        assert loaded.height == 128
        assert loaded.pixel_format in [
            sailpy.PixelFormat.BPP24_RGB,
            sailpy.PixelFormat.BPP24_YCBCR
        ]
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_webp_format():
    """Test WebP format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = [150, 200, 100]

    output_path = os.path.join(tempfile.gettempdir(), "test.webp")

    try:
        img.save(output_path)

        # Verify codec
        codec = sailpy.CodecInfo.from_path(output_path)
        assert codec.name == "WEBP"

        # Load back
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 64
        assert loaded.height == 64
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_bmp_format():
    """Test BMP format"""
    # BMP has specific supported formats, use BPP32_BGRA
    img = sailpy.Image(sailpy.PixelFormat.BPP32_BGRA, 48, 48)
    arr = img.to_numpy()
    arr[:] = [255, 128, 0, 255]

    output_path = os.path.join(tempfile.gettempdir(), "test.bmp")

    try:
        img.save(output_path)

        # Load back
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 48
        assert loaded.height == 48
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_qoi_format():
    """Test QOI (Quite OK Image) format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 32, 32)
    arr = img.to_numpy()
    arr[:] = [200, 100, 50, 255]

    output_path = os.path.join(tempfile.gettempdir(), "test.qoi")

    try:
        img.save(output_path)

        # Load back
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_format_detection_by_extension():
    """Test that format is correctly detected by file extension"""
    formats_to_test = [
        ("test.png", "PNG"),
        ("test.jpg", "JPEG"),
        ("test.webp", "WEBP"),
        ("test.bmp", "BMP"),
        ("test.tga", "TGA"),
    ]

    for path, expected_codec in formats_to_test:
        codec = sailpy.CodecInfo.from_path(path)
        assert codec.is_valid
        assert codec.name == expected_codec


def test_all_codecs_have_extensions():
    """Test that all codecs have at least one extension"""
    codecs = sailpy.list_codecs()

    for codec in codecs:
        assert len(codec.extensions) > 0, f"Codec {
            codec.name} has no extensions"


def test_save_to_different_formats():
    """Test saving same image to different formats"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = 128

    # Use formats that support BPP24_RGB
    formats = ["png", "qoi"]

    for ext in formats:
        output_path = os.path.join(
            tempfile.gettempdir(), f"multi_format.{ext}")
        try:
            img.save(output_path)
            assert os.path.exists(output_path)

            # Verify can be loaded back
            loaded = sailpy.Image.from_file(output_path)
            assert loaded.is_valid
        finally:
            if os.path.exists(output_path):
                os.remove(output_path)


def test_rgba_formats_preserve_alpha():
    """Test that RGBA formats handle alpha channel"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 16, 16)
    arr = img.to_numpy()

    # Set specific alpha values
    arr[:, :, 0:3] = 200  # RGB
    arr[:, :, 3] = 128  # Alpha = 50% transparent

    output_path = os.path.join(tempfile.gettempdir(), "test_alpha.png")

    try:
        img.save(output_path)

        # Load back
        loaded = sailpy.Image.from_file(output_path)
        loaded_arr = loaded.to_numpy()

        # PNG preserves alpha
        assert loaded_arr.shape[2] == 4  # Has alpha channel
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_format_conversion():
    """Test converting between pixel formats"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [100, 150, 200]

    # Convert to RGBA
    img.convert(sailpy.PixelFormat.BPP32_RGBA)

    assert img.pixel_format == sailpy.PixelFormat.BPP32_RGBA
    assert img.width == 32
    assert img.height == 32

    arr = img.to_numpy()
    assert arr.shape == (32, 32, 4)
