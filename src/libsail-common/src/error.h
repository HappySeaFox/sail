#ifndef SAIL_ERROR_H
#define SAIL_ERROR_H

/*
 * Try to execute the specified SAIL function. If it fails, execute the rest of arguments
 * (so called cleanup), and return the error code. Use do/while to require ';' at the end.
 */
#define SAIL_TRY(sail_func, ...)  \
do {                              \
    int res;                      \
                                  \
    if ((res = sail_func) != 0) { \
        __VA_ARGS__;              \
        return res;               \
    }                             \
} while(0)

#endif
