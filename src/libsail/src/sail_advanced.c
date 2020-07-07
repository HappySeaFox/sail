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

#include "config.h"

#include <stdlib.h>

#include "sail-common.h"
#include "sail.h"

sail_error_t sail_probe_io(struct sail_io *io, struct sail_context *context, struct sail_image **image, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_IO_PTR(io);
    SAIL_CHECK_CONTEXT_PTR(context);

    const struct sail_plugin_info *plugin_info_noop;
    const struct sail_plugin_info **plugin_info_local = plugin_info == NULL ? &plugin_info_noop : plugin_info;

    SAIL_TRY(sail_plugin_info_by_magic_number_from_io(io, context, plugin_info_local));

    const struct sail_plugin *plugin;
    SAIL_TRY(load_plugin_by_plugin_info(context, *plugin_info_local, &plugin));

    struct sail_read_options *read_options_local = NULL;
    void *state = NULL;

    SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features((*plugin_info_local)->read_features, &read_options_local),
                        /* cleanup */ sail_destroy_read_options(read_options_local));

    SAIL_TRY_OR_CLEANUP(plugin->v3->read_init(io, read_options_local, &state),
                        /* cleanup */ plugin->v3->read_finish(&state, io),
                                      sail_destroy_read_options(read_options_local));

    sail_destroy_read_options(read_options_local);

    SAIL_TRY_OR_CLEANUP(plugin->v3->read_seek_next_frame(state, io, image),
                        /* cleanup */ plugin->v3->read_finish(&state, io));
    SAIL_TRY(plugin->v3->read_finish(&state, io));

    return 0;
}

sail_error_t sail_probe_mem(const void *buffer, size_t buffer_length, struct sail_context *context, struct sail_image **image, const struct sail_plugin_info **plugin_info) {

    SAIL_CHECK_BUFFER_PTR(buffer);
    SAIL_CHECK_CONTEXT_PTR(context);

    struct sail_io *io;
    SAIL_TRY(alloc_io_read_mem(buffer, buffer_length, &io));

    SAIL_TRY_OR_CLEANUP(sail_probe_io(io, context, image, plugin_info),
                        /* cleanup */ sail_destroy_io(io));

    sail_destroy_io(io);

    return 0;
}

sail_error_t sail_start_reading_file(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_file_with_options(path, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_reading_mem(const void *buffer, size_t buffer_length, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_mem_with_options(buffer, buffer_length, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_read_next_frame(void *state, struct sail_image **image, void **image_bits) {

    SAIL_CHECK_STATE_PTR(state);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_CHECK_IO(state_of_mind->io);
    SAIL_CHECK_STATE_PTR(state_of_mind->state);
    SAIL_CHECK_PLUGIN_PTR(state_of_mind->plugin);

    SAIL_TRY(state_of_mind->plugin->v3->read_seek_next_frame(state_of_mind->state, state_of_mind->io, image));

    /* Detect the number of passes needed to write an interlaced image. */
    int interlaced_passes;
    if ((*image)->source_image->properties & SAIL_IMAGE_PROPERTY_INTERLACED) {
        interlaced_passes = (*image)->interlaced_passes;

        if (interlaced_passes < 1) {
            sail_destroy_image(*image);
            return SAIL_INTERLACED_UNSUPPORTED;
        }
    } else {
        interlaced_passes = 1;
    }

    /* Allocate pixel data. */
    *image_bits = malloc((*image)->bytes_per_line * (*image)->height);

    if (*image_bits == NULL) {
        sail_destroy_image(*image);
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    for (int pass = 0; pass < interlaced_passes; pass++) {
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->read_seek_next_pass(state_of_mind->state, state_of_mind->io, *image),
                            /* cleanup */ free(*image_bits),
                                          sail_destroy_image(*image));

        for (unsigned j = 0; j < (*image)->height; j++) {
            SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->read_scan_line(state_of_mind->state,
                                                                            state_of_mind->io,
                                                                            *image,
                                                                            ((char *)*image_bits) + j * (*image)->bytes_per_line),
                                /* cleanup */ free(*image_bits),
                                              sail_destroy_image(*image));
        }
    }

    return 0;
}

sail_error_t sail_stop_reading(void *state) {

    /* Not an error. */
    if (state == NULL) {
        return 0;
    }

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    /* Not an error. */
    if (state_of_mind->plugin == NULL) {
        destroy_hidden_state(state_of_mind);
        return 0;
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->read_finish(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    destroy_hidden_state(state_of_mind);

    return 0;
}

sail_error_t sail_start_writing_file(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_writing_file_with_options(path, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_start_writing_mem(void *buffer, size_t buffer_length, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_writing_mem_with_options(buffer, buffer_length, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_write_next_frame(void *state, const struct sail_image *image, const void *image_bits) {

    SAIL_CHECK_STATE_PTR(state);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_CHECK_IO(state_of_mind->io);
    SAIL_CHECK_STATE_PTR(state_of_mind->state);
    SAIL_CHECK_PLUGIN_INFO_PTR(state_of_mind->plugin_info);
    SAIL_CHECK_PLUGIN_PTR(state_of_mind->plugin);

    /* Check if we actually able to write the requested pixel format. */
    SAIL_TRY(allowed_write_output_pixel_format(state_of_mind->plugin_info->write_features,
                                                image->pixel_format,
                                                state_of_mind->write_options->output_pixel_format));

    /* Detect the number of passes needed to write an interlaced image. */
    int interlaced_passes;
    if (state_of_mind->write_options->io_options & SAIL_IO_OPTION_INTERLACED) {
        interlaced_passes = state_of_mind->plugin_info->write_features->interlaced_passes;

        if (interlaced_passes < 1) {
            return SAIL_INTERLACED_UNSUPPORTED;
        }
    } else {
        interlaced_passes = 1;
    }

    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

    SAIL_TRY(state_of_mind->plugin->v3->write_seek_next_frame(state_of_mind->state, state_of_mind->io, image));

    for (int pass = 0; pass < interlaced_passes; pass++) {
        SAIL_TRY(state_of_mind->plugin->v3->write_seek_next_pass(state_of_mind->state, state_of_mind->io, image));

        for (unsigned j = 0; j < image->height; j++) {
            SAIL_TRY(state_of_mind->plugin->v3->write_scan_line(state_of_mind->state,
                                                                state_of_mind->io,
                                                                image,
                                                                ((const char *)image_bits) + j * bytes_per_line));
        }
    }

    return 0;
}

sail_error_t sail_stop_writing(void *state) {

    SAIL_TRY(stop_writing(state, NULL));

    return 0;
}
