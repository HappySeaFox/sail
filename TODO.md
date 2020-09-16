## Priority 1

- Move back to DLLs for dependencies? Rename them as libpng16-sail.dll?

- Read/write indexed PNGs with alpha

- Make meta info keys as enum

- document c++ classes

- static compilation

- remove SAIL_CHECK_xxx checks in codecs? Are they needed? libsail does them as well

- C++ move ctors

- add benchmarks to README

- tests
  - check all codecs have the same layout version
  - check all codecs could be loaded
  - decode/encode with reference data
  - all codecs have RGB and RGBA output

## Priority 2

- Add `sail` command line tool? Usa-case: `sail list -v` to list all the installed codecs

- Print more errors on 'return SAIL_XXX'

- initializer lists in C++

- distribute cmake rules along with .pc files

- macros to hide boilerplate code in codecs

## Priority 3

- what to do when 'features' is 0 and we try to read an image?

- only-, except- cmake runtime defines for bindings ?

- document how SAIL uses codecs APIs

- enum class in C++?
