add_library(sail-munit STATIC
                munit.c)

set_target_properties(sail-munit PROPERTIES
                                 VERSION "0.4.1"
                                 SOVERSION 0)

# Definitions, includes, link
#
target_include_directories(sail-munit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
