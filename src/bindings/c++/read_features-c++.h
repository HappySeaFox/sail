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

#ifndef SAIL_READ_FEATURES_CPP_H
#define SAIL_READ_FEATURES_CPP_H

#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_read_features;

namespace sail
{

class read_options;

/*
 * Read features. Use this class to determine what a codec can actually read.
 * See codec_info.
 */
class SAIL_EXPORT read_features
{
    friend class codec_info;

public:
    /*
     * Copies the read features.
     */
    read_features(const read_features &rf);

    /*
     * Copies the read features.
     */
    read_features& operator=(const sail::read_features &read_features);

    /*
     * Moves the read features.
     */
    read_features(sail::read_features &&read_features) noexcept;

    /*
     * Moves the read features.
     */
    read_features& operator=(sail::read_features &&read_features) noexcept;

    /*
     * Destroys the read features.
     */
    ~read_features();

    /*
     * Returns the supported or-ed features of reading operations. See SailCodecFeature.
     */
    int features() const;

    /*
     * Builds default read options from the read features. Can be used to build
     * default read options and then slightly modify them before passing to image_input.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t to_read_options(sail::read_options *read_options) const;

private:
    read_features();

    /*
     * Makes a deep copy of the specified read features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit read_features(const sail_read_features *rf);

    const sail_read_features* sail_read_features_c() const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
