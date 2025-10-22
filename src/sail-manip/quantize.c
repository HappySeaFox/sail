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

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "quantize.h"

// #define SAIL_ORIGINAL_WU_IMPLEMENTATION

/*
 * ============================================================================
 * XIAOLIN WU'S COLOR QUANTIZER (v. 2) - ORIGINAL CODE
 * ============================================================================
 * C Implementation of Wu's Color Quantizer (v. 2)
 * (see Graphics Gems vol. II, pp. 126-133)
 *
 * Author: Xiaolin Wu
 *         Dept. of Computer Science
 *         Univ. of Western Ontario
 *         London, Ontario N6A 5B7
 *         wu@csd.uwo.ca
 *
 * Algorithm: Greedy orthogonal bipartition of RGB space for variance
 *            minimization aided by inclusion-exclusion tricks.
 *            For speed no nearest neighbor search is done. Slightly
 *            better performance can be expected by more sophisticated
 *            but more expensive versions.
 *
 * Free to distribute, comments and suggestions are appreciated.
 * ============================================================================
 */

#define WU_MAXCOLOR 256
#define WU_RED 2
#define WU_GREEN 1
#define WU_BLUE 0

struct wu_box
{
    int r0; /* min value, exclusive */
    int r1; /* max value, inclusive */
    int g0;
    int g1;
    int b0;
    int b1;
    int vol;
};

/* Histogram is in elements 1..HISTSIZE along each axis,
 * element 0 is for base or marginal value
 * NB: these must start out 0!
 */
typedef struct wu_state
{
    float m2[33][33][33];
    long int wt[33][33][33];
    long int mr[33][33][33];
    long int mg[33][33][33];
    long int mb[33][33][33];
    unsigned char* Ir;
    unsigned char* Ig;
    unsigned char* Ib;
    int size; /* image size */
    int K;    /* color look-up table size */
    unsigned short int* Qadd;
} wu_state_t;

/* build 3-D color histogram of counts, r/g/b, c^2 */
static void wu_Hist3d(wu_state_t* state, long int* vwt, long int* vmr, long int* vmg, long int* vmb, float* m2)
{
    int ind, r, g, b;
    int inr, ing, inb, table[256];
    long int i;

    for (i = 0; i < 256; ++i)
    {
        table[i] = i * i;
    }

    state->Qadd = (unsigned short int*)malloc(sizeof(short int) * state->size);
    if (state->Qadd == NULL)
    {
        return;
    }

    for (i = 0; i < state->size; ++i)
    {
        r              = state->Ir[i];
        g              = state->Ig[i];
        b              = state->Ib[i];
        inr            = (r >> 3) + 1;
        ing            = (g >> 3) + 1;
        inb            = (b >> 3) + 1;
        state->Qadd[i] = ind = (inr << 10) + (inr << 6) + inr + (ing << 5) + ing + inb;
        /* [inr][ing][inb] */
        ++vwt[ind];
        vmr[ind] += r;
        vmg[ind] += g;
        vmb[ind] += b;
        m2[ind]  += (float)(table[r] + table[g] + table[b]);
    }
}

/* At conclusion of the histogram step, we can interpret
 *   wt[r][g][b] = sum over voxel of P(c)
 *   mr[r][g][b] = sum over voxel of r*P(c)  ,  similarly for mg, mb
 *   m2[r][g][b] = sum over voxel of c^2*P(c)
 * Actually each of these should be divided by 'size' to give the usual
 * interpretation of P() as ranging from 0 to 1, but we needn't do that here.
 */

/* We now convert histogram into moments so that we can rapidly calculate
 * the sums of the above quantities over any desired box.
 */

