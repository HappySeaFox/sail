/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#include "sail-common.h"

const char* sail_pixel_format_to_string(enum SailPixelFormat pixel_format) {

    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_UNKNOWN:               return "UNKNOWN";

        case SAIL_PIXEL_FORMAT_BPP1:                  return "BPP1";
        case SAIL_PIXEL_FORMAT_BPP2:                  return "BPP2";
        case SAIL_PIXEL_FORMAT_BPP4:                  return "BPP4";
        case SAIL_PIXEL_FORMAT_BPP8:                  return "BPP8";
        case SAIL_PIXEL_FORMAT_BPP16:                 return "BPP16";
        case SAIL_PIXEL_FORMAT_BPP24:                 return "BPP24";
        case SAIL_PIXEL_FORMAT_BPP32:                 return "BPP32";
        case SAIL_PIXEL_FORMAT_BPP48:                 return "BPP48";
        case SAIL_PIXEL_FORMAT_BPP64:                 return "BPP64";
        case SAIL_PIXEL_FORMAT_BPP72:                 return "BPP72";
        case SAIL_PIXEL_FORMAT_BPP96:                 return "BPP96";
        case SAIL_PIXEL_FORMAT_BPP128:                return "BPP128";

        case SAIL_PIXEL_FORMAT_BPP1_INDEXED:          return "BPP1-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP2_INDEXED:          return "BPP2-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP4_INDEXED:          return "BPP4-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP8_INDEXED:          return "BPP8-INDEXED";
        case SAIL_PIXEL_FORMAT_BPP16_INDEXED:         return "BPP16-INDEXED";

        case SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE:        return "BPP1-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE:        return "BPP2-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE:        return "BPP4-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE:        return "BPP8-GRAYSCALE";
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE:       return "BPP16-GRAYSCALE";

        case SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA:  return "BPP4-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA:  return "BPP8-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA: return "BPP16-GRAYSCALE-ALPHA";
        case SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA: return "BPP32-GRAYSCALE-ALPHA";

        case SAIL_PIXEL_FORMAT_BPP16_RGB555:          return "BPP16-RGB555";
        case SAIL_PIXEL_FORMAT_BPP16_BGR555:          return "BPP16-BGR555";
        case SAIL_PIXEL_FORMAT_BPP16_RGB565:          return "BPP16-RGB565";
        case SAIL_PIXEL_FORMAT_BPP16_BGR565:          return "BPP16-BGR565";

        case SAIL_PIXEL_FORMAT_BPP24_RGB:             return "BPP24-RGB";
        case SAIL_PIXEL_FORMAT_BPP24_BGR:             return "BPP24-BGR";

        case SAIL_PIXEL_FORMAT_BPP48_RGB:             return "BPP48-RGB";
        case SAIL_PIXEL_FORMAT_BPP48_BGR:             return "BPP48-BGR";

        case SAIL_PIXEL_FORMAT_BPP32_RGBX:            return "BPP32-RGBX";
        case SAIL_PIXEL_FORMAT_BPP32_BGRX:            return "BPP32-BGRX";
        case SAIL_PIXEL_FORMAT_BPP32_XRGB:            return "BPP32-XRGB";
        case SAIL_PIXEL_FORMAT_BPP32_XBGR:            return "BPP32-XBGR";
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:            return "BPP32-RGBA";
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:            return "BPP32-BGRA";
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:            return "BPP32-ARGB";
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:            return "BPP32-ABGR";

        case SAIL_PIXEL_FORMAT_BPP64_RGBX:            return "BPP64-RGBX";
        case SAIL_PIXEL_FORMAT_BPP64_BGRX:            return "BPP64-BGRX";
        case SAIL_PIXEL_FORMAT_BPP64_XRGB:            return "BPP64-XRGB";
        case SAIL_PIXEL_FORMAT_BPP64_XBGR:            return "BPP64-XBGR";
        case SAIL_PIXEL_FORMAT_BPP64_RGBA:            return "BPP64-RGBA";
        case SAIL_PIXEL_FORMAT_BPP64_BGRA:            return "BPP64-BGRA";
        case SAIL_PIXEL_FORMAT_BPP64_ARGB:            return "BPP64-ARGB";
        case SAIL_PIXEL_FORMAT_BPP64_ABGR:            return "BPP64-ABGR";

        case SAIL_PIXEL_FORMAT_BPP32_CMYK:            return "BPP32-CMYK";
        case SAIL_PIXEL_FORMAT_BPP64_CMYK:            return "BPP64-CMYK";

        case SAIL_PIXEL_FORMAT_BPP24_YCBCR:           return "BPP24-YCBCR";

        case SAIL_PIXEL_FORMAT_BPP32_YCCK:            return "BPP32-YCCK";

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LAB:         return "BPP24-CIE-LAB";
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LAB:         return "BPP40-CIE-LAB";

        case SAIL_PIXEL_FORMAT_BPP24_CIE_LUV:         return "BPP24-CIE-LUV";
        case SAIL_PIXEL_FORMAT_BPP40_CIE_LUV:         return "BPP40-CIE-LUV";

        case SAIL_PIXEL_FORMAT_BPP24_YUV:             return "BPP24-YUV";
        case SAIL_PIXEL_FORMAT_BPP30_YUV:             return "BPP30-YUV";
        case SAIL_PIXEL_FORMAT_BPP36_YUV:             return "BPP36-YUV";
        case SAIL_PIXEL_FORMAT_BPP48_YUV:             return "BPP48-YUV";

        case SAIL_PIXEL_FORMAT_BPP32_YUVA:            return "BPP32-YUVA";
        case SAIL_PIXEL_FORMAT_BPP40_YUVA:            return "BPP40-YUVA";
        case SAIL_PIXEL_FORMAT_BPP48_YUVA:            return "BPP48-YUVA";
        case SAIL_PIXEL_FORMAT_BPP64_YUVA:            return "BPP64-YUVA";
    }

    return NULL;
}

