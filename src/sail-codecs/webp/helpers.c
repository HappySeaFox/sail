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

#include <string.h>

#include <webp/encode.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

void webp_private_fill_color(uint8_t* pixels,
                             unsigned bytes_per_line,
                             unsigned bytes_per_pixel,
                             uint32_t color,
                             unsigned x,
                             unsigned y,
                             unsigned width,
                             unsigned height)
{
    uint8_t* scanline = pixels + y * bytes_per_line + x * bytes_per_pixel;

    for (unsigned row = 0; row < height; row++, scanline += bytes_per_line)
    {
        for (unsigned column = 0; column < width * bytes_per_pixel; column += bytes_per_pixel)
        {
            memcpy(scanline + column, &color, sizeof(color));
        }
    }
}

sail_status_t webp_private_blend_over(
    void* dst_raw, unsigned dst_offset, const void* src_raw, unsigned width, unsigned bytes_per_pixel)
{
    SAIL_CHECK_PTR(src_raw);
    SAIL_CHECK_PTR(dst_raw);

    if (bytes_per_pixel == 4)
    {
        const uint8_t* src = src_raw;
        uint8_t* dst       = (uint8_t*)dst_raw + dst_offset * bytes_per_pixel;

        while (width--)
        {
            const double src_a = *(src + 3) / 255.0;
            const double dst_a = *(dst + 3) / 255.0;

            *dst = (uint8_t)(src_a * (*src) + (1 - src_a) * dst_a * (*dst));
            src++;
            dst++;
            *dst = (uint8_t)(src_a * (*src) + (1 - src_a) * dst_a * (*dst));
            src++;
            dst++;
            *dst = (uint8_t)(src_a * (*src) + (1 - src_a) * dst_a * (*dst));
            src++;
            dst++;
            *dst = (uint8_t)((src_a + (1 - src_a) * dst_a) * 255);
            src++;
            dst++;
        }
    }
    else
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_BIT_DEPTH);
    }

    return SAIL_OK;
}

sail_status_t webp_private_fetch_iccp(WebPDemuxer* webp_demux, struct sail_iccp** iccp)
{
    SAIL_CHECK_PTR(webp_demux);
    SAIL_CHECK_PTR(iccp);

    const uint32_t webp_flags = WebPDemuxGetI(webp_demux, WEBP_FF_FORMAT_FLAGS);

    if (webp_flags & ICCP_FLAG)
    {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "ICCP", 1, &chunk_iterator))
        {
            SAIL_TRY_OR_CLEANUP(
                sail_alloc_iccp_from_data(chunk_iterator.chunk.bytes, (unsigned)chunk_iterator.chunk.size, iccp),
                /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            WebPDemuxReleaseChunkIterator(&chunk_iterator);
        }
    }

    return SAIL_OK;
}

sail_status_t webp_private_fetch_meta_data(WebPDemuxer* webp_demux, struct sail_meta_data_node** last_meta_data_node)
{
    SAIL_CHECK_PTR(webp_demux);
    SAIL_CHECK_PTR(last_meta_data_node);

    const uint32_t webp_flags = WebPDemuxGetI(webp_demux, WEBP_FF_FORMAT_FLAGS);

    if (webp_flags & XMP_FLAG)
    {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "XMP ", 1, &chunk_iterator))
        {
            struct sail_meta_data_node* meta_data_node;

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node),
                                /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(
                sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_XMP, &meta_data_node->meta_data),
                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_substring(meta_data_node->meta_data->value,
                                                           (const char*)chunk_iterator.chunk.bytes,
                                                           chunk_iterator.chunk.size),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                WebPDemuxReleaseChunkIterator(&chunk_iterator));

            WebPDemuxReleaseChunkIterator(&chunk_iterator);

            *last_meta_data_node = meta_data_node;
            last_meta_data_node  = &meta_data_node->next;
        }
    }

    if (webp_flags & EXIF_FLAG)
    {
        WebPChunkIterator chunk_iterator;

        if (WebPDemuxGetChunk(webp_demux, "EXIF", 1, &chunk_iterator))
        {
            struct sail_meta_data_node* meta_data_node;

            SAIL_TRY_OR_CLEANUP(sail_alloc_meta_data_node(&meta_data_node),
                                /* cleanup */ WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(
                sail_alloc_meta_data_and_value_from_known_key(SAIL_META_DATA_EXIF, &meta_data_node->meta_data),
                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                WebPDemuxReleaseChunkIterator(&chunk_iterator));
            SAIL_TRY_OR_CLEANUP(sail_set_variant_data(meta_data_node->meta_data->value, chunk_iterator.chunk.bytes,
                                                      chunk_iterator.chunk.size),
                                /* cleanup */ sail_destroy_meta_data_node(meta_data_node),
                                WebPDemuxReleaseChunkIterator(&chunk_iterator));

            WebPDemuxReleaseChunkIterator(&chunk_iterator);

            *last_meta_data_node = meta_data_node;
            last_meta_data_node  = &meta_data_node->next;
        }
    }

    return SAIL_OK;
}

