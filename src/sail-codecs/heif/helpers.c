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

#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

enum SailPixelFormat heif_private_sail_pixel_format_from_heif(enum heif_chroma chroma,
                                                              enum heif_channel channel,
                                                              int bits_per_pixel)
{
    /* Check for monochrome/grayscale images */
    if (channel == heif_channel_Y && chroma == heif_chroma_monochrome)
    {
        switch (bits_per_pixel)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }

    /* Check for RGB formats */
    switch (chroma)
    {
    case heif_chroma_interleaved_RGB:
    {
        return bits_per_pixel == 8 ? SAIL_PIXEL_FORMAT_BPP24_RGB : SAIL_PIXEL_FORMAT_BPP48_RGB;
    }
    case heif_chroma_interleaved_RGBA:
    {
        return bits_per_pixel == 8 ? SAIL_PIXEL_FORMAT_BPP32_RGBA : SAIL_PIXEL_FORMAT_BPP64_RGBA;
    }
    case heif_chroma_interleaved_RRGGBB_BE:
    case heif_chroma_interleaved_RRGGBB_LE:
    {
        return SAIL_PIXEL_FORMAT_BPP48_RGB;
    }
    case heif_chroma_interleaved_RRGGBBAA_BE:
    case heif_chroma_interleaved_RRGGBBAA_LE:
    {
        return SAIL_PIXEL_FORMAT_BPP64_RGBA;
    }
    /* YUV formats */
    case heif_chroma_420:
    case heif_chroma_422:
    case heif_chroma_444:
    {
        switch (bits_per_pixel)
        {
        case 8: return SAIL_PIXEL_FORMAT_BPP24_YUV;
        case 10: return SAIL_PIXEL_FORMAT_BPP30_YUV;
        case 12: return SAIL_PIXEL_FORMAT_BPP36_YUV;
        case 16: return SAIL_PIXEL_FORMAT_BPP48_YUV;
        default: return SAIL_PIXEL_FORMAT_UNKNOWN;
        }
    }
    case heif_chroma_monochrome:
    {
        return bits_per_pixel == 8 ? SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE : SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
    }
    default:
    {
        return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
    }
}

bool heif_private_heif_chroma_from_sail_pixel_format(enum SailPixelFormat pixel_format,
                                                     enum heif_chroma* chroma,
                                                     int* bits_per_component,
                                                     bool* has_alpha)
{
    *has_alpha = false;

    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:
    {
        *chroma             = heif_chroma_monochrome;
        *bits_per_component = 8;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:
    {
        *chroma             = heif_chroma_monochrome;
        *bits_per_component = 16;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        *chroma             = heif_chroma_interleaved_RGB;
        *bits_per_component = 8;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        *chroma             = heif_chroma_interleaved_RGBA;
        *bits_per_component = 8;
        *has_alpha          = true;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP48_RGB:
    {
        *chroma             = heif_chroma_interleaved_RRGGBB_BE;
        *bits_per_component = 16;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP64_RGBA:
    {
        *chroma             = heif_chroma_interleaved_RRGGBBAA_BE;
        *bits_per_component = 16;
        *has_alpha          = true;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP24_YUV:
    {
        *chroma             = heif_chroma_444;
        *bits_per_component = 8;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP30_YUV:
    {
        *chroma             = heif_chroma_444;
        *bits_per_component = 10;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP36_YUV:
    {
        *chroma             = heif_chroma_444;
        *bits_per_component = 12;
        return true;
    }
    case SAIL_PIXEL_FORMAT_BPP48_YUV:
    {
        *chroma             = heif_chroma_444;
        *bits_per_component = 16;
        return true;
    }
    default:
    {
        return false;
    }
    }
}

sail_status_t heif_private_fetch_iccp(struct heif_image_handle* handle, struct sail_iccp** iccp)
{
    SAIL_CHECK_PTR(handle);
    SAIL_CHECK_PTR(iccp);

    size_t profile_size = heif_image_handle_get_raw_color_profile_size(handle);

    if (profile_size > 0)
    {
        void* profile_data;
        SAIL_TRY(sail_malloc(profile_size, &profile_data));

        struct heif_error error = heif_image_handle_get_raw_color_profile(handle, profile_data);

        if (error.code != heif_error_Ok)
        {
            sail_free(profile_data);
            SAIL_LOG_ERROR("HEIF: Failed to get ICC profile: %s", error.message);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        SAIL_TRY_OR_CLEANUP(sail_alloc_iccp_from_data(profile_data, profile_size, iccp),
                            /* cleanup */ sail_free(profile_data));

        sail_free(profile_data);
        SAIL_LOG_TRACE("HEIF: Found ICC profile %u bytes long", (unsigned)profile_size);
    }
    else
    {
        SAIL_LOG_TRACE("HEIF: ICC profile is not found");
    }

    return SAIL_OK;
}

sail_status_t heif_private_fetch_meta_data(struct heif_image_handle* handle,
                                           struct sail_meta_data_node** last_meta_data_node)
{
    SAIL_CHECK_PTR(handle);
    SAIL_CHECK_PTR(last_meta_data_node);

    /* Fetch EXIF */
    int num_metadata = heif_image_handle_get_number_of_metadata_blocks(handle, "Exif");

    if (num_metadata > 0)
    {
        heif_item_id metadata_id;
        heif_image_handle_get_list_of_metadata_block_IDs(handle, "Exif", &metadata_id, 1);

        size_t metadata_size = heif_image_handle_get_metadata_size(handle, metadata_id);

        if (metadata_size > 0)
        {
            void* metadata;
            SAIL_TRY(sail_malloc(metadata_size, &metadata));

            struct heif_error error = heif_image_handle_get_metadata(handle, metadata_id, metadata);

            if (error.code == heif_error_Ok)
            {
                struct sail_meta_data_node* meta_data_node_local;
                SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                                    /* cleanup */ sail_free(metadata));

                SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_EXIF,
                                                                                  &meta_data_node_local->meta_data),
                                    /* cleanup */ sail_free(metadata), sail_destroy_meta_data_node(meta_data_node_local));

                SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node_local->meta_data->value, metadata, metadata_size),
                    /* cleanup */ sail_free(metadata), sail_destroy_meta_data_node(meta_data_node_local));

                *last_meta_data_node = meta_data_node_local;
                last_meta_data_node  = &meta_data_node_local->next;

                SAIL_LOG_TRACE("HEIF: Found EXIF metadata %u bytes long", (unsigned)metadata_size);
            }

            sail_free(metadata);
        }
    }

    /* Fetch XMP */
    num_metadata = heif_image_handle_get_number_of_metadata_blocks(handle, "mime");

    for (int i = 0; i < num_metadata; i++)
    {
        heif_item_id metadata_id;
        heif_image_handle_get_list_of_metadata_block_IDs(handle, "mime", &metadata_id, 1);

        const char* content_type = heif_image_handle_get_metadata_content_type(handle, metadata_id);

        if (content_type != NULL && strcmp(content_type, "application/rdf+xml") == 0)
        {
            size_t metadata_size = heif_image_handle_get_metadata_size(handle, metadata_id);

            if (metadata_size > 0)
            {
                void* metadata;
                SAIL_TRY(sail_malloc(metadata_size, &metadata));

                struct heif_error error = heif_image_handle_get_metadata(handle, metadata_id, metadata);

                if (error.code == heif_error_Ok)
                {
                    struct sail_meta_data_node* meta_data_node_local;
                    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node_local),
                                        /* cleanup */ sail_free(metadata));

                    SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_XMP,
                                                                                      &meta_data_node_local->meta_data),
                                        /* cleanup */ sail_free(metadata), sail_destroy_meta_data_node(meta_data_node_local));

                    SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node_local->meta_data->value, metadata, metadata_size),
                        /* cleanup */ sail_free(metadata), sail_destroy_meta_data_node(meta_data_node_local));

                    *last_meta_data_node = meta_data_node_local;

                    SAIL_LOG_TRACE("HEIF: Found XMP metadata %u bytes long", (unsigned)metadata_size);
                }

                sail_free(metadata);
            }
            break;
        }
    }

    return SAIL_OK;
}

