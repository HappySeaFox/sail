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
    </td>
    <td>Blend operations with pixel formats other than BPP16-GRAYSCALE-ALPHA, BPP32-GRAYSCALE-ALPHA, BPP32-RGBA, BPP64-RGBA.</td>
    <td>Unsupported</td>
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
        Possible values: positive integer (default: 1).
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
        <b>Compressions:</b> AV1.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
        <br/><br/>
        <b>Tuning:</b> Key: <i>"avif-speed"</i>. Description: Encoding speed (0=slowest/best compression, 10=fastest).
        Possible values: 0-10 (default: 6).
        <br/>Key: <i>"avif-threads"</i>. Description: Number of encoder threads.
        Possible values: positive integer (default: 1).
        <br/>Key: <i>"avif-auto-tiling"</i>. Description: Enable automatic tiling for parallel encoding.
        Possible values: true or false.
    </td>
    <td>10-bit and 12-bit depths</td>
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
             <i>"gif-transparency-index"</i>: Transparent color index in palette (-1 for no transparency). Default: -1.
        <br/><i>"gif-loop-count"</i>: Number of animation loops (0 for infinite). Default: 0.
        <br/><i>"gif-background-color"</i>: Background color index in palette. Default: 0.
    </td>
    <td>-</td>
    <td>giflib</td>
</tr>
<tr>
    <td>5</td>
    <td><a href="https://en.wikipedia.org/wiki/ICO_(file_format)">ICO and CUR</a></td>
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
    <td>6</td>
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
        <br/>Key: <i>"jpeg-smoothing-factor"</i>. Description: Smooth the image.
        Possible values: Unsigned int range from 1U to 100U.
        <br/>See the libjpeg docs for more.
    </td>
    <td>-</td>
    <td>libjpeg or libjpeg-turbo</td>
</tr>
<tr>
    <td>7</td>
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
        Possible values: unsigned int.
        <br/>Key: <i>"jpeg2000-layer"</i>. Description: Maximum quality layers to decode (0=all).
        Possible values: unsigned int.
        <br/>Key: <i>"jpeg2000-tile-index"</i>. Description: Specific tile index to decode.
        Possible values: unsigned int.
        <br/>Key: <i>"jpeg2000-num-tiles"</i>. Description: Number of tiles to decode.
        Possible values: unsigned int.
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
        Possible values: int.
        <br/>Key: <i>"jpeg2000-prog-order"</i>. Description: Progression order.
        Possible values: "lrcp", "rlcp", "rpcl", "pcrl", "cprl".
        <br/>Key: <i>"jpeg2000-codeblock-width"</i>. Description: Code block width (4-1024, power of 2).
        Possible values: int.
        <br/>Key: <i>"jpeg2000-codeblock-height"</i>. Description: Code block height (4-1024, power of 2).
        Possible values: int.
    </td>
    <td>
        <b>Content:</b> Meta data, ICC profiles (OpenJPEG &lt; 2.5.4 has encoding bug).
    </td>
    <td>openjpeg</td>
</tr>
<tr>
    <td>8</td>
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
        <b>Tuning:</b> Key: <i>"jpegxl-effort"</i>. Description: Encoder effort/speed (1=fastest, 9=slowest). Possible values: 1-9. Default: 7.
        <br/>Key: <i>"jpegxl-decoding-speed"</i>. Description: Decoding speed tier (0=best quality, 4=fastest). Possible values: 0-4. Default: 0.
        <br/>Key: <i>"jpegxl-modular"</i>. Description: Encoding mode (-1=auto, 0=VarDCT, 1=modular). Possible values: -1, 0, 1. Default: -1.
        <br/>Key: <i>"jpegxl-progressive-ac"</i>. Description: Progressive AC mode. Possible values: -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-progressive-dc"</i>. Description: Progressive DC mode. Possible values: -1 (auto), 0 (off), 1, 2. Default: -1.
        <br/>Key: <i>"jpegxl-responsive"</i>. Description: Responsive mode for modular. Possible values: -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-epf"</i>. Description: Edge Preserving Filter strength. Possible values: -1 (auto), 0-3. Default: -1.
        <br/>Key: <i>"jpegxl-gaborish"</i>. Description: Gaborish filter. Possible values: -1 (auto), 0 (off), 1 (on). Default: -1.
        <br/>Key: <i>"jpegxl-photon-noise"</i>. Description: Film grain noise (0=none, 3200=high). Possible values: 0+. Default: 0.
        <br/>Key: <i>"jpegxl-modular-predictor"</i>. Description: Predictor for modular. Possible values: -1 (auto), 0-15. Default: -1.
        <br/>Key: <i>"jpegxl-palette-colors"</i>. Description: Use palette if colors ≤ N. Possible values: -1 (auto), 0-1024. Default: -1.
        <br/>Key: <i>"jpegxl-resampling"</i>. Description: Downsampling factor. Possible values: -1, 1, 2, 4, 8. Default: -1.
    </td>
    <td>
        <b>Pixel formats:</b> CMYK, CMYKA.
        <br/><br/>
        <b>Content:</b> Meta data.
    </td>
    <td>libjxl</td>
