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

#include <unordered_map>

#include <sail-c++/sail-c++.h>

namespace sail
{

namespace
{

const std::unordered_map<std::size_t, SailVariantType> variant_types_map {
    { 0,  SAIL_VARIANT_TYPE_INVALID        },
    { 1,  SAIL_VARIANT_TYPE_BOOL           },
    { 2,  SAIL_VARIANT_TYPE_CHAR           },
    { 3,  SAIL_VARIANT_TYPE_UNSIGNED_CHAR  },
    { 4,  SAIL_VARIANT_TYPE_SHORT          },
    { 5,  SAIL_VARIANT_TYPE_UNSIGNED_SHORT },
    { 6,  SAIL_VARIANT_TYPE_INT            },
    { 7,  SAIL_VARIANT_TYPE_UNSIGNED_INT   },
    { 8,  SAIL_VARIANT_TYPE_LONG           },
    { 9,  SAIL_VARIANT_TYPE_UNSIGNED_LONG  },
    { 10, SAIL_VARIANT_TYPE_FLOAT          },
    { 11, SAIL_VARIANT_TYPE_DOUBLE         },
    { 12, SAIL_VARIANT_TYPE_STRING         },
    { 13, SAIL_VARIANT_TYPE_DATA           },
};

}

sail::variant from_struct(const sail_variant *sail_variant)
{
    switch (sail_variant->type) {
        case SAIL_VARIANT_TYPE_BOOL:           return variant(sail_variant_to_bool(sail_variant));
        case SAIL_VARIANT_TYPE_CHAR:           return variant(sail_variant_to_char(sail_variant));
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  return variant(sail_variant_to_unsigned_char(sail_variant));
        case SAIL_VARIANT_TYPE_SHORT:          return variant(sail_variant_to_short(sail_variant));
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: return variant(sail_variant_to_unsigned_short(sail_variant));
        case SAIL_VARIANT_TYPE_INT:            return variant(sail_variant_to_int(sail_variant));
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   return variant(sail_variant_to_unsigned_int(sail_variant));
        case SAIL_VARIANT_TYPE_LONG:           return variant(sail_variant_to_long(sail_variant));
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  return variant(sail_variant_to_unsigned_long(sail_variant));
        case SAIL_VARIANT_TYPE_FLOAT:          return variant(sail_variant_to_float(sail_variant));
        case SAIL_VARIANT_TYPE_DOUBLE:         return variant(sail_variant_to_double(sail_variant));
        case SAIL_VARIANT_TYPE_STRING:         return variant(std::string(sail_variant_to_string(sail_variant)));
        case SAIL_VARIANT_TYPE_DATA: {
            const void *data = sail_variant_to_data(sail_variant);
            sail::arbitrary_data arbitrary_data(sail_variant->size);
            memcpy(arbitrary_data.data(), data, sail_variant->size);
            return variant(std::move(arbitrary_data));
        }
        case SAIL_VARIANT_TYPE_INVALID: return {};
    }

    return {};
}

sail_status_t to_struct(const sail::variant &variant, sail_variant **sail_variant)
{
    SAIL_CHECK_PTR(sail_variant);

    struct sail_variant *variant_local;
    SAIL_TRY(sail_alloc_variant(&variant_local));

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_variant(variant_local);
    );

    variant_local->type = variant_types_map.at(variant.index());

    switch (variant_local->type) {
        case SAIL_VARIANT_TYPE_BOOL:           SAIL_TRY(sail_set_variant_bool(variant_local,           variant.value<bool>()));                break;
        case SAIL_VARIANT_TYPE_CHAR:           SAIL_TRY(sail_set_variant_char(variant_local,           variant.value<char>()));                break;
        case SAIL_VARIANT_TYPE_UNSIGNED_CHAR:  SAIL_TRY(sail_set_variant_unsigned_char(variant_local,  variant.value<unsigned char>()));       break;
        case SAIL_VARIANT_TYPE_SHORT:          SAIL_TRY(sail_set_variant_short(variant_local,          variant.value<short>()));               break;
        case SAIL_VARIANT_TYPE_UNSIGNED_SHORT: SAIL_TRY(sail_set_variant_unsigned_short(variant_local, variant.value<unsigned short>()));      break;
        case SAIL_VARIANT_TYPE_INT:            SAIL_TRY(sail_set_variant_int(variant_local,            variant.value<int>()));                 break;
        case SAIL_VARIANT_TYPE_UNSIGNED_INT:   SAIL_TRY(sail_set_variant_unsigned_int(variant_local,   variant.value<unsigned int>()));        break;
        case SAIL_VARIANT_TYPE_LONG:           SAIL_TRY(sail_set_variant_long(variant_local,           variant.value<long>()));                break;
        case SAIL_VARIANT_TYPE_UNSIGNED_LONG:  SAIL_TRY(sail_set_variant_unsigned_long(variant_local,  variant.value<unsigned long>()));       break;
        case SAIL_VARIANT_TYPE_FLOAT:          SAIL_TRY(sail_set_variant_float(variant_local,          variant.value<float>()));               break;
        case SAIL_VARIANT_TYPE_DOUBLE:         SAIL_TRY(sail_set_variant_double(variant_local,         variant.value<double>()));              break;
        case SAIL_VARIANT_TYPE_STRING:         SAIL_TRY(sail_set_variant_string(variant_local,         variant.value<std::string>().c_str())); break;
        case SAIL_VARIANT_TYPE_DATA: {
            const sail::arbitrary_data &arbitrary_data = variant.value<sail::arbitrary_data>();
            SAIL_TRY(sail_set_variant_data(variant_local, arbitrary_data.data(), arbitrary_data.size()));
            break;
        }
        case SAIL_VARIANT_TYPE_INVALID: break;
    }

    *sail_variant = variant_local;
    variant_local = nullptr;

    return SAIL_OK;
}

}
