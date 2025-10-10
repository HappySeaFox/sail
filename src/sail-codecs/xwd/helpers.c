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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <sail-common/sail-common.h>

#include "helpers.h"

bool xwd_private_is_native_byte_order(uint32_t byte_order)
{

    uint32_t test         = 1;
    bool is_little_endian = (*((uint8_t*)&test) == 1);

    return (byte_order == LSBFirst && is_little_endian) || (byte_order == MSBFirst && !is_little_endian);
}

sail_status_t xwd_private_read_header(struct sail_io* io, struct XWDFileHeader* header)
{

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(header);

    uint32_t buffer[25];

    SAIL_TRY(io->strict_read(io->stream, buffer, sizeof(buffer)));

    /* Detect byte order by checking header_size field. */
    bool need_swap       = false;
    uint32_t header_size = buffer[0];

    if (header_size != XWD_HEADER_SIZE)
    {
        header_size = sail_reverse_uint32(header_size);
        if (header_size == XWD_HEADER_SIZE)
        {
            need_swap = true;
        }
        else
        {
            SAIL_LOG_ERROR("XWD: Invalid header size %u", buffer[0]);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
        }
    }

    /* Read all fields. */
    header->header_size         = need_swap ? sail_reverse_uint32(buffer[0]) : buffer[0];
    header->file_version        = need_swap ? sail_reverse_uint32(buffer[1]) : buffer[1];
    header->pixmap_format       = need_swap ? sail_reverse_uint32(buffer[2]) : buffer[2];
    header->pixmap_depth        = need_swap ? sail_reverse_uint32(buffer[3]) : buffer[3];
    header->pixmap_width        = need_swap ? sail_reverse_uint32(buffer[4]) : buffer[4];
    header->pixmap_height       = need_swap ? sail_reverse_uint32(buffer[5]) : buffer[5];
    header->xoffset             = need_swap ? sail_reverse_uint32(buffer[6]) : buffer[6];
    header->byte_order          = need_swap ? sail_reverse_uint32(buffer[7]) : buffer[7];
    header->bitmap_unit         = need_swap ? sail_reverse_uint32(buffer[8]) : buffer[8];
    header->bitmap_bit_order    = need_swap ? sail_reverse_uint32(buffer[9]) : buffer[9];
    header->bitmap_pad          = need_swap ? sail_reverse_uint32(buffer[10]) : buffer[10];
    header->bits_per_pixel      = need_swap ? sail_reverse_uint32(buffer[11]) : buffer[11];
    header->bytes_per_line      = need_swap ? sail_reverse_uint32(buffer[12]) : buffer[12];
    header->visual_class        = need_swap ? sail_reverse_uint32(buffer[13]) : buffer[13];
    header->red_mask            = need_swap ? sail_reverse_uint32(buffer[14]) : buffer[14];
    header->green_mask          = need_swap ? sail_reverse_uint32(buffer[15]) : buffer[15];
    header->blue_mask           = need_swap ? sail_reverse_uint32(buffer[16]) : buffer[16];
    header->bits_per_rgb        = need_swap ? sail_reverse_uint32(buffer[17]) : buffer[17];
    header->colormap_entries    = need_swap ? sail_reverse_uint32(buffer[18]) : buffer[18];
    header->ncolors             = need_swap ? sail_reverse_uint32(buffer[19]) : buffer[19];
    header->window_width        = need_swap ? sail_reverse_uint32(buffer[20]) : buffer[20];
    header->window_height       = need_swap ? sail_reverse_uint32(buffer[21]) : buffer[21];
    header->window_x            = need_swap ? sail_reverse_uint32(buffer[22]) : buffer[22];
    header->window_y            = need_swap ? sail_reverse_uint32(buffer[23]) : buffer[23];
    header->window_border_width = need_swap ? sail_reverse_uint32(buffer[24]) : buffer[24];

    /* Validate header. */
    if (header->file_version != XWD_FILE_VERSION)
    {
        SAIL_LOG_ERROR("XWD: Unsupported file version %u", header->file_version);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_FORMAT);
    }

    if (header->pixmap_width == 0 || header->pixmap_height == 0)
    {
        SAIL_LOG_ERROR("XWD: Invalid image dimensions %ux%u", header->pixmap_width, header->pixmap_height);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_BROKEN_IMAGE);
    }

    return SAIL_OK;
}

