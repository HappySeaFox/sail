# List of codecs
#
set(CODECS gif jpeg png tiff)

list(SORT CODECS)

# Filter out codecs
#
if (SAIL_ONLY_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if ("${codec}" IN_LIST SAIL_ONLY_CODECS)
            list(APPEND ENABLED_CODECS "${codec}")
        endif()
    endforeach()

    set(CODECS "${ENABLED_CODECS}")
endif()

if (SAIL_EXCEPT_CODECS)
    set(ENABLED_CODECS "")

    foreach (codec IN LISTS CODECS)
        if (NOT "${codec}" IN_LIST SAIL_EXCEPT_CODECS)
            list(APPEND ENABLED_CODECS "${codec}")
        endif()
    endforeach()

    set(CODECS "${ENABLED_CODECS}")
endif()

# Add enabled codecs (but codecs still can disable themselves due to missing dependencies).
# Every codec updates the value of ENABLED_CODECS.
#
foreach(codec ${CODECS})
    add_subdirectory(${codec})
endforeach()

# Export codecs to the parent cmake file to print statistics
#
string(TOUPPER "${ENABLED_CODECS}" ENABLED_CODECS)
set(ENABLED_CODECS ${ENABLED_CODECS} PARENT_SCOPE)

string(TOUPPER "${DISABLED_CODECS}" DISABLED_CODECS)
set(DISABLED_CODECS ${DISABLED_CODECS} PARENT_SCOPE)
