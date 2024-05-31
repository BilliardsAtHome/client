#ifndef LIBKIWI_UTIL_IOS_VECTOR_H
#define LIBKIWI_UTIL_IOS_VECTOR_H
#include <libkiwi/core/kiwiMemoryMgr.h>
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <revolution/IPC.h>

namespace kiwi {

/**
 * @brief Multiple, contiguous IOS vectors
 */
class IosVectors {
public:
    /**
     * @brief IOS I/O vector
     */
    class Data : public IPCIOVector {
    public:
        /**
         * @brief Constructor
         */
        Data() {
            Clear();
        }

        /**
         * @brief Set vector contents
         *
         * @param _base Memory base
         * @param _length Memory size
         */
        void Set(void* _base, u32 _length) {
            base = _base;
            length = _length;
        }

        /**
         * @brief Clear vector contents
         */
        void Clear() {
            Set(NULL, 0);
        }
    };

public:
    /**
     * @brief Constructor
     *
     * @param capacity Number of vectors
     */
    explicit IosVectors(u32 capacity) : mCapacity(capacity), mpVectors(NULL) {
        mpVectors = new (32, EMemory_MEM2) Data[mCapacity];
        K_ASSERT(mpVectors != NULL);
        K_ASSERT(OSIsMEM2Region(mpVectors));
    }

    /**
     * @brief Destructor
     */
    ~IosVectors() {
        delete[] mpVectors;
    }

    /**
     * @brief Access number of vectors
     */
    u32 Capacity() const {
        return mCapacity;
    }

    /**
     * @brief Access vector by index
     */
    Data& At(int i) {
        K_ASSERT(i >= 0 && i < mCapacity);
        K_ASSERT(mpVectors != NULL);
        return mpVectors[i];
    }
    /**
     * @brief Access vector by index (read-only)
     */
    const Data& At(int i) const {
        K_ASSERT(i >= 0 && i < mCapacity);
        K_ASSERT(mpVectors != NULL);
        return mpVectors[i];
    }

    /**
     * @brief Array access
     *
     * @param i Vector index
     */
    Data& operator[](int i) {
        return At(i);
    }
    /**
     * @brief Array access (read-only)
     *
     * @param i Vector index
     */
    const Data& operator[](int i) const {
        return At(i);
    }

    /**
     * @brief IPC vector type conversion operator
     */
    operator IPCIOVector*() const {
        return mpVectors;
    }

private:
    u32 mCapacity;   // Vector count
    Data* mpVectors; // Real vectors in MEM2
};

} // namespace kiwi

#endif