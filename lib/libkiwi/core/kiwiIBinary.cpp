#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Deserializes binary contents after ensuring they belong to this
 * class
 *
 * @param bin Binary contents
 */
void IBinary::Deserialize(const void* bin) {
    K_ASSERT(bin != NULL);
    const Header& header = *static_cast<const Header*>(bin);

    // Check header kind
    K_ASSERT_EX(GetBinaryKind() == header.block.kind,
                "Not for this class. Got %08X (%s)", header.block.kind,
                header.block.Kind());

    // Check header version
    K_ASSERT_EX(GetVersion() == header.version,
                "Not for this version. Expected %04X, got %04X", GetVersion(),
                header.version);

    DeserializeImpl(header);
}

/**
 * @brief Serializes binary contents
 *
 * @param bin Binary contents
 */
void IBinary::Serialize(void* bin) const {
    K_ASSERT(bin != NULL);
    Header& header = *static_cast<Header*>(bin);

    header.version = GetVersion();
    header.block.kind = GetBinaryKind();

    SerializeImpl(header);
}

} // namespace kiwi