sail_status_t xwd_private_write_header(struct sail_io* io, const struct XWDFileHeader* header)
{

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(header);

    uint32_t buffer[25];

    buffer[0]  = header->header_size;
    buffer[1]  = header->file_version;
    buffer[2]  = header->pixmap_format;
    buffer[3]  = header->pixmap_depth;
    buffer[4]  = header->pixmap_width;
    buffer[5]  = header->pixmap_height;
    buffer[6]  = header->xoffset;
    buffer[7]  = header->byte_order;
    buffer[8]  = header->bitmap_unit;
    buffer[9]  = header->bitmap_bit_order;
    buffer[10] = header->bitmap_pad;
    buffer[11] = header->bits_per_pixel;
    buffer[12] = header->bytes_per_line;
    buffer[13] = header->visual_class;
    buffer[14] = header->red_mask;
    buffer[15] = header->green_mask;
    buffer[16] = header->blue_mask;
    buffer[17] = header->bits_per_rgb;
    buffer[18] = header->colormap_entries;
    buffer[19] = header->ncolors;
    buffer[20] = header->window_width;
    buffer[21] = header->window_height;
    buffer[22] = header->window_x;
    buffer[23] = header->window_y;
    buffer[24] = header->window_border_width;

    SAIL_TRY(io->strict_write(io->stream, buffer, sizeof(buffer)));

    return SAIL_OK;
}

sail_status_t xwd_private_read_colormap(struct sail_io* io,
                                        uint32_t ncolors,
                                        bool byte_swap,
                                        struct XWDColor** colormap)
{

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(colormap);

    if (ncolors == 0)
    {
        *colormap = NULL;
        return SAIL_OK;
    }

    void* ptr;
    SAIL_TRY(sail_malloc(ncolors * sizeof(struct XWDColor), &ptr));
    *colormap = ptr;

    for (uint32_t i = 0; i < ncolors; i++)
    {
        uint32_t pixel;
        uint16_t red, green, blue;
        uint8_t flags, pad;

        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &pixel, sizeof(pixel)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);
        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &red, sizeof(red)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);
        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &green, sizeof(green)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);
        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &blue, sizeof(blue)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);
        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &flags, sizeof(flags)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);
        SAIL_TRY_OR_CLEANUP(io->strict_read(io->stream, &pad, sizeof(pad)),
                            /* cleanup */ sail_free(*colormap);
                            *colormap = NULL);

        (*colormap)[i].pixel = byte_swap ? sail_reverse_uint32(pixel) : pixel;
        (*colormap)[i].red   = byte_swap ? sail_reverse_uint16(red) : red;
        (*colormap)[i].green = byte_swap ? sail_reverse_uint16(green) : green;
        (*colormap)[i].blue  = byte_swap ? sail_reverse_uint16(blue) : blue;
        (*colormap)[i].flags = flags;
        (*colormap)[i].pad   = pad;
    }

    return SAIL_OK;
}

sail_status_t xwd_private_write_colormap(struct sail_io* io, const struct XWDColor* colormap, uint32_t ncolors)
{

    SAIL_CHECK_PTR(io);

    if (ncolors == 0)
    {
        return SAIL_OK;
    }

    SAIL_CHECK_PTR(colormap);

    for (uint32_t i = 0; i < ncolors; i++)
    {
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].pixel, sizeof(colormap[i].pixel)));
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].red, sizeof(colormap[i].red)));
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].green, sizeof(colormap[i].green)));
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].blue, sizeof(colormap[i].blue)));
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].flags, sizeof(colormap[i].flags)));
        SAIL_TRY(io->strict_write(io->stream, &colormap[i].pad, sizeof(colormap[i].pad)));
    }

    return SAIL_OK;
}

enum SailPixelFormat xwd_private_pixel_format_from_header(const struct XWDFileHeader* header)
{

