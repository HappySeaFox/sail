/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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
        , animated(false)
        , delay(0)
        , properties(0)
        , source_pixel_format(SAIL_PIXEL_FORMAT_UNKNOWN)
        , source_properties(0)
        , source_compression_type(SAIL_COMPRESSION_UNSUPPORTED)
        , bits(nullptr)
        , bits_size(0)
        , shallow_bits(nullptr)
    {}

    ~pimpl()
    {
        free(bits);
    }

    unsigned width;
    unsigned height;
    unsigned bytes_per_line;
    SailPixelFormat pixel_format;
    bool animated;
    int delay;
    sail::palette palette;
    std::map<std::string, std::string> meta_entries;
    int properties;
    SailPixelFormat source_pixel_format;
    int source_properties;
    SailCompressionType source_compression_type;
    void *bits;
    unsigned bits_size;
    const void *shallow_bits;
    sail::iccp iccp;
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
        .with_animated(img.animated())
        .with_delay(img.delay())
        .with_palette(img.palette())
        .with_meta_entries(meta_entries())
        .with_properties(img.properties())
        .with_source_pixel_format(img.source_pixel_format())
        .with_source_properties(img.source_properties())
        .with_source_compression_type(img.source_compression_type());

    if (img.shallow_bits() != nullptr) {
        with_shallow_bits(img.shallow_bits());
    } else {
        with_bits(img.bits(), img.bits_size());
    }

    with_iccp(img.iccp());

    return *this;
}

image::~image()
{
    delete d;
}

bool image::is_valid() const
{
    return d->width > 0 && d->height > 0 && d->bytes_per_line > 0 && (d->bits != nullptr || d->shallow_bits != nullptr);
}

unsigned image::width() const
{
    return d->width;
}

unsigned image::height() const
{
    return d->height;
}

unsigned image::bytes_per_line() const
{
    return d->bytes_per_line;
}

SailPixelFormat image::pixel_format() const
{
    return d->pixel_format;
}

bool image::animated() const
{
    return d->animated;
}

int image::delay() const
{
    return d->delay;
}

sail::palette image::palette() const
{
    return d->palette;
}

std::map<std::string, std::string> image::meta_entries() const
{
    return d->meta_entries;
}

int image::properties() const
{
    return d->properties;
}

SailPixelFormat image::source_pixel_format() const
{
    return d->source_pixel_format;
}

int image::source_properties() const
{
    return d->source_properties;
}

SailCompressionType image::source_compression_type() const
{
    return d->source_compression_type;
}

void* image::bits()
{
    return d->bits;
}

const void* image::bits() const
{
    return d->bits;
}

unsigned image::bits_size() const
{
    return d->bits_size;
}

const void* image::shallow_bits() const
{
    return d->shallow_bits;
}

sail::iccp image::iccp() const
{
    return d->iccp;
}

image& image::with_width(unsigned width)
{
    d->width = width;
    return *this;
}

image& image::with_height(unsigned height)
{
    d->height = height;
    return *this;
}

image& image::with_bytes_per_line(unsigned bytes_per_line)
{
    d->bytes_per_line = bytes_per_line;
    return *this;
}

image& image::with_bytes_per_line_auto()
{
    unsigned bytes_per_line = 0;
    image::bytes_per_line(d->width, d->pixel_format, &bytes_per_line);

    return with_bytes_per_line(bytes_per_line);
}

image& image::with_pixel_format(SailPixelFormat pixel_format)
{
    d->pixel_format = pixel_format;
    return *this;
}

image& image::with_delay(int delay)
{
    d->delay = delay;
    return *this;
}

image& image::with_palette(const sail::palette &pal)
{
    d->palette = pal;
    return *this;
}

image& image::with_meta_entries(const std::map<std::string, std::string> &meta_entries)
{
    d->meta_entries = meta_entries;
    return *this;
}

