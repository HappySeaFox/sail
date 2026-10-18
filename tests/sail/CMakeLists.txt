sail_test(TARGET sail-bugs                   SOURCES bugs.c                    LINK sail)
sail_test(TARGET sail-io-expanding-buffer    SOURCES io-expanding-buffer.c     LINK sail)
sail_test(TARGET sail-io-file                SOURCES io-file.c                 LINK sail)
sail_test(TARGET sail-io-memory              SOURCES io-memory.c               LINK sail)
sail_test(TARGET sail-io-produce-same-images SOURCES io-produce-same-images.c  LINK sail sail-comparators)
sail_test(TARGET sail-multi-frame            SOURCES multi-frame.c             LINK sail)
sail_test(TARGET sail-edge-cases             SOURCES edge-cases.c              LINK sail)
sail_test(TARGET sail-threading              SOURCES threading.c               LINK sail)
sail_test(TARGET sail-threading-stress       SOURCES threading-stress.c        LINK sail sail-manip)
sail_test(TARGET sail-advanced-api           SOURCES advanced-api.c            LINK sail sail-manip)
sail_test(TARGET sail-deep-diver-api         SOURCES deep-diver-api.c          LINK sail sail-manip)
sail_test(TARGET sail-technical-diver-api    SOURCES technical-diver-api.c     LINK sail sail-manip)

target_compile_definitions(sail-edge-cases PRIVATE
    SAIL_TEST_IMAGES_ACCEPTANCE_PATH="${CMAKE_SOURCE_DIR}/tests/images/acceptance"
    SAIL_TEST_IMAGES_EDGE_CASES_PATH="${CMAKE_SOURCE_DIR}/tests/images/edge-cases"
)

# Custom Zlib-based I/O test
find_package(ZLIB)
if (ZLIB_FOUND)
    sail_test(TARGET sail-custom-io SOURCES custom-io.c LINK sail)
    target_link_libraries(sail-custom-io PRIVATE ZLIB::ZLIB)
    target_compile_definitions(sail-custom-io PRIVATE
        SAIL_TEST_IMAGES_ACCEPTANCE_PATH="${CMAKE_SOURCE_DIR}/tests/images/acceptance"
    )
else()
    message(WARNING "ZLIB not found, skipping custom-io test")
endif()
