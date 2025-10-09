/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

enum SailPixelFormat avif_private_sail_pixel_format(enum avifPixelFormat avif_pixel_format, uint32_t depth, bool has_alpha) {

    switch (avif_pixel_format) {
        case AVIF_PIXEL_FORMAT_NONE: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }

        default: {
            switch (depth) {
                case 8: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP32_YUVA : SAIL_PIXEL_FORMAT_BPP24_YUV;
                }
                case 10: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP40_YUVA : SAIL_PIXEL_FORMAT_BPP30_YUV;
                }
                case 12: {
                    return has_alpha ? SAIL_PIXEL_FORMAT_BPP48_YUVA : SAIL_PIXEL_FORMAT_BPP36_YUV;
                }
                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
    }
}

enum SailChromaSubsampling avif_private_sail_chroma_subsampling(enum avifPixelFormat avif_pixel_format) {

    switch (avif_pixel_format) {
        case AVIF_PIXEL_FORMAT_YUV444: return SAIL_CHROMA_SUBSAMPLING_444;
        case AVIF_PIXEL_FORMAT_YUV422: return SAIL_CHROMA_SUBSAMPLING_422;
        case AVIF_PIXEL_FORMAT_YUV420: return SAIL_CHROMA_SUBSAMPLING_420;
        case AVIF_PIXEL_FORMAT_YUV400: return SAIL_CHROMA_SUBSAMPLING_400;

        default: {
            return SAIL_CHROMA_SUBSAMPLING_UNKNOWN;
        }
    }
}

enum SailPixelFormat avif_private_rgb_sail_pixel_format(enum avifRGBFormat rgb_pixel_format, uint32_t depth) {

