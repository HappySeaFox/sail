# SAIL Python Bindings - Examples

Practical examples demonstrating sailpy features, organized by category.

## Installation

These examples are included with the `sailpy` package:

```bash
pip install sailpy
```

## Running Examples

After installation, run examples using Python's module syntax:

```bash
# Run specific example
python -m sailpy.examples.01_quickstart
python -m sailpy.examples.12_image_viewer

# Find examples location
python -c "import sailpy.examples; print(sailpy.examples.__path__)"
```

Or run from source:

```bash
cd examples
python 01_quickstart.py
```

## Examples (in recommended order)

### 01_quickstart.py - Getting Started
**Duration:** < 1 minute  
**Best for:** First-time users

The absolute minimum to load and save images:
```python
import sailpy

# Create
img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 256, 256)
img.save("output.png")

# Load
loaded = sailpy.Image.from_file("output.png")
```

**Learn:** Basic I/O, simplest API usage

---

### 02_memory_io.py - Working Without Files
**Duration:** 1-2 minutes  
**Best for:** API developers, microservices

Load/save images in memory (bytes):
```python
# Image to bytes
png_bytes = img.to_bytes("png")

# Bytes to image
loaded = sailpy.Image.from_bytes(png_bytes)
```

**Real-world use cases:**
- HTTP API responses/requests
- Database storage (BLOB fields)
- Microservices communication
- Format conversion without temporary files
- Thumbnail generation for APIs

**6 examples:** to_bytes, roundtrip, API simulation, format conversion, thumbnails, batch processing

---

###  03_features_and_options.py - Codec Features & Correct Defaults
**Duration:** 2-3 minutes  
**Best for:** Advanced users, quality-focused apps

**IMPORTANT:** Always get defaults from codec features!

```python
# ✓ CORRECT WAY:
codec = sailpy.CodecInfo.from_name("PNG")
options = codec.save_features.to_options()  # Get defaults!

# Check valid compression range
cl = codec.save_features.compression_level
print(f"Valid range: {cl.min_level} - {cl.max_level}")

# Customize within range
options.compression_level = cl.max_level

# Save
writer = sailpy.ImageWriter("output.png")
writer.with_options(options).write(img)
writer.finish()
```

**✗ WRONG WAY:**
```python
options = sailpy.SaveOptions()
options.compression_level = 9.0  # May be invalid!
```

**7 examples:**
- Codec features discovery
- Default options from features
- Custom compression (min/max/default)
- Option flags (META_DATA, ICCP)
- Codec-specific compressions
- Pixel format support checking
- Source image properties

---

### 04_numpy_integration.py - NumPy for Image Processing
**Duration:** 1-2 minutes  
**Best for:** Data scientists, ML engineers, image processing

Zero-copy NumPy integration:
```python
img = sailpy.Image.from_file("photo.jpg")
arr = img.to_numpy()  # Zero-copy!

# Process with NumPy
arr[:] = arr // 2  # Darken (modifies original!)

# Works with 16-bit too
img16 = sailpy.Image(sailpy.PixelFormat.BPP48_RGB, 128, 128)
arr16 = img16.to_numpy()  # dtype=uint16!
```

**5 examples:**
- Zero-copy access
- uint16 support (16-bit images)
- Image processing operations
- Creating image from NumPy array
- Slicing and channel operations

---

### 05_multiframe.py - Animation Handling
**Duration:** < 1 minute  
**Best for:** Working with GIF, WebP, multi-page TIFF

Read and write multi-frame images:
```python
# Read all frames (iterator)
for frame in sailpy.ImageReader("animation.gif"):
    process(frame)

# Write multiple frames
with sailpy.ImageWriter("output.gif") as writer:
    writer.write(frame1)
    writer.write(frame2)
```

**2 examples:** Frame reading, frame writing

---

### 06_probe.py - Efficient Metadata Extraction
**Duration:** < 1 minute  
**Best for:** Getting image info without loading pixels

Probe metadata instantly (no pixel loading):
```python
# Probe WITHOUT loading pixels (fast!)
metadata = sailpy.ImageReader.probe("huge_image.tif")
print(f"{metadata['width']}x{metadata['height']}")  # Instant!

# Performance: probe vs full load comparison
```

**Demonstrates:** probe() speed advantage, source image properties

---

### 07_codec_info.py - Codec Discovery
**Duration:** < 1 minute  
**Best for:** Discovering available codecs

```python
# List all codecs
codecs = sailpy.list_codecs()

# Find codec by extension
codec = sailpy.get_codec_info("jpg")
```

**Learn:** Available codecs, codec features

---

### 08_logging.py - Custom Logging
**Duration:** < 1 minute  
**Best for:** Debugging, production monitoring

```python
# Set log level
sailpy.set_log_barrier(sailpy.LogLevel.WARNING)

# Custom logger
def my_logger(level, file, line, msg):
    print(f"[{level}] {msg}")

sailpy.set_logger(my_logger)
```

**Learn:** Log levels, custom logging callbacks

---

### 09_image_transformations.py - Image Transformations
**Duration:** < 1 minute (requires input image)
**Best for:** Image processing, format conversion

```python
# Rotate 90° (returns new image)
rotated = img.rotate_to(sailpy.Orientation.ROTATED_90)

# Mirror in-place
img.mirror(sailpy.Orientation.MIRRORED_HORIZONTALLY)

# Convert pixel format
grayscale = img.convert_to(sailpy.PixelFormat.BPP8_GRAYSCALE)

# Set properties
img.resolution = sailpy.Resolution(sailpy.ResolutionUnit.INCH, 300, 300)
```

