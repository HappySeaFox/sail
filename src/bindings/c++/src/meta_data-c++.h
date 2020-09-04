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

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#include <string>

struct sail_meta_data;

namespace sail
{

/*
 * Image meta data.
 */
class SAIL_EXPORT meta_data
{
    friend class image;

public:
    meta_data();
    meta_data(const meta_data &md);
    meta_data& operator=(const meta_data &md);
    ~meta_data();

    /*
     * Returns the meta data key.
     */
    SailMetaData key() const;

    /*
     * Returns the meta data string key representation when key() is SAIL_META_DATA_UNKNOWN.
     */
    std::string key_unknown() const;

    /*
     * Returns the meta data value.
     */
    std::string value() const;

    /*
     * Sets new meta data key.
     */
    meta_data& with_key(SailMetaData key);

    /*
     * Sets a new meta data string key representation when key() is SAIL_META_DATA_UNKNOWN.
     */
    meta_data& with_key_unknown(const std::string &key_unknown);

    /*
     * Sets a new meta data value.
     */
    meta_data& with_value(const std::string &value);

    /*
     * Assigns a non-NULL string representation of the specified meta data key. See SailMetaData.
     * The assigned string MUST NOT be destroyed. For example: "Author".
     *
     * Returns 0 on success or sail_status_t on error.
     */
    static sail_status_t meta_data_to_string(enum SailMetaData meta_data, const char **result);

    /*
     * Assigns meta data key from a string representation. See SailMetaData.
     * For example: SAIL_META_DATA_AUTHOR is assigned for "Author".
     *
     * Returns 0 on success or sail_status_t on error.
     */
    static sail_status_t meta_data_from_string(const char *str, enum SailMetaData *result);

private:
    /*
     * Makes a deep copy of the specified meta data.
     */
    meta_data(const sail_meta_data_node *md);

    sail_status_t to_sail_meta_data_node(sail_meta_data_node *md) const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
