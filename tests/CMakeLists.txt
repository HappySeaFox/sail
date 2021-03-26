enable_testing()

# Dependencies
#
add_subdirectory(munit)
add_subdirectory(sail-comparators)

# Actual tests
#
add_subdirectory(sail-common)
add_subdirectory(sail)
