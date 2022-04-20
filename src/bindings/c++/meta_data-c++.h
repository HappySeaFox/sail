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

#ifndef SAIL_META_DATA_CPP_H
#define SAIL_META_DATA_CPP_H

#include <memory>
#include <string>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "arbitrary_data-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/arbitrary_data-c++.h>
#endif

struct sail_meta_data;

namespace sail
{

class variant;

/*
 * meta_data represents a meta data element like a JPEG comment or a binary EXIF profile.
 */
class SAIL_EXPORT meta_data
{
    friend class image;

public:
    /*
     * Constructs an empty meta data entry.
     */
    meta_data();

    /*
     * Constructs a new meta data entry out of the known meta data
     * key and the value.
     */
    meta_data(SailMetaData key, const variant &value);

    /*
     * Constructs a new meta data entry out of the known meta data
     * key and the value.
     */
    meta_data(SailMetaData key, variant &&value) noexcept;

    /*
     * Constructs a new meta data entry out of the unknown meta data
     * string key and the value.
     */
    meta_data(const std::string &key_unknown, const variant &value);

    /*
     * Constructs a new meta data entry out of the unknown meta data
     * string key and the value.
     */
    meta_data(const std::string &key_unknown, variant &&value);

    /*
     * Constructs a new meta data entry out of the unknown meta data
     * string key and the value.
     */
    meta_data(std::string &&key_unknown, variant &&value) noexcept;

    /*
     * Copies the meta data entry.
     */
    meta_data(const meta_data &md);

    /*
     * Copies the meta data entry.
     */
    meta_data& operator=(const sail::meta_data &meta_data);

    /*
     * Moves the meta data entry.
     */
    meta_data(sail::meta_data &&meta_data) noexcept;

    /*
     * Moves the meta data entry.
     */
    meta_data& operator=(sail::meta_data &&meta_data) noexcept;

    /*
     * Destroys the meta data entry.
     */
    ~meta_data();

    /*
     * Returns the meta data key when it's well known like Artist or Comment.
     * When key() returns SAIL_META_DATA_UNKNOWN, use key_unknown() to get the
     * key string representation.
     */
    SailMetaData key() const;

    /*
     * Returns the meta data string key representation when key() returns SAIL_META_DATA_UNKNOWN.
     * For example: "Person on the Image".
     */
    const std::string& key_unknown() const;

    /*
     * Returns the actual meta data value.
     */
    const variant& value() const;

    /*
     * Sets a new known meta data key like Artist or Comment. Resets the saved unknown key to an empty string.
     */
    void set_key(SailMetaData key);

    /*
     * Sets a new unknown meta data string key representation. Resets the saved key to SAIL_META_DATA_UNKNOWN.
     * For example: "Person on the Image".
     */
    void set_key(const std::string &key_unknown);

    /*
     * Sets a new unknown meta data string key representation. Resets the saved key to SAIL_META_DATA_UNKNOWN.
     * For example: "Person on the Image".
     */
    void set_key(std::string &&key_unknown) noexcept;

    /*
     * Sets a new meta data binary value. Resets the saved string value.
     */
    void set_value(const variant &value);

    /*
     * Sets a new meta data binary value. Resets the saved string value.
     */
    void set_value(variant &&value) noexcept;

    /*
     * Returns a string representation of the specified meta data key. See SailMetaData.
     * For example: "Author" is returned for SAIL_META_DATA_AUTHOR.
     *
     * Returns nullptr if the meta data key is not known.
     */
    static const char* meta_data_to_string(SailMetaData meta_data);

    /*
     * Returns a meta data key from the string representation. See SailMetaData.
     * For example: SAIL_META_DATA_AUTHOR is returned for "Author".
     *
     * Returns SAIL_META_DATA_UNKNOWN if the meta data key is not known.
     */
    static SailMetaData meta_data_from_string(const std::string &str);

private:
    /*
     * Makes a deep copy of the specified meta data.
     */
    explicit meta_data(const sail_meta_data *meta_data);

    sail_status_t to_sail_meta_data(sail_meta_data **meta_data) const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
