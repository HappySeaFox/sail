#include <setjmp.h>
#include <stdio.h>

#include <jpeglib.h>

void text(void)
{
    jpeg_create_decompress(NULL);
}

#if 0

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr;

    myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

int sail_plugin_layout_version(void) {

    return 1;
}

const char* sail_plugin_version(void) {

    return "1.3.4.1";
}

const char* sail_plugin_description(void) {

    return "JPEG compressed";
}

const char* sail_plugin_extensions(void) {

    return "jpg;jpeg;jpe";
}

const char* sail_plugin_mime_types(void) {

    return "image/jpeg";
}

const char* sail_plugin_magic(void) {

    return "\x00FF\x00D8\x00FF";
}

int sail_plugin_features(void) {

    /* TODO */
/*
    o->readable = true;
    o->canbemultiple = false;
    o->writestatic = true;
    o->writeanimated = false;
*/
    return 0;
}

int sail_plugin_read_init(struct sail_file *file, struct sail_read_options *read_options) {

    zerror = false;

    fptr = fopen(file.c_str(), "rb");

    if(!fptr)
        return SQE_R_NOFILE;

    currentImage = -1;

    finfo.animated = false;

    return SQE_OK;
}

s32 fmt_codec::read_next()
{
    currentImage++;
    
    if(currentImage)
	return SQE_NOTOK;

    fmt_image image;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if(setjmp(jerr.setjmp_buffer)) 
    {
        zerror = true;
	return SQE_R_BADFILE;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fptr);
    jpeg_save_markers(&cinfo, JPEG_COM, 0xffff);
    jpeg_read_header(&cinfo, TRUE);

    if(cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        cinfo.out_color_space = JCS_RGB;
	cinfo.desired_number_of_colors = 256;
	cinfo.quantize_colors = FALSE;
	cinfo.two_pass_quantize = FALSE;
    }

    jpeg_start_decompress(&cinfo);

    image.w = cinfo.output_width;
    image.h = cinfo.output_height;
    
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    std::string type;

    switch(cinfo.jpeg_color_space)
    {
	case JCS_GRAYSCALE: type =  "Grayscale"; image.bpp = 8;  break;
	case JCS_RGB:       type =  "RGB";       image.bpp = 24; break;
	case JCS_YCbCr:     type =  "YUV";       image.bpp = 24; break;
	case JCS_CMYK:      type =  "CMYK";      image.bpp = 32; break;
	case JCS_YCCK:      type =  "YCCK";      image.bpp = 32; break;

	default:
	    type = "Unknown";
    }

    image.compression = "JPEG";
    image.colorspace = type;

    jpeg_saved_marker_ptr it = cinfo.marker_list;

    while(it)
    {
	if(it->marker == JPEG_COM)
	{
            fmt_metaentry mt;

	    mt.group = "Comment";
	    s8 data[it->data_length+1];
	    memcpy(data, it->data, it->data_length);
	    data[it->data_length] = '\0';
	    mt.data = data;

	    addmeta(mt);

    	    break;
	}

	it = it->next;
    }

    finfo.image.push_back(image);

    return SQE_OK;
}

s32 fmt_codec::read_next_pass()
{
    return SQE_OK;
}
	
s32 fmt_codec::read_scanline(RGBA *scan)
{
    fmt_image *im = image(currentImage);
    fmt_utils::fillAlpha(scan, im->w);

    if(zerror || setjmp(jerr.setjmp_buffer)) 
    {
        zerror = true;
	return SQE_R_BADFILE;
    }

    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    for(s32 i = 0;i < im->w;i++)
	memcpy(scan+i, buffer[0] + i*3, 3);

    return SQE_OK;
}

void fmt_codec::read_close()
{
    jpeg_abort_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

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

    m_cinfo.err = jpeg_std_error(&m_jerr);

    jpeg_create_compress(&m_cinfo);

    jpeg_stdio_dest(&m_cinfo, m_fptr);

    m_cinfo.image_width = image.w;
    m_cinfo.image_height = image.h;
    m_cinfo.input_components = 3;
    m_cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&m_cinfo);

    jpeg_set_quality(&m_cinfo, 100-opt.compression_level, true);

    jpeg_start_compress(&m_cinfo, true);

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

    row_pointer = (JSAMPLE *)sr;

    (void)jpeg_write_scanlines(&m_cinfo, &row_pointer, 1);

    return SQE_OK;
}

void fmt_codec::write_close()
{
    jpeg_finish_compress(&m_cinfo);

    fclose(m_fptr);

    jpeg_destroy_compress(&m_cinfo);
}

std::string fmt_codec::extension(const s32 /*bpp*/)
{
    return std::string("jpeg");
}

#endif
