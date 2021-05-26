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

#ifndef SAIL_IMAGE_WRITER_CPP_H
#define SAIL_IMAGE_WRITER_CPP_H

#include <cstddef>
#include <string_view>

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
class write_options;

/*
 * Class to write images into files, memory, and custom I/O targets.
 */
class SAIL_EXPORT image_writer
{
public:
    /*
     * Constructs a new image writer.
     */
    image_writer();

    /*
     * Stops writing if it was started and destroys the image writer.
     */
    ~image_writer();

    /*
     * Disables copying image writers.
     */
    image_writer(const image_writer&) = delete;

    /*
     * Disables copying image writers.
     */
    image_writer& operator=(const image_writer&) = delete;

    /*
     * Disables moving image writers.
     */
    image_writer(image_writer&&) = delete;

    /*
     * Disables moving image writers.
     */
    image_writer& operator=(image_writer&&) = delete;

    /*
     * Writes the specified image into the file.
     *
     * If the selected image format doesn't support the image pixel format, an error is returned.
     * Consider converting the image into a supported image format beforehand.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t write(std::string_view path, const sail::image &image);

    /*
     * Writes the specified image into the specified memory buffer.
     *
     * If the selected image format doesn't support the image pixel format, an error is returned.
     * Consider converting the image into a supported image format beforehand.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t write(void *buffer, size_t buffer_length, const sail::image &image);

    /*
     * Writes the specified image into the specified memory buffer.
     *
     * If the selected image format doesn't support the image pixel format, an error is returned.
     * Consider converting the image into a supported image format beforehand.
     *
     * Saves the number of bytes written into the 'written' argument if it's not nullptr.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t write(void *buffer, size_t buffer_length, const sail::image &image, size_t *written);

    /*
     * Starts writing into the specified image file.
     *
     * Typical usage: start_writing()        ->
     *                write_next_frame() x n ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(std::string_view path);

    /*
     * Starts writing into the specified image file with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(std::string_view path, const sail::codec_info &codec_info);

    /*
     * Starts writing into the specified image file with the specified write options.
     *
     * Typical usage: start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(std::string_view path, const sail::write_options &write_options);

    /*
     * Starts writing into the specified image file with the specified codec and write options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(std::string_view path, const sail::codec_info &codec_info, const sail::write_options &write_options);

    /*
     * Starts writing into the specified memory buffer with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(void *buffer, size_t buffer_length, const sail::codec_info &codec_info);

    /*
     * Starts writing into the specified memory buffer with the specified codec and write options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(void *buffer, size_t buffer_length, const sail::codec_info &codec_info, const sail::write_options &write_options);

    /*
     * Starts writing into the specified I/O target with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(const sail::io &io, const sail::codec_info &codec_info);

    /*
     * Starts writing into the specified I/O target with the specified codec and write options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start_writing()              ->
     *                write_next_frame() x n       ->
     *                stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start_writing(const sail::io &io, const sail::codec_info &codec_info, const sail::write_options &write_options);

    /*
     * Continues writing started by start_writing(). Writes the specified image into the underlying I/O target.
     *
     * If the selected image format doesn't support the image pixel format, an error is returned.
     * Consider converting the image into a supported image format beforehand.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t write_next_frame(const sail::image &image);

    /*
     * Stops writing started by start_writing() and closes the underlying I/O target.
     *
     * It is essential to always stop writing to free memory resources. Failure to do so will lead to memory leaks.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t stop_writing();

    /*
     * Stops writing started by start_writing() and closes the underlying I/O target.
     * Assigns the number of bytes written to the 'written' argument.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t stop_writing(size_t *written);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
