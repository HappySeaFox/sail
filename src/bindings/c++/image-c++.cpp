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
#include "sail-manip.h"
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
    return d->width > 0 && d->height > 0 && d->bytes_per_line > 0 && d->pixels != nullptr && d->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN;
}

bool image::is_indexed() const
{
    return is_indexed(d->pixel_format);
}

bool image::is_grayscale() const
{
    return is_grayscale(d->pixel_format);
}

bool image::is_rgb_family() const
{
    return is_rgb_family(d->pixel_format);
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
    const unsigned bytes_per_image = height() * bytes_per_line();

    if (bytes_per_image == 0) {
        SAIL_LOG_ERROR("Cannot assign pixels as the image height or bytes_per_line is 0");
        return *this;
    }

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
    const unsigned bytes_per_image = height() * bytes_per_line();

    if (bytes_per_image == 0) {
        SAIL_LOG_ERROR("Cannot assign shallow pixels as the image height or bytes_per_line is 0");
        return *this;
    }

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

bool image::can_convert(SailPixelFormat pixel_format)
{
    if (!is_valid()) {
        return false;
    }

    return sail_can_convert(d->pixel_format, pixel_format);
}

sail_status_t image::convert(SailPixelFormat pixel_format) {

    SAIL_TRY(convert(pixel_format, conversion_options{}));

    return SAIL_OK;
}

sail_status_t image::convert(SailPixelFormat pixel_format, const conversion_options &options) {

    if (!is_valid()) {
        SAIL_LOG_ERROR("Conversion failed as the input image is invalid");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    sail_conversion_options *sail_conversion_options = nullptr;
    sail_image *sail_img = nullptr;

    SAIL_AT_SCOPE_EXIT(
        if (sail_img != nullptr) {
            if (sail_img->palette != nullptr) {
                sail_img->palette->data = nullptr;
            }

            sail_img->pixels = nullptr;
            sail_destroy_image(sail_img);
        }

        sail_destroy_conversion_options(sail_conversion_options);
    );

    SAIL_TRY(options.to_sail_conversion_options(&sail_conversion_options));

    SAIL_TRY(sail_alloc_image(&sail_img));

    sail_img->width          = d->width;
    sail_img->height         = d->height;
    sail_img->bytes_per_line = d->bytes_per_line;
    sail_img->pixel_format   = d->pixel_format;
    sail_img->pixels         = d->pixels;

    if (d->palette.is_valid()) {
        SAIL_TRY(sail_alloc_palette(&sail_img->palette));

        sail_img->palette->data         = const_cast<void *>(reinterpret_cast<const void *>(d->palette.data().data()));
        sail_img->palette->color_count  = d->palette.color_count();
        sail_img->palette->pixel_format = d->palette.pixel_format();
    }

    sail_image *sail_image_output = nullptr;
    SAIL_TRY(sail_convert_image_with_options(sail_img, pixel_format, sail_conversion_options, &sail_image_output));

    d->reset_pixels();

    d->bytes_per_line = sail_image_output->bytes_per_line;
    d->pixel_format   = sail_image_output->pixel_format;
    d->pixels         = sail_image_output->pixels;
    d->pixels_size    = sail_image_output->height * sail_image_output->bytes_per_line;
    d->shallow_pixels = false;

    sail_image_output->pixels = nullptr;
    sail_destroy_image(sail_image_output);

    return SAIL_OK;
}

sail_status_t image::convert(const write_features &wf) {

    SAIL_TRY(convert(wf, conversion_options{}));

    return SAIL_OK;
}

sail_status_t image::convert(const write_features &wf, const conversion_options &options) {

    if (!is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SailPixelFormat best_pixel_format = closest_pixel_format(d->pixel_format, wf);

    if (best_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("Failed to find the best output format for saving %s image", sail_pixel_format_to_string(d->pixel_format));

        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (best_pixel_format == d->pixel_format) {
        return SAIL_OK;
    } else {
        SAIL_TRY(convert(best_pixel_format, options));
    }

    return SAIL_OK;
}

sail_status_t image::convert_to(SailPixelFormat pixel_format, sail::image *image)
{
    SAIL_TRY(convert_to(pixel_format, conversion_options{}, image));

    return SAIL_OK;
}

sail_status_t image::convert_to(SailPixelFormat pixel_format, const conversion_options &options, sail::image *image)
{
    SAIL_CHECK_IMAGE_PTR(image);

    if (!is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    sail_conversion_options *sail_conversion_options = nullptr;

    sail_image *sail_img;
    SAIL_TRY(to_sail_image(&sail_img));

    SAIL_AT_SCOPE_EXIT(
        sail_img->pixels = nullptr;
        sail_destroy_image(sail_img);

        sail_destroy_conversion_options(sail_conversion_options);
    );

    SAIL_TRY(options.to_sail_conversion_options(&sail_conversion_options));

    sail_image *sail_image_output = nullptr;
    SAIL_TRY(sail_convert_image_with_options(sail_img, pixel_format, sail_conversion_options, &sail_image_output));

    *image = sail::image(sail_image_output);

    sail_image_output->pixels = nullptr;
    sail_destroy_image(sail_image_output);

    return SAIL_OK;
}

sail_status_t image::convert_to(const write_features &wf, sail::image *image)
{
    SAIL_TRY(convert_to(wf, conversion_options{}, image));

    return SAIL_OK;
}

sail_status_t image::convert_to(const write_features &wf, const conversion_options &options, sail::image *image)
{
    if (!is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SailPixelFormat best_pixel_format = closest_pixel_format(d->pixel_format, wf);

    if (best_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("Failed to find the best output format for saving %s image", sail_pixel_format_to_string(d->pixel_format));

        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (best_pixel_format == d->pixel_format) {
        *image = *this;
        return SAIL_OK;
    } else {
        SAIL_TRY(convert_to(best_pixel_format, options, image));
    }

    return SAIL_OK;
}

image image::convert_to(SailPixelFormat pixel_format)
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(pixel_format, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(SailPixelFormat pixel_format, const conversion_options &options)
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(pixel_format, options, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(const write_features &wf)
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(wf, conversion_options{}, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(const write_features &wf, const conversion_options &options)
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(wf, options, &img),
                        /* on error */ return img);

    return img;
}

SailPixelFormat image::closest_pixel_format(const std::vector<SailPixelFormat> &pixel_formats)
{
    return sail_closest_pixel_format(d->pixel_format, pixel_formats.data(), pixel_formats.size());
}

SailPixelFormat image::closest_pixel_format(const write_features &wf)
{
    return sail_closest_pixel_format(d->pixel_format, wf.output_pixel_formats().data(), wf.output_pixel_formats().size());
}

bool image::can_convert(SailPixelFormat input_pixel_format, SailPixelFormat output_pixel_format) {

    return sail_can_convert(input_pixel_format, output_pixel_format);
}

SailPixelFormat image::closest_pixel_format(SailPixelFormat input_pixel_format, const std::vector<SailPixelFormat> &pixel_formats)
{
    return sail_closest_pixel_format(input_pixel_format, pixel_formats.data(), pixel_formats.size());
}

SailPixelFormat image::closest_pixel_format(SailPixelFormat input_pixel_format, const write_features &wf)
{
    return sail_closest_pixel_format(input_pixel_format, wf.output_pixel_formats().data(), wf.output_pixel_formats().size());
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

bool image::is_indexed(SailPixelFormat pixel_format)
{
    return sail_is_indexed(pixel_format);
}

bool image::is_grayscale(SailPixelFormat pixel_format)
{
    return sail_is_grayscale(pixel_format);
}

bool image::is_rgb_family(SailPixelFormat pixel_format)
{
    return sail_is_rgb_family(pixel_format);
}

const char* image::pixel_format_to_string(SailPixelFormat pixel_format)
{
    return sail_pixel_format_to_string(pixel_format);
}

SailPixelFormat image::pixel_format_from_string(const std::string_view str)
{
    return sail_pixel_format_from_string(str.data());
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

const char* image::compression_to_string(SailCompression compression)
{
    return sail_compression_to_string(compression);
}

SailCompression image::compression_from_string(const std::string_view str)
{
    return sail_compression_from_string(str.data());
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

    d->pixels      = sail_image->pixels;
    d->pixels_size = sail_image->height * sail_image->bytes_per_line;

    return SAIL_OK;
}

sail_status_t image::to_sail_image(sail_image **image) const
{
    SAIL_CHECK_IMAGE_PTR(image);

    sail_image *image_local = nullptr;

    SAIL_AT_SCOPE_EXIT(
        // Pixels are shallow copied
        if (image_local != nullptr) {
            image_local->pixels = nullptr;
        }

        sail_destroy_image(image_local);
    );

    SAIL_TRY(sail_alloc_image(&image_local));

    // Pixels are shallow copied
    image_local->pixels         = d->pixels;
    image_local->width          = d->width;
    image_local->height         = d->height;
    image_local->bytes_per_line = d->bytes_per_line;

    // Resulting meta entries
    sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

    for (const sail::meta_data &meta_data : d->meta_data) {
        sail_meta_data_node *meta_data_node;

        SAIL_TRY(meta_data.to_sail_meta_data_node(&meta_data_node));

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;
    }

    if (d->resolution.is_valid()) {
        SAIL_TRY(d->resolution.to_sail_resolution(&image_local->resolution));
    }

    image_local->pixel_format = d->pixel_format;
    image_local->animated     = d->animated;
    image_local->delay        = d->delay;

    if (d->palette.is_valid()) {
        SAIL_TRY(d->palette.to_sail_palette(&image_local->palette));
    }

    if (d->iccp.is_valid()) {
        SAIL_TRY(d->iccp.to_sail_iccp(&image_local->iccp));
    }

    image_local->properties = d->properties;

    if (d->source_image.is_valid()) {
        SAIL_TRY(d->source_image.to_sail_source_image(&image_local->source_image));
    }

    *image = image_local;
    image_local = nullptr;

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
