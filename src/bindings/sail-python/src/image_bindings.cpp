/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sail-c++/sail-c++.h>
#include <sail-manip/manip_common.h>

#include <fstream>
#include <limits>

namespace py = pybind11;

// Helper to check if pixel format uses 16-bit per channel
static bool is_16bit_per_channel(SailPixelFormat pixel_format)
{
    unsigned bpp      = sail::image::bits_per_pixel(pixel_format);
    unsigned channels = sail::image::pixel_format_channels(pixel_format);

    if (channels == 0)
    {
        return false;
    }

    // If bits per pixel / channels = 16, then it's 16-bit per channel
    unsigned bits_per_channel = bpp / channels;
    return bits_per_channel == 16;
}

// Convert sail::image to NumPy array with appropriate dtype (uint8 or uint16)
py::object image_to_numpy(sail::image& img)
{
    auto pixel_format     = img.pixel_format();
    unsigned channels     = sail::image::pixel_format_channels(pixel_format);
    bool use_uint16       = is_16bit_per_channel(pixel_format);
    int bytes_per_element = use_uint16 ? 2 : 1;

    std::vector<py::ssize_t> shape;
    std::vector<py::ssize_t> strides;

    // Determine dimensions based on pixel format
    if (img.is_rgb_family())
    {
        // RGB/RGBA format: (height, width, channels)
        shape   = {static_cast<py::ssize_t>(img.height()), static_cast<py::ssize_t>(img.width()),
                   static_cast<py::ssize_t>(channels)};
        strides = {static_cast<py::ssize_t>(img.bytes_per_line()),
                   static_cast<py::ssize_t>(channels * bytes_per_element), static_cast<py::ssize_t>(bytes_per_element)};
    }
    else if (img.is_grayscale())
    {
        // Grayscale: (height, width) for single channel, (height, width, channels) for GA
        if (channels == 1)
        {
            shape   = {static_cast<py::ssize_t>(img.height()), static_cast<py::ssize_t>(img.width())};
            strides = {static_cast<py::ssize_t>(img.bytes_per_line()), static_cast<py::ssize_t>(bytes_per_element)};
        }
        else
        {
            shape   = {static_cast<py::ssize_t>(img.height()), static_cast<py::ssize_t>(img.width()),
                       static_cast<py::ssize_t>(channels)};
            strides = {static_cast<py::ssize_t>(img.bytes_per_line()),
                       static_cast<py::ssize_t>(channels * bytes_per_element),
                       static_cast<py::ssize_t>(bytes_per_element)};
        }
    }
    else
    {
        // For other formats return flat array
        py::ssize_t num_elements = img.pixels_size() / bytes_per_element;
        shape                    = {num_elements};
        strides                  = {static_cast<py::ssize_t>(bytes_per_element)};
    }

    // Return appropriate dtype array
    if (use_uint16)
    {
        return py::array_t<uint16_t>(shape, strides, static_cast<uint16_t*>(img.pixels()),
                                     py::cast(img) // base object - owns the memory
        );
    }
    else
    {
        return py::array_t<uint8_t>(shape, strides, static_cast<uint8_t*>(img.pixels()),
                                    py::cast(img) // base object - owns the memory
        );
    }
}

// Create sail::image from NumPy array (supports uint8 and uint16)
sail::image numpy_to_image(py::array arr, SailPixelFormat pixel_format)
{
    auto buf = arr.request();

    if (buf.ndim != 2 && buf.ndim != 3)
    {
        throw std::runtime_error("Array must be 2D or 3D");
    }

    if (buf.shape[0] > std::numeric_limits<unsigned>::max() || buf.shape[1] > std::numeric_limits<unsigned>::max())
    {
        PyErr_SetString(PyExc_ValueError,
                        "Failed to create image from NumPy array: the array is too large");
        throw py::error_already_set();
    }

    unsigned height = static_cast<unsigned>(buf.shape[0]);
    unsigned width  = static_cast<unsigned>(buf.shape[1]);

    // Create image
    sail::image img(pixel_format, width, height);

    if (!img.is_valid())
    {
        PyErr_SetString(PyExc_ValueError,
                        "Failed to create image from NumPy array: invalid dimensions or pixel format");
        throw py::error_already_set();
    }

    // Determine element size
    size_t element_size = 0;
    if (buf.format == py::format_descriptor<uint8_t>::format())
    {
        element_size = sizeof(uint8_t);
    }
    else if (buf.format == py::format_descriptor<uint16_t>::format())
    {
        element_size = sizeof(uint16_t);
    }
    else
    {
        throw std::runtime_error("Unsupported array dtype. Expected uint8 or uint16.");
    }

    // Copy data
    std::memcpy(img.pixels(), buf.ptr, buf.size * element_size);

    return img;
}

