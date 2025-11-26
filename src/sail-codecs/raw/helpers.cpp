/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <cstdio>
#include <cstring>
#include <ctime>

#include <libraw/libraw.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

enum SailPixelFormat raw_private_libraw_to_pixel_format(unsigned colors, unsigned bits)
{
    if (colors == 1)
    {
        switch (bits)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
    else if (colors == 3)
    {
        switch (bits)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
    else if (colors == 4)
    {
        switch (bits)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case 16: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

static sail_status_t add_string_meta_data(enum SailMetaData key,
                                          const char* value,
                                          struct sail_meta_data_node*** last_meta_data_node)
{
    if (value == NULL || value[0] == '\0')
    {
        return SAIL_OK;
    }

    struct sail_meta_data_node* meta_data_node;
    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(key, &meta_data_node->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_string(meta_data_node->meta_data->value, value),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    **last_meta_data_node = meta_data_node;
    *last_meta_data_node  = &meta_data_node->next;

    return SAIL_OK;
}

static sail_status_t add_time_meta_data(time_t timestamp, struct sail_meta_data_node*** last_meta_data_node)
{
    if (timestamp == 0)
    {
        return SAIL_OK;
    }

    struct tm* time_info = gmtime(&timestamp);

    if (time_info == NULL)
    {
        return SAIL_OK;
    }

    char time_string[32];
    strftime(time_string, sizeof(time_string), "%Y:%m:%d %H:%M:%S", time_info);

    return add_string_meta_data(SAIL_META_DATA_CREATION_TIME, time_string, last_meta_data_node);
}

static sail_status_t add_binary_meta_data(enum SailMetaData key,
                                          const void* data,
                                          size_t size,
                                          struct sail_meta_data_node*** last_meta_data_node)
{
    if (data == NULL || size == 0)
    {
        return SAIL_OK;
    }

    struct sail_meta_data_node* meta_data_node;
    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(key, &meta_data_node->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

    void* data_copy = NULL;
    SAIL_TRY_OR_CLEANUP(sail_malloc(size, &data_copy),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node));
    memcpy(data_copy, data, size);

    SAIL_TRY_OR_CLEANUP(sail_set_variant_shallow_data(meta_data_node->meta_data->value, data_copy, size),
                        /* cleanup */ sail_free(data_copy), sail_destroy_meta_data_node(meta_data_node));

    **last_meta_data_node = meta_data_node;
    *last_meta_data_node  = &meta_data_node->next;

    return SAIL_OK;
}

sail_status_t raw_private_fetch_meta_data(libraw_data_t* raw_data,
                                          struct sail_meta_data_node** meta_data_node,
                                          const std::vector<unsigned char>& exif_data)
{
    SAIL_CHECK_PTR(raw_data);
    SAIL_CHECK_PTR(meta_data_node);

    struct sail_meta_data_node** last_meta_data_node = meta_data_node;

    SAIL_TRY(add_string_meta_data(SAIL_META_DATA_ARTIST, raw_data->other.artist, &last_meta_data_node));

    SAIL_TRY(add_string_meta_data(SAIL_META_DATA_DESCRIPTION, raw_data->other.desc, &last_meta_data_node));

    SAIL_TRY(add_string_meta_data(SAIL_META_DATA_MAKE, raw_data->idata.make, &last_meta_data_node));

    SAIL_TRY(add_string_meta_data(SAIL_META_DATA_MODEL, raw_data->idata.model, &last_meta_data_node));

    SAIL_TRY(add_string_meta_data(SAIL_META_DATA_SOFTWARE, raw_data->idata.software, &last_meta_data_node));

    SAIL_TRY(add_time_meta_data(raw_data->other.timestamp, &last_meta_data_node));

    if (raw_data->idata.xmplen > 0 && raw_data->idata.xmpdata != NULL)
    {
        SAIL_TRY(add_binary_meta_data(SAIL_META_DATA_XMP, raw_data->idata.xmpdata, raw_data->idata.xmplen,
                                      &last_meta_data_node));
    }

    if (!exif_data.empty())
    {
        SAIL_TRY(add_binary_meta_data(SAIL_META_DATA_EXIF, exif_data.data(), exif_data.size(), &last_meta_data_node));
    }

    return SAIL_OK;
}

static sail_status_t store_float_property(struct sail_hash_map* special_properties, const char* key, float value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_float(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_unsigned_int_property(struct sail_hash_map* special_properties,
                                                 const char* key,
                                                 unsigned value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_unsigned_int(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_int_property(struct sail_hash_map* special_properties, const char* key, int value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_int(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_ushort_property(struct sail_hash_map* special_properties,
                                           const char* key,
                                           unsigned short value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_unsigned_short(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_bool_property(struct sail_hash_map* special_properties, const char* key, bool value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_bool(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_string_property(struct sail_hash_map* special_properties, const char* key, const char* value)
{
    if (special_properties == NULL || value == NULL || value[0] == '\0')
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    sail_set_variant_string(variant, value);
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

static sail_status_t store_unsigned_long_long_property(struct sail_hash_map* special_properties,
                                                       const char* key,
                                                       unsigned long long value)
{
    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    // Use unsigned long as closest match for unsigned long long.
    sail_set_variant_unsigned_long(variant, static_cast<unsigned long>(value));
    SAIL_TRY_OR_CLEANUP(sail_put_hash_map(special_properties, key, variant),
                        /* cleanup */ sail_destroy_variant(variant));

    sail_destroy_variant(variant);

    return SAIL_OK;
}

sail_status_t raw_private_store_special_properties(libraw_data_t* raw_data, struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(raw_data);

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    if (raw_data->other.iso_speed > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-iso", raw_data->other.iso_speed));
    }
    if (raw_data->other.shutter > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-shutter", raw_data->other.shutter));
    }
    if (raw_data->other.aperture > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-aperture", raw_data->other.aperture));
    }
    if (raw_data->other.focal_len > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-focal-length", raw_data->other.focal_len));
    }

    // Temperatures are not directly available in libraw_imgother_t.
    // They may be available through other structures, but for now we skip them.

    // Lens parameters from libraw_lensinfo_t.
    if (raw_data->lens.makernotes.LensID != 0)
    {
        SAIL_TRY(
            store_unsigned_long_long_property(special_properties, "raw-lens-id", raw_data->lens.makernotes.LensID));
    }
    SAIL_TRY(store_string_property(special_properties, "raw-lens", raw_data->lens.Lens));
    if (raw_data->lens.MinFocal > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-min-focal-length", raw_data->lens.MinFocal));
    }
    if (raw_data->lens.MaxFocal > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-max-focal-length", raw_data->lens.MaxFocal));
    }
    if (raw_data->lens.MaxAp4MinFocal > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-max-aperture-min-focal", raw_data->lens.MaxAp4MinFocal));
    }
    if (raw_data->lens.MaxAp4MaxFocal > 0.0f)
    {
        SAIL_TRY(store_float_property(special_properties, "raw-max-aperture-max-focal", raw_data->lens.MaxAp4MaxFocal));
    }
    if (raw_data->lens.FocalLengthIn35mmFormat > 0)
    {
        SAIL_TRY(store_ushort_property(special_properties, "raw-focal-length-in-35mm-format",
                                       raw_data->lens.FocalLengthIn35mmFormat));
    }

    SAIL_TRY(store_unsigned_int_property(special_properties, "raw-filters", raw_data->idata.filters));
    SAIL_TRY(store_int_property(special_properties, "raw-colors", raw_data->idata.colors));
    SAIL_TRY(store_ushort_property(special_properties, "raw-width", raw_data->sizes.raw_width));
    SAIL_TRY(store_ushort_property(special_properties, "raw-height", raw_data->sizes.raw_height));
    SAIL_TRY(store_ushort_property(special_properties, "raw-top-margin", raw_data->sizes.top_margin));
    SAIL_TRY(store_ushort_property(special_properties, "raw-left-margin", raw_data->sizes.left_margin));
    SAIL_TRY(store_bool_property(special_properties, "raw-is-foveon", raw_data->idata.is_foveon != 0));

    return SAIL_OK;
}

static float raw_private_read_variant_float(const struct sail_variant* value)
{
    if (value->type == SAIL_VARIANT_TYPE_DOUBLE)
    {
        return static_cast<float>(sail_variant_to_double(value));
    }
    else
    {
        return sail_variant_to_float(value);
    }
}

bool raw_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    LibRaw* raw_processor = static_cast<LibRaw*>(user_data);

    if (strcmp(key, "raw-brightness") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            float brightness                     = raw_private_read_variant_float(value);
            raw_processor->imgdata.params.bright = brightness;
            SAIL_LOG_TRACE("RAW: brightness=%f", brightness);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-brightness' must be a float or double");
        }
    }
    else if (strcmp(key, "raw-highlight") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int highlight = (value->type == SAIL_VARIANT_TYPE_INT)
                                ? sail_variant_to_int(value)
                                : static_cast<int>(sail_variant_to_unsigned_int(value));
            if (highlight >= 0 && highlight <= 9)
            {
                raw_processor->imgdata.params.highlight = highlight;
                SAIL_LOG_TRACE("RAW: highlight=%d", highlight);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-highlight' must be between 0 and 9");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-highlight' must be an integer");
        }
    }
    else if (strcmp(key, "raw-output-color") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int output_color      = -1;

            if (strcmp(str_value, "srgb") == 0)
            {
                output_color = 0;
            }
            else if (strcmp(str_value, "adobe-rgb") == 0)
            {
                output_color = 1;
            }
            else if (strcmp(str_value, "wide-gamut-rgb") == 0)
            {
                output_color = 2;
            }
            else if (strcmp(str_value, "prophoto-rgb") == 0)
            {
                output_color = 3;
            }
            else if (strcmp(str_value, "xyz") == 0)
            {
                output_color = 4;
            }
            else if (strcmp(str_value, "aces") == 0)
            {
                output_color = 5;
            }
            else if (strcmp(str_value, "rec2020") == 0)
            {
                output_color = 6;
            }

            if (output_color >= 0)
            {
                raw_processor->imgdata.params.output_color = output_color;
                SAIL_LOG_TRACE("RAW: output-color=%s (%d)", str_value, output_color);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-output-color' must be one of: srgb, adobe-rgb, wide-gamut-rgb, prophoto-rgb, "
                               "xyz, aces, rec2020");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-output-color' must be a string");
        }
    }
    else if (strcmp(key, "raw-output-bits-per-sample") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int output_bps = (value->type == SAIL_VARIANT_TYPE_INT)
                                 ? sail_variant_to_int(value)
                                 : static_cast<int>(sail_variant_to_unsigned_int(value));
            if (output_bps == 8 || output_bps == 16)
            {
                raw_processor->imgdata.params.output_bps = output_bps;
                SAIL_LOG_TRACE("RAW: output-bits-per-sample=%d", output_bps);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-output-bits-per-sample' must be 8 or 16");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-output-bits-per-sample' must be an integer");
        }
    }
    else if (strcmp(key, "raw-demosaic") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            int demosaic          = -1;

            if (strcmp(str_value, "linear") == 0)
            {
                demosaic = 0;
            }
            else if (strcmp(str_value, "vng") == 0)
            {
                demosaic = 1;
            }
            else if (strcmp(str_value, "ppg") == 0)
            {
                demosaic = 2;
            }
            else if (strcmp(str_value, "ahd") == 0)
            {
                demosaic = 3;
            }
            else if (strcmp(str_value, "dcb") == 0)
            {
                demosaic = 4;
            }
            else if (strcmp(str_value, "dht") == 0)
            {
                demosaic = 5;
            }
            else if (strcmp(str_value, "aahd") == 0)
            {
                demosaic = 6;
            }

            if (demosaic >= 0)
            {
                raw_processor->imgdata.params.user_qual = demosaic;
                SAIL_LOG_TRACE("RAW: demosaic=%s (%d)", str_value, demosaic);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-demosaic' must be one of: linear, vng, ppg, ahd, dcb, dht, aahd");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-demosaic' must be a string");
        }
    }
    else if (strcmp(key, "raw-four-color-rgb") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.four_color_rgb = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: four-color-rgb=%d", raw_processor->imgdata.params.four_color_rgb);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-four-color-rgb' must be a bool");
        }
    }
    else if (strcmp(key, "raw-dcb-iterations") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int dcb_iterations = (value->type == SAIL_VARIANT_TYPE_INT)
                                     ? sail_variant_to_int(value)
                                     : static_cast<int>(sail_variant_to_unsigned_int(value));
            if (dcb_iterations >= 0 && dcb_iterations <= 100)
            {
                raw_processor->imgdata.params.dcb_iterations = dcb_iterations;
                SAIL_LOG_TRACE("RAW: dcb-iterations=%d", dcb_iterations);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-dcb-iterations' must be between 0 and 100");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-dcb-iterations' must be an integer");
        }
    }
    else if (strcmp(key, "raw-dcb-enhance-focal-length") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int dcb_enhance_fl = (value->type == SAIL_VARIANT_TYPE_INT)
                                     ? sail_variant_to_int(value)
                                     : static_cast<int>(sail_variant_to_unsigned_int(value));
            if (dcb_enhance_fl >= 0 && dcb_enhance_fl <= 100)
            {
                raw_processor->imgdata.params.dcb_enhance_fl = dcb_enhance_fl;
                SAIL_LOG_TRACE("RAW: dcb-enhance-focal-length=%d", dcb_enhance_fl);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-dcb-enhance-focal-length' must be between 0 and 100");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-dcb-enhance-focal-length' must be an integer");
        }
    }
    else if (strcmp(key, "raw-use-camera-white-balance") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.use_camera_wb = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: use-camera-white-balance=%d", raw_processor->imgdata.params.use_camera_wb);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-use-camera-white-balance' must be a bool");
        }
    }
    else if (strcmp(key, "raw-use-auto-white-balance") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.use_auto_wb = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: use-auto-white-balance=%d", raw_processor->imgdata.params.use_auto_wb);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-use-auto-white-balance' must be a bool");
        }
    }
    else if (strcmp(key, "raw-user-multiplier") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            float user_mul[4]     = {1.0f, 1.0f, 1.0f, 1.0f};
            int parsed = sscanf(str_value, "%f %f %f %f", &user_mul[0], &user_mul[1], &user_mul[2], &user_mul[3]);

            if (parsed == 4)
            {
                raw_processor->imgdata.params.user_mul[0] = user_mul[0];
                raw_processor->imgdata.params.user_mul[1] = user_mul[1];
                raw_processor->imgdata.params.user_mul[2] = user_mul[2];
                raw_processor->imgdata.params.user_mul[3] = user_mul[3];
                SAIL_LOG_TRACE("RAW: user-multiplier=%f %f %f %f", user_mul[0], user_mul[1], user_mul[2], user_mul[3]);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-user-multiplier' must be a string with 4 float values: 'r g1 b g2'");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-user-multiplier' must be a string");
        }
    }
    else if (strcmp(key, "raw-auto-brightness") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.no_auto_bright = sail_variant_to_bool(value) ? 0 : 1;
            SAIL_LOG_TRACE("RAW: auto-brightness=%d", raw_processor->imgdata.params.no_auto_bright == 0 ? 1 : 0);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-auto-brightness' must be a bool");
        }
    }
    else if (strcmp(key, "raw-half-size") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.half_size = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: half-size=%d", raw_processor->imgdata.params.half_size);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-half-size' must be a bool");
        }
    }
    else if (strcmp(key, "raw-use-fuji-rotate") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.use_fuji_rotate = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: use-fuji-rotate=%d", raw_processor->imgdata.params.use_fuji_rotate);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-use-fuji-rotate' must be a bool");
        }
    }
    else if (strcmp(key, "raw-no-interpolation") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_BOOL)
        {
            raw_processor->imgdata.params.no_interpolation = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("RAW: no-interpolation=%d", raw_processor->imgdata.params.no_interpolation);
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-no-interpolation' must be a bool");
        }
    }
    else if (strcmp(key, "raw-median-passes") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int med_passes = (value->type == SAIL_VARIANT_TYPE_INT)
                                 ? sail_variant_to_int(value)
                                 : static_cast<int>(sail_variant_to_unsigned_int(value));
            if (med_passes >= 0 && med_passes <= 100)
            {
                raw_processor->imgdata.params.med_passes = med_passes;
                SAIL_LOG_TRACE("RAW: median-passes=%d", med_passes);
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-median-passes' must be between 0 and 100");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-median-passes' must be an integer");
        }
    }
    else if (strcmp(key, "raw-gamma") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "power") == 0)
            {
                raw_processor->imgdata.params.gamm[0] = 1.0;
                raw_processor->imgdata.params.gamm[1] = 0.0;
                raw_processor->imgdata.params.gamm[2] = 0.0;
                raw_processor->imgdata.params.gamm[3] = 0.0;
                raw_processor->imgdata.params.gamm[4] = 0.0;
                raw_processor->imgdata.params.gamm[5] = 0.0;
                SAIL_LOG_TRACE("RAW: gamma=power");
            }
            else if (strcmp(str_value, "bt709") == 0)
            {
                raw_processor->imgdata.params.gamm[0] = 0.0;
                raw_processor->imgdata.params.gamm[1] = 0.0;
                raw_processor->imgdata.params.gamm[2] = 0.0;
                raw_processor->imgdata.params.gamm[3] = 0.0;
                raw_processor->imgdata.params.gamm[4] = 0.0;
                raw_processor->imgdata.params.gamm[5] = 0.0;
                SAIL_LOG_TRACE("RAW: gamma=bt709");
            }
            else
            {
                SAIL_LOG_ERROR("RAW: 'raw-gamma' must be one of: power, bt709");
            }
        }
        else
        {
            SAIL_LOG_ERROR("RAW: 'raw-gamma' must be a string");
        }
    }

    return true;
}