/* compute cumulative moments. */
static void wu_M3d(long int* vwt, long int* vmr, long int* vmg, long int* vmb, float* m2)
{
    unsigned short int ind1, ind2;
    unsigned char i, r, g, b;
    long int line, line_r, line_g, line_b;
    long int area[33], area_r[33], area_g[33], area_b[33];
    float line2, area2[33];

    for (r = 1; r <= 32; ++r)
    {
        for (i = 0; i <= 32; ++i)
        {
            area2[i] = area[i] = area_r[i] = area_g[i] = area_b[i] = 0;
        }
        for (g = 1; g <= 32; ++g)
        {
            line2 = line = line_r = line_g = line_b = 0;
            for (b = 1; b <= 32; ++b)
            {
                ind1       = (r << 10) + (r << 6) + r + (g << 5) + g + b; /* [r][g][b] */
                line      += vwt[ind1];
                line_r    += vmr[ind1];
                line_g    += vmg[ind1];
                line_b    += vmb[ind1];
                line2     += m2[ind1];
                area[b]   += line;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area2[b]  += line2;
                ind2       = ind1 - 1089; /* [r-1][g][b] */
                vwt[ind1]  = vwt[ind2] + area[b];
                vmr[ind1]  = vmr[ind2] + area_r[b];
                vmg[ind1]  = vmg[ind2] + area_g[b];
                vmb[ind1]  = vmb[ind2] + area_b[b];
                m2[ind1]   = m2[ind2] + area2[b];
            }
        }
    }
}

/* Compute sum over a box of any given statistic */
static long int wu_Vol(struct wu_box* cube, long int mmt[33][33][33])
{
    return (mmt[cube->r1][cube->g1][cube->b1] - mmt[cube->r1][cube->g1][cube->b0] - mmt[cube->r1][cube->g0][cube->b1]
            + mmt[cube->r1][cube->g0][cube->b0] - mmt[cube->r0][cube->g1][cube->b1] + mmt[cube->r0][cube->g1][cube->b0]
            + mmt[cube->r0][cube->g0][cube->b1] - mmt[cube->r0][cube->g0][cube->b0]);
}

/* The next two routines allow a slightly more efficient calculation
 * of Vol() for a proposed subbox of a given box.  The sum of Top()
 * and Bottom() is the Vol() of a subbox split in the given direction
 * and with the specified new upper bound.
 */

/* Compute part of Vol(cube, mmt) that doesn't depend on r1, g1, or b1 */
/* (depending on dir) */
static long int wu_Bottom(struct wu_box* cube, unsigned char dir, long int mmt[33][33][33])
{
    switch (dir)
    {
    case WU_RED:
        return (-mmt[cube->r0][cube->g1][cube->b1] + mmt[cube->r0][cube->g1][cube->b0]
                + mmt[cube->r0][cube->g0][cube->b1] - mmt[cube->r0][cube->g0][cube->b0]);
    case WU_GREEN:
        return (-mmt[cube->r1][cube->g0][cube->b1] + mmt[cube->r1][cube->g0][cube->b0]
                + mmt[cube->r0][cube->g0][cube->b1] - mmt[cube->r0][cube->g0][cube->b0]);
    case WU_BLUE:
        return (-mmt[cube->r1][cube->g1][cube->b0] + mmt[cube->r1][cube->g0][cube->b0]
                + mmt[cube->r0][cube->g1][cube->b0] - mmt[cube->r0][cube->g0][cube->b0]);
    }
    return 0;
}

/* Compute remainder of Vol(cube, mmt), substituting pos for */
/* r1, g1, or b1 (depending on dir) */
static long int wu_Top(struct wu_box* cube, unsigned char dir, int pos, long int mmt[33][33][33])
{
    switch (dir)
    {
    case WU_RED:
        return (mmt[pos][cube->g1][cube->b1] - mmt[pos][cube->g1][cube->b0] - mmt[pos][cube->g0][cube->b1]
                + mmt[pos][cube->g0][cube->b0]);
    case WU_GREEN:
        return (mmt[cube->r1][pos][cube->b1] - mmt[cube->r1][pos][cube->b0] - mmt[cube->r0][pos][cube->b1]
                + mmt[cube->r0][pos][cube->b0]);
    case WU_BLUE:
        return (mmt[cube->r1][cube->g1][pos] - mmt[cube->r1][cube->g0][pos] - mmt[cube->r0][cube->g1][pos]
                + mmt[cube->r0][cube->g0][pos]);
    }
    return 0;
}

