#ifndef SAIL_SAIL_H
#define SAIL_SAIL_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    /* Universal include. */

    /* libsail-common. */
    #include <sail/common.h>
    #include <sail/error.h>
    #include <sail/export.h>
    #include <sail/meta_entry_node.h>
    #include <sail/log.h>
    #include <sail/utils.h>

    /* libsail. */
    #include <sail/plugin_info.h>
    #include <sail/plugin.h>
    #include <sail/string_node.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_plugin_info_node;
struct sail_plugin_info;

struct sail_context {

    struct sail_plugin_info_node *plugin_info_node;
};

typedef struct sail_context sail_context_t;

/*
 * Initializes SAIL. This is the main entry point to start working with SAIL.
 * Builds a list of available SAIL plugins.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_init(struct sail_context **context);

/*
 * Finilizes working with the specified SAIL context. Frees the context and all its internal
 * memory buffers. The context MUST NOT be used anymore after calling this function.
 */
SAIL_EXPORT void sail_finish(struct sail_context *context);

/*
 * Finds a first plugin info object that supports the specified file extension. For example: "jpg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_extension(struct sail_context *context, const char *extension, const struct sail_plugin_info **plugin_info);

/*
 * Finds a first plugin info object that supports the specified mime type. For example: "image/jpeg".
 * The assigned plugin info MUST NOT be destroyed. It's a pointer to an internal data structure.
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_plugin_info_by_mime_type(struct sail_context *context, const char *mime_type, const struct sail_plugin_info **plugin_info);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
