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

#include "sail-c++.h"

namespace sail
{

tuning utils_private::c_tuning_to_cpp_tuning(const sail_hash_map *c_tuning) {

    if (c_tuning == nullptr) {
        return tuning{};
    }

    tuning tuning;

    sail_traverse_hash_map_with_user_data(c_tuning, sail_key_value_into_tuning, &tuning);

    return tuning;
}

sail_status_t utils_private::cpp_tuning_to_sail_tuning(const tuning &cpp_tuning, sail_hash_map *c_tuning) {

    sail_clear_hash_map(c_tuning);

    for (const auto &kv : cpp_tuning) {
        struct sail_variant *sail_variant;

        SAIL_TRY(kv.second.to_sail_variant(&sail_variant));

        sail_put_hash_map(c_tuning, kv.first.c_str(), sail_variant);

        sail_destroy_variant(sail_variant);
    }

    return SAIL_OK;
}

bool utils_private::sail_key_value_into_tuning(const char *key, const sail_variant *value, void *user_data) {

    tuning *cpp_tuning = reinterpret_cast<tuning *>(user_data);

    cpp_tuning->emplace(key, variant(value));

    return true;
}

}
