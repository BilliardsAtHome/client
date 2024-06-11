// Implementation header
#ifndef LIBKIWI_PRIM_HASHMAP_IMPL_HPP
#define LIBKIWI_PRIM_HASHMAP_IMPL_HPP

// Declaration header
#ifndef LIBKIWI_PRIM_HASHMAP_H
#include <libkiwi/prim/kiwiHashMap.h>
#endif

namespace kiwi {

/**
 * @brief Pre-increment operator
 */
template <typename TKey, typename TValue>
TMap<TKey, TValue>::ConstIterator&
TMap<TKey, TValue>::ConstIterator::operator++() {
    // Can't iterate
    if (mpIter == NULL) {
        return *this;
    }

    // Increment
    mpIter = mpIter->chained;

    // Find next non-empty chain
    while (true) {
        // End of chain, advance to next bucket
        if (mpIter == NULL) {
            if (++mIndex >= mCapacity) {
                break;
            }

            mpIter = &mpBuckets[mIndex];
        }

        // Did we find an item?
        K_ASSERT(mpIter != NULL);
        if (mpIter->used) {
            break;
        }

        // Keep searching
        mpIter = mpIter->chained;
    }

    return *this;
}

/**
 * @brief Constructor
 * @details Copy constructor
 *
 * @param rOther Map to copy
 */
template <typename TKey, typename TValue>
TMap<TKey, TValue>::TMap(const TMap& rOther) : mCapacity(rOther.mCapacity) {
    K_ASSERT(mCapacity > 0);
    K_ASSERT(mCapacity < HASH_MAX);

    // Create buckets for copy
    mpBuckets = new Bucket[mCapacity];
    K_ASSERT(mpBuckets != NULL);

    // Re-insert all members
    for (ConstIterator it = rOther.Begin(); it != rOther.End(); ++it) {
        Insert(it.Key(), it.Value());
    }
}

/**
 * @brief Remove a key
 *
 * @param rKey Key
 * @param[out] pRemoved Removed value
 * @return Success
 */
template <typename TKey, typename TValue>
bool TMap<TKey, TValue>::Remove(const TKey& rKey, TValue* pRemoved) {
    Bucket* pBucket = Search(rKey);

    // Can't remove, doesn't exist
    if (pBucket == NULL) {
        return false;
    }

    // Write out value about to be removed
    if (pRemoved != NULL) {
        *pRemoved = *pBucket->value;
    }

    // Just mark as unused
    pBucket->used = false;
    mSize--;
    return true;
}

/**
 * @brief Find key in hashmap
 *
 * @param rKey Key
 */
template <typename TKey, typename TValue>
TMap<TKey, TValue>::Bucket* TMap<TKey, TValue>::Search(const TKey& rKey) const {
    // Calculate bucket index
    u32 i = Hash(rKey) % mCapacity;

    // Iterate through chains
    for (Bucket* pIt = &mpBuckets[i]; pIt != NULL; pIt = pIt->chained) {
        // Unused entry
        if (!pIt->used) {
            continue;
        }

        // Matches key
        if (*pIt->key == rKey) {
            return pIt;
        }
    }

    return NULL;
}

/**
 * @brief Create key in hashmap
 *
 * @param rKey Key
 */
template <typename TKey, typename TValue>
TMap<TKey, TValue>::Bucket& TMap<TKey, TValue>::Create(const TKey& rKey) {
    // Calculate bucket index
    u32 i = Hash(rKey) % mCapacity;

    // Iterate through chains
    Bucket* pLast = NULL;
    for (Bucket* it = &mpBuckets[i]; it != NULL; it = it->chained) {
        // Unused entry
        if (!it->used) {
            // Override this entry
            it->key = rKey;
            it->value.Emplace();
            it->used = true;
            mSize++;
            return *it;
        }

        // Matches key
        if (*it->key == rKey) {
            return *it;
        }

        pLast = it;
    }

    // Chain new bucket
    K_ASSERT(pLast != NULL);
    pLast->chained = new Bucket();
    K_ASSERT(pLast->chained != NULL);

    pLast->chained->key = rKey;
    pLast->chained->value.Emplace();
    pLast->chained->used = true;
    mSize++;
    return *pLast->chained;
}

} // namespace kiwi

#endif
