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

#ifndef SAIL_ICCP_CPP_H
#define SAIL_ICCP_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_iccp;

namespace sail
{

/*
 * ICC profile representation. It provides access to raw ICC profile data.
 */
class SAIL_EXPORT iccp
{
    friend class image;

public:
    iccp();
    iccp(const iccp &ic);
    iccp& operator=(const iccp &ic);
    ~iccp();

    /*
     * Returns true if the ICC profile has non-NULL data. It doesn't validate the data.
     */
    bool is_valid() const;

    /*
     * Returns the ICC profile binary data.
     */
    void* data() const;

    /*
     * Returns the length of the ICC binary data.
     */
    unsigned data_length() const;

    /*
     * Sets a new ICC profile binary data.
     */
    iccp& with_data(const void *data, unsigned data_length);

private:
    /*
     * Makes a deep copy of the specified ICC profile.
     */
    iccp(const sail_iccp *ic);

    sail_error_t to_sail_iccp(sail_iccp *ic) const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
