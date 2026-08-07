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

struct sail_codec_info;
struct sail_io;
struct sail_image;

/*
 * Loads an image from the specified I/O source and returns its properties without pixels.
 * The assigned codec info is a borrowed pointer to internal context data. Do not free or modify it.
 * Remains valid until sail_finish(). If you don't need it, just pass NULL.
 *
 * This function is pretty fast because it doesn't decode the whole image data for most image formats.
 *
 * The codec is selected by magic number. Some formats share the same magic numbers
 * (for example TIFF and DNG), so the selected codec may be incorrect. Prefer
 * sail_probe_file() or sail_probe_io_with_options() with an explicit codec when the
 * format is known.
 *
 * On success, the I/O position is restored to where it was before probing, so subsequent loading
 * from the same I/O source can continue from the original position. The I/O source must be seekable.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_probe_io(struct sail_io* io,
                                        struct sail_image** image,
                                        const struct sail_codec_info** codec_info);

/*
 * Loads an image from the specified memory buffer and returns its properties without pixels.
 * The assigned codec info is a borrowed pointer to internal context data. Do not free or modify it.
 * Remains valid until sail_finish(). If you don't need it, just pass NULL.
 *
 * This function is pretty fast because it doesn't decode the whole image data for most image formats.
 *
 * The codec is selected by magic number. Some formats share the same magic numbers
 * (for example TIFF and DNG), so the selected codec may be incorrect. Prefer
 * sail_probe_file() or sail_probe_io_with_options() with an explicit codec when the
 * format is known.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_probe_memory(const void* buffer,
                                            size_t buffer_size,
                                            struct sail_image** image,
                                            const struct sail_codec_info** codec_info);

/*
 * Starts loading the specified image file. Pass codec info if you would like to start loading
 * with a specific codec. If not, just pass NULL, and SAIL will detect it automatically.
 *
 * Typical usage: sail_start_loading_from_file() ->
 *                sail_load_next_frame()         ->
 *                sail_stop_loading().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_loading_from_file()   ->
 *                sail_load_next_frame()           ->
 *                sail_stop_loading().
 *
 * STATE: Pass the address of a local void* initialized to NULL. On success, SAIL stores opaque
 * loading state in it. Do not free state. Always call sail_stop_loading() when done, including
 * on error. After sail_stop_loading() the state handle is invalid. Set your local void* to NULL.
 * Use one state per image source. Do not call sail_finish() or sail_unload_codecs() between
 * sail_start_loading_from_file() and sail_stop_loading().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_loading_from_file(const char* path,
                                                       const struct sail_codec_info* codec_info,
                                                       void** state);

/*
 * Starts loading the specified memory buffer.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_loading_from_memory() ->
 *                sail_load_next_frame()           ->
 *                sail_stop_loading().
 *
 * STATE: Pass the address of a local void* initialized to NULL. On success, SAIL stores opaque
 * loading state in it. Do not free state. Always call sail_stop_loading() when done, including
 * on error. After sail_stop_loading() the state handle is invalid. Set your local void* to NULL.
 * Use one state per image source. Do not call sail_finish() or sail_unload_codecs() between
 * sail_start_loading_from_file() and sail_stop_loading().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_loading_from_memory(const void* buffer,
                                                         size_t buffer_size,
                                                         const struct sail_codec_info* codec_info,
                                                         void** state);

/*
 * Continues loading started by sail_start_loading_from_file() and brothers.
 *
 * Returns SAIL_OK on success.
 * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
 *
 * Always call sail_stop_loading() when you are done, including after SAIL_ERROR_NO_MORE_FRAMES
 * and after any other error. The result of calling sail_load_next_frame() again after an error
 * is unspecified.
 */
SAIL_EXPORT sail_status_t sail_load_next_frame(void* state, struct sail_image** image);

/*
 * Stops loading started by sail_start_loading_from_file() and brothers.
 * Does nothing if state is NULL.
 *
 * Always call sail_stop_loading() after sail_start_loading_*(), including on error. Frees internal
 * resources allocated by the matching start function. After this call the state handle is invalid.
 * Set your local void* to NULL.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_loading(void* state);

/*
 * Starts saving into the specified image file. Pass codec info if you'd like to start saving
 * with a specific codec. If not, just pass NULL, and SAIL will detect it automatically.
 *
 * Typical usage: sail_start_saving_into_file() ->
 *                sail_write_next_frame()       ->
 *                sail_stop_saving().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_saving_into_file()    ->
 *                sail_write_next_frame()          ->
 *                sail_stop_saving().
 *
 * STATE: Pass the address of a local void* initialized to NULL. On success, SAIL stores opaque
 * saving state in it. Do not free state. Always call sail_stop_saving() when done, including
 * on error. After sail_stop_saving() the state handle is invalid. Set your local void* to NULL.
 * Use one state per image target. Do not call sail_finish() or sail_unload_codecs() between
 * sail_start_saving_into_file() and sail_stop_saving().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_saving_into_file(const char* path,
                                                      const struct sail_codec_info* codec_info,
                                                      void** state);

/*
 * Starts saving the specified memory buffer.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_saving_into_memory()  ->
 *                sail_write_next_frame()          ->
 *                sail_stop_saving().
 *
 * STATE: Pass the address of a local void* initialized to NULL. On success, SAIL stores opaque
 * saving state in it. Do not free state. Always call sail_stop_saving() when done, including
 * on error. After sail_stop_saving() the state handle is invalid. Set your local void* to NULL.
 * Use one state per image target. Do not call sail_finish() or sail_unload_codecs() between
 * sail_start_saving_into_file() and sail_stop_saving().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_saving_into_memory(void* buffer,
                                                        size_t buffer_size,
                                                        const struct sail_codec_info* codec_info,
                                                        void** state);

/*
 * Continues saving started by sail_start_saving_into_file() and brothers. Writes the specified
 * image into the underlying I/O target.
 *
 * If the selected image format doesn't support the image pixel format, an error is returned.
 * Consider converting the image into a supported image format beforehand with functions
 * from sail-manip.
 *
 * On error always call sail_stop_saving(). The result of calling sail_write_next_frame() again
 * after an error is unspecified.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_write_next_frame(void* state, const struct sail_image* image);

/*
 * Stops saving started by sail_start_saving_into_file() and brothers. Closes the underlying I/O target.
 * Does nothing if state is NULL.
 *
 * Always call sail_stop_saving() after sail_start_saving_*(), including on error. Frees internal
 * resources allocated by the matching start function. After this call the state handle is invalid.
 * Set your local void* to NULL.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_saving(void* state);

/* extern "C" */
#ifdef __cplusplus
}
#endif
