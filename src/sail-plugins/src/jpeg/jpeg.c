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
#include "plugin.h"

/*
 * Plugin-specific data types.
 */
struct my_error_context {

    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_context * my_error_context_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_context_ptr myerr;

    myerr = (my_error_context_ptr)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

/*
 * Plugin-specific PIMPL.
 */
struct pimpl {

    struct jpeg_decompress_struct decompress_context;
    struct my_error_context error_context;
    JSAMPARRAY buffer;
    bool zerror;
    struct sail_read_options read_options;
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

    return "jpg;jpeg;jpe";
}

const char* SAIL_EXPORT sail_plugin_mime_types(void) {

    return "image/jpeg";
}

const char* SAIL_EXPORT sail_plugin_magic(void) {

    return "\x00FF\x00D8\x00FF";
}

int SAIL_EXPORT sail_plugin_features(void) {

    return SAIL_PLUGIN_FEATURE_READ_STATIC | SAIL_PLUGIN_FEATURE_WRITE_STATIC;
}

int SAIL_EXPORT sail_plugin_read_init(struct sail_file *file, struct sail_read_options *read_options) {

    struct pimpl *pimpl = (struct pimpl *)malloc(sizeof(struct pimpl));

    if (pimpl == NULL) {
        return ENOMEM;
    }

    file->pimpl = pimpl;

    pimpl->zerror = false;

    if (read_options == NULL) {
        pimpl->read_options = sail_default_read_options();
    } else {
        pimpl->read_options = *read_options;
    }

    // TODO
    //currentImage = -1;

    return 0;
}

int SAIL_EXPORT sail_plugin_read_seek_next_frame(struct sail_file *file, struct sail_image **image) {

    struct pimpl *pimpl = (struct pimpl *)file->pimpl;

    if (pimpl == NULL) {
        return ENOMEM;
    }

    int res;

    if((res = sail_image_alloc(image)) != 0) {
        return res;
    }

    // TODO
    //currentImage++;

    //if(currentImage) {
    //    return EIO;
    //}

    pimpl->decompress_context.err = jpeg_std_error(&pimpl->error_context.pub);
    pimpl->error_context.pub.error_exit = my_error_exit;

    if (setjmp(pimpl->error_context.setjmp_buffer)) {
        pimpl->zerror = true;
        return EIO;
    }

    jpeg_create_decompress(&pimpl->decompress_context);
    jpeg_stdio_src(&pimpl->decompress_context, file->fptr);
    jpeg_save_markers(&pimpl->decompress_context, JPEG_COM, 0xffff);
    jpeg_read_header(&pimpl->decompress_context, TRUE);

    if (pimpl->decompress_context.jpeg_color_space != JCS_RGB) {
        pimpl->decompress_context.out_color_space = JCS_RGB;
        pimpl->decompress_context.desired_number_of_colors = 256;
        pimpl->decompress_context.quantize_colors = FALSE;
        pimpl->decompress_context.two_pass_quantize = FALSE;
    }

    jpeg_start_decompress(&pimpl->decompress_context);

    (*image)->width = pimpl->decompress_context.output_width;
    (*image)->height = pimpl->decompress_context.output_height;

    pimpl->buffer = (*pimpl->decompress_context.mem->alloc_sarray)((j_common_ptr)&pimpl->decompress_context,
                                                                    JPOOL_IMAGE,
                                                                    pimpl->decompress_context.output_width * pimpl->decompress_context.output_components,
                                                                    1);

    switch (pimpl->decompress_context.jpeg_color_space) {

        case JCS_GRAYSCALE: (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_GRAYSCALE; break;

        case JCS_EXT_RGB:
        case JCS_RGB:       (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_RGB;       break;

        case JCS_YCbCr:     (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_YCBCR;     break;
        case JCS_CMYK:      (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_CMYK;      break;
        case JCS_YCCK:      (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_YCCK;      break;
        case JCS_EXT_RGBX:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_RGBX;      break;
        case JCS_EXT_BGR:   (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_BGR;       break;
        case JCS_EXT_BGRX:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_BGRX;      break;
        case JCS_EXT_XBGR:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_XBGR;      break;
        case JCS_EXT_XRGB:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_ARGB;      break;
        case JCS_EXT_RGBA:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_RGBA;      break;
        case JCS_EXT_BGRA:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_BGRA;      break;
        case JCS_EXT_ABGR:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_ABGR;      break;
        case JCS_EXT_ARGB:  (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_ARGB;      break;
        case JCS_RGB565:    (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_RGB565;    break;

        default:            (*image)->source_pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;   break;
    }

    if (pimpl->read_options.meta_info) {

        jpeg_saved_marker_ptr it = pimpl->decompress_context.marker_list;
        struct sail_meta_entry_node *last_meta_entry_node;

        while(it) {
            if(it->marker == JPEG_COM) {

                struct sail_meta_entry_node *meta_entry_node;

                if ((res = sail_alloc_meta_entry_node(&meta_entry_node)) != 0) {
                    return res;
                }

                if ((*image)->meta_entry_node == NULL) {
                    (*image)->meta_entry_node = last_meta_entry_node = meta_entry_node;
                } else {
                    last_meta_entry_node->next = meta_entry_node;
                    last_meta_entry_node = meta_entry_node;
                }

                last_meta_entry_node->key = "Comment";

                last_meta_entry_node->value = (char *)malloc(it->data_length+1);

                if (last_meta_entry_node->value == NULL) {
                    return ENOMEM;
                }

                memcpy(last_meta_entry_node->value, it->data, it->data_length);
                last_meta_entry_node->value[it->data_length] = '\0';
            }

            it = it->next;
        }
    }

    return 0;
}

#if 0
s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}
	
s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(zerror || setjmp(error_context.setjmp_buffer)) 
    {
        zerror = true;
	return SQE_R_BADFILE;
    }

    (void)jpeg_read_scanlines(&decompress_context, buffer, 1);

    for(s32 i = 0;i < im->w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

    return SQE_OK;
}

void fmt_codec::read_close()
{
    jpeg_abort_decompress(&decompress_context);
    jpeg_destroy_decompress(&decompress_context);

    if(fptr)
        fclose(fptr);

    finfo.meta.clear();
    finfo.image.clear();
}

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
