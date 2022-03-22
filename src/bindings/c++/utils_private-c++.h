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

#ifndef SAIL_UTILS_PRIVATE_CPP_H
#define SAIL_UTILS_PRIVATE_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "tuning-c++.h"
#else
    INTERNAL ERROR: For internal use only
#endif

struct sail_hash_map;
struct sail_variant;

namespace sail
{

class SAIL_HIDDEN utils_private
{
public:
    static tuning c_tuning_to_cpp_tuning(const sail_hash_map *c_tuning);

    static sail_status_t cpp_tuning_to_sail_tuning(const tuning &cpp_tuning, sail_hash_map *c_tuning);

private:
    // Needs to be in utils_private to allow creating sail:variant from sail_variant
    static bool sail_key_value_into_tuning(const char *key, const sail_variant *value, void *user_data);
};

}

#endif
