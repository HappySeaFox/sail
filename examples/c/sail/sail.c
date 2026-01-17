/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* atoi */
#include <string.h>

#ifdef _WIN32
#include <io.h> /* _access */
#define F_OK 0
#else
#include <unistd.h> /* access */
#endif

#include <sail/sail.h>

#include <sail-manip/sail-manip.h>

/*
 * MSVC-specific safe functions.
 * MSVC provides _s (secure) versions of standard C functions that include additional
 * buffer size checks and validation. These macros provide compatibility between
 * MSVC and other compilers while maintaining safety.
 */
#ifdef _MSC_VER
/* Use MSVC secure versions with _TRUNCATE flag for safe truncation */
#define sail_snprintf(buf, size, fmt, ...) _snprintf_s(buf, size, _TRUNCATE, fmt, __VA_ARGS__)
#define sail_strncpy(dst, src, size) strncpy_s(dst, size, src, _TRUNCATE)
#define sail_sscanf sscanf_s
#define sail_access _access
#else
/* Standard C functions with manual null-termination for safety */
#define sail_snprintf snprintf
#define sail_strncpy(dst, src, size) (strncpy(dst, src, size), (dst)[(size) - 1] = '\0')
#define sail_sscanf sscanf
#define sail_access access
#endif

static void print_invalid_argument(void)
{
    fprintf(stderr, "Error: Invalid arguments. Run with -h to see command arguments.\n");
}

