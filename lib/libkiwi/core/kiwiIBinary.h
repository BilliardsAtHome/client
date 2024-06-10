#ifndef LIBKIWI_CORE_I_BINARY_H
#define LIBKIWI_CORE_I_BINARY_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiString.h>

/**
 * @brief Helper for creating version numbers
 */
#define K_VERSION(major, minor) ((major << 8) | minor)

namespace kiwi {

/**
 * @brief Interface for binary file serialization/deserialization
 */
class IBinary {
public:
    /**
     * @brief Generic binary file block/section
     */
    struct Block {
        /**
         * @brief Interprets kind as a string
         */
        String Kind() const {
            return String(reinterpret_cast<char*>(kind), sizeof(u32));
        }

        /* 0x00 */ u32 kind;
        /* 0x04 */ u32 size;
    };

    /**
     * @brief Generic header block for libkiwi file formats
     */
    struct Header {
        /* 0x00 */ Block block;
        /* 0x08 */ u16 version;
        /* 0x0A */ u16 numBlocks;
    };

public:
    /**
     * @brief Destructor
     */
    virtual ~IBinary() {}

    /**
     * @brief Gets the kind/magic of this object
     */
    virtual u32 GetBinaryKind() const = 0;
    /**
     * @brief Gets the serialized size of this object
     */
    virtual u32 GetBinarySize() const = 0;
    /**
     * @brief Gets the expected version of this object
     */
    virtual u16 GetVersion() const = 0;

    /**
     * @brief Deserializes binary contents after ensuring they belong to this
     * class
     *
     * @param pBin Binary contents
     */
    void Deserialize(const void* pBin);
    /**
     * @brief Serializes binary contents
     *
     * @param pBin Binary contents
     */
    void Serialize(void* pBin) const;

private:
    /**
     * @brief Deserializes binary contents (internal implementation)
     *
     * @param rHeader Binary file header
     */
    virtual void DeserializeImpl(const Header& rHeader) = 0;
    /**
     * @brief Serializes binary contents (internal implementation)
     *
     * @param rHeader Binary file header
     */
    virtual void SerializeImpl(Header& rHeader) const = 0;
};

} // namespace kiwi

#endif
