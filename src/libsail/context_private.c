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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SAIL_WIN32
    #include <windows.h> /* FindFirstFile */
#else
    #include <dirent.h> /* opendir */
    #include <sys/types.h>
#endif

#include "sail.h"

/*
 * Private functions.
 */

static struct sail_context *global_context = NULL;

static sail_mutex_t global_context_guard_mutex;

static bool global_context_guard_mutex_initialized = false;

/* Must be called by threading_call_once() to guarantee atomic operation. */
static void initialize_global_context_guard_mutex_callback(void) {

    SAIL_TRY_OR_EXECUTE(threading_init_mutex(&global_context_guard_mutex),
                        /* on error */ return);

    SAIL_LOG_DEBUG("Allocated new global context mutex");

    global_context_guard_mutex_initialized = true;
}

static sail_status_t initialize_global_context_guard_mutex(void) {

    static sail_once_flag_t once_flag = SAIL_ONCE_DEFAULT_VALUE;
    SAIL_TRY(threading_call_once(&once_flag, initialize_global_context_guard_mutex_callback));

    if (!global_context_guard_mutex_initialized) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_CONTEXT_UNINITIALIZED);
    }

    return SAIL_OK;
}

#ifdef SAIL_WIN32
static sail_status_t add_dll_directory(const char *path) {

    SAIL_CHECK_PTR(path);

    SAIL_LOG_DEBUG("Add '%s' to the DLL search paths", path);

    wchar_t *path_w;
    SAIL_TRY(sail_to_wchar(path, &path_w));

    if (!AddDllDirectory(path_w)) {
        SAIL_LOG_ERROR("Failed to update library search path with '%s'. Error: 0x%X", path, GetLastError());
        sail_free(path_w);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_ENV_UPDATE);
    }

    sail_free(path_w);

    return SAIL_OK;
}

static sail_status_t get_sail_dll_path(char *dll_path, int dll_path_size) {

    HMODULE thisModule;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)&get_sail_dll_path, &thisModule) == 0) {
        SAIL_LOG_ERROR("GetModuleHandleEx() failed with error code 0x%X. sail.dll location will not be added as a DLL search path", GetLastError());
        return SAIL_ERROR_GET_DLL_PATH;
    } else if (GetModuleFileName(thisModule, dll_path, dll_path_size) == 0) {
        SAIL_LOG_ERROR("GetModuleFileName() failed with error code 0x%X. sail.dll location will not be added as a DLL search path", GetLastError());
        return SAIL_ERROR_GET_DLL_PATH;
    } else {
        /* "...\bin\sail.dll" -> "...\bin". */
        char *last_sep = strrchr(dll_path, '\\');

        if (last_sep == NULL) {
            return SAIL_ERROR_GET_DLL_PATH;
        } else {
            *last_sep = '\0';
        }
    }

    return SAIL_OK;
}
#endif

#ifndef SAIL_COMBINE_CODECS
static const char* sail_codecs_path_env(void) {

    static SAIL_THREAD_LOCAL bool codecs_path_env_called = false;
    static SAIL_THREAD_LOCAL const char *env = NULL;

    if (codecs_path_env_called) {
        return env;
    }

    codecs_path_env_called = true;

#ifdef _MSC_VER
    _dupenv_s((char **)&env, NULL, "SAIL_CODECS_PATH");
#else
    env = getenv("SAIL_CODECS_PATH");
#endif

    return env;
}
#endif

#ifdef SAIL_THIRD_PARTY_CODECS_PATH
static const char* client_codecs_path(void) {

    static SAIL_THREAD_LOCAL bool codecs_path_called = false;
    static SAIL_THREAD_LOCAL const char *env = NULL;

    if (codecs_path_called) {
        return env;
    }

    codecs_path_called = true;

#ifdef _MSC_VER
    _dupenv_s((char **)&env, NULL, "SAIL_THIRD_PARTY_CODECS_PATH");
#else
    env = getenv("SAIL_THIRD_PARTY_CODECS_PATH");
#endif

    if (env == NULL) {
        SAIL_LOG_DEBUG("SAIL_THIRD_PARTY_CODECS_PATH environment variable is not set. Not loading codecs from it");
    } else {
        SAIL_LOG_DEBUG("SAIL_THIRD_PARTY_CODECS_PATH environment variable is set. Loading codecs from '%s'", env);
    }

    return env;
}

