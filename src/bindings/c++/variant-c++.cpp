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

#include <cstring>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "sail-c++.h"

namespace sail
{

namespace
{

const std::unordered_map<std::type_index, SailVariantType> cpp_to_sail_variant_type_mapping {
    { std::type_index(typeid(bool)),                 SAIL_VARIANT_TYPE_BOOL           },
    { std::type_index(typeid(char)),                 SAIL_VARIANT_TYPE_CHAR           },
    { std::type_index(typeid(unsigned char)),        SAIL_VARIANT_TYPE_UNSIGNED_CHAR  },
    { std::type_index(typeid(short)),                SAIL_VARIANT_TYPE_SHORT          },
    { std::type_index(typeid(unsigned short)),       SAIL_VARIANT_TYPE_UNSIGNED_SHORT },
    { std::type_index(typeid(int)),                  SAIL_VARIANT_TYPE_INT            },
    { std::type_index(typeid(unsigned int)),         SAIL_VARIANT_TYPE_UNSIGNED_INT   },
    { std::type_index(typeid(long)),                 SAIL_VARIANT_TYPE_LONG           },
    { std::type_index(typeid(unsigned long)),        SAIL_VARIANT_TYPE_UNSIGNED_LONG  },
    { std::type_index(typeid(float)),                SAIL_VARIANT_TYPE_FLOAT          },
    { std::type_index(typeid(double)),               SAIL_VARIANT_TYPE_DOUBLE         },
    { std::type_index(typeid(std::string)),          SAIL_VARIANT_TYPE_STRING         },
    { std::type_index(typeid(sail::arbitrary_data)), SAIL_VARIANT_TYPE_DATA           },
};

}

class SAIL_HIDDEN variant::pimpl
{
public:
    pimpl()
        : type(SAIL_VARIANT_TYPE_INVALID)
    {
    }
    ~pimpl()
    {
        destroy_value();
    }

    void destroy_value() {

        switch (type) {
            case SAIL_VARIANT_TYPE_STRING: v_string.~basic_string();   break;
            case SAIL_VARIANT_TYPE_DATA:   v_arbitrary_data.~vector(); break;

            default: {
                break;
            }
        }
    }

    template<typename T>
    SailVariantType type_to_sail_variant_type() {

        auto it = cpp_to_sail_variant_type_mapping.find(std::type_index(typeid(T)));

        return it == cpp_to_sail_variant_type_mapping.end() ? SAIL_VARIANT_TYPE_INVALID : it->second;
    }

    union {
        bool           v_bool;
        char           v_char;
        unsigned char  v_unsigned_char;
        short          v_short;
        unsigned short v_unsigned_short;
        int            v_int;
        unsigned int   v_unsigned_int;
        long           v_long;
        unsigned long  v_unsigned_long;
        float          v_float;
        double         v_double;
        std::string    v_string;
        arbitrary_data v_arbitrary_data;
    };

