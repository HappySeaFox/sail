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

#ifndef SAIL_IMAGE_INPUT_CPP_H
#define SAIL_IMAGE_INPUT_CPP_H

#include <cstddef>
#include <memory>
#include <string>
#include <tuple>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "arbitrary_data-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/arbitrary_data-c++.h>
#endif

namespace sail
{

class abstract_io;
class image;
class codec_info;
class load_options;

/*
 * Class to probe and load images form files, memory, and custom I/O sources.
 */
class SAIL_EXPORT image_input
{
public:
    /*
     * Constructs a new image reader.
     */
    image_input();

    /*
     * Stops loading if it was started and destroys the image reader.
     */
    ~image_input();

    /*
     * Starts loading the specified image file.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const std::string &path);

    /*
     * Starts loading the specified image file with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const std::string &path, const sail::codec_info &codec_info);

    /*
     * Starts loading the specified image file with the specified load options.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const std::string &path, const sail::load_options &load_options);

    /*
     * Starts loading the specified image file with the specified codec and load options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const std::string &path, const sail::codec_info &codec_info, const sail::load_options &load_options);

    /*
     * Starts loading the specified memory buffer.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const void *buffer, std::size_t buffer_length);

    /*
     * Starts loading the specified memory buffer with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info);

    /*
     * Starts loading the specified memory buffer with the specified load options.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const void *buffer, std::size_t buffer_length, const sail::load_options &load_options);

    /*
     * Starts loading the specified memory buffer with the specified codec and load options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const void *buffer, std::size_t buffer_length, const sail::codec_info &codec_info, const sail::load_options &load_options);

    /*
     * Starts loading the specified memory buffer.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const sail::arbitrary_data &arbitrary_data);

    /*
     * Starts loading the specified memory buffer with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const sail::arbitrary_data &arbitrary_data, const sail::codec_info &codec_info);

    /*
     * Starts loading the specified memory buffer with the specified load options.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const sail::arbitrary_data &arbitrary_data, const sail::load_options &load_options);

    /*
     * Starts loading the specified memory buffer with the specified codec and load options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(const sail::arbitrary_data &arbitrary_data, const sail::codec_info &codec_info, const sail::load_options &load_options);

    /*
     * Starts loading the specified I/O source.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(sail::abstract_io &abstract_io);

    /*
     * Starts loading the specified I/O source with the specified codec.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info);

    /*
     * Starts loading the specified I/O source with the specified load options.
     *
     * Typical usage: start()          ->
     *                next_frame() x n ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(sail::abstract_io &abstract_io, const sail::load_options &load_options);

    /*
     * Starts loading the specified I/O source with the specified codec and load options.
     *
     * Typical usage: codec_info::from_extension() ->
     *                start()                      ->
     *                next_frame() x n             ->
     *                stop().
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t start(sail::abstract_io &abstract_io, const sail::codec_info &codec_info, const sail::load_options &load_options);

    /*
     * Continues loading the source started by the previous call to start().
     * Assigns the loaded image to the 'image' argument.
     *
     * Returns SAIL_OK on success.
     * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
     */
    sail_status_t next_frame(sail::image *image);

    /*
     * Continues loading the source started by the previous call to start().
     *
     * Returns an invalid image on error.
     */
    image next_frame();

    /*
     * Stops loading the source started by the previous call to start(). Does nothing
     * if no loading was started.
     *
     * It is essential to always stop loading to free memory and I/O resources. Failure to do so
     * will lead to memory leaks.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t stop();

    /*
     * Loads the specified image file and returns its properties without pixels and the corresponding
     * codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    static std::tuple<image, codec_info> probe(const std::string &path);

    /*
     * Loads an image from the specified memory buffer and returns its properties without pixels
     * and the corresponding codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    static std::tuple<image, codec_info> probe(const void *buffer, std::size_t buffer_length);

    /*
     * Loads an image from the specified memory buffer and returns its properties without pixels
     * and the corresponding codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    static std::tuple<image, codec_info> probe(const sail::arbitrary_data &arbitrary_data);

    /*
     * Loads an image from the specified I/O source and returns its properties without pixels
     * and the corresponding codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    static std::tuple<image, codec_info> probe(sail::abstract_io &abstract_io);

    /*
     * Loads the specified image file.
     *
     * Returns an invalid image on error.
     */
    static image load(const std::string &path);

    /*
     * Loads an image from the specified memory buffer.
     *
     * Returns an invalid image on error.
     */
    static image load(const void *buffer, std::size_t buffer_length);

    /*
     * Loads an image from the specified memory buffer.
     *
     * Returns an invalid image on error.
     */
    static image load(const sail::arbitrary_data &arbitrary_data);

private:
    class pimpl;
    const std::unique_ptr<pimpl> d;
};

}

#endif
