#!/usr/bin/env python3

"""
SAIL Python Bindings - Image Transformations

Demonstrates rotation, mirroring, and pixel format conversion.
"""

import sailpy
import sys
import tempfile
from pathlib import Path


def main():
    print("=" * 70)
    print(" SAIL Python - Image Transformations")
    print("=" * 70)
    print()

    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # Create temporary directory for output files
    tmp_dir = Path(tempfile.gettempdir())

    # Load or create image
    if len(sys.argv) < 2:
        print("[1/4] Creating image in memory (96x64)")
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 96, 64)
        arr = img.to_numpy()

        # Create gradient pattern
        for y in range(64):
            for x in range(96):
                arr[y, x] = [x * 2, y * 4, (x + y) % 256]

        print(f"      {img.width}x{img.height}, {img.pixel_format.name}")
        base_name = "generated_image"
    else:
        input_path = Path(sys.argv[1])
        if not input_path.exists():
            print(f"Error: File '{input_path}' not found")
            sys.exit(1)

        print(f"[1/4] Loading: {input_path.name}")
        img = sailpy.Image.from_file(str(input_path))
        print(f"      {img.width}x{img.height}, {img.pixel_format.name}")
        base_name = input_path.stem

    print()

    # Ensure byte-aligned format
    if img.bits_per_pixel % 8 != 0 or img.is_indexed:
        img = img.convert_to(sailpy.PixelFormat.BPP24_RGB)

    # Rotate 90° (returns new image)
    print("[2/4] Rotating 90 degrees clockwise")
    rotated = img.rotate_to(sailpy.Orientation.ROTATED_90)
    output = tmp_dir / f"{base_name}_rotated_90.png"
    rotated.save(str(output))
    print(f"      {img.width}x{img.height} -> {rotated.width}x{rotated.height}")
    print(f"      Saved: {output}")
    print()

    # Mirror (in-place)
    print("[3/4] Mirroring horizontally")
    img.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)
    output = tmp_dir / f"{base_name}_mirrored.png"
    img.save(str(output))
    print(f"      Saved: {output}")
    print()

    # Convert to grayscale
    print("[4/4] Converting to grayscale")
    if img.can_convert(sailpy.PixelFormat.BPP8_GRAYSCALE):
        gray = img.convert_to(sailpy.PixelFormat.BPP8_GRAYSCALE)
        output = tmp_dir / f"{base_name}_grayscale.png"
        gray.save(str(output))
        print(f"      {img.pixel_format.name} -> {gray.pixel_format.name}")
        print(f"      Saved: {output}")
    print()

    print("=" * 70)
    print("Y All transformations completed!")
    print("=" * 70)


if __name__ == "__main__":
    main()
