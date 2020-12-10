add_executable(sail-convert sail-convert.c)

# Depend on sail
#
target_link_libraries(sail-convert PRIVATE sail)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-convert)

# Installation
#
install(TARGETS sail-convert DESTINATION "${CMAKE_INSTALL_BINDIR}")
