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

#include <jxl/encode.h>
#include <jxl/version.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

SAIL_HIDDEN bool jpegxl_private_is_cmyk(JxlDecoder *decoder, uint32_t num_extra_channels) {

    for (uint32_t i = 0; i < num_extra_channels; i++) {
        JxlExtraChannelInfo extra_channel_info;

        if (JxlDecoderGetExtraChannelInfo(decoder, i, &extra_channel_info) != JXL_DEC_SUCCESS) {
            return false;
        }

        if (extra_channel_info.type == JXL_CHANNEL_BLACK) {
            return true;
        }
    }

    return false;
}

enum SailPixelFormat jpegxl_private_source_pixel_format_cmyk(uint32_t bits_per_sample, uint32_t alpha_bits) {

    SAIL_LOG_TRACE("JPEGXL: CMYK bits per sample(%u), alpha bits(%u)", bits_per_sample, alpha_bits);

    switch (bits_per_sample) {
        case 8:  return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP40_CMYKA : SAIL_PIXEL_FORMAT_BPP32_CMYK;
        case 16: return alpha_bits > 0 ? SAIL_PIXEL_FORMAT_BPP80_CMYKA : SAIL_PIXEL_FORMAT_BPP64_CMYK;

        default: {
            return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
}

enum SailPixelFormat jpegxl_private_source_pixel_format(uint32_t bits_per_sample, uint32_t num_color_channels, uint32_t alpha_bits) {

    SAIL_LOG_TRACE("JPEGXL: Bits per sample(%u), number of channels(%u), alpha bits(%u)",
        bits_per_sample, num_color_channels, alpha_bits);

    /*
     * Also update jpegxl_private_pixel_format_to_num_channels() with new pixel formats.
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

enum SailPixelFormat jpegxl_private_source_pixel_format_to_output(enum SailPixelFormat pixel_format) {

    switch(pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP32_CMYK: return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case SAIL_PIXEL_FORMAT_BPP64_CMYK: return SAIL_PIXEL_FORMAT_BPP48_RGB;

        case SAIL_PIXEL_FORMAT_BPP40_CMYKA: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case SAIL_PIXEL_FORMAT_BPP80_CMYKA: return SAIL_PIXEL_FORMAT_BPP64_RGBA;

        default: {
            return pixel_format;
        }
    }
}

unsigned jpegxl_private_pixel_format_to_num_channels(enum SailPixelFormat pixel_format) {

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

JxlDataType jpegxl_private_pixel_format_to_jxl_data_type(enum SailPixelFormat pixel_format) {

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
                                        iccp_local->size) != JXL_DEC_SUCCESS) {
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

    SAIL_LOG_TRACE("JPEGXL: exponent_bits_per_sample(%u)", basic_info->exponent_bits_per_sample);
    sail_set_variant_unsigned_int(variant, basic_info->exponent_bits_per_sample);
    sail_put_hash_map(special_properties, "jpegxl-exponent-bits-per-sample", variant);

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

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_NAME, &meta_data_node_local->meta_data),
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

    void *data;
    SAIL_TRY(sail_malloc(size, &data));

    struct sail_meta_data_node *meta_data_node_local;
    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                        /* cleanup */ sail_free(data));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(meta_data, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_free(data), sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_shallow_data(meta_data_node_local->meta_data->value, data, size),
                        /* cleanup */ sail_free(data), sail_destroy_meta_data_node(meta_data_node_local));

    JxlDecoderReleaseBoxBuffer(decoder);
    JxlDecoderSetBoxBuffer(decoder, data, size);

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t jpegxl_private_write_output(JxlEncoder *encoder, struct sail_io *io, unsigned char *buffer, size_t buffer_size) {

    uint8_t *next_out = buffer;
    size_t avail_out = buffer_size;

    JxlEncoderStatus status;

    do {
        status = JxlEncoderProcessOutput(encoder, &next_out, &avail_out);

        if (status == JXL_ENC_NEED_MORE_OUTPUT) {
            /* Write the filled buffer. */
            size_t bytes_written = buffer_size - avail_out;
            SAIL_TRY(io->strict_write(io->stream, buffer, bytes_written));

            next_out = buffer;
            avail_out = buffer_size;
        } else if (status == JXL_ENC_SUCCESS) {
            /* Write any remaining data. */
            size_t bytes_written = buffer_size - avail_out;
            if (bytes_written > 0) {
                SAIL_TRY(io->strict_write(io->stream, buffer, bytes_written));
            }
            return SAIL_OK;
        } else if (status == JXL_ENC_ERROR) {
            JxlEncoderError error = JxlEncoderGetError(encoder);
            SAIL_LOG_ERROR("JPEGXL: Encoder error %d", error);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        } else {
            SAIL_LOG_ERROR("JPEGXL: Unexpected encoder status %d", status);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
    } while (true);
}

sail_status_t jpegxl_private_pixel_format_to_jxl_basic_info(enum SailPixelFormat pixel_format,
                                                              JxlBasicInfo *basic_info,
                                                              JxlPixelFormat *jxl_pixel_format) {

    JxlEncoderInitBasicInfo(basic_info);

    jxl_pixel_format->endianness = JXL_NATIVE_ENDIAN;
    jxl_pixel_format->align = 0;

    switch(pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: {
            basic_info->bits_per_sample = 8;
            basic_info->num_color_channels = 1;
            basic_info->alpha_bits = 0;
            jxl_pixel_format->num_channels = 1;
            jxl_pixel_format->data_type = JXL_TYPE_UINT8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: {
            basic_info->bits_per_sample = 16;
            basic_info->num_color_channels = 1;
            basic_info->alpha_bits = 0;
            jxl_pixel_format->num_channels = 1;
            jxl_pixel_format->data_type = JXL_TYPE_UINT16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: {
            basic_info->bits_per_sample = 8;
            basic_info->num_color_channels = 1;
            basic_info->alpha_bits = 8;
            basic_info->num_extra_channels = 1;
            jxl_pixel_format->num_channels = 2;
            jxl_pixel_format->data_type = JXL_TYPE_UINT8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
            basic_info->bits_per_sample = 16;
            basic_info->num_color_channels = 1;
            basic_info->alpha_bits = 16;
            basic_info->num_extra_channels = 1;
            jxl_pixel_format->num_channels = 2;
            jxl_pixel_format->data_type = JXL_TYPE_UINT16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            basic_info->bits_per_sample = 8;
            basic_info->num_color_channels = 3;
            basic_info->alpha_bits = 0;
            jxl_pixel_format->num_channels = 3;
            jxl_pixel_format->data_type = JXL_TYPE_UINT8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP48_RGB: {
            basic_info->bits_per_sample = 16;
            basic_info->num_color_channels = 3;
            basic_info->alpha_bits = 0;
            jxl_pixel_format->num_channels = 3;
            jxl_pixel_format->data_type = JXL_TYPE_UINT16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            basic_info->bits_per_sample = 8;
            basic_info->num_color_channels = 3;
            basic_info->alpha_bits = 8;
            basic_info->num_extra_channels = 1;
            jxl_pixel_format->num_channels = 4;
            jxl_pixel_format->data_type = JXL_TYPE_UINT8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: {
            basic_info->bits_per_sample = 16;
            basic_info->num_color_channels = 3;
            basic_info->alpha_bits = 16;
            basic_info->num_extra_channels = 1;
            jxl_pixel_format->num_channels = 4;
            jxl_pixel_format->data_type = JXL_TYPE_UINT16;
            return SAIL_OK;
        }
        default: {
            SAIL_LOG_ERROR("JPEGXL: %s pixel format is not supported for saving", sail_pixel_format_to_string(pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }
}

bool jpegxl_private_encoder_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    JxlEncoderFrameSettings *frame_settings = user_data;

    if (strcmp(key, "jpegxl-effort") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t effort = sail_variant_to_int(value);
            if (effort >= 1 && effort <= 9) {
                SAIL_LOG_TRACE("JPEGXL: Setting effort to %lld", (long long)effort);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_EFFORT, effort);
            }
        }
    } else if (strcmp(key, "jpegxl-decoding-speed") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t speed = sail_variant_to_int(value);
            if (speed >= 0 && speed <= 4) {
                SAIL_LOG_TRACE("JPEGXL: Setting decoding speed to %lld", (long long)speed);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_DECODING_SPEED, speed);
            }
        }
    } else if (strcmp(key, "jpegxl-modular") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t modular = sail_variant_to_int(value);
            if (modular >= -1 && modular <= 1) {
                SAIL_LOG_TRACE("JPEGXL: Setting modular to %lld", (long long)modular);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_MODULAR, modular);
            }
        }
    } else if (strcmp(key, "jpegxl-progressive-ac") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t progressive = sail_variant_to_int(value);
            if (progressive >= -1 && progressive <= 1) {
                SAIL_LOG_TRACE("JPEGXL: Setting progressive-ac to %lld", (long long)progressive);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_PROGRESSIVE_AC, progressive);
            }
        }
    } else if (strcmp(key, "jpegxl-progressive-dc") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t progressive = sail_variant_to_int(value);
            if (progressive >= -1 && progressive <= 2) {
                SAIL_LOG_TRACE("JPEGXL: Setting progressive-dc to %lld", (long long)progressive);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_PROGRESSIVE_DC, progressive);
            }
        }
    } else if (strcmp(key, "jpegxl-responsive") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t responsive = sail_variant_to_int(value);
            if (responsive >= -1 && responsive <= 1) {
                SAIL_LOG_TRACE("JPEGXL: Setting responsive to %lld", (long long)responsive);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_RESPONSIVE, responsive);
            }
        }
    } else if (strcmp(key, "jpegxl-epf") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t epf = sail_variant_to_int(value);
            if (epf >= -1 && epf <= 3) {
                SAIL_LOG_TRACE("JPEGXL: Setting EPF to %lld", (long long)epf);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_EPF, epf);
            }
        }
    } else if (strcmp(key, "jpegxl-gaborish") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t gaborish = sail_variant_to_int(value);
            if (gaborish >= -1 && gaborish <= 1) {
                SAIL_LOG_TRACE("JPEGXL: Setting gaborish to %lld", (long long)gaborish);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_GABORISH, gaborish);
            }
        }
    } else if (strcmp(key, "jpegxl-photon-noise") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t noise = sail_variant_to_int(value);
            if (noise >= 0) {
                SAIL_LOG_TRACE("JPEGXL: Setting photon noise to %lld", (long long)noise);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_PHOTON_NOISE, noise);
            }
        }
    } else if (strcmp(key, "jpegxl-modular-predictor") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t predictor = sail_variant_to_int(value);
            if (predictor >= -1 && predictor <= 15) {
                SAIL_LOG_TRACE("JPEGXL: Setting modular predictor to %lld", (long long)predictor);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_MODULAR_PREDICTOR, predictor);
            }
        }
    } else if (strcmp(key, "jpegxl-palette-colors") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t colors = sail_variant_to_int(value);
            SAIL_LOG_TRACE("JPEGXL: Setting palette colors to %lld", (long long)colors);
            JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_PALETTE_COLORS, colors);
        }
    } else if (strcmp(key, "jpegxl-resampling") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT) {
            int64_t resampling = sail_variant_to_int(value);
            if (resampling >= -1 && (resampling == -1 || resampling == 1 || resampling == 2 || resampling == 4 || resampling == 8)) {
                SAIL_LOG_TRACE("JPEGXL: Setting resampling to %lld", (long long)resampling);
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_RESAMPLING, resampling);
            }
        }
    }

    return true;
}

bool jpegxl_private_decoder_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    JxlDecoder *decoder = user_data;

    if (strcmp(key, "jpegxl-desired-intensity-target") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_FLOAT) {
            float intensity = sail_variant_to_float(value);
            SAIL_LOG_TRACE("JPEGXL: Setting desired intensity target to %.1f", intensity);
            JxlDecoderSetDesiredIntensityTarget(decoder, intensity);
        }
    } else if (strcmp(key, "jpegxl-render-spotcolors") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_BOOL) {
            bool render = sail_variant_to_bool(value);
            SAIL_LOG_TRACE("JPEGXL: Setting render spotcolors to %s", render ? "true" : "false");
            JxlDecoderSetRenderSpotcolors(decoder, render ? JXL_TRUE : JXL_FALSE);
        }
    }

    return true;
}
