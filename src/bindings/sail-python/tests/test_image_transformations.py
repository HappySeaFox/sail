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
Tests for Image transformations: rotate, mirror, convert_to
"""

import sailpy
import numpy as np


# ============================================================================
# rotate() and rotate_to() - Complete coverage
# ============================================================================

def test_rotate_90_changes_dimensions():
    """Test that rotate(90°) swaps width and height"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 200, 100)
    assert img.width == 200
    assert img.height == 100

    # Rotate 90° in-place
    img.rotate(sailpy.Orientation.ROTATED_90)

    assert img.width == 100  # Swapped
    assert img.height == 200  # Swapped
    assert img.is_valid


def test_rotate_180_keeps_dimensions():
    """Test that rotate(180°) keeps same dimensions"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 200, 100)

    img.rotate(sailpy.Orientation.ROTATED_180)

    assert img.width == 200  # Same
    assert img.height == 100  # Same
    assert img.is_valid


def test_rotate_270_swaps_dimensions():
    """Test that rotate(270°) swaps width and height"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 200, 100)

    img.rotate(sailpy.Orientation.ROTATED_270)

    assert img.width == 100  # Swapped
    assert img.height == 200  # Swapped
    assert img.is_valid


def test_rotate_to_returns_new_image():
    """Test that rotate_to() returns new image, leaves original unchanged"""
    original = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 200, 100)
    original_width = original.width
    original_height = original.height

    # Rotate to new image
    rotated = original.rotate_to(sailpy.Orientation.ROTATED_90)

    # Original unchanged
    assert original.width == original_width
    assert original.height == original_height

    # New image rotated
    assert rotated.width == 100
    assert rotated.height == 200
    assert rotated.is_valid


def test_rotate_to_all_angles():
    """Test rotate_to() with all supported angles"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 300, 200)

    # 90°
    r90 = img.rotate_to(sailpy.Orientation.ROTATED_90)
    assert r90.width == 200 and r90.height == 300

    # 180°
    r180 = img.rotate_to(sailpy.Orientation.ROTATED_180)
    assert r180.width == 300 and r180.height == 200

    # 270°
    r270 = img.rotate_to(sailpy.Orientation.ROTATED_270)
    assert r270.width == 200 and r270.height == 300


def test_rotate_preserves_pixel_format():
    """Test that rotation preserves pixel format"""
    for pf in [sailpy.PixelFormat.BPP24_RGB,
               sailpy.PixelFormat.BPP32_RGBA,
               sailpy.PixelFormat.BPP8_GRAYSCALE]:
        img = sailpy.Image(pf, 100, 100)
        img.rotate(sailpy.Orientation.ROTATED_90)
        assert img.pixel_format == pf


def test_rotate_90_pixel_data():
    """Test that 90° rotation actually rotates pixel data correctly"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 3, 2)
    arr = img.to_numpy()

    # Create pattern (2 rows, 3 cols):
    # Row 0: [Red, Green, Blue]
    # Row 1: [Yellow, Cyan, Magenta]
    arr[0, 0] = [255, 0, 0]    # Red
    arr[0, 1] = [0, 255, 0]    # Green
    arr[0, 2] = [0, 0, 255]    # Blue
    arr[1, 0] = [255, 255, 0]  # Yellow
    arr[1, 1] = [0, 255, 255]  # Cyan
    arr[1, 2] = [255, 0, 255]  # Magenta

    # Rotate 90 degrees clockwise (width 3, height 2 -> width 2, height 3)
    rotated = img.rotate_to(sailpy.Orientation.ROTATED_90)
    r_arr = rotated.to_numpy()

    # After 90° CW rotation:
    # [0,0] was bottom-left (Yellow)
    # [0,1] was top-left (Red)
    # [2,0] was bottom-right (Magenta)
    # [2,1] was top-right (Blue)
    assert rotated.width == 2 and rotated.height == 3
    assert np.array_equal(r_arr[0, 0], [255, 255, 0])  # Yellow
    assert np.array_equal(r_arr[0, 1], [255, 0, 0])    # Red
    assert np.array_equal(r_arr[2, 0], [255, 0, 255])  # Magenta
    assert np.array_equal(r_arr[2, 1], [0, 0, 255])    # Blue


