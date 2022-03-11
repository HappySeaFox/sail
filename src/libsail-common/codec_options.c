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

#include "sail-common.h"

static const char * const SAIL_CODEC_OPTION_META_DATA  = "META-DATA";
static const char * const SAIL_CODEC_OPTION_INTERLACED = "INTERLACED";
static const char * const SAIL_CODEC_OPTION_ICCP       = "ICCP";

void sail_put_meta_data_codec_option(struct sail_hash_map *codec_options, bool value) {

    struct sail_variant *variant;
    SAIL_TRY_OR_EXECUTE(sail_alloc_variant(&variant),
                        /* on error */ return);

    sail_set_variant_bool(variant, value);

    sail_put_hash_map(codec_options, SAIL_CODEC_OPTION_META_DATA, variant);

    sail_destroy_variant(variant);
}

bool sail_meta_data_codec_option(const struct sail_hash_map *codec_options) {

    const struct sail_variant *variant = sail_hash_map_value(codec_options, SAIL_CODEC_OPTION_META_DATA);

    if (SAIL_UNLIKELY(variant == NULL)) {
        return true;
    } else {
        return sail_variant_to_bool(variant);
    }
}

void sail_put_interlaced_codec_option(struct sail_hash_map *codec_options, bool value) {

    struct sail_variant *variant;
    SAIL_TRY_OR_EXECUTE(sail_alloc_variant(&variant),
                        /* on error */ return);

    sail_set_variant_bool(variant, value);

    sail_put_hash_map(codec_options, SAIL_CODEC_OPTION_INTERLACED, variant);

    sail_destroy_variant(variant);
}

bool sail_interlaced_codec_option(const struct sail_hash_map *codec_options) {

    const struct sail_variant *variant = sail_hash_map_value(codec_options, SAIL_CODEC_OPTION_INTERLACED);

    if (SAIL_UNLIKELY(variant == NULL)) {
        return true;
    } else {
        return sail_variant_to_bool(variant);
    }
}

void sail_put_iccp_codec_option(struct sail_hash_map *codec_options, bool value) {

    struct sail_variant *variant;
    SAIL_TRY_OR_EXECUTE(sail_alloc_variant(&variant),
                        /* on error */ return);

    sail_set_variant_bool(variant, value);

    sail_put_hash_map(codec_options, SAIL_CODEC_OPTION_ICCP, variant);

    sail_destroy_variant(variant);
}

bool sail_iccp_codec_option(const struct sail_hash_map *codec_options) {

    const struct sail_variant *variant = sail_hash_map_value(codec_options, SAIL_CODEC_OPTION_ICCP);

    if (SAIL_UNLIKELY(variant == NULL)) {
        return true;
    } else {
        return sail_variant_to_bool(variant);
    }
}
