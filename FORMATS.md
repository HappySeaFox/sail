<table>
<thead>
    <td><b>N</b></td>
    <td><b>Image&nbsp;Format</b></td>
    <td><b>Can&nbsp;Load</b></td>
    <td><b>Cannot&nbsp;Load</b></td>
    <td><b>Can&nbsp;Save</b></td>
    <td><b>Cannot&nbsp;Save</b></td>
    <td><b>Dependencies</b></td>
</thead>
<tbody>
<tr>
    <td>1</td>
    <td><a href="https://wikipedia.org/wiki/APNG">APNG</a></td>
    <td>
        <b>Grayscale:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit.
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"apng-frames"</i>. Description: Number of frames in the animation.
        Possible values: unsigned int.
        Key: <i>"apng-plays"</i>. Description: Number of plays of the animation.
        Possible values: unsigned int.
        Key: <i>"apng-first-frame-hidden"</i>. Description: Whether the first frame is hidden.
        Possible values: bool.
    </td>
    <td>Blend operations with pixel formats other than BPP16-GRAYSCALE-ALPHA, BPP32-GRAYSCALE-ALPHA, BPP32-RGBA, BPP64-RGBA.</td>
    <td>
        <b>Grayscale:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>BGR:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>BGRA:</b> 32-bit, 64-bit.
        <b>ARGB:</b> 32-bit, 64-bit.
        <b>ABGR:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Compressions:</b> DEFLATE.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"png-filter"</i>. Description: PNG filter methods (comma-separated).
        Possible values: none, sub, up, avg, paeth.
        <br/>Key: <i>"apng-frames"</i>. Description: Number of frames to write in animation.
        Possible values: unsigned int (>1 enables APNG mode).
        <br/>Key: <i>"apng-plays"</i>. Description: Number of animation loops (0 = infinite).
        Possible values: unsigned int (default: 0).
    </td>
    <td>-</td>
    <td>libpng+APNG patch</td>
</tr>
<tr>
    <td>2</td>
    <td><a href="https://wikipedia.org/wiki/AV1#AV1_Image_File_Format_(AVIF)">AVIF</a></td>
    <td>
        <b>YUV:</b> 8-bit, 10-bit, 12-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"avif-threads"</i>. Description: Number of decoder threads.
        Possible values: positive int/unsigned int (default: 1).
    </td>
    <td>Thumbnails.</td>
    <td>
        <b>RGB:</b> 24-bit.
        <b>BGR:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <b>BGRA:</b> 32-bit.
        <b>ARGB:</b> 32-bit.
        <b>ABGR:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> AV1.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"avif-speed"</i>. Description: Encoding speed (0=slowest/best compression, 10=fastest).
        Possible values: 0-10 int/unsigned int (default: 6).
        <br/>Key: <i>"avif-threads"</i>. Description: Number of encoder threads.
        Possible values: positive int/unsigned int (default: 1).
        <br/>Key: <i>"avif-auto-tiling"</i>. Description: Enable automatic tiling for parallel encoding.
        Possible values: true or false.
    </td>
    <td>10-bit and 12-bit depths, Thumbnails.</td>
    <td>libavif</td>
</tr>
<tr>
    <td>3</td>
    <td><a href="https://wikipedia.org/wiki/BMP_file_format">BMP</a></td>
    <td>
        <b>Indexed:</b> 1-bit, 4-bit, 8-bit (DIB only).
        <b>RGB:</b> 16-bit, 24-bit, 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE4, RLE8.
        <br/><br/>
        <b>BMP Versions:</b> V1 (DDB), V2, V3, V4, V5.
        <br/><br/>
        <b>Content:</b> Static, Meta data, ICC profiles.
    </td>
    <td>
        <b>Indexed:</b> 8-bit (in DDB images).
        <br/><br/>
        <b>Compressions:</b> ALPHABITFIELDS, BITFIELDS, CMYK, CMYK-RLE4, CMYK-RLE8, JPEG, PNG.
        <br/><br/>
        <b>BMP Versions:</b> OS/2.
    </td>
    <td>
        <b>Indexed:</b> 1-bit, 4-bit, 8-bit.
        <b>Grayscale:</b> 8-bit.
        <b>RGB:</b> 16-bit, 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE (RLE4 for 4-bit, RLE8 for 8-bit).
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>
        <b>Compressions:</b> ALPHABITFIELDS, BITFIELDS, JPEG, PNG.
        <br/><br/>
        <b>Content:</b> Meta data, ICC profiles.
        <br/><br/>
        <b>BMP Versions:</b> V1 (DDB), V2, V4, V5.
    </td>
    <td>-</td>
</tr>
<tr>
    <td>4</td>
    <td><a href="http://www.compuphase.com/flic.htm">FLI</a></td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Compressions:</b> RLE (BRUN, LC, SS2).
        <br/><br/>
        <b>Content:</b> Static, Animated.
    </td>
    <td>Delta compression (LC, SS2), Postage stamp chunks.</td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Compressions:</b> RLE.
        <br/><br/>
        <b>Content:</b> Static, Animated.
        <br/><br/>
        <b>Note:</b> Both FLI (320x200) and FLC (any resolution) formats are supported.
        First frame uses BRUN compression, subsequent frames use COPY for simplicity.
        Delta compression for writing is not implemented.
    </td>
    <td>Delta compression (LC, SS2) for writing.</td>
    <td>-</td>
