add_executable(sail-convert sail-convert.c)

# Depend on sail
#
target_link_libraries(sail-convert PRIVATE sail)

# Enable ASAN
if (SAIL_ASAN)
    target_compile_options(sail-convert PRIVATE "${SAIL_ASAN}")
    target_link_libraries(sail-convert PRIVATE "${SAIL_ASAN}")
endif()

# Installation
#
install(TARGETS sail-convert DESTINATION "${CMAKE_INSTALL_BINDIR}")
