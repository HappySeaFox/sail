add_library(sail-dump STATIC
                sail-dump.h
                sail-dump.c)

set_target_properties(sail-dump PROPERTIES
                                VERSION "1.0.0"
                                SOVERSION 1)

# Definitions, includes, link
#
target_include_directories(sail-dump PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sail-dump PRIVATE sail-common)

# Application to dump images
#
add_executable(sail-dump-app sail-dump-app.c)
target_link_libraries(sail-dump-app PRIVATE sail sail-dump)