</tr>
<tr>
    <td>5</td>
    <td><a href="https://wikipedia.org/wiki/GIF">GIF</a></td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data.
    </td>
    <td>-</td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <br/><br/>
        <b>Compressions:</b> LZW.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data (comments only).
        <br/><br/>
        <b>Tuning<sup><a href="#star-tuning">[3]</a></sup>:</b>
             <i>"gif-transparency-index"</i>: Transparent color index in palette (-1 for no transparency). Values: int/unsigned int. Default: -1.
        <br/><i>"gif-loop-count"</i>: Number of animation loops (0 for infinite). Values: int/unsigned int. Default: 0.
        <br/><i>"gif-background-color"</i>: Background color index in palette. Values: int/unsigned int. Default: 0.
    </td>
    <td>-</td>
    <td>giflib</td>
</tr>
<tr>
    <td>6</td>
    <td><a href="https://wikipedia.org/wiki/RGBE_image_format">HDR</a></td>
    <td>
        <b>RGB:</b> 96-bit (32-bit float per channel).
        <br/><br/>
        <b>Compressions:</b> RLE.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"hdr-exposure"</i>. Description: Exposure value.
        Possible values: float.
        Key: <i>"hdr-gamma"</i>. Description: Gamma correction value.
        Possible values: float.
        Key: <i>"hdr-view"</i>. Description: View parameters.
        Possible values: string.
        Key: <i>"hdr-primaries"</i>. Description: Color primaries.
        Possible values: string.
        Key: <i>"hdr-color-correction-1"</i>, <i>"hdr-color-correction-2"</i>, <i>"hdr-color-correction-3"</i>. Description: Color correction per channel.
        Possible values: float.
    </td>
    <td>-</td>
    <td>
        <b>RGB:</b> 96-bit (32-bit float per channel).
        <br/><br/>
        <b>Compressions:</b> RLE.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Tuning<sup><a href="#star-tuning">[3]</a></sup>:</b>
             <i>"hdr-rle-compression"</i>: Enable/disable RLE compression. Values: 0 or 1. Default: 1.
        <br/><i>"hdr-y-direction"</i>: Y axis direction ("increasing"/"+" or "decreasing"/"-"). Values: string. Default: "decreasing".
        <br/><i>"hdr-x-direction"</i>: X axis direction ("increasing"/"+" or "decreasing"/"-"). Values: string. Default: "increasing".
        <br/><i>"hdr-exposure"</i>: Exposure value for header. Values: positive float/double. Default: 1.0.
        <br/><i>"hdr-gamma"</i>: Gamma correction value for header. Values: positive float/double. Default: 1.0.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>7</td>
    <td><a href="https://wikipedia.org/wiki/High_Efficiency_Image_File_Format">HEIF/HEIC</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>RGB:</b> 24-bit, 48-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>RGBA:</b> 32-bit, 64-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>YUV:</b> 24-bit, 30-bit<sup><a href="#star-underlying">[1]</a></sup>, 36-bit<sup><a href="#star-underlying">[1]</a></sup>, 48-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data (EXIF, XMP), ICC profiles.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"heif-has-depth"</i>. Description: Has depth map. Possible values: bool.
        Key: <i>"heif-depth-count"</i>. Description: Number of depth images. Possible values: int.
        Key: <i>"heif-thumbnail-count"</i>. Description: Number of thumbnails. Possible values: int.
        Key: <i>"heif-is-primary"</i>. Description: Is primary image. Possible values: bool.
        Key: <i>"heif-content-light-level-max"</i>. Description: Max content light level (HDR). Possible values: unsigned int.
        Key: <i>"heif-content-light-level-avg"</i>. Description: Average content light level (HDR). Possible values: unsigned int.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"heif-threads"</i>. Description: Number of decoder threads. Possible values: int (1-256, default: 1).
    </td>
    <td>Thumbnails.</td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>RGB:</b> 24-bit, 48-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>RGBA:</b> 32-bit, 64-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <b>YUV:</b> 24-bit, 30-bit<sup><a href="#star-underlying">[1]</a></sup>, 36-bit<sup><a href="#star-underlying">[1]</a></sup>, 48-bit<sup><a href="#star-underlying">[1]</a></sup>.
        <br/><br/>
        <b>Compressions:</b> HEVC.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data (EXIF, XMP), ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"heif-preset"</i>. Description: Encoding speed preset. Possible values: ultrafast, superfast, veryfast, faster, fast, medium (default), slow, slower, veryslow, placebo.
        <br/>Key: <i>"heif-tune"</i>. Description: Encoding optimization. Possible values: psnr, ssim, grain, fastdecode.
        <br/>Key: <i>"heif-tu-intra-depth"</i>. Description: Transform Unit intra depth. Possible values: int (1-4).
        <br/>Key: <i>"heif-complexity"</i>. Description: Encoding complexity. Possible values: int/unsigned int/float/double (0-100).
        <br/>Key: <i>"heif-chroma"</i>. Description: Chroma subsampling. Possible values: 420, 422, 444.
        <br/>Key: <i>"heif-threads"</i>. Description: Number of encoder threads. Possible values: int (1-256, default: 1).
    </td>
    <td>Thumbnails.</td>
    <td>libheif</td>
</tr>
<tr>
    <td>8</td>
    <td><a href="https://wikipedia.org/wiki/ICO_(file_format)">ICO and CUR</a></td>
    <td>
        <b>Bit depth:</b> Same to BMP.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"cur-hotspot-x"</i>. Description: X coordinate of the hotspot.
        Possible values: unsigned int.
        Key: <i>"cur-hotspot-y"</i>. Description: Y coordinate of the hotspot.
        Possible values: unsigned int.
    </td>
    <td>PNG contained images</td>
    <td>
        <b>Indexed:</b> 1-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit (BGR).
        <b>RGBA:</b> 32-bit (BGRA).
        <br/><br/>
        <b>Compressions:</b> NONE.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged.
        <br/><br/>
        <b>Note:</b> Maximum image size is 256x256 pixels.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"cur-hotspot-x"</i>, <i>"cur-hotspot-y"</i>. Description: CUR hotspot coordinates.
        Possible values: unsigned int.
    </td>
    <td>
        <b>Content:</b> PNG embedded images, transparency mask optimization.
    </td>
    <td>-</td>