def test_rotate_180_pixel_data():
    """Test that 180° rotation flips both axes"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 3, 2)
    arr = img.to_numpy()

    # Create pattern:
    # [Red, Green, Blue]
    # [Yellow, Cyan, Magenta]
    arr[0, 0] = [255, 0, 0]
    arr[0, 2] = [0, 0, 255]
    arr[1, 0] = [255, 255, 0]
    arr[1, 2] = [255, 0, 255]

    # Rotate 180°
    rotated = img.rotate_to(sailpy.Orientation.ROTATED_180)
    r_arr = rotated.to_numpy()

    # After 180°: top-left ↔ bottom-right
    assert np.array_equal(r_arr[0, 0], [255, 0, 255])  # Was bottom-right (Magenta)
    assert np.array_equal(r_arr[0, 2], [255, 255, 0])  # Was bottom-left (Yellow)
    assert np.array_equal(r_arr[1, 0], [0, 0, 255])    # Was top-right (Blue)
    assert np.array_equal(r_arr[1, 2], [255, 0, 0])    # Was top-left (Red)


def test_rotate_270_pixel_data():
    """Test that 270° rotation is correct"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 2, 3)
    arr = img.to_numpy()

    # Vertical stripes: left=Red, right=Blue
    arr[:, 0] = [255, 0, 0]  # Left column: Red
    arr[:, 1] = [0, 0, 255]  # Right column: Blue

    # Rotate 270° clockwise (or 90° counter-clockwise)
    rotated = img.rotate_to(sailpy.Orientation.ROTATED_270)
    r_arr = rotated.to_numpy()

    # After 270° CW: left column becomes bottom row
    assert rotated.width == 3 and rotated.height == 2
    # Bottom row should be red (was left column)
    assert np.array_equal(r_arr[1, 0], [255, 0, 0])
    # Top row should be blue (was right column)
    assert np.array_equal(r_arr[0, 0], [0, 0, 255])


# ============================================================================
# mirror() - Complete coverage
# ============================================================================

def test_mirror_horizontal():
    """Test horizontal mirroring"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 50)

    img.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)

    # Dimensions unchanged
    assert img.width == 100
    assert img.height == 50
    assert img.is_valid


def test_mirror_vertical():
    """Test vertical mirroring"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 50)

    img.mirror(sailpy.Orientation.MIRRORED_VERTICALLY)

    # Dimensions unchanged
    assert img.width == 100
    assert img.height == 50
    assert img.is_valid


def test_mirror_horizontal_pixel_data():
    """Test horizontal mirroring with colored stripes"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 4, 2)
    arr = img.to_numpy()

    # Create vertical stripes (columns): Red, Green, Blue, Yellow
    arr[:, 0] = [255, 0, 0]    # Left: Red
    arr[:, 1] = [0, 255, 0]    # Green
    arr[:, 2] = [0, 0, 255]    # Blue
    arr[:, 3] = [255, 255, 0]  # Right: Yellow

    # Mirror horizontally (left ↔ right)
    img.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)
    m_arr = img.to_numpy()

    # After horizontal mirror: columns reversed
    assert np.array_equal(m_arr[0, 0], [255, 255, 0])  # Left was right (Yellow)
    assert np.array_equal(m_arr[0, 1], [0, 0, 255])    # Was Blue
    assert np.array_equal(m_arr[0, 2], [0, 255, 0])    # Was Green
    assert np.array_equal(m_arr[0, 3], [255, 0, 0])    # Right was left (Red)


def test_mirror_vertical_pixel_data():
    """Test vertical mirroring with colored rows"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 2, 4)
    arr = img.to_numpy()

    # Create horizontal stripes (rows): Red, Green, Blue, Yellow
    arr[0, :] = [255, 0, 0]    # Top: Red
    arr[1, :] = [0, 255, 0]    # Green
    arr[2, :] = [0, 0, 255]    # Blue
    arr[3, :] = [255, 255, 0]  # Bottom: Yellow

    # Mirror vertically (top ↔ bottom)
    img.mirror(sailpy.Orientation.MIRRORED_VERTICALLY)
    m_arr = img.to_numpy()

    # After vertical mirror: rows reversed
    assert np.array_equal(m_arr[0, 0], [255, 255, 0])  # Top was bottom (Yellow)
    assert np.array_equal(m_arr[1, 0], [0, 0, 255])    # Was Blue
    assert np.array_equal(m_arr[2, 0], [0, 255, 0])    # Was Green
    assert np.array_equal(m_arr[3, 0], [255, 0, 0])    # Bottom was top (Red)


