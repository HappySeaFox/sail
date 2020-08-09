## Priority 1

- Add warnings with Clang

- Read/write indexed PNGs with alpha

- Make meta info keys as enum

- document c++ classes

- static compilation

- remove SAIL_CHECK_xxx checks in plugins? Are they needed? libsail does them as well

- C++ move ctors

- add benchmarks to README

- tests
  - check all plugins have the same layout version
  - check all plugins could be loaded
  - decode/encode with reference data
  - all plugins have RGB and RGBA output

## Priority 2

- Add `sail` command line tool? Usa-case: `sail list -v` to list all the installed plugins

- Print more errors on 'return SAIL_XXX'

- initializer lists in C++

- distribute cmake rules along with .pc files

- macros to hide boilerplate code in plugins

## Priority 3

- what to do when 'features' is 0 and we try to read an image?

- only-, except- cmake runtime defines for bindings ?

- document how SAIL uses plugins APIs

- enum class in C++?
