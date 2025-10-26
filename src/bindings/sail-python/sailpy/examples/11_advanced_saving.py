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
    print("=" * 50)
    print(" SAIL Python - Advanced Saving with PNG Filters")
    print("=" * 50)
    print()

    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    # Create test image (larger with details and noise for better filter comparison)
    print("[1/4] Creating 256x256 test image with patterns and noise")
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()

    # Create complex pattern with gradients, stripes, and noise
    import random
    random.seed(42)  # For reproducibility

    for y in range(img.height):
        for x in range(img.width):
            # Base gradient
            r = (x * 256) // img.width
            g = (y * 256) // img.height
            b = ((x + y) * 256) // (img.width + img.height)

            # Add vertical stripes (high frequency detail)
            if x % 8 < 4:
                r = min(255, r + 30)

            # Add horizontal stripes
            if y % 16 < 8:
                g = min(255, g + 20)

            # Add checkerboard pattern in some areas
            if (x // 32 + y // 32) % 2 == 0 and x < 512:
                b = min(255, b + 40)

            # Add random noise (5% of pixels)
            if random.random() < 0.05:
                noise = random.randint(-30, 30)
                r = max(0, min(255, r + noise))
                g = max(0, min(255, g + noise))
                b = max(0, min(255, b + noise))

            arr[y, x] = [r, g, b]

    print()

    # Get PNG codec and default options
    codec = sailpy.CodecInfo.from_name("PNG")
    base_options = codec.save_features.to_options()

    # Test configurations - all PNG filter types
    configs = [
        ("none",  "no filtering, fastest, largest"),
        ("sub",   "SUB filter, horizontal prediction"),
        ("up",    "UP filter, vertical prediction"),
        ("avg",   "average filter, avg of left and above"),
        ("paeth", "PAETH filter, adaptive, usually best"),
    ]

    results = []

    # Save with different PNG filters
    print("[2/4] Saving with different PNG filters")
    print()

    for filter_name, description in configs:
        # Create fresh options
        options = codec.save_features.to_options()

        # Set PNG filter via tuning, see https://sail.software/formats.html
        # IMPORTANT: Must set the whole dict, not individual keys
        options.tuning = {"png-filter": sailpy.Variant(filter_name)}

        # Save to temp file
        output_path = os.path.join(
            tempfile.gettempdir(), f"test_filter_{filter_name}.png")

        output = sailpy.ImageOutput(output_path)
        output.with_options(options)
        output.save(img)
        output.finish()

        # Get file size
        file_size = os.path.getsize(output_path)
        results.append((filter_name, description, file_size, output_path))

        print(f"  Filter {filter_name:6s}: {file_size:,} bytes ({description})")

    print()

    # Compare sizes
    print("[3/4] Size comparison")
    print()

    smallest = min(results, key=lambda x: x[2])
    largest = max(results, key=lambda x: x[2])

    print(f"  Smallest: {smallest[0]:6s} = {smallest[2]:,} bytes (100%)")
    print(f"  Largest:  {largest[0]:6s} = {largest[2]:,} bytes "
          f"({(largest[2] / smallest[2]) * 100:.1f}%)")
    print(f"  Difference: {largest[2] - smallest[2]:,} bytes saved by choosing best filter")
    print()
    print("  All filters (sorted by size):")

    # Sort and show all
    sorted_results = sorted(results, key=lambda x: x[2])
    for filter_name, desc, size, _ in sorted_results:
        percent = (size / smallest[2]) * 100
        print(f"    {filter_name:6s}: {size:,} bytes ({percent:.1f}%)")

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
    output = sailpy.ImageOutput(combined_path)
    output.with_options(options)
    output.save(img)
    output.finish()

    combined_size = os.path.getsize(combined_path)
    print(f"  Combined filters 'avg;paeth': {combined_size:,} bytes")
    print(f"  tuning is a Python dict with Variant values")

    os.remove(combined_path)
    print()

    # Cleanup
    for _, _, _, path in results:
        os.remove(path)

    print("=" * 50)
    print("+ PNG filters affect file size")
    print("  - Different filters work better for different image types")
    print("  - Combining filters (e.g., 'sub;paeth') lets libpng choose best")
    print("  - Use 'none' for fastest encoding (but largest files)")
    print("=" * 50)
    print()


if __name__ == "__main__":
    main()
