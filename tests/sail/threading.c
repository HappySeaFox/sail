/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

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

#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE thread_t;
typedef DWORD(WINAPI* thread_func_t)(LPVOID);
#define THREAD_RETURN DWORD WINAPI
#define THREAD_RETURN_VALUE 0
#else
#include <pthread.h>
typedef pthread_t thread_t;
typedef void* (*thread_func_t)(void*);
#define THREAD_RETURN void*
#define THREAD_RETURN_VALUE NULL
#endif

#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

#define NUM_THREADS 4

struct thread_data
{
    const char* path;
    bool success;
    int thread_id;
};

/* Platform-independent thread creation */
static int create_thread(thread_t* thread, thread_func_t func, void* arg)
{
#ifdef _WIN32
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
#else
    return pthread_create(thread, NULL, func, arg);
#endif
}

/* Platform-independent thread join */
static void join_thread(thread_t thread)
{
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_join(thread, NULL);
#endif
}

/* Thread function that loads an image */
static THREAD_RETURN load_image_thread(void* arg)
{
    struct thread_data* data = (struct thread_data*)arg;

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(data->path, &image);

    data->success = (status == SAIL_OK && image != NULL);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return THREAD_RETURN_VALUE;
}

/* Test concurrent loading of different images */
static MunitResult test_threading_concurrent_loads(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    thread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        thread_data[i].path      = SAIL_TEST_IMAGES[i];
        thread_data[i].success   = false;
        thread_data[i].thread_id = i;

        int result = create_thread(&threads[i], load_image_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        join_thread(threads[i]);
        munit_assert(thread_data[i].success);
    }

    return MUNIT_OK;
}

/* Thread function that loads same image */
static THREAD_RETURN load_same_image_thread(void* arg)
{
    struct thread_data* data = (struct thread_data*)arg;

    struct sail_image* image = NULL;
    sail_status_t status     = sail_load_from_file(data->path, &image);

    data->success = (status == SAIL_OK && image != NULL);

    if (image != NULL)
    {
        sail_destroy_image(image);
    }

    return THREAD_RETURN_VALUE;
}

/* Test concurrent loading of same image */
static MunitResult test_threading_same_image_loads(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    const char* path = SAIL_TEST_IMAGES[0];

    thread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data[i].path      = path;
        thread_data[i].success   = false;
        thread_data[i].thread_id = i;

        int result = create_thread(&threads[i], load_same_image_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        join_thread(threads[i]);
        munit_assert(thread_data[i].success);
    }

    return MUNIT_OK;
}

/* Thread function for codec info queries */
static THREAD_RETURN codec_info_thread(void* arg)
{
    struct thread_data* data = (struct thread_data*)arg;

    const struct sail_codec_info* codec_info = NULL;
    sail_status_t status                     = sail_codec_info_from_path(data->path, &codec_info);

    data->success = (status == SAIL_OK && codec_info != NULL);

    return THREAD_RETURN_VALUE;
}

/* Test concurrent codec info queries */
static MunitResult test_threading_codec_info_queries(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    thread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        thread_data[i].path      = SAIL_TEST_IMAGES[i];
        thread_data[i].success   = false;
        thread_data[i].thread_id = i;

        int result = create_thread(&threads[i], codec_info_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        join_thread(threads[i]);
        munit_assert(thread_data[i].success);
    }

    return MUNIT_OK;
}

/* Thread function for advanced API loading */
static THREAD_RETURN advanced_load_thread(void* arg)
{
    struct thread_data* data = (struct thread_data*)arg;

    void* state          = NULL;
    sail_status_t status = sail_start_loading_from_file(data->path, NULL, &state);

    if (status == SAIL_OK)
    {
        struct sail_image* image = NULL;
        status                   = sail_load_next_frame(state, &image);

        if (status == SAIL_OK && image != NULL)
        {
            data->success = true;
            sail_destroy_image(image);
        }

        sail_stop_loading(state);
    }

    return THREAD_RETURN_VALUE;
}

/* Test concurrent advanced API usage */
static MunitResult test_threading_advanced_api(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    thread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        thread_data[i].path      = SAIL_TEST_IMAGES[i];
        thread_data[i].success   = false;
        thread_data[i].thread_id = i;

        int result = create_thread(&threads[i], advanced_load_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        join_thread(threads[i]);
        munit_assert(thread_data[i].success);
    }

    return MUNIT_OK;
}

/* Test concurrent context initialization race condition */
static MunitResult test_threading_context_init_race(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    sail_finish();

    thread_t threads[NUM_THREADS];
    struct thread_data thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        thread_data[i].path      = SAIL_TEST_IMAGES[i];
        thread_data[i].success   = false;
        thread_data[i].thread_id = i;

        int result = create_thread(&threads[i], load_image_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < NUM_THREADS && SAIL_TEST_IMAGES[i] != NULL; i++)
    {
        join_thread(threads[i]);
        munit_assert(thread_data[i].success);
    }

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/advanced-api",       test_threading_advanced_api,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-info-queries", test_threading_codec_info_queries, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/concurrent-loads",   test_threading_concurrent_loads,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/context-init-race",  test_threading_context_init_race,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/same-image-loads",   test_threading_same_image_loads,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/threading", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