enum SailPixelFormat sail_pixel_format_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_PIXEL_FORMAT_UNKNOWN);

    /*
     * The switch doesn't look very nice, I know :) However, it's fast and doesn't require
     * extra data structures and initializations. It's not C++11, so we choose between two evils:
     *
     *     1. Introduce extra data structures and their initializations to work with hashes.
     *     2. Use a single ugly looking switch/case.
     */
    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_PIXEL_FORMAT_UNKNOWN;

        case UINT64_C(6383902552):           return SAIL_PIXEL_FORMAT_BPP1;
        case UINT64_C(6383902553):           return SAIL_PIXEL_FORMAT_BPP2;
        case UINT64_C(6383902555):           return SAIL_PIXEL_FORMAT_BPP4;
        case UINT64_C(6383902559):           return SAIL_PIXEL_FORMAT_BPP8;
        case UINT64_C(210668784270):         return SAIL_PIXEL_FORMAT_BPP16;
        case UINT64_C(210668784301):         return SAIL_PIXEL_FORMAT_BPP24;
        case UINT64_C(210668784332):         return SAIL_PIXEL_FORMAT_BPP32;
        case UINT64_C(210668784371):         return SAIL_PIXEL_FORMAT_BPP48;
        case UINT64_C(210668784433):         return SAIL_PIXEL_FORMAT_BPP64;
        case UINT64_C(210668784464):         return SAIL_PIXEL_FORMAT_BPP72;
        case UINT64_C(210668784534):         return SAIL_PIXEL_FORMAT_BPP96;
        case UINT64_C(6952069880834):        return SAIL_PIXEL_FORMAT_BPP128;

        case UINT64_C(13257949335914442470): return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
        case UINT64_C(13257950742323060711): return SAIL_PIXEL_FORMAT_BPP2_INDEXED;
        case UINT64_C(13257953555140297193): return SAIL_PIXEL_FORMAT_BPP4_INDEXED;
        case UINT64_C(13257959180774770157): return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
        case UINT64_C(13237225848150241308): return SAIL_PIXEL_FORMAT_BPP16_INDEXED;

        case UINT64_C(12552958524517323328): return SAIL_PIXEL_FORMAT_BPP1_GRAYSCALE;
        case UINT64_C(12554490103502587777): return SAIL_PIXEL_FORMAT_BPP2_GRAYSCALE;
        case UINT64_C(12557553261473116675): return SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE;
        case UINT64_C(12563679577414174471): return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE;
        case UINT64_C(8431824423011809526):  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE;

        case UINT64_C(9367569596161118198):  return SAIL_PIXEL_FORMAT_BPP4_GRAYSCALE_ALPHA;
        case UINT64_C(12512997289017890810): return SAIL_PIXEL_FORMAT_BPP8_GRAYSCALE_ALPHA;
        case UINT64_C(3292614999547101481):  return SAIL_PIXEL_FORMAT_BPP16_GRAYSCALE_ALPHA;
        case UINT64_C(5929884054553197927):  return SAIL_PIXEL_FORMAT_BPP32_GRAYSCALE_ALPHA;

        case UINT64_C(13257949683479278997): return SAIL_PIXEL_FORMAT_BPP16_RGB555;
        case UINT64_C(13257949682853687701): return SAIL_PIXEL_FORMAT_BPP16_BGR555;
        case UINT64_C(13257949683479279030): return SAIL_PIXEL_FORMAT_BPP16_RGB565;
        case UINT64_C(13257949682853687734): return SAIL_PIXEL_FORMAT_BPP16_BGR565;

        case UINT64_C(249836535348735093):   return SAIL_PIXEL_FORMAT_BPP24_RGB;
        case UINT64_C(249836535348717685):   return SAIL_PIXEL_FORMAT_BPP24_BGR;

        case UINT64_C(249836535431749563):   return SAIL_PIXEL_FORMAT_BPP48_RGB;
        case UINT64_C(249836535431732155):   return SAIL_PIXEL_FORMAT_BPP48_BGR;

        case UINT64_C(8244605667721455340):  return SAIL_PIXEL_FORMAT_BPP32_RGBX;
        case UINT64_C(8244605667720880876):  return SAIL_PIXEL_FORMAT_BPP32_BGRX;
        case UINT64_C(8244605667721683084):  return SAIL_PIXEL_FORMAT_BPP32_XRGB;
        case UINT64_C(8244605667721665676):  return SAIL_PIXEL_FORMAT_BPP32_XBGR;
        case UINT64_C(8244605667721455317):  return SAIL_PIXEL_FORMAT_BPP32_RGBA;
        case UINT64_C(8244605667720880853):  return SAIL_PIXEL_FORMAT_BPP32_BGRA;
        case UINT64_C(8244605667720856533):  return SAIL_PIXEL_FORMAT_BPP32_ARGB;
        case UINT64_C(8244605667720839125):  return SAIL_PIXEL_FORMAT_BPP32_ABGR;

        case UINT64_C(8244605671674130033):  return SAIL_PIXEL_FORMAT_BPP64_RGBX;
        case UINT64_C(8244605671673555569):  return SAIL_PIXEL_FORMAT_BPP64_BGRX;
        case UINT64_C(8244605671674357777):  return SAIL_PIXEL_FORMAT_BPP64_XRGB;
        case UINT64_C(8244605671674340369):  return SAIL_PIXEL_FORMAT_BPP64_XBGR;
        case UINT64_C(8244605671674130010):  return SAIL_PIXEL_FORMAT_BPP64_RGBA;
        case UINT64_C(8244605671673555546):  return SAIL_PIXEL_FORMAT_BPP64_BGRA;
        case UINT64_C(8244605671673531226):  return SAIL_PIXEL_FORMAT_BPP64_ARGB;
        case UINT64_C(8244605671673513818):  return SAIL_PIXEL_FORMAT_BPP64_ABGR;

        case UINT64_C(8244605667720923565):  return SAIL_PIXEL_FORMAT_BPP32_CMYK;
        case UINT64_C(8244605671673598258):  return SAIL_PIXEL_FORMAT_BPP64_CMYK;

        case UINT64_C(13817569962846953645): return SAIL_PIXEL_FORMAT_BPP24_YCBCR;

        case UINT64_C(8244605667721702563):  return SAIL_PIXEL_FORMAT_BPP32_YCCK;

        case UINT64_C(13237269438873232231): return SAIL_PIXEL_FORMAT_BPP24_CIE_LAB;
        case UINT64_C(13237356636207563173): return SAIL_PIXEL_FORMAT_BPP40_CIE_LAB;

        case UINT64_C(13237269438873232911): return SAIL_PIXEL_FORMAT_BPP24_CIE_LUV;
        case UINT64_C(13237356636207563853): return SAIL_PIXEL_FORMAT_BPP40_CIE_LUV;

        case UINT64_C(249836535348743198):   return SAIL_PIXEL_FORMAT_BPP24_YUV;
        case UINT64_C(249836535383134907):   return SAIL_PIXEL_FORMAT_BPP30_YUV;
        case UINT64_C(249836535390250433):   return SAIL_PIXEL_FORMAT_BPP36_YUV;
        case UINT64_C(249836535431757668):   return SAIL_PIXEL_FORMAT_BPP48_YUV;

        case UINT64_C(8244605667721722782):  return SAIL_PIXEL_FORMAT_BPP32_YUVA;
        case UINT64_C(8244605668934919965):  return SAIL_PIXEL_FORMAT_BPP40_YUVA;
        case UINT64_C(8244605669248003109):  return SAIL_PIXEL_FORMAT_BPP48_YUVA;
        case UINT64_C(8244605671674397475):  return SAIL_PIXEL_FORMAT_BPP64_YUVA;
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

