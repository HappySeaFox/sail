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
 * Advanced API Demo
 *
 * This demonstrates the Advanced API level for loading animated/multi-paged images.
 *
 * Differences from other API levels:
 * - Junior: Simple one-line functions, only single-frame images
 * - Advanced: Frame-by-frame loading for animated/multi-paged images (GIF, WebP, TIFF, etc.)
 * - Deep diver: Full control over codec selection, metadata, and loading/saving options
 * - Technical diver: Everything above plus custom I/O sources (files, memory, network, etc.)
 *
 * Perfect for: Animated images like GIF and WebP, multi-paged documents like TIFF and PDF,
 *              or when you need to extract individual frames from an animation.
 *
 * If you need fine-grained control over codec options or want to use custom I/O sources,
 * take a look at the Deep Diver or Technical Diver API examples.
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
        fprintf(stderr, "Example: %s animation.gif\n", argv[0]);
        fprintf(stderr, "This will load and display information about all frames.\n");
        return 1;
    }

    const char* input_path = argv[1];

    /* Get codec info from file extension. */
    const struct sail_codec_info* codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_path(input_path, &codec_info),
                        /* on error */ return 1);

    printf("Codec: %s [%s]\n", codec_info->name, codec_info->description);

    /* Start loading - this opens the file and prepares for frame-by-frame reading. */
    void* load_state = NULL;
    SAIL_TRY_OR_EXECUTE(sail_start_loading_from_file(input_path, codec_info, &load_state),
                        /* on error */ return 1);

    /* Load all frames one by one. */
    struct sail_image* image;
    unsigned frame_number = 0;
    sail_status_t status;

    while ((status = sail_load_next_frame(load_state, &image)) == SAIL_OK)
    {
        frame_number++;
        printf("\nFrame #%u:\n", frame_number);
        printf("  Size: %ux%u\n", image->width, image->height);
        printf("  Pixel format: %s\n", sail_pixel_format_to_string(image->pixel_format));
        printf("  Delay: %d ms\n", image->delay);

        /* Process frame here (e.g., display, save, etc.). */
        sail_destroy_image(image);
    }

    /* Check if we finished successfully. */
    if (status != SAIL_ERROR_NO_MORE_FRAMES)
    {
        sail_stop_loading(load_state);
        return 1;
    }

    /* Stop loading - this closes the file and frees resources. */
    SAIL_TRY_OR_EXECUTE(sail_stop_loading(load_state),
                        /* on error */ return 1);

    printf("\nTotal frames loaded: %u\n", frame_number);

    return 0;
}
