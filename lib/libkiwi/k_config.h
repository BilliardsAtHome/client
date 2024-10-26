#ifndef LIBKIWI_CONFIG_H
#define LIBKIWI_CONFIG_H

// Your custom defines
#ifdef LIBKIWI_USER_CONFIG
#include LIBKIWI_USER_CONFIG
#endif

// Default to big endian byte order
#ifndef LIBKIWI_BIG_ENDIAN
#define LIBKIWI_BIG_ENDIAN
#endif

// Function macros
#define K_INLINE inline
#define K_DONT_INLINE __attribute__((never_inline))

// Expose private members only to Kamek hooks
#ifdef LIBKIWI_INTERNAL
#define LIBKIWI_KAMEK_PUBLIC public:
#else
#define LIBKIWI_KAMEK_PUBLIC
#endif

// C++ exclusive options
#ifdef __cplusplus

// Some versions of CW allow C++11
#if defined(__MWERKS__) && __option(cpp1x)
#define LIBKIWI_CPP1X
#elif __cplusplus >= 199711L
#define LIBKIWI_CPP1X
#endif

#ifndef LIBKIWI_CPP1X
#define nullptr NULL
#endif

#endif // __cplusplus

#endif
