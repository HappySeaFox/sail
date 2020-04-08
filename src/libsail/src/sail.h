#ifndef SAIL_SAIL_H
#define SAIL_SAIL_H

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

struct sail_context {

    int nop;
};

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

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
