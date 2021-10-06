add_executable(sail-probe sail-probe.c)

# Depend on sail
#
target_link_libraries(sail-probe PRIVATE sail)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-probe)

# Install PDB
#
sail_install_pdb(TARGET sail-probe)

# Installation
#
install(TARGETS sail-probe DESTINATION "${CMAKE_INSTALL_BINDIR}")
