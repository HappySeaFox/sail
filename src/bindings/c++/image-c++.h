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

#ifndef SAIL_IMAGE_CPP_H
#define SAIL_IMAGE_CPP_H

#include <string_view>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "iccp-c++.h"
    #include "palette-c++.h"
    #include "source_image-c++.h"
    #include "resolution-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/iccp-c++.h>
    #include <sail-c++/palette-c++.h>
    #include <sail-c++/source_image-c++.h>
    #include <sail-c++/resolution-c++.h>
#endif

namespace sail {
    class meta_data;
}

struct sail_image;

namespace sail
{

/*
 * Image representation with direct access to the pixel data.
 */
class SAIL_EXPORT image
{
    friend class image_reader;
    friend class image_writer;

public:
    image();
    image(const image &img);
    image& operator=(const image &img);
    image(image &&img) noexcept;
    image& operator=(image &&img);
    ~image();

    /*
     * Returns true if the image has valid dimensions, bytes-per-line,
     * and the pixel data (deep copied or shallow).
     */
    bool is_valid() const;

    bool is_indexed() const;

    bool is_grayscale() const;

    /*
     * Returns image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    unsigned width() const;

    /*
     * Returns image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    unsigned height() const;

    /*
     * Returns bytes per line. Some image formats (like BMP) pad rows of pixels to some boundary.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *        it with bytes_per_line_auto() if scan lines are not padded to a certain boundary.
     */
    unsigned bytes_per_line() const;

    /*
     * Image resolution.
     *
     * READ:  Set by SAIL to a valid resolution if this information is available.
     * WRITE: Must be set by a caller to a valid image resolution if necessary.
     */
    const sail::resolution& resolution() const;

    /*
     * Returns image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by a certain codec can be obtained from read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *        to the codec by a caller later. The list of supported input pixel formats by a certain codec
     *        can be obtained from write_features.output_pixel_formats.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns true if the image a frame in an animation.
     *
     * READ:  Set by SAIL to true if the image is a frame in an animation.
     * WRITE: Must be set by a caller to true if the image is a frame in an animation.
     *        Codecs may need to know if they write a static or an animated image.
     */
    bool animated() const;

    /*
     * Returns delay in milliseconds to display the image on the screen if the image is a frame
     * in an animation or 0 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds.
     */
    int delay() const;

    /*
     * Returns palette if the image has a palette and the requested pixel format assumes having a palette.
     *
     * READ:  Set by SAIL to a valid palette if the image is indexed and the requested pixel format
     *        assumes having a palette.
     * WRITE: Must be set by a caller to a valid palette if the image is indexed.
     */
    const sail::palette& palette() const;

    /*
     * Returns image meta data.
     *
     * READ:  Set by SAIL to a valid map with meta data (like JPEG comments).
     * WRITE: Must be set by a caller to a valid map with meta data
     *        (like JPEG comments) if necessary.
     */
    const std::vector<sail::meta_data>& meta_data() const;

    /*
     * Returns embedded ICC profile.
     *
     * Note for animated/multi-paged images: only the first image in an animated/multi-paged
     * sequence might have an ICC profile.
     *
     * READ:  Set by SAIL to a valid ICC profile if any.
     * WRITE: Must be set by a caller to a valid ICC profile if necessary.
     */
    const sail::iccp& iccp() const;

    /*
     * Returns or-ed decoded image properties. See SailImageProperty.
     *
     * READ:  Set by SAIL to valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g., flip back etc.).
     * WRITE: Ignored.
     */
    int properties() const;

    /*
     * Source image properties.
     *
     * READ:  Set by SAIL to valid source image properties of the original image.
     * WRITE: Ignored.
     */
    const sail::source_image& source_image() const;

    /*
     * Returns the editable pixel data if any. Images hold deep copied or shallow data, but not both.
     *
     * READ:  Set by SAIL to valid pixel data.
     * WRITE: Must be set by a caller to valid pixel data using with_pixels() or with_shallow_pixels().
     */
    void* pixels();

    /*
     * Returns the constant pixel data if any. Images hold deep copied or shallow data, but not both.
     *
     * READ:  Set by SAIL to valid pixel data.
     * WRITE: Must be set by a caller to valid pixel data using with_pixels() or with_shallow_pixels().
     */
    const void* pixels() const;

    /*
     * Returns the size of deep copied pixel data in bytes.
     */
    unsigned pixels_size() const;

    /*
     * Sets a new width.
     */
    image& with_width(unsigned width);

    /*
     * Sets a new height.
     */
    image& with_height(unsigned height);

    /*
     * Sets a new bytes-per-line value.
     */
    image& with_bytes_per_line(unsigned bytes_per_line);

    /*
     * Calculates bytes-per-line automatically based on the image width
     * and the pixel format. These two properties must be set beforehand.
     */
    image& with_bytes_per_line_auto();

