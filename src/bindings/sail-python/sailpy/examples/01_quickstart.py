
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
SAIL Python Bindings - Simple Load/Save Example

The simplest way to load and save images with SAIL.
"""

import os
import tempfile
import sailpy


def main():
    print("SAILPY - Simple Load/Save Example")
    print("=" * 50)

    # Silence logs for cleaner output
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # Create an image
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
    arr = img.to_numpy()
    arr[:] = [100, 150, 200]  # Fill with light blue

    # Save
    output_path = os.path.join(tempfile.gettempdir(), "simple_output.png")
    img.save(output_path)
    print(f"Y Saved: {output_path}")

    # Load
    loaded = sailpy.Image.from_file(output_path)
    print(f"Y Loaded: {loaded.width}x{loaded.height} pixels")
    print(f"  Pixel format: {loaded.pixel_format.name}")
    print(f"  Bits per pixel: {loaded.bits_per_pixel}")

    # Quick transformation example
    rotated = loaded.rotate_to(sailpy.Orientation.ROTATED_90)
    print(f"Y Rotated 90°: {rotated.width}x{rotated.height} pixels")

    # Cleanup
    os.remove(output_path)

    print("\nY Complete!")


if __name__ == "__main__":
    main()
