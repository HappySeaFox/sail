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
  * [In what pixel format SAIL loading functions output images?](#in-what-pixel-format-sail-loading-functions-output-images)
  * [What pixel formats SAIL is able to write?](#what-pixel-formats-sail-is-able-to-write)
  * [Does SAIL support animated and multi\-paged images?](#does-sail-support-animated-and-multi-paged-images)
  * [Does SAIL support loading from memory?](#does-sail-support-loading-from-memory)
  * [How does SAIL support image formats?](#how-does-sail-support-image-formats)
  * [Does SAIL preload codecs in the initialization routine?](#does-sail-preload-codecs-in-the-initialization-routine)
    * [SAIL\_COMBINE\_CODECS is OFF](#sail_combine_codecs-is-off)
    * [SAIL\_COMBINE\_CODECS is ON](#sail_combine_codecs-is-on)
  * [How does SAIL look for codecs?](#how-does-sail-look-for-codecs)
    * [VCPKG port on any platform](#vcpkg-port-on-any-platform)
    * [Standalone build or bundle, both compiled with SAIL\_COMBINE\_CODECS=ON](#standalone-build-or-bundle-both-compiled-with-sail_combine_codecson)
    * [Windows standalone build or bundle, both compiled with SAIL\_COMBINE\_CODECS=OFF (the default)](#windows-standalone-build-or-bundle-both-compiled-with-sail_combine_codecsoff-the-default)
    * [Unix including macOS (standalone build), compiled with SAIL\_COMBINE\_CODECS=OFF (the default)](#unix-including-macos-standalone-build-compiled-with-sail_combine_codecsoff-the-default)
  * [How can I point SAIL to my custom codecs?](#how-can-i-point-sail-to-my-custom-codecs)
  * [I'd like to reorganize the standard SAIL folder layout on Windows (for standalone build or bundle)](#id-like-to-reorganize-the-standard-sail-folder-layout-on-windows-for-standalone-build-or-bundle)
  * [Describe the memory management techniques implemented in SAIL](#describe-the-memory-management-techniques-implemented-in-sail)
    * [The memory management technique implemented in SAIL](#the-memory-management-technique-implemented-in-sail)
    * [Convention to call SAIL functions](#convention-to-call-sail-functions)
    * [External pointers stay untouched on error](#external-pointers-stay-untouched-on-error)
    * [Always set a pointer to state to NULL (C only)](#always-set-a-pointer-to-state-to-null-c-only)
  * [Can I implement an image codec in C\+\+?](#can-i-implement-an-image-codec-in-c)
  * [Describe codec info file format](#describe-codec-info-file-format)
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

- Windows: `conan`, `vcpkg`
- macOS: `conan`, `brew`
- Linux: native packages if available, `conan`, `vcpkg`

See [BUILDING](BUILDING.md).

## Does SAIL support static linking?

Yes. Compile with `-DBUILD_SHARED_LIBS=OFF`. This automatically enables `SAIL_COMBINE_CODECS`.

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

## Does SAIL provide simple one-line APIs?

Yes. SAIL provides four levels of APIs, depending on your needs: `junior`, `advanced`, `deep diver`, and `technical diver`.
`junior` is your choice.

## What pixel formats SAIL is able to read?

SAIL codecs always try to support as many input pixel formats as possible. The list of
pixel formats that can be loaded by SAIL is codec-specific and is not publicly available.

For example, some codecs may be able to load just 3 input pixel formats. Other may be able to load 10.

## In what pixel format SAIL loading functions output images?

SAIL always tries to output pixel format close to the source pixel format as much as possible.
Ideally (but not always), it outputs the same pixel format as stored in the image.

For example, SAIL outputs BPP24-BGR images from full-color BMP files without transparency.

You can also consider conversion functions from `libsail-manip`.

## What pixel formats SAIL is able to write?

SAIL codecs always try to support as much output pixel formats as possible. SAIL doesn't convert
one pixel format to another in saving operations. Images are always written as is.

The list of pixel formats that can be written by SAIL is codec-specific and is publicly available in every
.codec.info file. It can be accessed through `sail_codec_info_from_extension() -> codec_info -> save_features ->
pixel_formats`.

## Does SAIL support animated and multi-paged images?

Yes. Just continue loading the image file until the loading functions return `SAIL_OK`.
If no more frames are available, the loading functions return `SAIL_ERROR_NO_MORE_FRAMES`.

## Does SAIL support loading from memory?

Yes. SAIL supports loading/saving from/to files and memory. For technical divers,
it's also possible to use custom I/O sources.

See `sail_start_loading_file()`, `sail_start_loading_mem()`, and `sail_start_loading_io()`.

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

### Standalone build or bundle, both compiled with SAIL_COMBINE_CODECS=ON

Same to VCPKG port.

### Windows standalone build or bundle, both compiled with SAIL_COMBINE_CODECS=OFF (the default)
1. `SAIL_CODECS_PATH` environment variable
2. `<SAIL DEPLOYMENT FOLDER>\lib\sail\codecs`
3. Hardcoded `SAIL_CODECS_PATH` in config.h

### Unix including macOS (standalone build), compiled with SAIL_COMBINE_CODECS=OFF (the default)
1. `SAIL_CODECS_PATH` environment variable
2. Hardcoded `SAIL_CODECS_PATH` in config.h

`<FOUND PATH>/lib` is added to `LD_LIBRARY_PATH`.

Additionally, `SAIL_THIRD_PARTY_CODECS_PATH` environment variable with a list of ';'-separated paths
is searched if `SAIL_THIRD_PARTY_CODECS_PATH` is enabled in CMake, (the default) so you can load your own codecs
from there.

## How can I point SAIL to my custom codecs?

If `SAIL_THIRD_PARTY_CODECS_PATH` is enabled in CMake (the default), you can set the `SAIL_THIRD_PARTY_CODECS_PATH` environment variable
to a list of ';'-separated paths containing your custom SAIL codecs.

On Windows, `sail.dll location` and `SAIL_THIRD_PARTY_CODECS_PATH/lib` are the only places where codecs DLL dependencies are searched.
No other paths are searched. Use WIN32 API `AddDllDirectory` to add your own DLL dependencies search path.
On other platforms, `SAIL_THIRD_PARTY_CODECS_PATH/lib` is added to `LD_LIBRARY_PATH`.

If `SAIL_THIRD_PARTY_CODECS_PATH` is `OFF`, loading custom codecs is disabled.

## I'd like to reorganize the standard SAIL folder layout on Windows (for standalone build or bundle)

You can surely do that. However, with the standard layout SAIL detects the codecs' location automatically.
If you reorganize the standard SAIL folder layout, you'll need to specify the new codecs' location by
setting the `SAIL_CODECS_PATH` environment variable.

## Describe the memory management techniques implemented in SAIL

### The memory management technique implemented in SAIL

Internally, SAIL always cleans up on errors. If you encounter a memory leak on error, please report it.

**C only:** However, if an engineer encounters an error in the middle of loading or saving an image with the `advanced`
or a deeper API, it's always a responsibility of the engineer to stop loading or saving with
`sail_stop_loading()` or `sail_stop_saving()`. These functions execute a proper cleanup in the underlying codec.
If you don't call `sail_stop_loading()` or `sail_stop_saving()` in this situation, be prepared for memory leaks.

**C++ only:** C++ engineers are more lucky. The C++ binding executes the necessary cleanup automatically in this
situation in `~image_input()` or `~image_output()`.

### Convention to call SAIL functions

It's always recommended (but not required) to use the `SAIL_TRY()` macro to call SAIL functions. It's also always recommended
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

SAIL_TRY(sail_start_loading_file(..., &state));

/*
 * If sail_load_next_frame() fails, the image pointer stays untouched (NULL).
 * This code prints NULL value on error.
 */
SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image),
                    /* cleanup */ printf("%p\n", image),
                                  sail_stop_loading(state));
```

### Always set a pointer to state to NULL (C only)

C loading and saving functions require a local void pointer to state. Always set it to NULL before
loading or saving. For example:

```C
void *state = NULL;

SAIL_TRY(sail_start_loading_file(..., &state));

SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, ...),
                    /* cleanup */ sail_stop_loading(state));
```

## Can I implement an image codec in C++?

Yes. Your codec just needs to export a set of public functions so SAIL can recognize and use it.
Theoretically, you can implement your codec in any programming language.

## Describe codec info file format

Let's take a hypothetical codec info:

```
# This section describes the codec per se.
#
[codec]

# Codec layout is a set of functions it exports. Different layouts generations are not compatible.
# libsail supports just a single (current) layout. Cannot be empty.
#
layout=6

# Semantic codec version. Cannot be empty.
#
version=1.0.0

# Codec priority. SAIL uses this property to sort the enumerated codecs by priority
# to speed up search of popular image formats by functions like sail_codec_info_from_path().
#
# HIGHEST = one of the most popular image formats like JPEG
# HIGH    = pretty popular and common image format like SVG
# MEDIUM  = moderate popularity
# LOW     = rare image format
# LOWEST  = very rare and/or too specific and/or ancient image format
#
priority=MEDIUM

# Short codec name. Must be uppercase. Cannot be empty.
#
name=ABC

# Codec description. Any human-readable string. Cannot be empty.
#
description=Some ABC Format

# ';'-separated list of hex-encoded magic numbers indentifying this image format.
# Can be empty only if the list of file extensions below is not empty.
#
magic-numbers=34 AB

# ';'-separated list of file extensions indentifying this image format.
# Can be empty only if the list of magic numbers above is not empty.
#
extensions=abc;bca

# ';'-separated list of MIME types indentifying this image format. Can be empty.
#
mime-types=image/abc

# Section of various features describing what the image codec can actually load.
#
[load-features]

# ';'-separated list of what the image codec can actually read.
# Can be empty if the image codec cannot load images.
#
# Possible values:
#    STATIC      - Can load static images.
#    ANIMATED    - Can load animated images.
#    MULTI-PAGED - Can load multi-paged (but not animated) images.
#    META-DATA   - Can load image meta data like JPEG comments or EXIF.
#    INTERLACED  - Can load interlaced images.
#    ICCP        - Can load embedded ICC profiles.
#
features=STATIC;META-DATA;INTERLACED;ICCP

# ';'-separated list of codec-specific tuning options. For example, it's possible
# to disable ABC codec filtering with setting abc-filtering to 0 in load options.
# Tuning options' names must start with the codec name to avoid confusing.
#
# The list of possible values for every tuning option is not current available
# programmatically. Every codec must document them in the codec info file.
#
# For example:
#   - abc-filtering: Tune filtering. Possible values: 0 (disable), 1 (light), 2 (hard).
#
tuning=abc-filtering

# Section of various features describing what the image codec can actually save.
#
[save-features]

# ';'-separated list of what the image codec can actually write.
# Can be empty if the image codec cannot save images.
#
# Possible values:
#    STATIC      - Can save static images.
#    ANIMATED    - Can save animated images.
#    MULTI-PAGED - Can save multi-paged (but not animated) images.
#    META-DATA   - Can save image meta data like JPEG comments or EXIF.
#    INTERLACED  - Can save interlaced images.
#    ICCP        - Can save embedded ICC profiles.
#
features=STATIC;META-DATA;INTERLACED;ICCP

# ';'-separated list of codec-specific tuning options. For example, it's possible
# to disable ABC codec filtering with setting abc-filtering to 0 in save options.
# Tuning options' names must start with the codec name to avoid confusing.
#
# The list of possible values for every tuning option is not current available
# programmatically. Every codec must document them in the codec info file.
#
# For example:
#   - abc-filtering: Tune filtering. Possible values: 0 (disable), 1 (light), 2 (hard).
#
tuning=abc-filtering

# ';'-separated list of pixel formats the image codec can write.
# Can be empty if the image codec cannot save images.
#
# Note: SAIL doesn't convert images while saving. It writes them 1:1. The image codec
# can take as input and save 8-bit indexed, 24-bit RGB, and 32-bit RGBA images.
#
pixel-formats=BPP8-INDEXED;BPP24-RGB;BPP32-BGRA

# ';'-separated list of compressions the image codec can write. Cannot be empty if the image codec
# can save images. If the image codec cannot select compressions, specify UNSUPPORTED.
#
compression-types=DEFLATE;RLE

# Default compression from the list above to use when no explicit compression was selected
# by a client application. Can be empty if the image codec cannot save images.
#
default-compression=DEFLATE

# Minimum compression level. This parameter is not used and must be 0 if the list
# of supported compressions has more than one compression or if the list is empty.
#
# The data type is double. C function atof() is used to convert the value to double.
# Decimal-point character is determined by the current C locale.
#
compression-level-min=1

# Maximum compression level. This parameter is not used and must be 0 if the list
# of supported compressions has more than one compression or if the list is empty.
#
# The data type is double. C function atof() is used to convert the value to double.
# Decimal-point character is determined by the current C locale.
#
compression-level-max=9

# Default compression level to use when no explicit compression level was selected
# by a client application. This parameter is not used and must be 0 if the list
# of supported compressions has more than one compression or if the list is empty.
# Must be in the range of the min/max limits above.
#
# The data type is double. C function atof() is used to convert the value to double.
# Decimal-point character is determined by the current C locale.
#
compression-level-default=6

# Step to increment or decrement compression level. This parameter is not used
# and must be 0 if the list of supported compressions has more than one compression
# or if the list is empty.
#
# The data type is double. C function atof() is used to convert the value to double.
# Decimal-point character is determined by the current C locale.
#
compression-level-step=1
```

## Are there any C/C++ examples?

Yes. See the `examples` directory in the source tree.

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
