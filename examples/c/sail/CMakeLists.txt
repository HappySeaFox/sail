add_executable(sail-app sail.c)

set_target_properties(sail-app PROPERTIES OUTPUT_NAME sail)

# Depend on sail
#
target_link_libraries(sail-app PRIVATE sail)

# Depend on sail-manip
#
target_link_libraries(sail-app PRIVATE sail-manip)

# Enable ASAN if possible
#
sail_enable_asan(TARGET sail-app)

# Install PDB
#
if (SAIL_INSTALL_PDB)
    sail_install_pdb(TARGET sail-app RENAME sail.app.pdb)
endif()

# Installation
#
install(TARGETS sail-app DESTINATION "${CMAKE_INSTALL_BINDIR}")
