# sailpy - Python bindings for SAIL

The missing small and fast image decoding library for humans (not for machines).

## Features

- **Fast** - Written in C/C++ for maximum performance
- **20+ image formats** - JPEG, PNG, WebP, AVIF, TIFF, GIF, BMP, and more
- **Format conversion** - Easy pixel format conversion
- **Metadata support** - Access EXIF, ICC profiles, and more
- **NumPy integration** - Zero-copy access to pixel data
- **Clean API** - Simple and intuitive interface

## Installation

Pre-built wheels with all dependencies included:

```bash
pip install sailpy
```

## Quick Start

```python
import sailpy
import numpy as np

# Load an image
image = sailpy.Image.from_file("photo.jpg")
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

| Format | Load | Save | Features |
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
```

Run examples from command line:
```bash
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
image = sailpy.Image.from_file("input.jpg")
print(f"Format: {image.pixel_format}")
print(f"Size: {image.width}x{image.height}")

# Save in different format
image.save("output.png")
```

### Format Conversion

```python
import sailpy

image = sailpy.Image.from_file("input.jpg")

# Convert to RGBA
image.convert(sailpy.PixelFormat.BPP32_RGBA)

image.save("output.png")
```

### NumPy Integration

```python
import sailpy
import numpy as np

# Load image
image = sailpy.Image.from_file("photo.jpg")

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
    image = sailpy.Image.from_file(str(img_path))
    image.convert(sailpy.PixelFormat.BPP24_RGB)
    image.save(str(output_dir / f"{img_path.stem}.png"))
```

### Image Information

```python
import sailpy

image = sailpy.Image.from_file("photo.jpg")

print(f"Dimensions: {image.width}x{image.height}")
print(f"Pixel format: {image.pixel_format}")
print(f"Bits per pixel: {image.bits_per_pixel}")
print(f"Is RGB: {image.is_rgb_family}")
print(f"Is grayscale: {image.is_grayscale}")
```

### Creating Images from Scratch

```python
import sailpy
import numpy as np

# Create a new image
image = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 640, 480)

# Fill with color using NumPy
pixels = image.to_numpy()
pixels[:] = [0, 128, 255]  # Blue color

# Or create gradient
pixels[:, :, 0] = np.linspace(0, 255, 640)  # Red gradient

image.save("created.png")
```

### Error Handling

```python
import sailpy

# Error handling with specific exceptions
try:
    image = sailpy.Image.from_file("image.xyz")
except FileNotFoundError:
    print("File not found")
except ValueError as e:
    print(f"Unsupported format or invalid parameters: {e}")
except Exception as e:
    print(f"Error loading image: {e}")

# Check if format is supported before loading
try:
    codec = sailpy.CodecInfo.from_path("image.xyz")
    if not codec.can_load:
        print(f"Format {codec.name} cannot load images")
    else:
        image = sailpy.Image.from_file("image.xyz")
except ValueError:
    print("Format not supported")
except FileNotFoundError:
    print("File not found")
```

### Multi-Frame Images (Animations)

```python
import sailpy

# Load animated GIF or multi-page TIFF
input = sailpy.ImageInput("animation.gif")
frames = input.load_all()

print(f"Total frames: {len(frames)}")

# Process each frame
for i, frame in enumerate(frames):
    print(f"Frame {i}: {frame.width}x{frame.height}, delay={frame.delay}ms")
    # Extract individual frames
    frame.save(f"frame_{i:03d}.png")

# Or iterate frame by frame (memory efficient)
input = sailpy.ImageInput("animation.gif")
for i, frame in enumerate(input):
    print(f"Processing frame {i}")
    # Process frame...

# Load with custom options
options = sailpy.LoadOptions()
input = sailpy.ImageInput("image.tiff").with_options(options)
frames = input.load_all()
```

### Working with Memory (Bytes)

```python
import sailpy
import requests

# Load from HTTP
response = requests.get("https://example.com/image.jpg")
image = sailpy.Image.from_bytes(response.content)

# Save to memory
image_bytes = image.to_bytes("png")
print(f"PNG size in memory: {len(image_bytes)} bytes")

# Send over network, save to database, etc.
```

### Image Transformations

```python
import sailpy

image = sailpy.Image.from_file("photo.jpg")

# Rotate (creates new image)
rotated = image.rotate_to(sailpy.Orientation.ROTATED_90)
rotated.save("rotated_90.jpg")

# Mirror (in-place)
image.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)
image.save("mirrored.jpg")

# Rotate in-place
image.rotate(sailpy.Orientation.ROTATED_180)
image.save("rotated_180.jpg")

# Apply EXIF orientation
if image.source_image and image.source_image.orientation:
    corrected = image.rotate_to(image.source_image.orientation)
    corrected.save("corrected.jpg")
```

### Advanced: Frame-by-Frame Writing

