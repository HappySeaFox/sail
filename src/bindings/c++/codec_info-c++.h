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

#include <string_view>
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

struct sail_codec_info;

namespace sail
{

/*
 * A C++ interface to struct sail_codec_info.
 */
class SAIL_EXPORT codec_info
{
    friend class image_reader;
    friend class image_writer;

public:
    codec_info();
    codec_info(const codec_info &ci);
    codec_info& operator=(const codec_info &ci);
    codec_info(codec_info &&ci) noexcept;
    codec_info& operator=(codec_info &&ci);
    ~codec_info();

    const std::string& version() const;
    const std::string& name() const;
    const std::string& description() const;
    const std::vector<std::string>& magic_numbers() const;
    const std::vector<std::string>& extensions() const;
    const std::vector<std::string>& mime_types() const;
    const sail::read_features& read_features() const;
    const sail::write_features& write_features() const;

    static sail_status_t codec_feature_to_string(SailCodecFeature codec_feature, const char **result);
    static sail_status_t codec_feature_from_string(std::string_view str, SailCodecFeature *result);

    /*
     * Finds a first codec info object that supports the magic number read from the specified file.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::by_magic_number_from_path() ->
     *                image_reader::start_reading_file()      ->
     *                image_reader::read_next_frame()         ->
     *                image_reader::stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_magic_number(std::string_view path, codec_info *scodec_info);

    /*
     * Finds a first codec info object that supports the magic number read from the specified memory buffer.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::by_magic_number_from_mem() ->
     *                image_reader::start_reading_file()     ->
     *                image_reader::read_next_frame()        ->
     *                image_reader::stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_magic_number(const void *buffer, size_t buffer_length, codec_info *scodec_info);

    /*
     * Finds a first codec info object that supports the magic number read from the specified I/O source.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: codec_info::by_magic_number_from_io() ->
     *                image_reader::start_reading_file()    ->
     *                image_reader::read_next_frame()       ->
     *                image_reader::stop_reading().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_magic_number(const sail::io &io, codec_info *scodec_info);

    /*
     * Finds a first codec info object that supports reading or writing the specified file path by its file extension.
     * The comparison algorithm is case-insensitive. For example: "/test.jpg". The path might not exist.
     *
     * Typical usage: codec_info::from_path()            ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            codec_info::from_path()         ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_path(std::string_view path, codec_info *scodec_info);

    /*
     * Finds a first codec info object that supports the specified file extension. The comparison
     * algorithm is case-insensitive. For example: "jpg".
     *
     * Typical usage: codec_info::from_extension()       ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            codec_info::from_extension()    ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_extension(std::string_view suffix, codec_info *scodec_info);

    /*
     * Finds a first codec info object that supports the specified mime type. The comparison
     * algorithm is case-insensitive. For example: "image/jpeg".
     *
     * Typical usage: codec_info::from_mime_type()       ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            codec_info::from_mime_type()    ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns SAIL_OK on success.
     */
    static sail_status_t from_mime_type(std::string_view mime_type, codec_info *scodec_info);

    /*
     * Returns a list of found codec info objects. Use it to determine the list of possible
     * image formats, file extensions, and mime types that could be hypothetically read or written by SAIL.
     */
    static std::vector<codec_info> list();

private:
    /*
     * Makes a deep copy of the specified codec info and stores the pointer for further use.
     * When the SAIL context gets uninitialized, the pointer becomes dangling.
     */
    explicit codec_info(const sail_codec_info *ci);

    codec_info& with_version(const std::string &version);
    codec_info& with_name(const std::string &name);
    codec_info& with_description(const std::string &description);
    codec_info& with_magic_numbers(const std::vector<std::string> &magic_numbers);
    codec_info& with_extensions(const std::vector<std::string> &extensions);
    codec_info& with_mime_types(const std::vector<std::string> &mime_types);
    codec_info& with_read_features(const sail::read_features &read_features);
    codec_info& with_write_features(const sail::write_features &write_features);

    const sail_codec_info* sail_codec_info_c() const;

private:
    class pimpl;
    pimpl *d;
};

}

#endif
