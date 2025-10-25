## Building SAIL

### Conan

SAIL is available in [Conan](https://conan.io/center/recipes/sail) on all supported platforms.

### VCPKG

SAIL is available in VCPKG on Windows, Linux, and macOS:

```
vcpkg install sail
```

### CMake options overview

- `BUILD_SHARED_LIBS=ON|OFF` - Build shared libraries. When disabled, automatically sets `SAIL_COMBINE_CODECS` to `ON`. Default: `ON`
- `SAIL_BUILD_APPS=ON|OFF` - Build client applications. Default: `ON`
- `SAIL_BUILD_BINDINGS=ON|OFF` - Build C++ and Python bindings. Default: `ON`
- `SAIL_BUILD_EXAMPLES=ON|OFF` - Build examples. Default: `ON`
- `SAIL_COLORED_OUTPUT=ON|OFF` - Enable colored console output on Windows 10+ and Unix platforms. Default: `ON`
- `SAIL_COMBINE_CODECS=ON|OFF` - Combine all codecs into a single library. Static builds automatically set this to `ON`. Default: `OFF`
- `SAIL_DEV=ON|OFF` - Enable developer mode with pedantic warnings and optional `ASAN` for examples. Default: `OFF`
- `SAIL_DISABLE_CODECS="a;b;c"` - Disable the codecs specified in this ';'-separated list. Supports individual codecs and codec groups by priority (e.g., `highest-priority;xbm`). Default: empty list
- `SAIL_ENABLE_CODECS="a;b;c"` - Force-enable the codecs specified in this ';'-separated list. Configuration fails if an enabled codec cannot find its dependencies. Supports individual codecs and codec groups by priority (e.g., `highest-priority;xbm`). Other codecs may be enabled based on available dependencies. When set, `SAIL_ONLY_CODECS` is ignored. Default: empty list
- `SAIL_ENABLE_OPENMP=ON|OFF` - Enable OpenMP support if available in the compiler. Default: `ON`
- `SAIL_ONLY_CODECS="a;b;c"` - Force-enable only the codecs specified in this ';'-separated list and disable all others. Configuration fails if an enabled codec cannot find its dependencies. Supports individual codecs and codec groups by priority (e.g., `highest-priority;xbm`). Default: empty list
- `SAIL_OPENMP_SCHEDULE="dynamic"` - OpenMP scheduling algorithm. Default: `dynamic`
- `SAIL_THIRD_PARTY_CODECS_PATH=ON|OFF` - Enable loading custom codecs from ';'-separated paths specified in the `SAIL_THIRD_PARTY_CODECS_PATH` environment variable. Default: `ON`
- `SAIL_THREAD_SAFE=ON|OFF` - Enable thread-safe operation by locking the internal context with a mutex. Default: `ON`
- `SAIL_WINDOWS_INSTALL_PDB=ON|OFF` - Install PDB debug files along with libraries (MSVC only). Default: `ON`
- `SAIL_WINDOWS_STATIC_CRT=ON|OFF` - Use static CRT (`/MT`) instead of dynamic CRT (`/MD`) for static builds (MSVC only). Default: `ON`
- `SAIL_WINDOWS_UTF8_PATHS=ON|OFF` - Convert file paths to UTF-8 on Windows. Default: `ON`

### Windows

#### Tested environments

- Windows 10 x64
- Windows 11 x64

#### Build requirements

- Git
- CMake 3.18 or later
- MSVC 2019 or later

#### Build steps

Open `Git Bash` (installed with Git) and execute the following commands:

```
git clone --recursive https://github.com/HappySeaFox/sail.git
cd sail

# Compile third-party dependencies
cd extra
./build
cd ..

# Compile SAIL
mkdir build
cd build
cmake -A x64 -DCMAKE_INSTALL_PREFIX="C:\SAIL" ..
cmake --build . --config Release

# Install
cmake --build . --config Release --target install
```

### macOS

#### Tested environments

- macOS 10.14 Mojave
- macOS 10.15 Catalina
- macOS 11.3 Big Sur

#### Installation steps

```
brew install HappySeaFox/sail/sail
```

Or

```
brew upgrade sail
```

### Linux

#### Tested environments

- LUbuntu 18.04 64-bit
- LUbuntu 20.04 64-bit

#### Build requirements

- Git
- CMake 3.18 or later
- GCC and G++ 7.5 or later
- Standard C/C++ development files (usually installed by metapackages like `build-essential`)
- Codec-specific development libraries. See `debian/control` for the complete list

#### Build steps

```
git clone --recursive https://github.com/HappySeaFox/sail.git
cd sail

# Install the required dependencies from debian/control
sudo apt install ...

# Compile SAIL
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Install (distro-specific)
sudo make install
```

Debian packaging rules are also provided.
