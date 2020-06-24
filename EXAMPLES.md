Table of Contents
=================

* [Table of Contents](#table-of-contents)
  * [Full\-featured applications](#full-featured-applications)
    * [Written in pure C and SDL](#written-in-pure-c-and-sdl)
    * [Written in Qt/C\+\+ utilizing SAIL C API](#written-in-qtc-utilizing-sail-c-api)
    * [Written in Qt/C\+\+ utilizing SAIL C\+\+ API](#written-in-qtc-utilizing-sail-c-api-1)
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
    * [4\. technical diver](#4-technical-diver)
      * [C:](#c-6)
      * [C\+\+:](#c-7)

## Full-featured applications

### Written in pure C and/or SDL

See [examples](examples/c).

### Written in Qt/C++ utilizing SAIL C API

![Screenshot](.github/qt-demo.jpg?raw=true)

See [*-with-c-api examples](examples/c++/qt). Qt is a pretty easy and a very good readable framework. It's easy to adapt them to your needs.

### Written in Qt/C++ utilizing SAIL C++ API

See [*-with-c++-api examples](examples/c++/qt). Qt is a pretty easy and a very good readable framework. It's easy to adapt them to your needs.

## APIs overview

SAIL provides 4 levels of APIs depending on your needs. Let's have a look at them.

### 1. `junior`

**Purpose:** read a single image frame in a one-line manner.

#### C:
```C
struct sail_context *context;

/*
 * Initialize SAIL context. You could cache the context and re-use it multiple times.
 * When it's not needed anymore, call sail_finish(context).
 */
SAIL_TRY(sail_init(&context));

struct sail_image *image;
unsigned char *image_bits;

/*
 * sail_read() reads the image and outputs pixels in BPP32-RGBA pixel format for image formats
 * with transparency support and BPP24-RGB otherwise.
 */
SAIL_TRY(sail_read(path,
                   context,
                   &image,
                   (void **)&image_bits));

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

// read() reads the image and outputs pixels in BPP32-RGBA pixel format for image formats
// with transparency support and BPP24-RGB otherwise.
//
SAIL_TRY(reader.read(path, &image));

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```

### 2. `advanced`

**Purpose:** read a single-paged or multi-paged image from a file or memory.

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
 * The subsequent calls to sail_read_next_frame() will output pixels in BPP32-RGBA pixel format
 * for image formats with transparency support and BPP24-RGB otherwise.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path, context, NULL, &state),
                    /* cleanup */ sail_stop_reading(state));

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns 0. If no more frames are available,
 * it returns SAIL_NO_MORE_FRAMES.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image, (void **)&image_bits),
                    /* cleanup */ sail_stop_reading(state));

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
 * If you have no plans to re-use the same context in the future, finish working with this
 * context and unload all loaded plugins attached to it.
 */
sail_finish(context);
context = NULL;
```

#### C++:
```C++
// Initialize SAIL context. You could cache the context as a class member and re-use it
// multiple times.
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
// The subsequent calls to read_next_frame() will output pixels in BPP32-RGBA pixel format
// for image formats with transparency support and BPP24-RGB otherwise.
//
SAIL_TRY(reader.start_reading(path));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in BPP32-RGBA pixel format for image formats
// with transparency support and BPP24-RGB otherwise.
//
SAIL_TRY(reader.read_next_frame(&image));

SAIL_TRY(reader.stop_reading());

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```

### 3. `deep diver`

**Purpose:** read a single-paged or multi-paged image from a file or memory. Specify a concrete plugin to use.
             Possibly specify a desired pixel format to output.

#### C:
```C
struct sail_context *context = NULL;

/*
 * Initialize SAIL context and preload all plugins. Plugins are lazy-loaded when
 * SAIL_FLAG_PRELOAD_PLUGINS is not specified. You could cache the context and re-use it
 * multiple times. When it's not needed anymore, call sail_finish(context).
 */
SAIL_TRY(sail_init_with_flags(&context, SAIL_FLAG_PRELOAD_PLUGINS));

struct sail_read_options *read_options = NULL;
struct sail_image *image = NULL;
unsigned char *image_bits = NULL;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to read JPEGs.
 */
const struct sail_plugin_info *plugin_info;
SAIL_TRY(sail_plugin_info_from_extension("JPEG", context, &plugin_info));

/*
 * Allocate new read options and copy defaults from the plugin-specific read features
 * (preferred output pixel format etc.).
 */
SAIL_TRY(sail_alloc_read_options_from_features(plugin_info->read_features, &read_options));

/*
 * Obtain an image data in a buffer: read it from a file etc.
 */
void *buffer = ...
size_t buffer_length = ...

/*
 * Initialize reading from memory with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_mem_with_options(buffer,
                                                        buffer_length,
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
 * sail_read_next_frame() outputs pixels in the requested pixel format (BPP24-RGB or BPP32-RGBA by default).
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state,
                                         &image,
                                         (void **)&image_bits),
                    /* cleanup */ sail_stop_reading(state));

/*
 * Finish reading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                    /* cleanup */ free(image_bits),
                                  sail_destroy_image(image));

/*
 * Print the image meta information if any (JPEG comments etc.).
 */
struct sail_meta_entry_node *node = image->meta_entry_node;

if (node != NULL) {
    SAIL_LOG_DEBUG("%s: %s", node->key, node->value);
}

/*
 * Handle the image bits here.
 * Use image->width, image->height, image->bytes_per_line,
 * and image->pixel_format for that.
 */

free(image_bits);
sail_destroy_image(image);

/*
 * Optional: unload all plugins to free up some memory if you plan to re-use the same
 * context in the future.
 */
sail_unload_plugins(context);

/*
 * If you have no plans to re-use the same context in the future, finish working with
 * this context and unload all loaded plugins attached to it.
 */
sail_finish(context);
context = NULL;
```

#### C++:
```C++
// Initialize SAIL context and preload all plugins. Plugins are lazy-loaded when
// SAIL_FLAG_PRELOAD_PLUGINS is not specified. You could cache the context
// and re-use it multiple times.
//
sail::context context(SAIL_FLAG_PRELOAD_PLUGINS);
sail::image_reader reader(&context);

// Find the codec to read JPEGs.
//
sail::plugin_info plugin_info;
SAIL_TRY(context.plugin_info_from_extension("JPEG", &plugin_info));

// Instantiate new read options and copy defaults from the read features
// (preferred output pixel format etc.).
//
sail::read_options read_options;
SAIL_TRY(plugin_info.read_features().to_read_options(&read_options));

// Obtain an image data in a buffer: read it from a file etc.
//
void *buffer = ...
size_t buffer_length = ...

// Initialize reading from memory with our options. The options will be deep copied.
//
SAIL_TRY(reader.start_reading(buffer, buffer_length, plugin_info, read_options));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in the requested pixel format (BPP24-RGB or BPP32-RGBA by default).
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

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```

### 4. `technical diver`

**Purpose:** Comprehensive control provided for deep divers plus a custom I/O source.

#### C:

Instead of using `sail_start_reading_file_with_options()` in the `deep diver` example create your own I/O stream
and call `sail_start_reading_io_with_options()`.

```C
struct sail_context *context = NULL;

/*
 * Initialize SAIL context and preload all plugins. Plugins are lazy-loaded when
 * SAIL_FLAG_PRELOAD_PLUGINS is not specified. You could cache the context and re-use it
 * multiple times. When it's not needed anymore, call sail_finish(context).
 */
SAIL_TRY(sail_init_with_flags(&context, SAIL_FLAG_PRELOAD_PLUGINS));

struct sail_read_options *read_options = NULL;
struct sail_image *image = NULL;
unsigned char *image_bits = NULL;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to read JPEGs.
 */
const struct sail_plugin_info *plugin_info;
SAIL_TRY(sail_plugin_info_from_extension("JPEG", context, &plugin_info));

/*
 * Create our custom I/O source.
 */
struct sail_io *io;
SAIL_TRY(sail_alloc_io(&io));

/*
 * Save a pointer to our data source. It will be passed back to the callback functions below.
 * You can free the data source in the close() callback.
 *
 * WARNING: If you don't call sail_stop_reading(), the close() callback is never called.
 *          Please make sure you always call sail_stop_reading().
 */
io->stream = my_data_source_pointer;

/*
 * Setup reading, seeking, flushing etc. callbacks for our custom I/O source.
 * All of them must be set.
 */
io->read  = io_my_data_source_read;
io->seek  = io_my_data_source_seek;
io->tell  = io_my_data_source_tell;
io->write = io_my_data_source_write;
io->flush = io_my_data_source_flush;
io->close = io_my_data_source_close;
io->eof   = io_my_data_source_eof;

/*
 * Allocate new read options and copy defaults from the plugin-specific read features
 * (preferred output pixel format etc.).
 */
SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(plugin_info->read_features,
                                                          &read_options),
                    /* cleanup */ sail_destroy_io(io));

/*
 * Initialize reading with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options(io,
                                                       context,
                                                       plugin_info,
                                                       read_options,
                                                       &state),
                    /* cleanup */ sail_destroy_read_options(read_options),
                                  sail_destroy_io(io));

