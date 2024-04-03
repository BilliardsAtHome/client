#ifndef LIBKIWI_NET_SOCKET_BASE_H
#define LIBKIWI_NET_SOCKET_BASE_H
#include <libkiwi/prim/kiwiOptional.hpp>
#include <libkiwi/rvl/kiwiLibSO.hpp>
#include <types.h>

namespace kiwi {

/**
 * IOS Berkeley socket wrapper
 */
class SocketBase : private NonCopyable {
public:
    /**
     * Connection establish callback
     *
     * @param result Socket library result
     * @param arg User callback argument
     */
    typedef void (*ConnectCallback)(SOResult result, void* arg);

    /**
     * Connection accept callback
     *
     * @param result Socket library result
     * @param peer Peer socket object
     * @param addr Peer address
     * @param arg User callback argument
     */
    typedef void (*AcceptCallback)(SOResult result, SocketBase* peer,
                                   const SockAddr& addr, void* arg);

    /**
     * Data receive callback
     *
     * @param result Socket library result
     * @param addr Peer address
     * @param arg User callback argument
     */
    typedef void (*ReceiveCallback)(SOResult result, const SockAddr& addr,
                                    void* arg);

    /**
     * Data send callback
     *
     * @param result Socket library result
     * @param arg User callback argument
     */
    typedef void (*SendCallback)(SOResult result, void* arg);

public:
    static void GetHostAddr(SockAddr4& addr);

    SocketBase(SOProtoFamily family, SOSockType type);
    virtual ~SocketBase();

    /**
     * Tests whether the socket has a valid descriptor
     */
    bool IsOpen() const {
        return mHandle >= 0;
    }

    virtual bool Connect(const SockAddr& addr, ConnectCallback callback = NULL,
                         void* arg = NULL) = 0;
    virtual SocketBase* Accept(AcceptCallback callback = NULL,
                               void* arg = NULL) = 0;

    bool Bind(SockAddr& addr) const;
    bool Listen(s32 backlog = SOMAXCONN) const;
    bool SetBlocking(bool enable) const;
    bool Shutdown(SOShutdownType how) const;
    bool Close();

    bool GetSocketAddr(SockAddr& addr) const;
    bool GetPeerAddr(SockAddr& addr) const;

    bool CanRecv() const;
    bool CanSend() const;

    // Receive bytes
    Optional<u32> RecvBytes(void* buf, u32 len, ReceiveCallback callback = NULL,
                            void* arg = NULL);
    Optional<u32> RecvBytesFrom(void* buf, u32 len, SockAddr& addr,
                                ReceiveCallback callback = NULL,
                                void* arg = NULL);

    // Send bytes
    Optional<u32> SendBytes(const void* buf, u32 len,
                            SendCallback callback = NULL, void* arg = NULL);
    Optional<u32> SendBytesTo(const void* buf, u32 len, const SockAddr& addr,
                              SendCallback callback = NULL, void* arg = NULL);

    // Receive object
    template <typename T>
    Optional<u32> Recv(T& dst, ReceiveCallback callback = NULL,
                       void* arg = NULL) {
        return RecvBytes(&dst, sizeof(T), callback, arg);
    }
    template <typename T>
    Optional<u32> RecvFrom(T& dst, SockAddr& addr,
                           ReceiveCallback callback = NULL, void* arg = NULL) {
        return RecvBytesFrom(&dst, sizeof(T), addr, callback, arg);
    }

    // Send object
    template <typename T>
    Optional<u32> Send(const T& src, SendCallback callback = NULL,
                       void* arg = NULL) {
        return SendBytes(&src, sizeof(T), callback, arg);
    }
    template <typename T>
    Optional<u32> SendTo(const T& src, const SockAddr& addr,
                         SendCallback callback = NULL, void* arg = NULL) {
        return SendBytesTo(&src, sizeof(T), addr, callback, arg);
    }

protected:
    SocketBase(SOSocket socket, SOProtoFamily family, SOSockType type);

private:
    virtual SOResult RecvImpl(void* dst, u32 len, u32& nrecv, SockAddr* addr,
                              ReceiveCallback callback, void* arg) = 0;
    virtual SOResult SendImpl(const void* src, u32 len, u32& nsend,
                              const SockAddr* addr, SendCallback callback,
                              void* arg) = 0;

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
