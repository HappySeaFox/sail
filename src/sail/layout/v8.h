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

/*
 * Codec layout V8 contract between libsail and codec plugins.
 *
 * Purpose:
 *   This file documents the eight functions every layout V8 codec must provide. It also declares
 *   those functions when SAIL_CODEC_NAME is defined, which the combined sail-codecs library uses
 *   to build function pointer tables. Implement the functions with SAIL_EXPORT in your codec source
 *   file. You do not need to include this header in the codec .c file.
 *
 * Export naming:
 *   Dynamic codecs must export all eight functions. libsail resolves them at runtime by name:
 *     sail_codec_load_init_v8_{name}
 *     sail_codec_load_seek_next_frame_v8_{name}
 *     sail_codec_load_frame_v8_{name}
 *     sail_codec_load_finish_v8_{name}
 *     sail_codec_save_init_v8_{name}
 *     sail_codec_save_seek_next_frame_v8_{name}
 *     sail_codec_save_frame_v8_{name}
 *     sail_codec_save_finish_v8_{name}
 *   {name} is the codec name from the .codec.info file converted to lower case (JPEG -> jpeg).
 *   All eight symbols must be present even when loading or saving is not supported. Return
 *   SAIL_ERROR_NOT_IMPLEMENTED from unsupported operations.
 *
 * .codec.info:
 *   Set layout=8 in the [codec] section. The name field must match the suffix used in exported
 *   symbols after lowercasing. Declare supported operations and pixel formats in [load-features]
 *   and [save-features].
 *
 * Call sequences:
 *   Full load, repeated per frame:
 *     load_init -> load_seek_next_frame -> load_frame -> load_seek_next_frame -> ... -> load_finish
 *   Probing, metadata only, no pixels:
 *     load_init -> load_seek_next_frame -> load_finish
 *     load_frame is not called during probing.
 *   Save, repeated per frame:
 *     save_init -> save_seek_next_frame -> save_frame -> save_seek_next_frame -> ... -> save_finish
 *
 * State ownership:
 *   The void** state argument in init and finish is codec internal state, not the application level
 *   state passed to sail_start_loading_from_file(). Allocate it in init. Destroy it in finish and set
 *   *state to NULL. Use one codec state per init/finish cycle. Do not reuse the same codec state for
 *   two IO streams at the same time.
 *   On any error after load_init or save_init, libsail calls the matching finish function to clean up.
 *   After an error in load_seek_next_frame, load_frame, save_seek_next_frame, or save_frame, the result
 *   of calling them again before finish is unspecified. Always make finish safe to call after a partial
 *   operation.
 *
 * Include guards are not used as the header may be included multiple times with different
 * SAIL_CODEC_NAME definitions.
 */

#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SAIL_CODEC_NAME
/*
 * Generate syntax error.
 *
 * Usage:
 *
 * #define SAIL_CODEC_NAME jpeg
 * #include <sail/layout/v8.h>
 */
Please define SAIL_CODEC_NAME before including this header.
#endif

