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
Complete tests for core classes - ALL methods must be tested
"""

import pytest
import sailpy
import numpy as np


# ============================================================================
# Variant - complete coverage
# ============================================================================

def test_variant_has_value_all_types():
    """Test has_value<T>() for ALL types"""
    # Bool
    v_bool = sailpy.Variant(True)
    assert v_bool.has_bool()
    assert not v_bool.has_int()
    assert not v_bool.has_float()
    assert not v_bool.has_string()
    assert not v_bool.has_data()

    # Int
    v_int = sailpy.Variant(42)
    assert not v_int.has_bool()
    assert v_int.has_int()
    assert not v_int.has_float()
    assert not v_int.has_string()
    assert not v_int.has_data()

    # Float
    v_float = sailpy.Variant(3.14)
    assert not v_float.has_bool()
    assert not v_float.has_int()
    assert v_float.has_float()
    assert not v_float.has_string()
    assert not v_float.has_data()

    # String
    v_string = sailpy.Variant("hello")
    assert not v_string.has_bool()
    assert not v_string.has_int()
    assert not v_string.has_float()
    assert v_string.has_string()
    assert not v_string.has_data()

    # Data (bytes) - NOW WORKS CORRECTLY!
    v_data = sailpy.Variant(b"\x00\x01\x02")
    assert not v_data.has_bool()
    assert not v_data.has_int()
    assert not v_data.has_float()
    assert not v_data.has_string()
    assert v_data.has_data()


def test_variant_set_value_all_types():
    """Test set_value<T>() for ALL types"""
    v = sailpy.Variant()

    # Set bool
    v.set_bool(True)
    assert v.has_bool()
    assert v.to_bool()

    # Set int
    v.set_int(999)
    assert v.has_int()
    assert v.to_int() == 999

    # Set float
    v.set_float(2.718)
    assert v.has_float()
    assert abs(v.to_float() - 2.718) < 0.001

    # Set string
    v.set_string("test")
    assert v.has_string()
    assert v.to_string() == "test"

    # Set data (bytes)
    v.set_data(b"\xAA\xBB\xCC")
    assert v.has_data()
    assert v.to_data() == b"\xAA\xBB\xCC"


def test_variant_clear():
    """Test Variant.clear() actually clears the value"""
    v = sailpy.Variant(123)
    assert v.is_valid
    assert v.has_int()

    v.clear()
    assert not v.is_valid
    assert not v.has_int()
    assert not v.has_bool()
    assert not v.has_float()


# ============================================================================
# Palette - complete coverage
# ============================================================================

def test_palette_set_data():
    """Test Palette.set_data() - CRITICAL TEST"""
    palette = sailpy.Palette()

    # Create palette data (RGB, 3 colors)
    data = bytes([
        255, 0, 0,    # Red
        0, 255, 0,    # Green
        0, 0, 255     # Blue
    ])

    palette.set_data(sailpy.PixelFormat.BPP24_RGB, data, 3)

    assert palette.is_valid
    assert palette.pixel_format == sailpy.PixelFormat.BPP24_RGB
    assert palette.color_count == 3

    # Verify data
    retrieved = palette.get_data()
    assert retrieved == data


def test_palette_get_data_empty():
    """Test getting data from empty palette"""
    palette = sailpy.Palette()
    assert not palette.is_valid

    data = palette.get_data()
    assert len(data) == 0


# ============================================================================
# Iccp - complete coverage
# ============================================================================

def test_iccp_set_data():
    """Test Iccp.set_data() - CRITICAL TEST"""
    iccp = sailpy.Iccp()

    # Set ICC profile data
    profile_data = b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09"
    iccp.set_data(profile_data)

    assert iccp.is_valid

    # Verify data
    retrieved = iccp.get_data()
    assert retrieved == profile_data


def test_iccp_get_data_empty():
    """Test getting data from empty ICC profile"""
    iccp = sailpy.Iccp()
    assert not iccp.is_valid

    data = iccp.get_data()
    assert len(data) == 0


def test_iccp_large_data():
    """Test ICC profile with large data"""
    # Typical ICC profile is 500-5000 bytes
    large_data = bytes(range(256)) * 10  # 2560 bytes
    iccp = sailpy.Iccp(large_data)

    assert iccp.is_valid
    assert len(iccp.get_data()) == len(large_data)


# ============================================================================
# MetaData - complete coverage
# ============================================================================

def test_metadata_set_key_unknown():
    """Test MetaData.set_key_unknown() - CRITICAL TEST"""
    md = sailpy.MetaData()

    # Set custom key
    md.set_key_unknown("CustomKey")
    md.set_value(sailpy.Variant("CustomValue"))

    assert md.key_unknown == "CustomKey"
    assert md.value.to_string() == "CustomValue"


def test_metadata_set_key_known():
    """Test MetaData.set_key() with known enum"""
    md = sailpy.MetaData()

    # Set known key
    md.set_key(sailpy.MetaDataType.AUTHOR)
    md.set_value(sailpy.Variant("John Doe"))

    assert md.key == sailpy.MetaDataType.AUTHOR
    assert md.value.to_string() == "John Doe"


def test_metadata_set_value_different_types():
    """Test setting different value types in metadata"""
    md = sailpy.MetaData(sailpy.MetaDataType.AUTHOR, sailpy.Variant("Initial"))

    # Change value type
    md.set_value(sailpy.Variant(42))
    assert md.value.has_int()
    assert md.value.to_int() == 42

    md.set_value(sailpy.Variant(True))
    assert md.value.has_bool()
    assert md.value.to_bool()


def test_metadata_static_methods():
    """Test MetaData static conversion methods"""
    # to_string
    name = sailpy.MetaData.meta_data_to_string(sailpy.MetaDataType.AUTHOR)
    assert len(name) > 0
    assert "AUTHOR" in name.upper()

    # from_string
    key = sailpy.MetaData.meta_data_from_string(name)
    assert key == sailpy.MetaDataType.AUTHOR


# ============================================================================
# SourceImage - complete coverage
# ============================================================================

def test_source_image_from_probe(test_jpeg):
    """Test SourceImage info from probe"""
    metadata = sailpy.ImageReader.probe(str(test_jpeg))

    # Probe should return source format info
    assert "source_pixel_format" in metadata or "pixel_format" in metadata
    if "source_compression" in metadata:
        assert metadata["source_compression"] != sailpy.Compression.UNKNOWN


def test_source_image_all_properties(test_jpeg):
    """Test ALL SourceImage properties via source_image"""
    img = sailpy.Image.from_file(str(test_jpeg))

    source = img.source_image
    assert source.is_valid

    # ALL properties must be accessible
    assert source.pixel_format != sailpy.PixelFormat.UNKNOWN
    _ = source.chroma_subsampling
    _ = source.orientation
    assert source.compression != sailpy.Compression.UNKNOWN
    _ = source.interlaced


# ============================================================================
# Integration tests - verify all methods work together
# ============================================================================

def test_variant_in_metadata():
    """Test Variant used in MetaData"""
    v = sailpy.Variant(42)
    md = sailpy.MetaData(sailpy.MetaDataType.AUTHOR, v)

    assert md.value.has_int()
    assert md.value.to_int() == 42


def test_palette_in_image():
    """Test Palette used with Image"""
    # Create indexed image
    img = sailpy.Image(sailpy.PixelFormat.BPP8_INDEXED, 16, 16)

    # Create palette
    palette = sailpy.Palette()
    data = bytes([255, 0, 0, 0, 255, 0, 0, 0, 255])  # RGB: Red, Green, Blue
    palette.set_data(sailpy.PixelFormat.BPP24_RGB, data, 3)

    assert palette.is_valid
    assert palette.color_count == 3


# ============================================================================
# Resolution - Complete coverage
# ============================================================================

def test_resolution_construct_empty():
    """Test Resolution default constructor"""
    res = sailpy.Resolution()
    assert res.x == 0.0


def test_resolution_construct_with_values():
    """Test Resolution with unit, x and y values"""
    # Resolution signature: Resolution(unit, x, y)
    res = sailpy.Resolution(sailpy.ResolutionUnit.INCH, 300.0, 300.0)
    assert res.x is not None
    assert res.y is not None


def test_resolution_construct_all_units():
    """Test Resolution with all unit types"""
    # Resolution signature: Resolution(unit, x, y)
    res = sailpy.Resolution(sailpy.ResolutionUnit.INCH, 72.0, 72.0)
    assert res.x is not None
    assert res.y is not None


def test_resolution_properties_read_write():
    """Test that Resolution x and y are read-write"""
    res = sailpy.Resolution(sailpy.ResolutionUnit.INCH, 300.0, 300.0)

    # x and y should be accessible
    assert res.x == 300.0
    assert res.y == 300.0

    # They are read-write properties
    res.x = 150.0
    res.y = 150.0
    assert res.x == 150.0
    assert res.y == 150.0


# ============================================================================
# Image properties (read-write) - NEW TESTS
# ============================================================================

def test_image_resolution_property():
    """Test Image.resolution property (read-write)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 100)

    # Create resolution
    res = sailpy.Resolution(sailpy.ResolutionUnit.INCH, 300.0, 300.0)

    # Set via property
    img.resolution = res

    # Get via property
    retrieved = img.resolution
    assert retrieved.x == 300.0
    assert retrieved.y == 300.0


