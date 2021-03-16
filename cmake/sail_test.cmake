# Intended to be included by every test.
#
macro(sail_test)
    cmake_parse_arguments(SAIL_TEST "" "TARGET" "SOURCES;LINK" ${ARGN})

    # Add a test
    #
    add_executable(${SAIL_TEST_TARGET} ${SAIL_TEST_SOURCES})

    if (WIN32)
        add_test(NAME ${SAIL_TEST_TARGET} WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin COMMAND ${SAIL_TEST_TARGET})
    else()
        add_test(NAME ${SAIL_TEST_TARGET} COMMAND ${SAIL_TEST_TARGET})
    endif()

    # Depend on sail
    #
    target_link_libraries(${SAIL_TEST_TARGET} sail)

    # Depend on sail-munit
    #
    target_link_libraries(${SAIL_TEST_TARGET} sail-munit)

    # Depend on LINK
    #
    if (SAIL_TEST_LINK)
        target_link_libraries(${SAIL_TEST_TARGET} ${SAIL_TEST_LINK})
    endif()
endmacro()
