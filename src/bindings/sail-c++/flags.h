/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#ifndef SAIL_FLAGS_CPP_H
#define SAIL_FLAGS_CPP_H

#include <type_traits>

namespace sail
{

template<typename T>
concept EnumFlag = std::is_enum_v<T>;

/*
 * Base class for combining enum flags.
 */
template<EnumFlag T>
class Flags {

public:
    using underlying_t = std::underlying_type_t<T>;

    /*
     * Constructs a default flags set.
     */
    constexpr Flags(T flag) : m_value(static_cast<underlying_t>(flag)) {}

    constexpr Flags operator|(T flag) const {
        return Flags(m_value | static_cast<underlying_t>(flag));
    }
    constexpr Flags operator|(Flags flags) const {
        return Flags(m_value | flags.value);
    }
    constexpr Flags& operator|=(T flag) {
        m_value |= static_cast<underlying_t>(flag);
        return *this;
    }
    constexpr Flags& operator|=(Flags flags) {
        m_value |= flags.value;
        return *this;
    }

    constexpr Flags operator&(T flag) const {
        return Flags(m_value & static_cast<underlying_t>(flag));
    }
    constexpr Flags operator&(Flags flags) const {
        return Flags(m_value & flags.value);
    }
    constexpr Flags& operator&=(T flag) {
        m_value &= static_cast<underlying_t>(flag);
        return *this;
    }
    constexpr Flags& operator&=(Flags flags) {
        m_value &= flags.value;
        return *this;
    }

    constexpr Flags operator^(T flag) const {
        return Flags(m_value ^ static_cast<underlying_t>(flag));
    }
    constexpr Flags operator^(Flags flags) const {
        return Flags(m_value ^ flags.value);
    }
    constexpr Flags& operator^=(T flag) {
        m_value ^= static_cast<underlying_t>(flag);
        return *this;
    }
    constexpr Flags& operator^=(Flags flags) {
        m_value ^= flags.value;
        return *this;
    }

    constexpr operator bool() const {
        return m_value != 0;
    }

    constexpr bool operator==(Flags flags) const {
        return m_value == flags.m_value;
    }
    constexpr bool operator!=(Flags flags) const {
        return m_value != flags.m_value;
    }

    constexpr underlying_t underlying_value() const {
        return m_value;
    }

private:
    constexpr explicit Flags(underlying_t value) : m_value(value) {}

    underlying_t m_value;
};

}

#endif
