# Intended to be included by SAIL.
#
function(sail_check_alignas)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/sail_check_alignas.c
"
int main(int argc, char *argv[]) {
    _Alignas(int) unsigned char data[4];
    return 0;
}
")

    message(STATUS "Performing Test _Alignas()")

    try_compile(SAIL_HAVE_ALIGNAS
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_BINARY_DIR}/sail_check_alignas.c
                C_STANDARD 11
                C_STANDARD_REQUIRED ON)

    if (SAIL_HAVE_ALIGNAS)
        message(STATUS "Performing Test _Alignas() - Success")
    else()
        message(STATUS "Performing Test _Alignas() - Failed")
    endif()
endfunction()
