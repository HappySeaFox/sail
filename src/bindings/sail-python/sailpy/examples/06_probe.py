#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - Efficient Metadata Probing

Probe image metadata WITHOUT loading pixels - fast and memory-efficient!
"""

import os
import sys
import sailpy
import tempfile
from pathlib import Path


def main():
    """Probe examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL Efficient Metadata Probing")
    print("=" * 50 + "\n")

    # Probe metadata without loading pixels
    print("EXAMPLE: Probe Metadata (No Pixel Loading)")
    print("=" * 50 + "\n")

    # Use provided image or create test image
    if len(sys.argv) > 1 and Path(sys.argv[1]).exists():
        probe_path = sys.argv[1]
        cleanup = False
    else:
        probe_path = os.path.join(tempfile.gettempdir(), "large_test.png")
        img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 1920, 1080)
        img.save(probe_path)
        cleanup = True

    try:
        import time

        # Probe metadata WITHOUT loading pixels (instant!)
        start = time.time()
        metadata = sailpy.ImageInput.probe(probe_path)
        probe_time = time.time() - start

        print(f"Image: {Path(probe_path).name}")
        print(f"  Dimensions: {metadata['width']}x{metadata['height']}")
        print(f"  Pixel format: {metadata['pixel_format'].name}")
        print(f"  Bits per pixel: {metadata['bits_per_pixel']}")
        print(f"  Codec: {metadata['codec_name']}")

        # Source image info (if available)
        if 'source_pixel_format' in metadata:
            print(f"  Source format: {metadata['source_pixel_format'].name}")
        if 'source_compression' in metadata:
            print(f"  Source compression: {metadata['source_compression'].name}")

        pixels_size_mb = metadata['width'] * metadata['height'] * \
            (metadata['bits_per_pixel'] // 8) / 1024 / 1024
        print(f"\n+ Probed without loading {pixels_size_mb:.1f}MB of pixels!")

        # Compare with full load
        start = time.time()
        full_img = sailpy.Image.from_file(probe_path)
        load_time = time.time() - start

        print(f"\nPerformance comparison:")
        print(f"  Probe time: {probe_time*1000:.2f} ms")
        print(f"  Full load:  {load_time*1000:.2f} ms")
        print(f"  Speedup:    {load_time/probe_time:.1f}x faster")

    finally:
        if cleanup:
            os.remove(probe_path)

    print()
    print("=" * 50)
    print("+ Use probe when you only need metadata")
    print("=" * 50)
    print()


if __name__ == "__main__":
    main()