static sail_status_t client_codecs_paths_to_string_node_chain(struct sail_string_node **string_node) {

    const char *client_codecs_path_value = client_codecs_path();

    if (client_codecs_path_value == NULL) {
        *string_node = NULL;
    } else {
        SAIL_TRY(split_into_string_node_chain(client_codecs_path_value, string_node));
    }

    return SAIL_OK;
}
#endif

static sail_status_t alloc_context(struct sail_context **context) {

    SAIL_CHECK_PTR(context);

    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_context), &ptr));
    *context = ptr;

    (*context)->initialized     = false;
    (*context)->codec_bundle_node = NULL;

    return SAIL_OK;
}

static sail_status_t allocate_global_context(struct sail_context **context) {

    SAIL_CHECK_PTR(context);

    if (global_context == NULL) {
        SAIL_TRY(alloc_context(&global_context));
        SAIL_LOG_DEBUG("Allocated new context %p", global_context);
    }

    *context = global_context;

    return SAIL_OK;
}

static sail_status_t destroy_context(struct sail_context *context) {

    if (context == NULL) {
        return SAIL_OK;
    }

    destroy_codec_bundle_node_chain(context->codec_bundle_node);
    sail_free(context);

    return SAIL_OK;
}

static sail_status_t preload_codecs(struct sail_context *context) {

    SAIL_CHECK_PTR(context);

    SAIL_TRY(lock_context());

    SAIL_LOG_DEBUG("Preloading codecs");

    for (struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next) {
        const struct sail_codec *codec;

        /* Ignore loading errors on purpose. */
        (void)load_codec_by_codec_info(codec_bundle_node->codec_bundle->codec_info, &codec);
    }

    SAIL_TRY(unlock_context());

    return SAIL_OK;
}

static int codec_bundle_priority_comparator(const void *elem1, const void *elem2) {

    const int priority1 = (*(struct sail_codec_bundle_node **)elem1)->codec_bundle->codec_info->priority;
    const int priority2 = (*(struct sail_codec_bundle_node **)elem2)->codec_bundle->codec_info->priority;

    return priority1 - priority2;
}

/*
 * Space complexity: O(n)
 * Time complexity: O(n * log(n))
 */
static sail_status_t sort_enumerated_codecs(struct sail_context *context) {

    /* 0 or 1 elements - nothing to sort. */
    if (context->codec_bundle_node == NULL || context->codec_bundle_node->next == NULL) {
        return SAIL_OK;
    }

    /* Count the number of codecs. */
    unsigned codecs_num = 0;

    for (struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next) {
        codecs_num++;
    }

    /* Copy codecs to an array. */
    struct sail_codec_bundle_node **codec_bundle_array;
    void *ptr;
    SAIL_TRY(sail_malloc(sizeof(struct sail_codec_bundle_node *) * codecs_num, &ptr));
    codec_bundle_array = ptr;

    {
        unsigned i = 0;
        for (struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next) {
            codec_bundle_array[i++] = codec_bundle_node;
        }
    }

    /* Sort the array. */
    qsort(codec_bundle_array, codecs_num, sizeof(struct sail_codec_bundle_node *), codec_bundle_priority_comparator);

    /* Reconstruct the linked list. */
    struct sail_codec_bundle_node *codec_bundle_node_sorted_it = codec_bundle_array[0];
    struct sail_codec_bundle_node *codec_bundle_node_sorted = codec_bundle_node_sorted_it;

    for (unsigned i = 1; i < codecs_num; i++) {
        codec_bundle_node_sorted_it->next = codec_bundle_array[i];
        codec_bundle_node_sorted_it = codec_bundle_node_sorted_it->next;
    }

    codec_bundle_node_sorted_it->next = NULL;

    context->codec_bundle_node = codec_bundle_node_sorted;

    /* Cleanup */
    sail_free(codec_bundle_array);

    return SAIL_OK;
}

