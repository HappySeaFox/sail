/*
 * File functions.
 */

/*
 * Returns an OS-specific file descriptor.
 */
int sail_file_fd(sail_file *file);

/*
 * Image functions.
 */

/*
 * Returns the image width.
 */
int sail_image_width(sail_image *image);

/*
 * Returns the image height.
 */
int sail_image_height(sail_image *image);

/*
 * Returns the image pixel format. See SailPixelFormat.
 */
int sail_image_pixel_format(sail_image *image);

/*
 * Returns the number of passes necessary to read the image.
 */
int sail_image_passes(sail_image *image);

/*
 * Returns true if the image is an animation.
 */
bool sail_image_animated(sail_image *image);

/*
 * Returns the image delay in milliseconds if the file is an animation.
 */
int sail_image_delay(sail_image *image);

/*
 * Returns the image palette pixel format.
 */
int sail_image_palette_pixel_format(sail_image *image);

/*
 * Returns the image palette if any.
 */
void* sail_image_palette(sail_image *image);

/*
 * Returns the image palette size in bytes.
 */
int sail_image_palette_size(sail_image *image);

#endif
