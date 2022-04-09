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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sail-common.h"

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct wal_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    unsigned frame_number;

    struct WalFileHeader wal_header;
    unsigned width;
    unsigned height;
};

static sail_status_t alloc_wal_state(struct wal_state **wal_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct wal_state), &ptr));
    *wal_state = ptr;

    (*wal_state)->load_options = NULL;
    (*wal_state)->save_options = NULL;

    (*wal_state)->frame_number  = 0;
    (*wal_state)->width         = 0;
    (*wal_state)->height        = 0;

    return SAIL_OK;
}

static void destroy_wal_state(struct wal_state *wal_state) {

    if (wal_state == NULL) {
        return;
    }

    sail_destroy_load_options(wal_state->load_options);
    sail_destroy_save_options(wal_state->save_options);

    sail_free(wal_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_wal(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct wal_state *wal_state;
    SAIL_TRY(alloc_wal_state(&wal_state));
    *state = wal_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &wal_state->load_options));

    /* Read WAL header. */
    SAIL_TRY(wal_private_read_file_header(io, &wal_state->wal_header));

    wal_state->width = wal_state->wal_header.width;
    wal_state->height = wal_state->wal_header.height;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_wal(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct wal_state *wal_state = (struct wal_state *)state;

    if (wal_state->frame_number == 4) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    if (wal_state->frame_number > 0) {
        wal_state->width /= 2;
        wal_state->height /= 2;
    }

    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
    image_local->source_image->compression = SAIL_COMPRESSION_NONE;

    image_local->width = wal_state->width;
    image_local->height = wal_state->height;
    image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));
    SAIL_TRY_OR_CLEANUP(wal_private_assign_palette(image_local),
                        /* cleanup */ sail_destroy_image(image_local));
    SAIL_TRY_OR_CLEANUP(wal_private_assign_meta_data(&wal_state->wal_header, &image_local->meta_data_node),
                        /* cleanup */ sail_destroy_image(image_local));

    SAIL_TRY_OR_CLEANUP(io->seek(io->stream, wal_state->wal_header.offset[wal_state->frame_number], SEEK_SET),
                        /* cleanup */ sail_destroy_image(image_local));

    wal_state->frame_number++;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_wal(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    SAIL_TRY(io->strict_read(io->stream, image->pixels, (size_t)image->bytes_per_line * image->height));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_wal(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct wal_state *wal_state = (struct wal_state *)(*state);

    *state = NULL;

    destroy_wal_state(wal_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_wal(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_wal(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_wal(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_wal(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
