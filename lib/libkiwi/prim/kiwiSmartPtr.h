#ifndef LIBKIWI_PRIM_SMART_PTR_H
#define LIBKIWI_PRIM_SMART_PTR_H
#include <algorithm>
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/util/kiwiNonCopyable.h>

namespace kiwi {

/**
 * @brief "Smart" / scoped pointer (equivalent to std::unique_ptr)
 */
template <typename T> class SmartPtr : private NonCopyable {
public:
    /**
     * @brief Constructor
     *
     * @details Empty pointer
     */
    SmartPtr() : mpData(NULL) {}

    /**
     * @brief Constructor
     * @details Value constructor
     *
     * @param pPtr Pointer
     */
    SmartPtr(T* pPtr) : mpData(pPtr) {}

    /**
     * @brief Constructor
     * @details Copy constructor
     *
     * @param rOther Smart pointer
     */
    SmartPtr(SmartPtr& rOther) : mpData(NULL) {
        *this = rOther;
    }

#ifdef LIBKIWI_CPP1X
    /**
     * @brief Constructor
     * @details Move constructor
     *
     * @param rOther Smart pointer
     */
    SmartPtr(SmartPtr&& rOther) : mpData(NULL) {
        *this = std::move(rOther);
    }
#endif

    /**
     * @brief Destructor
     */
    ~SmartPtr() {
        Destroy();
    }

    /**
     * @brief Pointer assignment
     *
     * @param pPtr Pointer
     */
    SmartPtr& operator=(T* pPtr) {
        Assign(pPtr);
        return *this;
    }

    /**
     * @brief Move assignment
     *
     * @param rOther Smart pointer
     */
    SmartPtr& operator=(SmartPtr& rOther) {
        Move(rOther);
        return *this;
    }

#ifdef LIBKIWI_CPP1X
    /**
     * @brief Move assignment
     *
     * @param rOther Smart pointer
     */
    SmartPtr& operator=(SmartPtr&& rOther) {
        Move(rOther);
        return *this;
    }
#endif

    /**
     * @brief Access held pointer
     */
    T* Get() {
        return mpData;
    }
    /**
     * @brief Access held pointer (read-only)
     */
    const T* Get() const {
        return mpData;
    }

    /**
     * @brief Destroy and release object
     */
    void Destroy() {
        delete mpData;
        mpData = NULL;
    }

    /**
     * @brief Release object without destroying it
     *
     * @return Pointer to object
     */
    T* Release() const {
        T* old = mpData;
        mpData = NULL;
        return old;
    }

    // clang-format off
    operator bool() const { return Get() != NULL; }

    T*       operator->()       { return Get(); }
    const T* operator->() const { return Get(); }

    T& operator*()             { K_ASSERT(mpData != NULL); return *Get(); }
    const T& operator*() const { K_ASSERT(mpData != NULL); return *Get(); }
    // clang-format on

private:
    /**
     * @brief Assign primitive pointer value
     *
     * @param pPtr Primitive pointer
     */
    void Assign(T* pPtr) {
        // Release memory for old object
        Destroy();

        mpData = pPtr;
    }

    /**
     * @brief Move value from other smart pointer
     *
     * @param rOther Smart pointer
     */
    void Move(SmartPtr& rOther) {
        // Release memory for old object
        Destroy();

        mpData = NULL;
        std::swap(mpData, rOther.mpData);
    }

#ifdef LIBKIWI_CPP1X
    /**
     * @brief Move value from other smart pointer
     *
     * @param rOther Smart pointer
     */
    void Move(SmartPtr&& rOther) {
        // Release memory for old object
        Destroy();

        mpData = NULL;
        std::swap(mpData, rOther.mpData);
    }
#endif

private:
    // Object owned by this smart pointer
    T* mpData;
};

namespace {

/**
 * @brief Smart pointer construction helper
 */
template <typename T> SmartPtr<T> MakeSmartPtr() {
    return SmartPtr<T>(new T());
}

} // namespace
} // namespace kiwi

#endif