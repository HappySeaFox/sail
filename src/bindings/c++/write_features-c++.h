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

#ifndef SAIL_WRITE_FEATURES_CPP_H
#define SAIL_WRITE_FEATURES_CPP_H

#include <map>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_write_features;

namespace sail
{

class write_options;

/*
 * A C++ interface to struct sail_write_features.
 */
class SAIL_EXPORT write_features
{
    friend class codec_info;

public:
    write_features(const write_features &wf);
    write_features& operator=(const write_features &wf);
    write_features(write_features &&wf) noexcept;
    write_features& operator=(write_features &&wf);
    ~write_features();

    const std::vector<SailPixelFormat>& output_pixel_formats() const;
    int features() const;
    int properties() const;
    const std::vector<SailCompression>& compressions() const;
    SailCompression default_compression() const;
    double compression_level_min() const;
    double compression_level_max() const;
    double compression_level_default() const;
    double compression_level_step() const;

    sail_status_t to_write_options(write_options *swrite_options) const;

private:
    write_features();
    /*
     * Makes a deep copy of the specified write features and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit write_features(const sail_write_features *wf);

    write_features& with_output_pixel_formats(const std::vector<SailPixelFormat> &output_pixel_formats);
    write_features& with_features(int features);
    write_features& with_properties(int properties);
    write_features& with_compressions(const std::vector<SailCompression> &compressions);
    write_features& with_default_compression(SailCompression default_compression);
    write_features& with_compression_level_min(double compression_level_min);
    write_features& with_compression_level_max(double compression_level_max);
    write_features& with_compression_level_default(double compression_level_default);
    write_features& with_compression_level_step(double compression_level_step);

    const sail_write_features* sail_write_features_c() const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
