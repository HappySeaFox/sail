/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <string.h>

#include <openjpeg.h>

#include <sail-common/sail-common.h>

#include "helpers.h"
#include "io_dest.h"
#include "io_src.h"

/*
 * Codec-specific state.
 */
struct jpeg2000_state
{
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    bool frame_loaded;
    struct sail_io* io;
    opj_stream_t* opj_stream;
    opj_codec_t* opj_codec;
    opj_image_t* opj_image;

    int channel_depth_scaled;
    int shift;
};

static void jpeg2000_private_error_callback(const char* msg, void* client_data)
{
    (void)client_data;
    SAIL_LOG_ERROR("JPEG2000: %s", msg);
}

static void jpeg2000_private_warning_callback(const char* msg, void* client_data)
{
    (void)client_data;
    SAIL_LOG_WARNING("JPEG2000: %s", msg);
}

static void jpeg2000_private_info_callback(const char* msg, void* client_data)
{
    (void)client_data;
    SAIL_LOG_TRACE("JPEG2000: %s", msg);
}

static sail_status_t alloc_jpeg2000_state(struct sail_io* io,
                                          const struct sail_load_options* load_options,
                                          const struct sail_save_options* save_options,
                                          struct jpeg2000_state** jpeg2000_state)
{

    void* ptr;
    SAIL_TRY(sail_malloc(sizeof(struct jpeg2000_state), &ptr));
    *jpeg2000_state = ptr;

    **jpeg2000_state = (struct jpeg2000_state){
        .load_options = load_options,
        .save_options = save_options,

        .frame_loaded         = false,
        .io                   = io,
        .opj_stream           = NULL,
        .opj_codec            = NULL,
        .opj_image            = NULL,
        .channel_depth_scaled = 0,
        .shift                = 0,
    };

    return SAIL_OK;
}

