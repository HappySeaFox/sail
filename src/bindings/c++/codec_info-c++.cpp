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

#include "sail-c++.h"
#include "sail.h"

namespace sail
{

class SAIL_HIDDEN codec_info::pimpl
{
public:
    pimpl()
        : sail_codec_info_c(nullptr)
    {}

    const sail_codec_info *sail_codec_info_c;

    std::string version;
    std::string name;
    std::string description;
    std::vector<std::string> magic_numbers;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;
    sail::load_features load_features;
    sail::save_features save_features;
};

codec_info::codec_info()
    : d(new pimpl)
{
}

codec_info::codec_info(const codec_info &ci)
    : codec_info()
{
    *this = ci;
}

codec_info& codec_info::operator=(const codec_info &ci)
{
    d->sail_codec_info_c = ci.d->sail_codec_info_c;

    d->version       = ci.version();
    d->name          = ci.name();
    d->description   = ci.description();
    d->magic_numbers = ci.magic_numbers();
    d->extensions    = ci.extensions();
    d->mime_types    = ci.mime_types();
    d->load_features = ci.load_features();
    d->save_features = ci.save_features();

    return *this;
}

codec_info::codec_info(codec_info &&ci) noexcept
{
    *this = std::move(ci);
}

codec_info& codec_info::operator=(codec_info &&ci) noexcept
{
    d = std::move(ci.d);

    return *this;
}

codec_info::~codec_info()
{
}

bool codec_info::is_valid() const
{
    return d->sail_codec_info_c != nullptr && !d->name.empty() && !d->version.empty();
}

const std::string& codec_info::version() const
{
    return d->version;
}

const std::string& codec_info::name() const
{
    return d->name;
}

const std::string& codec_info::description() const
{
    return d->description;
}

const std::vector<std::string>& codec_info::magic_numbers() const
{
    return d->magic_numbers;
}

const std::vector<std::string>& codec_info::extensions() const
{
    return d->extensions;
}

const std::vector<std::string>& codec_info::mime_types() const
{
    return d->mime_types;
}

const load_features& codec_info::load_features() const
{
    return d->load_features;
}

const save_features& codec_info::save_features() const
{
    return d->save_features;
}

const char* codec_info::codec_feature_to_string(SailCodecFeature codec_feature)
{
    return sail_codec_feature_to_string(codec_feature);
}

SailCodecFeature codec_info::codec_feature_from_string(const std::string &str)
{
    return sail_codec_feature_from_string(str.c_str());
}

codec_info codec_info::from_magic_number(const std::string &path)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_by_magic_number_from_path(path.c_str(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_magic_number(const void *buffer, size_t buffer_length)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_by_magic_number_from_memory(buffer, buffer_length, &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_magic_number(sail::abstract_io &abstract_io)
{
    sail::abstract_io_adapter abstract_io_adapter(abstract_io);

    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_by_magic_number_from_io(&abstract_io_adapter.sail_io_c(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_path(const std::string &path)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_path(path.c_str(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_extension(const std::string &suffix)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_extension(suffix.c_str(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_mime_type(const std::string &mime_type)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_mime_type(mime_type.c_str(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

std::vector<codec_info> codec_info::list()
{
    std::vector<codec_info> codec_info_list;

    for (const sail_codec_bundle_node *codec_bundle_node = sail_codec_bundle_list(); codec_bundle_node != nullptr; codec_bundle_node = codec_bundle_node->next) {
        codec_info_list.push_back(codec_info(codec_bundle_node->codec_bundle->codec_info));
    }

    return codec_info_list;
}

codec_info::codec_info(const sail_codec_info *ci)
    : codec_info()
{
    if (ci == nullptr) {
        SAIL_LOG_TRACE("NULL pointer has been passed to sail::codec_info(). The object is untouched");
        return;
    }

    d->sail_codec_info_c = ci;

    std::vector<std::string> magic_numbers;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;

    // magic numbers
    for (const sail_string_node *magic_number_node = ci->magic_number_node; magic_number_node != nullptr; magic_number_node = magic_number_node->next) {
        magic_numbers.push_back(magic_number_node->string);
    }

    // extensions
    for (const sail_string_node *extension_node = ci->extension_node; extension_node != nullptr; extension_node = extension_node->next) {
        extensions.push_back(extension_node->string);
    }

    // mime types
    for (const sail_string_node *mime_type_node = ci->mime_type_node; mime_type_node != nullptr; mime_type_node = mime_type_node->next) {
        mime_types.push_back(mime_type_node->string);
    }

    d->version       = ci->version;
    d->name          = ci->name;
    d->description   = ci->description;
    d->magic_numbers = magic_numbers;
    d->extensions    = extensions;
    d->mime_types    = mime_types;
    d->load_features = sail::load_features(ci->load_features);
    d->save_features = sail::save_features(ci->save_features);
}

const sail_codec_info* codec_info::sail_codec_info_c() const
{
    return d->sail_codec_info_c;
}

}