static void consume_input_line(void)
{
    int next = getchar();
    if (next != '\n' && next != EOF)
    {
        /* Clear the rest of the line. */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
}

enum OverwriteChoice
{
    OVERWRITE_CHOICE_YES,
    OVERWRITE_CHOICE_NO,
    OVERWRITE_CHOICE_YES_ALL,
    OVERWRITE_CHOICE_NO_ALL
};

static enum OverwriteChoice read_overwrite_choice(void)
{
    int c = getchar();

    if (c == 'y' || c == 'Y')
    {
        consume_input_line();
        return OVERWRITE_CHOICE_YES;
    }
    else if (c == 'a' || c == 'A')
    {
        consume_input_line();
        return OVERWRITE_CHOICE_YES_ALL;
    }
    else if (c == 'd' || c == 'D')
    {
        consume_input_line();
        return OVERWRITE_CHOICE_NO_ALL;
    }
    else
    {
        if (c != '\n' && c != EOF)
        {
            consume_input_line();
        }
        return OVERWRITE_CHOICE_NO;
    }
}

static bool check_file_overwrite(const char* filepath, bool* auto_yes, bool* auto_no)
{
    if (sail_access(filepath, F_OK) == 0)
    {
        if (*auto_no)
        {
            fprintf(stderr, "Skipping file '%s'.\n", filepath);
            return false;
        }

        if (*auto_yes)
        {
            return true;
        }

        fprintf(stderr, "File '%s' already exists. Overwrite? [y/N/a(all)/d(none)]: ", filepath);
        fflush(stderr);

        enum OverwriteChoice choice = read_overwrite_choice();

        switch (choice)
        {
        case OVERWRITE_CHOICE_YES:
            return true;

        case OVERWRITE_CHOICE_YES_ALL:
            *auto_yes = true;
            return true;

        case OVERWRITE_CHOICE_NO_ALL:
            *auto_no = true;
            fprintf(stderr, "Skipping file '%s'.\n", filepath);
            return false;

        case OVERWRITE_CHOICE_NO:
        default:
            fprintf(stderr, "Skipping file '%s'.\n", filepath);
            return false;
        }
    }

    return true;
}

static sail_status_t convert_impl(const char** inputs,
                                  int input_count,
                                  const char* output,
                                  enum SailPixelFormat pixel_format,
                                  int compression,
                                  int max_frames,
                                  int target_frame,
                                  int delay,
                                  int colors,
                                  bool dither,
                                  const char* background,
                                  bool strip_metadata,
                                  bool flip_horizontal,
                                  bool flip_vertical,
                                  bool* auto_yes,
                                  bool* auto_no)
{
    SAIL_CHECK_PTR(inputs);
    SAIL_CHECK_PTR(output);

    if (input_count < 1)
    {
        SAIL_LOG_ERROR("No input files specified");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    const struct sail_codec_info* output_codec_info;
    void* save_state = NULL;

    /* Setup output. */
    SAIL_LOG_DEBUG("Output file: %s", output);
    SAIL_LOG_DEBUG("Processing %d input file(s)", input_count);

    SAIL_TRY(sail_codec_info_from_path(output, &output_codec_info));
    SAIL_LOG_DEBUG("Output codec: %s", output_codec_info->description);

    /* Check if output file exists and ask for confirmation. */
    if (!check_file_overwrite(output, auto_yes, auto_no))
    {
        return SAIL_OK;
    }

    struct sail_save_options* save_options;
    SAIL_TRY(sail_alloc_save_options_from_features(output_codec_info->save_features, &save_options));

    /* Apply our tuning. */
    SAIL_LOG_DEBUG("Compression: %d%s", compression, compression == -1 ? " (default)" : "");
    save_options->compression_level = compression;

    /* Determine output mode based on delay parameter and format capabilities. */
    bool output_supports_animated = (output_codec_info->save_features->features & SAIL_CODEC_FEATURE_ANIMATED) != 0;
    bool output_supports_multi_paged =
        (output_codec_info->save_features->features & SAIL_CODEC_FEATURE_MULTI_PAGED) != 0;

    /* Check if output format supports animation or multi-paged. If not, force limit to 1 frame. */
    if (!output_supports_animated && !output_supports_multi_paged)
    {
        if (max_frames > 0 || input_count > 1)
        {
            SAIL_LOG_WARNING("Output format doesn't support animation/multi-page, forcing to 1 frame");
        }
        max_frames = 1;
    }

    /* If target_frame is specified, adjust max_frames to allow reaching that frame. */
    if (target_frame > 0)
    {
        max_frames = target_frame;
    }

    /* Log the output mode. */
    if (input_count > 1)
    {
        if (delay >= 0)
        {
            SAIL_LOG_DEBUG("Composing %d files into animation with %d ms delay", input_count, delay);
        }
        else if (output_supports_multi_paged)
        {
            SAIL_LOG_DEBUG("Composing %d files into multi-page document", input_count);
        }
        else if (output_supports_animated)
        {
            SAIL_LOG_DEBUG("Composing %d files into animation with default delay", input_count);
        }
    }
    else
    {
        if (delay >= 0)
        {
            SAIL_LOG_DEBUG("Delay specified (%d ms), creating animation", delay);
        }
        else if (output_supports_multi_paged)
        {
            SAIL_LOG_DEBUG("No delay specified, creating multi-page document");
        }
        else if (output_supports_animated)
        {
            SAIL_LOG_DEBUG("Creating animation with original frame delays");
        }
    }

    /* Process all input files. */
    int total_frame_count = 0;

    for (int file_idx = 0; file_idx < input_count; file_idx++)
    {
        const char* input = inputs[file_idx];
        const struct sail_codec_info* input_codec_info;
        void* load_state;
        struct sail_image* image;

        /* Load the image. */
        SAIL_LOG_DEBUG("Input file #%d: %s", file_idx + 1, input);

        SAIL_TRY_OR_CLEANUP(sail_codec_info_from_path(input, &input_codec_info), sail_stop_saving(save_state);
                            sail_destroy_save_options(save_options));
        SAIL_LOG_DEBUG("Input codec: %s", input_codec_info->description);

        /* Use SOURCE_IMAGE option to preserve original pixel format when possible. */
        struct sail_load_options* load_options;
        SAIL_TRY_OR_CLEANUP(sail_alloc_load_options_from_features(input_codec_info->load_features, &load_options),
                            sail_stop_saving(save_state);
                            sail_destroy_save_options(save_options));

        SAIL_TRY_OR_CLEANUP(
            sail_start_loading_from_file_with_options(input, input_codec_info, load_options, &load_state),
            sail_destroy_load_options(load_options);
            sail_stop_saving(save_state); sail_destroy_save_options(save_options));
        sail_destroy_load_options(load_options);

        /* Convert all frames from this input file. */
        sail_status_t load_status;
        int file_frame_count = 0;

        while ((load_status = sail_load_next_frame(load_state, &image)) == SAIL_OK)
        {
            /* Check if we need to skip frames to reach target frame. */
            if (target_frame > 0 && total_frame_count < target_frame - 1)
            {
                SAIL_LOG_DEBUG("Skipping frame #%d (file #%d, frame #%d, waiting for frame #%d", total_frame_count,
                               file_idx + 1, file_frame_count, target_frame);
                sail_destroy_image(image);
                total_frame_count++;
                file_frame_count++;
                continue;
            }

            /* Check max frames limit. */
            if (max_frames > 0 && total_frame_count >= max_frames)
            {
                SAIL_LOG_DEBUG("Reached max frames limit (%d), stopping", max_frames);
                sail_destroy_image(image);
                break;
            }

            SAIL_LOG_DEBUG("Processing frame #%d (file #%d, frame #%d)", total_frame_count, file_idx + 1,
                           file_frame_count);

            /* Setup conversion options if needed. */
            struct sail_conversion_options* conversion_options = NULL;
            if (background != NULL || dither)
            {
                SAIL_TRY_OR_CLEANUP(sail_alloc_conversion_options(&conversion_options), sail_destroy_image(image);
                                    sail_stop_loading(load_state); sail_stop_saving(save_state);
                                    sail_destroy_save_options(save_options));

                if (background != NULL)
                {
                    /* Parse background color. */
                    unsigned r, g, b;
                    if (strcmp(background, "white") == 0)
                    {
                        r = g = b = 255;
                    }
                    else if (strcmp(background, "black") == 0)
                    {
                        r = g = b = 0;
                    }
                    else if (background[0] != '#' || strlen(background) != 7
                             || sail_sscanf(background + 1, "%02x%02x%02x", &r, &g, &b) != 3)
                    {
                        SAIL_LOG_ERROR("Invalid background color: %s", background);
                        sail_destroy_conversion_options(conversion_options);
                        sail_destroy_image(image);
                        sail_stop_loading(load_state);
                        sail_stop_saving(save_state);
                        sail_destroy_save_options(save_options);
                        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                    }

                    conversion_options->options                 |= SAIL_CONVERSION_OPTION_BLEND_ALPHA;
                    conversion_options->background24.component1  = (uint8_t)r;
                    conversion_options->background24.component2  = (uint8_t)g;
                    conversion_options->background24.component3  = (uint8_t)b;
                    conversion_options->background48.component1  = (uint16_t)(r * 257);
                    conversion_options->background48.component2  = (uint16_t)(g * 257);
                    conversion_options->background48.component3  = (uint16_t)(b * 257);
                    SAIL_LOG_DEBUG("Background color: #%02X%02X%02X", r, g, b);
                }

                if (dither)
                {
                    conversion_options->options |= SAIL_CONVERSION_OPTION_DITHERING;
                    SAIL_LOG_DEBUG("Dithering enabled");
                }
            }

            /* Convert to the appropriate pixel format. */
            struct sail_image* image_converted;

            /* If quantization is requested, convert to RGB for quantization input. */
            if (colors > 0)
            {
                SAIL_LOG_DEBUG("Converting to BPP24-RGB for quantization");
                if (conversion_options != NULL)
                {
                    SAIL_TRY_OR_CLEANUP(sail_convert_image_with_options(image, SAIL_PIXEL_FORMAT_BPP24_RGB,
                                                                        conversion_options, &image_converted),
                                        sail_destroy_conversion_options(conversion_options);
                                        sail_destroy_image(image); sail_stop_loading(load_state);
                                        sail_stop_saving(save_state); sail_destroy_save_options(save_options));
                }
                else
                {
                    SAIL_TRY_OR_CLEANUP(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &image_converted),
                                        sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }

                sail_destroy_conversion_options(conversion_options);
                sail_destroy_image(image);
                image = image_converted;

                /* Apply flip before quantization (RGB is byte-aligned). */
                if (flip_horizontal)
                {
                    SAIL_LOG_DEBUG("Flipping horizontally");
                    SAIL_TRY_OR_CLEANUP(sail_mirror_horizontally(image), sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }
                if (flip_vertical)
                {
                    SAIL_LOG_DEBUG("Flipping vertically");
                    SAIL_TRY_OR_CLEANUP(sail_mirror_vertically(image), sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }

                /* Now quantize RGB to indexed. */
                /* Determine indexed pixel format based on requested colors. */
                enum SailPixelFormat indexed_format;
                if (colors <= 2)
                {
                    indexed_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
                }
                else if (colors <= 4)
                {
                    indexed_format = SAIL_PIXEL_FORMAT_BPP2_INDEXED;
                }
                else if (colors <= 16)
                {
                    indexed_format = SAIL_PIXEL_FORMAT_BPP4_INDEXED;
                }
                else
                {
                    indexed_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
                }

                SAIL_LOG_DEBUG("Quantizing to %s%s", sail_pixel_format_to_string(indexed_format),
                               dither ? " with dithering" : "");
                struct sail_image* image_quantized;
                SAIL_TRY_OR_CLEANUP(sail_quantize_image(image, indexed_format, dither, &image_quantized),
                                    sail_destroy_image(image);
                                    sail_stop_loading(load_state); sail_stop_saving(save_state);
                                    sail_destroy_save_options(save_options));
                sail_destroy_image(image);
                image = image_quantized;
            }
            /* Otherwise convert to format suitable for saving. */
            else if (pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN)
            {
                SAIL_LOG_DEBUG("Converting to specified pixel format: %s", sail_pixel_format_to_string(pixel_format));
                if (conversion_options != NULL)
                {
                    SAIL_TRY_OR_CLEANUP(
                        sail_convert_image_with_options(image, pixel_format, conversion_options, &image_converted),
                        sail_destroy_conversion_options(conversion_options);
                        sail_destroy_image(image); sail_stop_loading(load_state); sail_stop_saving(save_state);
                        sail_destroy_save_options(save_options));
                }
                else
                {
                    SAIL_TRY_OR_CLEANUP(sail_convert_image(image, pixel_format, &image_converted),
                                        sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }

                sail_destroy_conversion_options(conversion_options);
                sail_destroy_image(image);
                image = image_converted;
            }
            else
            {
                if (conversion_options != NULL)
                {
                    SAIL_TRY_OR_CLEANUP(
                        sail_convert_image_for_saving_with_options(image, output_codec_info->save_features,
                                                                   conversion_options, &image_converted),
                        sail_destroy_conversion_options(conversion_options);
                        sail_destroy_image(image); sail_stop_loading(load_state); sail_stop_saving(save_state);
                        sail_destroy_save_options(save_options));
                }
                else
                {
                    SAIL_TRY_OR_CLEANUP(
                        sail_convert_image_for_saving(image, output_codec_info->save_features, &image_converted),
                        sail_destroy_image(image);
                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                        sail_destroy_save_options(save_options));
                }

                sail_destroy_conversion_options(conversion_options);
                sail_destroy_image(image);
                image = image_converted;
            }

            /* Apply flip after conversion if not quantizing. */
            if (colors == 0)
            {
                if (flip_horizontal)
                {
                    SAIL_LOG_DEBUG("Flipping horizontally");
                    SAIL_TRY_OR_CLEANUP(sail_mirror_horizontally(image), sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }
                if (flip_vertical)
                {
                    SAIL_LOG_DEBUG("Flipping vertically");
                    SAIL_TRY_OR_CLEANUP(sail_mirror_vertically(image), sail_destroy_image(image);
                                        sail_stop_loading(load_state); sail_stop_saving(save_state);
                                        sail_destroy_save_options(save_options));
                }
            }

            /* Strip metadata if requested. */
            if (strip_metadata)
            {
                if (image->meta_data_node != NULL)
                {
                    SAIL_LOG_DEBUG("Stripping metadata");
                    sail_destroy_meta_data_node_chain(image->meta_data_node);
                    image->meta_data_node = NULL;
                }
            }

            /* Apply delay based on user intent and format capabilities. */
            if (delay >= 0)
            {
                image->delay = delay;
            }
            else if (output_supports_multi_paged)
            {
                image->delay = 0;
            }

            /* Start saving on first frame to be processed (not skipped). */
            if (save_state == NULL)
            {
                SAIL_TRY_OR_CLEANUP(
                    sail_start_saving_into_file_with_options(output, output_codec_info, save_options, &save_state),
                    sail_destroy_image(image);
                    sail_stop_loading(load_state); sail_destroy_save_options(save_options));
            }

            /* Write frame. */
            SAIL_TRY_OR_CLEANUP(sail_write_next_frame(save_state, image), sail_destroy_image(image);
                                sail_stop_loading(load_state); sail_stop_saving(save_state);
                                sail_destroy_save_options(save_options));

            sail_destroy_image(image);
            total_frame_count++;
            file_frame_count++;

            /* If we're extracting a specific frame, stop after processing it. */
            if (target_frame > 0 && total_frame_count >= target_frame)
            {
                SAIL_LOG_DEBUG("Extracted target frame #%d, stopping", target_frame);
                break;
            }
        }

        SAIL_TRY_OR_CLEANUP(sail_stop_loading(load_state), sail_stop_saving(save_state);
                            sail_destroy_save_options(save_options));

        SAIL_LOG_DEBUG("Processed %d frame(s) from file #%d", file_frame_count, file_idx + 1);

        /* For composition mode, break after processing all frames if max_frames reached. */
        if (max_frames > 0 && total_frame_count >= max_frames)
        {
            break;
        }
    }

    /* Check if we processed at least one frame. */
    if (total_frame_count == 0)
    {
        SAIL_LOG_ERROR("No frames found in input files");
        sail_destroy_save_options(save_options);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    SAIL_LOG_DEBUG("Total: converted %d frame(s)", total_frame_count);

    /* Clean up. */
    SAIL_TRY(sail_stop_saving(save_state));
    sail_destroy_save_options(save_options);

    return SAIL_OK;
}

static sail_status_t extract_frames_impl(const char* input,
                                         const char* output_template,
                                         enum SailPixelFormat pixel_format,
                                         int compression,
                                         int max_frames,
                                         int target_frame,
                                         int colors,
                                         bool dither,
                                         const char* background,
                                         bool strip_metadata,
                                         bool flip_horizontal,
                                         bool flip_vertical,
                                         bool* auto_yes,
                                         bool* auto_no,
                                         int suffix_digits)
{
    SAIL_CHECK_PTR(input);
    SAIL_CHECK_PTR(output_template);

    const struct sail_codec_info* input_codec_info;
    void* load_state;
    struct sail_image* image;

    /* Load the image. */
    SAIL_LOG_DEBUG("Input file: %s", input);
    SAIL_LOG_DEBUG("Extracting frames to: %s", output_template);

    SAIL_TRY(sail_codec_info_from_path(input, &input_codec_info));
    SAIL_LOG_DEBUG("Input codec: %s", input_codec_info->description);

    /* Use SOURCE_IMAGE option to preserve original pixel format when possible. */
    struct sail_load_options* load_options;
    SAIL_TRY(sail_alloc_load_options_from_features(input_codec_info->load_features, &load_options));

    SAIL_TRY_OR_CLEANUP(sail_start_loading_from_file_with_options(input, input_codec_info, load_options, &load_state),
                        sail_destroy_load_options(load_options));
    sail_destroy_load_options(load_options);

    /* Extract the file extension from output template. */
    const char* ext           = strrchr(output_template, '.');
    const char* base_name_end = ext ? ext : (output_template + strlen(output_template));
    const char* dir_sep       = strrchr(output_template, '/');
    if (dir_sep == NULL)
    {
        dir_sep = strrchr(output_template, '\\');
    }
    const char* base_name_start = dir_sep ? (dir_sep + 1) : output_template;
    size_t base_name_len        = base_name_end - base_name_start;

    /* If target_frame is specified, adjust max_frames to allow reaching that frame. */
    if (target_frame > 0)
    {
        max_frames = target_frame;
    }

    /* Extract all frames. */
    sail_status_t load_status;
    int frame_count = 0;

    while ((load_status = sail_load_next_frame(load_state, &image)) == SAIL_OK)
    {
        /* Check if we need to skip frames to reach target frame. */
        if (target_frame > 0 && frame_count < target_frame - 1)
        {
            SAIL_LOG_DEBUG("Skipping frame #%d, waiting for frame #%d", frame_count, target_frame);
            sail_destroy_image(image);
            frame_count++;
            continue;
        }

        /* Check max frames limit. */
        if (max_frames > 0 && frame_count >= max_frames)
        {
            SAIL_LOG_DEBUG("Reached max frames limit (%d), stopping", max_frames);
            sail_destroy_image(image);
            break;
        }

        /* Construct output filename: base-N.ext */
        char output_filename[1024];
        size_t safe_base_name_len = base_name_len;

        if (safe_base_name_len > sizeof(output_filename) - 20)
        {
            safe_base_name_len = sizeof(output_filename) - 20; /* Reserve space for "-N" and extension */
        }

        long written;
        if (suffix_digits > 0)
        {
            /* Format with specified number of digits, e.g., %03d for 3 digits */
            char format_str[32];
            sail_snprintf(format_str, sizeof(format_str), "%%.*s-%%0%dd%%s", suffix_digits);
            written = sail_snprintf(output_filename, sizeof(output_filename), format_str, (int)safe_base_name_len,
                                     base_name_start, frame_count + 1, ext ? ext : "");
        }
        else
        {
            /* Default format without zero-padding */
            written = sail_snprintf(output_filename, sizeof(output_filename), "%.*s-%d%s", (int)safe_base_name_len,
                                     base_name_start, frame_count + 1, ext ? ext : "");
        }
        if (written < 0 || written >= (long)sizeof(output_filename))
        {
            SAIL_LOG_ERROR("Failed to construct path from '%s' and '%s'", input, output_template);
            sail_destroy_image(image);
            sail_stop_loading(load_state);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
        }

        /* Add directory prefix if present. */
        if (dir_sep)
        {
            char full_path[1024];
            size_t dir_len = dir_sep - output_template + 1;

            written =
                sail_snprintf(full_path, sizeof(full_path), "%.*s%s", (int)dir_len, output_template, output_filename);
            if (written < 0 || written >= (long)sizeof(full_path))
            {
                SAIL_LOG_ERROR("Failed to construct path from '%s' and '%s'", output_template, output_filename);
                sail_destroy_image(image);
                sail_stop_loading(load_state);
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }
            sail_strncpy(output_filename, full_path, sizeof(output_filename));
        }

        SAIL_LOG_DEBUG("Extracting frame #%d to %s", frame_count, output_filename);

        /* Check if output file exists and ask for confirmation. */
        if (!check_file_overwrite(output_filename, auto_yes, auto_no))
        {
            sail_destroy_image(image);
            frame_count++;
            continue;
        }

        /* Determine output codec from filename. */
        const struct sail_codec_info* output_codec_info;
        SAIL_TRY_OR_CLEANUP(sail_codec_info_from_path(output_filename, &output_codec_info), sail_destroy_image(image);
                            sail_stop_loading(load_state));

        /* Setup conversion options if needed. */
        struct sail_conversion_options* conversion_options = NULL;
        if (background != NULL || dither)
        {
            SAIL_TRY_OR_CLEANUP(sail_alloc_conversion_options(&conversion_options), sail_destroy_image(image);
                                sail_stop_loading(load_state));

            if (background != NULL)
            {
                /* Parse background color. */
                unsigned r, g, b;
                if (strcmp(background, "white") == 0)
                {
                    r = g = b = 255;
                }
                else if (strcmp(background, "black") == 0)
                {
                    r = g = b = 0;
                }
                else if (background[0] != '#' || strlen(background) != 7
                         || sail_sscanf(background + 1, "%02x%02x%02x", &r, &g, &b) != 3)
                {
                    SAIL_LOG_ERROR("Invalid background color: %s", background);
                    sail_destroy_conversion_options(conversion_options);
                    sail_destroy_image(image);
                    sail_stop_loading(load_state);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }

                conversion_options->options                 |= SAIL_CONVERSION_OPTION_BLEND_ALPHA;
                conversion_options->background24.component1  = (uint8_t)r;
                conversion_options->background24.component2  = (uint8_t)g;
                conversion_options->background24.component3  = (uint8_t)b;
                conversion_options->background48.component1  = (uint16_t)(r * 257);
                conversion_options->background48.component2  = (uint16_t)(g * 257);
                conversion_options->background48.component3  = (uint16_t)(b * 257);
            }

            if (dither)
            {
                conversion_options->options |= SAIL_CONVERSION_OPTION_DITHERING;
            }
        }

        /* Convert to the appropriate pixel format. */
        struct sail_image* image_converted;

        if (colors > 0)
        {
            if (conversion_options != NULL)
            {
                SAIL_TRY_OR_CLEANUP(sail_convert_image_with_options(image, SAIL_PIXEL_FORMAT_BPP24_RGB,
                                                                    conversion_options, &image_converted),
                                    sail_destroy_conversion_options(conversion_options);
                                    sail_destroy_image(image); sail_stop_loading(load_state));
            }
            else
            {
                SAIL_TRY_OR_CLEANUP(sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &image_converted),
                                    sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }

            sail_destroy_conversion_options(conversion_options);
            sail_destroy_image(image);
            image = image_converted;

            /* Apply flip before quantization (RGB is byte-aligned). */
            if (flip_horizontal)
            {
                SAIL_TRY_OR_CLEANUP(sail_mirror_horizontally(image), sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }
            if (flip_vertical)
            {
                SAIL_TRY_OR_CLEANUP(sail_mirror_vertically(image), sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }

            /* Now quantize RGB to indexed. */
            /* Determine indexed pixel format based on requested colors. */
            enum SailPixelFormat indexed_format;
            if (colors <= 2)
            {
                indexed_format = SAIL_PIXEL_FORMAT_BPP1_INDEXED;
            }
            else if (colors <= 4)
            {
                indexed_format = SAIL_PIXEL_FORMAT_BPP2_INDEXED;
            }
            else if (colors <= 16)
            {
                indexed_format = SAIL_PIXEL_FORMAT_BPP4_INDEXED;
            }
            else
            {
                indexed_format = SAIL_PIXEL_FORMAT_BPP8_INDEXED;
            }

            struct sail_image* image_quantized;
            SAIL_TRY_OR_CLEANUP(sail_quantize_image(image, indexed_format, dither, &image_quantized),
                                sail_destroy_image(image);
                                sail_stop_loading(load_state));
            sail_destroy_image(image);
            image = image_quantized;
        }
        /* Otherwise convert to format suitable for saving. */
        else if (pixel_format != SAIL_PIXEL_FORMAT_UNKNOWN)
        {
            if (conversion_options != NULL)
            {
                SAIL_TRY_OR_CLEANUP(
                    sail_convert_image_with_options(image, pixel_format, conversion_options, &image_converted),
                    sail_destroy_conversion_options(conversion_options);
                    sail_destroy_image(image); sail_stop_loading(load_state));
            }
            else
            {
                SAIL_TRY_OR_CLEANUP(sail_convert_image(image, pixel_format, &image_converted),
                                    sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }

            sail_destroy_conversion_options(conversion_options);
            sail_destroy_image(image);
            image = image_converted;
        }
        else
        {
            if (conversion_options != NULL)
            {
                SAIL_TRY_OR_CLEANUP(sail_convert_image_for_saving_with_options(image, output_codec_info->save_features,
                                                                               conversion_options, &image_converted),
                                    sail_destroy_conversion_options(conversion_options);
                                    sail_destroy_image(image); sail_stop_loading(load_state));
            }
            else
            {
                SAIL_TRY_OR_CLEANUP(
                    sail_convert_image_for_saving(image, output_codec_info->save_features, &image_converted),
                    sail_destroy_image(image);
                    sail_stop_loading(load_state));
            }

            sail_destroy_conversion_options(conversion_options);
            sail_destroy_image(image);
            image = image_converted;
        }

        /* Apply flip after conversion if not quantizing. */
        if (colors == 0)
        {
            if (flip_horizontal)
            {
                SAIL_TRY_OR_CLEANUP(sail_mirror_horizontally(image), sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }
            if (flip_vertical)
            {
                SAIL_TRY_OR_CLEANUP(sail_mirror_vertically(image), sail_destroy_image(image);
                                    sail_stop_loading(load_state));
            }
        }

        /* Strip metadata if requested. */
        if (strip_metadata)
        {
            if (image->meta_data_node != NULL)
            {
                sail_destroy_meta_data_node_chain(image->meta_data_node);
                image->meta_data_node = NULL;
            }
        }

        /* Setup save options. */
        struct sail_save_options* save_options;
        SAIL_TRY_OR_CLEANUP(sail_alloc_save_options_from_features(output_codec_info->save_features, &save_options),
                            sail_destroy_image(image);
                            sail_stop_loading(load_state));

        save_options->compression_level = compression;

        /* Save single frame. */
        void* save_state;
        SAIL_TRY_OR_CLEANUP(
            sail_start_saving_into_file_with_options(output_filename, output_codec_info, save_options, &save_state),
            sail_destroy_image(image);
            sail_destroy_save_options(save_options); sail_stop_loading(load_state));

        SAIL_TRY_OR_CLEANUP(sail_write_next_frame(save_state, image), sail_destroy_image(image);
                            sail_stop_saving(save_state); sail_destroy_save_options(save_options);
                            sail_stop_loading(load_state));

        SAIL_TRY_OR_CLEANUP(sail_stop_saving(save_state), sail_destroy_image(image);
                            sail_destroy_save_options(save_options); sail_stop_loading(load_state));

        sail_destroy_image(image);
        sail_destroy_save_options(save_options);
        frame_count++;

        /* If we're extracting a specific frame, stop after processing it. */
        if (target_frame > 0 && frame_count >= target_frame)
        {
            SAIL_LOG_DEBUG("Extracted target frame #%d, stopping", target_frame);
            break;
        }
    }

    SAIL_TRY(sail_stop_loading(load_state));

    /* Check if we processed at least one frame. */
    if (frame_count == 0)
    {
        SAIL_LOG_ERROR("No frames found in input file");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_NO_MORE_FRAMES);
    }

    SAIL_LOG_DEBUG("Extracted %d frame(s)", frame_count);

    return SAIL_OK;
}

static sail_status_t convert(int argc, char* argv[])
{
    if (argc < 4)
    {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* -1: default compression will be selected. */
    int compression = -1;
    /* 0: convert all frames. */
    int max_frames = 0;
    /* 0: no specific frame target, >0: extract specific frame number. */
    int target_frame = 0;
    /* UNKNOWN: auto-select best format. */
    enum SailPixelFormat pixel_format = SAIL_PIXEL_FORMAT_UNKNOWN;
    /* -1: no delay specified, use original or default based on format. */
    int delay = -1;
    /* false: compose/convert mode, true: extract frames mode. */
    bool extract_frames = false;
    /* 0: default suffix format, >0: number of digits in suffix (e.g., 3 for 001, 002, ...). */
    int suffix_digits = 0;
    /* 0: no quantization, >0: quantize to N colors. */
    int colors = 0;
    /* false: no dithering (default), true: apply Floyd-Steinberg dithering. */
    bool dither = false;
    /* NULL: no background (default). */
    const char* background = NULL;
    /* false: preserve metadata (default), true: strip metadata. */
    bool strip_metadata = false;
    /* false: no flip (default). */
    bool flip_horizontal = false;
    bool flip_vertical   = false;
    /* false: ask for confirmation (default), true: auto-overwrite. */
    bool auto_yes = false;
    /* false: ask for confirmation (default), true: auto-skip all. */
    bool auto_no = false;

    /* Collect positional arguments (file paths). */
    const char* files[256];
    int file_count = 0;

    /* Parse arguments: first collect all files, then parse options. */
    int i = 2; /* Skip program name and "convert" command. */

    while (i < argc)
    {
        /* Check if this is an option. */
        if (argv[i][0] == '-')
        {
            /* Parse option. */
            if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compression") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing compression value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                compression  = atoi(argv[i + 1]);
                i           += 2;
                continue;
            }

            if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max-frames") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing max-frames value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                max_frames  = atoi(argv[i + 1]);
                i          += 2;
                continue;
            }

            if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pixel-format") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing pixel-format value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                pixel_format = sail_pixel_format_from_string(argv[i + 1]);
                if (pixel_format == SAIL_PIXEL_FORMAT_UNKNOWN)
                {
                    fprintf(stderr, "Error: Unknown pixel format '%s'.\n", argv[i + 1]);
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                i += 2;
                continue;
            }

            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing delay value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                delay = atoi(argv[i + 1]);
                if (delay < 0)
                {
                    fprintf(stderr, "Error: Delay must be non-negative.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                i += 2;
                continue;
            }

            if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--extract-frames") == 0)
            {
                extract_frames  = true;
                i              += 1;
                continue;
            }

            if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--suffix-digits") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing suffix-digits value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                suffix_digits = atoi(argv[i + 1]);
                if (suffix_digits < 1 || suffix_digits > 10)
                {
                    fprintf(stderr, "Error: Suffix digits must be between 1 and 10.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                i += 2;
                continue;
            }

            if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--colors") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing colors value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                colors = atoi(argv[i + 1]);
                if (colors < 2 || colors > 256)
                {
                    fprintf(stderr, "Error: Colors must be between 2 and 256.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                i += 2;
                continue;
            }

            if (strcmp(argv[i], "-D") == 0 || strcmp(argv[i], "--dither") == 0)
            {
                dither  = true;
                i      += 1;
                continue;
            }

            if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--background") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing background value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                background  = argv[i + 1];
                i          += 2;
                continue;
            }

            if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--strip") == 0)
            {
                strip_metadata  = true;
                i              += 1;
                continue;
            }

            if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--flip-horizontal") == 0)
            {
                flip_horizontal  = true;
                i               += 1;
                continue;
            }

            if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--flip-vertical") == 0)
            {
                flip_vertical  = true;
                i             += 1;
                continue;
            }

            if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "--yes") == 0)
            {
                auto_yes  = true;
                i        += 1;
                continue;
            }

            if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--frame-number") == 0)
            {
                if (i == argc - 1)
                {
                    fprintf(stderr, "Error: Missing frame number value.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                int frame_number = atoi(argv[i + 1]);
                if (frame_number < 1)
                {
                    fprintf(stderr, "Error: Frame number must be positive.\n");
                    SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
                }
                target_frame  = frame_number;
                i            += 2;
                continue;
            }

            fprintf(stderr, "Error: Unrecognized option '%s'.\n", argv[i]);
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            /* This is a file path. */
            if (file_count >= 256)
            {
                fprintf(stderr, "Error: Too many input files (maximum 256).\n");
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }
            files[file_count++] = argv[i];
            i++;
        }
    }

    /* Check that we have at least 2 files (1 input + 1 output). */
    if (file_count < 2)
    {
        fprintf(stderr, "Error: Need at least one input file and one output file.\n");
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    /* Last file is output, all others are inputs. */
    const char* output = files[file_count - 1];
    int input_count    = file_count - 1;

    /* Choose mode: extract frames or compose/convert. */
    if (extract_frames)
    {
        /* Extract frames mode: only one input file is allowed. */
        if (input_count != 1)
        {
            fprintf(stderr, "Error: Extract frames mode requires exactly one input file.\n");
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
        }

        /* Delay option is not applicable in extract mode. */
        if (delay >= 0)
        {
            fprintf(stderr, "Warning: --delay option is ignored in extract frames mode.\n");
        }

        SAIL_TRY(extract_frames_impl(files[0], output, pixel_format, compression, max_frames, target_frame, colors,
                                     dither, background, strip_metadata, flip_horizontal, flip_vertical, &auto_yes,
                                     &auto_no, suffix_digits));
    }
    else
    {
        SAIL_TRY(convert_impl(files, input_count, output, pixel_format, compression, max_frames, target_frame, delay,
                              colors, dither, background, strip_metadata, flip_horizontal, flip_vertical, &auto_yes,
                              &auto_no));
    }

    return SAIL_OK;
}

static bool special_properties_printf_callback(const char* key, const struct sail_variant* value)
{
    printf("    %s : ", key);
    sail_printf_variant(value);
    printf("\n");

    return true;
}

static void print_aligned_image_info(const struct sail_image* image)
{
    printf("  Size        : %ux%u\n", image->width, image->height);

    if (image->resolution == NULL)
    {
        printf("  Resolution  : -\n");
    }
    else
    {
        printf("  Resolution  : %.1fx%.1f\n", image->resolution->x, image->resolution->y);
    }

    printf("  Pixel format: %s\n", sail_pixel_format_to_string(image->source_image->pixel_format));
    printf("  Compression : %s\n", sail_compression_to_string(image->source_image->compression));
    printf("  ICC profile : %s\n", image->iccp == NULL ? "no" : "yes");
    if (image->gamma != 0)
    {
        printf("  Gamma       : %.6f\n", image->gamma);
    }
    else
    {
        printf("  Gamma       : -\n");
    }
    printf("  Interlaced  : %s\n", image->source_image->interlaced ? "yes" : "no");
    printf("  Delay       : %d ms.\n", image->delay);

    if (image->meta_data_node != NULL)
    {
        for (const struct sail_meta_data_node* meta_data_node = image->meta_data_node; meta_data_node != NULL;
             meta_data_node                                   = meta_data_node->next)
        {
            const struct sail_meta_data* meta_data = meta_data_node->meta_data;
            const char* meta_data_str              = NULL;

            if (meta_data->key == SAIL_META_DATA_UNKNOWN)
            {
                meta_data_str = meta_data->key_unknown;
            }
            else
            {
                meta_data_str = sail_meta_data_to_string(meta_data->key);
            }

            printf("  %-12s: ", meta_data_str);
            sail_printf_variant(meta_data->value);
            printf("\n");
        }
    }

    if (image->source_image->special_properties != NULL)
    {
        printf("  Special properties:\n");
        sail_traverse_hash_map(image->source_image->special_properties, special_properties_printf_callback);
    }
}

static sail_status_t probe_impl(const char* path)
{
    SAIL_CHECK_PTR(path);

    /* Time counter. */
    uint64_t start_time = sail_now();

    struct sail_image* image;
    const struct sail_codec_info* codec_info;

    SAIL_TRY(sail_probe_file(path, &image, &codec_info));

    uint64_t elapsed_time = sail_now() - start_time;

    printf("File          : %s\n", path);
    printf("Codec         : %s [%s]\n", codec_info->name, codec_info->description);
    printf("Codec version : %s\n", codec_info->version);
    printf("Probe time    : %lu ms.\n", (unsigned long)elapsed_time);

    print_aligned_image_info(image);

    sail_destroy_image(image);

    return SAIL_OK;
}

static sail_status_t probe(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(probe_impl(argv[2]));

    return SAIL_OK;
}

static sail_status_t decode_impl(const char* path)
{
    SAIL_CHECK_PTR(path);

    const struct sail_codec_info* codec_info;
    SAIL_TRY(sail_codec_info_from_path(path, &codec_info));

    printf("File          : %s\n", path);
    printf("Codec         : %s [%s]\n", codec_info->name, codec_info->description);
    printf("Codec version : %s\n", codec_info->version);

    /* Time counter. */
    uint64_t start_time = sail_now();

    /* Decode. */
    void* state;
    SAIL_TRY(sail_start_loading_from_file(path, codec_info, &state));

    struct sail_image* image;
    sail_status_t status;
    unsigned frame = 0;

    while ((status = sail_load_next_frame(state, &image)) == SAIL_OK)
    {
        printf("Frame #%u\n", frame++);
        print_aligned_image_info(image);
        sail_destroy_image(image);
    }

    if (status != SAIL_ERROR_NO_MORE_FRAMES)
    {
        sail_stop_loading(state);
        fprintf(stderr, "Error: Decoder error %d.\n", status);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_IMAGE);
    }

    SAIL_TRY(sail_stop_loading(state));

    uint64_t elapsed_time = sail_now() - start_time;

    printf("Decode time   : %lu ms.\n", (unsigned long)elapsed_time);

    return SAIL_OK;
}

static sail_status_t decode(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    SAIL_TRY(decode_impl(argv[2]));

    return SAIL_OK;
}

static sail_status_t list_impl(bool verbose)
{
    const struct sail_codec_bundle_node* codec_bundle_node = sail_codec_bundle_list();

    for (int counter = 1; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next, counter++)
    {
        const struct sail_codec_info* codec_info = codec_bundle_node->codec_bundle->codec_info;

        printf("%2d. [p%d] %s [%s] %s\n", counter, codec_info->priority, codec_info->name, codec_info->description,
               codec_info->version);

        if (verbose)
        {
            /* Load features tuning. */
            if (codec_info->load_features->tuning != NULL)
            {
                printf("         Load tuning: ");

                int tuning_counter = 0;
                for (const struct sail_string_node* node = codec_info->load_features->tuning; node != NULL;
                     node                                = node->next)
                {
                    if (tuning_counter > 0 && tuning_counter % 2 == 0)
                    {
                        printf(",\n                      ");
                    }
                    else if (tuning_counter > 0)
                    {
                        printf(", ");
                    }

                    printf("%s", node->string);
                    tuning_counter++;
                }

                printf("\n");
            }

            /* Save features. */
            const struct sail_save_features* save_features = codec_info->save_features;

            if (save_features->features != 0)
            {
                /* Output pixel formats. */
                if (save_features->pixel_formats_length > 0)
                {
                    printf("         Output formats: ");
                    for (unsigned i = 0; i < save_features->pixel_formats_length; i++)
                    {
                        if (i > 0 && i % 2 == 0)
                        {
                            printf(",\n                         ");
                        }
                        else if (i > 0)
                        {
                            printf(", ");
                        }
                        printf("%s", sail_pixel_format_to_string(save_features->pixel_formats[i]));
                    }
                    printf("\n");
                }

                /* Compressions. */
                if (save_features->compressions_length > 0)
                {
                    printf("         Compressions: ");
                    for (unsigned i = 0; i < save_features->compressions_length; i++)
                    {
                        if (i > 0 && i % 5 == 0)
                        {
                            printf(",\n                       ");
                        }
                        else if (i > 0)
                        {
                            printf(", ");
                        }
                        printf("%s", sail_compression_to_string(save_features->compressions[i]));
                    }
                    printf(" (default: %s)\n", sail_compression_to_string(save_features->default_compression));
                }

                /* Compression levels. */
                if (save_features->compression_level != NULL)
                {
                    printf("         Compression levels: min=%.0f, max=%.0f, default=%.0f, step=%.0f\n",
                           save_features->compression_level->min_level, save_features->compression_level->max_level,
                           save_features->compression_level->default_level, save_features->compression_level->step);
                }

                /* Save features tuning. */
                if (save_features->tuning != NULL)
                {
                    printf("         Save tuning: ");

                    int tuning_counter = 0;
                    for (const struct sail_string_node* node = save_features->tuning; node != NULL; node = node->next)
                    {
                        if (tuning_counter > 0 && tuning_counter % 2 == 0)
                        {
                            printf(",\n                      ");
                        }
                        else if (tuning_counter > 0)
                        {
                            printf(", ");
                        }

                        printf("%s", node->string);
                        tuning_counter++;
                    }

                    printf("\n");
                }
            }

            /* Add blank line after each codec in verbose mode. */
            printf("\n");
        }
    }

    return SAIL_OK;
}

static sail_status_t list(int argc, char* argv[])
{
    (void)argv;

    if (argc < 2 || argc > 3)
    {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    bool verbose;

    if (argc == 2)
    {
        verbose = false;
    }
    else
    {
        verbose = strcmp(argv[2], "-v") == 0;
    }

    SAIL_TRY(list_impl(verbose));

    return SAIL_OK;
}

static void help(const char* app)
{
    fprintf(stderr, "SAIL command-line utility for image conversion.\n\n");
    fprintf(stderr, "Usage: %s <command> [arguments]\n\n", app);

    fprintf(stderr, "Commands:\n\n");

    fprintf(stderr, "  list [-v]    List all supported image codecs with details\n\n");

    fprintf(stderr, "  convert - Convert, compose, and extract image files\n");
    fprintf(stderr, "      Options:\n");
    fprintf(stderr, "        -p, --pixel-format <format>  Force specific output pixel format\n");
    fprintf(stderr, "        -c, --compression <level>    Set compression quality level (codec-specific)\n");
    fprintf(stderr, "        -m, --max-frames <count>     Limit number of frames to process\n");
    fprintf(stderr, "        -d, --delay <ms>             Set frame delay for animations in milliseconds\n");
    fprintf(stderr, "        -e, --extract-frames         Extract each frame to separate file\n");
    fprintf(stderr, "        -z, --suffix-digits <N>      Set number of digits in frame suffix (1-10, e.g., 3 for 001, 002, ...)\n");
    fprintf(stderr, "        -C, --colors <N>             Quantize image to N colors (2-256) using Wu algorithm\n");
    fprintf(stderr, "        -D, --dither                 Apply Floyd-Steinberg dithering for better gradients\n");
    fprintf(stderr,
            "        -b, --background <color>     Blend alpha channel with background (white, black, #RRGGBB)\n");
    fprintf(stderr, "        -s, --strip                  Remove all metadata from output files\n");
    fprintf(stderr, "        -H, --flip-horizontal        Flip image horizontally (mirror left-right)\n");
    fprintf(stderr, "        -V, --flip-vertical          Flip image vertically (mirror top-bottom)\n");
    fprintf(stderr, "        -n, --frame-number <N>       Extract specific frame number N (1-based)\n\n");
    fprintf(stderr, "      Use cases:\n");
    fprintf(stderr, "        Simple format conversion between codecs:\n");
    fprintf(stderr, "          %s convert input.jpg output.png\n\n", app);
    fprintf(stderr, "        Convert with custom quality and pixel format:\n");
    fprintf(stderr, "          %s convert input.png output.jpg -c 90 -p BPP24-RGB\n\n", app);
    fprintf(stderr, "        Convert animation with specified frame delay:\n");
    fprintf(stderr, "          %s convert animation.gif output.webp -d 100\n\n", app);
    fprintf(stderr, "        Convert animation to multi-page document format:\n");
    fprintf(stderr, "          %s convert animation.gif output.tiff\n\n", app);
    fprintf(stderr, "        Compose multiple images into single animation:\n");
    fprintf(stderr, "          %s convert frame1.png frame2.png frame3.png animation.gif -d 100\n\n", app);
        fprintf(stderr, "        Extract all frames from animation into (frame-1.jpg, frame-2.jpg, ...):\n");
        fprintf(stderr, "          %s convert animation.gif frame.jpg -e\n\n", app);
        fprintf(stderr, "        Extract frames with 3-digit suffix (frame-001.jpg, frame-002.jpg, ...):\n");
        fprintf(stderr, "          %s convert animation.gif frame.jpg -e -z 3\n\n", app);
    fprintf(stderr, "        Extract first 5 frames from animation:\n");
    fprintf(stderr, "          %s convert animation.webp frame.png -e -m 5\n\n", app);
    fprintf(stderr, "        Reduce colors to 16 with dithering for smaller file size:\n");
    fprintf(stderr, "          %s convert photo.jpg output.gif --colors 16 --dither\n\n", app);
    fprintf(stderr, "        Convert RGBA to RGB with white background blend:\n");
    fprintf(stderr, "          %s convert transparent.png opaque.jpg --background white\n\n", app);
    fprintf(stderr, "        Strip metadata for privacy and smaller size:\n");
    fprintf(stderr, "          %s convert photo.jpg clean.jpg --strip\n\n", app);
    fprintf(stderr, "        Flip image horizontally or vertically:\n");
    fprintf(stderr, "          %s convert photo.jpg flipped.jpg -H -V\n\n", app);
    fprintf(stderr, "        Extract frame #2 from animation\n");
    fprintf(stderr, "          %s convert animation.gif frame2.png -n 2\n\n", app);

    fprintf(stderr, "  probe <path>     Display detailed information about image file\n");
    fprintf(stderr, "  decode <path>    Decode file and show information for all frames\n\n");

    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  %s -h, --help                Display this help message and exit\n", app);
    fprintf(stderr, "  %s -v, --version             Display version information and exit\n", app);
    fprintf(stderr, "  %s -l, --log-level <level>   Set log level: silence, error, warning (default),\n", app);
    fprintf(stderr, "                                              info, message, debug, trace\n");
    fprintf(stderr, "  %s -y, --yes                 Automatically overwrite existing files without prompting\n", app);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        help(argv[0]);
        return 1;
    }

    /* Default log level. */
    enum SailLogLevel log_level = SAIL_LOG_LEVEL_WARNING;
    int arg_offset              = 1;

    /* Check for global options. */
    while (arg_offset < argc && argv[arg_offset][0] == '-')
    {
        if (strcmp(argv[arg_offset], "-h") == 0 || strcmp(argv[arg_offset], "--help") == 0)
        {
            help(argv[0]);
            return 0;
        }

        if (strcmp(argv[arg_offset], "-v") == 0 || strcmp(argv[arg_offset], "--version") == 0)
        {
            fprintf(stderr, "SAIL command-line utility 1.5.0\n");
            fprintf(stderr, "SAIL library %s\n", SAIL_VERSION_STRING);
            return 0;
        }

        if (strcmp(argv[arg_offset], "-l") == 0 || strcmp(argv[arg_offset], "--log-level") == 0)
        {
            if (arg_offset == argc - 1)
            {
                fprintf(stderr, "Error: Missing log level value.\n");
                return 1;
            }
            enum SailLogLevel parsed_level = sail_log_level_from_string(argv[arg_offset + 1]);
            if (parsed_level == SAIL_LOG_LEVEL_DEBUG && strcmp(argv[arg_offset + 1], "debug") != 0)
            {
                fprintf(stderr, "Error: Unknown log level '%s'\n", argv[arg_offset + 1]);
                return 1;
            }
            log_level   = parsed_level;
            arg_offset += 2;
            continue;
        }

        /* Not a global option, must be a command. */
        break;
    }

    if (arg_offset >= argc)
    {
        help(argv[0]);
        return 1;
    }

    sail_set_log_barrier(log_level);

    /* Adjust argv to skip processed global options. */
    argv += arg_offset - 1;
    argc -= arg_offset - 1;

    if (strcmp(argv[1], "convert") == 0)
    {
        SAIL_TRY(convert(argc, argv));
    }
    else if (strcmp(argv[1], "list") == 0)
    {
        SAIL_TRY(list(argc, argv));
    }
    else if (strcmp(argv[1], "probe") == 0)
    {
        SAIL_TRY(probe(argc, argv));
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        SAIL_TRY(decode(argc, argv));
    }
    else
    {
        print_invalid_argument();
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }

    sail_finish();

    return 0;
}
