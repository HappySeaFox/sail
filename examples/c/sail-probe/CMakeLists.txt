add_executable(sail-probe sail-probe.c)

# Depend on sail
#
target_link_libraries(sail-probe PRIVATE sail)

# Installation
#
install(TARGETS sail-probe DESTINATION "${CMAKE_INSTALL_BINDIR}")
