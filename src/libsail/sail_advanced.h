/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#ifndef SAIL_SAIL_ADVANCED_H
#define SAIL_SAIL_ADVANCED_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_codec_info;

/*
 * Loads an image from the specified I/O source and returns its properties without pixels. The assigned image
 * MUST be destroyed later with sail_destroy_image(). The assigned codec info MUST NOT be destroyed
 * because it is a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * This function is pretty fast because it doesn't decode whole image data for most image formats.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_probe_io(struct sail_io *io, struct sail_image **image, const struct sail_codec_info **codec_info);

/*
 * Loads an image from the specified memory buffer and returns its properties without pixels. The assigned image
 * MUST be destroyed later with sail_destroy_image(). The assigned codec info MUST NOT be destroyed
 * because it is a pointer to an internal data structure. If you don't need it, just pass NULL.
 *
 * This function is pretty fast because it doesn't decode whole image data for most image formats.
 *
 * Typical usage: This is a standalone function that could be called at any time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_probe_mem(const void *buffer, size_t buffer_length,
                                        struct sail_image **image, const struct sail_codec_info **codec_info);

/*
 * Starts reading the specified image file. Pass codec info if you would like to start reading
 * with a specific codec. If not, just pass NULL.
 *
 * Typical usage: sail_start_reading_file() ->
 *                sail_read_next_frame()    ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_reading_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to start reading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_file(const char *path, const struct sail_codec_info *codec_info, void **state);

/*
 * Starts reading the specified memory buffer.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_reading_mem()         ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading(). States must be used per image. DO NOT use the same state
 * to start reading multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_mem(const void *buffer, size_t buffer_length,
                                                const struct sail_codec_info *codec_info, void **state);

/*
 * Continues reading the file started by sail_start_reading_file() and brothers. The assigned image
 * MUST be destroyed later with sail_image_destroy().
 *
 * Returns SAIL_OK on success.
 * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
 */
SAIL_EXPORT sail_status_t sail_read_next_frame(void *state, struct sail_image **image);

/*
 * Stops reading the file started by sail_start_reading_file() and brothers.
 * Does nothing if the state is NULL.
 *
 * It is essential to always stop writing to free memory and I/O resources. Failure to do so
 * will lead to memory leaks.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_reading(void *state);

/*
 * Starts writing into the specified image file. Pass codec info if you'd like to start writing
 * with a specific codec. If not, just pass NULL.
 *
 * Typical usage: sail_start_writing_file() ->
 *                sail_write_next_frame()   ->
 *                sail_stop_writing().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_writing_file()        ->
 *                sail_write_next_frame()          ->
 *                sail_stop_writing().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to start writing multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_file(const char *path, const struct sail_codec_info *codec_info, void **state);

/*
 * Starts writing the specified memory buffer.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_writing_mem()         ->
 *                sail_write_next_frame()          ->
 *                sail_stop_writing().
 *
 * STATE explanation: Passes the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to start writing multiple images at the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_mem(void *buffer, size_t buffer_length,
                                                const struct sail_codec_info *codec_info, void **state);

/*
 * Continues writing started by sail_start_writing_file() and brothers. Writes the specified
 * image into the underlying I/O target.
 *
 * If the selected image format doesn't support the image pixel format, an error is returned.
 * Consider converting the image into a supported image format beforehand with functions
 * from sail-manip.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_write_next_frame(void *state, const struct sail_image *image);

/*
 * Stops writing started by sail_start_writing_file() and brothers. Closes the underlying I/O target.
 * Does nothing if the state is NULL.
 *
 * It is essential to always stop writing to free memory and I/O resources. Failure to do so
 * will lead to memory leaks.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_stop_writing(void *state);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