/* Compute the weighted variance of a box */
/* NB: as with the raw statistics, this is really the variance * size */
static float wu_Var(struct wu_box* cube, wu_state_t* state)
{
    float dr, dg, db, xx;

    dr = wu_Vol(cube, state->mr);
    dg = wu_Vol(cube, state->mg);
    db = wu_Vol(cube, state->mb);
    xx = state->m2[cube->r1][cube->g1][cube->b1] - state->m2[cube->r1][cube->g1][cube->b0]
         - state->m2[cube->r1][cube->g0][cube->b1] + state->m2[cube->r1][cube->g0][cube->b0]
         - state->m2[cube->r0][cube->g1][cube->b1] + state->m2[cube->r0][cube->g1][cube->b0]
         + state->m2[cube->r0][cube->g0][cube->b1] - state->m2[cube->r0][cube->g0][cube->b0];

    return (xx - (dr * dr + dg * dg + db * db) / (float)wu_Vol(cube, state->wt));
}

/* We want to minimize the sum of the variances of two subboxes.
 * The sum(c^2) terms can be ignored since their sum over both subboxes
 * is the same (the sum for the whole box) no matter where we split.
 * The remaining terms have a minus sign in the variance formula,
 * so we drop the minus sign and MAXIMIZE the sum of the two terms.
 */

static float wu_Maximize(struct wu_box* cube,
                         unsigned char dir,
                         int first,
                         int last,
                         int* cut,
                         long int whole_r,
                         long int whole_g,
                         long int whole_b,
                         long int whole_w,
                         wu_state_t* state)
{
    long int half_r, half_g, half_b, half_w;
    long int base_r, base_g, base_b, base_w;
    int i;
    float temp, max;

    base_r = wu_Bottom(cube, dir, state->mr);
    base_g = wu_Bottom(cube, dir, state->mg);
    base_b = wu_Bottom(cube, dir, state->mb);
    base_w = wu_Bottom(cube, dir, state->wt);
    max    = 0.0;
    *cut   = -1;

    for (i = first; i < last; ++i)
    {
        half_r = base_r + wu_Top(cube, dir, i, state->mr);
        half_g = base_g + wu_Top(cube, dir, i, state->mg);
        half_b = base_b + wu_Top(cube, dir, i, state->mb);
        half_w = base_w + wu_Top(cube, dir, i, state->wt);
        /* now half_x is sum over lower half of box, if split at i */
        if (half_w == 0)
        {             /* subbox could be empty of pixels! */
            continue; /* never split into an empty box */
        }
        else
        {
            temp = ((float)half_r * half_r + (float)half_g * half_g + (float)half_b * half_b) / half_w;
        }

        half_r = whole_r - half_r;
        half_g = whole_g - half_g;
        half_b = whole_b - half_b;
        half_w = whole_w - half_w;
        if (half_w == 0)
        {             /* subbox could be empty of pixels! */
            continue; /* never split into an empty box */
        }
        else
        {
            temp += ((float)half_r * half_r + (float)half_g * half_g + (float)half_b * half_b) / half_w;
        }

        if (temp > max)
        {
            max  = temp;
            *cut = i;
        }
    }
    return (max);
}

static int wu_Cut(struct wu_box* set1, struct wu_box* set2, wu_state_t* state)
{
    unsigned char dir;
    int cutr, cutg, cutb;
    float maxr, maxg, maxb;
    long int whole_r, whole_g, whole_b, whole_w;

    whole_r = wu_Vol(set1, state->mr);
    whole_g = wu_Vol(set1, state->mg);
    whole_b = wu_Vol(set1, state->mb);
    whole_w = wu_Vol(set1, state->wt);

    maxr = wu_Maximize(set1, WU_RED, set1->r0 + 1, set1->r1, &cutr, whole_r, whole_g, whole_b, whole_w, state);
    maxg = wu_Maximize(set1, WU_GREEN, set1->g0 + 1, set1->g1, &cutg, whole_r, whole_g, whole_b, whole_w, state);
    maxb = wu_Maximize(set1, WU_BLUE, set1->b0 + 1, set1->b1, &cutb, whole_r, whole_g, whole_b, whole_w, state);

    if ((maxr >= maxg) && (maxr >= maxb))
    {
        dir = WU_RED;
        if (cutr < 0)
        {
            return 0; /* can't split the box */
        }
    }
    else if ((maxg >= maxr) && (maxg >= maxb))
    {
        dir = WU_GREEN;
    }
    else
    {
        dir = WU_BLUE;
    }

    set2->r1 = set1->r1;
    set2->g1 = set1->g1;
    set2->b1 = set1->b1;

    switch (dir)
    {
    case WU_RED:
        set2->r0 = set1->r1 = cutr;
        set2->g0            = set1->g0;
        set2->b0            = set1->b0;
        break;
    case WU_GREEN:
        set2->g0 = set1->g1 = cutg;
        set2->r0            = set1->r0;
        set2->b0            = set1->b0;
        break;
    case WU_BLUE:
        set2->b0 = set1->b1 = cutb;
        set2->r0            = set1->r0;
        set2->g0            = set1->g0;
        break;
    }
    set1->vol = (set1->r1 - set1->r0) * (set1->g1 - set1->g0) * (set1->b1 - set1->b0);
    set2->vol = (set2->r1 - set2->r0) * (set2->g1 - set2->g0) * (set2->b1 - set2->b0);
    return 1;
}

