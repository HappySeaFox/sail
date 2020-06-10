- Print more errors on 'return SAIL_XXX'

- Remove SOURCE from the guaranteed output pixel formats. TIFF, for examaple, cannot output SOURCE pixels

- initializer lists in C++

- document c++ classes

- detect images by magic? Non-seekable I/O sources will not be allowed then

- MIT license?

- static compilation

- compile dependencies statically on Windows?

- remove SAIL_CHECK_xxx checks in plugins? Are they needed? libsail does them as well

- distribute cmake rules along with .pc files

- macros to hide boilerplate code in plugins

- what to do when 'features' is 0 and we try to read an image?

- only-, except- cmake runtime defines for bindings ?

- document how SAIL uses plugins APIs

- add benchmarks to README

- enum class in C++?

- log barrier to filter messages under a certain level

- tests
  - check all plugins have the same layout version
  - check all plugins could be loaded
  - decode/encode with reference data
  - all plugins have RGB, RGBA, and SOURCE output
