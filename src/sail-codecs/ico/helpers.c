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

#include <string.h>

#include "sail-common.h"

#include "helpers.h"

sail_status_t ico_private_read_header(struct sail_io *io, struct SailIcoHeader *header) {

    SAIL_TRY(io->strict_read(io->stream, &header->reserved,     sizeof(header->reserved)));
    SAIL_TRY(io->strict_read(io->stream, &header->type,         sizeof(header->type)));
    SAIL_TRY(io->strict_read(io->stream, &header->images_count, sizeof(header->images_count)));

    return SAIL_OK;
}

sail_status_t ico_private_read_dir_entry(struct sail_io *io, struct SailIcoDirEntry *dir_entry) {

    SAIL_TRY(io->strict_read(io->stream, &dir_entry->width,        sizeof(dir_entry->width)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->height,       sizeof(dir_entry->height)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->color_count,  sizeof(dir_entry->color_count)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->reserved,     sizeof(dir_entry->reserved)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->planes,       sizeof(dir_entry->planes)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->bit_count,    sizeof(dir_entry->bit_count)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->image_size,   sizeof(dir_entry->image_size)));
    SAIL_TRY(io->strict_read(io->stream, &dir_entry->image_offset, sizeof(dir_entry->image_offset)));

    return SAIL_OK;
}

sail_status_t ico_private_probe_image_type(struct sail_io *io, enum SailIcoImageType *ico_image_type) {

    size_t saved_offset;
    SAIL_TRY(io->tell(io->stream, &saved_offset));

    const unsigned char check[] = "\x89PNG";

    unsigned char buffer[4];
    SAIL_TRY(io->strict_read(io->stream, buffer, sizeof(buffer)));

    SAIL_TRY(io->seek(io->stream, (long)saved_offset, SEEK_SET));

    *ico_image_type = (memcmp(buffer, check, sizeof(buffer)) == 0) ? SAIL_ICO_IMAGE_PNG : SAIL_ICO_IMAGE_BMP;

    return SAIL_OK;
}

sail_status_t ico_private_store_cur_hotspot(const struct SailIcoDirEntry *ico_dir_entry, struct sail_hash_map *special_properties) {

    struct sail_variant *variant;
    SAIL_TRY(sail_alloc_variant(&variant));

    SAIL_LOG_TRACE("CUR: X hotspot(%u)", ico_dir_entry->planes);
    sail_set_variant_unsigned_int(variant, ico_dir_entry->planes);
    sail_put_hash_map(special_properties, "cur-hotspot-x", variant);

    SAIL_LOG_TRACE("CUR: Y hotspot(%u)", ico_dir_entry->bit_count);
    sail_set_variant_unsigned_int(variant, ico_dir_entry->bit_count);
    sail_put_hash_map(special_properties, "cur-hotspot-y", variant);

    sail_destroy_variant(variant);

    return SAIL_OK;
}
