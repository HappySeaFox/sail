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
#include <stdexcept>
#include <utility>

#include "sail-c++.h"
#include "sail.h"
#include "sail-manip.h"

namespace sail
{

class SAIL_HIDDEN image::pimpl
{
public:
    pimpl()
        : sail_image(nullptr)
        , pixels_size(0)
        , shallow_pixels(false)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_image(&sail_image),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        if (shallow_pixels) {
            sail_image->pixels = nullptr;
        }

        sail_destroy_image(sail_image);
    }

    void reset_pixels()
    {
        if (!shallow_pixels) {
            sail_free(sail_image->pixels);
        }

        sail_image->pixels = nullptr;
        pixels_size        = 0;
        shallow_pixels     = false;
    }

    struct sail_image *sail_image;
    sail::resolution resolution;
    sail::palette palette;
    std::vector<sail::meta_data> meta_data;
    sail::iccp iccp;
    sail::source_image source_image;
    unsigned pixels_size;
    bool shallow_pixels;
};

image::image()
    : d(new pimpl)
{
}

image::image(const std::string &path)
    : image()
{
    load(path);
}

image::image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height)
    : image()
{
    set_dimensions(width, height);
    set_pixel_format(pixel_format);
    set_bytes_per_line_auto();
    set_shallow_pixels(pixels);
}

image::image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height, unsigned bytes_per_line)
    : image()
{
    set_dimensions(width, height);
    set_pixel_format(pixel_format);
    set_bytes_per_line(bytes_per_line);
    set_shallow_pixels(pixels);
}

image::image(const image &img)
    : image()
{
    *this = img;
}

image& image::operator=(const sail::image &image)
{
    set_dimensions(image.width(), image.height());
    set_bytes_per_line(image.bytes_per_line());
    set_resolution(image.resolution());
    set_pixel_format(image.pixel_format());
    set_gamma(image.gamma());
    set_delay(image.delay());
    set_palette(image.palette());
    set_meta_data(image.meta_data());
    set_iccp(image.iccp());
    set_orientation(image.orientation());
    set_source_image(image.source_image());
    set_pixels(image.pixels(), image.pixels_size());

    return *this;
}

image::image(sail::image &&image) noexcept
{
    *this = std::move(image);
}

image& image::operator=(sail::image &&image) noexcept
{
    d = std::move(image.d);

    return *this;
}

image::~image()
{
}

bool image::is_valid() const
{
    return d->sail_image != nullptr && d->sail_image->width > 0 && d->sail_image->height > 0 && d->sail_image->bytes_per_line > 0 &&
            d->sail_image->pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN && d->sail_image->pixels != nullptr;
}

bool image::is_indexed() const
{
    return is_indexed(d->sail_image->pixel_format);
}

bool image::is_grayscale() const
{
    return is_grayscale(d->sail_image->pixel_format);
}

bool image::is_rgb_family() const
{
    return is_rgb_family(d->sail_image->pixel_format);
}

unsigned image::width() const
{
    return d->sail_image->width;
}

unsigned image::height() const
{
    return d->sail_image->height;
}

unsigned image::bytes_per_line() const
{
    return d->sail_image->bytes_per_line;
}

const sail::resolution& image::resolution() const
{
    return d->resolution;
}

SailPixelFormat image::pixel_format() const
{
    return d->sail_image->pixel_format;
}

double image::gamma() const
{
    return d->sail_image->gamma;
}

int image::delay() const
{
    return d->sail_image->delay;
}

const sail::palette& image::palette() const
{
    return d->palette;
}

const std::vector<sail::meta_data>& image::meta_data() const
{
    return d->meta_data;
}

std::vector<sail::meta_data>& image::meta_data()
{
    return d->meta_data;
}

const sail::iccp& image::iccp() const
{
    return d->iccp;
}

SailOrientation image::orientation() const
{
    return d->sail_image->orientation;
}

const sail::source_image& image::source_image() const
{
    return d->source_image;
}

void* image::pixels()
{
    return d->sail_image->pixels;
}

const void* image::pixels() const
{
    return d->sail_image->pixels;
}

void* image::scan_line(unsigned i)
{
    return reinterpret_cast<char *>(pixels()) + i * bytes_per_line();
}

const void* image::scan_line(unsigned i) const
{
    return reinterpret_cast<const char *>(pixels()) + i * bytes_per_line();
}

