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

#ifndef SAIL_CODEC_INFO_CPP_H
#define SAIL_CODEC_INFO_CPP_H

#include <memory>
#include <string>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_codec_info;

namespace sail
{

class abstract_io;
class load_features;
class save_features;

/*
 * codec_info represents codec information.
 */
class SAIL_EXPORT codec_info
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
     * Returns a string representation of the specified codec feature. See SailCodecFeature.
     * For example: "STATIC" is returned for SAIL_CODEC_FEATURE_STATIC.
     *
     * Returns NULL if the codec feature is not known.
     */
    static const char* codec_feature_to_string(SailCodecFeature codec_feature);

    /*
     * Returns a codec feature from the string representation. See SailCodecFeature.
     * For example: SAIL_CODEC_FEATURE_STATIC is returned for "STATIC".
     *
     * Returns SAIL_CODEC_FEATURE_UNKNOWN if the codec feature is not known.
     */
    static SailCodecFeature codec_feature_from_string(const std::string &str);

    /*
     * Returns a first codec info object that supports the magic number read from the specified file.
     * Returns an invalid codec info object if no suitable codecs were found.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::from_magic_number() ->
     *                image_input::start()            ->
     *                image_input::next_frame()       ->
     *                image_input::stop().
     */
    static codec_info from_magic_number(const std::string &path);

    /*
     * Returns a first codec info object that supports the magic number read from the specified memory buffer.
     * Returns an invalid codec info object if no suitable codecs were found.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::from_magic_number() ->
     *                image_input::start()            ->
     *                image_input::next_frame()       ->
     *                image_input::stop().
     */
    static codec_info from_magic_number(const void *buffer, size_t buffer_length);

    /*
     * Returns a first codec info object that supports the magic number read from the specified I/O source.
     * Returns an invalid codec info object if no suitable codecs were found.
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
     * Returns an invalid codec info object if no suitable codecs were found.
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
    static codec_info from_path(const std::string &path);

    /*
     * Returns a first codec info object that supports the specified file extension.
     * Returns an invalid codec info object if no suitable codecs were found.
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
    static codec_info from_extension(const std::string &suffix);

    /*
     * Returns a first codec info object that supports the specified mime type.
     * Returns an invalid codec info object if no suitable codecs were found.
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
    static codec_info from_mime_type(const std::string &mime_type);

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
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
