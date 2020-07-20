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

sail_error_t load_plugin(struct sail_plugin_info_node *node) {

    SAIL_CHECK_PTR(node);

    /* Already loaded. */
    if (node->plugin != NULL) {
        return 0;
    }

    /* Plugin is not loaded. Let's load it. */
    SAIL_TRY(alloc_plugin(node->plugin_info, &node->plugin));

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
    sail_free(state->state);

    sail_free(state);
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

    SAIL_TRY_OR_CLEANUP(state_of_mind->plugin->v3->write_finish(&state_of_mind->state, state_of_mind->io),
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

    /* Plugins will compute output pixel format automatically. */
    if (output_pixel_format == SAIL_PIXEL_FORMAT_AUTO) {
        return 0;
    }

    /*
     * For example:
     *
     * [write-pixel-formats-mapping]
     * BPP8-GRAYSCALE=SOURCE
     * BPP24-RGB=SOURCE;BPP24-YCBCR;BPP8-GRAYSCALE
     *
     * When input_pixel_format is BPP24-RGB and output_pixel_format is BPP24-YCBCR, success is returned.
     * When input_pixel_format is BPP24-RGB and output_pixel_format is BPP32-CMYK, error is returned.
     */
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

sail_error_t possibly_allocate_context(struct sail_context *context, struct sail_context **context_result) {

    SAIL_CHECK_CONTEXT_PTR(context_result);

    SAIL_THREAD_LOCAL static struct sail_context *tls_context = NULL;
    SAIL_THREAD_LOCAL static bool tls_context_initialized = false;

    if (context == NULL) {
        if (!tls_context_initialized) {
            SAIL_LOG_DEBUG("Initializing a thread-local static context");
            SAIL_TRY(sail_init(&tls_context));
            tls_context_initialized = true;
        }

        *context_result = tls_context;
    } else {
        *context_result = context;
    }

    return 0;
}