static void wu_Mark(struct wu_box* cube, int label, unsigned char* tag)
{
    int r, g, b;

    for (r = cube->r0 + 1; r <= cube->r1; ++r)
    {
        for (g = cube->g0 + 1; g <= cube->g1; ++g)
        {
            for (b = cube->b0 + 1; b <= cube->b1; ++b)
            {
                tag[(r << 10) + (r << 6) + r + (g << 5) + g + b] = label;
            }
        }
    }
}

/*
 * ============================================================================
 * END OF XIAOLIN WU'S ORIGINAL CODE
 * ============================================================================
 */

/*
 * ============================================================================
 * SAIL WRAPPER FOR WU QUANTIZER
 * ============================================================================
 */

static sail_status_t extract_rgb_channels(const struct sail_image* image,
                                          unsigned char** r_out,
                                          unsigned char** g_out,
                                          unsigned char** b_out)
{
    const unsigned int pixel_count = image->width * image->height;
    unsigned char* r_channel       = NULL;
    unsigned char* g_channel       = NULL;
    unsigned char* b_channel       = NULL;

    SAIL_TRY(sail_malloc(pixel_count, (void**)&r_channel));
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixel_count, (void**)&g_channel),
                        /* cleanup */ sail_free(r_channel));
    SAIL_TRY_OR_CLEANUP(sail_malloc(pixel_count, (void**)&b_channel),
                        /* cleanup */ sail_free(r_channel), sail_free(g_channel));

    /* Extract RGB channels based on pixel format. */
    const unsigned char* pixels = (const unsigned char*)image->pixels;
    unsigned int idx            = 0;

    for (unsigned int y = 0; y < image->height; y++)
    {
        const unsigned char* scan = pixels + y * image->bytes_per_line;

        for (unsigned int x = 0; x < image->width; x++)
        {
            switch (image->pixel_format)
            {
            case SAIL_PIXEL_FORMAT_BPP24_RGB:
                r_channel[idx]  = scan[0];
                g_channel[idx]  = scan[1];
                b_channel[idx]  = scan[2];
                scan           += 3;
                break;

            case SAIL_PIXEL_FORMAT_BPP24_BGR:
                b_channel[idx]  = scan[0];
                g_channel[idx]  = scan[1];
                r_channel[idx]  = scan[2];
                scan           += 3;
                break;

            case SAIL_PIXEL_FORMAT_BPP32_RGBA:
            case SAIL_PIXEL_FORMAT_BPP32_RGBX:
                r_channel[idx]  = scan[0];
                g_channel[idx]  = scan[1];
                b_channel[idx]  = scan[2];
                scan           += 4;
                break;

            case SAIL_PIXEL_FORMAT_BPP32_BGRA:
            case SAIL_PIXEL_FORMAT_BPP32_BGRX:
                b_channel[idx]  = scan[0];
                g_channel[idx]  = scan[1];
                r_channel[idx]  = scan[2];
                scan           += 4;
                break;

            default:
                sail_free(r_channel);
                sail_free(g_channel);
                sail_free(b_channel);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
            }
            idx++;
        }
    }

    *r_out = r_channel;
    *g_out = g_channel;
    *b_out = b_channel;

    return SAIL_OK;
}

/*
 * Build a 3D lookup table for fast palette index lookup.
 * Maps quantized RGB (5-bit per channel) to palette index.
 * This is ~100x faster than brute-force search for each pixel.
 */
