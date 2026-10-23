sail_test(TARGET sail-test-bugs                   SOURCES bugs.c                    LINK sail)
sail_test(TARGET sail-test-io-expanding-buffer    SOURCES io-expanding-buffer.c     LINK sail)
sail_test(TARGET sail-test-io-file                SOURCES io-file.c                 LINK sail)
sail_test(TARGET sail-test-io-memory              SOURCES io-memory.c               LINK sail)
sail_test(TARGET sail-test-io-produce-same-images SOURCES io-produce-same-images.c  LINK sail sail-comparators)
sail_test(TARGET sail-test-multi-frame            SOURCES multi-frame.c             LINK sail)
sail_test(TARGET sail-test-edge-cases             SOURCES edge-cases.c              LINK sail)
sail_test(TARGET sail-test-threading              SOURCES threading.c               LINK sail)
sail_test(TARGET sail-test-threading-stress       SOURCES threading-stress.c        LINK sail sail-manip)
sail_test(TARGET sail-test-advanced-api           SOURCES advanced-api.c            LINK sail sail-manip)
sail_test(TARGET sail-test-deep-diver-api         SOURCES deep-diver-api.c          LINK sail sail-manip)
sail_test(TARGET sail-test-technical-diver-api    SOURCES technical-diver-api.c     LINK sail sail-manip)

target_compile_definitions(sail-test-edge-cases PRIVATE
    SAIL_TEST_IMAGES_ACCEPTANCE_PATH="${CMAKE_SOURCE_DIR}/tests/images/acceptance"
    SAIL_TEST_IMAGES_EDGE_CASES_PATH="${CMAKE_SOURCE_DIR}/tests/images/edge-cases"
)

# Custom Zlib-based I/O test
find_package(ZLIB)
if (ZLIB_FOUND)
    sail_test(TARGET sail-test-custom-io SOURCES custom-io.c LINK sail)
    target_link_libraries(sail-test-custom-io PRIVATE ZLIB::ZLIB)
    target_compile_definitions(sail-test-custom-io PRIVATE
        SAIL_TEST_IMAGES_ACCEPTANCE_PATH="${CMAKE_SOURCE_DIR}/tests/images/acceptance"
    )
else()
    message(WARNING "ZLIB not found, skipping custom-io test")
endif()
