/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)
 *
 *  Copyright (c) 2026 Dmitry Baryshev
 *
 *  The MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "scale.h"
#include "scale_swscale.h"

/*
 * Forward declaration for manual scaling function.
 */
static sail_status_t scale_with_manual(const struct sail_image* src_image,
                                       struct sail_image* dst_image,
                                       enum SailScaling algorithm);

/*
 * Public functions.
 */

sail_status_t sail_scale_image(const struct sail_image* image,
                               unsigned new_width,
                               unsigned new_height,
                               enum SailScaling algorithm,
                               struct sail_image** image_output)
{
    SAIL_TRY(sail_check_image_valid(image));
    SAIL_CHECK_PTR(image_output);

    if (new_width == 0 || new_height == 0)
    {
        SAIL_LOG_ERROR("Output dimensions must be greater than zero");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    const unsigned bits_per_pixel = sail_bits_per_pixel(image->pixel_format);

    if (bits_per_pixel % 8 != 0)
    {
        SAIL_LOG_ERROR("Only byte-aligned pixels are supported for scaling");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* If dimensions are the same, just copy the image. */
    if (image->width == new_width && image->height == new_height)
    {
        SAIL_TRY(sail_copy_image(image, image_output));
        return SAIL_OK;
    }

    /* Determine if we need 64-bit RGBA (use 64-bit for formats with more than 32 bits per pixel). */
    const bool use_64bit                   = (bits_per_pixel > 32);
    const enum SailPixelFormat rgba_format = use_64bit ? SAIL_PIXEL_FORMAT_BPP64_RGBA : SAIL_PIXEL_FORMAT_BPP32_RGBA;

    /* Convert to RGBA format for scaling. */
    struct sail_image* rgba_image = NULL;
    SAIL_TRY(sail_convert_image(image, rgba_format, &rgba_image));

    /* Create output image skeleton with metadata. */
    struct sail_image* output = NULL;
    SAIL_TRY_OR_CLEANUP(sail_copy_image_skeleton(image, &output),
                        /* cleanup */ sail_destroy_image(rgba_image));

    /* Update dimensions and pixel format. */
    output->width          = new_width;
    output->height         = new_height;
    output->pixel_format   = rgba_format;
    output->bytes_per_line = sail_bytes_per_line(new_width, rgba_format);

    /* Copy palette if present. */
    if (image->palette != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_palette(image->palette, &output->palette),
                            /* cleanup */ sail_destroy_image(output);
                            sail_destroy_image(rgba_image));
    }

    /* Allocate pixels. */
    const size_t pixels_size = (size_t)output->height * output->bytes_per_line;
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &output->pixels),
                        /* cleanup */ sail_destroy_image(output);
                        sail_destroy_image(rgba_image));

    /* Try swscale first if available. */
#ifdef SAIL_MANIP_SWSCALE_ENABLED
    sail_status_t status = scale_with_swscale(rgba_image, output, algorithm);

    if (status == SAIL_OK)
    {
        /* Swscale succeeded - convert back to original format if needed. */
        if (output->pixel_format != image->pixel_format)
        {
            struct sail_image* converted = NULL;
            SAIL_TRY_OR_CLEANUP(sail_convert_image(output, image->pixel_format, &converted),
                                /* cleanup */ sail_destroy_image(output);
                                sail_destroy_image(rgba_image));
            sail_destroy_image(output);
            sail_destroy_image(rgba_image);
            output = converted;
        }
        else
        {
            sail_destroy_image(rgba_image);
        }

        *image_output = output;
        return SAIL_OK;
    }

    /* Swscale failed - fallback to manual scaling. */
    SAIL_LOG_DEBUG("SWSCALE: Scaling failed, falling back to manual scaling");
    sail_destroy_image(output);
    sail_destroy_image(rgba_image);
#endif /* SAIL_MANIP_SWSCALE_ENABLED */

    /* Fallback to manual scaling (or use it directly if swscale is not available). */
    /* Convert to RGBA format for scaling. */
    rgba_image = NULL;
    SAIL_TRY(sail_convert_image(image, rgba_format, &rgba_image));

    /* Create output image skeleton with metadata. */
    output = NULL;
    SAIL_TRY_OR_CLEANUP(sail_copy_image_skeleton(image, &output),
                        /* cleanup */ sail_destroy_image(rgba_image));

    /* Update dimensions and pixel format. */
    output->width          = new_width;
    output->height         = new_height;
    output->pixel_format   = rgba_format;
    output->bytes_per_line = sail_bytes_per_line(new_width, rgba_format);

    /* Copy palette if present. */
    if (image->palette != NULL)
    {
        SAIL_TRY_OR_CLEANUP(sail_copy_palette(image->palette, &output->palette),
                            /* cleanup */ sail_destroy_image(output);
                            sail_destroy_image(rgba_image));
    }

    /* Allocate pixels */
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixels_size, &output->pixels),
                        /* cleanup */ sail_destroy_image(output);
                        sail_destroy_image(rgba_image));

    /* Scale using manual implementation. */
    sail_status_t manual_status = scale_with_manual(rgba_image, output, algorithm);

    sail_destroy_image(rgba_image);

    if (manual_status != SAIL_OK)
    {
        sail_destroy_image(output);
        return manual_status;
    }

    /* Convert back to original format if needed. */
    if (output->pixel_format != image->pixel_format)
    {
        struct sail_image* converted = NULL;
        SAIL_TRY_OR_CLEANUP(sail_convert_image(output, image->pixel_format, &converted),
                            /* cleanup */ sail_destroy_image(output));
        sail_destroy_image(output);
        output = converted;
    }

    *image_output = output;

    return SAIL_OK;
}

/*
 * Manual scaling implementation (fallback when swscale is not available or fails).
 */

/* Clamp value to [0, max]. */
static inline unsigned clamp_unsigned(unsigned value, unsigned max)
{
    return (value > max) ? max : value;
}

/* Clamp value to [0, max] for int. */
static inline int clamp_int(int value, int max)
{
    if (value < 0)
    {
        return 0;
    }
    return (value > max) ? max : value;
}

/* Cubic kernel for bicubic interpolation. */
static inline float cubic_kernel(float x)
{
    x = fabsf(x);
    if (x <= 1.0f)
    {
        return 1.5f * x * x * x - 2.5f * x * x + 1.0f;
    }
    else if (x <= 2.0f)
    {
        return -0.5f * x * x * x + 2.5f * x * x - 4.0f * x + 2.0f;
    }
    return 0.0f;
}

/* Lanczos kernel. */
static inline float lanczos_kernel(float x, int a)
{
    if (x == 0.0f)
    {
        return 1.0f;
    }
    if (fabsf(x) >= (float)a)
    {
        return 0.0f;
    }
    const float pi_x = (float)M_PI * x;
    return (float)a * sinf(pi_x) * sinf(pi_x / (float)a) / (pi_x * pi_x);
}

