#ifndef LIBKIWI_KERNEL_STACK_CHECKER_H
#define LIBKIWI_KERNEL_STACK_CHECKER_H
#include <revolution/OS.h>
#include <types.h>

namespace kiwi {

/**
 * @brief Buffer overflow checker
 */
class StackChecker {
public:
    /**
     * @brief Constructor
     */
    StackChecker() : mGuard(0) {
        // Lazy initialization
        if (sStackCheckGuard == 0) {
            Initialize();
        }

        mGuard = sStackCheckGuard;
    }

    /**
     * @brief Destructor
     */
    ~StackChecker() {
        K_ASSERT_EX(mGuard == sStackCheckGuard, "Buffer overflow detected!");
    }

private:
    static void Initialize();
    static u32 sStackCheckGuard;

    // Guard value
    volatile u32 mGuard;
};

} // namespace kiwi

#endif