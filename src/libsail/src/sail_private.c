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

sail_error_t load_plugin(struct sail_plugin_info_node *node) {

    SAIL_CHECK_PTR(node);

    /* Already loaded. */
    if (node->plugin != NULL) {
        return 0;
    }

    struct sail_plugin *plugin;

    /* Plugin is not loaded. Let's load it. */
    SAIL_TRY(alloc_plugin(node->plugin_info, &plugin));

    node->plugin = plugin;

    return 0;
}

sail_error_t load_plugin_by_plugin_info(struct sail_context *context,
                                        const struct sail_plugin_info *plugin_info,
                                        const struct sail_plugin **plugin) {

    SAIL_CHECK_PLUGIN_INFO_PTR(plugin_info);
    SAIL_CHECK_PLUGIN_PTR(plugin);

    /* Find the plugin in the cache. */
    struct sail_plugin_info_node *node = context->plugin_info_node;
    struct sail_plugin_info_node *found_node = NULL;

    while (node != NULL) {
        if (node->plugin_info == plugin_info) {
            if (node->plugin != NULL) {
                *plugin = node->plugin;
                return 0;
            }

            found_node = node;
            break;
        }

        node = node->next;
    }

    /* Something weird. The pointer to the plugin info is not found the cache. */
    if (found_node == NULL) {
        return SAIL_PLUGIN_NOT_FOUND;
    }

    SAIL_TRY(load_plugin(found_node));

    *plugin = found_node->plugin;

    return 0;
}

void destroy_hidden_state(struct hidden_state *state) {

    if (state == NULL) {
        return;
    }

    if (state->own_io) {
        sail_destroy_io(state->io);
    }

    sail_destroy_write_options(state->write_options);

    /* This state must be freed and zeroed by plugins. We free it just in case to avoid memory leaks. */
    free(state->state);

    free(state);
}

sail_error_t stop_writing(void *state, size_t *written) {

    if (written != NULL) {
        *written = 0;
    }

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

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v2->write_finish_v2(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (written != NULL) {
        state_of_mind->io->tell(state_of_mind->io->stream, written);
    }

    destroy_hidden_state(state_of_mind);

    return 0;
}

sail_error_t allowed_read_output_pixel_format(const struct sail_read_features *read_features,
                                                enum SailPixelFormat pixel_format) {

    SAIL_CHECK_READ_FEATURES_PTR(read_features);

    for (int i = 0; i < read_features->output_pixel_formats_length; i++) {
        if (read_features->output_pixel_formats[i] == pixel_format) {
            return 0;
        }
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}

sail_error_t allowed_write_output_pixel_format(const struct sail_write_features *write_features,
                                                enum SailPixelFormat input_pixel_format,
                                                enum SailPixelFormat output_pixel_format) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);

    const struct sail_pixel_formats_mapping_node *node = write_features->pixel_formats_mapping_node;

    while (node != NULL) {
        if (node->input_pixel_format == input_pixel_format) {
            for (int i = 0; i < node->output_pixel_formats_length; i++) {
                if (node->output_pixel_formats[i] == output_pixel_format) {
                    return 0;
                }
            }

            return SAIL_UNSUPPORTED_PIXEL_FORMAT;
        }

        node = node->next;
    }

    return SAIL_UNSUPPORTED_PIXEL_FORMAT;
}
