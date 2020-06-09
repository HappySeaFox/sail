/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
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

    sail_error_t to_sail_palette(sail_palette *pal) const;

    sail_error_t copy(SailPixelFormat pixel_format, const void *data, unsigned color_count);

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
