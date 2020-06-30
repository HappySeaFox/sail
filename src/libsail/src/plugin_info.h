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

#ifndef SAIL_PLUGIN_INFO_H
#define SAIL_PLUGIN_INFO_H

struct sail_string_node;

struct sail_read_features;
struct sail_write_features;

struct sail_plugin;

/*
 * A structure representing plugin information.
 */
struct sail_plugin_info {

    /* Full path to the plugin. */
    char *path;

    /*
     * The plugin loader will use the plugin's layout version to correctly handle the plugin.
     * Unsupported plugin layout versions will be reported. This field must be the very first key
     * in a plugin information file.
     */
    int layout;

    /* Plugin version. For example: "1.5.2". */
    char *version;

    /* Short plugin name in upper case. For example: "JPEG". */
    char *name;

    /* Plugin description. For example: "Joint Photographic Experts Group". */
    char *description;

    /*
     * A linked list of supported magic numbers. For example: "FF D8" for JPEGs.
     * See https://en.wikipedia.org/wiki/File_format#Magic_number.
     */
    struct sail_string_node *magic_number_node;

    /* A linked list of supported file extensions. For example: "jpg", "jpeg". */
    struct sail_string_node *extension_node;

    /* A linked list of supported mime types. For example: "image/jpeg". */
    struct sail_string_node *mime_type_node;

    /* Read features of the plugin. */
    struct sail_read_features *read_features;

    /* Write features of the plugin. */
    struct sail_write_features *write_features;
};

typedef struct sail_plugin_info sail_plugin_info_t;

#endif
