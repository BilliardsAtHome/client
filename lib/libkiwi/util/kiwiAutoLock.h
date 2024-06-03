#ifndef LIBKIWI_UTIL_AUTOLOCK_H
#define LIBKIWI_UTIL_AUTOLOCK_H
#include <libkiwi/k_types.h>
#include <revolution/OS.h>

namespace kiwi {

/**
 * @brief Generic scoped lock
 */
template <typename T> class AutoLock : private NonCopyable {
public:
    AutoLock(T& object) : mObject(object) {
        Lock();
    }
    ~AutoLock() {
        Unlock();
    }

    // Implement these for custom types
    void Lock();
    void Unlock();

private:
    T& mObject; // Locked object
};

/**
 * @brief OSMutex specializations
 */
template <> K_INLINE void AutoLock<OSMutex>::Lock() {
    if (OSGetCurrentThread() != NULL) {
        OSLockMutex(&mObject);
    }
}
template <> K_INLINE void AutoLock<OSMutex>::Unlock() {
    if (OSGetCurrentThread() != NULL) {
        OSUnlockMutex(&mObject);
    }
}

typedef AutoLock<OSMutex> AutoMutexLock;

/**
 * @brief OS interrupt scoped lock
 */
class AutoInterruptLock : private NonCopyable {
public:
    AutoInterruptLock(bool enable = false)
        : mEnabled(enable ? OSEnableInterrupts() : OSDisableInterrupts()) {}
    ~AutoInterruptLock() {
        OSRestoreInterrupts(mEnabled);
    }

private:
    BOOL mEnabled; // Interrupt status
};

} // namespace kiwi

#endif
