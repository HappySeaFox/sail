# Intended to be included by every codec. Sets up necessary dependencies,
# installation targets, codec info.
#
# Creates SAIL_CODEC_TARGET variable with the created target name.
#
macro(sail_codec)
    cmake_parse_arguments(SAIL_CODEC "" "NAME;ICON" "SOURCES;LINK;DEPENDENCY_COMPILE_DEFINITIONS;DEPENDENCY_INCLUDE_DIRS;DEPENDENCY_LIBS" ${ARGN})

    if (NOT SAIL_CODEC_NAME MATCHES "^[a-z0-9]+$")
        message(FATAL_ERROR "Invalid codec name '${SAIL_CODEC_NAME}'. Only lower-case letters and numbers are allowed.")
    endif()

    # Use 'sail-codec-png' instead of just 'png' to avoid conflicts
    # with libpng cmake configs (they also export a 'png' target)
    # and possibly other libs in the future.
    #
    set(SAIL_CODEC_TARGET sail-codec-${SAIL_CODEC_NAME})

    # Add a codec
    #
    if (SAIL_COMBINE_CODECS)
        add_library(${SAIL_CODEC_TARGET} OBJECT ${SAIL_CODEC_SOURCES})
    else()
        add_library(${SAIL_CODEC_TARGET} MODULE ${SAIL_CODEC_SOURCES})
    endif()

    # Common flags
    #
    target_link_libraries(${SAIL_CODEC_TARGET} PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

    # Disable a "lib" prefix on Unix
    #
    set_target_properties(${SAIL_CODEC_TARGET} PROPERTIES PREFIX "")

    # Rename to 'sail-codec-png.dll'
    #
    set_target_properties(${SAIL_CODEC_TARGET} PROPERTIES OUTPUT_NAME sail-codec-${SAIL_CODEC_NAME})

    # Depend on sail-common
    #
    target_link_libraries(${SAIL_CODEC_TARGET} PRIVATE sail-common)

    # Depend on user-defined targets
    #
    target_link_libraries(${SAIL_CODEC_TARGET} PRIVATE ${SAIL_CODEC_LINK})

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
                target_sources(${SAIL_CODEC_TARGET} INTERFACE $<TARGET_OBJECTS:${LINK_DEPENDENCY}>)
            endif()
        endif()
    endforeach()

    # Link against the found libs
    #
    target_compile_definitions(${SAIL_CODEC_TARGET} PRIVATE ${SAIL_CODEC_DEPENDENCY_COMPILE_DEFINITIONS})
    target_include_directories(${SAIL_CODEC_TARGET} PRIVATE ${SAIL_CODEC_DEPENDENCY_INCLUDE_DIRS})
    target_link_libraries(${SAIL_CODEC_TARGET}      PRIVATE ${SAIL_CODEC_DEPENDENCY_LIBS})

    # Generate and copy .codec.info into the build dir
    #
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_CODEC_NAME}.codec.info.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sail-codec-${SAIL_CODEC_NAME}.codec.info
                   @ONLY)

    # Installation
    #
    if (NOT SAIL_COMBINE_CODECS)
        install(TARGETS ${SAIL_CODEC_TARGET} DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-codec-${SAIL_CODEC_NAME}.codec.info"
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")
    endif()

    # Install icon
    #
    install(FILES "${SAIL_CODEC_ICON}" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/sail/icons")
endmacro()
