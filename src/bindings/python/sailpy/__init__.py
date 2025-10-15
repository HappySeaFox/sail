
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
SAIL - Squirrel Abstract Imaging Library
Python bindings

Fast and easy-to-use image decoding library for Python.
"""

from ._version import __version__

# Pre-import NumPy internals to avoid GIL deadlock during lazy initialization
# when using Image.to_numpy() or Image.from_numpy() in multi-threaded code.
# This ensures numpy.core modules are loaded before any threads are created.
import numpy.core.multiarray  # noqa: F401
import numpy.core.umath  # noqa: F401

from ._libsail import (
    # Enums
    PixelFormat,
    Compression,
    Orientation,
    ChromaSubsampling,
    ResolutionUnit,
    MetaDataType,
    CodecFeature,
    Status,
    Option,
    LogLevel,

    # Core Classes
    Image,
    Resolution,
    ConversionOptions,
    Variant,
    Palette,
    Iccp,
    MetaData,
    SourceImage,

    # Loading and Saving Classes
    ImageReader,
    ImageWriter,

    # Options and Configuration Classes
    LoadOptions,
    SaveOptions,

    # Codec Information
    CodecInfo,
    LoadFeatures,
    SaveFeatures,
    CompressionLevel,
    list_codecs,
    get_codec_info,

    # Logging
    set_log_barrier,
    set_logger,
)

__author__ = "Dzmitry Baryshau"
__license__ = "MIT"

__all__ = [
    # Version info
    "__version__",
    "__author__",
    "__license__",

    # Enums
    "PixelFormat",
    "Compression",
    "Orientation",
    "ChromaSubsampling",
    "ResolutionUnit",
    "MetaDataType",
    "CodecFeature",
    "Status",
    "Option",
    "LogLevel",

    # Core Classes
    "Image",
    "Resolution",
    "ConversionOptions",
    "Variant",
    "Palette",
    "Iccp",
    "MetaData",
    "SourceImage",

    # Loading and Saving Classes
    "ImageReader",
    "ImageWriter",

    # Options and Configuration Classes
    "LoadOptions",
    "SaveOptions",

    # Codec Information
    "CodecInfo",
    "LoadFeatures",
    "SaveFeatures",
    "CompressionLevel",
    "list_codecs",
    "get_codec_info",

    # Logging
    "set_log_barrier",
    "set_logger",
]
