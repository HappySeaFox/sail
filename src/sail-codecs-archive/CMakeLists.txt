# Generate built-in codec info as C source files and compile it into the combined library
#
set(SAIL_CODEC_INFO_SOURCES "")
foreach(codec ${ENABLED_CODECS})
    get_target_property(CODEC_BINARY_DIR sail-codec-${codec} BINARY_DIR)

    file(READ ${CODEC_BINARY_DIR}/sail-codec-${codec}.codec.info SAIL_CODEC_INFO_CONTENTS)
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

# Link all the enabled codecs into sail-codecs
#
set(SAIL_CODECS_LIBS "")
foreach(codec ${ENABLED_CODECS})
    list(APPEND SAIL_CODECS_LIBS $<TARGET_OBJECTS:sail-codec-${codec}>)
endforeach()

# Add a combined library
#
if (SAIL_STATIC)
    add_library(sail-codecs-objects ${SAIL_CODEC_INFO_SOURCES} ${SAIL_CODECS_LIBS})
    target_link_libraries(sail-codecs-objects PRIVATE sail-common)

    # Add an extra library to link against it with a special 'whole archive' option.
    # Without that option compilers throw functions away as they think they're unreferenced.
    # We use dlsym/GetProcAddress to access them.
    #
    add_library(sail-codecs ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c)
    target_link_libraries(sail-codecs PRIVATE sail-common)

    # Generate a 'whole archive' expression per compiler
    #
    set(SAIL_ALLOWED_COMPILERS MSVC Clang AppleClang GNU)
    list(FIND SAIL_ALLOWED_COMPILERS ${CMAKE_C_COMPILER_ID} SAIL_ALLOWED_COMPILERS_INDEX)

    if (SAIL_ALLOWED_COMPILERS_INDEX LESS 0)
        message(FATAL_ERROR "Don't know how to build a compiler option for '${CMAKE_C_COMPILER_ID}'")
    endif()

    target_link_libraries(
        sail-codecs
        PRIVATE
            $<BUILD_INTERFACE:$<$<C_COMPILER_ID:MSVC>:-WHOLEARCHIVE:$<TARGET_FILE:sail-codecs-objects>>>
            $<BUILD_INTERFACE:$<$<C_COMPILER_ID:Clang>:-Wl,-force_load,$<TARGET_FILE:sail-codecs-objects>>>
            $<BUILD_INTERFACE:$<$<C_COMPILER_ID:AppleClang>:-Wl,-force_load,$<TARGET_FILE:sail-codecs-objects>>>
            $<BUILD_INTERFACE:$<$<C_COMPILER_ID:GNU>:-Wl,--whole-archive\ $<TARGET_FILE:sail-codecs-objects>\ -Wl,--no-whole-archive>>
            $<INSTALL_INTERFACE:$<$<C_COMPILER_ID:MSVC>:-WHOLEARCHIVE:$<TARGET_FILE:SAIL::sail-codecs-objects>>>
            $<INSTALL_INTERFACE:$<$<C_COMPILER_ID:Clang>:-Wl,-force_load,$<TARGET_FILE:SAIL::sail-codecs-objects>>>
            $<INSTALL_INTERFACE:$<$<C_COMPILER_ID:AppleClang>:-Wl,-force_load,$<TARGET_FILE:SAIL::sail-codecs-objects>>>
            $<INSTALL_INTERFACE:$<$<C_COMPILER_ID:GNU>:-Wl,--whole-archive\ $<TARGET_FILE:SAIL::sail-codecs-objects>\ -Wl,--no-whole-archive>>
    )

    # Link all the enabled codecs dependencies into sail-codecs
    #
    foreach(codec ${ENABLED_CODECS})
        get_target_property(CODEC_LINK_LIBRARIES sail-codec-${codec} LINK_LIBRARIES)
        target_link_libraries(sail-codecs PRIVATE $<BUILD_INTERFACE:${CODEC_LINK_LIBRARIES}>)
    endforeach()

    install(TARGETS sail-codecs-objects sail-codecs
            EXPORT SailCodecsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs)
else()
    add_library(sail-codecs ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c ${SAIL_CODEC_INFO_SOURCES} ${SAIL_CODECS_LIBS})
    target_link_libraries(sail-codecs PRIVATE sail-common)

    # Link all the enabled codecs dependencies into sail-codecs
    #
    foreach(codec ${ENABLED_CODECS})
        get_target_property(CODEC_LINK_LIBRARIES sail-codec-${codec} LINK_LIBRARIES)
        target_link_libraries(sail-codecs PRIVATE ${CODEC_LINK_LIBRARIES})
    endforeach()

    install(TARGETS sail-codecs
            EXPORT SailCodecsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs)
endif()

sail_install_cmake_config(TARGET "SailCodecs" VERSION 1.0.0)
