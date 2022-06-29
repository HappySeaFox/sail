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

#include <stdbool.h>
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

/*
 * Private functions.
 */

static sail_status_t alloc_codec(struct sail_codec **codec) {

    SAIL_CHECK_PTR(codec);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_codec), &ptr));
    *codec = ptr;

    (*codec)->layout = 0;
    (*codec)->handle = NULL;
    (*codec)->v8     = NULL;

    return SAIL_OK;
}

#ifdef SAIL_COMBINE_CODECS
static sail_status_t load_combined_codec(const struct sail_codec_info *codec_info, struct sail_codec *codec) {

#ifdef SAIL_STATIC
    /* For example: [ "gif", "jpeg", "png" ]. */
    extern const char * const sail_enabled_codecs[];
    extern struct sail_codec_layout_v8 const sail_enabled_codecs_layouts[];
#else
    SAIL_IMPORT extern const char * const sail_enabled_codecs[];
    SAIL_IMPORT extern struct sail_codec_layout_v8 const sail_enabled_codecs_layouts[];
#endif
    for (size_t i = 0; sail_enabled_codecs[i] != NULL; i++) {
        if (strcmp(sail_enabled_codecs[i], codec_info->name) == 0) {
            *codec->v8 = sail_enabled_codecs_layouts[i];
            return SAIL_OK;
        }
    }

    SAIL_LOG_ERROR("Failed to find combined %s codec", codec_info->name);
    SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
}
#endif

static sail_status_t load_codec_from_file(const struct sail_codec_info *codec_info, struct sail_codec *codec) {

#ifdef SAIL_WIN32
    HMODULE handle = LoadLibraryEx(codec_info->path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s'. Error: 0x%X", codec_info->path, GetLastError());
    }
#else
    void *handle = dlopen(codec_info->path, RTLD_LAZY | RTLD_LOCAL);

    if (handle == NULL) {
        SAIL_LOG_ERROR("Failed to load '%s': %s", codec_info->path, dlerror());
    }
#endif

    if (handle == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_LOAD);
    }

    codec->handle = handle;

#ifdef SAIL_WIN32
    #define SAIL_RESOLVE_FUNC GetProcAddress
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s'. Error: 0x%X", symbol, codec_info->path, GetLastError())
#else
    #define SAIL_RESOLVE_FUNC dlsym
    #define SAIL_RESOLVE_LOG_ERROR(symbol) \
        SAIL_LOG_ERROR("Failed to resolve '%s' in '%s': %s", symbol, codec_info->path, dlerror())
#endif

#define SAIL_RESOLVE(target, handle, symbol, name)                                 \
    {                                                                              \
        char *full_symbol_name;                                                    \
        SAIL_TRY(sail_concat(&full_symbol_name, 3, #symbol, "_", name));           \
                                                                                   \
        /* To avoid copying name, make the whole string lower-case. */             \
        sail_to_lower(full_symbol_name);                                           \
                                                                                   \
        target = (symbol##_t)SAIL_RESOLVE_FUNC(handle, full_symbol_name);          \
                                                                                   \
        if (target == NULL) {                                                      \
            SAIL_RESOLVE_LOG_ERROR(full_symbol_name);                              \
            sail_free(full_symbol_name);                                           \
            SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_SYMBOL_RESOLVE);                  \
        }                                                                          \
                                                                                   \
        sail_free(full_symbol_name);                                               \
    } do{} while(0)

    SAIL_RESOLVE(codec->v8->load_init,            handle, sail_codec_load_init_v8,            codec_info->name);
    SAIL_RESOLVE(codec->v8->load_seek_next_frame, handle, sail_codec_load_seek_next_frame_v8, codec_info->name);
    SAIL_RESOLVE(codec->v8->load_frame,           handle, sail_codec_load_frame_v8,           codec_info->name);
    SAIL_RESOLVE(codec->v8->load_finish,          handle, sail_codec_load_finish_v8,          codec_info->name);

    SAIL_RESOLVE(codec->v8->save_init,            handle, sail_codec_save_init_v8,            codec_info->name);
    SAIL_RESOLVE(codec->v8->save_seek_next_frame, handle, sail_codec_save_seek_next_frame_v8, codec_info->name);
    SAIL_RESOLVE(codec->v8->save_frame,           handle, sail_codec_save_frame_v8,           codec_info->name);
    SAIL_RESOLVE(codec->v8->save_finish,          handle, sail_codec_save_finish_v8,          codec_info->name);

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t alloc_and_load_codec(const struct sail_codec_info *codec_info, struct sail_codec **codec) {

    SAIL_CHECK_PTR(codec_info);
    SAIL_CHECK_PTR(codec);

    if (codec_info->layout != SAIL_CODEC_LAYOUT_V8) {
        SAIL_LOG_ERROR("Failed to load %s codec with unsupported layout V%d (expected V%d)", codec_info->name, codec_info->layout, SAIL_CODEC_LAYOUT_V8);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_UNSUPPORTED_CODEC_LAYOUT);
    }

    /*
     * When SAIL_COMBINE_CODECS is ON, we can load built-in codecs with empty paths,
     * and client codecs with non-empty paths from disk.
     *
     * When SAIL_COMBINE_CODECS is OFF, we can load only codecs with non-empty paths from disk.
     */
#ifndef SAIL_COMBINE_CODECS
    if (codec_info->path == NULL) {
        SAIL_LOG_ERROR("Failed to load %s codec with empty path when SAIL_COMBINE_CODECS is disabled", codec_info->name);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CODEC_NOT_FOUND);
    }
#endif

    const bool fetch_combined_codec = codec_info->path == NULL;

    struct sail_codec *codec_local;
    SAIL_TRY(alloc_codec(&codec_local));
    codec_local->layout = codec_info->layout;

    if (fetch_combined_codec) {
        SAIL_LOG_DEBUG("Fetching V%d functions for %s codec", codec_info->layout, codec_info->name);
    } else {
        SAIL_LOG_DEBUG("Loading %s codec from %s", codec_info->name, codec_info->path);
    }

    void *ptr;
    SAIL_TRY_OR_CLEANUP(sail_malloc(sizeof(struct sail_codec_layout_v8), &ptr),
                        /* cleanup */ destroy_codec(codec_local));
    codec_local->v8 = ptr;

#ifdef SAIL_COMBINE_CODECS
    if (fetch_combined_codec) {
        SAIL_TRY_OR_CLEANUP(load_combined_codec(codec_info, codec_local),
                            /* cleanup */ destroy_codec(codec_local));
    } else {
        SAIL_TRY_OR_CLEANUP(load_codec_from_file(codec_info, codec_local),
                            /* cleanup */ destroy_codec(codec_local));
    }
#else
    SAIL_TRY_OR_CLEANUP(load_codec_from_file(codec_info, codec_local),
                        /* cleanup */ destroy_codec(codec_local));
#endif

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

    sail_free(codec->v8);
    sail_free(codec);
}
