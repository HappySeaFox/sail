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

#include "sail.h"

/*
 * Private functions.
 */

static void print_unsupported_write_pixel_format(enum SailPixelFormat pixel_format) {

    SAIL_LOG_ERROR("This codec cannot save %s pixels. Use its save features to get the list of supported pixel formats for saving",
                    sail_pixel_format_to_string(pixel_format));
}

static sail_status_t load_codec_by_codec_info_unsafe(const struct sail_codec_info *codec_info, const struct sail_codec **codec) {

    SAIL_CHECK_PTR(codec_info);
    SAIL_CHECK_PTR(codec);

    struct sail_context *context;
    SAIL_TRY(fetch_global_context_unsafe(&context));

    /* Find the codec in the cache. */
    struct sail_codec_bundle *found_codec_bundle = NULL;

    for (struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next) {
        struct sail_codec_bundle *codec_bundle = codec_bundle_node->codec_bundle;

        if (codec_bundle->codec_info == codec_info) {
            if (codec_bundle->codec != NULL) {
                *codec = codec_bundle->codec;
                return SAIL_OK;
            }

            found_codec_bundle = codec_bundle_node->codec_bundle;
            break;
        }
    }

    /* Something weird. The pointer to the codec info is not found in the cache. */
    if (found_codec_bundle == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }

    if (found_codec_bundle->codec == NULL) {
        SAIL_TRY(alloc_and_load_codec(found_codec_bundle->codec_info, &found_codec_bundle->codec));
    }

    *codec = found_codec_bundle->codec;

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t load_codec_by_codec_info(const struct sail_codec_info *codec_info, const struct sail_codec **codec) {

    SAIL_CHECK_PTR(codec_info);
    SAIL_CHECK_PTR(codec);

    SAIL_TRY(lock_context());

    SAIL_TRY_OR_CLEANUP(load_codec_by_codec_info_unsafe(codec_info, codec),
                        /* cleanup */ unlock_context());

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

    sail_destroy_save_options(state->save_options);

    /* This state must be freed and zeroed by codecs. We free it just in case to avoid memory leaks. */
    sail_free(state->state);

    sail_free(state);
}

sail_status_t stop_saving(void *state, size_t *written) {

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

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v7->save_finish(&state_of_mind->state, state_of_mind->io),
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

sail_status_t allowed_write_output_pixel_format(const struct sail_save_features *save_features, enum SailPixelFormat pixel_format) {

    SAIL_CHECK_PTR(save_features);

    for (unsigned i = 0; i < save_features->pixel_formats_length; i++) {
        if (save_features->pixel_formats[i] == pixel_format) {
            return SAIL_OK;
        }
    }

    print_unsupported_write_pixel_format(pixel_format);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
}
