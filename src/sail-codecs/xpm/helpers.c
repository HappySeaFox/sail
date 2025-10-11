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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/* Standard XPM3 character set for color symbols. */
static const char XPM_CHARS[] =
    " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'[]{}|";

static sail_status_t parse_color_value(const char* str, unsigned char* r, unsigned char* g, unsigned char* b)
{
    /* Parse #RRGGBB format. */
    if (str[0] == '#')
    {
        unsigned long color = strtoul(str + 1, NULL, 16);
        int len             = strlen(str + 1);

        if (len == 6)
        {
            *r = (color >> 16) & 0xFF;
            *g = (color >> 8) & 0xFF;
            *b = color & 0xFF;
        }
        else if (len == 12)
        {
            /* #RRRRGGGGBBBB format - use high bytes. */
            *r = (color >> 40) & 0xFF;
            *g = (color >> 24) & 0xFF;
            *b = (color >> 8) & 0xFF;
        }
        else if (len == 3)
        {
            /* #RGB format. */
            *r = ((color >> 8) & 0xF) * 17;
            *g = ((color >> 4) & 0xF) * 17;
            *b = (color & 0xF) * 17;
        }
        else
        {
            SAIL_LOG_ERROR("XPM: Unsupported color format: %s", str);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        return SAIL_OK;
    }

    /* Named colors - support basic set. */
    if (strcmp(str, "black") == 0 || strcmp(str, "Black") == 0)
    {
        *r = *g = *b = 0;
    }
    else if (strcmp(str, "white") == 0 || strcmp(str, "White") == 0)
    {
        *r = *g = *b = 255;
    }
    else if (strcmp(str, "red") == 0 || strcmp(str, "Red") == 0)
    {
        *r = 255;
        *g = 0;
        *b = 0;
    }
    else if (strcmp(str, "green") == 0 || strcmp(str, "Green") == 0)
    {
        *r = 0;
        *g = 255;
        *b = 0;
    }
    else if (strcmp(str, "blue") == 0 || strcmp(str, "Blue") == 0)
    {
        *r = 0;
        *g = 0;
        *b = 255;
    }
    else if (strcmp(str, "yellow") == 0 || strcmp(str, "Yellow") == 0)
    {
        *r = 255;
        *g = 255;
        *b = 0;
    }
    else if (strcmp(str, "cyan") == 0 || strcmp(str, "Cyan") == 0)
    {
        *r = 0;
        *g = 255;
        *b = 255;
    }
    else if (strcmp(str, "magenta") == 0 || strcmp(str, "Magenta") == 0)
    {
        *r = 255;
        *g = 0;
        *b = 255;
    }
    else if (strcmp(str, "gray") == 0 || strcmp(str, "Gray") == 0 || strcmp(str, "grey") == 0
             || strcmp(str, "Grey") == 0)
    {
        *r = *g = *b = 128;
    }
    else
    {
        /* Default to black for unknown colors. */
        SAIL_LOG_WARNING("XPM: Unknown color name '%s', using black", str);
        *r = *g = *b = 0;
    }

    return SAIL_OK;
}

sail_status_t xpm_private_parse_xpm_header(struct sail_io* io,
                                           unsigned* width,
                                           unsigned* height,
                                           unsigned* num_colors,
                                           unsigned* cpp,
                                           int* x_hotspot,
                                           int* y_hotspot)
{
    char buf[512];

    /* Read lines until we find XPM comment. */
    bool found_xpm_marker = false;
    for (int i = 0; i < 10; i++)
    {
        SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));
        if (strstr(buf, "XPM") != NULL)
        {
            found_xpm_marker = true;
            break;
        }
    }

    if (!found_xpm_marker)
    {
        SAIL_LOG_ERROR("XPM: Missing XPM marker");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Read until we find the values line (contains width, height, etc.). */
    bool found_values = false;
    for (int i = 0; i < 10; i++)
    {
        SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));

        /* Look for a line with at least 4 numbers. */
        const char* ptr = strchr(buf, '"');
        if (ptr != NULL)
        {
            int w, h, nc, c;
            int xh = -1, yh = -1;
            int scanned = sscanf(ptr + 1, "%d %d %d %d %d %d", &w, &h, &nc, &c, &xh, &yh);

            if (scanned >= 4)
            {
                *width       = w;
                *height      = h;
                *num_colors  = nc;
                *cpp         = c;
                *x_hotspot   = xh;
                *y_hotspot   = yh;
                found_values = true;
                break;
            }
        }
    }

    if (!found_values)
    {
        SAIL_LOG_ERROR("XPM: Failed to parse XPM header values");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if (*cpp > 7)
    {
        SAIL_LOG_ERROR("XPM: Characters per pixel (%u) exceeds maximum (7)", *cpp);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if (*num_colors == 0 || *num_colors > 65536)
    {
        SAIL_LOG_ERROR("XPM: Invalid number of colors: %u", *num_colors);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    return SAIL_OK;
}

sail_status_t xpm_private_parse_colors(
    struct sail_io* io, unsigned num_colors, unsigned cpp, struct xpm_color** colors, bool* has_transparency)
{
    void* ptr;
    SAIL_TRY(sail_malloc(num_colors * sizeof(struct xpm_color), &ptr));
    *colors = ptr;

    *has_transparency = false;
    char buf[512];

    for (unsigned i = 0; i < num_colors; i++)
    {
        SAIL_TRY_OR_CLEANUP(sail_read_string_from_io(io, buf, sizeof(buf)),
                            /* cleanup */ sail_free(*colors));

        const char* line = strchr(buf, '"');
        if (line == NULL)
        {
            SAIL_LOG_ERROR("XPM: Failed to parse color line");
            sail_free(*colors);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        line++;

        /* Extract character(s). */
        if (cpp > sizeof((*colors)[i].chars) - 1)
        {
            SAIL_LOG_ERROR("XPM: cpp too large");
            sail_free(*colors);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        memcpy((*colors)[i].chars, line, cpp);
        (*colors)[i].chars[cpp]  = '\0';
        line                    += cpp;

        /* Skip whitespace. */
        while (*line && isspace((unsigned char)*line))
        {
            line++;
        }

        /* Parse color keys (c, m, g, s). We prioritize 'c' (color). */
        (*colors)[i].is_none = false;
        (*colors)[i].r       = 0;
        (*colors)[i].g       = 0;
        (*colors)[i].b       = 0;
        (*colors)[i].a       = 255;

        bool color_found      = false;
        const char* color_ptr = line;

        while (*color_ptr && *color_ptr != '"')
        {
            /* Skip whitespace. */
            while (*color_ptr && isspace((unsigned char)*color_ptr))
            {
                color_ptr++;
            }

            if (*color_ptr == '\0' || *color_ptr == '"')
            {
                break;
            }

            char key = *color_ptr++;

            /* Skip whitespace after key. */
            while (*color_ptr && isspace((unsigned char)*color_ptr))
            {
                color_ptr++;
            }

            /* Extract color value. */
            char color_value[64];
            int j = 0;
            while (*color_ptr && !isspace((unsigned char)*color_ptr) && *color_ptr != '"'
                   && j < (int)sizeof(color_value) - 1)
            {
                color_value[j++] = *color_ptr++;
            }
            color_value[j] = '\0';

            /* Process based on key. */
            if (key == 'c')
            {
                /* Color key - highest priority. */
                if (strcmp(color_value, "None") == 0 || strcmp(color_value, "none") == 0)
                {
                    (*colors)[i].is_none = true;
                    (*colors)[i].a       = 0;
                    *has_transparency    = true;
                }
                else
                {
                    SAIL_TRY_OR_CLEANUP(
                        parse_color_value(color_value, &(*colors)[i].r, &(*colors)[i].g, &(*colors)[i].b),
                        /* cleanup */ sail_free(*colors));
                }
                color_found = true;
                break;
            }
            else if ((key == 'm' || key == 'g') && !color_found)
            {
                /* Monochrome or grayscale - use as fallback. */
                if (strcmp(color_value, "None") == 0 || strcmp(color_value, "none") == 0)
                {
                    (*colors)[i].is_none = true;
                    (*colors)[i].a       = 0;
                    *has_transparency    = true;
                }
                else
                {
                    SAIL_TRY_OR_CLEANUP(
                        parse_color_value(color_value, &(*colors)[i].r, &(*colors)[i].g, &(*colors)[i].b),
                        /* cleanup */ sail_free(*colors));
                }
                color_found = true;
            }
            /* 's' (symbolic) is ignored. */
        }

        if (!color_found)
        {
            SAIL_LOG_WARNING("XPM: No color value found for color %u, using black", i);
        }
    }

    return SAIL_OK;
}

sail_status_t xpm_private_read_pixels(struct sail_io* io,
                                      unsigned width,
                                      unsigned height,
                                      unsigned cpp,
                                      const struct xpm_color* colors,
                                      unsigned num_colors,
                                      unsigned char* pixels,
                                      enum SailPixelFormat pixel_format)
{
    char buf[8192];
    char pixel_chars[8];

    for (unsigned y = 0; y < height; y++)
    {
        SAIL_TRY(sail_read_string_from_io(io, buf, sizeof(buf)));

        const char* line = strchr(buf, '"');
        if (line == NULL)
        {
            SAIL_LOG_ERROR("XPM: Failed to find pixel data on line %u", y);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
        line++;

        for (unsigned x = 0; x < width; x++)
        {
            /* Extract cpp characters. */
            memcpy(pixel_chars, line, cpp);
            pixel_chars[cpp]  = '\0';
            line             += cpp;

            /* Find matching color. */
            bool found = false;
            for (unsigned c = 0; c < num_colors; c++)
            {
                if (memcmp(pixel_chars, colors[c].chars, cpp) == 0)
                {
                    if (pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA)
                    {
                        unsigned char* pixel = pixels + (y * width + x) * 4;
                        pixel[0]             = colors[c].r;
                        pixel[1]             = colors[c].g;
                        pixel[2]             = colors[c].b;
                        pixel[3]             = colors[c].a;
                    }
                    else if (pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED)
                    {
                        pixels[y * width + x] = c;
                    }
                    else if (pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED)
                    {
                        unsigned byte_index = (y * width + x) / 2;
                        unsigned shift      = ((y * width + x) % 2) ? 0 : 4;
                        pixels[byte_index]  = (pixels[byte_index] & ~(0x0F << shift)) | ((c & 0x0F) << shift);
                    }
                    else if (pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED)
                    {
                        unsigned byte_index = (y * width + x) / 4;
                        unsigned shift      = 6 - ((y * width + x) % 4) * 2;
                        pixels[byte_index]  = (pixels[byte_index] & ~(0x03 << shift)) | ((c & 0x03) << shift);
                    }
                    else if (pixel_format == SAIL_PIXEL_FORMAT_BPP1_INDEXED)
                    {
                        unsigned byte_index = (y * width + x) / 8;
                        unsigned shift      = 7 - ((y * width + x) % 8);
                        pixels[byte_index]  = (pixels[byte_index] & ~(1 << shift)) | ((c & 1) << shift);
                    }
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                SAIL_LOG_ERROR("XPM: Unknown pixel character '%.*s' at (%u,%u)", cpp, pixel_chars, x, y);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }
    }

    return SAIL_OK;
}

sail_status_t xpm_private_write_header(struct sail_io* io,
                                       unsigned width,
                                       unsigned height,
                                       unsigned num_colors,
                                       unsigned cpp,
                                       const char* name,
                                       int x_hotspot,
                                       int y_hotspot)
{
    const char* var_name = (name != NULL && name[0] != '\0') ? name : "image";

    char header[512];
    int written;

    written = snprintf(header, sizeof(header), "/* XPM */\nstatic char *%s[] = {\n", var_name);
    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("XPM: Failed to format header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }
    SAIL_TRY(io->strict_write(io->stream, header, written));

    /* Values line. */
    if (x_hotspot >= 0 && y_hotspot >= 0)
    {
        written = snprintf(header, sizeof(header), "\"%u %u %u %u %d %d\",\n", width, height, num_colors, cpp,
                           x_hotspot, y_hotspot);
    }
    else
    {
        written = snprintf(header, sizeof(header), "\"%u %u %u %u\",\n", width, height, num_colors, cpp);
    }

    if (written < 0 || (size_t)written >= sizeof(header))
    {
        SAIL_LOG_ERROR("XPM: Failed to format values line");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }
    SAIL_TRY(io->strict_write(io->stream, header, written));

    return SAIL_OK;
}

sail_status_t xpm_private_write_colors(struct sail_io* io,
                                       const unsigned char* palette_data,
                                       unsigned num_colors,
                                       unsigned cpp,
                                       bool has_transparency,
                                       int transparency_index)
{
    char line[256];

    for (unsigned i = 0; i < num_colors; i++)
    {
        const unsigned char* color = palette_data + i * 3;

        /* Generate character(s) for this color. */
        char chars[8];
        unsigned idx = i;
        for (unsigned j = 0; j < cpp; j++)
        {
            chars[cpp - 1 - j]  = XPM_CHARS[idx % sizeof(XPM_CHARS)];
            idx                /= sizeof(XPM_CHARS);
        }
        chars[cpp] = '\0';

        int written;
        if (has_transparency && (int)i == transparency_index)
        {
            written = snprintf(line, sizeof(line), "\"%s c None\",\n", chars);
        }
        else
        {
            written = snprintf(line, sizeof(line), "\"%s c #%02X%02X%02X\",\n", chars, color[0], color[1], color[2]);
        }

        if (written < 0 || (size_t)written >= sizeof(line))
        {
            SAIL_LOG_ERROR("XPM: Failed to format color line");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
        }

        SAIL_TRY(io->strict_write(io->stream, line, written));
    }

    return SAIL_OK;
}

sail_status_t xpm_private_write_pixels(
    struct sail_io* io, const unsigned char* pixels, unsigned width, unsigned height, unsigned cpp, unsigned num_colors)
{
    char* line               = NULL;
    const unsigned line_size = width * cpp + 16;

    void* ptr;
    SAIL_TRY(sail_malloc(line_size, &ptr));
    line = ptr;

    for (unsigned y = 0; y < height; y++)
    {
        line[0]      = '"';
        unsigned pos = 1;

        for (unsigned x = 0; x < width; x++)
        {
            unsigned char pixel_index = pixels[y * width + x];

            if (pixel_index >= num_colors)
            {
                SAIL_LOG_ERROR("XPM: Pixel index %u out of range (max %u)", pixel_index, num_colors - 1);
                sail_free(line);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }

            /* Generate character(s) for this pixel. */
            unsigned idx = pixel_index;
            for (unsigned j = 0; j < cpp; j++)
            {
                line[pos + cpp - 1 - j]  = XPM_CHARS[idx % sizeof(XPM_CHARS)];
                idx                     /= sizeof(XPM_CHARS);
            }
            pos += cpp;
        }

        line[pos++] = '"';
        if (y < height - 1)
        {
            line[pos++] = ',';
        }
        line[pos++] = '\n';

        SAIL_TRY_OR_CLEANUP(io->strict_write(io->stream, line, pos),
                            /* cleanup */ sail_free(line));
    }

    sail_free(line);

    /* Write closing. */
    SAIL_TRY(io->strict_write(io->stream, "};\n", 3));

    return SAIL_OK;
}

bool xpm_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct xpm_state* xpm_state = user_data;

    if (strcmp(key, "xpm-name") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);

            if (str_value != NULL)
            {
                strncpy(xpm_state->var_name, str_value, sizeof(xpm_state->var_name) - 1);
                xpm_state->var_name[sizeof(xpm_state->var_name) - 1] = '\0';
                SAIL_LOG_TRACE("XPM: Using variable name '%s'", xpm_state->var_name);
            }
        }
        else
        {
            SAIL_LOG_ERROR("XPM: 'xpm-name' must be a string");
        }
    }

    return true;
}

sail_status_t xpm_private_skip_extensions(struct sail_io* io)
{
    /* XPM extensions start with "XPMEXT" and end with "XPMENDEXT".
     * We simply skip them for now. */
    char buf[512];

    while (true)
    {
        size_t bytes_read;
        SAIL_TRY(io->tolerant_read(io->stream, buf, sizeof(buf) - 1, &bytes_read));

        if (bytes_read == 0)
        {
            break;
        }

        buf[bytes_read] = '\0';

        if (strstr(buf, "XPMENDEXT") != NULL)
        {
            break;
        }
    }

    return SAIL_OK;
}

enum SailPixelFormat xpm_private_determine_pixel_format(unsigned num_colors, bool has_transparency)
{
    if (has_transparency)
    {
        return SAIL_PIXEL_FORMAT_BPP32_RGBA;
    }

    if (num_colors <= 2)
    {
        return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
    }
    else if (num_colors <= 4)
    {
        return SAIL_PIXEL_FORMAT_BPP2_INDEXED;
    }
    else if (num_colors <= 16)
    {
        return SAIL_PIXEL_FORMAT_BPP4_INDEXED;
    }
    else if (num_colors <= 256)
    {
        return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    }
    else
    {
        return SAIL_PIXEL_FORMAT_BPP24_RGB;
    }
}

sail_status_t xpm_private_build_palette(struct sail_palette** palette,
                                        const struct xpm_color* colors,
                                        unsigned num_colors)
{
    SAIL_TRY(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, num_colors, palette));

    unsigned char* palette_data = (*palette)->data;
    for (unsigned i = 0; i < num_colors; i++)
    {
        palette_data[i * 3 + 0] = colors[i].r;
        palette_data[i * 3 + 1] = colors[i].g;
        palette_data[i * 3 + 2] = colors[i].b;
    }

    return SAIL_OK;
}

sail_status_t xpm_private_store_hotspot(int x_hotspot, int y_hotspot, struct sail_hash_map* special_properties)
{
    if (x_hotspot < 0 || y_hotspot < 0)
    {
        return SAIL_OK;
    }

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct sail_variant* variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    SAIL_LOG_TRACE("XPM: X hotspot(%d)", x_hotspot);
    sail_set_variant_int(variant, x_hotspot);
    sail_put_hash_map(special_properties, "xpm-hotspot-x", variant);

    SAIL_LOG_TRACE("XPM: Y hotspot(%d)", y_hotspot);
    sail_set_variant_int(variant, y_hotspot);
    sail_put_hash_map(special_properties, "xpm-hotspot-y", variant);

    sail_destroy_variant(variant);

    return SAIL_OK;
}

sail_status_t xpm_private_fetch_hotspot(const struct sail_hash_map* special_properties, int* x_hotspot, int* y_hotspot)
{
    *x_hotspot = -1;
    *y_hotspot = -1;

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    const struct sail_variant* variant;

    /* Get X hotspot. */
    variant = sail_hash_map_value(special_properties, "xpm-hotspot-x");
    if (variant != NULL && variant->type == SAIL_VARIANT_TYPE_INT)
    {
        *x_hotspot = sail_variant_to_int(variant);
    }

    /* Get Y hotspot. */
    variant = sail_hash_map_value(special_properties, "xpm-hotspot-y");
    if (variant != NULL && variant->type == SAIL_VARIANT_TYPE_INT)
    {
        *y_hotspot = sail_variant_to_int(variant);
    }

    return SAIL_OK;
}

sail_status_t xpm_private_check_transparency(const struct sail_palette* palette,
                                             unsigned num_colors,
                                             bool* has_transparency,
                                             int* transparency_index)
{
    *has_transparency   = false;
    *transparency_index = -1;

    if (palette->pixel_format != SAIL_PIXEL_FORMAT_BPP32_RGBA && palette->pixel_format != SAIL_PIXEL_FORMAT_BPP32_BGRA
        && palette->pixel_format != SAIL_PIXEL_FORMAT_BPP32_ARGB
        && palette->pixel_format != SAIL_PIXEL_FORMAT_BPP32_ABGR)
    {
        return SAIL_OK;
    }

    const unsigned char* palette_data = palette->data;
    const unsigned bytes_per_color    = sail_bytes_per_line(1, palette->pixel_format);

    for (unsigned i = 0; i < num_colors; i++)
    {
        unsigned char alpha;

        switch (palette->pixel_format)
        {
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        {
            alpha = palette_data[i * bytes_per_color + 3];
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:
        {
            alpha = palette_data[i * bytes_per_color + 0];
            break;
        }
        default:
        {
            alpha = 255;
            break;
        }
        }

        if (alpha < 128)
        {
            *has_transparency   = true;
            *transparency_index = i;
            break;
        }
    }

    return SAIL_OK;
}

sail_status_t xpm_private_convert_palette_to_rgb(const unsigned char* src_palette,
                                                 enum SailPixelFormat src_format,
                                                 unsigned num_colors,
                                                 unsigned char** rgb_palette)
{
    if (src_format == SAIL_PIXEL_FORMAT_BPP24_RGB)
    {
        *rgb_palette = NULL;
        return SAIL_OK;
    }

    void* ptr;
    SAIL_TRY(sail_malloc(num_colors * 3, &ptr));
    *rgb_palette = ptr;

    unsigned char* dst_palette = *rgb_palette;

    for (unsigned i = 0; i < num_colors; i++)
    {
        switch (src_format)
        {
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        {
            dst_palette[0]  = src_palette[2];
            dst_palette[1]  = src_palette[1];
            dst_palette[2]  = src_palette[0];
            src_palette    += 3;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        {
            dst_palette[0]  = src_palette[0];
            dst_palette[1]  = src_palette[1];
            dst_palette[2]  = src_palette[2];
            src_palette    += 4;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        {
            dst_palette[0]  = src_palette[2];
            dst_palette[1]  = src_palette[1];
            dst_palette[2]  = src_palette[0];
            src_palette    += 4;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        {
            dst_palette[0]  = src_palette[1];
            dst_palette[1]  = src_palette[2];
            dst_palette[2]  = src_palette[3];
            src_palette    += 4;
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:
        {
            dst_palette[0]  = src_palette[3];
            dst_palette[1]  = src_palette[2];
            dst_palette[2]  = src_palette[1];
            src_palette    += 4;
            break;
        }
        default:
        {
            dst_palette[0] = 0;
            dst_palette[1] = 0;
            dst_palette[2] = 0;
            break;
        }
        }
        dst_palette += 3;
    }

    return SAIL_OK;
}
