Table of Contents
=================

* [SAIL Frequently Asked Questions (FAQ)](#sail-frequently-asked-questions-faq)
  * [What are the competitors of SAIL?](#what-are-the-competitors-of-sail)
  * [How old is SAIL?](#how-old-is-sail)
  * [Is SAIL cross\-platform?](#is-sail-cross-platform)
  * [How does SAIL support image formats?](#how-does-sail-support-image-formats)
  * [Can I implement an image decoding codec in C\+\+?](#can-i-implement-an-image-decoding-codec-in-c)
  * [Does SAIL preload all codecs in the initialization routine?](#does-sail-preload-all-codecs-in-the-initialization-routine)
  * [How does SAIL look for codecs?](#how-does-sail-look-for-codecs)
    * [Windows (VCPKG port)](#windows-vcpkg-port)
    * [Windows (standalone build)](#windows-standalone-build)
    * [Unix (including macOS)](#unix-including-macos)
  * [I'd like to reorganize the standard SAIL folder layout on Windows](#id-like-to-reorganize-the-standard-sail-folder-layout-on-windows)
  * [I moved SAIL codecs\. How can I point SAIL to the new location?](#i-moved-sail-codecs-how-can-i-point-sail-to-the-new-location)
  * [How can I point SAIL to my custom codecs?](#how-can-i-point-sail-to-my-custom-codecs)
  * [Describe the high\-level APIs](#describe-the-high-level-apis)
  * [Does SAIL provide simple one\-line APIs?](#does-sail-provide-simple-one-line-apis)
  * [How many image formats do you plan to implement?](#how-many-image-formats-do-you-plan-to-implement)
  * [Does SAIL support static linking?](#does-sail-support-static-linking)
  * [I have questions, issues, or proposals](#i-have-questions-issues-or-proposals)
  * [Please describe memory management techniques implemented in SAIL](#please-describe-memory-management-techniques-implemented-in-sail)
    * [The memory management technique implemented in SAIL](#the-memory-management-technique-implemented-in-sail)
    * [Convention to call SAIL functions](#convention-to-call-sail-functions)
    * [Pointers to images, pixels, etc\. are always freed on error](#pointers-to-images-pixels-etc-are-always-freed-on-error)
    * [Always set a pointer to state to NULL (C only)](#always-set-a-pointer-to-state-to-null-c-only)
  * [What pixel formats SAIL is able to read?](#what-pixel-formats-sail-is-able-to-read)
  * [What pixel formats SAIL is able to output after reading an image file?](#what-pixel-formats-sail-is-able-to-output-after-reading-an-image-file)
  * [What pixel formats SAIL is able to write?](#what-pixel-formats-sail-is-able-to-write)
  * [How can I read an image and output pixels in different formats?](#how-can-i-read-an-image-and-output-pixels-in-different-formats)
  * [Does SAIL support animated and multi\-paged images?](#does-sail-support-animated-and-multi-paged-images)
  * [Does SAIL support reading from memory?](#does-sail-support-reading-from-memory)
  * [Are there any C/C\+\+ examples?](#are-there-any-cc-examples)
  * [Are there any bindings to other programming languages?](#are-there-any-bindings-to-other-programming-languages)

# SAIL Frequently Asked Questions (FAQ)

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

## How old is SAIL?

SAIL is rebranded ksquirrel-libs rewritten in C, improved and with high-level APIs. Ksquirrel-libs was a set of C++ image codecs
for the KSquirrel image viewer. See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

Technically, SAIL (ksquirrel-libs) was founded in 2003 making it one of the oldest image decoding libraries.

## Is SAIL cross-platform?

Yes. It's written in pure C11 and is highly portable. However, only the Windows, macOS, and Linux platforms
are currently supported. Pull requests to support more platforms are highly welcomed.

## How does SAIL support image formats?

SAIL supports image formats through dynamically loaded SAIL codecs. End-users never work
with the codecs directly. They always work with the abstract high-level APIs.

## Can I implement an image decoding codec in C++?

Yes. Your codec just needs to export a set of public functions so SAIL can recognize and use it.
Theoretically, you can implement your codec in any programming language.

## Does SAIL preload all codecs in the initialization routine?

No. By default, SAIL doesn't preload all codecs in the initialization routine (`sail_init()`). It loads them on demand.
However, you can preload them explicitly with `sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS)`.

## How does SAIL look for codecs?

Codecs path search algorithm (first found path wins):

### Windows (VCPKG port)
1. `SAIL_CODECS_PATH` environment variable
2. `<SAIL DEPLOYMENT FOLDER>\sail\codecs`
3. Hardcoded `SAIL_CODECS_PATH` in config.h

### Windows (standalone build)
1. `SAIL_CODECS_PATH` environment variable
2. `<SAIL DEPLOYMENT FOLDER>\lib\sail\codecs`
3. Hardcoded `SAIL_CODECS_PATH` in config.h

### Unix (including macOS)
1. `SAIL_CODECS_PATH` environment variable
2. Hardcoded `SAIL_CODECS_PATH` in config.h

Additionally, `SAIL_MY_CODECS_PATH` environment variable is always searched so you can load your own codecs from there.

On Windows, `sail.dll location` and `<FOUND PATH>/lib` are the only places where codecs DLL dependencies are searched. No other paths are searched.
Use WIN32 API `AddDllDirectory` to add your own DLL dependencies search path.
On other platforms, `<FOUND PATH>/lib` is added to the DLL search path.

## I'd like to reorganize the standard SAIL folder layout on Windows

You can surely do that. However, with the standard layout SAIL detects the codecs' location automatically.
If you reorganize the standard SAIL folder layout, you'll need to specify the new codecs' location by
setting the `SAIL_CODECS_PATH` environment variable.

## I moved SAIL codecs. How can I point SAIL to the new location?

Set the `SAIL_CODECS_PATH` environment variable to a desired location of the prebuilt SAIL codecs.

On Windows, `sail.dll location` and `SAIL_CODECS_PATH/lib` are the only places where codecs DLL dependencies are searched. No other paths are searched.
Use WIN32 API `AddDllDirectory` to add your own DLL dependencies search path.
On other platforms, `SAIL_CODECS_PATH/lib` is added to the DLL search path.

## How can I point SAIL to my custom codecs?

Set the `SAIL_MY_CODECS_PATH` environment variable to a desired location of your custom SAIL codecs.

On Windows, `sail.dll location` and `SAIL_MY_CODECS_PATH/lib` are the only places where codecs DLL dependencies are searched. No other paths are searched.
Use WIN32 API `AddDllDirectory` to add your own DLL dependencies search path.
On other platforms, `SAIL_MY_CODECS_PATH/lib` is added to the DLL search path.

## Describe the high-level APIs

SAIL provides four levels of high-level APIs:

- `Junior`: I just want to load this JPEG from a file or memory
- `Advanced`: I want to load this animated GIF from a file or memory
- `Deep diver`: I want to load this animated GIF from a file or memory and have control over selected codecs and output pixel formats
- `Technical diver`: I want everything above and my custom I/O source

See [EXAMPLES](EXAMPLES.md) for more.

## Does SAIL provide simple one-line APIs?

Yes. SAIL provides four levels of APIs, depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
See [EXAMPLES](EXAMPLES.md) for more.

## How many image formats do you plan to implement?

Ksquirrel-libs supported around 60 image formats. I don't plan to port all of them. However,
the most popular image formats will be definitely ported from ksquirrel-libs.

## Does SAIL support static linking?

No. You're able to build `libsail` statically, however SAIL codecs are still standalone dynamically loaded files.

## I have questions, issues, or proposals

Opening a GitHub [issue](https://github.com/smoked-herring/sail/issues) is the preferred way
of communicating and solving problems.

Pull requests are always welcomed.

## Please describe memory management techniques implemented in SAIL

### The memory management technique implemented in SAIL

Internally, SAIL always tries to clean up on errors. If you encounter a memory leak on error, please report it.

**C only:** However, if an engineer encounters an error in the middle of reading or writing an image with the `advanced`
or a deeper API, it's always a responsibility of the engineer to stop reading or writing with
`sail_stop_reading()` or `sail_stop_writing()`. These functions execute a proper cleanup in the underlying codec.
If you don't call `sail_stop_reading()` or `sail_stop_writing()` in this situation, be prepared for memory leaks.

**C++ only:** C++ engineers are more lucky. The C++ binding executes the necessary cleanup automatically in this
situation in `~image_reader()` or `~image_writer()`.

### Convention to call SAIL functions

It's always recommended to use the `SAIL_TRY()` macro to call SAIL functions. It's also always recommended
to clean up in your code with the `SAIL_TRY_OR_CLEANUP()` macro if you need to.

### Pointers to images, pixels, etc. are always freed on error

Pointers that are modified by SAIL functions are always freed on error but may be left set
to a non-NULL value. SAIL does not reset them to a NULL value on error. For example:

```C
void *state = NULL;
struct sail_image *image;

SAIL_TRY(sail_start_reading_file(..., &state));

/*
 * SAIL frees the 'image' or error, but doesn't reset its value.
 * This code sample prints a non-NULL address on error.
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

## What pixel formats SAIL is able to read?

SAIL codecs always try to support as much input pixel formats as possible. The list of
pixel formats that can be read by SAIL is codec-specific and is not publicly available.

For example, some codecs may be able to read just 3 input pixel formats. Other may be able to read 10.

## What pixel formats SAIL is able to output after reading an image file?

SAIL is always able to output pixels in the `BPP32-RGBA` and `BPP32-BGRA` pixel formats after reading.
Most codecs are able to output the `SOURCE` pixel format as well. Some codecs support even more output pixel formats.
Use `sail_codec_info_from_extension() -> codec_info -> read_features -> output_pixel_formats` to determine
the list of supported output pixel formats per codec.

Use the `SOURCE` pixel format (if it's supported by the codec) to request the original image pixels.
For example, one may want to work with CMYK pixels in a print image without converting them to RGB.

## What pixel formats SAIL is able to write?

SAIL codecs always try to support as much output pixel formats as possible. The list of
pixel formats that can be written by SAIL is codec-specific and is publicly available in every
.codec.info file. It can be accessed through `sail_codec_info_from_extension() -> codec_info -> write_features ->
pixel_formats_mapping_node`.

`pixel_formats_mapping_node` is a map-like linked list describing what pixel formats SAIL is able to write from
the given input pixel format. Consider the following structure of `pixel_formats_mapping_node`:

| Input pixel format    | Output pixel formats                      |
| --------------------- | ----------------------------------------- |
| `BPP8-GRAYSCALE`      | `SOURCE`                                  |
| `BPP24-RGB`           | `SOURCE`, `BPP24-YCBCR`, `BPP8-GRAYSCALE` |

The structure above has the following meaning:

1. When a user has an image in `BPP8-GRAYSCALE` format, he/she is able to save it as a `BPP8-GRAYSCALE` (`SOURCE`) image only
2. When a user has an image in `BPP24-RGB` format, he/she is able to save it as a `BPP24-RGB` (`SOURCE`),
   `BPP24-YCBCR`, and `BPP8-GRAYSCALE` image

The `SOURCE` output pixel format is always supported.

## How can I read an image and output pixels in different formats?

Use read options for that. For example:

```C
sail_codec_info_from_extension(...);

sail_read_options_from_features(...);

read_options->output_pixel_format = SAIL_PIXEL_FORMAT_BPP32_RGBA;

sail_start_reading_file_with_options(...);
```

The `BPP24-RGB` and `BPP32-RGBA` output pixel formats are always supported.
See [EXAMPLES](EXAMPLES.md) for more.

## Does SAIL support animated and multi-paged images?

Yes. Just continue reading the image file until the reading functions return `0`.
If no more frames are available, the reading functions return `SAIL_ERROR_NO_MORE_FRAMES`.

## Does SAIL support reading from memory?

Yes. SAIL supports reading/writing from/to files and memory. For technical divers,
it's also possible to use custom I/O sources.

See `sail_start_reading_file()`, `sail_start_reading_mem()`, and `sail_start_reading_io()`.

## Are there any C/C++ examples?

Yes. See [EXAMPLES](EXAMPLES.md) for more.

## Are there any bindings to other programming languages?

Yes. Currently SAIL supports the following bindings:

1. C++

Pull requests to support more programming languages are highly welcomed.
