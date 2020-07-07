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

#ifndef SAIL_PLUGIN_INFO_CPP_H
#define SAIL_PLUGIN_INFO_CPP_H

#include <string>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"

    #include "read_features-c++.h"
    #include "write_features-c++.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>

    #include <sail-c++/read_features-c++.h>
    #include <sail-c++/write_features-c++.h>
#endif

struct sail_plugin_info;

namespace sail
{

/*
 * A C++ interface to struct sail_plugin_info.
 */
class SAIL_EXPORT plugin_info
{
    friend class context;
    friend class image_reader;
    friend class image_writer;

public:
    plugin_info();
    plugin_info(const plugin_info &pi);
    plugin_info& operator=(const plugin_info &pi);
    ~plugin_info();

    const std::string& path() const;
    const std::string& version() const;
    const std::string& name() const;
    const std::string& description() const;
    const std::vector<std::string>& magic_numbers() const;
    const std::vector<std::string>& extensions() const;
    const std::vector<std::string>& mime_types() const;
    const sail::read_features& read_features() const;
    const sail::write_features& write_features() const;

    static sail_error_t plugin_feature_to_string(SailPluginFeature plugin_feature, const char **result);
    static sail_error_t plugin_feature_from_string(const char *str, SailPluginFeature *result);

private:
    /*
     * Makes a deep copy of the specified plugin info and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    plugin_info(const sail_plugin_info *pi);

    plugin_info& with_path(const std::string &path);
    plugin_info& with_version(const std::string &version);
    plugin_info& with_name(const std::string &name);
    plugin_info& with_description(const std::string &description);
    plugin_info& with_magic_numbers(const std::vector<std::string> &magic_numbers);
    plugin_info& with_extensions(const std::vector<std::string> &extensions);
    plugin_info& with_mime_types(const std::vector<std::string> &mime_types);
    plugin_info& with_read_features(const sail::read_features &read_features);
    plugin_info& with_write_features(const sail::write_features &write_features);

    const sail_plugin_info* sail_plugin_info_c() const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
