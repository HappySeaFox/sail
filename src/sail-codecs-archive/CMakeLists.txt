# Generate built-in codecs info and compile it into the combined library.
# Needed for the configure_file() command below.
#
foreach(codec ${ENABLED_CODECS})
    get_target_property(CODEC_BINARY_DIR sail-codec-${codec} BINARY_DIR)

    set(SAIL_ENABLED_CODECS "${SAIL_ENABLED_CODECS}\"${codec}\", ")

    file(READ ${CODEC_BINARY_DIR}/sail-codec-${codec}.codec.info SAIL_CODEC_INFO_CONTENTS)
    string(REPLACE "\"" "\\\"" SAIL_CODEC_INFO_CONTENTS "${SAIL_CODEC_INFO_CONTENTS}")
    # Add \n\ on every line
    string(REGEX REPLACE "\n" "\\\\n\\\\\n" SAIL_CODEC_INFO_CONTENTS "${SAIL_CODEC_INFO_CONTENTS}")
    set(SAIL_CODEC_INFO_CONTENTS "\"${SAIL_CODEC_INFO_CONTENTS}\",\n")
    set(SAIL_ENABLED_CODECS_INFO "${SAIL_ENABLED_CODECS_INFO}${SAIL_CODEC_INFO_CONTENTS}")

    set(SAIL_ENABLED_CODECS_DECLARE_FUNCTIONS "${SAIL_ENABLED_CODECS_DECLARE_FUNCTIONS}
#define SAIL_CODEC_NAME ${codec}
#include \"layout/v6.h\"
#undef SAIL_CODEC_NAME
")

    set(SAIL_ENABLED_CODECS_LAYOUTS "${SAIL_ENABLED_CODECS_LAYOUTS}
    {
        #define SAIL_CODEC_NAME ${codec}
        .read_init            = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_read_init_v6),
        .read_seek_next_frame = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_read_seek_next_frame_v6),
        .read_frame           = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_read_frame_v6),
        .read_finish          = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_read_finish_v6),

        .write_init            = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_write_init_v6),
        .write_seek_next_frame = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_write_seek_next_frame_v6),
        .write_frame           = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_write_frame_v6),
        .write_finish          = SAIL_CONSTRUCT_CODEC_FUNC(sail_codec_write_finish_v6)
        #undef SAIL_CODEC_NAME
    },\n")
endforeach()

string(TOUPPER "${SAIL_ENABLED_CODECS}" SAIL_ENABLED_CODECS)
set(SAIL_ENABLED_CODECS "${SAIL_ENABLED_CODECS}NULL")

# List of enabled codecs and their info
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/enabled_codecs.c.in
               ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c
               @ONLY)

# Add a combined library
#
if (BUILD_SHARED_LIBS)
    add_library(sail-codecs ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c)
    target_link_libraries(sail-codecs PRIVATE sail-common)

    foreach(CODEC ${ENABLED_CODECS})
        target_link_libraries(sail-codecs PRIVATE $<BUILD_INTERFACE:sail-codec-${CODEC}>)
    endforeach()

    sail_install_pdb(TARGET sail-codecs)

    install(TARGETS sail-codecs
            EXPORT SailCodecsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs)
else()
    add_library(sail-codecs-objects codecs_objects_stub.c)
    target_link_libraries(sail-codecs-objects PRIVATE sail-common)

    # BUILD_INTERFACE is needed as CMake tries to export all the sail-codec-${CODEC} dependencies
    # in the below install(TARGETS) command even though they are OBJECT libraries. This is why we also
    # use target_link_libraries(sail-codecs INTERFACE ...) later.
    #
    foreach(CODEC ${ENABLED_CODECS})
        target_link_libraries(sail-codecs-objects PRIVATE $<BUILD_INTERFACE:sail-codec-${CODEC}>)
    endforeach()

    sail_install_pdb(TARGET sail-codecs-objects)

    # Add an extra library to link against it with a special 'whole archive' option.
    # Without that option compilers throw functions away as they think they're unreferenced.
    # We use dlsym/GetProcAddress to access them.
    #
    add_library(sail-codecs ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c)
    target_link_libraries(sail-codecs PRIVATE sail-common)

    sail_install_pdb(TARGET sail-codecs)

    # Generate a 'whole archive' expression per compiler
    #
    set(ALLOWED_COMPILERS "MSVC" "Clang" "AppleClang" "GNU")
    list(FIND ALLOWED_COMPILERS ${CMAKE_C_COMPILER_ID} ALLOWED_COMPILERS_INDEX)

    if (ALLOWED_COMPILERS_INDEX LESS 0)
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

    # External dependencies like libavif are exported through interfaces, grab them
    #
    foreach(CODEC ${ENABLED_CODECS})
        target_link_libraries(sail-codecs INTERFACE $<BUILD_INTERFACE:sail-codec-${CODEC}>)
    endforeach()

    install(TARGETS sail-codecs-objects sail-codecs
            EXPORT SailCodecsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs)
endif()

target_include_directories(sail-codecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libsail)

sail_install_cmake_config(TARGET "SailCodecs" FOLDER "sailcodecs" VERSION 1.0.0)
