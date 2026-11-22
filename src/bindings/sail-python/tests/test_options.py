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
Comprehensive coverage tests for LoadOptions and SaveOptions
Based on C++ tests: tests/bindings/c++/load_options.cpp and save_options.cpp
"""

import pytest
import sailpy


# ============================================================================
# LoadOptions - Full Coverage
# ============================================================================

def test_load_options_construct_empty():
    """Test LoadOptions default constructor"""
    options = sailpy.LoadOptions()
    assert options.options == 0


def test_load_options_construct_with_value():
    """Test LoadOptions constructor with value"""
    options = sailpy.LoadOptions(sailpy.Option.META_DATA)
    assert options.options == sailpy.Option.META_DATA


def test_load_options_copy_semantics():
    """Test LoadOptions copy behavior"""
    opt1 = sailpy.LoadOptions()
    opt1.options = sailpy.Option.META_DATA

    # Create new instance and copy values
    opt2 = sailpy.LoadOptions()
    opt2.options = opt1.options
    assert opt1.options == opt2.options

    # Verify independence
    opt1.options = sailpy.Option.ICCP
    assert opt2.options == sailpy.Option.META_DATA  # Should NOT change


def test_load_options_independence():
    """Test LoadOptions instances are independent"""
    opt1 = sailpy.LoadOptions(sailpy.Option.META_DATA)
    opt2 = sailpy.LoadOptions(sailpy.Option.ICCP)

    assert opt1.options != opt2.options
    assert opt1.options == sailpy.Option.META_DATA
    assert opt2.options == sailpy.Option.ICCP


def test_load_options_multiple_flags():
    """Test LoadOptions with multiple or-ed flags"""
    options = sailpy.LoadOptions()
    options.options = sailpy.Option.META_DATA | sailpy.Option.ICCP

    assert options.options & sailpy.Option.META_DATA
    assert options.options & sailpy.Option.ICCP


def test_load_options_modify_after_creation():
    """Test modifying LoadOptions after creation"""
    options = sailpy.LoadOptions(sailpy.Option.META_DATA)
    assert options.options == sailpy.Option.META_DATA

    # Modify
    options.options = sailpy.Option.ICCP
    assert options.options == sailpy.Option.ICCP

    # Clear
    options.options = 0
    assert options.options == 0


def test_load_options_with_image_input(test_jpeg):
    """Test LoadOptions with actual ImageInput"""
    options = sailpy.LoadOptions()
    options.options = sailpy.Option.META_DATA

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)

    img = input.load()
    assert img.is_valid


def test_load_options_max_dimensions():
    """Test LoadOptions dimension limits"""
    options = sailpy.LoadOptions()
    assert options.max_width == 0
    assert options.max_height == 0

    options.max_width = 8192
    options.max_height = 4096
    assert options.max_width == 8192
    assert options.max_height == 4096
    assert "max_width=8192" in repr(options)
    assert "max_height=4096" in repr(options)

    # max_dimensions mirrors max_width and max_height
    assert options.max_dimensions == (8192, 4096)

    options.max_dimensions = (100, 200)
    assert options.max_width == 100
    assert options.max_height == 200
    assert options.max_dimensions == (100, 200)

    options.max_dimensions = (0, 0)
    assert options.max_dimensions == (0, 0)

    assert sailpy.Status.ERROR_IMAGE_TOO_LARGE != sailpy.Status.ERROR_INVALID_IMAGE


def test_load_options_max_pixels_and_frames():
    """Test LoadOptions pixel and frame limits"""
    options = sailpy.LoadOptions()
    assert options.max_pixels == 0
    assert options.max_frames == 0

    options.max_pixels = 10_000_000_000
    options.max_frames = 100
    assert options.max_pixels == 10_000_000_000
    assert options.max_frames == 100
    assert "max_pixels=10000000000" in repr(options)
    assert "max_frames=100" in repr(options)

    options.max_pixels = 0
    options.max_frames = 0
    assert options.max_pixels == 0
    assert options.max_frames == 0

    with pytest.raises(TypeError):
        options.max_frames = -1

    assert sailpy.Status.ERROR_TOO_MANY_FRAMES != sailpy.Status.ERROR_IMAGE_TOO_LARGE


def test_load_options_max_dimensions_with_image_input(test_jpeg):
    """Test ImageInput honors LoadOptions dimension limits"""
    metadata = sailpy.ImageInput.probe(str(test_jpeg))
    width = metadata["width"]
    height = metadata["height"]

    # Limits equal to the image dimensions allow loading
    options = sailpy.LoadOptions()
    options.max_width = width
    options.max_height = height

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)
    assert input.load().is_valid

    # Too wide images are rejected
    options.max_width = width - 1

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)
    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        input.load()

    # Too tall images are rejected
    options.max_dimensions = (0, height - 1)

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)
    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        input.load()

    # Images with too many pixels are rejected
    options.max_dimensions = (0, 0)
    options.max_pixels = width * height

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)
    assert input.load().is_valid

    options.max_pixels = width * height - 1

    input = sailpy.ImageInput(str(test_jpeg))
    input.with_options(options)
    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        input.load()


def test_load_options_probe_with_dimension_limits(test_jpeg):
    """Test ImageInput.probe() honors LoadOptions dimension limits"""
    path = str(test_jpeg)

    def summary(metadata):
        # LoadOptions() don't request the source image info unlike the codec defaults, so compare just these
        return (metadata["width"], metadata["height"], metadata["codec_name"])

    metadata = sailpy.ImageInput.probe(path)
    width = metadata["width"]
    height = metadata["height"]

    # No options and default options mean no limits
    assert sailpy.ImageInput.probe(path, None) == metadata
    assert summary(sailpy.ImageInput.probe(path, sailpy.LoadOptions())) == summary(metadata)

    # Limits equal to the image dimensions allow probing
    options = sailpy.LoadOptions()
    options.max_dimensions = (width, height)
    options.max_pixels = width * height
    assert summary(sailpy.ImageInput.probe(path, options=options)) == summary(metadata)

    # Probing is not affected by the frame limit
    options.max_frames = 1
    assert summary(sailpy.ImageInput.probe(path, options)) == summary(metadata)

    # Too large images are rejected
    for max_width, max_height, max_pixels in (
        (width - 1, 0, 0),
        (0, height - 1, 0),
        (0, 0, width * height - 1),
    ):
        options = sailpy.LoadOptions()
        options.max_dimensions = (max_width, max_height)
        options.max_pixels = max_pixels

        with pytest.raises(RuntimeError, match="Failed to probe image"):
            sailpy.ImageInput.probe(path, options)


def test_load_options_probe_huge_dimensions(test_images_dir):
    """Test ImageInput.probe() rejects images with huge dimensions in their headers"""
    path = str(test_images_dir.parent / "edge-cases" / "huge-dimensions.png")

    options = sailpy.LoadOptions()
    options.max_pixels = 8192 * 8192

    with pytest.raises(RuntimeError, match="Failed to probe image"):
        sailpy.ImageInput.probe(path, options)


def test_load_options_max_frames_with_image_input(test_images_dir):
    """Test ImageInput honors the LoadOptions frame limit"""
    path = str(test_images_dir / "gif" / "bpp24-rgb-animated.gif")

    frame_count = len(sailpy.ImageInput(path).load_all())
    assert frame_count > 1

    # Images with exactly max_frames frames load fine
    options = sailpy.LoadOptions()
    options.max_frames = frame_count

    input = sailpy.ImageInput(path)
    input.with_options(options)
    assert len(input.load_all()) == frame_count

    # The frame that follows the last allowed one is rejected
    options.max_frames = 1

    input = sailpy.ImageInput(path)
    input.with_options(options)
    assert input.load().is_valid
    with pytest.raises(RuntimeError, match="ERROR_TOO_MANY_FRAMES"):
        input.load()

    input = sailpy.ImageInput(path)
    input.with_options(options)
    with pytest.raises(RuntimeError, match="ERROR_TOO_MANY_FRAMES"):
        input.load_all()

    # Iteration yields the allowed frames and then raises
    frames = []
    input = sailpy.ImageInput(path)
    input.with_options(options)
    with pytest.raises(RuntimeError, match="ERROR_TOO_MANY_FRAMES"):
        for frame in input:
            frames.append(frame)
    assert len(frames) == 1


def test_load_options_limit_errors_are_not_end_of_image(test_images_dir):
    """Test images that exceed the limits raise instead of looking empty"""
    path = str(test_images_dir / "gif" / "bpp24-rgb-animated.gif")

    options = sailpy.LoadOptions()
    options.max_dimensions = (1, 1)

    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        sailpy.ImageInput(path).with_options(options).load()

    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        sailpy.ImageInput(path).with_options(options).load_all()

    with pytest.raises(RuntimeError, match="ERROR_IMAGE_TOO_LARGE"):
        list(sailpy.ImageInput(path).with_options(options))


# ============================================================================
# SaveOptions - Full Coverage
# ============================================================================

def test_save_options_construct_empty():
    """Test SaveOptions default constructor"""
    options = sailpy.SaveOptions()
    assert options.options == 0
    assert options.compression == sailpy.Compression.UNKNOWN
    assert 0.0 <= options.compression_level <= 1.0


def test_save_options_construct_with_value():
    """Test SaveOptions constructor with value"""
    options = sailpy.SaveOptions(sailpy.Option.META_DATA)
    assert options.options == sailpy.Option.META_DATA


def test_save_options_copy_semantics():
    """Test SaveOptions copy behavior"""
    opt1 = sailpy.SaveOptions()
    opt1.options = sailpy.Option.META_DATA
    opt1.compression = sailpy.Compression.DEFLATE
    opt1.compression_level = 0.75

    # Create new instance and copy values
    opt2 = sailpy.SaveOptions()
    opt2.options = opt1.options
    opt2.compression = opt1.compression
    opt2.compression_level = opt1.compression_level

    assert opt1.options == opt2.options
    assert opt1.compression == opt2.compression
    assert opt1.compression_level == opt2.compression_level

    # Verify independence
    opt1.options = 0
    opt1.compression = sailpy.Compression.NONE
    opt1.compression_level = 0.5

    assert opt2.options == sailpy.Option.META_DATA
    assert opt2.compression == sailpy.Compression.DEFLATE
    assert opt2.compression_level == 0.75


def test_save_options_independence():
    """Test SaveOptions instances are independent"""
    opt1 = sailpy.SaveOptions()
    opt1.compression_level = 0.3

    opt2 = sailpy.SaveOptions()
    opt2.compression_level = 0.9

    assert abs(opt1.compression_level - 0.3) < 0.001
    assert abs(opt2.compression_level - 0.9) < 0.001


def test_save_options_all_properties_accessible():
    """Test all SaveOptions properties are accessible"""
    options = sailpy.SaveOptions()

    # Load all
    _ = options.options
    _ = options.compression
    _ = options.compression_level

    # Save all
    options.options = sailpy.Option.META_DATA
    options.compression = sailpy.Compression.DEFLATE
    options.compression_level = 0.8

    # Verify
    assert options.options == sailpy.Option.META_DATA
    assert options.compression == sailpy.Compression.DEFLATE
    assert abs(options.compression_level - 0.8) < 0.001


def test_save_options_compression_levels():
    """Test SaveOptions with various compression levels"""
    options = sailpy.SaveOptions()

    for level in [0.0, 0.25, 0.5, 0.75, 1.0]:
        options.compression_level = level
        assert abs(options.compression_level - level) < 0.001


def test_save_options_all_compression_types():
    """Test SaveOptions with all Compression types"""
    options = sailpy.SaveOptions()

    compressions = [
        sailpy.Compression.NONE,
        sailpy.Compression.DEFLATE,
        sailpy.Compression.JPEG,
        sailpy.Compression.RLE,
        sailpy.Compression.LZW,
    ]

    for comp in compressions:
        options.compression = comp
        assert options.compression == comp


def test_save_options_combined_settings():
    """Test SaveOptions with combined settings"""
    options = sailpy.SaveOptions()

    # Set everything
    options.options = sailpy.Option.META_DATA | sailpy.Option.ICCP
    options.compression = sailpy.Compression.DEFLATE
    options.compression_level = 0.95

    # Verify all
    assert options.options & sailpy.Option.META_DATA
    assert options.options & sailpy.Option.ICCP
    assert options.compression == sailpy.Compression.DEFLATE
    assert abs(options.compression_level - 0.95) < 0.001


def test_save_options_repr():
    """Test SaveOptions repr is informative"""
    options = sailpy.SaveOptions()
    options.options = 1
    options.compression = sailpy.Compression.DEFLATE
    options.compression_level = 0.5

    repr_str = repr(options)
    assert "SaveOptions" in repr_str
    assert "1" in repr_str or "options" in repr_str.lower()


def test_load_options_repr():
    """Test LoadOptions repr is informative"""
    options = sailpy.LoadOptions()
    options.options = 1

    repr_str = repr(options)
    assert "LoadOptions" in repr_str
    assert "1" in repr_str or "options" in repr_str.lower()


# ============================================================================
# Integration Tests - Options with CodecInfo
# ============================================================================

def test_codec_info_load_features_exists():
    """Test that CodecInfo has load_features and to_options()"""
    codec = sailpy.CodecInfo.from_name("PNG")
    assert codec.is_valid
    assert hasattr(codec, 'load_features')

    # Test to_options() method
    options = codec.load_features.to_options()
    assert isinstance(options, sailpy.LoadOptions)


def test_codec_info_save_features_exists():
    """Test that CodecInfo has save_features and to_options()"""
    codec = sailpy.CodecInfo.from_name("PNG")
    assert codec.is_valid
    assert hasattr(codec, 'save_features')

    # Test to_options() method
    options = codec.save_features.to_options()
    assert isinstance(options, sailpy.SaveOptions)


# ============================================================================
# Edge Cases
# ============================================================================

def test_options_invalid_compression_level():
    """Test SaveOptions with out-of-range compression level"""
    options = sailpy.SaveOptions()

    # Should accept any float
    options.compression_level = -1.0
    options.compression_level = 2.0
    options.compression_level = 100.0
    # Codec will validate these values


def test_options_independence():
    """Test that two Options instances are truly independent"""
    opt1 = sailpy.LoadOptions()
    opt2 = sailpy.LoadOptions()

    opt1.options = 1
    opt2.options = 2

    assert opt1.options == 1
    assert opt2.options == 2

    # Same for SaveOptions
    sopt1 = sailpy.SaveOptions()
    sopt2 = sailpy.SaveOptions()

    sopt1.compression_level = 0.3
    sopt2.compression_level = 0.7

    assert abs(sopt1.compression_level - 0.3) < 0.001
    assert abs(sopt2.compression_level - 0.7) < 0.001


def test_save_options_modify_sequence():
    """Test modifying SaveOptions in sequence"""
    options = sailpy.SaveOptions()

    # Sequence of modifications
    options.options = sailpy.Option.META_DATA
    assert options.options == sailpy.Option.META_DATA

    options.compression = sailpy.Compression.DEFLATE
    assert options.compression == sailpy.Compression.DEFLATE

    options.compression_level = 0.85
    assert abs(options.compression_level - 0.85) < 0.001

    # All should still be set
    assert options.options == sailpy.Option.META_DATA
    assert options.compression == sailpy.Compression.DEFLATE
    assert abs(options.compression_level - 0.85) < 0.001


def test_save_options_has_tuning():
    """Test that SaveOptions has tuning property"""
    opts = sailpy.SaveOptions()
    assert hasattr(opts, 'tuning')
    assert isinstance(opts.tuning, dict)


def test_load_options_has_tuning():
    """Test that LoadOptions has tuning property"""
    opts = sailpy.LoadOptions()
    assert hasattr(opts, 'tuning')
    assert isinstance(opts.tuning, dict)
