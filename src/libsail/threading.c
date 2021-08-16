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

#include "sail.h"

#ifdef _MSC_VER
struct callback_holder
{
    sail_status_t (*callback)(void);
};

static BOOL CALLBACK OnceHandler(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    (void)InitOnce;
    (void)lpContext;

    struct callback_holder *callback_holder = (struct callback_holder *)Parameter;

    return callback_holder->callback() == SAIL_OK;
}
#endif

sail_status_t sail_call_once(sail_once_flag_t *once_flag, sail_status_t (*callback)(void))
{
    SAIL_CHECK_PTR(once_flag);

#ifdef _MSC_VER
    struct callback_holder callback_holder = { callback };
    PVOID lpContext;

    if (InitOnceExecuteOnce(once_flag, OnceHandler, &callback_holder, &lpContext)) {
        return SAIL_OK;
    } else {
        SAIL_LOG_ERROR("Failed to execute call_once. Error: 0x%X", GetLastError());
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#else
    if (pthread_once(once_flag, (void (*)(void))callback) == 0) {
        return SAIL_OK;
    } else {
        SAIL_TRY(sail_print_errno("Failed to execute call_once: %s"));
        SAIL_LOG_AND_RETURN(SAIL_ERROR_INVALID_ARGUMENT);
    }
#endif
}
