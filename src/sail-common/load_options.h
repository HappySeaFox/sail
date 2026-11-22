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

#include <stdint.h>

#include <sail-common/export.h>
#include <sail-common/status.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sail_hash_map;
struct sail_load_features;

/*
 * Options to modify loading operations.
 *
 * Always allocate load options with sail_alloc_load_options() or sail_alloc_load_options_from_features()
 * and never create this structure on the stack or with your own allocations. New fields may be added
 * to the end of this structure, and SAIL relies on the full structure being allocated.
 */
struct sail_load_options
{
    /* Or-ed manipulation options for loading operations. See SailOption. */
    int options;

    /*
     * Codec-specific tuning options. For example, a hypothetical ABC image codec
     * can allow disabling filtering with setting the "abc-filtering" tuning option
     * to 0 in load options. Tuning options' names start with the codec name
     * to avoid confusion.
     *
     * Can be NULL.
     *
     * The list of possible values for every tuning option is not currently available
     * programmatically. Every codec must document them in the codec info.
     *
     * It's not guaranteed that tuning options and their values are backward
     * or forward compatible.
     */
    struct sail_hash_map* tuning;

    /*
     * Limits.
     *
     * Limits protect from images that need too much memory or time to load, for example
     * from malicious images with huge dimensions in their headers.
     *
     * For every limit, 0 means no limit, which is the default.
     *
     * Dimension limits (max_width, max_height, max_pixels) apply to loading and probing.
     * An image that exceeds any of them fails with SAIL_ERROR_IMAGE_TOO_LARGE before pixels
     * get allocated. For multi-frame images, they apply to every frame.
     *
     * The frame limit (max_frames) applies to loading only, as probing reads just the first frame.
     *
     * Exceeding a limit is handled like any other loading error. Stop loading with sail_stop_loading().
     * Skipping a too large frame and continuing to load the next frames is not supported.
     */

    /* Maximum allowed image width. */
    unsigned max_width;

    /* Maximum allowed image height. */
    unsigned max_height;

    /*
     * Maximum allowed number of pixels (width * height) in an image. Unlike max_width
     * and max_height, it limits the memory needed to hold the image pixels.
     */
    uint64_t max_pixels;

    /*
     * Maximum allowed number of frames to load. All the allowed frames load as usual.
     * sail_load_next_frame() fails with SAIL_ERROR_TOO_MANY_FRAMES when the image has more frames
     * and the caller requests the frame that follows the last allowed one.
     */
    unsigned max_frames;
};

typedef struct sail_load_options sail_load_options_t;

/*
 * Allocates load options.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_load_options(struct sail_load_options** load_options);

/*
 * Destroys the specified load options object and all its internal allocated memory buffers. The load options
 * MUST NOT be used anymore after calling this function. Does nothing if the load options is NULL.
 */
SAIL_EXPORT void sail_destroy_load_options(struct sail_load_options* load_options);

/*
 * Allocates and builds default load options from the load features.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_load_options_from_features(const struct sail_load_features* load_features,
                                                                struct sail_load_options** load_options);

/*
 * Makes a deep copy of the specified load options object.
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_copy_load_options(const struct sail_load_options* source,
                                                 struct sail_load_options** target);

/*
 * Checks the image dimensions against the max_width, max_height, and max_pixels limits
 * of the load options. Codecs call it as early as the dimensions are known to avoid allocating
 * memory for images that will be rejected anyway.
 *
 * Pass 0 as the width or height when it's not known yet. A zero dimension is never
 * reported as too large, so only the other dimension gets checked. max_pixels is checked
 * only when both dimensions are known.
 *
 * Returns SAIL_OK if the load options are NULL or the dimensions fit the limits.
 * Returns SAIL_ERROR_IMAGE_TOO_LARGE otherwise.
 */
SAIL_EXPORT sail_status_t sail_check_image_dimensions_allowed(const struct sail_load_options* load_options,
                                                              unsigned width,
                                                              unsigned height);

/* extern "C" */
#ifdef __cplusplus
}
#endif
