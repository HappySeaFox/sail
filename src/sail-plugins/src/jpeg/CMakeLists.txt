add_library(jpeg MODULE
            jpeg.c)

# Disable a "lib" prefix
#
set_target_properties(jpeg PROPERTIES PREFIX "")

# Depend on sail-common
#
target_link_libraries(jpeg PRIVATE sail-common)

# JPEG external dependency
#
pkg_check_modules(JPEG REQUIRED IMPORTED_TARGET libjpeg)

target_compile_options(jpeg PRIVATE ${JPEG_CFLAGS})
target_compile_options(jpeg PRIVATE ${JPEG_CFLAGS_OTHER})
target_include_directories(jpeg PRIVATE ${JPEG_INCLUDE_DIRS})
target_link_libraries(jpeg PRIVATE PkgConfig::JPEG)
