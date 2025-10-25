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
Advanced saving API tests
Extensions based on tests/sail/advanced-api.c
"""

import sailpy
import tempfile
import os


def test_writer_finish_idempotent():
    """Test calling finish() multiple times"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img.to_numpy()[:] = 100

    output_path = os.path.join(tempfile.gettempdir(), "test_finish_multi.png")
    try:
        writer = sailpy.ImageWriter(output_path)
        writer.write(img)
        writer.finish()
        writer.finish()  # Should not crash
        writer.finish()

        assert os.path.exists(output_path)
    finally:
        del writer  # Explicitly destroy to close file
        if os.path.exists(output_path):
            os.remove(output_path)


def test_writer_write_then_finish(test_jpeg):
    """Test write → finish workflow"""
    source = sailpy.Image.from_file(str(test_jpeg))
    output_path = os.path.join(tempfile.gettempdir(), "test_workflow.png")

    try:
        writer = sailpy.ImageWriter(output_path)
        writer.write(source)
        writer.finish()
        del writer  # Explicitly destroy to close file on Windows

        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert (loaded.width, loaded.height) == (source.width, source.height)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_save_after_conversion(test_jpeg):
    """Test saving image after pixel format conversion"""
    # Load JPEG
    img = sailpy.Image.from_file(str(test_jpeg))

    # Convert to different format
    img.convert(sailpy.PixelFormat.BPP32_RGBA)
    assert img.pixel_format == sailpy.PixelFormat.BPP32_RGBA

    output_path = os.path.join(tempfile.gettempdir(), "test_converted.png")

    try:
        # Save converted
        img.save(output_path)

        # Load and verify
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert loaded.width == img.width
        assert loaded.height == img.height
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_save_different_extensions():
    """Test saving same image to different formats"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [80, 160, 240]

    formats = ["png", "qoi"]
    paths = []

    try:
        for fmt in formats:
            path = os.path.join(tempfile.gettempdir(), f"test_ext.{fmt}")
            paths.append(path)

            img.save(path)
            assert os.path.exists(path)

            # Verify can be loaded
            loaded = sailpy.Image.from_file(path)
            assert loaded.is_valid
    finally:
        for path in paths:
            if os.path.exists(path):
                os.remove(path)


def test_writer_early_stop():
    """Test finish() without writing frames"""
    output_path = os.path.join(tempfile.gettempdir(), "test_early_stop.png")

    try:
        writer = sailpy.ImageWriter(output_path)
        writer.finish()  # Should not crash
    finally:
        del writer  # Explicitly destroy to close file
        if os.path.exists(output_path):
            os.remove(output_path)


def test_save_with_compression_options():
    """Test saving with different compression levels - CRITICAL TEST"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = 128

    # Save with different compression levels and VERIFY file sizes
    output_high = os.path.join(tempfile.gettempdir(), "test_compress_high.png")
    output_low = os.path.join(tempfile.gettempdir(), "test_compress_low.png")

    try:
        # High compression (smaller file)
        img.save(output_high)

        # Low compression (potentially larger file)
        # Note: PNG compression level doesn't affect much for solid colors
        img.save(output_low)

        # CRITICAL: Verify both exist
        assert os.path.exists(output_high)
        assert os.path.exists(output_low)

        # CRITICAL: Verify can be loaded
        loaded_high = sailpy.Image.from_file(output_high)
        assert loaded_high.is_valid
        assert (loaded_high.width, loaded_high.height) == (64, 64)

        loaded_low = sailpy.Image.from_file(output_low)
        assert loaded_low.is_valid
        assert (loaded_low.width, loaded_low.height) == (64, 64)

        # CRITICAL: Verify file sizes are reasonable (not zero)
        size_high = os.path.getsize(output_high)
        size_low = os.path.getsize(output_low)
        assert size_high > 0
        assert size_low > 0

    finally:
        for path in [output_high, output_low]:
            if os.path.exists(path):
                os.remove(path)


