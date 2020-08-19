# Unconditional plugins that don't require external dependencies
#
set(PLUGINS "")

list(SORT PLUGINS)

# Conditional plugins that require external dependencies like libjpeg
#
set(CONDITIONAL_PLUGINS jpeg png tiff)

# Actual dependencies. Will use pkg_check_modules() to find them. Nested lists must use \; as a separator
#
if (WIN32)
    set(TIFF_DEPS "libtiff-4\;libjpeg\;zlib\;libzstd")
else()
    set(TIFF_DEPS "libtiff-4\;libjpeg\;zlib")
endif()

set(CONDITIONAL_DEPENDENCIES libjpeg libpng16 "${TIFF_DEPS}")

################################################################################

# Filter out uncoditional plugins
#
if (SAIL_ONLY_PLUGINS)
    set(PLUGINS_ENABLED "")

    foreach (plugin IN LISTS PLUGINS)
        if ("${plugin}" IN_LIST SAIL_ONLY_PLUGINS)
            list(APPEND PLUGINS_ENABLED "${plugin}")
        endif()
    endforeach()

    set(PLUGINS "${PLUGINS_ENABLED}")
endif()

if (SAIL_EXCEPT_PLUGINS)
    set(PLUGINS_ENABLED "")

    foreach (plugin IN LISTS PLUGINS)
        if (NOT "${plugin}" IN_LIST SAIL_EXCEPT_PLUGINS)
            list(APPEND PLUGINS_ENABLED "${plugin}")
        endif()
    endforeach()

    set(PLUGINS "${PLUGINS_ENABLED}")
endif()

# Iterate over conditional plugins and enable/disable them based on their dependencies
#
list(LENGTH CONDITIONAL_PLUGINS length)
math(EXPR length ${length}-1)

set(ENABLED_CONDITIONAL_PLUGINS "")
set(DISABLED_CONDITIONAL_PLUGINS "")

foreach(i RANGE ${length})
    list(GET CONDITIONAL_PLUGINS      ${i} plugin)
    list(GET CONDITIONAL_DEPENDENCIES ${i} plugin_dependencies)

    # Check if the user enabled this plugin at all
    #
    if (SAIL_ONLY_PLUGINS)
        if (NOT "${plugin}" IN_LIST SAIL_ONLY_PLUGINS)
            continue()
        endif()
    endif()

    if (SAIL_EXCEPT_PLUGINS)
        if ("${plugin}" IN_LIST SAIL_EXCEPT_PLUGINS)
            continue()
        endif()
    endif()

    # "All dependencies are found" flag
    #
    set(plugin_ok ON)

    foreach(plugin_dependency ${plugin_dependencies})
        pkg_check_modules(${plugin_dependency} IMPORTED_TARGET ${plugin_dependency})

        if (NOT "${${plugin_dependency}_FOUND}")
            string(TOUPPER "${plugin}" plugin)
            message(WARNING "${plugin_dependency} development package is not found. ${plugin} plugin is disabled.")
            set(plugin_ok OFF)
            break()
        endif()
    endforeach()

    # Store this plugin in the list of enabled to disabled plugins
    #
    if (${plugin_ok})
        list(APPEND ENABLED_CONDITIONAL_PLUGINS ${plugin})
    else()
        list(APPEND DISABLED_CONDITIONAL_PLUGINS ${plugin})
    endif()
endforeach()

list(SORT ENABLED_CONDITIONAL_PLUGINS)
list(SORT DISABLED_CONDITIONAL_PLUGINS)

# Add unconditional subdirs
#
foreach(plugin ${PLUGINS})
    add_subdirectory(src/${plugin})
endforeach()

# Add conditional subdirs
#
foreach(plugin ${ENABLED_CONDITIONAL_PLUGINS})
    add_subdirectory(src/${plugin})
endforeach()

# Modify the lists for displaying
#
string(TOUPPER "${ENABLED_CONDITIONAL_PLUGINS}" ENABLED_CONDITIONAL_PLUGINS)
string(TOUPPER "${DISABLED_CONDITIONAL_PLUGINS}" DISABLED_CONDITIONAL_PLUGINS)

# Export plugins to the parent cmake file to print statistics
#
set(PLUGINS ${PLUGINS} PARENT_SCOPE)
set(ENABLED_CONDITIONAL_PLUGINS ${ENABLED_CONDITIONAL_PLUGINS} PARENT_SCOPE)
set(DISABLED_CONDITIONAL_PLUGINS ${DISABLED_CONDITIONAL_PLUGINS} PARENT_SCOPE)

# Installation
#
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/layouts/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/sail/layouts")
