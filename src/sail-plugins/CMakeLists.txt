# Unconditional plugins that don't require external dependencies
#
set(PLUGINS "")

list(SORT PLUGINS)

# Conditional plugins that require external dependencies like libjpeg
#
set(CONDITIONAL_PLUGINS jpeg)

# Actual dependencies. Will use pkg_check_modules() to find them
#
set(CONDITIONAL_DEPENDENCIES libjpeg)

################################################################################

# Iterate over conditional plugins and enable/disable them
#
list(LENGTH CONDITIONAL_PLUGINS length)
math(EXPR length ${length}-1)

set(ENABLED_CONDITIONAL_PLUGINS "")
set(DISABLED_CONDITIONAL_PLUGINS "")

foreach(i RANGE ${length})
    list(GET CONDITIONAL_PLUGINS      ${i} plugin)
    list(GET CONDITIONAL_DEPENDENCIES ${i} plugin_dependencies)

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
string(REPLACE ";" " " ENABLED_CONDITIONAL_PLUGINS "${ENABLED_CONDITIONAL_PLUGINS}")
string(REPLACE ";" " " DISABLED_CONDITIONAL_PLUGINS "${DISABLED_CONDITIONAL_PLUGINS}")

string(TOUPPER "${ENABLED_CONDITIONAL_PLUGINS}" ENABLED_CONDITIONAL_PLUGINS)
string(TOUPPER "${DISABLED_CONDITIONAL_PLUGINS}" DISABLED_CONDITIONAL_PLUGINS)

message("Enabled conditional plugins: ${ENABLED_CONDITIONAL_PLUGINS}")
message("Disabled conditional plugins: ${DISABLED_CONDITIONAL_PLUGINS}")
