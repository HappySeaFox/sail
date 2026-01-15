/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)
 *
 *    Copyright (c) 2026 Dmitry Baryshev
 *
 *    The MIT License
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files (the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING WITHOUT LIMITATION THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

#include <sail-manip/convert.h>
#include <sail/sail.h>

#include "munit.h"

#include "tests/images/acceptance/test-images.h"

/* Stress test configuration */
#define STRESS_NUM_THREADS 4
#define STRESS_ITERATIONS_PER_THREAD 5

struct stress_thread_data
{
    const char* const* paths;
    int num_paths;
    int thread_id;
    int iterations;
    volatile int* success_count;
    volatile int* error_count;
};

/* Create a thread that works on both Windows and Unix */
static int create_thread(thread_t* thread, thread_func_t func, void* arg)
{
#ifdef _WIN32
    *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
#else
    return pthread_create(thread, NULL, func, arg);
#endif
}

/* Wait for a thread to finish, works on both Windows and Unix */
static void join_thread(thread_t thread)
{
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_join(thread, NULL);
#endif
}

/* Thread function that loads images repeatedly */
static THREAD_RETURN stress_load_thread(void* arg)
{
    struct stress_thread_data* data = (struct stress_thread_data*)arg;

    for (int i = 0; i < data->iterations; i++)
    {
        /* Cycle through available images */
        const char* path = data->paths[i % data->num_paths];
        if (path == NULL)
        {
            break;
        }

        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_from_file(path, &image);

        if (status == SAIL_OK && image != NULL)
        {
            /* Convert the image to test conversion code paths */
            struct sail_image* converted = NULL;
            sail_status_t conv_status    = sail_convert_image(image, SAIL_PIXEL_FORMAT_BPP24_RGB, &converted);

            if (conv_status == SAIL_OK && converted != NULL)
            {
                sail_destroy_image(converted);
            }

            sail_destroy_image(image);

#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->success_count);
#else
            __sync_add_and_fetch(data->success_count, 1);
#endif
        }
        else
        {
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->error_count);
#else
            __sync_add_and_fetch(data->error_count, 1);
#endif
        }
    }

    return THREAD_RETURN_VALUE;
}

/* Thread function that queries codec info repeatedly */
static THREAD_RETURN stress_codec_info_thread(void* arg)
{
    struct stress_thread_data* data = (struct stress_thread_data*)arg;

    for (int i = 0; i < data->iterations; i++)
    {
        const char* path = data->paths[i % data->num_paths];
        if (path == NULL)
        {
            break;
        }

        const struct sail_codec_info* codec_info = NULL;
        sail_status_t status                     = sail_codec_info_from_path(path, &codec_info);

        if (status == SAIL_OK && codec_info != NULL)
        {
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->success_count);
#else
            __sync_add_and_fetch(data->success_count, 1);
#endif
        }
        else
        {
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->error_count);
#else
            __sync_add_and_fetch(data->error_count, 1);
#endif
        }
    }

    return THREAD_RETURN_VALUE;
}

/* Thread function that uses advanced API repeatedly */
static THREAD_RETURN stress_advanced_api_thread(void* arg)
{
    struct stress_thread_data* data = (struct stress_thread_data*)arg;

    for (int i = 0; i < data->iterations; i++)
    {
        const char* path = data->paths[i % data->num_paths];
        if (path == NULL)
        {
            break;
        }

        void* state          = NULL;
        sail_status_t status = sail_start_loading_from_file(path, NULL, &state);

        if (status == SAIL_OK)
        {
            struct sail_image* image = NULL;
            status                   = sail_load_next_frame(state, &image);

            if (status == SAIL_OK && image != NULL)
            {
                sail_destroy_image(image);
#ifdef _WIN32
                InterlockedIncrement((volatile LONG*)data->success_count);
#else
                __sync_add_and_fetch(data->success_count, 1);
#endif
            }

            sail_stop_loading(state);
        }
        else
        {
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->error_count);
#else
            __sync_add_and_fetch(data->error_count, 1);
#endif
        }
    }

    return THREAD_RETURN_VALUE;
}

/* Test concurrent image loading with many threads and iterations */
static MunitResult test_stress_concurrent_loads(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Count how many test images we have available */
    int num_images = 0;
    while (SAIL_TEST_IMAGES[num_images] != NULL && num_images < 32)
    {
        num_images++;
    }

    if (num_images == 0)
    {
        return MUNIT_SKIP;
    }

    volatile int success_count = 0;
    volatile int error_count   = 0;

    thread_t threads[STRESS_NUM_THREADS];
    struct stress_thread_data thread_data[STRESS_NUM_THREADS];

    /* Set up thread data and start all threads */
    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        thread_data[i].paths         = SAIL_TEST_IMAGES;
        thread_data[i].num_paths     = num_images;
        thread_data[i].thread_id     = i;
        thread_data[i].iterations    = STRESS_ITERATIONS_PER_THREAD;
        thread_data[i].success_count = &success_count;
        thread_data[i].error_count   = &error_count;

        int result = create_thread(&threads[i], stress_load_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    /* Wait until all threads finish */
    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        join_thread(threads[i]);
    }

    /* Make sure we got at least some successful operations */
    munit_assert(success_count > 0);

    return MUNIT_OK;
}

