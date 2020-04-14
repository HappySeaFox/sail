#include <stdio.h>
#include <stdint.h>

#undef SAIL_BUILD
#include <sail/sail.h>

#ifdef SAIL_WIN32
#else
    #include <errno.h>
    #include <sys/time.h>
    #include <string.h>
#endif

static uint64_t now() {

#ifdef SAIL_WIN32
#else
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        fprintf(stderr, "Failed to get the current time: %s\n", strerror(errno));
        return 0;
    }

    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

static sail_error_t probe(const char *path) {

    struct sail_context *context;

    /* Time counter. */
    const uint64_t start_time = now();

    SAIL_TRY(sail_init(&context));

    struct sail_image *image;
    const struct sail_plugin_info *plugin_info;

    SAIL_TRY(sail_probe_image(path, context, &plugin_info, &image));

    printf("File          : %s\n", path);
    printf("Probed time   : %ld ms.\n", (unsigned long)(now() - start_time));
    printf("Codec         : %s\n", plugin_info->description);
    printf("Codec version : %s\n", plugin_info->version);

    printf("\n");

    printf("Size          : %dx%d\n", image->width, image->height);
    printf("Source color  : %s\n", sail_pixel_format_to_string(image->source_pixel_format));
    printf("Output color  : %s\n", sail_pixel_format_to_string(image->pixel_format));

    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != NULL) {
        printf("%-14s: %s\n", node->key, node->value);
    }

    sail_destroy_image(image);

    sail_finish(context);

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <PATH TO IMAGE>\n", argv[0]);
        return 1;
    }

    SAIL_TRY(probe(argv[1]));

    return 0;
}
