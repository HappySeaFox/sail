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

#ifndef SAIL_CONTEXT_CPP_H
#define SAIL_CONTEXT_CPP_H

#include <string>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_context;

namespace sail
{

class plugin_info;
class io;

/*
 * Context is a main entry point to start working with SAIL. It enumerates plugin info objects which could be
 * used later in reading and writing operations.
 */
class SAIL_EXPORT context
{
    friend class image_reader;
    friend class image_writer;

public:
    /*
     * Initializes SAIL with default flags. This is a main entry point to start working with SAIL.
     * Builds a list of available SAIL plugins.
     *
     * Plugins (image codecs) paths search algorithm (first found path wins):
     *
     *   Windows:
     *     1. SAIL_PLUGINS_PATH environment variable
     *     2. <SAIL DEPLOYMENT FOLDER>\lib\sail\plugins
     *     3. Hardcoded SAIL_PLUGINS_PATH in config.h
     *
     *   Unix (including MacOS):
     *     1. SAIL_PLUGINS_PATH environment variable
     *     2. Hardcoded SAIL_PLUGINS_PATH in config.h
     *
     * See also status().
     */
    context();

    /*
     * Initializes SAIL with the specific flags. This is an alternative entry point to start working with SAIL.
     * Builds a list of available SAIL plugins. See SailInitFlags.
     *
     * Plugins (image codecs) paths search algorithm (first found path wins):
     *
     *   Windows:
     *     1. SAIL_PLUGINS_PATH environment variable
     *     2. <SAIL DEPLOYMENT FOLDER>\lib\sail\plugins
     *     3. Hardcoded SAIL_PLUGINS_PATH in config.h
     *
     *   Unix (including MacOS):
     *     1. SAIL_PLUGINS_PATH environment variable
     *     2. Hardcoded SAIL_PLUGINS_PATH in config.h
     *
     * See also status().
     */
    context(int flags);

    /*
     * When context gets destroyed, all plugin info objects, read and write features get invalidated.
     * Using them when the context doesn't exist anymore will lead to a crash.
     */
    ~context();

    /*
     * Returns a context initialization status. Using SAIL when this function returns a non-zero status
     * has no sense as most methods will return errors.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t status() const;

    /*
     * Returns a list of found plugin info objects. Use it to determine the list of possible
     * image formats, file extensions, and mime types that could be hypothetically read or written by SAIL.
     */
    const std::vector<plugin_info>& plugin_info_list() const;

    /*
     * Unloads all loaded plugins (codecs) to free some memory. Plugin info objects attached
     * to the context remain untouched.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t unload_plugins();

    /*
     * Finds a first plugin info object that supports the magic number read from the specified file.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: context::plugin_info_by_magic_number_from_path() ->
     *                image_reader::start_reading_file()               ->
     *                image_reader::read_next_frame()                  ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_by_magic_number_from_path(const std::string &path, plugin_info *splugin_info) const;
    sail_error_t plugin_info_by_magic_number_from_path(const char *path, plugin_info *splugin_info) const;

    /*
     * Finds a first plugin info object that supports the magic number read from the specified memory buffer.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: context::plugin_info_by_magic_number_from_mem() ->
     *                image_reader::start_reading_file()              ->
     *                image_reader::read_next_frame()                 ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_by_magic_number_from_mem(const void *buffer, size_t buffer_length, plugin_info *splugin_info) const;

    /*
     * Finds a first plugin info object that supports the magic number read from the specified I/O source.
     * The comparison algorithm is case insensitive.
     *
     * Typical usage: context::plugin_info_by_magic_number_from_io() ->
     *                image_reader::start_reading_file()             ->
     *                image_reader::read_next_frame()                ->
     *                image_reader::stop_reading().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_by_magic_number_from_io(const sail::io &io, plugin_info *splugin_info) const;

    /*
     * Finds a first plugin info object that supports reading or writing the specified file path by its file extension.
     * The comparison algorithm is case-insensitive. For example: "/test.jpg". The path might not exist.
     *
     * Typical usage: context::plugin_info_from_path()   ->
     *                image_reader::start_reading_file() ->
     *                image_reader::read_next_frame()    ->
     *                image_reader::stop_reading().
     *
     * Or:            context::plugin_info_from_path() ->
     *                image_writer::start_writing()    ->
     *                image_writer::read_next_frame()  ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_from_path(const std::string &path, plugin_info *splugin_info) const;
    sail_error_t plugin_info_from_path(const char *path, plugin_info *splugin_info) const;

    /*
     * Finds a first plugin info object that supports the specified file extension. The comparison
     * algorithm is case-insensitive. For example: "jpg".
     *
     * Typical usage: context::plugin_info_from_extension() ->
     *                image_reader::start_reading_file()    ->
     *                image_reader::read_next_frame()       ->
     *                image_reader::stop_reading().
     *
     * Or:            context::plugin_info_from_extension() ->
     *                image_writer::start_writing()         ->
     *                image_writer::read_next_frame()       ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_from_extension(const std::string &suffix, plugin_info *splugin_info) const;
    sail_error_t plugin_info_from_extension(const char *suffix, plugin_info *splugin_info) const;

    /*
     * Finds a first plugin info object that supports the specified mime type. The comparison
     * algorithm is case-insensitive. For example: "image/jpeg".
     *
     * Typical usage: context::plugin_info_from_mime_type() ->
     *                image_reader::start_reading_file()    ->
     *                image_reader::read_next_frame()       ->
     *                image_reader::stop_reading().
     *
     * Or:            context::plugin_info_from_mime_type() ->
     *                image_writer::start_writing()         ->
     *                image_writer::read_next_frame()       ->
     *                image_writer::stop_writing().
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t plugin_info_from_mime_type(const std::string &mime_type, plugin_info *splugin_info) const;
    sail_error_t plugin_info_from_mime_type(const char *mime_type, plugin_info *splugin_info) const;

private:
    sail_error_t init(int flags);

    sail_context* sail_context_c() const;

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
