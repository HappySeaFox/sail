# Intended to be included by every codec. Sets up necessary dependencies,
# installation targets, codec info.
#
macro(sail_codec)
    cmake_parse_arguments(SAIL_CODEC "" "TARGET" "SOURCES;PKGCONFIG" ${ARGN})

    # Add a codec
    #
    add_library("${SAIL_CODEC_TARGET}" MODULE "${SAIL_CODEC_SOURCES}")

    # Disable a "lib" prefix
    #
    set_target_properties("${SAIL_CODEC_TARGET}" PROPERTIES PREFIX "")

    # Depend on sail-common
    #
    target_link_libraries("${SAIL_CODEC_TARGET}" PRIVATE sail-common)

    # Pkg-config dependencies
    #
    foreach(pkgconfig ${SAIL_CODEC_PKGCONFIG})
        pkg_check_modules(${pkgconfig} IMPORTED_TARGET REQUIRED ${pkgconfig})

        target_compile_options("${SAIL_CODEC_TARGET}"     PRIVATE "${${pkgconfig}_CFLAGS}")
        target_compile_options("${SAIL_CODEC_TARGET}"     PRIVATE "${${pkgconfig}_CFLAGS_OTHER}")
        target_include_directories("${SAIL_CODEC_TARGET}" PRIVATE "${${pkgconfig}_INCLUDE_DIRS}")
        target_link_libraries("${SAIL_CODEC_TARGET}"      PRIVATE "PkgConfig::${pkgconfig}")
    endforeach()

    # Copy .codec.info into the build dir
    #
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_CODEC_TARGET}.codec.info.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CODEC_TARGET}.codec.info"
                   @ONLY)
    # Installation
    #
    install(TARGETS "${SAIL_CODEC_TARGET}" DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${SAIL_CODEC_TARGET}.codec.info"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")
endmacro()
