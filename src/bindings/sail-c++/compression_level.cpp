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

import <stdexcept>;

#include <sail-common/compression_level.h>
#include <sail-common/export.h>

namespace sail
{

/*
 * Compression level.
 */
export class SAIL_EXPORT compression_level
{
    friend class save_features;

public:
    /*
     * Copies the compression level.
     */
    compression_level(const sail::compression_level &cl);

    /*
     * Copies the compression level.
     */
    compression_level& operator=(const compression_level &cl);

    /*
     * Moves the compression level.
     */
    compression_level(compression_level &&cl) noexcept;

    /*
     * Moves the compression level.
     */
    compression_level& operator=(compression_level &&cl) noexcept;

    /*
     * Destroys the compression level.
     */
    ~compression_level();

    /*
     * Returns true if min_level() < max_level() and default_level() is within the range.
     */
    bool is_valid() const;

    /*
     * Returns the minimum compression value. For lossy codecs, more compression
     * means less quality and vice versa. For lossless codecs, more compression
     * means nothing but a smaller file size.
     */
    double min_level() const;

    /*
     * Returns the maximum compression value. For lossy codecs, more compression
     * means less quality and vice versa. For lossless codecs, more compression
     * means nothing but a smaller file size.
     */
    double max_level() const;

    /*
     * Returns the default compression value within the min/max range.
     */
    double default_level() const;

    /*
     * Returns the step to increase or decrease compression levels in the range.
     * Can be used in UI to build a compression level selection component.
     */
    double step() const;

private:
    compression_level();

    /*
     * Makes a deep copy of the specified compression level.
     */
    explicit compression_level(const sail_compression_level *compression_level);

private:
    struct sail_compression_level *m_sail_compression_level{nullptr};
};

compression_level::compression_level(const compression_level &cl)
    : compression_level()
{
    *this = cl;
}

compression_level& compression_level::operator=(const compression_level &cl)
{
    m_sail_compression_level->min_level     = cl.min_level();
    m_sail_compression_level->max_level     = cl.max_level();
    m_sail_compression_level->default_level = cl.default_level();
    m_sail_compression_level->step          = cl.step();

    return *this;
}

compression_level::compression_level(compression_level &&cl) noexcept
{
    *this = std::move(cl);
}

compression_level& compression_level::operator=(compression_level &&cl) noexcept
{
    sail_destroy_compression_level(m_sail_compression_level);

    m_sail_compression_level = cl.m_sail_compression_level;
    cl.m_sail_compression_level = nullptr;

    return *this;
}

compression_level::~compression_level()
{
    sail_destroy_compression_level(m_sail_compression_level);
}

bool compression_level::is_valid() const
{
    return m_sail_compression_level->min_level < m_sail_compression_level->max_level &&
            m_sail_compression_level->default_level >= m_sail_compression_level->min_level &&
            m_sail_compression_level->default_level <= m_sail_compression_level->max_level;
}

double compression_level::min_level() const
{
    return m_sail_compression_level->min_level;
}

double compression_level::max_level() const
{
    return m_sail_compression_level->max_level;
}

double compression_level::default_level() const
{
    return m_sail_compression_level->default_level;
}

double compression_level::step() const
{
    return m_sail_compression_level->step;
}

compression_level::compression_level()
{
    SAIL_TRY_OR_EXECUTE(sail_alloc_compression_level(&m_sail_compression_level),
                        /* on error */ throw std::bad_alloc());
}

compression_level::compression_level(const sail_compression_level *cl)
    : compression_level()
{
    if (cl == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::compression_level(). The object is untouched");
        return;
    }

    *(m_sail_compression_level) = *cl;
}

}
