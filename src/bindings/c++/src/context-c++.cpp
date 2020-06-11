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

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN context::pimpl
{
public:
    sail_context *context = nullptr;
    sail_error_t init_result = SAIL_CONTEXT_UNINITIALIZED;
};

context::context()
    : d(new pimpl)
{
    d->init_result = init(0);

    if (d->init_result != 0) {
        SAIL_LOG_ERROR("Failed to initialize SAIL. Error: %d", d->init_result);
    }
}

context::context(int flags)
    : d(new pimpl)
{
    d->init_result = init(flags);

    if (d->init_result != 0) {
        SAIL_LOG_ERROR("Failed to initialize SAIL. Error: %d", d->init_result);
    }
}

context::~context()
{
    sail_finish(d->context);
    delete d;
}

sail_error_t context::status() const
{
    return d->init_result;
}

std::vector<plugin_info> context::plugin_info_list() const
{
    std::vector<plugin_info> list;

    const sail_plugin_info_node *plugin_info_node = sail_plugin_info_list(d->context);

    while (plugin_info_node != nullptr) {
        list.push_back(plugin_info(plugin_info_node->plugin_info));
        plugin_info_node = plugin_info_node->next;
    }

    return list;
}

sail_error_t context::unload_plugins()
{
    SAIL_TRY(sail_unload_plugins(d->context));

    return 0;
}

sail_error_t context::plugin_info_from_path(const std::string &path, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_path(path.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_path(const char *path, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_path(path, d->context, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::plugin_info_from_extension(const std::string &suffix, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_extension(suffix.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_extension(const char *suffix, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_extension(suffix, d->context, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::plugin_info_from_mime_type(const std::string &mime_type, plugin_info *splugin_info) const
{
    SAIL_TRY(plugin_info_from_mime_type(mime_type.c_str(), splugin_info));

    return 0;
}

sail_error_t context::plugin_info_from_mime_type(const char *mime_type, plugin_info *splugin_info) const
{
    SAIL_CHECK_PLUGIN_INFO_PTR(splugin_info);

    const struct sail_plugin_info *sail_plugin_info;
    SAIL_TRY(sail_plugin_info_from_mime_type(d->context, mime_type, &sail_plugin_info));

    *splugin_info = plugin_info(sail_plugin_info);

    return 0;
}

sail_error_t context::init(int flags)
{
    SAIL_TRY(sail_init_with_flags(&d->context, flags));

    return 0;
}

sail_context* context::sail_context_c() const
{
    return d->context;
}

}
