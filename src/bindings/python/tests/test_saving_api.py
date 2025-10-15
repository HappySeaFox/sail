
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
Tests for SAIL Python Saving API
"""

import sailpy
import os
import tempfile


def test_image_writer_single_frame():
    """Test ImageWriter for single frame"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    arr = img.to_numpy()
    arr[:] = [0, 255, 0]  # Green

    # Use temp directory
    output_path = os.path.join(
        tempfile.gettempdir(), "sail_test_writer_single.png")

    try:
        # Remove if exists
        if os.path.exists(output_path):
            os.remove(output_path)

        # Write using ImageWriter
        writer = sailpy.ImageWriter(output_path)
        writer.write(img)
        writer.finish()
        del writer  # Trigger destructor to close file

        # Verify file was created and can be loaded
        assert os.path.exists(output_path)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        # Cleanup
        if os.path.exists(output_path):
            os.remove(output_path)


def test_image_save_method():
    """Test Image.save() method (convenience)"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 24, 24)
    arr = img.to_numpy()
    arr[:] = [128, 128, 128]  # Gray

    output_path = os.path.join(
        tempfile.gettempdir(), "sail_test_save_method.png")

    try:
        # Remove if exists
        if os.path.exists(output_path):
            os.remove(output_path)

        # Save using instance method
        img.save(output_path)

        # Verify
        assert os.path.exists(output_path)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert loaded.width == 24
        assert loaded.height == 24
    finally:
        # Cleanup
        if os.path.exists(output_path):
            os.remove(output_path)


def test_image_writer_repr():
    """Test ImageWriter string representation"""
    output_path = os.path.join(
        tempfile.gettempdir(), "sail_test_writer_repr.png")

    try:
        # Remove if exists
        if os.path.exists(output_path):
            os.remove(output_path)

        writer = sailpy.ImageWriter(output_path)
        assert "ImageWriter" in repr(writer)
        writer.finish()
        del writer  # Trigger destructor
    finally:
        # Cleanup
        if os.path.exists(output_path):
            os.remove(output_path)