    /*
     * Sets a new resolution.
     */
    image& with_resolution(const sail::resolution &res);

    /*
     * Sets a new pixel format. See SailPixelFormat.
     */
    image& with_pixel_format(SailPixelFormat pixel_format);

    /*
     * Sets a new delay for an animated frame in a sequence.
     */
    image& with_delay(int delay);

    /*
     * Deep copies the specified palette.
     */
    image& with_palette(const sail::palette &pal);

    /*
     * Sets new meta entries.
     */
    image& with_meta_data(const std::vector<sail::meta_data> &md);

    /*
     * Deep copies the specified pixel data. The data can be accessed later with pixels().
     * The size of the pixel data is calculated based on the image width, height, and the pixel
     * format which must be set beforehand. The deep copied data is deleted upon image destruction.
     */
    image& with_pixels(const void *pixels);

    /*
     * Deep copies the specified pixel data and stores its size. The data can be accessed later with pixels().
     * The deep copied data is deleted upon image destruction.
     */
    image& with_pixels(const void *pixels, unsigned pixels_size);

    /*
     * Stores the pointer to the external pixel data. Frees the previously stored deep-copied pixel data.
     * The pixel data must remain valid until the image exists. The shallow data is not deleted upon
     * image destruction.
     *
     * The size of the pixel data is calculated based on the image width, height, and the pixel
     * format which must be set beforehand.
     */
    image& with_shallow_pixels(void *pixels);

    /*
     * Stores the pointer to the external pixel data and stores its size. Frees the previously stored
     * deep-copied pixel data. The pixel data must remain valid until the image exists. The shallow data
     * is not deleted upon image destruction.
     */
    image& with_shallow_pixels(void *pixels, unsigned pixels_size);

    /*
     * Sets a new ICC profile.
     */
    image& with_iccp(const sail::iccp &ic);

    /*
     * Calculates the number of bits per pixel in the specified pixel format.
     * For example, for SAIL_PIXEL_FORMAT_RGB 24 is assigned.
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t bits_per_pixel(SailPixelFormat pixel_format, unsigned *result);

    /*
     * Calculates the number of bytes per line needed to hold a scan line without padding.
     *
     * For example:
     *   - 12 pixels * 1 bits per pixel / 8 + 1 ==
     *     12 * 1/8 + 1                         ==
     *     12 * 0.125 + 1                       ==
     *     1.5 + 1                              ==
     *     2.5                                  ==
     *     2 bytes per line
     *
     *   - 12 pixels * 16 bits per pixel / 8 + 0 ==
     *     12 * 16/8 + 0                         ==
     *     12 * 2 + 0                            ==
     *     24 + 0                                ==
     *     24 bytes per line
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t bytes_per_line(unsigned width, SailPixelFormat pixel_format, unsigned *result);

    static bool is_indexed(SailPixelFormat pixel_format);

    static bool is_grayscale(SailPixelFormat pixel_format);

    /*
     * Assigns a non-NULL string representation of the specified pixel format.
     * The assigned string MUST NOT be destroyed. For example: "RGB".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t pixel_format_to_string(SailPixelFormat pixel_format, const char **result);

    /*
     * Assigns pixel format from a string representation.
     * For example: SAIL_PIXEL_FORMAT_SOURCE is assigned for "SOURCE".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t pixel_format_from_string(std::string_view str, SailPixelFormat *result);

    /*
     * Assigns a non-NULL string representation of the specified image property. See SailImageProperty.
     * The assigned string MUST NOT be destroyed. For example: "FLIPPED-VERTICALLY".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t image_property_to_string(SailImageProperty image_property, const char **result);

    /*
     * Assigns image property from a string representation. See SailImageProperty.
     * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is assigned for "FLIPPED-VERTICALLY".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t image_property_from_string(std::string_view str, SailImageProperty *result);

    /*
     * Assigns a non-NULL string representation of the specified compression type. See SailCompression.
     * The assigned string MUST NOT be destroyed. For example: "RLE".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t compression_to_string(SailCompression compression, const char **result);

    /*
     * Assigns compression from a string representation. See SailCompression.
     * For example: SAIL_COMPRESSION_RLE is assigned for "RLE".
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t compression_from_string(std::string_view str, SailCompression *result);

private:
    /*
     * Makes a deep copy of the specified image. The pixels are transferred. The caller must set the pixels
     * in the sail_image object to NULL afterwards to avoid destructing them in sail_destroy_image().
     */
    image(const sail_image *sail_image);

    sail_status_t transfer_pixels_pointer(const sail_image *sail_image);

    sail_status_t to_sail_image(sail_image *sail_image) const;

    image& with_animated(bool animated);
    image& with_properties(int properties);
    image& with_source_image(const sail::source_image &si);

private:
    class pimpl;
    pimpl *d;
};

}

#endif