</tr>
<tr>
    <td>9</td>
    <td><a href="https://wikipedia.org/wiki/JBIG">JBIG</a></td>
    <td>
        <b>Indexed:</b> 1-bit.
        <br/><br/>
        <b>Compressions:</b> JBIG.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>Multi-plane images.</td>
    <td>
        <b>Indexed:</b> 1-bit.
        <br/><br/>
        <b>Compressions:</b> JBIG.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jbig-stripe-height"</i>. Description: Number of lines per stripe for progressive encoding.
        Possible values: unsigned int (default: 0 = automatic).
        <br/>Key: <i>"jbig-typical-prediction"</i>. Description: Enable Typical Prediction for better compression.
        Possible values: 0 (disabled) or 1 (enabled).
    </td>
    <td>Multi-plane images.</td>
    <td>libjbig</td>
</tr>
<tr>
    <td>10</td>
    <td><a href="https://wikipedia.org/wiki/JPEG">JPEG</a></td>
    <td>
        <b>Grayscale:</b> 8-bit.
        <b>RGB:</b> 565 16-bit<sup><a href="#star-underlying">[1]</a></sup>, 24-bit.
        <b>BGR:</b><sup><a href="#star-underlying">[1]</a></sup> 24-bit.
        <b>YCbCr:</b> 24-bit.
        <b>RGBA:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>BGRA:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>ARGB:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>ABGR:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>CMYK:</b> 32-bit.
        <b>YCCK:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>
        <b>Grayscale:</b> 8-bit.
        <b>RGB:</b> 565 16-bit<sup><a href="#star-underlying">[1]</a></sup>, 24-bit.
        <b>BGR:</b><sup><a href="#star-underlying">[1]</a></sup> 24-bit.
        <b>YCbCr:</b> 24-bit.
        <b>RGBA:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>BGRA:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>ARGB:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>ABGR:</b><sup><a href="#star-underlying">[1]</a></sup> 32-bit.
        <b>CMYK:</b> 32-bit.
        <b>YCCK:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jpeg-dct-method"</i>. Description: JPEG DCT method.
        Possible values: "slow", "fast", "float".
        <br/>Key: <i>"jpeg-optimize-coding"</i>. Description: Compute optimal Huffman tables.
        Possible values: true or false.
        <br/>Key: <i>"jpeg-smoothing-factor"</i>. Description: Image smoothing (0-100).
        Possible values: int/unsigned int 0-100.
        <br/>See the libjpeg docs for more.
    </td>
    <td>-</td>
    <td>libjpeg or libjpeg-turbo</td>
</tr>
<tr>
    <td>11</td>
    <td><a href="https://wikipedia.org/wiki/JPEG_2000">JPEG 2000</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>YCbCr:</b> 24-bit.
        <b>CMYK:</b> 32-bit, 64-bit.
        <b>CMYKA:</b> 40-bit, 80-bit.
        <br/><br/>
        <b>Content:</b> Static, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jpeg2000-reduce"</i>. Description: Discard resolution levels (0=full resolution, 1=half, 2=quarter, etc.).
        Possible values: int/unsigned int.
        <br/>Key: <i>"jpeg2000-layer"</i>. Description: Maximum quality layers to decode (0=all).
        Possible values: int/unsigned int.
        <br/>Key: <i>"jpeg2000-tile-index"</i>. Description: Specific tile index to decode.
        Possible values: int/unsigned int.
        <br/>Key: <i>"jpeg2000-num-tiles"</i>. Description: Number of tiles to decode.
        Possible values: int/unsigned int.
    </td>
    <td>
        <b>Pixel formats:</b> LAB, XYZ, and other exotic color spaces.
        <br/><br/>
        <b>Content:</b> Meta data, images with non-zero position, images with bits per channel greater than 16.
    </td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>YCbCr:</b> 24-bit.
        <b>CMYK:</b> 32-bit, 64-bit.
        <b>CMYKA:</b> 40-bit, 80-bit.
        <br/><br/>
        <b>Content:</b> Static, ICC profiles (requires OpenJPEG 2.5.4+).
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jpeg2000-irreversible"</i>. Description: Use lossy DWT 9-7 instead of lossless 5-3.
        Possible values: bool.
        <br/>Key: <i>"jpeg2000-numresolution"</i>. Description: Number of resolution levels (1-32).
        Possible values: int/unsigned int.
        <br/>Key: <i>"jpeg2000-prog-order"</i>. Description: Progression order.
        Possible values: "lrcp", "rlcp", "rpcl", "pcrl", "cprl".
        <br/>Key: <i>"jpeg2000-codeblock-width"</i>. Description: Code block width (4-1024, power of 2).
        Possible values: int/unsigned int.
        <br/>Key: <i>"jpeg2000-codeblock-height"</i>. Description: Code block height (4-1024, power of 2).
        Possible values: int/unsigned int.
    </td>
    <td>
        <b>Content:</b> Meta data, ICC profiles (OpenJPEG &lt; 2.5.4 has encoding bug).
    </td>
    <td>openjpeg</td>
