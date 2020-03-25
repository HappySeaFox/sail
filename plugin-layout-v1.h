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
 * what a plugin can actually do. E.g. decode, encode etc.
 *
 * See SailPluginFeatures.
 */
int sail_plugin_features(void);

/*
 * Starts decoding the specified image file. The assigned file MUST be closed later with sail_file_close().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_open(const char *filepath, int flags, sail_file **file);

/*
 * Seeks to the next frame. The frame is NOT immediately read. Use sail_plugin_read_next_pass() +
 * sail_plugin_read_scanline() to actually read the frame. The assigned image MUST be closed later
 * with sail_image_destroy().
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_next_frame(sail_file *file, sail_image **image);

/*
 * Seeks to the next pass if the specified image is interlaced. Does nothing otherwise.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_next_pass(sail_file *file, sail_image *image);

/*
 * Read a scan line of the current image in the current pass.
 *
 * Returns 0 on success or errno on error.
 */
int sail_plugin_read_scanline(sail_file *file, sail_image *image, void *scanline);
