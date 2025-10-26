#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - Codec Discovery Examples

Discovering available codecs, checking capabilities, and finding codecs
by extension or MIME type.
"""

import sailpy


def main():
    """Codec discovery examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL Codec Discovery")
    print("=" * 50 + "\n")

    # Example 1: List all codecs
    print("=" * 50)
    print("EXAMPLE 1: List All Available Codecs")
    print("=" * 50 + "\n")

    codecs = sailpy.CodecInfo.list()
    print(f"Total codecs: {len(codecs)}\n")

    for codec in codecs:
        load = "Y" if codec.can_load else "N"
        save = "Y" if codec.can_save else "N"
        exts = ', '.join(codec.extensions[:3])
        if len(codec.extensions) > 3:
            exts += "..."
        print(f"{codec.name:12} v{codec.version:8}  Load:{load}  Save:{save}  [{exts}]")

    print()

    # Example 2: Get codec by extension
    print("=" * 50)
    print("EXAMPLE 2: Find Codec by Extension")
    print("=" * 50 + "\n")

    jpeg_codec = sailpy.CodecInfo.from_name("JPEG") # case insensitive
    print(f"JPEG codec (by name 'JPEG'):")
    print(f"  Name: {jpeg_codec.name}")
    print(f"  Extensions: {', '.join(jpeg_codec.extensions)}")
    print(f"  MIME types: {', '.join(jpeg_codec.mime_types)}")
    print(f"  Can load: {jpeg_codec.can_load}")
    print(f"  Can save: {jpeg_codec.can_save}")

    print()

    # Example 3: Get codec by MIME type
    print("=" * 50)
    print("EXAMPLE 3: Find Codec by MIME Type")
    print("=" * 50 + "\n")

    png_codec = sailpy.CodecInfo.from_mime_type("image/png")
    print(f"PNG codec (by MIME 'image/png'):")
    print(f"  Name: {png_codec.name}")
    print(f"  Version: {png_codec.version}")
    print(f"  Description: {png_codec.description}")

    print()

    # Example 4: Check codec capabilities
    print("=" * 50)
    print("EXAMPLE 4: Check Codec Capabilities")
    print("=" * 50 + "\n")

    test_codecs = ["PNG", "JPEG", "WEBP"]

    for codec_name in test_codecs:
        try:
            codec = sailpy.CodecInfo.from_name(codec_name)
            print(f"{codec.name}:")
            print(f"  Load: {'Y' if codec.can_load else 'N'}")
            print(f"  Save: {'Y' if codec.can_save else 'N'}")
            print(f"  MIME types: {', '.join(codec.mime_types)}")
            print(f"  Magic numbers: {codec.magic_numbers[0] if codec.magic_numbers else 'none'}")
            print(f"  Extensions: {', '.join(codec.extensions)}")

            if codec.can_save:
                print(f"  Save pixel formats: {len(codec.save_features.pixel_formats)} formats")
                print(f"  Save compressions: {', '.join(c.name for c in codec.save_features.compressions)}")
                print(f"  Save default compression: {codec.save_features.default_compression.name}")
                print(f"  Compression level: {codec.save_features.compression_level}")
            print()
        except ValueError:
            print(f"{codec_name}: codec not available\n")

    print("=" * 50)
    print("+ All examples completed!")
    print()


if __name__ == "__main__":
    main()
