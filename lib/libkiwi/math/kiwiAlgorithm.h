#ifndef LIBKIWI_MATH_ALGORITHM_H
#define LIBKIWI_MATH_ALGORITHM_H
#include <libkiwi/k_types.h>
#include <nw4r/math.h>

namespace kiwi {
namespace {

/**
 * Get maximum of two values
 */
template <typename T> K_INLINE const T& Max(const T& a, const T& b) {
    return a < b ? b : a;
}

/**
 * Get minimum of two values
 */
template <typename T> K_INLINE const T& Min(const T& a, const T& b) {
    return b < a ? b : a;
}

/**
 * Absolute value
 */
template <typename T> K_INLINE T Abs(const T& x) {
#ifdef __MWCC__
    return __abs(x);
#else
    return x < 0 ? -x : x;
#endif
}

/**
 * @brief Logarithm
 */
template <typename T> K_INLINE T Log(const T& x) {
    return nw4r::math::FLog(static_cast<f32>(x));
}

/**
 * Clamp value to range
 */
template <typename T> K_INLINE T Clamp(const T& x, const T& min, const T& max) {
    if (x < min) {
        return min;
    }

    if (x > max) {
        return max;
    }

    return x;
}

/**
 * Distance between two pointers
 */
K_INLINE std::ptrdiff_t PtrDistance(const void* start, const void* end) {
    return reinterpret_cast<std::uintptr_t>(end) -
           reinterpret_cast<std::uintptr_t>(start);
}

/**
 * Add offset to pointer
 */
K_INLINE void* AddToPtr(void* ptr, std::ptrdiff_t ofs) {
    return static_cast<char*>(ptr) + ofs;
}
K_INLINE const void* AddToPtr(const void* ptr, std::ptrdiff_t ofs) {
    return static_cast<const char*>(ptr) + ofs;
}
template <typename T> K_INLINE T* AddToPtr(void* ptr, std::ptrdiff_t ofs) {
    return reinterpret_cast<T*>(static_cast<char*>(ptr) + ofs);
}
template <typename T>
K_INLINE const T* AddToPtr(const void* ptr, std::ptrdiff_t ofs) {
    return reinterpret_cast<const T*>(static_cast<const char*>(ptr) + ofs);
}

} // namespace
} // namespace kiwi

#endif