</tr>
<tr>
    <td>12</td>
    <td><a href="https://wikipedia.org/wiki/JPEG_XL">JPEG XL</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>CMYK:</b> 32-bit, 64-bit.
        <b>CMYKA:</b> 40-bit, 80-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Special properties:</b>
             Key: <i>"jpegxl-bits-per-sample"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-exponent-bits-per-sample"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-intensity-target"</i>. Possible values: float.
        <br/>Key: <i>"jpegxl-min-nits"</i>. Possible values: float.
        <br/>Key: <i>"jpegxl-relative-to-max-display"</i>. Possible values: bool.
        <br/>Key: <i>"jpegxl-linear-below"</i>. Possible values: float.
        <br/>Key: <i>"jpegxl-color-channels"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-extra-channels"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-alpha-bits"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-intrinsic-width"</i>. Possible values: unsigned int.
        <br/>Key: <i>"jpegxl-intrinsic-height"</i>. Possible values: unsigned int.
        <br/>See the <a href="https://libjxl.readthedocs.io/en/latest/api_metadata.html#_CPPv412JxlBasicInfo">JxlBasicInfo structure</a> documentation in libjxl for more.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jpegxl-desired-intensity-target"</i>. Description: Desired display intensity target. Possible values: float/double.
        <br/>Key: <i>"jpegxl-render-spotcolors"</i>. Description: Render spot colors. Possible values: bool.
    </td>
    <td>Wide color gamut data gets clipped.</td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"jpegxl-effort"</i>. Description: Encoder effort/speed (1=fastest, 9=slowest). Possible values: int/unsigned int 1-9. Default: 7.
        <br/>Key: <i>"jpegxl-decoding-speed"</i>. Description: Decoding speed tier (0=best quality, 4=fastest). Possible values: int/unsigned int 0-4. Default: 0.
        <br/>Key: <i>"jpegxl-modular"</i>. Description: Encoding mode (-1=auto, 0=VarDCT, 1=modular). Possible values: int/unsigned int -1, 0, 1. Default: -1.
        <br/>Key: <i>"jpegxl-progressive-ac"</i>. Description: Progressive AC mode. Possible values: int/unsigned int -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-progressive-dc"</i>. Description: Progressive DC mode. Possible values: int/unsigned int -1 (auto), 0 (off), 1, 2. Default: -1.
        <br/>Key: <i>"jpegxl-responsive"</i>. Description: Responsive mode for modular. Possible values: int/unsigned int -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-epf"</i>. Description: Edge Preserving Filter strength. Possible values: int/unsigned int -1 (auto), 0-3. Default: -1.
        <br/>Key: <i>"jpegxl-gaborish"</i>. Description: Gaborish filter. Possible values: int/unsigned int -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-photon-noise"</i>. Description: Film grain noise (0=none, 3200=high). Possible values: int/unsigned int 0+. Default: 0.
        <br/>Key: <i>"jpegxl-modular-predictor"</i>. Description: Predictor for modular. Possible values: int/unsigned int -1 (auto), 0-15. Default: -1.
        <br/>Key: <i>"jpegxl-palette-colors"</i>. Description: Use palette if colors ≤ N. Possible values: int/unsigned int -1 (auto), 0-1024. Default: -1.
        <br/>Key: <i>"jpegxl-resampling"</i>. Description: Downsampling factor. Possible values: int/unsigned int -1, 1, 2, 4, 8. Default: -1.
    </td>
    <td>
        <b>Pixel formats:</b> CMYK, CMYKA.
        <br/><br/>
        <b>Content:</b> Meta data.
    </td>
    <td>libjxl</td>
</tr>
<tr>
    <td>13</td>
    <td><a href="https://wikipedia.org/wiki/OpenEXR">OpenEXR</a></td>
    <td>
        <b>Grayscale:</b> 16-bit (half), 32-bit (float), 32-bit (uint).
        <b>Grayscale+Alpha:</b> 32-bit (half), 64-bit (float), 64-bit (uint).
        <b>RGB:</b> 48-bit (half), 96-bit (float), 96-bit (uint).
        <b>RGBA:</b> 64-bit (half), 128-bit (float), 128-bit (uint).
        <br/><br/>
        <b>Compressions:</b> NONE, RLE, ZIPS, ZIP, PIZ, PXR24, B44, B44A, DWAA, DWAB.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>Multipart, Deep images, Tiled images.</td>
    <td>
        <b>Grayscale:</b> 16-bit (half), 32-bit (float), 32-bit (uint).
        <b>Grayscale+Alpha:</b> 32-bit (half), 64-bit (float), 64-bit (uint).
        <b>RGB:</b> 48-bit (half), 96-bit (float), 96-bit (uint).
        <b>RGBA:</b> 64-bit (half), 128-bit (float), 128-bit (uint).
        <br/><br/>
        <b>Compressions:</b> NONE, RLE, ZIPS, ZIP, PIZ, PXR24, B44, B44A, DWAA, DWAB.
        <br/><br/>
        <b>Content:</b> Static, HDR.
        <br/><br/>
        <b>Note:</b> Initial implementation uses simplified RGBA API. Full support for all channels and pixel types planned.
    </td>
    <td>Multipart, Deep images, Tiled images.</td>
    <td>libopenexr-dev</td>
</tr>
<tr>
    <td>14</td>
    <td><a href="https://wikipedia.org/wiki/PCX">PCX</a></td>
    <td>
        <b>Indexed:</b> 1-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Compressions:</b> NONE<sup><a href="#star-pcx-rle">[2]</a></sup>, RLE.
    </td>
    <td>-</td>
    <td>
        <b>Indexed:</b> 1-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>15</td>
    <td><a href="https://wikipedia.org/wiki/Portable_Network_Graphics">PNG</a></td>
    <td>
        <b>Grayscale:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit.
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>
        <b>Grayscale:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit.
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"png-filter"</i>. Description: PNG filters to apply.
        Possible values: "none", "sub", "up", "avg", "paeth".
        It's also possible to combine filters with ';' like that: "none;sub;paeth".
        <br/>See the libpng docs for more.
    </td>
    <td>-</td>
    <td>libpng</td>
