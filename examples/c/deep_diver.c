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
 * Deep Diver API Demo
 *
 * This demonstrates the Deep Diver API level with full control over codec selection,
 * metadata, and loading options.
 *
 * Differences from other API levels:
 * - Junior: Simple one-line functions, no control over options
 * - Advanced: Frame-by-frame loading, but no control over codec options
 * - Deep diver: Full control over codec selection, load options, metadata access
 * - Technical diver: Everything above plus custom I/O sources (files, memory, network, etc.)
 *
 * Perfect for: When you need to fine-tune codec behavior, access metadata, or specify
 *              format-specific loading options. This gives you full control over how
 *              images are loaded.
 *
 * If you need custom I/O sources like network streams or encrypted files, check out
 * the Technical Diver API example instead.
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
        fprintf(stderr, "This demonstrates loading with options and displaying detailed information.\n");
        return 1;
    }

    const char* input_path = argv[1];

    /* Get codec info for input. */
    const struct sail_codec_info* input_codec_info;
    SAIL_TRY_OR_EXECUTE(sail_codec_info_from_path(input_path, &input_codec_info),
                        /* on error */ return 1);

    printf("Input codec: %s [%s]\n", input_codec_info->name, input_codec_info->description);

    /* Allocate load options from codec features. */
    struct sail_load_options* load_options;
    SAIL_TRY_OR_EXECUTE(sail_alloc_load_options_from_features(input_codec_info->load_features, &load_options),
                        /* on error */ return 1);

    /* Configure load options if needed (e.g., for JPEG: quality, progressive, etc.). */
    /* For this demo, we use defaults. */

    /* Start loading with options. */
    void* load_state = NULL;
    SAIL_TRY_OR_EXECUTE(sail_start_loading_from_file_with_options(input_path, input_codec_info, load_options, &load_state),
                        /* on error */ sail_destroy_load_options(load_options);
                                      return 1);

    sail_destroy_load_options(load_options);

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
        printf("  Source pixel format: %s\n",
               sail_pixel_format_to_string(image->source_image->pixel_format));
        printf("  Compression: %s\n", sail_compression_to_string(image->source_image->compression));

        if (image->resolution != NULL)
        {
            printf("  Resolution: %.1fx%.1f DPI\n", image->resolution->x, image->resolution->y);
        }

        if (image->iccp != NULL)
        {
            printf("  ICC profile: yes (%zu bytes)\n", image->iccp->size);
        }

        if (image->gamma != 0)
        {
            printf("  Gamma: %.6f\n", image->gamma);
        }

        printf("  Interlaced: %s\n", image->source_image->interlaced ? "yes" : "no");
        printf("  Delay: %d ms\n", image->delay);

        /* Display metadata if available. */
        if (image->meta_data_node != NULL)
        {
            printf("  Metadata:\n");
            for (const struct sail_meta_data_node* node = image->meta_data_node; node != NULL; node = node->next)
            {
                const struct sail_meta_data* meta_data = node->meta_data;
                const char* key_str                    = (meta_data->key == SAIL_META_DATA_UNKNOWN) ? meta_data->key_unknown
                                                                                                    : sail_meta_data_to_string(meta_data->key);
                printf("    %s: ", key_str);
                sail_printf_variant(meta_data->value);
                printf("\n");
            }
        }

        sail_destroy_image(image);
    }

    /* Check if we finished successfully. */
    if (status != SAIL_ERROR_NO_MORE_FRAMES)
    {
        sail_stop_loading(load_state);
        return 1;
    }

    /* Stop loading. */
    SAIL_TRY_OR_EXECUTE(sail_stop_loading(load_state),
                        /* on error */ return 1);

    printf("\nTotal frames loaded: %u\n", frame_number);

    return 0;
}
