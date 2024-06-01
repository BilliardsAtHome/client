#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Attempt to open this device
 *
 * @param path Virtual file path
 * @param timeOut Time-out period, in milliseconds
 * @return Success
 */
bool IosDevice::Open(const String& path, u32 timeOut) {
    // Try until we succeed or time-out
    s32 start = OSGetTick();
    do {
        mHandle = IOS_Open(path, IPC_OPEN_NONE);
        if (mHandle >= 0) {
            return true;
        }
    } while (OSGetTick() - start < OS_MSEC_TO_TICKS(timeOut));

    return false;
}

/**
 * @brief Attempt to close the device
 *
 * @return Success
 */
bool IosDevice::Close() {
    // Nothing to do, no device is open
    if (mHandle < 0) {
        return true;
    }

    s32 result = IOS_Close(mHandle);

    // May not be allowed to close(?)
    if (result == IPC_RESULT_OK) {
        mHandle = -1;
        return true;
    }

    return false;
}

/**
 * @brief Perform I/O control (single vectors) on this device
 *
 * @param id Ioctl ID
 * @param in Input data
 * @param out Output data
 * @return IOS result code
 */
s32 IosDevice::Ioctl(s32 id, const IosVectors* in, IosVectors* out) const {
    K_ASSERT_EX(mHandle > 0, "Please open this device");

    K_ASSERT_EX(in == NULL || in->Capacity() == 1,
                "Ioctl only supports one argument per vector");
    K_ASSERT_EX(out == NULL || out->Capacity() == 1,
                "Ioctl only supports one argument per vector");

    return IOS_Ioctl(mHandle, id,
                     in != NULL ? in->At(0).base : NULL,   //
                     in != NULL ? in->At(0).length : 0,    //
                     out != NULL ? out->At(0).base : NULL, //
                     out != NULL ? out->At(0).length : 0);
}

/**
 * @brief Perform I/O control (multiple vectors) on this device
 *
 * @param id Ioctl ID
 * @param in Input vectors
 * @param out Output vectors
 * @return IOS result code
 */
s32 IosDevice::IoctlV(s32 id, const IosVectors* in, IosVectors* out) const {
    K_ASSERT_EX(mHandle > 0, "Please open this device");

    // Mark which sets of vectors are present
    enum { IN = (1 << 0), OUT = (1 << 1) };
    u32 type = (in != NULL ? IN : 0) | (out != NULL ? OUT : 0);

    switch (type) {
    // No parameters
    case 0: return IOS_Ioctlv(mHandle, id, 0, 0, NULL);
    // Only input vectors
    case IN: return IOS_Ioctlv(mHandle, id, in->Capacity(), 0, *in);
    // Only output vectors
    case OUT: return IOS_Ioctlv(mHandle, id, 0, out->Capacity(), *out);

    // Both input & output vectors
    case (IN | OUT): {
        // Vectors need to be contiguous for IOS
        IosVectors all(in->Capacity() + out->Capacity());
        int i = 0;

        // Copy into contiguous buffer
        for (int j = 0; j < in->Capacity();) {
            all[i++] = (*in)[j++];
        }
        for (int j = 0; j < out->Capacity();) {
            all[i++] = (*out)[j++];
        }

        return IOS_Ioctlv(mHandle, id, in->Capacity(), out->Capacity(), all);
    }

    default: K_ASSERT(false); return IPC_RESULT_INVALID_INTERNAL;
    }
}

} // namespace kiwi