#define SAIL_CONSTRUCT_CODEC_FUNC_CONCAT2_IMPL(a, b) a##_##b
#define SAIL_CONSTRUCT_CODEC_FUNC_CONCAT2(a, b) SAIL_CONSTRUCT_CODEC_FUNC_CONCAT2_IMPL(a, b)
#define SAIL_CONSTRUCT_CODEC_FUNC(name) SAIL_CONSTRUCT_CODEC_FUNC_CONCAT2(name, SAIL_CODEC_NAME)

    /*
     * Decoding functions.
     */

    /*
     * Starts decoding the specified io stream using the specified options.
     *
     * libsail, the caller of this function, guarantees the following:
     *   - The IO is valid and open.
     *   - The load options is not NULL.
     *
     * This function MUST:
     *   - Allocate an internal state object with internal data structures necessary to decode a file,
     *     and assign its value to the state.
     *
     * STATE: Pass the address of a local void* initialized to NULL. On success, store the codec internal
     * state in *state. Destroy it in load_finish and set *state to NULL. Use one codec state per IO stream.
     * Do not reuse the same state pointer for concurrent loads.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_load_init_v8)(struct sail_io* io,
                                                                     const struct sail_load_options* load_options,
                                                                     void** state);

/*
 * Seeks to the next frame. The frame is NOT immediately loaded or decoded by most SAIL codecs.
 * SAIL uses this method in loading and probing operations.
 *
 * SAIL uses sail_codec_load_frame_v8() to actually load the frame.
 *
 * During probing, libsail calls load_init, then this function once, then load_finish. load_frame is
 * not called. Fill every field needed to describe the image without pixels.
 *
 * Respect load_options->options flags. For example, allocate and fill sail_image.source_image only when
 * SAIL_OPTION_SOURCE_IMAGE is set. Fill meta data only when SAIL_OPTION_META_DATA is set. Fill ICC profile
 * only when SAIL_OPTION_ICCP is set.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state points to the state allocated by sail_codec_load_init_v8().
 *
 * This function MUST:
 *   - Allocate the image and the source image (sail_image.source_image) when requested by load options.
 *   - Fill the expected image properties (width, height, pixel format, bytes_per_line, image properties
 *     etc.) and meta data.
 *     The image pixel format must be as close to the source as possible.
 *   - Seek to the next image frame.
 *
 * This function MUST NOT:
 *   - Allocate the image pixels. They will be allocated by libsail and will be available in
 *     sail_codec_load_frame_v8().
 *
 * Returns SAIL_OK on success.
 * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
 *
 * After an error, the result of calling this function again before load_finish is unspecified.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_load_seek_next_frame_v8)(void* state, struct sail_image** image);

/*
 * Reads the next frame of the current image. The image pixels are pre-allocated by libsail.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state is valid and points to the state allocated by sail_codec_load_init_v8().
 *   - The image points to the image allocated by sail_codec_load_seek_next_frame_v8().
 *   - The image pixels are allocated.
 *
 * This function MUST:
 *   - Read the image pixels into sail_image.pixels.
 *   - Output pixels with the origin in the top left corner (i.e. not flipped).
 *   - Output pixels in format as close to the source as possible.
 *
 * Returns SAIL_OK on success.
 *
 * After an error, the result of calling this function again before load_finish is unspecified.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_load_frame_v8)(void* state, struct sail_image* image);

/*
 * Finalizes loading operation. No more loadings are possible after calling this function.
 * This function doesn't close the io stream. It just stops decoding. Use io->close() or sail_destroy_io()
 * to actually close the io stream.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state points to the state allocated by sail_codec_load_init_v8().
 *
 * This function MUST:
 *   - Destroy the state and set it to NULL.
 *
 * This function MUST NOT:
 *   - Close the IO.
 *
 * libsail may call this function after a failed load_init, load_seek_next_frame, or load_frame. The
 * function must tolerate a partially initialized state and always clear *state on success.
 *
 * Returns SAIL_OK on success.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_load_finish_v8)(void** state);

/*
 * Encoding functions.
 */

/*
 * Starts encoding the specified io stream using the specified options. The specified save options
 * will be deep copied into an internal buffer.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The IO is valid and open.
 *   - The save options is not NULL.
 *
 * This function MUST:
 *   - Allocate an internal state object with internal data structures necessary to encode a file,
 *     and assign its value to the state.
 *
 * STATE: Pass the address of a local void* initialized to NULL. On success, store the codec internal
 * state in *state. Destroy it in save_finish and set *state to NULL. Use one codec state per IO stream.
 * Do not reuse the same state pointer for concurrent saves.
 *
 * Returns SAIL_OK on success.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_save_init_v8)(struct sail_io* io,
                                                                 const struct sail_save_options* save_options,
                                                                 void** state);

/*
 * Seeks to a next frame before saving it. The frame is NOT immediately written. Use sail_codec_save_frame_v8()
 * to actually save a frame.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state points to the state allocated by sail_codec_save_init_v8().
 *   - The image is valid.
 *
 * This function MUST:
 *   - Seek to the right position before saving the next image frame.
 *
 * Returns SAIL_OK on success.
 *
 * After an error, the result of calling this function again before save_finish is unspecified.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_save_seek_next_frame_v8)(void* state,
                                                                            const struct sail_image* image);

/*
 * Writes a next frame of the current image.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state is valid and points to the state allocated by sail_codec_save_init_v8().
 *   - The image is valid.
 *
 * This function MUST:
 *   - Write the image pixels and meta data into the IO.
 *
 * Returns SAIL_OK on success.
 *
 * After an error, the result of calling this function again before save_finish is unspecified.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_save_frame_v8)(void* state, const struct sail_image* image);

/*
 * Finalizes saving operation. No more savings are possible after calling this function.
 * This function doesn't close the io stream. Use io->close() or sail_destroy_io() to actually
 * close the io stream.
 *
 * libsail, the caller of this function, guarantees the following:
 *   - The state points to the state allocated by sail_codec_save_init_v8().
 *
 * This function MUST:
 *   - Destroy the state and set it to NULL.
 *
 * This function MUST NOT:
 *   - Close the IO.
 *
 * libsail may call this function after a failed save_init, save_seek_next_frame, or save_frame. The
 * function must tolerate a partially initialized state and always clear *state on success.
 *
 * Returns SAIL_OK on success.
 */
sail_status_t SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_save_finish_v8)(void** state);

/* extern "C" */
#ifdef __cplusplus
}
#endif
