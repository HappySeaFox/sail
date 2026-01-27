"""
Tests for various pixel formats support
"""

import pytest
import sailpy
import numpy as np


# Common 8-bit formats
COMMON_8BIT_FORMATS = [
    (sailpy.PixelFormat.BPP24_RGB, (3,), np.uint8),
    (sailpy.PixelFormat.BPP24_BGR, (3,), np.uint8),
    (sailpy.PixelFormat.BPP32_RGBA, (4,), np.uint8),
    (sailpy.PixelFormat.BPP32_BGRA, (4,), np.uint8),
    (sailpy.PixelFormat.BPP8_GRAYSCALE, (), np.uint8),  # No channel dimension
]

# Common 16-bit formats
COMMON_16BIT_FORMATS = [
    (sailpy.PixelFormat.BPP48_RGB, (3,), np.uint16),
    (sailpy.PixelFormat.BPP48_BGR, (3,), np.uint16),
    (sailpy.PixelFormat.BPP64_RGBA, (4,), np.uint16),
    (sailpy.PixelFormat.BPP64_BGRA, (4,), np.uint16),
    (sailpy.PixelFormat.BPP16_GRAYSCALE, (), np.uint16),  # No channel dimension
]


@pytest.mark.parametrize("pixel_format,channels,expected_dtype", COMMON_8BIT_FORMATS)
def test_8bit_formats(pixel_format, channels, expected_dtype):
    """Test creation and NumPy conversion for common 8-bit formats"""
    img = sailpy.Image(pixel_format, 32, 32)

    assert img.is_valid
    assert img.width == 32
    assert img.height == 32
    assert img.pixel_format == pixel_format

    # Convert to NumPy
    arr = img.to_numpy()
    assert arr.dtype == expected_dtype

    if channels:
        assert arr.shape == (32, 32) + channels
    else:
        assert arr.shape == (32, 32)


@pytest.mark.parametrize("pixel_format,channels,expected_dtype", COMMON_16BIT_FORMATS)
def test_16bit_formats(pixel_format, channels, expected_dtype):
    """Test creation and NumPy conversion for common 16-bit formats"""
    img = sailpy.Image(pixel_format, 32, 32)

    assert img.is_valid
    assert img.width == 32
    assert img.height == 32
    assert img.pixel_format == pixel_format

    # Convert to NumPy
    arr = img.to_numpy()
    assert arr.dtype == expected_dtype

    if channels:
        assert arr.shape == (32, 32) + channels
    else:
        assert arr.shape == (32, 32)


