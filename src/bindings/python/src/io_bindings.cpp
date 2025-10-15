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

void init_io(py::module_& m)
{
    // ============================================================================
    // Junior API - convenience functions
    // ============================================================================

    m.def(
        "load_from_file",
        [](const std::string& path) {
            sail::image img(path);
            if (!img.is_valid())
            {
                throw std::runtime_error("Failed to load image from: " + path);
            }
            return img;
        },
        py::arg("path"), "Load image from file");

    m.def(
        "save_into_file",
        [](const sail::image& img, const std::string& path) {
            auto status = const_cast<sail::image&>(img).save(path);
            if (status != SAIL_OK)
            {
                throw std::runtime_error("Failed to save image to: " + path);
            }
        },
        py::arg("image"), py::arg("path"), "Save image to file");

    // TODO: Add load_from_memory and save_into_memory
    // TODO: Add ImageInput and ImageOutput for frame-by-frame loading
}
