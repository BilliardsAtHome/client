#include <libkiwi.h>

namespace kiwi {

/**
 * Constructor
 *
 * @param family Socket protocol family
 * @param type Socket type
 */
SocketBase::SocketBase(SOProtoFamily family, SOSockType type)
    : mHandle(-1), mFamily(family), mType(type) {
    mHandle = LibSO::Socket(mFamily, mType);
    K_ASSERT_EX(mHandle >= 0, "Failed to create socket");
}

/**
 * Constructor
 *
 * @param socket Socket file descriptor
 * @param type Socket protocol family
 * @param type Socket type
 */
SocketBase::SocketBase(SOSocket socket, SOProtoFamily family, SOSockType type)
    : mHandle(socket), mFamily(family), mType(type) {
    K_ASSERT(mHandle >= 0);
}

/**
 * Destructor
 */
SocketBase::~SocketBase() {
    K_ASSERT(mHandle >= 0);

    bool success = Close();
    K_ASSERT(success);
}

/**
 * Gets the console's IP address
 *
 * @return Raw IPv4 address
 */
u32 SocketBase::GetHostAddr() {
    SOInAddr addr;

    LibSO::GetHostID(addr);
    return addr.raw;
}

/**
 * Binds socket to local address
 *
 * @note Bind to port zero for a random port (written out)
 * @param addr[in,out] Local address
 * @return Success
 */
bool SocketBase::Bind(SOSockAddr& addr) const {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(mFamily == addr.in.family);

    return LibSO::Bind(mHandle, addr) >= 0;
}

/**
 * Listens for incoming connections
 *
 * @param backlog Maximum pending connections
 * @return Success
 */
bool SocketBase::Listen(s32 backlog) const {
    K_ASSERT(mHandle >= 0);
    K_WARN(mType == SO_SOCK_DGRAM, "Listen won't do anything for dgram.");

    return LibSO::Listen(mHandle, backlog) >= 0;
}

/**
 * Toggles socket blocking
 *
 * @param enable Whether to enable blocking
 * @return Success
 */
bool SocketBase::SetBlocking(bool enable) const {
    K_ASSERT(mHandle >= 0);

    s32 flags = LibSO::Fcntl(mHandle, SO_F_GETFL, 0);

    if (enable) {
        flags &= ~SO_O_NONBLOCK;
    } else {
        flags |= SO_O_NONBLOCK;
    }

    return LibSO::Fcntl(mHandle, SO_F_SETFL, flags) >= 0;
}

/**
 * Stops socket from reading/writing
 *
 * @param how How to shutdown connection
 * @return Success
 */
bool SocketBase::Shutdown(SOShutdownType how) const {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(how == SO_SHUT_RD || how == SO_SHUT_WR || how == SO_SHUT_RDWR);

    return LibSO::Shutdown(mHandle, how) >= 0;
}

/**
 * Closes socket
 *
 * @return Success
 */
bool SocketBase::Close() {
    K_ASSERT(mHandle >= 0);

    // Attempt to close
    if (LibSO::Close(mHandle) < 0) {
        return false;
    }

    // Invalidate socket descriptor
    mHandle = -1;
    return true;
}

/**
 * Gets endpoint of socket
 *
 * @param[out] addr Socket address
 * @return Success
 */
bool SocketBase::GetSocketAddr(SOSockAddr& addr) const {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(mFamily == addr.in.family);

    return LibSO::GetSockName(mHandle, addr) >= 0;
}

/**
 * Gets endpoint of peer
 *
 * @param[out] addr Peer address
 * @return Success
 */
bool SocketBase::GetPeerAddr(SOSockAddr& addr) const {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(mFamily == addr.in.family);

    return LibSO::GetPeerName(mHandle, addr) >= 0;
}

/**
 * Tests if socket can receive data
 */
bool SocketBase::CanRecv() const {
    K_ASSERT(mHandle >= 0);

    SOPollFD fd[1];
    fd[0].fd = mHandle;
    fd[0].events = SO_POLLRDNORM;
    fd[0].revents = 0;

    bool success = LibSO::Poll(fd, LENGTHOF(fd), 0);
    return success && fd[0].events == fd[0].revents;
}

/**
 * Tests if socket can send data
 */
bool SocketBase::CanSend() const {
    K_ASSERT(mHandle >= 0);

    SOPollFD fd[1];
    fd[0].fd = mHandle;
    fd[0].events = SO_POLLWRNORM;
    fd[0].revents = 0;

    bool success = LibSO::Poll(fd, LENGTHOF(fd), 0);
    return success && fd[0].events == fd[0].revents;
}

/**
 * Receives bytes from bound connection
 *
 * @param buf Destination buffer
 * @param len Buffer size
 * @param[out] nrecv Number of bytes received
 * @return Success or would-be-blocking
 */
bool SocketBase::RecvBytes(void* buf, u32 len, u32& nrecv) {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(buf != NULL);
    K_ASSERT(len > 0);

    s32 result = RecvImpl(buf, len, NULL, nrecv);
    return result >= 0 || result == SO_EWOULDBLOCK;
}

/**
 * Receives bytes from specified connection
 *
 * @param buf Destination buffer
 * @param len Buffer size
 * @param[out] addr Sender address
 * @param[out] nrecv Number of bytes received
 * @return Success or would-be-blocking
 */
bool SocketBase::RecvBytesFrom(void* buf, u32 len, SOSockAddr& addr,
                               u32& nrecv) {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(buf != NULL);
    K_ASSERT(len > 0);

    s32 result = RecvImpl(buf, len, &addr, nrecv);
    return result >= 0 || result == SO_EWOULDBLOCK;
}

/**
 * Sends bytes to bound connection
 *
 * @param buf Source buffer
 * @param len Buffer size
 * @param[out] nsend Number of bytes sent
 * @return Success or would-be-blocking
 */
bool SocketBase::SendBytes(const void* buf, u32 len, u32& nsend) {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(buf != NULL);
    K_ASSERT(len > 0);

    s32 result = SendImpl(buf, len, NULL, nsend);
    return result >= 0 || result == SO_EWOULDBLOCK;
}

/**
 * Sends bytes to specified connection
 *
 * @param buf Source buffer
 * @param len Buffer size
 * @param addr Destination address
 * @param[out] nsend Number of bytes sent
 * @return Success or would-be-blocking
 */
bool SocketBase::SendBytesTo(const void* buf, u32 len, const SOSockAddr& addr,
                             u32& nsend) {
    K_ASSERT(mHandle >= 0);
    K_ASSERT(buf != NULL);
    K_ASSERT(len > 0);

    s32 result = SendImpl(buf, len, &addr, nsend);
    return result >= 0 || result == SO_EWOULDBLOCK;
}

} // namespace kiwi