const char* sail_image_property_to_string(enum SailImageProperty image_property) {

    switch (image_property) {
        case SAIL_IMAGE_PROPERTY_UNKNOWN:              return "UNKNOWN";
        case SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY:   return "FLIPPED-VERTICALLY";
        case SAIL_IMAGE_PROPERTY_FLIPPED_HORIZONTALLY: return "FLIPPED-HORIZONTALLY";
        case SAIL_IMAGE_PROPERTY_INTERLACED:           return "INTERLACED";
    }

    return NULL;
}

enum SailImageProperty sail_image_property_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_IMAGE_PROPERTY_UNKNOWN);

    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_IMAGE_PROPERTY_UNKNOWN;
        case UINT64_C(17202465669660106453): return SAIL_IMAGE_PROPERTY_FLIPPED_VERTICALLY;
        case UINT64_C(9347591013355692325):  return SAIL_IMAGE_PROPERTY_FLIPPED_HORIZONTALLY;
        case UINT64_C(8244927930303708800):  return SAIL_IMAGE_PROPERTY_INTERLACED;
    }

    return SAIL_IMAGE_PROPERTY_UNKNOWN;
}

const char* sail_compression_to_string(enum SailCompression compression) {

    switch (compression) {
        case SAIL_COMPRESSION_UNSUPPORTED:   return "UNSUPPORTED";
        case SAIL_COMPRESSION_UNKNOWN:       return "UNKNOWN";
        case SAIL_COMPRESSION_NONE:          return "NONE";
        case SAIL_COMPRESSION_ADOBE_DEFLATE: return "ADOBE-DEFLATE";
        case SAIL_COMPRESSION_AV1:           return "AV1";
        case SAIL_COMPRESSION_CCITT_FAX3:    return "CCITT-FAX3";
        case SAIL_COMPRESSION_CCITT_FAX4:    return "CCITT-FAX4";
        case SAIL_COMPRESSION_CCITT_RLE:     return "CCITT-RLE";
        case SAIL_COMPRESSION_CCITT_RLEW:    return "CCITT-RLEW";
        case SAIL_COMPRESSION_CCITT_T4:      return "CCITT-T4";
        case SAIL_COMPRESSION_CCITT_T6:      return "CCITT-T6";
        case SAIL_COMPRESSION_DCS:           return "DCS";
        case SAIL_COMPRESSION_DEFLATE:       return "DEFLATE";
        case SAIL_COMPRESSION_IT8_BL:        return "IT8-BL";
        case SAIL_COMPRESSION_IT8_CTPAD:     return "IT8-CTPAD";
        case SAIL_COMPRESSION_IT8_LW:        return "IT8-LW";
        case SAIL_COMPRESSION_IT8_MP:        return "IT8-MP";
        case SAIL_COMPRESSION_JBIG:          return "JBIG";
        case SAIL_COMPRESSION_JPEG:          return "JPEG";
        case SAIL_COMPRESSION_JPEG_2000:     return "JPEG-2000";
        case SAIL_COMPRESSION_JPEG_XL:       return "JPEG-XL";
        case SAIL_COMPRESSION_JPEG_XR:       return "JPEG-XR";
        case SAIL_COMPRESSION_LERC:          return "LERC";
        case SAIL_COMPRESSION_LZMA:          return "LZMA";
        case SAIL_COMPRESSION_LZW:           return "LZW";
        case SAIL_COMPRESSION_NEXT:          return "NEXT";
        case SAIL_COMPRESSION_OJPEG:         return "OJPEG";
        case SAIL_COMPRESSION_PACKBITS:      return "PACKBITS";
        case SAIL_COMPRESSION_PIXAR_FILM:    return "PIXAR-FILM";
        case SAIL_COMPRESSION_PIXAR_LOG:     return "PIXAR-LOG";
        case SAIL_COMPRESSION_RLE:           return "RLE";
        case SAIL_COMPRESSION_SGI_LOG:       return "SGI-LOG";
        case SAIL_COMPRESSION_SGI_LOG24:     return "SGI-LOG24";
        case SAIL_COMPRESSION_T43:           return "T43";
        case SAIL_COMPRESSION_T85:           return "T85";
        case SAIL_COMPRESSION_THUNDERSCAN:   return "THUNDERSCAN";
        case SAIL_COMPRESSION_WEBP:          return "WEBP";
        case SAIL_COMPRESSION_ZSTD:          return "ZSTD";
    }

    return NULL;
}

