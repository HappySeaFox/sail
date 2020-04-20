/*  This file is part of SAIL (https://github.com/sailor-keg/sail)

    Copyright (c) 2020 Dmitry Baryshev <dmitrymq@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SAIL_EXPORT_H
#define SAIL_EXPORT_H

#if defined _WIN32 || defined __CYGWIN__
    #ifdef SAIL_BUILD
        #define SAIL_EXPORT __declspec(dllexport)
    #else
        #define SAIL_EXPORT __declspec(dllimport)
    #endif

    #define SAIL_HIDDEN
#else
    #define SAIL_EXPORT __attribute__((visibility("default")))
    #define SAIL_HIDDEN __attribute__((visibility("hidden")))
#endif

#endif
