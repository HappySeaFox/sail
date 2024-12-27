## Building SAIL

### Conan

SAIL is available in [Conan](https://conan.io/center/recipes/sail) on all supported platforms.

### VCPKG

SAIL is available in VCPKG on Windows, Linux, and macOS:

```
vcpkg install sail
```

### CMake options overview

- `SAIL_BUILD_APPS=ON|OFF` - Build client applications. Default: `ON`
- `SAIL_BUILD_EXAMPLES=ON|OFF` - Build examples. Default: `ON`
- `SAIL_COLORED_OUTPUT=ON|OFF` - Enable colored console output on Windows >= 10 and Unix platforms. Default: `ON`
- `SAIL_COMBINE_CODECS=ON|OFF` - Combine all codecs into a single library. Static build always sets this option to ON. Default: `OFF`
- `SAIL_DEV=ON|OFF` - Enable developer mode with pedantic warnings and possible `ASAN` enabled for examples. Default: `OFF`
- `SAIL_DISABLE_CODECS="a;b;c"` - Disable the codecs specified in this ';'-separated list. One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm. Default: empty list
- `SAIL_ENABLE_CODECS="a;b;c"` - Forcefully enable the codecs specified in this ';'-separated list. If an enabled codec fails to find its dependencies, the configuration process fails. One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm. Other codecs may or may not be enabled depending on found dependencies. When SAIL_ENABLE_CODECS is enabled, SAIL_ONLY_CODECS gets ignored. Default: empty list
- `SAIL_ENABLE_OPENMP=ON|OFF` - Enable OpenMP support if it's available in the compiler. Default: ON
- `SAIL_THIRD_PARTY_CODECS_PATH=ON|OFF` - Enable loading custom codecs from the ';'-separated paths specified in the `SAIL_THIRD_PARTY_CODECS_PATH` environment variable. Default: `ON`
- `SAIL_THREAD_SAFE=ON|OFF` - Enable working in multi-threaded environments by locking the internal context with a mutex. Default: `ON`
- `SAIL_ONLY_CODECS="a;b;c"` - Forcefully enable only the codecs specified in this ';'-separated list and disable the rest. If an enabled codec fails to find its dependencies, the configuration process fails. One can also specify not just individual codecs but codec groups by their priority like that: highest-priority;xbm. Default: empty list
- `SAIL_OPENMP_SCHEDULE="dynamic"` - OpenMP scheduling algorithm. Default: dynamic

- `BUILD_TESTING=ON|OFF` - Enable generation of tests using CTest. Default: `ON`

### Windows

#### Tested environments

- Windows 7 x64

#### Build requirements

- git
- cmake 3.12 or later
- MSVC 2019 or later

#### Build steps

Open `Git Bash` (installed along with `git`) and execute the following commands:

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

#### Package Managers

SAIL is not always available in official distro repositories. Here is an incomplete list of available packages for various distributions:

- Ubuntu 24.04 - `sudo apt-get install sail-codecs libsail-dev libsail-common-dev libsail-manip-dev libsail-c++-dev`
- Arch User Repository - [`sail-img`](https://aur.archlinux.org/packages/sail-img)

#### Build requirements

- git
- cmake 3.12 or later
- GCC and G++ 7.5 or later
- standard C/C++ development files installed (usually installed by metapackages like `build-essential`)
- codec-specific development libraries installed. You can grab the list from `debian/control`

#### Build steps

```
git clone --recursive https://github.com/HappySeaFox/sail.git
cd sail

# Install the required dependencies grabbed from debian/control
sudo apt install ...

# Compile SAIL
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Install
... distro-specific installation
```

Debian rules are provided as well.
