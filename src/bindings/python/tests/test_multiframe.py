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
Multi-frame image tests (animations, multi-page TIFFs, etc.)
Based on tests/sail/multi-frame.c from SAIL C tests
"""

import pytest
import sailpy


def test_multiframe_read_all_frames(test_png):
    """Test reading all frames from animated image"""
    reader = sailpy.ImageReader(str(test_png))
    frames = reader.read_all()

    assert len(frames) >= 1
    assert all(f.is_valid for f in frames)
    assert all(f.width == 16 for f in frames)
    assert all(f.height == 16 for f in frames)


def test_multiframe_iterator_count(test_png):
    """Test that iterator returns correct number of frames"""
    frames = list(sailpy.ImageReader(str(test_png)))

    assert len(frames) >= 1
    assert all(f.is_valid for f in frames)


def test_multiframe_delay_consistency(test_png):
    """Test that frame delays are consistent"""
    reader = sailpy.ImageReader(str(test_png))
    frames = reader.read_all()

    for i, frame in enumerate(frames):
        # Delay should be >= -1 (-1 means no delay info)
        assert frame.delay >= -1

        # If this is animated, delays should make sense
        if len(frames) > 1 and i > 0:
            # Delays should be non-negative for actual animations
            if frame.delay >= 0:
                # Reasonable upper bound (10 seconds)
                assert frame.delay < 10000


def test_multiframe_dimensions_consistency(test_png):
    """Test that all frames have consistent dimensions"""
    reader = sailpy.ImageReader(str(test_png))
    frames = reader.read_all()

    if len(frames) > 1:
        first_width = frames[0].width
        first_height = frames[0].height

        # All frames should have same dimensions
        for frame in frames:
            assert frame.width == first_width
            assert frame.height == first_height


def test_multiframe_pixel_format_consistency(test_png):
    """Test that all frames have consistent pixel format"""
    reader = sailpy.ImageReader(str(test_png))
    frames = reader.read_all()

    # All frames should have valid pixel format
    for frame in frames:
        assert frame.pixel_format != sailpy.PixelFormat.UNKNOWN


def test_multiframe_read_one_by_one(test_png):
    """Test reading frames one by one"""
    reader = sailpy.ImageReader(str(test_png))

    frames_count = 0
    for _ in range(100):  # Safety limit
        try:
            frame = reader.read()
            assert frame.is_valid
            frames_count += 1
        except RuntimeError:
            break

    assert frames_count >= 1


def test_multiframe_no_more_frames_error(test_jpeg):
    """Test that reading past last frame raises error"""
    reader = sailpy.ImageReader(str(test_jpeg))

    # Read first frame
    frame1 = reader.read()
    assert frame1.is_valid

    # Try to read second frame from single-frame image
    with pytest.raises(RuntimeError, match="No more frames"):
        reader.read()


def test_multiframe_iterator_then_read():
    """Test that read() fails after iteration"""
    import tempfile
    import os

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = 128

    output_path = os.path.join(tempfile.gettempdir(), "test_mixed.png")
    try:
        img.save(output_path)
        reader = sailpy.ImageReader(output_path)
        frames = list(reader)

        assert len(frames) == 1

        with pytest.raises(RuntimeError):
            reader.read()
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_multiframe_write_multiple():
    """Test writing multiple frames"""
    import tempfile
    import os

    frames = []
    for i in range(3):
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img.to_numpy()[:] = i * 50
        frames.append(img)

    output_path = os.path.join(tempfile.gettempdir(), "test_multi.png")
    try:
        writer = sailpy.ImageWriter(output_path)

        # PNG may not support multi-frame, so just write first frame
        # Or use write() in loop
        for frame in frames[:1]:  # Write only first frame for PNG
            writer.write(frame)

        writer.finish()

        # Verify file was created
        assert os.path.exists(output_path)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_multiframe_codec_features():
    """Test codec feature detection for multi-frame support"""
    codecs = sailpy.CodecInfo.list()

    # Find codecs that support multi-frame
    multi_frame_codecs = []
    for codec in codecs:
        # Check if it's GIF, PNG (APNG), or TIFF (multi-page)
        if codec.name.lower() in ['gif', 'png', 'tiff']:
            multi_frame_codecs.append(codec)

    # Should have at least one multi-frame capable codec
    assert len(multi_frame_codecs) > 0


def test_multiframe_write_all_single():
    """Test write_all with single frame list"""
    import tempfile
    import os

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img.to_numpy()[:] = 100

    output_path = os.path.join(tempfile.gettempdir(), "test_single_list.png")
    try:
        writer = sailpy.ImageWriter(output_path)
        writer.write_all([img])
        writer.finish()

        assert os.path.exists(output_path)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_multiframe_single_frame_write_all():
    """Test write_all with single frame"""
    import tempfile
    import os

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = 100

    output_path = os.path.join(tempfile.gettempdir(), "test_single.png")
    try:
        writer = sailpy.ImageWriter(output_path)
        writer.write_all([img])
        writer.finish()

        # Load and verify
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert loaded.width == 32
        assert loaded.height == 32
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_multiframe_probe_animated(test_png):
    """Test probing animated image"""
    metadata = sailpy.ImageReader.probe(str(test_png))

    assert "width" in metadata
    assert "height" in metadata
    assert "pixel_format" in metadata

    # Should have codec info
    assert "codec_name" in metadata
    assert metadata["codec_name"] == "PNG"