/*
 * Our read options are not needed anymore.
 */
sail_destroy_read_options(read_options);

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns 0. If no more frames are available,
 * it returns SAIL_NO_MORE_FRAMES.
 *
 * sail_read_next_frame() outputs pixels in the requested pixel format (BPP24-RGB or BPP32-RGBA by default).
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state,
                                         &image,
                                         (void **)&image_bits),
                    /* cleanup */ sail_stop_reading(state),
                                  sail_destroy_io(io));

/*
 * Finish reading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
                    /* cleanup */ free(image_bits),
                                  sail_destroy_image(image),
                                  sail_destroy_io(io));

sail_destroy_io(io);

/*
 * Print the image meta information if any (JPEG comments etc.).
 */
struct sail_meta_entry_node *node = image->meta_entry_node;

if (node != NULL) {
    SAIL_LOG_DEBUG("%s: %s", node->key, node->value);
}

/*
 * Handle the image bits here.
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
 * If you have no plans to re-use the same context in the future, finish working with this
 * context and unload all loaded plugins attached to it.
 */
sail_finish(context);
context = NULL;
```

#### C++:
```C++
// Initialize SAIL context and preload all plugins. Plugins are lazy-loaded when
// SAIL_FLAG_PRELOAD_PLUGINS is not specified. You could cache the context and re-use
// it multiple times.
//
sail::context context(SAIL_FLAG_PRELOAD_PLUGINS);
sail::image_reader reader(&context);

