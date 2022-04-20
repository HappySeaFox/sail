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

#include <memory>
#include <string>
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
class save_features;

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
    explicit image(const std::string &path);

    /*
     * Constructs a new image out of the specified image properties and the pixels.
     * Assumes the pixels have no padding bytes in the end of every scan line. The pixels
     * must remain valid as long as the image exists.
     */
    image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height);

    /*
     * Constructs a new image out of the specified image properties and the pixels.
     * The pixels must remain valid as long as the image exists.
     */
    image(void *pixels, SailPixelFormat pixel_format, unsigned width, unsigned height, unsigned bytes_per_line);

    /*
     * Makes a deep copy of the image.
     */
    image(const image &img);

    /*
     * Makes a deep copy of the image.
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
     * and the pixel data.
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
     * LOAD: Set by SAIL to a positive image width in pixels.
     * SAVE: Must be set by a caller to a positive image width in pixels.
     */
    unsigned width() const;

    /*
     * Returns the image height.
     *
     * LOAD: Set by SAIL to a positive image height in pixels.
     * SAVE: Must be set by a caller to a positive image height in pixels.
     */
    unsigned height() const;

    /*
     * Returns the bytes per line.
     *
     * LOAD: Set by SAIL to a positive length of a row of pixels in bytes.
     * SAVE: Must be set by a caller to a positive number of bytes per line. A caller could set
     *       it with bytes_per_line_auto() if scan lines are not padded to a certain boundary.
     */
    unsigned bytes_per_line() const;

    /*
     * Returns the image resolution.
     *
     * LOAD: Set by SAIL to a valid resolution if this information is available.
     * SAVE: Must be set by a caller to a valid image resolution if necessary.
     */
    const sail::resolution& resolution() const;

    /*
     * Returns the image pixel format. See SailPixelFormat.
     *
     * LOAD: Set by SAIL to a valid image pixel format.
     * SAVE: Must be set by a caller to a valid input image pixel format. Pixels in this format will be supplied
     *       to the codec by a caller later. The list of supported input pixel formats by a certain codec
     *       can be obtained from save_features.pixel_formats.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns the image gamma.
     *
     * LOAD: Set by SAIL to a valid gamma if it's available. 1 by default.
     * SAVE: Must be set by a caller to a valid gamma. Not all codecs support saving
     *       gamma.
     */
    double gamma() const;

    /*
     * Returns the delay in milliseconds to display the image on the screen if the image is a frame
     * in an animation or -1 otherwise.
     *
     * LOAD: Set by SAIL to a non-negative number of milliseconds if the image is a frame
     *       in an animation or to -1 otherwise.
     *       For animations, it's guaranteed that all the frames have non-negative delays.
     *       For multi-paged sequences, it's guaranteed that all the pages have delays equal to -1.
     * SAVE: Must be set by a caller to a non-negative number of milliseconds if the image is a frame
     *       in an animation.
     */
    int delay() const;

    /*
     * Returns the image palette if the image has it.
     *
     * LOAD: Set by SAIL to a valid palette if the image is indexed and the requested pixel format
     *       assumes having a palette.
     * SAVE: Must be set by a caller to a valid palette if the image is indexed.
     */
    const sail::palette& palette() const;

    /*
     * Returns the image meta data.
     *
     * LOAD: Set by SAIL to a valid map with meta data (like JPEG comments).
     * SAVE: Must be set by a caller to a valid map with meta data (like JPEG comments) if necessary.
     */
    const std::vector<sail::meta_data>& meta_data() const;

    /*
     * Returns the editable image meta data.
     *
     * LOAD: Set by SAIL to a valid map with meta data (like JPEG comments).
     * SAVE: Must be set by a caller to a valid map with meta data (like JPEG comments) if necessary.
     */
    std::vector<sail::meta_data>& meta_data();

    /*
     * Returns the embedded ICC profile.
     *
     * Note for animated/multi-paged images: only the first image in an animated/multi-paged
     * sequence might have an ICC profile.
     *
     * LOAD: Set by SAIL to a valid ICC profile if any.
     * SAVE: Must be set by a caller to a valid ICC profile if necessary.
     */
    const sail::iccp& iccp() const;

    /*
     * Returns the image orientation.
     *
     * LOAD: Set by SAIL to an image orientation. The user can use this field
     *       to manipulate the output image accordingly (e.g., rotate).
     * SAVE: Ignored.
     */
    SailOrientation orientation() const;

    /*
     * Returns the source image properties.
     *
     * LOAD: Set by SAIL to valid source image properties of the original image.
     * SAVE: Ignored.
     */
    const sail::source_image& source_image() const;

    /*
     * Returns the editable pixel data if any.
     *
     * LOAD: Set by SAIL to valid pixel data.
     * SAVE: Must be set by a caller to valid pixel data.
     */
    void* pixels();

    /*
     * Returns the constant pixel data if any.
     *
     * LOAD: Set by SAIL to valid pixel data.
     * SAVE: Must be set by a caller to valid pixel data.
     */
    const void* pixels() const;

    /*
     * Returns a pointer to the pixels scan line with index i.
     */
    void* scan_line(unsigned i);

    /*
     * Returns a pointer to the pixels scan line with index i.
     */
    const void* scan_line(unsigned i) const;

    /*
     * Returns the size of the pixel data in bytes.
     */
    unsigned pixels_size() const;

    /*
     * Sets a new resolution.
     */
    void set_resolution(const sail::resolution &resolution);

    /*
     * Sets a new resolution.
     */
    void set_resolution(sail::resolution &&resolution) noexcept;

    /*
     * Sets a new gamma.
     */
    void set_gamma(double gamma);

    /*
     * Sets a new delay for an animated frame in a sequence.
     */
    void set_delay(int delay);

    /*
     * Sets a new palette.
     */
    void set_palette(const sail::palette &palette);

    /*
     * Sets a new palette.
     */
    void set_palette(sail::palette &&palette) noexcept;

    /*
     * Sets new meta data.
     */
    void set_meta_data(const std::vector<sail::meta_data> &meta_data);

    /*
     * Sets new meta data.
     */
    void set_meta_data(std::vector<sail::meta_data> &&meta_data) noexcept;

    /*
     * Sets a new ICC profile.
     */
    void set_iccp(const sail::iccp &iccp);

    /*
     * Sets a new ICC profile.
     */
    void set_iccp(sail::iccp &&iccp) noexcept;

    /*
     * Replaces the image with the image from the specified file path. Reads just a single frame
     * from the file.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t load(const std::string &path);

    /*
     * Saves the image with into the specified file path.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t save(const std::string &path);

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
    sail_status_t convert(const sail::save_features &save_features);

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
    sail_status_t convert(const sail::save_features &save_features, const conversion_options &options);

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
    sail_status_t convert_to(const sail::save_features &save_features, sail::image *image) const;

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
    sail_status_t convert_to(const sail::save_features &save_features, const conversion_options &options, sail::image *image) const;

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
    image convert_to(const sail::save_features &save_features) const;

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
    image convert_to(const sail::save_features &save_features, const conversion_options &options) const;

    /*
     * Returns the closest pixel format from the list.
     *
     * This method can be used to find the best pixel format to save the image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    SailPixelFormat closest_pixel_format(const std::vector<SailPixelFormat> &pixel_formats) const;

    /*
     * Returns the closest pixel format from the save features.
     *
     * This method can be used to find the best pixel format to save the image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    SailPixelFormat closest_pixel_format(const sail::save_features &save_features) const;

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
     * Returns the closest pixel format to the input pixel format from the save features.
     *
     * This method can be used to find the best pixel format to save an image into.
     *
     * Returns SAIL_PIXEL_FORMAT_UNKNOWN if no candidates found at all.
     */
    static SailPixelFormat closest_pixel_format(SailPixelFormat input_pixel_format, const sail::save_features &save_features);

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
    static SailPixelFormat pixel_format_from_string(const std::string &str);

    /*
     * Returns a string representation of the specified chroma subsampling.
     * For example: "311" is returned for SAIL_CHROMA_SUBSAMPLING_311.
     *
     * Returns NULL if the chroma subsampling is not known.
     */
    static const char* chroma_subsampling_to_string(SailChromaSubsampling chroma_subsampling);

    /*
     * Returns a chroma subsampling from the string representation.
     * For example: SAIL_CHROMA_SUBSAMPLING_311 is returned for "311".
     *
     * Returns SAIL_CHROMA_SUBSAMPLING_UNKNOWN if the chroma subsampling is not known.
     */
    static SailChromaSubsampling chroma_subsampling_from_string(const std::string &str);

    /*
     * Returns a string representation of the specified orientation. See SailOrientation.
     * For example: "NORMAL" is returned for SAIL_ORIENTATION_NORMAL.
     *
     * Returns NULL if the property is not known.
     */
    static const char* orientation_to_string(SailOrientation orientation);

    /*
     * Returns orientation from the string representation. See SailOrientation.
     * For example: SAIL_ORIENTATION_NORMAL is returned for "NORMAL".
     *
     * Returns SAIL_ORIENTATION_NORMAL if the orientation is not known.
     */
    static SailOrientation orientation_from_string(const std::string &str);

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
    static SailCompression compression_from_string(const std::string &str);

private:
    /*
     * Makes a deep copy of the specified image. The pixels are transferred. The caller must set the pixels
     * in the sail_image object to NULL afterwards to avoid destructing them in sail_destroy_image().
     */
    image(const sail_image *sail_image);

    sail_status_t transfer_pixels_pointer(const sail_image *sail_image);

    sail_status_t to_sail_image(sail_image **image) const;

    void set_dimensions(unsigned width, unsigned height);
    void set_pixel_format(SailPixelFormat pixel_format);
    void set_bytes_per_line(unsigned bytes_per_line);
    void set_bytes_per_line_auto();
    void set_pixels(const void *pixels);
    void set_pixels(const void *pixels, unsigned pixels_size);
    void set_shallow_pixels(void *pixels);
    void set_shallow_pixels(void *pixels, unsigned pixels_size);
    void set_orientation(SailOrientation orientation);
    void set_source_image(const sail::source_image &source_image);

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
