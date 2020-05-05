# Intended to be included by every plugin. Sets up necessary dependencies,
# installation targets, plugin info.
#
macro(sail_plugin)
    cmake_parse_arguments(SAIL_PLUGIN "" "TARGET" "SOURCES;PKGCONFIG" ${ARGN})

    # Add a plugin
    #
    add_library("${SAIL_PLUGIN_TARGET}" MODULE "${SAIL_PLUGIN_SOURCES}")

    # Disable a "lib" prefix
    #
    set_target_properties("${SAIL_PLUGIN_TARGET}" PROPERTIES PREFIX "")

    # Depend on sail-common
    #
    target_link_libraries("${SAIL_PLUGIN_TARGET}" PRIVATE sail-common)

    # Pkg-config dependencies
    #
    foreach(pkgconfig ${SAIL_PLUGIN_PKGCONFIG})
        target_compile_options("${SAIL_PLUGIN_TARGET}"     PRIVATE "${${pkgconfig}_CFLAGS}")
        target_compile_options("${SAIL_PLUGIN_TARGET}"     PRIVATE "${${pkgconfig}_CFLAGS_OTHER}")
        target_include_directories("${SAIL_PLUGIN_TARGET}" PRIVATE "${${pkgconfig}_INCLUDE_DIRS}")
        target_link_libraries("${SAIL_PLUGIN_TARGET}"      PRIVATE "PkgConfig::${pkgconfig}")
    endforeach()

    # Copy .plugin.info into the build dir
    #
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_PLUGIN_TARGET}.plugin.info"
                   "${CMAKE_CURRENT_BINARY_DIR}/${SAIL_PLUGIN_TARGET}.plugin.info"
                   COPYONLY)
    # Installation
    #
    install(TARGETS "${SAIL_PLUGIN_TARGET}" DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/plugins")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_PLUGIN_TARGET}.plugin.info"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/plugins")
endmacro()