static sail_status_t print_enumerated_codecs(struct sail_context *context) {

    SAIL_CHECK_PTR(context);

    const struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node;

    if (codec_bundle_node == NULL) {
        return SAIL_OK;
    }

    /* Print the found codec infos. */
    SAIL_LOG_DEBUG("Enumerated codecs:");

    for (int counter = 1; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next, counter++) {
        const struct sail_codec_info *codec_info = codec_bundle_node->codec_bundle->codec_info;

        SAIL_LOG_DEBUG("%d. [p%d] %s [%s] %s", counter, codec_info->priority, codec_info->name, codec_info->description, codec_info->version);
    }

    return SAIL_OK;
}

#if !defined SAIL_COMBINE_CODECS || defined SAIL_THIRD_PARTY_CODECS_PATH
/* Add codecs_path/lib to the DLL/SO search path. */
static sail_status_t add_lib_subdir_to_dll_search_path(const char *codecs_path) {

#ifdef SAIL_WIN32
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, codecs_path, "\\lib"));

    if (!sail_is_dir(full_path_to_lib)) {
        SAIL_LOG_DEBUG("Optional DLL directory '%s' doesn't exist, so not loading DLLs from it", full_path_to_lib);
        sail_free(full_path_to_lib);
        return SAIL_OK;
    }

    SAIL_TRY_OR_CLEANUP(add_dll_directory(full_path_to_lib),
                        /* cleanup */ sail_free(full_path_to_lib));

    sail_free(full_path_to_lib);
#else
    char *full_path_to_lib;
    SAIL_TRY(sail_concat(&full_path_to_lib, 2, codecs_path, "/lib"));

    if (!sail_is_dir(full_path_to_lib)) {
        SAIL_LOG_DEBUG("Optional LIB directory '%s' doesn't exist, so not updating LD_LIBRARY_PATH with it", full_path_to_lib);
        sail_free(full_path_to_lib);
        return SAIL_OK;
    }

    char *combined_ld_library_path;
    char *env = getenv("LD_LIBRARY_PATH");

    if (env == NULL) {
        SAIL_TRY_OR_CLEANUP(sail_strdup(full_path_to_lib, &combined_ld_library_path),
                            sail_free(full_path_to_lib));
    } else {
        SAIL_TRY_OR_CLEANUP(sail_concat(&combined_ld_library_path, 3, env, ":", full_path_to_lib),
                            sail_free(full_path_to_lib));
    }

    sail_free(full_path_to_lib);
    SAIL_LOG_DEBUG("Set LD_LIBRARY_PATH to '%s'", combined_ld_library_path);

    if (setenv("LD_LIBRARY_PATH", combined_ld_library_path, true) != 0) {
        SAIL_LOG_ERROR("Failed to update library search path: %s", strerror(errno));
        sail_free(combined_ld_library_path);
        SAIL_LOG_AND_RETURN(SAIL_ERROR_ENV_UPDATE);
    }

    sail_free(combined_ld_library_path);
#endif

    return SAIL_OK;
}

static sail_status_t build_full_path(const char *sail_codecs_path, const char *name, char **full_path) {

#ifdef SAIL_WIN32
    SAIL_TRY(sail_concat(full_path, 3, sail_codecs_path, "\\", name));
#else
    SAIL_TRY(sail_concat(full_path, 3, sail_codecs_path, "/", name));
#endif

    return SAIL_OK;
}

