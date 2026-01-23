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

sail_status_t gif_private_convert_rgba_palette_to_rgb(const struct sail_palette* source_palette,
                                                      struct sail_palette** target_palette,
                                                      int* transparency_index)
{
    SAIL_CHECK_PTR(source_palette);
    SAIL_CHECK_PTR(target_palette);
    SAIL_CHECK_PTR(transparency_index);

    *transparency_index = -1;

    /* Determine channel offsets based on pixel format. */
    int r_offset, g_offset, b_offset, a_offset;
    enum SailPixelFormat output_format;

    switch (source_palette->pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        r_offset      = 0;
        g_offset      = 1;
        b_offset      = 2;
        a_offset      = 3;
        output_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
    {
        b_offset      = 0;
        g_offset      = 1;
        r_offset      = 2;
        a_offset      = 3;
        output_format = SAIL_PIXEL_FORMAT_BPP24_BGR;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ARGB:
    {
        a_offset      = 0;
        r_offset      = 1;
        g_offset      = 2;
        b_offset      = 3;
        output_format = SAIL_PIXEL_FORMAT_BPP24_RGB;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ABGR:
    {
        a_offset      = 0;
        b_offset      = 1;
        g_offset      = 2;
        r_offset      = 3;
        output_format = SAIL_PIXEL_FORMAT_BPP24_BGR;
        break;
    }
    default:
    {
        SAIL_LOG_ERROR("GIF: Cannot convert palette format %s to RGB",
                       sail_pixel_format_to_string(source_palette->pixel_format));
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }
    }

    /* Find first color with alpha < 128 (transparent color). */
    const unsigned char* src_data = (const unsigned char*)source_palette->data;
    for (unsigned i = 0; i < source_palette->color_count; i++)
    {
        if (src_data[i * 4 + a_offset] < 128)
        {
            *transparency_index = (int)i;
            SAIL_LOG_DEBUG("GIF: Found transparent color at index %d (alpha=%d)", i, src_data[i * 4 + a_offset]);
            break;
        }
    }

    /* Allocate RGB palette. */
    SAIL_TRY(sail_alloc_palette_for_data(output_format, source_palette->color_count, target_palette));

    /* Convert RGBA to RGB. */
    unsigned char* dst_data = (unsigned char*)(*target_palette)->data;

    for (unsigned i = 0; i < source_palette->color_count; i++)
    {
        dst_data[i * 3 + 0] = src_data[i * 4 + r_offset];
        dst_data[i * 3 + 1] = src_data[i * 4 + g_offset];
        dst_data[i * 3 + 2] = src_data[i * 4 + b_offset];
    }

    SAIL_LOG_DEBUG("GIF: Converted %s palette to %s. Partial transparency lost, only index %d is transparent",
                   sail_pixel_format_to_string(source_palette->pixel_format),
                   sail_pixel_format_to_string(output_format), *transparency_index);

    return SAIL_OK;
}

sail_status_t gif_private_build_color_map(const struct sail_palette* palette,
                                          ColorMapObject** color_map,
                                          int* auto_transparency_index)
{
    SAIL_CHECK_PTR(palette);
    SAIL_CHECK_PTR(color_map);
    SAIL_CHECK_PTR(auto_transparency_index);

    const struct sail_palette* palette_to_use = palette;
    struct sail_palette* converted_palette    = NULL;

    *auto_transparency_index = -1;

    /* Convert RGBA palettes to RGB automatically. */
    if (palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_RGB && palette->pixel_format != SAIL_PIXEL_FORMAT_BPP24_BGR)
    {
        SAIL_TRY(gif_private_convert_rgba_palette_to_rgb(palette, &converted_palette, auto_transparency_index));
        palette_to_use = converted_palette;
    }

    if (palette_to_use->color_count > 256)
    {
        sail_destroy_palette(converted_palette);
        SAIL_LOG_ERROR("GIF: Palette has %u colors, but GIF supports maximum 256 colors", palette_to_use->color_count);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* GIF requires power-of-2 palette sizes. */
    unsigned color_count_pow2 = 2;
    while (color_count_pow2 < palette_to_use->color_count)
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
        sail_destroy_palette(converted_palette);
        SAIL_LOG_ERROR("GIF: Failed to allocate color map");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Copy colors to the GIF color map. */
    const unsigned char* palette_data = palette_to_use->data;

    int r = (palette_to_use->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) ? 2 : 0;
    int g = 1;
    int b = (palette_to_use->pixel_format == SAIL_PIXEL_FORMAT_BPP24_BGR) ? 0 : 2;

    for (unsigned i = 0; i < palette_to_use->color_count; i++)
    {
        (*color_map)->Colors[i].Red   = palette_data[i * 3 + r];
        (*color_map)->Colors[i].Green = palette_data[i * 3 + g];
        (*color_map)->Colors[i].Blue  = palette_data[i * 3 + b];
    }

    /* Fill remaining colors with black. */
    for (unsigned i = palette_to_use->color_count; i < color_count_pow2; i++)
    {
        (*color_map)->Colors[i].Red   = 0;
        (*color_map)->Colors[i].Green = 0;
        (*color_map)->Colors[i].Blue  = 0;
    }

    sail_destroy_palette(converted_palette);

    return SAIL_OK;
}

bool gif_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct gif_tuning_state* tuning_state = (struct gif_tuning_state*)user_data;

    if (strcmp(key, "gif-transparency-index") == 0)
    {
        int transparency_index = sail_variant_to_int(value);
        /* GIF palette has 256 colors max, -1 for no transparency */
        if (transparency_index >= -1 && transparency_index <= 255)
        {
            *tuning_state->transparency_index_save = transparency_index;
            SAIL_LOG_DEBUG("GIF: Set transparency index to %d", *tuning_state->transparency_index_save);
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-transparency-index' must be in range [-1, 255], got %d", transparency_index);
        }
        return true;
    }
    else if (strcmp(key, "gif-loop-count") == 0)
    {
        int loop_count = sail_variant_to_int(value);
        /* 0 = infinite, max 65535 (uint16 max) */
        if (loop_count >= 0 && loop_count <= 65535)
        {
            *tuning_state->loop_count = loop_count;
            SAIL_LOG_DEBUG("GIF: Set loop count to %d", *tuning_state->loop_count);
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-loop-count' must be in range [0, 65535], got %d", loop_count);
        }
        return true;
    }
    else if (strcmp(key, "gif-background-color") == 0)
    {
        int background_color_index = sail_variant_to_int(value);
        /* GIF palette has 256 colors max */
        if (background_color_index >= 0 && background_color_index <= 255)
        {
            *tuning_state->background_color_index = background_color_index;
            SAIL_LOG_DEBUG("GIF: Set background color index to %d", *tuning_state->background_color_index);
        }
        else
        {
            SAIL_LOG_ERROR("GIF: 'gif-background-color' must be in range [0, 255], got %d", background_color_index);
        }
        return true;
    }

    return false;
}
