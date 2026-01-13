#!/bin/bash

#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

set -euo pipefail

cd "$(dirname "$0")"

SCRIPT_DIR="$PWD"
BUILD_DIR="$SCRIPT_DIR/build"

if [ ! -x "$SCRIPT_DIR/test-conversion" ] || [ "$SCRIPT_DIR/test-conversion.cpp" -nt "$SCRIPT_DIR/test-conversion" ]; then
    echo "Compiling test-conversion..."
    g++ -std=c++20 -O2 -Wall -Wextra \
        -I"$BUILD_DIR/include" \
        -I"$SCRIPT_DIR/src" \
        -I"$SCRIPT_DIR/src/bindings/sail-c++" \
        -I"$SCRIPT_DIR/src/bindings" \
        "$SCRIPT_DIR/test-conversion.cpp" \
        -o "$SCRIPT_DIR/test-conversion" \
        -L"$BUILD_DIR/src/sail" \
        -L"$BUILD_DIR/src/sail-manip" \
        -L"$BUILD_DIR/src/sail-common" \
        -L"$BUILD_DIR/src/bindings/sail-c++" \
        -Wl,-rpath,"$BUILD_DIR/src/sail" \
        -Wl,-rpath,"$BUILD_DIR/src/sail-manip" \
        -Wl,-rpath,"$BUILD_DIR/src/sail-common" \
        -Wl,-rpath,"$BUILD_DIR/src/bindings/sail-c++" \
        -lsail-c++ -lsail -lsail-manip -lsail-common \
        -lpthread -lm
fi

input_dir="$1"
target_ext="$2"

if [ $# -eq 3 ]; then
    nproc=$3
else
    nproc=$(nproc)
fi

LD_PRELOAD=/lib/x86_64-linux-gnu/libasan.so.8 "$SCRIPT_DIR/test-conversion" "$input_dir" "$target_ext" "$nproc"
