# Common dependencies that can be re-used by different codecs
#
add_subdirectory(common/bmp)

# List of codecs
#
set(CODECS avif bmp gif ico jpeg jpeg2000 pcx png qoi svg tga tiff wal webp xbm)

list(SORT CODECS)

# Filter out codecs
#
if (SAIL_ONLY_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if (${codec} IN_LIST SAIL_ONLY_CODECS)
            list(APPEND ENABLED_CODECS ${codec})
        else()
            list(APPEND DISABLED_CODECS ${codec})
        endif()
    endforeach()
else()
    set(ENABLED_CODECS ${CODECS})
endif()

if (SAIL_DISABLE_CODECS)
    foreach (codec IN LISTS ENABLED_CODECS)
        if (${codec} IN_LIST SAIL_DISABLE_CODECS)
            list(REMOVE_ITEM ENABLED_CODECS ${codec})
            list(APPEND DISABLED_CODECS ${codec})
        endif()
    endforeach()
endif()

# Add enabled codecs.
#
foreach(codec ${ENABLED_CODECS})
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
find_dependency(${dependency_name} REQUIRED)
set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${dependency_link_target})
")
        elseif(dependency_search_mechanism STREQUAL "find_library")
            list(GET dependency 1 dependency_release_name)
            list(GET dependency 2 dependency_debug_name)
            set(dependency_name ${dependency_release_name})

            set(SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED "${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED}
find_library(${dependency_name}_RELEASE_LIBRARY NAMES ${dependency_release_name})
find_library(${dependency_name}_DEBUG_LIBRARY NAMES ${dependency_debug_name} ${dependency_release_name})

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
