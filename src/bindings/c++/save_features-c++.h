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

#ifndef SAIL_SAVE_FEATURES_CPP_H
#define SAIL_SAVE_FEATURES_CPP_H

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

struct sail_save_features;

namespace sail
{

class save_options;

/*
 * Save features. Use this structure to determine what a codec can actually write.
 * See codec_info.
 */
class SAIL_EXPORT save_features
{
    friend class codec_info;

public:
    /*
     * Copies the save features.
     */
    save_features(const save_features &wf);

    /*
     * Copies the save features.
     */
    save_features& operator=(const sail::save_features &save_features);

    /*
     * Moves the save features.
     */
    save_features(sail::save_features &&save_features) noexcept;

    /*
     * Moves the save features.
     */
    save_features& operator=(sail::save_features &&save_features) noexcept;

    /*
     * Destroys the save features.
     */
    ~save_features();

    /*
     * Returns the list of supported pixel formats that can be written by this codec.
     */
    const std::vector<SailPixelFormat>& pixel_formats() const;

    /*
     * Returns the supported or-ed features of saving operations. See SailCodecFeature.
     */
    int features() const;

    /*
     * Returns the list of supported pixels compression types by this codec. If the list has more than
     * two entries, compression levels are ignored.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. compression_level() can be used
     *        to select its compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.).
     *        Compression levels are ignored.
     */
    const std::vector<SailCompression>& compressions() const;

    /*
     * Returns the compression type to use by default.
     */
    SailCompression default_compression() const;

    /*
     * Returns the supported compression level range or an ivalid object if
     * compression levels are not supported by the codec.
     */
    const sail::compression_level& compression_level() const;

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
     * Builds default save options from the save features. Can be used to build
     * default save options and then slightly modify them before passing to image_output.
     *
     * Returns SAIL_OK on success.
     */
    sail_status_t to_options(sail::save_options *save_options) const;

private:
    save_features();
    /*
     * Makes a deep copy of the specified save features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit save_features(const sail_save_features *wf);

    const sail_save_features* sail_save_features_c() const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
