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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sail-common.h"

#include "helpers.h"

void jpeg_private_my_output_message(j_common_ptr cinfo) {
    char buffer[JMSG_LENGTH_MAX];

    (*cinfo->err->format_message)(cinfo, buffer);

    SAIL_LOG_ERROR("JPEG: %s", buffer);
}

void jpeg_private_my_error_exit(j_common_ptr cinfo) {
    struct jpeg_private_my_error_context *myerr = (struct jpeg_private_my_error_context *)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

enum SailPixelFormat jpeg_private_color_space_to_pixel_format(J_COLOR_SPACE color_space) {
    switch (color_space) {
        case JCS_GRAYSCALE: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case JCS_RGB:       return SAIL_PIXEL_FORMAT_BPP24_RGB;

#ifdef SAIL_HAVE_JPEG_JCS_EXT
        case JCS_RGB565:    return SAIL_PIXEL_FORMAT_BPP16_RGB565;

        case JCS_EXT_RGB:   return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case JCS_EXT_BGR:   return SAIL_PIXEL_FORMAT_BPP24_BGR;

        case JCS_EXT_RGBA:  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case JCS_EXT_BGRA:  return SAIL_PIXEL_FORMAT_BPP32_BGRA;
        case JCS_EXT_ABGR:  return SAIL_PIXEL_FORMAT_BPP32_ABGR;
        case JCS_EXT_ARGB:  return SAIL_PIXEL_FORMAT_BPP32_ARGB;
#endif

        case JCS_YCbCr:     return SAIL_PIXEL_FORMAT_BPP24_YCBCR;
        case JCS_CMYK:      return SAIL_PIXEL_FORMAT_BPP32_CMYK;
        case JCS_YCCK:      return SAIL_PIXEL_FORMAT_BPP32_YCCK;

        default:            return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

J_COLOR_SPACE jpeg_private_pixel_format_to_color_space(enum SailPixelFormat pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:  return JCS_GRAYSCALE;
        case SAIL_PIXEL_FORMAT_BPP24_RGB:       return JCS_RGB;

#ifdef SAIL_HAVE_JPEG_JCS_EXT
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:    return JCS_RGB565;
        case SAIL_PIXEL_FORMAT_BPP24_BGR:       return JCS_EXT_BGR;

        case SAIL_PIXEL_FORMAT_BPP32_RGBA:      return JCS_EXT_RGBA;
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:      return JCS_EXT_BGRA;
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:      return JCS_EXT_ABGR;
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:      return JCS_EXT_ARGB;
#endif

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:     return JCS_YCbCr;
        case SAIL_PIXEL_FORMAT_BPP32_CMYK:      return JCS_CMYK;
        case SAIL_PIXEL_FORMAT_BPP32_YCCK:      return JCS_YCCK;

        default:                                return JCS_UNKNOWN;
    }
}

sail_status_t jpeg_private_fetch_meta_data(struct jpeg_decompress_struct *decompress_context, struct sail_meta_data_node **last_meta_data_node) {

    SAIL_CHECK_PTR(last_meta_data_node);

    jpeg_saved_marker_ptr it = decompress_context->marker_list;

    while(it != NULL) {
        if(it->marker == JPEG_COM) {
            struct sail_meta_data_node *meta_data_node;
            SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_COMMENT, &meta_data_node->meta_data),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node->meta_data->value),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_substring(meta_data_node->meta_data->value, (const char *)it->data, it->data_length),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

            *last_meta_data_node = meta_data_node;
            last_meta_data_node = &meta_data_node->next;
        }

        it = it->next;
    }

    return SAIL_OK;
}

sail_status_t jpeg_private_write_meta_data(struct jpeg_compress_struct *compress_context, const struct sail_meta_data_node *meta_data_node) {

    while (meta_data_node != NULL) {
        if (meta_data_node->meta_data->value->type == SAIL_VARIANT_TYPE_STRING) {
            jpeg_write_marker(compress_context,
                                JPEG_COM,
                                (JOCTET *)sail_variant_to_string(meta_data_node->meta_data->value),
                                (unsigned)meta_data_node->meta_data->value->size - 1);
        } else {
            SAIL_LOG_WARNING("JPEG: Ignoring unsupported binary key '%s'", sail_meta_data_to_string(meta_data_node->meta_data->key));
        }

        meta_data_node = meta_data_node->next;
    }

    return SAIL_OK;
}

