#ifndef LIBKIWI_KERNEL_ASSERT_H
#define LIBKIWI_KERNEL_ASSERT_H
#include <types.h>

// General macros for external usage
#define LOG(msg) K_LOG(msg)
#define LOG_EX(msg, ...) K_LOG_EX(msg, __VA_ARGS__)

#define ASSERT(msg) K_ASSERT(msg)
#define ASSERT_EX(msg, ...) K_ASSERT_EX(msg, __VA_ARGS__)

#define STATIC_ASSERT(expr) K_STATIC_ASSERT(expr)
#define STATIC_ASSERT_EX(expr, msg) K_STATIC_ASSERT_EX(expr, msg)

// For compiling modern libraries
#define static_assert(expr, msg) K_STATIC_ASSERT_EX(expr, msg)

#ifndef NDEBUG
// Log a message to the console
#define K_LOG(msg) kiwi_log(msg)
// Log a variadic message to the console
#define K_LOG_EX(msg, ...) kiwi_log(msg, __VA_ARGS__)

// Log a message to the console when a condition is met
#define K_WARN(exp, msg) ((exp) ? (K_LOG(msg), 1) : 0)
// Log a variadic message to the console when a condition is met
#define K_WARN_EX(exp, msg, ...) ((exp) ? (K_LOG_EX(msg, __VA_ARGS__), 1) : 0)

// Assert a condition and halt execution when it fails to hold.
// Can be used as either an expression or statement.
#define K_ASSERT(exp)                                                          \
    (!(exp) ? (kiwi_fail_assert(__FILE__, __LINE__, #exp), 0) : 1)
// Assert a condition and halt execution when it fails to hold,
// displaying a custom error message.
// Can be used as either an expression or statement.
#define K_ASSERT_EX(exp, ...)                                                  \
    (!(exp) ? (kiwi_fail_assert(__FILE__, __LINE__, __VA_ARGS__), 0) : 1)
#else
#define K_LOG(msg)
#define K_LOG_EX(msg, ...)
#define K_WARN(exp, msg)
#define K_WARN_EX(exp, msg, ...)
#define K_ASSERT(exp, ...)
#define K_ASSERT_EX(exp, ...)
#endif

// Compile-time assertion
#define K_STATIC_ASSERT(expr) extern u8 __K_PREDICATE[(expr) ? 1 : -1]
#define K_STATIC_ASSERT_EX(expr, msg) K_STATIC_ASSERT(expr)

#ifdef __cplusplus
extern "C" {
#endif

void kiwi_log(const char* msg, ...);
void kiwi_fail_assert(const char* file, int line, const char* msg, ...);

#ifdef __cplusplus
}
#endif

// Override RP_GET_INSTANCE
#define RP_GET_INSTANCE(T)                                                     \
    (K_ASSERT(T::GetInstance() != NULL), T::GetInstance())

#endif
