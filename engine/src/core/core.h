#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef MOON_ENGINE_EXPORTS
        #define MOON_API __declspec(dllexport)
    #else
        #define MOON_API __declspec(dllimport)
    #endif
#else
    #if __GNUC__ >= 4
        #define MOON_API __attribute__((visibility("default")))
    #else
        #define MOON_API
    #endif
#endif

#define BIT(x) (1 << x)
