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

#ifndef SAIL_IMAGE_CPP_H
#define SAIL_IMAGE_CPP_H

#include <map>
#include <string>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

struct sail_image;

namespace sail
{

class plugin_info;

/*
 * Image representation with direct access to the pixel data.
 */
class SAIL_EXPORT image
{
    friend class image_reader;
    friend class image_writer;

public:
    image();
    image(const image &image);
    image& operator=(const image &image);
    ~image();

    /*
     * Returns true if the image has valid dimensions, bytes-per-line,
     * and pixel data (deep copied or shallow).
     */
    bool is_valid() const;

    /*
     * Returns image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    int width() const;

    /*
     * Returns image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    int height() const;

    /*
     * Returns bytes per line. Some image formats (like BMP) pad rows of pixels to some boundary.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *        it with bytes_per_line_auto() if scan lines are not padded to a certain booundary.
     */
    int bytes_per_line() const;

    /*
     * Returns image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by a certain plugin could be obtained from read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. The list of supported input pixel
     *        formats by a certain plugin could be obtained from write_features.output_pixel_formats.
     */
    int pixel_format() const;

    /*
     * Returns number of passes needed to read or write an entire image frame. 1 by default.
     *
     * READ:  Set by SAIL to a positive number of passes needed to read an image. For example, interlaced PNGs
     *        have 8 passes.
     * WRITE: Ignored. Use write_features.passes to determine the actual number of passes needed to write
     *        an interlaced image.
     */
    int passes() const;

    /*
     * Returns true if the image is a frame in an animation.
     *
     * READ:  Set by SAIL to true if the image is a frame in an animation.
     * WRITE: Ignored.
     */
    bool animated() const;

    /*
     * Returns delay in milliseconds if the image is a frame in an animation or 0 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds.
     */
    int delay() const;

    /*
     * Returns palette pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid palette pixel format if the image is indexed (palette is not NULL).
     * WRITE: Must be set by a caller to a valid palette pixel format if the image is indexed
     *        (palette is not NULL).
     */
    int palette_pixel_format() const;

    /*
     * Returns palette data if the image has a palette and the requested pixel format assumes having a palette.
     *
     * READ:  Set by SAIL to a valid pixel data if the image is indexed.
     * WRITE: Must be set by a caller to a valid pixel data if the image is indexed.
     */
    void* palette() const;

    /*
     * Returns size of the palette data in bytes.
     *
     * READ:  Set by SAIL to a valid palette size in bytes if the image is indexed or to 0.
     * WRITE: Must be set by a caller to a valid palette size in bytes if the image is indexed.
     */
    int palette_size() const;

    /*
     * Returns image meta information.
     *
     * READ:  Set by SAIL to a valid map with simple meta information (like JPEG comments).
     * WRITE: Must be set by a caller to a valid map with simple meta information
     *        (like JPEG comments) if necessary.
     */
    std::map<std::string, std::string> meta_entries() const;

    /*
     * Returns decoded image properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g. flip back etc.).
     * WRITE: Ignored.
     */
    int properties() const;

    /*
     * Returns image source pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid source image pixel format before converting it to a requested pixel format
     *        with read_options.pixel_format.
     * WRITE: Ignored.
     */
    int source_pixel_format() const;

    /*
     * Returns image source properties. See SailImageProperties.
     *
     * READ:  Set by SAIL to a valid source image properties or to 0.
     * WRITE: Ignored.
     */
    int source_properties() const;

    /*
     * Image source compression type. See SailCompressionTypes.
     *
     * READ:  Set by SAIL to a valid source image compression type if the image pixels are compressed or to 0.
     * WRITE: Ignored.
     */
    int source_compression_type() const;

    /*
     * Returns editable deep copied pixel data if any. Images can hold deep copied or shallow data,
     * but not both.
     */
    void* bits();

    /*
     * Returns constant deep copied pixel data if any. Images can hold deep copied or shallow data,
     * but not both.
     */
    const void* bits() const;