/*
 * Pixel format descriptor structure.
 * Describes how to read/write pixels for a specific format.
 */
struct pixel_format_desc
{
    unsigned bytes_per_pixel;
    unsigned channels;                          /* 1=grayscale, 2=grayscale+alpha, 3=RGB, 4=RGBA */
    int r_offset, g_offset, b_offset, a_offset; /* Byte offsets, -1 if channel doesn't exist. */
    bool is_16bit;                              /* true for 16-bit per channel formats */
    bool is_64bit;                              /* true for 64-bit formats (16-bit per channel RGBA) */
};

/*
 * Template macros for generating pixel sampling functions.
 */

#ifdef _MSC_VER
/*
 * MSVC warning C4127: conditional expression is constant. For example:
 *
 * if (R_OFF >= 0) becomes if (0 >= 0)
 *
 * It's intentional in template macros, so we need to disable the warning.
 */
#pragma warning(push)
#pragma warning(disable: 4127)
#endif

/* Sample pixel from format with channel offsets (8-bit channels). */
#define SAMPLE_PIXEL_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL, R_OFF, G_OFF, B_OFF, A_OFF, IS_16BIT)                        \
    static inline void FUNC_NAME(const uint8_t* pixels, unsigned width, unsigned height, unsigned bytes_per_line,      \
                                 int x, int y, uint8_t* r_out, uint8_t* g_out, uint8_t* b_out, uint8_t* a_out)         \
    {                                                                                                                  \
        x                    = clamp_int(x, (int)width - 1);                                                           \
        y                    = clamp_int(y, (int)height - 1);                                                          \
        const uint8_t* pixel = pixels + y * bytes_per_line + x * BYTES_PER_PIXEL;                                      \
        if (IS_16BIT)                                                                                                  \
        {                                                                                                              \
            /* 16-bit per channel: take high byte (MSB) for 8-bit output. */                                            \
            if (R_OFF >= 0)                                                                                            \
                *r_out = pixel[R_OFF + 1];                                                                             \
            if (G_OFF >= 0)                                                                                            \
                *g_out = pixel[G_OFF + 1];                                                                             \
            if (B_OFF >= 0)                                                                                            \
                *b_out = pixel[B_OFF + 1];                                                                             \
            if (A_OFF >= 0)                                                                                            \
                *a_out = pixel[A_OFF + 1];                                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            if (R_OFF >= 0)                                                                                            \
                *r_out = pixel[R_OFF];                                                                                 \
            if (G_OFF >= 0)                                                                                            \
                *g_out = pixel[G_OFF];                                                                                 \
            if (B_OFF >= 0)                                                                                            \
                *b_out = pixel[B_OFF];                                                                                 \
            if (A_OFF >= 0)                                                                                            \
                *a_out = pixel[A_OFF];                                                                                 \
        }                                                                                                              \
    }

/* Sample grayscale pixel. */
#define SAMPLE_GRAYSCALE_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL)                                                          \
    static inline uint8_t FUNC_NAME(const uint8_t* pixels, unsigned width, unsigned height, unsigned bytes_per_line,   \
                                    int x, int y)                                                                      \
    {                                                                                                                  \
        x                    = clamp_int(x, (int)width - 1);                                                           \
        y                    = clamp_int(y, (int)height - 1);                                                          \
        const uint8_t* pixel = pixels + y * bytes_per_line + x * BYTES_PER_PIXEL;                                      \
        return pixel[0];                                                                                               \
    }

/* Sample grayscale+alpha pixel. */
#define SAMPLE_GRAYSCALE_ALPHA_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL)                                                    \
    static inline void FUNC_NAME(const uint8_t* pixels, unsigned width, unsigned height, unsigned bytes_per_line,      \
                                 int x, int y, uint8_t* g_out, uint8_t* a_out)                                         \
    {                                                                                                                  \
        x                    = clamp_int(x, (int)width - 1);                                                           \
        y                    = clamp_int(y, (int)height - 1);                                                          \
        const uint8_t* pixel = pixels + y * bytes_per_line + x * BYTES_PER_PIXEL;                                      \
        *g_out               = pixel[0];                                                                               \
        *a_out               = pixel[BYTES_PER_PIXEL / 2];                                                             \
    }

/* Sample grayscale+alpha pixel (8-bit: 1 byte grayscale + 1 byte alpha). */
static inline void sample_grayscale_alpha8(const uint8_t* pixels,
                                           unsigned width,
                                           unsigned height,
                                           unsigned bytes_per_line,
                                           int x,
                                           int y,
                                           uint8_t* g_out,
                                           uint8_t* a_out)
{
    x                    = clamp_int(x, (int)width - 1);
    y                    = clamp_int(y, (int)height - 1);
    const uint8_t* pixel = pixels + y * bytes_per_line + x * 2;
    *g_out               = pixel[0];
    *a_out               = pixel[1];
}

/* Sample grayscale+alpha pixel (16-bit: 2 bytes grayscale + 2 bytes alpha). */
static inline void sample_grayscale_alpha16(const uint8_t* pixels,
                                            unsigned width,
                                            unsigned height,
                                            unsigned bytes_per_line,
                                            int x,
                                            int y,
                                            uint8_t* g_out,
                                            uint8_t* a_out)
{
    x                    = clamp_int(x, (int)width - 1);
    y                    = clamp_int(y, (int)height - 1);
    const uint8_t* pixel = pixels + y * bytes_per_line + x * 4;
    *g_out               = pixel[0];
    *a_out               = pixel[2];
}

/* Sample grayscale+alpha pixel (32-bit: 2 bytes grayscale + 2 bytes alpha, but stored as 4 bytes). */
static inline void sample_grayscale_alpha32(const uint8_t* pixels,
                                            unsigned width,
                                            unsigned height,
                                            unsigned bytes_per_line,
                                            int x,
                                            int y,
                                            uint8_t* g_out,
                                            uint8_t* a_out)
{
    x                    = clamp_int(x, (int)width - 1);
    y                    = clamp_int(y, (int)height - 1);
    const uint8_t* pixel = pixels + y * bytes_per_line + x * 8;
    *g_out               = pixel[0];
    *a_out               = pixel[4];
}

/* Write pixel to format with channel offsets (8-bit channels). */
#define WRITE_PIXEL_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL, R_OFF, G_OFF, B_OFF, A_OFF)                                   \
    static inline void FUNC_NAME(uint8_t* pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t a)                           \
    {                                                                                                                  \
        if (R_OFF >= 0)                                                                                                \
            pixel[R_OFF] = r;                                                                                          \
        if (G_OFF >= 0)                                                                                                \
            pixel[G_OFF] = g;                                                                                          \
        if (B_OFF >= 0)                                                                                                \
            pixel[B_OFF] = b;                                                                                          \
        if (A_OFF >= 0)                                                                                                \
            pixel[A_OFF] = a;                                                                                          \
    }

