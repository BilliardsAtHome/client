#ifndef LIBKIWI_MATH_ALGORITHM_H
#define LIBKIWI_MATH_ALGORITHM_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <nw4r/math.h>

namespace kiwi {
namespace {

/**
 * @brief Gets minimum of two values
 *
 * @param a First value
 * @param b Second value
 */
template <typename T> K_INLINE const T& Min(const T& a, const T& b) {
    return b < a ? b : a;
}
/**
 * @brief Gets maximum of two values
 *
 * @param a First value
 * @param b Second value
 */
template <typename T> K_INLINE const T& Max(const T& a, const T& b) {
    return a < b ? b : a;
}

/**
 * @brief Absolute value
 *
 * @param x Initial value
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
 *
 * @param x Initial value
 */
template <typename T> K_INLINE T Log(const T& x) {
    f32 y = nw4r::math::FLog(static_cast<f32>(x));
    return static_cast<T>(y);
}

/**
 * @brief Clamps value to range
 *
 * @param x Initial value
 * @param min Range minimum (exclusive)
 * @param max Range maximum (exclusive)
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
 * @brief Gets the distance between two pointers
 *
 * @param start Start pointer
 * @param end End pointer
 */
K_INLINE std::ptrdiff_t PtrDistance(const void* start, const void* end) {
    return reinterpret_cast<std::uintptr_t>(end) -
           reinterpret_cast<std::uintptr_t>(start);
}

/**
 * @brief Adds offset to pointer
 *
 * @param ptr Base pointer
 * @param ofs Offset to apply
 */
K_INLINE void* AddToPtr(void* ptr, std::ptrdiff_t ofs) {
    return static_cast<char*>(ptr) + ofs;
}

/**
 * @brief Adds offset to pointer (const-view)
 *
 * @param ptr Base pointer
 * @param ofs Offset to apply
 */
K_INLINE const void* AddToPtr(const void* ptr, std::ptrdiff_t ofs) {
    return static_cast<const char*>(ptr) + ofs;
}

/**
 * @brief Adds offset to pointer
 *
 * @tparam T Type of resulting pointer
 * @param ptr Base pointer
 * @param ofs Offset to apply
 */
template <typename T> K_INLINE T* AddToPtr(void* ptr, std::ptrdiff_t ofs) {
    return reinterpret_cast<T*>(static_cast<char*>(ptr) + ofs);
}

/**
 * @brief Adds offset to pointer (const-view)
 *
 * @tparam T Type of resulting pointer
 * @param ptr Base pointer
 * @param ofs Offset to apply
 */
template <typename T>
K_INLINE const T* AddToPtr(const void* ptr, std::ptrdiff_t ofs) {
    return reinterpret_cast<const T*>(static_cast<const char*>(ptr) + ofs);
}

} // namespace
} // namespace kiwi

#endif