def test_indexed_format():
    """Test indexed pixel format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP8_INDEXED, 32, 32)

    assert img.is_valid
    assert img.is_indexed


def test_rgb_family_detection():
    """Test RGB family detection"""
    rgb_formats = [
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP48_RGB,
        sailpy.PixelFormat.BPP64_RGBA,
    ]

    for pf in rgb_formats:
        img = sailpy.Image(pf, 16, 16)
        assert img.is_rgb_family


def test_grayscale_detection():
    """Test grayscale detection"""
    gray_formats = [
        sailpy.PixelFormat.BPP8_GRAYSCALE,
        sailpy.PixelFormat.BPP16_GRAYSCALE,
    ]

    for pf in gray_formats:
        img = sailpy.Image(pf, 16, 16)
        assert img.is_grayscale


def test_has_alpha_detection():
    """Test alpha channel detection"""
    alpha_formats = [
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP32_BGRA,
        sailpy.PixelFormat.BPP64_RGBA,
        sailpy.PixelFormat.BPP64_BGRA,
        sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA,
        sailpy.PixelFormat.BPP16_GRAYSCALE_ALPHA,
    ]

    for pf in alpha_formats:
        img = sailpy.Image(pf, 16, 16)
        assert img.has_alpha

    # Formats without alpha
    no_alpha_formats = [
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP24_BGR,
        sailpy.PixelFormat.BPP8_GRAYSCALE,
        sailpy.PixelFormat.BPP16_GRAYSCALE,
    ]

    for pf in no_alpha_formats:
        img = sailpy.Image(pf, 16, 16)
        assert not img.has_alpha


def test_bits_per_pixel_calculation():
    """Test bits per pixel for various formats"""
    test_cases = [
        (sailpy.PixelFormat.BPP24_RGB, 24),
        (sailpy.PixelFormat.BPP32_RGBA, 32),
        (sailpy.PixelFormat.BPP48_RGB, 48),
        (sailpy.PixelFormat.BPP64_RGBA, 64),
        (sailpy.PixelFormat.BPP8_GRAYSCALE, 8),
        (sailpy.PixelFormat.BPP16_GRAYSCALE, 16),
    ]

    for pixel_format, expected_bpp in test_cases:
        img = sailpy.Image(pixel_format, 16, 16)
        assert img.bits_per_pixel == expected_bpp


def test_bytes_per_line():
    """Test bytes per line calculation"""
    # 24-bit RGB: 256 pixels × 3 bytes = 768 bytes
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    assert img.bytes_per_line == 256 * 3

    # 48-bit RGB: 256 pixels × 6 bytes = 1536 bytes
    img16 = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 256, 256)
    assert img16.bytes_per_line == 256 * 6


def test_pixel_format_properties():
    """Test pixel format property queries"""
    # RGB
    assert sailpy.Image.check_rgb_family(sailpy.PixelFormat.BPP24_RGB)
    assert not sailpy.Image.check_grayscale(sailpy.PixelFormat.BPP24_RGB)
    assert not sailpy.Image.check_indexed(sailpy.PixelFormat.BPP24_RGB)
    assert not sailpy.Image.check_has_alpha(sailpy.PixelFormat.BPP24_RGB)

    # RGBA (RGB with alpha)
    assert sailpy.Image.check_rgb_family(sailpy.PixelFormat.BPP32_RGBA)
    assert not sailpy.Image.check_grayscale(sailpy.PixelFormat.BPP32_RGBA)
    assert not sailpy.Image.check_indexed(sailpy.PixelFormat.BPP32_RGBA)
    assert sailpy.Image.check_has_alpha(sailpy.PixelFormat.BPP32_RGBA)

    # Grayscale
    assert not sailpy.Image.check_rgb_family(sailpy.PixelFormat.BPP8_GRAYSCALE)
    assert sailpy.Image.check_grayscale(sailpy.PixelFormat.BPP8_GRAYSCALE)
    assert not sailpy.Image.check_indexed(sailpy.PixelFormat.BPP8_GRAYSCALE)
    assert not sailpy.Image.check_has_alpha(sailpy.PixelFormat.BPP8_GRAYSCALE)

    # Grayscale with alpha
    assert not sailpy.Image.check_rgb_family(sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA)
    assert sailpy.Image.check_grayscale(sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA)
    assert not sailpy.Image.check_indexed(sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA)
    assert sailpy.Image.check_has_alpha(sailpy.PixelFormat.BPP8_GRAYSCALE_ALPHA)

    # Indexed
    assert not sailpy.Image.check_rgb_family(sailpy.PixelFormat.BPP8_INDEXED)
    assert not sailpy.Image.check_grayscale(sailpy.PixelFormat.BPP8_INDEXED)
    assert sailpy.Image.check_indexed(sailpy.PixelFormat.BPP8_INDEXED)
    assert not sailpy.Image.check_has_alpha(sailpy.PixelFormat.BPP8_INDEXED)


def test_pixel_format_string_conversion():
    """Test pixel format .name and .from_string()"""
    pf = sailpy.PixelFormat.BPP24_RGB

    # Standard Python property
    assert pf.name == "BPP24_RGB"
    assert "RGB" in pf.name

    # SAIL format parsing
    pf2 = sailpy.PixelFormat.from_string("BPP24-RGB")
    assert pf2 == pf


def test_format_conversion_check():
    """Test checking if conversion is possible"""
    # RGB to RGBA should be possible
    can_convert = sailpy.Image.check_conversion(
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP32_RGBA
    )
    assert can_convert

    # RGBA to RGB should be possible
    can_convert = sailpy.Image.check_conversion(
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP24_RGB
    )
    assert can_convert


def test_different_dimensions():
    """Test creating images with various dimensions"""
    test_sizes = [
        (1, 1),
        (16, 16),
        (256, 256),
        (1920, 1080),
        (100, 50),  # Non-square
    ]

    for width, height in test_sizes:
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height)
        assert img.is_valid
        assert img.width == width
        assert img.height == height


def test_numpy_shapes_for_formats():
    """Test NumPy shapes match expected for each format"""
    # RGB: (H, W, 3)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 48)
    assert img.to_numpy().shape == (48, 64, 3)

    # RGBA: (H, W, 4)
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 64, 48)
    assert img.to_numpy().shape == (48, 64, 4)

    # Grayscale: (H, W)
    img = sailpy.Image(sailpy.PixelFormat.BPP8_GRAYSCALE, 64, 48)
    assert img.to_numpy().shape == (48, 64)

    # 16-bit RGB: (H, W, 3)
    img = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 64, 48)
    assert img.to_numpy().shape == (48, 64, 3)
