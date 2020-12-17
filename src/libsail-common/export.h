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

#ifndef SAIL_EXPORT_H
#define SAIL_EXPORT_H

#ifdef SAIL_BUILD
    #include "config.h"
#else
    #include <sail-common/config.h>
#endif

#if defined _WIN32 || defined __CYGWIN__
    #ifdef SAIL_STATIC
        #define SAIL_IMPORT
    #else
        #define SAIL_IMPORT __declspec(dllimport)
    #endif

    #ifdef SAIL_BUILD
        #define SAIL_EXPORT __declspec(dllexport)
    #else
        #define SAIL_EXPORT SAIL_IMPORT
    #endif

    #define SAIL_HIDDEN
#else
    #define SAIL_EXPORT __attribute__((visibility("default")))
    #define SAIL_IMPORT
    #define SAIL_HIDDEN __attribute__((visibility("hidden")))
#endif

#endif
