/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

module sail.cpp;

import <string_view>;
import <string>;
import <vector>;

#include <sail/sail.h>

#include <sail-common/export.h>
#include <sail-common/status.h>

namespace sail
{

/*
 * codec_info represents image codec information.
 */
export class SAIL_EXPORT codec_info
{
    friend class image_input;
    friend class image_output;

public:
    /*
     * Constructs an invalid codec info object that can be later initialized with,
     * for example, from_magic_number().
     */
    codec_info();

    /*
     * Copies the codec info object.
     */
    codec_info(const codec_info &ci);

    /*
     * Copies the codec info object.
     */
    codec_info& operator=(const codec_info &ci);

    /*
     * Moves the codec info object.
     */
    codec_info(codec_info &&ci) noexcept;

    /*
     * Moves the codec info object.
     */
    codec_info& operator=(codec_info &&ci) noexcept;

    /*
     * Destroys the codec info object.
     */
    ~codec_info();

    /*
     * Returns true if the codec info object is valid.
     */
    bool is_valid() const;

    /*
     * Returns the codec version as a semver string. For example: "1.2.0".
     */
    const std::string& version() const;

    /*
     * Returns the short codec name in upper case. For example: "JPEG".
     */
    const std::string& name() const;

    /*
     * Returns the codec description. For example: "Joint Photographic Experts Group".
     */
    const std::string& description() const;

    /*
     * Returns the list of supported magic numbers. It can be empty. For example: "FF D8" for JPEGs.
     * See https://en.wikipedia.org/wiki/File_format#Magic_number.
     */
    const std::vector<std::string>& magic_numbers() const;

    /*
     * Returns the list of supported file extensions. It can be empty. For example: "jpg", "jpeg".
     */
    const std::vector<std::string>& extensions() const;

    /*
     * Returns the list of supported mime types. It can be empty. For example: "image/jpeg".
     */
    const std::vector<std::string>& mime_types() const;

    /*
     * Returns the load features of the codec.
     */
    const sail::load_features& load_features() const;

    /*
     * Returns the save features of the codec.
     */
    const sail::save_features& save_features() const;

    /*
     * Returns a string representation of the specified codec feature. See CodecFeature.
     * For example: "STATIC" is returned for CodecFeature::Static.
     *
     * Returns NULL if the codec feature is not known.
     */
    static const char* codec_feature_to_string(CodecFeature codec_feature);

    /*
     * Returns a codec feature from the string representation. See CodecFeature.
     * For example: CodecFeature::Static is returned for "STATIC".
     *
     * Returns CodecFeature::Unknown if the codec feature is not known.
     */
    static CodecFeature codec_feature_from_string(const std::string_view str);

    /*
     * Returns a first codec info object that supports the magic number read from the specified file.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::from_magic_number() ->
     *                image_input::start()            ->
     *                image_input::next_frame()       ->
     *                image_input::stop().
     */
    static codec_info from_magic_number(const std::string_view path);

    /*
     * Returns a first codec info object that supports the magic number read from the specified memory buffer.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::from_magic_number() ->
     *                image_input::start()            ->
     *                image_input::next_frame()       ->
     *                image_input::stop().
     */
    static codec_info from_magic_number(const void *buffer, size_t buffer_size);

    /*
     * Returns a first codec info object that supports the magic number read from the specified I/O source.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::from_magic_number() ->
     *                image_input::start()            ->
     *                image_input::next_frame()       ->
     *                image_input::stop().
     */
    static codec_info from_magic_number(sail::abstract_io &abstract_io);

    /*
     * Returns a first codec info object that supports loading or saving the specified file path by its file extension.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case insensitive. For example: "/test.jpg". The path might not exist.
     *
     * Typical usage: codec_info::from_path()   ->
     *                image_input::start()      ->
     *                image_input::next_frame() ->
     *                image_input::stop().
     *
     * Or:            codec_info::from_path()    ->
     *                image_output::start()      ->
     *                image_output::next_frame() ->
     *                image_output::stop().
     */
    static codec_info from_path(const std::string_view path);

    /*
     * Returns a first codec info object that supports the specified file extension.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case-insensitive. For example: "jpg".
     *
     * Typical usage: codec_info::from_extension() ->
     *                image_input::start()         ->
     *                image_input::next_frame()    ->
     *                image_input::stop().
     *
     * Or:            codec_info::from_extension() ->
     *                image_output::start()        ->
     *                image_output::next_frame()   ->
     *                image_output::stop().
     */
    static codec_info from_extension(const std::string_view suffix);

    /*
     * Returns a first codec info object that supports the specified mime type.
     * Returns an invalid codec info object if no suitable codec was found.
     * The comparison algorithm is case-insensitive. For example: "image/jpeg".
     *
     * Typical usage: codec_info::from_mime_type() ->
     *                image_input::start()         ->
     *                image_input::next_frame()    ->
     *                image_input::stop().
     *
     * Or:            codec_info::from_mime_type() ->
     *                image_output::start()        ->
     *                image_output::next_frame()   ->
     *                image_output::stop().
     */
    static codec_info from_mime_type(const std::string_view mime_type);

