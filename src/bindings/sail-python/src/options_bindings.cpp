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

void init_options_classes(py::module_& m)
{
    // ============================================================================
    // LoadOptions - Options for loading operations
    // ============================================================================

    py::class_<sail::load_options>(m, "LoadOptions", "Options to modify loading operations")
        .def(py::init<>(), "Create empty load options")
        .def(py::init<int>(), py::arg("options"), "Create load options with specified or-ed options")

        // Properties
        .def_property("options", &sail::load_options::options, &sail::load_options::set_options,
                      "Or-ed manipulation options (see Option enum)")

        .def_property(
            "tuning",
            [](sail::load_options& opts) -> std::unordered_map<std::string, sail::variant>& { return opts.tuning(); },
            [](sail::load_options& opts, const std::unordered_map<std::string, sail::variant>& tuning) {
                opts.set_tuning(tuning);
            },
            py::return_value_policy::reference_internal, "Codec-specific tuning options (dict[str, Variant])")

        // Methods
        .def("__repr__", [](const sail::load_options& opts) {
            return "LoadOptions(options=" + std::to_string(opts.options()) + ")";
        });

    // ============================================================================
    // SaveOptions - Options for saving operations
    // ============================================================================

    py::class_<sail::save_options>(m, "SaveOptions", "Options to modify saving operations")
        .def(py::init<>(), "Create empty save options")
        .def(py::init<int>(), py::arg("options"), "Create save options with specified or-ed options")

        // Properties
        .def_property("options", &sail::save_options::options, &sail::save_options::set_options,
                      "Or-ed manipulation options (see Option enum)")

        .def_property("compression", &sail::save_options::compression, &sail::save_options::set_compression,
                      "Compression type (see Compression enum)")

        .def_property("compression_level", &sail::save_options::compression_level,
                      &sail::save_options::set_compression_level, "Compression level (codec-specific range)")

        .def_property(
            "tuning",
            [](sail::save_options& opts) -> std::unordered_map<std::string, sail::variant>& { return opts.tuning(); },
            [](sail::save_options& opts, const std::unordered_map<std::string, sail::variant>& tuning) {
                opts.set_tuning(tuning);
            },
            py::return_value_policy::reference_internal, "Codec-specific tuning options (dict[str, Variant])")

        // Methods
        .def("__repr__", [](const sail::save_options& opts) {
            return "SaveOptions(options=" + std::to_string(opts.options())
                   + ", compression=" + std::to_string(static_cast<int>(opts.compression()))
                   + ", compression_level=" + std::to_string(opts.compression_level()) + ")";
        });
}
