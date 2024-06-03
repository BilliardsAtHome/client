#ifndef LIBKIWI_NET_PACKET_H
#define LIBKIWI_NET_PACKET_H
#include <climits>
#include <libkiwi/k_types.h>
#include <libkiwi/math/kiwiAlgorithm.h>
#include <libkiwi/prim/kiwiOptional.h>
#include <libkiwi/support/kiwiLibSO.h>
#include <revolution/OS.h>

namespace kiwi {

/**
 * @brief Network packet wrapper
 */
class Packet {
public:
    /**
     * @brief Constructor
     *
     * @param size Packet buffer size
     * @param dest Packet recipient
     */
    Packet(u32 size, const SockAddrAny* dest = NULL)
        : mpBuffer(NULL), mBufferSize(0), mReadOffset(0), mWriteOffset(0) {
        OSInitMutex(&mBufferMutex);
        Alloc(size);

        if (dest != NULL) {
            mAddress = *dest;
        } else {
            mAddress = SockAddr4();
        }
    }

    /**
     * @brief Destructor
     */
    ~Packet() {
        Free();
    }

    /**
     * @brief Message buffer size
     */
    virtual u32 GetBufferSize() const {
        return mBufferSize;
    }
    /**
     * @brief Largest allowable message buffer
     */
    virtual u32 GetMaxBuffer() const {
        return ULONG_MAX;
    }

    /**
     * @brief Message content size
     */
    virtual u32 GetContentSize() const {
        return mBufferSize;
    }
    /**
     * @brief Largest allowable message content
     */
    virtual u32 GetMaxContent() const {
        return ULONG_MAX;
    }
    /**
     * @brief Access message content
     */
    const void* GetContent() const {
        return mpBuffer + GetOverhead();
    }

    /**
     * @brief Message buffer overhead
     */
    virtual u32 GetOverhead() const {
        return 0;
    }

    /**
     * @brief Whether the packet contains no data
     */
    bool IsEmpty() const {
        return mpBuffer == NULL || GetContentSize() == 0;
    }

    /**
     * @brief The number of bytes that must be read to complete the packet
     */
    u32 ReadRemain() const {
        return Max<s32>(GetContentSize() - mReadOffset, 0);
    }
    /**
     * @brief The number of bytes that must be written to complete the packet
     */
    u32 WriteRemain() const {
        return Max<s32>(GetContentSize() - mWriteOffset, 0);
    }

    /**
     * @brief Whether read operation has been completed
     */
    bool IsReadComplete() const {
        return ReadRemain() == 0;
    }
    /**
     * @brief Whether write operation has been completed
     */
    bool IsWriteComplete() const {
        return WriteRemain() == 0;
    }

    /**
     * @brief Get peer socket address
     */
    const SockAddrAny& GetPeer() const {
        return mAddress;
    }

    void Alloc(u32 size);

    u32 Read(void* dst, u32 n);
    u32 Write(const void* src, u32 n);

    Optional<u32> Send(SOSocket socket);
    Optional<u32> Recv(SOSocket socket);

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

    SockAddrAny mAddress; // Sender (recv) or recipient (send)
};

} // namespace kiwi

#endif
