/*
 * This is a plugin layout definition file (version 1) created just for
 * informational purposes. It's not used in SAIL and not to be included
 * anywhere. Use this file just for reference how plugins version 1
 * are organized.
 */

/*
 * Generate a syntax error.
 */
NOT TO BE INCLUDED

/*
 * Decoding functions.
 */

/*
 * Assigns possible read features for this plugin. The assigned read features MUST be destroyed later
 * with sail_read_features_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_features(struct sail_read_features **read_features);

/*
 * Initializes decoding the specified file using the specified options (or NULL to use defaults).
 * The specified read options will be copied into an internal buffer. For a list of default options
 * please see sail_read_options_alloc().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_init(struct sail_file *file, struct sail_read_options *read_options);

/*
 * Seeks to the next frame. The frame is NOT immediately read or parsed by most plugins. One could
 * use this method to quickly detect the image dimensions without parsing the whole file.
 *
 * Use sail_plugin_read_seek_next_pass() + sail_plugin_read_scan_line() to actually read the frame.
 * The assigned image MUST be destroyed later with sail_image_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_seek_next_frame(struct sail_file *file, struct sail_image **image);

/*
 * Seeks to the next pass if the specified image has multiple passes. Does nothing otherwise.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_seek_next_pass(struct sail_file *file, struct sail_image *image);

/*
 * Reads a scan line of the current image in the current pass. The specified scan line must be
 * allocated by the caller and must be be large enough.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_scan_line(struct sail_file *file, struct sail_image *image, unsigned char *scanline);

/*
 * Reads a scan line of the current image in the current pass. The assigned scan line MUST be freed
 * later with free().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_alloc_scan_line(struct sail_file *file, struct sail_image *image, unsigned char **scanline);

/*
 * Finilizes reading operation. No more readings are possible after calling this function.
 * This function doesn't close the file. It just stops decoding. Use sail_file_close()
 * to actually close the file.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_finish(struct sail_file *file, struct sail_image *image);

/*
 * Encoding functions.
 */

/*
 * Assigns possible write features for this plugin. The assigned write features MUST be destroyed later
 * with sail_write_features_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_features(struct sail_write_features **write_features);

/*
 * Initializes encoding the file using the specified options (or NULL to use defaults).
 * The specified write options will be copied into an internal buffer. For a list of default options
 * please see sail_write_options_alloc().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_init(struct sail_file *file, struct sail_write_options *write_options);

/*
 * Seeks to a next frame before writing it. The frame is NOT immediately written. Use sail_plugin_write_seek_next_pass()
 * and sail_plugin_write_scan_line() to actually write a frame. The assigned image MUST be destroyed later
 * with sail_image_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_seek_next_frame(struct sail_file *file, struct sail_image **image);

/*
 * Seeks to a next pass before writing it if the specified image is interlaced. Does nothing otherwise.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_seek_next_pass(struct sail_file *file, struct sail_image *image);

/*
 * Writes a scan line of the current image in the current pass.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_scan_line(struct sail_file *file, struct sail_image *image, void *scanline);

/*
 * Finilizes writing operation. No more writings are possible after calling this function.
 * This function doesn't close the file. Use sail_file_close() for that.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_write_finish(struct sail_file *file, struct sail_image *image);
