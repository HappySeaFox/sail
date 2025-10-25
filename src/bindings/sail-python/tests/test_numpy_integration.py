
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
Tests for SAIL Python NumPy Integration
"""

import sailpy
import numpy as np


def test_to_numpy_basic():
    """Test basic to_numpy conversion"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    assert isinstance(arr, np.ndarray)
    assert arr.dtype == np.uint8
    assert arr.shape == (16, 16, 3)  # height, width, channels


def test_to_numpy_shapes():
    """Test to_numpy shapes for different pixel formats"""
    # RGB
    img_rgb = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 24)
    arr_rgb = img_rgb.to_numpy()
    assert arr_rgb.shape == (24, 32, 3)  # height, width, 3 channels

    # RGBA
    img_rgba = sailpy.Image(sailpy.PixelFormat.BPP32_RGBA, 32, 24)
    arr_rgba = img_rgba.to_numpy()
    assert arr_rgba.shape == (24, 32, 4)  # height, width, 4 channels


def test_to_numpy_zero_copy():
    """Test that to_numpy is zero-copy when possible"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    # Modify NumPy array
    arr[0, 0, 0] = 255

    # Check if image was modified (zero-copy)
    arr2 = img.to_numpy()
    assert arr2[0, 0, 0] == 255


def test_to_numpy_writeable():
    """Test that NumPy arrays are writeable"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    assert arr.flags['WRITEABLE']
    arr[:] = 100
    assert np.all(arr == 100)


def test_from_numpy_basic():
    """Test basic from_numpy conversion"""
    arr = np.zeros((16, 16, 3), dtype=np.uint8)
    arr[:] = [255, 128, 64]

    img = sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)

    assert img.is_valid
    assert img.width == 16
    assert img.height == 16
    assert img.pixel_format == sailpy.PixelFormat.BPP24_RGB


def test_numpy_roundtrip():
    """Test converting to NumPy and back"""
    img1 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr1 = img1.to_numpy()
    arr1[:] = [100, 150, 200]

    img2 = sailpy.Image.from_numpy(arr1, sailpy.PixelFormat.BPP24_RGB)
    arr2 = img2.to_numpy()

    assert np.array_equal(arr1, arr2)


def test_numpy_memory_layout():
    """Test NumPy array memory layout"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 24)
    arr = img.to_numpy()

    # Check C-contiguous (row-major)
    assert arr.flags['C_CONTIGUOUS']

    # Check strides
    # (bytes_per_row, bytes_per_pixel, bytes_per_channel)
    expected_strides = (32 * 3, 3, 1)
    assert arr.strides == expected_strides


def test_numpy_buffer_protocol():
    """Test buffer protocol support"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)

    # Should work with memoryview
    mem_view = memoryview(img)
    assert mem_view.nbytes > 0


def test_numpy_modify_through_array():
    """Test modifying image through NumPy array"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    # Set gradient
    for y in range(16):
        for x in range(16):
            arr[y, x] = [x * 16, y * 16, 128]

    # Verify through another array
    arr2 = img.to_numpy()
    assert arr2[0, 0, 0] == 0
    assert arr2[0, 15, 0] == 15 * 16
    assert arr2[15, 0, 1] == 15 * 16


def test_numpy_contiguous():
    """Test that arrays are contiguous"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    assert arr.data.contiguous


def test_numpy_array_interface():
    """Test __array_interface__ compatibility"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    # Should have __array_interface__
    assert hasattr(arr, '__array_interface__')

    # Can create new array from it
    arr2 = np.asarray(arr)
    assert arr2.shape == arr.shape


def test_numpy_slicing():
    """Test NumPy array slicing"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
    arr = img.to_numpy()

    # Slice operations
    top_left = arr[:16, :16]
    assert top_left.shape == (16, 16, 3)

    # Channel slicing
    red_channel = arr[:, :, 0]
    assert red_channel.shape == (32, 32)


