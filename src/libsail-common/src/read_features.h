/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_READ_FEATURES_H
#define SAIL_READ_FEATURES_H

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

/*
 * Read features. Use this structure to determine what a plugin can actually read.
 */
struct sail_read_features {

    /*
     * A list of supported pixel formats that can be read by this plugin. One of these values
     * will be stored in sail_image.source_pixel_format. See SailPixelFormat.
     *
     * For example: BPP32-CMYK, BPP24-YCBCR, BPP24-RGB.
     *
     * NOTE: Some input pixel formats might not map to some output pixel formats.
     *       Let's take a look at a hypothetical example:
     *
     * A hypothetical SAIL plugin supports input YCBCR images and it's able to output RGB pixel data from them.
     * Additionally, it supports input CMYK images and it's able to output YCCK pixel data from them.
     * So the full conversion table with all possible input/output variants looks like this:
     *
     * [ Read from file ] YCBCR => RGB  [ Output to memory ]
     * [ Read from file ] CMYK  => YCCK [ Output to memory ]
     *
     * sail_read_features.input_pixel_formats will contain YCBCR and CMYK pixel formats.
     * sail_read_features.output_pixel_formats will contain RGB and YCCK pixel formats.
     *
     * However, if you try to read a YCBCR image and output YCCK pixels, the codec will return
     * an error.
     */
    int *input_pixel_formats;

    /* The length of input_pixel_formats. */
    int input_pixel_formats_length;

    /*
     * A list of supported pixel formats that can be output by this plugin.
     *
     * It's not guaranteed that every input pixel format in input_pixel_formats can be converted
     * to every output pixel format in output_pixel_formats. Some could be converted and some not.
     * See the comments above.
     *
     * If the array contains SAIL_PIXEL_FORMAT_SOURCE, then the codec is able to output raw pixel data.
     * It is a caller's responsibility to convert it to a suitable pixel format. Refer to
     * sail_image.pixel_format to detect the actual pixel format of the raw pixel data in that case.
     *
     * For example: SOURCE, BPP24-RGB, BPP32-YCCK.
     */
    int *output_pixel_formats;

    /* The length of output_pixel_formats. */
    int output_pixel_formats_length;

    /*
     * Output pixel format to use by default when no specific output pixel format was requested by user.
     * It's always BPP24-RGB for image formats without transparency support and BPP32-RGBA otherwise.
     */
    int preferred_output_pixel_format;

    /* Supported plugin features of reading operations. See SailPluginFeatures. */
    int features;
};

typedef struct sail_read_features sail_read_features_t;

/*
 * Allocates read features. The assigned read features MUST be destroyed later
 * with sail_destroy_read_features().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_read_features(struct sail_read_features **read_features);

/*
 * Destroys the specified read features object and all its internal allocated memory buffers. The read features
 * MUST NOT be used anymore after calling this function. Does nothing if the read features is NULL.
 */
SAIL_EXPORT void sail_destroy_read_features(struct sail_read_features *read_features);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