static void destroy_jpeg2000_state(struct jpeg2000_state* jpeg2000_state)
{

    if (jpeg2000_state == NULL)
    {
        return;
    }

    if (jpeg2000_state->opj_image != NULL)
    {
        opj_image_destroy(jpeg2000_state->opj_image);
    }

    if (jpeg2000_state->opj_codec != NULL)
    {
        opj_destroy_codec(jpeg2000_state->opj_codec);
    }

    if (jpeg2000_state->opj_stream != NULL)
    {
        opj_stream_destroy(jpeg2000_state->opj_stream);
    }

    sail_free(jpeg2000_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_jpeg2000(struct sail_io* io,
                                                           const struct sail_load_options* load_options,
                                                           void** state)
{

    *state = NULL;

    /* Allocate a new state. */
    struct jpeg2000_state* jpeg2000_state;
    SAIL_TRY(alloc_jpeg2000_state(io, load_options, NULL, &jpeg2000_state));
    *state = jpeg2000_state;

    /* Detect format by reading magic numbers. */
    unsigned char magic[12];
    size_t magic_read;
    OPJ_CODEC_FORMAT format = OPJ_CODEC_JP2;

    if (io->tolerant_read(io->stream, magic, sizeof(magic), &magic_read) == SAIL_OK && magic_read >= 4)
    {
        /* J2K magic: FF 4F FF 51 */
        if (magic[0] == 0xFF && magic[1] == 0x4F && magic[2] == 0xFF && magic[3] == 0x51)
        {
            format = OPJ_CODEC_J2K;
        }
        /* JP2 magic is default: 00 00 00 0C 6A 50 20 20 0D 0A 87 0A */
    }

    /* Seek back to beginning. */
    SAIL_TRY(io->seek(io->stream, 0, SEEK_SET));

    /* Create OpenJPEG stream from SAIL I/O. */
    jpeg2000_state->opj_stream = jpeg2000_private_sail_io_src(io);

    if (jpeg2000_state->opj_stream == NULL)
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to create stream");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Create decoder for detected format. */
    jpeg2000_state->opj_codec = opj_create_decompress(format);

    if (jpeg2000_state->opj_codec == NULL)
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to create decoder");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Set up error handlers. */
    opj_set_error_handler(jpeg2000_state->opj_codec, jpeg2000_private_error_callback, NULL);
    opj_set_warning_handler(jpeg2000_state->opj_codec, jpeg2000_private_warning_callback, NULL);
    opj_set_info_handler(jpeg2000_state->opj_codec, jpeg2000_private_info_callback, NULL);

    /* Setup decoder with default parameters. */
    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);

    /* Handle tuning. */
    if (load_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(load_options->tuning, jpeg2000_private_tuning_key_value_callback_load,
                                              &parameters);
    }

    if (!opj_setup_decoder(jpeg2000_state->opj_codec, &parameters))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to setup decoder");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_jpeg2000(void* state, struct sail_image** image)
{

    struct jpeg2000_state* jpeg2000_state = state;

    if (jpeg2000_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    jpeg2000_state->frame_loaded = true;

    /* Read header. */
    if (!opj_read_header(jpeg2000_state->opj_stream, jpeg2000_state->opj_codec, &jpeg2000_state->opj_image))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to read header");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (jpeg2000_state->opj_image == NULL)
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to get image information");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Decode the image. */
    if (!opj_decode(jpeg2000_state->opj_codec, jpeg2000_state->opj_stream, jpeg2000_state->opj_image))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to decode image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (!opj_end_decompress(jpeg2000_state->opj_codec, jpeg2000_state->opj_stream))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to end decompression");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    const opj_image_t* opj_image = jpeg2000_state->opj_image;

    /* Check image validity. */
    if (opj_image->numcomps == 0)
    {
        SAIL_LOG_ERROR("JPEG2000: Image has no components");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    const OPJ_UINT32 width  = opj_image->comps[0].w;
    const OPJ_UINT32 height = opj_image->comps[0].h;
    const OPJ_UINT32 prec   = opj_image->comps[0].prec;

    /* Verify all components have same dimensions and precision. */
    for (OPJ_UINT32 i = 0; i < opj_image->numcomps; i++)
    {
        if (opj_image->comps[i].w != width || opj_image->comps[i].h != height)
        {
            SAIL_LOG_ERROR("JPEG2000: Component %u dimensions (%ux%u) don't match image dimensions (%ux%u)", i,
                           opj_image->comps[i].w, opj_image->comps[i].h, width, height);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (opj_image->comps[i].prec != prec)
        {
            SAIL_LOG_ERROR("JPEG2000: Component %u precision %u doesn't match expected precision %u", i,
                           opj_image->comps[i].prec, prec);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (opj_image->comps[i].sgnd != 0)
        {
            SAIL_LOG_ERROR("JPEG2000: Component %u has signed data type", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (opj_image->comps[i].x0 != 0 || opj_image->comps[i].y0 != 0)
        {
            SAIL_LOG_ERROR("JPEG2000: Component %u has non-zero position", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }

        if (opj_image->comps[i].dx != 1 || opj_image->comps[i].dy != 1)
        {
            SAIL_LOG_ERROR("JPEG2000: Component %u has subsampling factor not equal to 1", i);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    /* Calculate scaled precision and shift. */
    jpeg2000_state->channel_depth_scaled = ((prec + 7) / 8) * 8;

    if (jpeg2000_state->channel_depth_scaled != 8 && jpeg2000_state->channel_depth_scaled != 16)
    {
        SAIL_LOG_ERROR("JPEG2000: Unsupported bit depth %d scaled from %u", jpeg2000_state->channel_depth_scaled, prec);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    jpeg2000_state->shift = jpeg2000_state->channel_depth_scaled - prec;

    SAIL_LOG_TRACE("JPEG2000: Components: %u, Precision: %u (scaled to %d), shift samples by %d", opj_image->numcomps,
                   prec, jpeg2000_state->channel_depth_scaled, jpeg2000_state->shift);

    /* Determine pixel format. */
    const enum SailPixelFormat pixel_format =
        jpeg2000_private_sail_pixel_format(opj_image->color_space, opj_image->numcomps, prec);

    if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_ERROR("JPEG2000: Unsupported pixel format (color space: %d, components: %u, precision: %u)",
                       opj_image->color_space, opj_image->numcomps, prec);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    /* Allocate image. */
    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (jpeg2000_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));

        image_local->source_image->pixel_format = pixel_format;
        image_local->source_image->compression  = SAIL_COMPRESSION_JPEG_2000;
    }

    image_local->width          = width;
    image_local->height         = height;
    image_local->pixel_format   = pixel_format;
    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    /* Fetch ICC profile. */
    if (jpeg2000_state->load_options->options & SAIL_OPTION_ICCP)
    {
        if (opj_image->icc_profile_buf != NULL && opj_image->icc_profile_len > 0)
        {
            SAIL_TRY_OR_CLEANUP(sail_alloc_iccp(&image_local->iccp),
                                /* cleanup */ sail_destroy_image(image_local));

            void* icc_data;
            SAIL_TRY_OR_CLEANUP(sail_malloc(opj_image->icc_profile_len, &icc_data),
                                /* cleanup */ sail_destroy_image(image_local));

            memcpy(icc_data, opj_image->icc_profile_buf, opj_image->icc_profile_len);

            image_local->iccp->data = icc_data;
            image_local->iccp->size = opj_image->icc_profile_len;

            SAIL_LOG_TRACE("JPEG2000: ICC profile loaded (%u bytes)", opj_image->icc_profile_len);
        }
    }

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_jpeg2000(void* state, struct sail_image* image)
{

    const struct jpeg2000_state* jpeg2000_state = state;
    const opj_image_t* opj_image                = jpeg2000_state->opj_image;

    /* Copy image data component by component. */
    for (OPJ_UINT32 row = 0; row < image->height; row++)
    {
        if (jpeg2000_state->channel_depth_scaled == 8)
        {
            unsigned char* scan = sail_scan_line(image, row);

            for (OPJ_UINT32 column = 0; column < image->width; column++)
            {
                for (OPJ_UINT32 comp = 0; comp < opj_image->numcomps; comp++)
                {
                    const OPJ_INT32 value = opj_image->comps[comp].data[row * image->width + column];
                    *scan++               = (unsigned char)(value << jpeg2000_state->shift);
                }
            }
        }
        else
        {
            uint16_t* scan = sail_scan_line(image, row);

            for (OPJ_UINT32 column = 0; column < image->width; column++)
            {
                for (OPJ_UINT32 comp = 0; comp < opj_image->numcomps; comp++)
                {
                    const OPJ_INT32 value = opj_image->comps[comp].data[row * image->width + column];
                    *scan++               = (uint16_t)(value << jpeg2000_state->shift);
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_jpeg2000(void** state)
{

    struct jpeg2000_state* jpeg2000_state = *state;

    *state = NULL;

    destroy_jpeg2000_state(jpeg2000_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_jpeg2000(struct sail_io* io,
                                                           const struct sail_save_options* save_options,
                                                           void** state)
{

    *state = NULL;

    /* Allocate a new state. */
    struct jpeg2000_state* jpeg2000_state;
    SAIL_TRY(alloc_jpeg2000_state(io, NULL, save_options, &jpeg2000_state));
    *state = jpeg2000_state;

    /* Check compression type. */
    if (jpeg2000_state->save_options->compression != SAIL_COMPRESSION_JPEG_2000)
    {
        SAIL_LOG_ERROR("JPEG2000: Only JPEG-2000 compression is allowed for saving");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_COMPRESSION);
    }

    /* Create OpenJPEG stream for writing. */
    jpeg2000_state->opj_stream = jpeg2000_private_sail_io_dest(io);

    if (jpeg2000_state->opj_stream == NULL)
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to create output stream");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_jpeg2000(void* state, const struct sail_image* image)
{

    struct jpeg2000_state* jpeg2000_state = state;

    if (jpeg2000_state->frame_loaded)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    jpeg2000_state->frame_loaded = true;

    /* Determine color space and channel configuration. */
    OPJ_COLOR_SPACE color_space;
    int num_comps;
    int prec;

    SAIL_TRY_OR_CLEANUP(jpeg2000_private_pixel_format_to_openjpeg(image->pixel_format, &color_space, &num_comps, &prec),
                        /* cleanup */ SAIL_LOG_ERROR("JPEG2000: %s pixel format is not supported for saving",
                                                     sail_pixel_format_to_string(image->pixel_format)));

    /* Create image component parameters. */
    opj_image_cmptparm_t cmptparms[5];

    for (int i = 0; i < num_comps; i++)
    {
        memset(&cmptparms[i], 0, sizeof(opj_image_cmptparm_t));
        cmptparms[i].dx   = 1;
        cmptparms[i].dy   = 1;
        cmptparms[i].w    = image->width;
        cmptparms[i].h    = image->height;
        cmptparms[i].x0   = 0;
        cmptparms[i].y0   = 0;
        cmptparms[i].prec = prec;
        cmptparms[i].sgnd = 0;
    }

    /* Create OpenJPEG image. */
    jpeg2000_state->opj_image = opj_image_create(num_comps, cmptparms, color_space);

    if (jpeg2000_state->opj_image == NULL)
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to create image");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    jpeg2000_state->opj_image->x0 = 0;
    jpeg2000_state->opj_image->y0 = 0;
    jpeg2000_state->opj_image->x1 = image->width;
    jpeg2000_state->opj_image->y1 = image->height;

    jpeg2000_state->channel_depth_scaled = prec;

    /*
     * OpenJPEG 2.5.4+ has ICC profile encoding bug fixed
     * https://github.com/uclouvain/openjpeg/pull/1574
     */
#if (OPJ_VERSION_MAJOR > 2) || (OPJ_VERSION_MAJOR == 2 && OPJ_VERSION_MINOR > 5)                                       \
    || (OPJ_VERSION_MAJOR == 2 && OPJ_VERSION_MINOR == 5 && OPJ_VERSION_BUILD >= 4)
    if ((jpeg2000_state->save_options->options & SAIL_OPTION_ICCP) && image->iccp != NULL && image->iccp->data != NULL
        && image->iccp->size > 0)
    {
        void* icc_copy;
        SAIL_TRY(sail_malloc(image->iccp->size, &icc_copy));

        memcpy(icc_copy, image->iccp->data, image->iccp->size);

        jpeg2000_state->opj_image->icc_profile_buf = (OPJ_BYTE*)icc_copy;
        jpeg2000_state->opj_image->icc_profile_len = (OPJ_UINT32)image->iccp->size;

        SAIL_LOG_TRACE("JPEG2000: ICC profile will be saved (%u bytes)", image->iccp->size);
    }
#endif

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_jpeg2000(void* state, const struct sail_image* image)
{

    struct jpeg2000_state* jpeg2000_state = state;
    opj_image_t* opj_image                = jpeg2000_state->opj_image;

    /* Copy pixels from sail_image to opj_image components. */
    for (OPJ_UINT32 row = 0; row < image->height; row++)
    {
        if (jpeg2000_state->channel_depth_scaled == 8)
        {
            const unsigned char* scan = sail_scan_line(image, row);

            for (OPJ_UINT32 column = 0; column < image->width; column++)
            {
                for (OPJ_UINT32 comp = 0; comp < opj_image->numcomps; comp++)
                {
                    opj_image->comps[comp].data[row * image->width + column] = *scan++;
                }
            }
        }
        else
        {
            const uint16_t* scan = sail_scan_line(image, row);

            for (OPJ_UINT32 column = 0; column < image->width; column++)
            {
                for (OPJ_UINT32 comp = 0; comp < opj_image->numcomps; comp++)
                {
                    opj_image->comps[comp].data[row * image->width + column] = *scan++;
                }
            }
        }
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_jpeg2000(void** state)
{

    struct jpeg2000_state* jpeg2000_state = *state;

    *state = NULL;

    /* Create encoder. */
    if (jpeg2000_state->opj_codec == NULL)
    {
        jpeg2000_state->opj_codec = opj_create_compress(OPJ_CODEC_JP2);

        if (jpeg2000_state->opj_codec == NULL)
        {
            SAIL_LOG_ERROR("JPEG2000: Failed to create encoder");
            destroy_jpeg2000_state(jpeg2000_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        /* Set up error handlers. */
        opj_set_error_handler(jpeg2000_state->opj_codec, jpeg2000_private_error_callback, NULL);
        opj_set_warning_handler(jpeg2000_state->opj_codec, jpeg2000_private_warning_callback, NULL);
        opj_set_info_handler(jpeg2000_state->opj_codec, jpeg2000_private_info_callback, NULL);
    }

    /* Setup encoder parameters. */
    opj_cparameters_t parameters;
    opj_set_default_encoder_parameters(&parameters);

    /* Set compression level. */
    const double compression_level = jpeg2000_state->save_options->compression_level;

    if (compression_level > 0 && compression_level <= 100)
    {
        /* Lossy compression with quality. */
        parameters.tcp_numlayers  = 1;
        parameters.tcp_rates[0]   = 100.0 / compression_level;
        parameters.cp_disto_alloc = 1;
    }
    else
    {
        /* Lossless compression (default). */
        parameters.tcp_numlayers  = 1;
        parameters.tcp_rates[0]   = 0;
        parameters.cp_disto_alloc = 1;
        parameters.irreversible   = 0;
    }

    /* Handle tuning. */
    if (jpeg2000_state->save_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(jpeg2000_state->save_options->tuning,
                                              jpeg2000_private_tuning_key_value_callback_save, &parameters);
    }

    SAIL_LOG_TRACE("JPEG2000: Setting up encoder (codec=%p, image=%p, stream=%p)...", jpeg2000_state->opj_codec,
                   jpeg2000_state->opj_image, jpeg2000_state->opj_stream);

    if (!opj_setup_encoder(jpeg2000_state->opj_codec, &parameters, jpeg2000_state->opj_image))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to setup encoder");
        destroy_jpeg2000_state(jpeg2000_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    /* Encode the image. */
    if (!opj_start_compress(jpeg2000_state->opj_codec, jpeg2000_state->opj_image, jpeg2000_state->opj_stream))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to start compression");
        destroy_jpeg2000_state(jpeg2000_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (!opj_encode(jpeg2000_state->opj_codec, jpeg2000_state->opj_stream))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to encode image");
        destroy_jpeg2000_state(jpeg2000_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    if (!opj_end_compress(jpeg2000_state->opj_codec, jpeg2000_state->opj_stream))
    {
        SAIL_LOG_ERROR("JPEG2000: Failed to end compression");
        destroy_jpeg2000_state(jpeg2000_state);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    destroy_jpeg2000_state(jpeg2000_state);

    return SAIL_OK;
}
