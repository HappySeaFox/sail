# Intended to be included by every codec. Sets up necessary dependencies,
# installation targets, codec info.
#
# Usage:
#
#   1. When SYSTEM_HEADERS and SYSTEM_LIBS are specified, they are used to search
#      system headers and libs.
#   2. When SYSTEM_HEADERS and SYSTEM_LIBS are not specified, CMAKE is included
#      and sail_find_dependencies() is called to search CMake packages.
#   3. When CMAKE is specified, sail_codec_post_add() is called right after a new target
#      is added. sail_codec_post_add() could be used for tests like check_c_source_compiles().
#
macro(sail_codec)
    cmake_parse_arguments(SAIL_CODEC "" "NAME" "SOURCES;SYSTEM_HEADERS;SYSTEM_LIBS;CMAKE" ${ARGN})

    # Put this codec into the disabled list so when we return from here
    # on error it's get automatically marked as disabled. If no errors were found,
    # we remove the codec from the disabled list at the end.
    set(DISABLED_CODECS ${DISABLED_CODECS} ${SAIL_CODEC_NAME} PARENT_SCOPE)
    set(DISABLED_CODECS ${DISABLED_CODECS} ${SAIL_CODEC_NAME})

    if (SAIL_CODEC_CMAKE)
        include(${SAIL_CODEC_CMAKE})
    endif()

    # Check dependencies
    #
	if (NOT SAIL_CODEC_CMAKE AND NOT SAIL_CODEC_SYSTEM_HEADERS AND NOT SAIL_CODEC_SYSTEM_LIBS)
		message(FATAL_ERROR "Use CMAKE or SYSTEM_HEADERS SYSTEM_LIBS arguments to search dependencies")
	endif()

	foreach(header ${SAIL_CODEC_SYSTEM_HEADERS})
		find_path(sail_include_dir_${header} ${header})

		if (NOT sail_include_dir_${header})
			string(TOUPPER ${SAIL_CODEC_NAME} SAIL_CODEC_NAME)
			message("*** CODECS: ${header} header file is not found. ${SAIL_CODEC_NAME} codec is disabled.")
			return()
		endif()

		list(APPEND sail_${SAIL_CODEC_NAME}_include_dirs ${sail_include_dir_${header}})
	endforeach()

	foreach(lib ${SAIL_CODEC_SYSTEM_LIBS})
		find_library(sail_lib_${lib} NAMES ${lib})

		if (NOT sail_lib_${lib})
			string(TOUPPER "${SAIL_CODEC_NAME}" SAIL_CODEC_NAME)
			message("*** CODECS: ${lib} library is not found. ${SAIL_CODEC_NAME} codec is disabled.")
			return()
		endif()

		list(APPEND sail_${SAIL_CODEC_NAME}_libs ${sail_lib_${lib}})
	endforeach()

	if (SAIL_CODEC_CMAKE AND NOT SAIL_CODEC_SYSTEM_HEADERS AND NOT SAIL_CODEC_SYSTEM_LIBS AND COMMAND sail_find_dependencies)
		sail_find_dependencies()
	endif()

    set(sail_${SAIL_CODEC_NAME}_cflags       ${sail_${SAIL_CODEC_NAME}_cflags}       CACHE INTERNAL "List of ${SAIL_CODEC_NAME} CFLAGS")
    set(sail_${SAIL_CODEC_NAME}_include_dirs ${sail_${SAIL_CODEC_NAME}_include_dirs} CACHE INTERNAL "List of ${SAIL_CODEC_NAME} include dirs")
    set(sail_${SAIL_CODEC_NAME}_libs         ${sail_${SAIL_CODEC_NAME}_libs}         CACHE INTERNAL "List of ${SAIL_CODEC_NAME} libs")

    # Use 'sail-codec-png' instead of just 'png' to avoid conflicts
    # with libpng cmake configs (they also export a 'png' target)
    # and possibly other libs in the future
    #
    set(TARGET sail-codec-${SAIL_CODEC_NAME})

    # Generate and copy .codec.info into the build dir
    #
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${SAIL_CODEC_NAME}.codec.info.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sail-${SAIL_CODEC_NAME}.codec.info
                   @ONLY)

    # Add a codec
    #
    if (SAIL_COMBINE_CODECS)
        add_library(${TARGET} OBJECT ${SAIL_CODEC_SOURCES})
    else()
        add_library(${TARGET} MODULE ${SAIL_CODEC_SOURCES})
    endif()

    # Disable a "lib" prefix on Unix
    #
    set_target_properties(${TARGET} PROPERTIES PREFIX "")

    # Rename to just 'sail-png.dll'
    #
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME sail-${SAIL_CODEC_NAME})

    # Depend on sail-common
    #
    target_link_libraries(${TARGET} PRIVATE sail-common)

    if (COMMAND sail_codec_post_add)
        sail_codec_post_add()
    endif()

    # Link against the found libs
    #
    target_compile_options(${TARGET}     PRIVATE ${sail_${SAIL_CODEC_NAME}_cflags})
    target_include_directories(${TARGET} PRIVATE ${sail_${SAIL_CODEC_NAME}_include_dirs})
    target_link_libraries(${TARGET}      PRIVATE ${sail_${SAIL_CODEC_NAME}_libs})

    # Installation
    #
    if (NOT SAIL_COMBINE_CODECS)
        install(TARGETS ${TARGET} DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sail-${SAIL_CODEC_NAME}.codec.info"
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/sail/codecs")
    endif()

    # Export this codec name into the parent scope
    #
    set(ENABLED_CODECS ${ENABLED_CODECS} ${SAIL_CODEC_NAME} PARENT_SCOPE)
    list(REMOVE_ITEM DISABLED_CODECS ${SAIL_CODEC_NAME})
    set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
endmacro()