static void build_palette_lookup_table(const unsigned char* lut_r,
                                       const unsigned char* lut_g,
                                       const unsigned char* lut_b,
                                       unsigned char palette_size,
                                       unsigned char lookup[32][32][32])
{
    /* For each quantized RGB value, find the closest palette entry. */
    for (unsigned qr = 0; qr < 32; qr++)
    {
        for (unsigned qg = 0; qg < 32; qg++)
        {
            for (unsigned qb = 0; qb < 32; qb++)
            {
                /* Convert quantized coordinates back to 8-bit RGB. */
                const int r = (qr << 3) | (qr >> 2); /* Spread 5 bits to 8 bits. */
                const int g = (qg << 3) | (qg >> 2);
                const int b = (qb << 3) | (qb >> 2);

                /* Find closest palette entry. */
                unsigned char best_idx = 0;
                int best_distance      = INT_MAX;

                for (unsigned char i = 0; i < palette_size; i++)
                {
                    const int dr       = r - (int)lut_r[i];
                    const int dg       = g - (int)lut_g[i];
                    const int db       = b - (int)lut_b[i];
                    const int distance = dr * dr + dg * dg + db * db;

                    if (distance < best_distance)
                    {
                        best_distance = distance;
                        best_idx      = i;

                        /* Early exit on exact match. */
                        if (distance == 0)
                        {
                            break;
                        }
                    }
                }

                lookup[qr][qg][qb] = best_idx;
            }
        }
    }
}

/*
 * Floyd-Steinberg dithering algorithm (1976)
 * Distributes quantization error to neighboring pixels:
 *          X    7/16
 *   3/16  5/16  1/16
 *
 * This is a clean-room implementation based on the published algorithm description.
 * Optimized with 3D lookup table for O(1) palette index lookup.
 */
