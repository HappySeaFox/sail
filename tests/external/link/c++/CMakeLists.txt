cmake_minimum_required(VERSION 3.10)

# Enable strict C++17
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

project(external-c++-api-link)

add_executable(external-c++-api-link main.cpp)

# Add SAIL dependency
#
find_package(SailC++ REQUIRED)
target_link_libraries(external-c++-api-link SAIL::sail-c++)
