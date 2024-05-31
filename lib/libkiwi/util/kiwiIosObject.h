#ifndef LIBKIWI_UTIL_IOS_OBJECT_H
#define LIBKIWI_UTIL_IOS_OBJECT_H
#include <libkiwi/core/kiwiMemoryMgr.h>
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/util/kiwiIosVector.h>

namespace kiwi {

/**
 * @brief Strongly typed IOS I/O vector
 *
 * @tparam T Underlying object type
 */
template <typename T> class IosObject : public IosVectors, public NonCopyable {
public:
    /**
     * @brief Constructor
     */
    IosObject() : IosVectors(1) {
        Allocate();
    }

    /**
     * @brief Destructor
     */
    ~IosObject() {
        Ptr()->~T();
    }

    /**
     * @brief Access object address
     */
    void* Base() const {
        K_ASSERT(Capacity() == 1);
        return At(0).base;
    }
    /**
     * @brief Access object size
     */
    u32 Size() const {
        return sizeof(T);
    }

    /**
     * @brief Access underlying object (pointer)
     */
    T* Ptr() const {
        K_ASSERT(Base() != NULL);
        return reinterpret_cast<T*>(Base());
    }
    /**
     * @brief Access underlying object (reference)
     */
    T& Ref() {
        return *Ptr();
    }
    /**
     * @brief Access underlying object (reference, read-only)
     */
    const T& Ref() const {
        return *Ptr();
    }

    /**
     * @brief Dereference pointer
     */
    T& operator*() {
        return Ref();
    }
    /**
     * @brief Dereference pointer (read-only)
     */
    const T& operator*() const {
        return Ref();
    }

    /**
     * @brief Pointer access
     */
    T* operator->() {
        return Ptr();
    }
    /**
     * @brief Pointer access (read-only)
     */
    const T* operator->() const {
        return Ptr();
    }

private:
    void Allocate() {
        void* base = new (32, EMemory_MEM2) T();
        K_ASSERT(base != NULL && OSIsMEM2Region(base));
        At(0).Set(base, Size());
    }
};

} // namespace kiwi

#endif