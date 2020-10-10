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

#include "sail-common.h"
#include "sail.h"

/*
 * Private functions.
 */

static sail_status_t load_codec(struct sail_codec_info_node *node) {

    SAIL_CHECK_PTR(node);

    /* Already loaded. */
    if (node->codec != NULL) {
        return SAIL_OK;
    }

    /* Codec is not loaded. Let's load it. */
    SAIL_TRY(alloc_and_load_codec(node->codec_info, &node->codec));

    return SAIL_OK;
}

static void print_unsupported_write_output_pixel_format(enum SailPixelFormat input_pixel_format, enum SailPixelFormat output_pixel_format) {

    const char *input_pixel_format_str = NULL;
    const char *output_pixel_format_str = NULL;

    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(input_pixel_format, &input_pixel_format_str));
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(output_pixel_format, &output_pixel_format_str));

    SAIL_LOG_ERROR("This codec cannot output %s pixels from %s pixels. Use its write features to get the list of supported output pixel formats",
                    input_pixel_format_str, output_pixel_format_str);
}

static void print_unsupported_write_input_pixel_format(enum SailPixelFormat input_pixel_format) {

    const char *input_pixel_format_str = NULL;
    SAIL_TRY_OR_SUPPRESS(sail_pixel_format_to_string(input_pixel_format, &input_pixel_format_str));

    SAIL_LOG_ERROR("This codec cannot take %s pixels as input. Use its write features to get the list of supported input pixel formats",
                    input_pixel_format_str);
}

/*
 * Public functions.
 */

sail_status_t load_codec_by_codec_info(const struct sail_codec_info *codec_info, const struct sail_codec **codec) {

    SAIL_CHECK_CODEC_INFO_PTR(codec_info);
    SAIL_CHECK_CODEC_PTR(codec);

    struct sail_context *context;
    SAIL_TRY(current_tls_context(&context));

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

    /* Something weird. The pointer to the codec info is not found the cache. */
    if (found_node == NULL) {
        return SAIL_ERROR_CODEC_NOT_FOUND;
    }

    SAIL_TRY(load_codec(found_node));

    *codec = found_node->codec;

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

    SAIL_TRY_OR_CLEANUP(state_of_mind->codec->v3->write_finish(&state_of_mind->state, state_of_mind->io),
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

sail_status_t allowed_write_output_pixel_format(const struct sail_write_features *write_features,
                                                enum SailPixelFormat input_pixel_format,
                                                enum SailPixelFormat output_pixel_format) {

    SAIL_CHECK_WRITE_FEATURES_PTR(write_features);

    /* Codecs will compute output pixel format automatically. */
    if (output_pixel_format == SAIL_PIXEL_FORMAT_AUTO) {
        return SAIL_OK;
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
            for (unsigned i = 0; i < node->output_pixel_formats_length; i++) {
                if (node->output_pixel_formats[i] == output_pixel_format) {
                    return SAIL_OK;
                }
            }

            print_unsupported_write_output_pixel_format(input_pixel_format, output_pixel_format);
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }

        node = node->next;
    }

    print_unsupported_write_input_pixel_format(input_pixel_format);
    return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
}
