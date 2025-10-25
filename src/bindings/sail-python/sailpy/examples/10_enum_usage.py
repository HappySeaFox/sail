#!/usr/bin/env python3

"""
SAIL Python Bindings - Example 10: Enum Usage (Python Way)

This example demonstrates:
- Standard Python enum properties (.name, .value)
- String representation (str, repr)
- Parsing from SAIL format strings (.from_string)
- All enum types: PixelFormat, Compression, Orientation, ChromaSubsampling
"""

import sailpy


def main():
    print("=" * 70)
    print(" SAIL Python - Enum Usage (Python Way)")
    print("=" * 70)
    print()

    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # PixelFormat enum
    print("[1/4] PixelFormat enum")
    print()

    pf = sailpy.PixelFormat.BPP24_RGB

    print("  Standard Python properties:")
    print(f"    .name:  {pf.name}")
    print(f"    .value: {pf.value}")
    print(f"    str():  {str(pf)}")
    print(f"    repr(): {repr(pf)}")
    print()

    print("  Parse from SAIL string format:")
    parsed = sailpy.PixelFormat.from_string("BPP24-RGB")
    print(f"    PixelFormat.from_string('BPP24-RGB') = {parsed}")
    print(f"    Matches original: {parsed == pf}")
    print()

    print("  More examples:")
    for sail_str in ["BPP32-RGBA", "BPP8-GRAYSCALE", "BPP4-INDEXED"]:
        fmt = sailpy.PixelFormat.from_string(sail_str)
        print(f"    '{sail_str}' → {fmt.name}")
    print()

    # Compression enum
    print("[2/4] Compression enum")
    print()

    comp = sailpy.Compression.JPEG
    print(f"  .name: {comp.name}")
    print(f"  .value: {comp.value}")
    print()

    print("  Parse from SAIL strings:")
    for sail_str in ["JPEG", "DEFLATE", "RLE", "LZW"]:
        c = sailpy.Compression.from_string(sail_str)
        print(f"    '{sail_str}' → {c.name}")
    print()

    # Orientation enum
    print("[3/4] Orientation enum")
    print()

    orient = sailpy.Orientation.ROTATED_90
    print(f"  .name: {orient.name}")
    print(f"  str(): {str(orient)}")
    print()

    print("  Parse from SAIL strings (with dashes):")
    for sail_str in ["NORMAL", "ROTATED-90", "ROTATED-180", "MIRRORED-HORIZONTALLY"]:
        o = sailpy.Orientation.from_string(sail_str)
        print(f"    '{sail_str:25s}' → {o.name}")
    print()

    # ChromaSubsampling enum
    print("[4/4] ChromaSubsampling enum")
    print()

    cs = sailpy.ChromaSubsampling.C420
    print(f"  .name: {cs.name}")
    print()

    print("  Parse from SAIL strings (numeric format):")
    for sail_str in ["420", "422", "444"]:
        chroma = sailpy.ChromaSubsampling.from_string(sail_str)
        print(f"    '{sail_str}' → {chroma.name}")
    print()

    # Practical usage
    print("=" * 70)
    print("PRACTICAL USAGE")
    print("=" * 70)
    print()

    print("1. Create image from parsed format:")
    fmt = sailpy.PixelFormat.from_string("BPP32-RGBA")
    img = sailpy.Image(fmt, 100, 100)
    print(f"   Created {img.width}x{img.height} image with {img.pixel_format.name}")
    print()

    print("2. Use enum .name for logging:")
    print(f"   Image format: {img.pixel_format.name}")
    print(f"   Is RGB family: {img.is_rgb_family}")
    print()

    print("3. Compare enums:")
    print(f"   BPP24_RGB == BPP32_RGBA: {sailpy.PixelFormat.BPP24_RGB == sailpy.PixelFormat.BPP32_RGBA}")
    print(f"   JPEG == JPEG: {sailpy.Compression.JPEG == sailpy.Compression.JPEG}")
    print()

    print("=" * 70)
    print("✓ Summary:")
    print("=" * 70)
    print()
    print("  Use .name for:")
    print("    • Logging and debug output")
    print("    • Python code comparisons")
    print("    • Display in UI")
    print()
    print("  Use .from_string() for:")
    print("    • Parsing from config files")
    print("    • Parsing from API responses")
    print("    • Converting SAIL format strings")
    print()
    print("=" * 70)


if __name__ == "__main__":
    main()

