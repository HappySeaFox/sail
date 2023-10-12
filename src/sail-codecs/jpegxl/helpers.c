/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2023 Dmitry Baryshev

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

#include <stddef.h> /* size_t */
#include <string.h> /* memmove */

#include <jxl/version.h>

#include "sail-common.h"

#include "helpers.h"

enum SailPixelFormat jpegxl_private_sail_pixel_format(uint32_t bits_per_sample, uint32_t num_color_channels, uint32_t alpha_bits) {

    SAIL_LOG_TRACE("JPEGXL: Bits per sample(%u), number of channels(%u), alpha bits(%u)",
        bits_per_sample, num_color_channels, alpha_bits);

    /*
     * Also update jpegxl_private_sail_pixel_format_to_num_channels() with new pixel formats.
     */
    switch (num_color_channels) {
        case 1: {
            switch (bits_per_sample) {
                case 8:  return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA : SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                case 16: return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA : SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
        case 3: {
            switch (bits_per_sample) {
                case 8:  return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP32_RGBA : SAIL_PIXEL_FORMAT_BPP24_RGB;
                case 16: return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP64_RGBA : SAIL_PIXEL_FORMAT_BPP48_RGB;

                default: {
                    return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            }
        }
        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

unsigned jpegxl_private_sail_pixel_format_to_num_channels(enum SailPixelFormat pixel_format) {

    switch(pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       return 1;
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: return 2;
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP48_RGB:             return 3;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            return 4;

        default: {
            return 0;
        }
    }
}

JxlDataType jpegxl_private_sail_pixel_format_to_jxl_data_type(enum SailPixelFormat pixel_format) {

    switch(pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            return JXL_TYPE_UINT8;

        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA:
        case SAIL_PIXEL_FORMAT_BPP48_RGB:
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            return JXL_TYPE_UINT16;

        default: {
            return JXL_TYPE_UINT8;
        }
    }
}

sail_status_t jpegxl_private_fetch_iccp(JxlDecoder *decoder, struct sail_iccp **iccp) {

    size_t icc_size;
    if (JxlDecoderGetICCProfileSize(decoder,
#if JPEGXL_NUMERIC_VERSION < JPEGXL_COMPUTE_NUMERIC_VERSION(0, 9, 0)
                                    /* unused */ NULL,
#endif
                                    JXL_COLOR_PROFILE_TARGET_DATA,
                                    &icc_size) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to get ICC size");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp_for_data((unsigned)icc_size, &iccp_local));

    if (JxlDecoderGetColorAsICCProfile(decoder,
#if JPEGXL_NUMERIC_VERSION < JPEGXL_COMPUTE_NUMERIC_VERSION(0, 9, 0)
                                        /* unused */ NULL,
#endif
                                        JXL_COLOR_PROFILE_TARGET_DATA,
                                        iccp_local->data,
                                        iccp_local->data_length) != JXL_DEC_SUCCESS) {
        sail_destroy_iccp(iccp_local);
        SAIL_LOG_ERROR("JPEGXL: Failed to get ICC profile");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    *iccp = iccp_local;

    return SAIL_OK;
}

sail_status_t jpegxl_private_read_more_data(struct sail_io *io, JxlDecoder *decoder, unsigned char *buffer, size_t buffer_size) {

    size_t remaining = JxlDecoderReleaseInput(decoder);

    if (remaining > 0) {
        memmove(buffer, buffer + buffer_size - remaining, remaining);
    }

    size_t bytes_read;
    SAIL_TRY(io->tolerant_read(io->stream, buffer + remaining, buffer_size - remaining, &bytes_read));

    if (bytes_read == 0) {
        JxlDecoderCloseInput(decoder);
        return SAIL_OK;
    }

    if (JxlDecoderSetInput(decoder, buffer, bytes_read + remaining) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to set input buffer");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

sail_status_t jpegxl_private_fetch_special_properties(const JxlBasicInfo *basic_info, struct sail_hash_map *special_properties) {

    struct sail_variant *variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    SAIL_LOG_TRACE("JPEGXL: bits_per_sample(%u)", basic_info->bits_per_sample);
    sail_set_variant_unsigned_int(variant, basic_info->bits_per_sample);
    sail_put_hash_map(special_properties, "jpegxl-bits-per-sample", variant);

    SAIL_LOG_TRACE("JPEGXL: intensity_target(%.1f)", basic_info->intensity_target);
    sail_set_variant_float(variant, basic_info->intensity_target);
    sail_put_hash_map(special_properties, "jpegxl-intensity-target", variant);

    SAIL_LOG_TRACE("JPEGXL: min_nits(%.1f)", basic_info->min_nits);
    sail_set_variant_float(variant, basic_info->min_nits);
    sail_put_hash_map(special_properties, "jpegxl-min-nits", variant);

    SAIL_LOG_TRACE("JPEGXL: relative_to_max_display(%s)", basic_info->relative_to_max_display ? "yes" : "no");
    sail_set_variant_bool(variant, basic_info->relative_to_max_display);
    sail_put_hash_map(special_properties, "jpegxl-relative-to-max-display", variant);

    SAIL_LOG_TRACE("JPEGXL: linear_below(%.1f)", basic_info->linear_below);
    sail_set_variant_float(variant, basic_info->linear_below);
    sail_put_hash_map(special_properties, "jpegxl-linear-below", variant);

    SAIL_LOG_TRACE("JPEGXL: num_color_channels(%u)", basic_info->num_color_channels);
    sail_set_variant_unsigned_int(variant, basic_info->num_color_channels);
    sail_put_hash_map(special_properties, "jpegxl-color-channels", variant);

    SAIL_LOG_TRACE("JPEGXL: num_extra_channels(%u)", basic_info->num_extra_channels);
    sail_set_variant_unsigned_int(variant, basic_info->num_extra_channels);
    sail_put_hash_map(special_properties, "jpegxl-extra-channels", variant);

    SAIL_LOG_TRACE("JPEGXL: alpha_bits(%u)", basic_info->alpha_bits);
    sail_set_variant_unsigned_int(variant, basic_info->alpha_bits);
    sail_put_hash_map(special_properties, "jpegxl-alpha-bits", variant);

    SAIL_LOG_TRACE("JPEGXL: intrinsic_xsize(%u)", basic_info->intrinsic_xsize);
    sail_set_variant_unsigned_int(variant, basic_info->intrinsic_xsize);
    sail_put_hash_map(special_properties, "jpegxl-intrinsic-width", variant);

    SAIL_LOG_TRACE("JPEGXL: intrinsic_ysize(%u)", basic_info->intrinsic_ysize);
    sail_set_variant_unsigned_int(variant, basic_info->intrinsic_ysize);
    sail_put_hash_map(special_properties, "jpegxl-intrinsic-height", variant);

    sail_destroy_variant(variant);

    return SAIL_OK;
}

sail_status_t jpegxl_private_fetch_name(JxlDecoder *decoder, uint32_t name_length, struct sail_meta_data_node **meta_data_node) {

    struct sail_meta_data_node *meta_data_node_local = NULL;

    void *ptr;
    SAIL_TRY(sail_malloc(name_length + 1, &ptr));
    char *name = ptr;

    if (JxlDecoderGetFrameName(decoder, name, name_length + 1) != JXL_DEC_SUCCESS) {
        sail_free(name);
        SAIL_LOG_ERROR("JPEGXL: Failed to get frame name");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                        /* cleanup */ sail_free(name));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(SAIL_META_DATA_NAME, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_free(name), sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node_local->meta_data->value),
                        /* cleanup */ sail_free(name), sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_shallow_string(meta_data_node_local->meta_data->value, name),
                        /* cleanup */ sail_free(name), sail_destroy_meta_data_node(meta_data_node_local));

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t jpegxl_private_fetch_metadata(JxlDecoder *decoder, struct sail_meta_data_node **meta_data_node) {

    JxlBoxType type;
    if (JxlDecoderGetBoxType(decoder, type, JXL_FALSE) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to get box type");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    SAIL_LOG_TRACE("JPEGXL: Box %c%c%c%c", type[0], type[1], type[2], type[3]);

    enum SailMetaData meta_data;

    if (strncmp(type, "Exif", 4) == 0) {
        meta_data = SAIL_META_DATA_EXIF;
    } else if (strncmp(type, "xml ", 4) == 0) {
        meta_data = SAIL_META_DATA_XMP;
    } else if (strncmp(type, "jumb", 4) == 0) {
        meta_data = SAIL_META_DATA_JUMBF;
    } else {
        return SAIL_OK;
    }

    uint64_t size;
    if (JxlDecoderGetBoxSizeRaw(decoder, &size) != JXL_DEC_SUCCESS) {
        SAIL_LOG_ERROR("JPEGXL: Failed to get box size");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    struct sail_meta_data_node *meta_data_node_local = NULL;

    void *data;
    SAIL_TRY(sail_malloc(size, &data));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                        /* cleanup */ sail_free(data));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_from_known_key(meta_data, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_free(data), sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_variant(&meta_data_node_local->meta_data->value),
                        /* cleanup */ sail_free(data), sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_shallow_data(meta_data_node_local->meta_data->value, data, size),
                        /* cleanup */ sail_free(data), sail_destroy_meta_data_node(meta_data_node_local));

    JxlDecoderReleaseBoxBuffer(decoder);
    JxlDecoderSetBoxBuffer(decoder, data, size);

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}
