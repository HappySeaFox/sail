/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_ICCP_H
#define SAIL_ICCP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sail_meta_entry_node;

/*
 * A structure representing an ICC profile.
 */
struct sail_iccp {

    /* Optional ICC profile name. Can be NULL. */
    char *name;

    /* ICC profile binary data. */
    void *data;

    /* The length of the data. */
    unsigned data_length;
};

typedef struct sail_iccp sail_iccp_t;

/*
 * Allocates a new ICC profile. The assigned profile MUST be destroyed later with sail_destroy_iccp().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_alloc_iccp(struct sail_iccp **iccp);

/*
 * Destroys the specified ICC profile and all its internal allocated memory buffers.
 * Does nothing if the profile is NULL.
 */
SAIL_EXPORT void sail_destroy_iccp(struct sail_iccp *iccp);

/*
 * Makes a deep copy of the specified ICC profile. The assigned profile MUST be destroyed later
 * with sail_destroy_iccp().
 *
 * Returns 0 on success or sail_error_t on error.
 */
SAIL_EXPORT sail_error_t sail_copy_iccp(const struct sail_iccp *source_iccp, struct sail_iccp **target_iccp);

/* extern "C" */
#ifdef __cplusplus
}
#endif

#endif
