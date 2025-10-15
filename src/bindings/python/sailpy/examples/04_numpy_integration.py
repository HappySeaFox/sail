#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - NumPy Integration Examples

Zero-copy NumPy integration for efficient image processing.
Combines numpy_example.py and numpy_processing.py
"""

import sailpy
import numpy as np


def main():
    """NumPy integration examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL NumPy Integration")
    print("=" * 70 + "\n")

    # Example 1: Zero-copy access
    print("=" * 70)
    print("EXAMPLE 1: Zero-Copy NumPy Access")
    print("=" * 70)

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()  # Zero-copy!

    print(f"\n✓ Zero-copy array:")
    print(f"  Shape: {arr.shape}")
    print(f"  Dtype: {arr.dtype}")
    print(f"  Flags: C_CONTIGUOUS={arr.flags['C_CONTIGUOUS']}")

    # Modify in-place (affects original image)
    arr[:] = [100, 150, 200]
    print(f"✓ Modified in-place (no copy!)\n")

    # Example 2: uint16 support
    print("=" * 70)
    print("EXAMPLE 2: 16-bit Images (uint16)")
    print("=" * 70)

    img16 = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 128, 128)
    arr16 = img16.to_numpy()

    print(f"\n✓ 16-bit array:")
    print(f"  Shape: {arr16.shape}")
    print(f"  Dtype: {arr16.dtype}")  # uint16!

    arr16[:] = [10000, 30000, 50000]  # Full 16-bit range
    print(f"✓ Set 16-bit values\n")

    # Example 3: Image processing
    print("=" * 70)
    print("EXAMPLE 3: Image Processing with NumPy")
    print("=" * 70 + "\n")

    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()

    # Create gradient
    for y in range(256):
        arr[y, :, 0] = y  # Red
        arr[y, :, 1] = 255 - y  # Green
        arr[y, :, 2] = 128  # Blue

    print("✓ Created gradient with NumPy")

    # Brighten
    arr[:] = np.clip(arr * 1.5, 0, 255).astype(np.uint8)
    print("✓ Brightened image")

    # Invert
    arr[:] = 255 - arr
    print("✓ Inverted colors\n")

    # Example 4: From NumPy array
    print("=" * 70)
    print("EXAMPLE 4: Create Image from NumPy Array")
    print("=" * 70 + "\n")

    # Create NumPy array
    arr = np.zeros((100, 100, 3), dtype=np.uint8)
    arr[:, :50, 0] = 255  # Left half red
    arr[:, 50:, 2] = 255  # Right half blue

    # Create SAIL image from array
    img = sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)

    print(f"✓ Created from NumPy: {img.width}x{img.height}")
    print(f"  Valid: {img.is_valid}\n")

    # Example 5: Slicing and operations
    print("=" * 70)
    print("EXAMPLE 5: NumPy Slicing and Operations")
    print("=" * 70 + "\n")

    img = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 200, 200)
    arr = img.to_numpy()

    # Slice operations
    arr[:100, :, :3] = [255, 0, 0]  # Top half red
    arr[100:, :, :3] = [0, 0, 255]  # Bottom half blue
    arr[:, :, 3] = 255  # Full opacity

    print("✓ Sliced and modified regions")

    # Channel operations
    red_channel = arr[:, :, 0]
    avg_red = red_channel.mean()
    print(f"✓ Average red value: {avg_red:.1f}\n")

    print("=" * 70)
    print("✓ All examples completed!")
    print()


if __name__ == "__main__":
    main()

