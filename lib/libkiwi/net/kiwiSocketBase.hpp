#ifndef LIBKIWI_NET_SOCKET_BASE_H
#define LIBKIWI_NET_SOCKET_BASE_H
#include <libkiwi/rvl/kiwiLibSO.hpp>
#include <libkiwi/util/kiwiNonCopyable.hpp>
#include <types.h>

namespace kiwi {

/**
 * IOS Berkeley socket wrapper
 */
class SocketBase : private NonCopyable {
public:
    static u32 GetHostAddr();

    SocketBase(SOProtoFamily family, SOSockType type);
    virtual ~SocketBase();
    virtual bool Connect(const SOSockAddr& addr) = 0;
    virtual SocketBase* Accept() = 0;

    /**
     * Tests whether the socket has a valid descriptor
     */
    bool IsOpen() const {
        return mHandle >= 0;
    }

    bool Bind(SOSockAddr& addr) const;
    bool Listen(s32 backlog = SOMAXCONN) const;
    bool SetBlocking(bool enable) const;
    bool Shutdown(SOShutdownType how) const;
    bool Close();

    bool GetSocketAddr(SOSockAddr& addr) const;
    bool GetPeerAddr(SOSockAddr& addr) const;

    bool CanRecv() const;
    bool CanSend() const;

    // Receive bytes
    bool RecvBytes(void* buf, u32 len, u32& nrecv);
    bool RecvBytesFrom(void* buf, u32 len, SOSockAddr& addr, u32& nrecv);

    // Send bytes
    bool SendBytes(const void* buf, u32 len, u32& nsend);
    bool SendBytesTo(const void* buf, u32 len, const SOSockAddr& addr,
                     u32& nsend);

    // Receive object
    template <typename T> bool Recv(T& dst, u32& nrecv) {
        return RecvBytes(&dst, sizeof(T), nrecv);
    }
    template <typename T> bool RecvFrom(T& dst, SOSockAddr& addr, u32& nrecv) {
        return RecvBytesFrom(&dst, sizeof(T), addr, nrecv);
    }

    // Send object
    template <typename T> bool Send(const T& src, u32& nsend) {
        return SendBytes(&src, sizeof(T), nsend);
    }
    template <typename T>
    bool SendTo(const T& src, const SOSockAddr& addr, u32& nsend) {
        return SendBytesTo(&src, sizeof(T), addr, nsend);
    }

protected:
    SocketBase(SOSocket socket, SOProtoFamily family, SOSockType type);

private:
    virtual bool RecvImpl(void* dst, u32 len, SOSockAddr* addr, u32& nrecv) = 0;
    virtual bool SendImpl(const void* src, u32 len, const SOSockAddr* addr,
                          u32& nsend) = 0;

protected:
    // Socket file descriptor
    SOSocket mHandle;
    // Socket protocol family
    SOProtoFamily mFamily;
    // Socket type
    SOSockType mType;
};

} // namespace kiwi

#endif
