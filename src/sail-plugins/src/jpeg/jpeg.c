#include "config.h"

#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

#include "common.h"
#include "export.h"
#include "meta_entry_node.h"
#include "utils.h"

/*
 * Plugin-specific data types.
 */
struct my_error_context {
    struct jpeg_error_mgr jpeg_error_mgr;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_context * my_error_context_ptr;

static void my_error_exit(j_common_ptr cinfo) {
    my_error_context_ptr myerr;

    myerr = (my_error_context_ptr)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

static int color_space_to_pixel_format(J_COLOR_SPACE color_space) {
    switch (color_space) {
        case JCS_GRAYSCALE: return SAIL_PIXEL_FORMAT_GRAYSCALE;

        case JCS_EXT_RGB:
        case JCS_RGB:       return SAIL_PIXEL_FORMAT_RGB;

        case JCS_YCbCr:     return SAIL_PIXEL_FORMAT_YCBCR;
        case JCS_CMYK:      return SAIL_PIXEL_FORMAT_CMYK;
        case JCS_YCCK:      return SAIL_PIXEL_FORMAT_YCCK;
        case JCS_EXT_RGBX:  return SAIL_PIXEL_FORMAT_RGBX;
        case JCS_EXT_BGR:   return SAIL_PIXEL_FORMAT_BGR;
        case JCS_EXT_BGRX:  return SAIL_PIXEL_FORMAT_BGRX;
        case JCS_EXT_XBGR:  return SAIL_PIXEL_FORMAT_XBGR;
        case JCS_EXT_XRGB:  return SAIL_PIXEL_FORMAT_XRGB;
        case JCS_EXT_RGBA:  return SAIL_PIXEL_FORMAT_RGBA;
        case JCS_EXT_BGRA:  return SAIL_PIXEL_FORMAT_BGRA;
        case JCS_EXT_ABGR:  return SAIL_PIXEL_FORMAT_ABGR;
        case JCS_EXT_ARGB:  return SAIL_PIXEL_FORMAT_ARGB;
        case JCS_RGB565:    return SAIL_PIXEL_FORMAT_RGB565;

        default:            return SAIL_PIXEL_FORMAT_UNKNOWN;
    }
}

static J_COLOR_SPACE pixel_format_to_color_space(int pixel_format) {
    switch (pixel_format) {
        case SAIL_PIXEL_FORMAT_GRAYSCALE: return JCS_GRAYSCALE;
        case SAIL_PIXEL_FORMAT_RGB:       return JCS_RGB;
        case SAIL_PIXEL_FORMAT_YCBCR:     return JCS_YCbCr;
        case SAIL_PIXEL_FORMAT_CMYK:      return JCS_CMYK;
        case SAIL_PIXEL_FORMAT_YCCK:      return JCS_YCCK;
        case SAIL_PIXEL_FORMAT_RGBX:      return JCS_EXT_RGBX;
        case SAIL_PIXEL_FORMAT_BGR:       return JCS_EXT_BGR;
        case SAIL_PIXEL_FORMAT_BGRX:      return JCS_EXT_BGRX;
        case SAIL_PIXEL_FORMAT_XBGR:      return JCS_EXT_XBGR;
        case SAIL_PIXEL_FORMAT_XRGB:      return JCS_EXT_XRGB;
        case SAIL_PIXEL_FORMAT_RGBA:      return JCS_EXT_RGBA;
        case SAIL_PIXEL_FORMAT_BGRA:      return JCS_EXT_BGRA;
        case SAIL_PIXEL_FORMAT_ABGR:      return JCS_EXT_ABGR;
        case SAIL_PIXEL_FORMAT_ARGB:      return JCS_EXT_ARGB;
        case SAIL_PIXEL_FORMAT_RGB565:    return JCS_RGB565;

        default:                          return JCS_UNKNOWN;
    }
}

/*
 * Plugin-specific PIMPL.
 */
struct pimpl {
    struct jpeg_decompress_struct decompress_context;
    struct my_error_context error_context;
    JSAMPARRAY buffer;
    bool libjpeg_error;
    struct sail_read_options *read_options;
};

/*
 * Plugin interface.
 */
int SAIL_EXPORT sail_plugin_layout_version(void) {

    return 1;
}

const char* SAIL_EXPORT sail_plugin_version(void) {

    return "1.3.4.1";
}

const char* SAIL_EXPORT sail_plugin_description(void) {

    return "JPEG compressed";
}

const char* SAIL_EXPORT sail_plugin_extensions(void) {

    return "jpg;jpeg;jpe;jif;jfif;jfi";
}

const char* SAIL_EXPORT sail_plugin_mime_types(void) {

    return "image/jpeg";
}

const char* SAIL_EXPORT sail_plugin_magic(void) {

    return "\x00FF\x00D8\x00FF";
}

int SAIL_EXPORT sail_plugin_read_features(struct sail_read_features **read_features) {

    int res;

    if ((res = sail_read_features_alloc(read_features)) != 0) {
        return res;
    }

    (*read_features)->pixel_formats_length = 15;
    (*read_features)->pixel_formats = (int *)malloc((*read_features)->pixel_formats_length * sizeof(int));

    if ((*read_features)->pixel_formats == NULL) {
        return ENOMEM;
    }

    (*read_features)->pixel_formats[0]  = SAIL_PIXEL_FORMAT_GRAYSCALE;
    (*read_features)->pixel_formats[1]  = SAIL_PIXEL_FORMAT_RGB;
    (*read_features)->pixel_formats[2]  = SAIL_PIXEL_FORMAT_YCBCR;
    (*read_features)->pixel_formats[3]  = SAIL_PIXEL_FORMAT_CMYK;
    (*read_features)->pixel_formats[4]  = SAIL_PIXEL_FORMAT_YCCK;
    (*read_features)->pixel_formats[5]  = SAIL_PIXEL_FORMAT_RGBX;
    (*read_features)->pixel_formats[6]  = SAIL_PIXEL_FORMAT_BGR;
    (*read_features)->pixel_formats[7]  = SAIL_PIXEL_FORMAT_BGRX;
    (*read_features)->pixel_formats[8]  = SAIL_PIXEL_FORMAT_XBGR;
    (*read_features)->pixel_formats[9]  = SAIL_PIXEL_FORMAT_XRGB;
    (*read_features)->pixel_formats[10] = SAIL_PIXEL_FORMAT_RGBA;
    (*read_features)->pixel_formats[11] = SAIL_PIXEL_FORMAT_BGRA;
    (*read_features)->pixel_formats[12] = SAIL_PIXEL_FORMAT_ABGR;
    (*read_features)->pixel_formats[13] = SAIL_PIXEL_FORMAT_ARGB;
    (*read_features)->pixel_formats[14] = SAIL_PIXEL_FORMAT_RGB565;

    (*read_features)->io_options = SAIL_IO_OPTION_STATIC | SAIL_IO_OPTION_META_INFO;

    return 0;
}

int SAIL_EXPORT sail_plugin_read_init(struct sail_file *file, struct sail_read_options *read_options) {

    if (file == NULL) {
        return EINVAL;
    }

    struct pimpl *pimpl = (struct pimpl *)malloc(sizeof(struct pimpl));

    if (pimpl == NULL) {
        return ENOMEM;
    }

    file->pimpl = pimpl;

    pimpl->libjpeg_error = false;

    int res;

    if (read_options == NULL) {
        if ((res = sail_read_options_alloc(&pimpl->read_options)) != 0) {
            return res;
        }
    } else {
        pimpl->read_options = (struct sail_read_options *)malloc(sizeof(struct sail_read_options));
        memcpy(pimpl->read_options, read_options, sizeof(struct sail_read_options));
    }

    pimpl->decompress_context.err = jpeg_std_error(&pimpl->error_context.jpeg_error_mgr);
    pimpl->error_context.jpeg_error_mgr.error_exit = my_error_exit;

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return EIO;
    }

    jpeg_create_decompress(&pimpl->decompress_context);
    jpeg_stdio_src(&pimpl->decompress_context, file->fptr);

    if (pimpl->read_options->options & SAIL_IO_OPTION_META_INFO) {
        jpeg_save_markers(&pimpl->decompress_context, JPEG_COM, 0xffff);
    }

    jpeg_read_header(&pimpl->decompress_context, TRUE);

    if (pimpl->read_options->pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN) {
        return EINVAL;
    }

    if (pimpl->read_options->pixel_format != SAIL_PIXEL_FORMAT_SOURCE) {
        J_COLOR_SPACE color_space = pixel_format_to_color_space(pimpl->read_options->pixel_format);

        if (pimpl->decompress_context.jpeg_color_space != color_space) {
            pimpl->decompress_context.out_color_space = color_space;
            pimpl->decompress_context.quantize_colors = FALSE;
        }
    }

    jpeg_start_decompress(&pimpl->decompress_context);

    // TODO
    //currentImage = -1;

    return 0;
}

int SAIL_EXPORT sail_plugin_read_seek_next_frame(struct sail_file *file, struct sail_image **image) {

    if (file == NULL) {
        return EINVAL;
    }

    struct pimpl *pimpl = (struct pimpl *)file->pimpl;

    if (pimpl == NULL) {
        return ENOMEM;
    }

    int res;

    if((res = sail_image_alloc(image)) != 0) {
        return res;
    }

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return EIO;
    }

