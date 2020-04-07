option(SAIL_NO_COLOR "Disable colored console output" OFF)

add_library(sail-common
                src/common.c
                src/meta_entry_node.c
                src/ini.c
                src/log.c
                src/plugin.c
                src/utils.c)

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.5.0"
                                  SOVERSION 0)

if (SAIL_NO_COLOR)
    target_compile_definitions(sail-common PRIVATE SAIL_NO_COLOR=1)
endif()

target_compile_definitions(sail-common PRIVATE INI_STOP_ON_FIRST_ERROR=1)

target_include_directories(sail-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Headers-only interface library for internal projects that need just headers
#
add_library(sail-common-headers INTERFACE)

target_include_directories(sail-common-headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