</tr>
<tr>
    <td>16</td>
    <td><a href="https://wikipedia.org/wiki/Portable_anymap">PNM</a></td>
    <td>
        <b>Indexed:</b> 1-bit.
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Formats:</b> PBM (P1/P4), PGM (P2/P5), PPM (P3/P6), PAM (P7).
        <br/><br/>
        <b>Special properties:</b> Key: <i>"pnm-ascii"</i>. Description: True if the image pixels are encoded in ASCII mode (P1-P3).
        Possible values: bool.
    </td>
    <td>
        <b>Indexed:</b> 1-bit (P4).
        <b>Grayscale:</b> 8-bit, 16-bit (P5).
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit (P7/PAM).
        <b>RGB:</b> 24-bit, 48-bit (P6).
        <b>RGBA:</b> 32-bit, 64-bit (P7/PAM).
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Compression:</b> None.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>17</td>
    <td><a href="https://wikipedia.org/wiki/Adobe_Photoshop#File_format">PSD</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Grayscale+Alpha:</b> 16-bit, 32-bit.
        <b>Indexed:</b> 1-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>CMYK:</b> 32-bit, 64-bit.
        <b>CMYKA:</b> 40-bit, 80-bit.
        <b>LAB:</b> 24-bit, 40-bit.
        <b>LABA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE.
        <br/><br/>
        <b>Content:</b> Static (Composite Image Only).
    </td>
    <td>
        <b>Grayscale:</b> 32-bit (float/HDR).
        <b>RGB:</b> 96-bit (float/HDR).
        <b>RGBA:</b> 128-bit (float/HDR).
        <br/><br/>
        <b>Color Modes:</b> Multichannel (mode 7), Duotone (mode 8).
        <br/><br/>
        <b>Compressions:</b> ZIP.
        <br/><br/>
        <b>Content:</b> Layers, masks, annotations, etc.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>18</td>
    <td><a href="http://qoiformat.org">QOI</a></td>
    <td>
        <b>RGB:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>Linear color space.</td>
    <td>
        <b>RGB:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>Linear color space.</td>
    <td>-</td>
</tr>
<tr>
    <td>19</td>
    <td><a href="https://wikipedia.org/wiki/Raw_image_format">RAW</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Supported formats:</b> 3FR, ARW, BAY, CR2, CR3, CRW, DCR, DNG, ERF, FFF, GPR, IIQ, K25, KDC, MEF, MOS, MRW, NEF, NRW, ORF, PEF, RAF, RAW, RW2, RWL, RWZ, SR2, SRF, SRW, X3F.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"raw-iso"</i>. Description: ISO speed. Possible values: float.
        Key: <i>"raw-shutter"</i>. Description: Shutter speed. Possible values: float.
        Key: <i>"raw-aperture"</i>. Description: Aperture value. Possible values: float.
        Key: <i>"raw-focal-length"</i>. Description: Focal length. Possible values: float.
        Key: <i>"raw-lens-id"</i>. Description: Lens ID. Possible values: unsigned long long.
        Key: <i>"raw-lens"</i>. Description: Lens name. Possible values: string.
        Key: <i>"raw-min-focal-length"</i>. Description: Minimum focal length. Possible values: float.
        Key: <i>"raw-max-focal-length"</i>. Description: Maximum focal length. Possible values: float.
        Key: <i>"raw-max-aperture-min-focal"</i>. Description: Maximum aperture at minimum focal length. Possible values: float.
        Key: <i>"raw-max-aperture-max-focal"</i>. Description: Maximum aperture at maximum focal length. Possible values: float.
        Key: <i>"raw-focal-length-in-35mm-format"</i>. Description: Focal length in 35mm format. Possible values: unsigned short.
        Key: <i>"raw-filters"</i>. Description: Bayer filter pattern. Possible values: unsigned int.
        Key: <i>"raw-colors"</i>. Description: Number of color channels. Possible values: int.
        Key: <i>"raw-width"</i>. Description: Raw image width. Possible values: unsigned short.
        Key: <i>"raw-height"</i>. Description: Raw image height. Possible values: unsigned short.
        Key: <i>"raw-top-margin"</i>. Description: Top margin. Possible values: unsigned short.
        Key: <i>"raw-left-margin"</i>. Description: Left margin. Possible values: unsigned short.
        Key: <i>"raw-is-foveon"</i>. Description: Is Foveon sensor. Possible values: bool.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"raw-brightness"</i>. Description: Brightness adjustment. Possible values: float/double.
        <br/>Key: <i>"raw-gamma"</i>. Description: Gamma curve. Possible values: "power", "bt709".
        <br/>Key: <i>"raw-highlight"</i>. Description: Highlight mode (0-9). Possible values: int/unsigned int 0-9.
        <br/>Key: <i>"raw-output-color"</i>. Description: Output color space. Possible values: string ("srgb", "adobe-rgb", "wide-gamut-rgb", "prophoto-rgb", "xyz", "aces", "rec2020").
        <br/>Key: <i>"raw-output-bits-per-sample"</i>. Description: Output bit depth. Possible values: int/unsigned int 8 or 16.
        <br/>Key: <i>"raw-demosaic"</i>. Description: Demosaicing algorithm. Possible values: string ("linear", "vng", "ppg", "ahd", "dcb", "dht", "aahd").
        <br/>Key: <i>"raw-four-color-rgb"</i>. Description: Use four-color RGB interpolation. Possible values: bool.
        <br/>Key: <i>"raw-dcb-iterations"</i>. Description: DCB iterations (0-100). Possible values: int/unsigned int 0-100.
        <br/>Key: <i>"raw-dcb-enhance-focal-length"</i>. Description: DCB enhance focal length (0-100). Possible values: int/unsigned int 0-100.
        <br/>Key: <i>"raw-use-camera-white-balance"</i>. Description: Use camera white balance. Possible values: bool.
        <br/>Key: <i>"raw-use-auto-white-balance"</i>. Description: Use auto white balance. Possible values: bool.
        <br/>Key: <i>"raw-user-multiplier"</i>. Description: User white balance multipliers. Possible values: string ("r g1 b g2").
        <br/>Key: <i>"raw-auto-brightness"</i>. Description: Auto brightness adjustment. Possible values: bool.
        <br/>Key: <i>"raw-half-size"</i>. Description: Output half-size image. Possible values: bool.
        <br/>Key: <i>"raw-use-fuji-rotate"</i>. Description: Use Fuji rotation. Possible values: bool.
        <br/>Key: <i>"raw-no-interpolation"</i>. Description: Disable interpolation. Possible values: bool.
        <br/>Key: <i>"raw-median-passes"</i>. Description: Median filter passes (0-100). Possible values: int/unsigned int 0-100.
    </td>
    <td><b>Content:</b> Multi-paged, Thumbnails.</td>
    <td>Unsupported</td>
    <td>-</td>
    <td>libraw</td>
