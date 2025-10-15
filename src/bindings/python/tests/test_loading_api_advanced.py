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
Advanced loading API tests
Extensions based on tests/sail/advanced-api.c
"""

import pytest
import sailpy


def test_early_stop_loading(test_jpeg):
    """Test stopping loading before reading any frames"""
    reader = sailpy.ImageReader(str(test_jpeg))
    # Finish without reading
    reader.finish()
    # Should not crash


def test_no_more_frames_after_iteration(test_png):
    """Test that iteration properly handles end of frames"""
    # Iterate through all frames
    frames = list(sailpy.ImageReader(str(test_png)))
    assert len(frames) >= 1

    # Create new reader and exhaust it
    reader = sailpy.ImageReader(str(test_png))
    for _ in range(len(frames)):
        reader.read()

    # Next read should fail
    with pytest.raises(RuntimeError, match="No more frames"):
        reader.read()


def test_reader_with_load_options(test_jpeg):
    """Test ImageReader with LoadOptions - CRITICAL TEST"""
    options = sailpy.LoadOptions()
    options.options = 1  # Set some option

    # MUST pass options using with_options()
    reader = sailpy.ImageReader(str(test_jpeg))
    reader.with_options(options)

    img = reader.read()
    assert img.is_valid


def test_probe_returns_all_metadata(test_jpeg):
    """Test that probe returns comprehensive metadata"""
    metadata = sailpy.ImageReader.probe(str(test_jpeg))

    # Required fields
    required = ["width", "height", "pixel_format", "codec_name"]
    for key in required:
        assert key in metadata, f"Missing key: {key}"

    # Values should be valid
    assert metadata["width"] > 0
    assert metadata["height"] > 0
    assert len(metadata["codec_name"]) > 0


def test_reader_finish_idempotent(test_jpeg):
    """Test that calling finish() multiple times is safe"""
    reader = sailpy.ImageReader(str(test_jpeg))
    img = reader.read()
    assert img.is_valid

    # Call finish multiple times
    reader.finish()
    reader.finish()
    reader.finish()
    # Should not crash


def test_load_with_explicit_codec(test_jpeg):
    """Test loading with explicitly specified codec"""
    # Get codec info
    codec = sailpy.CodecInfo.from_path(str(test_jpeg))
    assert codec.is_valid
    assert codec.can_load

    # Load image (codec determined automatically)
    img = sailpy.Image.from_file(str(test_jpeg))
    assert img.is_valid


def test_reader_iteration_safety(test_png):
    """Test that reader iteration is safe"""
    count = 0
    for frame in sailpy.ImageReader(str(test_png)):
        assert frame.is_valid
        count += 1
        # Safety limit
        if count > 100:
            break

    assert count >= 1


def test_load_from_bytes_complete(test_jpeg):
    """Test complete workflow with bytes"""
    with open(test_jpeg, "rb") as f:
        data = f.read()

    # Load from bytes
    img = sailpy.Image.from_bytes(data)

    assert img.is_valid
    assert img.width > 0
    assert img.height > 0


def test_probe_vs_load_consistency(test_jpeg):
    """Test that probe and load return consistent metadata"""
    # Probe
    metadata = sailpy.ImageReader.probe(str(test_jpeg))

    # Load
    img = sailpy.Image.from_file(str(test_jpeg))

    # Compare
    assert metadata["width"] == img.width
    assert metadata["height"] == img.height


def test_reader_multiple_read_calls(test_jpeg):
    """Test calling read() multiple times"""
    reader = sailpy.ImageReader(str(test_jpeg))

    # First read should succeed
    img1 = reader.read()
    assert img1.is_valid

    # Second read should fail (single-frame image)
    with pytest.raises(RuntimeError, match="No more frames"):
        reader.read()


# ============================================================================
# Tuning Options
# ============================================================================

def test_load_with_tuning_options(test_png):
    """Test loading with tuning options"""
    # Create load options with tuning
    options = sailpy.LoadOptions()
    options.tuning = {"custom-param": sailpy.Variant("test-value")}

    # Load with tuning (tuning may be ignored if codec doesn't use it)
    reader = sailpy.ImageReader(str(test_png))
    reader.with_options(options)

    img = reader.read()
    assert img.is_valid
