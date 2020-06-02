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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

sail_error_t sail_probe(const char *path, struct sail_context *context, struct sail_image **image, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_noop;
    const struct sail_plugin_info **plugin_info_local = plugin_info == NULL ? &plugin_info_noop : plugin_info;

    SAIL_TRY(sail_plugin_info_from_path(path, context, plugin_info_local));

    const struct sail_plugin *plugin;
    SAIL_TRY(load_plugin_by_plugin_info(context, *plugin_info_local, &plugin));

    struct sail_io *io;
    SAIL_TRY(alloc_io_read_file(path, &io));

    struct sail_read_options *read_options_local = NULL;
    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features((*plugin_info_local)->read_features, &read_options_local),
                        /* cleanup */ sail_destroy_read_options(read_options_local),
                                      sail_destroy_io(io));

    /* Leave the pixel data as is. */
    read_options_local->output_pixel_format = SAIL_PIXEL_FORMAT_SOURCE;

    SAIL_TRY_OR_CLEANUP(plugin->v2->read_init_v2(io, read_options_local, &state),
                        /* cleanup */ plugin->v2->read_finish_v2(&state, io),
                                      sail_destroy_read_options(read_options_local),
                                      sail_destroy_io(io));

    sail_destroy_read_options(read_options_local);

    SAIL_TRY_OR_CLEANUP(plugin->v2->read_seek_next_frame_v2(state, io, image),
                        /* cleanup */ plugin->v2->read_finish_v2(&state, io),
                                      sail_destroy_io(io));
    SAIL_TRY_OR_CLEANUP(plugin->v2->read_finish_v2(&state, io),
                        /* cleanup */ sail_destroy_io(io));

    sail_destroy_io(io);

    return 0;
}

sail_error_t sail_read(const char *path, struct sail_context *context, struct sail_image **image, void **image_bits) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_IMAGE_PTR(image);
    SAIL_CHECK_PTR(image_bits);

    void *state = NULL;
    *image_bits = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path, context, NULL /* plugin info */, &state),
                        /* cleanup */ sail_stop_reading(state));

    SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, image, image_bits),
                        /* cleanup */ sail_stop_reading(state));

    SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                        /* cleanup */ free(image_bits),
                                      sail_destroy_image(*image));

    return 0;
}

sail_error_t sail_write(const char *path, struct sail_context *context, const struct sail_image *image, const void *image_bits) {

    SAIL_CHECK_PATH_PTR(path);
    SAIL_CHECK_CONTEXT_PTR(context);
    SAIL_CHECK_IMAGE(image);
    SAIL_CHECK_PTR(image_bits);

    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_start_writing_file(path, context, NULL /* plugin info */, &state),
                        sail_stop_writing(state));

    SAIL_TRY_OR_CLEANUP(sail_write_next_frame(state, image, image_bits),
                        sail_stop_writing(state));

    SAIL_TRY(sail_stop_writing(state));

    return 0;
}
