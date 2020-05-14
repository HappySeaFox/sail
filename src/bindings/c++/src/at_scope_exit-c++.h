/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

#ifndef SAIL_AT_SCOPE_EXIT_CPP_H
#define SAIL_AT_SCOPE_EXIT_CPP_H

namespace sail
{

template<typename F>
class scope_cleanup
{
public:
    scope_cleanup(F f)
        : m_f(f)
    {
    }
    ~scope_cleanup()
    {
        m_f();
    }

private:
    F m_f;
};

}

/*
 * Executes the specified code when the scope exits. This macro could be used to perform
 * complex cleanup procedures.
 *
 * For example:
 *
 *    SAIL_AT_SCOPE_EXIT (
 *        delete image;
 *        delete data;
 *    );
 *
 *    SAIL_TRY(...);
 *    SAIL_TRY(...);
 */
#define SAIL_AT_SCOPE_EXIT(code)                           \
    auto lambda = [&] {                                    \
        code                                               \
    };                                                     \
    sail::scope_cleanup<decltype(lambda)> scp_ext(lambda); \
do {                                                       \
    (void)scp_ext;                                         \
} while(0)

#endif
