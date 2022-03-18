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

#include <cstring>
#include <stdexcept>

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN meta_data::pimpl
{
public:
    pimpl()
        : sail_meta_data(nullptr)
    {
        SAIL_TRY_OR_EXECUTE(sail_alloc_meta_data(&sail_meta_data),
                            /* on error */ throw std::bad_alloc());
    }

    ~pimpl()
    {
        sail_destroy_meta_data(sail_meta_data);
    }

    struct sail_meta_data *sail_meta_data;
    std::string key_unknown;
    variant value;
};

meta_data::meta_data()
    : d(new pimpl)
{
}

meta_data::meta_data(const sail::meta_data &md)
    : meta_data()
{
    *this = md;
}

meta_data& meta_data::operator=(const sail::meta_data &meta_data)
{
    if (meta_data.key() == SAIL_META_DATA_UNKNOWN) {
        with_key_unknown(meta_data.key_unknown());
    } else {
        with_key(meta_data.key());
    }

    with_value(meta_data.value());

    return *this;
}

meta_data::meta_data(sail::meta_data &&meta_data) noexcept
{
    *this = std::move(meta_data);
}

meta_data& meta_data::operator=(sail::meta_data &&meta_data) noexcept
{
    d = std::move(meta_data.d);

    return *this;
}

meta_data::~meta_data()
{
}

SailMetaData meta_data::key() const
{
    return d->sail_meta_data->key;
}

const std::string& meta_data::key_unknown() const
{
    return d->key_unknown;
}

const variant& meta_data::value() const
{
    return d->value;
}

meta_data& meta_data::with_key(SailMetaData key)
{
    d->sail_meta_data->key = key;
    d->key_unknown         = std::string{};

    return *this;
}

meta_data& meta_data::with_key_unknown(const std::string &key_unknown)
{
    d->sail_meta_data->key = SAIL_META_DATA_UNKNOWN;
    d->key_unknown         = key_unknown;

    return *this;
}

meta_data& meta_data::with_value(const variant &value)
{
    d->value = value;

    return *this;
}

const char* meta_data::meta_data_to_string(SailMetaData meta_data) {

    return sail_meta_data_to_string(meta_data);
}

SailMetaData meta_data::meta_data_from_string(const std::string_view str) {

    return sail_meta_data_from_string(str.data());
}

static inline std::string empty_string_on_nullptr(const char *str) {

    return str == nullptr ? std::string{} : str;
}

meta_data::meta_data(const sail_meta_data *meta_data)
    : sail::meta_data()
{
    if (meta_data == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::meta_data(). The object is untouched");
        return;
    }

    if (meta_data->key == SAIL_META_DATA_UNKNOWN) {
        with_key_unknown(empty_string_on_nullptr(meta_data->key_unknown));
    } else {
        with_key(meta_data->key);
    }

    with_value(variant(meta_data->value));
}

sail_status_t meta_data::to_sail_meta_data(sail_meta_data **meta_data) const
{
    SAIL_CHECK_PTR(meta_data);

    struct sail_meta_data *meta_data_local;
    SAIL_TRY(sail_alloc_meta_data(&meta_data_local));

    SAIL_AT_SCOPE_EXIT(
        sail_destroy_meta_data(meta_data_local);
    );

    meta_data_local->key = d->sail_meta_data->key;

    if (d->sail_meta_data->key == SAIL_META_DATA_UNKNOWN) {
        SAIL_TRY(sail_strdup(d->key_unknown.c_str(), &meta_data_local->key_unknown));
    }

    SAIL_TRY(d->value.to_sail_variant(&meta_data_local->value));

    *meta_data = meta_data_local;
    meta_data_local = nullptr;

    return SAIL_OK;
}

}
