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

#ifndef SAIL_PALETTE_CPP_H
#define SAIL_PALETTE_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "arbitrary_data-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/arbitrary_data-c++.h>
#endif

struct sail_palette;

namespace sail
{

/*
 * palette represents an image palette used in indexed images.
 */
class SAIL_EXPORT palette
{
    friend class image;

public:
    /*
     * Constructs an invalid palette.
     */
    palette();

    /*
     * Constructs palette from the data. The palette stays invalid if the pixel format
     * is SAIL_PIXEL_FORMAT_UNKNOWN, or the color count is 0, or the data is null.
     */
    palette(SailPixelFormat pixel_format, const void *data, unsigned color_count);

    /*
     * Constructs palette from the data. Calculates the color count from the data size
     * and the pixel format. The palette stays invalid if the pixel format
     * is SAIL_PIXEL_FORMAT_UNKNOWN or the data is empty.
     */
    palette(SailPixelFormat pixel_format, const arbitrary_data &data);

    /*
     * Copies the palette.
     */
    palette(const palette &pal);

    /*
     * Copies the palette.
     */
    palette& operator=(const sail::palette &palette);

    /*
     * Moves the palette.
     */
    palette(sail::palette &&palette) noexcept;

    /*
     * Moves the palette.
     */
    palette& operator=(sail::palette &&palette) noexcept;

    /*
     * Destroys the palette.
     */
    ~palette();

    /*
     * Returns true if the palette has non-empty data, known pixel format, and a positive color count.
     */
    bool is_valid() const;

    /*
     * Returns the palette pixel format.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns the palette binary data.
     */
    const arbitrary_data& data() const;

    /*
     * Returns the number of colors in the palette.
     */
    unsigned color_count() const;

    /*
     * Sets new palette data, pixel format, and colors count. Makes the palette invalid
     * if the pixel format is SAIL_PIXEL_FORMAT_UNKNOWN, or the color count is 0,
     * or the data is null.
     */
    palette& with_data(SailPixelFormat pixel_format, const void *data, unsigned color_count);

    /*
     * Sets new palette data, pixel format, and colors count. Calculates the color count
     * from the data size and the pixel format. Makes the palette invalid if the pixel
     * format is SAIL_PIXEL_FORMAT_UNKNOWN or the data is empty.
     */
    palette& with_data(SailPixelFormat pixel_format, const arbitrary_data &data);

private:
    /*
     * Makes a deep copy of the specified palette.
     */
    explicit palette(const sail_palette *pal);

    sail_status_t to_sail_palette(sail_palette **palette) const;

    sail_status_t copy(SailPixelFormat pixel_format, const void *data, unsigned color_count);

private:
    class pimpl;
    pimpl *d;
};

}

#endif
