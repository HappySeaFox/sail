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

#include <sail-manip/sail-manip.h>

/* https://en.wikipedia.org/wiki/Grayscale */
static const double R_TO_GRAY_COEFFICIENT = 0.299;
static const double G_TO_GRAY_COEFFICIENT = 0.587;
static const double B_TO_GRAY_COEFFICIENT = 0.114;

sail_status_t get_palette_rgba32(const struct sail_palette* palette, unsigned index, sail_rgba32_t* rgba32)
{

    if (index >= palette->color_count)
    {
        SAIL_LOG_ERROR("Palette index %u is out of range [0; %u)", index, palette->color_count);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    switch (palette->pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        const uint8_t* entry = (uint8_t*)palette->data + index * 3;

        rgba32->component1 = *(entry + 0);
        rgba32->component2 = *(entry + 1);
        rgba32->component3 = *(entry + 2);
        rgba32->component4 = 255;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        const uint8_t* entry = (uint8_t*)palette->data + index * 4;

        rgba32->component1 = *(entry + 0);
        rgba32->component2 = *(entry + 1);
        rgba32->component3 = *(entry + 2);
        rgba32->component4 = *(entry + 3);
        break;
    }
    default:
    {
        SAIL_LOG_ERROR("Palette pixel format %s is not currently supported",
                       sail_pixel_format_to_string(palette->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }

    return SAIL_OK;
}

void spread_gray8_to_rgba32(uint8_t value, sail_rgba32_t* rgba32)
{

    rgba32->component1 = rgba32->component2 = rgba32->component3 = value;
    rgba32->component4                                           = 255;
}

void spread_gray16_to_rgba32(uint16_t value, sail_rgba32_t* rgba32)
{

    rgba32->component1 = rgba32->component2 = rgba32->component3 = SAIL_COMPONENT_16_TO_8(value);
    rgba32->component4                                           = 255;
}

void spread_gray8_to_rgba64(uint8_t value, sail_rgba64_t* rgba64)
{

    rgba64->component1 = rgba64->component2 = rgba64->component3 = SAIL_COMPONENT_8_TO_16(value);
    rgba64->component4                                           = 65535;
}

void spread_gray16_to_rgba64(uint16_t value, sail_rgba64_t* rgba64)
{

    rgba64->component1 = rgba64->component2 = rgba64->component3 = value;
    rgba64->component4                                           = 65535;
}

void fill_gray8_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                        uint8_t* scan,
                                        const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    *scan = SAIL_RGB8_TO_GRAY8(rgb24.component1, rgb24.component2, rgb24.component3);
}

void fill_gray8_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                         uint8_t* scan,
                                         const struct sail_conversion_options* options)
{
    sail_rgb24_t rgb24;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        rgb24.component1 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1));
        rgb24.component2 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2));
        rgb24.component3 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3));
    }
    else
    {
        rgb24.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
        rgb24.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
        rgb24.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    }

    *scan = SAIL_RGB8_TO_GRAY8(rgb24.component1, rgb24.component2, rgb24.component3);
}

void fill_gray16_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         const struct sail_conversion_options* options)
{
    sail_rgb48_t rgb48;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb48.component1 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component1)
                                      + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component2)
                                      + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component3)
                                      + (1 - opacity) * options->background48.component3);
    }
    else
    {
        rgb48.component1 = SAIL_COMPONENT_8_TO_16(rgba32->component1);
        rgb48.component2 = SAIL_COMPONENT_8_TO_16(rgba32->component2);
        rgb48.component3 = SAIL_COMPONENT_8_TO_16(rgba32->component3);
    }

    *scan = SAIL_RGB16_TO_GRAY16(rgb48.component1, rgb48.component2, rgb48.component3);
}

void fill_gray16_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          const struct sail_conversion_options* options)
{
    sail_rgb48_t rgb48;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        rgb48.component1 = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        rgb48.component1 = rgba64->component1;
        rgb48.component2 = rgba64->component2;
        rgb48.component3 = rgba64->component3;
    }

    *scan = SAIL_RGB16_TO_GRAY16(rgb48.component1, rgb48.component2, rgb48.component3);
}

void fill_rgb24_pixel_from_uint8_values(
    const sail_rgba32_t* rgba32, uint8_t* scan, int r, int g, int b, const struct sail_conversion_options* options)
{

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        *(scan + r) = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        *(scan + g) = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        *(scan + b) = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        *(scan + r) = rgba32->component1;
        *(scan + g) = rgba32->component2;
        *(scan + b) = rgba32->component3;
    }
}