enum SailCompression sail_compression_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_COMPRESSION_UNKNOWN);

    switch (hash) {
        case UINT64_C(13846582888989074574): return SAIL_COMPRESSION_UNSUPPORTED;
        case UINT64_C(229442760833397):      return SAIL_COMPRESSION_UNKNOWN;
        case UINT64_C(6384332661):           return SAIL_COMPRESSION_NONE;
        case UINT64_C(10962109560604417378): return SAIL_COMPRESSION_ADOBE_DEFLATE;
        case UINT64_C(193450669):            return SAIL_COMPRESSION_AV1;
        case UINT64_C(8244633541513328571):  return SAIL_COMPRESSION_CCITT_FAX3;
        case UINT64_C(8244633541513328572):  return SAIL_COMPRESSION_CCITT_FAX4;
        case UINT64_C(249837380045871852):   return SAIL_COMPRESSION_CCITT_RLE;
        case UINT64_C(8244633541513771203):  return SAIL_COMPRESSION_CCITT_RLEW;
        case UINT64_C(7570829698359793):     return SAIL_COMPRESSION_CCITT_T4;
        case UINT64_C(7570829698359795):     return SAIL_COMPRESSION_CCITT_T6;
        case UINT64_C(193453343):            return SAIL_COMPRESSION_DCS;
        case UINT64_C(229420447642554):      return SAIL_COMPRESSION_DEFLATE;
        case UINT64_C(6952347705973):        return SAIL_COMPRESSION_IT8_BL;
        case UINT64_C(249846519511114451):   return SAIL_COMPRESSION_IT8_CTPAD;
        case UINT64_C(6952347706314):        return SAIL_COMPRESSION_IT8_LW;
        case UINT64_C(6952347706340):        return SAIL_COMPRESSION_IT8_MP;
        case UINT64_C(6384174593):           return SAIL_COMPRESSION_JBIG;
        case UINT64_C(6384189707):           return SAIL_COMPRESSION_JPEG;
        case UINT64_C(249847773225217050):   return SAIL_COMPRESSION_JPEG_2000;
        case UINT64_C(229428625552444):      return SAIL_COMPRESSION_JPEG_XL;
        case UINT64_C(229428625552450):      return SAIL_COMPRESSION_JPEG_XR;
        case UINT64_C(6384250027):           return SAIL_COMPRESSION_LERC;
        case UINT64_C(6384272729):           return SAIL_COMPRESSION_LZMA;
        case UINT64_C(193462818):            return SAIL_COMPRESSION_LZW;
        case UINT64_C(6384322116):           return SAIL_COMPRESSION_NEXT;
        case UINT64_C(210683986298):         return SAIL_COMPRESSION_OJPEG;
        case UINT64_C(7571380909080566):     return SAIL_COMPRESSION_PACKBITS;
        case UINT64_C(8245245943922754206):  return SAIL_COMPRESSION_PIXAR_FILM;
        case UINT64_C(249855937694635640):   return SAIL_COMPRESSION_PIXAR_LOG;
        case UINT64_C(193468872):            return SAIL_COMPRESSION_RLE;
        case UINT64_C(229439900388407):      return SAIL_COMPRESSION_SGI_LOG;
        case UINT64_C(249860051522976925):   return SAIL_COMPRESSION_SGI_LOG24;
        case UINT64_C(193470240):            return SAIL_COMPRESSION_T43;
        case UINT64_C(193470374):            return SAIL_COMPRESSION_T85;
        case UINT64_C(13844775339661004164): return SAIL_COMPRESSION_THUNDERSCAN;
        case UINT64_C(6384644819):           return SAIL_COMPRESSION_WEBP;
        case UINT64_C(6384768458):           return SAIL_COMPRESSION_ZSTD;
    }

    return SAIL_COMPRESSION_UNKNOWN;
}

