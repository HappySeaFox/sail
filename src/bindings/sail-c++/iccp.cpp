/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

module sail.cpp;

import <cstddef>; /* std::size_t */
import <cstring>;

#include <sail/sail.h>

namespace sail
{

/*
 * ICC profile.
 */
export class SAIL_EXPORT iccp
{
    friend class image;

public:
    /*
     * Constructs an invalid ICC profile.
     */
    iccp();

    /*
     * Constructs a new ICC profile from the binary data. The data is deep copied.
     */
    iccp(const void *data, std::size_t data_size);

    /*
     * Constructs a new ICC profile from the binary data.
     */
    explicit iccp(const arbitrary_data &data);

    /*
     * Copies the ICC profile.
     */
    iccp(const sail::iccp &iccp);

    /*
     * Copies the ICC profile.
     */
    iccp& operator=(const sail::iccp &iccp);

    /*
     * Moves the ICC profile.
     */
    iccp(sail::iccp &&iccp) noexcept;

    /*
     * Moves the ICC profile.
     */
    iccp& operator=(sail::iccp &&iccp) noexcept;

    /*
     * Destroys the ICC profile.
     */
    ~iccp();

    /*
     * Returns true if the ICC profile data is not empty. It doesn't validate the data.
     */
    bool is_valid() const;

    /*
     * Returns the ICC profile binary data.
     */
    const arbitrary_data& data() const;

    /*
     * Sets new ICC profile binary data. The data is deep copied.
     */
    void set_data(const void *data, std::size_t data_size);

    /*
     * Sets new ICC profile binary data.
     */
    void set_data(const arbitrary_data &data);

private:
    /*
     * Makes a deep copy of the specified ICC profile.
     */
    explicit iccp(const sail_iccp *ic);

    sail_status_t to_sail_iccp(sail_iccp **iccp) const;

    void copy(const void *data, std::size_t data_size);

private:
    arbitrary_data m_data;
};

iccp::iccp()
{
}

iccp::iccp(const void *data, std::size_t data_size)
    : iccp()
{
    set_data(data, data_size);
}

iccp::iccp(const arbitrary_data &data)
    : iccp()
{
    set_data(data);
}

iccp::iccp(const sail::iccp &iccp)
    : iccp()
{
    *this = iccp;
}

iccp& iccp::operator=(const sail::iccp &iccp)
{
    m_data.clear();

    if (iccp.is_valid()) {
        set_data(iccp.data());
    }

    return *this;
}

iccp::iccp(sail::iccp &&iccp) noexcept
{
    *this = std::move(iccp);
}

iccp& iccp::operator=(sail::iccp &&iccp) noexcept
{
    m_data = std::move(iccp.m_data);

    return *this;
}

iccp::~iccp()
{
}

bool iccp::is_valid() const
{
    return !m_data.empty();
}

const arbitrary_data& iccp::data() const
{
    return m_data;
}

void iccp::set_data(const void *data, std::size_t data_size)
{
    m_data.clear();

    copy(data, data_size);
}

void iccp::set_data(const arbitrary_data &data)
{
    set_data(data.data(), data.size());
}

iccp::iccp(const sail_iccp *ic)
    : iccp()
{
    if (ic == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::iccp(). The object is untouched");
        return;
    }

    set_data(ic->data, ic->size);
}

sail_status_t iccp::to_sail_iccp(sail_iccp **iccp) const
{
    SAIL_CHECK_PTR(iccp);

    sail_iccp *iccp_local;
    SAIL_TRY(sail_alloc_iccp_from_data(m_data.data(), static_cast<std::size_t>(m_data.size()), &iccp_local));

    *iccp = iccp_local;

    return SAIL_OK;
}

void iccp::copy(const void *data, std::size_t data_size)
{
    m_data.resize(data_size);

    if (data_size > 0) {
        memcpy(m_data.data(), data, data_size);
    }
}

}