    // TODO
    //currentImage++;

    //if(currentImage) {
    //    return EIO;
    //}

    pimpl->buffer = (*pimpl->decompress_context.mem->alloc_sarray)((j_common_ptr)&pimpl->decompress_context,
                                                                    JPOOL_IMAGE,
                                                                    pimpl->decompress_context.output_width *
                                                                        pimpl->decompress_context.output_components,
                                                                    1);

    if (pimpl->buffer == NULL) {
        return ENOMEM;
    }

    (*image)->width               = pimpl->decompress_context.output_width;
    (*image)->height              = pimpl->decompress_context.output_height;
    (*image)->pixel_format        = SAIL_PIXEL_FORMAT_RGB;
    (*image)->passes              = 1;
    (*image)->source_pixel_format = color_space_to_pixel_format(pimpl->decompress_context.jpeg_color_space);

    if (pimpl->read_options->options & SAIL_IO_OPTION_META_INFO) {
        jpeg_saved_marker_ptr it = pimpl->decompress_context.marker_list;
        struct sail_meta_entry_node *last_meta_entry_node;

        while(it != NULL) {
            if(it->marker == JPEG_COM) {
                struct sail_meta_entry_node *meta_entry_node;

                if ((res = sail_alloc_meta_entry_node(&meta_entry_node)) != 0) {
                    return res;
                }

                if ((res = sail_strdup("Comment", &meta_entry_node->key)) != 0) {
                    sail_destroy_meta_entry_node(meta_entry_node);
                    return res;
                }

                if ((res = sail_strdup_length((char *)it->data, it->data_length, &meta_entry_node->value)) != 0) {
                    sail_destroy_meta_entry_node(meta_entry_node);
                    return res;
                }

                if ((*image)->meta_entry_node == NULL) {
                    (*image)->meta_entry_node = last_meta_entry_node = meta_entry_node;
                } else {
                    last_meta_entry_node->next = meta_entry_node;
                    last_meta_entry_node = meta_entry_node;
                }
            }

            it = it->next;
        }
    }

