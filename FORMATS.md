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
    </td>
    <td>-</td>
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
    </td>
    <td>-</td>
    <td>Unsupported</td>
    <td>-</td>
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
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>4</td>
    <td><a href="https://wikipedia.org/wiki/GIF">GIF</a></td>
    <td>
        <b>Indexed:</b> 8-bit.
        <br/><br/>
        <b>Content:</b> Static, Animated, Meta data.
    </td>
    <td>-</td>
    <td>Unsupported</td>
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
    <td>Unsupported</td>
    <td>-</td>
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
    <td><a href="https://wikipedia.org/wiki/JPEG_2000">JPEG2000</a></td>
    <td>
        <b>Grayscale:</b> 8-bit, 16-bit.
        <b>RGB:</b> 24-bit, 48-bit.
        <b>YCbCr:</b> 24-bit.
        <b>RGBA:</b> 32-bit, 64-bit.
        <br/><br/>
        <b>Content:</b> Static.
    </td>
    <td>
        <b>Pixel formats:</b> YCCK, CMYK, LAB, XYZ, and other.
        <br/><br/>
        <b>Content:</b> Meta data, ICC profiles, images with non-zero position, images with bits per channel greater than 16.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>jasper</td>
</tr>
<tr>
    <td>8</td>
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
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>9</td>
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
    <td>10</td>
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
    <td>11</td>
    <td><a href="https://wikipedia.org/wiki/Scalable_Vector_Graphics">SVG</a></td>
    <td>
        <b>Bit depth:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static.
        <br/><br/>
        See <a href="https://razrfalcon.github.io/resvg-test-suite/svg-support-table.html">more</a>.
    </td>
    <td>
        <b>Content:</b> Animated, Meta data, ICC profiles.
        <br/><br/>
        See <a href="https://razrfalcon.github.io/resvg-test-suite/svg-support-table.html">more</a>.
    </td>
    <td>Unsupported</td>
    <td>-</td>
    <td>resvg</td>
</tr>
<tr>
    <td>12</td>
    <td><a href="https://wikipedia.org/wiki/Truevision_TGA">TGA</a></td>
    <td>
        <b>Grayscale:</b> 8-bit.
        <b>Indexed:</b> 8-bit.
        <b>RGB:</b> 24-bit.
        <b>RGBA:</b> 32-bit.
        <br/><br/>
        <b>Content:</b> Static, Meta data.
    </td>
    <td><b>Content:</b> Thumbnail images.</td>
    <td>Unsupported</td>
    <td>-</td>
    <td>-</td>
</tr>
<tr>
    <td>13</td>
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
    <td>14</td>
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
    <td>15</td>
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
    <td>16</td>
    <td><a href="https://en.wikipedia.org/wiki/X_BitMap">XBM</a></td>
    <td>
        <b>Bit depth:</b> 1-bit.
        <br/><br/>
        <b>Content:</b> Static.
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
