add_executable(sail-app sail.c)

set_target_properties(sail-app PROPERTIES OUTPUT_NAME sail)

# Depend on sail
#
target_link_libraries(sail-app PRIVATE sail)

# Depend on sail-manip
#
target_link_libraries(sail-app PRIVATE sail-manip)

target_link_libraries(sail-app PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

# Enable POSIX declarations like strtok_r()
#
sail_enable_posix_c_source(TARGET sail-app VERSION 200112L)

# Install PDB
#
if (SAIL_WINDOWS_INSTALL_PDB)
    sail_windows_install_pdb(TARGET sail-app RENAME sail.app.pdb)
endif()

# Installation
#
install(TARGETS sail-app DESTINATION "${CMAKE_INSTALL_BINDIR}")
