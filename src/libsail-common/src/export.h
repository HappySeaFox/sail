#ifndef SAIL_EXPORT_H
#define SAIL_EXPORT_H

#if defined _WIN32 || defined __CYGWIN__
    #ifdef SAIL_BUILD
        #define SAIL_EXPORT __declspec(dllexport)
    #else
        #define SAIL_EXPORT __declspec(dllimport)
    #endif
#else
    #define SAIL_EXPORT
#endif

#endif