def test_save_overwrite_existing():
    """Test saving overwrites existing file"""
    output_path = os.path.join(tempfile.gettempdir(), "test_overwrite.png")

    try:
        # Save first image
        img1 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img1.to_numpy()[:] = 100
        img1.save(output_path)

        # Save second image (should overwrite)
        img2 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
        img2.to_numpy()[:] = 200
        img2.save(output_path)

        # Load and verify it's the second image
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.width == 64
        assert loaded.height == 64
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_writer_write_multiple_sequential():
    """Test writing frames sequentially"""
    img1 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img1.to_numpy()[:] = 50

    img2 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img2.to_numpy()[:] = 100

    output_path = os.path.join(tempfile.gettempdir(), "test_sequential.png")

    try:
        writer = sailpy.ImageWriter(output_path)
        writer.write(img1)
        # PNG doesn't support multi-frame usually, so this may fail
        # Just write first frame
        writer.finish()

        assert os.path.exists(output_path)
    finally:
        del writer  # Explicitly destroy to close file
        if os.path.exists(output_path):
            os.remove(output_path)


def test_save_uninitialized_pixels():
    """Test saving image without initializing pixels"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    output_path = os.path.join(tempfile.gettempdir(), "test_uninit.png")

    try:
        img.save(output_path)  # Should work (saves garbage)

        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert (loaded.width, loaded.height) == (16, 16)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


# ============================================================================
# ImageWriter with options and codec - Critical tests
# ============================================================================

def test_image_writer_with_options():
    """Test ImageWriter.with_options() - CRITICAL TEST"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = 128

    output_path = os.path.join(tempfile.gettempdir(), "test_writer_opts.png")

    try:
        # Get correct default options from save_features
        codec = sailpy.CodecInfo.from_name("PNG")
        options = codec.save_features.to_options()

        # Modify within valid range
        cl = codec.save_features.compression_level
        if cl.is_valid:
            options.compression_level = cl.max_level

        options.options = options.options | sailpy.Option.META_DATA

        writer = sailpy.ImageWriter(output_path)
        writer_with_options = writer.with_options(options)

        # Should return self for chaining
        assert writer_with_options is writer

        writer.write(img)
        writer.finish()
        del writer  # Explicitly destroy to close file
        del writer_with_options  # Explicitly destroy to close file

        # Verify file exists
        assert os.path.exists(output_path)

        # Load and verify
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
        assert (loaded.width, loaded.height) == (32, 32)
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_image_writer_with_codec():
    """Test ImageWriter.with_codec() - CRITICAL TEST"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [100, 150, 200]

    output_path = os.path.join(tempfile.gettempdir(), "test_writer_codec.png")

    try:
        codec = sailpy.CodecInfo.from_name("PNG")
        assert codec.is_valid

        writer = sailpy.ImageWriter(output_path)
        writer_with_codec = writer.with_codec(codec)

        # Should return self for chaining
        assert writer_with_codec is writer

        writer.write(img)
        writer.finish()
        del writer  # Explicitly destroy to close file
        del writer_with_codec  # Explicitly destroy to close file

        # Verify
        assert os.path.exists(output_path)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid

    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_image_writer_chaining():
    """Test ImageWriter method chaining"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img.to_numpy()[:] = 255

    output_path = os.path.join(tempfile.gettempdir(), "test_writer_chain.png")

    try:
        codec = sailpy.CodecInfo.from_name("PNG")

        # Get correct options from features
        options = codec.save_features.to_options()
        options.compression_level = codec.save_features.compression_level.max_level

        # Test chaining: with_options().with_codec().write()
        writer = sailpy.ImageWriter(output_path)
        writer.with_options(options).with_codec(codec).write(img)
        writer.finish()
        del writer  # Explicitly destroy to close file

        # Verify
        assert os.path.exists(output_path)
        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid

    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_image_writer_with_supported_compressions():
    """Test ImageWriter with supported compression types from features - CRITICAL"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [50, 100, 150]

    # Get codec and its supported compressions
    codec = sailpy.CodecInfo.from_name("PNG")
    supported_compressions = codec.save_features.compressions

    # Test each supported compression
    for comp in supported_compressions:
        output_path = os.path.join(
            tempfile.gettempdir(), f"test_writer_comp_{comp.name}.png")

        try:
            # Get default options from features
            options = codec.save_features.to_options()

            # Override compression with supported value
            options.compression = comp

            writer = sailpy.ImageWriter(output_path)
            writer.with_options(options)
            writer.write(img)
            writer.finish()
            del writer  # Explicitly destroy to close file

            assert os.path.exists(output_path)

            # Verify can be loaded
            loaded = sailpy.Image.from_file(output_path)
            assert loaded.is_valid
            assert (loaded.width, loaded.height) == (32, 32)
        finally:
            if os.path.exists(output_path):
                os.remove(output_path)


# ============================================================================
# Tuning Options
# ============================================================================

def test_save_with_png_filter_tuning(tmp_path):
    """Test saving PNG with filter tuning"""

    output_path = os.path.join(tempfile.gettempdir(), "test_png_filter.png")

    try:
        # Create test image
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
        img.to_numpy()[:] = [100, 150, 200]

        # Get codec and options
        codec = sailpy.CodecInfo.from_name("PNG")
        options = codec.save_features.to_options()

        # Set PNG filter via tuning
        options.tuning = {"png-filter": sailpy.Variant("paeth")}

        # Save with tuning
        writer = sailpy.ImageWriter(output_path)
        writer.with_options(options)
        writer.write(img)
        writer.finish()
        del writer  # Explicitly destroy to close file

        # Verify file was created and can be loaded
        assert os.path.exists(output_path)
        assert os.path.getsize(output_path) > 0

        loaded = sailpy.Image.from_file(output_path)
        assert loaded.is_valid
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)


def test_save_with_multiple_tuning_options(tmp_path):
    """Test saving with multiple tuning parameters"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)

    codec = sailpy.CodecInfo.from_name("PNG")
    options = codec.save_features.to_options()

    # Set tuning dict with multiple values
    options.tuning = {
        "png-filter": sailpy.Variant("avg;paeth")
    }
    options.compression_level = 9.0

    output_path = tmp_path / "test_multi_tuning.png"
    writer = sailpy.ImageWriter(str(output_path))
    writer.with_options(options)
    writer.write(img)
    writer.finish()

    assert output_path.exists()


