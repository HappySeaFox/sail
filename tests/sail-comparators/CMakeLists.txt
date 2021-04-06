add_library(sail-comparators STATIC
                sail-comparators.h
                sail-comparators.c)

set_target_properties(sail-comparators PROPERTIES
                                        VERSION "1.0.0"
                                        SOVERSION 1)

# Definitions, includes, link
#
target_include_directories(sail-comparators PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sail-comparators PRIVATE sail-common)
target_link_libraries(sail-comparators PRIVATE sail-munit)
