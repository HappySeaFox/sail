/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sail-common.h"

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include "qoi.h"

/*
 * Codec-specific state.
 */
struct qoi_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;
    bool frame_saved;

    void *image_data;
    size_t image_data_size;
    void *pixels;

    qoi_desc qoi_desc;
};

static sail_status_t alloc_qoi_state(struct qoi_state **qoi_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct qoi_state), &ptr));
    *qoi_state = ptr;

    (*qoi_state)->load_options = NULL;
    (*qoi_state)->save_options = NULL;

    (*qoi_state)->frame_loaded = false;
    (*qoi_state)->frame_saved  = false;

    (*qoi_state)->image_data      = NULL;
    (*qoi_state)->image_data_size = 0;
    (*qoi_state)->pixels          = NULL;

    return SAIL_OK;
}

static void destroy_qoi_state(struct qoi_state *qoi_state) {

    if (qoi_state == NULL) {
        return;
    }

    sail_destroy_load_options(qoi_state->load_options);
    sail_destroy_save_options(qoi_state->save_options);

    sail_free(qoi_state->image_data);
    sail_free(qoi_state->pixels);

    sail_free(qoi_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_qoi(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct qoi_state *qoi_state;
    SAIL_TRY(alloc_qoi_state(&qoi_state));
    *state = qoi_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &qoi_state->load_options));

    /* Cache the entire file as the QOI API requires. */
    SAIL_TRY(sail_alloc_data_from_io_contents(io, &qoi_state->image_data, &qoi_state->image_data_size));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_qoi(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct qoi_state *qoi_state = (struct qoi_state *)state;

    if (qoi_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    qoi_state->frame_loaded = true;

    /* Decode the image. */
    /* TODO Remove (int) when QOI supports size_t. */
    qoi_state->pixels = qoi_decode(qoi_state->image_data, (int)qoi_state->image_data_size, &qoi_state->qoi_desc, 0);

    if (qoi_state->pixels == NULL) {
        SAIL_LOG_ERROR("QOI: Image is broken without any details");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    if (qoi_state->qoi_desc.colorspace != QOI_SRGB) {
        SAIL_LOG_ERROR("QOI: Only RGB images are supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Construct the SAIL image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    switch (qoi_state->qoi_desc.channels) {
        case 3: image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP24_RGB;  break;
        case 4: image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA; break;
        default: {
            sail_destroy_image(image_local);

            SAIL_LOG_ERROR("QOI: Number of channels is %d, but only RGB24 and RGB32 images are supported", qoi_state->qoi_desc.channels);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    image_local->source_image->compression = SAIL_COMPRESSION_QOI;

    image_local->width = qoi_state->qoi_desc.width;
    image_local->height = qoi_state->qoi_desc.height;
    image_local->pixel_format = image_local->source_image->pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_qoi(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    const struct qoi_state *qoi_state = (struct qoi_state *)state;

    const size_t pixels_size = (size_t)image->bytes_per_line * image->height;

    memcpy(image->pixels, qoi_state->pixels, pixels_size);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_qoi(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct qoi_state *qoi_state = (struct qoi_state *)(*state);

    *state = NULL;

    destroy_qoi_state(qoi_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_qoi(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    struct qoi_state *qoi_state;
    SAIL_TRY(alloc_qoi_state(&qoi_state));

    *state = qoi_state;

    /* Deep copy save options. */
    SAIL_TRY(sail_copy_save_options(save_options, &qoi_state->save_options));

    /* Sanity check. */
    if (qoi_state->save_options->compression != SAIL_COMPRESSION_QOI) {
        SAIL_LOG_ERROR("QOI: Only QOI compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_qoi(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct qoi_state *qoi_state = (struct qoi_state *)state;

    unsigned char channels;

    switch (image->pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP24_RGB:  channels = 3; break;
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: channels = 4; break;
        default: {
            SAIL_LOG_ERROR("QOI: %s pixel format is not currently supported for saving", sail_pixel_format_to_string(image->pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    int written;
    qoi_state->pixels = qoi_encode(image->pixels, &(qoi_desc){
    	                    .width      = image->width,
                    	    .height     = image->height,
                        	.channels   = channels,
                        	.colorspace = QOI_SRGB
                        }, &written);

    if (qoi_state->pixels == NULL) {
        SAIL_LOG_ERROR("QOI: Encoding failed without any details");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_qoi(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    struct qoi_state *qoi_state = (struct qoi_state *)state;

    const size_t pixels_size = (size_t)image->bytes_per_line * image->height;

    SAIL_TRY(io->strict_write(io->stream, qoi_state->pixels, pixels_size));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_qoi(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct qoi_state *qoi_state = (struct qoi_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_qoi_state(qoi_state);

    return SAIL_OK;
}