    SailVariantType type;
};

variant::variant()
    : d(new pimpl)
{
}

// Allow only specific types. Other types will fail to link.
//
template<>
SAIL_EXPORT void variant::set_value<>(const bool &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_BOOL;
    d->v_bool = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const char &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_CHAR;
    d->v_char = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const unsigned char &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_UNSIGNED_CHAR;
    d->v_unsigned_char = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const short &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_SHORT;
    d->v_short = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const unsigned short &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_UNSIGNED_SHORT;
    d->v_unsigned_short = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const int &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_INT;
    d->v_int = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const unsigned int &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_UNSIGNED_INT;
    d->v_unsigned_int = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const long &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_LONG;
    d->v_long = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const unsigned long &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_UNSIGNED_LONG;
    d->v_unsigned_long = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const float &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_FLOAT;
    d->v_float = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const double &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_DOUBLE;
    d->v_double = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const std::string &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_STRING;
    new (&d->v_string) std::string;
    d->v_string = value;
}

template<>
SAIL_EXPORT void variant::set_value<>(const sail::arbitrary_data &value)
{
    d->destroy_value();

    d->type = SAIL_VARIANT_TYPE_DATA;
    new (&d->v_arbitrary_data) arbitrary_data;
    d->v_arbitrary_data = value;
}

template<typename T>
variant::variant(const T &value)
    : variant()
{
    set_value(value);
}

// Allow only specific types. Other types will fail to link.
//
template SAIL_EXPORT variant::variant(const bool &);

template SAIL_EXPORT variant::variant(const char &);
template SAIL_EXPORT variant::variant(const unsigned char &);

template SAIL_EXPORT variant::variant(const short &);
template SAIL_EXPORT variant::variant(const unsigned short &);

template SAIL_EXPORT variant::variant(const int &);
template SAIL_EXPORT variant::variant(const unsigned int &);

template SAIL_EXPORT variant::variant(const long &);
template SAIL_EXPORT variant::variant(const unsigned long &);

template SAIL_EXPORT variant::variant(const float &);
template SAIL_EXPORT variant::variant(const double &);

template SAIL_EXPORT variant::variant(const std::string &);
template SAIL_EXPORT variant::variant(const sail::arbitrary_data &);

using sail_variant_type_workaround_alias_const     = const sail_variant *;
using sail_variant_type_workaround_alias_non_const = sail_variant *;

template<>
SAIL_EXPORT variant::variant(const sail_variant_type_workaround_alias_const &variant)
    : sail::variant()
{
    if (variant == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::variant(). The object is untouched");
        return;
    }

    switch (variant->type) {
        case SAIL_VARIANT_TYPE_BOOL:           set_value(sail_variant_to_bool(variant));                break;
        case SAIL_VARIANT_TYPE_CHAR:           set_value(sail_variant_to_char(variant));                break;
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  set_value(sail_variant_to_unsigned_char(variant));       break;
        case SAIL_VARIANT_TYPE_SHORT:          set_value(sail_variant_to_short(variant));               break;
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: set_value(sail_variant_to_unsigned_short(variant));      break;
        case SAIL_VARIANT_TYPE_INT:            set_value(sail_variant_to_int(variant));                 break;
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   set_value(sail_variant_to_unsigned_int(variant));        break;
        case SAIL_VARIANT_TYPE_LONG:           set_value(sail_variant_to_long(variant));                break;
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  set_value(sail_variant_to_unsigned_long(variant));       break;
        case SAIL_VARIANT_TYPE_FLOAT:          set_value(sail_variant_to_float(variant));               break;
        case SAIL_VARIANT_TYPE_DOUBLE:         set_value(sail_variant_to_double(variant));              break;
        case SAIL_VARIANT_TYPE_STRING:         set_value(std::string(sail_variant_to_string(variant))); break;
        case SAIL_VARIANT_TYPE_DATA: {
            const void *data = sail_variant_to_data(variant);
            sail::arbitrary_data arbitrary_data(variant->size);
            memcpy(arbitrary_data.data(), data, variant->size);
            set_value(arbitrary_data);
            break;
        }
        case SAIL_VARIANT_TYPE_INVALID: break;
    }
}

template<>
SAIL_EXPORT variant::variant(const sail_variant_type_workaround_alias_non_const &variant)
    : sail::variant(const_cast<sail_variant_type_workaround_alias_const>(variant))
{
}

variant::variant(const sail::variant &var)
    : variant()
{
    *this = var;
}

variant& variant::operator=(const sail::variant &variant)
{
    switch (variant.d->type) {
        case SAIL_VARIANT_TYPE_BOOL:           set_value(variant.d->v_bool);           break;
        case SAIL_VARIANT_TYPE_CHAR:           set_value(variant.d->v_char);           break;
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  set_value(variant.d->v_unsigned_char);  break;
        case SAIL_VARIANT_TYPE_SHORT:          set_value(variant.d->v_short);          break;
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: set_value(variant.d->v_unsigned_short); break;
        case SAIL_VARIANT_TYPE_INT:            set_value(variant.d->v_int);            break;
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   set_value(variant.d->v_unsigned_int);   break;
        case SAIL_VARIANT_TYPE_LONG:           set_value(variant.d->v_long);           break;
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  set_value(variant.d->v_unsigned_long);  break;
        case SAIL_VARIANT_TYPE_FLOAT:          set_value(variant.d->v_float);          break;
        case SAIL_VARIANT_TYPE_DOUBLE:         set_value(variant.d->v_double);         break;
        case SAIL_VARIANT_TYPE_STRING:         set_value(variant.d->v_string);         break;
        case SAIL_VARIANT_TYPE_DATA:           set_value(variant.d->v_arbitrary_data); break;

        case SAIL_VARIANT_TYPE_INVALID: {
            d->destroy_value();
            d->type = SAIL_VARIANT_TYPE_INVALID;
            break;
        }
    }

    return *this;
}

variant::variant(sail::variant &&variant) noexcept
{
    *this = std::move(variant);
}

variant& variant::operator=(sail::variant &&variant) noexcept
{
    d = std::move(variant.d);

    return *this;
}

variant::~variant()
{
}

bool variant::is_valid() const
{
    return d->type != SAIL_VARIANT_TYPE_INVALID;
}

template<typename T>
bool variant::has_value() const
{
    return d->type == d->type_to_sail_variant_type<T>();
}

// Allow only specific types. Other types will fail to link.
//
template SAIL_EXPORT bool variant::has_value<bool>() const;

template SAIL_EXPORT bool variant::has_value<char>() const;
template SAIL_EXPORT bool variant::has_value<unsigned char>() const;

template SAIL_EXPORT bool variant::has_value<short>() const;
template SAIL_EXPORT bool variant::has_value<unsigned short>() const;

template SAIL_EXPORT bool variant::has_value<int>() const;
template SAIL_EXPORT bool variant::has_value<unsigned int>() const;

template SAIL_EXPORT bool variant::has_value<long>() const;
template SAIL_EXPORT bool variant::has_value<unsigned long>() const;

template SAIL_EXPORT bool variant::has_value<float>() const;
template SAIL_EXPORT bool variant::has_value<double>() const;

template SAIL_EXPORT bool variant::has_value<std::string>() const;
template SAIL_EXPORT bool variant::has_value<sail::arbitrary_data>() const;

// Allow only specific types. Other types will fail to link.
//
template<>
SAIL_EXPORT const bool& variant::value() const
{
    return d->v_bool;
}

template<>
SAIL_EXPORT const char& variant::value() const
{
    return d->v_char;
}

template<>
SAIL_EXPORT const unsigned char& variant::value() const
{
    return d->v_unsigned_char;
}

template<>
SAIL_EXPORT const short& variant::value() const
{
    return d->v_short;
}

template<>
SAIL_EXPORT const unsigned short& variant::value() const
{
    return d->v_unsigned_short;
}

template<>
SAIL_EXPORT const int& variant::value() const
{
    return d->v_int;
}

template<>
SAIL_EXPORT const unsigned int& variant::value() const
{
    return d->v_unsigned_int;
}

template<>
SAIL_EXPORT const long& variant::value() const
{
    return d->v_long;
}

template<>
SAIL_EXPORT const unsigned long& variant::value() const
{
    return d->v_unsigned_long;
}

template<>
SAIL_EXPORT const float& variant::value() const
{
    return d->v_float;
}

template<>
SAIL_EXPORT const double& variant::value() const
{
    return d->v_double;
}

template<>
SAIL_EXPORT const std::string& variant::value() const
{
    return d->v_string;
}

template<>
SAIL_EXPORT const arbitrary_data& variant::value() const
{
    return d->v_arbitrary_data;
}

sail_status_t variant::to_sail_variant(sail_variant **variant) const
{
    SAIL_CHECK_PTR(variant);

    struct sail_variant *variant_local;
    SAIL_TRY(sail_alloc_variant(&variant_local));

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_variant(variant_local);
    );

