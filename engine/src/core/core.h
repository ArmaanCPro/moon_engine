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

#ifdef MOON_ENABLE_ASSERTS
    #define MOON_ASSERT(x, ...) { if(!(x)) { MOON_ERROR("Assertion Failed: {0} ({1}:{2} in function {3})", __VA_ARGS__, __FILE__, __LINE__, __FUNCTION__); __debugbreak(); } }
    #define MOON_CORE_ASSERT(x, ...) { if(!(x)) { MOON_CORE_ERROR("Assertion Failed: {0} ({1}:{2})", __VA_ARGS__, __FILE__, __LINE__); __debugbreak(); } }
#else
    #define MOON_ASSERT(x, ...)
    #define MOON_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
