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

#ifndef SAIL_CODEC_LAYOUT_H
#define SAIL_CODEC_LAYOUT_H

#ifdef SAIL_BUILD
    #include "layout/v7_pointers.h"
#else
    #include <sail/layout/v7_pointers.h>
#endif

/*
 * Currently supported codec layout version.
 */
#define SAIL_CODEC_LAYOUT_V7 7

struct sail_codec_layout_v7 {
    sail_codec_load_init_v7_t            load_init;
    sail_codec_load_seek_next_frame_v7_t load_seek_next_frame;
    sail_codec_load_frame_v7_t           load_frame;
    sail_codec_load_finish_v7_t          load_finish;

    sail_codec_save_init_v7_t            save_init;
    sail_codec_save_seek_next_frame_v7_t save_seek_next_frame;
    sail_codec_save_frame_v7_t           save_frame;
    sail_codec_save_finish_v7_t          save_finish;
};

#endif
