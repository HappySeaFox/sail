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

![Screenshot](.github/qt-demo.png?raw=true)

See [*-with-c-api examples](examples/c++/qt). Qt is a pretty easy and a very good readable framework. It's easy to adapt them to your needs.

### Written in Qt/C++ utilizing SAIL C++ API

See [*-with-c++-api examples](examples/c++/qt). Qt is a pretty easy and a very good readable framework. It's easy to adapt them to your needs.

## APIs overview

SAIL provides 4 levels of APIs depending on your needs. Let's have a look at them.

### 1. `junior`

**Purpose:** load a single image frame in a one-line manner from a file or memory.

#### C:
```C
struct sail_image *image;

SAIL_TRY(sail_load_image_from_file(path, &image));

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 *
 * In particular, you can convert it to a different pixel format with functions
 * from libsail-manip. With sail_convert_image(), for example.
 */

/*
 * Destroy the image when it's not needed anymore.
 */
sail_destroy_image(image);
```

#### C++:
```C++
sail::image image(path);

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
//
// In particular, you can convert it to a different pixel format with image::convert().
```

### 2. `advanced`

**Purpose:** load a single-paged or multi-paged image from a file or memory.

#### C:
```C
/*
 * A local void pointer is needed for SAIL to save a state in it.
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;
struct sail_image *image;

/*
 * Starts loading the specified file.
 */
SAIL_TRY_OR_CLEANUP(sail_start_loading_file(path, NULL, &state),
                    /* cleanup */ sail_stop_loading(state));

/*
 * Load just a single frame. It's possible to load more frames if any. Just continue
 * loading frames till sail_load_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 *
 * SAIL always outputs frames of the same size.
 */
SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image),
                    /* cleanup */ sail_stop_loading(state));

/*
 * It's essential to ALWAYS stop loading to free memory resources.
 * Avoiding doing so will lead to memory leaks.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_loading(state),
         /* cleanup */ sail_destroy_image(image));

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 */

/*
 * Destroy the image when it's not needed anymore.
 */
sail_destroy_image(image);
```

#### C++:
```C++
sail::image_input input;
sail::image image;

// Starts loading the specified file.
//
SAIL_TRY(input.start(path));

// Load just a single frame. It's possible to load more frames if any. Just continue
// loading frames till load_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
// SAIL always outputs frames of the same size.
//
SAIL_TRY(input.next_frame(&image));

// It's essential to ALWAYS stop loading to free memory resources.
// Avoiding doing so will lead to memory leaks. ~image_input() always
// stops loading.
//
SAIL_TRY(input.stop());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
```

### 3. `deep diver`

**Purpose:** load a single-paged or multi-paged image from a file or memory. Specify a concrete codec to use.
             Possibly specify I/O controlling options.

#### C:
```C
/*
 * Optional: Initialize a new SAIL context explicitly and preload all codecs.
 * Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
 */
SAIL_TRY(sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS));

struct sail_load_options *load_options;
struct sail_image *image;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to load JPEGs.
 */
const struct sail_codec_info *codec_info;
SAIL_TRY(sail_codec_info_from_extension("JPEG", &codec_info));

/*
 * Allocate new load options and copy defaults from the codec-specific load features.
 */
SAIL_TRY(sail_alloc_load_options_from_features(codec_info->load_features, &load_options));

/*
 * Obtain an image data in a buffer: load it from a file etc.
 */
void *buffer = ...
size_t buffer_length = ...

/*
 * Initialize loading from memory with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_loading_mem_with_options(buffer,
                                                        buffer_length,
                                                        codec_info,
                                                        load_options,
                                                        &state),
                    /* cleanup */ sail_destroy_load_options(load_options));

/*
 * Our load options are not needed anymore.
 */
sail_destroy_load_options(load_options);

/*
 * Load just a single frame. It's possible to load more frames if any. Just continue
 * loading frames till sail_load_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 *
 * SAIL always outputs frames of the same size.
 */
SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image),
                    /* cleanup */ sail_stop_loading(state));

/*
 * Finish loading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_loading(state),
                    /* cleanup */ sail_destroy_image(image));

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 */

/*
 * Destroy the image when it's not needed anymore.
 */
sail_destroy_image(image);

/*
 * Optional: unload all codecs to free up some memory.
 */
sail_unload_codecs();
```

#### C++:
```C++
// Optional: Initialize a new SAIL context explicitly and preload all codecs.
// Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
//
sail::context::init(SAIL_FLAG_PRELOAD_CODECS);
sail::image_input input;

// Find the codec to load JPEGs.
//
sail::codec_info codec_info;
SAIL_TRY(codec_info::from_extension("JPEG", &codec_info));

// Instantiate new load options and copy defaults from the load features.
//
sail::load_options load_options;
SAIL_TRY(codec_info.load_features().to_load_options(&load_options));

// Obtain an image data in a buffer: load it from a file etc.
//
void *buffer = ...
size_t buffer_length = ...

// Initialize loading from memory with our options. The options will be deep copied.
//
SAIL_TRY(input.start(buffer, buffer_length, codec_info, load_options));

// Load just a single frame. It's possible to load more frames if any. Just continue
// loading frames till load_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
// SAIL always outputs frames of the same size.
//
sail::image image;
SAIL_TRY(input.next_frame(&image));

// It's essential to ALWAYS stop loading to free memory resources.
// Avoiding doing so will lead to memory leaks. ~image_input() always
// stops loading.
//
SAIL_TRY(input.stop());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
```

