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
SAIL Python Bindings - Memory I/O Examples

Real-world examples showing how to work with images in memory
without touching the filesystem.
"""

import sailpy
import numpy as np


def example_image_to_bytes():
    """Convert image to bytes (useful for network transfer, API responses)"""
    print("=" * 70)
    print("EXAMPLE 1: Image to Bytes (Network/API)")
    print("=" * 70)

    # Create image
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()
    arr[:] = [70, 130, 180]  # Steel blue

    # Convert to bytes in different formats
    formats = ["png", "jpg", "qoi", "webp"]

    print("\nImage sizes in different formats:")
    for fmt in formats:
        try:
            data = img.to_bytes(fmt)
            print(f"  {fmt.upper():6}: {len(data):,} bytes")

            # Verify it can be decoded
            decoded = sailpy.Image.from_bytes(data)
            assert decoded.is_valid
        except RuntimeError as e:
            print(f"  {fmt.upper():6}: not available ({e})")

    print("\nUse case: Send images over HTTP, store in database, etc.")
    print()


def example_bytes_roundtrip():
    """Full roundtrip: Image -> bytes -> Image"""
    print("=" * 70)
    print("EXAMPLE 2: Bytes Roundtrip")
    print("=" * 70)

    # Create original
    original = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 128, 128)
    arr = original.to_numpy()
    arr[:, :, 0] = 255  # Red
    arr[:, :, 1] = 100  # Green
    arr[:, :, 2] = 50   # Blue
    arr[:, :, 3] = 200  # Alpha

    # To bytes
    png_data = original.to_bytes("png")
    print(f"\nY Encoded to PNG: {len(png_data):,} bytes")

    # From bytes
    loaded = sailpy.Image.from_bytes(png_data)
    print(f"Y Decoded from PNG: {loaded.width}x{loaded.height}")

    # Verify integrity (PNG is lossless)
    diff = np.abs(original.to_numpy().astype(int) - loaded.to_numpy().astype(int)).max()
    print(f"Y Max pixel difference: {diff} (lossless)")

    print("\nUse case: Microservices, image processing pipelines")
    print()


def example_load_from_api_response():
    """Simulate loading image from API response"""
    print("=" * 70)
    print("EXAMPLE 3: Loading from API Response (Simulated)")
    print("=" * 70)

    # Simulate getting image bytes from API
    print("\n1. Creating 'API response' (simulated)...")
    source_img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    source_img.to_numpy()[:] = [50, 100, 150]

    # This would be bytes from API response
    api_response_bytes = source_img.to_bytes("png")
    print(f"   Y Got {len(api_response_bytes):,} bytes from 'API'")

    # Process the image
    print("\n2. Processing image from bytes...")
    img = sailpy.Image.from_bytes(api_response_bytes)

    # Do some processing
    arr = img.to_numpy()
    arr[:] = arr // 2  # Darken

    # Convert back to bytes for response
    response_bytes = img.to_bytes("webp")
    print(f"   Y Processed and encoded to WebP: {len(response_bytes):,} bytes")
    print(f"   Size reduction: {(1 - len(response_bytes)/len(api_response_bytes))*100:.1f}%")

    print("\nUse case: Image proxy services, CDN transformations")
    print()


def example_format_conversion_in_memory():
    """Convert between formats without saving to disk"""
    print("=" * 70)
    print("EXAMPLE 4: Format Conversion in Memory")
    print("=" * 70)

    # Create JPEG image (lossy)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 100, 100)
    img.to_numpy()[:] = [255, 200, 150]

    jpeg_data = img.to_bytes("jpg")
    print(f"\nOriginal JPEG: {len(jpeg_data):,} bytes")

    # Load from JPEG bytes
    from_jpeg = sailpy.Image.from_bytes(jpeg_data)

    # Convert to PNG (lossless) without touching disk
    png_data = from_jpeg.to_bytes("png")
    print(f"Converted to PNG: {len(png_data):,} bytes")

    # Convert to WebP
    webp_data = from_jpeg.to_bytes("webp")
    print(f"Converted to WebP: {len(webp_data):,} bytes")

    print("\nUse case: Format conversion API, image optimizer")
    print()


def example_thumbnail_generation():
    """Generate thumbnail and return as bytes"""
    print("=" * 70)
    print("EXAMPLE 5: Thumbnail Generation")
    print("=" * 70)

    # Create "large" image
    large = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 512, 512)
    large.to_numpy()[:] = [180, 140, 200]

    print(f"\nOriginal: {large.width}x{large.height}")

    # Create thumbnail (in real scenario, use proper resize)
    # For demo, just create smaller image
    thumbnail = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 64, 64)
    thumbnail.to_numpy()[:] = [180, 140, 200]

    # Get codec with optimal settings for thumbnails
    codec = sailpy.CodecInfo.from_name("WEBP")
    options = codec.save_features.to_options()

    # High compression for thumbnails
    cl = codec.save_features.compression_level
    if cl.is_valid:
        options.compression_level = cl.max_level

    # Convert to bytes (for storage or transmission)
    thumb_bytes = thumbnail.to_bytes("webp")

    print(f"Thumbnail: {thumbnail.width}x{thumbnail.height}")
    print(f"Size: {len(thumb_bytes):,} bytes")
    print(f"Compression: {options.compression.name} (level {options.compression_level})")

    print("\nUse case: Thumbnail generation service, preview images")
    print()


def example_batch_processing():
    """Process multiple images in memory"""
    print("=" * 70)
    print("EXAMPLE 6: Batch Processing in Memory")
    print("=" * 70)

    # Simulate batch of images
    images_data = []
    for i in range(3):
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img.to_numpy()[:] = [i * 50, i * 70, i * 90]
        images_data.append(img.to_bytes("png"))

    print(f"\nY Created batch of {len(images_data)} images")
    total_size = sum(len(d) for d in images_data)
    print(f"  Total size: {total_size:,} bytes")

    # Process each
    processed = []
    for i, data in enumerate(images_data):
        img = sailpy.Image.from_bytes(data)
        # Process (invert colors)
        arr = img.to_numpy()
        arr[:] = 255 - arr

        processed_data = img.to_bytes("png")
        processed.append(processed_data)

    print(f"Y Processed {len(processed)} images")
    print(f"  Output size: {sum(len(d) for d in processed):,} bytes")

    print("\nUse case: Batch image processor, photo filters")
    print()


def main():
    """Run all memory I/O examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL Memory I/O Examples")
    print("=" * 70 + "\n")

    try:
        example_image_to_bytes()
        example_bytes_roundtrip()
        example_load_from_api_response()
        example_format_conversion_in_memory()
        example_thumbnail_generation()
        example_batch_processing()

        print("=" * 70)
        print("Y All examples completed!")
        print()

    finally:
        sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)


if __name__ == "__main__":
    main()