/* Write pixel to format with channel offsets (16-bit channels). */
#define WRITE_PIXEL_16BIT_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL, R_OFF, G_OFF, B_OFF, A_OFF)                             \
    static inline void FUNC_NAME(uint8_t* pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t a)                           \
    {                                                                                                                  \
        /* 16-bit per channel: duplicate byte for both LSB and MSB. */                                                  \
        if (R_OFF >= 0)                                                                                                \
        {                                                                                                              \
            pixel[R_OFF]     = r;                                                                                      \
            pixel[R_OFF + 1] = r;                                                                                      \
        }                                                                                                              \
        if (G_OFF >= 0)                                                                                                \
        {                                                                                                              \
            pixel[G_OFF]     = g;                                                                                      \
            pixel[G_OFF + 1] = g;                                                                                      \
        }                                                                                                              \
        if (B_OFF >= 0)                                                                                                \
        {                                                                                                              \
            pixel[B_OFF]     = b;                                                                                      \
            pixel[B_OFF + 1] = b;                                                                                      \
        }                                                                                                              \
        if (A_OFF >= 0)                                                                                                \
        {                                                                                                              \
            pixel[A_OFF]     = a;                                                                                      \
            pixel[A_OFF + 1] = a;                                                                                      \
        }                                                                                                              \
    }

/* Write grayscale pixel. */
#define WRITE_GRAYSCALE_TEMPLATE(FUNC_NAME, BYTES_PER_PIXEL)                                                           \
    static inline void FUNC_NAME(uint8_t* pixel, uint8_t g)                                                            \
    {                                                                                                                  \
        pixel[0] = g;                                                                                                  \
    }

/* Write grayscale+alpha pixel. */
static inline void write_grayscale_alpha8(uint8_t* pixel, uint8_t g, uint8_t a)
{
    pixel[0] = g;
    pixel[1] = a;
}

static inline void write_grayscale_alpha16(uint8_t* pixel, uint8_t g, uint8_t a)
{
    pixel[0] = g;
    pixel[2] = a;
}

static inline void write_grayscale_alpha32(uint8_t* pixel, uint8_t g, uint8_t a)
{
    pixel[0] = g;
    pixel[4] = a;
}

/*
 * Generate sampling functions for supported formats.
 */

/* Grayscale formats */
SAMPLE_GRAYSCALE_TEMPLATE(sample_grayscale8, 1)
SAMPLE_GRAYSCALE_TEMPLATE(sample_grayscale16, 2)

/* Grayscale+Alpha formats - defined as inline functions above. */

/* RGB24/BGR24 */
SAMPLE_PIXEL_TEMPLATE(sample_rgb24, 3, 0, 1, 2, -1, false)
SAMPLE_PIXEL_TEMPLATE(sample_bgr24, 3, 2, 1, 0, -1, false)

/* RGB48/BGR48 */
SAMPLE_PIXEL_TEMPLATE(sample_rgb48, 6, 0, 2, 4, -1, true)
SAMPLE_PIXEL_TEMPLATE(sample_bgr48, 6, 4, 2, 0, -1, true)

/* RGBA32 variants */
SAMPLE_PIXEL_TEMPLATE(sample_rgba32, 4, 0, 1, 2, 3, false)
SAMPLE_PIXEL_TEMPLATE(sample_bgra32, 4, 2, 1, 0, 3, false)
SAMPLE_PIXEL_TEMPLATE(sample_argb32, 4, 1, 2, 3, 0, false)
SAMPLE_PIXEL_TEMPLATE(sample_abgr32, 4, 3, 2, 1, 0, false)

/* RGBX32 variants (X = unused). */
SAMPLE_PIXEL_TEMPLATE(sample_rgbx32, 4, 0, 1, 2, -1, false)
SAMPLE_PIXEL_TEMPLATE(sample_bgrx32, 4, 2, 1, 0, -1, false)
SAMPLE_PIXEL_TEMPLATE(sample_xrgb32, 4, 1, 2, 3, -1, false)
SAMPLE_PIXEL_TEMPLATE(sample_xbgr32, 4, 3, 2, 1, -1, false)

/* RGBA64 variants */
SAMPLE_PIXEL_TEMPLATE(sample_rgba64, 8, 0, 2, 4, 6, true)
SAMPLE_PIXEL_TEMPLATE(sample_bgra64, 8, 4, 2, 0, 6, true)

/*
 * Generate write functions for supported formats.
 */

/* Grayscale formats */
WRITE_GRAYSCALE_TEMPLATE(write_grayscale8, 1)
WRITE_GRAYSCALE_TEMPLATE(write_grayscale16, 2)

/* Grayscale+Alpha formats - defined as inline functions above. */

/* RGB24/BGR24 */
WRITE_PIXEL_TEMPLATE(write_rgb24, 3, 0, 1, 2, -1)
WRITE_PIXEL_TEMPLATE(write_bgr24, 3, 2, 1, 0, -1)

/* RGB48/BGR48 */
WRITE_PIXEL_16BIT_TEMPLATE(write_rgb48, 6, 0, 2, 4, -1)
WRITE_PIXEL_16BIT_TEMPLATE(write_bgr48, 6, 4, 2, 0, -1)

/* RGBA32 variants */
WRITE_PIXEL_TEMPLATE(write_rgba32, 4, 0, 1, 2, 3)
WRITE_PIXEL_TEMPLATE(write_bgra32, 4, 2, 1, 0, 3)
WRITE_PIXEL_TEMPLATE(write_argb32, 4, 1, 2, 3, 0)
WRITE_PIXEL_TEMPLATE(write_abgr32, 4, 3, 2, 1, 0)

/* RGBX32 variants */
WRITE_PIXEL_TEMPLATE(write_rgbx32, 4, 0, 1, 2, -1)
WRITE_PIXEL_TEMPLATE(write_bgrx32, 4, 2, 1, 0, -1)
WRITE_PIXEL_TEMPLATE(write_xrgb32, 4, 1, 2, 3, -1)
WRITE_PIXEL_TEMPLATE(write_xbgr32, 4, 3, 2, 1, -1)

/* RGBA64 variants */
WRITE_PIXEL_16BIT_TEMPLATE(write_rgba64, 8, 0, 2, 4, 6)
WRITE_PIXEL_16BIT_TEMPLATE(write_bgra64, 8, 4, 2, 0, 6)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/*
 * Template macros for generating scaling functions.
 */

