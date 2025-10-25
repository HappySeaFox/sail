#!/usr/bin/env python3

#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License
#

"""
SAIL Python Bindings - Advanced Saving with Tuning Options

Demonstrates codec-specific tuning options, specifically PNG filters.
Compares file sizes with different PNG filter configurations.
"""

import sailpy
import os
import tempfile


def main():
    print("=" * 70)
    print(" SAIL Python - Advanced Saving with PNG Filters")
    print("=" * 70)
    print()

    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # Create test image (gradient for better compression testing)
    print("[1/4] Creating test image (256x256 gradient)")
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()

    # Create gradient pattern (compresses differently with different filters)
    for y in range(256):
        for x in range(256):
            arr[y, x] = [x, y, (x + y) % 256]

    print("      Image created with RGB gradient pattern")
    print()

    # Get PNG codec and default options
    codec = sailpy.CodecInfo.from_name("PNG")
    base_options = codec.save_features.to_options()

    # Test configurations
    configs = [
        ("none", "No filtering (fastest, largest)"),
        ("sub", "SUB filter (horizontal prediction)"),
        ("paeth", "PAETH filter (adaptive, best compression)"),
    ]

    results = []

    # Save with different PNG filters
    print("[2/4] Saving with different PNG filters")
    print()

    for filter_name, description in configs:
        # Create fresh options
        options = codec.save_features.to_options()

        # Set PNG filter via tuning, see https://sail.software/formats.html
        options.tuning["png-filter"] = sailpy.Variant(filter_name)

        # Save to temp file
        output_path = os.path.join(
            tempfile.gettempdir(), f"test_filter_{filter_name}.png")

        writer = sailpy.ImageWriter(output_path)
        writer.with_options(options)
        writer.write(img)
        writer.finish()

        # Get file size
        file_size = os.path.getsize(output_path)
        results.append((filter_name, description, file_size, output_path))

        print(f"  Filter '{filter_name:6s}': {
              file_size:,} bytes - {description}")

    print()

    # Compare sizes
    print("[3/4] Size comparison")
    print()

    smallest = min(results, key=lambda x: x[2])
    largest = max(results, key=lambda x: x[2])

    print(f"  Smallest: '{smallest[0]}' = {smallest[2]:,} bytes")
    print(f"  Largest:  '{largest[0]}' = {largest[2]:,} bytes")
    print(f"  Difference: {
          largest[2] - smallest[2]:,} bytes ({(largest[2] / smallest[2] - 1) * 100:.1f}% larger)")
    print()

    # Demonstrate tuning as dict
    print("[4/4] Tuning options work as dict[str, Variant]")
    print()

    # Create new tuning dict
    tuning_dict = {
        "png-filter": sailpy.Variant("avg;paeth")
    }

    options = codec.save_features.to_options()
    options.tuning = tuning_dict
    options.compression_level = 9.0

    # Save with combined filters
    combined_path = os.path.join(
        tempfile.gettempdir(), "test_filter_combined.png")
    writer = sailpy.ImageWriter(combined_path)
    writer.with_options(options)
    writer.write(img)
    writer.finish()

    combined_size = os.path.getsize(combined_path)
    print(f"  Combined filters 'avg;paeth': {combined_size:,} bytes")
    print(f"  tuning is a Python dict with Variant values")

    os.remove(combined_path)
    print()

    # Cleanup
    for _, _, _, path in results:
        os.remove(path)

    print("=" * 70)
    print("✓ PNG filters affect file size!")
    print("  Use 'paeth' for best compression, 'none' for speed")
    print("=" * 70)
    print()


if __name__ == "__main__":
    main()
