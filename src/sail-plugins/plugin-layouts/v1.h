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
 * Plugin-related functions.
 */

/*
 * The loader will use the plugin's layout version to correctly handle the plugin. Unsupported
 * plugin layout versions will be reported.
 *
 * Plugin layout is a list of exported functions. We use plugin layout versions to implement
 * backward compatibility in a simple and maintanable way.
 *
 * This is the only function that is expected to be exported by every plugin. Other exported
 * functions depend on the returned version number.
 */
int sail_plugin_layout_version(void);

/*
 * Returns a plugin version string. For example: "1.3.4".
 */
const char* sail_plugin_version(void);

/*
 * Returns a plugin description string. For example: "Windows Bitmap".
 */
const char* sail_plugin_description(void);

/*
 * Returns a semicolon-separated list of supported file extensions. For example: "bmp" or "bmp;dib".
 */
const char* sail_plugin_extensions(void);

/*
 * Returns a semicolon-separated list of supported file mime types. For example: "image/x-bmp"
 * or "image/x-bmp;image/x-dib".
 */
const char* sail_plugin_mime_types(void);

/*
 * Returns a file magic identifier as a regex string. For example: "GIF8[79]a".
 *
 * See https://en.wikipedia.org/wiki/List_of_file_signatures for more.
 */
const char* sail_plugin_magic(void);

/*
 * Returns combined plugin features. Use the returned value to determine
 * what a plugin can actually do. E.g. decode, encode, read meta information etc.
 *
 * See SailPluginFeatures.
 */
int sail_plugin_features(void);

/*
 * Decoding functions.
 */

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
