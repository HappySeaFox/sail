/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

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

#ifndef SAIL_IMAGE_CPP_H
#define SAIL_IMAGE_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <map>
#include <string>
#include <vector>

namespace sail
{

class plugin_info;

/*
 * A C++ interface to struct sail_image.
 */
class SAIL_EXPORT image
{
    friend class image_reader;

public:
    image();
    image(const image &image);
    image& operator=(const image &image);
    ~image();

    bool is_valid() const;

    sail_error_t to_sail_image(sail_image *image) const;

    static sail_error_t bits_per_pixel(int pixel_format, int *result);
    static sail_error_t bytes_per_line(const image &simage, int *result);
    static sail_error_t bytes_per_image(const image &simage, int *result);
    static sail_error_t pixel_format_to_string(int pixel_format, const char **result);
    static sail_error_t pixel_format_from_string(const char *str, int *result);
    static sail_error_t image_property_to_string(int image_property, const char **result);
    static sail_error_t image_property_from_string(const char *str, int *result);
    static sail_error_t compression_type_to_string(int compression, const char **result);
    static sail_error_t compression_type_from_string(const char *str, int *result);

    int width() const;
    int height() const;
    int bytes_per_line() const;
    int pixel_format() const;
    int passes() const;
    bool animated() const;
    int delay() const;
    int palette_pixel_format() const;
    void* palette() const;
    int palette_size() const;
    std::map<std::string, std::string> meta_entries() const;
    int properties() const;
    int source_pixel_format() const;
    int source_properties() const;
    void* bits();
    const void* bits() const;
    int bits_size() const;
    const void* shallow_bits() const;

    image& with_width(int width);
    image& with_height(int height);
    image& with_bytes_per_line(int bytes_per_line);
    image& with_bytes_per_line_auto();
    image& with_pixel_format(int pixel_format);
    image& with_delay(int delay);
    image& with_palette(void *palette, int palette_size, int palette_pixel_format);
    image& with_meta_entries(const std::map<std::string, std::string> &meta_entries);

    // Deep copy
    //
    image& with_bits(const void *bits, int bits_size);

    // Shallow copy
    //
    image& with_shallow_bits(const void *bits);

private:
    // Makes a deep copy of the specified image and the bits
    //
    image(const sail_image *im, const void *bits, int bits_size);
    image(const sail_image *im);

    image& with_passes(int passes);
    image& with_animated(bool animated);
    image& with_properties(int properties);
    image& with_source_pixel_format(int source_pixel_format);
    image& with_source_properties(int source_properties);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
