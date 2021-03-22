Table of Contents
=================

* [SAIL Frequently Asked Questions (FAQ)](#sail-frequently-asked-questions-faq)
  * [How old is SAIL?](#how-old-is-sail)
  * [Is SAIL cross\-platform?](#is-sail-cross-platform)
  * [What's the preferred way of installation?](#whats-the-preferred-way-of-installation)
  * [Does SAIL support static linking?](#does-sail-support-static-linking)
  * [What are the competitors of SAIL?](#what-are-the-competitors-of-sail)
  * [Describe the high\-level APIs](#describe-the-high-level-apis)
  * [Does SAIL provide simple one\-line APIs?](#does-sail-provide-simple-one-line-apis)
  * [What pixel formats SAIL is able to read?](#what-pixel-formats-sail-is-able-to-read)
  * [In what pixel format SAIL reading functions output images?](#in-what-pixel-format-sail-reading-functions-output-images)
  * [What pixel formats SAIL is able to write?](#what-pixel-formats-sail-is-able-to-write)
  * [Does SAIL support animated and multi\-paged images?](#does-sail-support-animated-and-multi-paged-images)
  * [Does SAIL support reading from memory?](#does-sail-support-reading-from-memory)
  * [How does SAIL support image formats?](#how-does-sail-support-image-formats)
  * [Does SAIL preload codecs in the initialization routine?](#does-sail-preload-codecs-in-the-initialization-routine)
    * [SAIL\_COMBINE\_CODECS is OFF](#sail_combine_codecs-is-off)
    * [SAIL\_COMBINE\_CODECS is ON](#sail_combine_codecs-is-on)
  * [How does SAIL look for codecs?](#how-does-sail-look-for-codecs)
    * [VCPKG port on any platform](#vcpkg-port-on-any-platform)
    * [Standalone build or bundle compiled with SAIL\_COMBINE\_CODECS=ON](#standalone-build-or-bundle-compiled-with-sail_combine_codecson)
    * [Windows (standalone build or bundle)](#windows-standalone-build-or-bundle)
    * [Unix including macOS (standalone build)](#unix-including-macos-standalone-build)
  * [How can I point SAIL to my custom codecs?](#how-can-i-point-sail-to-my-custom-codecs)
  * [I'd like to reorganize the standard SAIL folder layout on Windows (for standalone build or bundle)](#id-like-to-reorganize-the-standard-sail-folder-layout-on-windows-for-standalone-build-or-bundle)
  * [Please describe the memory management techniques implemented in SAIL](#please-describe-the-memory-management-techniques-implemented-in-sail)
    * [The memory management technique implemented in SAIL](#the-memory-management-technique-implemented-in-sail)
    * [Convention to call SAIL functions](#convention-to-call-sail-functions)
    * [External pointers stay untouched on error](#external-pointers-stay-untouched-on-error)
    * [Always set a pointer to state to NULL (C only)](#always-set-a-pointer-to-state-to-null-c-only)
  * [Can I implement an image codec in C\+\+?](#can-i-implement-an-image-codec-in-c)
  * [Are there any C/C\+\+ examples?](#are-there-any-cc-examples)
  * [Are there any bindings to other programming languages?](#are-there-any-bindings-to-other-programming-languages)
  * [How many image formats do you plan to implement?](#how-many-image-formats-do-you-plan-to-implement)
  * [I have questions, issues, or proposals](#i-have-questions-issues-or-proposals)

# SAIL Frequently Asked Questions (FAQ)

## How old is SAIL?

SAIL is rebranded ksquirrel-libs rewritten in C, improved and with high-level APIs. Ksquirrel-libs was a set of C++ image codecs
for the KSquirrel image viewer. See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

Technically, SAIL (ksquirrel-libs) was founded in 2003 making it one of the oldest image decoding libraries.

## Is SAIL cross-platform?

Yes. It's written in pure C11 and is highly portable. However, only the Windows, macOS, and Linux platforms
are currently supported. Pull requests to support more platforms are highly welcomed.

## What's the preferred way of installation?

- Windows: `vcpkg`
- macOS: `brew`
- Linux: native packages if available

See [BUILDING](BUILDING.md).

## Does SAIL support static linking?

Yes. Compile with `-DSAIL_STATIC=ON`. This automatically enables `SAIL_COMBINE_CODECS`.

Note for Unix platforms: the client application must be built with `-rdynamic` or an equivalent
to enable `dlopen` and `dlsym` on the same binary. If you use CMake, this could be achieved by
setting `CMAKE_ENABLE_EXPORTS` to `ON`.

## What are the competitors of SAIL?

- [FreeImage](https://freeimage.sourceforge.io)
- [DevIL](http://openil.sourceforge.net)
- [SDL_Image](https://www.libsdl.org/projects/SDL_image)
- [stb_image](https://github.com/nothings/stb)
- [Boost.GIL](https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/index.html)
- [gdk-pixbuf](https://developer.gnome.org/gdk-pixbuf)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html)
- [CImg](https://github.com/dtschump/CImg)
- [WIC (Windows only)](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec)

## Describe the high-level APIs

SAIL provides four levels of high-level APIs:

- `Junior`: I just want to load this JPEG from a file or memory
- `Advanced`: I want to load this animated GIF from a file or memory
- `Deep diver`: I want to load this animated GIF from a file or memory and have control over selected codecs and meta data
- `Technical diver`: I want everything above and my custom I/O source

See [EXAMPLES](EXAMPLES.md) for more.

## Does SAIL provide simple one-line APIs?

Yes. SAIL provides four levels of APIs, depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
`junior` is your choice. See [EXAMPLES](EXAMPLES.md) for more.

## What pixel formats SAIL is able to read?

SAIL codecs always try to support as much input pixel formats as possible. The list of
pixel formats that can be read by SAIL is codec-specific and is not publicly available.

For example, some codecs may be able to read just 3 input pixel formats. Other may be able to read 10.

## In what pixel format SAIL reading functions output images?

SAIL always tries to output pixel format close to the source pixel format as much as possible.
Ideally (but not always), it outputs the same pixel format as stored in the image.

For example, SAIL outputs indexed RGB images from indexed BMP files.

## What pixel formats SAIL is able to write?

SAIL codecs always try to support as much output pixel formats as possible. The list of
pixel formats that can be written by SAIL is codec-specific and is publicly available in every
.codec.info file. It can be accessed through `sail_codec_info_from_extension() -> codec_info -> write_features ->
output_pixel_formats`.

## Does SAIL support animated and multi-paged images?

Yes. Just continue reading the image file until the reading functions return `0`.
If no more frames are available, the reading functions return `SAIL_ERROR_NO_MORE_FRAMES`.

## Does SAIL support reading from memory?

Yes. SAIL supports reading/writing from/to files and memory. For technical divers,
it's also possible to use custom I/O sources.

See `sail_start_reading_file()`, `sail_start_reading_mem()`, and `sail_start_reading_io()`.

## How does SAIL support image formats?

SAIL supports image formats through dynamically loaded SAIL codecs. End-users never work
with the codecs directly. They always work with the abstract high-level APIs.

## Does SAIL preload codecs in the initialization routine?

### `SAIL_COMBINE_CODECS` is `OFF`

SAIL doesn't preload codecs in the initialization routine (`sail_init()`). It loads them on demand.
However, you can preload them explicitly with `sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS)`.

### `SAIL_COMBINE_CODECS` is `ON`

All codecs get loaded on application startup.

## How does SAIL look for codecs?

Codecs path search algorithm (first found path wins):

### VCPKG port on any platform

Codecs are combined into a dynamically linked library, so no need to search them.

Note for Unix platforms: the client application must be built with `-rdynamic` or an equivalent
to enable `dlopen` and `dlsym` on the same binary. If you use CMake, this could be achieved by
setting `CMAKE_ENABLE_EXPORTS` to `ON`.

### Standalone build or bundle compiled with SAIL_COMBINE_CODECS=ON

Same to VCPKG port.

### Windows (standalone build or bundle)
1. `SAIL_CODECS_PATH` environment variable
2. `<SAIL DEPLOYMENT FOLDER>\lib\sail\codecs`
3. Hardcoded `SAIL_CODECS_PATH` in config.h

### Unix including macOS (standalone build)
1. `SAIL_CODECS_PATH` environment variable
2. Hardcoded `SAIL_CODECS_PATH` in config.h

`<FOUND PATH>/lib` is added to `LD_LIBRARY_PATH`.

Additionally, `SAIL_MY_CODECS_PATH` environment variable is always searched so you can load your own codecs from there.

## How can I point SAIL to my custom codecs?

Set the `SAIL_MY_CODECS_PATH` environment variable to the location of your custom SAIL codecs.

On Windows, `sail.dll location` and `SAIL_MY_CODECS_PATH/lib` are the only places where codecs DLL dependencies are searched.
No other paths are searched. Use WIN32 API `AddDllDirectory` to add your own DLL dependencies search path.
On other platforms, `SAIL_MY_CODECS_PATH/lib` is added to `LD_LIBRARY_PATH`.

## I'd like to reorganize the standard SAIL folder layout on Windows (for standalone build or bundle)

You can surely do that. However, with the standard layout SAIL detects the codecs' location automatically.
If you reorganize the standard SAIL folder layout, you'll need to specify the new codecs' location by
setting the `SAIL_CODECS_PATH` environment variable.

## Please describe the memory management techniques implemented in SAIL

### The memory management technique implemented in SAIL

Internally, SAIL always cleans up on errors. If you encounter a memory leak on error, please report it.

**C only:** However, if an engineer encounters an error in the middle of reading or writing an image with the `advanced`
or a deeper API, it's always a responsibility of the engineer to stop reading or writing with
`sail_stop_reading()` or `sail_stop_writing()`. These functions execute a proper cleanup in the underlying codec.
If you don't call `sail_stop_reading()` or `sail_stop_writing()` in this situation, be prepared for memory leaks.

**C++ only:** C++ engineers are more lucky. The C++ binding executes the necessary cleanup automatically in this
situation in `~image_reader()` or `~image_writer()`.

### Convention to call SAIL functions

It's always recommended to use the `SAIL_TRY()` macro to call SAIL functions. It's also always recommended
to clean up in your code with the `SAIL_TRY_OR_CLEANUP()` macro if you need to.

### External pointers stay untouched on error

External pointers that are allocated or modified by SAIL functions stay untouched on error. For example:

```C
struct sail_image *image = NULL;
SAIL_TRY(sail_alloc_image(&image));

/*
 * If sail_alloc_palette_for_data() fails, the palette pointer stays untouched (NULL).
 * This code prints NULL value on error.
 */
SAIL_TRY_OR_CLEANUP(sail_alloc_palette_for_data(SAIL_PIXEL_FORMAT_BPP24_RGB, color_count, &image->palette));
                    /* cleanup */ printf("%p\n", image->palette),
                                  sail_destroy_image(image));
```

Or:

```C
void *state = NULL;
struct sail_image *image = NULL;

SAIL_TRY(sail_start_reading_file(..., &state));

/*
 * If sail_read_next_frame() fails, the image pointer stays untouched (NULL).
 * This code prints NULL value on error.
 */
SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, &image),
                    /* cleanup */ printf("%p\n", image),
                                  sail_stop_reading(state));
```

### Always set a pointer to state to NULL (C only)

C reading and writing functions require a local void pointer to state. Always set it to NULL before
reading or writing. For example:

```C
void *state = NULL;

SAIL_TRY(sail_start_reading_file(..., &state));

SAIL_TRY_OR_CLEANUP(sail_read_next_frame(state, ...),
                    /* cleanup */ sail_stop_reading(state));
```

## Can I implement an image codec in C++?

Yes. Your codec just needs to export a set of public functions so SAIL can recognize and use it.
Theoretically, you can implement your codec in any programming language.

## Are there any C/C++ examples?

Yes. See [EXAMPLES](EXAMPLES.md) for more.

## Are there any bindings to other programming languages?

Yes. Currently SAIL supports the following bindings:

1. C++

Pull requests to support more programming languages are highly welcomed.

## How many image formats do you plan to implement?

Ksquirrel-libs supported around 60 image formats. I don't plan to port all of them. However,
the most popular image formats will be definitely ported from ksquirrel-libs.

## I have questions, issues, or proposals

Opening a GitHub [issue](https://github.com/smoked-herring/sail/issues) is the preferred way
of communicating and solving problems.

Pull requests are always welcomed.
