# Enable as many warnings as possible
#
macro(sail_enable_warnings)
    if (MSVC)
        if (CMAKE_C_FLAGS MATCHES "/W[0-4]")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        else()
            string(APPEND CMAKE_C_FLAGS " " "/W4")
            string(APPEND CMAKE_CXX_FLAGS " " "/W4")
        endif()
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        string(APPEND CMAKE_C_FLAGS " " "-Wall -Wextra")
        string(APPEND CMAKE_CXX_FLAGS " " "-Wall -Wextra")

        if (SAIL_DEV)
            string(APPEND CMAKE_C_FLAGS " " "-pedantic")
            string(APPEND CMAKE_CXX_FLAGS " " "-pedantic")
        endif()
    endif()
endmacro()

