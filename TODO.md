## Priority 1

- Windows INNO installer with a config file to store SAIL_PLUGINS_PATH

- Read/write indexed PNGs with alpha

- Remove the scanline reading interface and make an interface to read entire frames?

- document c++ classes

- static compilation

- remove SAIL_CHECK_xxx checks in plugins? Are they needed? libsail does them as well

- C++ move ctors

- C++ return const refs

- add benchmarks to README

- tests
  - check all plugins have the same layout version
  - check all plugins could be loaded
  - decode/encode with reference data
  - all plugins have RGB and RGBA output

## Priority 2

- Add HomeBrew tap for MacOS

- Print more errors on 'return SAIL_XXX'

- initializer lists in C++

- distribute cmake rules along with .pc files

- macros to hide boilerplate code in plugins

## Priority 3

- what to do when 'features' is 0 and we try to read an image?

- only-, except- cmake runtime defines for bindings ?

- document how SAIL uses plugins APIs

- enum class in C++?

- log barrier to filter messages under a certain level
