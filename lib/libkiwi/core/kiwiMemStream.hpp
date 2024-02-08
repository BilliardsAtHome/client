#ifndef LIBKIWI_CORE_MEM_STREAM_H
#define LIBKIWI_CORE_MEM_STREAM_H
#include <libkiwi/core/kiwiFileStream.hpp>
#include <types.h>

namespace kiwi {

/**
 * @brief Memory buffer stream
 */
class MemStream : public FileStream {
public:
    /**
     * @brief Constructor
     *
     * @param readOnly Read-only buffer
     * @param size Buffer size
     * @param owns Whether the stream owns the buffer
     */
    MemStream(const void* readOnly, u32 size, bool owns = false)
        : FileStream(EOpenMode_Read), mBufferSize(size), mOwnsBuffer(owns) {
        mBufferDataConst = static_cast<const u8*>(readOnly);
    }

    /**
     * @brief Constructor
     *
     * @param readWrite Mutable buffer
     * @param size Buffer size
     * @param owns Whether the stream owns the buffer
     */
    MemStream(void* readWrite, u32 size, bool owns = false)
        : FileStream(EOpenMode_RW), mBufferSize(size), mOwnsBuffer(owns) {
        mBufferData = static_cast<u8*>(readWrite);
    }

    /**
     * @brief Destructor
     */
    virtual ~MemStream() {
        if (mOwnsBuffer) {
            delete mBufferData;
        }
    }

    virtual void Close() {}

    virtual u32 GetSize() const {
        return mBufferSize;
    }

    virtual bool CanSeek() const {
        return true;
    }
    virtual bool CanRead() const {
        return true;
    }
    virtual bool CanWrite() const {
        return mOpenMode == EOpenMode_RW;
    }
    virtual bool CanPeek() const {
        return true;
    }

    virtual s32 GetAlign() const {
        return 4;
    }

private:
    virtual void SeekImpl(ESeekDir dir, s32 offset);
    virtual s32 ReadImpl(void* dst, u32 size);
    virtual s32 WriteImpl(const void* src, u32 size);
    virtual s32 PeekImpl(void* dst, u32 size);

private:
    union {
        u8* mBufferData;
        const u8* mBufferDataConst;
    };
    u32 mBufferSize;

    bool mOwnsBuffer;
};

} // namespace kiwi

#endif