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

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN meta_data::pimpl
{
public:
    pimpl()
        : key(SAIL_META_DATA_UNKNOWN)
        , value_type(SAIL_META_DATA_TYPE_STRING)
    {
    }

    ~pimpl()
    {
        free();
    }

    void free()
    {
        value_string.clear();
        value_data.clear();
    }

    SailMetaData key;
    std::string key_unknown;
    SailMetaDataType value_type;
    std::string value_string;
    arbitrary_data value_data;
};

meta_data::meta_data()
    : d(new pimpl)
{
}

meta_data::meta_data(const meta_data &md)
    : meta_data()
{
    *this = md;
}

meta_data& meta_data::operator=(const meta_data &md)
{
    if (md.key() == SAIL_META_DATA_UNKNOWN) {
        with_key_unknown(md.key_unknown());
    } else {
        with_key(md.key());
    }

    with_value_type(md.value_type());

    if (md.value_type() == SAIL_META_DATA_TYPE_STRING) {
        with_value(md.value<std::string>());
    } else {
        with_value(md.value<sail::arbitrary_data>());
    }

    return *this;
}

meta_data::meta_data(meta_data &&md) noexcept
{
    d = md.d;
    md.d = nullptr;
}

meta_data& meta_data::operator=(meta_data &&md)
{
    delete d;
    d = md.d;
    md.d = nullptr;

    return *this;
}

meta_data::~meta_data()
{
    delete d;
}

SailMetaData meta_data::key() const
{
    return d->key;
}

const std::string& meta_data::key_unknown() const
{
    return d->key_unknown;
}

SailMetaDataType meta_data::value_type() const
{
    return d->value_type;
}

meta_data& meta_data::with_key(SailMetaData key)
{
    d->key         = key;
    d->key_unknown = std::string();

    return *this;
}

meta_data& meta_data::with_key_unknown(const std::string &key_unknown)
{
    d->key         = SAIL_META_DATA_UNKNOWN;
    d->key_unknown = key_unknown;

    return *this;
}

meta_data& meta_data::with_value(const std::string &value)
{
    d->free();

    d->value_type   = SAIL_META_DATA_TYPE_STRING;
    d->value_string = value;

    return *this;
}

meta_data& meta_data::with_value(const char *value)
{
    with_value(std::string(value));
    return *this;
}

meta_data& meta_data::with_value(const arbitrary_data &value)
{
    d->free();

    d->value_type = SAIL_META_DATA_TYPE_DATA;
    d->value_data = value;

    return *this;
}

sail_status_t meta_data::meta_data_to_string(enum SailMetaData meta_data, const char **result) {

    SAIL_TRY(sail_meta_data_to_string(meta_data, result));

    return SAIL_OK;
}

sail_status_t meta_data::meta_data_from_string(const char *str, enum SailMetaData *result) {

    SAIL_TRY(sail_meta_data_from_string(str, result));

    return SAIL_OK;
}

static inline std::string empty_string_on_nullptr(const char *str) {

    return str == nullptr ? std::string() : str;
}

meta_data::meta_data(const sail_meta_data_node *md)
    : meta_data()
{
    if (md == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::meta_data(). The object is untouched");
        return;
    }

    if (md->key == SAIL_META_DATA_UNKNOWN) {
        with_key_unknown(empty_string_on_nullptr(md->key_unknown));
    } else {
        with_key(md->key);
    }

    with_value_type(md->value_type);

    if (md->value_type == SAIL_META_DATA_TYPE_STRING) {
        with_value(reinterpret_cast<const char *>(md->value));
    } else {
        arbitrary_data ad;
        ad.resize(md->value_length);
        memcpy(&ad.front(), md->value, md->value_length);
        with_value(ad);
    }
}

const std::string& meta_data::value_string() const
{
    return d->value_string;
}

const sail::arbitrary_data& meta_data::value_arbitrary_data() const
{
    return d->value_data;
}

meta_data& meta_data::with_value_type(SailMetaDataType type)
{
    d->value_type = type;
    return *this;
}

sail_status_t meta_data::to_sail_meta_data_node(sail_meta_data_node *md) const
{
    SAIL_CHECK_META_DATA_NODE_PTR(md);

    md->key = d->key;

    if (d->key == SAIL_META_DATA_UNKNOWN) {
        SAIL_TRY(sail_strdup(d->key_unknown.c_str(), &md->key_unknown));
    }

    md->value_type = d->value_type;

    if (d->value_type == SAIL_META_DATA_TYPE_STRING) {
        md->value_length = d->value_string.length() + 1;

        void *ptr;
        SAIL_TRY_OR_CLEANUP(sail_memdup(d->value_string.c_str(), md->value_length, &ptr),
                            /* cleanup */ sail_free(md->key_unknown));
        md->value = ptr;
    } else {
        md->value_length = d->value_data.size();

        void *ptr;
        SAIL_TRY_OR_CLEANUP(sail_memdup(d->value_data.data(), md->value_length, &ptr),
                            /* cleanup */ sail_free(md->key_unknown));

        md->value = ptr;
    }

    return SAIL_OK;
}

}
