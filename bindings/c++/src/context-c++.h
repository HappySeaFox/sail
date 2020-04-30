/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_CONTEXT_CPP_H
#define SAIL_CONTEXT_CPP_H

#include <string>
#include <vector>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

struct sail_context;

namespace sail
{

class plugin_info;

/*
 * Context is a main entry point to start working with SAIL. It enumerates plugin info objects which could be
 * used later in reading and writing methods.
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
     * You could point SAIL to search plugins in a different location by setting SAIL_PLUGINS_PATH
     * environment variable to a path with SAIL plugins.
     */
    context();

    /*
     * Initializes SAIL with the specific flags. This is an alternative entry point to start working with SAIL.
     * Builds a list of available SAIL plugins. See SailInitFlags.
     *
     * You could point SAIL to search plugins in a different location by setting SAIL_PLUGINS_PATH
     * environment variable to a path with SAIL plugins.
     */
    context(int flags);

    /*
     * When context gets destroyed, all plugin info objects, read and write features get invalidated.
     * Using them when the context doesn't exist anymore may lead to a crash.
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
    std::vector<plugin_info> plugin_info_list() const;

    /*
     * Unloads all loaded plugins (codecs) to free some memory. Plugin info objects attached
     * to the context remain untouched.
     *
     * Returns 0 on success or sail_error_t on error.
     */
    sail_error_t unload_plugins();

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