image& image::with_bits(const void *bits, unsigned bits_size)
{
    free(d->bits);

    d->bits         = nullptr;
    d->bits_size    = 0;
    d->shallow_bits = nullptr;

    if (bits == nullptr || bits_size == 0) {
        return *this;
    }

    d->bits_size = bits_size;
    d->bits = malloc(d->bits_size);

    if (d->bits == nullptr) {
        SAIL_LOG_ERROR("Memory allocation failed of bits size %u", d->bits_size);
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

image& image::with_iccp(const sail::iccp &ic)
{
    d->iccp = ic;

    return *this;
}

sail_error_t image::bits_per_pixel(SailPixelFormat pixel_format, unsigned *result)
{
    SAIL_TRY(sail_bits_per_pixel(pixel_format, result));

    return 0;
}

sail_error_t image::bytes_per_line(unsigned width, SailPixelFormat pixel_format, unsigned *result)
{
    SAIL_CHECK_PTR(result);

    SAIL_TRY(sail_bytes_per_line(width, pixel_format, result));

    return 0;
}

sail_error_t image::bytes_per_image(const image &simage, unsigned *result)
{
    SAIL_CHECK_PTR(result);

    sail_image sail_image;

    sail_image.width        = simage.width();
    sail_image.height       = simage.height();
    sail_image.pixel_format = simage.pixel_format();

    SAIL_TRY(sail_bytes_per_image(&sail_image, result));

    return 0;
}

sail_error_t image::pixel_format_to_string(SailPixelFormat pixel_format, const char **result)
{
    SAIL_TRY(sail_pixel_format_to_string(pixel_format, result));

    return 0;
}

sail_error_t image::pixel_format_from_string(const char *str, SailPixelFormat *result)
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

sail_error_t image::compression_type_to_string(SailCompressionType compression, const char **result)
{
    SAIL_TRY(sail_compression_type_to_string(compression, result));

    return 0;
}

sail_error_t image::compression_type_from_string(const char *str, SailCompressionType *result)
{
    SAIL_TRY(sail_compression_type_from_string(str, result));

    return 0;
}

image::image(const sail_image *im, const void *bits, unsigned bits_size)
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
        .with_animated(im->animated)
        .with_delay(im->delay)
        .with_meta_entries(meta_entries)
        .with_properties(im->properties)
        .with_source_pixel_format(im->source_pixel_format)
        .with_source_properties(im->source_properties)
        .with_source_compression_type(im->source_compression_type)
        .with_bits(bits, bits_size);

    if (im->palette != nullptr) {
        sail::palette palette(im->palette);
        with_palette(palette);
    }

    if (im->iccp != nullptr) {
        sail::iccp iccp(im->iccp);
        with_iccp(iccp);
    }
}

image::image(const sail_image *im)
    : image(im, nullptr, 0)
{
}

sail_error_t image::to_sail_image(sail_image *image) const
{
    SAIL_CHECK_IMAGE_PTR(image);

    // Resulting meta entries
    sail_meta_entry_node *image_meta_entry_node = nullptr;

    sail_meta_entry_node **last_meta_entry_node = &image_meta_entry_node;
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

        *last_meta_entry_node = meta_entry_node;
        last_meta_entry_node = &meta_entry_node->next;

        ++it;
    }

    image->width          = d->width;
    image->height         = d->height;
    image->bytes_per_line = d->bytes_per_line;
    image->pixel_format   = d->pixel_format;
    image->animated       = d->animated;
    image->delay          = d->delay;

    image->meta_entry_node         = image_meta_entry_node;
    image->properties              = d->properties;
    image->source_pixel_format     = d->source_pixel_format;
    image->source_properties       = d->source_properties;
    image->source_compression_type = d->source_compression_type;

    if (d->palette.is_valid()) {
        image->palette = (sail_palette *)malloc(sizeof(sail_palette));

        if (image->palette == nullptr) {
            sail_destroy_meta_entry_node_chain(image->meta_entry_node);
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        SAIL_TRY_OR_CLEANUP(d->palette.to_sail_palette(image->palette),
                            /* cleanup */ sail_destroy_palette(image->palette);
                                          sail_destroy_meta_entry_node_chain(image->meta_entry_node));
    }

    if (d->iccp.is_valid()) {
        image->iccp = (sail_iccp *)malloc(sizeof(sail_iccp));

        if (image->iccp == nullptr) {
            sail_destroy_palette(image->palette);
            sail_destroy_meta_entry_node_chain(image->meta_entry_node);
            return SAIL_MEMORY_ALLOCATION_FAILED;
        }

        SAIL_TRY_OR_CLEANUP(d->iccp.to_sail_iccp(image->iccp),
                            /* cleanup */ sail_destroy_iccp(image->iccp),
                                          sail_destroy_palette(image->palette);
                                          sail_destroy_meta_entry_node_chain(image->meta_entry_node));
    }

    return 0;
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

image& image::with_source_pixel_format(SailPixelFormat source_pixel_format)
{
    d->source_pixel_format = source_pixel_format;
    return *this;
}

image& image::with_source_properties(int source_properties)
{
    d->source_properties = source_properties;
    return *this;
}

image& image::with_source_compression_type(SailCompressionType source_compression_type)
{
    d->source_compression_type = source_compression_type;
    return *this;
}

}
