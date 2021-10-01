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

#ifndef SAIL_READ_OPTIONS_CPP_H
#define SAIL_READ_OPTIONS_CPP_H

#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_read_options;

namespace sail
{

/*
 * read_options represents options to modify reading operations. See image_input.
 */
class SAIL_EXPORT read_options
{
    friend class image_input;
    friend class read_features;

public:
    /*
     * Constructs empty read options.
     */
    read_options();

    /*
     * Copies the read options.
     */
    read_options(const read_options &ro);

    /*
     * Copies the read options.
     */
    read_options& operator=(const sail::read_options &read_options);

    /*
     * Moves the read options.
     */
    read_options(sail::read_options &&read_options) noexcept;

    /*
     * Moves the read options.
     */
    read_options& operator=(sail::read_options &&read_options) noexcept;

    /*
     * Destroys the read options.
     */
    ~read_options();

    /*
     * Returns the or-ed I/O manipulation options for reading operations. See SailIoOption.
     */
    int io_options() const;

    /*
     * Sets new or-ed I/O manipulation options for reading operations. See SailIoOption.
     */
    read_options& with_io_options(int io_options);

private:
    /*
     * Makes a deep copy of the specified read options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit read_options(const sail_read_options *ro);

    sail_status_t to_sail_read_options(sail_read_options *read_options) const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