</tr>
<tr>
    <td>20</td>
    <td><a href="https://wikipedia.org/wiki/Scalable_Vector_Graphics">SVG</a></td>
    <td>
        <b>Bit depth:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        See <a href="https://razrfalcon.github.io/resvg-test-suite/svg-support-table.html">resvg support table</a> when compiled with resvg.
    </td>
    <td>
        <b>Content:</b> Animated, Meta data, ICC profiles.
        <br/><br/>
        See <a href="https://razrfalcon.github.io/resvg-test-suite/svg-support-table.html">resvg support table</a> when compiled with resvg.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>resvg or nanosvg</td>
</tr>
<tr>
    <td>21</td>
    <td><a href="https://wikipedia.org/wiki/Truevision_TGA">TGA</a></td>
    <td>
        <b>Grayscale:</b> 8-bit.
        <b>Indexed:</b> 8-bit.
        <b>BGR:</b> 16-bit (BGR555), 24-bit.
        <b>BGRA:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>TGA Versions:</b> TGA 1.0, TGA 2.0 (with Extension Area).
        <br/><br/>
        <b>Special properties:</b> Key: <i>"tga-origin-x"</i>. Description: X coordinate of image origin.
        Possible values: unsigned short.
        Key: <i>"tga-origin-y"</i>. Description: Y coordinate of image origin.
        Possible values: unsigned short.
        Key: <i>"tga-alpha-bits"</i>. Description: Number of alpha channel bits (0-15).
        Possible values: unsigned char.
        Key: <i>"tga-flipped-h"</i>. Description: Image is flipped horizontally.
        Possible values: bool.
        Key: <i>"tga-flipped-v"</i>. Description: Image is flipped vertically.
        Possible values: bool.
        Key: <i>"tga-key-color"</i>. Description: Key color (ARGB, 32-bit).
        Possible values: unsigned int (only if not zero).
        Key: <i>"tga-pixel-aspect-ratio"</i>. Description: Pixel aspect ratio.
        Possible values: double.
        Key: <i>"tga-scan-line-offset"</i>. Description: Offset to scan line table.
        Possible values: unsigned int (only if not zero).
        Key: <i>"tga-attributes-type"</i>. Description: Attributes type byte indicating how to interpret alpha channel (0=no alpha, 1=undefined/ignore, 2=useful non-premultiplied, 3=useful premultiplied, 4=premultiplied non-linear).
        Possible values: unsigned char.
    </td>
    <td><b>Content:</b> Thumbnails.</td>
    <td>
        <b>Grayscale:</b> 8-bit.
        <b>Indexed:</b> 8-bit.
        <b>BGR:</b> 16-bit (BGR555), 24-bit.
        <b>BGRA:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Output format:</b> TGA 2.0 with Extension Area.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>22</td>
    <td><a href="https://wikipedia.org/wiki/Video_file_format">VIDEO</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <b>YUV:</b> 24-bit, 30-bit, 36-bit, 48-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Supported formats:</b> MP4, M4V, MOV, AVI, MKV, WEBM, FLV, WMV, MPG, MPEG, 3GP, OGV, TS, MTS, M2TS.
        <br/><br/>
        <b>Special properties:</b>
        Key: <i>"video-codec"</i>. Description: Video codec name. Possible values: string.
        Key: <i>"video-profile"</i>. Description: Codec profile name. Possible values: string.
        Key: <i>"video-level"</i>. Description: Codec level. Possible values: string.
        Key: <i>"video-bitrate"</i>. Description: Video bitrate in bits per second. Possible values: unsigned long.
        Key: <i>"video-framerate-num"</i>. Description: Frame rate numerator. Possible values: int.
        Key: <i>"video-framerate-den"</i>. Description: Frame rate denominator. Possible values: int.
        Key: <i>"video-width"</i>. Description: Video width in pixels. Possible values: int.
        Key: <i>"video-height"</i>. Description: Video height in pixels. Possible values: int.
        <br/><br/>
        <b>Tuning:</b>
        Key: <i>"video-threads"</i>. Description: Number of decoder threads. Possible values: int/unsigned int (default: 0 = auto).
        <br/>Key: <i>"video-low-resolution"</i>. Description: Decode at lower resolution for speed. Possible values: bool.
        <br/>Key: <i>"video-skip-frame"</i>. Description: Skip frame decoding. Possible values: int/unsigned int (0=none, 1=skip non-reference, 2=skip bidir, 4=skip non-keyframes, 8=skip all).
        <br/>Key: <i>"video-skip-idct"</i>. Description: Skip IDCT step. Possible values: int/unsigned int (0=none, 1=skip B-frames, 2=skip all).
        <br/>Key: <i>"video-skip-loop-filter"</i>. Description: Skip loop filtering. Possible values: int/unsigned int (0=none, 1=skip non-reference, 2=skip bidir, 4=skip non-keyframes, 8=skip all).
        <br/>Key: <i>"video-error-concealment"</i>. Description: Error concealment strategy. Possible values: int/unsigned int (bitmask: 1=frame, 2=partitions, 4=drift, 8=merge).
        <br/>Key: <i>"video-seek-time"</i>. Description: Extract frames at specific timestamps or time ranges (milliseconds). Possible values: string (semicolon-separated list of timestamps or ranges, e.g., "1000;2000;3000-5000").
    </td>
    <td>
        <b>Content:</b> Animated (only first frame extracted), Audio streams, Subtitles.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>FFmpeg (libavcodec, libavformat, libavutil, libswscale, libswresample)</td>
