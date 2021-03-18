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
        , pixels(nullptr)
        , pixels_size(0)
        , shallow_pixels(false)
    {}

    ~pimpl()
    {
        if (!shallow_pixels) {
            sail_free(pixels);
        }
    }

    void reset_pixels()
    {
        if (!shallow_pixels) {
            sail_free(pixels);
        }

        pixels         = nullptr;
        pixels_size    = 0;
        shallow_pixels = false;
    }

    unsigned width;
    unsigned height;
    unsigned bytes_per_line;
    sail::resolution resolution;
    SailPixelFormat pixel_format;
    bool animated;
    int delay;
    sail::palette palette;
    std::vector<sail::meta_data> meta_data;
    sail::iccp iccp;
    int properties;
    sail::source_image source_image;
    void *pixels;
    unsigned pixels_size;
    bool shallow_pixels;
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
        .with_resolution(img.resolution())
        .with_pixel_format(img.pixel_format())
        .with_animated(img.animated())
        .with_delay(img.delay())
        .with_palette(img.palette())
        .with_meta_data(img.meta_data())
        .with_iccp(img.iccp())
        .with_properties(img.properties())
        .with_source_image(img.source_image())
        .with_pixels(img.pixels(), img.pixels_size());

    return *this;
}

image::image(image &&img) noexcept
{
    d = img.d;
    img.d = nullptr;
}

image& image::operator=(image &&img)
{
    delete d;
    d = img.d;
    img.d = nullptr;

    return *this;
}

image::~image()
{
    delete d;
}

