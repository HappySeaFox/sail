# sailpy - Python bindings for SAIL

Fast and easy-to-use image decoding library for Python.

## Features

- **Fast** - Written in C/C++ for maximum performance
- **20+ image formats** - JPEG, PNG, WebP, AVIF, TIFF, GIF, BMP, and more
- **Format conversion** - Easy pixel format conversion
- **Metadata support** - Access EXIF, ICC profiles, and more
- **NumPy integration** - Zero-copy access to pixel data
- **Clean API** - Simple and intuitive interface

## Installation

### From PyPI (Recommended)

Pre-built wheels with all dependencies included:

```bash
pip install sailpy
```

### From Source

If building from source, you'll need system libraries installed first.

### Linux (Debian/Ubuntu)

**Minimal installation (JPEG, PNG, basic formats):**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libjpeg-dev \
    libpng-dev \
    libgif-dev \
    zlib1g-dev
pip install sailpy
```

**Full installation (all codecs):**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libwebp-dev \
    libavif-dev \
    libheif-dev \
    libgif-dev \
    libjxl-dev \
    libopenjp2-7-dev \
    libopenexr-dev \
    zlib1g-dev
pip install sailpy
```

### macOS (Homebrew)

```bash
brew install \
    cmake \
    jpeg \
    libpng \
    libtiff \
    webp \
    libavif \
    libheif \
    giflib \
    jpeg-xl \
    openjpeg \
    openexr \
    zlib
pip install sailpy
```

## Quick Start

```python
import sailpy
import numpy as np

# Load an image
image = sailpy.load_from_file("photo.jpg")
print(f"Size: {image.width}x{image.height}")

# Convert to NumPy array (zero-copy!)
pixels = image.to_numpy()

# Process with NumPy
inverted = 255 - pixels

# Save result
new_image = sailpy.Image.from_numpy(inverted, image.pixel_format)
new_image.save("output.png")
```

## Supported Formats

| Format | Read | Write | Features |
|--------|------|-------|----------|
| AVIF | ✅ | ✅ | Modern efficient format |
| BMP | ✅ | ✅ | Windows Bitmap |
| GIF | ✅ | ✅ | Animations |
| JPEG | ✅ | ✅ | Lossy compression |
| JPEG 2000 | ✅ | ✅ | Advanced JPEG |
| JPEG XL | ✅ | ✅ | Next-gen format |
| PNG | ✅ | ✅ | Lossless with alpha |
| PSD | ✅ | ❌ | Photoshop files |
| QOI | ✅ | ✅ | Quite OK Image |
| SVG | ✅ | ❌ | Vector graphics |
| TGA | ✅ | ✅ | Targa |
| TIFF | ✅ | ✅ | Multi-page support |
| WebP | ✅ | ✅ | Google's format |

