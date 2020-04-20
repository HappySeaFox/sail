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

#ifndef SAIL_CONTEXT_CPP_H
#define SAIL_CONTEXT_CPP_H

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail/error.h>
    #include <sail/export.h>
#endif

#include <vector>

struct sail_context;

namespace sail
{

class plugin_info;

/*
 * A C++ interface to struct sail_context.
 */
class SAIL_EXPORT context
{
public:
    context();
    ~context();

    /*
     * Returns true if SAIL was initialized successfully. Using SAIL when this
     * function returns false has no sense as most methods will return errors.
     */
    bool is_valid() const;

    std::vector<plugin_info> plugin_info_list() const;

    sail_context* to_sail_context() const;

private:
    sail_error_t init();

private:
    class pimpl;
    pimpl * const d;
};

}

#endif
