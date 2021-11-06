Table of Contents
=================

* [Conditions](#conditions)
* [Results](#results)
  * [JPEG Gray](#jpeg-gray)
  * [JPEG YCbCr](#jpeg-ycbcr)
  * [PNG Gray](#png-gray)
  * [PNG RGBA](#png-rgba)

## Conditions

| Condition                               | Value                |
| --------------------------------------- | -------------------- |
| **Operating System**                    | Windows 7 x64        |
| **Compiler**                            | MSVC 2019 x64        |
| **Libraries Under Test (LUT)**          | [Boost.GIL](https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/index.html), [CImg](https://github.com/dtschump/CImg), [DevIL](http://openil.sourceforge.net), [FreeImage](https://freeimage.sourceforge.io), [OpenImageIO](https://github.com/OpenImageIO/oiio.git), [SAIL](https://github.com/smoked-herring/sail), [SDL_Image](https://www.libsdl.org/projects/SDL_image), [stb_image](https://github.com/nothings/stb), [WIC](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec) |
| **LUT Installation Mode (except SAIL)** | vcpkg                |
| **SAIL Installation Mode**              | vcpkg                |
| **Benchmarks**                          | [sail-benchmark](https://github.com/smoked-herring/sail-benchmark) |
| **Measured Time**                       | Real (wall clock)    |
| **Measurement Units**                   | Microseconds         |
| **Output Pixels**                       | Default              |
| **Hardware**                            | Lenovo Thinkpad W540 |
| **CPU**                                 | Intel Core i7-4800MQ |
| **Power Cable**                         | Connected            |
| **Power Plan**                          | Maximum Performance  |

## Results

### JPEG Gray
<img alt="JPEG-Gray-100x67" src=".github/benchmarks/JPEG-Gray-100x67.png" width="500px" />
<img alt="JPEG-Gray-1000x669" src=".github/benchmarks/JPEG-Gray-1000x669.png" width="500px" />
<img alt="JPEG-Gray-6000x4016" src=".github/benchmarks/JPEG-Gray-6000x4016.png" width="500px" />
<img alt="JPEG-Gray-15000x10040" src=".github/benchmarks/JPEG-Gray-15000x10040.png" width="500px" />

### JPEG YCbCr
<img alt="JPEG-YCbCr-100x67" src=".github/benchmarks/JPEG-YCbCr-100x67.png" width="500px" />
<img alt="JPEG-YCbCr-1000x669" src=".github/benchmarks/JPEG-YCbCr-1000x669.png" width="500px" />
<img alt="JPEG-YCbCr-6000x4016" src=".github/benchmarks/JPEG-YCbCr-6000x4016.png" width="500px" />
<img alt="JPEG-YCbCr-15000x10040" src=".github/benchmarks/JPEG-YCbCr-15000x10040.png" width="500px" />

### PNG Gray
<img alt="PNG-Gray-100x71" src=".github/benchmarks/PNG-Gray-100x71.png" width="500px" />
<img alt="PNG-Gray-1000x709" src=".github/benchmarks/PNG-Gray-1000x709.png" width="500px" />
<img alt="PNG-Gray-6000x4256" src=".github/benchmarks/PNG-Gray-6000x4256.png" width="500px" />
<img alt="PNG-Gray-15000x10640" src=".github/benchmarks/PNG-Gray-15000x10640.png" width="500px" />

### PNG RGBA
<img alt="PNG-RGBA-100x71" src=".github/benchmarks/PNG-RGBA-100x71.png" width="500px" />
<img alt="PNG-RGBA-1000x709" src=".github/benchmarks/PNG-RGBA-1000x709.png" width="500px" />
<img alt="PNG-RGBA-6000x4256" src=".github/benchmarks/PNG-RGBA-6000x4256.png" width="500px" />
<img alt="PNG-RGBA-15000x10640" src=".github/benchmarks/PNG-RGBA-15000x10640.png" width="500px" />
