/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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
#include <string.h>

#include <jbig.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

/*
 * Codec-specific state.
 */
struct jbig_codec_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    bool frame_loaded;

    unsigned long width;
    unsigned long height;
    int planes;
};

static sail_status_t alloc_jbig_codec_state(struct sail_io *io,
                                              const struct sail_load_options *load_options,
                                              const struct sail_save_options *save_options,
                                              struct jbig_codec_state **jbig_codec_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jbig_codec_state), &ptr));
    *jbig_codec_state = ptr;

    **jbig_codec_state = (struct jbig_codec_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded = false,

        .width        = 0,
        .height       = 0,
        .planes       = 0,
    };

    return SAIL_OK;
}

static void destroy_jbig_codec_state(struct jbig_codec_state *jbig_codec_state) {

    if (jbig_codec_state == NULL) {
        return;
    }

    sail_free(jbig_codec_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_jbig(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct jbig_codec_state *jbig_codec_state;
    SAIL_TRY(alloc_jbig_codec_state(io, load_options, NULL, &jbig_codec_state));
    *state = jbig_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jbig(void *state, struct sail_image **image) {

    struct jbig_codec_state *jbig_state = state;

    if (jbig_state->frame_loaded) {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    jbig_state->frame_loaded = true;

    /* Read JBIG header to get image dimensions. */
    SAIL_TRY(jbig_private_read_header(jbig_state->io,
                                       &jbig_state->width,
                                       &jbig_state->height,
                                       &jbig_state->planes));

    SAIL_LOG_TRACE("JBIG: %lux%lu, %d planes",
                   jbig_state->width, jbig_state->height, jbig_state->planes);

    /* Construct image. */
    struct sail_image *image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    image_local->width = jbig_state->width;
    image_local->height = jbig_state->height;

    /* JBIG is 1 bpp. */
    if (jbig_state->planes == 1) {
        image_local->pixel_format = SAIL_PIXEL_FORMAT_BPP1;
    } else {
        sail_destroy_image(image_local);
        SAIL_LOG_ERROR("JBIG: Multi-plane images not supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Add source image info if requested. */
    if (jbig_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                           /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = image_local->pixel_format;
        image_local->source_image->compression = SAIL_COMPRESSION_JBIG;
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_jbig(void *state, struct sail_image *image) {

    const struct jbig_codec_state *jbig_codec_state = state;

    struct jbg_dec_state decoder;
    unsigned char buffer[4096];
    size_t bytes_read;
    int result;
    size_t cnt;

    /* Initialize JBIG decoder. */
    jbg_dec_init(&decoder);

    /* Read and decode JBIG data. */
    while (true) {
        SAIL_TRY(jbig_codec_state->io->tolerant_read(
            jbig_codec_state->io->stream, buffer, sizeof(buffer), &bytes_read));

        if (bytes_read == 0) {
            break;
        }

        result = jbg_dec_in(&decoder, buffer, bytes_read, &cnt);

        if (result == JBG_EOK) {
            break;
        } else if (result == JBG_EAGAIN) {
            continue;
        } else {
            jbg_dec_free(&decoder);
            SAIL_LOG_ERROR("JBIG: Decoding error: %s", jbg_strerror(result));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    /* Get decoded image dimensions. */
    int planes = jbg_dec_getplanes(&decoder);

    SAIL_LOG_TRACE("JBIG: %d planes", planes);

    /* For single-plane images, copy directly. */
    if (planes == 1) {
        unsigned char *jbig_data = jbg_dec_getimage(&decoder, 0);
        unsigned long jbig_size = jbg_dec_getsize(&decoder);

        size_t expected_size = ((image->width + 7) / 8) * image->height;

        if (jbig_size < expected_size) {
            jbg_dec_free(&decoder);
            SAIL_LOG_ERROR("JBIG: Insufficient decoded data");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        /* Copy scanlines. */
        unsigned char *dest = image->pixels;
        unsigned long src_bytes_per_line = (image->width + 7) / 8;

        for (unsigned long y = 0; y < image->height; y++) {
            memcpy(dest + y * image->bytes_per_line,
                jbig_data + y * src_bytes_per_line, src_bytes_per_line);
        }
    } else {
        /* TODO: Multi-plane JBIG images are not yet supported. */
        jbg_dec_free(&decoder);
        SAIL_LOG_ERROR("JBIG: Multi-plane images not yet supported");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }

    jbg_dec_free(&decoder);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_jbig(void **state) {

    struct jbig_codec_state *jbig_codec_state = *state;

    *state = NULL;

    destroy_jbig_codec_state(jbig_codec_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

/* Callback for JBIG encoder to write data. */
struct jbig_write_context {
    struct sail_io *io;
    sail_status_t status;
    unsigned long stripe_height;
    int options;
};

static void jbig_write_callback(unsigned char *data, size_t len, void *context) {

    struct jbig_write_context *ctx = context;

    if (ctx->status != SAIL_OK) {
        return;
    }

    ctx->status = ctx->io->strict_write(ctx->io->stream, data, len);
}

static bool jbig_private_tuning_key_value_callback(const char *key, const struct sail_variant *value, void *user_data) {

    struct jbig_write_context *write_ctx = user_data;

    if (strcmp(key, "jbig-stripe-height") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned long val = (value->type == SAIL_VARIANT_TYPE_INT)
                ? (unsigned long)sail_variant_to_int(value)
                : (unsigned long)sail_variant_to_unsigned_int(value);
            write_ctx->stripe_height = val;
            SAIL_LOG_TRACE("JBIG: stripe-height=%lu", write_ctx->stripe_height);
        } else {
            SAIL_LOG_ERROR("JBIG: 'jbig-stripe-height' must be an integer");
        }
    } else if (strcmp(key, "jbig-typical-prediction") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned val = (value->type == SAIL_VARIANT_TYPE_INT)
                ? (unsigned)sail_variant_to_int(value)
                : sail_variant_to_unsigned_int(value);
            if (val != 0) {
                write_ctx->options |= JBG_TPDON;
                SAIL_LOG_TRACE("JBIG: typical-prediction enabled");
            }
        } else {
            SAIL_LOG_ERROR("JBIG: 'jbig-typical-prediction' must be an integer");
        }
    }

    return true;
}

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_jbig(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(state);

    *state = NULL;

    /* Allocate a new state. */
    struct jbig_codec_state *jbig_codec_state;
    SAIL_TRY(alloc_jbig_codec_state(io, NULL, save_options, &jbig_codec_state));
    *state = jbig_codec_state;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_jbig(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct jbig_codec_state *jbig_codec_state = state;

    if (jbig_codec_state->frame_loaded) {
        SAIL_LOG_ERROR("JBIG: Only single frame is supported for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    /* JBIG only supports BPP1. */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP1) {
        SAIL_LOG_ERROR("JBIG: Only BPP1 pixel format is supported for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    jbig_codec_state->frame_loaded = true;
    jbig_codec_state->width = image->width;
    jbig_codec_state->height = image->height;
    jbig_codec_state->planes = 1;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_jbig(void *state, const struct sail_image *image) {

    SAIL_CHECK_PTR(state);
    SAIL_CHECK_PTR(image);

    struct jbig_codec_state *jbig_codec_state = state;

    /* JBIG only supports 1-bit per pixel images. */
    if (image->pixel_format != SAIL_PIXEL_FORMAT_BPP1) {
        SAIL_LOG_ERROR("JBIG: Only BPP1 pixel format is supported for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    struct jbig_write_context write_ctx = {
        .io = jbig_codec_state->io,
        .status = SAIL_OK,
        .stripe_height = 0,  /* 0 = automatic. */
        .options = 0
    };

    /* Handle tuning options. */
    if (jbig_codec_state->save_options != NULL && jbig_codec_state->save_options->tuning != NULL) {
        sail_traverse_hash_map_with_user_data(jbig_codec_state->save_options->tuning,
                                               jbig_private_tuning_key_value_callback,
                                               &write_ctx);
    }

    struct jbg_enc_state encoder;

    /* Prepare image data as required by JBIG encoder (array of plane pointers). */
    unsigned char *planes[1];
    planes[0] = image->pixels;

    /* Initialize JBIG encoder. */
    jbg_enc_init(&encoder,
                 image->width,
                 image->height,
                 1,  /* 1 plane. */
                 planes,
                 jbig_write_callback,
                 &write_ctx);

    /* Set encoding options with tuning parameters. */
    jbg_enc_options(&encoder,
                    JBG_ILEAVE | JBG_SMID,
                    write_ctx.options,
                    write_ctx.stripe_height,
                    -1, -1);  /* No ATMOVE. */

    /* Encode and write. */
    jbg_enc_out(&encoder);

    jbg_enc_free(&encoder);

    if (write_ctx.status != SAIL_OK) {
        SAIL_LOG_ERROR("JBIG: Write error during encoding");
        return write_ctx.status;
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_jbig(void **state) {

    SAIL_CHECK_PTR(state);

    struct jbig_codec_state *jbig_codec_state = *state;

    *state = NULL;

    destroy_jbig_codec_state(jbig_codec_state);

    return SAIL_OK;
}
