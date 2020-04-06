/*
 * This is a plugin layout definition file (version 2) created just for
 * informational purposes. It's not used in SAIL and not to be included
 * anywhere. Use this file just for reference how plugins version 2
 * are organized.
 */

/*
 * Generate a syntax error.
 */
NOT TO BE INCLUDED

/* V2 inherits all the functions from V1. */

/*
 * Decoding functions.
 */

/*
 * Reads a scan line of the current image in the current pass. Allocates a new scan line. The assigned scan line
 * MUST be freed later with free().
 *
 * Returns 0 on success or sail_error_t on error.
 */
int sail_plugin_read_scan_line_v1(struct sail_file *file, struct sail_image *image, void **scanline);

/*
 * Encoding functions.
 */
