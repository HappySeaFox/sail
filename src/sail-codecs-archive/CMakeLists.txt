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

# Add library
#
add_library(sail-codecs-archive ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c ${SAIL_CODEC_INFO_SOURCES} ${SAIL_CODECS_LIBS})

target_link_libraries(sail-codecs-archive PRIVATE sail-common)

# Dependents must include the whole static archive
#
if (MSVC)
    target_link_libraries(sail-codecs-archive INTERFACE -WHOLEARCHIVE:$<TARGET_FILE:sail-codecs-archive>)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_link_libraries(sail-codecs-archive INTERFACE -Wl,-force_load,$<TARGET_FILE:sail-codecs-archive>)
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(sail-codecs-archive INTERFACE -Wl,--whole-archive $<TARGET_FILE:sail-codecs-archive> -Wl,--no-whole-archive)
else()
    message(FATAL_ERROR "Don't know how to build a compiler option for '${CMAKE_C_COMPILER_ID}'")
endif()

# Link all the enabled codecs dependencies into sail-codecs-archive
#
foreach(codec ${ENABLED_CODECS})
    get_target_property(CODEC_LINK_DIRECTORIES sail-codec-${codec} LINK_DIRECTORIES)
    target_link_directories(sail-codecs-archive PRIVATE ${CODEC_LINK_DIRECTORIES})

    get_target_property(CODEC_LINK_LIBRARIES sail-codec-${codec} LINK_LIBRARIES)
    target_link_libraries(sail-codecs-archive PRIVATE ${CODEC_LINK_LIBRARIES})
endforeach()

install(TARGETS sail-codecs-archive
        EXPORT SailCodecsArchiveTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs-archive)

sail_install_cmake_config(TARGET "SailCodecsArchive" VERSION 1.0.0)
