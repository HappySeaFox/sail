# Intended to be included by SAIL libraries to install their .cmake configs
#
macro(sail_install_cmake_config)
    cmake_parse_arguments(SAIL_CMAKE_CONFIG "" "TARGET;VERSION;FOLDER" "" ${ARGN})

    install(EXPORT ${SAIL_CMAKE_CONFIG_TARGET}Targets
            FILE ${SAIL_CMAKE_CONFIG_TARGET}Targets.cmake
            NAMESPACE SAIL::
            DESTINATION lib/cmake/${SAIL_CMAKE_CONFIG_FOLDER})

    include(CMakePackageConfigHelpers)

    configure_package_config_file(cmake/${SAIL_CMAKE_CONFIG_TARGET}Config.cmake.in
                                  ${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CMAKE_CONFIG_TARGET}Config.cmake
                                  INSTALL_DESTINATION lib/cmake/sail
                                  NO_SET_AND_CHECK_MACRO
                                  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CMAKE_CONFIG_TARGET}ConfigVersion.cmake
                                     VERSION ${SAIL_CMAKE_CONFIG_VERSION}
                                     COMPATIBILITY SameMajorVersion)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CMAKE_CONFIG_TARGET}Config.cmake
                  ${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CMAKE_CONFIG_TARGET}ConfigVersion.cmake
            DESTINATION lib/cmake/${SAIL_CMAKE_CONFIG_FOLDER})
endmacro()
