#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"

int sail_strdup(char *input, char **output) {

    return sail_strdup_length(input, -1, output);
}

int sail_strdup_length(char *input, int length, char **output) {

    if (input == NULL) {
        *output = NULL;
        return 0;
    }

    const int input_len = strlen(input);
    const int len = (length < 0 || length > input_len) ? input_len : length;

    *output = (char *)malloc(len+1);

    if (*output == NULL) {
        return errno;
    }

    memcpy(*output, input, len);
    (*output)[len] = '\0';

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
