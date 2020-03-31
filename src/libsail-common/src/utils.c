#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"

int sail_strdup(char *input, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    const size_t len = strlen(input);

    *output = (char *)malloc(len+1);

    if (*output == NULL) {
        return errno;
    }

    memcpy(*output, input, len);
    (*output)[len] = '\0';

    return 0;
}

int sail_strdup_length(char *input, size_t length, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    if (length == 0) {
        return EINVAL;
    }

    *output = (char *)malloc(length+1);

    if (*output == NULL) {
        return errno;
    }

    memcpy(*output, input, length);
    (*output)[length] = '\0';

    return 0;
}

const char* sail_pixel_format_to_string(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   return "Unknown";
        case SAIL_PIXEL_FORMAT_SOURCE:    return "Source";

        case SAIL_PIXEL_FORMAT_MONO:      return "Mono";
        case SAIL_PIXEL_FORMAT_GRAYSCALE: return "Grayscale";
        case SAIL_PIXEL_FORMAT_INDEXED:   return "Indexed";
        case SAIL_PIXEL_FORMAT_RGB:       return "RGB";
        case SAIL_PIXEL_FORMAT_YCBCR:     return "YCbCr";
        case SAIL_PIXEL_FORMAT_CMYK:      return "CMYK";
        case SAIL_PIXEL_FORMAT_YCCK:      return "YCCK";
        case SAIL_PIXEL_FORMAT_RGBX:      return "RGBX";
        case SAIL_PIXEL_FORMAT_BGR:       return "BGR";
        case SAIL_PIXEL_FORMAT_BGRX:      return "BGRX";
        case SAIL_PIXEL_FORMAT_XBGR:      return "XBGR";
        case SAIL_PIXEL_FORMAT_XRGB:      return "XRGB";
        case SAIL_PIXEL_FORMAT_RGBA:      return "RGBA";
        case SAIL_PIXEL_FORMAT_BGRA:      return "BGRA";
        case SAIL_PIXEL_FORMAT_ABGR:      return "ABGR";
        case SAIL_PIXEL_FORMAT_ARGB:      return "ARGB";
        case SAIL_PIXEL_FORMAT_RGB565:    return "RGB565";
    }

    return "Unknown";
}

int sail_bits_per_pixel(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:   return 0;
        case SAIL_PIXEL_FORMAT_SOURCE:    return 0;

        case SAIL_PIXEL_FORMAT_MONO:      return 1;

        case SAIL_PIXEL_FORMAT_GRAYSCALE: return 8;
        case SAIL_PIXEL_FORMAT_INDEXED:   return 8;

        case SAIL_PIXEL_FORMAT_RGB565:    return 16;

        case SAIL_PIXEL_FORMAT_RGB:       return 24;
        case SAIL_PIXEL_FORMAT_YCBCR:     return 24;
        case SAIL_PIXEL_FORMAT_BGR:       return 24;

        case SAIL_PIXEL_FORMAT_CMYK:      return 32;
        case SAIL_PIXEL_FORMAT_YCCK:      return 32;
        case SAIL_PIXEL_FORMAT_RGBX:      return 32;
        case SAIL_PIXEL_FORMAT_BGRX:      return 32;
        case SAIL_PIXEL_FORMAT_XBGR:      return 32;
        case SAIL_PIXEL_FORMAT_XRGB:      return 32;
        case SAIL_PIXEL_FORMAT_RGBA:      return 32;
        case SAIL_PIXEL_FORMAT_BGRA:      return 32;
        case SAIL_PIXEL_FORMAT_ABGR:      return 32;
        case SAIL_PIXEL_FORMAT_ARGB:      return 32;
    }

    return 0;
}
