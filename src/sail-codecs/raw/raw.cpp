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

#include <cstring>
#include <memory>
#include <vector>

#include <libraw/libraw.h>

#include <sail-common/sail-common.h>

#include "datastream.h"
#include "helpers.h"

namespace
{

void raw_exif_parser_callback(void* context, int tag, int type, int len, unsigned ord, void* ifp, INT64 base)
{
    std::vector<unsigned char>* exif_data = static_cast<std::vector<unsigned char>*>(context);

    if (exif_data == NULL || ifp == NULL)
    {
        return;
    }

    (void)tag;
    (void)type;
    (void)len;
    (void)ord;
    (void)ifp;
    (void)base;
}

} // namespace

/*
 * Codec-specific state.
 */
struct raw_state
{
    struct sail_io* io;
    const struct sail_load_options* load_options;
    const struct sail_save_options* save_options;

    std::unique_ptr<LibRaw> raw_processor;
    libraw_processed_image_t* processed_image;
    std::unique_ptr<LibRaw_abstract_datastream> datastream;
    std::vector<unsigned char> exif_data;
    bool frame_processed;
};

static sail_status_t alloc_raw_state(struct sail_io* io,
                                     const struct sail_load_options* load_options,
                                     const struct sail_save_options* save_options,
                                     struct raw_state** raw_state_ptr)
{
    *raw_state_ptr = new raw_state{
        io,                                              // io
        load_options,                                    // load_options
        save_options,                                    // save_options
        std::unique_ptr<LibRaw>(),                       // raw_processor
        nullptr,                                         // processed_image
        std::unique_ptr<sail::raw::SailRawDatastream>(), // datastream
        {},                                              // exif_data
        false                                            // frame_processed
    };

    return SAIL_OK;
}

static void destroy_raw_state(struct raw_state* raw_state)
{
    if (raw_state == NULL)
    {
        return;
    }

    if (raw_state->processed_image != NULL)
    {
        libraw_dcraw_clear_mem(raw_state->processed_image);
    }

    delete raw_state;
}

/*
 * Decoding functions.
 */

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_init_v8_raw(struct sail_io* io,
                                                                 const struct sail_load_options* load_options,
                                                                 void** state)
{
    *state = NULL;

    struct raw_state* raw_state;
    SAIL_TRY(alloc_raw_state(io, load_options, NULL, &raw_state));
    *state = raw_state;

    raw_state->raw_processor.reset(new LibRaw());

    if (raw_state->raw_processor == NULL)
    {
        SAIL_LOG_ERROR("RAW: Failed to initialize LibRaw");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    // Set EXIF parser callback to collect EXIF data.
    raw_state->raw_processor->set_exifparser_handler(raw_exif_parser_callback, &raw_state->exif_data);

    // Handle tuning options.
    if (load_options != NULL && load_options->tuning != NULL)
    {
        sail_traverse_hash_map_with_user_data(load_options->tuning, raw_private_tuning_key_value_callback,
                                              raw_state->raw_processor.get());
    }

    raw_state->datastream.reset(new sail::raw::SailRawDatastream(io));

    if (raw_state->datastream == NULL || raw_state->datastream->valid() == 0)
    {
        SAIL_LOG_ERROR("RAW: Failed to create datastream");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    int ret;

    if (ret = raw_state->raw_processor->open_datastream(raw_state->datastream.get()); ret != LIBRAW_SUCCESS)
    {
        SAIL_LOG_ERROR("RAW: %s", libraw_strerror(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }
    if (ret = raw_state->raw_processor->unpack(); ret != LIBRAW_SUCCESS)
    {
        SAIL_LOG_ERROR("RAW: %s", libraw_strerror(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }
    if (ret = raw_state->raw_processor->dcraw_process(); ret != LIBRAW_SUCCESS)
    {
        SAIL_LOG_ERROR("RAW: %s", libraw_strerror(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    raw_state->processed_image = raw_state->raw_processor->dcraw_make_mem_image(&ret);

    if (raw_state->processed_image == NULL)
    {
        SAIL_LOG_ERROR("RAW: %s", libraw_strerror(ret));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
    }

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_raw(void* state, struct sail_image** image)
{
    struct raw_state* raw_state = static_cast<struct raw_state*>(state);

    if (raw_state->frame_processed)
    {
        return SAIL_ERROR_NO_MORE_FRAMES;
    }

    raw_state->frame_processed = true;

    struct sail_image* image_local;
    SAIL_TRY(sail_alloc_image(&image_local));

    if (raw_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    libraw_processed_image_t* processed = raw_state->processed_image;

    image_local->width  = processed->width;
    image_local->height = processed->height;

    unsigned bits_per_pixel = processed->bits;
    unsigned colors         = processed->colors;

    image_local->pixel_format = raw_private_libraw_to_pixel_format(colors, bits_per_pixel);

    if (image_local->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
    {
        SAIL_LOG_ERROR("RAW: Unsupported pixel format: %u colors, %u bits per pixel", colors, bits_per_pixel);
        sail_destroy_image(image_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    image_local->bytes_per_line = sail_bytes_per_line(image_local->width, image_local->pixel_format);

    if (raw_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE)
    {
        image_local->source_image->pixel_format = image_local->pixel_format;

        if (image_local->source_image->special_properties == NULL)
        {
            SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->source_image->special_properties),
                                /* cleanup */ sail_destroy_image(image_local));
        }

        libraw_data_t* raw_data = &raw_state->raw_processor->imgdata;
        SAIL_TRY_OR_CLEANUP(raw_private_store_special_properties(raw_data, image_local->source_image->special_properties),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    if (raw_state->load_options->options & SAIL_OPTION_META_DATA)
    {
        libraw_data_t* raw_data = &raw_state->raw_processor->imgdata;
        SAIL_TRY_OR_CLEANUP(raw_private_fetch_meta_data(raw_data, &image_local->meta_data_node, raw_state->exif_data),
                            /* cleanup */ sail_destroy_image(image_local));
    }

    *image = image_local;

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_raw(void* state, struct sail_image* image)
{
    struct raw_state* raw_state = static_cast<struct raw_state*>(state);

    libraw_processed_image_t* processed = raw_state->processed_image;

    unsigned bytes_per_pixel = processed->colors * (processed->bits / 8);
    unsigned row_size        = processed->width * bytes_per_pixel;

    const unsigned char* src = (const unsigned char*)processed->data;
    unsigned char* dst       = (unsigned char*)image->pixels;

    for (unsigned row = 0; row < image->height; row++)
    {
        memcpy(dst, src, row_size);
        src += row_size;
        dst += image->bytes_per_line;
    }

    return SAIL_OK;
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_raw(void** state)
{
    struct raw_state* raw_state_local = static_cast<struct raw_state*>(*state);

    *state = NULL;

    destroy_raw_state(raw_state_local);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_init_v8_raw(struct sail_io* io,
                                                                 const struct sail_save_options* save_options,
                                                                 void** state)
{
    (void)io;
    (void)save_options;
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_raw(void* state, const struct sail_image* image)
{
    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_raw(void* state, const struct sail_image* image)
{
    (void)state;
    (void)image;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

extern "C" SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_raw(void** state)
{
    (void)state;

    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}
