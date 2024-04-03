#include <climits>
#include <libkiwi.h>

namespace kiwi {

/**
 * Connects to another socket
 *
 * @param addr Remote address
 * @param callback Connection callback
 * @param arg Callback user argument
 * @return Success
 */
bool SyncSocket::Connect(const SockAddr& addr, ConnectCallback callback,
                         void* arg) {
    K_ASSERT(IsOpen());

    // Blocking call
    SOResult result = LibSO::Connect(mHandle, addr);
    if (result != SO_SUCCESS) {
        return false;
    }

    if (callback != NULL) {
        callback(result, arg);
    }

    return true;
}

/**
 * Accepts remote connection
 *
 * @param callback Acceptance callback
 * @param arg Callback user argument
 * @return New socket
 */
SyncSocket* SyncSocket::Accept(AcceptCallback callback, void* arg) {
    K_ASSERT(IsOpen());

    // TODO: Will forcing ipv4 cause problems?
    kiwi::SockAddr4 peer;

    // Blocking call
    s32 result = LibSO::Accept(mHandle, peer);
    if (result < 0) {
        return NULL;
    }

    // Result code is the peer descriptor
    SyncSocket* socket = new SyncSocket(mHandle, mFamily, mType);
    K_ASSERT(socket != NULL);

    if (callback != NULL) {
        callback(socket, peer, arg);
    }

    return socket;
}

/**
 * Receives data and records sender address
 *
 * @param dst Destination buffer
 * @param len Buffer size
 * @param[out] nrecv Number of bytes received
 * @param[out] addr Sender address
 * @param callback Completion callback
 * @param arg Callback user argument
 * @return Socket library result
 */
SOResult SyncSocket::RecvImpl(void* dst, u32 len, u32& nrecv, SockAddr* addr,
                              ReceiveCallback callback, void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(dst != NULL);
    K_ASSERT(len > 0 && len < ULONG_MAX);

    // TODO: Will forcing ipv4 cause problems?
    kiwi::SockAddr4 peer;

    nrecv = 0;
    while (nrecv < len) {
        // Blocking call
        s32 result = LibSO::RecvFrom(mHandle, dst, len - nrecv, 0, peer);
        if (result < 0) {
            return static_cast<SOResult>(result);
        }

        nrecv += len;
    }

    K_ASSERT_EX(nrecv <= len, "Overflow???");

    if (addr != NULL) {
        std::memcpy(addr, &peer, peer.len);
    }

    if (callback != NULL) {
        callback(peer, dst, nrecv, arg);
    }

    return SO_SUCCESS;
}

/**
 * Sends data to specified connection
 *
 * @param dst Destination buffer
 * @param len Buffer size
 * @param[out] nsend Number of bytes sent
 * @param[out] addr Sender address
 * @param callback Completion callback
 * @param arg Callback user argument
 * @return Socket library result
 */
SOResult SyncSocket::SendImpl(const void* src, u32 len, u32& nsend,
                              const SockAddr* addr, SendCallback callback,
                              void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(src != NULL);
    K_ASSERT(len > 0 && len < ULONG_MAX);

    nsend = 0;
    while (nsend < len) {
        s32 result;

        // Blocking calls
        if (addr != NULL) {
            result = LibSO::SendTo(mHandle, src, len - nsend, 0, *addr);
        } else {
            result = LibSO::Send(mHandle, src, len - nsend, 0);
        }

        if (result < 0) {
            return static_cast<SOResult>(result);
        }

        nsend += result;
    }

    if (callback != NULL) {
        callback(arg);
    }

    return SO_SUCCESS;
}

} // namespace kiwi
