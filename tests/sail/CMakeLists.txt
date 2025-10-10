sail_test(TARGET bugs                   SOURCES bugs.c                   LINK sail)
sail_test(TARGET io-produce-same-images SOURCES io-produce-same-images.c LINK sail sail-comparators)
sail_test(TARGET multi-frame            SOURCES multi-frame.c            LINK sail)
sail_test(TARGET edge-cases             SOURCES edge-cases.c             LINK sail)
sail_test(TARGET threading              SOURCES threading.c              LINK sail)
sail_test(TARGET advanced-api           SOURCES advanced-api.c           LINK sail sail-manip)
sail_test(TARGET deep-diver-api         SOURCES deep-diver-api.c         LINK sail sail-manip)
sail_test(TARGET technical-diver-api    SOURCES technical-diver-api.c    LINK sail sail-manip)

target_compile_definitions(edge-cases PRIVATE
    SAIL_TEST_IMAGES_ACCEPTANCE_PATH="${CMAKE_SOURCE_DIR}/tests/images/acceptance"
    SAIL_TEST_IMAGES_EDGE_CASES_PATH="${CMAKE_SOURCE_DIR}/tests/images/edge-cases"
)