static sail_status_t apply_floyd_steinberg_dithering(struct sail_image* indexed_image,
                                                     const unsigned char* lut_r,
                                                     const unsigned char* lut_g,
                                                     const unsigned char* lut_b,
                                                     const unsigned char* original_r,
                                                     const unsigned char* original_g,
                                                     const unsigned char* original_b)
{
    SAIL_CHECK_PTR(indexed_image);
    SAIL_CHECK_PTR(lut_r);
    SAIL_CHECK_PTR(lut_g);
    SAIL_CHECK_PTR(lut_b);
    SAIL_CHECK_PTR(original_r);
    SAIL_CHECK_PTR(original_g);
    SAIL_CHECK_PTR(original_b);

    /* Only BPP2/4/8_INDEXED are supported for dithering currently. */
    if (indexed_image->pixel_format != SAIL_PIXEL_FORMAT_BPP2_INDEXED &&
        indexed_image->pixel_format != SAIL_PIXEL_FORMAT_BPP4_INDEXED &&
        indexed_image->pixel_format != SAIL_PIXEL_FORMAT_BPP8_INDEXED)
    {
        return SAIL_OK;
    }

    /* Ignore large palette. */
    if (indexed_image->palette->color_count > 256)
    {
        return SAIL_OK;
    }

    const unsigned width  = indexed_image->width;
    const unsigned height = indexed_image->height;

    /* Build lookup table for fast palette index lookup (32×32×32 = 32KB). */
    unsigned char(*lookup)[32][32] = NULL;
    SAIL_TRY(sail_malloc(32 * 32 * 32, (void**)&lookup));

    build_palette_lookup_table(lut_r, lut_g, lut_b, (unsigned char)indexed_image->palette->color_count, lookup);

    /* Allocate error buffers for current and next row (R, G, B channels). */
    int* error_r_current = NULL;
    int* error_g_current = NULL;
    int* error_b_current = NULL;
    int* error_r_next    = NULL;
    int* error_g_next    = NULL;
    int* error_b_next    = NULL;

    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_r_current),
                        /* cleanup */ sail_free(lookup));
    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_g_current),
                        /* cleanup */ sail_free(lookup), sail_free(error_r_current));
    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_b_current),
                        /* cleanup */ sail_free(lookup), sail_free(error_r_current), sail_free(error_g_current));
    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_r_next),
                        /* cleanup */ sail_free(lookup), sail_free(error_r_current), sail_free(error_g_current),
                        sail_free(error_b_current));
    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_g_next),
                        /* cleanup */ sail_free(lookup), sail_free(error_r_current), sail_free(error_g_current),
                        sail_free(error_b_current), sail_free(error_r_next));
    SAIL_TRY_OR_CLEANUP(sail_malloc((width + 2) * sizeof(int), (void**)&error_b_next),
                        /* cleanup */ sail_free(lookup), sail_free(error_r_current), sail_free(error_g_current),
                        sail_free(error_b_current), sail_free(error_r_next), sail_free(error_g_next));

    /* Initialize error buffers to zero (add padding of 1 pixel on each side). */
    memset(error_r_current, 0, (width + 2) * sizeof(int));
    memset(error_g_current, 0, (width + 2) * sizeof(int));
    memset(error_b_current, 0, (width + 2) * sizeof(int));
    memset(error_r_next, 0, (width + 2) * sizeof(int));
    memset(error_g_next, 0, (width + 2) * sizeof(int));
    memset(error_b_next, 0, (width + 2) * sizeof(int));

    unsigned char* pixels = (unsigned char*)indexed_image->pixels;

    /* Process each row. */
    for (unsigned y = 0; y < height; y++)
    {
        /* Process each pixel in the row. */
        for (unsigned x = 0; x < width; x++)
        {
            const unsigned pixel_idx = y * width + x;

            /* Get original RGB values with accumulated error. */
            int r = (int)original_r[pixel_idx] + error_r_current[x + 1];
            int g = (int)original_g[pixel_idx] + error_g_current[x + 1];
            int b = (int)original_b[pixel_idx] + error_b_current[x + 1];

            r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
            g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
            b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

            /* Fast palette lookup using 3D table (O(1) instead of O(palette_size)). */
            const unsigned qr            = r >> 3; /* Quantize to 5 bits (0-31). */
            const unsigned qg            = g >> 3;
            const unsigned qb            = b >> 3;
            const unsigned char best_idx = lookup[qr][qg][qb];

            /* Update pixel with best match. */
            if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED)
            {
                pixels[pixel_idx] = best_idx;
            }
            else if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED)
            {
                int byte_idx = y * indexed_image->bytes_per_line + x / 2;
                if (x % 2 == 0)
                {
                    pixels[byte_idx] = (pixels[byte_idx] & 0x0F) | ((best_idx & 0x0F) << 4);
                }
                else
                {
                    pixels[byte_idx] = (pixels[byte_idx] & 0xF0) | (best_idx & 0x0F);
                }
            }
            else if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED)
            {
                int byte_idx = y * indexed_image->bytes_per_line + x / 4;
                int shift = 6 - ((x % 4) * 2);
                pixels[byte_idx] = (pixels[byte_idx] & ~(0x03 << shift)) | ((best_idx & 0x03) << shift);
            }

            /* Calculate quantization error. */
            const int error_r = r - (int)lut_r[best_idx];
            const int error_g = g - (int)lut_g[best_idx];
            const int error_b = b - (int)lut_b[best_idx];

            /* Distribute error to neighboring pixels using Floyd-Steinberg coefficients:
             *          X    7/16
             *   3/16  5/16  1/16
             */
            if (x + 1 < width)
            {
                /* Right pixel: 7/16. */
                error_r_current[x + 2] += (error_r * 7) / 16;
                error_g_current[x + 2] += (error_g * 7) / 16;
                error_b_current[x + 2] += (error_b * 7) / 16;
            }

            if (y + 1 < height)
            {
                /* Bottom-left pixel: 3/16. */
                if (x > 0)
                {
                    error_r_next[x] += (error_r * 3) / 16;
                    error_g_next[x] += (error_g * 3) / 16;
                    error_b_next[x] += (error_b * 3) / 16;
                }

                /* Bottom pixel: 5/16. */
                error_r_next[x + 1] += (error_r * 5) / 16;
                error_g_next[x + 1] += (error_g * 5) / 16;
                error_b_next[x + 1] += (error_b * 5) / 16;

                /* Bottom-right pixel: 1/16. */
                if (x + 1 < width)
                {
                    error_r_next[x + 2] += error_r / 16;
                    error_g_next[x + 2] += error_g / 16;
                    error_b_next[x + 2] += error_b / 16;
                }
            }
        }

        /* Swap error buffers for next row. */
        int* temp;
        temp            = error_r_current;
        error_r_current = error_r_next;
        error_r_next    = temp;
        temp            = error_g_current;
        error_g_current = error_g_next;
        error_g_next    = temp;
        temp            = error_b_current;
        error_b_current = error_b_next;
        error_b_next    = temp;

        /* Clear next row error buffer. */
        memset(error_r_next, 0, (width + 2) * sizeof(int));
        memset(error_g_next, 0, (width + 2) * sizeof(int));
        memset(error_b_next, 0, (width + 2) * sizeof(int));
    }

    /* Cleanup. */
    sail_free(lookup);
    sail_free(error_r_current);
    sail_free(error_g_current);
    sail_free(error_b_current);
    sail_free(error_r_next);
    sail_free(error_g_next);
    sail_free(error_b_next);

    return SAIL_OK;
}

