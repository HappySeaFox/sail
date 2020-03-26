#ifndef SAIL_PLUGIN_H
#define SAIL_PLUGIN_H

#include <limits.h>

/*
 * Features to determine what a plugin can actually do. E.g. decode, encode etc.
 *
 * Use sail_plugin_features() to get plugin features.
 */
enum SailPluginFeatures {
    SAIL_PLUGIN_FEATURE_READ_STATIC     = 1 << 0,
    SAIL_PLUGIN_FEATURE_READ_ANIMATED   = 1 << 1,
    SAIL_PLUGIN_FEATURE_READ_MULTIPAGED = 1 << 2,
    SAIL_PLUGIN_FEATURE_READ_METAINFO   = 1 << 3,

    SAIL_PLUGIN_FEATURE_WRITE_STATIC     = 1 << 4,
    SAIL_PLUGIN_FEATURE_WRITE_ANIMATED   = 1 << 5,
    SAIL_PLUGIN_FEATURE_WRITE_MULTIPAGED = 1 << 6,
    SAIL_PLUGIN_FEATURE_WRITE_METAINFO   = 1 << 7,

    /* Not to be used. Resize the enum for future elements. */
    SAIL_PLUGIN_FEATURE_RESIZE_ENUM_TO_INT = INT_MAX
};

/*
 * Common functions exported by ANY plugin layout version:
 * -------------------------------------------------------
 *
 * int sail_plugin_layout_version();
 *
 * SAIL plugin layout versions:
 * ----------------------------
 *
 * +---------+---------------------+
 * | Version | Reference file      |
 * +---------+---------------------+
 * |    1    | plugin-layouts/v1.h |
 * +---------+---------------------+
 */

#endif