/* Nearest neighbor scaling template. */
#define SCALE_NEAREST_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                                    \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const int src_y   = (int)((double)row * y_scale + 0.5);                                                    \
            uint8_t* dst_scan = dst_pixels + row * dst_bytes_per_line;                                                 \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const int src_x = (int)((double)col * x_scale + 0.5);                                                  \
                uint8_t r, g, b, a;                                                                                    \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, src_x, src_y, &r, &g, &b, &a);      \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, r, g, b, a);                                              \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Nearest neighbor for grayscale. */
#define SCALE_NEAREST_GRAYSCALE_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                          \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const int src_y   = (int)((double)row * y_scale + 0.5);                                                    \
            uint8_t* dst_scan = dst_pixels + row * dst_bytes_per_line;                                                 \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const int src_x = (int)((double)col * x_scale + 0.5);                                                  \
                uint8_t g       = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, src_x, src_y);    \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, g);                                                       \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Nearest neighbor for grayscale+alpha. */
#define SCALE_NEAREST_GRAYSCALE_ALPHA_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                    \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const int src_y   = (int)((double)row * y_scale + 0.5);                                                    \
            uint8_t* dst_scan = dst_pixels + row * dst_bytes_per_line;                                                 \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const int src_x = (int)((double)col * x_scale + 0.5);                                                  \
                uint8_t g, a;                                                                                          \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, src_x, src_y, &g, &a);              \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, g, a);                                                    \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Bilinear scaling template for RGB/RGBA. */
#define SCALE_BILINEAR_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                                   \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const double src_y = (double)row * y_scale;                                                                \
            const int y0       = (int)src_y;                                                                           \
            const int y1       = clamp_unsigned(y0 + 1, src_height - 1);                                               \
            const float dy     = (float)(src_y - (double)y0);                                                          \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const double src_x = (double)col * x_scale;                                                            \
                const int x0       = (int)src_x;                                                                       \
                const int x1       = clamp_unsigned(x0 + 1, src_width - 1);                                            \
                const float dx     = (float)(src_x - (double)x0);                                                      \
                uint8_t r00, g00, b00, a00;                                                                            \
                uint8_t r01, g01, b01, a01;                                                                            \
                uint8_t r10, g10, b10, a10;                                                                            \
                uint8_t r11, g11, b11, a11;                                                                            \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y0, &r00, &g00, &b00, &a00);    \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y0, &r01, &g01, &b01, &a01);    \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y1, &r10, &g10, &b10, &a10);    \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y1, &r11, &g11, &b11, &a11);    \
                const float w00 = (1.0f - dx) * (1.0f - dy);                                                           \
                const float w01 = dx * (1.0f - dy);                                                                    \
                const float w10 = (1.0f - dx) * dy;                                                                    \
                const float w11 = dx * dy;                                                                             \
                const float r   = (float)r00 * w00 + (float)r01 * w01 + (float)r10 * w10 + (float)r11 * w11;           \
                const float g   = (float)g00 * w00 + (float)g01 * w01 + (float)g10 * w10 + (float)g11 * w11;           \
                const float b   = (float)b00 * w00 + (float)b01 * w01 + (float)b10 * w10 + (float)b11 * w11;           \
                const float a   = (float)a00 * w00 + (float)a01 * w01 + (float)a10 * w10 + (float)a11 * w11;           \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(r + 0.5f), (uint8_t)(g + 0.5f),                 \
                           (uint8_t)(b + 0.5f), (uint8_t)(a + 0.5f));                                                  \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Bilinear scaling template for grayscale. */
#define SCALE_BILINEAR_GRAYSCALE_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                         \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const double src_y = (double)row * y_scale;                                                                \
            const int y0       = (int)src_y;                                                                           \
            const int y1       = clamp_unsigned(y0 + 1, src_height - 1);                                               \
            const float dy     = (float)(src_y - (double)y0);                                                          \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const double src_x = (double)col * x_scale;                                                            \
                const int x0       = (int)src_x;                                                                       \
                const int x1       = clamp_unsigned(x0 + 1, src_width - 1);                                            \
                const float dx     = (float)(src_x - (double)x0);                                                      \
                const uint8_t g00  = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y0);       \
                const uint8_t g01  = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y0);       \
                const uint8_t g10  = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y1);       \
                const uint8_t g11  = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y1);       \
                const float w00    = (1.0f - dx) * (1.0f - dy);                                                        \
                const float w01    = dx * (1.0f - dy);                                                                 \
                const float w10    = (1.0f - dx) * dy;                                                                 \
                const float w11    = dx * dy;                                                                          \
                const float g      = (float)g00 * w00 + (float)g01 * w01 + (float)g10 * w10 + (float)g11 * w11;        \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(g + 0.5f));                                     \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Bilinear scaling template for grayscale+alpha. */
#define SCALE_BILINEAR_GRAYSCALE_ALPHA_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                   \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const double src_y = (double)row * y_scale;                                                                \
            const int y0       = (int)src_y;                                                                           \
            const int y1       = clamp_unsigned(y0 + 1, src_height - 1);                                               \
            const float dy     = (float)(src_y - (double)y0);                                                          \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const double src_x = (double)col * x_scale;                                                            \
                const int x0       = (int)src_x;                                                                       \
                const int x1       = clamp_unsigned(x0 + 1, src_width - 1);                                            \
                const float dx     = (float)(src_x - (double)x0);                                                      \
                uint8_t g00, a00, g01, a01, g10, a10, g11, a11;                                                        \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y0, &g00, &a00);                \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y0, &g01, &a01);                \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x0, y1, &g10, &a10);                \
                SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x1, y1, &g11, &a11);                \
                const float w00 = (1.0f - dx) * (1.0f - dy);                                                           \
                const float w01 = dx * (1.0f - dy);                                                                    \
                const float w10 = (1.0f - dx) * dy;                                                                    \
                const float w11 = dx * dy;                                                                             \
                const float g   = (float)g00 * w00 + (float)g01 * w01 + (float)g10 * w10 + (float)g11 * w11;           \
                const float a   = (float)a00 * w00 + (float)a01 * w01 + (float)a10 * w10 + (float)a11 * w11;           \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(g + 0.5f), (uint8_t)(a + 0.5f));                \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Bicubic scaling template for RGB/RGBA. */
