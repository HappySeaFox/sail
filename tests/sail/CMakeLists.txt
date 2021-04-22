set(SAIL_TEST_IMAGES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/images")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/images/test-images.h.in" "${PROJECT_BINARY_DIR}/include/test-images.h" @ONLY)

sail_test(TARGET io-produce-same-images SOURCES io-produce-same-images.c LINK sail sail-comparators)
