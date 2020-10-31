# Unconditional codecs that don't require external dependencies
#
set(CODECS "")

list(SORT CODECS)

# Conditional codecs that require external dependencies like libjpeg
#
set(CONDITIONAL_CODECS gif jpeg png tiff)

# Actual dependencies. Will use pkg_check_modules() to find them. Nested lists must use \; as a separator like that: "a\;b"
#
set(CONDITIONAL_DEPENDENCIES giflib libjpeg libpng16 libtiff-4)

################################################################################

# Filter out uncoditional codecs
#
if (SAIL_ONLY_CODECS)
    set(CODECS_ENABLED "")

    foreach (codec IN LISTS CODECS)
        if ("${codec}" IN_LIST SAIL_ONLY_CODECS)
            list(APPEND CODECS_ENABLED "${codec}")
        endif()
    endforeach()

    set(CODECS "${CODECS_ENABLED}")
endif()

if (SAIL_EXCEPT_CODECS)
    set(CODECS_ENABLED "")

    foreach (codec IN LISTS CODECS)
        if (NOT "${codec}" IN_LIST SAIL_EXCEPT_CODECS)
            list(APPEND CODECS_ENABLED "${codec}")
        endif()
    endforeach()

    set(CODECS "${CODECS_ENABLED}")
endif()

# Iterate over conditional codecs and enable/disable them based on their dependencies
#
list(LENGTH CONDITIONAL_CODECS length)
math(EXPR length ${length}-1)

set(ENABLED_CONDITIONAL_CODECS "")
set(DISABLED_CONDITIONAL_CODECS "")

foreach(i RANGE ${length})
    list(GET CONDITIONAL_CODECS      ${i} codec)
    list(GET CONDITIONAL_DEPENDENCIES ${i} codec_dependencies)

    # Check if the user enabled this codec at all
    #
    if (SAIL_ONLY_CODECS)
        if (NOT "${codec}" IN_LIST SAIL_ONLY_CODECS)
            continue()
        endif()
    endif()

    if (SAIL_EXCEPT_CODECS)
        if ("${codec}" IN_LIST SAIL_EXCEPT_CODECS)
            continue()
        endif()
    endif()

    # "All dependencies are found" flag
    #
    set(codec_ok ON)

    foreach(codec_dependency ${codec_dependencies})
        pkg_check_modules(${codec_dependency} IMPORTED_TARGET ${codec_dependency})

        if (NOT "${${codec_dependency}_FOUND}")
            string(TOUPPER "${codec}" codec)
            message(WARNING "${codec_dependency} development package is not found. ${codec} codec is disabled.")
            set(codec_ok OFF)
            break()
        endif()
    endforeach()

    # Store this codec in the list of enabled to disabled codecs
    #
    if (${codec_ok})
        list(APPEND ENABLED_CONDITIONAL_CODECS ${codec})
    else()
        list(APPEND DISABLED_CONDITIONAL_CODECS ${codec})
    endif()
endforeach()

list(SORT ENABLED_CONDITIONAL_CODECS)
list(SORT DISABLED_CONDITIONAL_CODECS)

# Add unconditional subdirs
#
foreach(codec ${CODECS})
    add_subdirectory(src/${codec})
endforeach()

# Add conditional subdirs
#
foreach(codec ${ENABLED_CONDITIONAL_CODECS})
    add_subdirectory(src/${codec})
endforeach()

# Modify the lists for displaying
#
string(TOUPPER "${ENABLED_CONDITIONAL_CODECS}" ENABLED_CONDITIONAL_CODECS)
string(TOUPPER "${DISABLED_CONDITIONAL_CODECS}" DISABLED_CONDITIONAL_CODECS)

# Export codecs to the parent cmake file to print statistics
#
set(CODECS ${CODECS} PARENT_SCOPE)
set(ENABLED_CONDITIONAL_CODECS ${ENABLED_CONDITIONAL_CODECS} PARENT_SCOPE)
set(DISABLED_CONDITIONAL_CODECS ${DISABLED_CONDITIONAL_CODECS} PARENT_SCOPE)

# Installation
#
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/layouts/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/layouts")