...and more! See [full list](https://sail.software/formats.html).

## Examples

The package includes comprehensive examples demonstrating various features:

```python
# View all available examples
import sailpy.examples
print(sailpy.examples.__doc__)

# Run examples
python -m sailpy.examples.01_quickstart
python -m sailpy.examples.12_image_viewer  # Qt-based image viewer
```

**Available examples:**
- `01_quickstart.py` - Basic image loading and saving
- `02_memory_io.py` - Working with memory buffers
- `03_features_and_options.py` - Codec features and save options
- `04_numpy_integration.py` - NumPy array integration
- `05_multiframe.py` - Multi-frame image handling
- `06_probe.py` - Image probing without full loading
- `07_codec_info.py` - Querying codec information
- `08_logging.py` - Logging configuration
- `09_image_transformations.py` - Image transformations
- `10_enum_usage.py` - Working with enums
- `11_advanced_saving.py` - Advanced saving options
- `12_image_viewer.py` - Simple Qt-based image viewer

### Basic Loading and Saving

```python
import sailpy

# Load
image = sailpy.load_from_file("input.jpg")
print(f"Format: {image.pixel_format}")
print(f"Size: {image.width}x{image.height}")

# Save in different format
image.save("output.png")
```

### Format Conversion

```python
import sailpy

image = sailpy.load_from_file("input.jpg")

# Convert to RGB
image.convert(sailpy.PixelFormat.BPP24_RGB)

# Convert with options
options = sailpy.ConversionOptions()
options.background = True
image.convert(sailpy.PixelFormat.BPP32_RGBA, options)

image.save("output.png")
```

### NumPy Integration

```python
import sailpy
import numpy as np

# Load image
image = sailpy.load_from_file("photo.jpg")

# Get NumPy array (zero-copy when possible!)
pixels = image.to_numpy()
print(f"Array shape: {pixels.shape}")

# Apply filters
brightened = np.clip(pixels * 1.2, 0, 255).astype(np.uint8)

# Create new image from array
result = sailpy.Image.from_numpy(brightened, image.pixel_format)
result.save("brightened.png")
```

### Batch Conversion

```python
import sailpy
from pathlib import Path

input_dir = Path("images")
output_dir = Path("converted")
output_dir.mkdir(exist_ok=True)

for img_path in input_dir.glob("*.jpg"):
    image = sailpy.load_from_file(str(img_path))
    image.convert(sailpy.PixelFormat.BPP24_RGB)
    image.save(str(output_dir / f"{img_path.stem}.png"))
```

### Image Information

```python
import sailpy

image = sailpy.load_from_file("photo.jpg")

print(f"Dimensions: {image.width}x{image.height}")
print(f"Pixel format: {image.pixel_format}")
print(f"Bits per pixel: {image.bits_per_pixel}")
print(f"Is RGB: {image.is_rgb_family()}")
print(f"Is grayscale: {image.is_grayscale()}")
```

## API Reference

### Core Classes

#### `Image`

Main class representing an image with pixel data.

**Constructors:**
```python
Image()  # Create invalid image
Image(path: str)  # Load from file
Image(pixel_format: PixelFormat, width: int, height: int)  # Create empty
```

**Properties:**
- `width: int` - Image width in pixels
- `height: int` - Image height in pixels
- `pixel_format: PixelFormat` - Pixel format
- `bits_per_pixel: int` - Bits per pixel
- `bytes_per_line: int` - Bytes per scan line
- `pixels_size: int` - Total pixel data size
- `gamma: float` - Gamma value
- `delay: int` - Animation frame delay (milliseconds)
- `resolution: Resolution` - Image resolution
- `source_image: SourceImage` - Source image information
- `meta_data: MetaData` - Image metadata (EXIF, etc.)
- `iccp: Iccp` - ICC profile
- `palette: Palette` - Color palette (for indexed images)

**Methods:**
- `is_valid() -> bool` - Check if image is valid
- `is_rgb_family() -> bool` - Check if RGB-like format
- `is_grayscale() -> bool` - Check if grayscale
- `is_indexed() -> bool` - Check if indexed (palette-based)
- `load(path: str)` - Load from file
- `save(path: str, options: WriteOptions = None)` - Save to file
- `convert(pixel_format: PixelFormat)` - Convert pixel format (in-place)
- `convert(save_features: SaveFeatures)` - Convert for saving
- `to_numpy() -> np.ndarray` - Get NumPy array view (zero-copy when possible)
- `from_numpy(array: np.ndarray, pixel_format: PixelFormat) -> Image` - Create from NumPy array

#### `ImageReader`

Low-level interface for reading images with more control.

```python
reader = ImageReader(path: str)
frames = reader.read_all()  # Read all frames
frame = reader.next_frame()  # Read next frame
reader.stop()  # Stop reading
```

#### `ImageWriter`

Low-level interface for writing images with more control.

```python
writer = ImageWriter(path: str)
writer.write(image: Image, options: WriteOptions = None)
writer.finish()
```

#### `CodecInfo`

Information about image format codecs.

**Methods:**
- `from_extension(ext: str) -> CodecInfo` - Get codec by extension
- `from_path(path: str) -> CodecInfo` - Get codec from file path
- `from_mime_type(mime: str) -> CodecInfo` - Get codec by MIME type
- `list() -> List[CodecInfo]` - Get all available codecs

**Properties:**
- `name: str` - Codec name
- `description: str` - Codec description
- `version: str` - Codec version
- `extensions: List[str]` - Supported file extensions
- `mime_types: List[str]` - Supported MIME types
- `can_load: bool` - Can read images
- `can_save: bool` - Can write images
- `save_features: SaveFeatures` - Supported save features

### Enums

#### `PixelFormat`

Pixel format enumeration.

**RGB formats:**
- `BPP24_RGB`, `BPP24_BGR` - 24-bit RGB/BGR
- `BPP32_RGBA`, `BPP32_BGRA` - 32-bit RGB/BGR with alpha
- `BPP48_RGB`, `BPP48_BGR` - 48-bit RGB/BGR
- `BPP64_RGBA`, `BPP64_BGRA` - 64-bit RGB/BGR with alpha

**Grayscale:**
- `BPP8_GRAYSCALE`, `BPP16_GRAYSCALE` - 8/16-bit grayscale
- `BPP8_GRAYSCALE_ALPHA`, `BPP16_GRAYSCALE_ALPHA` - With alpha

**Indexed:**
- `BPP1_INDEXED`, `BPP2_INDEXED`, `BPP4_INDEXED`, `BPP8_INDEXED`

#### `Compression`

Compression types for saving images.

Values: `NONE`, `RLE`, `JPEG`, `DEFLATE`, `LZW`, `ZIP`, `ZSTD`, etc.

#### `LogLevel`

Logging levels.

Values: `SILENCE`, `ERROR`, `WARNING`, `INFO`, `DEBUG`, `TRACE`

### Support Classes

#### `WriteOptions`

Options for saving images.

```python
options = WriteOptions()
options.compression = Compression.DEFLATE
options.compression_level = 6
options.interlaced = True
```

#### `Resolution`

Image resolution information.

```python
resolution = Resolution()
resolution.x = 300
resolution.y = 300
resolution.unit = ResolutionUnit.INCH
```

#### `MetaData`

Image metadata container.

```python
meta = image.meta_data
entries = meta.to_dict()  # Get all metadata
value = meta.get("Author")  # Get specific entry
meta.set("Author", "John Doe")  # Set entry
```

### Functions

#### `load_from_file(path: str) -> Image`

Convenience function to load an image.

#### `save_into_file(image: Image, path: str)`

Convenience function to save an image.

#### `version() -> str`

Get SAIL library version.

#### `version_tuple() -> tuple`

Get version as tuple (major, minor, patch).

#### `set_log_barrier(level: LogLevel)`

Set minimum logging level.

```python
sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)
```

### Example Modules

The `sailpy.examples` package contains working examples:

```python
import sailpy.examples
print(sailpy.examples.__path__)  # Find examples location
```

## Performance

SAIL is designed for performance:

- Zero-copy NumPy integration when possible
- Efficient C/C++ implementation
- Optimized codecs
- Minimal memory allocations

See [benchmarks](https://github.com/HappySeaFox/sail/blob/master/BENCHMARKS.md) comparing SAIL to other libraries.

## Requirements

- Python 3.8+
- NumPy 1.20+

## Building from Source

```bash
git clone https://github.com/HappySeaFox/sail.git
cd sail/src/bindings/python
pip install -e .
```

## License

MIT License - see [LICENSE](https://github.com/HappySeaFox/sail/blob/master/LICENSE.txt) for details.

## Links

- **Homepage:** https://github.com/HappySeaFox/sail
- **Documentation:** https://github.com/HappySeaFox/sail/wiki
- **Issues:** https://github.com/HappySeaFox/sail/issues
- **PyPI:** https://pypi.org/project/sailpy/

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