void fill_rgb24_pixel_from_uint16_values(
    const sail_rgba64_t* rgba64, uint8_t* scan, int r, int g, int b, const struct sail_conversion_options* options)
{
    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan + r) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1));
        *(scan + g) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2));
        *(scan + b) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3));
    }
    else
    {
        *(scan + r) = SAIL_COMPONENT_16_TO_8(rgba64->component1);
        *(scan + g) = SAIL_COMPONENT_16_TO_8(rgba64->component2);
        *(scan + b) = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    }
}

void fill_rgb48_pixel_from_uint8_values(
    const sail_rgba32_t* rgba32, uint16_t* scan, int r, int g, int b, const struct sail_conversion_options* options)
{

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        *(scan + r) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component1)
                                 + (1 - opacity) * options->background48.component1);
        *(scan + g) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component2)
                                 + (1 - opacity) * options->background48.component2);
        *(scan + b) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component3)
                                 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        *(scan + r) = SAIL_COMPONENT_8_TO_16(rgba32->component1);
        *(scan + g) = SAIL_COMPONENT_8_TO_16(rgba32->component2);
        *(scan + b) = SAIL_COMPONENT_8_TO_16(rgba32->component3);
    }
}

void fill_rgb48_pixel_from_uint16_values(
    const sail_rgba64_t* rgba64, uint16_t* scan, int r, int g, int b, const struct sail_conversion_options* options)
{

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan + r) = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        *(scan + g) = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        *(scan + b) = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        *(scan + r) = rgba64->component1;
        *(scan + g) = rgba64->component2;
        *(scan + b) = rgba64->component3;
    }
}

void fill_rgba32_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint8_t* scan,
                                         int r,
                                         int g,
                                         int b,
                                         int a,
                                         const struct sail_conversion_options* options)
{

    if (a < 0 && rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        *(scan + r) = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        *(scan + g) = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        *(scan + b) = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        *(scan + r) = rgba32->component1;
        *(scan + g) = rgba32->component2;
        *(scan + b) = rgba32->component3;
    }

    if (a >= 0)
    {
        *(scan + a) = rgba32->component4;
    }
}

void fill_rgba32_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint8_t* scan,
                                          int r,
                                          int g,
                                          int b,
                                          int a,
                                          const struct sail_conversion_options* options)
{

    if (a < 0 && rgba64->component4 < 65535 && options != NULL
        && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan + r) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1));
        *(scan + g) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2));
        *(scan + b) = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3));
    }
    else
    {
        *(scan + r) = SAIL_COMPONENT_16_TO_8(rgba64->component1);
        *(scan + g) = SAIL_COMPONENT_16_TO_8(rgba64->component2);
        *(scan + b) = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    }

    if (a >= 0)
    {
        *(scan + a) = SAIL_COMPONENT_16_TO_8(rgba64->component4);
    }
}

void fill_rgba64_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         int r,
                                         int g,
                                         int b,
                                         int a,
                                         const struct sail_conversion_options* options)
{

    if (a < 0 && rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        *(scan + r) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component1)
                                 + (1 - opacity) * options->background48.component1);
        *(scan + g) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component2)
                                 + (1 - opacity) * options->background48.component2);
        *(scan + b) = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component3)
                                 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        *(scan + r) = SAIL_COMPONENT_8_TO_16(rgba32->component1);
        *(scan + g) = SAIL_COMPONENT_8_TO_16(rgba32->component2);
        *(scan + b) = SAIL_COMPONENT_8_TO_16(rgba32->component3);
    }

    if (a >= 0)
    {
        *(scan + a) = SAIL_COMPONENT_8_TO_16(rgba32->component4);
    }
}

void fill_rgba64_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          int r,
                                          int g,
                                          int b,
                                          int a,
                                          const struct sail_conversion_options* options)
{

    if (a < 0 && rgba64->component4 < 65535 && options != NULL
        && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        *(scan + r) = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        *(scan + g) = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        *(scan + b) = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        *(scan + r) = rgba64->component1;
        *(scan + g) = rgba64->component2;
        *(scan + b) = rgba64->component3;
    }

    if (a >= 0)
    {
        *(scan + a) = rgba64->component4;
    }
}

