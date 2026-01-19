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
 * Technical Diver API Demo
 *
 * This demonstrates the Technical Diver API level with custom I/O sources.
 *
 * Differences from other API levels:
 * - Junior: Simple one-line functions, only files
 * - Advanced: Frame-by-frame loading, but only files and memory buffers
 * - Deep diver: Full control over codec options, but only files and memory buffers
 * - Technical diver: Everything above plus custom I/O sources (files, memory, network,
 *                    encrypted streams, databases, etc.)
 *
 * Perfect for: Custom I/O sources like network streams, encrypted files, databases,
 *              or in-memory processing pipelines. This is the most flexible API level
 *              that lets you read images from anywhere you can implement I/O callbacks.
 *
 * For simple file operations, the Junior, Advanced, or Deep Diver APIs are usually
 * easier to use. This level is for when you need something special.
 *
 * Supported file formats: All formats supported by SAIL codecs
 *
 * This example demonstrates loading from a custom I/O source (simulating sail_io_file
 * with direct file operations).
 */

#include <stdio.h>

#ifdef _MSC_VER
#include <sys/stat.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <sail/sail.h>

/* Custom I/O context that simulates sail_io_file. */
typedef struct
{
    FILE* fptr;
} CustomIOContext;

/* Tolerant read callback for custom I/O. */
static sail_status_t custom_tolerant_read(void* stream, void* buf, size_t size_to_read, size_t* read_size)
{
    CustomIOContext* ctx = stream;

    *read_size = fread(buf, 1, size_to_read, ctx->fptr);

    return SAIL_OK;
}

/* Strict read callback for custom I/O. */
static sail_status_t custom_strict_read(void* stream, void* buf, size_t size_to_read)
{
    CustomIOContext* ctx = stream;
    size_t read_size;

    read_size = fread(buf, 1, size_to_read, ctx->fptr);

    if (read_size != size_to_read)
    {
        return SAIL_ERROR_READ_IO;
    }

    return SAIL_OK;
}

/* Seek callback for custom I/O. */
static sail_status_t custom_seek(void* stream, long offset, int whence)
{
    CustomIOContext* ctx = stream;

    if (fseek(ctx->fptr, offset, whence) != 0)
    {
        return SAIL_ERROR_SEEK_IO;
    }

    return SAIL_OK;
}

/* Tell callback for custom I/O. */
static sail_status_t custom_tell(void* stream, size_t* offset)
{
    CustomIOContext* ctx = stream;
    long offset_local;

    offset_local = ftell(ctx->fptr);

    if (offset_local < 0)
    {
        return SAIL_ERROR_TELL_IO;
    }

    *offset = (size_t)offset_local;

    return SAIL_OK;
}

/* EOF callback for custom I/O. */
static sail_status_t custom_eof(void* stream, bool* result)
{
    CustomIOContext* ctx = stream;

    *result = (feof(ctx->fptr) != 0);

    return SAIL_OK;
}

/* Size callback for custom I/O. */
static sail_status_t custom_size(void* stream, size_t* size)
{
    CustomIOContext* ctx = stream;

    /* Try to get size efficiently using stat. */
#ifdef _MSC_VER
    int fd = _fileno(ctx->fptr);
    if (fd >= 0)
    {
        struct _stat attrs;
        if (_fstat(fd, &attrs) == 0)
        {
            *size = attrs.st_size;
            return SAIL_OK;
        }
    }
#else
    int fd = fileno(ctx->fptr);
    if (fd >= 0)
    {
        struct stat attrs;
        if (fstat(fd, &attrs) == 0)
        {
            *size = attrs.st_size;
            return SAIL_OK;
        }
    }
#endif

    /* Fallback to seek/tell method. */
    size_t saved_position;
    sail_status_t status = custom_tell(stream, &saved_position);
    if (status != SAIL_OK)
    {
        return status;
    }

    size_t size_local;
    status = custom_seek(stream, 0, SEEK_END);
    if (status != SAIL_OK)
    {
        return status;
    }

    status = custom_tell(stream, &size_local);
    if (status != SAIL_OK)
    {
        return status;
    }

    status = custom_seek(stream, (long)saved_position, SEEK_SET);
    if (status != SAIL_OK)
    {
        return status;
    }

    *size = size_local;

    return SAIL_OK;
}

/* Close callback for custom I/O. */
static sail_status_t custom_close(void* stream)
{
    CustomIOContext* ctx = stream;

    if (ctx->fptr != NULL)
    {
        fclose(ctx->fptr);
        ctx->fptr = NULL;
    }

    return SAIL_OK;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        fprintf(stderr, "Example: %s input.jpg\n", argv[0]);
        fprintf(stderr, "This demonstrates loading from a custom I/O source (simulating sail_io_file).\n");
        return 1;
    }

    const char* input_path = argv[1];

    /* Open file for reading. */
    FILE* file = fopen(input_path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Failed to open file '%s'\n", input_path);
        return 1;
    }

    /* Create custom I/O context. */
    CustomIOContext io_ctx = {
        .fptr = file,
    };

    /* Allocate I/O structure. */
    struct sail_io* io;
    SAIL_TRY_OR_EXECUTE(sail_alloc_io(&io),
                        /* on error */ fclose(file);
                                      return 1);

    /* Set I/O features and callbacks. */
    io->features       = SAIL_IO_FEATURE_SEEKABLE;
    io->stream         = &io_ctx;
    io->tolerant_read  = custom_tolerant_read;
    io->strict_read    = custom_strict_read;
    io->tolerant_write = sail_io_not_implemented_tolerant_write;
    io->strict_write   = sail_io_not_implemented_strict_write;
    io->seek           = custom_seek;
    io->tell           = custom_tell;
    io->flush          = sail_io_noop_flush;
    io->close          = custom_close;
    io->eof            = custom_eof;
    io->size           = custom_size;

    /* Probe I/O to detect codec and get image properties. */
    struct sail_image* probed_image;
    const struct sail_codec_info* codec_info;
    SAIL_TRY_OR_EXECUTE(sail_probe_io(io, &probed_image, &codec_info),
                        /* on error */ sail_destroy_io(io);
                                      return 1);

    printf("Detected codec: %s [%s]\n", codec_info->name, codec_info->description);
    sail_destroy_image(probed_image);

    /* Reset I/O position to beginning. */
    io->seek(io->stream, 0, SEEK_SET);

    /* Start loading from custom I/O. */
    void* load_state = NULL;
    SAIL_TRY_OR_EXECUTE(sail_start_loading_from_io(io, codec_info, &load_state),
                        /* on error */ sail_destroy_io(io);
                                      return 1);

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
        sail_destroy_io(io);
        return 1;
    }

    /* Stop loading. */
    SAIL_TRY_OR_EXECUTE(sail_stop_loading(load_state),
                        /* on error */ sail_destroy_io(io);
                                      return 1);

    printf("\nTotal frames loaded: %u\n", frame_number);

    /* Cleanup. */
    sail_destroy_io(io);

    return 0;
}
