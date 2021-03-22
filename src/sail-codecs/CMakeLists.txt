# List of codecs
#
set(CODECS bmp gif jpeg png) # tiff

list(SORT CODECS)

# Filter out codecs
#
if (SAIL_ONLY_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if (${codec} IN_LIST SAIL_ONLY_CODECS)
            list(APPEND ENABLED_CODECS ${codec})
        endif()
    endforeach()

    set(CODECS ${ENABLED_CODECS})
endif()

if (SAIL_EXCEPT_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if (NOT ${codec} IN_LIST SAIL_EXCEPT_CODECS)
            list(APPEND ENABLED_CODECS ${codec})
        endif()
    endforeach()

    set(CODECS ${ENABLED_CODECS})
endif()

# Add enabled codecs (but codecs still can disable themselves due to missing dependencies).
# Every codec updates the value of ENABLED_CODECS.
#
foreach(codec ${CODECS})
    add_subdirectory(${codec})
endforeach()

# Handle and export extra dependencies like giflib for static builds to the parent scope
#
foreach (dependency IN LISTS SAIL_CODECS_FIND_DEPENDENCIES)
    string(REPLACE "," ";" dependency ${dependency})
    list(GET dependency 0 dependency_name)
    list(GET dependency 1 dependency_link_target)

    set(SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED "${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED}
find_dependency(${dependency_name} REQUIRED)
set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${dependency_link_target})
")
endforeach()

set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES_EXPANDED} PARENT_SCOPE)

# Export codecs to the parent cmake file to print statistics
#
set(ENABLED_CODECS ${ENABLED_CODECS} PARENT_SCOPE)
set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