void fill_ycbcr_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                        uint8_t* scan,
                                        const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32_no_alpha;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgba32_no_alpha.component1 =
            (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgba32_no_alpha.component2 =
            (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgba32_no_alpha.component3 =
            (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgba32_no_alpha = *rgba32;
    }

    convert_rgba32_to_ycbcr24(&rgba32_no_alpha, scan + 0, scan + 1, scan + 2);
}

void fill_ycbcr_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                         uint8_t* scan,
                                         const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32_no_alpha;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        rgba32_no_alpha.component1 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1));
        rgba32_no_alpha.component2 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2));
        rgba32_no_alpha.component3 = SAIL_COMPONENT_16_TO_8(
            (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3));
    }
    else
    {
        rgba32_no_alpha.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
        rgba32_no_alpha.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
        rgba32_no_alpha.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    }

    convert_rgba32_to_ycbcr24(&rgba32_no_alpha, scan + 0, scan + 1, scan + 2);
}

void fill_gray_alpha8_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                              uint8_t* scan,
                                              const struct sail_conversion_options* options)
{

    (void)options;

    uint8_t gray = (uint8_t)((R_TO_GRAY_COEFFICIENT * rgba32->component1) + (G_TO_GRAY_COEFFICIENT * rgba32->component2)
                             + (B_TO_GRAY_COEFFICIENT * rgba32->component3));

    /* BPP8_GRAYSCALE_ALPHA: 4 bits gray + 4 bits alpha */
    *scan = (gray & 0xF0) | ((rgba32->component4 >> 4) & 0x0F);
}

void fill_gray_alpha8_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                               uint8_t* scan,
                                               const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_gray_alpha8_pixel_from_uint8_values(&rgba32, scan, options);
}

void fill_gray_alpha16_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                               uint8_t* scan,
                                               const struct sail_conversion_options* options)
{

    (void)options;

    uint8_t gray = (uint8_t)((R_TO_GRAY_COEFFICIENT * rgba32->component1) + (G_TO_GRAY_COEFFICIENT * rgba32->component2)
                             + (B_TO_GRAY_COEFFICIENT * rgba32->component3));

    /* BPP16_GRAYSCALE_ALPHA: 8 bits gray + 8 bits alpha */
    *(scan + 0) = gray;
    *(scan + 1) = rgba32->component4;
}

void fill_gray_alpha16_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                                uint8_t* scan,
                                                const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_gray_alpha16_pixel_from_uint8_values(&rgba32, scan, options);
}

void fill_gray_alpha32_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                               uint16_t* scan,
                                               const struct sail_conversion_options* options)
{

    (void)options;

    uint16_t gray =
        SAIL_COMPONENT_8_TO_16(SAIL_RGB8_TO_GRAY8(rgba32->component1, rgba32->component2, rgba32->component3));

    /* BPP32_GRAYSCALE_ALPHA: 16 bits gray + 16 bits alpha */
    *(scan + 0) = gray;
    *(scan + 1) = SAIL_COMPONENT_8_TO_16(rgba32->component4);
}

void fill_gray_alpha32_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                                uint16_t* scan,
                                                const struct sail_conversion_options* options)
{

    (void)options;

    uint16_t gray =
        (uint16_t)((R_TO_GRAY_COEFFICIENT * rgba64->component1) + (G_TO_GRAY_COEFFICIENT * rgba64->component2)
                   + (B_TO_GRAY_COEFFICIENT * rgba64->component3));

    /* BPP32_GRAYSCALE_ALPHA: 16 bits gray + 16 bits alpha */
    *(scan + 0) = gray;
    *(scan + 1) = rgba64->component4;
}

void fill_rgb555_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         int r_shift,
                                         int g_shift,
                                         int b_shift,
                                         const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    /* RGB555: 5 bits per component */
    *scan = (uint16_t)(((rgb24.component1 >> 3) << r_shift) | ((rgb24.component2 >> 3) << g_shift)
                       | ((rgb24.component3 >> 3) << b_shift));
}

void fill_rgb555_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          int r_shift,
                                          int g_shift,
                                          int b_shift,
                                          const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_rgb555_pixel_from_uint8_values(&rgba32, scan, r_shift, g_shift, b_shift, options);
}

void fill_rgb565_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         int r_shift,
                                         int g_shift,
                                         int b_shift,
                                         const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    /* RGB565: 5 bits red, 6 bits green, 5 bits blue */
    *scan = (uint16_t)(((rgb24.component1 >> 3) << r_shift) | ((rgb24.component2 >> 2) << g_shift)
                       | ((rgb24.component3 >> 3) << b_shift));
}

