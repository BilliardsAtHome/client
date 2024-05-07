#ifndef LIBKIWI_UTIL_STORAGE_FOR_H
#define LIBKIWI_UTIL_STORAGE_FOR_H
#include <libkiwi/kernel/kiwiAssert.h>
#include <libkiwi/kernel/kiwiStackChecker.h>
#include <types.h>

namespace kiwi {

/**
 * @brief Uninitialized storage for a specific type
 */
template <typename T> class StorageFor {
public:
    /**
     * @brief Constructor
     */
    StorageFor() : mInitialized(false), mpStorage(NULL) {
        mpStorage = new u8[sizeof(T)];
        K_ASSERT(mpStorage != NULL);
    }

    /**
     * @brief Destructor
     */
    ~StorageFor() {
        // Destroy object
        if (mInitialized) {
            Get().~T();
        }

        // Free storage
        delete mpStorage;
    }

    /**
     * @brief Whether this storage has been initialized
     */
    bool Initialized() const {
        return mInitialized;
    }

    /**
     * @brief Set value
     *
     * @param x New value
     */
    void Set(const T& x) {
        // Construct in-place
        K_ASSERT(mpStorage != NULL);
        new (mpStorage) T(x);
        mInitialized = true;
    }

    /**
     * @brief Get value
     *
     * @return Value reference
     */
    T& Get() {
        K_ASSERT(mpStorage != NULL);
        K_ASSERT_EX(mInitialized, "Uninitialized storage");
        return *reinterpret_cast<T*>(mpStorage);
    }

    /**
     * @brief Get value (read-only)
     *
     * @return Value reference (read-only)
     */
    const T& Get() const {
        K_ASSERT(mpStorage != NULL);
        K_ASSERT_EX(mInitialized, "Uninitialized storage");
        return *reinterpret_cast<T*>(mpStorage);
    }

    /**
     * @brief Assign a new value
     *
     * @param x New value
     */
    StorageFor& operator=(const T& x) {
        Set(x);
    }

    // Dereference access
    T& operator*() {
        return Get();
    }
    const T& operator*() const {
        return Get();
    }

    // Pointer access
    T* operator->() {
        return &Get();
    }
    const T* operator->() const {
        return &Get();
    }

private:
    // Whether the storage has been initialized
    bool mInitialized;
    // Storage buffer
    u8* mpStorage;
};

} // namespace kiwi

#endif