#ifdef SAIL_HAVE_JPEG_ICCP
sail_status_t jpeg_private_fetch_iccp(struct jpeg_decompress_struct *decompress_context, struct sail_iccp **iccp) {

    SAIL_CHECK_PTR(iccp);

    JOCTET *data = NULL;
    unsigned data_length = 0;

    SAIL_LOG_DEBUG("JPEG: ICC profile is %sfound",
                   jpeg_read_icc_profile(decompress_context, &data, &data_length)
                   ? "" : "not ");

    if (data != NULL && data_length > 0) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_from_data(data, data_length, iccp),
                            /* cleanup */ sail_free(data));
    }

    return SAIL_OK;
}
#endif

sail_status_t jpeg_private_fetch_resolution(struct jpeg_decompress_struct *decompress_context, struct sail_resolution **resolution) {

    SAIL_CHECK_PTR(resolution);

    /* Resolution information is not valid. */
    if (decompress_context->X_density == 0 && decompress_context->Y_density == 0) {
        return SAIL_OK;
    }

    SAIL_TRY(sail_alloc_resolution(resolution));

    switch (decompress_context->density_unit) {
        case 1: {
            (*resolution)->unit = SAIL_RESOLUTION_UNIT_INCH;
            break;
        }
        case 2: {
            (*resolution)->unit = SAIL_RESOLUTION_UNIT_CENTIMETER;
            break;
        }
    }

    (*resolution)->x = (float)decompress_context->X_density;
    (*resolution)->y = (float)decompress_context->Y_density;

    return SAIL_OK;
}

sail_status_t jpeg_private_write_resolution(struct jpeg_compress_struct *compress_context, const struct sail_resolution *resolution) {

    /* Not an error. */
    if (resolution == NULL) {
        return SAIL_OK;
    }

    switch (resolution->unit) {
        case SAIL_RESOLUTION_UNIT_INCH: {
            compress_context->density_unit = 1;
            break;
        }
        case SAIL_RESOLUTION_UNIT_CENTIMETER: {
            compress_context->density_unit = 2;
            break;
        }
        default: {
            compress_context->density_unit = 0;
            break;
        }
    }

    compress_context->X_density = (UINT16)resolution->x;
    compress_context->Y_density = (UINT16)resolution->y;

    return SAIL_OK;
}

bool jpeg_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    struct jpeg_compress_struct *compress_context = user_data;

    if (strcmp(key, "jpeg-dct-method") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_STRING) {
            const char *str_value = sail_variant_to_string(value);

            if (strcmp(str_value, "slow") == 0) {
                SAIL_LOG_TRACE("JPEG: Applying SLOW DCT method");
                compress_context->dct_method = JDCT_ISLOW;
            } else if (strcmp(str_value, "fast") == 0) {
                SAIL_LOG_TRACE("JPEG: Applying FAST DCT method");
                compress_context->dct_method = JDCT_IFAST;
            } else if (strcmp(str_value, "float") == 0) {
                SAIL_LOG_TRACE("JPEG: Applying FLOAT DCT method");
                compress_context->dct_method = JDCT_FLOAT;
            }
        }
    } else if (strcmp(key, "jpeg-optimize-coding") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_BOOL) {
            const bool optimize_coding = sail_variant_to_bool(value);

            if (optimize_coding) {
                SAIL_LOG_TRACE("JPEG: Optimizing coding");
                compress_context->optimize_coding = optimize_coding;
            }
        }
    } else if (strcmp(key, "jpeg-smoothing-factor") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            SAIL_LOG_TRACE("JPEG: Smoothing the image");
            compress_context->smoothing_factor = sail_variant_to_unsigned_int(value);
        }
    }

    return true;
}