void init_image(py::module_& m)
{
    // ============================================================================
    // Resolution
    // ============================================================================

    py::class_<sail::resolution>(m, "Resolution", "Image resolution (DPI)")
        .def(py::init<>(), "Create resolution with default values")
        .def(py::init<SailResolutionUnit, double, double>(), py::arg("unit"), py::arg("x"), py::arg("y"),
             "Create resolution with unit, x and y values")
        .def_property("unit", &sail::resolution::unit,
                      [](sail::resolution& r, SailResolutionUnit unit) { r.set_unit(unit); },
                      "Resolution unit (unknown, micrometer, centimeter, inch)")
        .def_property("x", &sail::resolution::x, [](sail::resolution& r, double x) { r.set_x(x); },
                      "Horizontal resolution value")
        .def_property("y", &sail::resolution::y, [](sail::resolution& r, double y) { r.set_y(y); },
                      "Vertical resolution value")
        .def("__repr__", [](const sail::resolution& r) {
            return "Resolution(unit=" + std::to_string(static_cast<int>(r.unit())) + ", x=" + std::to_string(r.x())
                   + ", y=" + std::to_string(r.y()) + ")";
        });

    // ============================================================================
    // ConversionOptions
    // ============================================================================

    py::class_<sail::conversion_options>(m, "ConversionOptions", "Options for pixel format conversion")
        .def(py::init<>(), "Create default conversion options")
        .def_property(
            "blend_alpha",
            [](const sail::conversion_options& opts) { return opts.options() & SAIL_CONVERSION_OPTION_BLEND_ALPHA; },
            [](sail::conversion_options& opts, bool blend) {
                opts.set_option(SAIL_CONVERSION_OPTION_BLEND_ALPHA, blend);
            },
            "Blend alpha channel with background when converting")
        .def_property(
            "preserve_iccp",
            [](const sail::conversion_options& opts) { return opts.options() & SAIL_CONVERSION_OPTION_PRESERVE_ICCP; },
            [](sail::conversion_options& opts, bool preserve) {
                opts.set_option(SAIL_CONVERSION_OPTION_PRESERVE_ICCP, preserve);
            },
            "Preserve ICC profile when converting between pixel formats")
        .def("__repr__", [](const sail::conversion_options&) { return "ConversionOptions()"; });

    // ============================================================================
    // Image
    // ============================================================================

    py::class_<sail::image>(m, "Image", py::buffer_protocol(), "Image representation with direct access to pixel data")
        .def(py::init<>(), "Create an invalid image")
        .def(py::init([](const std::string& path) {
                 // Check if file exists first
                {
                    std::ifstream file(path);
                    if (!file.good())
                    {
                        PyErr_SetString(PyExc_FileNotFoundError, ("File not found: " + path).c_str());
                        throw py::error_already_set();
                    }
                }

                 sail::image img(path);
                 if (!img.is_valid())
                 {
                     throw std::runtime_error("Failed to load image from: " + path);
                 }
                 return img;
             }),
             py::arg("path"), "Load image from file")
        .def(py::init([](SailPixelFormat pixel_format, unsigned width, unsigned height) {
                 sail::image img(pixel_format, width, height);
                 if (!img.is_valid())
                 {
                     PyErr_SetString(PyExc_ValueError, "Invalid image dimensions or pixel format");
                     throw py::error_already_set();
                 }
                 return img;
             }),
             py::arg("pixel_format"), py::arg("width"), py::arg("height"),
             "Create empty image with specified format and dimensions")
        .def(py::init([](SailPixelFormat pixel_format, unsigned width, unsigned height, unsigned bytes_per_line) {
                 sail::image img(pixel_format, width, height, bytes_per_line);
                 if (!img.is_valid())
                 {
                     PyErr_SetString(PyExc_ValueError, "Invalid image dimensions, pixel format, or bytes per line");
                     throw py::error_already_set();
                 }
                 return img;
             }),
             py::arg("pixel_format"), py::arg("width"), py::arg("height"), py::arg("bytes_per_line"),
             "Create empty image with specified format, dimensions and bytes per line")

        // Properties (read-only)
        .def_property_readonly("width", &sail::image::width, "Image width in pixels")
        .def_property_readonly("height", &sail::image::height, "Image height in pixels")
        .def_property_readonly("pixel_format", &sail::image::pixel_format, "Pixel format")
        .def_property_readonly("bytes_per_line",
                               static_cast<unsigned (sail::image::*)() const>(&sail::image::bytes_per_line),
                               "Number of bytes per scan line")
        .def_property_readonly("bits_per_pixel",
                               static_cast<unsigned (sail::image::*)() const>(&sail::image::bits_per_pixel),
                               "Number of bits per pixel")
        .def_property_readonly("pixels_size", &sail::image::pixels_size, "Total size of pixel data in bytes")
        .def_property_readonly("is_valid", &sail::image::is_valid, "Check if image has valid dimensions and pixel data")
        .def_property_readonly("is_indexed", static_cast<bool (sail::image::*)() const>(&sail::image::is_indexed),
                               "Check if pixel format is indexed with palette")
        .def_property_readonly("is_grayscale", static_cast<bool (sail::image::*)() const>(&sail::image::is_grayscale),
                               "Check if pixel format is grayscale")
        .def_property_readonly("is_rgb_family", static_cast<bool (sail::image::*)() const>(&sail::image::is_rgb_family),
                               "Check if pixel format is RGB-like")

        // Properties (read-write)
        .def_property("gamma", &sail::image::gamma, &sail::image::set_gamma, "Image gamma value")
        .def_property("delay", &sail::image::delay, &sail::image::set_delay,
                      "Delay in milliseconds for animation frames")
        .def_property(
            "resolution",
            static_cast<const sail::resolution& (sail::image::*)() const>(&sail::image::resolution),
            py::overload_cast<const sail::resolution&>(&sail::image::set_resolution),
            "Image resolution (DPI)")
        .def_property(
            "palette",
            static_cast<const sail::palette& (sail::image::*)() const>(&sail::image::palette),
            py::overload_cast<const sail::palette&>(&sail::image::set_palette),
            "Image palette for indexed formats")
        .def_property(
            "iccp",
            static_cast<const sail::iccp& (sail::image::*)() const>(&sail::image::iccp),
            py::overload_cast<const sail::iccp&>(&sail::image::set_iccp),
            "Embedded ICC profile")
        .def_property(
            "meta_data",
            static_cast<const std::vector<sail::meta_data>& (sail::image::*)() const>(&sail::image::meta_data),
            py::overload_cast<const std::vector<sail::meta_data>&>(&sail::image::set_meta_data),
            "Image metadata (EXIF, comments, etc.)")

        // Source image (readonly)
        .def_property_readonly(
            "source_image", static_cast<const sail::source_image& (sail::image::*)() const>(&sail::image::source_image),
            "Get source image properties")

        // Methods

        .def("load",
             [](sail::image& img, const std::string& path) {
                 // Check if file exists first
                {
                    std::ifstream file(path);
                    if (!file.good())
                    {
                        PyErr_SetString(PyExc_FileNotFoundError, ("File not found: " + path).c_str());
                        throw py::error_already_set();
                    }
                }

                 auto status = img.load(path);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to load image from: " + path);
                 }
             },
             py::arg("path"), "Load image from file")
        .def(
            "save",
            [](sail::image& img, const std::string& path) {
                auto status = img.save(path);
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to save image");
                }
            },
            py::arg("path"), "Save image to file")

        .def(
            "to_bytes",
            [](sail::image& img, const std::string& format) -> py::bytes {
                // Get codec info
                sail::codec_info codec = sail::codec_info::from_extension(format);
                if (!codec.is_valid())
                {
                    throw std::runtime_error("Unknown format: " + format);
                }

                // Create expanding buffer I/O (starts with 64KB)
                sail::io_expanding_buffer io(64 * 1024);

                // Create image_output
                sail::image_output output(io, codec);

                // Write image
                if (output.next_frame(img) != SAIL_OK)
                {
                    throw std::runtime_error("Failed to save image to bytes");
                }

                // Finish writing
                output.finish();

                // Get size and read data
                size_t data_size;
                if (io.size(&data_size) != SAIL_OK)
                {
                    throw std::runtime_error("Failed to get expanding buffer size");
                }
                if (io.seek(0, SEEK_SET) != SAIL_OK)
                {
                    throw std::runtime_error("Failed to seek to beginning of expanding buffer");
                }

                // Read all data
                std::vector<char> buffer(data_size);
                if (io.strict_read(buffer.data(), data_size) != SAIL_OK)
                {
                    throw std::runtime_error("Failed to read from expanding buffer");
                }

                return py::bytes(buffer.data(), data_size);
            },
            py::arg("format") = "png", "Save image to bytes in specified format (default: png)")

        .def("convert",
             [](sail::image& img, SailPixelFormat pixel_format) {
                 auto status = img.convert(pixel_format);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to convert image");
                 }
             },
             py::arg("pixel_format"), "Convert image to specified pixel format")
        .def("convert",
             [](sail::image& img, SailPixelFormat pixel_format, const sail::conversion_options& options) {
                 auto status = img.convert(pixel_format, options);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to convert image with options");
                 }
             },
             py::arg("pixel_format"), py::arg("options"), "Convert image with options")
        .def("convert",
             [](sail::image& img, const sail::save_features& save_features) {
                 auto status = img.convert(save_features);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to convert image to best pixel format for saving");
                 }
             },
             py::arg("save_features"), "Convert image to best pixel format for saving")
        .def("convert",
             [](sail::image& img, const sail::save_features& save_features, const sail::conversion_options& options) {
                 auto status = img.convert(save_features, options);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to convert image to best pixel format for saving with options");
                 }
             },
             py::arg("save_features"), py::arg("options"), "Convert image to best pixel format for saving with options")

        .def("convert_to",
             [](const sail::image& img, SailPixelFormat pixel_format) {
                 sail::image result = img.convert_to(pixel_format);
                 if (!result.is_valid())
                 {
                     throw std::runtime_error("Failed to convert image to specified pixel format");
                 }
                 return result;
             },
             py::arg("pixel_format"), "Convert to specified pixel format and return new image")
        .def("convert_to",
             [](const sail::image& img, SailPixelFormat pixel_format, const sail::conversion_options& options) {
                 sail::image result = img.convert_to(pixel_format, options);
                 if (!result.is_valid())
                 {
                     throw std::runtime_error("Failed to convert image with options");
                 }
                 return result;
             },
             py::arg("pixel_format"), py::arg("options"), "Convert to specified pixel format with options and return new image")
        .def("convert_to",
             [](const sail::image& img, const sail::save_features& save_features) {
                 sail::image result = img.convert_to(save_features);
                 if (!result.is_valid())
                 {
                     throw std::runtime_error("Failed to convert image for saving");
                 }
                 return result;
             },
             py::arg("save_features"), "Convert to best pixel format for saving and return new image")
        .def("convert_to",
             [](const sail::image& img, const sail::save_features& save_features, const sail::conversion_options& options) {
                 sail::image result = img.convert_to(save_features, options);
                 if (!result.is_valid())
                 {
                     throw std::runtime_error("Failed to convert image for saving with options");
                 }
                 return result;
             },
             py::arg("save_features"), py::arg("options"),
             "Convert to best pixel format for saving with options and return new image")

        .def("can_convert", py::overload_cast<SailPixelFormat>(&sail::image::can_convert), py::arg("pixel_format"),
             "Check if image can be converted to specified pixel format")
        .def("closest_pixel_format",
             py::overload_cast<const std::vector<SailPixelFormat>&>(&sail::image::closest_pixel_format, py::const_),
             py::arg("pixel_formats"), "Find closest pixel format from list")
        .def("closest_pixel_format",
             py::overload_cast<const sail::save_features&>(&sail::image::closest_pixel_format, py::const_),
             py::arg("save_features"), "Find closest pixel format from save features")

        .def("mirror",
             [](sail::image& img, SailOrientation orientation) {
                 auto status = img.mirror(orientation);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to mirror image");
                 }
             },
             py::arg("orientation"), "Mirror image horizontally or vertically")

        .def("rotate",
             [](sail::image& img, SailOrientation angle) {
                 auto status = img.rotate(angle);
                 if (status != SAIL_OK)
                 {
                     throw std::runtime_error("Failed to rotate image");
                 }
             },
             py::arg("angle"), "Rotate image in-place by 90, 180, or 270 degrees clockwise")
        .def("rotate_to",
             [](const sail::image& img, SailOrientation angle) {
                 sail::image result = img.rotate_to(angle);
                 if (!result.is_valid())
                 {
                     throw std::runtime_error("Failed to rotate image");
                 }
                 return result;
             },
             py::arg("angle"), "Rotate image by 90, 180, or 270 degrees clockwise and return new image")

        // NumPy integration
        .def("to_numpy", &image_to_numpy,
             "Convert image to NumPy array with appropriate dtype (uint8 or uint16, zero-copy)")
        .def_static("from_numpy", &numpy_to_image, py::arg("array"), py::arg("pixel_format"),
                    "Create image from NumPy array (supports uint8 and uint16)")

        // Buffer protocol support
        .def_buffer([](sail::image& img) -> py::buffer_info {
            bool use_uint16 = is_16bit_per_channel(img.pixel_format());

            if (use_uint16)
            {
                std::vector<py::ssize_t> shape = {
                    static_cast<py::ssize_t>(img.height()),
                    static_cast<py::ssize_t>(img.bytes_per_line() / 2) // Elements, not bytes
                };
                std::vector<py::ssize_t> strides = {
                    static_cast<py::ssize_t>(img.bytes_per_line()),
                    2 // 2 bytes per uint16
                };
                return py::buffer_info(img.pixels(), sizeof(uint16_t), py::format_descriptor<uint16_t>::format(), 2,
                                       shape, strides);
            }
            else
            {
                std::vector<py::ssize_t> shape   = {static_cast<py::ssize_t>(img.height()),
                                                    static_cast<py::ssize_t>(img.bytes_per_line())};
                std::vector<py::ssize_t> strides = {static_cast<py::ssize_t>(img.bytes_per_line()), 1};
                return py::buffer_info(img.pixels(), sizeof(uint8_t), py::format_descriptor<uint8_t>::format(), 2,
                                       shape, strides);
            }
        })

        .def("__repr__",
             [](const sail::image& img) {
                 return "Image(" + std::to_string(img.width()) + "x" + std::to_string(img.height()) + ", "
                        + std::to_string(img.bits_per_pixel()) + "bpp)";
             })

        // Static helper methods
        .def_static(
            "check_conversion", static_cast<bool (*)(SailPixelFormat, SailPixelFormat)>(&sail::image::can_convert),
            py::arg("input_format"), py::arg("output_format"), "Check if conversion between formats is possible")
        .def_static("find_closest_pixel_format",
                    static_cast<SailPixelFormat (*)(SailPixelFormat, const std::vector<SailPixelFormat>&)>(
                        &sail::image::closest_pixel_format),
                    py::arg("input_format"), py::arg("pixel_formats"),
                    "Find closest pixel format from list for given input format")
        .def_static("get_channels", static_cast<unsigned (*)(SailPixelFormat)>(&sail::image::pixel_format_channels),
                    py::arg("pixel_format"), "Get number of channels for format")
        .def_static("get_bits_per_pixel", static_cast<unsigned (*)(SailPixelFormat)>(&sail::image::bits_per_pixel),
                    py::arg("pixel_format"), "Get bits per pixel for format")
        .def_static("calculate_bytes_per_line",
                    static_cast<unsigned (*)(unsigned, SailPixelFormat)>(&sail::image::bytes_per_line),
                    py::arg("width"), py::arg("pixel_format"), "Calculate bytes per line for given width and format")
        .def_static("check_indexed", static_cast<bool (*)(SailPixelFormat)>(&sail::image::is_indexed),
                    py::arg("pixel_format"), "Check if format is indexed")
        .def_static("check_grayscale", static_cast<bool (*)(SailPixelFormat)>(&sail::image::is_grayscale),
                    py::arg("pixel_format"), "Check if format is grayscale")
        .def_static("check_rgb_family", static_cast<bool (*)(SailPixelFormat)>(&sail::image::is_rgb_family),
                    py::arg("pixel_format"), "Check if format is RGB-like")
        .def_static("check_floating_point", static_cast<bool (*)(SailPixelFormat)>(&sail::image::is_floating_point),
                    py::arg("pixel_format"), "Check if format uses floating point representation")

        // Pythonic loading API
        .def_static(
            "from_file",
            [](const std::string& path) {
                // Check if file exists first
                {
                    std::ifstream file(path);
                    if (!file.good())
                    {
                        PyErr_SetString(PyExc_FileNotFoundError, ("File not found: " + path).c_str());
                        throw py::error_already_set();
                    }
                }

                sail::image img(path);
                if (!img.is_valid())
                {
                    throw std::runtime_error("Failed to load image from: " + path);
                }
                return img;
            },
            py::arg("path"), "Load image from file (convenience method)")

        .def_static(
            "from_bytes",
            [](py::bytes data) {
                char* buf_ptr;
                Py_ssize_t buf_size;
                if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                {
                    throw std::runtime_error("Failed to get buffer data");
                }

                sail::image_input input(buf_ptr, static_cast<std::size_t>(buf_size));
                sail::image img = input.next_frame();

                if (!img.is_valid())
                {
                    throw std::runtime_error("Failed to load image from bytes");
                }

                return img;
            },
            py::arg("data"), "Load image from bytes (convenience method)");

    // ============================================================================
    // ImageInput - Pythonic wrapper over sail::image_input
    // ============================================================================

    py::class_<sail::image_input>(m, "ImageInput", "Load images with support for animations and multi-page formats")
        .def(py::init([](const std::string& path) {
                 try
                 {
                     return new sail::image_input(path);
                 }
                 catch (const std::exception& e)
                 {
                     throw std::runtime_error("Failed to open image file '" + path + "' for loading: " + e.what());
                 }
             }),
             py::arg("path"), "Open image file for loading")
        .def(py::init([](py::bytes data) {
                 char* buf_ptr;
                 Py_ssize_t buf_size;
                 if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                 {
                     throw std::runtime_error("Failed to get buffer data");
                 }
                 return new sail::image_input(buf_ptr, static_cast<std::size_t>(buf_size));
             }),
             py::arg("data"), "Open image from bytes for loading")

        // Iterator protocol
        .def("__iter__", [](sail::image_input& input) -> sail::image_input& { return input; })
        .def("__next__",
             [](sail::image_input& input) {
                 sail::image img = input.next_frame();
                 if (!img.is_valid())
                 {
                     throw py::stop_iteration();
                 }
                 return img;
             })

        // Context manager
        .def("__enter__", [](sail::image_input& input) -> sail::image_input& { return input; })
        .def("__exit__", [](sail::image_input& input, py::object, py::object, py::object) { input.finish(); })

        // Direct methods
        .def(
            "load",
            [](sail::image_input& input) {
                sail::image img = input.next_frame();
                if (!img.is_valid())
                {
                    throw std::runtime_error("No more frames available");
                }
                return img;
            },
            "Load next frame/image")

        .def(
            "load_all",
            [](sail::image_input& input) {
                std::vector<sail::image> images;
                while (true)
                {
                    sail::image img = input.next_frame();
                    if (!img.is_valid())
                    {
                        break;
                    }
                    images.push_back(std::move(img));
                }
                return images;
            },
            "Load all frames/images")

        .def_static(
            "probe",
            [](const std::string& path) -> py::dict {
                try
                {
                    sail::image_input input(path);
                    auto probe_result           = input.probe();
                    sail::image img             = std::get<0>(probe_result);
                    sail::codec_info codec_info = std::get<1>(probe_result);

                    py::dict result;
                    result["width"]             = img.width();
                    result["height"]            = img.height();
                    result["pixel_format"]      = img.pixel_format();
                    result["bits_per_pixel"]    = img.bits_per_pixel();
                    result["codec_name"]        = codec_info.name();
                    result["codec_description"] = codec_info.description();

                    // Add source image info if available
                    if (img.source_image().is_valid())
                    {
                        result["source_pixel_format"] = img.source_image().pixel_format();
                        result["source_compression"]  = img.source_image().compression();
                    }

                    return result;
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::string("Failed to probe image: ") + e.what());
                }
            },
            py::arg("path"), "Probe image metadata without loading pixels (static method)")

        .def(
            "finish",
            [](sail::image_input& input) {
                auto status = input.finish();
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to finish loading");
                }
            },
            "Finish loading")

        // Options and codec override
        .def(
            "with_options",
            [](sail::image_input& input, const sail::load_options& options) -> sail::image_input& {
                return input.with(options);
            },
            py::arg("options"), py::return_value_policy::reference_internal,
            "Override load options (returns self for chaining)")

        .def(
            "with_codec",
            [](sail::image_input& input, const sail::codec_info& codec) -> sail::image_input& {
                return input.with(codec);
            },
            py::arg("codec"), py::return_value_policy::reference_internal, "Override codec (returns self for chaining)")

        .def("__repr__", [](const sail::image_input&) { return "ImageInput()"; });

    // ============================================================================
    // ImageOutput - Pythonic wrapper over sail::image_output
    // ============================================================================

    py::class_<sail::image_output>(m, "ImageOutput", "Save images with support for animations and multi-page formats")
        .def(py::init([](const std::string& path) {
                 try
                 {
                     return new sail::image_output(path);
                 }
                 catch (const std::exception& e)
                 {
                     throw std::runtime_error("Failed to open image file '" + path + "' for writing: " + e.what());
                 }
             }),
             py::arg("path"), "Open image file for writing")

        // Context manager (must keep object alive)
        .def(
            "__enter__", [](sail::image_output& output) -> sail::image_output* { return &output; },
            py::return_value_policy::reference_internal)
        .def("__exit__",
             [](sail::image_output& output, py::object exc_type, py::object exc_value, py::object traceback) {
                 output.finish();
                 return false; // Don't suppress exceptions
             })

        // Direct methods
        .def(
            "save",
            [](sail::image_output& output, const sail::image& img) {
                auto status = output.next_frame(img);
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to save image frame");
                }
            },
            py::arg("image"), "Save single frame/image")

        .def(
            "save_all",
            [](sail::image_output& output, const std::vector<sail::image>& images) {
                for (const auto& img : images)
                {
                    auto status = output.next_frame(img);
                    if (status != SAIL_OK)
                    {
                        throw std::runtime_error("Failed to save image frame");
                    }
                }
            },
            py::arg("images"), "Save multiple frames/images")

        .def(
            "finish",
            [](sail::image_output& output) {
                auto status = output.finish();
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to finish saving");
                }
            },
            "Finish saving")

        // Options and codec override
        .def(
            "with_options",
            [](sail::image_output& output, const sail::save_options& options) -> sail::image_output& {
                return output.with(options);
            },
            py::arg("options"), py::return_value_policy::reference_internal,
            "Override save options (returns self for chaining)")

        .def(
            "with_codec",
            [](sail::image_output& output, const sail::codec_info& codec) -> sail::image_output& {
                return output.with(codec);
            },
            py::arg("codec"), py::return_value_policy::reference_internal, "Override codec (returns self for chaining)")

        .def("__repr__", [](const sail::image_output&) { return "ImageOutput()"; });
}