</tr>
<tr>
    <td>23</td>
    <td><a href="https://wikipedia.org/wiki/TIFF">TIFF</a></td>
    <td>
        <b>Bit depth:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit, 24-bit, 32-bit, 40-bit, 48-bit, 64-bit, 80-bit.
        <br/><br/>
        <b>Color spaces:</b> Grayscale, Indexed, RGB, RGBA, CMYK, CMYKA, YCbCr, CIELab.
        <br/><br/>
        <b>Compressions:</b><sup><a href="#star-underlying">[1]</a></sup> ADOBE-DEFLATE, CCITT-RLE, CCITT-RLEW, CCITT-T4, CCITT-T6, DCS, DEFLATE, IT-8BL, IT8-CTPAD, IT8-LW, IT8-MP, JBIG, JPEG, JPEG-2000, LERC, LZMA, LZW, NEXT, NONE, OJPEG, PACKBITS, PIXAR-FILM, PIXAR-LOG, SGI-LOG24, SGI-LOG, T43, T85, THUNDERSCAN, WEBP, ZSTD.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged, Meta data, ICC profiles.
    </td>
    <td>Tiled TIFFs, Planar configuration (PLANARCONFIG_SEPARATE), EXIF, Thumbnails./td>
    <td>
        <b>Grayscale:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit, 32-bit (float), 32-bit (uint).
        <br/>
        <b>Grayscale+Alpha:</b> 8-bit (4+4), 16-bit (8+8), 32-bit (16+16).
        <br/>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <br/>
        <b>RGB:</b> 24-bit (8×3), 48-bit (16×3).
        <br/>
        <b>RGBA:</b> 32-bit (8×4), 64-bit (16×4).
        <br/>
        <b>CMYK:</b> 32-bit (8×4), 64-bit (16×4).
        <br/>
        <b>CMYKA:</b> 40-bit (8×5), 80-bit (16×5).
        <br/>
        <b>YCbCr:</b> 24-bit (8×3).
        <br/>
        <b>CIELab:</b> 24-bit (8×3).
        <br/><br/>
        <b>Compressions:</b><sup><a href="#star-underlying">[1]</a></sup> ADOBE-DEFLATE, CCITT-RLE, CCITT-RLEW, CCITT-T4, CCITT-T6, DCS, DEFLATE, IT-8BL, IT8-CTPAD, IT8-LW, IT8-MP, JBIG, JPEG, JPEG-2000, LERC, LZMA, LZW, NEXT, NONE, OJPEG, PACKBITS, PIXAR-FILM, PIXAR-LOG, SGI-LOG24, SGI-LOG, T43, T85, THUNDERSCAN, WEBP, ZSTD.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning options:</b>
        <br/><i>tiff-predictor</i>: Prediction scheme for LZW/DEFLATE. Values: "none", "horizontal", "floating-point".
        <br/><i>tiff-jpeg-quality</i>: JPEG quality (1-100). Values: int/unsigned int. Default: 75.
        <br/><i>tiff-zip-quality</i>: ZIP/DEFLATE quality (1-9). Values: int/unsigned int. Default: 6.
    </td>
    <td>Tiled TIFFs, Planar configuration (PLANARCONFIG_SEPARATE), EXIF, Thumbnails.</td>
    <td>libtiff</td>
