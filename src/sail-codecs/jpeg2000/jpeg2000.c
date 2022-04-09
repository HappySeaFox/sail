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
#include <stddef.h>
#include <stdint.h>

#include <jasper/jasper.h>

#include "sail-common.h"

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct jpeg2000_state {
    struct sail_load_options *load_options;
    struct sail_save_options *save_options;

    bool frame_loaded;
    void *image_data;
    jas_stream_t *jas_stream;
    jas_image_t *jas_image;

    jas_clrspc_t jas_color_space_family;
    int channels[4];
    int number_channels;
    jas_matrix_t *matrix[4];
    /* Channel depth in bits scaled to a byte boundary. For example, 12 bit images are scaled to 16 bit. */
    unsigned channel_depth_scaled;
    unsigned shift;
};

static sail_status_t alloc_jpeg2000_state(struct jpeg2000_state **jpeg2000_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg2000_state), &ptr));
    *jpeg2000_state = ptr;

    jas_init();

    (*jpeg2000_state)->load_options = NULL;
    (*jpeg2000_state)->save_options = NULL;

    (*jpeg2000_state)->frame_loaded    = false;
    (*jpeg2000_state)->image_data      = NULL;
    (*jpeg2000_state)->jas_stream      = NULL;
    (*jpeg2000_state)->jas_image       = NULL;
    (*jpeg2000_state)->number_channels = 0;

    for (int i = 0; i < 4; i++) {
        (*jpeg2000_state)->matrix[i] = NULL;
    }

    (*jpeg2000_state)->shift = 0;

    return SAIL_OK;
}