    switch (depth) {
        case 8: {
            switch (rgb_pixel_format) {
                case AVIF_RGB_FORMAT_RGB:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                case AVIF_RGB_FORMAT_RGBA: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                case AVIF_RGB_FORMAT_ARGB: return SAIL_PIXEL_FORMAT_BPP32_ARGB;
                case AVIF_RGB_FORMAT_BGR:  return SAIL_PIXEL_FORMAT_BPP24_BGR;
                case AVIF_RGB_FORMAT_BGRA: return SAIL_PIXEL_FORMAT_BPP32_BGRA;
                case AVIF_RGB_FORMAT_ABGR: return SAIL_PIXEL_FORMAT_BPP32_ABGR;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case 16: {
            switch (rgb_pixel_format) {
                case AVIF_RGB_FORMAT_RGB:  return SAIL_PIXEL_FORMAT_BPP48_RGB;
                case AVIF_RGB_FORMAT_RGBA: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
                case AVIF_RGB_FORMAT_ARGB: return SAIL_PIXEL_FORMAT_BPP64_ARGB;
                case AVIF_RGB_FORMAT_BGR:  return SAIL_PIXEL_FORMAT_BPP48_BGR;
                case AVIF_RGB_FORMAT_BGRA: return SAIL_PIXEL_FORMAT_BPP64_BGRA;
                case AVIF_RGB_FORMAT_ABGR: return SAIL_PIXEL_FORMAT_BPP64_ABGR;

                default: return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

uint32_t avif_private_round_depth(uint32_t depth) {

    if (depth > 8) {
        return 16;
    } else {
        return 8;
    }
}

sail_status_t avif_private_fetch_iccp(const struct avifRWData *avif_iccp, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(avif_iccp);
    SAIL_CHECK_PTR(iccp);

    if (avif_iccp->data != NULL) {
        SAIL_TRY(sail_alloc_iccp_from_data(avif_iccp->data, avif_iccp->size, iccp));
        SAIL_LOG_TRACE("AVIF: Found ICC profile %u bytes long", (unsigned)avif_iccp->size);
    } else {
        SAIL_LOG_TRACE("AVIF: ICC profile is not found");
    }

    return SAIL_OK;
}

sail_status_t avif_private_fetch_meta_data(enum SailMetaData key, const struct avifRWData *avif_rw_data, struct sail_meta_data_node **meta_data_node) {

    if (avif_rw_data->data != NULL) {
        struct sail_meta_data_node *meta_data_node_local;

        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node_local));

        SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(key, &meta_data_node_local->meta_data),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
        SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node_local->meta_data->value, avif_rw_data->data, avif_rw_data->size),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

        *meta_data_node = meta_data_node_local;
    }

    return SAIL_OK;
}

bool avif_private_sail_pixel_format_to_avif_rgb_format(enum SailPixelFormat pixel_format, enum avifRGBFormat *rgb_format, uint32_t *depth) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            *rgb_format = AVIF_RGB_FORMAT_RGB;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            *rgb_format = AVIF_RGB_FORMAT_RGBA;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB: {
            *rgb_format = AVIF_RGB_FORMAT_ARGB;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP24_BGR: {
            *rgb_format = AVIF_RGB_FORMAT_BGR;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA: {
            *rgb_format = AVIF_RGB_FORMAT_BGRA;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR: {
            *rgb_format = AVIF_RGB_FORMAT_ABGR;
            *depth = 8;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP48_RGB: {
            *rgb_format = AVIF_RGB_FORMAT_RGB;
            *depth = 16;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: {
            *rgb_format = AVIF_RGB_FORMAT_RGBA;
            *depth = 16;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP64_ARGB: {
            *rgb_format = AVIF_RGB_FORMAT_ARGB;
            *depth = 16;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP48_BGR: {
            *rgb_format = AVIF_RGB_FORMAT_BGR;
            *depth = 16;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP64_BGRA: {
            *rgb_format = AVIF_RGB_FORMAT_BGRA;
            *depth = 16;
            return true;
        }
        case SAIL_PIXEL_FORMAT_BPP64_ABGR: {
            *rgb_format = AVIF_RGB_FORMAT_ABGR;
            *depth = 16;
            return true;
        }
        default: {
            return false;
        }
    }
}

sail_status_t avif_private_write_iccp(struct avifImage *avif_image, const struct sail_iccp *iccp) {

    SAIL_CHECK_PTR(avif_image);

    if (iccp != NULL && iccp->data != NULL) {
        avifResult result = avifImageSetProfileICC(avif_image, iccp->data, iccp->size);

        if (result != AVIF_RESULT_OK) {
            SAIL_LOG_ERROR("AVIF: Failed to set ICC profile: %s", avifResultToString(result));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        SAIL_LOG_TRACE("AVIF: ICC profile has been written");
    }

    return SAIL_OK;
}

sail_status_t avif_private_write_meta_data(struct avifEncoder *encoder, struct avifImage *avif_image, const struct sail_meta_data_node *meta_data_node) {

    SAIL_CHECK_PTR(encoder);
    SAIL_CHECK_PTR(avif_image);

    (void)encoder;

    for (const struct sail_meta_data_node *node = meta_data_node; node != NULL; node = node->next) {
        if (node->meta_data->key == SAIL_META_DATA_EXIF &&
            node->meta_data->value->type == SAIL_VARIANT_TYPE_DATA) {
            avifResult result = avifImageSetMetadataExif(avif_image,
                                                          (const uint8_t *)node->meta_data->value->value,
                                                          node->meta_data->value->size);

            if (result != AVIF_RESULT_OK) {
                SAIL_LOG_ERROR("AVIF: Failed to set EXIF: %s", avifResultToString(result));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            SAIL_LOG_TRACE("AVIF: EXIF has been written");
        } else if (node->meta_data->key == SAIL_META_DATA_XMP &&
                   node->meta_data->value->type == SAIL_VARIANT_TYPE_DATA) {
            avifResult result = avifImageSetMetadataXMP(avif_image,
                                                         (const uint8_t *)node->meta_data->value->value,
                                                         node->meta_data->value->size);

            if (result != AVIF_RESULT_OK) {
                SAIL_LOG_ERROR("AVIF: Failed to set XMP: %s", avifResultToString(result));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            SAIL_LOG_TRACE("AVIF: XMP has been written");
        }
    }

    return SAIL_OK;
}

bool avif_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    struct avifEncoder *encoder = user_data;

    if (strcmp(key, "avif-speed") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int speed = sail_variant_to_int(value);
            if (speed >= 0 && speed <= 10) {
                encoder->speed = speed;
                SAIL_LOG_TRACE("AVIF: Set speed to %d", speed);
            }
        }
    } else if (strcmp(key, "avif-threads") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int threads = sail_variant_to_int(value);
            if (threads > 0) {
                encoder->maxThreads = threads;
                SAIL_LOG_TRACE("AVIF: Set max threads to %d", threads);
            }
        }
    } else if (strcmp(key, "avif-auto-tiling") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_BOOL) {
            encoder->autoTiling = sail_variant_to_bool(value) ? AVIF_TRUE : AVIF_FALSE;
            SAIL_LOG_TRACE("AVIF: Set auto tiling to %s", sail_variant_to_bool(value) ? "true" : "false");
        }
    }

    return true;
}

bool avif_private_load_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    struct avifDecoder *decoder = user_data;

    if (strcmp(key, "avif-threads") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int threads = sail_variant_to_int(value);
            if (threads > 0) {
                decoder->maxThreads = threads;
                SAIL_LOG_TRACE("AVIF: Set decoder max threads to %d", threads);
            }
        }
    }

    return true;
}
