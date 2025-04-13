#pragma once

#ifdef MOON_IS_MONOLITHIC
    #define MOON_API
#else
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
#endif

// Debug break cross-platform implementation
#ifdef DEBUG
    #if defined(_MSC_VER)
        #define MOON_DEBUGBREAK() __debugbreak()
    #elif defined(__MINGW32__) || defined(__MINGW64__)
        #define MOON_DEBUGBREAK() __debugbreak()
    #elif defined(__clang__)
        #if defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
            #define MOON_DEBUGBREAK() __builtin_debugtrap()
        #else
            #include <signal.h>
            #define MOON_DEBUGBREAK() raise(SIGTRAP)
        #endif
    #elif defined(__GNUC__)
        #include <signal.h>
        #define MOON_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Platform doesn't support debugbreak yet!"
    #endif
#endif

// assertions
#ifdef DEBUG
    #define MOON_ENABLE_ASSERTS
#endif

#ifdef MOON_ENABLE_ASSERTS
    #define MOON_ASSERT(x, ...) { if(!(x)) { MOON_ERROR("Assertion Failed at {}:{}: {}", __FILE__, __LINE__, __VA_ARGS__); MOON_DEBUGBREAK(); } }
    #define MOON_CORE_ASSERT(x, ...) { if(!(x)) { MOON_CORE_ERROR("Assertion Failed at {}:{}: {}", __FILE__, __LINE__, __VA_ARGS__); MOON_DEBUGBREAK(); } }
#else // these macros must expand to something, otherwise the variable may be marked as unused and propogate a warning in -Werror mode
    #define MOON_ASSERT(x, ...) do { (void)(x); } while(0)
    #define MOON_CORE_ASSERT(x, ...) do { (void)(x); } while(0)
#endif

#define BIT(x) (1 << x)


#include <memory>
namespace moon
{
    template<typename T>
    using ref = std::shared_ptr<T>;

    template<typename T>
    using scope = std::unique_ptr<T>;
}
