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

#include <stdbool.h>

#include "sail-common.h"

#include "common/bmp/bmp.h"

/*
 * Codec-specific state.
 */
struct bmp_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;
    void *common_bmp_state;
};

static sail_status_t alloc_bmp_state(struct bmp_state **bmp_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct bmp_state), &ptr));
    *bmp_state = ptr;

    (*bmp_state)->load_options = NULL;
    (*bmp_state)->save_options = NULL;

    (*bmp_state)->frame_loaded     = false;
    (*bmp_state)->common_bmp_state = NULL;

    return SAIL_OK;
}

static void destroy_bmp_state(struct bmp_state *bmp_state) {

    if (bmp_state == NULL) {
        return;
    }

    sail_destroy_load_options(bmp_state->load_options);
    sail_destroy_save_options(bmp_state->save_options);

    sail_free(bmp_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_bmp(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct bmp_state *bmp_state;
    SAIL_TRY(alloc_bmp_state(&bmp_state));
    *state = bmp_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &bmp_state->load_options));

    SAIL_TRY(bmp_private_read_init(io, bmp_state->load_options, &bmp_state->common_bmp_state, SAIL_READ_BMP_FILE_HEADER));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_bmp(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct bmp_state *bmp_state = (struct bmp_state *)state;

    if (bmp_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    bmp_state->frame_loaded = true;

    SAIL_TRY(bmp_private_read_seek_next_frame(bmp_state->common_bmp_state, io, image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_bmp(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct bmp_state *bmp_state = (struct bmp_state *)state;

    SAIL_TRY(bmp_private_read_frame(bmp_state->common_bmp_state, io, image));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_bmp(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct bmp_state *bmp_state = (struct bmp_state *)(*state);

    *state = NULL;

    if (bmp_state->common_bmp_state != NULL) {
        SAIL_TRY_OR_CLEANUP(bmp_private_read_finish(&bmp_state->common_bmp_state, io),
                            /* cleanup */ destroy_bmp_state(bmp_state));
    }

    destroy_bmp_state(bmp_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_bmp(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_bmp(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_bmp(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
