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

void init_core_classes(py::module_& m)
{
    // ============================================================================
    // ArbitraryData (represented as bytes in Python)
    // ============================================================================
    // sail::arbitrary_data is std::vector<uint8_t>, will be converted automatically

    // ============================================================================
    // Variant
    // ============================================================================

    py::class_<sail::variant>(m, "Variant", "Variant type that can hold different data types")
        .def(py::init<>(), "Create an invalid variant")
        // bytes MUST come before string to avoid implicit conversion
        .def(py::init([](py::bytes data) {
                 char* buf_ptr;
                 Py_ssize_t buf_size;
                 if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                 {
                     throw std::runtime_error("Failed to get buffer data");
                 }
                 sail::arbitrary_data arb_data(reinterpret_cast<const uint8_t*>(buf_ptr),
                                               reinterpret_cast<const uint8_t*>(buf_ptr) + buf_size);
                 return sail::variant(arb_data);
             }),
             py::arg("data"), "Create variant from binary data (bytes)")
        .def(py::init<bool>(), py::arg("value"), "Create variant from bool")
        .def(py::init<int>(), py::arg("value"), "Create variant from int")
        .def(py::init<double>(), py::arg("value"), "Create variant from float")
        .def(py::init<const std::string&>(), py::arg("value"), "Create variant from string")

        .def_property_readonly("is_valid", &sail::variant::is_valid, "Check if variant has a value")

        .def("has_bool", &sail::variant::has_value<bool>, "Check if variant contains a bool")
        .def("has_int", &sail::variant::has_value<int>, "Check if variant contains an int")
        .def("has_float", &sail::variant::has_value<double>, "Check if variant contains a float")
        .def("has_string", &sail::variant::has_value<std::string>, "Check if variant contains a string")
        .def("has_data", &sail::variant::has_value<sail::arbitrary_data>, "Check if variant contains binary data")

        .def("to_bool", &sail::variant::value<bool>, "Get bool value")
        .def("to_int", &sail::variant::value<int>, "Get int value")
        .def("to_float", &sail::variant::value<double>, "Get float value")
        .def("to_string", &sail::variant::value<std::string>, "Get string value")
        .def(
            "to_data",
            [](const sail::variant& v) -> py::bytes {
                const auto& data = v.value<sail::arbitrary_data>();
                return py::bytes(reinterpret_cast<const char*>(data.data()), data.size());
            },
            "Get binary data as bytes")

        .def("set_bool", &sail::variant::set_value<bool>, py::arg("value"), "Set bool value")
        .def("set_int", &sail::variant::set_value<int>, py::arg("value"), "Set int value")
        .def("set_float", &sail::variant::set_value<double>, py::arg("value"), "Set float value")
        .def("set_string", &sail::variant::set_value<std::string>, py::arg("value"), "Set string value")
        .def(
            "set_data",
            [](sail::variant& v, py::bytes data) {
                char* buf_ptr;
                Py_ssize_t buf_size;
                if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                {
                    throw std::runtime_error("Failed to get buffer data");
                }
                sail::arbitrary_data vec(buf_ptr, buf_ptr + buf_size);
                v.set_value(vec);
            },
            py::arg("data"), "Set binary data from bytes")

        .def("clear", &sail::variant::clear, "Clear the variant")

        .def("__repr__", [](const sail::variant& v) {
            if (!v.is_valid())
            {
                return std::string("Variant(invalid)");
            }
            if (v.has_value<bool>())
            {
                return std::string("Variant(bool: ") + (v.value<bool>() ? "true" : "false") + ")";
            }
            if (v.has_value<int>())
            {
                return std::string("Variant(int: ") + std::to_string(v.value<int>()) + ")";
            }
            if (v.has_value<double>())
            {
                return std::string("Variant(float: ") + std::to_string(v.value<double>()) + ")";
            }
            if (v.has_value<std::string>())
            {
                return std::string("Variant(string: \"") + v.value<std::string>() + "\")";
            }
            if (v.has_value<sail::arbitrary_data>())
            {
                return std::string("Variant(data: ") + std::to_string(v.value<sail::arbitrary_data>().size())
                       + " bytes)";
            }
            return std::string("Variant(unknown)");
        });

    // ============================================================================
    // Palette
    // ============================================================================

    py::class_<sail::palette>(m, "Palette", "Image palette for indexed images")
        .def(py::init<>(), "Create an invalid palette")
        .def(py::init<SailPixelFormat, const void*, unsigned>(), py::arg("pixel_format"), py::arg("data"),
             py::arg("color_count"), "Create palette from pixel format, data, and color count")

        .def_property_readonly("is_valid", &sail::palette::is_valid, "Check if palette is valid")
        .def_property_readonly("pixel_format", &sail::palette::pixel_format, "Get palette pixel format")
        .def_property_readonly("color_count", &sail::palette::color_count, "Get number of colors in palette")

        .def(
            "get_data",
            [](const sail::palette& p) -> py::bytes {
                const auto& data = p.data();
                return py::bytes(reinterpret_cast<const char*>(data.data()), data.size());
            },
            "Get palette data as bytes")

        .def(
            "set_data",
            [](sail::palette& p, SailPixelFormat pixel_format, py::bytes data, unsigned color_count) {
                char* buf_ptr;
                Py_ssize_t buf_size;
                if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                {
                    throw std::runtime_error("Failed to get buffer data");
                }
                p.set_data(pixel_format, buf_ptr, color_count);
            },
            py::arg("pixel_format"), py::arg("data"), py::arg("color_count"), "Set palette data")

        .def("__repr__",
             [](const sail::palette& p) { return "Palette(" + std::to_string(p.color_count()) + " colors)"; });

    // ============================================================================
    // Iccp (ICC Profile)
    // ============================================================================

    py::class_<sail::iccp>(m, "Iccp", "ICC color profile")
        .def(py::init<>(), "Create an invalid ICC profile")
        .def(py::init([](py::bytes data) {
                 char* buf_ptr;
                 Py_ssize_t buf_size;
                 if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                 {
                     throw std::runtime_error("Failed to get buffer data");
                 }
                 return new sail::iccp(buf_ptr, static_cast<std::size_t>(buf_size));
             }),
             py::arg("data"), "Create ICC profile from binary data (bytes)")

        .def_property_readonly("is_valid", &sail::iccp::is_valid, "Check if ICC profile is valid")

        .def(
            "get_data",
            [](const sail::iccp& ic) -> py::bytes {
                const auto& data = ic.data();
                return py::bytes(reinterpret_cast<const char*>(data.data()), data.size());
            },
            "Get ICC profile data as bytes")

        .def(
            "set_data",
            [](sail::iccp& ic, py::bytes data) {
                char* buf_ptr;
                Py_ssize_t buf_size;
                if (PYBIND11_BYTES_AS_STRING_AND_SIZE(data.ptr(), &buf_ptr, &buf_size) != 0)
                {
                    throw std::runtime_error("Failed to get buffer data");
                }
                ic.set_data(buf_ptr, static_cast<std::size_t>(buf_size));
            },
            py::arg("data"), "Set ICC profile data")

        .def("__repr__", [](const sail::iccp& ic) { return "Iccp(" + std::to_string(ic.data().size()) + " bytes)"; });

    // ============================================================================
    // MetaData
    // ============================================================================

    py::class_<sail::meta_data>(m, "MetaData", "Image metadata (EXIF, comments, etc.)")
        .def(py::init<>(), "Create empty metadata entry")
        .def(py::init<SailMetaData, const sail::variant&>(), py::arg("key"), py::arg("value"),
             "Create metadata entry with known key")
        .def(py::init<const std::string&, const sail::variant&>(), py::arg("key_unknown"), py::arg("value"),
             "Create metadata entry with custom string key")

        .def_property_readonly("key", &sail::meta_data::key, "Known metadata key (SailMetaData enum)")
        .def_property_readonly("key_unknown", &sail::meta_data::key_unknown, "Custom metadata key string")
        .def_property_readonly("value", &sail::meta_data::value, "Metadata value (Variant)")

        .def("set_key", static_cast<void (sail::meta_data::*)(SailMetaData)>(&sail::meta_data::set_key), py::arg("key"),
             "Set known metadata key")
        .def("set_key_unknown", static_cast<void (sail::meta_data::*)(const std::string&)>(&sail::meta_data::set_key),
             py::arg("key_unknown"), "Set custom metadata key string")
        .def("set_value", static_cast<void (sail::meta_data::*)(const sail::variant&)>(&sail::meta_data::set_value),
             py::arg("value"), "Set metadata value")

        .def_static("meta_data_to_string", &sail::meta_data::meta_data_to_string, py::arg("meta_data"),
                    "Convert metadata key enum to string")
        .def_static("meta_data_from_string", &sail::meta_data::meta_data_from_string, py::arg("str"),
                    "Convert string to metadata key enum")

        .def("__repr__", [](const sail::meta_data& md) {
            std::string key_str;
            if (md.key() == SAIL_META_DATA_UNKNOWN)
            {
                key_str = "\"" + md.key_unknown() + "\"";
            }
            else
            {
                const char* k = sail::meta_data::meta_data_to_string(md.key());
                key_str       = k ? k : "Unknown";
            }
            return "MetaData(key=" + key_str + ")";
        });

    // ============================================================================
    // SourceImage
    // ============================================================================

    py::class_<sail::source_image>(m, "SourceImage", "Source image properties (preserved from loading)")
        .def(py::init<>(), "Create empty source image")

        .def_property_readonly("is_valid", &sail::source_image::is_valid, "Check if source image is valid")
        .def_property_readonly("pixel_format", &sail::source_image::pixel_format, "Get source image pixel format")
        .def_property_readonly("chroma_subsampling", &sail::source_image::chroma_subsampling,
                               "Get source image chroma subsampling")
        .def_property_readonly("orientation", &sail::source_image::orientation, "Get source image orientation")
        .def_property_readonly("compression", &sail::source_image::compression, "Get source image compression type")
        .def_property_readonly("interlaced", &sail::source_image::interlaced, "Check if source image was interlaced")

        .def("__repr__", [](const sail::source_image& si) {
            return "SourceImage(format=" + std::to_string(si.pixel_format())
                   + ", compression=" + std::to_string(si.compression()) + ")";
        });
}
