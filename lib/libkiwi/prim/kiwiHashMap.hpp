#ifndef LIBKIWI_PRIM_HASHMAP_H
#define LIBKIWI_PRIM_HASHMAP_H
#include <types.h>

namespace kiwi {

// Hash value type
typedef u32 hash_t;
// Largest representable hash
static const hash_t HASH_MAX = (1 << (sizeof(hash_t) * 8)) - 1;

// Raw data hash function (call this from Hasher)
hash_t HashImpl(const void* key, s32 len);

/**
 * @brief Key hasher
 * @note Specialize the call operator for your custom types
 */
template <typename TKey> struct Hasher {
    // Default hash function
    hash_t operator()(const TKey& key) const {
        return HashImpl(&key, sizeof(TKey));
    }
};

/**
 * @brief Key/value map
 *
 * TODO: Track load factor, expand + re-hash when high
 */
template <typename TKey, typename TValue> class TMap {
public:
    // Default bucket count
    static const u32 DEFAULT_CAPACITY = 32;

public:
    /**
     * @brief Constructor
     *
     * @param capacity Starting number of buckets
     */
    TMap(u32 capacity = DEFAULT_CAPACITY)
        : mCapacity(capacity), mpBuckets(NULL) {
        K_ASSERT(mCapacity > 0);
        K_ASSERT(mCapacity < HASH_MAX);

        mpBuckets = new Bucket[mCapacity];
        K_ASSERT(mpBuckets != NULL);
    }

    /**
     * @brief Destructor
     */
    ~TMap() {
        delete[] mpBuckets;
    }

    /**
     * @brief Access a value by key
     * @note Inserts key if it does not already exist
     *
     * @param key Key
     * @return Existing value, or new entry
     */
    TValue& operator[](const TKey& key) {
        return FindImpl(key, true)->value;
    }

    /**
     * @brief Insert a new key or update an existing value
     *
     * @param key Key
     * @param value Value
     */
    void Insert(const TKey& key, const TValue& value) {
        FindImpl(key, true)->value = value;
    }

    /**
     * @brief Remove a key
     *
     * @param key Key
     * @param[out] removed Removed value
     * @return Success
     */
    bool Remove(const TKey& key, TValue* removed = NULL) {
        Bucket* bucket = Find(key);

        // Can't remove, doesn't exist
        if (bucket == NULL) {
            return false;
        }

        // Write out value about to be removed
        if (removed != NULL) {
            *removed = bucket->value;
        }

        // Just mark as unused
        bucket->used = false;
        return true;
    }

    /**
     * @brief Look for the value corresponding to a key
     *
     * @param key Key
     * @return Value if it exists
     */
    TValue* Find(const TKey& key) const {
        Bucket* bucket = FindImpl(key, false);
        if (bucket == NULL) {
            return NULL;
        }

        return &bucket->value;
    }

    /**
     * @brief Check whether a key exists
     *
     * @param key
     * @return true
     * @return false
     */
    bool Contains(const TKey& key) const {
        return Find(key) != NULL;
    }

private:
    struct Bucket {
        /**
         * @brief Destructor
         */
        ~Bucket() {
            // Recurse
            delete chained;
        }

        // Key/value pair
        TKey key;
        TValue value;

        // Bucket is in use
        bool used;

        // Chains
        Bucket* chained;
    };

private:
    /**
     * @brief Find key in hashmap
     *
     * @param key Key
     * @param insert Insert key if it doesn't exist
     */
    Bucket* FindImpl(const TKey& key, bool insert) {
        // Calculate bucket index
        u32 i = Hasher<TKey>()(key) % mCapacity;

        // Iterate through chains
        Bucket* last = NULL;
        for (Bucket* it = &mpBuckets[i]; it != NULL; it = it->chained) {
            // Unused entry
            if (!it->used) {
                continue;
            }

            // Matches key
            if (it->key == key) {
                return it;
            }

            last = it;
        }

        K_ASSERT(last != NULL && last->used);

        // Chain new bucket
        if (insert) {
            last->chained = new Bucket();
            K_ASSERT(last->chained != NULL);

            return last->chained;
        }

        return NULL;
    }

private:
    u32 mCapacity;
    Bucket* mpBuckets;
};

} // namespace kiwi

#endif