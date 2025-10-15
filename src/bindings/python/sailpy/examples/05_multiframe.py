#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - Multi-frame Images

Working with animations and multi-page images (GIF, WebP, TIFF).
"""

import os
import sailpy
import tempfile


def main():
    """Multi-frame examples"""
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print("\nSAIL Multi-frame Images")
    print("=" * 70 + "\n")

    # Reading frames with iterato
    print("EXAMPLE 1: Reading Frames with Iterator")
    print("=" * 70 + "\n")

    test_path = os.path.join(tempfile.gettempdir(), "test_multiframe.png")
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    img.to_numpy()[:] = [255, 128, 64]
    img.save(test_path)

    try:
        # Method: read_all()
        print("  Using read_all():")
        reader = sailpy.ImageReader(test_path)
        frames = reader.read_all()
        print(f"    Read {len(frames)} frame(s)")
        for i, frame in enumerate(frames):
            print(f"    Frame {i}: {frame.width}x{frame.height}")

    finally:
        os.remove(test_path)

    print()

    # Writing multiple frames
    print("EXAMPLE 2: Writing Multiple Frames")
    print("=" * 70 + "\n")

    # Use GIF for animation support
    output_path = os.path.join(tempfile.gettempdir(), "multi_output.gif")

    try:
        # Get GIF codec and check animation support
        codec = sailpy.CodecInfo.from_name("GIF")
        supports_animated = (codec.save_features.features & sailpy.CodecFeature.ANIMATED) != 0

        print(f"Using codec: {codec.name}")
        print(f"  Animated support: {'✓' if supports_animated else '✗'}")
        print(f"  Supported formats: {', '.join([pf.name for pf in codec.save_features.pixel_formats][:4])}")
        print()

        # Create frames with different colors
        img1 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img1.to_numpy()[:] = [255, 0, 0]  # Red
        img1.delay = 100  # 100ms

        img2 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img2.to_numpy()[:] = [0, 255, 0]  # Green
        img2.delay = 100

        img3 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img3.to_numpy()[:] = [0, 0, 255]  # Blue
        img3.delay = 100

        # GIF only supports indexed formats - convert to best format
        print("Converting frames to GIF format...")
        img1.convert(codec.save_features)
        img2.convert(codec.save_features)
        img3.convert(codec.save_features)
        print(f"  Converted to: {img1.pixel_format.name}")
        print()

        # Write animation
        with sailpy.ImageWriter(output_path) as writer:
            writer.write(img1)
            writer.write(img2)
            writer.write(img3)

        print(f"✓ Written 3 frames to animated {codec.name}")
        print(f"  File: {output_path}")
        print(f"  Total delay: 300ms")

    finally:
        if os.path.exists(output_path):
            os.remove(output_path)

    print()
    print("=" * 70)
    print("✓ All examples completed!")
    print("=" * 70)
    print()


if __name__ == "__main__":
    main()
