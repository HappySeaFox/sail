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

#include <stdbool.h>
#include <string.h>

#include <gif_lib.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

static sail_status_t save_str_in_meta_data(const char* str,
                                           unsigned length_wo_null,
                                           enum SailMetaData key,
                                           struct sail_meta_data_node** meta_data_node)
{
    SAIL_CHECK_PTR(str);
    SAIL_CHECK_PTR(meta_data_node);

    struct sail_meta_data_node* meta_data_node_local;

    SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node_local));

    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(key, &meta_data_node_local->meta_data),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));
    SAIL_TRY_OR_CLEANUP(sail_set_variant_substring(meta_data_node_local->meta_data->value, str, length_wo_null),
                        /* cleanup */ sail_destroy_meta_data_node(meta_data_node_local));

    *meta_data_node = meta_data_node_local;

    return SAIL_OK;
}

sail_status_t gif_private_fetch_comment(const GifByteType* extension, struct sail_meta_data_node** meta_data_node)
{
    SAIL_CHECK_PTR(extension);

    const unsigned length = *(unsigned char*)(extension);

    if (length == 0)
    {
        return SAIL_OK;
    }

    SAIL_TRY(save_str_in_meta_data((const char*)extension + 1, length, SAIL_META_DATA_COMMENT, meta_data_node));

    return SAIL_OK;
}

sail_status_t gif_private_fetch_application(const GifByteType* extension, struct sail_meta_data_node** meta_data_node)
{
    SAIL_CHECK_PTR(extension);

    /* 8 bytes as per the spec. */
    SAIL_TRY(save_str_in_meta_data((const char*)extension + 1, 8, SAIL_META_DATA_SOFTWARE, meta_data_node));

    return SAIL_OK;
}

sail_status_t gif_private_pixel_format_to_bpp(enum SailPixelFormat pixel_format, int* bpp)
{
    SAIL_CHECK_PTR(bpp);

    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
    {
        *bpp = 1;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
    {
        *bpp = 2;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
    {
        *bpp = 4;
        return SAIL_OK;
    }
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
    {
        *bpp = 8;
        return SAIL_OK;
    }
    default:
    {
        SAIL_LOG_ERROR("GIF: Unsupported pixel format %s for saving", sail_pixel_format_to_string(pixel_format));
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }
    }
}

sail_status_t gif_private_build_color_map(const struct sail_palette* palette, ColorMapObject** color_map)
{
    SAIL_CHECK_PTR(palette);
    SAIL_CHECK_PTR(color_map);

    if (palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB && palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_BGR)
    {
        SAIL_LOG_ERROR("GIF: Unsupported palette pixel format %s. Only BPP24-RGB and BPP24-BGR are supported",
                       sail_pixel_format_to_string(palette->pixel_format));
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    if (palette->color_count > 256)
    {
        SAIL_LOG_ERROR("GIF: Palette has %u colors, but GIF supports maximum 256 colors", palette->color_count);
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    /* GIF requires power-of-2 palette sizes. */
    int color_count_pow2 = 2;
    while (color_count_pow2 < (int)palette->color_count)
    {
        color_count_pow2 *= 2;
    }

    if (color_count_pow2 > 256)
    {
        color_count_pow2 = 256;
    }

    *color_map = GifMakeMapObject(color_count_pow2, NULL);

    if (*color_map == NULL)
    {
        SAIL_LOG_ERROR("GIF: Failed to allocate color map");
        return SAIL_ERROR_MEMORY_ALLOCATION;
    }

    /* Copy colors to the GIF color map. */
    const unsigned char* palette_data = palette->data;
    const bool is_bgr                 = (palette->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR);

    for (unsigned i = 0; i < palette->color_count; i++)
    {
        if (is_bgr)
        {
            (*color_map)->Colors[i].Blue  = palette_data[i * 3 + 0];
            (*color_map)->Colors[i].Green = palette_data[i * 3 + 1];
            (*color_map)->Colors[i].Red   = palette_data[i * 3 + 2];
        }
        else
        {
            (*color_map)->Colors[i].Red   = palette_data[i * 3 + 0];
            (*color_map)->Colors[i].Green = palette_data[i * 3 + 1];
            (*color_map)->Colors[i].Blue  = palette_data[i * 3 + 2];
        }
    }

    /* Fill remaining colors with black. */
    for (unsigned i = palette->color_count; i < (unsigned)color_count_pow2; i++)
    {
        (*color_map)->Colors[i].Red   = 0;
        (*color_map)->Colors[i].Green = 0;
        (*color_map)->Colors[i].Blue  = 0;
    }

    return SAIL_OK;
}

bool gif_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct gif_tuning_state* tuning_state = (struct gif_tuning_state*)user_data;

    if (strcmp(key, "gif-transparency-index") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int transparency_index = (value->type == SAIL_VARIANT_TYPE_INT) ? sail_variant_to_int(value)
                                                                            : (int)sail_variant_to_unsigned_int(value);
            /* GIF palette has 256 colors max, -1 for no transparency */
            if (transparency_index >= -1 && transparency_index <= 255)
            {
                *tuning_state->transparency_index_save = transparency_index;
                SAIL_LOG_DEBUG("GIF: Set transparency index to %d", *tuning_state->transparency_index_save);
            }
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-transparency-index' must be an integer");
        }
        return true;
    }
    else if (strcmp(key, "gif-loop-count") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int loop_count = (value->type == SAIL_VARIANT_TYPE_INT) ? sail_variant_to_int(value)
                                                                    : (int)sail_variant_to_unsigned_int(value);
            /* 0 = infinite, max 65535 (uint16 max) */
            if (loop_count >= 0 && loop_count <= 65535)
            {
                *tuning_state->loop_count = loop_count;
                SAIL_LOG_DEBUG("GIF: Set loop count to %d", *tuning_state->loop_count);
            }
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-loop-count' must be an integer");
        }
        return true;
    }
    else if (strcmp(key, "gif-background-color") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            int background_color_index = (value->type == SAIL_VARIANT_TYPE_INT)
                                             ? sail_variant_to_int(value)
                                             : (int)sail_variant_to_unsigned_int(value);
            /* GIF palette has 256 colors max */
            if (background_color_index >= 0 && background_color_index <= 255)
            {
                *tuning_state->background_color_index = background_color_index;
                SAIL_LOG_DEBUG("GIF: Set background color index to %d", *tuning_state->background_color_index);
            }
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-background-color' must be an integer");
        }
        return true;
    }

    return false;
}