/* Test concurrent codec info queries under stress */
static MunitResult test_stress_codec_info_queries(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    int num_images = 0;
    while (SAIL_TEST_IMAGES[num_images] != NULL && num_images < 32)
    {
        num_images++;
    }

    if (num_images == 0)
    {
        return MUNIT_SKIP;
    }

    volatile int success_count = 0;
    volatile int error_count   = 0;

    thread_t threads[STRESS_NUM_THREADS];
    struct stress_thread_data thread_data[STRESS_NUM_THREADS];

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        thread_data[i].paths         = SAIL_TEST_IMAGES;
        thread_data[i].num_paths     = num_images;
        thread_data[i].thread_id     = i;
        thread_data[i].iterations    = STRESS_ITERATIONS_PER_THREAD;
        thread_data[i].success_count = &success_count;
        thread_data[i].error_count   = &error_count;

        int result = create_thread(&threads[i], stress_codec_info_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        join_thread(threads[i]);
    }

    munit_assert(success_count > 0);

    return MUNIT_OK;
}

/* Test advanced API usage under stress */
static MunitResult test_stress_advanced_api(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    int num_images = 0;
    while (SAIL_TEST_IMAGES[num_images] != NULL && num_images < 32)
    {
        num_images++;
    }

    if (num_images == 0)
    {
        return MUNIT_SKIP;
    }

    volatile int success_count = 0;
    volatile int error_count   = 0;

    thread_t threads[STRESS_NUM_THREADS];
    struct stress_thread_data thread_data[STRESS_NUM_THREADS];

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        thread_data[i].paths         = SAIL_TEST_IMAGES;
        thread_data[i].num_paths     = num_images;
        thread_data[i].thread_id     = i;
        thread_data[i].iterations    = STRESS_ITERATIONS_PER_THREAD;
        thread_data[i].success_count = &success_count;
        thread_data[i].error_count   = &error_count;

        int result = create_thread(&threads[i], stress_advanced_api_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        join_thread(threads[i]);
    }

    munit_assert(success_count > 0);

    return MUNIT_OK;
}

/* Thread function that does operations with shared context */
static THREAD_RETURN stress_shared_context_thread(void* arg)
{
    struct stress_thread_data* data = (struct stress_thread_data*)arg;

    /* Do many operations with shared context */
    for (int i = 0; i < data->iterations; i++)
    {
        const char* path = data->paths[i % data->num_paths];
        if (path == NULL)
        {
            break;
        }

        struct sail_image* image = NULL;
        sail_status_t status     = sail_load_from_file(path, &image);

        if (status == SAIL_OK && image != NULL)
        {
            sail_destroy_image(image);
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->success_count);
#else
            __sync_add_and_fetch(data->success_count, 1);
#endif
        }
        else
        {
#ifdef _WIN32
            InterlockedIncrement((volatile LONG*)data->error_count);
#else
            __sync_add_and_fetch(data->error_count, 1);
#endif
        }
    }

    return THREAD_RETURN_VALUE;
}

/* Test operations with shared context initialized once */
static MunitResult test_stress_shared_context(const MunitParameter params[], void* user_data)
{
    (void)params;
    (void)user_data;

    /* Make sure we start with a clean state */
    sail_finish();

    int num_images = 0;
    while (SAIL_TEST_IMAGES[num_images] != NULL && num_images < 32)
    {
        num_images++;
    }

    if (num_images == 0)
    {
        return MUNIT_SKIP;
    }

    /* Initialize context once for all threads */
    sail_status_t init_status = sail_init();
    if (init_status != SAIL_OK)
    {
        return MUNIT_ERROR;
    }

    volatile int success_count = 0;
    volatile int error_count   = 0;

    thread_t threads[STRESS_NUM_THREADS];
    struct stress_thread_data thread_data[STRESS_NUM_THREADS];

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        thread_data[i].paths         = SAIL_TEST_IMAGES;
        thread_data[i].num_paths     = num_images;
        thread_data[i].thread_id     = i;
        thread_data[i].iterations    = STRESS_ITERATIONS_PER_THREAD;
        thread_data[i].success_count = &success_count;
        thread_data[i].error_count   = &error_count;

        int result = create_thread(&threads[i], stress_shared_context_thread, &thread_data[i]);
        munit_assert(result == 0);
    }

    for (int i = 0; i < STRESS_NUM_THREADS; i++)
    {
        join_thread(threads[i]);
    }

    /* Clean up context after all threads finished */
    sail_finish();

    munit_assert(success_count > 0);

    return MUNIT_OK;
}

// clang-format off
static MunitTest test_suite_tests[] = {
    { (char *)"/concurrent-loads",   test_stress_concurrent_loads,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/codec-info-queries", test_stress_codec_info_queries, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/advanced-api",       test_stress_advanced_api,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/shared-context",     test_stress_shared_context,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/threading-stress", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
// clang-format on

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
