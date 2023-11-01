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

#ifndef SAIL_IMAGE_OUTPUT_CPP_H
#define SAIL_IMAGE_OUTPUT_CPP_H

#include <cstddef> /* std::size_t */
#include <memory>
#include <string>

#include <sail-common/export.h>
#include <sail-common/status.h>

#include <sail-c++/arbitrary_data.h>

namespace sail
{

class abstract_io;
class codec_info;
class image;
class save_options;

/*
 * Saves images to files, memory, and custom I/O targets.
 */
class SAIL_EXPORT image_output
{
public:
    /*
     * Constructs a new image output to the specified image file.
     * Detects the image format based on the file extension.
     */
    explicit image_output(const std::string &path);

    /*
     * Constructs a new image output to the specified memory buffer.
     */
    image_output(void *buffer, std::size_t buffer_size, const sail::codec_info &codec_info);

    /*
     * Constructs a new image output to the specified memory buffer.
     */
    image_output(sail::arbitrary_data *arbitrary_data, const sail::codec_info &codec_info);

    /*
     * Constructs a new image output to the specified I/O source.
     */
    image_output(sail::abstract_io &abstract_io, const sail::codec_info &codec_info);

    /*
     * Finishes saving and destroys the image output.
     */
    ~image_output();

    /*
     * Moves the image output.
     */
    image_output(image_output &&other);

    /*
     * Moves the image output.
     */
    image_output& operator=(image_output &&other);

    /*
     * Overrides the automatically detected codec info used to save the image.
     */
    image_output& with(const sail::codec_info &codec_info);

    /*
     * Overrides the save options used to save the image.
     */
    image_output& with(const sail::save_options &save_options);

    /*
     * Continues saving into the I/O target.
     *
     * If the selected image format doesn't support the image pixel format, an error is returned.
     * Consider converting the image into a supported image format beforehand.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t next_frame(const sail::image &image);

    /*
     * Finishes saving and closes the I/O stream. Call to finish() is recommended
     * if you want to ensure the I/O stream is flushed and closed successfully.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t finish();

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