#define SCALE_BICUBIC_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                                    \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const double src_y = (double)row * y_scale;                                                                \
            const int y0       = (int)floor(src_y);                                                                    \
            const float dy     = (float)(src_y - (double)y0);                                                          \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const double src_x = (double)col * x_scale;                                                            \
                const int x0       = (int)floor(src_x);                                                                \
                const float dx     = (float)(src_x - (double)x0);                                                      \
                float r_sum = 0.0f, g_sum = 0.0f, b_sum = 0.0f, a_sum = 0.0f;                                          \
                float weight_sum = 0.0f;                                                                               \
                for (int j = -1; j <= 2; j++)                                                                          \
                {                                                                                                      \
                    const int y    = clamp_unsigned(y0 + j, src_height - 1);                                           \
                    const float wy = cubic_kernel((float)j - dy);                                                      \
                    for (int i = -1; i <= 2; i++)                                                                      \
                    {                                                                                                  \
                        const int x        = clamp_unsigned(x0 + i, src_width - 1);                                    \
                        const float wx     = cubic_kernel((float)i - dx);                                              \
                        const float weight = wx * wy;                                                                  \
                        uint8_t r, g, b, a;                                                                            \
                        SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x, y, &r, &g, &b, &a);      \
                        r_sum      += (float)r * weight;                                                               \
                        g_sum      += (float)g * weight;                                                               \
                        b_sum      += (float)b * weight;                                                               \
                        a_sum      += (float)a * weight;                                                               \
                        weight_sum += weight;                                                                          \
                    }                                                                                                  \
                }                                                                                                      \
                if (weight_sum > 0.0f)                                                                                 \
                {                                                                                                      \
                    r_sum /= weight_sum;                                                                               \
                    g_sum /= weight_sum;                                                                               \
                    b_sum /= weight_sum;                                                                               \
                    a_sum /= weight_sum;                                                                               \
                }                                                                                                      \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(r_sum + 0.5f), (uint8_t)(g_sum + 0.5f),         \
                           (uint8_t)(b_sum + 0.5f), (uint8_t)(a_sum + 0.5f));                                          \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Bicubic scaling template for grayscale. */
#define SCALE_BICUBIC_GRAYSCALE_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                           \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                  \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,                \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                    \
    {                                                                                                                   \
        const double x_scale = (double)src_width / (double)dst_width;                                                   \
        const double y_scale = (double)src_height / (double)dst_height;                                                 \
        unsigned row;                                                                                                   \
        SAIL_OMP_PARALLEL_FOR                                                                                           \
        for (row = 0; row < dst_height; row++)                                                                          \
        {                                                                                                               \
            const double src_y = (double)row * y_scale;                                                                 \
            const int y0       = (int)floor(src_y);                                                                     \
            const float dy     = (float)(src_y - (double)y0);                                                           \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                 \
            for (unsigned col = 0; col < dst_width; col++)                                                              \
            {                                                                                                           \
                const double src_x = (double)col * x_scale;                                                             \
                const int x0       = (int)floor(src_x);                                                                 \
                const float dx     = (float)(src_x - (double)x0);                                                       \
                float g_sum        = 0.0f;                                                                              \
                float weight_sum   = 0.0f;                                                                              \
                for (int j = -1; j <= 2; j++)                                                                           \
                {                                                                                                       \
                    const int y    = clamp_unsigned(y0 + j, src_height - 1);                                            \
                    const float wy = cubic_kernel((float)j - dy);                                                       \
                    for (int i = -1; i <= 2; i++)                                                                       \
                    {                                                                                                   \
                        const int x         = clamp_unsigned(x0 + i, src_width - 1);                                    \
                        const float wx      = cubic_kernel((float)i - dx);                                              \
                        const float weight  = wx * wy;                                                                  \
                        const uint8_t g     = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x, y); \
                        g_sum              += (float)g * weight;                                                        \
                        weight_sum         += weight;                                                                   \
                    }                                                                                                   \
                }                                                                                                       \
                if (weight_sum > 0.0f)                                                                                  \
                {                                                                                                       \
                    g_sum /= weight_sum;                                                                                \
                }                                                                                                       \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(g_sum + 0.5f));                                  \
            }                                                                                                           \
        }                                                                                                               \
        return SAIL_OK;                                                                                                 \
    }

/* Lanczos scaling template for RGB/RGBA. */
#define SCALE_LANCZOS_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                                    \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                 \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,               \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                   \
    {                                                                                                                  \
        const int lanczos_a  = 3;                                                                                      \
        const double x_scale = (double)src_width / (double)dst_width;                                                  \
        const double y_scale = (double)src_height / (double)dst_height;                                                \
        unsigned row;                                                                                                  \
        SAIL_OMP_PARALLEL_FOR                                                                                          \
        for (row = 0; row < dst_height; row++)                                                                         \
        {                                                                                                              \
            const double src_y = (double)row * y_scale;                                                                \
            const int y0       = (int)floor(src_y);                                                                    \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                \
            for (unsigned col = 0; col < dst_width; col++)                                                             \
            {                                                                                                          \
                const double src_x = (double)col * x_scale;                                                            \
                const int x0       = (int)floor(src_x);                                                                \
                float r_sum = 0.0f, g_sum = 0.0f, b_sum = 0.0f, a_sum = 0.0f;                                          \
                float weight_sum = 0.0f;                                                                               \
                for (int j = -lanczos_a + 1; j <= lanczos_a; j++)                                                      \
                {                                                                                                      \
                    const int y    = clamp_unsigned(y0 + j, src_height - 1);                                           \
                    const float wy = lanczos_kernel((float)j + (float)(src_y - (double)y0), lanczos_a);                \
                    if (wy == 0.0f)                                                                                    \
                        continue;                                                                                      \
                    for (int i = -lanczos_a + 1; i <= lanczos_a; i++)                                                  \
                    {                                                                                                  \
                        const int x    = clamp_unsigned(x0 + i, src_width - 1);                                        \
                        const float wx = lanczos_kernel((float)i + (float)(src_x - (double)x0), lanczos_a);            \
                        if (wx == 0.0f)                                                                                \
                            continue;                                                                                  \
                        const float weight = wx * wy;                                                                  \
                        uint8_t r, g, b, a;                                                                            \
                        SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x, y, &r, &g, &b, &a);      \
                        r_sum      += (float)r * weight;                                                               \
                        g_sum      += (float)g * weight;                                                               \
                        b_sum      += (float)b * weight;                                                               \
                        a_sum      += (float)a * weight;                                                               \
                        weight_sum += weight;                                                                          \
                    }                                                                                                  \
                }                                                                                                      \
                if (weight_sum > 0.0f)                                                                                 \
                {                                                                                                      \
                    r_sum /= weight_sum;                                                                               \
                    g_sum /= weight_sum;                                                                               \
                    b_sum /= weight_sum;                                                                               \
                    a_sum /= weight_sum;                                                                               \
                }                                                                                                      \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(r_sum + 0.5f), (uint8_t)(g_sum + 0.5f),         \
                           (uint8_t)(b_sum + 0.5f), (uint8_t)(a_sum + 0.5f));                                          \
            }                                                                                                          \
        }                                                                                                              \
        return SAIL_OK;                                                                                                \
    }

