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

# Link all the enabled codecs into sail-codecs
#
set(SAIL_CODECS_LIBS "")
foreach(codec ${ENABLED_CODECS})
    list(APPEND SAIL_CODECS_LIBS $<TARGET_OBJECTS:sail-codec-${codec}>)
endforeach()

# Figure out which of the codecs' dependencies are object libraries (to be combined) and
# which are external dependencies (to be linked against).
#
set(SAIL_CODECS_DEPENDENCIES "sail-common")

foreach (codec ${ENABLED_CODECS})
    get_target_property(CODEC_LINK_LIBRARIES sail-codec-${codec} LINK_LIBRARIES)

    foreach (codec_dependency ${CODEC_LINK_LIBRARIES})
        set(IS_OBJECT_LIBRARY FALSE)
        if (TARGET ${codec_dependency})
            get_target_property(TARGET_TYPE ${codec_dependency} TYPE)
            if (TARGET_TYPE STREQUAL "OBJECT_LIBRARY")
                set(IS_OBJECT_LIBRARY TRUE)
            endif()
        endif()

        if (IS_OBJECT_LIBRARY)
            list(APPEND SAIL_CODECS_LIBS $<TARGET_OBJECTS:${codec_dependency}>)
        else()
            list(APPEND SAIL_CODECS_DEPENDENCIES ${codec_dependency})
        endif()
    endforeach()
endforeach()

list(REMOVE_DUPLICATES SAIL_CODECS_LIBS)

# Add a combined library
#
add_library(sail-codecs ${CMAKE_CURRENT_BINARY_DIR}/enabled_codecs.c ${SAIL_CODECS_LIBS})

# Link against all the external dependencies of sail-codecs
#
target_link_libraries(sail-codecs PRIVATE ${SAIL_CODECS_DEPENDENCIES})

sail_install_pdb(TARGET sail-codecs)

install(TARGETS sail-codecs
        EXPORT SailCodecsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sail/sail-codecs)

target_include_directories(sail-codecs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libsail)

sail_install_cmake_config(TARGET "SailCodecs" FOLDER "sailcodecs" VERSION 1.0.0)