const char* sail_meta_data_to_string(enum SailMetaData meta_data) {

    switch (meta_data) {
        case SAIL_META_DATA_UNKNOWN:       return "Unknown";

        case SAIL_META_DATA_ARTIST:        return "Artist";
        case SAIL_META_DATA_AUTHOR:        return "Author";
        case SAIL_META_DATA_COMMENT:       return "Comment";
        case SAIL_META_DATA_COMPUTER:      return "Computer";
        case SAIL_META_DATA_COPYRIGHT:     return "Copyright";
        case SAIL_META_DATA_CREATION_TIME: return "Creation Time";
        case SAIL_META_DATA_DESCRIPTION:   return "Description";
        case SAIL_META_DATA_DISCLAIMER:    return "Disclaimer";
        case SAIL_META_DATA_DOCUMENT:      return "Document";
        case SAIL_META_DATA_EXIF:          return "EXIF";
        case SAIL_META_DATA_ID:            return "ID";
        case SAIL_META_DATA_IPTC:          return "IPTC";
        case SAIL_META_DATA_LABEL:         return "Label";
        case SAIL_META_DATA_MAKE:          return "Make";
        case SAIL_META_DATA_MODEL:         return "Model";
        case SAIL_META_DATA_NAME:          return "Name";
        case SAIL_META_DATA_PRINTER:       return "Printer";
        case SAIL_META_DATA_SOFTWARE:      return "Software";
        case SAIL_META_DATA_SOURCE:        return "Source";
        case SAIL_META_DATA_TITLE:         return "Title";
        case SAIL_META_DATA_URL:           return "URL";
        case SAIL_META_DATA_WARNING:       return "Warning";
        case SAIL_META_DATA_XMP:           return "XMP";
    }

    return NULL;
}

