// Implementation header
#ifndef LIBKIWI_PRIM_VECTOR_IMPL_HPP
#define LIBKIWI_PRIM_VECTOR_IMPL_HPP

// Declaration header
#ifndef LIBKIWI_PRIM_VECTOR_H
#include <libkiwi/prim/kiwiVector.h>
#endif

#include <cstring>

namespace kiwi {

template <typename T> void DumpVector(const TVector<T>& v) {
    K_LOG("{");

    for (int i = 0; i < v.Size(); i++) {
        K_LOG_EX("%s, ", v[i].CStr());
    }

    K_LOG("}\n");
}

// Allocate 20% extra space
static const f32 scAllocBonus = 0.20f;

/**
 * @brief Clear vector contents
 */
template <typename T> void TVector<T>::Clear() {
    K_ASSERT(mSize == 0 || mpData != NULL);

    for (u32 i = 0; i < mSize; i++) {
        mpData[i].~T();
    }

    mSize = 0;
}

/**
 * @brief Insert a new element at the specified position
 *
 * @param t New element
 * @param pos Element position
 */
template <typename T> void TVector<T>::Insert(const T& t, u32 pos) {
    K_ASSERT(pos <= mSize);

    K_LOG_EX("Before insert %s -> %d: \n", t.CStr(), pos);
    DumpVector(*this);

    // Need to reallocate
    if (mSize >= mCapacity) {
        // Allocate 20% extra space
        u32 newSize = mSize + 1;
        newSize += static_cast<u32>(scAllocBonus * newSize);
        Reserve(newSize);
    }

    K_ASSERT(mpData != NULL);

    // Inserted in the middle, copy forward
    if (pos < mSize) {
        std::memcpy(mpData + pos + 1, //
                    mpData + pos,     //
                    (mSize - pos) * sizeof(T));
    }

    K_LOG_EX("Set mpData[%d]=%s, previously %s\n", pos, t.CStr(),
             mpData[pos].CStr());

    mpData[pos] = t;
    mSize++;

    K_LOG_EX("After insert %s -> %d: \n", t.CStr(), pos);
    DumpVector(*this);
}

/**
 * @brief Remove an element if it exists in the vector
 *
 * @param t Element to remove
 * @return Success
 */
template <typename T> bool TVector<T>::Remove(const T& t) {
    K_ASSERT(mpData != NULL);

    // Linear search for the target
    for (u32 i = 0; i < mSize; i++) {
        if (mpData[i] == t) {
            RemoveAt(i);
            return true;
        }
    }

    // Element not in the vector
    return false;
}

/**
 * @brief Remove an element at the specified position
 *
 * @param pos Element position
 */
template <typename T> void TVector<T>::RemoveAt(u32 pos) {
    K_ASSERT(pos < mSize);
    K_ASSERT(mpData != NULL);

    // Destroy element
    mpData[pos].~T();

    // Removed from the middle, copy backward
    if (pos < mSize) {
        std::memcpy(mpData + pos,     //
                    mpData + pos + 1, //
                    (mSize - pos) * sizeof(T));
    }

    mSize--;
}

/**
 * @brief Insert a new element at the back of the vector
 *
 * @param t New element
 */
template <typename T> void TVector<T>::PushBack(const T& t) {
    Insert(t, mSize);
}

/**
 * @brief Remove the last element from the vector
 */
template <typename T> void TVector<T>::PopBack() {
    K_ASSERT(mSize > 0);
    Remove(mSize - 1);
}

/**
 * @brief Reserve space for elements in the vector
 *
 * @param capacity New capacity
 */
template <typename T> void TVector<T>::Reserve(u32 capacity) {
    K_ASSERT_EX(false, "Don't use this class. Still broken");

    // All good!
    if (mCapacity >= capacity) {
        return;
    }

    K_LOG_EX("Before reallocate for %d: \n", capacity);
    DumpVector(*this);

    // Need to reallocate
    T* buffer = new T[capacity];
    K_ASSERT(buffer != NULL);

    // Copy in old data
    if (mpData != NULL) {
        std::memcpy(buffer, mpData, mSize * sizeof(T));

        // TODO: WHY?????
        // delete[] mpData;
        K_LOG_EX("    LEAKING %d BYTES YEAAAAAAA\n", mCapacity * sizeof(T));
    }

    // Swap buffer
    mpData = buffer;
    mCapacity = capacity;

    K_LOG_EX("After reallocate for %d: \n", capacity);
    DumpVector(*this);
}

/**
 * @brief Copy vector contents
 *
 * @param other Vector to copy
 */
template <typename T> void TVector<T>::CopyFrom(const TVector& other) {
    // Destroy existing contents
    Clear();

    // Make sure we can fit the contents
    Reserve(other.mSize);
    std::memcpy(mpData, other.mpData, other.mSize);
}

} // namespace kiwi

#endif