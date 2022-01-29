set(BMP_COMMON_SOURCE
        helpers.h
        helpers.c
        bmp.h
        bmp.c)

if (SAIL_COMBINE_CODECS)
    add_library(bmp-common OBJECT ${BMP_COMMON_SOURCE})
    set_target_properties(bmp-common PROPERTIES
            SAIL_COMMON_OBJECT_LIBRARY TRUE)
else()
    add_library(bmp-common STATIC ${BMP_COMMON_SOURCE})
    set_target_properties(bmp-common PROPERTIES
            VERSION "1.0.0"
            SOVERSION 1)
endif()

target_include_directories(bmp-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(bmp-common PRIVATE sail-common)
