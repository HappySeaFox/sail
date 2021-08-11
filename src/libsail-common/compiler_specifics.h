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

#ifndef SAIL_COMPILER_SPECIFICS_H
#define SAIL_COMPILER_SPECIFICS_H

/* Thread local flag used with static variables. */
#if defined __GNUC__
    #define SAIL_THREAD_LOCAL __thread
#elif defined _MSC_VER
    #define SAIL_THREAD_LOCAL __declspec(thread)
#elif defined __STDC_VERSION__  && __STDC_VERSION__ >= 201112L
    #define SAIL_THREAD_LOCAL _Thread_local
#elif defined _Thread_local
    #define SAIL_THREAD_LOCAL _Thread_local
#else
    /* Syntax error. */
    Do not know how to define thread local variables for this compiler.
#endif

/* Branch predictions. */
#ifdef __GNUC__
    #define SAIL_LIKELY(x)   (__builtin_expect((x), 1))
    #define SAIL_UNLIKELY(x) (__builtin_expect((x), 0))
#else
    #define SAIL_LIKELY(x)   (x)
    #define SAIL_UNLIKELY(x) (x)
#endif

#endif
