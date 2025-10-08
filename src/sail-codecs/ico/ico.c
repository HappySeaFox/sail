/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "common/bmp/bmp.h"

#include "helpers.h"

#define SAIL_ICO_TYPE_ICO 1
#define SAIL_ICO_TYPE_CUR 2

/* Maximum number of ICO frames that can be saved. */
static const unsigned SAIL_ICO_MAX_RESERVED_IMAGES = 64;

/*
 * Codec-specific state.
 */
struct ico_state {
    struct sail_io *io;
    const struct sail_load_options *load_options;
    const struct sail_save_options *save_options;

    struct SailIcoHeader ico_header;
    struct SailIcoDirEntry *ico_dir_entries;
    unsigned current_frame;

    void *common_bmp_state;

    /* For saving. */
    size_t *frame_data_offsets;
    size_t *frame_data_sizes;
    unsigned frames_to_save;
    size_t directory_size_reserved;
};

static sail_status_t alloc_ico_state(struct sail_io *io,
                                        const struct sail_load_options *load_options,
                                        const struct sail_save_options *save_options,
                                        struct ico_state **ico_state) {

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct ico_state), &ptr));
    *ico_state = ptr;

    **ico_state = (struct ico_state) {
        .io           = io,
        .load_options = load_options,
        .save_options = save_options,

        .ico_dir_entries  = NULL,
        .current_frame    = 0,
        .common_bmp_state = NULL,

        .frame_data_offsets = NULL,
        .frame_data_sizes   = NULL,
        .frames_to_save     = 0,
        .directory_size_reserved = 0,
    };

    return SAIL_OK;
}

static void destroy_ico_state(struct ico_state *ico_state) {

    if (ico_state == NULL) {
        return;
    }

    sail_free(ico_state->ico_dir_entries);
    sail_free(ico_state->frame_data_offsets);
    sail_free(ico_state->frame_data_sizes);

    sail_free(ico_state);
}