    /* ZPixmap format is most common for color images. */
    if (header->pixmap_format == ZPixmap)
    {
        if (header->visual_class == TrueColor || header->visual_class == DirectColor)
        {
            /* True color images. */
            if (header->bits_per_pixel == 32)
            {
                /* Check mask pattern for RGBA/BGRA. */
                if (header->red_mask == 0xFF000000 && header->green_mask == 0x00FF0000
                    && header->blue_mask == 0x0000FF00)
                {
                    return SAIL_PIXEL_FORMAT_BPP32_RGBA;
                }
                else if (header->red_mask == 0x0000FF00 && header->green_mask == 0x00FF0000
                         && header->blue_mask == 0xFF000000)
                {
                    return SAIL_PIXEL_FORMAT_BPP32_BGRA;
                }
                else if (header->red_mask == 0x00FF0000 && header->green_mask == 0x0000FF00
                         && header->blue_mask == 0x000000FF)
                {
                    return SAIL_PIXEL_FORMAT_BPP32_ARGB;
                }
                else if (header->red_mask == 0x000000FF && header->green_mask == 0x0000FF00
                         && header->blue_mask == 0x00FF0000)
                {
                    return SAIL_PIXEL_FORMAT_BPP32_ABGR;
                }
                return SAIL_PIXEL_FORMAT_BPP32_RGBA;
            }
            else if (header->bits_per_pixel == 24)
            {
                if (header->red_mask == 0xFF0000 && header->green_mask == 0x00FF00 && header->blue_mask == 0x0000FF)
                {
                    return SAIL_PIXEL_FORMAT_BPP24_RGB;
                }
                else if (header->red_mask == 0x0000FF && header->green_mask == 0x00FF00
                         && header->blue_mask == 0xFF0000)
                {
                    return SAIL_PIXEL_FORMAT_BPP24_BGR;
                }
                return SAIL_PIXEL_FORMAT_BPP24_RGB;
            }
            else if (header->bits_per_pixel == 16)
            {
                if (header->red_mask == 0xF800 && header->green_mask == 0x07E0 && header->blue_mask == 0x001F)
                {
                    return SAIL_PIXEL_FORMAT_BPP16_RGB565;
                }
                else if (header->red_mask == 0x001F && header->green_mask == 0x07E0 && header->blue_mask == 0xF800)
                {
                    return SAIL_PIXEL_FORMAT_BPP16_BGR565;
                }
                else if (header->red_mask == 0x7C00 && header->green_mask == 0x03E0 && header->blue_mask == 0x001F)
                {
                    return SAIL_PIXEL_FORMAT_BPP16_RGB555;
                }
                else if (header->red_mask == 0x001F && header->green_mask == 0x03E0 && header->blue_mask == 0x7C00)
                {
                    return SAIL_PIXEL_FORMAT_BPP16_BGR555;
                }
                return SAIL_PIXEL_FORMAT_BPP16_RGB565;
            }
        }
        else if (header->visual_class == PseudoColor || header->visual_class == StaticColor
                 || header->visual_class == GrayScale || header->visual_class == StaticGray)
        {
            /* Indexed/grayscale images. */
            if (header->bits_per_pixel == 8 || header->pixmap_depth == 8)
            {
                return SAIL_PIXEL_FORMAT_BPP8_INDEXED;
            }
            else if (header->bits_per_pixel == 4 || header->pixmap_depth == 4)
            {
                return SAIL_PIXEL_FORMAT_BPP4_INDEXED;
            }
            else if (header->bits_per_pixel == 2 || header->pixmap_depth == 2)
            {
                return SAIL_PIXEL_FORMAT_BPP2_INDEXED;
            }
            else if (header->bits_per_pixel == 1 || header->pixmap_depth == 1)
            {
                return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
            }
        }
    }

    /* XYPixmap or XYBitmap format. */
    if (header->pixmap_format == XYPixmap || header->pixmap_format == XYBitmap)
    {
        if (header->pixmap_depth == 1)
        {
            return SAIL_PIXEL_FORMAT_BPP1_INDEXED;
        }
    }

