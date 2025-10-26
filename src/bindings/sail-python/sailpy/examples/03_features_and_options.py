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
SAIL Python Bindings - Features and Options Examples

Real-world examples showing how to use codec features to get
correct default values for compression, options, and more.
"""

import os
import sailpy
import tempfile


def example_get_codec_features():
    """Discover what a codec can do"""
    print("=" * 70)
    print("EXAMPLE 1: Discovering Codec Features")
    print("=" * 70)

    # Get codec info
    codec = sailpy.CodecInfo.from_name("PNG")

    print(f"\nCodec: {codec.name} v{codec.version}")
    print(f"Description: {codec.description}")

    # Load features
    lf = codec.load_features
    print(f"\nLoad features: {lf.features}")
    if lf.features & sailpy.CodecFeature.META_DATA:
        print("  Y Can load metadata (EXIF, comments)")
    if lf.features & sailpy.CodecFeature.ICCP:
        print("  Y Can load ICC profiles")

    # Save features
    sf = codec.save_features
    print(f"\nSave features: {sf.features}")
    print(f"  Default compression: {sf.default_compression}")
    print(f"  Supported compressions: {[c.name for c in sf.compressions]}")
    print(f"  Supported pixel formats: {len(sf.pixel_formats)} formats")

    # Compression level range
    cl = sf.compression_level
    if cl.is_valid:
        print(f"\n  Compression level range:")
        print(f"    Min: {cl.min_level}")
        print(f"    Max: {cl.max_level}")
        print(f"    Default: {cl.default_level}")
        print(f"    Step: {cl.step}")

    print()


def example_use_default_options():
    """Get correct default options from codec features"""
    print("=" * 70)
    print("EXAMPLE 2: Using Default Options from Features")
    print("=" * 70)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = [100, 150, 200]

    # Get codec
    codec = sailpy.CodecInfo.from_name("PNG")

    # Y CORRECT WAY: Get defaults from features
    options = codec.save_features.to_options()

    print(f"\nDefault options from PNG codec:")
    print(f"  Compression: {options.compression}")
    print(f"  Compression level: {options.compression_level}")
    print(f"  Options flags: {options.options}")

    # Save with defaults
    output_path = os.path.join(tempfile.gettempdir(), "default_options.png")
    try:
        output = sailpy.ImageOutput(output_path)
        output.with_options(options)
        output.save(img)
        output.finish()

        print(f"\nY Saved with codec defaults: {output_path}")
        print(f"  File size: {os.path.getsize(output_path)} bytes")
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)

    print()


def example_custom_compression():
    """Customize compression within valid range"""
    print("=" * 70)
    print("EXAMPLE 3: Custom Compression Settings")
    print("=" * 70)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 128, 128)
    img.to_numpy()[:] = [80, 120, 160]

    codec = sailpy.CodecInfo.from_name("PNG")
    cl = codec.save_features.compression_level

    # Save with different compression levels
    levels = {
        "minimum": cl.min_level,
        "default": cl.default_level,
        "maximum": cl.max_level,
    }

    print(f"\nTesting different compression levels:")

    for name, level in levels.items():
        output_path = os.path.join(
            tempfile.gettempdir(), f"compress_{name}.png")

        try:
            # Get default options
            options = codec.save_features.to_options()

            # Customize compression level
            options.compression_level = level

            output = sailpy.ImageOutput(output_path)
            output.with_options(options)
            output.save(img)
            output.finish()

            size = os.path.getsize(output_path)
            print(f"  {name:8} (level {level}): {size:,} bytes")

        finally:
            if os.path.exists(output_path):
                os.remove(output_path)

    print()


def example_option_flags():
    """Use Option flags to control save behavior"""
    print("=" * 70)
    print("EXAMPLE 4: Using Option Flags")
    print("=" * 70)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [200, 100, 50]

    codec = sailpy.CodecInfo.from_name("PNG")

    # Get default options
    options = codec.save_features.to_options()

    print(f"\nDefault options: {options.options}")

    # Add metadata option
    options.options = options.options | sailpy.Option.META_DATA
    print(f"With META_DATA: {options.options}")

    # Add ICC profile option
    options.options = options.options | sailpy.Option.ICCP
    print(f"With META_DATA | ICCP: {options.options}")

    # Check if flag is set
    if options.options & sailpy.Option.META_DATA:
        print("  Y META_DATA flag is set")

    output_path = os.path.join(tempfile.gettempdir(), "with_options.png")
    try:
        output = sailpy.ImageOutput(output_path)
        output.with_options(options)
        output.save(img)
        output.finish()

        print(f"\nY Saved with custom options")
    finally:
        if os.path.exists(output_path):
            os.remove(output_path)

    print()


def example_codec_specific_compression():
    """Use codec-specific compression types"""
    print("=" * 70)
    print("EXAMPLE 5: Codec-Specific Compression Types")
    print("=" * 70)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    img.to_numpy()[:] = [150, 100, 200]

    # Test different codecs with their supported compressions
    test_codecs = ["png", "webp"]

    for codec_name in test_codecs:
        codec = sailpy.CodecInfo.from_name(codec_name)
        if not codec.can_save:
            continue

        print(f"\n{codec.name} codec:")
        print(f"  Supported compressions: {[c.name for c in codec.save_features.compressions]}")
        print(f"  Default: {codec.save_features.default_compression.name}")

        # Use each supported compression
        # Test first 2
        for compression in codec.save_features.compressions[:2]:
            output_path = os.path.join(tempfile.gettempdir(),
                                       f"test_{codec_name}_{compression.name}.{codec_name}")

            try:
                # Get defaults
                options = codec.save_features.to_options()

                # Override compression
                options.compression = compression

                output = sailpy.ImageOutput(output_path)
                output.with_options(options)
                output.save(img)
                output.finish()

                size = os.path.getsize(output_path)
                print(f"    {compression.name}: {size:,} bytes")

            finally:
                if os.path.exists(output_path):
                    os.remove(output_path)

    print()


def example_check_format_support():
    """Check if a pixel format is supported by codec"""
    print("=" * 70)
    print("EXAMPLE 6: Checking Pixel Format Support")
    print("=" * 70)

    codec = sailpy.CodecInfo.from_name("PNG")
    test_formats = [
        sailpy.PixelFormat.BPP24_RGB,
        sailpy.PixelFormat.BPP32_RGBA,
        sailpy.PixelFormat.BPP48_RGB,
        sailpy.PixelFormat.BPP8_GRAYSCALE,
    ]

    print(f"\n{codec.name} pixel format support:")

    for pf in test_formats:
        supported = pf in codec.save_features.pixel_formats
        status = "Y" if supported else "N"
        print(f"  {status} {pf.name}")

    print()


def example_source_image_info():
    """Access original image properties"""
    print("=" * 70)
    print("EXAMPLE 7: Source Image Properties")
    print("=" * 70)

    # Create test image
    test_path = os.path.join(tempfile.gettempdir(), "test_source.png")
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.save(test_path)

    try:
        # Load and check source properties
        loaded = sailpy.Image.from_file(test_path)

        source = loaded.source_image
        print(f"\nSource image properties:")
        print(f"  Valid: {source.is_valid}")
        if source.is_valid:
            print(f"  Original pixel format: {source.pixel_format}")
            print(f"  Original compression: {source.compression}")
            print(f"  Chroma subsampling: {source.chroma_subsampling}")
            print(f"  Orientation: {source.orientation}")
            print(f"  Interlaced: {source.interlaced}")

    finally:
        if os.path.exists(test_path):
            os.remove(test_path)

    print()


def main():
    """Run all feature and option examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL Features & Options Examples")
    print("=" * 70 + "\n")

    try:
        example_get_codec_features()
        example_use_default_options()
        example_custom_compression()
        example_option_flags()
        example_codec_specific_compression()
        example_check_format_support()
        example_source_image_info()

        print("=" * 70)
        print("Y All examples completed!")
        print()

    finally:
        sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)


if __name__ == "__main__":
    main()
