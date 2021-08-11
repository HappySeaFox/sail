enable_testing()

# Dependencies
#
add_subdirectory(munit)
add_subdirectory(sail-comparators)
add_subdirectory(sail-dump)

# Actual tests
#
add_subdirectory(sail-common)
add_subdirectory(sail)
add_subdirectory(sail-manip)
add_subdirectory(bindings/c++)
