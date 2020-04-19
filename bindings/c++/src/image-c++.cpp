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

#include "config.h"

#include <cstdlib>
#include <cstring>

// libsail-common.
#include "common.h"
#include "error.h"
#include "meta_entry_node.h"
#include "log.h"

// libsail.
#include "context.h"
#include "plugin_info.h"
#include "sail.h"
#include "string_node.h"

#include "image-c++.h"

namespace sail
{

class image::pimpl
{
public:
    pimpl()
        : width(0)
        , height(0)
        , bytes_per_line(0)
        , pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , passes(0)
        , animated(false)
        , delay(0)
        , palette_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , palette(nullptr)
        , palette_size(0)
        , properties(0)
        , source_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , source_properties(0)
        , bits(nullptr)
        , bits_size(0)
    {}

    ~pimpl()
    {
        free(palette);
        free(bits);
    }

    int width;
    int height;
    int bytes_per_line;
    int pixel_format;
    int passes;
    bool animated;
    int delay;
    int palette_pixel_format;
    void *palette;
    int palette_size;
    std::map<std::string, std::string> meta_entries;
    int properties;
    int source_pixel_format;
    int source_properties;
    void *bits;
    int bits_size;
};

image::image()
    : d(new pimpl)
{
}

image::image(const sail_image *im)
    : image()
{
    if (im == nullptr) {
        SAIL_LOG_ERROR("NULL pointer has been passed to sail::image()");
        return;
    }

    std::map<std::string, std::string> meta_entries;

    sail_meta_entry_node *node = im->meta_entry_node;

    while (node != nullptr) {
        meta_entries.insert({ node->key, node->value });
        node = node->next;
    }

    with_width(im->width)
    .with_height(im->height)
    .with_bytes_per_line(im->bytes_per_line)
    .with_pixel_format(im->pixel_format)
    .with_passes(im->passes)
    .with_animated(im->animated)
    .with_delay(im->delay)
    .with_palette(im->palette, im->palette_size, im->palette_pixel_format)
    .with_meta_entries(meta_entries)
    .with_properties(im->properties)
    .with_source_pixel_format(im->source_pixel_format)
    .with_source_properties(im->source_properties);
}

image::image(const image &img)
    : image()
{
    with_width(img.width())
    .with_height(img.height())
    .with_bytes_per_line(img.bytes_per_line())
    .with_pixel_format(img.pixel_format())
    .with_passes(img.passes())
    .with_animated(img.animated())
    .with_delay(img.delay())
    .with_palette(img.palette(), img.palette_size(), img.palette_pixel_format())
    .with_meta_entries(meta_entries())
    .with_properties(img.properties())
    .with_source_pixel_format(img.source_pixel_format())
    .with_source_properties(img.source_properties())
    .with_bits(img.bits(), img.bits_size());
}

image::~image()
{
}

bool image::is_valid() const
{
    return d->width > 0 && d->height > 0;
}

int image::width() const
{
    return d->width;
}

int image::height() const
{
    return d->height;
}

int image::bytes_per_line() const
{
    return d->bytes_per_line;
}

int image::pixel_format() const
{
    return d->pixel_format;
}

int image::passes() const
{
    return d->passes;
}

bool image::animated() const
{
    return d->animated;
}

int image::delay() const
{
    return d->delay;
}

int image::palette_pixel_format() const
{
    return d->palette_pixel_format;
}

void* image::palette() const
{
    return d->palette;
}

int image::palette_size() const
{
    return d->palette_size;
}

std::map<std::string, std::string> image::meta_entries() const
{
    return d->meta_entries;
}

int image::properties() const
{
    return d->properties;
}

int image::source_pixel_format() const
{
    return d->source_pixel_format;
}

int image::source_properties() const
{
    return d->source_properties;
}

void* image::bits() const
{
    return d->bits;
}

int image::bits_size() const
{
    return d->bits_size;
}

image& image::with_width(int width)
{
    d->width = width;
    return *this;
}

image& image::with_height(int height)
{
    d->height = height;
    return *this;
}

image& image::with_bytes_per_line(int bytes_per_line)
{
    d->bytes_per_line = bytes_per_line;
    return *this;
}

image& image::with_pixel_format(int pixel_format)
{
    d->pixel_format = pixel_format;
    return *this;
}

image& image::with_passes(int passes)
{
    d->passes = passes;
    return *this;
}

image& image::with_animated(bool animated)
{
    d->animated = animated;
    return *this;
}

image& image::with_delay(int delay)
{
    d->delay = delay;
    return *this;
}

image& image::with_palette(void *palette, int palette_size, int palette_pixel_format)
{
    free(d->palette);

    d->palette = nullptr;
    d->palette_size = 0;
    d->palette_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;

    if (palette == nullptr || palette_size < 1) {
        return *this;
    }

    d->palette = malloc(palette_size);

    if (d->palette == nullptr) {
        SAIL_LOG_ERROR("Memory allocation failed of palette size %d", palette_size);
        return *this;
    }

    memcpy(d->palette, palette, palette_size);

    d->palette_size         = palette_size;
    d->palette_pixel_format = palette_pixel_format;

    return *this;
}

image& image::with_meta_entries(const std::map<std::string, std::string> &meta_entries)
{
    d->meta_entries = meta_entries;
    return *this;
}

image& image::with_properties(int properties)
{
    d->properties = properties;
    return *this;
}

image& image::with_source_pixel_format(int source_pixel_format)
{
    d->source_pixel_format = source_pixel_format;
    return *this;
}

image& image::with_source_properties(int source_properties)
{
    d->source_properties = source_properties;
    return *this;
}

image& image::with_bits(void *bits, int bits_size)
{
    free(d->bits);

    d->bits = nullptr;
    d->bits_size = 0;

    if (bits == nullptr || bits_size < 1) {
        return *this;
    }

    d->bits = malloc(bits_size);

    if (d->bits == nullptr) {
        SAIL_LOG_ERROR("Memory allocation failed of bits size %d", bits_size);
        return *this;
    }

    memcpy(d->bits, bits, bits_size);

    return *this;
}

}
