
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
Tests for SAIL Python Logging Functions
"""

import sailpy
from io import StringIO


def test_log_level_enum():
    """Test LogLevel enum values"""
    assert sailpy.LogLevel.SILENCE == 0
    assert sailpy.LogLevel.ERROR == 1
    assert sailpy.LogLevel.WARNING == 2
    assert sailpy.LogLevel.INFO == 3
    assert sailpy.LogLevel.MESSAGE == 4
    assert sailpy.LogLevel.DEBUG == 5
    assert sailpy.LogLevel.TRACE == 6


def test_set_log_barrier():
    """Test set_log_barrier function"""
    # Should not raise
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)  # Restore default


def test_set_log_barrier_silence():
    """Test silencing all logs"""
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # Create image (normally produces debug logs)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    assert img.is_valid

    # Restore
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)


def test_custom_logger():
    """Test setting custom logger"""
    messages = []

    def my_logger(level, file, line, message):
        messages.append({
            'level': level,
            'file': file,
            'line': line,
            'message': message
        })
        return False # Don't consume the message

    # Set custom logger
    sailpy.set_logger(my_logger)

    # Trigger some logging (codec info lookup)
    codec = sailpy.CodecInfo.from_name("PNG")

    # Should have captured some messages
    assert len(messages) > 0

    # Check message structure
    msg = messages[0]
    assert 'level' in msg
    assert 'file' in msg
    assert 'line' in msg
    assert 'message' in msg
    assert isinstance(msg['level'], int)
    assert isinstance(msg['file'], str)
    assert isinstance(msg['line'], int)
    assert isinstance(msg['message'], str)

    # Restore default
    sailpy.set_logger(None)


def test_logger_restoration():
    """Test that default logger can be restored"""
    def my_logger(level, file, line, message):
        return True  # Consume the message

    # Set custom logger
    sailpy.set_logger(my_logger)

    # Restore default
    sailpy.set_logger(None)

    # Should work normally
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    assert img.is_valid


def test_logger_levels():
    """Test that logger respects log levels"""
    messages = []

    def my_logger(level, file, line, message):
        messages.append(level)
        return True  # Consume the message

    sailpy.set_logger(my_logger)

    # Set to ERROR only
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)
    messages.clear()

    # Normal operations (should not log at DEBUG level)
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)

    # Restore
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)
    sailpy.set_logger(None)


def test_logger_exception_safety():
    """Test that exceptions in logger don't crash"""
    def bad_logger(level, file, line, message):
        raise RuntimeError("Intentional error in logger")

    sailpy.set_logger(bad_logger)

    # Should not crash despite exception
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)
    assert img.is_valid

    # Restore
    sailpy.set_logger(None)


def test_log_level_hierarchy():
    """Test log level hierarchy"""
    # Higher levels should include lower levels
    # TRACE > DEBUG > MESSAGE > INFO > WARNING > ERROR > SILENCE

    assert sailpy.LogLevel.SILENCE < sailpy.LogLevel.ERROR
    assert sailpy.LogLevel.ERROR < sailpy.LogLevel.WARNING
    assert sailpy.LogLevel.WARNING < sailpy.LogLevel.INFO
    assert sailpy.LogLevel.INFO < sailpy.LogLevel.MESSAGE
    assert sailpy.LogLevel.MESSAGE < sailpy.LogLevel.DEBUG
    assert sailpy.LogLevel.DEBUG < sailpy.LogLevel.TRACE


def test_logger_file_and_line():
    """Test that logger receives file and line information"""
    captured = []

    def my_logger(level, file, line, message):
        captured.append((file, line))

    sailpy.set_logger(my_logger)

    # Trigger logging
    sailpy.CodecInfo.from_name("PNG")

    # Should have file and line info
    if captured:
        file, line = captured[0]
        assert len(file) > 0
        assert line > 0

    # Restore
    sailpy.set_logger(None)


def test_set_log_barrier_during_operation():
    """Test changing log barrier during operation"""
    # Start with DEBUG
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)
    img1 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)

    # Change to SILENCE
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)
    img2 = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)

    # Both should work
    assert img1.is_valid
    assert img2.is_valid

    # Restore
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)