    return SAIL_PIXEL_FORMAT_UNKNOWN;
}

sail_status_t xwd_private_read_pixels(struct sail_io* io,
                                      const struct XWDFileHeader* header,
                                      const struct XWDColor* colormap,
                                      struct sail_image* image)
{

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(header);
    SAIL_CHECK_PTR(image);

    bool byte_swap = !xwd_private_is_native_byte_order(header->byte_order);

    /* Read scanlines. */
    for (uint32_t y = 0; y < header->pixmap_height; y++)
    {
        unsigned char* scan = (unsigned char*)image->pixels + y * image->bytes_per_line;

        SAIL_TRY(io->strict_read(io->stream, scan, header->bytes_per_line));

        /* Handle byte swapping for multi-byte pixels. */
        if (byte_swap)
        {
            if (header->bits_per_pixel == 32)
            {
                uint32_t* pixels = (uint32_t*)scan;
                for (uint32_t x = 0; x < header->pixmap_width; x++)
                {
                    pixels[x] = sail_reverse_uint32(pixels[x]);
                }
            }
            else if (header->bits_per_pixel == 24)
            {
                /* 24-bit BGR/RGB - swap each pixel. */
                for (uint32_t x = 0; x < header->pixmap_width; x++)
                {
                    unsigned char tmp = scan[x * 3];
                    scan[x * 3]       = scan[x * 3 + 2];
                    scan[x * 3 + 2]   = tmp;
                }
            }
            else if (header->bits_per_pixel == 16)
            {
                uint16_t* pixels = (uint16_t*)scan;
                for (uint32_t x = 0; x < header->pixmap_width; x++)
                {
                    pixels[x] = sail_reverse_uint16(pixels[x]);
                }
            }
        }

        /* For indexed formats, convert pixel values to palette indices. */
        if (colormap != NULL && (header->visual_class == PseudoColor || header->visual_class == StaticColor))
        {
            /* Pixel values might not be sequential, need to map them. */
            if (header->bits_per_pixel == 8)
            {
                for (uint32_t x = 0; x < header->pixmap_width; x++)
                {
                    uint8_t pixel_value = scan[x];
                    /* Find the index in colormap. */
                    for (uint32_t i = 0; i < header->ncolors; i++)
                    {
                        if (colormap[i].pixel == pixel_value)
                        {
                            scan[x] = (uint8_t)i;
                            break;
                        }
                    }
                }
            }
        }
    }

    return SAIL_OK;
}

sail_status_t xwd_private_write_pixels(struct sail_io* io,
                                       const struct XWDFileHeader* header,
                                       const struct sail_image* image)
{

    SAIL_CHECK_PTR(io);
    SAIL_CHECK_PTR(header);
    SAIL_CHECK_PTR(image);

    /* Write scanlines. */
    for (uint32_t y = 0; y < header->pixmap_height; y++)
    {
        const unsigned char* scan = (const unsigned char*)image->pixels + y * image->bytes_per_line;

        SAIL_TRY(io->strict_write(io->stream, scan, header->bytes_per_line));
    }

    return SAIL_OK;
}

