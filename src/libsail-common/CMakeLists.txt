add_library(sail-common src/common.c)

target_include_directories(sail-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