// Find the codec info by a file extension.
//
sail::plugin_info plugin_info;
SAIL_TRY(context.plugin_info_from_path(path, &plugin_info));

/*
 * Create our custom I/O source.
 */
sail::io io;

/*
 * Save a pointer to our data source. It will be passed back to the callback functions below.
 * You can free the data source in the close() callback.
 *
 * WARNING: If you don't call reader.stop_reading(), the close() callback is never called.
 *          Please make sure you always call reader.stop_reading().
 */
io.with_stream(my_data_source_pointer);

/*
 * Setup reading, seeking, flushing etc. callbacks for our custom I/O source.
 * All of them must be set.
 */
io.with_read(io_my_data_source_read)
  .with_seek(io_my_data_source_seek)
  .with_tell(io_my_data_source_tell)
  .with_write(io_my_data_source_write)
  .with_flush(io_my_data_source_flush)
  .with_close(io_my_data_source_close)
  .with_eof(io_my_data_source_eof);

// Instantiate new read options and copy defaults from the read features
// (preferred output pixel format etc.).
//
sail::read_options read_options;
SAIL_TRY(plugin_info.read_features().to_read_options(&read_options));

// Initialize reading with our I/O stream and options.
//
SAIL_TRY(reader.start_reading(io, plugin_info, read_options));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns 0. If no more frames are available,
// it returns SAIL_NO_MORE_FRAMES.
//
// read_next_frame() outputs pixels in the requested pixel format (BPP24-RGB or BPP32-RGBA by default).
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

// Handle the image and its bits here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.bits() for that.
```