/* Lanczos scaling template for grayscale. */
#define SCALE_LANCZOS_GRAYSCALE_TEMPLATE(FUNC_NAME, SAMPLE_FUNC, WRITE_FUNC, BYTES_PER_PIXEL)                           \
    static sail_status_t FUNC_NAME(const uint8_t* src_pixels, unsigned src_width, unsigned src_height,                  \
                                   unsigned src_bytes_per_line, uint8_t* dst_pixels, unsigned dst_width,                \
                                   unsigned dst_height, unsigned dst_bytes_per_line)                                    \
    {                                                                                                                   \
        const int lanczos_a  = 3;                                                                                       \
        const double x_scale = (double)src_width / (double)dst_width;                                                   \
        const double y_scale = (double)src_height / (double)dst_height;                                                 \
        unsigned row;                                                                                                   \
        SAIL_OMP_PARALLEL_FOR                                                                                           \
        for (row = 0; row < dst_height; row++)                                                                          \
        {                                                                                                               \
            const double src_y = (double)row * y_scale;                                                                 \
            const int y0       = (int)floor(src_y);                                                                     \
            uint8_t* dst_scan  = dst_pixels + row * dst_bytes_per_line;                                                 \
            for (unsigned col = 0; col < dst_width; col++)                                                              \
            {                                                                                                           \
                const double src_x = (double)col * x_scale;                                                             \
                const int x0       = (int)floor(src_x);                                                                 \
                float g_sum        = 0.0f;                                                                              \
                float weight_sum   = 0.0f;                                                                              \
                for (int j = -lanczos_a + 1; j <= lanczos_a; j++)                                                       \
                {                                                                                                       \
                    const int y    = clamp_unsigned(y0 + j, src_height - 1);                                            \
                    const float wy = lanczos_kernel((float)j + (float)(src_y - (double)y0), lanczos_a);                 \
                    if (wy == 0.0f)                                                                                     \
                        continue;                                                                                       \
                    for (int i = -lanczos_a + 1; i <= lanczos_a; i++)                                                   \
                    {                                                                                                   \
                        const int x    = clamp_unsigned(x0 + i, src_width - 1);                                         \
                        const float wx = lanczos_kernel((float)i + (float)(src_x - (double)x0), lanczos_a);             \
                        if (wx == 0.0f)                                                                                 \
                            continue;                                                                                   \
                        const float weight  = wx * wy;                                                                  \
                        const uint8_t g     = SAMPLE_FUNC(src_pixels, src_width, src_height, src_bytes_per_line, x, y); \
                        g_sum              += (float)g * weight;                                                        \
                        weight_sum         += weight;                                                                   \
                    }                                                                                                   \
                }                                                                                                       \
                if (weight_sum > 0.0f)                                                                                  \
                {                                                                                                       \
                    g_sum /= weight_sum;                                                                                \
                }                                                                                                       \
                WRITE_FUNC(dst_scan + col * BYTES_PER_PIXEL, (uint8_t)(g_sum + 0.5f));                                  \
            }                                                                                                           \
        }                                                                                                               \
        return SAIL_OK;                                                                                                 \
    }

/*
 * Generate scaling functions for all supported formats.
 * Priority 1: RGB24, Grayscale8, Grayscale16
 */

/* Grayscale8 */
SCALE_NEAREST_GRAYSCALE_TEMPLATE(scale_nearest_grayscale8, sample_grayscale8, write_grayscale8, 1)
SCALE_BILINEAR_GRAYSCALE_TEMPLATE(scale_bilinear_grayscale8, sample_grayscale8, write_grayscale8, 1)
SCALE_BICUBIC_GRAYSCALE_TEMPLATE(scale_bicubic_grayscale8, sample_grayscale8, write_grayscale8, 1)
SCALE_LANCZOS_GRAYSCALE_TEMPLATE(scale_lanczos_grayscale8, sample_grayscale8, write_grayscale8, 1)

/* Grayscale16 */
SCALE_NEAREST_GRAYSCALE_TEMPLATE(scale_nearest_grayscale16, sample_grayscale16, write_grayscale16, 2)
SCALE_BILINEAR_GRAYSCALE_TEMPLATE(scale_bilinear_grayscale16, sample_grayscale16, write_grayscale16, 2)
SCALE_BICUBIC_GRAYSCALE_TEMPLATE(scale_bicubic_grayscale16, sample_grayscale16, write_grayscale16, 2)
SCALE_LANCZOS_GRAYSCALE_TEMPLATE(scale_lanczos_grayscale16, sample_grayscale16, write_grayscale16, 2)

/* RGB24 */
SCALE_NEAREST_TEMPLATE(scale_nearest_rgb24, sample_rgb24, write_rgb24, 3)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_rgb24, sample_rgb24, write_rgb24, 3)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_rgb24, sample_rgb24, write_rgb24, 3)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_rgb24, sample_rgb24, write_rgb24, 3)

/* BGR24 */
SCALE_NEAREST_TEMPLATE(scale_nearest_bgr24, sample_bgr24, write_bgr24, 3)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_bgr24, sample_bgr24, write_bgr24, 3)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_bgr24, sample_bgr24, write_bgr24, 3)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_bgr24, sample_bgr24, write_bgr24, 3)

/* Priority 2: RGBA32 variants */
SCALE_NEAREST_TEMPLATE(scale_nearest_rgba32, sample_rgba32, write_rgba32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_rgba32, sample_rgba32, write_rgba32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_rgba32, sample_rgba32, write_rgba32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_rgba32, sample_rgba32, write_rgba32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_bgra32, sample_bgra32, write_bgra32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_bgra32, sample_bgra32, write_bgra32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_bgra32, sample_bgra32, write_bgra32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_bgra32, sample_bgra32, write_bgra32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_argb32, sample_argb32, write_argb32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_argb32, sample_argb32, write_argb32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_argb32, sample_argb32, write_argb32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_argb32, sample_argb32, write_argb32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_abgr32, sample_abgr32, write_abgr32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_abgr32, sample_abgr32, write_abgr32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_abgr32, sample_abgr32, write_abgr32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_abgr32, sample_abgr32, write_abgr32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_rgbx32, sample_rgbx32, write_rgbx32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_rgbx32, sample_rgbx32, write_rgbx32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_rgbx32, sample_rgbx32, write_rgbx32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_rgbx32, sample_rgbx32, write_rgbx32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_bgrx32, sample_bgrx32, write_bgrx32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_bgrx32, sample_bgrx32, write_bgrx32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_bgrx32, sample_bgrx32, write_bgrx32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_bgrx32, sample_bgrx32, write_bgrx32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_xrgb32, sample_xrgb32, write_xrgb32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_xrgb32, sample_xrgb32, write_xrgb32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_xrgb32, sample_xrgb32, write_xrgb32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_xrgb32, sample_xrgb32, write_xrgb32, 4)

SCALE_NEAREST_TEMPLATE(scale_nearest_xbgr32, sample_xbgr32, write_xbgr32, 4)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_xbgr32, sample_xbgr32, write_xbgr32, 4)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_xbgr32, sample_xbgr32, write_xbgr32, 4)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_xbgr32, sample_xbgr32, write_xbgr32, 4)