    /*
     * Returns the size of deep copied pixel data in bytes.
     */
    int bits_size() const;

    /*
     * Returns a constant shallow pointer to external pixel data if any. Images can hold deep copied
     * or shallow data, but not both.
     */
    const void* shallow_bits() const;

    /*
     * Sets a new width.
     */
    image& with_width(int width);

    /*
     * Sets a new height.
     */
    image& with_height(int height);

    /*
     * Sets a new bytes-per-line value.
     */
    image& with_bytes_per_line(int bytes_per_line);

    /*
     * Calculates bytes-per-line automatically based on the image width
     * and the pixel format. These two properties must be set beforehand.
     */
    image& with_bytes_per_line_auto();

    /*
     * Sets a new pixel format. See SailPixelFormat.
     */
    image& with_pixel_format(int pixel_format);

    /*
     * Sets a new delay for an animated frame in a sequence.
     */
    image& with_delay(int delay);

    /*
     * Deep copies the specified palette.
     */
    image& with_palette(void *palette, int palette_size, int palette_pixel_format);

    /*
     * Sets new meta entries.
     */
    image& with_meta_entries(const std::map<std::string, std::string> &meta_entries);

    /*
     * Deep copies the specified bits. Resets the pointer to shallow bits previously saved if any.
     */
    image& with_bits(const void *bits, int bits_size);

    /*
     * Stores the pointer to external data. Frees the previously stored deep-copied bits if any.
     * The pixel data must remain valid until the image exists.
     */
    image& with_shallow_bits(const void *bits);

    /*
     * Calculates a number of bits per pixel in the specified pixel format.
     * For example, for SAIL_PIXEL_FORMAT_BPP24_RGB 24 is assigned.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t bits_per_pixel(int pixel_format, int *result);

    /*
     * Calculates a number of bytes per line needed to hold a scan line without padding.
     * 'width' and 'pixel_format' fields are used to calculate a result.
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
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t bytes_per_line(const image &simage, int *result);

    /*
     * Calculates a number of bytes needed to hold an entire image in memory without padding.
     * It's effectively bytes per line * image height.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t bytes_per_image(const image &simage, int *result);

    /*
     * Assigns a non-NULL string representation of the specified pixel format. The assigned string
     * MUST NOT be destroyed. For example: "BPP24-RGB".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t pixel_format_to_string(int pixel_format, const char **result);

    /*
     * Assigns pixel format from a string representation.
     * For example: SAIL_PIXEL_FORMAT_SOURCE is assigned for "SOURCE".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t pixel_format_from_string(const char *str, int *result);

    /*
     * Assigns a non-NULL string representation of the specified image property. See SailImageProperties.
     * The assigned string MUST NOT be destroyed. For example: "FLIPPED-VERTICALLY".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t image_property_to_string(int image_property, const char **result);

    /*
     * Assigns image property from a string representation or 0. See SailImageProperties.
     * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is assigned for "FLIPPED-VERTICALLY".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t image_property_from_string(const char *str, int *result);

    /*
     * Assigns a non-NULL string representation of the specified compression type. See SailCompressionTypes.
     * The assigned string MUST NOT be destroyed. For example: "RLE".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t compression_type_to_string(int compression, const char **result);

    /*
     * Assigns compression from a string representation or 0. See SailCompressionTypes.
     * For example: SAIL_COMPRESSION_RLE is assigned for "RLE".
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t compression_type_from_string(const char *str, int *result);

private:
    // Makes a deep copy of the specified image and the bits
    //
    image(const sail_image *im, const void *bits, int bits_size);
    image(const sail_image *im);

    sail_error_t to_sail_image(sail_image *image) const;

    image& with_passes(int passes);
    image& with_animated(bool animated);
    image& with_properties(int properties);
    image& with_source_pixel_format(int source_pixel_format);
    image& with_source_properties(int source_properties);
    image& with_source_compression_type(int source_compression_type);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
