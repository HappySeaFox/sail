/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2022 Dmitry Baryshev

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

#ifndef SAIL_VARIANT_CPP_H
#define SAIL_VARIANT_CPP_H

#include <compare>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <sail-common/export.h>
#include <sail-common/status.h>

#include <sail-c++/arbitrary_data.h>

struct sail_variant;

namespace sail
{

struct invalid_variant_type
{
    auto operator<=>(const invalid_variant_type &other) const = default;
};

using variant_type = std::variant<
                                     invalid_variant_type,
                                     bool,
                                     char,
                                     unsigned char,
                                     short,
                                     unsigned short,
                                     int,
                                     unsigned int,
                                     long,
                                     unsigned long,
                                     float,
                                     double,
                                     std::string,
                                     arbitrary_data
                                 >;

/*
 * Variant with limited possible data types.
 */
class SAIL_EXPORT variant : public variant_type
{
public:
    using variant_type::variant_type;

    /*
     * Returns true if the variant has some value stored.
     */
    inline bool is_valid() const
    {
        return index() != 0;
    }

    /*
     * Returns true if the value stored in the variant is of the requested type.
     */
    template<typename U>
    bool has_value() const
    {
        return std::holds_alternative<std::decay_t<U>>(*this);
    }

    /*
     * Returns the current value. If the requested type doesn't match the actual type stored
     * in the variant, throws std::bad_variant_access. Use has_value<T>() to check
     * the stored type.
     */
    template<typename U>
    const U& value() const
    {
        return std::get<std::decay_t<U>>(*this);
    }

    /*
     * Sets a new value.
     */
    template<typename U>
    void set_value(U&& value)
    {
        emplace<std::decay_t<U>>(std::forward<U>(value));
    }

    /*
     * Resets the variant to the invalid state.
     */
    inline void clear()
    {
        *this = {};
    }
};

SAIL_EXPORT sail::variant from_struct(const sail_variant *sail_variant);

SAIL_EXPORT sail_status_t to_struct(const sail::variant &variant, sail_variant **sail_variant);

}

#endif
