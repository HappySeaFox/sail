/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <cstddef> /* std::size_t */
#include <memory>
#include <string>
#include <tuple>

#include <sail-common/export.h>
#include <sail-common/status.h>

#include <sail-c++/arbitrary_data.h>
#include <sail-c++/image.h>

namespace sail
{

class abstract_io;
class codec_info;
class load_options;

/*
 * Probes and loads images from files, memory, and custom I/O sources.
 */
class SAIL_EXPORT image_input
{
public:
    /*
     * Constructs a new image input from the specified image file.
     */
    explicit image_input(const std::string &path);

    /*
     * Constructs a new image input from the specified memory buffer.
     */
    image_input(const void *buffer, std::size_t buffer_size);

    /*
     * Constructs a new image input from the specified memory buffer.
     */
    explicit image_input(const sail::arbitrary_data &arbitrary_data);

    /*
     * Constructs a new image input from the specified I/O source.
     */
    explicit image_input(sail::abstract_io &abstract_io);

    /*
     * Finishes loading and destroys the image input.
     */
    ~image_input();

    /*
     * Moves the image input.
     */
    image_input(image_input &&other);

    /*
     * Moves the image input.
     */
    image_input& operator=(image_input &&other);

    /*
     * Overrides the automatically detected codec info used to load the image.
     */
    image_input& with(const sail::codec_info &codec_info);

    /*
     * Overrides the load options used to load the image.
     */
    image_input& with(const sail::load_options &load_options);

    /*
     * Continues loading the image. Assigns the loaded image to the 'image' argument.
     *
     * Returns SAIL_OK on success.
     * Returns SAIL_ERROR_NO_MORE_FRAMES when no more frames are available.
     */
    sail_status_t next_frame(sail::image *image);
 
    /*
     * Continues loading the image.
     *
     * Returns an invalid image on error.
     */
    image next_frame();

    /*
     * Finishes loading and closes the I/O stream. Call to finish() is optional.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t finish();

    /*
     * Loads the image and returns its properties without pixels and the corresponding
     * codec info.
     *
     * This method is pretty fast because it doesn't decode whole image data for most image formats.
     *
     * Returns an invalid image on error.
     */
    std::tuple<image, codec_info> probe();

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
