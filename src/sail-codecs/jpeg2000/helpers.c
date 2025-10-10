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

#include <sail-common/sail-common.h>

#include "helpers.h"

enum SailPixelFormat jpeg2000_private_sail_pixel_format(OPJ_COLOR_SPACE color_space, int num_comps, int prec) {

    /* Scale precision to byte boundary. */
    const int scaled_prec = ((prec + 7) / 8) * 8;

    switch (color_space) {
        case OPJ_CLRSPC_GRAY: {
            if (num_comps == 1) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                    case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 2) {
                /* Grayscale with alpha. */
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
                    case 16: return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else {
                return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case OPJ_CLRSPC_SRGB: {
            if (num_comps == 3) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                    case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 4) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                    case 16: return SAIL_PIXEL_FORMAT_BPP64_RGBA;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else {
                return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case OPJ_CLRSPC_SYCC:
        case OPJ_CLRSPC_EYCC: {
            /* YCbCr and e-YCC - map to YCbCr if supported. */
            if (num_comps == 3 && scaled_prec == 8) {
                return SAIL_PIXEL_FORMAT_BPP24_YCBCR;
            } else {
                return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case OPJ_CLRSPC_CMYK: {
            if (num_comps == 4) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP32_CMYK;
                    case 16: return SAIL_PIXEL_FORMAT_BPP64_CMYK;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 5) {
                /* CMYK with alpha. */
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP40_CMYKA;
                    case 16: return SAIL_PIXEL_FORMAT_BPP80_CMYKA;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else {
                return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
        case OPJ_CLRSPC_UNSPECIFIED:
        default: {
            /* Unspecified color space - try to guess from component count. */
            if (num_comps == 1) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
                    case 16: return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 3) {
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP24_RGB;
                    case 16: return SAIL_PIXEL_FORMAT_BPP48_RGB;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 4) {
                /* Could be RGBA or CMYK, assume CMYK for unspecified. */
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP32_CMYK;
                    case 16: return SAIL_PIXEL_FORMAT_BPP64_CMYK;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else if (num_comps == 5) {
                /* Assume CMYKA. */
                switch (scaled_prec) {
                    case 8:  return SAIL_PIXEL_FORMAT_BPP40_CMYKA;
                    case 16: return SAIL_PIXEL_FORMAT_BPP80_CMYKA;
                    default: return SAIL_PIXEL_FORMAT_UNKNOWN;
                }
            } else {
                return SAIL_PIXEL_FORMAT_UNKNOWN;
            }
        }
    }
}

sail_status_t jpeg2000_private_pixel_format_to_openjpeg(enum SailPixelFormat pixel_format, OPJ_COLOR_SPACE *color_space, int *num_comps, int *prec) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE: {
            *color_space = OPJ_CLRSPC_GRAY;
            *num_comps = 1;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE: {
            *color_space = OPJ_CLRSPC_GRAY;
            *num_comps = 1;
            *prec = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: {
            *color_space = OPJ_CLRSPC_GRAY;
            *num_comps = 2;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: {
            *color_space = OPJ_CLRSPC_GRAY;
            *num_comps = 2;
            *prec = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP24_RGB: {
            *color_space = OPJ_CLRSPC_SRGB;
            *num_comps = 3;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP48_RGB: {
            *color_space = OPJ_CLRSPC_SRGB;
            *num_comps = 3;
            *prec = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA: {
            *color_space = OPJ_CLRSPC_SRGB;
            *num_comps = 4;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP64_RGBA: {
            *color_space = OPJ_CLRSPC_SRGB;
            *num_comps = 4;
            *prec = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP24_YCBCR: {
            *color_space = OPJ_CLRSPC_SYCC;
            *num_comps = 3;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP32_CMYK: {
            *color_space = OPJ_CLRSPC_CMYK;
            *num_comps = 4;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP64_CMYK: {
            *color_space = OPJ_CLRSPC_CMYK;
            *num_comps = 4;
            *prec = 16;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP40_CMYKA: {
            *color_space = OPJ_CLRSPC_CMYK;
            *num_comps = 5;
            *prec = 8;
            return SAIL_OK;
        }
        case SAIL_PIXEL_FORMAT_BPP80_CMYKA: {
            *color_space = OPJ_CLRSPC_CMYK;
            *num_comps = 5;
            *prec = 16;
            return SAIL_OK;
        }
        default: {
            return SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;
        }
    }
}

bool jpeg2000_private_tuning_key_value_callback_load(const char *key, const struct sail_variant *value, void *user_data) {

    opj_dparameters_t *parameters = user_data;

    if (strcmp(key, "jpeg2000-reduce") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned reduce = (value->type == SAIL_VARIANT_TYPE_INT)
                               ? (unsigned)sail_variant_to_int(value)
                               : sail_variant_to_unsigned_int(value);
            parameters->cp_reduce = (OPJ_UINT32)reduce;
            SAIL_LOG_TRACE("JPEG2000: Set reduce to %u", parameters->cp_reduce);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-reduce' must be an integer");
        }
    } else if (strcmp(key, "jpeg2000-layer") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned layer = (value->type == SAIL_VARIANT_TYPE_INT)
                              ? (unsigned)sail_variant_to_int(value)
                              : sail_variant_to_unsigned_int(value);
            parameters->cp_layer = (OPJ_UINT32)layer;
            SAIL_LOG_TRACE("JPEG2000: Set layer to %u", parameters->cp_layer);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-layer' must be an integer");
        }
    } else if (strcmp(key, "jpeg2000-tile-index") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned tile_index = (value->type == SAIL_VARIANT_TYPE_INT)
                                   ? (unsigned)sail_variant_to_int(value)
                                   : sail_variant_to_unsigned_int(value);
            parameters->tile_index = (OPJ_UINT32)tile_index;
            SAIL_LOG_TRACE("JPEG2000: Set tile-index to %u", parameters->tile_index);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-tile-index' must be an integer");
        }
    } else if (strcmp(key, "jpeg2000-num-tiles") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            unsigned num_tiles = (value->type == SAIL_VARIANT_TYPE_INT)
                                  ? (unsigned)sail_variant_to_int(value)
                                  : sail_variant_to_unsigned_int(value);
            parameters->nb_tile_to_decode = (OPJ_UINT32)num_tiles;
            SAIL_LOG_TRACE("JPEG2000: Set num-tiles to %u", parameters->nb_tile_to_decode);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-num-tiles' must be an integer");
        }
    }

    return true;
}

bool jpeg2000_private_tuning_key_value_callback_save(const char *key, const struct sail_variant *value, void *user_data) {

    opj_cparameters_t *parameters = user_data;

    if (strcmp(key, "jpeg2000-irreversible") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_BOOL) {
            parameters->irreversible = sail_variant_to_bool(value) ? 1 : 0;
            SAIL_LOG_TRACE("JPEG2000: Set irreversible to %d", parameters->irreversible);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-irreversible' must be a bool");
        }
    } else if (strcmp(key, "jpeg2000-numresolution") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            int numresolution = (value->type == SAIL_VARIANT_TYPE_INT)
                                 ? sail_variant_to_int(value)
                                 : (int)sail_variant_to_unsigned_int(value);
            if (numresolution >= 1 && numresolution <= 32) {
                parameters->numresolution = numresolution;
                SAIL_LOG_TRACE("JPEG2000: Set numresolution to %d", parameters->numresolution);
            }
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-numresolution' must be an integer");
        }
    } else if (strcmp(key, "jpeg2000-prog-order") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_STRING) {
            const char *str_value = sail_variant_to_string(value);

            if (strcmp(str_value, "lrcp") == 0) {
                parameters->prog_order = OPJ_LRCP;
            } else if (strcmp(str_value, "rlcp") == 0) {
                parameters->prog_order = OPJ_RLCP;
            } else if (strcmp(str_value, "rpcl") == 0) {
                parameters->prog_order = OPJ_RPCL;
            } else if (strcmp(str_value, "pcrl") == 0) {
                parameters->prog_order = OPJ_PCRL;
            } else if (strcmp(str_value, "cprl") == 0) {
                parameters->prog_order = OPJ_CPRL;
            }

            SAIL_LOG_TRACE("JPEG2000: Set prog-order to %s", str_value);
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-prog-order' must be a string");
        }
    } else if (strcmp(key, "jpeg2000-codeblock-width") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            int cblockw = (value->type == SAIL_VARIANT_TYPE_INT)
                           ? sail_variant_to_int(value)
                           : (int)sail_variant_to_unsigned_int(value);
            /* Must be power of 2 between 4 and 1024 */
            if (cblockw >= 4 && cblockw <= 1024 && (cblockw & (cblockw - 1)) == 0) {
                parameters->cblockw_init = cblockw;
                SAIL_LOG_TRACE("JPEG2000: Set codeblock-width to %d", parameters->cblockw_init);
            }
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-codeblock-width' must be an integer");
        }
    } else if (strcmp(key, "jpeg2000-codeblock-height") == 0) {
        if (value->type == SAIL_VARIANT_TYPE_INT || value->type == SAIL_VARIANT_TYPE_UNSIGNED_INT) {
            int cblockh = (value->type == SAIL_VARIANT_TYPE_INT)
                           ? sail_variant_to_int(value)
                           : (int)sail_variant_to_unsigned_int(value);
            /* Must be power of 2 between 4 and 1024 */
            if (cblockh >= 4 && cblockh <= 1024 && (cblockh & (cblockh - 1)) == 0) {
                parameters->cblockh_init = cblockh;
                SAIL_LOG_TRACE("JPEG2000: Set codeblock-height to %d", parameters->cblockh_init);
            }
        } else {
            SAIL_LOG_ERROR("JPEG2000: 'jpeg2000-codeblock-height' must be an integer");
        }
    }

    return true;
}
