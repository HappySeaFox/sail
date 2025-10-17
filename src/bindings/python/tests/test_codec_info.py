
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
Tests for SAIL Python Codec Information
"""

import sailpy


def test_list_codecs():
    """Test listing all available codecs"""
    codecs = sailpy.CodecInfo.list()

    assert len(codecs) > 0
    assert all(isinstance(c, sailpy.CodecInfo) for c in codecs)
    assert all(c.is_valid for c in codecs)


def test_codec_info_properties():
    """Test CodecInfo properties"""
    codecs = sailpy.CodecInfo.list()

    # Find PNG codec
    png_codec = None
    for codec in codecs:
        if codec.name == "PNG":
            png_codec = codec
            break

    assert png_codec is not None
    assert png_codec.is_valid
    assert png_codec.name == "PNG"
    assert len(png_codec.version) > 0
    assert "Portable Network Graphics" in png_codec.description
    assert "png" in png_codec.extensions
    assert png_codec.can_load
    assert png_codec.can_save


def test_get_codec_info_by_extension():
    """Test getting codec info by extension"""
    codec = sailpy.CodecInfo.from_extension("jpg")

    assert codec.is_valid
    assert codec.name == "JPEG"
    assert "jpg" in codec.extensions or "jpeg" in codec.extensions


def test_get_codec_info_by_mime_type():
    """Test getting codec info by MIME type"""
    codec = sailpy.CodecInfo.from_mime_type("image/png")

    assert codec.is_valid
    assert codec.name == "PNG"


def test_codec_info_from_path():
    """Test CodecInfo.from_path()"""
    codec = sailpy.CodecInfo.from_path("/test/image.png")

    assert codec.is_valid
    assert codec.name == "PNG"


def test_codec_info_from_extension():
    """Test CodecInfo.from_extension()"""
    codec = sailpy.CodecInfo.from_extension("jpeg")

    assert codec.is_valid
    assert codec.name == "JPEG"


def test_codec_info_from_mime_type():
    """Test CodecInfo.from_mime_type()"""
    codec = sailpy.CodecInfo.from_mime_type("image/webp")

    assert codec.is_valid
    assert codec.name == "WEBP"


def test_codec_info_from_name():
    """Test CodecInfo.from_name()"""
    codec = sailpy.CodecInfo.from_name("PNG")

    assert codec.is_valid
    assert codec.name == "PNG"

    # Test case insensitive
    codec_lower = sailpy.CodecInfo.from_name("png")
    assert codec_lower.is_valid
    assert codec_lower.name == "PNG"


def test_codec_info_invalid():
    """Test invalid codec info"""
    codec = sailpy.CodecInfo()

    assert not codec.is_valid


def test_codec_info_repr():
    """Test CodecInfo string representation"""
    codec = sailpy.CodecInfo.from_extension("png")

    repr_str = repr(codec)
    assert "CodecInfo" in repr_str
    assert "PNG" in repr_str


def test_codec_info_list_static():
    """Test CodecInfo.list() static method"""
    codecs = sailpy.CodecInfo.list()

    assert len(codecs) > 0
    assert all(c.is_valid for c in codecs)

    # Check that common formats are present
    codec_names = {c.name for c in codecs}
    assert "PNG" in codec_names
    assert "JPEG" in codec_names


def test_codec_info_extensions():
    """Test that codecs have extensions"""
    jpeg_codec = sailpy.CodecInfo.from_extension("jpg")

    assert len(jpeg_codec.extensions) > 0
    assert any(ext in ["jpg", "jpeg"] for ext in jpeg_codec.extensions)


def test_codec_info_mime_types():
    """Test that codecs have MIME types"""
    png_codec = sailpy.CodecInfo.from_extension("png")

    assert len(png_codec.mime_types) > 0
    assert "image/png" in png_codec.mime_types


def test_get_codec_info_invalid():
    """Test getting codec info with invalid input"""
    codec = sailpy.CodecInfo.from_extension("invalid_format_xyz")

    assert not codec.is_valid


def test_codec_features():
    """Test checking codec features"""
    codecs = sailpy.CodecInfo.list()

    # At least some codecs should support loading
    assert any(c.can_load for c in codecs)

    # At least some codecs should support saving
    assert any(c.can_save for c in codecs)


def test_codec_info_case_insensitive():
    """Test that codec lookups are case-insensitive"""
    codec_lower = sailpy.CodecInfo.from_extension("png")
    codec_upper = sailpy.CodecInfo.from_extension("PNG")

    assert codec_lower.is_valid
    assert codec_upper.is_valid
    assert codec_lower.name == codec_upper.name
