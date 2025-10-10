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

#pragma once

#include <stdbool.h>

#include <sail-common/common.h>
#include <sail-common/export.h>
#include <sail-common/status.h>

struct sail_io;
struct sail_variant;
struct sail_iccp;
struct sail_meta_data_node;
struct sail_palette;
struct sail_hash_map;

/*
 * XPM color entry.
 */
struct xpm_color {
    char chars[8];            /* Character(s) representing this color. */
    unsigned char r, g, b, a; /* RGBA values. */
    bool is_none;             /* True if this is a transparent color. */
};

/*
 * Tuning state for callback.
 */
struct xpm_state {
    char var_name[256];
};

SAIL_HIDDEN sail_status_t xpm_private_parse_xpm_header(struct sail_io *io,
                                                       unsigned *width,
                                                       unsigned *height,
                                                       unsigned *num_colors,
                                                       unsigned *cpp,
                                                       int *x_hotspot,
                                                       int *y_hotspot);

SAIL_HIDDEN sail_status_t xpm_private_parse_colors(struct sail_io *io,
                                                   unsigned num_colors,
                                                   unsigned cpp,
                                                   struct xpm_color **colors,
                                                   bool *has_transparency);

SAIL_HIDDEN sail_status_t xpm_private_read_pixels(struct sail_io *io,
                                                  unsigned width,
                                                  unsigned height,
                                                  unsigned cpp,
                                                  const struct xpm_color *colors,
                                                  unsigned num_colors,
                                                  unsigned char *pixels,
                                                  enum SailPixelFormat pixel_format);

SAIL_HIDDEN sail_status_t xpm_private_write_header(struct sail_io *io,
                                                   unsigned width,
                                                   unsigned height,
                                                   unsigned num_colors,
                                                   unsigned cpp,
                                                   const char *name,
                                                   int x_hotspot,
                                                   int y_hotspot);

SAIL_HIDDEN sail_status_t xpm_private_write_colors(struct sail_io *io,
                                                   const unsigned char *palette_data,
                                                   unsigned num_colors,
                                                   unsigned cpp,
                                                   bool has_transparency,
                                                   int transparency_index);

SAIL_HIDDEN sail_status_t xpm_private_write_pixels(struct sail_io *io,
                                                   const unsigned char *pixels,
                                                   unsigned width,
                                                   unsigned height,
                                                   unsigned cpp,
                                                   unsigned num_colors);

SAIL_HIDDEN bool xpm_private_tuning_key_value_callback(const char *key,
                                                       const struct sail_variant *value,
                                                       void *user_data);

SAIL_HIDDEN sail_status_t xpm_private_skip_extensions(struct sail_io *io);

SAIL_HIDDEN enum SailPixelFormat xpm_private_determine_pixel_format(unsigned num_colors, bool has_transparency);

SAIL_HIDDEN sail_status_t xpm_private_build_palette(struct sail_palette **palette,
                                                     const struct xpm_color *colors,
                                                     unsigned num_colors);

SAIL_HIDDEN sail_status_t xpm_private_store_hotspot(int x_hotspot,
                                                     int y_hotspot,
                                                     struct sail_hash_map *special_properties);

SAIL_HIDDEN sail_status_t xpm_private_fetch_hotspot(const struct sail_hash_map *special_properties,
                                                     int *x_hotspot,
                                                     int *y_hotspot);

SAIL_HIDDEN sail_status_t xpm_private_check_transparency(const struct sail_palette *palette,
                                                         unsigned num_colors,
                                                         bool *has_transparency,
                                                         int *transparency_index);

SAIL_HIDDEN sail_status_t xpm_private_convert_palette_to_rgb(const unsigned char *src_palette,
                                                             enum SailPixelFormat src_format,
                                                             unsigned num_colors,
                                                             unsigned char **rgb_palette);