static sail_status_t build_codec_bundle_from_codec_info_path(const char *codec_info_full_path,
                                                             struct sail_codec_bundle_node **codec_bundle_node) {

    SAIL_CHECK_PTR(codec_info_full_path);
    SAIL_CHECK_PTR(codec_bundle_node);

    /* Build "/path/jpeg.so" from "/path/jpeg.codec.info". */
    char *codec_info_part = strstr(codec_info_full_path, ".codec.info");

    if (codec_info_part == NULL) {
        SAIL_LOG_AND_RETURN(SAIL_ERROR_MEMORY_ALLOCATION);
    }

    /* The length of "/path/jpeg". */
    size_t codec_full_path_length = strlen(codec_info_full_path) - strlen(codec_info_part);
    char *codec_full_path;

#ifdef SAIL_WIN32
    static const char * const LIB_SUFFIX = "dll";
#else
    static const char * const LIB_SUFFIX = "so";
#endif

    /* The resulting string will be "/path/jpeg.plu" (on Windows) or "/path/jpeg.pl". */
    SAIL_TRY(sail_strdup_length(codec_info_full_path,
                                codec_full_path_length + strlen(LIB_SUFFIX) + 1, &codec_full_path));

#ifdef _MSC_VER
    /* Overwrite the end of the path with "dll". */
    strcpy_s(codec_full_path + codec_full_path_length + 1, strlen(LIB_SUFFIX) + 1, LIB_SUFFIX);
#else
    /* Overwrite the end of the path with "so". */
    strcpy(codec_full_path + codec_full_path_length + 1, LIB_SUFFIX);
#endif

    struct sail_codec_bundle_node *local_codec_bundle_node;

    /* Parse codec info. */
    SAIL_TRY_OR_CLEANUP(alloc_codec_bundle_node(&local_codec_bundle_node),
                        /* cleanup */ sail_free(codec_full_path));
    SAIL_TRY_OR_CLEANUP(alloc_codec_bundle(&local_codec_bundle_node->codec_bundle),
                        /* cleanup */ destroy_codec_bundle_node(local_codec_bundle_node),
                                      sail_free(codec_full_path));

    SAIL_TRY_OR_CLEANUP(codec_read_info_from_file(codec_info_full_path, &local_codec_bundle_node->codec_bundle->codec_info),
                        destroy_codec_bundle_node(local_codec_bundle_node),
                        sail_free(codec_full_path));
    local_codec_bundle_node->codec_bundle->codec_info->path = codec_full_path;

    /* Save the parsed codec info into the SAIL context. */
    *codec_bundle_node = local_codec_bundle_node;

    return SAIL_OK;
}

static sail_status_t enumerate_codecs_in_paths(struct sail_context *context, const struct sail_string_node *string_node) {

    SAIL_CHECK_PTR(context);

    /* Used to load and store codec info objects. */
    struct sail_codec_bundle_node **last_codec_bundle_node = &context->codec_bundle_node;
    struct sail_codec_bundle_node *codec_bundle_node;

    for (; string_node != NULL; string_node = string_node->next) {
        const char *codecs_path = string_node->value;

        SAIL_TRY(add_lib_subdir_to_dll_search_path(codecs_path));

        SAIL_LOG_DEBUG("Enumerating codecs in '%s'", codecs_path);

#ifdef SAIL_WIN32
        const char *plugs_info_mask = "\\*.codec.info";

        size_t codecs_path_with_mask_length = strlen(codecs_path) + strlen(plugs_info_mask) + 1;

        void *ptr;
        SAIL_TRY(sail_malloc(codecs_path_with_mask_length, &ptr));
        char *codecs_path_with_mask = ptr;

#ifdef _MSC_VER
        strcpy_s(codecs_path_with_mask, codecs_path_with_mask_length, codecs_path);
        strcat_s(codecs_path_with_mask, codecs_path_with_mask_length, plugs_info_mask);
#else
        strcpy(codecs_path_with_mask, codecs_path);
        strcat(codecs_path_with_mask, plugs_info_mask);
#endif

        WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile(codecs_path_with_mask, &data);

        if (hFind == INVALID_HANDLE_VALUE) {
            SAIL_LOG_ERROR("Failed to list files in '%s'. Error: 0x%X. No codecs loaded from it", codecs_path, GetLastError());
            sail_free(codecs_path_with_mask);
            continue;
        }

        do {
            /* Build a full path. */
            char *full_path;

            /* Ignore errors and try to load as much as possible. */
            SAIL_TRY_OR_EXECUTE(build_full_path(codecs_path, data.cFileName, &full_path),
                                /* on error */ continue);

            SAIL_LOG_DEBUG("Found codec info '%s'", data.cFileName);

            if (build_codec_bundle_from_codec_info_path(full_path, &codec_bundle_node) == SAIL_OK) {
                *last_codec_bundle_node = codec_bundle_node;
                last_codec_bundle_node = &codec_bundle_node->next;
            }

            sail_free(full_path);
        } while (FindNextFile(hFind, &data));

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            SAIL_LOG_ERROR("Failed to list files in '%s'. Error: 0x%X. Some codecs may not be loaded from it", codecs_path, GetLastError());
        }

        sail_free(codecs_path_with_mask);
        FindClose(hFind);
#else
        DIR *d = opendir(codecs_path);

        if (d == NULL) {
            SAIL_LOG_ERROR("Failed to list files in '%s': %s", codecs_path, strerror(errno));
            continue;
        }

        struct dirent *dir;

        while ((dir = readdir(d)) != NULL) {
            /* Build a full path. */
            char *full_path;

            /* Ignore errors and try to load as much as possible. */
            SAIL_TRY_OR_EXECUTE(build_full_path(codecs_path, dir->d_name, &full_path),
                                /* on error */ continue);

            /* Handle files only. */
            if (sail_is_file(full_path)) {
                bool is_codec_info = strstr(full_path, ".codec.info") != NULL;

                if (is_codec_info) {
                    SAIL_LOG_DEBUG("Found codec info '%s'", dir->d_name);

                    if (build_codec_bundle_from_codec_info_path(full_path, &codec_bundle_node) == SAIL_OK) {
                        *last_codec_bundle_node = codec_bundle_node;
                        last_codec_bundle_node = &codec_bundle_node->next;
                    }
                }
            }

            sail_free(full_path);
        }

        closedir(d);
#endif
    }

    return SAIL_OK;
}
#endif