    return 0;
}

int SAIL_EXPORT sail_plugin_read_seek_next_pass(struct sail_file *file, struct sail_image *image) {

    if (file == NULL || image == NULL) {
        return EINVAL;
    }

    return 0;
}

int SAIL_EXPORT sail_plugin_read_scan_line(struct sail_file *file, struct sail_image *image, unsigned char *scanline) {

    if (file == NULL || image == NULL || scanline == NULL) {
        return EINVAL;
    }

    struct pimpl *pimpl = (struct pimpl *)file->pimpl;

    if (pimpl == NULL) {
        return ENOMEM;
    }

    if (pimpl->libjpeg_error) {
        return EIO;
    }

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return EIO;
    }

    const int color_components = pimpl->decompress_context.output_components;

    (void)jpeg_read_scanlines(&pimpl->decompress_context, pimpl->buffer, 1);

    memcpy(scanline, pimpl->buffer[0], image->width * color_components);

    return 0;
}

int SAIL_EXPORT sail_plugin_read_alloc_scan_line(struct sail_file *file, struct sail_image *image, unsigned char **scanline) {

    if (file == NULL || image == NULL) {
        return EINVAL;
    }

    struct pimpl *pimpl = (struct pimpl *)file->pimpl;

    if (pimpl == NULL) {
        return ENOMEM;
    }

    const int color_components = pimpl->decompress_context.output_components;

    *scanline = (unsigned char *)malloc(image->width * color_components);

    if (*scanline == NULL) {
        return ENOMEM;
    }

    return sail_plugin_read_scan_line(file, image, *scanline);
}

