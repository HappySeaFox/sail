if (SAIL_DEV)
    enable_testing()

    add_subdirectory(munit)
    add_subdirectory(sail)
endif()
