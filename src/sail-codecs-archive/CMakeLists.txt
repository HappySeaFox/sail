# Generate a built-in codec info as a C source file and compile it into the codec
#
set(SAIL_CODEC_INFO_SOURCES "")
foreach(codec ${ENABLED_CODECS})
    get_target_property(CODEC_BINARY_DIR sail-codec-${codec} BINARY_DIR)

    file(READ ${CODEC_BINARY_DIR}/sail-${codec}.codec.info SAIL_CODEC_INFO_CONTENTS)
    string(REPLACE "\"" "\\\"" SAIL_CODEC_INFO_CONTENTS "${SAIL_CODEC_INFO_CONTENTS}")
    # Add \n\ on every line
    string(REGEX REPLACE "\n" "\\\\n\\\\\n" SAIL_CODEC_INFO_CONTENTS "${SAIL_CODEC_INFO_CONTENTS}")

    set(SAIL_CODEC_NAME ${codec})

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/codec_info.c.in
                   ${CMAKE_CURRENT_BINARY_DIR}/codec_info_${codec}.c
                   @ONLY)

    list(APPEND SAIL_CODEC_INFO_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/codec_info_${codec}.c)
endforeach()

# Needed for the configure_file() command below
#
set(SAIL_ENABLED_CODECS "")
foreach(codec ${ENABLED_CODECS})
    list(APPEND SAIL_ENABLED_CODECS ${codec})
endforeach()

# List of enabled codecs
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/enabled_codecs.c.in
               ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c
               @ONLY)

# Link all the enabled codecs into sail-codecs-archive
#
set(SAIL_CODECS_LIBS "")
foreach(codec ${ENABLED_CODECS})
    list(APPEND SAIL_CODECS_LIBS $<TARGET_OBJECTS:sail-codec-${codec}>)
endforeach()

# Add a combined library
#
add_library(sail-codecs-objects ${SAIL_CODEC_INFO_SOURCES} ${SAIL_CODECS_LIBS})
target_link_libraries(sail-codecs-objects PRIVATE sail-common)

# Add an extra library to link against it with a special 'whole archive' option.
# Without that option compilers throw functions away as they think they're unreferenced.
# We use dlsym/GetProcAddress to access them.
#
add_library(sail-codecs-archive ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c)
target_link_libraries(sail-codecs-archive PRIVATE sail-common)

# Generate a 'whole archive' expression per compiler
#
set(SAIL_ALLOWED_COMPILERS MSVC Clang GNU)
list(FIND SAIL_ALLOWED_COMPILERS ${CMAKE_C_COMPILER_ID} SAIL_ALLOWED_COMPILERS_INDEX)

if (SAIL_ALLOWED_COMPILERS_INDEX LESS 0)
    message(FATAL_ERROR "Don't know how to build a compiler option for '${CMAKE_C_COMPILER_ID}'")
endif()

target_link_libraries(
    sail-codecs-archive
    INTERFACE
        $<$<C_COMPILER_ID:MSVC>:-WHOLEARCHIVE:$<TARGET_FILE:sail-codecs-objects>>
        $<$<C_COMPILER_ID:Clang>:-Wl,-force_load,$<TARGET_FILE:sail-codecs-objects>>
        $<$<C_COMPILER_ID:GNU>:-Wl,--whole-archive $<TARGET_FILE:sail-codecs-objects> -Wl,--no-whole-archive>
)

# Link all the enabled codecs dependencies into sail-codecs-archive interface
#
foreach(codec ${ENABLED_CODECS})
    get_target_property(CODEC_LINK_LIBRARIES sail-codec-${codec} LINK_LIBRARIES)
    target_link_libraries(sail-codecs-archive INTERFACE ${CODEC_LINK_LIBRARIES})
endforeach()

install(TARGETS sail-codecs-objects sail-codecs-archive
        EXPORT SailCodecsArchiveTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs-archive)

sail_install_cmake_config(TARGET "SailCodecsArchive" VERSION 1.0.0)