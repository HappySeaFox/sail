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

#ifndef SAIL_SOURCE_IMAGE_CPP_H
#define SAIL_SOURCE_IMAGE_CPP_H

#include <memory>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "special_properties-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/special_properties-c++.h>
#endif

struct sail_source_image;

namespace sail
{

/*
 * source_image represents source image properties. The class is used in loading operations only
 * to preserve the source image properties. It's ignored in saving operations.
 */
class SAIL_EXPORT source_image
{
    friend class image;

public:
    /*
     * Constructs an empty source image with unknown pixel format and zero properties.
     */
    source_image();

    /*
     * Copies the source image.
     */
    source_image(const source_image &si);

    /*
     * Copies the source image.
     */
    source_image& operator=(const source_image &si);

    /*
     * Moves the source image.
     */
    source_image(source_image &&si) noexcept;

    /*
     * Moves the source image.
     */
    source_image& operator=(source_image &&si) noexcept;

    /*
     * Destroys the source image.
     */
    ~source_image();

    /*
     * Returns true if the source image is valid. The source image is always valid
     * except when a memory allocation failure occurs.
     */
    bool is_valid() const;

    /*
     * Returns the source image pixel format. See SailPixelFormat.
     *
     * LOAD: Set by SAIL to a source image pixel format of the original image.
     * SAVE: Ignored.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns the source image chroma subsampling. See SailChromaSubsampling.
     *
     * LOAD: Set by SAIL to a source image chroma subsampling of the original image.
     * SAVE: Ignored.
     */
    SailChromaSubsampling chroma_subsampling() const;

    /*
     * Returns the source image orientation.
     *
     * LOAD: Set by SAIL to the source image orientation.
     * SAVE: Ignored.
     */
    SailOrientation orientation() const;

    /*
     * Returns the source image compression type. See SailCompression.
     *
     * LOAD: Set by SAIL to a valid source image compression type.
     * SAVE: Ignored.
     */
    SailCompression compression() const;

    /*
     * Returns the source image interlacing.
     *
     * LOAD: Set by SAIL to true if the source image is interlaced.
     * SAVE: Ignored.
     */
    bool interlaced() const;

    /*
     * Returns image format-specific properties that cannot be expressed
     * in a common way. For example, a cursor hot spot.
     *
     * Special properties' names start with the codec name to avoid confusing.
     * For example, "cur-hotspot-x".
     *
     * LOAD: Set by SAIL to valid source image special properties.
     * SAVE: Ignored.
     */
    const sail::special_properties &special_properties() const;

private:
    /*
     * Makes a deep copy of the specified source image.
     */
    explicit source_image(const sail_source_image *si);

    sail_status_t to_sail_source_image(sail_source_image **source_image) const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