enum SailMetaData sail_meta_data_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_META_DATA_UNKNOWN);

    switch (hash) {
        case UINT64_C(229444052301365):      return SAIL_META_DATA_UNKNOWN;

        case UINT64_C(6952072423676):        return SAIL_META_DATA_ARTIST;
        case UINT64_C(6952075980216):        return SAIL_META_DATA_AUTHOR;
        case UINT64_C(229420847338040):      return SAIL_META_DATA_COMMENT;
        case UINT64_C(7570887966294228):     return SAIL_META_DATA_COMPUTER;
        case UINT64_C(249839307110380862):   return SAIL_META_DATA_COPYRIGHT;
        case UINT64_C(16658027699238675945): return SAIL_META_DATA_CREATION_TIME;
        case UINT64_C(13821659157043486569): return SAIL_META_DATA_DESCRIPTION;
        case UINT64_C(8244735206874071778):  return SAIL_META_DATA_DISCLAIMER;
        case UINT64_C(7570930199009348):     return SAIL_META_DATA_DOCUMENT;
        case UINT64_C(6384018865):           return SAIL_META_DATA_EXIF;
        case UINT64_C(5862386):              return SAIL_META_DATA_ID;
        case UINT64_C(6384154261):           return SAIL_META_DATA_IPTC;
        case UINT64_C(210681275781):         return SAIL_META_DATA_LABEL;
        case UINT64_C(6384317315):           return SAIL_META_DATA_MAKE;
        case UINT64_C(210682966998):         return SAIL_META_DATA_MODEL;
        case UINT64_C(6384353318):           return SAIL_META_DATA_NAME;
        case UINT64_C(229437749136105):      return SAIL_META_DATA_PRINTER;
        case UINT64_C(7571569592229392):     return SAIL_META_DATA_SOFTWARE;
        case UINT64_C(6952773348182):        return SAIL_META_DATA_SOURCE;
        case UINT64_C(210691070471):         return SAIL_META_DATA_TITLE;
        case UINT64_C(193472344):            return SAIL_META_DATA_URL;
        case UINT64_C(229446134771803):      return SAIL_META_DATA_WARNING;
        case UINT64_C(193475450):            return SAIL_META_DATA_XMP;
    }

    return SAIL_META_DATA_UNKNOWN;
}

