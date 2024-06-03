#ifndef LIBKIWI_NET_RELIABLE_PACKET_H
#define LIBKIWI_NET_RELIABLE_PACKET_H
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_types.h>
#include <libkiwi/net/kiwiPacket.h>

namespace kiwi {

/**
 * @brief Reliable packet header
 *
 * @note Concepts adapted from PRUDP
 */
struct KUDPHeader {
    /**
     * @brief Constructor
     */
    KUDPHeader()
        : magic(KUDP_MAGIC), size(0), sequence(0), fragment(0), flags(0) {}

    /* 0x00 */ u32 magic;    // Identification
    /* 0x04 */ u16 size;     // Message size
    /* 0x06 */ u16 sequence; // Sequence ID
    /* 0x08 */ u16 fragment; // Fragment ID
    /* 0x0A */ u16 flags;    // Status/info

    // Identifier string
    static const u32 KUDP_MAGIC = 'KUv0';

    /**
     * @brief Packet flags
     */
    enum EFlags {
        // This packet is only a fragment of the message, and there are more
        EFlags_MoreFragments = (1 << 0),
    };
};

/**
 * @brief Reliable UDP protocol (KUDP) packet
 */
class ReliablePacket : public Packet {
public:
    /**
     * @brief Size constraints to avoid going over MTU
     */
    static const u16 MAX_BUFFER_SIZE = 1000;
    static const u16 MAX_CONTENT_SIZE = MAX_BUFFER_SIZE - sizeof(KUDPHeader);

public:
    /**
     * @brief Constructor
     *
     * @param size Packet buffer size
     * @param dest Packet recipient
     */
    ReliablePacket(u32 size, const SockAddrAny* dest = NULL)
        : Packet(size, dest) {}

    /**
     * @brief Access KUDP protocol header
     */
    KUDPHeader& GetHeader() {
        K_ASSERT(mpBuffer != NULL);
        return *reinterpret_cast<KUDPHeader*>(mpBuffer);
    }
    const KUDPHeader& GetHeader() const {
        K_ASSERT(mpBuffer != NULL);
        return *reinterpret_cast<KUDPHeader*>(mpBuffer);
    }

    /**
     * @brief Largest allowable message buffer
     */
    virtual u32 GetMaxBuffer() const {
        return MAX_BUFFER_SIZE;
    }

    /**
     * @brief Message content size
     */
    virtual u32 GetContentSize() const {
        return GetHeader().size;
    }
    /**
     * @brief Largest allowable message content
     */
    virtual u32 GetMaxContent() const {
        return MAX_CONTENT_SIZE;
    }

    /**
     * @brief Message buffer overhead
     */
    virtual u32 GetOverhead() const {
        return sizeof(KUDPHeader);
    }
};

} // namespace kiwi

#endif
