/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#ifndef SAIL_ABSTRACT_IO_ADAPTER_CPP_H
#define SAIL_ABSTRACT_IO_ADAPTER_CPP_H

#include <memory>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_io;

namespace sail
{

class abstract_io;

/*
 * Adapter to make abstract I/O streams suitable for C functions.
 */
class SAIL_EXPORT abstract_io_adapter
{
public:
    /*
     * Constructs a new I/O wrapper with the specified abstract I/O stream to wrap.
     */
    explicit abstract_io_adapter(sail::abstract_io &abstract_io);

    /*
     * Destroys the I/O wrapper.
     */
    ~abstract_io_adapter();

    /*
     * Returns the I/O stream suitable for passing it to C functions.
     */
    struct sail_io& sail_io_c() const;

private:
    class pimpl;
    const std::unique_ptr<pimpl> d;
};

}

#endif
