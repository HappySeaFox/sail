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

#ifndef SAIL_CODEC_OPTIONS_H
#define SAIL_CODEC_OPTIONS_H

#include <stdbool.h>

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

/* Codec options. See sail_put_codec_option(). */
static const char * const SAIL_CODEC_OPTION_META_DATA  = "META-DATA";
static const char * const SAIL_CODEC_OPTION_INTERLACED = "INTERLACED";
static const char * const SAIL_CODEC_OPTION_ICCP       = "ICCP";

struct sail_hash_map;

/*
 * Sets the specified option value in the codec options.
 */
SAIL_EXPORT void sail_put_codec_option(struct sail_hash_map *codec_options, const char *codec_option, bool value);

/*
 * Returns the codec option value or the specified default value if the option is absent.
 */
SAIL_EXPORT bool sail_codec_option(const struct sail_hash_map *codec_options, const char *codec_option, bool def);

/*
 * Sets the meta data option value.
 */
SAIL_EXPORT void sail_put_meta_data_codec_option(struct sail_hash_map *codec_options, bool value);

/*
 * Returns the meta data option value or true if the option is absent.
 */
SAIL_EXPORT bool sail_meta_data_codec_option(const struct sail_hash_map *codec_options);

/*
 * Sets the interlaced option value.
 */
SAIL_EXPORT void sail_put_interlaced_codec_option(struct sail_hash_map *codec_options, bool value);

/*
 * Returns the interlaced option value or true if the option is absent.
 */
SAIL_EXPORT bool sail_interlaced_codec_option(const struct sail_hash_map *codec_options);

/*
 * Sets the ICC profile option value.
 */
SAIL_EXPORT void sail_put_iccp_codec_option(struct sail_hash_map *codec_options, bool value);

/*
 * Returns the ICC profile option value or true if the option is absent.
 */
SAIL_EXPORT bool sail_iccp_codec_option(const struct sail_hash_map *codec_options);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