    variant_local->type = d->type;

    switch (d->type) {
        case SAIL_VARIANT_TYPE_BOOL:           SAIL_TRY(sail_set_variant_bool(variant_local,           d->v_bool));           break;
        case SAIL_VARIANT_TYPE_CHAR:           SAIL_TRY(sail_set_variant_char(variant_local,           d->v_char));           break;
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  SAIL_TRY(sail_set_variant_unsigned_char(variant_local,  d->v_unsigned_char));  break;
        case SAIL_VARIANT_TYPE_SHORT:          SAIL_TRY(sail_set_variant_short(variant_local,          d->v_short));          break;
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: SAIL_TRY(sail_set_variant_unsigned_short(variant_local, d->v_unsigned_short)); break;
        case SAIL_VARIANT_TYPE_INT:            SAIL_TRY(sail_set_variant_int(variant_local,            d->v_int));            break;
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   SAIL_TRY(sail_set_variant_unsigned_int(variant_local,   d->v_unsigned_int));   break;
        case SAIL_VARIANT_TYPE_LONG:           SAIL_TRY(sail_set_variant_long(variant_local,           d->v_long));           break;
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  SAIL_TRY(sail_set_variant_unsigned_long(variant_local,  d->v_unsigned_long));  break;
        case SAIL_VARIANT_TYPE_FLOAT:          SAIL_TRY(sail_set_variant_float(variant_local,          d->v_float));          break;
        case SAIL_VARIANT_TYPE_DOUBLE:         SAIL_TRY(sail_set_variant_double(variant_local,         d->v_double));         break;
        case SAIL_VARIANT_TYPE_STRING:         SAIL_TRY(sail_set_variant_string(variant_local,         d->v_string.c_str())); break;
        case SAIL_VARIANT_TYPE_DATA: {
            const sail::arbitrary_data &arbitrary_data = d->v_arbitrary_data;
            SAIL_TRY(sail_set_variant_data(variant_local, arbitrary_data.data(), arbitrary_data.size()));
            break;
        }
        case SAIL_VARIANT_TYPE_INVALID: break;
    }