```python
import sailpy

# Save multi-page TIFF (memory efficient for large datasets)
output = sailpy.ImageOutput("document.tiff")

# Optional: Set save options
options = sailpy.SaveOptions()
options.compression = sailpy.Compression.DEFLATE
options.compression_level = 6
output = output.with_options(options)

# Save pages one by one
for page_num in range(10):
    # Create or load page
    page = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 1024, 768)
    # ... fill page data ...

    output.save(page)

output.finish()
print("Multi-page document created")
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

**Properties:**
- `is_valid: bool` - Check if image is valid
- `is_rgb_family: bool` - Check if RGB-like format
- `is_grayscale: bool` - Check if grayscale
- `is_indexed: bool` - Check if indexed (palette-based)

**Methods:**
- `from_file(path: str) -> Image` - Load from file (static)
- `load(path: str)` - Load from file (instance method)
- `save(path: str, options: SaveOptions = None)` - Save to file
- `convert(pixel_format: PixelFormat)` - Convert pixel format (in-place)
- `convert_to(pixel_format: PixelFormat, options: ConversionOptions = None) -> Image` - Convert to new image
- `rotate(orientation: Orientation)` - Rotate image (in-place)
- `rotate_to(orientation: Orientation) -> Image` - Rotate to new image
- `mirror(orientation: Orientation)` - Mirror image (in-place)
- `to_numpy() -> np.ndarray` - Get NumPy array view (zero-copy when possible)
- `from_numpy(array: np.ndarray, pixel_format: PixelFormat) -> Image` - Create from NumPy array (static)

#### `ImageInput`

Low-level interface for loading images with more control.

```python
# Basic usage
input = ImageInput(path: str)
frames = input.load_all()  # Load all frames
frame = input.next_frame()  # Load next frame
input.stop()  # Stop loading

# With custom options (builder pattern)
options = LoadOptions()
input = ImageInput("image.tiff").with_options(options)
frames = input.load_all()

# Force specific codec (for files with wrong/missing extension)
codec = CodecInfo.from_name("png")
input = ImageInput("image.dat").with_codec(codec)
```

#### `ImageOutput`

Low-level interface for saving images with more control.

```python
# Basic usage
output = ImageOutput(path: str)
output.save(image: Image)
output.finish()

# With save options (builder pattern)
options = SaveOptions()
options.compression = Compression.JPEG
output = ImageOutput("output.tiff").with_options(options)
output.save(image)
output.finish()

# Force specific codec (for files with custom extension)
codec = CodecInfo.from_name("tiff")
output = ImageOutput("output.dat").with_codec(codec)
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
- `magic_numbers: List[bytes]` - Magic number patterns
- `can_load: bool` - Can load images
- `can_save: bool` - Can save images
- `is_valid: bool` - Check if codec info is valid
- `load_features: LoadFeatures` - Supported load features
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

#### `SaveOptions`

Options for saving images.

```python
options = SaveOptions()
options.compression = Compression.DEFLATE
options.compression_level = 6

# Codec-specific tuning (dict[str, Variant])
# IMPORTANT: Set the whole dict, not individual keys
options.tuning = {"png-filter": Variant("paeth")}  # PNG: none, sub, up, avg, paeth

# Use with ImageOutput
output = ImageOutput("output.png").with_options(options)
output.save(image)
output.finish()
```

#### `LoadOptions`

Options for loading images.

```python
options = LoadOptions()
# Most codecs don't require load options
# LoadOptions.tuning is available for codec-specific settings if needed

# Use with ImageInput
input = ImageInput("image.tiff").with_options(options)
frames = input.load_all()
```

#### `Resolution`

Image resolution information.

```python
# Create and set resolution for an image
resolution = Resolution()
resolution.x = 300
resolution.y = 300
resolution.unit = ResolutionUnit.INCH

# Assign via property
image.resolution = resolution

# Access existing resolution
print(f"DPI: {image.resolution.x}x{image.resolution.y} {image.resolution.unit}")
```

#### `MetaData`

Image metadata container.

```python
# Load metadata from loaded image
image = Image.from_file("photo.jpg")
meta = image.meta_data
if meta:
    print(f"Key: {meta.key()}")
    value = meta.value  # Property, not method
    if value.has_string():
        print(f"Value: {value.to_string()}")

# Create and set metadata
new_meta = MetaData()
new_meta.set_key(MetaDataType.ARTIST)

variant = Variant()
variant.set_string("John Doe")
new_meta.set_value(variant)

# Add to image
image.set_meta_data([new_meta])
```

### Functions

#### `set_log_barrier(level: LogLevel)`

Set minimum logging level.

```python
sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)
```

#### `set_logger(callback: Callable)`

Set custom logging callback.

```python
def my_logger(level, message):
    print(f"[SAIL {level}] {message}")

sailpy.set_logger(my_logger)
```


### Version Information

```python
import sailpy

# Get version string
print(sailpy.__version__)  # e.g., "0.9.10"
```

### Example Modules

The `sailpy.examples` package contains working examples:

```python
# Find examples location
import sailpy.examples
print(sailpy.examples.__path__)

# Run programmatically
from sailpy.examples import example_01_quickstart
# example_01_quickstart.main()  # If example has main()
```

## Performance

SAIL is designed for performance:

- Zero-copy NumPy integration when possible
- Efficient C/C++ implementation
- Optimized codecs
- Minimal memory allocations

See [benchmarks](https://github.com/HappySeaFox/sail/blob/master/BENCHMARKS.md) comparing SAIL to other libraries.

## Requirements

- Python 3.9+
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

- **Homepage:** https://sail.software
- **GitHub:** https://github.com/HappySeaFox/sail
- **PyPI:** https://pypi.org/project/sailpy/

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