sail_status_t heif_private_write_iccp(struct heif_image* image, const struct sail_iccp* iccp)
{
    SAIL_CHECK_PTR(image);

    if (iccp != NULL && iccp->data != NULL)
    {
        struct heif_error error = heif_image_set_raw_color_profile(image, "prof", iccp->data, iccp->size);

        if (error.code != heif_error_Ok)
        {
            SAIL_LOG_ERROR("HEIF: Failed to set ICC profile: %s", error.message);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }

        SAIL_LOG_TRACE("HEIF: ICC profile has been written");
    }

    return SAIL_OK;
}

sail_status_t heif_private_write_meta_data(struct heif_context* ctx,
                                           struct heif_image_handle* handle,
                                           const struct sail_meta_data_node* meta_data_node)
{
    SAIL_CHECK_PTR(ctx);
    SAIL_CHECK_PTR(handle);

    for (const struct sail_meta_data_node* node = meta_data_node; node != NULL; node = node->next)
    {
        if (node->meta_data->key == SAIL_META_DATA_EXIF && node->meta_data->value->type == SAIL_VARIANT_TYPE_DATA)
        {
            struct heif_error error = heif_context_add_exif_metadata(ctx, handle, node->meta_data->value->value,
                                                                     (int)node->meta_data->value->size);

            if (error.code != heif_error_Ok)
            {
                SAIL_LOG_ERROR("HEIF: Failed to set EXIF: %s", error.message);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            SAIL_LOG_TRACE("HEIF: EXIF has been written");
        }
        else if (node->meta_data->key == SAIL_META_DATA_XMP && node->meta_data->value->type == SAIL_VARIANT_TYPE_DATA)
        {
            struct heif_error error = heif_context_add_XMP_metadata(ctx, handle, node->meta_data->value->value,
                                                                    (int)node->meta_data->value->size);

            if (error.code != heif_error_Ok)
            {
                SAIL_LOG_ERROR("HEIF: Failed to set XMP: %s", error.message);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
            }

            SAIL_LOG_TRACE("HEIF: XMP has been written");
        }
    }

    return SAIL_OK;
}

/* Append to helpers.c - Specialized Properties and Tuning Functions */

#include <string.h>

sail_status_t heif_private_fetch_depth_info(const struct heif_image_handle* image_handle,
                                            struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(image_handle);

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    int has_depth = heif_image_handle_has_depth_image(image_handle);

    if (has_depth)
    {
        SAIL_TRY(sail_put_hash_map_bool(special_properties, "heif-has-depth", true));

        /* Get depth image count */
        int depth_count = heif_image_handle_get_number_of_depth_images(image_handle);
        if (depth_count > 0)
        {
            SAIL_TRY(sail_put_hash_map_int(special_properties, "heif-depth-count", depth_count));
        }
    }

    return SAIL_OK;
}

sail_status_t heif_private_fetch_thumbnail_info(const struct heif_image_handle* image_handle,
                                                struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(image_handle);

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    int thumbnail_count = heif_image_handle_get_number_of_thumbnails(image_handle);

    if (thumbnail_count > 0)
    {
        SAIL_TRY(sail_put_hash_map_int(special_properties, "heif-thumbnail-count", thumbnail_count));
    }

    return SAIL_OK;
}

sail_status_t heif_private_fetch_primary_flag(const struct heif_image_handle* image_handle,
                                              struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(image_handle);

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    int is_primary = heif_image_handle_is_primary_image(image_handle);

    if (is_primary)
    {
        SAIL_TRY(sail_put_hash_map_bool(special_properties, "heif-is-primary", true));
    }

    return SAIL_OK;
}

sail_status_t heif_private_fetch_hdr_metadata(const struct heif_image* heif_image,
                                              struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(heif_image);

    if (special_properties == NULL)
    {
        return SAIL_OK;
    }

    struct heif_content_light_level cll;
    heif_image_get_content_light_level(heif_image, &cll);

    if (cll.max_content_light_level > 0 || cll.max_pic_average_light_level > 0)
    {
        if (cll.max_content_light_level > 0)
        {
            SAIL_TRY(sail_put_hash_map_unsigned_int(special_properties, "heif-content-light-level-max",
                                                    cll.max_content_light_level));
        }

        if (cll.max_pic_average_light_level > 0)
        {
            SAIL_TRY(sail_put_hash_map_unsigned_int(special_properties, "heif-content-light-level-avg",
                                                    cll.max_pic_average_light_level));
        }
    }

    return SAIL_OK;
}

sail_status_t heif_private_fetch_premultiplied_alpha(const struct heif_image* heif_image,
                                                     struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(heif_image);

    /* Note: libheif does not provide heif_image_get_premultiplied_alpha() getter function.
     * Only heif_image_set_premultiplied_alpha() setter is available.
     * This information is not available for reading from decoded images.
     */
    (void)special_properties;

    return SAIL_OK;
}

/* Helper function to read int/unsigned/float/double values from variant. */
static int heif_private_variant_to_int(const struct sail_variant* value)
{
    switch (value->type)
    {
    case SAIL_VARIANT_TYPE_INT: return sail_variant_to_int(value);
    case SAIL_VARIANT_TYPE_UNSIGNED_INT: return (int)sail_variant_to_unsigned_int(value);
    case SAIL_VARIANT_TYPE_FLOAT: return (int)sail_variant_to_float(value);
    case SAIL_VARIANT_TYPE_DOUBLE: return (int)sail_variant_to_double(value);
    default: return 0;
    }
}

/* Tuning options callback. */
bool heif_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct heif_tuning_state* tuning_state = (struct heif_tuning_state*)user_data;

    /* heif-preset: encoding speed preset */
    if (strcmp(key, "heif-preset") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* preset = sail_variant_to_string(value);

            /* Validate preset value */
            const char* valid_presets[] = {"ultrafast", "superfast", "veryfast", "faster",  "fast", "medium",
                                           "slow",      "slower",    "veryslow", "placebo", NULL};

            bool valid = false;
            for (int i = 0; valid_presets[i] != NULL; i++)
            {
                if (strcmp(preset, valid_presets[i]) == 0)
                {
                    valid = true;
                    break;
                }
            }

            if (valid)
            {
                heif_encoder_set_parameter_string(tuning_state->encoder, "preset", preset);
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: Invalid 'heif-preset' value '%s'. Valid values: ultrafast, superfast, veryfast, "
                               "faster, fast, medium, slow, slower, veryslow, placebo",
                               preset);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-preset' must be a string");
        }
        return true;
    }

    /* heif-tune: optimization tune */
    else if (strcmp(key, "heif-tune") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* tune = sail_variant_to_string(value);

            /* Validate tune value */
            const char* valid_tunes[] = {"psnr", "ssim", "grain", "fastdecode", NULL};

            bool valid = false;
            for (int i = 0; valid_tunes[i] != NULL; i++)
            {
                if (strcmp(tune, valid_tunes[i]) == 0)
                {
                    valid = true;
                    break;
                }
            }

            if (valid)
            {
                heif_encoder_set_parameter_string(tuning_state->encoder, "tune", tune);
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: Invalid 'heif-tune' value '%s'. Valid values: psnr, ssim, grain, fastdecode",
                               tune);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-tune' must be a string");
        }
        return true;
    }

    /* heif-tu-intra-depth: Transform Unit intra depth (1-4) */
    else if (strcmp(key, "heif-tu-intra-depth") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT
            || value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            int tu_depth = heif_private_variant_to_int(value);

            if (tu_depth >= 1 && tu_depth <= 4)
            {
                heif_encoder_set_parameter_integer(tuning_state->encoder, "tu-intra-depth", tu_depth);
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: 'heif-tu-intra-depth' must be in range [1, 4], got %d", tu_depth);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-tu-intra-depth' must be a number");
        }
        return true;
    }

    /* heif-complexity: encoding complexity (0-100) */
    else if (strcmp(key, "heif-complexity") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT
            || value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            int complexity = heif_private_variant_to_int(value);

            if (complexity >= 0 && complexity <= 100)
            {
                heif_encoder_set_parameter_integer(tuning_state->encoder, "complexity", complexity);
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: 'heif-complexity' must be in range [0, 100], got %d", complexity);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-complexity' must be a number");
        }
        return true;
    }

    /* heif-chroma: chroma subsampling */
    else if (strcmp(key, "heif-chroma") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* chroma = sail_variant_to_string(value);

            /* Validate chroma value */
            const char* valid_chromas[] = {"420", "422", "444", NULL};

            bool valid = false;
            for (int i = 0; valid_chromas[i] != NULL; i++)
            {
                if (strcmp(chroma, valid_chromas[i]) == 0)
                {
                    valid = true;
                    break;
                }
            }

            if (valid)
            {
                heif_encoder_set_parameter_string(tuning_state->encoder, "chroma", chroma);
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: Invalid 'heif-chroma' value '%s'. Valid values: 420, 422, 444", chroma);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-chroma' must be a string");
        }
        return true;
    }

    /* heif-threads: number of decoding/encoding threads */
    else if (strcmp(key, "heif-threads") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT
            || value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            int threads = heif_private_variant_to_int(value);

            if (threads >= 1 && threads <= 256)
            {
                *(tuning_state->threads) = threads;
            }
            else
            {
                SAIL_LOG_ERROR("HEIF: 'heif-threads' must be in range [1, 256], got %d", threads);
            }
        }
        else
        {
            SAIL_LOG_ERROR("HEIF: 'heif-threads' must be a number");
        }
        return true;
    }

    return false;
}

