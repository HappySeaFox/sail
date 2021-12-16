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

#ifndef SAIL_IO_PRIVATE_CPP_H
#define SAIL_IO_PRIVATE_CPP_H

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN io::pimpl
{
public:
    pimpl()
    {
        empty_sail_io();
    }

    inline void empty_sail_io();

    struct sail_io sail_io;
};

void io::pimpl::empty_sail_io()
{
    sail_io.id             = 0;
    sail_io.features       = 0;
    sail_io.stream         = nullptr;
    sail_io.tolerant_read  = nullptr;
    sail_io.strict_read    = nullptr;
    sail_io.seek           = nullptr;
    sail_io.tell           = nullptr;
    sail_io.tolerant_write = nullptr;
    sail_io.strict_write   = nullptr;
    sail_io.flush          = nullptr;
    sail_io.close          = nullptr;
    sail_io.eof            = nullptr;
}

}

#endif
