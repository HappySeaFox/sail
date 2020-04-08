add_library(jpeg MODULE
            jpeg.c)

# Disable a "lib" prefix
#
set_target_properties(jpeg PROPERTIES PREFIX "")

# Depend on sail-common
#
target_link_libraries(jpeg PRIVATE sail-common)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/jpeg.plugin.info
                ${CMAKE_CURRENT_BINARY_DIR}/jpeg.plugin.info
                COPYONLY)

# JPEG external dependency
#
target_compile_options(jpeg PRIVATE ${libjpeg_CFLAGS})
target_compile_options(jpeg PRIVATE ${libjpeg_CFLAGS_OTHER})
target_include_directories(jpeg PRIVATE ${libjpeg_INCLUDE_DIRS})
target_link_libraries(jpeg PRIVATE PkgConfig::libjpeg)

# Installation
#
install(TARGETS jpeg DESTINATION lib/sail/plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/jpeg.plugin.info DESTINATION lib/sail/plugins)
