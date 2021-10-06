# List of codecs
#
set(CODECS avif bmp gif jpeg jpeg2000 png tga tiff webp)

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

if (SAIL_EXCEPT_CODECS)
    foreach (codec IN LISTS ENABLED_CODECS)
        if (${codec} IN_LIST SAIL_EXCEPT_CODECS)
            list(REMOVE_ITEM ENABLED_CODECS ${codec})
            list(APPEND DISABLED_CODECS ${codec})
        endif()
    endforeach()
endif()

# Add enabled codecs (but codecs still can disable themselves later due to missing dependencies).
# Every codec CMake script updates the value of ENABLED_CODECS.
#
foreach(codec ${ENABLED_CODECS})
    add_subdirectory(${codec})
endforeach()

# Export extra dependencies like giflib for static builds to the parent scope
#
if (SAIL_STATIC)
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
endif()

# Export the list of codecs to the parent CMake file to print statistics
#
set(ENABLED_CODECS ${ENABLED_CODECS} PARENT_SCOPE)
set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
