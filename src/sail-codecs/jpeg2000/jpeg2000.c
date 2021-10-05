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

#include <jasper/jasper.h>

#include "sail-common.h"

#include "helpers.h"
#include "io.h"

/*
 * Codec-specific state.
 */
struct jpeg2000_state {
    struct sail_read_options *read_options;
    struct sail_write_options *write_options;

    bool frame_read;
    void *image_data;
    jas_stream_t *jas_stream;
    jas_image_t *jas_image;

    jas_clrspc_t jas_color_space_family;
    int channels[4];
    int number_channels;
    jas_matrix_t *matrix[4];
};

static sail_status_t alloc_jpeg2000_state(struct jpeg2000_state **jpeg2000_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg2000_state), &ptr));
    *jpeg2000_state = ptr;

    jas_init();

    (*jpeg2000_state)->read_options  = NULL;
    (*jpeg2000_state)->write_options = NULL;

    (*jpeg2000_state)->frame_read      = false;
    (*jpeg2000_state)->image_data      = NULL;
    (*jpeg2000_state)->jas_stream      = NULL;
    (*jpeg2000_state)->jas_image       = NULL;
    (*jpeg2000_state)->number_channels = 0;

    for (int i = 0; i < 4; i++) {
        (*jpeg2000_state)->matrix[i] = NULL;
    }

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

    sail_destroy_read_options(jpeg2000_state->read_options);
    sail_destroy_write_options(jpeg2000_state->write_options);

    sail_free(jpeg2000_state->image_data);

    sail_free(jpeg2000_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_read_init_v6_jpeg2000(struct sail_io *io, const struct sail_read_options *read_options, void **state) {

    SAIL_CHECK_PTR(state);
    *state = NULL;

    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(read_options);

    /* Allocate a new state. */
    struct jpeg2000_state *jpeg2000_state;
    SAIL_TRY(alloc_jpeg2000_state(&jpeg2000_state));
    *state = jpeg2000_state;

    /* Deep copy read options. */
    SAIL_TRY(sail_copy_read_options(read_options, &jpeg2000_state->read_options));

    /* Read image. */
    size_t image_size;
    SAIL_TRY(io->seek(io->stream, 0, SEEK_END));
    SAIL_TRY(io->tell(io->stream, &image_size));
    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    void *ptr;
    SAIL_TRY(sail_malloc(image_size, &ptr));
    jpeg2000_state->image_data = ptr;

    SAIL_LOG_TRACE("JPEG2000: Reading %lu bytes", (unsigned long)image_size);
    SAIL_TRY(io->strict_read(io->stream, jpeg2000_state->image_data, image_size));

    /* Init decoder. */
    jpeg2000_state->jas_stream = jas_stream_memopen2(jpeg2000_state->image_data, image_size);

    if (jpeg2000_state->jas_stream == NULL) {
        SAIL_LOG_ERROR("JPEG2000: Failed to open the specified file");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_OPEN_FILE);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_seek_next_frame_v6_jpeg2000(void *state, struct sail_io *io, struct sail_image **image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(image);

    struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)state;

    if (jpeg2000_state->frame_read) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    jpeg2000_state->frame_read = true;

    /* Get image info. */
    jpeg2000_state->jas_image = jas_image_decode(jpeg2000_state->jas_stream, -1 /* format */, NULL /* options */);

    if (jpeg2000_state->jas_image == NULL) {
        SAIL_LOG_ERROR("JPEG2000: Failed to read an image");
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

        const int bpc = jas_image_cmptprec(jpeg2000_state->jas_image, jpeg2000_state->channels[i]);

        if (bpc != 8) {
            SAIL_LOG_ERROR("JPEG2000: Channel #%d has bit depth %d, but only 8 bits is supported", i, bpc);
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
    const enum SailPixelFormat pixel_format = jpeg2000_private_sail_pixel_format(jpeg2000_state->jas_color_space_family, jpeg2000_state->number_channels);

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

    /* Fetch ICC profile. */
#if 0
    if (jpeg2000_state->read_options->io_options & SAIL_IO_OPTION_ICCP) {
        SAIL_TRY_OR_CLEANUP(jpeg2000_private_fetch_iccp(&jpeg2000_image->icc, &image_local->iccp),
                            /* cleanup */ sail_destroy_image(image_local));
    }
#endif

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_frame_v6_jpeg2000(void *state, struct sail_io *io, struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_skeleton_valid(image));

    struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)state;

    jas_seqent_t *raw_data[4];

    for (int i = 0; i < jpeg2000_state->number_channels; i++) {
        raw_data[i] = jas_matrix_getref(jpeg2000_state->matrix[i], 0, 0);
    }

    for (unsigned row = 0; row < image->height; row++) {
        unsigned char *scan = (unsigned char *)image->pixels + row * image->bytes_per_line;

        for (int i = 0; i < jpeg2000_state->number_channels; i++) {
            if (jas_image_readcmpt(jpeg2000_state->jas_image, jpeg2000_state->channels[i], 0 /* x */, row /* y */,
                    image->width /* width */, 1 /* height */, jpeg2000_state->matrix[i]) != 0) {
                SAIL_LOG_ERROR("JPEG2000: Failed to read image row #%u", row);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
            }
        }

        switch (jpeg2000_state->jas_color_space_family) {
            case JAS_CLRSPC_FAM_GRAY: {
                for (unsigned column = 0; column < image->width; column++) {
                    *scan++ = (unsigned char)*(raw_data[0] + column);
                }
                break;
            }
            case JAS_CLRSPC_FAM_RGB:
            case JAS_CLRSPC_FAM_YCBCR: {
                for (unsigned column = 0; column < image->width; column++) {
                    if (jpeg2000_state->number_channels == 3) {
                        *scan++ = (unsigned char)*(raw_data[0] + column + 0);
                        *scan++ = (unsigned char)*(raw_data[1] + column + 1);
                        *scan++ = (unsigned char)*(raw_data[2] + column + 2);
                    }
                }
                break;
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_read_finish_v6_jpeg2000(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    struct jpeg2000_state *jpeg2000_state = (struct jpeg2000_state *)(*state);

    /* Subsequent calls to finish() will expectedly fail in the above line. */
    *state = NULL;

    destroy_jpeg2000_state(jpeg2000_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_write_init_v6_jpeg2000(struct sail_io *io, const struct sail_write_options *write_options, void **state) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_CHECK_PTR(write_options);

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_seek_next_frame_v6_jpeg2000(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_frame_v6_jpeg2000(void *state, struct sail_io *io, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));
    SAIL_TRY(sail_check_image_valid(image));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

SAIL_EXPORT sail_status_t sail_codec_write_finish_v6_jpeg2000(void **state, struct sail_io *io) {

    SAIL_CHECK_PTR(state);
    SAIL_TRY(sail_check_io_valid(io));

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
