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
#ifndef SAIL_STATIC
    SAIL_CHECK_PATH_PTR(codec_info->path);
#endif
    SAIL_CHECK_CODEC_PTR(codec);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_codec), &ptr));
    struct sail_codec *codec_local = ptr;

    codec_local->layout = codec_info->layout;
    codec_local->handle = NULL;
    codec_local->v4     = NULL;

    SAIL_LOG_DEBUG("Loading codec '%s'", codec_info->path);

#ifdef SAIL_WIN32
#ifdef SAIL_STATIC
    HMODULE handle = GetModuleHandle(NULL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to open the current executable. Error: %d", GetLastError());
    }
#else
    HMODULE handle = LoadLibraryEx(codec_info->path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s'. Error: %d", codec_info->path, GetLastError());
    }
#endif

    if (handle == NULL) {
        destroy_codec(codec_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_LOAD);
    }

    codec_local->handle = handle;
#else
#ifdef SAIL_STATIC
    void *handle = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to open the current executable: %s", dlerror());
    }
#else
    void *handle = dlopen(codec_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", codec_info->path, dlerror());
    }
#endif

    if (handle == NULL) {
        destroy_codec(codec_local);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_LOAD);
    }

    codec_local->handle = handle;
#endif

#ifdef SAIL_WIN32
    #define SAIL_RESOLVE_FUNC GetProcAddress
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s'. Error: %d", symbol, codec_info->path, GetLastError())
#else
    #define SAIL_RESOLVE_FUNC dlsym
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s': %s", symbol, codec_info->path, dlerror())
#endif

#define SAIL_RESOLVE(target, handle, symbol, name)                                 \
    {                                                                              \
        char *full_symbol_name;                                                    \
        SAIL_TRY_OR_CLEANUP(sail_concat(&full_symbol_name, 3, #symbol, "_", name), \
                            /* cleanup */ destroy_codec(codec_local));             \
                                                                                   \
        /* To avoid copying name, make the whole string lower-case. */             \
        sail_to_lower(full_symbol_name);                                           \
                                                                                   \
        target = (symbol##_t)SAIL_RESOLVE_FUNC(handle, full_symbol_name);          \
                                                                                   \
        if (target == NULL) {                                                      \
            SAIL_RESOLVE_LOG_ERROR(full_symbol_name);                              \
            sail_free(full_symbol_name);                                           \
            destroy_codec(codec_local);                                            \
            SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_SYMBOL_RESOLVE);                  \
        }                                                                          \
                                                                                   \
        sail_free(full_symbol_name);                                               \
    } do{} while(0)

    if (codec_local->layout == SAIL_CODEC_LAYOUT_V4) {
        SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct sail_codec_layout_v4), &ptr),
                            /* cleanup */ destroy_codec(codec_local));
        codec_local->v4 = ptr;

        SAIL_RESOLVE(codec_local->v4->read_init,            handle, sail_codec_read_init_v4,            codec_info->name);
        SAIL_RESOLVE(codec_local->v4->read_seek_next_frame, handle, sail_codec_read_seek_next_frame_v4, codec_info->name);
        SAIL_RESOLVE(codec_local->v4->read_seek_next_pass,  handle, sail_codec_read_seek_next_pass_v4,  codec_info->name);
        SAIL_RESOLVE(codec_local->v4->read_frame,           handle, sail_codec_read_frame_v4,           codec_info->name);
        SAIL_RESOLVE(codec_local->v4->read_finish,          handle, sail_codec_read_finish_v4,          codec_info->name);

        SAIL_RESOLVE(codec_local->v4->write_init,            handle, sail_codec_write_init_v4,            codec_info->name);
        SAIL_RESOLVE(codec_local->v4->write_seek_next_frame, handle, sail_codec_write_seek_next_frame_v4, codec_info->name);
        SAIL_RESOLVE(codec_local->v4->write_seek_next_pass,  handle, sail_codec_write_seek_next_pass_v4,  codec_info->name);
        SAIL_RESOLVE(codec_local->v4->write_frame,           handle, sail_codec_write_frame_v4,           codec_info->name);
        SAIL_RESOLVE(codec_local->v4->write_finish,          handle, sail_codec_write_finish_v4,          codec_info->name);
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
    /* With static builds this handle is returned by GetModuleHandle, and the official docs don't recommend freeing it. */
    #ifdef SAIL_STATIC
        FreeLibrary((HMODULE)codec->handle);
    #endif
#else
        dlclose(codec->handle);
#endif
    }

    sail_free(codec->v4);
    sail_free(codec);
}
