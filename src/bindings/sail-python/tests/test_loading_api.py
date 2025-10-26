
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
Tests for SAIL Python Loading API
"""

import pytest
import sailpy


def test_image_from_file(test_png):
    """Test Image.from_file() static method"""
    img = sailpy.Image.from_file(str(test_png))

    assert img.is_valid
    assert img.width == 16
    assert img.height == 16


def test_image_from_bytes(test_jpeg):
    """Test Image.from_bytes() static method"""
    # Load file as bytes
    with open(test_jpeg, "rb") as f:
        data = f.read()

    img = sailpy.Image.from_bytes(data)

    assert img.is_valid
    assert img.width == 32
    assert img.height == 32


def test_image_input_single_frame(test_jpeg):
    """Test ImageInput for single frame image"""
    input = sailpy.ImageInput(str(test_jpeg))

    # Read first frame
    img = input.load()
    assert img.is_valid
    assert img.width == 32
    assert img.height == 32

    # Try to read second frame (should fail for single frame)
    with pytest.raises(RuntimeError, match="No more frames"):
        input.load()


def test_image_input_iterator(test_png):
    """Test ImageInput as iterator"""
    frames = []

    # This PNG is animated
    for frame in sailpy.ImageInput(str(test_png)):
        frames.append(frame)
        if len(frames) >= 10:  # Safety limit
            break

    # Should have at least 1 frame
    assert len(frames) >= 1
    assert all(f.is_valid for f in frames)


def test_image_input_load_all(test_png):
    """Test ImageInput.load_all()"""
    input = sailpy.ImageInput(str(test_png))
    frames = input.load_all()

    assert len(frames) >= 1
    assert all(f.is_valid for f in frames)
    assert all(f.width == 16 for f in frames)
    assert all(f.height == 16 for f in frames)


def test_image_input_context_manager(test_jpeg):
    """Test ImageInput as context manager"""
    with sailpy.ImageInput(str(test_jpeg)) as input:
        img = input.load()
        assert img.is_valid

    # After context manager, input should be finished
    # (no way to test this directly, but it shouldn't crash)


def test_image_input_probe(test_png):
    """Test ImageInput.probe() for metadata"""
    metadata = sailpy.ImageInput.probe(str(test_png))

    assert "width" in metadata
    assert "height" in metadata
    assert "pixel_format" in metadata
    assert "codec_name" in metadata
    assert "codec_description" in metadata

    assert metadata["width"] == 16
    assert metadata["height"] == 16
    assert metadata["codec_name"] == "PNG"


def test_image_input_repr(test_png):
    """Test ImageInput string representation"""
    input = sailpy.ImageInput(str(test_png))
    assert "ImageInput" in repr(input)


def test_multiple_inputs(test_png, test_jpeg):
    """Test using multiple inputs simultaneously"""
    input1 = sailpy.ImageInput(str(test_png))
    input2 = sailpy.ImageInput(str(test_jpeg))

    img1 = input1.load()
    img2 = input2.load()

    assert img1.is_valid
    assert img2.is_valid
    assert img1.width == 16
    assert img2.width == 32
