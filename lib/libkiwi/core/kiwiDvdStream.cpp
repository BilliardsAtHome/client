#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Opens stream to DVD file
 *
 * @param path File path
 * @return Success
 */
bool DvdStream::Open(const String& path) {
    // Close existing file
    if (IsOpen()) {
        Close();
    }

    // Try to locate file on disc
    s32 entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum < 0) {
        K_LOG_EX("Can't find DVD file: %s\n", path.CStr());
        return false;
    }

    // Get handle to file
    if (!DVDFastOpen(entrynum, &mFileInfo)) {
        K_LOG_EX("Can't open DVD file: %s\n", path.CStr());
        return false;
    }

    mIsOpen = true;
    return true;
}

/**
 * @brief Closes this stream
 */
void DvdStream::Close() {
    if (!IsOpen()) {
        return;
    }

    DVDClose(&mFileInfo);
    mIsOpen = false;
}

/**
 * @brief Gets the size of the currently open file
 */
u32 DvdStream::GetSize() const {
    return mFileInfo.size;
}

/**
 * @brief Advances this stream's position (internal implementation)
 *
 * @param dir Seek direction
 * @param offset Seek offset
 */
void DvdStream::SeekImpl(ESeekDir dir, s32 offset) {
    switch (dir) {
    case ESeekDir_Begin:   mPosition = offset; break;
    case ESeekDir_Current: mPosition += offset; break;
    case ESeekDir_End:
        K_ASSERT_EX(offset < 0, "Can't seek forward from end of file");
        K_ASSERT_EX(offset > -GetSize(), "Too far backwards");
        mPosition = GetSize() + offset;
        break;
    }

    K_ASSERT_EX(mPosition < GetSize(), "Can't seek past end of file");
}

/**
 * @brief Reads data from this stream (internal implementation)
 *
 * @param dst Destination buffer
 * @param size Number of bytes to read
 * @return Number of bytes read, or DVD error code
 */
s32 DvdStream::ReadImpl(void* dst, u32 size) {
    K_ASSERT(dst != NULL);
    return DVDReadPrio(&mFileInfo, dst, size, mPosition, DVD_PRIO_MEDIUM);
}

/**
 * @brief Writes data to this stream (internal implementation)
 *
 * @param src Source buffer
 * @param size Number of bytes to write
 * @return Number of bytes written, or DVD error code
 */
s32 DvdStream::WriteImpl(const void* src, u32 size) {
    K_ASSERT(src != NULL);
    K_ASSERT_EX(false, "Can't write to the DVD");
    return DVD_RESULT_FATAL;
}

/**
 * @brief Reads data from this stream without advancing the stream's
 * position (internal implementation)
 *
 * @param dst Destination buffer
 * @param size Number of bytes to read
 * @return Number of bytes read, or DVD error code
 */
s32 DvdStream::PeekImpl(void* dst, u32 size) {
    K_ASSERT(dst != NULL);
    return ReadImpl(dst, size);
}

} // namespace kiwi