**4 operations:** Rotation, mirroring, conversion, properties

---

### 10_enum_usage.py - Enum Usage (Python Way)
**Duration:** < 1 minute  
**Best for:** Understanding enum API

```python
# Standard Python enum
pf = sailpy.PixelFormat.BPP24_RGB
pf.name           # "BPP24_RGB"
pf.value          # 31
str(pf)           # "PixelFormat.BPP24_RGB"

# Parse from SAIL format
PixelFormat.from_string("BPP24-RGB")
```

**Covers:** All 4 enum types with examples

---

### 11_advanced_saving.py - Advanced Save Options
**Duration:** 1-2 minutes  
**Best for:** Fine-tuning image output quality

```python
# Get codec-specific options
codec = sailpy.CodecInfo.from_path("output.webp")
options = codec.save_features.to_options()

# Customize compression
options.compression = sailpy.Compression.WEBP_LOSSY
options.compression_level = 80.0

# Set properties
options.io_options = sailpy.IoOption.WRITE_META_DATA | sailpy.IoOption.WRITE_ICCP

# Write with options
writer = sailpy.ImageWriter("output.webp")
writer.write(img, options)
writer.finish()
```

**Learn:** Advanced save options, codec-specific features, quality tuning

---

### 12_image_viewer.py - Qt-Based Image Viewer
**Duration:** Interactive  
**Best for:** Understanding SAIL in real applications

A complete Qt-based image viewer with:
- Multi-format support (all SAIL formats)
- Animation playback (GIF, WebP, etc.)
- Zoom and pan controls
- Frame navigation
- Save to different formats
- Auto-fit for large images

**Requirements:**
```bash
pip install PySide6
```

**Run:**
```bash
python -m sailpy.examples.12_image_viewer
```

**Controls:**
- Ctrl+O to open image
- Ctrl+S to save
- Mouse wheel to zoom
- Arrow keys for frame navigation

**Learn:** Real-world SAIL integration, UI handling, multi-frame playback

---

## Key Concepts & Best Practices

### 1. Always Get Defaults from Features

**✓ DO THIS:**
```python
codec = sailpy.CodecInfo.from_name("PNG")
options = codec.save_features.to_options()
# Now options have CORRECT defaults for this codec
```

**✗ DON'T DO THIS:**
```python
options = sailpy.SaveOptions()
# Defaults may be invalid for specific codec!
```

### 2. Check Compression Level Validity

```python
cl = codec.save_features.compression_level
if cl.is_valid:
    options.compression_level = cl.max_level  # Safe!
```

### 3. Use Supported Compressions

```python
supported = codec.save_features.compressions
for comp in supported:
    options.compression = comp  # Always valid
```

### 4. Use Memory I/O When Possible

```python
# Instead of:
img.save("/tmp/temp.png")
with open("/tmp/temp.png", "rb") as f:
    data = f.read()
os.remove("/tmp/temp.png")

# Do this:
data = img.to_bytes("png")  # No temp file!
```

### 5. Probe Before Loading Large Images

```python
# Instead of:
img = sailpy.Image.from_file("huge.tif")  # Loads all pixels!
width = img.width

# Do this:
metadata = sailpy.ImageReader.probe("huge.tif")  # Fast!
width = metadata['width']
```

---

## Example File Structure

| File | Category | Lines |
|------|----------|-------|
| 01_quickstart.py | Getting Started | ~70 |
| 02_memory_io.py | Memory I/O | ~250 |
| 03_features_and_options.py | Features API | ~335 |
| 04_numpy_integration.py | NumPy | ~125 |
| 05_multiframe.py | Multi-frame | ~85 |
| 06_probe.py | Metadata Probe | ~80 |
| 07_codec_info.py | Codec Discovery | ~95 |
| 08_logging.py | Logging | ~115 |
| 09_image_transformations.py | Transformations | ~75 |
| 10_enum_usage.py | Enum Usage | ~135 |
| 11_advanced_saving.py | Advanced Saving | ~120 |
| 12_image_viewer.py | Qt Image Viewer | ~350 |

**Total:** 12 focused examples (~1,835 lines)

---

## Common Patterns

### Pattern 1: Simple Load -> Process -> Save
```python
img = sailpy.Image.from_file("input.jpg")
arr = img.to_numpy()
arr[:] = arr // 2  # Darken
img.save("output.jpg")
```

### Pattern 2: Memory I/O (API Response)
```python
# Receive bytes from API
image_bytes = requests.get(url).content
img = sailpy.Image.from_file(image_bytes)

# Process and return bytes
processed = img.to_bytes("webp")
return Response(processed, mimetype="image/webp")
```

### Pattern 3: Custom Compression
```python
codec = sailpy.CodecInfo.from_name("PNG")
options = codec.save_features.to_options()
options.compression_level = codec.save_features.compression_level.max_level

writer = sailpy.ImageWriter("output.png")
writer.with_options(options).write(img)
writer.finish()
```

---

## Tips

1. **Start with 01_quickstart.py** - easiest introduction
2. **02_memory_io.py for APIs** - if building web services
3. **03_features_and_options.py is critical** - learn defaults!
4. **04_numpy_integration.py for processing** - if using NumPy
5. **Try 12_image_viewer.py** - see SAIL in a real application
6. **Run examples in order** - they build on each other (01-11)
7. **Examples are installed with sailpy** - run with `python -m sailpy.examples.XX`

---

## Need Help?

- Main README: `../README.md`
- Test files: `../tests/` for more patterns
- Documentation: https://sail.software
