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

#include <string.h>

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static void print_unsupported_write_pixel_format(enum SailPixelFormat pixel_format) {

    SAIL_LOG_ERROR("This codec cannot write %s pixels. Use its write features to get the list of supported pixel formats for writing",
                    sail_pixel_format_to_string(pixel_format));
}

/*
 * Public functions.
 */

sail_status_t load_codec_by_codec_info(const struct sail_codec_info *codec_info, const struct sail_codec **codec) {

    SAIL_CHECK_CODEC_INFO_PTR(codec_info);
    SAIL_CHECK_CODEC_PTR(codec);

    SAIL_TRY(lock_context());

    struct sail_context *context;
    SAIL_TRY_OR_CLEANUP(current_tls_context_unsafe(&context),
                        /* cleanup */ unlock_context());

    /* Find the codec in the cache. */
    struct sail_codec_info_node *node = context->codec_info_node;
    struct sail_codec_info_node *found_node = NULL;

    while (node != NULL) {
        if (node->codec_info == codec_info) {
            if (node->codec != NULL) {
                *codec = node->codec;
                return SAIL_OK;
            }

            found_node = node;
            break;
        }

        node = node->next;
    }

    /* Something weird. The pointer to the codec info is not found in the cache. */
    if (found_node == NULL) {
        unlock_context();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    if (found_node->codec == NULL) {
        SAIL_TRY_OR_CLEANUP(alloc_and_load_codec(found_node->codec_info, &found_node->codec),
                            /* cleanup */ unlock_context());
    }

    *codec = found_node->codec;

    SAIL_TRY(unlock_context());

    return SAIL_OK;
}

void destroy_hidden_state(struct hidden_state *state) {

    if (state == NULL) {
        return;
    }

    if (state->own_io) {
        sail_destroy_io(state->io);
    }

    sail_destroy_write_options(state->write_options);

    /* This state must be freed and zeroed by codecs. We free it just in case to avoid memory leaks. */
    sail_free(state->state);

    sail_free(state);
}

sail_status_t stop_writing(void *state, size_t *written) {

    if (written != NULL) {
        *written = 0;
    }

    /* Not an error. */
    if (state == NULL) {
        return SAIL_OK;
    }

    struct hidden_state *state_of_mind = (struct hidden_state *)state;

    /* Not an error. */
    if (state_of_mind->codec == NULL) {
        destroy_hidden_state(state_of_mind);
        return SAIL_OK;
    }

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v5->write_finish(&state_of_mind->state, state_of_mind->io),
                        /* cleanup */ destroy_hidden_state(state_of_mind));

    if (written != NULL) {
        /* The stream cursor may not be positioned at the end. Let's move it. */
        SAIL_TRY_OR_CLEANUP(state_of_mind->io->seek(state_of_mind->io->stream, 0, SEEK_END),
                            /* cleanup */ destroy_hidden_state(state_of_mind));
        state_of_mind->io->tell(state_of_mind->io->stream, written);
    }

    destroy_hidden_state(state_of_mind);

    return SAIL_OK;
}

sail_status_t allowed_write_output_pixel_format(const struct sail_write_features *write_features, enum SailPixelFormat pixel_format) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);

    for (unsigned i = 0; i < write_features->output_pixel_formats_length; i++) {
        if (write_features->output_pixel_formats[i] == pixel_format) {
            return SAIL_OK;
        }
    }

    print_unsupported_write_pixel_format(pixel_format);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}

sail_status_t alloc_string_node(struct sail_string_node **string_node) {

    SAIL_CHECK_STRING_NODE_PTR(string_node);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_string_node), &ptr));
    *string_node = ptr;

    (*string_node)->value = NULL;
    (*string_node)->next  = NULL;

    return SAIL_OK;
}

void destroy_string_node(struct sail_string_node *string_node) {

    if (string_node == NULL) {
        return;
    }

    sail_free(string_node->value);
    sail_free(string_node);
}

void destroy_string_node_chain(struct sail_string_node *string_node) {

    while (string_node != NULL) {
        struct sail_string_node *string_node_next = string_node->next;

        destroy_string_node(string_node);

        string_node = string_node_next;
    }
}

sail_status_t split_into_string_node_chain(const char *value, struct sail_string_node **target_string_node) {

    struct sail_string_node **last_string_node = target_string_node;

    while (*(value += strspn(value, ";")) != '\0') {
        size_t length = strcspn(value, ";");

        struct sail_string_node *string_node;

        SAIL_TRY(alloc_string_node(&string_node));

        SAIL_TRY_OR_CLEANUP(sail_strdup_length(value, length, &string_node->value),
                            /* cleanup */ destroy_string_node(string_node));

        *last_string_node = string_node;
        last_string_node = &string_node->next;

        value += length;
    }

    return SAIL_OK;
}
