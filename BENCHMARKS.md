Table of Contents
=================

* [Conditions](#conditions)
* [Results](#results)
  * [JPEG Gray](#jpeg-gray)
  * [JPEG YCbCr](#jpeg-ycbcr)
  * [PNG Gray](#png-gray)
  * [PNG RGBA](#png-rgba)

## Conditions

| Condition                      | Value                |
| ------------------------------ | -------------------- |
| **Date**                       | Oct 27, 2020         |
| **Operating System**           | Windows 7 x64        |
| **Compiler**                   | MSVC 2019 x64        |
| **Libraries Under Test (LUT)** | [Boost.GIL](https://www.boost.org/doc/libs/1_68_0/libs/gil/doc/html/index.html), [CImg](https://github.com/dtschump/CImg), [DevIL](http://openil.sourceforge.net), [FreeImage](https://freeimage.sourceforge.io), [OpenImageIO](https://github.com/OpenImageIO/oiio.git), [SAIL](https://github.com/smoked-herring/sail), [SDL_Image](https://www.libsdl.org/projects/SDL_image), [stb_image](https://github.com/nothings/stb), [WIC](https://docs.microsoft.com/en-us/windows/win32/wic/-wic-about-windows-imaging-codec) |
| **LUT Installation Mode**      | vcpkg                |
| **Benchmarks**                 | [sail-benchmark](https://github.com/smoked-herring/sail-benchmark) |
| **Measurement Unit**           | Microseconds         |
| **Output Pixels**              | RGBA8888 or similar  |
| **Hardware**                   | Lenovo Thinkpad W540 |
| **CPU**                        | Intel Core i7-4800MQ |
| **Power Cable**                | Connected            |
| **Power Plan**                 | Maximum Performance  |

## Results

### JPEG Gray
![JPEG-Gray-100x67](.github/benchmarks/JPEG-Gray-100x67.png)
![JPEG-Gray-1000x669](.github/benchmarks/JPEG-Gray-1000x669.png)
![JPEG-Gray-6000x4016](.github/benchmarks/JPEG-Gray-6000x4016.png)
![JPEG-Gray-15000x10040](.github/benchmarks/JPEG-Gray-15000x10040.png)

### JPEG YCbCr
![JPEG-YCbCr-100x67](.github/benchmarks/JPEG-YCbCr-100x67.png)
![JPEG-YCbCr-1000x669](.github/benchmarks/JPEG-YCbCr-1000x669.png)
![JPEG-YCbCr-6000x4016](.github/benchmarks/JPEG-YCbCr-6000x4016.png)
![JPEG-YCbCr-15000x10040](.github/benchmarks/JPEG-YCbCr-15000x10040.png)

### PNG Gray
![PNG-Gray-100x71](.github/benchmarks/PNG-Gray-100x71.png)
![PNG-Gray-1000x709](.github/benchmarks/PNG-Gray-1000x709.png)
![PNG-Gray-6000x4256](.github/benchmarks/PNG-Gray-6000x4256.png)
![PNG-Gray-15000x10640](.github/benchmarks/PNG-Gray-15000x10640.png)

### PNG RGBA
![PNG-RGBA-100x71](.github/benchmarks/PNG-RGBA-100x71.png)
![PNG-RGBA-1000x709](.github/benchmarks/PNG-RGBA-1000x709.png)
![PNG-RGBA-6000x4256](.github/benchmarks/PNG-RGBA-6000x4256.png)
![PNG-RGBA-15000x10640](.github/benchmarks/PNG-RGBA-15000x10640.png)