sail_status_t sail_quantize_image(const struct sail_image* source_image,
                                  enum SailPixelFormat output_pixel_format,
                                  bool dither,
                                  struct sail_image** target_image)
{
    SAIL_CHECK_PTR(source_image);
    SAIL_CHECK_PTR(target_image);

    /* Determine max_colors based on output pixel format. */
    unsigned max_colors;
    switch (output_pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
        max_colors = 2;
        break;
    case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
        max_colors = 4;
        break;
    case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
        max_colors = 16;
        break;
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
        max_colors = 256;
        break;
    default:
        SAIL_LOG_ERROR("Output pixel format must be indexed (BPP 1/2/4/8)");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    wu_state_t* state = NULL;
    SAIL_TRY(sail_calloc(1, sizeof(wu_state_t), (void**)&state));

    unsigned char* original_r = NULL;
    unsigned char* original_g = NULL;
    unsigned char* original_b = NULL;
    sail_status_t status;
    if ((status = extract_rgb_channels(source_image, &original_r, &original_g, &original_b)) != SAIL_OK)
    {
        sail_free(state);
        return status;
    }

    /* Keep references for Wu algorithm. */
    state->Ir   = original_r;
    state->Ig   = original_g;
    state->Ib   = original_b;
    state->size = source_image->width * source_image->height;
    state->K    = max_colors;

    wu_Hist3d(state, (long int*)state->wt, (long int*)state->mr, (long int*)state->mg, (long int*)state->mb,
              (float*)state->m2);

    if (state->Qadd == NULL)
    {
        sail_free(state->Ir);
        sail_free(state->Ig);
        sail_free(state->Ib);
        sail_free(state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* Don't free original RGB channels yet. */
    state->Ir = state->Ig = state->Ib = NULL;

    wu_M3d((long int*)state->wt, (long int*)state->mr, (long int*)state->mg, (long int*)state->mb, (float*)state->m2);

    /* Perform color space partition. */
    struct wu_box cube[WU_MAXCOLOR];
    float vv[WU_MAXCOLOR];
    unsigned char lut_r[WU_MAXCOLOR];
    unsigned char lut_g[WU_MAXCOLOR];
    unsigned char lut_b[WU_MAXCOLOR];

    cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
    cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;

    int next = 0;
    int i;
    for (i = 1; i < state->K; ++i)
    {
        if (wu_Cut(&cube[next], &cube[i], state))
        {
            /* volume test ensures we won't try to cut one-cell box */
            vv[next] = (cube[next].vol > 1) ? wu_Var(&cube[next], state) : 0;
            vv[i]    = (cube[i].vol > 1) ? wu_Var(&cube[i], state) : 0;
        }
        else
        {
            vv[next] = 0.0; /* don't try to split this box again */
            i--;            /* didn't create box i */
        }

        next       = 0;
        float temp = vv[0];
        for (int k = 1; k <= i; ++k)
        {
            if (vv[k] > temp)
            {
                temp = vv[k];
                next = k;
            }
        }
        if (temp <= 0.0)
        {
            state->K = i + 1;
            break;
        }
    }

    /* Build color lookup table. */
    unsigned char* tag = NULL;
    SAIL_TRY_OR_CLEANUP(sail_malloc(33 * 33 * 33, (void**)&tag),
                        /* cleanup */ sail_free(state->Qadd), sail_free(state));

    memset(tag, 0, 33 * 33 * 33);

    for (int k = 0; k < state->K; ++k)
    {
        wu_Mark(&cube[k], k, tag);
        long int weight = wu_Vol(&cube[k], state->wt);
        if (weight)
        {
            lut_r[k] = (unsigned char)(wu_Vol(&cube[k], state->mr) / weight);
            lut_g[k] = (unsigned char)(wu_Vol(&cube[k], state->mg) / weight);
            lut_b[k] = (unsigned char)(wu_Vol(&cube[k], state->mb) / weight);
        }
        else
        {
            lut_r[k] = lut_g[k] = lut_b[k] = 0;
        }
    }

    /* Map pixels to palette indices. */
    for (i = 0; i < state->size; ++i)
    {
        state->Qadd[i] = tag[state->Qadd[i]];
    }

    sail_free(tag);

    /* Create output indexed image. */
    struct sail_image* indexed_image = NULL;
    SAIL_TRY_OR_CLEANUP(sail_alloc_image(&indexed_image),
                        /* cleanup */ sail_free(state->Qadd), sail_free(state));

    indexed_image->width  = source_image->width;
    indexed_image->height = source_image->height;

    /* Use the requested output pixel format. */
    indexed_image->pixel_format = output_pixel_format;
    indexed_image->bytes_per_line = sail_bytes_per_line(indexed_image->width, indexed_image->pixel_format);

    SAIL_TRY_OR_CLEANUP(
        sail_malloc((size_t)indexed_image->bytes_per_line * indexed_image->height, &indexed_image->pixels),
        /* cleanup */ sail_destroy_image(indexed_image), sail_free(state->Qadd), sail_free(state));

    /* Copy indexed pixels. */
    if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP8_INDEXED)
    {
        /*
         * state->Qadd is unsigned short*, but we need unsigned char* for BPP8_INDEXED.
         * Extract the lower byte from each short (palette index 0-255).
         */
        unsigned char* dest = indexed_image->pixels;
        for (int k = 0; k < state->size; k++)
        {
            dest[k] = (unsigned char)state->Qadd[k];
        }
    }
    else
    {
        unsigned char* dest = (unsigned char*)indexed_image->pixels;
        memset(dest, 0, (size_t)indexed_image->bytes_per_line * indexed_image->height);

        for (unsigned int y = 0; y < source_image->height; y++)
        {
            for (unsigned int x = 0; x < source_image->width; x++)
            {
                unsigned char idx = (unsigned char)state->Qadd[y * source_image->width + x];
                if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP4_INDEXED)
                {
                    int byte_idx = y * indexed_image->bytes_per_line + x / 2;
                    if (x % 2 == 0)
                    {
                        dest[byte_idx] = (idx & 0x0F) << 4;
                    }
                    else
                    {
                        dest[byte_idx] |= (idx & 0x0F);
                    }
                }
                else if (indexed_image->pixel_format == SAIL_PIXEL_FORMAT_BPP2_INDEXED)
                {
                    int byte_idx = y * indexed_image->bytes_per_line + x / 4;
                    int shift = 6 - ((x % 4) * 2);
                    dest[byte_idx] |= (idx & 0x03) << shift;
                }
                else
                {
                    int byte_idx = y * indexed_image->bytes_per_line + x / 8;
                    int bit_idx  = 7 - (x % 8);
                    if (idx)
                    {
                        dest[byte_idx] |= (1 << bit_idx);
                    }
                }
            }
        }
    }

    sail_free(state->Qadd);

    /* Create palette. */
    struct sail_palette* palette = NULL;
    SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, state->K, &palette),
                        /* cleanup */ sail_destroy_image(indexed_image), sail_free(state));

    unsigned char* pal_data = (unsigned char*)palette->data;
    for (int k = 0; k < state->K; ++k)
    {
        pal_data[k * 3 + 0] = lut_r[k];
        pal_data[k * 3 + 1] = lut_g[k];
        pal_data[k * 3 + 2] = lut_b[k];
    }

    indexed_image->palette = palette;

    /* Apply dithering if requested. */
    if (dither)
    {
        SAIL_TRY_OR_CLEANUP(
            apply_floyd_steinberg_dithering(indexed_image, lut_r, lut_g, lut_b, original_r, original_g, original_b),
            /* cleanup */ sail_destroy_image(indexed_image), sail_free(original_r), sail_free(original_g),
            sail_free(original_b), sail_free(state));
    }

    /* Free original RGB channels. */
    sail_free(original_r);
    sail_free(original_g);
    sail_free(original_b);

    sail_free(state);
    *target_image = indexed_image;

    return SAIL_OK;
}