sail_status_t webp_private_store_loop_count(WebPDemuxer* webp_demux, struct sail_hash_map* special_properties)
{
    SAIL_CHECK_PTR(webp_demux);
    SAIL_CHECK_PTR(special_properties);

    const uint32_t webp_flags = WebPDemuxGetI(webp_demux, WEBP_FF_FORMAT_FLAGS);

    /* Only set loop count for animated images. */
    if (webp_flags & ANIMATION_FLAG)
    {
        const uint32_t loop_count = WebPDemuxGetI(webp_demux, WEBP_FF_LOOP_COUNT);

        SAIL_LOG_TRACE("WEBP: Loop count: %u", loop_count);
        SAIL_TRY(sail_put_hash_map_unsigned_int(special_properties, "webp-loop-count", loop_count));
    }

    return SAIL_OK;
}

sail_status_t webp_private_supported_write_pixel_format(enum SailPixelFormat pixel_format)
{
    switch (pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    case SAIL_PIXEL_FORMAT_BPP24_BGR:
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
    case SAIL_PIXEL_FORMAT_BPP32_ARGB:
    case SAIL_PIXEL_FORMAT_BPP32_ABGR:
    {
        return SAIL_OK;
    }
    default:
    {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }
}

sail_status_t webp_private_convert_argb_to_rgba(
    const uint8_t* pixels, unsigned width, unsigned height, unsigned stride, void** rgba_pixels)
{
    SAIL_CHECK_PTR(pixels);
    SAIL_CHECK_PTR(rgba_pixels);

    SAIL_TRY(sail_malloc((size_t)stride * height, rgba_pixels));

    /* Convert ARGB to RGBA. */
    for (unsigned y = 0; y < height; y++)
    {
        const uint8_t* src = pixels + y * stride;
        uint8_t* dst       = (uint8_t*)*rgba_pixels + y * stride;
        for (unsigned x = 0; x < width; x++)
        {
            dst[0]  = src[1]; /* R */
            dst[1]  = src[2]; /* G */
            dst[2]  = src[3]; /* B */
            dst[3]  = src[0]; /* A */
            src    += 4;
            dst    += 4;
        }
    }

    return SAIL_OK;
}

sail_status_t webp_private_convert_abgr_to_rgba(
    const uint8_t* pixels, unsigned width, unsigned height, unsigned stride, void** rgba_pixels)
{
    SAIL_CHECK_PTR(pixels);
    SAIL_CHECK_PTR(rgba_pixels);

    SAIL_TRY(sail_malloc((size_t)stride * height, rgba_pixels));

    /* Convert ABGR to RGBA. */
    for (unsigned y = 0; y < height; y++)
    {
        const uint8_t* src = pixels + y * stride;
        uint8_t* dst       = (uint8_t*)*rgba_pixels + y * stride;
        for (unsigned x = 0; x < width; x++)
        {
            dst[0]  = src[3]; /* R */
            dst[1]  = src[2]; /* G */
            dst[2]  = src[1]; /* B */
            dst[3]  = src[0]; /* A */
            src    += 4;
            dst    += 4;
        }
    }

    return SAIL_OK;
}

sail_status_t webp_private_import_pixels(struct WebPPicture* picture, const struct sail_image* image)
{
    SAIL_CHECK_PTR(picture);
    SAIL_CHECK_PTR(image);

    const uint8_t* pixels = (const uint8_t*)image->pixels;
    const int stride      = image->bytes_per_line;

    switch (image->pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        if (!WebPPictureImportRGB(picture, pixels, stride))
        {
            SAIL_LOG_ERROR("WEBP: Failed to import RGB pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP24_BGR:
    {
        if (!WebPPictureImportBGR(picture, pixels, stride))
        {
            SAIL_LOG_ERROR("WEBP: Failed to import BGR pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        if (!WebPPictureImportRGBA(picture, pixels, stride))
        {
            SAIL_LOG_ERROR("WEBP: Failed to import RGBA pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
    {
        if (!WebPPictureImportBGRA(picture, pixels, stride))
        {
            SAIL_LOG_ERROR("WEBP: Failed to import BGRA pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ARGB:
    {
        /* WebP doesn't have direct ARGB import, need to convert to RGBA. */
        void* rgba_pixels;
        SAIL_TRY(webp_private_convert_argb_to_rgba(pixels, image->width, image->height, stride, &rgba_pixels));

        const int import_result = WebPPictureImportRGBA(picture, rgba_pixels, stride);
        sail_free(rgba_pixels);

        if (!import_result)
        {
            SAIL_LOG_ERROR("WEBP: Failed to import ARGB pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ABGR:
    {
        /* WebP doesn't have direct ABGR import, need to convert to RGBA. */
        void* rgba_pixels;
        SAIL_TRY(webp_private_convert_abgr_to_rgba(pixels, image->width, image->height, stride, &rgba_pixels));

        const int import_result = WebPPictureImportRGBA(picture, rgba_pixels, stride);
        sail_free(rgba_pixels);

        if (!import_result)
        {
            SAIL_LOG_ERROR("WEBP: Failed to import ABGR pixels");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNDERLYING_CODEC);
        }
        break;
    }
    default:
    {
        SAIL_LOG_ERROR("WEBP: Unsupported pixel format for writing");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }

    return SAIL_OK;
}

static unsigned webp_private_read_variant_uint(const struct sail_variant* value)
{
    if (value->type == SAIL_VARIANT_TYPE_INT)
    {
        int64_t int_val = sail_variant_to_int(value);
        return (int_val < 0) ? 0 : (unsigned)int_val;
    }
    else
    {
        return sail_variant_to_unsigned_int(value);
    }
}

bool webp_private_tuning_key_value_callback(const char* key, const struct sail_variant* value, void* user_data)
{
    struct WebPConfig* config = user_data;

    if (strcmp(key, "webp-lossless") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val     = webp_private_read_variant_uint(value);
            config->lossless = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: lossless=%d", config->lossless);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-lossless' must be an integer");
        }
    }
    else if (strcmp(key, "webp-method") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val   = webp_private_read_variant_uint(value);
            config->method = (val > 6) ? 6 : (int)val;
            SAIL_LOG_TRACE("WEBP: method=%d", config->method);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-method' must be an integer");
        }
    }
    else if (strcmp(key, "webp-image-hint") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "picture") == 0)
            {
                config->image_hint = WEBP_HINT_PICTURE;
            }
            else if (strcmp(str_value, "photo") == 0)
            {
                config->image_hint = WEBP_HINT_PHOTO;
            }
            else if (strcmp(str_value, "graph") == 0)
            {
                config->image_hint = WEBP_HINT_GRAPH;
            }
            else
            {
                config->image_hint = WEBP_HINT_DEFAULT;
            }
            SAIL_LOG_TRACE("WEBP: image-hint=%s", str_value);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-image-hint' must be a string");
        }
    }
    else if (strcmp(key, "webp-target-size") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            config->target_size = (value->type == SAIL_VARIANT_TYPE_INT) ? sail_variant_to_int(value)
                                                                         : (int)sail_variant_to_unsigned_int(value);
            SAIL_LOG_TRACE("WEBP: target-size=%d", config->target_size);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-target-size' must be an integer");
        }
    }
    else if (strcmp(key, "webp-target-psnr") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_FLOAT || value->type == SAIL_VARIANT_TYPE_DOUBLE)
        {
            float val           = sail_variant_to_float(value);
            config->target_PSNR = (val < 0.0f) ? 0.0f : val;
            SAIL_LOG_TRACE("WEBP: target-psnr=%f", config->target_PSNR);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-target-psnr' must be a float or double");
        }
    }
    else if (strcmp(key, "webp-segments") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val     = webp_private_read_variant_uint(value);
            config->segments = (val < 1) ? 1 : (val > 4) ? 4 : (int)val;
            SAIL_LOG_TRACE("WEBP: segments=%d", config->segments);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-segments' must be an integer");
        }
    }
    else if (strcmp(key, "webp-sns-strength") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val         = webp_private_read_variant_uint(value);
            config->sns_strength = (val > 100) ? 100 : (int)val;
            SAIL_LOG_TRACE("WEBP: sns-strength=%d", config->sns_strength);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-sns-strength' must be an integer");
        }
    }
    else if (strcmp(key, "webp-filter-strength") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val            = webp_private_read_variant_uint(value);
            config->filter_strength = (val > 100) ? 100 : (int)val;
            SAIL_LOG_TRACE("WEBP: filter-strength=%d", config->filter_strength);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-filter-strength' must be an integer");
        }
    }
    else if (strcmp(key, "webp-filter-sharpness") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val             = webp_private_read_variant_uint(value);
            config->filter_sharpness = (val > 7) ? 7 : (int)val;
            SAIL_LOG_TRACE("WEBP: filter-sharpness=%d", config->filter_sharpness);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-filter-sharpness' must be an integer");
        }
    }
    else if (strcmp(key, "webp-filter-type") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "simple") == 0)
            {
                config->filter_type = 0;
            }
            else if (strcmp(str_value, "strong") == 0)
            {
                config->filter_type = 1;
            }
            SAIL_LOG_TRACE("WEBP: filter-type=%s (%d)", str_value, config->filter_type);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-filter-type' must be a string");
        }
    }
    else if (strcmp(key, "webp-autofilter") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val       = webp_private_read_variant_uint(value);
            config->autofilter = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: autofilter=%d", config->autofilter);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-autofilter' must be an integer");
        }
    }
    else if (strcmp(key, "webp-alpha-compression") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val              = webp_private_read_variant_uint(value);
            config->alpha_compression = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: alpha-compression=%d", config->alpha_compression);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-alpha-compression' must be an integer");
        }
    }
    else if (strcmp(key, "webp-alpha-filtering") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "none") == 0)
            {
                config->alpha_filtering = 0;
            }
            else if (strcmp(str_value, "fast") == 0)
            {
                config->alpha_filtering = 1;
            }
            else if (strcmp(str_value, "best") == 0)
            {
                config->alpha_filtering = 2;
            }
            SAIL_LOG_TRACE("WEBP: alpha-filtering=%s (%d)", str_value, config->alpha_filtering);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-alpha-filtering' must be a string");
        }
    }
    else if (strcmp(key, "webp-alpha-quality") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val          = webp_private_read_variant_uint(value);
            config->alpha_quality = (val > 100) ? 100 : (int)val;
            SAIL_LOG_TRACE("WEBP: alpha-quality=%d", config->alpha_quality);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-alpha-quality' must be an integer");
        }
    }
    else if (strcmp(key, "webp-pass") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val = webp_private_read_variant_uint(value);
            config->pass = (val < 1) ? 1 : (val > 10) ? 10 : (int)val;
            SAIL_LOG_TRACE("WEBP: pass=%d", config->pass);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-pass' must be an integer");
        }
    }
    else if (strcmp(key, "webp-preprocessing") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_STRING)
        {
            const char* str_value = sail_variant_to_string(value);
            if (strcmp(str_value, "none") == 0)
            {
                config->preprocessing = 0;
            }
            else if (strcmp(str_value, "segment-smooth") == 0)
            {
                config->preprocessing = 1;
            }
            else if (strcmp(str_value, "pseudo-random-dithering") == 0)
            {
                config->preprocessing = 2;
            }
            SAIL_LOG_TRACE("WEBP: preprocessing=%s (%d)", str_value, config->preprocessing);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-preprocessing' must be a string");
        }
    }
    else if (strcmp(key, "webp-partitions") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val       = webp_private_read_variant_uint(value);
            config->partitions = (val > 3) ? 3 : (int)val;
            SAIL_LOG_TRACE("WEBP: partitions=%d", config->partitions);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-partitions' must be an integer");
        }
    }
    else if (strcmp(key, "webp-partition-limit") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val            = webp_private_read_variant_uint(value);
            config->partition_limit = (val > 100) ? 100 : (int)val;
            SAIL_LOG_TRACE("WEBP: partition-limit=%d", config->partition_limit);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-partition-limit' must be an integer");
        }
    }
    else if (strcmp(key, "webp-emulate-jpeg-size") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val              = webp_private_read_variant_uint(value);
            config->emulate_jpeg_size = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: emulate-jpeg-size=%d", config->emulate_jpeg_size);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-emulate-jpeg-size' must be an integer");
        }
    }
    else if (strcmp(key, "webp-thread-level") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val         = webp_private_read_variant_uint(value);
            config->thread_level = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: thread-level=%d", config->thread_level);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-thread-level' must be an integer");
        }
    }
    else if (strcmp(key, "webp-low-memory") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val       = webp_private_read_variant_uint(value);
            config->low_memory = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: low-memory=%d", config->low_memory);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-low-memory' must be an integer");
        }
    }
    else if (strcmp(key, "webp-near-lossless") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val          = webp_private_read_variant_uint(value);
            config->near_lossless = (val > 100) ? 100 : (int)val;
            SAIL_LOG_TRACE("WEBP: near-lossless=%d", config->near_lossless);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-near-lossless' must be an integer");
        }
    }
    else if (strcmp(key, "webp-exact") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val  = webp_private_read_variant_uint(value);
            config->exact = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: exact=%d", config->exact);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-exact' must be an integer");
        }
    }
    else if (strcmp(key, "webp-use-delta-palette") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val              = webp_private_read_variant_uint(value);
            config->use_delta_palette = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: use-delta-palette=%d", config->use_delta_palette);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-use-delta-palette' must be an integer");
        }
    }
    else if (strcmp(key, "webp-use-sharp-yuv") == 0)
    {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT)
        {
            unsigned val          = webp_private_read_variant_uint(value);
            config->use_sharp_yuv = (val != 0) ? 1 : 0;
            SAIL_LOG_TRACE("WEBP: use-sharp-yuv=%d", config->use_sharp_yuv);
        }
        else
        {
            SAIL_LOG_ERROR("WEBP: 'webp-use-sharp-yuv' must be an integer");
        }
    }

    return true;
}