### 4. `technical diver`

**Purpose:** Comprehensive control provided for deep divers plus a custom I/O source.

#### C:

Instead of using `sail_start_loading_file_with_options()` in the `deep diver` example, create your own I/O stream
and call `sail_start_loading_io_with_options()`.

```C
/*
 * Optional: Initialize a new SAIL context explicitly and preload all codecs.
 * Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
 */
SAIL_TRY(sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS));

struct sail_load_options *load_options;
struct sail_image *image;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to load JPEGs.
 */
const struct sail_codec_info *codec_info;
SAIL_TRY(sail_codec_info_from_extension("JPEG", &codec_info));

/*
 * Create our custom I/O source.
 */
struct sail_io *io;
SAIL_TRY(sail_alloc_io(&io));

/*
 * Save a pointer to our data source. It will be passed back to the callback functions below.
 * You can free the data source in the close() callback.
 *
 * WARNING: If you don't call sail_stop_loading(), the close() callback is never called.
 *          Please make sure you always call sail_stop_loading().
 */
io->stream = my_data_source_pointer;

/*
 * Setup loading, seeking, flushing etc. callbacks for our custom I/O source.
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
 * Allocate new load options and copy defaults from the codec-specific load features.
 */
SAIL_TRY_OR_CLEANUP(sail_alloc_load_options_from_features(codec_info->load_features,
                                                          &load_options),
                    /* cleanup */ sail_destroy_io(io));

/*
 * Initialize loading with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_loading_io_with_options(io,
                                                       codec_info,
                                                       load_options,
                                                       &state),
                    /* cleanup */ sail_destroy_load_options(load_options),
                                  sail_destroy_io(io));

/*
 * Our load options are not needed anymore.
 */
sail_destroy_load_options(load_options);

/*
 * Load just a single frame. It's possible to load more frames if any. Just continue
 * loading frames till sail_load_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 *
 * SAIL always outputs frames of the same size.
 */
SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image),
                    /* cleanup */ sail_stop_loading(state),
                                  sail_destroy_io(io));

/*
 * Finish loading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_loading(state),
                    /* cleanup */ sail_destroy_image(image),
                                  sail_destroy_io(io));

sail_destroy_io(io);

/*
 * Handle the image pixels here.
 * Use image->width, image->height, image->bytes_per_line,
 * image->pixel_format, and image->pixels for that.
 */

/*
 * Destroy the image when it's not needed anymore.
 */
sail_destroy_image(image);

/*
 * Optional: unload all codecs to free up some memory.
 */
sail_unload_codecs();
```

#### C++:
```C++
// Optional: Initialize a new SAIL context explicitly and preload all codecs.
// Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
//
sail::context::init(SAIL_FLAG_PRELOAD_CODECS);
sail::image_input input;

// Find the codec info by a file extension.
//
sail::codec_info codec_info;
SAIL_TRY(codec_info::from_path(path, &codec_info));

// Create our custom I/O source.
//
sail::io io;

//
// Save a pointer to our data source. It will be passed back to the callback functions below.
// You can free the data source in the close() callback.
//
// WARNING: If you don't call input.stop(), the close() callback is never called.
//          Please make sure you always call input.stop().
//
io.with_stream(my_data_source_pointer);

// Setup loading, seeking, flushing etc. callbacks for our custom I/O source.
// All of them must be set.
//
io.with_read(io_my_data_source_read)
  .with_seek(io_my_data_source_seek)
  .with_tell(io_my_data_source_tell)
  .with_write(io_my_data_source_write)
  .with_flush(io_my_data_source_flush)
  .with_close(io_my_data_source_close)
  .with_eof(io_my_data_source_eof);

// Instantiate new load options and copy defaults from the load features.
//
sail::load_options load_options;
SAIL_TRY(codec_info.load_features().to_load_options(&load_options));

// Initialize loading with our I/O stream and options.
//
SAIL_TRY(input.start(io, codec_info, load_options));

// Load just a single frame. It's possible to load more frames if any. Just continue
// loading frames till load_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
// SAIL always outputs frames of the same size.
//
sail::image image;
SAIL_TRY(input.next_frame(&image));

// It's essential to ALWAYS stop loading to free memory resources.
// Avoiding doing so will lead to memory leaks. ~image_input() always
// stops loading.
//
SAIL_TRY(input.stop());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
```