    /*
     * Returns the list of found codec info objects. Use it to determine the list of possible
     * image formats, file extensions, and mime types that could be hypothetically loaded or saved by SAIL.
     */
    static std::vector<codec_info> list();

private:
    /*
     * Makes a deep copy of the specified codec info and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit codec_info(const sail_codec_info *ci);

    const sail_codec_info* sail_codec_info_c() const;

private:
    const sail_codec_info *m_sail_codec_info_c{nullptr};

    std::string m_version;
    std::string m_name;
    std::string m_description;
    std::vector<std::string> m_magic_numbers;
    std::vector<std::string> m_extensions;
    std::vector<std::string> m_mime_types;
    sail::load_features m_load_features;
    sail::save_features m_save_features;
};

codec_info::codec_info()
{
}

codec_info::codec_info(const codec_info &ci)
    : codec_info()
{
    *this = ci;
}

codec_info& codec_info::operator=(const codec_info &ci)
{
    m_sail_codec_info_c = ci.m_sail_codec_info_c;

    m_version       = ci.version();
    m_name          = ci.name();
    m_description   = ci.description();
    m_magic_numbers = ci.magic_numbers();
    m_extensions    = ci.extensions();
    m_mime_types    = ci.mime_types();
    m_load_features = ci.load_features();
    m_save_features = ci.save_features();

    return *this;
}

codec_info::codec_info(codec_info &&ci) noexcept
{
    *this = std::move(ci);
}

codec_info& codec_info::operator=(codec_info &&ci) noexcept
{
    m_sail_codec_info_c = ci.m_sail_codec_info_c;
    ci.m_sail_codec_info_c = nullptr;

    m_version       = std::move(ci.version());
    m_name          = std::move(ci.name());
    m_description   = std::move(ci.description());
    m_magic_numbers = std::move(ci.magic_numbers());
    m_extensions    = std::move(ci.extensions());
    m_mime_types    = std::move(ci.mime_types());
    m_load_features = std::move(ci.load_features());
    m_save_features = std::move(ci.save_features());

    return *this;
}

codec_info::~codec_info()
{
}

bool codec_info::is_valid() const
{
    return m_sail_codec_info_c != nullptr && !m_name.empty() && !m_version.empty();
}

const std::string& codec_info::version() const
{
    return m_version;
}

const std::string& codec_info::name() const
{
    return m_name;
}

const std::string& codec_info::description() const
{
    return m_description;
}

const std::vector<std::string>& codec_info::magic_numbers() const
{
    return m_magic_numbers;
}

const std::vector<std::string>& codec_info::extensions() const
{
    return m_extensions;
}

const std::vector<std::string>& codec_info::mime_types() const
{
    return m_mime_types;
}

const load_features& codec_info::load_features() const
{
    return m_load_features;
}

const save_features& codec_info::save_features() const
{
    return m_save_features;
}

const char* codec_info::codec_feature_to_string(CodecFeature codec_feature)
{
    return sail_codec_feature_to_string(static_cast<SailCodecFeature>(codec_feature));
}

CodecFeature codec_info::codec_feature_from_string(const std::string_view str)
{
    return static_cast<CodecFeature>(sail_codec_feature_from_string(str.data()));
}

codec_info codec_info::from_magic_number(const std::string_view path)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_by_magic_number_from_path(path.data(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_magic_number(const void *buffer, size_t buffer_size)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_by_magic_number_from_memory(buffer, buffer_size, &sail_codec_info),
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

codec_info codec_info::from_path(const std::string_view path)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_path(path.data(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_extension(const std::string_view suffix)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_extension(suffix.data(), &sail_codec_info),
                        /* on error */ return codec_info{});

    return codec_info(sail_codec_info);
}

codec_info codec_info::from_mime_type(const std::string_view mime_type)
{
    const struct sail_codec_info *sail_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_mime_type(mime_type.data(), &sail_codec_info),
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

    m_sail_codec_info_c = ci;

    // magic numbers
    for (const sail_string_node *magic_number_node = ci->magic_number_node; magic_number_node != nullptr; magic_number_node = magic_number_node->next) {
        m_magic_numbers.push_back(magic_number_node->string);
    }

    // extensions
    for (const sail_string_node *extension_node = ci->extension_node; extension_node != nullptr; extension_node = extension_node->next) {
        m_extensions.push_back(extension_node->string);
    }

    // mime types
    for (const sail_string_node *mime_type_node = ci->mime_type_node; mime_type_node != nullptr; mime_type_node = mime_type_node->next) {
        m_mime_types.push_back(mime_type_node->string);
    }

    m_version       = ci->version;
    m_name          = ci->name;
    m_description   = ci->description;
    m_load_features = sail::load_features(ci->load_features);
    m_save_features = sail::save_features(ci->save_features);
}

const sail_codec_info* codec_info::sail_codec_info_c() const
{
    return m_sail_codec_info_c;
}

}
