#pragma once

// Always inline macro
#if defined(_MSC_VER)
    #define LEMON_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define LEMON_ALWAYS_INLINE inline __attribute__((always_inline))
#else
    #define LEMON_ALWAYS_INLINE inline
#endif

// Likely/unlikely branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
    #define LEMON_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define LEMON_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LEMON_LIKELY(x)   (x)
    #define LEMON_UNLIKELY(x) (x)
#endif
