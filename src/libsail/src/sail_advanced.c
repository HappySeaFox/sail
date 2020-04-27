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

#include "sail-common.h"
#include "sail.h"

sail_error_t sail_start_reading_file(const char *path, struct sail_context *context, const struct sail_plugin_info *plugin_info, void **state) {

    SAIL_TRY(sail_start_reading_file_with_options(path, context, plugin_info, NULL, state));

    return 0;
}

sail_error_t sail_read_next_frame(void *state, struct sail_image **image, void **image_bits) {

    SAIL_CHECK_STATE_PTR(state);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_CHECK_IO(state_of_mind->io);
    SAIL_CHECK_STATE_PTR(state_of_mind->state);
    SAIL_CHECK_PLUGIN_PTR(state_of_mind->plugin);

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    SAIL_TRY(state_of_mind->plugin->v2->read_seek_next_frame_v2(state_of_mind->state, state_of_mind->io, image));

    *image_bits = malloc((*image)->bytes_per_line * (*image)->height);

    if (*image_bits == NULL) {
        return SAIL_MEMORY_ALLOCATION_FAILED;
    }

    for (int pass = 0; pass < (*image)->passes; pass++) {
        SAIL_TRY(state_of_mind->plugin->v2->read_seek_next_pass_v2(state_of_mind->state, state_of_mind->io, *image));

        for (int j = 0; j < (*image)->height; j++) {
            SAIL_TRY(state_of_mind->plugin->v2->read_scan_line_v2(state_of_mind->state,
                                                                    state_of_mind->io,
                                                                    *image,
                                                                    ((char *)*image_bits) + j * (*image)->bytes_per_line));
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

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        destroy_hidden_state(state_of_mind);
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
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

sail_error_t sail_write_next_frame(void *state, const struct sail_image *image, const void *image_bits) {

    SAIL_CHECK_STATE_PTR(state);

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    SAIL_CHECK_IO(state_of_mind->io);
    SAIL_CHECK_STATE_PTR(state_of_mind->state);
    SAIL_CHECK_PLUGIN_INFO_PTR(state_of_mind->plugin_info);
    SAIL_CHECK_PLUGIN_PTR(state_of_mind->plugin);

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    /* Detect the number of passes needed to write an interlaced image. */
    int passes;
    if (image->properties & SAIL_IMAGE_PROPERTY_INTERLACED) {
        passes = state_of_mind->plugin_info->write_features->passes;

        if (passes < 1) {
            return SAIL_INTERLACED_UNSUPPORTED;
        }
    } else {
        passes = 1;
    }

    int bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image, &bytes_per_line));

    SAIL_TRY(state_of_mind->plugin->v2->write_seek_next_frame_v2(state_of_mind->state, state_of_mind->io, image));

    for (int pass = 0; pass < passes; pass++) {
        SAIL_TRY(state_of_mind->plugin->v2->write_seek_next_pass_v2(state_of_mind->state, state_of_mind->io, image));

        for (int j = 0; j < image->height; j++) {
            SAIL_TRY(state_of_mind->plugin->v2->write_scan_line_v2(state_of_mind->state,
                                                                    state_of_mind->io,
                                                                    image,
                                                                    ((char *)image_bits) + j * bytes_per_line));
        }
    }

    return 0;
}

sail_error_t sail_stop_writing(void *state) {

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

    if (state_of_mind->plugin->layout != SAIL_PLUGIN_LAYOUT_V2) {
        destroy_hidden_state(state_of_mind);
        return SAIL_UNSUPPORTED_PLUGIN_LAYOUT;
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->write_finish_v2(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    destroy_hidden_state(state_of_mind);

    return 0;
}
