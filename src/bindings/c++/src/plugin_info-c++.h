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

    /*
     * Finds a first plugin info object that supports the magic number read from the specified file.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: plugin_info::by_magic_number_from_path() ->
     *                image_reader::start_reading_file(        ->
     *                image_reader::read_next_frame()          ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t by_magic_number_from_path(const std::string &path, plugin_info *splugin_info);
    static sail_error_t by_magic_number_from_path(const char *path, plugin_info *splugin_info);

    /*
     * Finds a first plugin info object that supports the magic number read from the specified memory buffer.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: plugin_info::by_magic_number_from_mem() ->
     *                image_reader::start_reading_file()      ->
     *                image_reader::read_next_frame()         ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t by_magic_number_from_mem(const void *buffer, size_t buffer_length, plugin_info *splugin_info);

    /*
     * Finds a first plugin info object that supports the magic number read from the specified I/O source.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: plugin_info::by_magic_number_from_io() ->
     *                image_reader::start_reading_file()     ->
     *                image_reader::read_next_frame()        ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t by_magic_number_from_io(const sail::io &io, plugin_info *splugin_info);

    /*
     * Finds a first plugin info object that supports reading or writing the specified file path by its file extension.
     * The comparison algorithm is case-insensitive. For example: "/test.jpg". The path might not exist.
     *
     * Typical usage: plugin_info::from_path()           ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            plugin_info::from_path()        ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t from_path(const std::string &path, plugin_info *splugin_info);
    static sail_error_t from_path(const char *path, plugin_info *splugin_info);

    /*
     * Finds a first plugin info object that supports the specified file extension. The comparison
     * algorithm is case-insensitive. For example: "jpg".
     *
     * Typical usage: plugin_info::from_extension()      ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            plugin_info::from_extension()   ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t from_extension(const std::string &suffix, plugin_info *splugin_info);
    static sail_error_t from_extension(const char *suffix, plugin_info *splugin_info);

    /*
     * Finds a first plugin info object that supports the specified mime type. The comparison
     * algorithm is case-insensitive. For example: "image/jpeg".
     *
     * Typical usage: plugin_info::from_mime_type()      ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            plugin_info::from_mime_type()   ->
     *                image_writer::start_writing()   ->
     *                image_writer::read_next_frame() ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    static sail_error_t from_mime_type(const std::string &mime_type, plugin_info *splugin_info);
    static sail_error_t from_mime_type(const char *mime_type, plugin_info *splugin_info);

    /*
     * Returns a list of found plugin info objects. Use it to determine the list of possible
     * image formats, file extensions, and mime types that could be hypothetically read or written by SAIL.
     */
    static std::vector<plugin_info> list();

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