void fill_rgb565_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          int r_shift,
                                          int g_shift,
                                          int b_shift,
                                          const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_rgb565_pixel_from_uint8_values(&rgba32, scan, r_shift, g_shift, b_shift, options);
}

void fill_cmyk32_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint8_t* scan,
                                         const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    /* RGB to CMYK conversion */
    const double r = rgb24.component1 / 255.0;
    const double g = rgb24.component2 / 255.0;
    const double b = rgb24.component3 / 255.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint8_t)(c * 255);
    *(scan + 1) = (uint8_t)(m * 255);
    *(scan + 2) = (uint8_t)(y * 255);
    *(scan + 3) = (uint8_t)(k * 255);
}

void fill_cmyk32_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint8_t* scan,
                                          const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_cmyk32_pixel_from_uint8_values(&rgba32, scan, options);
}

void fill_cmyk64_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    /* RGB to CMYK conversion */
    const double r = rgb24.component1 / 255.0;
    const double g = rgb24.component2 / 255.0;
    const double b = rgb24.component3 / 255.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint16_t)(c * 65535);
    *(scan + 1) = (uint16_t)(m * 65535);
    *(scan + 2) = (uint16_t)(y * 65535);
    *(scan + 3) = (uint16_t)(k * 65535);
}

void fill_cmyk64_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          const struct sail_conversion_options* options)
{

    sail_rgb48_t rgb48;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        rgb48.component1 = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        rgb48.component1 = rgba64->component1;
        rgb48.component2 = rgba64->component2;
        rgb48.component3 = rgba64->component3;
    }

    /* RGB to CMYK conversion */
    const double r = rgb48.component1 / 65535.0;
    const double g = rgb48.component2 / 65535.0;
    const double b = rgb48.component3 / 65535.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint16_t)(c * 65535);
    *(scan + 1) = (uint16_t)(m * 65535);
    *(scan + 2) = (uint16_t)(y * 65535);
    *(scan + 3) = (uint16_t)(k * 65535);
}

void fill_cmyka40_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                          uint8_t* scan,
                                          const struct sail_conversion_options* options)
{

    sail_rgb24_t rgb24;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb24.component1 = (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgb24.component2 = (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgb24.component3 = (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgb24.component1 = rgba32->component1;
        rgb24.component2 = rgba32->component2;
        rgb24.component3 = rgba32->component3;
    }

    /* RGB to CMYK conversion */
    const double r = rgb24.component1 / 255.0;
    const double g = rgb24.component2 / 255.0;
    const double b = rgb24.component3 / 255.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint8_t)(c * 255);
    *(scan + 1) = (uint8_t)(m * 255);
    *(scan + 2) = (uint8_t)(y * 255);
    *(scan + 3) = (uint8_t)(k * 255);
    *(scan + 4) = rgba32->component4;
}

void fill_cmyka40_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                           uint8_t* scan,
                                           const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_cmyka40_pixel_from_uint8_values(&rgba32, scan, options);
}

void fill_cmyka80_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                          uint16_t* scan,
                                          const struct sail_conversion_options* options)
{

    sail_rgb48_t rgb48;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgb48.component1 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component1)
                                      + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component2)
                                      + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * SAIL_COMPONENT_8_TO_16(rgba32->component3)
                                      + (1 - opacity) * options->background48.component3);
    }
    else
    {
        rgb48.component1 = SAIL_COMPONENT_8_TO_16(rgba32->component1);
        rgb48.component2 = SAIL_COMPONENT_8_TO_16(rgba32->component2);
        rgb48.component3 = SAIL_COMPONENT_8_TO_16(rgba32->component3);
    }

    /* RGB to CMYK conversion */
    const double r = rgb48.component1 / 65535.0;
    const double g = rgb48.component2 / 65535.0;
    const double b = rgb48.component3 / 65535.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint16_t)(c * 65535);
    *(scan + 1) = (uint16_t)(m * 65535);
    *(scan + 2) = (uint16_t)(y * 65535);
    *(scan + 3) = (uint16_t)(k * 65535);
    *(scan + 4) = SAIL_COMPONENT_8_TO_16(rgba32->component4);
}

