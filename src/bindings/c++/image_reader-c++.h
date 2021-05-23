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

#ifndef SAIL_IMAGE_READER_CPP_H
#define SAIL_IMAGE_READER_CPP_H

#include <cstddef>
#include <string_view>
#include <tuple>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

namespace sail
{

class image;
class io;
class codec_info;
class read_options;

/*
 * Class to probe and read images.
 */
class SAIL_EXPORT image_reader
{
public:
    /*
     * Constructs a new image reader.
     */
    image_reader();

    /*
     * Destroys the image reader.
     */
    ~image_reader();

    /*
     * Disables copying image readers.
     */
    image_reader(const image_reader&) = delete;

    /*
     * Disables copying image readers.
     */
    image_reader& operator=(const image_reader&) = delete;

    /*
     * Disables moving image readers.
     */
    image_reader(image_reader&&) = delete;

    /*
     * Disables moving image readers.
     */
    image_reader& operator=(image_reader&&) = delete;

    /*
     * Loads the specified image file and returns its properties without pixels and the corresponding
     * codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    std::tuple<image, codec_info> probe(std::string_view path) const;

    /*
     * Loads an image from the specified memory buffer and returns its properties without pixels
     * and the corresponding codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    std::tuple<image, codec_info> probe(const void *buffer, size_t buffer_length) const;

    /*
     * Loads an image from the specified I/O source and returns its properties without pixels
     * and the corresponding codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    std::tuple<image, codec_info> probe(const sail::io &io) const;

    /*
     * Loads the specified image file.
     *
     * Returns an invalid image on error.
     */
    image read(std::string_view path) const;

    /*
     * Loads an image from the specified memory buffer.
     *
     * Returns an invalid image on error.
     */
    image read(const void *buffer, size_t buffer_length) const;

    /*
     * Starts reading the specified image file.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(std::string_view path);

    /*
     * Starts reading the specified image file with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(std::string_view path, const sail::codec_info &codec_info);

    /*
     * Starts reading the specified image file with the specified read options.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(std::string_view path, const sail::read_options &read_options);

    /*
     * Starts reading the specified image file with the specified codec and read options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(std::string_view path, const sail::codec_info &codec_info, const sail::read_options &read_options);

    /*
     * Starts reading the specified memory buffer.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const void *buffer, size_t buffer_length);

    /*
     * Starts reading the specified memory buffer with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const void *buffer, size_t buffer_length, const sail::codec_info &codec_info);

    /*
     * Starts reading the specified memory buffer with the specified read options.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const void *buffer, size_t buffer_length, const sail::read_options &read_options);

    /*
     * Starts reading the specified memory buffer with the specified codec and read options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const void *buffer, size_t buffer_length, const sail::codec_info &codec_info, const sail::read_options &read_options);

    /*
     * Starts reading the specified I/O source.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const sail::io &io);

    /*
     * Starts reading the specified I/O source with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const sail::io &io, const sail::codec_info &codec_info);

    /*
     * Starts reading the specified I/O source with the specified read options.
     *
     * Typical usage: start_reading()   ->
     *                read_next_frame() ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const sail::io &io, const sail::read_options &read_options);

    /*
     * Starts reading the specified I/O source with the specified codec and read options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_reading()              ->
     *                read_next_frame()            ->
     *                stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_reading(const sail::io &io, const sail::codec_info &codec_info, const sail::read_options &read_options);

    /*
     * Continues reading the source started by the previous call to start_reading().
     * Assigns the read image to the 'image' argument.
     *
     * Returns SAIL_OK on success.
     * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
     */
    sail_status_t read_next_frame(sail::image *image);

    /*
     * Stops reading the source started by the previous call to start_reading(). Does nothing
     * if no reading was started.
     *
     * It is essential to always stop writing to free memory resources. Failure to do so
     * will lead to memory leaks.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t stop_reading();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
