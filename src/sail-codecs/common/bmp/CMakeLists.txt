add_library(bmp-common STATIC
                helpers.h
                helpers.c
                bmp.h
                bmp.c)

set_target_properties(bmp-common PROPERTIES
                                 VERSION "1.0.0"
                                 SOVERSION 1)

target_include_directories(bmp-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(bmp-common PRIVATE sail-common)