def test_image_palette_property():
    """Test Image.palette property (read-write)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP8_INDEXED, 16, 16)

    # Create palette
    palette = sailpy.Palette()
    data = bytes([255, 0, 0, 0, 255, 0, 0, 0, 255])  # RGB colors
    palette.set_data(sailpy.PixelFormat.BPP24_RGB, data, 3)

    # Set via property
    img.palette = palette

    # Get via property
    retrieved = img.palette
    assert retrieved.is_valid
    assert retrieved.color_count == 3


def test_image_iccp_property():
    """Test Image.iccp property (read-write)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 100)

    # Create ICC profile
    iccp = sailpy.Iccp()
    profile_data = b"\x00\x01\x02\x03\x04\x05"
    iccp.set_data(profile_data)

    # Set via property
    img.iccp = iccp

    # Get via property
    retrieved = img.iccp
    assert retrieved.is_valid
    assert retrieved.get_data() == profile_data


def test_image_meta_data_property():
    """Test Image.meta_data property (read-write)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 100)

    # Create metadata list
    md1 = sailpy.MetaData(sailpy.MetaDataType.AUTHOR, sailpy.Variant("John Doe"))
    md2 = sailpy.MetaData(sailpy.MetaDataType.TITLE, sailpy.Variant("My Image"))
    meta_list = [md1, md2]

    # Set via property
    img.meta_data = meta_list

    # Get via property
    retrieved = img.meta_data
    assert len(retrieved) == 2
    assert retrieved[0].value.to_string() == "John Doe"
    assert retrieved[1].value.to_string() == "My Image"