</tr>
<tr>
    <td>24</td>
    <td><a href="http://fileformats.archiveteam.org/wiki/Quake_2_Texture">WAL</a></td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged (4 mipmap levels).
    </td>
    <td>-</td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged (4 mipmap levels).
        <br/><br/>
        <b>Note:</b> Dimensions must be divisible by 8. If only one frame is provided, mipmaps are generated automatically.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>25</td>
    <td><a href="https://wikipedia.org/wiki/WebP">WEBP</a></td>
    <td>
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>
        <b>RGB:</b> 24-bit.
        <b>BGR:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <b>BGRA:</b> 32-bit.
        <b>ARGB:</b> 32-bit.
        <b>ABGR:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> WebP.
        <br/><br/>
        <b>Content:</b> Static, Animated.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"webp-lossless"</i>. Description: Lossless encoding (0=lossy, 1=lossless).
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-method"</i>. Description: Quality/speed trade-off (0=fast, 6=slower-better).
        Possible values: int/unsigned int 0-6 (default: 4).
        <br/>Key: <i>"webp-image-hint"</i>. Description: Image type hint.
        Possible values: "default", "picture", "photo", "graph" (default: "default").
        <br/>Key: <i>"webp-target-size"</i>. Description: Target size in bytes (0=disabled).
        Possible values: int/unsigned int (default: 0).
        <br/>Key: <i>"webp-target-psnr"</i>. Description: Minimal distortion to achieve (0=disabled).
        Possible values: float/double (default: 0).
        <br/>Key: <i>"webp-segments"</i>. Description: Maximum number of segments.
        Possible values: int/unsigned int 1-4 (default: 4).
        <br/>Key: <i>"webp-sns-strength"</i>. Description: Spatial Noise Shaping (0=off, 100=max).
        Possible values: int/unsigned int 0-100 (default: 50).
        <br/>Key: <i>"webp-filter-strength"</i>. Description: Filtering strength (0=off, 100=strongest).
        Possible values: int/unsigned int 0-100 (default: 60).
        <br/>Key: <i>"webp-filter-sharpness"</i>. Description: Filtering sharpness (0=off, 7=least sharp).
        Possible values: int/unsigned int 0-7 (default: 0).
        <br/>Key: <i>"webp-filter-type"</i>. Description: Filtering type.
        Possible values: "simple", "strong" (default: "strong").
        <br/>Key: <i>"webp-autofilter"</i>. Description: Auto adjust filter strength.
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-alpha-compression"</i>. Description: Alpha plane compression (0=none, 1=compressed).
        Possible values: int/unsigned int 0 or 1 (default: 1).
        <br/>Key: <i>"webp-alpha-filtering"</i>. Description: Alpha predictive filtering.
        Possible values: "none", "fast", "best" (default: "fast").
        <br/>Key: <i>"webp-alpha-quality"</i>. Description: Alpha quality (0=smallest, 100=lossless).
        Possible values: int/unsigned int 0-100 (default: 100).
        <br/>Key: <i>"webp-pass"</i>. Description: Number of entropy analysis passes.
        Possible values: int/unsigned int 1-10 (default: 1).
        <br/>Key: <i>"webp-preprocessing"</i>. Description: Preprocessing filter.
        Possible values: "none", "segment-smooth", "pseudo-random-dithering" (default: "none").
        <br/>Key: <i>"webp-partitions"</i>. Description: log2(number of token partitions).
        Possible values: int/unsigned int 0-3 (default: 0).
        <br/>Key: <i>"webp-partition-limit"</i>. Description: Quality degradation for 512k limit (0=none, 100=max).
        Possible values: int/unsigned int 0-100 (default: 0).
        <br/>Key: <i>"webp-emulate-jpeg-size"</i>. Description: Remap parameters to match JPEG size.
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-thread-level"</i>. Description: Use multi-threaded encoding.
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-low-memory"</i>. Description: Reduce memory usage (increases CPU).
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-near-lossless"</i>. Description: Near lossless encoding (0=max loss, 100=off).
        Possible values: int/unsigned int 0-100 (default: 100).
        <br/>Key: <i>"webp-exact"</i>. Description: Preserve exact RGB under transparent areas.
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-use-delta-palette"</i>. Description: Use delta palette (reserved for future).
        Possible values: int/unsigned int 0 or 1 (default: 0).
        <br/>Key: <i>"webp-use-sharp-yuv"</i>. Description: Use sharp RGB→YUV conversion.
        Possible values: int/unsigned int 0 or 1 (default: 0).
    </td>
    <td>-</td>
    <td>libwebp</td>
</tr>
<tr>
    <td>26</td>
    <td><a href="https://wikipedia.org/wiki/X_BitMap">XBM</a></td>
    <td>
        <b>Indexed:</b> 1-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Specifications:</b> X10, X11.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"xbm-version"</i>. Description: XBM format version detected during load (X10 or X11).
        Possible values: "X10", "X11".
    </td>
    <td>
        <b>Content:</b> Multi-paged, C-style /*...*/ comments.
    </td>
    <td>
        <b>Indexed:</b> 1-bit.
        <br/><br/>
        <b>Compressions:</b> NONE.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"xbm-version"</i>. Description: XBM format version. Possible values: "X10", "X11" (default: "X11").
        Key: <i>"xbm-name"</i>. Description: Variable name for the C array. Possible values: any valid C identifier (default: "image").
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>27</td>
    <td><a href="https://wikipedia.org/wiki/X_PixMap">XPM</a></td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"xpm-hotspot-x"</i>. Description: X coordinate of hotspot.
        Possible values: int (default: -1).
        Key: <i>"xpm-hotspot-y"</i>. Description: Y coordinate of hotspot.
        Possible values: int (default: -1).
    </td>
    <td>-</td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <br/><br/>
        <b>Compressions:</b> NONE.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"xpm-name"</i>. Description: Variable name for the C array. Possible values: any valid C identifier (default: "image").
    </td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>28</td>
    <td><a href="https://wikipedia.org/wiki/Xwd">XWD</a></td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 16-bit (555, 565), 24-bit.
        <b>BGR:</b> 16-bit (555, 565), 24-bit.
        <b>RGBA:</b> 32-bit.
        <b>BGRA:</b> 32-bit.
        <b>ARGB:</b> 32-bit.
        <b>ABGR:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Specifications:</b> X11.
    </td>
    <td>XYPixmap and XYBitmap formats (only ZPixmap is supported).</td>
    <td>
        <b>Indexed:</b> 1-bit, 2-bit, 4-bit, 8-bit.
        <b>RGB:</b> 16-bit (555, 565), 24-bit.
        <b>BGR:</b> 16-bit (555, 565), 24-bit.
        <b>RGBA:</b> 32-bit.
        <b>BGRA:</b> 32-bit.
        <b>ARGB:</b> 32-bit.
        <b>ABGR:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b> NONE.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>-</td>
    <td>-</td>
</tr>
</tbody>
</table>

## References

1. <a name="star-underlying"></a> If supported by the underlying codec like libjpeg.
1. <a name="star-pcx-rle"></a> Even though uncompressed PCX files are not considered valid by the spec.
