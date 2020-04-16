add_executable(sail-probe sail-probe.c)

# Depend on sail
#
target_link_libraries(sail-probe sail)

# Enable ASAN
#
if (SAIL_ASAN)
    if (CMAKE_COMPILER_IS_GNUCC)
        target_compile_options(sail-probe PRIVATE "${SAIL_ASAN}")
        target_link_libraries(sail-probe "${SAIL_ASAN}")
    else()
        message("ASAN is not supported on this platform.")
    endif()
endif()

# Installation
#
install(TARGETS sail-probe DESTINATION "${CMAKE_INSTALL_BINDIR}")
