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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN image::pimpl
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
        , shallow_bits(nullptr)
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
    const void *shallow_bits;
};

image::image()
    : d(new pimpl)
{
}

image::image(const image &img)
    : image()
{
    *this = img;
}

image& image::operator=(const image &img)
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
        .with_source_properties(img.source_properties());

    if (img.shallow_bits()) {
        with_shallow_bits(img.shallow_bits());
    } else {
        with_bits(img.bits(), img.bits_size());
    }

    return *this;
}

image::~image()
{
    delete d;
}

bool image::is_valid() const
{
    return d->width > 0 && d->height > 0;
}

sail_error_t image::to_sail_image(sail_image *image) const
{
    SAIL_CHECK_IMAGE_PTR(image);

    // Resulting meta entries
    sail_meta_entry_node *image_meta_entry_node = nullptr;

    sail_meta_entry_node *last_meta_entry_node = nullptr;
    auto it = d->meta_entries.begin();

    while (it != d->meta_entries.end()) {
        sail_meta_entry_node *meta_entry_node;

        SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup(it->first.c_str(), &meta_entry_node->key),
                            sail_destroy_meta_entry_node(meta_entry_node),
                            sail_destroy_meta_entry_node_chain(image_meta_entry_node));
        SAIL_TRY_OR_CLEANUP(sail_strdup(it->second.c_str(), &meta_entry_node->value),
                            sail_destroy_meta_entry_node(meta_entry_node),
                            sail_destroy_meta_entry_node_chain(image_meta_entry_node));

        if (image_meta_entry_node == nullptr) {
            image_meta_entry_node = last_meta_entry_node = meta_entry_node;
        } else {
            last_meta_entry_node->next = meta_entry_node;
            last_meta_entry_node = meta_entry_node;
        }

        ++it;
    }

    image->width          = d->width;
    image->height         = d->height;
    image->bytes_per_line = d->bytes_per_line;
    image->pixel_format   = d->pixel_format;
    image->passes         = d->passes;
    image->animated       = d->animated;
    image->delay          = d->delay;

    if (d->palette != nullptr && d->palette_size > 0) {
        image->palette = malloc(d->palette_size);

        if (image->palette == nullptr) {
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        memcpy(image->palette, d->palette, d->palette_size);

        image->palette              = d->palette;
        image->palette_size         = d->palette_size;
        image->palette_pixel_format = d->palette_pixel_format;
    }

    image->meta_entry_node     = image_meta_entry_node;
    image->properties          = d->properties;
    image->source_pixel_format = d->source_pixel_format;
    image->source_properties   = d->source_properties;

    return 0;
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

void* image::bits()
{
    return d->bits;
}

const void* image::bits() const
{
    return d->bits;
}

int image::bits_size() const
{
    return d->bits_size;
}

const void* image::shallow_bits() const
{
    return d->shallow_bits;
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

image& image::with_bytes_per_line_auto()
{
    int bytes_per_line = 0;
    image::bytes_per_line(*this, &bytes_per_line);

    return with_bytes_per_line(bytes_per_line);
}

image& image::with_pixel_format(int pixel_format)
{
    d->pixel_format = pixel_format;
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

image& image::with_bits(const void *bits, int bits_size)
{
    free(d->bits);

    d->bits         = nullptr;
    d->bits_size    = 0;
    d->shallow_bits = nullptr;

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

image& image::with_shallow_bits(const void *bits)
{
    free(d->bits);

    d->bits = nullptr;
    d->bits_size = 0;

    if (bits == nullptr ) {
        SAIL_LOG_ERROR("Not assigning invalid bits. Bits pointer: %p", bits);
        return *this;
    }

    d->shallow_bits = bits;

    return *this;
}

sail_error_t image::bits_per_pixel(int pixel_format, int *result)
{
    SAIL_TRY(sail_bits_per_pixel(pixel_format, result));

    return 0;
}

sail_error_t image::bytes_per_line(const image &simage, int *result)
{
    SAIL_CHECK_PTR(result);

    sail_image sail_image;

    sail_image.width        = simage.width();
    sail_image.pixel_format = simage.pixel_format();

    SAIL_TRY(sail_bytes_per_line(&sail_image, result));

    return 0;
}

sail_error_t image::bytes_per_image(const image &simage, int *result)
{
    SAIL_CHECK_PTR(result);

    sail_image sail_image;

    sail_image.width        = simage.width();
    sail_image.height       = simage.height();
    sail_image.pixel_format = simage.pixel_format();

    SAIL_TRY(sail_bytes_per_image(&sail_image, result));

    return 0;
}

sail_error_t image::pixel_format_to_string(int pixel_format, const char **result)
{
    SAIL_TRY(sail_pixel_format_to_string(pixel_format, result));

    return 0;
}

sail_error_t image::pixel_format_from_string(const char *str, int *result)
{
    SAIL_TRY(sail_pixel_format_from_string(str, result));

    return 0;
}

sail_error_t image::image_property_to_string(int image_property, const char **result)
{
    SAIL_TRY(sail_image_property_to_string(image_property, result));

    return 0;
}

sail_error_t image::image_property_from_string(const char *str, int *result)
{
    SAIL_TRY(sail_image_property_from_string(str, result));

    return 0;
}

sail_error_t image::compression_type_to_string(int compression, const char **result)
{
    SAIL_TRY(sail_compression_type_to_string(compression, result));

    return 0;
}

sail_error_t image::compression_type_from_string(const char *str, int *result)
{
    SAIL_TRY(sail_compression_type_from_string(str, result));

    return 0;
}

image::image(const sail_image *im, const void *bits, int bits_size)
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
        .with_source_properties(im->source_properties)
        .with_bits(bits, bits_size);
}

image::image(const sail_image *im)
    : image(im, nullptr, 0)
{
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

}
