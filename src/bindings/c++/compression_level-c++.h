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

#ifndef SAIL_COMPRESSION_LEVEL_CPP_H
#define SAIL_COMPRESSION_LEVEL_CPP_H

#include <memory>

#ifdef SAIL_BUILD
    #include "export.h"
#else
    #include <sail-common/export.h>
#endif

struct sail_compression_level;

namespace sail
{

/*
 * Compression level.
 */
class SAIL_EXPORT compression_level
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
    compression_level& operator=(const sail::compression_level &compression_level);

    /*
     * Moves the compression level.
     */
    compression_level(sail::compression_level &&compression_level) noexcept;

    /*
     * Moves the compression level.
     */
    compression_level& operator=(sail::compression_level &&compression_level) noexcept;

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
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