/* Initializes the context and loads all the codec info files. */
#ifdef SAIL_COMBINE_CODECS
static sail_status_t init_context_impl(struct sail_context *context) {

    SAIL_CHECK_PTR(context);

    /* Externs from sail-codecs. */
#ifdef SAIL_STATIC
    /* For example: [ "gif", "jpeg", "png" ]. */
    extern const char * const sail_enabled_codecs[];
    extern const char * const sail_enabled_codecs_info[];
#else
    SAIL_IMPORT extern const char * const sail_enabled_codecs[];
    SAIL_IMPORT extern const char * const sail_enabled_codecs_info[];
#endif

    /* Load codec info objects. */
    struct sail_codec_bundle_node **last_codec_bundle_node = &context->codec_bundle_node;

    for (size_t i = 0; sail_enabled_codecs[i] != NULL; i++) {
        const char *sail_codec_info = sail_enabled_codecs_info[i];

        /* Parse codec info. */
        struct sail_codec_bundle_node *codec_bundle_node;
        SAIL_TRY_OR_EXECUTE(alloc_codec_bundle_node(&codec_bundle_node),
                            /* on error */ continue);

        SAIL_TRY_OR_EXECUTE(alloc_codec_bundle(&codec_bundle_node->codec_bundle),
                            /* on error */ destroy_codec_bundle_node(codec_bundle_node);
                                           continue);

        SAIL_TRY_OR_EXECUTE(codec_read_info_from_string(sail_codec_info, &codec_bundle_node->codec_bundle->codec_info),
                            /* on error */ destroy_codec_bundle_node(codec_bundle_node);
                                           continue);

        *last_codec_bundle_node = codec_bundle_node;
        last_codec_bundle_node = &codec_bundle_node->next;
    }

#ifdef SAIL_THIRD_PARTY_CODECS_PATH
    /* Load client codecs. */
    struct sail_string_node *client_codecs_paths;
    SAIL_TRY(client_codecs_paths_to_string_node_chain(&client_codecs_paths));

    SAIL_TRY_OR_CLEANUP(enumerate_codecs_in_paths(context, client_codecs_paths),
                        /* cleanup */ destroy_string_node_chain(client_codecs_paths));

    destroy_string_node_chain(client_codecs_paths);
#endif

    return SAIL_OK;
}
#else /* SAIL_COMBINE_CODECS=OFF. */
static const char* sail_codecs_path(void) {

    static SAIL_THREAD_LOCAL bool codecs_path_called = false;
    static SAIL_THREAD_LOCAL const char *path = NULL;

    if (codecs_path_called) {
        return path;
    }

    codecs_path_called = true;

#ifdef SAIL_WIN32
    char dll_path[MAX_PATH];

    /* Construct "\bin\..\lib\sail\codecs" from "\bin\sail.dll". */
    if (get_sail_dll_path(dll_path, sizeof(dll_path)) == SAIL_OK) {
        char *lib_sail_codecs_path;

        #ifdef SAIL_VCPKG
            /* "\bin" -> "\bin\sail\codecs" */
            const char *CODECS_RELATIVE_PATH = "\\sail\\codecs";
        #else
            /* "\bin" -> "\bin\..\lib\sail\codecs" */
            const char *CODECS_RELATIVE_PATH = "\\..\\lib\\sail\\codecs";
        #endif

        if (sail_concat(&lib_sail_codecs_path, 2, dll_path, CODECS_RELATIVE_PATH) == SAIL_OK) {
            path = lib_sail_codecs_path;
        } else {
            SAIL_LOG_ERROR("Failed to concat strings. Falling back to loading codecs from '%s'", SAIL_CODECS_PATH);
            path = SAIL_CODECS_PATH;
        }
    } else {
        path = SAIL_CODECS_PATH;
        SAIL_LOG_ERROR("Failed to get the sail.dll path. Falling back to loading codecs from '%s'", path);
    }
#else
    path = SAIL_CODECS_PATH;
#endif

    return path;
}

