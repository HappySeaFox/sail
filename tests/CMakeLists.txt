# Test images
#
set(SAIL_TEST_IMAGES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/images)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/images/test-images.h.in ${PROJECT_BINARY_DIR}/include/test-images.h @ONLY)

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
if (SAIL_BUILD_BINDINGS)
  add_subdirectory(bindings/c++)
endif()