sail_status_t heif_private_heif_error_to_sail_status(const struct heif_error* error)
{
    SAIL_CHECK_PTR(error);

    if (error->code == heif_error_Ok)
    {
        return SAIL_OK;
    }

    switch (error->subcode)
    {
    case heif_suberror_Unsupported_bit_depth: return SAIL_ERROR_UNSUPPORTED_BIT_DEPTH;

    case heif_suberror_Unsupported_codec:
    case heif_suberror_Unsupported_image_type:
    case heif_suberror_Unsupported_data_version:
    case heif_suberror_Unsupported_color_conversion:
    case heif_suberror_Unsupported_item_construction_method:
    case heif_suberror_Unsupported_parameter:
    case heif_suberror_Unsupported_header_compression_method: return SAIL_ERROR_UNSUPPORTED_FORMAT;

    case heif_suberror_Invalid_image_size: return SAIL_ERROR_INVALID_IMAGE_DIMENSIONS;

    case heif_suberror_End_of_data: return SAIL_ERROR_NO_MORE_FRAMES;

    case heif_suberror_Invalid_box_size:
    case heif_suberror_No_ftyp_box:
    case heif_suberror_No_idat_box:
    case heif_suberror_No_meta_box:
    case heif_suberror_No_hdlr_box:
    case heif_suberror_No_hvcC_box:
    case heif_suberror_No_pitm_box:
    case heif_suberror_No_ipco_box:
    case heif_suberror_No_ipma_box:
    case heif_suberror_No_iloc_box:
    case heif_suberror_No_iinf_box:
    case heif_suberror_No_iprp_box:
    case heif_suberror_No_iref_box:
    case heif_suberror_No_pict_handler:
    case heif_suberror_No_av1C_box: return SAIL_ERROR_INVALID_IMAGE;

    default: return SAIL_ERROR_UNDERLYING_CODEC;
    }
}
