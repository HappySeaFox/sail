/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2022 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "sail-c++.h"

std::ostream& operator<<(std::ostream &os, SailPixelFormat pixel_format)
{
    os << sail_pixel_format_to_string(pixel_format);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailChromaSubsampling chroma_subsampling)
{
    os << sail_chroma_subsampling_to_string(chroma_subsampling);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailOrientation orientation)
{
    os << sail_orientation_to_string(orientation);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailCompression compression)
{
    os << sail_compression_to_string(compression);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailMetaData meta_data)
{
    os << sail_meta_data_to_string(meta_data);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailResolutionUnit resolution_unit)
{
    os << sail_resolution_unit_to_string(resolution_unit);
    return os;
}

std::ostream& operator<<(std::ostream &os, SailCodecFeature codec_feature)
{
    os << sail_codec_feature_to_string(codec_feature);
    return os;
}
