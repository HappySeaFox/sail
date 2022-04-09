/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020-2021 Dmitry Baryshev

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

#include <stdio.h>

#include "sail.h"

#include "sail-dump.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path to image>", argv[0]);
        return 1;
    }

    void *state = NULL;
    SAIL_TRY_OR_EXECUTE(sail_start_loading_file(argv[1], NULL, &state),
                        /* on error */ return 2);

    sail_status_t res;
    struct sail_image *image;

    while ((res = sail_load_next_frame(state, &image)) == SAIL_OK) {

        SAIL_TRY_OR_EXECUTE(sail_dump(image),
                            /* on error */ return 3);

        sail_destroy_image(image);
    }

    if (res != SAIL_ERROR_NO_MORE_FRAMES) {
        sail_stop_loading(state);
        return res;
    }

    SAIL_TRY_OR_EXECUTE(sail_stop_loading(state),
                        /* on error */ return 4);

    return 0;
}
