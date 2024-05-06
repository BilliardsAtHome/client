#ifndef LIBKIWI_PRIM_VECTOR_H
#define LIBKIWI_PRIM_VECTOR_H
#include <libkiwi/kernel/kiwiAssert.h>
#include <types.h>

namespace kiwi {

/**
 * @brief Dynamically-sized, contiguous array (std::vector)
 */
template <typename T> class TVector {
public:
    /**
     * @brief Constructor
     */
    TVector() : mpData(NULL), mCapacity(0), mSize(0) {}

    /**
     * @brief Constructor
     *
     * @param capacity Buffer capacity
     */
    explicit TVector(u32 capacity) : mpData(NULL), mCapacity(0), mSize(0) {
        Reserve(capacity);
    }

    /**
     * @brief Constructor
     *
     * @param other Vector to copy
     */
    TVector(const TVector& other) : mpData(NULL), mCapacity(0), mSize(0) {
        CopyFrom(other);
    }

    /**
     * @brief Destructor
     */
    ~TVector() {
        delete[] mpData;
        mpData = NULL;
    }

    /**
     * @brief Gets the number of elements in the vector
     */
    u32 Size() const {
        return mSize;
    }

    /**
     * @brief Tests whether the vector is empty
     */
    bool Empty() const {
        return mSize == 0;
    }

    /**
     * @brief Access element
     *
     * @param i Element index
     * @return Reference to element
     */
    T& operator[](u32 i) {
        K_ASSERT(i < mSize);
        K_ASSERT(mpData != NULL);
        return mpData[i];
    }

    /**
     * @brief Access element (read-only)
     *
     * @param i Element index
     * @return Reference to element
     */
    const T& operator[](u32 i) const {
        K_ASSERT(i < mSize);
        K_ASSERT(mpData != NULL);
        return mpData[i];
    }

    void Clear();

    void Insert(const T& t, u32 pos);
    bool Remove(const T& t);
    void RemoveAt(u32 pos);

    void PushBack(const T& t);
    void PopBack();

private:
    void Reserve(u32 capacity);
    void CopyFrom(const TVector& other);

private:
    // Allocated buffer
    T* mpData;
    // Buffer size
    u32 mCapacity;
    // Number of elements
    u32 mSize;
};

} // namespace kiwi

// Implementation header
#ifndef LIBKIWI_PRIM_VECTOR_IMPL_HPP
#include <libkiwi/prim/kiwiVectorImpl.hpp>
#endif

#endif