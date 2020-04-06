#ifndef SAIL_PLUGIN_H
#define SAIL_PLUGIN_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A structure representing an image.
 */
struct sail_plugin_info {

    /*
     * The plugin loader will use the plugin's layout version to correctly handle the plugin. Unsupported
     * plugin layout versions will be reported. This field must be the very first key in a plugin information
     * file.
     *
     * Plugin layout is a list of exported functions. We use plugin layout versions to implement
     * backward compatibility in a simple and maintanable way.
     */
    int layout;

    /* Plugin version. For example: "1.5.2". */
    char *version;

    /* Plugin description. For example: "JPEG image". */
    char *description;

    /* Semicolon-separated list of supported file extensions. For example: "jpg;jpeg". */
    char *extensions;

    /* Semicolon-separated list of supported file mime types. For example: "image/jpeg". */
    char *mime_types;

    /*
     * Magic identifier as a regex string. For example: "GIF8[79]a".
     *
     * See https://en.wikipedia.org/wiki/List_of_file_signatures for more.
     */
    char *magic;
};

typedef struct sail_plugin_info sail_plugin_info_t;

/*
 * Plugin info functions.
 */

/*
 * Allocates a new plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
sail_error_t SAIL_EXPORT sail_alloc_plugin_info(struct sail_plugin_info **plugin_info);

/*
 * Destroys the specified plugin_info and all its internal allocated memory buffers.
 * The "plugin_info" pointer MUST NOT be used after calling this function.
 */
void SAIL_EXPORT sail_destroy_plugin_info(struct sail_plugin_info *plugin_info);

/*
 * Reads SAIL plugin info from the specified file and stores the parsed information into
 * the specified plugin info object. The assigned plugin info MUST be destroyed later
 * with sail_destroy_plugin_info().
 *
 * Returns 0 on success or sail_error_t on error.
 */
sail_error_t SAIL_EXPORT sail_plugin_read_info(const char *file, struct sail_plugin_info **plugin_info);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
