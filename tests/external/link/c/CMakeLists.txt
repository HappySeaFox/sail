cmake_minimum_required(VERSION 3.10)

project(external-c-api-link)

add_executable(external-c-api-link main.c)

# Add SAIL dependency
#
find_package(Sail REQUIRED)
target_link_libraries(external-c-api-link SAIL::sail)
