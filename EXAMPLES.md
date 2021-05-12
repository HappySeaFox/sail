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

**Purpose:** read a single image frame in a one-line manner from a file or memory.

#### C:
```C
struct sail_image *image;

SAIL_TRY(sail_read_file(path, &image));

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

/*
 * Recommended: finish working with the implicitly allocated SAIL context in this thread
 * and unload all loaded codecs attached to it.
 */
sail_finish();
```

#### C++:
```C++
sail::image_reader reader;
sail::image image;

SAIL_TRY(reader.read(path, &image));

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.
//
// In particular, you can convert it to a different pixel format with image::convert().
```

### 2. `advanced`

**Purpose:** read a single-paged or multi-paged image from a file or memory.

#### C:
```C
/*
 * A local void pointer is needed for SAIL to save a state in it.
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;
struct sail_image *image;

/*
 * Starts reading the specified file.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_file(path, NULL, &state),
                    /* cleanup */ sail_stop_reading(state));

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image),
                    /* cleanup */ sail_stop_reading(state));

/*
 * It's essential to ALWAYS stop reading to free memory resources.
 * Avoiding doing so will lead to memory leaks.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
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
 * Recommended: finish working with the implicitly allocated SAIL context in this thread
 * and unload all loaded codecs attached to it.
 */
sail_finish();
```

#### C++:
```C++
sail::image_reader reader;
sail::image image;

// It's essential to ALWAYS stop reading to free memory resources.
// Avoiding doing so will lead to memory leaks. This code gets executed
// when the outer scope exits.
//
SAIL_AT_SCOPE_EXIT (
    reader.stop_reading();
);

// Starts reading the specified file.
//
SAIL_TRY(reader.start_reading(path));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
SAIL_TRY(reader.read_next_frame(&image));

SAIL_TRY(reader.stop_reading());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.

// Recommended: finish working with the implicitly allocated SAIL context in this thread
// and unload all loaded codecs attached to it.
//
sail::context::finish();
```

### 3. `deep diver`

**Purpose:** read a single-paged or multi-paged image from a file or memory. Specify a concrete codec to use.
             Possibly specify I/O controlling options.

#### C:
```C
/*
 * Optional: Initialize a new SAIL thread-local static context explicitly and preload all codecs.
 * Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
 */
SAIL_TRY(sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS));

struct sail_read_options *read_options;
struct sail_image *image;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to read JPEGs.
 */
const struct sail_codec_info *codec_info;
SAIL_TRY(sail_codec_info_from_extension("JPEG", &codec_info));

/*
 * Allocate new read options and copy defaults from the codec-specific read features.
 */
SAIL_TRY(sail_alloc_read_options_from_features(codec_info->read_features, &read_options));

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
                                                        codec_info,
                                                        read_options,
                                                        &state),
                    /* cleanup */ sail_destroy_read_options(read_options));

/*
 * Our read options are not needed anymore.
 */
sail_destroy_read_options(read_options);

/*
 * Read just a single frame. It's possible to read more frames if any. Just continue
 * reading frames till sail_read_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 *
 * By default, sail_read_next_frame() may convert specific pixel formats to be more prepared
 * for displaying. Use SAIL_IO_OPTION_CLOSE_TO_SOURCE to output pixels as close as possible
 * to the source.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image),
                    /* cleanup */ sail_stop_reading(state));

/*
 * Finish reading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
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

/*
 * Recommended: finish working with the explicitly allocated SAIL context in this thread
 * and unload all loaded codecs attached to it.
 */
sail_finish();
```

#### C++:
```C++
// Optional: Initialize a new SAIL thread-local static context explicitly and preload all codecs.
// Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
//
sail::context::init(SAIL_FLAG_PRELOAD_CODECS);
sail::image_reader reader;

// Find the codec to read JPEGs.
//
sail::codec_info codec_info;
SAIL_TRY(codec_info::from_extension("JPEG", &codec_info));

// Instantiate new read options and copy defaults from the read features.
//
sail::read_options read_options;
SAIL_TRY(codec_info.read_features().to_read_options(&read_options));

// Obtain an image data in a buffer: read it from a file etc.
//
void *buffer = ...
size_t buffer_length = ...

// Initialize reading from memory with our options. The options will be deep copied.
//
SAIL_TRY(reader.start_reading(buffer, buffer_length, codec_info, read_options));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
// By default, read_next_frame() may convert specific pixel formats to be more prepared
// for displaying. Use SAIL_IO_OPTION_CLOSE_TO_SOURCE to output pixels as close as possible
// to the source.
//
sail::image image;
SAIL_TRY(reader.read_next_frame(&image));

// Finish reading.
//
SAIL_TRY(reader.stop_reading());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.

// Recommended: finish working with the implicitly allocated SAIL context in this thread
// and unload all loaded codecs attached to it.
//
sail::context::finish();
```