static sail_status_t init_context_impl(struct sail_context *context) {

    SAIL_CHECK_PTR(context);

    /* Our own codecs. */
    const char *env = sail_codecs_path_env();
    const char *our_codecs_path;

    if (env == NULL) {
        our_codecs_path = sail_codecs_path();
        SAIL_LOG_DEBUG("SAIL_CODECS_PATH environment variable is not set. Loading codecs from '%s'", our_codecs_path);
    } else {
        our_codecs_path = env;
        SAIL_LOG_DEBUG("SAIL_CODECS_PATH environment variable is set. Loading codecs from '%s'", env);
    }

    /* Construct a list of paths to search. */
    struct sail_string_node *codecs_paths;
    SAIL_TRY(alloc_string_node(&codecs_paths));

    SAIL_TRY_OR_CLEANUP(sail_strdup(our_codecs_path, &codecs_paths->value),
                        /* cleanup */ destroy_string_node(codecs_paths));

#ifdef SAIL_THIRD_PARTY_CODECS_PATH
    SAIL_TRY(client_codecs_paths_to_string_node_chain(&codecs_paths->next));
#endif

    SAIL_TRY_OR_CLEANUP(enumerate_codecs_in_paths(context, codecs_paths),
                        /* cleanup */ destroy_string_node_chain(codecs_paths));

    destroy_string_node_chain(codecs_paths);

    return SAIL_OK;
}
#endif

static void print_no_codecs_found(void) {

    const char *message = "\n"
        "\n*** No codecs were found. You could try the following:                       ***"
        "\n*** - Inspect the error messages printed in stderr.                          ***"
#ifdef SAIL_STATIC
        "\n*** - Make sure the application is linked against the sail-codecs            ***"
        "\n***   and sail-codecs-objects libraries using the 'whole archive' option.    ***"
#elif defined SAIL_COMBINE_CODECS
        "\n*** - Make sure the application is linked against the sail-codecs library.   ***"
#else
        "\n*** - Check the installation directory.                                      ***"
#endif
        "\n";

    SAIL_LOG_ERROR("%s", message);
}

static void print_build_statistics(void) {

    SAIL_LOG_INFO("Version: %s", SAIL_VERSION_STRING);

#ifdef SAIL_VCPKG
    SAIL_LOG_INFO("Build type: VCPKG");
#else
    SAIL_LOG_INFO("Build type: Standalone");
#endif

#ifdef SAIL_STATIC
    SAIL_LOG_INFO("Static build: yes");
#else
    SAIL_LOG_INFO("Static build: no");
#endif

#ifdef SAIL_COMBINE_CODECS
    SAIL_LOG_INFO("Combine codecs: yes");
#else
    SAIL_LOG_INFO("Combine codecs: no");
#endif

#ifdef SAIL_THIRD_PARTY_CODECS_PATH
    SAIL_LOG_INFO("SAIL_THIRD_PARTY_CODECS_PATH: enabled");
#else
    SAIL_LOG_INFO("SAIL_THIRD_PARTY_CODECS_PATH: disabled");
#endif
}

