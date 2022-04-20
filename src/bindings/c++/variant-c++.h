/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include <memory>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "arbitrary_data-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/arbitrary_data-c++.h>
#endif

namespace sail
{

/*
 * Variant with limited possible data types. Supports only the following data types:
 *   - bool
 *   - char
 *   - unsigned char
 *   - short
 *   - unsigned short
 *   - int
 *   - unsigned int
 *   - long
 *   - unsigned long
 *   - float
 *   - double
 *   - std::string
 *   - sail::arbitrary_data
 */
class SAIL_EXPORT variant
{
    friend class meta_data;
    friend class utils_private;
    friend SAIL_EXPORT bool operator==(const sail::variant &a, const sail::variant &b);

public:
    /*
     * Constructs an invalid variant.
     */
    variant();

    /*
     * Constructs a new variant from the value.
     */
    template<typename T>
    variant(const T &value);

    /*
     * Copies the variant.
     */
    variant(const variant &var);

    /*
     * Copies the variant.
     */
    variant& operator=(const sail::variant &variant);

    /*
     * Moves the variant.
     */
    variant(sail::variant &&variant) noexcept;

    /*
     * Moves the variant.
     */
    variant& operator=(sail::variant &&variant) noexcept;

    /*
     * Destroys the variant.
     */
    ~variant();

    /*
     * Returns true if the variant has some value stored.
     */
    bool is_valid() const;

    /*
     * Returns true if the value stored in the variant is of the requested type.
     */
    template<typename T>
    bool has_value() const;

    /*
     * Returns the current value. The behavior is undefined if the requested type doesn't match
     * the actual type stored in the variant. Use has_value<T>() to check the stored data type.
     */
    template<typename T>
    const T& value() const;

    /*
     * Sets a new value.
     */
    template<typename T>
    void set_value(const T &value);

private:
    sail_status_t to_sail_variant(sail_variant **variant) const;

    class pimpl;
    std::unique_ptr<pimpl> d;
};

/*
 * Returns true if the variants have the same type and value.
 */
SAIL_EXPORT bool operator==(const sail::variant &a, const sail::variant &b);

/*
 * Returns true if the variants have different types or values.
 */
SAIL_EXPORT bool operator!=(const sail::variant &a, const sail::variant &b);

}

#endif
