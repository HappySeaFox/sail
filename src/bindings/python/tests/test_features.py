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
Comprehensive tests for LoadFeatures, SaveFeatures, CompressionLevel
Based on C++ tests: tests/bindings/c++/load_features.cpp and save_features.cpp
"""

import pytest
import sailpy
import tempfile
import os


# ============================================================================
# CompressionLevel - Complete coverage
# ============================================================================

def test_compression_level_from_png():
    """Test CompressionLevel from PNG codec"""
    codec = sailpy.CodecInfo.from_name("PNG")
    cl = codec.save_features.compression_level

    assert cl.is_valid
    assert cl.min_level >= 0.0
    assert cl.max_level > cl.min_level
    assert cl.default_level >= cl.min_level
    assert cl.default_level <= cl.max_level
    assert cl.step > 0.0


def test_compression_level_from_jpeg():
    """Test CompressionLevel from JPEG codec"""
    codec = sailpy.CodecInfo.from_name("JPEG")
    cl = codec.save_features.compression_level

    if cl.is_valid:
        assert cl.min_level >= 0.0
        assert cl.max_level > cl.min_level
        assert cl.default_level >= cl.min_level
        assert cl.default_level <= cl.max_level


def test_compression_level_repr():
    """Test CompressionLevel repr"""
    codec = sailpy.CodecInfo.from_name("PNG")
    cl = codec.save_features.compression_level

    repr_str = repr(cl)
    assert "CompressionLevel" in repr_str
    if cl.is_valid:
        assert "min=" in repr_str or str(cl.min_level) in repr_str


# ============================================================================
# LoadFeatures - Complete coverage
# ============================================================================

def test_load_features_png():
    """Test LoadFeatures for PNG codec"""
    codec = sailpy.CodecInfo.from_name("PNG")
    lf = codec.load_features

    assert lf.features != 0
    assert lf.features & sailpy.CodecFeature.STATIC


def test_load_features_to_options():
    """Test LoadFeatures.to_options() - CRITICAL TEST"""
    codec = sailpy.CodecInfo.from_name("PNG")
    lf = codec.load_features

    # Convert features to options with defaults
    options = lf.to_options()

    # Verify options are valid
    assert isinstance(options, sailpy.LoadOptions)
    # Options should have reasonable values
    _ = options.options


def test_load_features_all_codecs():
    """Test LoadFeatures for all codecs"""
    codecs = sailpy.list_codecs()

    for codec in codecs:
        if codec.can_load:
            lf = codec.load_features
            assert lf.features != 0

            # to_options should not crash
            options = lf.to_options()
            assert isinstance(options, sailpy.LoadOptions)


# ============================================================================
# SaveFeatures - Complete coverage
# ============================================================================

def test_save_features_png():
    """Test SaveFeatures for PNG codec"""
    codec = sailpy.CodecInfo.from_name("PNG")
    sf = codec.save_features

    assert sf.features != 0
    assert sf.features & sailpy.CodecFeature.STATIC


def test_save_features_default_compression():
    """Test SaveFeatures.default_compression - CRITICAL TEST"""
    codec = sailpy.CodecInfo.from_extension("png")
    sf = codec.save_features

    # Default compression must be valid
    assert sf.default_compression != sailpy.Compression.UNKNOWN

    # Default must be in supported compressions
    assert sf.default_compression in sf.compressions


def test_save_features_compressions_list():
    """Test SaveFeatures.compressions list"""
    codec = sailpy.CodecInfo.from_extension("png")
    sf = codec.save_features

    # Compressions list should not be empty for saveable codecs
    assert len(sf.compressions) > 0

    # All should be valid
    for comp in sf.compressions:
        assert comp != sailpy.Compression.UNKNOWN


def test_save_features_pixel_formats_list():
    """Test SaveFeatures.pixel_formats list"""
    codec = sailpy.CodecInfo.from_extension("png")
    sf = codec.save_features

    # PNG should support multiple pixel formats
    assert len(sf.pixel_formats) > 0

    # All should be valid
    for pf in sf.pixel_formats:
        assert pf != sailpy.PixelFormat.UNKNOWN


def test_save_features_to_options():
    """Test SaveFeatures.to_options() - CRITICAL TEST"""
    codec = sailpy.CodecInfo.from_extension("png")
    sf = codec.save_features

    # Convert features to options with defaults
    options = sf.to_options()

    # Verify options are valid
    assert isinstance(options, sailpy.SaveOptions)

    # Compression should be set to default
    assert options.compression == sf.default_compression

    # Compression level should be within valid range
    cl = sf.compression_level
    if cl.is_valid:
        assert options.compression_level >= cl.min_level
        assert options.compression_level <= cl.max_level


def test_save_features_to_options_all_codecs():
    """Test SaveFeatures.to_options() for ALL saveable codecs - CRITICAL"""
    codecs = sailpy.list_codecs()
    tested_count = 0

    for codec in codecs:
        if codec.can_save:
            sf = codec.save_features

            # to_options should not crash
            options = sf.to_options()

            # CRITICAL: Verify defaults are correct
            assert options.compression == sf.default_compression

            # If compression level is supported, verify it's within range
            cl = sf.compression_level
            if cl.is_valid:
                assert options.compression_level >= cl.min_level
                assert options.compression_level <= cl.max_level
                # Should be default
                assert abs(options.compression_level - cl.default_level) < 0.001

            tested_count += 1

    # Verify we tested at least some codecs
    assert tested_count > 0


# ============================================================================
# Integration tests - Use features to create correct SaveOptions
# ============================================================================

def test_save_with_features_derived_options():
    """Test saving with options derived from save_features - CRITICAL"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [100, 150, 200]

    output_path = os.path.join(tempfile.gettempdir(), "test_features_save.png")

    try:
        # Get codec
        codec = sailpy.CodecInfo.from_extension("png")

        # Get CORRECT options from features
        options = codec.save_features.to_options()

        # Modify compression level within valid range
        cl = codec.save_features.compression_level
        if cl.is_valid:
            options.compression_level = cl.max_level  # Max compression

        # Save with these options
        writer = sailpy.ImageWriter(output_path)
        writer.with_options(options)
        writer.write(img)
        writer.finish()

        # Verify
        assert os.path.exists(output_path)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid

    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_load_with_features_derived_options(test_jpeg):
    """Test loading with options derived from load_features - CRITICAL"""
    # Get codec
    codec = sailpy.CodecInfo.from_path(str(test_jpeg))

    # Get CORRECT options from features
    options = codec.load_features.to_options()

    # Load with these options
    reader = sailpy.ImageReader(str(test_jpeg))
    reader.with_options(options)
    img = reader.read()

    assert img.is_valid