/* Priority 3: RGB48 and Grayscale+Alpha */
SCALE_NEAREST_TEMPLATE(scale_nearest_rgb48, sample_rgb48, write_rgb48, 6)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_rgb48, sample_rgb48, write_rgb48, 6)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_rgb48, sample_rgb48, write_rgb48, 6)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_rgb48, sample_rgb48, write_rgb48, 6)

SCALE_NEAREST_TEMPLATE(scale_nearest_bgr48, sample_bgr48, write_bgr48, 6)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_bgr48, sample_bgr48, write_bgr48, 6)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_bgr48, sample_bgr48, write_bgr48, 6)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_bgr48, sample_bgr48, write_bgr48, 6)

SCALE_NEAREST_GRAYSCALE_ALPHA_TEMPLATE(scale_nearest_grayscale_alpha8,
                                       sample_grayscale_alpha8,
                                       write_grayscale_alpha8,
                                       2)
SCALE_BILINEAR_GRAYSCALE_ALPHA_TEMPLATE(scale_bilinear_grayscale_alpha8,
                                        sample_grayscale_alpha8,
                                        write_grayscale_alpha8,
                                        2)
SCALE_NEAREST_GRAYSCALE_ALPHA_TEMPLATE(scale_nearest_grayscale_alpha16,
                                       sample_grayscale_alpha16,
                                       write_grayscale_alpha16,
                                       4)
SCALE_BILINEAR_GRAYSCALE_ALPHA_TEMPLATE(scale_bilinear_grayscale_alpha16,
                                        sample_grayscale_alpha16,
                                        write_grayscale_alpha16,
                                        4)
SCALE_NEAREST_GRAYSCALE_ALPHA_TEMPLATE(scale_nearest_grayscale_alpha32,
                                       sample_grayscale_alpha32,
                                       write_grayscale_alpha32,
                                       8)
SCALE_BILINEAR_GRAYSCALE_ALPHA_TEMPLATE(scale_bilinear_grayscale_alpha32,
                                        sample_grayscale_alpha32,
                                        write_grayscale_alpha32,
                                        8)

SCALE_NEAREST_TEMPLATE(scale_nearest_rgba64, sample_rgba64, write_rgba64, 8)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_rgba64, sample_rgba64, write_rgba64, 8)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_rgba64, sample_rgba64, write_rgba64, 8)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_rgba64, sample_rgba64, write_rgba64, 8)

SCALE_NEAREST_TEMPLATE(scale_nearest_bgra64, sample_bgra64, write_bgra64, 8)
SCALE_BILINEAR_TEMPLATE(scale_bilinear_bgra64, sample_bgra64, write_bgra64, 8)
SCALE_BICUBIC_TEMPLATE(scale_bicubic_bgra64, sample_bgra64, write_bgra64, 8)
SCALE_LANCZOS_TEMPLATE(scale_lanczos_bgra64, sample_bgra64, write_bgra64, 8)

/*
 * Scaling function pointer type.
 */
typedef sail_status_t (*scale_func_t)(const uint8_t* src_pixels,
                                      unsigned src_width,
                                      unsigned src_height,
                                      unsigned src_bytes_per_line,
                                      uint8_t* dst_pixels,
                                      unsigned dst_width,
                                      unsigned dst_height,
                                      unsigned dst_bytes_per_line);

/*
 * Format dispatcher structure.
 */
struct format_dispatcher
{
    enum SailPixelFormat format;
    scale_func_t nearest;
    scale_func_t bilinear;
    scale_func_t bicubic;
    scale_func_t lanczos;
};

/*
 * Format dispatcher table.
 */
static const struct format_dispatcher format_dispatchers[] = {
    /* Priority 1: RGB24, Grayscale8, Grayscale16 */
    {SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE, scale_nearest_grayscale8, scale_bilinear_grayscale8, scale_bicubic_grayscale8,
     scale_lanczos_grayscale8},
    {SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE, scale_nearest_grayscale16, scale_bilinear_grayscale16,
     scale_bicubic_grayscale16, scale_lanczos_grayscale16},
    {SAIL_PIXEL_FORMAT_BPP24_RGB, scale_nearest_rgb24, scale_bilinear_rgb24, scale_bicubic_rgb24, scale_lanczos_rgb24},
    {SAIL_PIXEL_FORMAT_BPP24_BGR, scale_nearest_bgr24, scale_bilinear_bgr24, scale_bicubic_bgr24, scale_lanczos_bgr24},

    /* Priority 2: RGBA32 variants */
    {SAIL_PIXEL_FORMAT_BPP32_RGBA, scale_nearest_rgba32, scale_bilinear_rgba32, scale_bicubic_rgba32,
     scale_lanczos_rgba32},
    {SAIL_PIXEL_FORMAT_BPP32_BGRA, scale_nearest_bgra32, scale_bilinear_bgra32, scale_bicubic_bgra32,
     scale_lanczos_bgra32},
    {SAIL_PIXEL_FORMAT_BPP32_ARGB, scale_nearest_argb32, scale_bilinear_argb32, scale_bicubic_argb32,
     scale_lanczos_argb32},
    {SAIL_PIXEL_FORMAT_BPP32_ABGR, scale_nearest_abgr32, scale_bilinear_abgr32, scale_bicubic_abgr32,
     scale_lanczos_abgr32},
    {SAIL_PIXEL_FORMAT_BPP32_RGBX, scale_nearest_rgbx32, scale_bilinear_rgbx32, scale_bicubic_rgbx32,
     scale_lanczos_rgbx32},
    {SAIL_PIXEL_FORMAT_BPP32_BGRX, scale_nearest_bgrx32, scale_bilinear_bgrx32, scale_bicubic_bgrx32,
     scale_lanczos_bgrx32},
    {SAIL_PIXEL_FORMAT_BPP32_XRGB, scale_nearest_xrgb32, scale_bilinear_xrgb32, scale_bicubic_xrgb32,
     scale_lanczos_xrgb32},
    {SAIL_PIXEL_FORMAT_BPP32_XBGR, scale_nearest_xbgr32, scale_bilinear_xbgr32, scale_bicubic_xbgr32,
     scale_lanczos_xbgr32},

    /* Priority 3: RGB48 and Grayscale+Alpha */
    {SAIL_PIXEL_FORMAT_BPP48_RGB, scale_nearest_rgb48, scale_bilinear_rgb48, scale_bicubic_rgb48, scale_lanczos_rgb48},
    {SAIL_PIXEL_FORMAT_BPP48_BGR, scale_nearest_bgr48, scale_bilinear_bgr48, scale_bicubic_bgr48, scale_lanczos_bgr48},
    {SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA, scale_nearest_grayscale_alpha8, scale_bilinear_grayscale_alpha8, NULL,
     NULL},
    {SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA, scale_nearest_grayscale_alpha16, scale_bilinear_grayscale_alpha16, NULL,
     NULL},
    {SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA, scale_nearest_grayscale_alpha32, scale_bilinear_grayscale_alpha32, NULL,
     NULL},
    {SAIL_PIXEL_FORMAT_BPP64_RGBA, scale_nearest_rgba64, scale_bilinear_rgba64, scale_bicubic_rgba64,
     scale_lanczos_rgba64},
    {SAIL_PIXEL_FORMAT_BPP64_BGRA, scale_nearest_bgra64, scale_bilinear_bgra64, scale_bicubic_bgra64,
     scale_lanczos_bgra64},
};