unsigned image::pixels_size() const
{
    return d->pixels_size;
}

void image::set_resolution(const sail::resolution &resolution)
{
    d->resolution = resolution;
}

void image::set_resolution(sail::resolution &&resolution) noexcept
{
    d->resolution = std::move(resolution);
}

void image::set_gamma(double gamma)
{
    d->sail_image->gamma = gamma;
}

void image::set_delay(int delay)
{
    d->sail_image->delay = delay;
}

void image::set_palette(const sail::palette &palette)
{
    d->palette = palette;
}

void image::set_palette(sail::palette &&palette) noexcept
{
    d->palette = std::move(palette);
}

void image::set_meta_data(const std::vector<sail::meta_data> &meta_data)
{
    d->meta_data = meta_data;
}

void image::set_meta_data(std::vector<sail::meta_data> &&meta_data) noexcept
{
    d->meta_data = std::move(meta_data);
}

void image::set_iccp(const sail::iccp &iccp)
{
    d->iccp = iccp;
}

void image::set_iccp(sail::iccp &&iccp) noexcept
{
    d->iccp = std::move(iccp);
}

sail_status_t image::load(const std::string &path)
{
    image_input input;
    SAIL_TRY(input.start(path));

    image img;
    SAIL_TRY(input.next_frame(&img));

    *this = std::move(img);

    return SAIL_OK;
}

sail_status_t image::save(const std::string &path)
{
    image_output output;

    SAIL_TRY(output.save(path, *this));

    return SAIL_OK;
}

bool image::can_convert(SailPixelFormat pixel_format)
{
    if (!is_valid()) {
        return false;
    }

    return sail_can_convert(d->sail_image->pixel_format, pixel_format);
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

    sail_img->width          = d->sail_image->width;
    sail_img->height         = d->sail_image->height;
    sail_img->bytes_per_line = d->sail_image->bytes_per_line;
    sail_img->pixel_format   = d->sail_image->pixel_format;
    sail_img->pixels         = d->sail_image->pixels;

    if (d->palette.is_valid()) {
        SAIL_TRY(sail_alloc_palette(&sail_img->palette));

        sail_img->palette->data         = const_cast<void *>(reinterpret_cast<const void *>(d->palette.data().data()));
        sail_img->palette->color_count  = d->palette.color_count();
        sail_img->palette->pixel_format = d->palette.pixel_format();
    }

    sail_image *sail_image_output = nullptr;
    SAIL_TRY(sail_convert_image_with_options(sail_img, pixel_format, sail_conversion_options, &sail_image_output));

    d->reset_pixels();

    d->sail_image->bytes_per_line = sail_image_output->bytes_per_line;
    d->sail_image->pixel_format   = sail_image_output->pixel_format;
    d->sail_image->pixels         = sail_image_output->pixels;
    d->pixels_size                = sail_image_output->height * sail_image_output->bytes_per_line;
    d->shallow_pixels             = false;

    sail_image_output->pixels = nullptr;
    sail_destroy_image(sail_image_output);

    return SAIL_OK;
}

sail_status_t image::convert(const sail::save_features &save_features) {

    SAIL_TRY(convert(save_features, conversion_options{}));

    return SAIL_OK;
}

