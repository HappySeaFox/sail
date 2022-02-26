# Intended to be included by every codec. Sets up necessary dependencies,
# installation targets, codec info.
#
# Usage:
#
#   1. When CMAKE is specified, sail_find_dependencies() is called to search CMake packages.
#   2. When CMAKE is specified, sail_codec_post_add() is called right after a new target
#      is added. sail_codec_post_add() could be used for tests like check_c_source_compiles().
#
macro(sail_codec)
    cmake_parse_arguments(SAIL_CODEC "" "NAME;ICON" "SOURCES;LINK;CMAKE" ${ARGN})

    # Put this codec into the disabled list so when we return from here
    # on error it's get automatically marked as disabled. If no errors were found,
    # we remove the codec from the disabled list at the end.
    list(REMOVE_ITEM ENABLED_CODECS ${SAIL_CODEC_NAME})
    set(ENABLED_CODECS ${ENABLED_CODECS} PARENT_SCOPE)
    set(DISABLED_CODECS ${DISABLED_CODECS} ${SAIL_CODEC_NAME} PARENT_SCOPE)
    set(DISABLED_CODECS ${DISABLED_CODECS} ${SAIL_CODEC_NAME})

    if (SAIL_CODEC_CMAKE)
        include(${SAIL_CODEC_CMAKE})

        if (COMMAND sail_find_dependencies)
            sail_find_dependencies()
        endif()
    endif()

    set(sail_${SAIL_CODEC_NAME}_cflags       ${sail_${SAIL_CODEC_NAME}_cflags}       CACHE INTERNAL "List of ${SAIL_CODEC_NAME} CFLAGS")
    set(sail_${SAIL_CODEC_NAME}_include_dirs ${sail_${SAIL_CODEC_NAME}_include_dirs} CACHE INTERNAL "List of ${SAIL_CODEC_NAME} include dirs")
    set(sail_${SAIL_CODEC_NAME}_libs         ${sail_${SAIL_CODEC_NAME}_libs}         CACHE INTERNAL "List of ${SAIL_CODEC_NAME} libs")

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

    if (COMMAND sail_codec_post_add)
        sail_codec_post_add()
    endif()

    # Link against the found libs
    #
    target_compile_options(${TARGET}     PRIVATE ${sail_${SAIL_CODEC_NAME}_cflags})
    target_include_directories(${TARGET} PRIVATE ${sail_${SAIL_CODEC_NAME}_include_dirs})
    target_link_libraries(${TARGET}      PRIVATE ${sail_${SAIL_CODEC_NAME}_libs})

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

    # Export this codec name into the parent scope
    #
    set(ENABLED_CODECS ${ENABLED_CODECS} ${SAIL_CODEC_NAME} PARENT_SCOPE)
    list(REMOVE_ITEM DISABLED_CODECS ${SAIL_CODEC_NAME})
    set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
endmacro()
