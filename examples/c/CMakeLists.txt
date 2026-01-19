# Junior API example with simple one-line image loading
add_executable(example-junior-api junior.c)
target_link_libraries(example-junior-api PRIVATE sail)
target_link_libraries(example-junior-api PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

# Advanced API example with animated/multi-paged images
add_executable(example-advanced-api advanced.c)
target_link_libraries(example-advanced-api PRIVATE sail)
target_link_libraries(example-advanced-api PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

# Deep Diver API example with full control over codec options
add_executable(example-deep-diver-api deep_diver.c)
target_link_libraries(example-deep-diver-api PRIVATE sail)
target_link_libraries(example-deep-diver-api PRIVATE $<BUILD_INTERFACE:sail-common-flags>)

# Technical Diver API example with custom I/O sources
add_executable(example-technical-diver-api technical_diver.c)
target_link_libraries(example-technical-diver-api PRIVATE sail)
target_link_libraries(example-technical-diver-api PRIVATE $<BUILD_INTERFACE:sail-common-flags>)
