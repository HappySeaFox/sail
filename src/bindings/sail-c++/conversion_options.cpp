/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include <cstdint>
#include <stdexcept>

#include <sail-c++/sail-c++.h>

#include <sail-manip/sail-manip.h>

#include <sail-common/export.h>
#include <sail-common/pixel.h>
#include <sail-common/status.h>

namespace sail
{

/*
 * Image conversion options.
 */
export class SAIL_EXPORT conversion_options
{
    friend class image;

public:
    /*
     * Constructs an empty conversion options object.
     */
    conversion_options();

    /*
     * Constructs a new conversion options object out of the or-ed SailConversionOption-s
     * and the 48-bit color to blend 48-bit images.
     * If the options argument is zero, SAIL_CONVERSION_OPTION_DROP_ALPHA is assumed.
     * Additionally, calculates and sets a new 24-bit background color to blend 24-bit images.
     */
    conversion_options(int options, const sail_rgb48_t &rgb48);

    /*
     * Constructs a new conversion options object out of the or-ed SailConversionOption-s
     * and the 24-bit color to blend 24-bit images.
     * If the options argument is zero, SAIL_CONVERSION_OPTION_DROP_ALPHA is assumed.
     * Additionally, calculates and sets a new 48-bit background color to blend 48-bit images.
     */
    conversion_options(int options, const sail_rgb24_t &rgb24);

    /*
     * Copies the conversion options object.
     */
    conversion_options(const conversion_options &co);

    /*
     * Copies the conversion options object.
     */
    conversion_options& operator=(const conversion_options &co);

    /*
     * Moves the conversion options object.
     */
    conversion_options(conversion_options &&co) noexcept;

    /*
     * Moves the conversion options object.
     */
    conversion_options& operator=(conversion_options &&co) noexcept;

    /*
     * Destroys the conversion options object.
     */
    ~conversion_options();

    /*
     * Returns the or-ed SailConversionOption-s.
     */
    int options() const;

    /*
     * Returns the 48-bit background color to blend 48-bit images.
     */
    sail_rgb48_t background48() const;

    /*
     * Returns the 24-bit background color to blend 24-bit images.
     */
    sail_rgb24_t background24() const;

    /*
     * Sets new or-ed SailConversionOption-s. If zero, SAIL_CONVERSION_OPTION_DROP_ALPHA is assumed.
     */
    void set_options(int options);

    /*
     * Sets a new 48-bit background color to blend 48-bit images.
     * Additionally, calculates and sets a new 24-bit background color to blend 24-bit images.
     */
    void set_background(const sail_rgb48_t &rgb48);

    /*
     * Sets a new 24-bit background color to blend 24-bit images.
     * Additionally, calculates and sets a new 48-bit background color to blend 48-bit images.
     */
    void set_background(const sail_rgb24_t &rgb24);

private:
    sail_status_t to_sail_conversion_options(sail_conversion_options **conversion_options) const;

private:
    struct sail_conversion_options *m_sail_conversion_options{nullptr};
};

conversion_options::conversion_options()
{
    SAIL_TRY_OR_EXECUTE(sail_alloc_conversion_options(&m_sail_conversion_options),
                        /* on error */ throw std::bad_alloc());
}

conversion_options::conversion_options(int options, const sail_rgb48_t &rgb48)
    : conversion_options()
{
    set_options(options);
    set_background(rgb48);
}

conversion_options::conversion_options(int options, const sail_rgb24_t &rgb24)
    : conversion_options()
{
    set_options(options);
    set_background(rgb24);
}

conversion_options::conversion_options(const conversion_options &co)
    : conversion_options()
{
    *this = co;
}

conversion_options& conversion_options::operator=(const conversion_options &co)
{
    set_options(co.options());
    set_background(co.background48());
    set_background(co.background24());

    return *this;
}

conversion_options::conversion_options(conversion_options &&co) noexcept
{
    *this = std::move(co);
}

conversion_options& conversion_options::operator=(conversion_options &&co) noexcept
{
    sail_destroy_conversion_options(m_sail_conversion_options);

    m_sail_conversion_options = co.m_sail_conversion_options;
    co.m_sail_conversion_options = nullptr;

    return *this;
}

conversion_options::~conversion_options()
{
    sail_destroy_conversion_options(m_sail_conversion_options);
}

int conversion_options::options() const
{
    return m_sail_conversion_options->options;
}

sail_rgb48_t conversion_options::background48() const
{
    return m_sail_conversion_options->background48;
}

sail_rgb24_t conversion_options::background24() const
{
    return m_sail_conversion_options->background24;
}

void conversion_options::set_options(int options)
{
    m_sail_conversion_options->options = options;
}

void conversion_options::set_background(const sail_rgb48_t &rgb48)
{
    m_sail_conversion_options->background48 = rgb48;

    m_sail_conversion_options->background24 = {
        static_cast<std::uint8_t>(rgb48.component1 / 257),
        static_cast<std::uint8_t>(rgb48.component2 / 257),
        static_cast<std::uint8_t>(rgb48.component3 / 257)
    };
}

void conversion_options::set_background(const sail_rgb24_t &rgb24)
{
    m_sail_conversion_options->background24 = rgb24;

    m_sail_conversion_options->background48 = {
        static_cast<std::uint16_t>(rgb24.component1 * 257),
        static_cast<std::uint16_t>(rgb24.component2 * 257),
        static_cast<std::uint16_t>(rgb24.component3 * 257)
    };
}

sail_status_t conversion_options::to_sail_conversion_options(sail_conversion_options **conversion_options) const
{
    SAIL_CHECK_PTR(conversion_options);

    SAIL_TRY(sail_alloc_conversion_options(conversion_options));
    **conversion_options = *m_sail_conversion_options;

    return SAIL_OK;
}

}
