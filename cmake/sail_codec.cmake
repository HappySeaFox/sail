# Intended to be included by every codec. Sets up necessary dependencies,
# installation targets, codec info.
#
macro(sail_codec)
    cmake_parse_arguments(SAIL_CODEC "" "NAME;ICON" "SOURCES;LINK;DEPENDENCY_COMPILE_OPTIONS;DEPENDENCY_INCLUDE_DIRS;DEPENDENCY_LIBS" ${ARGN})

    # Use 'sail-codec-png' instead of just 'png' to avoid conflicts
    # with libpng cmake configs (they also export a 'png' target)
    # and possibly other libs in the future.
    #
    set(TARGET sail-codec-${SAIL_CODEC_NAME})

    # Add a codec
    #
    if (SAIL_COMBINE_CODECS)
        add_library(${TARGET} OBJECT ${SAIL_CODEC_SOURCES})
    else()
        add_library(${TARGET} MODULE ${SAIL_CODEC_SOURCES})

        sail_enable_asan(TARGET ${TARGET})
    endif()

    # Disable a "lib" prefix on Unix
    #
    set_target_properties(${TARGET} PROPERTIES PREFIX "")

    # Rename to 'sail-codec-png.dll'
    #
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME sail-codec-${SAIL_CODEC_NAME})

    # Depend on sail-common
    #
    target_link_libraries(${TARGET} PRIVATE sail-common)

    # Depend on user-defined targets
    #
    target_link_libraries(${TARGET} PRIVATE ${SAIL_CODEC_LINK})

    # This hack is needed for static builds where we have the following dependencies
    # ("->" is "links to"):
    #
    # sail-codecs-objects -> bmp -> bmp-common
    #
    # When object libraries (bmp) link to other object libraries (bmp-common), CMake pulls
    # just their usage requirements, but not the actual objects. So sail-codecs-objects never
    # contains the bmp-common objects which leads to a linking error. We pull the dependent
    # objects and export them to the outer world explicitly.
    #
    # See also https://gitlab.kitware.com/cmake/cmake/-/issues/18090
    #
    foreach (LINK_DEPENDENCY ${SAIL_CODEC_LINK})
        if (TARGET ${LINK_DEPENDENCY})
            get_target_property(TARGET_TYPE ${LINK_DEPENDENCY} TYPE)

            if (TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
                target_sources(${TARGET} INTERFACE $<TARGET_OBJECTS:${LINK_DEPENDENCY}>)
            endif()
        endif()
    endforeach()

    # Link against the found libs
    #
    target_compile_options(${TARGET}     PRIVATE ${SAIL_CODEC_DEPENDENCY_COMPILE_OPTIONS})
    target_include_directories(${TARGET} PRIVATE ${SAIL_CODEC_DEPENDENCY_INCLUDE_DIRS})
    target_link_libraries(${TARGET}      PRIVATE ${SAIL_CODEC_DEPENDENCY_LIBS})

    # Generate and copy .codec.info into the build dir
    #
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_CODEC_NAME}.codec.info.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sail-codec-${SAIL_CODEC_NAME}.codec.info
                   @ONLY)

    # Installation
    #
    if (NOT SAIL_COMBINE_CODECS)
        install(TARGETS ${TARGET} DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-codec-${SAIL_CODEC_NAME}.codec.info"
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")
    endif()

    # Install icon
    #
    install(FILES "${SAIL_CODEC_ICON}" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/sail/icons")
endmacro()
