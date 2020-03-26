add_library(sail-common src/common.c)

set_target_properties(sail-common PROPERTIES
                                  VERSION "0.5.0"
                                  SOVERSION 0)

target_include_directories(sail-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Headers-only interface library for internal projects that need just headers
#
add_library(sail-common-headers INTERFACE)

target_include_directories(sail-common-headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
