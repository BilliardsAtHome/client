#include <libkiwi.h>

namespace kiwi {
namespace detail {

/**
 * @brief Constructor
 */
ThreadImpl::ThreadImpl() : mpOSThread(NULL), mpThreadStack(NULL) {
    // Thread & stack aligned to 32
    mpOSThread = new (32) OSThread();
    K_ASSERT(mpOSThread != NULL);
    mpThreadStack = new (32) u8[cStackSize];
    K_ASSERT(mpThreadStack != NULL);

    BOOL success =
        OSCreateThread(mpOSThread, NULL, NULL, mpThreadStack + cStackSize,
                       cStackSize, cPriority, 0);
    K_ASSERT(success);
}

/**
 * @brief Destructor
 */
ThreadImpl::~ThreadImpl() {
    K_ASSERT(mpOSThread != NULL);
    K_ASSERT_EX(*mpOSThread->stackEnd == OS_THREAD_STACK_MAGIC,
                "Thread stack overflow!!!");

    // Wait for task to finish
    Join();

    // Destroy thread
    if (!OSIsThreadTerminated(mpOSThread)) {
        OSDetachThread(mpOSThread);
        OSCancelThread(mpOSThread);
    }

    delete mpOSThread;
    delete mpThreadStack;
}

/**
 * @brief Begin execution on this thread
 */
void ThreadImpl::Start() {
    K_ASSERT(mpOSThread != NULL);
    K_ASSERT(mpOSThread->state == OS_THREAD_STATE_READY);
    K_ASSERT_EX(mpOSThread->context.srr0 != 0, "No function to call");

    // Resume thread to call function
    s32 suspend = OSResumeThread(mpOSThread);
    K_ASSERT(suspend == 1);
}

/**
 * @brief Wait for this thread to finish executing
 */
void ThreadImpl::Join() {
    K_ASSERT(mpOSThread != NULL);
    OSJoinThread(mpOSThread, NULL);
}

/**
 * @brief Set a function for this thread to run
 *
 * @param addr Function address (new SRR0 value)
 */
void ThreadImpl::SetFunction(const void* addr) {
    K_ASSERT(mpOSThread != NULL);
    K_ASSERT(addr != 0);
    mpOSThread->context.srr0 = BitCast<u32>(addr);
}

/**
 * @brief Set a GPR's value in this thread
 *
 * @param i GPR number
 * @param value New value
 */
void ThreadImpl::SetGPR(u32 i, u32 value) {
    K_ASSERT(mpOSThread != NULL);
    K_ASSERT(i >= 0 && i < LENGTHOF(mpOSThread->context.gprs));
    mpOSThread->context.gprs[i] = value;
}

} // namespace detail
} // namespace kiwi