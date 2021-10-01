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

struct sail_image;

namespace sail
{

class conversion_options;
class meta_data;
class write_features;

/*
 * Image representation with direct access to the pixel data.
 */
class SAIL_EXPORT image
{
    friend class image_input;
    friend class image_output;

public:
    /*
     * Constructs an invalid image.
     */
    image();

    /*
     * Constructs a new image out of the specified file path. Reads just a single frame
     * from the file.
     */
    image(std::string_view path);

    /*
     * Constructs a new image out of the specified image properties and pixels.
     */
    image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height);

    /*
     * Constructs a new image out of the specified image properties and pixels.
     */
    image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height, unsigned bytes_per_line);

    /*
     * Copies the image.
     */
    image(const image &img);

    /*
     * Copies the image.
     */
    image& operator=(const sail::image &image);

    /*
     * Moves the image.
     */
    image(sail::image &&image) noexcept;

    /*
     * Moves the image.
     */
    image& operator=(sail::image &&image) noexcept;

    /*
     * Destroys the image and the deep copied pixel data.
     */
    ~image();

    /*
     * Returns true if the image has valid dimensions, pixel format, bytes per line,
     * and the pixel data (deep copied or shallow).
     */
    bool is_valid() const;

    /*
     * Returns true if the image pixel format is indexed with palette.
     */
    bool is_indexed() const;

    /*
     * Returns true if the image pixel format is grayscale.
     */
    bool is_grayscale() const;

    /*
     * Returns true if the image pixel format is RGB-like (RGBA, BGR, etc.).
     */
    bool is_rgb_family() const;

    /*
     * Returns the image width.
     *
     * READ:  Set by SAIL to a positive image width in pixels.
     * WRITE: Must be set by a caller to a positive image width in pixels.
     */
    unsigned width() const;

    /*
     * Returns the image height.
     *
     * READ:  Set by SAIL to a positive image height in pixels.
     * WRITE: Must be set by a caller to a positive image height in pixels.
     */
    unsigned height() const;

    /*
     * Returns the bytes per line.
     *
     * READ:  Set by SAIL to a positive length of a row of pixels in bytes.
     * WRITE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *        it with bytes_per_line_auto() if scan lines are not padded to a certain boundary.
     */
    unsigned bytes_per_line() const;

    /*
     * Returns the image resolution.
     *
     * READ:  Set by SAIL to a valid resolution if this information is available.
     * WRITE: Must be set by a caller to a valid image resolution if necessary.
     */
    const sail::resolution& resolution() const;

    /*
     * Returns the image pixel format. See SailPixelFormat.
     *
     * READ:  Set by SAIL to a valid output image pixel format. The list of supported output pixel formats
     *        by a certain codec can be obtained from read_features.input_pixel_formats.
     * WRITE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *        to the codec by a caller later. The list of supported input pixel formats by a certain codec
     *        can be obtained from write_features.output_pixel_formats.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns the image gamma.
     *
     * READ:  Set by SAIL to a valid gamma if it's available. 1 by default.
     * WRITE: Must be set by a caller to a valid gamma. Not all codecs support saving
     *        gamma.
     */
    double gamma() const;

    /*
     * Returns the delay in milliseconds to display the image on the screen if the image is a frame
     * in an animation or -1 otherwise.
     *
     * READ:  Set by SAIL to a non-negative number of milliseconds if the image is a frame
     *        in an animation or to -1 otherwise.
     *        For animations, it's guaranteed that all the frames have non-negative delays.
     *        For multi-paged sequences, it's guaranteed that all the pages have delays equal to -1.
     * WRITE: Must be set by a caller to a non-negative number of milliseconds if the image is a frame
     *        in an animation.
     */
    int delay() const;

    /*
     * Returns the image palette if the image has it.
     *
     * READ:  Set by SAIL to a valid palette if the image is indexed and the requested pixel format
     *        assumes having a palette.
     * WRITE: Must be set by a caller to a valid palette if the image is indexed.
     */
    const sail::palette& palette() const;

    /*
     * Returns the image meta data.
     *
     * READ:  Set by SAIL to a valid map with meta data (like JPEG comments).
     * WRITE: Must be set by a caller to a valid map with meta data (like JPEG comments) if necessary.
     */
    const std::vector<sail::meta_data>& meta_data() const;

    /*
     * Returns the embedded ICC profile.
     *
     * Note for animated/multi-paged images: only the first image in an animated/multi-paged
     * sequence might have an ICC profile.
     *
     * READ:  Set by SAIL to a valid ICC profile if any.
     * WRITE: Must be set by a caller to a valid ICC profile if necessary.
     */
    const sail::iccp& iccp() const;

    /*
     * Returns the or-ed image properties. See SailImageProperty.
     *
     * READ:  Set by SAIL to valid image properties. For example, some image formats store images flipped.
     *        A caller must use this field to manipulate the output image accordingly (e.g., flip back etc.).
     * WRITE: Ignored.
     */
    int properties() const;

    /*
     * Returns the source image properties.
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
     * Returns the size of the deep copied pixel data in bytes.
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
    image& with_resolution(const sail::resolution &resolution);

    /*
     * Sets a new pixel format. See SailPixelFormat.
     */
    image& with_pixel_format(SailPixelFormat pixel_format);

    /*
     * Sets a new gamma.
     */
    image& with_gamma(double gamma);

    /*
     * Sets a new delay for an animated frame in a sequence.
     */
    image& with_delay(int delay);

    /*
     * Sets a new palette.
     */
    image& with_palette(const sail::palette &palette);

    /*
     * Sets new meta data.
     */
    image& with_meta_data(const std::vector<sail::meta_data> &meta_data);

    /*
     * Appends the meta data entry to the image meta data.
     */
    image& with_meta_data(const sail::meta_data &meta_data);

    /*
     * Deep copies the specified pixel data. The data can be accessed later with pixels().
     * The size of the pixel data is calculated based on the image height, bytes per line, and the pixel
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
     * The size of the pixel data is calculated based on the image height, bytes per line, and the pixel
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
    image& with_iccp(const sail::iccp &iccp);

    /*
     * Replaces the image with the image from the specified file path. Reads just a single frame
     * from the file.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t load(std::string_view path);

    /*
     * Saves the image with into the specified file path.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t save(std::string_view path);

    /*
     * Returns true if the image can be converted into the specified pixel format.
     */
    bool can_convert(SailPixelFormat pixel_format);

    /*
     * Converts the image to the specified pixel format. Use can_convert() to quickly check if the conversion
     * can actually be done.
     *
     * Updates the image pixel format and bytes per line.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert(SailPixelFormat pixel_format);

    /*
     * Converts the image to the specified pixel format using the specified conversion options.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Updates the image pixel format and bytes per line.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert(SailPixelFormat pixel_format, const conversion_options &options);

    /*
     * Converts the image to the best pixel format for saving. Use can_convert()
     * to quickly check if the conversion can actually be done.
     *
     * Updates the image pixel format and bytes per line.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert(const sail::write_features &write_features);

    /*
     * Converts the image to the best pixel format for saving using the specified conversion options.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Updates the image pixel format and bytes per line.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert(const sail::write_features &write_features, const conversion_options &options);

    /*
     * Converts the image to the specified pixel format and assigns the resulting image to the 'image' argument.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert_to(SailPixelFormat pixel_format, sail::image *image) const;

    /*
     * Converts the image to the specified pixel format using the specified conversion options
     * and assigns the resulting image to the 'image' argument.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert_to(SailPixelFormat pixel_format, const conversion_options &options, sail::image *image) const;

    /*
     * Converts the image to the best pixel format for saving and assigns the resulting image to the 'image' argument.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert_to(const sail::write_features &write_features, sail::image *image) const;

    /*
     * Converts the image to the best pixel format for saving using the specified conversion options
     * and assigns the resulting image to the 'image' argument.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t convert_to(const sail::write_features &write_features, const conversion_options &options, sail::image *image) const;

    /*
     * Converts the image to the specified pixel format and returns the resulting image.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns an invalid image on error.
     */
    image convert_to(SailPixelFormat pixel_format) const;

    /*
     * Converts the image to the specified pixel format using the specified conversion options
     * and returns the resulting image.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns an invalid image on error.
     */
    image convert_to(SailPixelFormat pixel_format, const conversion_options &options) const;

    /*
     * Converts the image to the best pixel format for saving and returns the resulting image.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * Drops the input alpha channel if the output alpha channel doesn't exist. For example,
     * when converting RGBA pixels to RGB. If you need to control this behavior,
     * use the overloaded method with conversion_options.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns an invalid image on error.
     */
    image convert_to(const sail::write_features &write_features) const;

    /*
     * Converts the image to the best pixel format for saving using the specified conversion options
     * and returns the resulting image.
     * Use can_convert() to quickly check if the conversion can actually be done.
     *
     * The conversion procedure may be slow. It converts every pixel into the BPP32-RGBA or
     * BPP64-RGBA formats first, and only then to the requested output format. No platform-specific
     * instructions (like AVX or SSE) are used.
     *
     * The image ICC profile is not involved in the conversion procedure.
     *
     * Returns an invalid image on error.
     */
    image convert_to(const sail::write_features &write_features, const conversion_options &options) const;

    /*
     * Returns the closest pixel format from the list.
     *
     * This method can be used to find the best pixel format to save the image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    SailPixelFormat closest_pixel_format(const std::vector<SailPixelFormat> &pixel_formats) const;

    /*
     * Returns the closest pixel format from the write features.
     *
     * This method can be used to find the best pixel format to save the image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    SailPixelFormat closest_pixel_format(const sail::write_features &write_features) const;

    /*
     * Returns true if the conversion or updating functions can convert or update from the input
     * pixel format to the output pixel format.
     */
    static bool can_convert(SailPixelFormat input_pixel_format, SailPixelFormat output_pixel_format);

    /*
     * Returns the closest pixel format to the input pixel format from the list.
     *
     * This method can be used to find the best pixel format to save an image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    static SailPixelFormat closest_pixel_format(SailPixelFormat input_pixel_format, const std::vector<SailPixelFormat> &pixel_formats);

    /*
     * Returns the closest pixel format to the input pixel format from the write features.
     *
     * This method can be used to find the best pixel format to save an image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    static SailPixelFormat closest_pixel_format(SailPixelFormat input_pixel_format, const sail::write_features &write_features);

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

    /*
     * Returns true if the specified pixel format is indexed with palette.
     */
    static bool is_indexed(SailPixelFormat pixel_format);

    /*
     * Returns true if the specified pixel format is grayscale.
     */
    static bool is_grayscale(SailPixelFormat pixel_format);

    /*
     * Returns true if the specified pixel format is RGB-like (RGBA, BGR, etc.).
     */
    static bool is_rgb_family(SailPixelFormat pixel_format);

    /*
     * Returns a string representation of the specified pixel format.
     * For example: "BPP32-RGBA" is returned for SAIL_PIXEL_FORMAT_BPP32_RGBA.
     *
     * Returns NULL if the pixel format is not known.
     */
    static const char* pixel_format_to_string(SailPixelFormat pixel_format);

    /*
     * Returns a pixel format from the string representation.
     * For example: SAIL_PIXEL_FORMAT_BPP32_RGBA is returned for "BPP32-RGBA".
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if the pixel format is not known.
     */
    static SailPixelFormat pixel_format_from_string(std::string_view str);

    /*
     * Returns a string representation of the specified image property. See SailImageProperty.
     * For example: "FLIPPED-VERTICALLY" is returned for SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY.
     *
     * Returns NULL if the property is not known.
     */
    static const char* image_property_to_string(SailImageProperty image_property);

    /*
     * Returns an image property from the string representation. See SailImageProperty.
     * For example: SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY is returned for "FLIPPED-VERTICALLY".
     *
     * Returns SAIL_IMAGE_PROPERTY_UNKNOWN if the property is not known.
     */
    static SailImageProperty image_property_from_string(std::string_view str);

    /*
     * Returns string representation of the specified compression type. See SailCompression.
     * For example: "RLE" is returned for SAIL_COMPRESSION_RLE.
     *
     * Returns NULL if the compression is not known.
     */
    static const char* compression_to_string(SailCompression compression);

    /*
     * Returns a compression from the string representation. See SailCompression.
     * For example: SAIL_COMPRESSION_RLE is returned for "RLE".
     *
     * Returns SAIL_COMPRESSION_UNKNOWN if the compression is not known.
     */
    static SailCompression compression_from_string(std::string_view str);

private:
    /*
     * Makes a deep copy of the specified image. The pixels are transferred. The caller must set the pixels
     * in the sail_image object to NULL afterwards to avoid destructing them in sail_destroy_image().
     */
    image(const sail_image *sail_image);

    sail_status_t transfer_pixels_pointer(const sail_image *sail_image);

    sail_status_t to_sail_image(sail_image **image) const;

    image& with_properties(int properties);
    image& with_source_image(const sail::source_image &source_image);

private:
    class pimpl;
    pimpl *d;
};

}

#endif
