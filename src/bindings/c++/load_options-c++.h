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

#ifndef SAIL_LOAD_OPTIONS_CPP_H
#define SAIL_LOAD_OPTIONS_CPP_H

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

struct sail_load_options;

namespace sail
{

/*
 * load_options represents options to modify loading operations. See image_input.
 */
class SAIL_EXPORT load_options
{
    friend class image_input;
    friend class load_features;

public:
    /*
     * Constructs empty load options.
     */
    load_options();

    /*
     * Copies the load options.
     */
    load_options(const load_options &ro);

    /*
     * Copies the load options.
     */
    load_options& operator=(const sail::load_options &load_options);

    /*
     * Moves the load options.
     */
    load_options(sail::load_options &&load_options) noexcept;

    /*
     * Moves the load options.
     */
    load_options& operator=(sail::load_options &&load_options) noexcept;

    /*
     * Destroys the load options.
     */
    ~load_options();

    /*
     * Returns the or-ed manipulation options for loading operations. See SailOption.
     */
    int options() const;

    /*
     * Returns editable codec-specific tuning options. For example, a hypothetical ABC
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
    sail::tuning& tuning();

    /*
     * Returns constant codec-specific tuning options. For example, a hypothetical ABC
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
    const sail::tuning& tuning() const;

    /*
     * Sets new or-ed manipulation options for loading operations. See SailOption.
     */
    void set_options(int options);

    /*
     * Sets new codec tuning.
     */
    void set_tuning(const sail::tuning &tuning);

private:
    /*
     * Makes a deep copy of the specified load options and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit load_options(const sail_load_options *ro);

    sail_status_t to_sail_load_options(sail_load_options **load_options) const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
