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

#ifndef SAIL_RESOLUTION_CPP_H
#define SAIL_RESOLUTION_CPP_H

#include <memory>

#ifdef SAIL_BUILD
    #include "error.h"
    #include "export.h"
#else
    #include <sail-common/error.h>
    #include <sail-common/export.h>
#endif

struct sail_resolution;

namespace sail
{

/*
 * resolution represents image resolution unit and values.
 */
class SAIL_EXPORT resolution
{
    friend class image;

public:
    /*
     * Constructs a new resolution with unknown units and zero values.
     */
    resolution();

    /*
     * Constructs a new resolution with the specified unit and values.
     */
    resolution(SailResolutionUnit unit, double x, double y);

    /*
     * Copies the resolution.
     */
    resolution(const resolution &res);

    /*
     * Copies the resolution.
     */
    resolution& operator=(const resolution &res);

    /*
     * Moves the resolution.
     */
    resolution(resolution &&res) noexcept;

    /*
     * Moves the resolution.
     */
    resolution& operator=(resolution &&res) noexcept;

    /*
     * Destroys the resolution.
     */
    ~resolution();

    /*
     * Returns true if the resolution has known units and positive x/y values.
     */
    bool is_valid() const;

    /*
     * Returns the resolution unit.
     */
    SailResolutionUnit unit() const;

    /*
     * Returns the resolution x value.
     */
    double x() const;

    /*
     * Returns the resolution y value.
     */
    double y() const;

    /*
     * Sets a new resolution units.
     */
    void set_unit(SailResolutionUnit unit);

    /*
     * Sets a new resolution x value.
     */
    void set_x(double x);

    /*
     * Sets a new resolution y value.
     */
    void set_y(double y);

    /*
     * Returns a string representation of the specified resolution unit. See SailResolutionUnit.
     * For example: "Micrometer" is returned for SAIL_RESOLUTION_UNIT_MICROMETER.
     *
     * Returns NULL if the resolution unit is not known.
     */
    static const char* resolution_unit_to_string(SailResolutionUnit resolution_unit);

    /*
     * Returns a resolution unit from the string representation. See SailResolutionUnit.
     * For example: SAIL_RESOLUTION_UNIT_MICROMETER is returned for "Micrometer".
     *
     * Returns SAIL_RESOLUTION_UNIT_UNKNOWN if the resolution unit is not known.
     */
    static SailResolutionUnit resolution_unit_from_string(const std::string &str);

private:
    /*
     * Makes a deep copy of the specified resolution.
     */
    explicit resolution(const sail_resolution *res);

    sail_status_t to_sail_resolution(sail_resolution **resolution) const;

private:
    class pimpl;
    std::unique_ptr<pimpl> d;
};

}

#endif