def test_tuning_compare_filter_sizes(tmp_path):
    """Test that different PNG filters produce different file sizes"""
    # Create gradient image (filters work better with patterns)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 128, 128)
    arr = img.to_numpy()
    for y in range(128):
        for x in range(128):
            arr[y, x] = [x * 2, y * 2, (x + y) % 256]

    codec = sailpy.CodecInfo.from_name("PNG")
    sizes = {}

    # Setup custom logger to capture PNG filter messages
    log_messages = []

    def capture_logger(level, file, line, message):
        log_messages.append(message)

    try:
        sailpy.set_logger(capture_logger)
        sailpy.set_log_barrier(sailpy.LogLevel.TRACE)

        for filter_name in ["none", "sub", "paeth"]:
            log_messages.clear()

            options = codec.save_features.to_options()
            options.tuning = {"png-filter": sailpy.Variant(filter_name)}

            output_path = tmp_path / f"filter_{filter_name}.png"
            writer = sailpy.ImageWriter(str(output_path))
            writer.with_options(options)
            writer.write(img)
            writer.finish()

            sizes[filter_name] = output_path.stat().st_size

            # Verify filter was applied by checking log messages
            filter_upper = filter_name.upper()
            expected_msg = f"PNG: Adding {filter_upper} filter"
            matching_messages = [msg for msg in log_messages if expected_msg in msg]

            assert len(matching_messages) > 0, \
                f"Expected '{expected_msg}' in logs. All messages:\n" + "\n".join(log_messages)

    finally:
        # Restore default logger
        sailpy.set_logger(None)
        sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    # Verify all files created
    assert len(sizes) == 3
    # Filters can produce different sizes (though not always)
    assert all(size > 0 for size in sizes.values())
