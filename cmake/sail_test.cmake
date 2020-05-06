# Intended to be included by every test.
#
macro(sail_test)
    cmake_parse_arguments(SAIL_TEST "" "TARGET" "SOURCES" ${ARGN})

    # Add a test
    #
    add_executable("${SAIL_TEST_TARGET}" "${SAIL_TEST_SOURCES}")

    add_test(NAME "${SAIL_TEST_TARGET}" COMMAND "${SAIL_TEST_TARGET}")

    # Depend on sail
    #
    target_link_libraries("${SAIL_TEST_TARGET}" sail)

    # Depend on sail-munit
    #
    target_link_libraries("${SAIL_TEST_TARGET}" sail-munit)
endmacro()
