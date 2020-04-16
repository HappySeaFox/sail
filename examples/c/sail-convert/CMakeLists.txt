add_executable(sail-convert sail-convert.c)

# Depend on sail
#
target_link_libraries(sail-convert sail)

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_COMPILER_IS_GNUCC)
        target_compile_options(sail-convert PRIVATE "${SAIL_ASAN}")
        target_link_libraries(sail-convert "${SAIL_ASAN}")
    else()
        message("ASAN is not supported on this platform.")
    endif()
endif()

# Installation
#
install(TARGETS sail-convert DESTINATION "${CMAKE_INSTALL_BINDIR}")
