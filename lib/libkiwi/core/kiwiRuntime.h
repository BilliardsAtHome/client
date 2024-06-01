#ifndef LIBKIWI_CORE_RUNTIME_H
#define LIBKIWI_CORE_RUNTIME_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/math/kiwiAlgorithm.h>
#include <revolution/OS.h>

namespace kiwi {

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Linker Generated Symbols
 */
/**@{*/
extern funcptr_t __ctor_loc;
extern funcptr_t __ctor_end;

extern void* const _f_init;
extern void* const _e_init;

extern void* const _f_text;
extern void* const _e_text;

extern void* const _f_ctors;
extern void* const _e_ctors;

extern void* const _f_dtors;
extern void* const _e_dtors;

extern void* const _f_rodata;
extern void* const _e_rodata;

extern void* const _f_data;
extern void* const _e_data;
/**@}*/

namespace {

/**
 * @name ELF section information
 */
/**@{*/
/**
 * @brief Gets the start address of the .init ELF section
 */
const void* GetInitStart() {
    return &_f_init;
}
/**
 * @brief Gets the end address of the .init ELF section
 */
const void* GetInitEnd() {
    return &_e_init;
}
/**
 * @brief Gets the size of the .init ELF section
 */
u32 GetInitSize() {
    return PtrDistance(GetInitStart(), GetInitEnd());
}

/**
 * @brief Gets the start address of the .text ELF section
 */
const void* GetTextStart() {
    return &_f_text;
}
/**
 * @brief Gets the end address of the .text ELF section
 */
const void* GetTextEnd() {
    return &_e_text;
}
/**
 * @brief Gets the size of the .text ELF section
 */
u32 GetTextSize() {
    return PtrDistance(GetTextStart(), GetTextEnd());
}

/**
 * @brief Gets the start address of the .ctors ELF section
 */
const void* GetCtorsStart() {
    return &_f_ctors;
}
/**
 * @brief Gets the end address of the .ctors ELF section
 */
const void* GetCtorsEnd() {
    return &_e_ctors;
}
/**
 * @brief Gets the size of the .ctors ELF section
 */
u32 GetCtorsSize() {
    return PtrDistance(GetCtorsStart(), GetCtorsEnd());
}

/**
 * @brief Gets the start address of the .dtors ELF section
 */
const void* GetDtorsStart() {
    return &_f_dtors;
}
/**
 * @brief Gets the end address of the .dtors ELF section
 */
const void* GetDtorsEnd() {
    return &_e_dtors;
}
/**
 * @brief Gets the size of the .dtors ELF section
 */
u32 GetDtorsSize() {
    return PtrDistance(GetDtorsStart(), GetDtorsEnd());
}

/**
 * @brief Gets the start address of the .rodata ELF section
 */
const void* GetRodataStart() {
    return &_f_rodata;
}
/**
 * @brief Gets the end address of the .rodata ELF section
 */
const void* GetRodataEnd() {
    return &_e_rodata;
}
/**
 * @brief Gets the size of the .rodata ELF section
 */
u32 GetRodataSize() {
    return PtrDistance(GetRodataStart(), GetRodataEnd());
}

/**
 * @brief Gets the start address of the .data ELF section
 */
const void* GetDataStart() {
    return &_f_data;
}
/**
 * @brief Gets the end address of the .data ELF section
 */
const void* GetDataEnd() {
    return &_e_data;
}
/**
 * @brief Gets the size of the .data ELF section
 */
u32 GetDataSize() {
    return PtrDistance(GetDataStart(), GetDataEnd());
}
/**@}*/

/**
 * @brief Tests whether a memory address lies on the stack
 *
 * @param addr Memory address
 */
bool IsStack(const void* addr) {
    return addr >= _stack_end && addr < _stack_addr;
}

} // namespace

#ifdef __cplusplus
}
#endif

/**
 * @brief Simulate a breakpoint for Dolphin debugging
 */
#define K_DEBUG_BREAK()                                                        \
    {                                                                          \
        BOOL __enabled__ = OSDisableInterrupts();                              \
        K_LOG("************ BREAKPOINT! ************\n");                      \
        K_LOG_EX("Source: " __FILE__ "(%d)\n", __LINE__);                      \
        volatile int __x__ = 1;                                                \
        do {                                                                   \
            ;                                                                  \
        } while (__x__);                                                       \
        OSRestoreInterrupts(__enabled__);                                      \
    }

/**
 * @name Assembly utilities
 */
/**@{*/
/**
 * @brief Begin an inline assembly block
 */
#define K_ASM_BEGIN asm volatile

/**
 * @brief Copy the contents of a GPR to a variable
 * @note Compiler optimizations usually alias the variable to the specified GPR
 */
#define K_GET_GPR(r, var)                                                      \
    K_ASM_BEGIN {                                                              \
        mr var, r;                                                             \
    }
/**
 * @brief Copy the contents of a variable to a GPR
 */
#define K_SET_GPR(r, var)                                                      \
    K_ASM_BEGIN {                                                              \
        mr r, var;                                                             \
    }

/**
 * @brief Create a stack frame and save all GPRs
 */
#define K_SAVE_GPRS                                                            \
    K_ASM_BEGIN {                                                              \
        stwu r1, -0x90(r1);                                                    \
        stmw r3, 0xC(r1);                                                      \
        mflr r12;                                                              \
        stw r12, 0x8(r1);                                                      \
    }

/**
 * @brief Destroy the stack frame and restore all GPRs
 */
#define K_REST_GPRS                                                            \
    K_ASM_BEGIN {                                                              \
        lwz r12, 0x8(r1);                                                      \
        mtlr r12;                                                              \
        lmw r3, 0xC(r1);                                                       \
        addi r1, r1, 0x90;                                                     \
    }
/**@}*/

} // namespace kiwi

#endif
