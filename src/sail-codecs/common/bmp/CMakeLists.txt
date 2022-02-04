add_library(bmp-common OBJECT
                helpers.h
                helpers.c
                bmp.h
                bmp.c)

target_include_directories(bmp-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_link_libraries(bmp-common PRIVATE sail-common)
