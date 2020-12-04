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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include "sail-common.h"
#include "sail.h"

sail_status_t alloc_and_load_codec(const struct sail_codec_info *codec_info, struct sail_codec **codec) {

    SAIL_CHECK_CODEC_INFO_PTR(codec_info);
    SAIL_CHECK_PATH_PTR(codec_info->path);
    SAIL_CHECK_CODEC_PTR(codec);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_codec), &ptr));
    struct sail_codec *codec_local = ptr;

    codec_local->layout = codec_info->layout;
    codec_local->handle = NULL;
    codec_local->v3     = NULL;

    SAIL_LOG_DEBUG("Loading codec '%s'", codec_info->path);

#ifdef SAIL_WIN32
    HMODULE handle = LoadLibraryEx(codec_info->path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s'. Error: %d", codec_info->path, GetLastError());
        destroy_codec(codec_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_LOAD);
    }

    codec_local->handle = handle;
#else
    void *handle = dlopen(codec_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", codec_info->path, dlerror());
        destroy_codec(codec_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_LOAD);
    }

    codec_local->handle = handle;
#endif

#ifdef SAIL_WIN32
    #define SAIL_RESOLVE_FUNC GetProcAddress
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s'. Error: %d", #symbol, codec_info->path, GetLastError())
#else
    #define SAIL_RESOLVE_FUNC dlsym
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s': %s", #symbol, codec_info->path, dlerror())
#endif

#define SAIL_RESOLVE(target, handle, symbol)                      \
    {                                                             \
        target = (symbol##_t)SAIL_RESOLVE_FUNC(handle, #symbol);  \
                                                                  \
        if (target == NULL) {                                     \
            SAIL_RESOLVE_LOG_ERROR(symbol);                       \
            destroy_codec(codec_local);                           \
            SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_SYMBOL_RESOLVE); \
        }                                                         \
    } do{} while(0)

    if (codec_local->layout == SAIL_CODEC_LAYOUT_V3) {
        SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct sail_codec_layout_v3), &ptr),
                            /* cleanup */ destroy_codec(codec_local));
        codec_local->v3 = ptr;

        SAIL_RESOLVE(codec_local->v3->read_init,            handle, sail_codec_read_init_v3);
        SAIL_RESOLVE(codec_local->v3->read_seek_next_frame, handle, sail_codec_read_seek_next_frame_v3);
        SAIL_RESOLVE(codec_local->v3->read_seek_next_pass,  handle, sail_codec_read_seek_next_pass_v3);
        SAIL_RESOLVE(codec_local->v3->read_frame,           handle, sail_codec_read_frame_v3);
        SAIL_RESOLVE(codec_local->v3->read_finish,          handle, sail_codec_read_finish_v3);

        SAIL_RESOLVE(codec_local->v3->write_init,            handle, sail_codec_write_init_v3);
        SAIL_RESOLVE(codec_local->v3->write_seek_next_frame, handle, sail_codec_write_seek_next_frame_v3);
        SAIL_RESOLVE(codec_local->v3->write_seek_next_pass,  handle, sail_codec_write_seek_next_pass_v3);
        SAIL_RESOLVE(codec_local->v3->write_frame,           handle, sail_codec_write_frame_v3);
        SAIL_RESOLVE(codec_local->v3->write_finish,          handle, sail_codec_write_finish_v3);
    } else {
        destroy_codec(codec_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_CODEC_LAYOUT);
    }

    *codec = codec_local;

    return SAIL_OK;
}

void destroy_codec(struct sail_codec *codec) {

    if (codec == NULL) {
        return;
    }

    if (codec->handle != NULL) {
#ifdef SAIL_WIN32
        FreeLibrary((HMODULE)codec->handle);
#else
        dlclose(codec->handle);
#endif
    }

    if (codec->layout == SAIL_CODEC_LAYOUT_V3) {
        sail_free(codec->v3);
    } else {
        SAIL_LOG_WARNING("Don't know how to destroy codec interface version %d", codec->layout);
    }

    sail_free(codec);
}
