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

#ifndef SAIL_LOAD_FEATURES_CPP_H
#define SAIL_LOAD_FEATURES_CPP_H

#include <memory>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "tuning-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/tuning-c++.h>
#endif

struct sail_load_features;

namespace sail
{

class load_options;

/*
 * Load features. Use this class to determine what a codec can actually read.
 * See codec_info.
 */
class SAIL_EXPORT load_features
{
    friend class codec_info;

public:
    /*
     * Copies the load features.
     */
    load_features(const load_features &rf);

    /*
     * Copies the load features.
     */
    load_features& operator=(const sail::load_features &load_features);

    /*
     * Moves the load features.
     */
    load_features(sail::load_features &&load_features) noexcept;

    /*
     * Moves the load features.
     */
    load_features& operator=(sail::load_features &&load_features) noexcept;

    /*
     * Destroys the load features.
     */
    ~load_features();

    /*
     * Returns the supported or-ed features of loading operations. See SailCodecFeature.
     */
    int features() const;

    /*
     * Returns supported codec-specific tuning options. For example, a hypothetical ABC
     * image codec can allow disabling filtering with setting the "abc-filtering"
     * tuning option to 0 in load options. Tuning options' names start with the codec name
     * to avoid confusing.
     *
     * The list of possible values for every tuning option is not current available
     * programmatically. Every codec must document them in the codec info.
     *
     * It's not guaranteed that tuning options and their values are backward
     * or forward compatible.
     */
    const sail::supported_tuning& supported_tuning() const;

    /*
     * Builds default load options from the load features. Can be used to build
     * default load options and then slightly modify them before passing to image_input.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t to_options(sail::load_options *load_options) const;

private:
    load_features();

    /*
     * Makes a deep copy of the specified load features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit load_features(const sail_load_features *rf);

    const sail_load_features* sail_load_features_c() const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
