# Prefer a system qoi.h when it actually provides the QOI API.
# Fall back to the bundled header otherwise.
#
find_path(SAIL_QOI_INCLUDE_DIR NAMES qoi.h)

set(SAIL_QOI_SYSTEM OFF)

if (SAIL_QOI_INCLUDE_DIR)
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${SAIL_QOI_INCLUDE_DIR})

        check_c_source_compiles(
            "
            #define QOI_IMPLEMENTATION
            #define QOI_NO_STDIO
            #include <qoi.h>

            int main(int argc, char *argv[]) {
                qoi_desc desc = {0};
                void *pixels = qoi_decode(NULL, 0, &desc, 4);
                return 0;
            }
        "
        HAVE_QOI_API
        )
    cmake_pop_check_state()

    if (HAVE_QOI_API)
        set(SAIL_QOI_SYSTEM ON)
        set(QOI_INCLUDE_DIRS ${SAIL_QOI_INCLUDE_DIR})
    endif()
endif()

# Common codec configuration
#
sail_codec(NAME qoi
            SOURCES qoi.c
            ICON qoi.png
            DEPENDENCY_INCLUDE_DIRS ${QOI_INCLUDE_DIRS})

if (SAIL_QOI_SYSTEM)
    target_compile_definitions(${SAIL_CODEC_TARGET} PRIVATE SAIL_HAVE_SYSTEM_QOI)
endif()
