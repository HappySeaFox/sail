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

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <cstdarg>
#include <cstdio>

#include <sail-c++/sail-c++.h>
#include <sail/sail.h>

namespace py = pybind11;

// Python logger callback wrapper
static py::object python_logger_callback;

static bool python_logger_wrapper(enum SailLogLevel level, const char* file, int line, const char* format, va_list args)
{
    if (python_logger_callback.is_none())
    {
        return false;
    }

    // Format the message
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);

    try
    {
        // Call Python callback
        py::object result =
            python_logger_callback(static_cast<int>(level), std::string(file), line, std::string(buffer));

        // Check if callback returned a boolean value
        if (py::isinstance<py::bool_>(result))
        {
            return result.cast<bool>();
        }

        // Default: message consumed
        return true;
    }
    catch (const std::exception& e)
    {
        // Don't let Python exceptions propagate to C code
        fprintf(stderr, "Error in Python logger callback: %s\n", e.what());
        return false;
    }
}

void init_log(py::module_& m)
{
    // ============================================================================
    // Logging functions
    // ============================================================================

    m.def(
        "set_log_barrier", [](SailLogLevel level) { sail::log::set_barrier(level); }, py::arg("level"),
        R"doc(Set maximum log level barrier.

Only messages at this level or lower will be displayed.
This function is not thread-safe. Call it before using SAIL.

Args:
    level: LogLevel enum value (SILENCE, ERROR, WARNING, INFO, MESSAGE, DEBUG, TRACE)

Example:
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)  # Only errors
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)  # No logs
)doc");

    m.def(
        "set_logger",
        [](py::object callback) {
            if (callback.is_none())
            {
                // Disable custom logger
                python_logger_callback = py::none();
                sail::log::set_logger(nullptr);
            }
            else
            {
                // Set custom Python logger
                python_logger_callback = callback;
                sail::log::set_logger(python_logger_wrapper);
            }
        },
        py::arg("callback"),
        R"doc(Set custom logger callback.

The callback signature: callback(level: int, file: str, line: int, message: str) -> bool (optional)
This function is not thread-safe. Call it before using SAIL.

If the callback returns True (or any truthy value), the log message is considered consumed
and will not be passed to the default logger. If the callback returns False (or None),
the message will be passed to the default logger.

Args:
    callback: Python function or None to restore default logging

Example:
    def my_logger(level, file, line, message):
        print(f"[{level}] {file}:{line} - {message}")
        return True  # Consume message (optional, True by default)

    sailpy.set_logger(my_logger)

    # Restore default
    sailpy.set_logger(None)
)doc");
}
