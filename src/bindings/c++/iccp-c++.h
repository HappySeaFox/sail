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

#ifndef SAIL_ICCP_CPP_H
#define SAIL_ICCP_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "arbitrary_data-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/arbitrary_data-c++.h>
#endif

struct sail_iccp;

namespace sail
{

/*
 * ICC profile.
 */
class SAIL_EXPORT iccp
{
    friend class image;

public:
    /*
     * Constructs an invalid ICC profile.
     */
    iccp();

    /*
     * Constructs a new ICC profile from the binary data. The data is deep copied.
     */
    iccp(const void *data, unsigned data_length);

    /*
     * Constructs a new ICC profile from the binary data.
     */
    iccp(const arbitrary_data &data);

    /*
     * Copies the ICC profile.
     */
    iccp(const sail::iccp &ic);

    /*
     * Copies the ICC profile.
     */
    iccp& operator=(const sail::iccp &iccp);

    /*
     * Moves the ICC profile.
     */
    iccp(sail::iccp &&iccp) noexcept;

    /*
     * Moves the ICC profile.
     */
    iccp& operator=(sail::iccp &&iccp) noexcept;

    /*
     * Destroys the ICC profile.
     */
    ~iccp();

    /*
     * Returns true if the ICC profile data is not empty. It doesn't validate the data.
     */
    bool is_valid() const;

    /*
     * Returns the ICC profile binary data.
     */
    const arbitrary_data& data() const;

    /*
     * Sets new ICC profile binary data. The data is deep copied.
     */
    iccp& with_data(const void *data, unsigned data_length);

    /*
     * Sets new ICC profile binary data.
     */
    iccp& with_data(const arbitrary_data &data);

private:
    /*
     * Makes a deep copy of the specified ICC profile.
     */
    explicit iccp(const sail_iccp *ic);

    sail_status_t to_sail_iccp(sail_iccp **iccp) const;

    sail_status_t copy(const void *data, unsigned data_length);

private:
    class pimpl;
    pimpl *d;
};

}

#endif
