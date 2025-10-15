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
Threading and concurrency tests
Based on tests/sail/threading.c
"""

import sailpy
import threading
import tempfile
import os


def test_threading_concurrent_codec_queries():
    """Test concurrent codec info queries from multiple threads"""
    results = {}

    def query_codec(name, result_dict):
        codec = sailpy.CodecInfo.from_extension(name)
        result_dict[name] = codec.is_valid

    threads = []
    for ext in ["png", "jpg", "gif", "bmp", "webp"] * 3:  # 15 queries
        t = threading.Thread(target=query_codec, args=(ext, results))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    # Should have found codecs
    assert len(results) > 0


def test_threading_concurrent_image_creation():
    """Test creating images from multiple threads"""
    images = []
    lock = threading.Lock()

    def create_image(idx):
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)
        img.to_numpy()[:] = idx
        with lock:
            images.append(img)

    threads = []
    for i in range(5):
        t = threading.Thread(target=create_image, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    # Should have created all images
    assert len(images) == 5
    assert all(img.is_valid for img in images)


def test_threading_list_codecs():
    """Test list_codecs from multiple threads"""
    results = []
    lock = threading.Lock()

    def list_all():
        codecs = sailpy.list_codecs()
        with lock:
            results.append(len(codecs))

    threads = []
    for _ in range(5):
        t = threading.Thread(target=list_all)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    # All queries should return same count
    assert len(results) == 5
    assert all(r == results[0] for r in results)


def test_threading_save_different_files():
    """Test saving to different files from multiple threads"""

    def save_image(idx):
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
        img.to_numpy()[:] = idx * 20

        output_path = os.path.join(
            tempfile.gettempdir(), f"test_thread_{idx}.png")
        img.save(output_path)

        # Cleanup
        if os.path.exists(output_path):
            os.remove(output_path)

    threads = []
    for i in range(5):
        t = threading.Thread(target=save_image, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()


def test_threading_load_same_file(test_png):
    """Test loading same file from multiple threads"""
    images = []
    lock = threading.Lock()

    def load_image():
        img = sailpy.Image.from_file(str(test_png))
        with lock:
            images.append(img)

    threads = []
    for _ in range(5):
        t = threading.Thread(target=load_image)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    # Should have loaded all
    assert len(images) == 5
    assert all(img.is_valid for img in images)
    assert all(img.width == images[0].width for img in images)
    assert all(img.height == images[0].height for img in images)


def test_threading_numpy_conversions():
    """Test NumPy conversions from multiple threads"""

    def convert_image():
        img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
        arr = img.to_numpy()
        arr[:] = [50, 100, 150]

        # Convert back
        img2 = sailpy.Image.from_numpy(arr, sailpy.PixelFormat.BPP24_RGB)
        assert img2.is_valid

    threads = []
    for _ in range(5):
        t = threading.Thread(target=convert_image)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

