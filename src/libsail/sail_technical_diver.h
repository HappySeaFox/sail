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

#ifndef SAIL_SAIL_TECHNICAL_DIVER_H
#define SAIL_SAIL_TECHNICAL_DIVER_H

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

struct sail_io;
struct sail_codec_info;
struct sail_read_options;
struct sail_write_options;

/*
 * Starts reading the specified I/O stream.
 *
 * Typical usage: sail_alloc_io()                  ->
 *                set I/O callbacks                ->
 *                sail_codec_info_from_extension() ->
 *                sail_start_reading_io()          ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading()              ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_io(struct sail_io *io, const struct sail_codec_info *codec_info, void **state);

/*
 * Starts reading the specified I/O stream with the specified read options. If you don't need specific read options,
 * just pass NULL. Codec-specific defaults will be used in this case. The read options are deep copied.
 *
 * Typical usage: sail_alloc_io()                      ->
 *                set I/O callbacks                    ->
 *                sail_codec_info_from_extension()     ->
 *                sail_start_reading_io_with_options() ->
 *                sail_read_next_frame()               ->
 *                sail_stop_reading()                  ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_reading. States must be used per image. DO NOT use the same state
 * to read multiple images in the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_reading_io_with_options(struct sail_io *io,
                                                            const struct sail_codec_info *codec_info,
                                                            const struct sail_read_options *read_options, void **state);

/*
 * Starts writing into the specified I/O stream.
 *
 * Typical usage: sail_alloc_io()                  ->
 *                set I/O callbacks                ->
 *                sail_codec_info_from_extension() ->
 *                sail_start_writing_file()        ->
 *                sail_write_next_frame()          ->
 *                sail_stop_writing()              ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_io(struct sail_io *io, const struct sail_codec_info *codec_info, void **state);

/*
 * Starts writing the specified I/O stream with the specified write options. If you don't need specific write options,
 * just pass NULL. Codec-specific defaults will be used in this case. The write options are deep copied.
 *
 * Typical usage: sail_alloc_io()                      ->
 *                set I/O callbacks                    ->
 *                sail_codec_info_from_extension()     ->
 *                sail_start_writing_io_with_options() ->
 *                sail_write_next_frame()              ->
 *                sail_stop_writing()                  ->
 *                sail_destroy_io().
 *
 * STATE explanation: Pass the address of a local void* pointer. SAIL will store an internal state
 * in it and destroy it in sail_stop_writing. States must be used per image. DO NOT use the same state
 * to write multiple images in the same time.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_start_writing_io_with_options(struct sail_io *io,
                                                            const struct sail_codec_info *codec_info,
                                                            const struct sail_write_options *write_options, void **state);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
