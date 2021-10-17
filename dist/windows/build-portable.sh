#!/bin/sh -e

cd "$(dirname "$0")"

rm -rf build
mkdir -p build
cd build

install_root="$PWD/install-root"
source_root="../../.."

clean()
{
    rm -rf "$install_root"
    rm -f CMakeCache.txt
}

build_install()
{
    local build_type="$1"

    if [ -z "$build_type" ]; then
        echo "Error: Build type is empty" >&2
        exit 1
    fi

    "$source_root/extra/build" $build_type
    cmake -A x64 -S "$source_root" -DCMAKE_INSTALL_PREFIX="$install_root" -DSAIL_DEV=OFF -DCMAKE_BUILD_TYPE="$build_type" -DSAIL_BUILD_TESTS=ON -DSAIL_COMBINE_CODECS=ON ..
    cmake --build . --config "$build_type" --target install

    cd tests
    ctest -C "$build_type"
    cd -
}

make_portable()
{
    ./make-portable.sh
}

clean
build_install Debug
make_portable

clean
build_install Release
make_portable

echo
echo Success
echo