/* Initializes the context and loads all the codec info files if the context is not initialized. */
static sail_status_t init_context(struct sail_context *context, int flags) {

    SAIL_CHECK_PTR(context);

    if (context->initialized) {
        return SAIL_OK;
    }

    context->initialized = true;

    /* Time counter. */
    uint64_t start_time = sail_now();

    print_build_statistics();

    /* Always search DLLs in the sail.dll location so custom codecs can hold dependencies there. */
#ifdef SAIL_WIN32
    char dll_path[MAX_PATH];
    if (get_sail_dll_path(dll_path, sizeof(dll_path)) == SAIL_OK) {
        SAIL_TRY_OR_SUPPRESS(add_dll_directory(dll_path));
    }
#endif

    SAIL_TRY(init_context_impl(context));

    if (context->codec_bundle_node == NULL) {
        print_no_codecs_found();
    }

    SAIL_TRY(sort_enumerated_codecs(context));

    SAIL_TRY(print_enumerated_codecs(context));

    if (flags & SAIL_FLAG_PRELOAD_CODECS) {
        SAIL_TRY(preload_codecs(context));
    }

    SAIL_LOG_DEBUG("Initialized in %lu ms.", (unsigned long)(sail_now() - start_time));

    return SAIL_OK;
}

/*
 * Public functions.
 */

sail_status_t destroy_global_context(void) {

    SAIL_TRY(lock_context());

    SAIL_LOG_DEBUG("Destroyed context %p", global_context);
    destroy_context(global_context);
    global_context = NULL;

    SAIL_TRY(unlock_context());

    return SAIL_OK;
}

sail_status_t fetch_global_context_guarded(struct sail_context **context) {

    SAIL_TRY(fetch_global_context_guarded_with_flags(context, /* flags */ 0));

    return SAIL_OK;
}

sail_status_t fetch_global_context_unsafe(struct sail_context **context) {

    SAIL_TRY(fetch_global_context_unsafe_with_flags(context, /* flags */ 0));

    return SAIL_OK;
}

sail_status_t fetch_global_context_guarded_with_flags(struct sail_context **context, int flags) {

    SAIL_CHECK_PTR(context);

    SAIL_TRY(lock_context());

    SAIL_TRY_OR_CLEANUP(fetch_global_context_unsafe_with_flags(context, flags),
                        /* cleanup */ unlock_context());

    SAIL_TRY(unlock_context());

    return SAIL_OK;
}

sail_status_t fetch_global_context_unsafe_with_flags(struct sail_context **context, int flags) {

    SAIL_CHECK_PTR(context);

    struct sail_context *local_context;

    SAIL_TRY(allocate_global_context(&local_context));
    SAIL_TRY(init_context(local_context, flags));

    *context = local_context;

    return SAIL_OK;
}

sail_status_t sail_unload_codecs_private(void) {

    SAIL_TRY(lock_context());

    if (global_context == NULL) {
        unlock_context();
        SAIL_LOG_DEBUG("Context doesn't exist so not unloading codecs from it");
        return SAIL_OK;
    }

    struct sail_context *context;
    SAIL_TRY_OR_CLEANUP(fetch_global_context_unsafe(&context),
                /* cleanup */ unlock_context());

    int counter = 0;

    for (struct sail_codec_bundle_node *codec_bundle_node = context->codec_bundle_node; codec_bundle_node != NULL; codec_bundle_node = codec_bundle_node->next) {
        struct sail_codec_bundle *codec_bundle = codec_bundle_node->codec_bundle;

        if (codec_bundle->codec != NULL) {
            destroy_codec(codec_bundle->codec);
            codec_bundle->codec = NULL;
            counter++;
        }
    }

    SAIL_TRY(unlock_context());

    SAIL_LOG_DEBUG("Unloaded codecs number: %d", counter);

    return SAIL_OK;
}

sail_status_t lock_context(void) {

    SAIL_TRY(initialize_global_context_guard_mutex());

    SAIL_TRY(threading_lock_mutex(&global_context_guard_mutex));

    return SAIL_OK;
}

sail_status_t unlock_context(void) {

    SAIL_TRY(threading_unlock_mutex(&global_context_guard_mutex));

    return SAIL_OK;
}
