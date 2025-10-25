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
  * [In what pixel format do SAIL loading functions output images?](#in-what-pixel-format-do-sail-loading-functions-output-images)
  * [What pixel formats can SAIL write?](#what-pixel-formats-can-sail-write)
  * [Why does a codec support more pixel formats for writing than sail\_convert can convert to?](#why-does-a-codec-support-more-pixel-formats-for-writing-than-sail_convert-can-convert-to)
  * [Does SAIL support animated and multi\-paged images?](#does-sail-support-animated-and-multi-paged-images)
  * [Does SAIL support loading from memory?](#does-sail-support-loading-from-memory)
  * [How does SAIL support image formats?](#how-does-sail-support-image-formats)
  * [Does SAIL preload codecs in the initialization routine?](#does-sail-preload-codecs-in-the-initialization-routine)
    * [SAIL\_COMBINE\_CODECS is OFF](#sail_combine_codecs-is-off)
    * [SAIL\_COMBINE\_CODECS is ON](#sail_combine_codecs-is-on)
  * [How does SAIL look for codecs?](#how-does-sail-look-for-codecs)
    * [Conan recipe on any platform](#conan-recipe-on-any-platform)
    * [VCPKG port on any platform](#vcpkg-port-on-any-platform)
    * [Manually compiled on any platform with SAIL\_COMBINE\_CODECS=ON](#manually-compiled-on-any-platform-with-sail_combine_codecson)
    * [Manually compiled on Windows with SAIL\_COMBINE\_CODECS=OFF (the default)](#manually-compiled-on-windows-with-sail_combine_codecsoff-the-default)
    * [Manually compiled on Unix (including macOS) SAIL\_COMBINE\_CODECS=OFF (the default)](#manually-compiled-on-unix-including-macos-sail_combine_codecsoff-the-default)
  * [How can I point SAIL to my custom codecs?](#how-can-i-point-sail-to-my-custom-codecs)
  * [I'd like to reorganize the standard SAIL folder layout on Windows](#id-like-to-reorganize-the-standard-sail-folder-layout-on-windows)
  * [Describe the memory management techniques implemented in SAIL](#describe-the-memory-management-techniques-implemented-in-sail)
    * [The memory management technique implemented in SAIL](#the-memory-management-technique-implemented-in-sail)
    * [Convention to call SAIL functions](#convention-to-call-sail-functions)
    * [External pointers stay untouched on error](#external-pointers-stay-untouched-on-error)
    * [Always set a pointer to state to NULL (C only)](#always-set-a-pointer-to-state-to-null-c-only)
  * [Can I implement an image codec in C\+\+?](#can-i-implement-an-image-codec-in-c)
  * [Describe codec info file format](#describe-codec-info-file-format)
  * [Can I compile codecs dependencies (like libjpeg) directly into SAIL?](#can-i-compile-codecs-dependencies-like-libjpeg-directly-into-sail)
  * [Are there any C/C\+\+ examples?](#are-there-any-cc-examples)
  * [Are there any bindings to other programming languages?](#are-there-any-bindings-to-other-programming-languages)
  * [How many image formats are you going to implement?](#how-many-image-formats-are-you-going-to-implement)
  * [I have problems with include paths with vcpkg without CMake](#i-have-problems-with-include-paths-with-vcpkg-without-cmake)
  * [How to embed SAIL as a subproject?](#how-to-embed-sail-as-a-subproject)
  * [Does SAIL support big\-endian platforms?](#does-sail-support-big-endian-platforms)
  * [I have questions, issues, or proposals](#i-have-questions-issues-or-proposals)

# SAIL Frequently Asked Questions (FAQ)

## How old is SAIL?

SAIL is a rebranded ksquirrel-libs rewritten in C, improved, and enhanced with high-level APIs. Ksquirrel-libs was a set of C++ image codecs
for the KSquirrel image viewer. See [http://ksquirrel.sourceforge.net](http://ksquirrel.sourceforge.net).

Technically, SAIL (ksquirrel-libs) was founded in 2003, making it one of the oldest image decoding libraries.

## Is SAIL cross-platform?

Yes. It's written in pure C11 and is highly portable. However, only Windows, macOS, and Linux platforms
are officially supported. SAIL may or may not compile on other platforms. Pull requests to support additional platforms are highly welcomed.

## What's the preferred way of installation?

- Windows: [Conan](https://conan.io/center/recipes/sail), `vcpkg`
- macOS: [Conan](https://conan.io/center/recipes/sail), `brew`, `vcpkg`
- Linux: native packages if available, [Conan](https://conan.io/center/recipes/sail), `vcpkg`

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

- `Junior`: Simple one-line image loading from file or memory
- `Advanced`: Loading animated/multi-paged images from file or memory
- `Deep diver`: Full control over codec selection, meta data, and loading options
- `Technical diver`: Everything above plus custom I/O sources

## Does SAIL provide simple one-line APIs?

Yes. Use the `junior` API level for simple one-line image loading.

## In what pixel format do SAIL loading functions output images?

SAIL attempts to output a pixel format as close to the source format as possible.
Ideally, it outputs the same pixel format as stored in the image.

For example, SAIL outputs BPP24-BGR images from full-color BMP files without transparency.

Consider using conversion functions from `libsail-manip` for format conversion.

## What pixel formats can SAIL write?

SAIL codecs support as many output pixel formats as possible. SAIL does not convert
pixel formats during save operations. Images are always written as-is.

The list of pixel formats that can be written by SAIL is codec-specific and is publicly available in every
.codec.info file. It can be accessed through `sail_codec_info_from_extension() -> codec_info -> save_features ->
pixel_formats`.

## Why does a codec support more pixel formats for writing than sail_convert can convert to?

SAIL codecs intentionally support more pixel formats for writing than `sail_convert()` can convert to.
This design allows you to write images with pixel formats that come from other sources (like other image
files, cameras, or custom renderers) without losing the original pixel format.

For example, a codec might support writing 32-bit float grayscale images. You can load such an image
from one file and save it to another file preserving the pixel format. However, `sail_convert()` might
not support converting from RGB to this specific format because such conversions are rare and complex.

```
    ┌─────────────────────────────────────────────────┐
    │  Pixel formats a codec can WRITE                │
    │  (e.g., BPP8, BPP16, BPP24, BPP32-FLOAT, etc.)  │
    │                                                 │
    │   ┌──────────────────────────────────────┐      │
    │   │ Pixel formats sail_convert()         │      │
    │   │ can convert to                       │      │
    │   │ (most common conversions)            │      │
    │   │                                      │      │
    │   │ [Common conversions like             │      │
    │   │  RGB → BGR, RGBA → RGB, etc.]        │      │
    │   └──────────────────────────────────────┘      │
    │                                                 │
    │   [Exotic formats that can be written as-is     │
    │    but not converted to: HDR, depth maps,       │
    │    32-bit float, uint, etc.]                    │
    └─────────────────────────────────────────────────┘
```

**Workflow example:**

1. Load a TIFF with 32-bit float grayscale (depth map from a camera)
2. Save it to PNG as-is → Not supported, PNG doesn't support this format
3. Save it to OpenEXR as-is → Supported, written without conversion
4. Convert with `sail_convert()` to BPP24-RGB → No such conversion
5. Write your own converter for your specific use case → Then save to any format

This approach provides maximum flexibility while keeping the conversion library focused on
the most common use cases.

## Does SAIL support animated and multi-paged images?

Yes. Continue loading the image file until the loading functions return `SAIL_OK`.
When no more frames are available, the loading functions return `SAIL_ERROR_NO_MORE_FRAMES`.

## Does SAIL support loading from memory?

Yes. SAIL supports loading and saving from files and memory. For technical divers,
custom I/O sources are also supported.

See `sail_start_loading_from_file()`, `sail_start_loading_mem()`, and `sail_start_loading_from_io()`.

## How does SAIL support image formats?

SAIL supports image formats through dynamically loaded codecs. End users never work
with codecs directly; they interact exclusively with the high-level APIs.

## Does SAIL preload codecs in the initialization routine?

### `SAIL_COMBINE_CODECS` is `OFF`

SAIL does not preload codecs in the initialization routine (`sail_init()`). It loads them on demand.
However, you can preload them explicitly with `sail_init_with_flags(SAIL_FLAG_PRELOAD_CODECS)`.

### `SAIL_COMBINE_CODECS` is `ON`

All codecs are loaded on application startup.

## How does SAIL look for codecs?

Codec path search algorithm (first found path wins):

### Conan recipe on any platform

Codecs are combined into a dynamically linked library, so no search is required.

### VCPKG port on any platform

Codecs are combined into a dynamically linked library, so no search is required.

### Manually compiled on any platform with SAIL_COMBINE_CODECS=ON

Codecs are combined into a dynamically linked library, so no search is required.

### Manually compiled on Windows with SAIL_COMBINE_CODECS=OFF (the default)
1. `SAIL_CODECS_PATH` environment variable
2. `<SAIL DEPLOYMENT FOLDER>\lib\sail\codecs`
3. Hardcoded `SAIL_CODECS_PATH` in config.h

### Manually compiled on Unix (including macOS) SAIL_COMBINE_CODECS=OFF (the default)

1. `SAIL_CODECS_PATH` environment variable
2. Hardcoded `SAIL_CODECS_PATH` in config.h

`<FOUND PATH>/lib` is added to `LD_LIBRARY_PATH`.

Additionally, if `SAIL_THIRD_PARTY_CODECS_PATH` is enabled in CMake (the default), the `SAIL_THIRD_PARTY_CODECS_PATH`
environment variable is searched for a list of ';'-separated paths containing third-party codecs.

## How can I point SAIL to my custom codecs?

If `SAIL_THIRD_PARTY_CODECS_PATH` is enabled in CMake (the default), set the `SAIL_THIRD_PARTY_CODECS_PATH` environment variable
to a list of ';'-separated paths containing your custom SAIL codecs.

On Windows, only the `sail.dll` location and `SAIL_THIRD_PARTY_CODECS_PATH/lib` are searched for codec DLL dependencies.
Use the WIN32 API `AddDllDirectory()` to add custom DLL dependency search paths.
On other platforms, `SAIL_THIRD_PARTY_CODECS_PATH/lib` is added to `LD_LIBRARY_PATH`.

If `SAIL_THIRD_PARTY_CODECS_PATH` is `OFF`, custom codec loading is disabled.

## I'd like to reorganize the standard SAIL folder layout on Windows

This is supported. However, with the standard layout, SAIL automatically detects the codec location.
If you reorganize the folder layout, you must specify the new codec location by
setting the `SAIL_CODECS_PATH` environment variable.

## Describe the memory management techniques implemented in SAIL

### The memory management technique implemented in SAIL

Internally, SAIL always cleans up on errors. If you encounter a memory leak on error, please report it.

**C only:** If an error occurs during loading or saving with the `advanced` or deeper API,
the developer must call `sail_stop_loading()` or `sail_stop_saving()` to perform proper cleanup
in the underlying codec. Failure to call these functions will result in memory leaks.

**C++ only:** The C++ binding automatically performs cleanup in `~image_input()` or `~image_output()` destructors.

### Convention to call SAIL functions

Use the `SAIL_TRY()` macro when calling SAIL functions. For cleanup on error,
use the `SAIL_TRY_OR_CLEANUP()` macro.

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

SAIL_TRY(sail_start_loading_from_file(..., &state));

/*
 * If sail_load_next_frame() fails, the image pointer stays untouched (NULL).
 * This code prints NULL value on error.
 */
SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, &image),
                    /* cleanup */ printf("%p\n", image),
                                  sail_stop_loading(state));
```

### Always set a pointer to state to NULL (C only)

C loading and saving functions require a local void pointer to state. Always initialize it to NULL before
loading or saving. For example:

```C
void *state = NULL;

SAIL_TRY(sail_start_loading_from_file(..., &state));

SAIL_TRY_OR_CLEANUP(sail_load_next_frame(state, ...),
                    /* cleanup */ sail_stop_loading(state));
```

## Can I implement an image codec in C++?

Yes. A codec must export a set of public functions for SAIL to recognize and use it.
Theoretically, codecs can be implemented in any programming language that supports C-compatible exports.

## Describe codec info file format

Let's take a hypothetical codec info:

```
# This section describes the codec per se.
#
[codec]

# Codec layout is a set of functions the codec exports. Different layout versions are not compatible.
# libsail supports a single (current) layout version. Cannot be empty.
#
layout=8

# Semantic codec version. Cannot be empty.
#
version=1.0.0

# Codec priority. SAIL uses this property to sort enumerated codecs by priority
# to speed up searches for popular image formats in functions like sail_codec_info_from_path().
#
# HIGHEST = Most popular image formats (e.g., JPEG)
# HIGH    = Popular and common image formats (e.g., SVG)
# MEDIUM  = Moderately popular formats
# LOW     = Rare image formats
# LOWEST  = Very rare, highly specific, or ancient image formats
#
priority=MEDIUM

# Short codec name. Must be uppercase letters and numbers only. Cannot be empty.
#
name=ABC

# Codec description. Any human-readable string. Cannot be empty.
#
description=Some ABC Format

# ';'-separated list of hex-encoded magic numbers identifying this image format.
# Can be empty only if the list of file extensions below is not empty.
#
magic-numbers=34 AB

# ';'-separated list of file extensions identifying this image format.
# Can be empty only if the list of magic numbers above is not empty.
#
extensions=abc;bca

# ';'-separated list of MIME types identifying this image format. Can be empty.
#
mime-types=image/abc

# Section of various features describing what the image codec can actually load.
#
[load-features]

# ';'-separated list of features the codec supports for reading.
# Can be empty if the codec cannot load images.
#
# Possible values:
#    STATIC       - Can load static images.
#    ANIMATED     - Can load animated images.
#    MULTI-PAGED  - Can load multi-paged (but not animated) images.
#    META-DATA    - Can load image metadata (e.g., JPEG comments, EXIF).
#    ICCP         - Can load embedded ICC profiles.
#    SOURCE-IMAGE - Can populate source image information in sail_image.source_image.
#
features=STATIC;META-DATA;INTERLACED;ICCP

# ';'-separated list of codec-specific tuning options. For example,
# disable ABC codec filtering by setting abc-filtering to 0 in load options.
# Tuning option names must start with the codec name to avoid conflicts.
#
# The list of possible values for each tuning option is not available programmatically.
# Each codec must document them in the codec info file.
#
# For example:
#   - abc-filtering: Tune filtering. Possible values: 0 (disable), 1 (light), 2 (hard).
#
tuning=abc-filtering

# Section describing features the codec supports for saving.
#
[save-features]

# ';'-separated list of features the codec supports for writing.
# Can be empty if the codec cannot save images.
#
# Possible values:
#    STATIC      - Can save static images.
#    ANIMATED    - Can save animated images.
#    MULTI-PAGED - Can save multi-paged (but not animated) images.
#    META-DATA   - Can save image metadata (e.g., JPEG comments, EXIF).
#    INTERLACED  - Can save interlaced images.
#    ICCP        - Can save embedded ICC profiles.
#
features=STATIC;META-DATA;INTERLACED;ICCP

# ';'-separated list of codec-specific tuning options. For example,
# disable ABC codec filtering by setting abc-filtering to 0 in save options.
# Tuning option names must start with the codec name to avoid conflicts.
#
# The list of possible values for each tuning option is not available programmatically.
# Each codec must document them in the codec info file.
#
# For example:
#   - abc-filtering: Tune filtering. Possible values: 0 (disable), 1 (light), 2 (hard).
#
tuning=abc-filtering

# ';'-separated list of pixel formats the codec can write.
# Can be empty if the codec cannot save images.
#
# Note: SAIL does not convert images during saving. Images are written as-is.
# This example codec accepts and saves 8-bit indexed, 24-bit RGB, and 32-bit RGBA images.
#
pixel-formats=BPP8-INDEXED;BPP24-RGB;BPP32-BGRA

# ';'-separated list of compressions the codec can write. Cannot be empty if the codec
# can save images. If the codec cannot select compressions, specify UNSUPPORTED.
#
compression-types=DEFLATE;RLE

# Default compression to use when no explicit compression is specified
# by the client application. Can be empty if the codec cannot save images.
#
default-compression=DEFLATE

# Minimum compression level. Not used and must be 0 if multiple compressions are supported
# or if the list is empty.
#
# Data type: double. Converted using C function atof().
# Decimal-point character is determined by the current C locale.
#
compression-level-min=1

# Maximum compression level. Not used and must be 0 if multiple compressions are supported
# or if the list is empty.
#
# Data type: double. Converted using C function atof().
# Decimal-point character is determined by the current C locale.
#
compression-level-max=9

# Default compression level to use when no explicit level is specified.
# Not used and must be 0 if multiple compressions are supported or if the list is empty.
# Must be within the min/max range above.
#
# Data type: double. Converted using C function atof().
# Decimal-point character is determined by the current C locale.
#
compression-level-default=6

# Increment/decrement step for compression level. Not used and must be 0 if
# multiple compressions are supported or if the list is empty.
#
# Data type: double. Converted using C function atof().
# Decimal-point character is determined by the current C locale.
#
compression-level-step=1
```

## Can I compile codec dependencies (like libjpeg) directly into SAIL?

Unlike FreeImage, SAIL does not support compiling external dependencies into a single library, and there are no plans to add this feature.

You can achieve this using external build systems like `vcpkg`.

If you prefer compiling SAIL manually as a shared library, compile it against a static vcpkg triplet such as `x64-windows-static`.
For example:

```
# Install vcpkg using the official installation guide into your preferred path.
# For example, into F:/vcpkg.

# Install SAIL in vcpkg just to install all its dependencies
# as static libs. We'll compile SAIL manually later.
cd F:/vcpkg/
vcpkg install sail[all] --triplet x64-windows-static

# Go to the cloned SAIL sources
cd F:/sail/

# Compile SAIL against vcpkg
mkdir build
cd build
cmake -DSAIL_COMBINE_CODECS=ON -DCMAKE_TOOLCHAIN_FILE=F:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static ..
cmake --build . --config Release
```

This way all codecs and their dependencies will be compiled into `sail-codecs.dll`.

## Are there any C/C++ examples?

Yes. See the `examples` directory in the source tree.

## Are there any bindings to other programming languages?

Yes. SAIL currently supports the following language bindings:

1. C++
2. Python

Pull requests to support additional programming languages are highly welcomed.

## How many image formats are you going to implement?

Ksquirrel-libs supported around 60 image formats. Not all of them will be ported. However,
the most popular image formats will be ported from ksquirrel-libs.

## I have problems with include paths with vcpkg without CMake

Add `VcpkgInstalledDir/include/sail` to the project include path.
See https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/msbuild-integration.

## How to embed SAIL as a subproject?

Embedding SAIL is not fully supported. SAIL modifies global CMake variables like `CMAKE_C_FLAGS`,
making embedding not recommended. The recommended approach is to build SAIL separately.

## Does SAIL support big-endian platforms?

No. SAIL currently supports little-endian platforms only.

## I have questions, issues, or proposals

Opening a GitHub [issue](https://github.com/HappySeaFox/sail/issues) is the preferred method
for communication and problem resolution.

Pull requests are always welcome.
