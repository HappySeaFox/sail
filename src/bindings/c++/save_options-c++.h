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

#ifndef SAIL_SAVE_OPTIONS_CPP_H
#define SAIL_SAVE_OPTIONS_CPP_H

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

struct sail_save_options;

namespace sail
{

/*
 * Options to modify saving operations.
 */
class SAIL_EXPORT save_options
{
    friend class image_output;
    friend class save_features;

public:
    /*
     * Constructs empty save options.
     */
    save_options();

    /*
     * Copies the save options.
     */
    save_options(const save_options &wo);

    /*
     * Copies the save options.
     */
    save_options& operator=(const sail::save_options &save_options);

    /*
     * Moves the save options.
     */
    save_options(sail::save_options &&save_options) noexcept;

    /*
     * Moves the save options.
     */
    save_options& operator=(sail::save_options &&save_options) noexcept;

    /*
     * Destroys the save options.
     */
    ~save_options();

    /*
     * Returns the or-ed manipulation options for saving operations. See SailOption.
     */
    int options() const;

    /*
     * Returns the compression type. For example: SAIL_COMPRESSION_RLE. See SailCompression.
     * Use save_features to determine what compression types or values are supported by a particular codec.
     *
     * If a codec supports more than two compression types, compression levels are ignored in this case.
     *
     * For example:
     *
     *     1. The JPEG codec supports only one compression, JPEG. save_features.compression_level() can be used
     *        to select a compression level.
     *     2. The TIFF codec supports more than two compression types (PACKBITS, JPEG, etc.). Compression levels
     *        are ignored.
     */
    SailCompression compression() const;

    /*
     * Returns the requested compression level. Must be in the range specified in
     * save_features.compression_level().
     */
    double compression_level() const;

    /*
     * Returns modifiable codec tuning.
     */
    sail::tuning& tuning();

    /*
     * Returns constant codec tuning.
     */
    const sail::tuning& tuning() const;

    /*
     * Sets new or-ed manipulation options for saving operations. See SailOption.
     */
    void set_options(int options);

    /*
     * Sets a new compression type.
     */
    void set_compression(SailCompression compression);

    /*
     * Sets a new compression level.
     */
    void set_compression_level(double compression_level);

    /*
     * Sets a new codec tuning.
     */
    void set_tuning(const sail::tuning &tuning);

private:
    /*
     * Makes a deep copy of the specified save options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit save_options(const sail_save_options *wo);

    sail_status_t to_sail_save_options(sail_save_options **save_options) const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
