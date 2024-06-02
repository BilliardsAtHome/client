#ifndef LIBKIWI_CORE_THREAD_H
#define LIBKIWI_CORE_THREAD_H
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiBitCast.h>
#include <revolution/OS.h>

namespace kiwi {
namespace detail {

/**
 * @brief Common thread implementation
 */
class ThreadImpl {
public:
    /**
     * @brief Waits for this thread to finish executing
     */
    void Join();

protected:
    /**
     * @brief Constructor
     */
    ThreadImpl();

    /**
     * @brief Destructor
     */
    ~ThreadImpl();

    /**
     * @brief Begins execution on this thread
     */
    void Start();

    /**
     * @brief Sets a function for this thread to run
     *
     * @param addr Function address (new SRR0 value)
     */
    void SetFunction(const void* addr);
    /**
     * @brief Sets a GPR's value in this thread
     *
     * @param i GPR number
     * @param value New value
     */
    void SetGPR(u32 i, u32 value);

    /**
     * @brief Sets a member function to run on this thread
     *
     * @param fn Function
     * @param obj Class instance
     */
    template <typename TFunc, typename TClass>
    void SetMemberFunction(TFunc fn, const TClass& obj);

private:
    OSThread* mpOSThread; // RVL thread
    u8* mpThreadStack;    // RVL thread stack

    /**
     * @brief Thread stack size
     */
    static const u32 scStackSize = 0x4000;
    /**
     * @brief Thread priority
     */
    static const s32 scPriority = OS_PRIORITY_MAX / 2;
};

} // namespace detail

/**
 * @brief Similar to std::thread
 * @note Only allows GPR arguments
 */
class Thread : public detail::ThreadImpl {
public:
    // Thread function parameter
    typedef void* Param;

public:
    /**
     * Non-member function
     */

    /**
     * @brief Constructor
     *
     * @param fn Static, no-parameter function
     */
    template <typename TRet> Thread(TRet (*fn)()) {
        K_ASSERT(fn != NULL);

        SetFunction(fn);
        Start();
    }

    /**
     * @brief Constructor
     *
     * @param fn Static, single-parameter function
     * @param arg Function argument
     */
    template <typename TRet> Thread(TRet (*fn)(Param), Param arg) {
        K_ASSERT(fn != NULL);

        SetFunction(fn);
        SetGPR(3, BitCast<u32>(arg));
        Start();
    }

    /**
     * Member function (non-const)
     */

    /**
     * @brief Constructor
     *
     * @param fn No-parameter member function
     * @param obj Class instance
     */
    template <typename TRet, typename TClass>
    Thread(TRet (TClass::*fn)(), TClass& obj) {
        K_ASSERT(fn != NULL);

        SetMemberFunction(fn, obj);
        Start();
    }

    /**
     * @brief Constructor
     *
     * @param fn Single-parameter member function
     * @param obj Class instance
     * @param arg Function argument
     */
    template <typename TRet, typename TClass>
    Thread(TRet (TClass::*fn)(Param), TClass& obj, Param arg) {
        K_ASSERT(fn != NULL);

        SetMemberFunction(fn, obj);
        SetGPR(4, BitCast<u32>(arg));
        Start();
    }

    /**
     * Member function (const)
     */

    /**
     * @brief Constructor
     *
     * @param fn No-parameter, const member function
     * @param obj Class instance
     */
    template <typename TRet, typename TClass>
    Thread(TRet (TClass::*fn)() const, const TClass& obj) {
        K_ASSERT(fn != NULL);

        SetMemberFunction(fn, obj);
        Start();
    }

    /**
     * @brief Constructor
     *
     * @param fn Single-parameter, const member function
     * @param obj Class instance
     * @param arg Function argument
     */
    template <typename TRet, typename TClass>
    Thread(TRet (TClass::*fn)(Param) const, const TClass& obj, Param arg) {
        K_ASSERT(fn != NULL);

        SetMemberFunction(fn, obj);
        SetGPR(4, BitCast<u32>(arg));
        Start();
    }
};

namespace detail {

/**
 * @brief Pointer-to-member-function (PTMF)
 */
struct MemberFunction {
    s32 toff; // 'This' pointer offset for target object type
    s32 voff; // Vtable offset into class
    union {
        s32 foff;   // Function offset into vtable
        void* addr; // Raw function address (if voff is -1)
    };
};

/**
 * @brief Sets a member function to run on this thread
 *
 * @param fn Function
 * @param obj Class instance
 */
template <typename TFunc, typename TClass>
K_DONT_INLINE void ThreadImpl::SetMemberFunction(TFunc fn, const TClass& obj) {
    K_STATIC_ASSERT_EX(sizeof(TFunc) == sizeof(MemberFunction),
                       "Not a member function");

    register const MemberFunction* ptmf;
    register u32 self;

    // clang-format off
    asm volatile {
        mr ptmf, r4 // fn  -> ptmf
        mr self, r5 // obj -> self
    }
    K_ASSERT(ptmf != NULL);
    K_ASSERT(self != NULL);
    // clang-format on

    K_ASSERT(mpOSThread != NULL);
    K_ASSERT(mpOSThread->state == OS_THREAD_STATE_READY);

    // Adjust this pointer
    self += ptmf->toff;
    SetGPR(3, self);

    // Non-virtual function?
    if (ptmf->voff == -1) {
        SetFunction(ptmf->addr);
        return;
    }

    // Find virtual function table
    const void** vt = BitCast<const void**>(self + ptmf->voff);

    // Find virtual function address
    K_ASSERT(ptmf->foff >= 0);
    SetFunction(vt[ptmf->foff / sizeof(void*)]);
}

} // namespace detail
} // namespace kiwi

#endif
