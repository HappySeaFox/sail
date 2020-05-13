/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_PNG_HELPERS_H
#define SAIL_PNG_HELPERS_H

#include <stdbool.h>
#include <stdio.h>

#include <png.h>

#include "error.h"
#include "export.h"

struct sail_meta_entry_node;

SAIL_HIDDEN void my_error_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN void my_warning_fn(png_structp png_ptr, png_const_charp text);

SAIL_HIDDEN int png_color_type_to_pixel_format(int color_type, int bit_depth);

SAIL_HIDDEN sail_error_t pixel_format_to_png_color_type(int pixel_format, int *color_type, int *bit_depth);

SAIL_HIDDEN sail_error_t supported_read_output_pixel_format(int pixel_format);

SAIL_HIDDEN sail_error_t supported_write_input_pixel_format(int pixel_format);

SAIL_HIDDEN sail_error_t supported_write_output_pixel_format(int pixel_format);

SAIL_HIDDEN sail_error_t read_png_text(png_structp png_ptr, png_infop info_ptr, struct sail_meta_entry_node **target_meta_entry_node);

SAIL_HIDDEN sail_error_t write_png_text(png_structp png_ptr, png_infop info_ptr, const struct sail_meta_entry_node *meta_entry_node);

#endif