### 4. `technical diver`

**Purpose:** Comprehensive control provided for deep divers plus a custom I/O source.

#### C:

Instead of using `sail_start_reading_file_with_options()` in the `deep diver` example, create your own I/O stream
and call `sail_start_reading_io_with_options()`.

```C
/*
 * Optional: Initialize a new SAIL thread-local static context explicitly and preload all codecs.
 * Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
 */
SAIL_TRY(sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS));

struct sail_read_options *read_options;
struct sail_image *image;

/*
 * Always set the initial state to NULL in C or nullptr in C++.
 */
void *state = NULL;

/*
 * Find the codec to read JPEGs.
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
 * Allocate new read options and copy defaults from the codec-specific read features.
 */
SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(codec_info->read_features,
                                                          &read_options),
                    /* cleanup */ sail_destroy_io(io));

/*
 * Initialize reading with our options. The options will be deep copied.
 */
SAIL_TRY_OR_CLEANUP(sail_start_reading_io_with_options(io,
                                                       codec_info,
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
 * reading frames till sail_read_next_frame() returns SAIL_OK. If no more frames are available,
 * it returns SAIL_ERROR_NO_MORE_FRAMES.
 *
 * By default, sail_read_next_frame() may convert specific pixel formats to be more prepared
 * for displaying. Use SAIL_IO_OPTION_CLOSE_TO_SOURCE to output pixels as close as possible
 * to the source.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image),
                    /* cleanup */ sail_stop_reading(state),
                                  sail_destroy_io(io));

/*
 * Finish reading.
 */
SAIL_TRY_OR_CLEANUP(sail_stop_reading(state),
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

/*
 * Recommended: finish working with the explicitly allocated SAIL context in this thread
 * and unload all loaded codecs attached to it.
 */
sail_finish();
```

#### C++:
```C++
// Optional: Initialize a new SAIL thread-local static context explicitly and preload all codecs.
// Codecs are lazy-loaded when SAIL_FLAG_PRELOAD_CODECS is not specified.
//
sail::context::init(SAIL_FLAG_PRELOAD_CODECS);
sail::image_reader reader;

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
// WARNING: If you don't call reader.stop_reading(), the close() callback is never called.
//          Please make sure you always call reader.stop_reading().
//
io.with_stream(my_data_source_pointer);

// Setup reading, seeking, flushing etc. callbacks for our custom I/O source.
// All of them must be set.
//
io.with_read(io_my_data_source_read)
  .with_seek(io_my_data_source_seek)
  .with_tell(io_my_data_source_tell)
  .with_write(io_my_data_source_write)
  .with_flush(io_my_data_source_flush)
  .with_close(io_my_data_source_close)
  .with_eof(io_my_data_source_eof);

// Instantiate new read options and copy defaults from the read features.
//
sail::read_options read_options;
SAIL_TRY(codec_info.read_features().to_read_options(&read_options));

// Initialize reading with our I/O stream and options.
//
SAIL_TRY(reader.start_reading(io, codec_info, read_options));

// Read just a single frame. It's possible to read more frames if any. Just continue
// reading frames till read_next_frame() returns SAIL_OK. If no more frames are available,
// it returns SAIL_ERROR_NO_MORE_FRAMES.
//
// By default, read_next_frame() may convert specific pixel formats to be more prepared
// for displaying. Use SAIL_IO_OPTION_CLOSE_TO_SOURCE to output pixels as close as possible
// to the source.
//
sail::image image;
SAIL_TRY(reader.read_next_frame(&image));

// Finish reading.
//
SAIL_TRY(reader.stop_reading());

// Handle the image and its pixels here.
// Use image.width(), image.height(), image.bytes_per_line(),
// image.pixel_format(), and image.pixels() for that.

// Recommended: finish working with the implicitly allocated SAIL context in this thread
// and unload all loaded codecs attached to it.
//
sail::context::finish();
```