/*
 * Decoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_load_init_v8_ico(struct sail_io *io, const struct sail_load_options *load_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct ico_state *ico_state;
    SAIL_TRY(alloc_ico_state(io, load_options, NULL, &ico_state));
    *state = ico_state;

    SAIL_TRY(ico_private_read_header(ico_state->io, &ico_state->ico_header));

    if (ico_state->ico_header.images_count == 0) {
        SAIL_LOG_ERROR("ICO: No images found");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    /* Check the image type. */
    switch (ico_state->ico_header.type) {
        case SAIL_ICO_TYPE_ICO:
        case SAIL_ICO_TYPE_CUR: break;
        default: {
            SAIL_LOG_ERROR("ICO: Invalid image type %u", ico_state->ico_header.type);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct SailIcoDirEntry) * ico_state->ico_header.images_count, &ptr));
    ico_state->ico_dir_entries = ptr;

    for (unsigned i = 0; i < ico_state->ico_header.images_count; i++) {
        SAIL_TRY(ico_private_read_dir_entry(ico_state->io, &ico_state->ico_dir_entries[i]));
    }

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_seek_next_frame_v8_ico(void *state, struct sail_image **image) {

    struct ico_state *ico_state = state;

    /* Skip non-BMP images. */
    enum SailIcoImageType ico_image_type;

    do {
        if (ico_state->current_frame >= ico_state->ico_header.images_count) {
            SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
        }

        SAIL_TRY(ico_state->io->seek(ico_state->io->stream, (long)ico_state->ico_dir_entries[ico_state->current_frame++].image_offset, SEEK_SET));

        /* Check the image is not PNG. */
        SAIL_TRY(ico_private_probe_image_type(ico_state->io, &ico_image_type));
    } while (ico_image_type != SAIL_ICO_IMAGE_BMP);

    /* Continue to loading BMP. */
    struct sail_image *image_local;

    SAIL_TRY(bmp_private_read_init(ico_state->io, ico_state->load_options, &ico_state->common_bmp_state, SAIL_NO_BMP_FLAGS));
    SAIL_TRY(bmp_private_read_seek_next_frame(ico_state->common_bmp_state, ico_state->io, &image_local));

    /* Store CUR hotspot. */
    if (ico_state->load_options->options & SAIL_OPTION_META_DATA) {
        if (ico_state->ico_header.type == SAIL_ICO_TYPE_CUR) {
            SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                                /* cleanup */ sail_destroy_image(image_local));
            SAIL_TRY_OR_CLEANUP(ico_private_store_cur_hotspot(&ico_state->ico_dir_entries[ico_state->current_frame - 1],
                                                                image_local->special_properties),
                                /* cleanup */ sail_destroy_image(image_local));
        }
    }

    if (ico_state->load_options->options & SAIL_OPTION_SOURCE_IMAGE) {
        if (ico_state->load_options->options & SAIL_OPTION_META_DATA) {
            if (ico_state->ico_header.type == SAIL_ICO_TYPE_CUR) {
                if (image_local->source_image == NULL) {
                    SAIL_TRY_OR_CLEANUP(sail_alloc_source_image(&image_local->source_image),
                                        /* cleanup */ sail_destroy_image(image_local));
                }

                SAIL_TRY_OR_CLEANUP(sail_alloc_hash_map(&image_local->special_properties),
                                    /* cleanup */ sail_destroy_image(image_local));
                SAIL_TRY_OR_CLEANUP(ico_private_store_cur_hotspot(&ico_state->ico_dir_entries[ico_state->current_frame - 1],
                                                                    image_local->special_properties),
                                    /* cleanup */ sail_destroy_image(image_local));
            }
        }
    }

    /*
     * The contained image is twice the height declared in the directory.
     * The second half is a mask. We need just the image.
     */
    image_local->height /= 2;

    *image = image_local;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_frame_v8_ico(void *state, struct sail_image *image) {

    struct ico_state *ico_state = state;

    SAIL_TRY(bmp_private_read_frame(ico_state->common_bmp_state, ico_state->io, image));
    SAIL_TRY(bmp_private_read_finish(&ico_state->common_bmp_state, ico_state->io));

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_load_finish_v8_ico(void **state) {

    struct ico_state *ico_state = *state;

    *state = NULL;

    if (ico_state->common_bmp_state != NULL) {
        SAIL_TRY_OR_CLEANUP(bmp_private_read_finish(&ico_state->common_bmp_state, ico_state->io),
                            /* cleanup */ destroy_ico_state(ico_state));
    }

    destroy_ico_state(ico_state);

    return SAIL_OK;
}

/*
 * Encoding functions.
 */

SAIL_EXPORT sail_status_t sail_codec_save_init_v8_ico(struct sail_io *io, const struct sail_save_options *save_options, void **state) {

    *state = NULL;

    /* Allocate a new state. */
    struct ico_state *ico_state;
    SAIL_TRY(alloc_ico_state(io, NULL, save_options, &ico_state));
    *state = ico_state;

    /* ICO files start with a header. We'll write it in save_finish after collecting all frames. */
    ico_state->frames_to_save = 0;
    ico_state->current_frame = 0;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_seek_next_frame_v8_ico(void *state, const struct sail_image *image) {

    struct ico_state *ico_state = state;

    /* Validate image size. ICO dimensions are stored as uint8_t (0 means 256). */
    if (image->width > 256 || image->height > 256) {
        SAIL_LOG_ERROR("ICO: Image dimensions must be <= 256x256");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY);
    }

    bool is_cur = false;
    uint16_t hotspot_x = 0;
    uint16_t hotspot_y = 0;
    if (image->special_properties != NULL) {
        ico_private_fetch_cur_hotspot(image->special_properties, &hotspot_x, &hotspot_y);
        if (hotspot_x != 0 || hotspot_y != 0) {
            is_cur = true;
        }
    }

    if (ico_state->current_frame == 0) {
        ico_state->ico_header.type = is_cur ? SAIL_ICO_TYPE_CUR : SAIL_ICO_TYPE_ICO;
    } else if (is_cur) {
        ico_state->ico_header.type = SAIL_ICO_TYPE_CUR;
    }

    if (ico_state->frames_to_save >= SAIL_ICO_MAX_RESERVED_IMAGES) {
        SAIL_LOG_ERROR("ICO: Too many frames. Maximum is %u", SAIL_ICO_MAX_RESERVED_IMAGES);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CONFLICTING_OPERATION);
    }

    ico_state->frames_to_save++;

    SAIL_TRY(sail_realloc(sizeof(struct SailIcoDirEntry) * ico_state->frames_to_save, (void **)&ico_state->ico_dir_entries));
    SAIL_TRY(sail_realloc(sizeof(size_t) * ico_state->frames_to_save, (void **)&ico_state->frame_data_offsets));
    SAIL_TRY(sail_realloc(sizeof(size_t) * ico_state->frames_to_save, (void **)&ico_state->frame_data_sizes));

    struct SailIcoDirEntry *dir_entry = &ico_state->ico_dir_entries[ico_state->current_frame];
    *dir_entry = (struct SailIcoDirEntry){ 0 };
    dir_entry->width = (image->width == 256) ? 0 : (uint8_t)image->width;
    dir_entry->height = (image->height == 256) ? 0 : (uint8_t)image->height;

    /* For CUR files, planes and bit_count store hotspot coordinates. For ICO, they store color info. */
    if (is_cur) {
        dir_entry->planes = hotspot_x;
        dir_entry->bit_count = hotspot_y;
    } else {
        dir_entry->planes = 1;
        dir_entry->bit_count = (uint16_t)sail_bits_per_pixel(image->pixel_format);
    }

    if (ico_state->current_frame == 0) {
        /* Write header with placeholder count. */
        struct SailIcoHeader header = {
            .reserved = 0,
            .type = ico_state->ico_header.type,
            .images_count = 0,
        };
        SAIL_TRY(ico_private_write_header(ico_state->io, &header));

        /* Reserve space for maximum directory entries (64 * 16 = 1024 bytes). */
        ico_state->directory_size_reserved = SAIL_ICO_MAX_RESERVED_IMAGES * sizeof(struct SailIcoDirEntry);

        unsigned char *zero_buffer;
        void *ptr;
        SAIL_TRY(sail_malloc(ico_state->directory_size_reserved, &ptr));
        zero_buffer = ptr;
        memset(zero_buffer, 0, ico_state->directory_size_reserved);

        SAIL_TRY_OR_CLEANUP(ico_state->io->strict_write(ico_state->io->stream, zero_buffer, ico_state->directory_size_reserved),
                            /* cleanup */ sail_free(zero_buffer));
        sail_free(zero_buffer);
    }

    /* Remember where image data starts. */
    SAIL_TRY(ico_state->io->tell(ico_state->io->stream, &ico_state->frame_data_offsets[ico_state->current_frame]));

    /* Initialize BMP writer (without file header, as ICO contains raw BMP data). */
    SAIL_TRY(bmp_private_write_init(ico_state->io, ico_state->save_options, &ico_state->common_bmp_state, SAIL_NO_BMP_WRITE_FLAGS));

    /* Double the height for ICO (includes AND mask). */
    struct sail_image *image_doubled;
    SAIL_TRY(sail_alloc_image(&image_doubled));
    *image_doubled = *image;
    image_doubled->height *= 2;

    SAIL_TRY_OR_CLEANUP(bmp_private_write_seek_next_frame(ico_state->common_bmp_state, ico_state->io, image_doubled),
                        /* cleanup */ sail_destroy_image(image_doubled));

    sail_free(image_doubled);

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_frame_v8_ico(void *state, const struct sail_image *image) {

    struct ico_state *ico_state = state;

    SAIL_TRY(bmp_private_write_frame(ico_state->common_bmp_state, ico_state->io, image));

    /* Write AND mask (all zeros for now, no transparency). */
    const unsigned mask_bytes_per_line = ((image->width + 31) / 32) * 4;
    unsigned char *mask_line;
    void *ptr;
    SAIL_TRY(sail_malloc(mask_bytes_per_line, &ptr));
    mask_line = ptr;
    memset(mask_line, 0, mask_bytes_per_line);

    for (unsigned i = 0; i < image->height; i++) {
        SAIL_TRY_OR_CLEANUP(ico_state->io->strict_write(ico_state->io->stream, mask_line, mask_bytes_per_line),
                            /* cleanup */ sail_free(mask_line));
    }

    sail_free(mask_line);

    SAIL_TRY(bmp_private_write_finish(&ico_state->common_bmp_state, ico_state->io));

    size_t current_offset;
    SAIL_TRY(ico_state->io->tell(ico_state->io->stream, &current_offset));
    ico_state->frame_data_sizes[ico_state->current_frame] = current_offset - ico_state->frame_data_offsets[ico_state->current_frame];

    ico_state->current_frame++;

    return SAIL_OK;
}

SAIL_EXPORT sail_status_t sail_codec_save_finish_v8_ico(void **state) {

    struct ico_state *ico_state = *state;

    *state = NULL;

    if (ico_state->frames_to_save > 0) {
        /* Go back and update header with correct frame count. */
        SAIL_TRY_OR_CLEANUP(ico_state->io->seek(ico_state->io->stream, 0, SEEK_SET),
                            /* cleanup */ destroy_ico_state(ico_state));

        struct SailIcoHeader header = {
            .reserved = 0,
            .type = ico_state->ico_header.type,
            .images_count = (uint16_t)ico_state->frames_to_save,
        };
        SAIL_TRY_OR_CLEANUP(ico_private_write_header(ico_state->io, &header),
                            /* cleanup */ destroy_ico_state(ico_state));

        /* Update directory entries with correct offsets and sizes. */
        for (unsigned i = 0; i < ico_state->frames_to_save; i++) {
            /* Update directory entry with actual offset and size. */
            ico_state->ico_dir_entries[i].image_offset = (uint32_t)ico_state->frame_data_offsets[i];
            ico_state->ico_dir_entries[i].image_size = (uint32_t)ico_state->frame_data_sizes[i];

            SAIL_TRY_OR_CLEANUP(ico_state->io->seek(ico_state->io->stream, (long)(6 + i * 16), SEEK_SET),
                                /* cleanup */ destroy_ico_state(ico_state));
            SAIL_TRY_OR_CLEANUP(ico_private_write_dir_entry(ico_state->io, &ico_state->ico_dir_entries[i]),
                                /* cleanup */ destroy_ico_state(ico_state));
        }
    }

    destroy_ico_state(ico_state);

    return SAIL_OK;
}