sail_status_t image::convert(const sail::save_features &save_features, const conversion_options &options) {

    if (!is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SailPixelFormat best_pixel_format = closest_pixel_format(d->sail_image->pixel_format, save_features);

    if (best_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("Failed to find the best output format for saving %s image", sail_pixel_format_to_string(d->sail_image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (best_pixel_format == d->sail_image->pixel_format) {
        return SAIL_OK;
    } else {
        SAIL_TRY(convert(best_pixel_format, options));
    }

    return SAIL_OK;
}

sail_status_t image::convert_to(SailPixelFormat pixel_format, sail::image *image) const
{
    SAIL_TRY(convert_to(pixel_format, conversion_options{}, image));

    return SAIL_OK;
}

sail_status_t image::convert_to(SailPixelFormat pixel_format, const conversion_options &options, sail::image *image) const
{
    SAIL_CHECK_PTR(image);

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

sail_status_t image::convert_to(const sail::save_features &save_features, sail::image *image) const
{
    SAIL_TRY(convert_to(save_features, conversion_options{}, image));

    return SAIL_OK;
}

sail_status_t image::convert_to(const sail::save_features &save_features, const conversion_options &options, sail::image *image) const
{
    if (!is_valid()) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    SailPixelFormat best_pixel_format = closest_pixel_format(d->sail_image->pixel_format, save_features);

    if (best_pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("Failed to find the best output format for saving %s image", sail_pixel_format_to_string(d->sail_image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    if (best_pixel_format == d->sail_image->pixel_format) {
        *image = *this;
        return SAIL_OK;
    } else {
        SAIL_TRY(convert_to(best_pixel_format, options, image));
    }

    return SAIL_OK;
}

image image::convert_to(SailPixelFormat pixel_format) const
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(pixel_format, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(SailPixelFormat pixel_format, const conversion_options &options) const
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(pixel_format, options, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(const sail::save_features &save_features) const
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(save_features, conversion_options{}, &img),
                        /* on error */ return img);

    return img;
}

image image::convert_to(const sail::save_features &save_features, const conversion_options &options) const
{
    image img;
    SAIL_TRY_OR_EXECUTE(convert_to(save_features, options, &img),
                        /* on error */ return img);

    return img;
}

SailPixelFormat image::closest_pixel_format(const std::vector<SailPixelFormat> &pixel_formats) const
{
    return sail_closest_pixel_format(d->sail_image->pixel_format, pixel_formats.data(), pixel_formats.size());
}

SailPixelFormat image::closest_pixel_format(const sail::save_features &save_features) const
{
    return sail_closest_pixel_format(d->sail_image->pixel_format, save_features.pixel_formats().data(), save_features.pixel_formats().size());
}

bool image::can_convert(SailPixelFormat input_pixel_format, SailPixelFormat output_pixel_format)
{
    return sail_can_convert(input_pixel_format, output_pixel_format);
}

SailPixelFormat image::closest_pixel_format(SailPixelFormat input_pixel_format, const std::vector<SailPixelFormat> &pixel_formats)
{
    return sail_closest_pixel_format(input_pixel_format, pixel_formats.data(), pixel_formats.size());
}

SailPixelFormat image::closest_pixel_format(SailPixelFormat input_pixel_format, const sail::save_features &save_features)
{
    return sail_closest_pixel_format(input_pixel_format, save_features.pixel_formats().data(), save_features.pixel_formats().size());
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

SailPixelFormat image::pixel_format_from_string(const std::string &str)
{
    return sail_pixel_format_from_string(str.c_str());
}

const char* chroma_subsampling_to_string(SailChromaSubsampling chroma_subsampling)
{
    return sail_chroma_subsampling_to_string(chroma_subsampling);
}

SailChromaSubsampling chroma_subsampling_from_string(const std::string &str)
{
    return sail_chroma_subsampling_from_string(str.data());
}

const char* image::orientation_to_string(SailOrientation orientation)
{
    return sail_orientation_to_string(orientation);
}

SailOrientation image::orientation_from_string(const std::string &str)
{
    return sail_orientation_from_string(str.data());
}

const char* image::compression_to_string(SailCompression compression)
{
    return sail_compression_to_string(compression);
}

SailCompression image::compression_from_string(const std::string &str)
{
    return sail_compression_from_string(str.c_str());
}

image::image(const sail_image *sail_image)
    : image()
{
    if (sail_image == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::image(). The object is untouched");
        return;
    }

    std::vector<sail::meta_data> meta_data;

    for(const sail_meta_data_node *node = sail_image->meta_data_node; node != nullptr; node = node->next) {
        meta_data.push_back(sail::meta_data(node->meta_data));
    }

    set_dimensions(sail_image->width, sail_image->height);
    set_bytes_per_line(sail_image->bytes_per_line);
    set_resolution(sail::resolution(sail_image->resolution));
    set_pixel_format(sail_image->pixel_format);
    set_gamma(sail_image->gamma);
    set_delay(sail_image->delay);
    set_palette(sail::palette(sail_image->palette));
    set_meta_data(meta_data);
    set_iccp(sail::iccp(sail_image->iccp));
    set_orientation(sail_image->orientation);
    set_source_image(sail::source_image(sail_image->source_image));

    if (sail_image->pixels != nullptr) {
        SAIL_TRY_OR_EXECUTE(transfer_pixels_pointer(sail_image),
                            /* on error */ return);
    }
}

sail_status_t image::transfer_pixels_pointer(const sail_image *sail_image)
{
    SAIL_CHECK_PTR(sail_image);

    sail_free(d->sail_image->pixels);

    d->sail_image->pixels = nullptr;
    d->pixels_size        = 0;
    d->shallow_pixels     = false;

    if (sail_image->pixels == nullptr) {
        return SAIL_OK;
    }

    d->sail_image->pixels = sail_image->pixels;
    d->pixels_size        = sail_image->height * sail_image->bytes_per_line;

    return SAIL_OK;
}

sail_status_t image::to_sail_image(sail_image **image) const
{
    SAIL_CHECK_PTR(image);

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
    image_local->pixels         = d->sail_image->pixels;
    image_local->width          = d->sail_image->width;
    image_local->height         = d->sail_image->height;
    image_local->bytes_per_line = d->sail_image->bytes_per_line;

    // Resulting meta entries
    sail_meta_data_node **last_meta_data_node = &image_local->meta_data_node;

    for (const sail::meta_data &meta_data : d->meta_data) {
        sail_meta_data_node *meta_data_node;

        SAIL_TRY(sail_alloc_meta_data_node(&meta_data_node));
        SAIL_TRY_OR_CLEANUP(meta_data.to_sail_meta_data(&meta_data_node->meta_data),
                            /* cleanup */ sail_destroy_meta_data_node(meta_data_node));

        *last_meta_data_node = meta_data_node;
        last_meta_data_node = &meta_data_node->next;
    }

    if (d->resolution.is_valid()) {
        SAIL_TRY(d->resolution.to_sail_resolution(&image_local->resolution));
    }

    image_local->pixel_format = d->sail_image->pixel_format;
    image_local->delay        = d->sail_image->delay;

    if (d->palette.is_valid()) {
        SAIL_TRY(d->palette.to_sail_palette(&image_local->palette));
    }

    if (d->iccp.is_valid()) {
        SAIL_TRY(d->iccp.to_sail_iccp(&image_local->iccp));
    }

    image_local->orientation = d->sail_image->orientation;

    if (d->source_image.is_valid()) {
        SAIL_TRY(d->source_image.to_sail_source_image(&image_local->source_image));
    }

    *image = image_local;
    image_local = nullptr;

    return SAIL_OK;
}

void image::set_dimensions(unsigned width, unsigned height)
{
    d->sail_image->width = width;
    d->sail_image->height = height;
}

void image::set_pixel_format(SailPixelFormat pixel_format)
{
    d->sail_image->pixel_format = pixel_format;
}

void image::set_bytes_per_line(unsigned bytes_per_line)
{
    d->sail_image->bytes_per_line = bytes_per_line;
}

void image::set_bytes_per_line_auto()
{
    unsigned bytes_per_line = 0;
    image::bytes_per_line(d->sail_image->width, d->sail_image->pixel_format, &bytes_per_line);

    return set_bytes_per_line(bytes_per_line);
}

void image::set_pixels(const void *pixels)
{
    set_pixels(pixels, height() * bytes_per_line());
}

void image::set_pixels(const void *pixels, unsigned pixels_size)
{
    d->reset_pixels();

    if (pixels == nullptr || pixels_size == 0) {
        return;
    }

    SAIL_TRY_OR_EXECUTE(sail_malloc(pixels_size, &d->sail_image->pixels),
                        /* on error */ return);

    memcpy(d->sail_image->pixels, pixels, pixels_size);
    d->pixels_size    = pixels_size;
    d->shallow_pixels = false;
}

void image::set_shallow_pixels(void *pixels)
{
    set_shallow_pixels(pixels, height() * bytes_per_line());
}

void image::set_shallow_pixels(void *pixels, unsigned pixels_size)
{
    d->reset_pixels();

    if (pixels == nullptr || pixels_size == 0) {
        return;
    }

    d->sail_image->pixels = pixels;
    d->pixels_size        = pixels_size;
    d->shallow_pixels     = true;
}

void image::set_orientation(SailOrientation orientation)
{
    d->sail_image->orientation = orientation;
}

void image::set_source_image(const sail::source_image &source_image)
{
    d->source_image = source_image;
}

}
