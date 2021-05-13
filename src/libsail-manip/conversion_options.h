/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_CONVERSION_OPTIONS_H
#define SAIL_CONVERSION_OPTIONS_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
    #include "pixel.h"

    #include "manip_common.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
    #include <sail-common/pixel.h>

    #include <sail-manip/manip_common.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Options to control image conversion behavior.
 */
struct sail_conversion_options {

    /*
     * Or-ed SailConversionOption-s. If zero, SAIL_CONVERSION_OPTION_DROP_ALPHA is assumed.
     */
    int options;

    /*
     * 48-bit background color to blend into other 16-bit color components instead of alpha
     * when options has SAIL_CONVERSION_OPTION_BLEND_ALPHA.
     */
    sail_rgb48_t background48;

    /*
     * 24-bit background color to blend into other 8-bit color components instead of alpha
     * when options has SAIL_CONVERSION_OPTION_BLEND_ALPHA.
     */
    sail_rgb24_t background24;
};

typedef struct sail_conversion_options sail_conversion_options_t;

/*
 * Allocates new conversion options. The assigned options MUST be destroyed later with sail_destroy_conversion_options().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_alloc_conversion_options(struct sail_conversion_options **options);

/*
 * Destroys the specified conversion options and all its internal allocated memory buffers.
 * The options MUST NOT be used anymore after calling this function. Does nothing if the options is NULL.
 */
SAIL_EXPORT void sail_destroy_conversion_options(struct sail_conversion_options *options);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
