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

#include <sail-common/config.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Forward declarations for binding functions from other files
void init_enums(py::module_& m);
void init_image(py::module_& m);
void init_log(py::module_& m);
void init_core_classes(py::module_& m);
void init_options_classes(py::module_& m);
void init_codec_info(py::module_& m);

PYBIND11_MODULE(_libsail, m)
{
    m.doc()               = "Python bindings for SAIL - Squirrel Abstract Imaging Library";
    m.attr("__version__") = SAIL_VERSION_STRING;

    // Initialize submodules
    init_enums(m);
    init_core_classes(m);
    init_options_classes(m);
    init_codec_info(m);
    init_image(m);
    init_log(m);
}