bool image::is_valid() const
{
    return d->width > 0 && d->height > 0 && d->bytes_per_line > 0 && d->pixels != nullptr;
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

const sail::resolution& image::resolution() const
{
    return d->resolution;
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

const sail::palette& image::palette() const
{
    return d->palette;
}

const std::vector<sail::meta_data>& image::meta_data() const
{
    return d->meta_data;
}

const sail::iccp& image::iccp() const
{
    return d->iccp;
}

int image::properties() const
{
    return d->properties;
}

const sail::source_image& image::source_image() const
{
    return d->source_image;
}

void* image::pixels()
{
    return d->pixels;
}

const void* image::pixels() const
{
    return d->pixels;
}

unsigned image::pixels_size() const
{
    return d->pixels_size;
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

image& image::with_resolution(const sail::resolution &res)
{
    d->resolution = res;
    return *this;
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

image& image::with_meta_data(const std::vector<sail::meta_data> &meta_data)
{
    d->meta_data = meta_data;
    return *this;
}

image& image::with_pixels(const void *pixels)
{
    unsigned bytes_per_image;
    SAIL_TRY_OR_EXECUTE(image::bytes_per_image(*this, &bytes_per_image),
                        /* on error */ return *this);

    with_pixels(pixels, bytes_per_image);
    return *this;
}

image& image::with_pixels(const void *pixels, unsigned pixels_size)
{
    d->reset_pixels();

    if (pixels == nullptr || pixels_size == 0) {
        return *this;
    }

    SAIL_TRY_OR_EXECUTE(sail_malloc(pixels_size, &d->pixels),
                        /* on error */ return *this);

    memcpy(d->pixels, pixels, pixels_size);
    d->pixels_size    = pixels_size;
    d->shallow_pixels = false;

    return *this;
}

image& image::with_shallow_pixels(void *pixels)
{
    unsigned bytes_per_image;
    SAIL_TRY_OR_EXECUTE(image::bytes_per_image(*this, &bytes_per_image),
                        /* on error */ return *this);

    with_shallow_pixels(pixels, bytes_per_image);
    return *this;
}

image& image::with_shallow_pixels(void *pixels, unsigned pixels_size)
{
    d->reset_pixels();

    if (pixels == nullptr || pixels_size == 0) {
        return *this;
    }

    d->pixels         = pixels;
    d->pixels_size    = pixels_size;
    d->shallow_pixels = true;

    return *this;
}

image& image::with_iccp(const sail::iccp &ic)
{
    d->iccp = ic;

    return *this;
}

sail_status_t image::bits_per_pixel(SailPixelFormat pixel_format, unsigned *result)
{
    SAIL_TRY(sail_bits_per_pixel(pixel_format, result));

    return SAIL_OK;
}

sail_status_t image::bytes_per_line(unsigned width, SailPixelFormat pixel_format, unsigned *result)
{
    SAIL_CHECK_PTR(result);

    SAIL_TRY(sail_bytes_per_line(width, pixel_format, result));

    return SAIL_OK;
}

sail_status_t image::bytes_per_image(const image &simage, unsigned *result)
{
    SAIL_CHECK_PTR(result);

    sail_image sail_image;

    sail_image.width        = simage.width();
    sail_image.height       = simage.height();
    sail_image.pixel_format = simage.pixel_format();

    SAIL_TRY(sail_bytes_per_image(&sail_image, result));

    return SAIL_OK;
}

sail_status_t image::pixel_format_to_string(SailPixelFormat pixel_format, const char **result)
{
    SAIL_TRY(sail_pixel_format_to_string(pixel_format, result));

    return SAIL_OK;
}

sail_status_t image::pixel_format_from_string(const std::string_view str, SailPixelFormat *result)
{
    SAIL_TRY(sail_pixel_format_from_string(str.data(), result));

    return SAIL_OK;
}

sail_status_t image::image_property_to_string(SailImageProperty image_property, const char **result)
{
    SAIL_TRY(sail_image_property_to_string(image_property, result));

    return SAIL_OK;
}

sail_status_t image::image_property_from_string(const std::string_view str, SailImageProperty *result)
{
    SAIL_TRY(sail_image_property_from_string(str.data(), result));

    return SAIL_OK;
}

sail_status_t image::compression_to_string(SailCompression compression, const char **result)
{
    SAIL_TRY(sail_compression_to_string(compression, result));

    return SAIL_OK;
}

sail_status_t image::compression_from_string(const std::string_view str, SailCompression *result)
{
    SAIL_TRY(sail_compression_from_string(str.data(), result));

    return SAIL_OK;
}

image::image(const sail_image *sail_image)
    : image()
{
    if (sail_image == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::image(). The object is untouched");
        return;
    }

    std::vector<sail::meta_data> meta_data;
    sail_meta_data_node *node = sail_image->meta_data_node;

    while (node != nullptr) {
        meta_data.push_back(sail::meta_data(node));
        node = node->next;
    }

    with_width(sail_image->width)
        .with_height(sail_image->height)
        .with_bytes_per_line(sail_image->bytes_per_line)
        .with_resolution(sail::resolution(sail_image->resolution))
        .with_pixel_format(sail_image->pixel_format)
        .with_animated(sail_image->animated)
        .with_delay(sail_image->delay)
        .with_palette(sail::palette(sail_image->palette))
        .with_meta_data(meta_data)
        .with_iccp(sail::iccp(sail_image->iccp))
        .with_properties(sail_image->properties)
        .with_source_image(sail::source_image(sail_image->source_image));

    if (sail_image->pixels != nullptr) {
        SAIL_TRY_OR_EXECUTE(transfer_pixels_pointer(sail_image),
                            /* on error */ return);
    }
}

sail_status_t image::transfer_pixels_pointer(const sail_image *sail_image)
{
    SAIL_CHECK_IMAGE_PTR(sail_image);

    sail_free(d->pixels);

    d->pixels         = nullptr;
    d->pixels_size    = 0;
    d->shallow_pixels = false;

    if (sail_image->pixels == nullptr) {
        return SAIL_OK;
    }

    unsigned bytes_per_image;
    SAIL_TRY(sail_bytes_per_image(sail_image, &bytes_per_image));

    d->pixels      = sail_image->pixels;
    d->pixels_size = bytes_per_image;

    return SAIL_OK;
}

sail_status_t image::to_sail_image(sail_image *sail_image) const
{
    SAIL_CHECK_IMAGE_PTR(sail_image);

    // Resulting meta entries
    sail_meta_data_node *image_meta_data_node = nullptr;
    sail_meta_data_node **last_meta_data_node = &image_meta_data_node;

    for (const sail::meta_data &meta_data : d->meta_data) {
        sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));

        SAIL_TRY_OR_CLEANUP(meta_data.to_sail_meta_data_node(meta_data_node),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                          sail_destroy_meta_data_node_chain(image_meta_data_node));

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;
    }

    sail_image->pixels         = d->pixels;
    sail_image->width          = d->width;
    sail_image->height         = d->height;
    sail_image->bytes_per_line = d->bytes_per_line;

    if (d->resolution.is_valid()) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_resolution(&sail_image->resolution),
                            /* cleanup */ sail_destroy_meta_data_node_chain(sail_image->meta_data_node));

        SAIL_TRY_OR_CLEANUP(d->resolution.to_sail_resolution(sail_image->resolution),
                            /* cleanup */ sail_destroy_meta_data_node_chain(sail_image->meta_data_node));
    }

    sail_image->pixel_format   = d->pixel_format;
    sail_image->animated       = d->animated;
    sail_image->delay          = d->delay;

    if (d->palette.is_valid()) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_palette(&sail_image->palette),
                            /* cleanup */ sail_destroy_meta_data_node_chain(sail_image->meta_data_node));

        SAIL_TRY_OR_CLEANUP(d->palette.to_sail_palette(sail_image->palette),
                            /* cleanup */ sail_destroy_palette(sail_image->palette),
                                          sail_destroy_meta_data_node_chain(sail_image->meta_data_node));
    }

    sail_image->meta_data_node = image_meta_data_node;

    if (d->iccp.is_valid()) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_iccp(&sail_image->iccp),
                            /* cleanup */ sail_destroy_palette(sail_image->palette),
                                          sail_destroy_meta_data_node_chain(sail_image->meta_data_node));

        SAIL_TRY_OR_CLEANUP(d->iccp.to_sail_iccp(sail_image->iccp),
                            /* cleanup */ sail_destroy_iccp(sail_image->iccp),
                                          sail_destroy_palette(sail_image->palette),
                                          sail_destroy_meta_data_node_chain(sail_image->meta_data_node));
    }

    sail_image->properties = d->properties;

    if (d->source_image.is_valid()) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&sail_image->source_image),
                            /* cleanup */ sail_destroy_iccp(sail_image->iccp),
                                          sail_destroy_palette(sail_image->palette),
                                          sail_destroy_meta_data_node_chain(sail_image->meta_data_node));

        SAIL_TRY_OR_CLEANUP(d->source_image.to_sail_source_image(sail_image->source_image),
                            /* cleanup */ sail_destroy_source_image(sail_image->source_image),
                                          sail_destroy_iccp(sail_image->iccp),
                                          sail_destroy_palette(sail_image->palette),
                                          sail_destroy_meta_data_node_chain(sail_image->meta_data_node));
    }

    return SAIL_OK;
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

image& image::with_source_image(const sail::source_image &si)
{
    d->source_image = si;
    return *this;
}

}