int SAIL_EXPORT sail_plugin_read_finish(struct sail_file *file, struct sail_image *image) {

    if (file == NULL) {
        return EINVAL;
    }

    (void)image;

    struct pimpl *pimpl = (struct pimpl *)file->pimpl;

    if (pimpl == NULL) {
        return ENOMEM;
    }

    sail_read_options_destroy(pimpl->read_options);

    if (setjmp(pimpl->error_context.setjmp_buffer) != 0) {
        pimpl->libjpeg_error = true;
        return EIO;
    }

    jpeg_abort_decompress(&pimpl->decompress_context);
    jpeg_destroy_decompress(&pimpl->decompress_context);

    return 0;
}

#if 0
void fmt_codec::getwriteoptions(fmt_writeoptionsabs *opt)
{
    opt->interlaced = false;
    opt->compression_scheme = CompressionInternal;
    opt->compression_min = 0;
    opt->compression_max = 100;
    opt->compression_def = 25;
    opt->passes = 1;
    opt->needflip = false;
    opt->palette_flags = 0 | fmt_image::pure32;
}

s32 fmt_codec::write_init(const std::string &file, const fmt_image &image, const fmt_writeoptions &opt)
{
    if(!image.w || !image.h || file.empty())
	return SQE_W_WRONGPARAMS;

    writeimage = image;
    writeopt = opt;

    m_fptr = fopen(file.c_str(), "wb");

    if(!m_fptr)
        return SQE_W_NOFILE;

    decompress_context.err = jpeg_std_error(&error_context);

    jpeg_create_compress(&decompress_context);

    jpeg_stdio_dest(&decompress_context, m_fptr);

    decompress_context.image_width = image.w;
    decompress_context.image_height = image.h;
    decompress_context.input_components = 3;
    decompress_context.in_color_space = JCS_RGB;

    jpeg_set_defaults(&decompress_context);

    jpeg_set_quality(&decompress_context, 100-opt.compression_level, true);

    jpeg_start_compress(&decompress_context, true);

    return SQE_OK;
}

s32 fmt_codec::write_next()
{
    return SQE_OK;
}

s32 fmt_codec::write_next_pass()
{
    return SQE_OK;
}

s32 fmt_codec::write_scanline(RGBA *scan)
{
    RGB sr[writeimage.w];

    for(s32 s = 0;s < writeimage.w;s++)
    {
        memcpy(sr+s, scan+s, sizeof(RGB));
    }

    JSAMPROW row_pointer = (JSAMPLE *)sr;

    (void)jpeg_write_scanlines(&decompress_context, &row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::write_close()
{
    jpeg_finish_compress(&decompress_context);

    fclose(m_fptr);

    jpeg_destroy_compress(&decompress_context=);
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("jpeg");
}

#endif
