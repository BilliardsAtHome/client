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
     * @brief Set value
     *
     * @param t New value
     */
    void Set(const T& t) {
        Assign(&t);
    }
    /**
     * @brief Construct value
     */
    void Emplace() {
        Assign(NULL);
    }

    /**
     * @brief Assign a new value
     *
     * @param t New value
     */
    StorageFor& operator=(const T& t) {
        Set(t);
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
    /**
     * @brief Assign value through copy
     *
     * @param t New value
     */
    void Assign(const T* t) {
        K_ASSERT(mpStorage != NULL);

        // Construct in-place
        if (t != NULL) {
            new (mpStorage) T(*t);
        } else {
            new (mpStorage) T();
        }

        mInitialized = true;
    }

private:
    // Whether the storage has been initialized
    bool mInitialized;
    // Storage buffer
    u8* mpStorage;
};

} // namespace kiwi

#endif