const char* sail_codec_feature_to_string(enum SailCodecFeature codec_feature) {

    switch (codec_feature) {
        case SAIL_CODEC_FEATURE_UNKNOWN:     return "UNKNOWN";
        case SAIL_CODEC_FEATURE_STATIC:      return "STATIC";
        case SAIL_CODEC_FEATURE_ANIMATED:    return "ANIMATED";
        case SAIL_CODEC_FEATURE_MULTI_PAGED: return "MULTI-PAGED";
        case SAIL_CODEC_FEATURE_META_DATA:   return "META-DATA";
        case SAIL_CODEC_FEATURE_INTERLACED:  return "INTERLACED";
        case SAIL_CODEC_FEATURE_ICCP:        return "ICCP";
    }

    return NULL;
}

enum SailCodecFeature sail_codec_feature_from_string(const char *str) {

    uint64_t hash;
    SAIL_TRY_OR_EXECUTE(sail_string_hash(str, &hash),
                        /* cleanup */ return SAIL_CODEC_FEATURE_UNKNOWN);

    switch (hash) {
        case UINT64_C(229442760833397):      return SAIL_CODEC_FEATURE_UNKNOWN;
        case UINT64_C(6952739426029):        return SAIL_CODEC_FEATURE_STATIC;
        case UINT64_C(7570758658679240):     return SAIL_CODEC_FEATURE_ANIMATED;
        case UINT64_C(13834645239609548286): return SAIL_CODEC_FEATURE_MULTI_PAGED;
        case UINT64_C(249851542786072787):   return SAIL_CODEC_FEATURE_META_DATA;
        case UINT64_C(8244927930303708800):  return SAIL_CODEC_FEATURE_INTERLACED;
        case UINT64_C(6384139556):           return SAIL_CODEC_FEATURE_ICCP;
    }

    return SAIL_CODEC_FEATURE_UNKNOWN;
}
