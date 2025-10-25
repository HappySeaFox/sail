cmake_minimum_required(VERSION 3.18) # Require CMake 3.18 or later for SAIL
project(sail-python)

# Require C++14 (for pybind11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Python executable if provided, otherwise find it
if (PYTHON_EXECUTABLE)
    set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
endif()
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG)

if (NOT pybind11_FOUND)
    message(STATUS "pybind11 not found, fetching from GitHub")
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

if (NOT TARGET sail)
    if (NOT DEFINED SAIL_ALREADY_INCLUDED)
        # Try to find SAIL root directory
        if (DEFINED SAIL_ROOT_DIR AND EXISTS "${SAIL_ROOT_DIR}/CMakeLists.txt")
            set(SAIL_SOURCE_DIR "${SAIL_ROOT_DIR}")
        else()
            set(SAIL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
        endif()

        if (NOT EXISTS "${SAIL_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR
                "Cannot find SAIL root directory with CMakeLists.txt.\n"
                "Looked in: ${SAIL_SOURCE_DIR}\n"
                "\n"
                "This package cannot be built from PyPI source distribution.\n"
                "Please either:\n"
                "  1. Build from the SAIL git repository\n"
                "  2. Install pre-built wheels: pip install sailpy\n"
                "\n"
                "To build from source:\n"
                "  git clone https://github.com/HappySeaFox/sail\n"
                "  cd sail/src/bindings/python\n"
                "  pip install -e .\n"
            )
        endif()

        message(STATUS "Building SAIL from: ${SAIL_SOURCE_DIR}")

        set(SAIL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(SAIL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SAIL_BUILD_APPS OFF CACHE BOOL "" FORCE)
        set(SAIL_BUILD_BINDINGS ON CACHE BOOL "" FORCE)  # Need C++ bindings
        set(SAIL_COMBINE_CODECS ON CACHE BOOL "" FORCE)  # Combine all codecs into one library
        add_subdirectory(${SAIL_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/sail-build EXCLUDE_FROM_ALL)
        set(SAIL_ALREADY_INCLUDED YES)
    endif()
endif()

# Get SAIL version from parent project
get_directory_property(SAIL_VERSION DIRECTORY ${SAIL_SOURCE_DIR} DEFINITION PROJECT_VERSION)
message(STATUS "SAIL version: ${SAIL_VERSION}")

# Generate pyproject.toml with version
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml.in
    ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml
    @ONLY
)

set(PYTHON_BINDING_SOURCES
    src/codec_bindings.cpp
    src/core_bindings.cpp
    src/enums_bindings.cpp
    src/image_bindings.cpp
    src/log_bindings.cpp
    src/options_bindings.cpp
    src/sail_python.cpp
)

pybind11_add_module(_libsail ${PYTHON_BINDING_SOURCES})

target_link_libraries(_libsail PRIVATE
    sail
    sail-c++
    sail-common
    sail-manip
)

if (MSVC)
    target_compile_options(_libsail PRIVATE "/EHsc")
endif()

target_include_directories(_libsail PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_BINARY_DIR}/sail-build/include  # Generated config.h
)

install(TARGETS _libsail
        LIBRARY DESTINATION ${Python3_SITEARCH}/sailpy
        RUNTIME DESTINATION ${Python3_SITEARCH}/sailpy
)

install(DIRECTORY sailpy/
        DESTINATION ${Python3_SITEARCH}/sailpy
        FILES_MATCHING PATTERN "*.py"
)

if (BUILD_TESTING)
    enable_testing()
    add_test(
        NAME python_tests
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
