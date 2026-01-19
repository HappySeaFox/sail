/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

/*
 * Junior API Demo
 *
 * This demonstrates the simplest SAIL API level - one-line image loading.
 *
 * Differences from other API levels:
 * - Junior: Simple one-line functions (sail_load_from_file)
 * - Advanced: Supports animated/multi-paged images with frame-by-frame loading
 * - Deep diver: Full control over codec selection, metadata, and loading options
 * - Technical diver: Everything above plus custom I/O sources (files, memory, network, etc.)
 *
 * Perfect for: Static single-frame images like JPEG, PNG, BMP. If you just need to load
 *              a regular image file, this is the easiest way to do it.
 *
 * For animated images (GIF, WebP) or multi-paged documents (TIFF, PDF), check out
 * the Advanced API example instead.
 *
 * Supported file formats: All formats supported by SAIL codecs
 */

#include <stdio.h>

#include <sail/sail.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        fprintf(stderr, "Example: %s input.jpg\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];

    /* Load image - one line. */
    struct sail_image* image;
    SAIL_TRY_OR_EXECUTE(sail_load_from_file(input_path, &image),
                        /* on error */ return 1);

    printf("File: %s\n", input_path);
    printf("Size: %ux%u\n", image->width, image->height);
    printf("Pixel format: %s\n", sail_pixel_format_to_string(image->pixel_format));
    printf("Source pixel format: %s\n",
           sail_pixel_format_to_string(image->source_image->pixel_format));
    printf("Compression: %s\n", sail_compression_to_string(image->source_image->compression));

    if (image->resolution != NULL)
    {
        printf("Resolution: %.1fx%.1f DPI\n", image->resolution->x, image->resolution->y);
    }

    if (image->iccp != NULL)
    {
        printf("ICC profile: yes (%zu bytes)\n", image->iccp->size);
    }

    if (image->gamma != 0)
    {
        printf("Gamma: %.6f\n", image->gamma);
    }

    printf("Interlaced: %s\n", image->source_image->interlaced ? "yes" : "no");
    printf("Delay: %d ms\n", image->delay);

    sail_destroy_image(image);

    return 0;
}