#define FORMAT_DISPATCHER_COUNT (sizeof(format_dispatchers) / sizeof(format_dispatchers[0]))

/*
 * Find dispatcher for pixel format (manual scaling).
 */
static const struct format_dispatcher* find_dispatcher_manual(enum SailPixelFormat format)
{
    for (size_t i = 0; i < FORMAT_DISPATCHER_COUNT; i++)
    {
        if (format_dispatchers[i].format == format)
        {
            return &format_dispatchers[i];
        }
    }
    return NULL;
}

/* Scale using manual implementation (fallback). */
static sail_status_t scale_with_manual(const struct sail_image* src_image,
                                       struct sail_image* dst_image,
                                       enum SailScaling algorithm)
{
    /* Try to find direct format support. */
    const struct format_dispatcher* dispatcher = find_dispatcher_manual(src_image->pixel_format);

    if (dispatcher != NULL)
    {
        /* Direct format support - scale without conversion. */
        scale_func_t scale_func = NULL;

        switch (algorithm)
        {
        case SAIL_SCALING_NEAREST_NEIGHBOR:
            scale_func = dispatcher->nearest;
            break;
        case SAIL_SCALING_BILINEAR:
            scale_func = dispatcher->bilinear;
            break;
        case SAIL_SCALING_BICUBIC:
            scale_func = dispatcher->bicubic;
            break;
        case SAIL_SCALING_LANCZOS:
            scale_func = dispatcher->lanczos;
            break;
        default:
            SAIL_LOG_ERROR("Unsupported scaling algorithm for manual scaling");
            return SAIL_ERROR_INVALID_ARGUMENT;
        }

        if (scale_func == NULL)
        {
            SAIL_LOG_ERROR("Scaling algorithm not supported for this format");
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }

        /* Perform scaling. */
        return scale_func((const uint8_t*)src_image->pixels, src_image->width, src_image->height,
                          src_image->bytes_per_line, (uint8_t*)dst_image->pixels, dst_image->width, dst_image->height,
                          dst_image->bytes_per_line);
    }

    /* Fallback: convert to RGBA32/64, scale, then convert back. */
    const unsigned bits_per_pixel = sail_bits_per_pixel(src_image->pixel_format);
    const enum SailPixelFormat rgba_format =
        (bits_per_pixel > 32) ? SAIL_PIXEL_FORMAT_BPP64_RGBA : SAIL_PIXEL_FORMAT_BPP32_RGBA;

    /* Check if RGBA format is supported. */
    const struct format_dispatcher* rgba_dispatcher = find_dispatcher_manual(rgba_format);
    if (rgba_dispatcher == NULL)
    {
        SAIL_LOG_ERROR("RGBA format not supported for scaling fallback");
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    /* Convert to RGBA format for scaling. */
    struct sail_image* rgba_image = NULL;
    sail_status_t status          = sail_convert_image(src_image, rgba_format, &rgba_image);
    if (status != SAIL_OK)
    {
        return status;
    }

    /* Create RGBA output image skeleton. */
    struct sail_image* rgba_output = NULL;
    status                         = sail_copy_image_skeleton(src_image, &rgba_output);
    if (status != SAIL_OK)
    {
        sail_destroy_image(rgba_image);
        return status;
    }

    rgba_output->width          = dst_image->width;
    rgba_output->height         = dst_image->height;
    rgba_output->pixel_format   = rgba_format;
    rgba_output->bytes_per_line = sail_bytes_per_line(dst_image->width, rgba_format);

    if (src_image->palette != NULL)
    {
        status = sail_copy_palette(src_image->palette, &rgba_output->palette);
        if (status != SAIL_OK)
        {
            sail_destroy_image(rgba_output);
            sail_destroy_image(rgba_image);
            return status;
        }
    }

    const size_t rgba_pixels_size = (size_t)rgba_output->height * rgba_output->bytes_per_line;
    status                        = sail_malloc(rgba_pixels_size, &rgba_output->pixels);
    if (status != SAIL_OK)
    {
        sail_destroy_image(rgba_output);
        sail_destroy_image(rgba_image);
        return status;
    }

    /* Scale RGBA image. */
    scale_func_t scale_func = NULL;

    switch (algorithm)
    {
    case SAIL_SCALING_NEAREST_NEIGHBOR:
        scale_func = rgba_dispatcher->nearest;
        break;
    case SAIL_SCALING_BILINEAR:
        scale_func = rgba_dispatcher->bilinear;
        break;
    case SAIL_SCALING_BICUBIC:
        scale_func = rgba_dispatcher->bicubic;
        break;
    case SAIL_SCALING_LANCZOS:
        scale_func = rgba_dispatcher->lanczos;
        break;
    default:
        sail_destroy_image(rgba_output);
        sail_destroy_image(rgba_image);
        SAIL_LOG_ERROR("Unsupported scaling algorithm");
        return SAIL_ERROR_INVALID_ARGUMENT;
    }

    if (scale_func == NULL)
    {
        sail_destroy_image(rgba_output);
        sail_destroy_image(rgba_image);
        SAIL_LOG_ERROR("Scaling algorithm not supported for RGBA format");
        return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    status = scale_func((const uint8_t*)rgba_image->pixels, rgba_image->width, rgba_image->height,
                        rgba_image->bytes_per_line, (uint8_t*)rgba_output->pixels, rgba_output->width,
                        rgba_output->height, rgba_output->bytes_per_line);

    sail_destroy_image(rgba_image);

    if (status != SAIL_OK)
    {
        sail_destroy_image(rgba_output);
        return status;
    }

    /* Convert back to original format if needed. */
    if (rgba_output->pixel_format != dst_image->pixel_format)
    {
        struct sail_image* converted = NULL;
        status                       = sail_convert_image(rgba_output, dst_image->pixel_format, &converted);
        sail_destroy_image(rgba_output);
        if (status != SAIL_OK)
        {
            return status;
        }
        rgba_output = converted;
    }

    /* Copy result to destination. */
    memcpy(dst_image->pixels, rgba_output->pixels, (size_t)dst_image->height * dst_image->bytes_per_line);
    sail_destroy_image(rgba_output);

    return SAIL_OK;
}