    *variant = variant_local;
    variant_local = nullptr;

    return SAIL_OK;
}

bool operator==(const sail::variant &a, const sail::variant &b) {

    if (!a.is_valid() || !b.is_valid() || a.d->type != b.d->type) {
        return false;
    }

    switch (a.d->type) {
        case SAIL_VARIANT_TYPE_BOOL:           return a.d->v_bool           == b.d->v_bool;
        case SAIL_VARIANT_TYPE_CHAR:           return a.d->v_char           == b.d->v_char;
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  return a.d->v_unsigned_char  == b.d->v_unsigned_char;
        case SAIL_VARIANT_TYPE_SHORT:          return a.d->v_short          == b.d->v_short;
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: return a.d->v_unsigned_short == b.d->v_unsigned_short;
        case SAIL_VARIANT_TYPE_INT:            return a.d->v_int            == b.d->v_int;
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   return a.d->v_unsigned_int   == b.d->v_unsigned_int;
        case SAIL_VARIANT_TYPE_LONG:           return a.d->v_long           == b.d->v_long;
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  return a.d->v_unsigned_long  == b.d->v_unsigned_long;
        case SAIL_VARIANT_TYPE_FLOAT:          return a.d->v_float          == b.d->v_float;
        case SAIL_VARIANT_TYPE_DOUBLE:         return a.d->v_double         == b.d->v_double;
        case SAIL_VARIANT_TYPE_STRING:         return a.d->v_string         == b.d->v_string;
        case SAIL_VARIANT_TYPE_DATA: {
            const auto &a_arbitrary_data = a.d->v_arbitrary_data;
            const auto &b_arbitrary_data = b.d->v_arbitrary_data;

            return a_arbitrary_data.size() == b_arbitrary_data.size() &&
                    memcmp(a_arbitrary_data.data(), b_arbitrary_data.data(), a_arbitrary_data.size()) == 0;
        }
        case SAIL_VARIANT_TYPE_INVALID: return false;
    }

    return false;
}

bool operator!=(const sail::variant &a, const sail::variant &b) {

    return !(a == b);
}

}