def test_numpy_operations():
    """Test NumPy operations on image arrays"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()
    arr[:] = 100

    # Arithmetic operations
    arr_plus = arr + 50
    assert np.all(arr_plus == 150)

    # Logical operations
    mask = arr > 50
    assert np.all(mask)


def test_numpy_copy_vs_view():
    """Test copy vs view behavior"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr1 = img.to_numpy()
    arr1[:] = 100

    # Getting array again should show same data (view)
    arr2 = img.to_numpy()
    assert np.all(arr2 == 100)

    # Explicit copy should be independent
    arr3 = arr1.copy()
    arr3[:] = 200
    assert np.all(arr1 == 100)  # Original unchanged


def test_numpy_dtype_uint8():
    """Test uint8 dtype support"""
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()

    assert arr.dtype == np.uint8
    assert arr.max() <= 255
    assert arr.min() >= 0


def test_numpy_memory_safety():
    """Test memory safety with image lifecycle"""
    arr = None

    # Create image in local scope
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    arr = img.to_numpy()
    arr[:] = 123

    # Array should still be valid (image keeps memory)
    assert np.all(arr == 123)

    # Explicitly delete image
    del img

    # Array should still be accessible (pybind11 manages lifecycle)
    assert arr.shape == (16, 16, 3)


def test_numpy_uint16_rgb():
    """Test uint16 support for 16-bit RGB images"""
    img = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 32, 32)
    arr = img.to_numpy()

    assert arr.dtype == np.uint16
    assert arr.shape == (32, 32, 3)

    # Test full 16-bit range
    arr[:] = [10000, 30000, 50000]
    assert np.all(arr[:, :, 0] == 10000)
    assert np.all(arr[:, :, 1] == 30000)
    assert np.all(arr[:, :, 2] == 50000)


def test_numpy_uint16_rgba():
    """Test uint16 support for 16-bit RGBA images"""
    img = sailpy.Image(sailpy.PixelFormat.BPP64_RGBA, 32, 32)
    arr = img.to_numpy()

    assert arr.dtype == np.uint16
    assert arr.shape == (32, 32, 4)

    # Test with high values
    arr[:] = [20000, 40000, 60000, 65535]
    assert np.all(arr[:, :, 3] == 65535)  # Alpha channel


def test_numpy_uint16_grayscale():
    """Test uint16 support for 16-bit grayscale"""
    img = sailpy.Image(sailpy.PixelFormat.BPP16_GRAYSCALE, 32, 32)
    arr = img.to_numpy()

    assert arr.dtype == np.uint16
    assert arr.shape == (32, 32)  # No channel dimension for single-channel

    arr[:] = 45000
    assert np.all(arr == 45000)


def test_numpy_uint16_from_array():
    """Test creating image from uint16 NumPy array"""
    arr = np.ones((16, 16, 3), dtype=np.uint16) * 35000

    img = sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP48_RGB)
    arr2 = img.to_numpy()

    assert arr2.dtype == np.uint16
    assert np.array_equal(arr, arr2)


def test_numpy_uint16_roundtrip():
    """Test uint16 roundtrip: array -> image -> array"""
    arr_original = np.random.randint(0, 65536, (24, 24, 3), dtype=np.uint16)

    img = sailpy.Image.from_numpy(arr_original, sailpy.PixelFormat.BPP48_RGB)
    arr_result = img.to_numpy()

    assert arr_result.dtype == np.uint16
    assert np.array_equal(arr_original, arr_result)


def test_numpy_uint8_vs_uint16():
    """Test that uint8 and uint16 are correctly distinguished"""
    img8 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    img16 = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 16, 16)

    arr8 = img8.to_numpy()
    arr16 = img16.to_numpy()

    assert arr8.dtype == np.uint8
    assert arr16.dtype == np.uint16
    assert arr8.shape == (16, 16, 3)
    assert arr16.shape == (16, 16, 3)
