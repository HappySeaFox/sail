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
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_palette;

namespace sail
{

/*
 * ICC profile representation. It provides access to raw ICC profile data.
 */
class SAIL_EXPORT palette
{
    friend class image;

public:
    palette();
    palette(const palette &pal);
    palette& operator=(const palette &pal);
    palette(palette &&pal);
    palette& operator=(palette &&pal);
    ~palette();

    /*
     * Returns true if the palette has non-NULL data and a positive color count.
     */
    bool is_valid() const;

    /*
     * Returns the palette pixel format.
     */
    SailPixelFormat pixel_format() const;

    /*
     * Returns the palette binary data.
     */
    const void* data() const;

    /*
     * Returns the number of colors in the palette.
     */
    unsigned color_count() const;

    /*
     * Sets a new palette data, pixel format, and colors count.
     */
    palette& with_data(SailPixelFormat pixel_format, const void *data, unsigned color_count);

private:
    /*
     * Makes a deep copy of the specified palette.
     */
    palette(const sail_palette *pal);

    sail_status_t to_sail_palette(sail_palette *pal) const;

    sail_status_t copy(SailPixelFormat pixel_format, const void *data, unsigned color_count);

private:
    class pimpl;
    pimpl *d;
};

}

#endif