sail_status_t xwd_private_header_from_image(const struct sail_image* image, struct XWDFileHeader* header)
{

    SAIL_CHECK_PTR(image);
    SAIL_CHECK_PTR(header);

    memset(header, 0, sizeof(*header));

    header->header_size         = XWD_HEADER_SIZE;
    header->file_version        = XWD_FILE_VERSION;
    header->pixmap_width        = image->width;
    header->pixmap_height       = image->height;
    header->xoffset             = 0;
    header->window_width        = image->width;
    header->window_height       = image->height;
    header->window_x            = 0;
    header->window_y            = 0;
    header->window_border_width = 0;

    /* Determine byte order - use native. */
    uint32_t test         = 1;
    bool is_little_endian = (*((uint8_t*)&test) == 1);
    header->byte_order    = is_little_endian ? LSBFirst : MSBFirst;

    /* Configure based on pixel format. */
    switch (image->pixel_format)
    {
    case SAIL_PIXEL_FORMAT_BPP1_INDEXED:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 1;
        header->bits_per_pixel   = 1;
        header->visual_class     = PseudoColor;
        header->bitmap_unit      = 8;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 8;
        header->bytes_per_line   = (image->width + 7) / 8;
        header->red_mask         = 0;
        header->green_mask       = 0;
        header->blue_mask        = 0;
        header->bits_per_rgb     = 8;
        header->colormap_entries = (image->palette != NULL) ? image->palette->color_count : 2;
        header->ncolors          = (image->palette != NULL) ? image->palette->color_count : 2;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP2_INDEXED:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 2;
        header->bits_per_pixel   = 2;
        header->visual_class     = PseudoColor;
        header->bitmap_unit      = 8;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 8;
        header->bytes_per_line   = (image->width * 2 + 7) / 8;
        header->red_mask         = 0;
        header->green_mask       = 0;
        header->blue_mask        = 0;
        header->bits_per_rgb     = 8;
        header->colormap_entries = (image->palette != NULL) ? image->palette->color_count : 4;
        header->ncolors          = (image->palette != NULL) ? image->palette->color_count : 4;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP4_INDEXED:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 4;
        header->bits_per_pixel   = 4;
        header->visual_class     = PseudoColor;
        header->bitmap_unit      = 8;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 8;
        header->bytes_per_line   = (image->width * 4 + 7) / 8;
        header->red_mask         = 0;
        header->green_mask       = 0;
        header->blue_mask        = 0;
        header->bits_per_rgb     = 8;
        header->colormap_entries = (image->palette != NULL) ? image->palette->color_count : 16;
        header->ncolors          = (image->palette != NULL) ? image->palette->color_count : 16;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP8_INDEXED:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 8;
        header->bits_per_pixel   = 8;
        header->visual_class     = PseudoColor;
        header->bitmap_unit      = 8;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 8;
        header->bytes_per_line   = image->width;
        header->red_mask         = 0;
        header->green_mask       = 0;
        header->blue_mask        = 0;
        header->bits_per_rgb     = 8;
        header->colormap_entries = (image->palette != NULL) ? image->palette->color_count : 256;
        header->ncolors          = (image->palette != NULL) ? image->palette->color_count : 256;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP16_RGB555:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 15;
        header->bits_per_pixel   = 16;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 16;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 16;
        header->bytes_per_line   = image->width * 2;
        header->red_mask         = 0x7C00;
        header->green_mask       = 0x03E0;
        header->blue_mask        = 0x001F;
        header->bits_per_rgb     = 5;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP16_BGR555:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 15;
        header->bits_per_pixel   = 16;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 16;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 16;
        header->bytes_per_line   = image->width * 2;
        header->red_mask         = 0x001F;
        header->green_mask       = 0x03E0;
        header->blue_mask        = 0x7C00;
        header->bits_per_rgb     = 5;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP16_RGB565:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 16;
        header->bits_per_pixel   = 16;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 16;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 16;
        header->bytes_per_line   = image->width * 2;
        header->red_mask         = 0xF800;
        header->green_mask       = 0x07E0;
        header->blue_mask        = 0x001F;
        header->bits_per_rgb     = 6;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP16_BGR565:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 16;
        header->bits_per_pixel   = 16;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 16;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 16;
        header->bytes_per_line   = image->width * 2;
        header->red_mask         = 0x001F;
        header->green_mask       = 0x07E0;
        header->blue_mask        = 0xF800;
        header->bits_per_rgb     = 6;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP24_RGB:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 24;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 3;
        header->red_mask         = 0xFF0000;
        header->green_mask       = 0x00FF00;
        header->blue_mask        = 0x0000FF;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP24_BGR:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 24;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 3;
        header->red_mask         = 0x0000FF;
        header->green_mask       = 0x00FF00;
        header->blue_mask        = 0xFF0000;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_RGBA:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 32;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 4;
        header->red_mask         = 0xFF000000;
        header->green_mask       = 0x00FF0000;
        header->blue_mask        = 0x0000FF00;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_BGRA:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 32;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 4;
        header->red_mask         = 0x0000FF00;
        header->green_mask       = 0x00FF0000;
        header->blue_mask        = 0xFF000000;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ARGB:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 32;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 4;
        header->red_mask         = 0x00FF0000;
        header->green_mask       = 0x0000FF00;
        header->blue_mask        = 0x000000FF;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    case SAIL_PIXEL_FORMAT_BPP32_ABGR:
    {
        header->pixmap_format    = ZPixmap;
        header->pixmap_depth     = 24;
        header->bits_per_pixel   = 32;
        header->visual_class     = TrueColor;
        header->bitmap_unit      = 32;
        header->bitmap_bit_order = MSBFirst;
        header->bitmap_pad       = 32;
        header->bytes_per_line   = image->width * 4;
        header->red_mask         = 0x000000FF;
        header->green_mask       = 0x0000FF00;
        header->blue_mask        = 0x00FF0000;
        header->bits_per_rgb     = 8;
        header->colormap_entries = 0;
        header->ncolors          = 0;
        break;
    }
    default:
    {
        SAIL_LOG_ERROR("XWD: Unsupported pixel format %s for writing",
                       sail_pixel_format_to_string(image->pixel_format));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }
    }

    return SAIL_OK;
}

