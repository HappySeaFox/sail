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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sail-c++/sail-c++.h>

namespace py = pybind11;

void init_codec_info(py::module_& m)
{
    // ============================================================================
    // CompressionLevel - Compression level range
    // ============================================================================

    py::class_<sail::compression_level>(m, "CompressionLevel", "Compression level range supported by a codec")
        .def_property_readonly("is_valid", &sail::compression_level::is_valid, "Check if compression level is valid")
        .def_property_readonly("min_level", &sail::compression_level::min_level, "Minimum compression level")
        .def_property_readonly("max_level", &sail::compression_level::max_level, "Maximum compression level")
        .def_property_readonly("default_level", &sail::compression_level::default_level, "Default compression level")
        .def_property_readonly("step", &sail::compression_level::step, "Compression level step")

        .def("__repr__", [](const sail::compression_level& cl) {
            if (!cl.is_valid())
            {
                return std::string("CompressionLevel(invalid)");
            }
            return "CompressionLevel(min=" + std::to_string(cl.min_level()) + ", max=" + std::to_string(cl.max_level())
                   + ", default=" + std::to_string(cl.default_level()) + ")";
        });

    // ============================================================================
    // LoadFeatures - What a codec can load
    // ============================================================================

    py::class_<sail::load_features>(m, "LoadFeatures", "Features supported by a codec for loading")
        .def_property_readonly("features", &sail::load_features::features,
                               "Or-ed codec features (see CodecFeature enum)")

        .def(
            "to_options",
            [](const sail::load_features& lf) -> sail::load_options {
                sail::load_options options;
                auto status = lf.to_options(&options);
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to convert load features to options");
                }
                return options;
            },
            "Convert load features to load options with default values")

        .def("__repr__", [](const sail::load_features& lf) {
            return "LoadFeatures(features=" + std::to_string(lf.features()) + ")";
        });

    // ============================================================================
    // SaveFeatures - What a codec can save
    // ============================================================================

    py::class_<sail::save_features>(m, "SaveFeatures", "Features supported by a codec for saving")
        .def_property_readonly("features", &sail::save_features::features,
                               "Or-ed codec features (see CodecFeature enum)")

        .def_property_readonly("pixel_formats", &sail::save_features::pixel_formats,
                               "List of pixel formats supported for saving")

        .def_property_readonly("compression_level", &sail::save_features::compression_level,
                               "Compression level range (CompressionLevel object)")

        .def_property_readonly("compressions", &sail::save_features::compressions,
                               "List of supported compression types")

        .def_property_readonly("default_compression", &sail::save_features::default_compression,
                               "Default compression type for this codec")

        .def(
            "to_options",
            [](const sail::save_features& sf) -> sail::save_options {
                sail::save_options options;
                auto status = sf.to_options(&options);
                if (status != SAIL_OK)
                {
                    throw std::runtime_error("Failed to convert save features to options");
                }
                return options;
            },
            "Convert save features to save options with default values")

        .def("__repr__", [](const sail::save_features& sf) {
            return "SaveFeatures(features=" + std::to_string(sf.features())
                   + ", default_compression=" + std::to_string(static_cast<int>(sf.default_compression())) + ")";
        });

    // ============================================================================
    // CodecInfo - Codec information and discovery
    // ============================================================================

    py::class_<sail::codec_info>(m, "CodecInfo", "Information about an image codec")
        .def(py::init<>(), "Create empty codec info")

        // Properties (read-only)
        .def_property_readonly("is_valid", &sail::codec_info::is_valid, "Check if codec info is valid")
        .def_property_readonly("version", &sail::codec_info::version, "Codec version (e.g., '1.2.0')")
        .def_property_readonly("name", &sail::codec_info::name, "Codec name (e.g., 'JPEG')")
        .def_property_readonly("description", &sail::codec_info::description, "Codec description")
        .def_property_readonly("magic_numbers", &sail::codec_info::magic_numbers, "List of magic numbers")
        .def_property_readonly("extensions", &sail::codec_info::extensions, "List of file extensions")
        .def_property_readonly("mime_types", &sail::codec_info::mime_types, "List of MIME types")

        // Features properties
        .def_property_readonly("load_features", &sail::codec_info::load_features,
                               "Load features object with detailed capabilities")

        .def_property_readonly("save_features", &sail::codec_info::save_features,
                               "Save features object with detailed capabilities")

        .def_property_readonly(
            "can_load", [](const sail::codec_info& ci) { return ci.load_features().features() != 0; },
            "Check if codec can load images")

        .def_property_readonly(
            "can_save", [](const sail::codec_info& ci) { return ci.save_features().features() != 0; },
            "Check if codec can save images")

        // Static methods for codec discovery
        .def_static(
            "from_path",
            [](const std::string& path) {
                sail::codec_info codec = sail::codec_info::from_path(path);
                if (!codec.is_valid())
                {
                    PyErr_SetString(PyExc_ValueError, ("No codec found for path: " + path).c_str());
                    throw py::error_already_set();
                }
                return codec;
            },
            py::arg("path"), "Find codec by file path extension")

        .def_static(
            "from_extension",
            [](const std::string& extension) {
                sail::codec_info codec = sail::codec_info::from_extension(extension);
                if (!codec.is_valid())
                {
                    PyErr_SetString(PyExc_ValueError, ("No codec found for extension: " + extension).c_str());
                    throw py::error_already_set();
                }
                return codec;
            },
            py::arg("extension"), "Find codec by file extension (e.g., '.jpg' or 'jpg')")

        .def_static(
            "from_mime_type",
            [](const std::string& mime_type) {
                sail::codec_info codec = sail::codec_info::from_mime_type(mime_type);
                if (!codec.is_valid())
                {
                    PyErr_SetString(PyExc_ValueError, ("No codec found for MIME type: " + mime_type).c_str());
                    throw py::error_already_set();
                }
                return codec;
            },
            py::arg("mime_type"), "Find codec by MIME type (e.g., 'image/jpeg')")

        .def_static(
            "from_name",
            [](const std::string& name) {
                sail::codec_info codec = sail::codec_info::from_name(name);
                if (!codec.is_valid())
                {
                    PyErr_SetString(PyExc_ValueError, ("No codec found with name: " + name).c_str());
                    throw py::error_already_set();
                }
                return codec;
            },
            py::arg("name"), "Find codec by codec name (e.g., 'JPEG' or 'jpeg')")

        .def_static("list", &sail::codec_info::list, "Get list of all available codecs")

        .def("__repr__", [](const sail::codec_info& ci) {
            if (!ci.is_valid())
            {
                return std::string("CodecInfo(invalid)");
            }
            return "CodecInfo(name='" + ci.name() + "', version='" + ci.version() + "')";
        });
}