static void destroy_jpeg2000_state(struct jpeg2000_state *jpeg2000_state) {

    if (jpeg2000_state == NULL) {
        return;
    }

    if (jpeg2000_state->jas_image != NULL) {
        jas_image_destroy(jpeg2000_state->jas_image);
    }

    if (jpeg2000_state->jas_stream != NULL) {
        jas_stream_close(jpeg2000_state->jas_stream);
    }

    for (int i = 0; i < 4; i++) {
        if (jpeg2000_state->matrix[i] != NULL) {
            jas_matrix_destroy(jpeg2000_state->matrix[i]);
        }
    }

    jas_cleanup();

    sail_destroy_load_options(jpeg2000_state->load_options);
    sail_destroy_save_options(jpeg2000_state->save_options);

    sail_free(jpeg2000_state->image_data);

    sail_free(jpeg2000_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v7_jpeg2000(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(load_options);

    /* Allocate a new state. */
    struct jpeg2000_state *jpeg2000_state;
    SAIL_TRY(alloc_jpeg2000_state(&jpeg2000_state));
    *state = jpeg2000_state;

    /* Deep copy load options. */
    SAIL_TRY(sail_copy_load_options(load_options, &jpeg2000_state->load_options));

    /* Read the entire image to use the JasPer memory API. */
    size_t image_size;
    SAIL_TRY(sail_alloc_data_from_io_contents(io, &jpeg2000_state->image_data, &image_size));

    /* This function may generate a warning on old versions of Jasper: conversion from size_t to int. */
    jpeg2000_state->jas_stream = jas_stream_memopen(jpeg2000_state->image_data, image_size);

    if (jpeg2000_state->jas_stream == NULL) {
        SAIL_LOG_ERROR("JPEG2000: Failed to open the specified file");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v7_jpeg2000(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)state;

    if (jpeg2000_state->frame_loaded) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jpeg2000_state->frame_loaded = true;

    /* Get image info. */
    jpeg2000_state->jas_image = jas_image_decode(jpeg2000_state->jas_stream, -1 /* format */, NULL /* options */);

    if (jpeg2000_state->jas_image == NULL) {
        SAIL_LOG_ERROR("JPEG2000: Failed to read image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    jpeg2000_state->jas_color_space_family = jas_clrspc_fam(jas_image_clrspc(jpeg2000_state->jas_image));

    /* Initialize image channels. */
    switch (jpeg2000_state->jas_color_space_family) {
        case JAS_CLRSPC_FAM_GRAY: {
            jpeg2000_state->channels[0] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_GRAY_Y));

            jpeg2000_state->number_channels = 1;
            break;
        }
        case JAS_CLRSPC_FAM_RGB: {
            jpeg2000_state->channels[0] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_R));
            jpeg2000_state->channels[1] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_G));
            jpeg2000_state->channels[2] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_B));
            jpeg2000_state->channels[3] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_OPACITY));

            jpeg2000_state->number_channels = jpeg2000_state->channels[3] > 0 ? 4 : 3;
            break;
        }
        case JAS_CLRSPC_FAM_YCBCR: {
            jpeg2000_state->channels[0] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_Y));
            jpeg2000_state->channels[1] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_CB));
            jpeg2000_state->channels[2] = jas_image_getcmptbytype(jpeg2000_state->jas_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_YCBCR_CR));

            jpeg2000_state->number_channels = 3;
            break;
        }
        default: {
            SAIL_LOG_ERROR("JPEG2000: Unsupported pixel format");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
    }

    /* Check channels ids are valid. */
    for (int i = 0; i < jpeg2000_state->number_channels; i++) {
        if (jpeg2000_state->channels[i] < 0) {
            SAIL_LOG_ERROR("JPEG2000: Some channels are missing");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    const unsigned width = jas_image_width(jpeg2000_state->jas_image);
    const unsigned height = jas_image_height(jpeg2000_state->jas_image);

    /* Check image parameters per channel. */
    for (int i = 0; i < jpeg2000_state->number_channels; i++) {
        const unsigned channel_width = jas_image_cmptwidth(jpeg2000_state->jas_image, jpeg2000_state->channels[i]);
        const unsigned channel_height = jas_image_cmptheight(jpeg2000_state->jas_image, jpeg2000_state->channels[i]);

        if (channel_width != width || channel_height != height) {
            SAIL_LOG_ERROR("JPEG2000: Channel #%d dimensions (%ux%u) don't match image dimensions (%ux%u)",
                            i, channel_width, channel_height, width, height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (jas_image_cmptsgnd(jpeg2000_state->jas_image, jpeg2000_state->channels[i]) != 0) {
            SAIL_LOG_ERROR("JPEG2000: Channel #%d has signed data type", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (jas_image_cmpttlx(jpeg2000_state->jas_image, jpeg2000_state->channels[i]) != 0 ||
                jas_image_cmpttly(jpeg2000_state->jas_image, jpeg2000_state->channels[i]) != 0) {
            SAIL_LOG_ERROR("JPEG2000: Channel #%d has non-zero position", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (jas_image_cmpthstep(jpeg2000_state->jas_image, jpeg2000_state->channels[i]) != 1 ||
                jas_image_cmptvstep(jpeg2000_state->jas_image, jpeg2000_state->channels[i]) != 1) {
            SAIL_LOG_ERROR("JPEG2000: Channel #%d has subsampling factor not equal to 1", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    /* Allocate matrix per channel for reading. */
    for (int i = 0; i < jpeg2000_state->number_channels; i++) {
        if ((jpeg2000_state->matrix[i] = jas_matrix_create(1, width)) == NULL) {
            SAIL_LOG_ERROR("JPEG2000: Matrix allocation failure");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
        }
    }

    /* Detect image format. */
    const unsigned channel_depth = jas_image_cmptprec(jpeg2000_state->jas_image, 0);
    jpeg2000_state->channel_depth_scaled = ((channel_depth + 7) / 8) * 8;

    if (jpeg2000_state->channel_depth_scaled != 8 && jpeg2000_state->channel_depth_scaled != 16) {
        SAIL_LOG_ERROR("JPEG2000: Unsupported bit depth %u scaled from %u", jpeg2000_state->channel_depth_scaled, channel_depth);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    jpeg2000_state->shift = jpeg2000_state->channel_depth_scaled - channel_depth;
    SAIL_LOG_TRACE("JPEG2000: Channels: %d, Channel depth %d (scaled to %d), shift samples by %u",
                    jpeg2000_state->number_channels, channel_depth, jpeg2000_state->channel_depth_scaled, jpeg2000_state->shift);
    const enum SailPixelFormat pixel_format =
        jpeg2000_private_sail_pixel_format(jpeg2000_state->jas_color_space_family, jpeg2000_state->channel_depth_scaled * jpeg2000_state->number_channels);

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        SAIL_LOG_ERROR("JPEG2000: Unsupported pixel format");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Allocate image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));
    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                        /* cleanup */ sail_destroy_image(image_local));

    image_local->source_image->pixel_format = pixel_format;
    image_local->source_image->compression = SAIL_COMPRESSION_JPEG_2000;

    image_local->width = width;
    image_local->height = height;
    image_local->pixel_format = pixel_format;

    SAIL_TRY_OR_CLEANUP(sail_bytes_per_line(image_local->width, image_local->pixel_format, &image_local->bytes_per_line),
                        /* cleanup */ sail_destroy_image(image_local));

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v7_jpeg2000(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    const struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)state;

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *scan = (unsigned char *)image->pixels + row * image->bytes_per_line;

        for (int i = 0; i < jpeg2000_state->number_channels; i++) {
            if (jas_image_readcmpt(jpeg2000_state->jas_image, jpeg2000_state->channels[i], 0 /* x */, row /* y */,
                    image->width /* width */, 1 /* height */, jpeg2000_state->matrix[i]) != 0) {
                SAIL_LOG_ERROR("JPEG2000: Failed to read image row #%u", row);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }

        for (unsigned column = 0; column < image->width; column++) {
            for (int i = 0; i < jpeg2000_state->number_channels; i++) {
                if (jpeg2000_state->channel_depth_scaled == 8) {
                    *scan++ = (unsigned char)(jas_matrix_getv(jpeg2000_state->matrix[i], column + i) << jpeg2000_state->shift);
                } else {
                    uint16_t **scan16 = (uint16_t **)&scan;
                    *(*scan16)++ = (uint16_t)(jas_matrix_getv(jpeg2000_state->matrix[i], column + i) << jpeg2000_state->shift);
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v7_jpeg2000(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)(*state);

    *state = NULL;

    destroy_jpeg2000_state(jpeg2000_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v7_jpeg2000(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(save_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v7_jpeg2000(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v7_jpeg2000(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v7_jpeg2000(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