void fill_cmyka80_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                           uint16_t* scan,
                                           const struct sail_conversion_options* options)
{

    sail_rgb48_t rgb48;

    if (rgba64->component4 < 65535 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba64->component4 / 65535.0;

        rgb48.component1 = (uint16_t)(opacity * rgba64->component1 + (1 - opacity) * options->background48.component1);
        rgb48.component2 = (uint16_t)(opacity * rgba64->component2 + (1 - opacity) * options->background48.component2);
        rgb48.component3 = (uint16_t)(opacity * rgba64->component3 + (1 - opacity) * options->background48.component3);
    }
    else
    {
        rgb48.component1 = rgba64->component1;
        rgb48.component2 = rgba64->component2;
        rgb48.component3 = rgba64->component3;
    }

    /* RGB to CMYK conversion */
    const double r = rgb48.component1 / 65535.0;
    const double g = rgb48.component2 / 65535.0;
    const double b = rgb48.component3 / 65535.0;

    const double k = 1.0 - SAIL_MAX(SAIL_MAX(r, g), b);
    const double c = (1.0 - r - k) / (1.0 - k + 1e-10);
    const double m = (1.0 - g - k) / (1.0 - k + 1e-10);
    const double y = (1.0 - b - k) / (1.0 - k + 1e-10);

    *(scan + 0) = (uint16_t)(c * 65535);
    *(scan + 1) = (uint16_t)(m * 65535);
    *(scan + 2) = (uint16_t)(y * 65535);
    *(scan + 3) = (uint16_t)(k * 65535);
    *(scan + 4) = rgba64->component4;
}

void fill_rgba16_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                         uint16_t* scan,
                                         int r_shift,
                                         int g_shift,
                                         int b_shift,
                                         int a_shift,
                                         int bits_per_component,
                                         const struct sail_conversion_options* options)
{

    (void)options;

    const uint8_t shift = 8 - bits_per_component;

    uint16_t result = (uint16_t)(((rgba32->component1 >> shift) << r_shift) | ((rgba32->component2 >> shift) << g_shift)
                                 | ((rgba32->component3 >> shift) << b_shift));

    if (a_shift >= 0)
    {
        result |= (rgba32->component4 >> shift) << a_shift;
    }

    *scan = result;
}

void fill_rgba16_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                          uint16_t* scan,
                                          int r_shift,
                                          int g_shift,
                                          int b_shift,
                                          int a_shift,
                                          int bits_per_component,
                                          const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_rgba16_pixel_from_uint8_values(&rgba32, scan, r_shift, g_shift, b_shift, a_shift, bits_per_component, options);
}

void fill_yuv24_pixel_from_uint8_values(const sail_rgba32_t* rgba32,
                                        uint8_t* scan,
                                        const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32_no_alpha;

    if (rgba32->component4 < 255 && options != NULL && (options->options & SAIL_CONVERSION_OPTION_BLEND_ALPHA))
    {
        const double opacity = rgba32->component4 / 255.0;

        rgba32_no_alpha.component1 =
            (uint8_t)(opacity * rgba32->component1 + (1 - opacity) * options->background24.component1);
        rgba32_no_alpha.component2 =
            (uint8_t)(opacity * rgba32->component2 + (1 - opacity) * options->background24.component2);
        rgba32_no_alpha.component3 =
            (uint8_t)(opacity * rgba32->component3 + (1 - opacity) * options->background24.component3);
    }
    else
    {
        rgba32_no_alpha = *rgba32;
    }

    /* RGB to YUV conversion using ITU-R BT.601 */
    const uint8_t r = rgba32_no_alpha.component1;
    const uint8_t g = rgba32_no_alpha.component2;
    const uint8_t b = rgba32_no_alpha.component3;

    *(scan + 0) = (uint8_t)((0.299 * r) + (0.587 * g) + (0.114 * b));
    *(scan + 1) = (uint8_t)(128 + ((-0.168736 * r) - (0.331264 * g) + (0.5 * b)));
    *(scan + 2) = (uint8_t)(128 + ((0.5 * r) - (0.418688 * g) - (0.081312 * b)));
}

void fill_yuv24_pixel_from_uint16_values(const sail_rgba64_t* rgba64,
                                         uint8_t* scan,
                                         const struct sail_conversion_options* options)
{

    sail_rgba32_t rgba32;
    rgba32.component1 = SAIL_COMPONENT_16_TO_8(rgba64->component1);
    rgba32.component2 = SAIL_COMPONENT_16_TO_8(rgba64->component2);
    rgba32.component3 = SAIL_COMPONENT_16_TO_8(rgba64->component3);
    rgba32.component4 = SAIL_COMPONENT_16_TO_8(rgba64->component4);

    fill_yuv24_pixel_from_uint8_values(&rgba32, scan, options);
}
