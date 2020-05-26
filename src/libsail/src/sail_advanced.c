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

    SAIL_TRY(state_of_mind->plugin->v2->read_seek_next_frame_v2(state_of_mind->state, state_of_mind->io, image));

    /* Detect the number of passes needed to write an interlaced image. */
    int interlaced_passes;
    if ((*image)->source_properties & SAIL_IMAGE_PROPERTY_INTERLACED) {
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
        SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_seek_next_pass_v2(state_of_mind->state, state_of_mind->io, *image),
                            /* cleanup */ free(*image_bits),
                                          sail_destroy_image(*image));

        for (unsigned j = 0; j < (*image)->height; j++) {
            SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_scan_line_v2(state_of_mind->state,
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

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->read_finish_v2(&state_of_mind->state, state_of_mind->io),
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

    SAIL_TRY(state_of_mind->plugin->v2->write_seek_next_frame_v2(state_of_mind->state, state_of_mind->io, image));

    for (int pass = 0; pass < interlaced_passes; pass++) {
        SAIL_TRY(state_of_mind->plugin->v2->write_seek_next_pass_v2(state_of_mind->state, state_of_mind->io, image));

        for (unsigned j = 0; j < image->height; j++) {
            SAIL_TRY(state_of_mind->plugin->v2->write_scan_line_v2(state_of_mind->state,
                                                                    state_of_mind->io,
                                                                    image,
                                                                    ((char *)image_bits) + j * bytes_per_line));
        }
    }

    return 0;
}

sail_error_t sail_stop_writing(void *state) {

    SAIL_TRY(stop_writing(state, NULL));

    return 0;
}