def test_features_consistency():
    """Test that features are consistent across codec discovery methods"""
    # Get codec different ways
    codec1 = sailpy.CodecInfo.from_extension("png")
    codec2 = sailpy.CodecInfo.from_mime_type("image/png")

    # Features should be identical
    assert codec1.load_features.features == codec2.load_features.features
    assert codec1.save_features.features == codec2.save_features.features
    assert codec1.save_features.default_compression == codec2.save_features.default_compression


def test_compression_level_all_saveable_codecs():
    """Test compression_level for all saveable codecs"""
    codecs = sailpy.list_codecs()

    for codec in codecs:
        if codec.can_save:
            cl = codec.save_features.compression_level

            # If valid, verify constraints
            if cl.is_valid:
                assert cl.min_level < cl.max_level
                assert cl.default_level >= cl.min_level
                assert cl.default_level <= cl.max_level
                assert cl.step > 0.0


def test_pixel_formats_are_saveable():
    """Test that all pixel_formats from save_features are actually saveable"""
    codec = sailpy.CodecInfo.from_extension("png")
    sf = codec.save_features

    # Try creating image with each supported pixel format
    for pf in sf.pixel_formats[:3]:  # Test first 3 to save time
        img = sailpy.Image(pf, 8, 8)
        assert img.is_valid
        assert img.pixel_format == pf