# ============================================================================
# convert_to() - Complete coverage
# ============================================================================

def test_convert_to_returns_new_image():
    """Test that convert_to() returns new image"""
    original = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 50, 50)

    converted = original.convert_to(sailpy.PixelFormat.BPP32_RGBA)

    # Original unchanged
    assert original.pixel_format == sailpy.PixelFormat.BPP24_RGB

    # New image converted
    assert converted.pixel_format == sailpy.PixelFormat.BPP32_RGBA
    assert converted.is_valid
    assert converted.width == 50
    assert converted.height == 50


def test_convert_to_rgb_to_rgba():
    """Test RGB to RGBA conversion"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)

    rgba = img.convert_to(sailpy.PixelFormat.BPP32_RGBA)

    assert rgba.pixel_format == sailpy.PixelFormat.BPP32_RGBA
    assert rgba.bits_per_pixel == 32


def test_convert_to_rgba_to_rgb():
    """Test RGBA to RGB conversion (drops alpha)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 10, 10)

    rgb = img.convert_to(sailpy.PixelFormat.BPP24_RGB)

    assert rgb.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert rgb.bits_per_pixel == 24


def test_convert_to_with_options():
    """Test convert_to() with ConversionOptions"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 10, 10)
    options = sailpy.ConversionOptions()
    options.blend_alpha = True

    rgb = img.convert_to(sailpy.PixelFormat.BPP24_RGB, options)

    assert rgb.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert rgb.is_valid


def test_convert_to_grayscale():
    """Test conversion to grayscale"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)

    gray = img.convert_to(sailpy.PixelFormat.BPP8_GRAYSCALE)

    assert gray.pixel_format == sailpy.PixelFormat.BPP8_GRAYSCALE
    assert gray.is_grayscale


# ============================================================================
# can_convert() and closest_pixel_format() - Complete coverage
# ============================================================================

def test_can_convert_instance_method():
    """Test Image.can_convert() instance method"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)

    # Should be able to convert RGB to RGBA
    assert img.can_convert(sailpy.PixelFormat.BPP32_RGBA)

    # Should be able to convert RGB to Grayscale
    assert img.can_convert(sailpy.PixelFormat.BPP8_GRAYSCALE)


def test_can_convert_static_method():
    """Test Image.check_conversion() static method"""
    # RGB to RGBA
    assert sailpy.Image.check_conversion(
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP32_RGBA
    )

    # RGBA to RGB
    assert sailpy.Image.check_conversion(
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP24_RGB
    )


def test_closest_pixel_format_instance():
    """Test Image.closest_pixel_format() instance method"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)

    candidates = [
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP8_GRAYSCALE,
        sailpy.PixelFormat.BPP24_BGR
    ]

    closest = img.closest_pixel_format(candidates)
    assert closest in candidates


def test_closest_pixel_format_static():
    """Test Image.find_closest_pixel_format() static method"""
    candidates = [
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP8_GRAYSCALE
    ]

    closest = sailpy.Image.find_closest_pixel_format(
        sailpy.PixelFormat.BPP24_RGB,
        candidates
    )
    assert closest in candidates


# ============================================================================
# Edge cases and error handling
# ============================================================================

def test_rotate_preserves_image_data():
    """Test that rotation preserves image validity"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 100)
    arr = img.to_numpy()
    arr.fill(128)  # Gray

    img.rotate(sailpy.Orientation.ROTATED_90)

    assert img.is_valid
    assert img.pixels_size > 0


def test_convert_to_same_format():
    """Test converting to the same format"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 10, 10)

    same = img.convert_to(sailpy.PixelFormat.BPP24_RGB)

    assert same.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert same.is_valid


