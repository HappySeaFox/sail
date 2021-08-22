/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

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

QStringList QtSail::filters() const
{
    QStringList filters { QStringLiteral("All Files (*.*)") };
    const sail_vector *codec_bundles = sail_codec_bundles();

    for (size_t i = 0; i < sail_vector_size(codec_bundles); i++) {
        const sail_codec_bundle *codec_bundle =
                reinterpret_cast<sail_codec_bundle *>(sail_get_vector_item(codec_bundles, i));

        QStringList masks;

        sail_string_node *extension_node = codec_bundle->codec_info->extension_node;

        while (extension_node != nullptr) {
            masks.append(QStringLiteral("*.%1").arg(extension_node->value));
            extension_node = extension_node->next;
        }

        filters.append(QStringLiteral("%1: %2 (%3)")
                       .arg(codec_bundle->codec_info->name)
                       .arg(codec_bundle->codec_info->description)
                       .arg(masks.join(QStringLiteral(" "))));
    }

    return filters;
}
