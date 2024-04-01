#ifndef LIBKIWI_NET_PACKET_H
#define LIBKIWI_NET_PACKET_H
#include <climits>
#include <libkiwi/math/kiwiAlgorithm.hpp>
#include <libkiwi/rvl/kiwiLibSO.hpp>
#include <revolution/OS.h>
#include <types.h>

namespace kiwi {

/**
 * Network packet wrapper
 */
class Packet {
public:
    /**
     * Constructor
     *
     * @param size Packet buffer size
     * @param dest Packet recipient
     */
    explicit Packet(u32 size, const SOSockAddr* dest = NULL)
        : mpBuffer(NULL), mBufferSize(0), mReadOffset(0), mWriteOffset(0) {
        OSInitMutex(&mBufferMutex);

        if (dest != NULL) {
            std::memcpy(&mAddress, &dest, dest->len);
            mHasAddress = true;
        } else {
            std::memset(&mAddress, 0, sizeof(SOSockAddr));
            mHasAddress = false;
        }
    }

    /**
     * Destructor
     */
    ~Packet() {
        Free();
    }

    /**
     * Message buffer size
     */
    virtual u32 GetBufferSize() const {
        return mBufferSize;
    }
    /**
     * Largest allowable message buffer
     */
    virtual u32 GetMaxBuffer() const {
        return ULONG_MAX;
    }

    /**
     * Message content size
     */
    virtual u32 GetContentSize() const {
        return mBufferSize;
    }
    /**
     * Largest allowable message content
     */
    virtual u32 GetMaxContent() const {
        return ULONG_MAX;
    }
    /**
     * Access message content
     */
    const void* GetContent() const {
        return mpBuffer + GetOverhead();
    }

    /**
     * Message buffer overhead
     */
    virtual u32 GetOverhead() const {
        return 0;
    }

    void Alloc(u32 size);

    /**
     * Whether the packet contains no data
     */
    bool IsEmpty() const {
        return mpBuffer == NULL || GetContentSize() == 0;
    }

    /**
     * The number of bytes that must be read to complete the packet
     */
    u32 ReadRemain() const {
        return Max<s32>(GetContentSize() - mReadOffset, 0);
    }
    /**
     * The number of bytes that must be written to complete the packet
     */
    u32 WriteRemain() const {
        return Max<s32>(GetContentSize() - mWriteOffset, 0);
    }

    /**
     * Whether read operation has been completed
     */
    bool IsReadComplete() const {
        return ReadRemain() == 0;
    }
    /**
     * Whether write operation has been completed
     */
    bool IsWriteComplete() const {
        return WriteRemain() == 0;
    }

    /**
     * Whether this packet has a designated peer
     */
    bool HasPeer() const {
        return mHasAddress;
    }
    /**
     * Get peer socket address
     *
     * @param addr Peer address
     */
    void GetPeer(SOSockAddr& addr) const {
        if (HasPeer()) {
            std::memcpy(&addr, &mAddress, mAddress.len);
        }
    }

    u32 Read(void* dst, u32 n);
    u32 Write(const void* src, u32 n);

    s32 Send(SOSocket socket);
    s32 Recv(SOSocket socket);

protected:
    void Free();
    void Clear();

protected:
    // Message buffer
    u8* mpBuffer;
    u32 mBufferSize;
    OSMutex mBufferMutex;

    // Read/write pointer
    s32 mReadOffset;
    s32 mWriteOffset;

    // Sender (recv) or recipient (send)
    SOSockAddr mAddress;
    bool mHasAddress;
};

} // namespace kiwi

#endif