def test_multiple_transformations():
    """Test chaining multiple transformations"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 50)

    # Rotate, then convert, then mirror
    img.rotate(sailpy.Orientation.ROTATED_90)
    converted = img.convert_to(sailpy.PixelFormat.BPP32_RGBA)
    converted.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)

    assert converted.is_valid
    assert converted.pixel_format == sailpy.PixelFormat.BPP32_RGBA


# ============================================================================
# Conversion with SaveFeatures
# ============================================================================

def test_convert_with_save_features():
    """Test in-place convert() with SaveFeatures"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 50, 50)
    img.to_numpy()[:] = [100, 150, 200, 255]

    # Get PNG codec features
    codec = sailpy.CodecInfo.from_name("PNG")
    save_features = codec.save_features

    # Convert to best format for PNG
    img.convert(save_features)

    assert img.is_valid
    assert img.pixel_format in save_features.pixel_formats


def test_convert_with_save_features_and_options():
    """Test in-place convert() with SaveFeatures and ConversionOptions"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 50, 50)
    img.to_numpy()[:] = [100, 150, 200, 128]  # Semi-transparent

    codec = sailpy.CodecInfo.from_name("PNG")
    save_features = codec.save_features

    options = sailpy.ConversionOptions()
    options.blend_alpha = True

    # Convert with alpha blending
    img.convert(save_features, options)

    assert img.is_valid
    assert img.pixel_format in save_features.pixel_formats


def test_convert_to_with_save_features():
    """Test convert_to() with SaveFeatures returns new image"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 50, 50)
    img.to_numpy()[:] = [100, 150, 200, 255]

    codec = sailpy.CodecInfo.from_name("PNG")
    save_features = codec.save_features

    # Convert to new image
    converted = img.convert_to(save_features)

    # Original unchanged
    assert img.pixel_format == sailpy.PixelFormat.BPP32_RGBA

    # New image has best format
    assert converted.is_valid
    assert converted.pixel_format in save_features.pixel_formats
    assert converted.width == img.width
    assert converted.height == img.height


def test_convert_to_with_save_features_and_options():
    """Test convert_to() with SaveFeatures and ConversionOptions"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 50, 50)
    img.to_numpy()[:] = [100, 150, 200, 128]

    codec = sailpy.CodecInfo.from_name("JPEG")
    save_features = codec.save_features

    options = sailpy.ConversionOptions()
    options.blend_alpha = True

    # Convert to new image with alpha blending
    converted = img.convert_to(save_features, options)

    # Original unchanged
    assert img.pixel_format == sailpy.PixelFormat.BPP32_RGBA

    # New image is valid
    assert converted.is_valid
    assert converted.pixel_format in save_features.pixel_formats


def test_closest_pixel_format_with_save_features():
    """Test closest_pixel_format() with SaveFeatures"""
    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 10, 10)

    codec = sailpy.CodecInfo.from_name("PNG")
    save_features = codec.save_features

    # Find closest format
    closest = img.closest_pixel_format(save_features)

    assert closest != sailpy.PixelFormat.UNKNOWN
    assert closest in save_features.pixel_formats


# ============================================================================
# Constructor with bytes_per_line
# ============================================================================

def test_image_constructor_with_bytes_per_line():
    """Test Image constructor with explicit bytes_per_line"""
    width = 100
    height = 50
    bytes_per_line = 400  # 100 pixels * 3 bytes + 100 padding

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height, bytes_per_line)

    assert img.is_valid
    assert img.width == width
    assert img.height == height
    assert img.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert img.bytes_per_line == bytes_per_line


def test_image_constructor_with_bytes_per_line_aligned():
    """Test Image with aligned bytes_per_line"""
    width = 97  # Not divisible by 8
    height = 50
    # Manual alignment to 16 bytes
    bytes_per_line = ((width * 3 + 15) // 16) * 16

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height, bytes_per_line)

    assert img.is_valid
    assert img.width == width
    assert img.height == height
    assert img.bytes_per_line == bytes_per_line
    assert img.bytes_per_line >= width * 3  # Has padding


def test_image_constructor_bytes_per_line_equals_calculated():
    """Test that bytes_per_line works when equal to calculated"""
    width = 100
    height = 50
    calculated_bpl = sailpy.Image.calculate_bytes_per_line(width, sailpy.PixelFormat.BPP24_RGB)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, width, height, calculated_bpl)

    assert img.is_valid
    assert img.bytes_per_line == calculated_bpl
