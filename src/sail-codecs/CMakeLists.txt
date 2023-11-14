# Common dependencies that can be re-used by different codecs
#
add_subdirectory(common/bmp)

# List of codecs
#
set(HIGHEST_PRIORITY_CODECS gif jpeg png tiff)
set(HIGH_PRIORITY_CODECS    bmp svg)
set(MEDIUM_PRIORITY_CODECS  avif jpeg2000 jpegxl webp)
set(LOW_PRIORITY_CODECS     ico pcx pnm psd qoi tga)
set(LOWEST_PRIORITY_CODECS  wal xbm)

set(CODECS ${HIGHEST_PRIORITY_CODECS}
           ${HIGH_PRIORITY_CODECS}
           ${MEDIUM_PRIORITY_CODECS}
           ${LOW_PRIORITY_CODECS}
           ${LOWEST_PRIORITY_CODECS})
list(SORT CODECS)

# Expand codecs priorities to actual codecs.
#
macro(sail_expand_priorities)
    set(TEMP_LIST)

    foreach (codec IN LISTS ${ARGV0})
        if (codec STREQUAL "highest-priority")
            list(APPEND TEMP_LIST ${HIGHEST_PRIORITY_CODECS})
        elseif (codec STREQUAL "high-priority")
            list(APPEND TEMP_LIST ${HIGH_PRIORITY_CODECS})
        elseif (codec STREQUAL "medium-priority")
            list(APPEND TEMP_LIST ${MEDIUM_PRIORITY_CODECS})
        elseif (codec STREQUAL "low-priority")
            list(APPEND TEMP_LIST ${LOW_PRIORITY_CODECS})
        elseif (codec STREQUAL "lowest-priority")
            list(APPEND TEMP_LIST ${LOWEST_PRIORITY_CODECS})
        else()
            list(APPEND TEMP_LIST ${codec})
        endif()
    endforeach()

    set(${ARGV0} ${TEMP_LIST})
endmacro()

# Check if the input codecs list matches the existing codecs list
# and fail with an error if any of the input codecs doesn't exist.
#
# For example: sail_check_codec_presence(jpeg;png jpeg;png;tiff) succeeds
#              sail_check_codec_presence(jpeg;png ico;jpeg;tiff) fails
#
function(sail_check_codec_presence VAR_INPUT_LIST CHECK_AGAINST_LIST)
    foreach (codec IN LISTS ${VAR_INPUT_LIST})
        if (NOT ${codec} IN_LIST CHECK_AGAINST_LIST)
            message(FATAL_ERROR "Codec '${codec}' doesn't exist in the list of the valid codecs '${CHECK_AGAINST_LIST}'")
        endif()
    endforeach()
endfunction()

sail_expand_priorities(SAIL_ONLY_CODECS)
sail_expand_priorities(SAIL_ENABLE_CODECS)
sail_expand_priorities(SAIL_DISABLE_CODECS)

list(REMOVE_DUPLICATES SAIL_ONLY_CODECS)
list(REMOVE_DUPLICATES SAIL_ENABLE_CODECS)
list(REMOVE_DUPLICATES SAIL_DISABLE_CODECS)

sail_check_codec_presence(SAIL_ONLY_CODECS "${CODECS}")
sail_check_codec_presence(SAIL_ENABLE_CODECS "${CODECS}")
sail_check_codec_presence(SAIL_DISABLE_CODECS "${CODECS}")

# Filter out codecs
#
if (SAIL_ONLY_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if (${codec} IN_LIST SAIL_ONLY_CODECS)
            list(APPEND ENABLED_CODECS ${codec})
            list(APPEND FORCED_CODECS ${codec})
        else()
            list(APPEND DISABLED_CODECS ${codec})
        endif()
    endforeach()
else()
    set(ENABLED_CODECS ${CODECS})

    if (SAIL_ENABLE_CODECS)
        foreach (codec IN LISTS SAIL_ENABLE_CODECS)
            list(APPEND FORCED_CODECS ${codec})
        endforeach()
    endif()
endif()

if (SAIL_DISABLE_CODECS)
    foreach (codec IN LISTS ENABLED_CODECS)
        if (${codec} IN_LIST SAIL_DISABLE_CODECS)
            list(REMOVE_ITEM ENABLED_CODECS ${codec})
            list(REMOVE_ITEM FORCED_CODECS ${codec})
            list(APPEND DISABLED_CODECS ${codec})
        endif()
    endforeach()
endif()

foreach (codec IN LISTS FORCED_CODECS)
    string(TOUPPER ${codec} codec_upper)
    set(SAIL_CODEC_${codec_upper}_REQUIRED_OPTION "REQUIRED")
endforeach()

# Add enabled codecs.
#
foreach (codec ${ENABLED_CODECS})
    add_subdirectory(${codec})

    # Codecs can disable themselves due to missing dependencies.
    #
    if (NOT TARGET sail-codec-${codec})
        list(REMOVE_ITEM ENABLED_CODECS ${codec})
        list(APPEND DISABLED_CODECS ${codec})
    endif()
endforeach()

# Export extra dependencies like giflib for static builds to the parent scope
#
if (NOT BUILD_SHARED_LIBS)
    foreach (dependency IN LISTS SAIL_CODECS_FIND_DEPENDENCIES)
        string(REPLACE "," ";" dependency ${dependency})
        list(GET dependency 0 dependency_search_mechanism)

        if (dependency_search_mechanism STREQUAL "find_dependency")
            list(GET dependency 1 dependency_name)
            list(GET dependency 2 dependency_link_target)

            set(SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED "${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED}
# ${dependency_name}
find_dependency(${dependency_name} REQUIRED)
set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${dependency_link_target})
")
        elseif(dependency_search_mechanism STREQUAL "find_library")
            list(GET dependency 1 dependency_release_names)
            list(GET dependency 2 dependency_debug_names)

            # Get "webp" from "webp libwebp"
            set(dependency_release_names_as_list ${dependency_release_names})
            string(REGEX REPLACE " +" ";" dependency_release_names_as_list ${dependency_release_names_as_list})
            list(GET dependency_release_names_as_list 0 dependency_name)

            set(SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED "${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED}
# ${dependency_name}
find_library(${dependency_name}_RELEASE_LIBRARY NAMES ${dependency_release_names})
find_library(${dependency_name}_DEBUG_LIBRARY NAMES ${dependency_debug_names} ${dependency_release_names})

if (NOT ${dependency_name}_RELEASE_LIBRARY OR NOT ${dependency_name}_DEBUG_LIBRARY)
    message(FATAL_ERROR \"Missing dependency: ${dependency_name}\")
endif()

set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES \$<\$<CONFIG:Release>:\${${dependency_name}_RELEASE_LIBRARY}> \$<\$<CONFIG:Debug>:\${${dependency_name}_DEBUG_LIBRARY}>)
")
        else()
            message(FATAL_ERROR "Unsupported dependencies search mechanism '${dependency_search_mechanism}'")
        endif()
    endforeach()

    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED} PARENT_SCOPE)
endif()

# Export the list of codecs to the parent CMake file to print statistics
#
set(ENABLED_CODECS ${ENABLED_CODECS} PARENT_SCOPE)
set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
