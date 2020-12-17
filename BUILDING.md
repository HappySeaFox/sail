## Building SAIL

Consider [EXAMPLES](EXAMPLES.md) after building and installing.

### Terminology and build types

**Standalone build** - manually compiled with `cmake` and respective build commands. Extra dependencies like libjpeg ARE NOT included into the build. macOS brew package is a good example of a standalone build.

**Standalone bundle** - manually compiled with `cmake` and respective build commands. Extra dependencies like libjpeg ARE included into the build.

**VCPKG port** - installed with `vcpkg install sail`.

### CMake options overview

- `SAIL_BUILD_EXAMPLES=ON|OFF` - Build examples. Default: `ON`
- `SAIL_BUILD_TESTS=ON|OFF` - Build tests. Default: `ON`
- `SAIL_COLORED_OUTPUT=ON|OFF` - Enable colored console output on Windows >= 10 and Unix platforms. Default: `ON`
- `SAIL_COMBINE_CODECS=ON|OFF` - Combine all codecs into a single library. Static build always sets this option to ON. Default: `OFF`
- `SAIL_DEV=ON|OFF` - Enable developer mode with pedantic warnings and possible `ASAN` enabled for examples. Default: `OFF`
- `SAIL_EXCEPT_CODECS="a;b;c"` - Enable all codecs except the codecs specified in this ';'-separated list.
  Codecs with missing dependencies will be disabled regardless this setting. Default: empty list
- `SAIL_ONLY_CODECS="a;b;c"` - Enable only the codecs specified in this ';'-separated list.
  Codecs with missing dependencies will be disabled regardless this setting. Default: empty list
- `SAIL_READ_OUTPUT_BPP32_BGRA=ON|OFF` - Make the read operations output BPP32-BGRA pixels instead of BPP32-RGBA. Default: `OFF`
- `SAIL_STATIC=ON|OFF` - Enable static build. Default: `OFF`

### VCPKG

SAIL is available in VCPKG on Windows, Linux, and macOS:

```
vcpkg install sail
```

### Windows (standalone bundle)

#### Tested environments

- Windows 7 x64

#### Build requirements

- git
- cmake 3.10 or later
- MSVC 2019 or later

#### Build steps

Open `Git Bash` (installed along with `git`) and execute the following commands:

```
git clone --recursive https://github.com/smoked-herring/sail.git
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

### macOS (standalone build)

#### Tested environments

- macOS 10.14 Mojave
- macOS 10.15 Catalina

#### Installation steps

```
brew install smoked-herring/sail/sail
```

Or

```
brew upgrade sail
```

### Linux (standalone build)

#### Tested environments

- LUbuntu 18.04 64-bit
- LUbuntu 20.04 64-bit

#### Build requirements

- git
- cmake 3.10 or later
- GCC and G++ 7.5 or later
- standard C/C++ development files installed (usually installed by metapackages like `build-essential`)
- codec-specific development libraries installed. You can grab the list from `debian/control`

#### Build steps

```
git clone --recursive https://github.com/smoked-herring/sail.git
cd sail

# Install the required dependencies grabbed from debian/control
sudo apt install ...

# Compile SAIL
mkdir build
cd build
cmake ..
make

# Install
... distro-specific installation
```

Debian rules are provided as well.
