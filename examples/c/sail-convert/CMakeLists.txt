add_executable(sail-convert sail-convert.c)

# Depend on sail
#
target_link_libraries(sail-convert PRIVATE sail)

# Installation
#
install(TARGETS sail-convert DESTINATION "${CMAKE_INSTALL_BINDIR}")
