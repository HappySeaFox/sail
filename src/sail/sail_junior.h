/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

#pragma once

#include <stddef.h> /* size_t */

#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * One shot load and save functions. They call sail_start_*() and sail_stop_*() internally.
 * The caller does not manage state. Multi frame files load only the first frame. Use the advanced
 * API to read all frames.
 */

struct sail_image;
struct sail_io;
struct sail_codec_info;

/*
 * Loads the specified image file and returns its properties without pixels.
 * The assigned codec info is a borrowed pointer to internal context data. Do not free or modify it.
 * Remains valid until sail_finish(). If you don't need it, just pass NULL.
 *
 * This function is pretty fast because it doesn't decode the whole image data for most image formats.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_probe_file(const char* path,
                                          struct sail_image** image,
                                          const struct sail_codec_info** codec_info);

/*
 * Loads the specified image file and returns its properties and pixels.
 *
 * Calls sail_start_loading_from_file() and sail_stop_loading() internally. Only the first frame
 * is loaded for multi frame files. Use sail_start_loading_from_file() to read all frames.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_load_from_file(const char* path, struct sail_image** image);

/*
 * Loads an image from the specified memory buffer and returns its properties and pixels.
 *
 * Calls sail_start_loading_from_memory() and sail_stop_loading() internally. Only the first frame
 * is loaded for multi frame files.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_load_from_memory(const void* buffer, size_t buffer_size, struct sail_image** image);

/*
 * Saves the specified image into the file.
 *
 * If the selected image format doesn't support the image pixel format, an error is returned.
 * Consider converting the image into a supported image format beforehand with functions
 * from sail-manip.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_save_into_file(const char* path, const struct sail_image* image);

/*
 * Saves the specified image into the specified memory buffer.
 *
 * If the selected image format doesn't support the image pixel format, an error is returned.
 * Consider converting the image into a supported image format beforehand with functions
 * from sail-manip.
 *
 * Saves the number of bytes written into the 'written' argument if it's not NULL.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_save_into_memory(void* buffer,
                                                size_t buffer_size,
                                                const struct sail_image* image,
                                                size_t* written);

/* extern "C" */
#ifdef __cplusplus
}
#endif