sail_status_t xwd_private_palette_to_colormap(const struct sail_palette* palette,
                                              struct XWDColor** colormap,
                                              uint32_t* ncolors)
{

    SAIL_CHECK_PTR(palette);
    SAIL_CHECK_PTR(colormap);
    SAIL_CHECK_PTR(ncolors);

    *ncolors = palette->color_count;

    if (*ncolors == 0)
    {
        *colormap = NULL;
        return SAIL_OK;
    }

    void* ptr;
    SAIL_TRY(sail_malloc(*ncolors * sizeof(struct XWDColor), &ptr));
    *colormap = ptr;

    /* Convert palette to XWD colormap. */
    const unsigned char* pal_data = palette->data;

    for (uint32_t i = 0; i < *ncolors; i++)
    {
        unsigned char r, g, b;

        switch (palette->pixel_format)
        {
        case SAIL_PIXEL_FORMAT_BPP24_RGB:
        {
            r = pal_data[i * 3 + 0];
            g = pal_data[i * 3 + 1];
            b = pal_data[i * 3 + 2];
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP24_BGR:
        {
            b = pal_data[i * 3 + 0];
            g = pal_data[i * 3 + 1];
            r = pal_data[i * 3 + 2];
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_RGBA:
        {
            r = pal_data[i * 4 + 0];
            g = pal_data[i * 4 + 1];
            b = pal_data[i * 4 + 2];
            /* Alpha channel ignored in XWD colormap. */
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_BGRA:
        {
            b = pal_data[i * 4 + 0];
            g = pal_data[i * 4 + 1];
            r = pal_data[i * 4 + 2];
            /* Alpha channel ignored in XWD colormap. */
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ARGB:
        {
            r = pal_data[i * 4 + 1];
            g = pal_data[i * 4 + 2];
            b = pal_data[i * 4 + 3];
            /* Alpha channel ignored in XWD colormap. */
            break;
        }
        case SAIL_PIXEL_FORMAT_BPP32_ABGR:
        {
            b = pal_data[i * 4 + 1];
            g = pal_data[i * 4 + 2];
            r = pal_data[i * 4 + 3];
            /* Alpha channel ignored in XWD colormap. */
            break;
        }
        default:
        {
            sail_free(*colormap);
            *colormap = NULL;
            SAIL_LOG_ERROR("XWD: Unsupported palette format %s", sail_pixel_format_to_string(palette->pixel_format));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT);
        }
        }

        (*colormap)[i].pixel = i;
        /* XWD uses 16-bit color values (0-65535). */
        (*colormap)[i].red   = (uint16_t)(r * 257);
        (*colormap)[i].green = (uint16_t)(g * 257);
        (*colormap)[i].blue  = (uint16_t)(b * 257);
        (*colormap)[i].flags = 0x07; /* DoRed | DoGreen | DoBlue. */
        (*colormap)[i].pad   = 0;
    }

    return SAIL_OK;
}
