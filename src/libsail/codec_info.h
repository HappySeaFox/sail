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

#ifndef SAIL_CODEC_INFO_H
#define SAIL_CODEC_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

struct sail_io;
struct sail_read_features;
struct sail_write_features;

/*
 * A structure representing codec information.
 */
struct sail_codec_info {

    /* Full path to the codec. NULL when SAIL_COMBINE_CODECS is ON. */
    char *path;

    /*
     * The codec loader will use the codec's layout version to correctly handle the codec.
     * Unsupported codec layout versions will be reported. This field must be the very first key
     * in a codec information file.
     */
    int layout;

    /*
     * Codec priority from 0 to 3. SAIL sorts the enumerated codecs by priority.
     *
     * 0 = popular image format like JPEG or PNG
     * 1 = moderate popularity
     * 2 = rare image format
     * 3 = Very rare, ancient image format
     */
    int priority;

    /* Codec version. For example: "1.5.2". */
    char *version;

    /* Short codec name in upper case. For example: "JPEG". */
    char *name;

    /* Codec description. For example: "Joint Photographic Experts Group". */
    char *description;

    /*
     * A linked list of supported magic numbers. It can be NULL. For example: "FF D8" for JPEGs.
     * See https://en.wikipedia.org/wiki/File_format#Magic_number.
     */
    struct sail_string_node *magic_number_node;

    /* A linked list of supported file extensions. It can be NULL. For example: "jpg", "jpeg". */
    struct sail_string_node *extension_node;

    /* A linked list of supported mime types. It can be NULL. For example: "image/jpeg". */
    struct sail_string_node *mime_type_node;

    /* Read features of the codec. */
    struct sail_read_features *read_features;

    /* Write features of the codec. */
    struct sail_write_features *write_features;
};

typedef struct sail_codec_info sail_codec_info_t;

/*
 * Finds a first codec info object that supports reading or writing the specified file path by its file extension.
 * For example: "/test.jpg". The path might not exist.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_path() ->
 *                sail_start_reading_file()   ->
 *                sail_read_next_frame()      ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_path() ->
 *                sail_start_writing_file()   ->
 *                sail_read_next_frame()      ->
 *                sail_stop_writing().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_path(const char *path, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified file.
 * The comparison algorithm is case insensitive.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_path() ->
 *                sail_start_reading_file()                   ->
 *                sail_read_next_frame()                      ->
 *                sail_stop_reading().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_path(const char *path, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified memory buffer.
 * The comparison algorithm is case insensitive.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_mem() ->
 *                sail_start_reading_file()                  ->
 *                sail_read_next_frame()                     ->
 *                sail_stop_reading().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_mem(const void *buffer, size_t buffer_length,
                                                                   const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the magic number read from the specified I/O data source.
 * The comparison algorithm is case insensitive. After reading a magic number, this function rewinds the I/O
 * cursor position back to the beginning. That's why the I/O source must be seekable.
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_by_magic_number_from_io() ->
 *                sail_start_reading_file()                 ->
 *                sail_read_next_frame()                    ->
 *                sail_stop_reading().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_by_magic_number_from_io(struct sail_io *io, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the specified file extension.
 * The comparison algorithm is case insensitive. For example: "jpg".
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_extension() ->
 *                sail_start_reading_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_extension() ->
 *                sail_start_writing_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_writing().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_extension(const char *extension, const struct sail_codec_info **codec_info);

/*
 * Finds a first codec info object that supports the specified mime type.
 * The comparison algorithm is case insensitive. For example: "image/jpeg".
 *
 * The assigned codec info MUST NOT be destroyed. It is a pointer to an internal data structure.
 *
 * Typical usage: sail_codec_info_from_mime_type() ->
 *                sail_start_reading_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_reading().
 *
 * Or:            sail_codec_info_from_mime_type() ->
 *                sail_start_writing_file()        ->
 *                sail_read_next_frame()           ->
 *                sail_stop_writing().
 *
 * Returns SAIL_OK on success.
 */
SAIL_EXPORT sail_status_t sail_codec_info_from_mime_type(const char *mime_type, const struct sail_codec_info **codec_info);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
