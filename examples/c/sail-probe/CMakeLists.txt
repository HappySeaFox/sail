add_executable(sail-probe sail-probe.c)

# Depend on sail
#
target_link_libraries(sail-probe PRIVATE sail)

# Enable ASAN
if (SAIL_ASAN)
    target_compile_options(sail-probe PRIVATE "${SAIL_ASAN}")
    target_link_libraries(sail-probe PRIVATE "${SAIL_ASAN}")
endif()

# Installation
#
install(TARGETS sail-probe DESTINATION "${CMAKE_INSTALL_BINDIR}")
