Table of Contents
=================

* [APIs overview](#apis-overview)
  * [1\. junior](#1-junior)
    * [C:](#c)
    * [C\+\+:](#c-1)
  * [2\. advanced](#2-advanced)
    * [C:](#c-2)
    * [C\+\+:](#c-3)
  * [3\. deep diver](#3-deep-diver)
    * [C:](#c-4)
    * [C\+\+:](#c-5)

## APIs overview

SAIL provides 3 levels of APIs depending on your needs. Let's have a look at them.

### 1. `junior`

**Purpose:** read a single image frame in a one-line manner.

#### C:
```C
struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in RGB pixel format for image formats
 * without transparency support and RGBA otherwise.
 */
SAIL_TRY(sail_read(path,
                   &image,
                   reinterpret_cast<void **>(&image_bits)));

/*
 * Handle the image bits here.
 * Use image->width, image->height, image->bytes_per_line,
 * and image->pixel_format for that.
 */

free(image_bits);
sail_destroy_image(image);
```

#### C++:
```C++
sail::image_reader reader;
sail::image image;

// read() reads the image and outputs pixels in RGB pixel format for image formats
// without transparency support and RGBA otherwise.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image->width(), image->height(), image->bytes_per_line(),
// image->pixel_format(), and image->bits() for that.
```

### 2. `advanced`

**Purpose:** read a single or multiple image frames.

#### C:
```C
struct sail_context *context = NULL;

/*
 * Initialize SAIL context. You could cache the context and re-use it multiple times.
 * When it's not needed anymore, call sail_finish(context).
 */
SAIL_TRY(sail_init(&context));

/*
 * A local void pointer is needed for SAIL to save a state in it.
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;
struct sail_image *image = NULL;
unsigned char *image_bits = NULL;

/*
 * Starts reading the specified file.
 * The subsequent calls to sail_read_next_frame() will output pixels in RGB pixel format for image formats
 * without transparency support and RGBA otherwise.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path, context, NULL, &state),
                    /* cleanup */ sail_stop_reading(state),
                                  free(image_bits),
                                  sail_destroy_image(image));

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns 0. If no more frames are available,
 * it returns SAIL_NO_MORE_FRAMES.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image, (void **)&image_bits),
                    /* cleanup */ sail_stop_reading(state),
                                  free(image_bits),
                                  sail_destroy_image(image));

/*
 * It's essential to ALWAYS stop reading to free memory resources.
 * Avoiding doing so will lead to memory leaks.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
         /* cleanup */ free(image_bits),
                       sail_destroy_image(image));

/*
 * Handle the image bits here.
 * Use image->width, image->height, image->bytes_per_line,
 * and image->pixel_format for that.
 */

free(image_bits);
sail_destroy_image(image);

/*
 * If you have no plans to re-use the same context in the future, finish working with this context
 * and unload all loaded plugins attached to it.
 */
sail_finish(context);
context = NULL;
```

#### C++:
```C++
// Initialize SAIL context. You could cache the context as a class member and re-use it multiple times.
//
sail::context context;

sail::image_reader reader(&context);
sail::image image;

// It's essential to ALWAYS stop reading to free memory resources.
// Avoiding doing so will lead to memory leaks. This code gets executed
// when the outer scope exits.
//
SAIL_AT_SCOPE_EXIT (
    reader.stop_reading();
);

// Starts reading the specified file.
// The subsequent calls to read_next_frame() will output pixels in RGB pixel format for image formats
// without transparency support and RGBA otherwise.
//
SAIL_TRY(reader.start_reading(path));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in RGB pixel format for image formats
// without transparency support and RGBA otherwise.
//
SAIL_TRY(reader.read_next_frame(&image));

SAIL_TRY(reader.stop_reading());

// Handle the image and its bits here.
// Use image->width(), image->height(), image->bytes_per_line(),
// image->pixel_format(), and image->bits() for that.
```

### 3. `deep diver`

**Purpose:** read a single or multiple image frames. Possibly specify a concrete plugin to use
             (e.g. to load an image file with no extension). Possibly specify a desired pixel format to output.

#### C:
```C
struct sail_context *context = NULL;

/*
 * Initialize SAIL context. You could cache the context and re-use it multiple times.
 * When it's not needed anymore, call sail_finish(context).
 */
SAIL_TRY(sail_init(&context));

struct sail_read_options *read_options = NULL;
struct sail_image *image = NULL;
unsigned char *image_bits = NULL;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec info by a file extension.
 */
const struct sail_plugin_info *plugin_info;
SAIL_TRY(sail_plugin_info_from_path(path, context, &plugin_info));

/*
 * Allocate new read options and copy defaults from the plugin-specific read features
 * (preferred output pixel format etc.).
 */
SAIL_TRY(sail_alloc_read_options_from_features(plugin_info->read_features, &read_options));

/*
 * Let's request RGB pixels only.
 */
if (read_options->output_pixel_format != SAIL_PIXEL_FORMAT_RGB) {
    bool foundRgb = false;

    /*
     * Check if the plugin supports outputting RGB pixels.
     */
    for (int i = 0; i < plugin_info->read_features->output_pixel_formats_length; i++) {
        if (plugin_info->read_features->output_pixel_formats[i] == SAIL_PIXEL_FORMAT_RGB) {
            foundRgb = true;
            break;
        }
    }

    /*
     * The plugin doesn't support outputting RGB pixels.
     */
    if (!foundRgb) {
        sail_destroy_read_options(read_options);
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    /*
     * Request the plugin to output RGB pixels.
     */
    read_options->output_pixel_format = SAIL_PIXEL_FORMAT_RGB;
}

/*
 * Initialize reading with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_file_with_options(path,
                                                         context,
                                                         plugin_info,
                                                         read_options,
                                                         &state),
                    /* cleanup */ sail_destroy_read_options(read_options));

/*
 * Our read options are not needed anymore.
 */
sail_destroy_read_options(read_options);

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns 0. If no more frames are available,
 * it returns SAIL_NO_MORE_FRAMES.
 *
 * sail_read_next_frame() outputs pixels in the requested pixel format (RGB).
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state,
                                         &image,
                                         (void **)&image_bits),
                    /* cleanup */ sail_destroy_image(image));

/*
 * Finish reading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                    /* cleanup */ sail_destroy_image(image));

/*
 * Print the image meta information if any (JPEG comments etc.).
 */
struct sail_meta_entry_node *node = image->meta_entry_node;

if (node != NULL) {
    SAIL_LOG_DEBUG("%s: %s", node->key, node->value);
}

/*
 * Handle the image RGB bits here.
 * Use image->width, image->height, image->bytes_per_line,
 * and image->pixel_format for that.
 */

free(image_bits);
sail_destroy_image(image);

/*
 * Optional: unload all plugins to free up some memory if you plan to re-use the same context
 * in the future.
 */
sail_unload_plugins(context);

/*
 * If you have no plans to re-use the same context in the future, finish working with this context
 * and unload all loaded plugins attached to it.
 */
sail_finish(context);
context = NULL;
```

#### C++:
```C++
// Initialize SAIL context. You could cache the context as a class member and re-use it multiple times.
//
sail::context context;
sail::image_reader reader(&context);

// Find the codec info by a file extension.
//
sail::plugin_info plugin_info;
SAIL_TRY(context.plugin_info_from_path(path, &plugin_info));

// Allocate new read options and copy defaults from the read features
// (preferred output pixel format etc.).
//
sail::read_options read_options;
SAIL_TRY(plugin_info.read_features().to_read_options(&read_options));

// Let's request RGB pixels only.
//
if (read_options.output_pixel_format() != SAIL_PIXEL_FORMAT_RGB) {
    const std::vector<int> output_pixel_formats = plugin_info.read_features().output_pixel_formats();

    // The plugin doesn't support outputting RGB pixels.
    //
    if (std::find(output_pixel_formats.begin(), output_pixel_formats.end(), SAIL_PIXEL_FORMAT_RGB) == output_pixel_formats.end()) {
        return SAIL_UNSUPPORTED_PIXEL_FORMAT;
    }

    // Request the plugin to output RGB pixels.
    //
    read_options.with_output_pixel_format(SAIL_PIXEL_FORMAT_RGB);
}

// Initialize reading with our options.
//
SAIL_TRY(reader.start_reading(path, plugin_info, read_options));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in the requested pixel format (RGB).
//
sail::image image;
SAIL_TRY(reader.read_next_frame(&image));

// Finish reading.
//
SAIL_TRY(reader.stop_reading());

// Print the image meta information if any (JPEG comments etc.).
//
const std::map<std::string, std::string> meta_entries = image.meta_entries();

if (!meta_entries.empty()) {
    const std::pair<std::string, std::string> first_pair = *meta_entries.begin();
    SAIL_LOG_DEBUG("%s: %s", first_pair.first.c_str(), first_pair.second.c_str());
}
```
