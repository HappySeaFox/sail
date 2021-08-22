/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include <errno.h>

#include "sail.h"

#ifdef SAIL_WIN32
struct callback_holder
{
    void (*callback)(void);
};

static BOOL CALLBACK OnceHandler(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    (void)InitOnce;
    (void)lpContext;

    const struct callback_holder *callback_holder = (struct callback_holder *)Parameter;

    callback_holder->callback();

    return TRUE;
}
#endif

sail_status_t threading_call_once(sail_once_flag_t *once_flag, void (*callback)(void))
{
    SAIL_CHECK_PTR(once_flag);

#ifdef SAIL_WIN32
    struct callback_holder callback_holder = { callback };

    if (SAIL_LIKELY(InitOnceExecuteOnce(once_flag, OnceHandler, &callback_holder, NULL))) {
        return SAIL_OK;
    } else {
        SAIL_LOG_ERROR("Failed to execute call_once. Error: 0x%X", GetLastError());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#else
    if (SAIL_LIKELY((errno = pthread_once(once_flag, callback)) == 0)) {
        return SAIL_OK;
    } else {
        SAIL_TRY(sail_print_errno("Failed to execute call_once: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}

sail_status_t threading_init_mutex(sail_mutex_t *mutex)
{
    SAIL_CHECK_PTR(mutex);

#ifdef SAIL_WIN32
    InitializeCriticalSection(mutex);
    return SAIL_OK;
#else
    pthread_mutexattr_t attr;

    if (SAIL_LIKELY((errno = pthread_mutexattr_init(&attr)) == 0)) {
        if (SAIL_LIKELY((errno = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) == 0)) {
            errno = pthread_mutex_init(mutex, &attr);
            pthread_mutexattr_destroy(&attr);

            if (SAIL_LIKELY(errno == 0)) {
                return SAIL_OK;
            } else {
                SAIL_TRY(sail_print_errno("Failed to initialize mutex: %s"));
                SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
            }
        } else {
            pthread_mutexattr_destroy(&attr);
            SAIL_TRY(sail_print_errno("Failed to set mutex attributes: %s"));
            SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
        }
    } else {
        SAIL_TRY(sail_print_errno("Failed to initialize mutex attributes: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}

sail_status_t threading_lock_mutex(sail_mutex_t *mutex)
{
    SAIL_CHECK_PTR(mutex);

#ifdef SAIL_WIN32
    EnterCriticalSection(mutex);
    return SAIL_OK;
#else
    if (SAIL_LIKELY((errno = pthread_mutex_lock(mutex)) == 0)) {
        return SAIL_OK;
    } else {
        SAIL_TRY(sail_print_errno("Failed to lock mutex: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}

sail_status_t threading_unlock_mutex(sail_mutex_t *mutex)
{
    SAIL_CHECK_PTR(mutex);

#ifdef SAIL_WIN32
    LeaveCriticalSection(mutex);
    return SAIL_OK;
#else
    if (SAIL_LIKELY((errno = pthread_mutex_unlock(mutex)) == 0)) {
        return SAIL_OK;
    } else {
        SAIL_TRY(sail_print_errno("Failed to unlock mutex: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}

sail_status_t threading_destroy_mutex(sail_mutex_t *mutex)
{
    SAIL_CHECK_PTR(mutex);

#ifdef SAIL_WIN32
    DeleteCriticalSection(mutex);
    return SAIL_OK;
#else
    if (SAIL_LIKELY((errno = pthread_mutex_destroy(mutex)) == 0)) {
        return SAIL_OK;
    } else {
        SAIL_TRY(sail_print_errno("Failed to initialize mutex: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}