</tr>
<tr>
    <td>9</td>
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
    <td>10</td>
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
    <td>11</td>
    <td><a href="https://wikipedia.org/wiki/Portable_anymap">PNM</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Indexed:</b> 1-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
        <br/><br/>
        <b>Special properties:</b> Key: <i>"pnm-ascii"</i>. Description: True if the image pixels are encoded in ASCII mode.
        Possible values: bool.
    </td>
    <td>-</td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>12</td>
    <td><a href="https://en.wikipedia.org/wiki/Adobe_Photoshop#File_format">PSD</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>Indexed:</b> 1-bit, 8-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Compressions:</b> NONE, RLE.
        <br/><br/>
        <b>Content:</b> Static (Preview Image Only).
    </td>
    <td>
        <b>Grayscale:</b> 32-bit.
        <b>RGB:</b> 96-bit.
        <b>RGBA:</b> 128-bit.
        <b>Pixel formats:</b> Multichannel, Duotone, LAB.
        <br/><br/>
        <b>Compressions:</b> ZIP.
        <br/><br/>
        <b>Content:</b> Layers, masks, etc.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>13</td>
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
    <td>14</td>
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
    <td>15</td>
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
    </td>
    <td><b>Content:</b> Thumbnail images.</td>
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
    <td>16</td>
    <td><a href="https://wikipedia.org/wiki/TIFF">TIFF</a></td>
    <td>
        <b>Bit depth:</b> 1-bit, 2-bit, 4-bit, 8-bit, 16-bit, 24-bit, 32-bit, 48-bit, 64-bit.
        <br/><br/>
        <b>Compressions:</b><sup><a href="#star-underlying">[1]</a></sup> ADOBE-DEFLATE, CCITT-RLE, CCITT-RLEW, CCITT-T4, CCITT-T6, DCS, DEFLATE, IT-8BL, IT8-CTPAD, IT8-LW, IT8-MP, JBIG, JPEG, JPEG-2000, LERC, LZMA, LZW, NEXT, NONE, OJPEG, PACKBITS, PIXAR-FILM, PIXAR-LOG, SGI-LOG24, SGI-LOG, T43, T85, THUNDERSCAN, WEBP, ZSTD.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Compressions:</b><sup><a href="#star-underlying">[1]</a></sup> ADOBE-DEFLATE, CCITT-RLE, CCITT-RLEW, CCITT-T4, CCITT-T6, DCS, DEFLATE, IT-8BL, IT8-CTPAD, IT8-LW, IT8-MP, JBIG, JPEG, JPEG-2000, LERC, LZMA, LZW, NEXT, NONE, OJPEG, PACKBITS, PIXAR-FILM, PIXAR-LOG, SGI-LOG24, SGI-LOG, T43, T85, THUNDERSCAN, WEBP, ZSTD.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>libtiff</td>
</tr>
<tr>
    <td>17</td>
    <td><a href="http://fileformats.archiveteam.org/wiki/Quake_2_Texture">WAL</a></td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Content:</b> Static, Multi-paged.
    </td>
    <td>-</td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>18</td>
    <td><a href="https://wikipedia.org/wiki/WebP">WEBP</a></td>
    <td>
        <b>Bit depth:</b> 24-bit, 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data, ICC profiles.
    </td>
    <td>-</td>
    <td>Unsupported</td>
    <td>-</td>
    <td>libwebp</td>
</tr>
<tr>
    <td>19</td>
    <td><a href="https://en.wikipedia.org/wiki/X_BitMap">XBM</a></td>
    <td>
        <b>Bit depth:</b> 1-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        <b>Specifications:</b> X10, X11.
    </td>
    <td>
        <b>Content:</b> Multi-paged, C-style /*...*/ comments.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
</tbody>
</table>

## References

1. <a name="star-underlying"></a> If supported by the underlying codec like libjpeg.
1. <a name="star-pcx-rle"></a> Even though uncompressed PCX files are not considered valid by the spec.
