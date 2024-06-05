#ifndef LIBKIWI_NET_SOCKET_BASE_H
#define LIBKIWI_NET_SOCKET_BASE_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiOptional.h>
#include <libkiwi/support/kiwiLibSO.h>

namespace kiwi {

/**
 * @brief IOS Berkeley socket wrapper
 */
class SocketBase : private NonCopyable {
public:
    /**
     * @brief Generic socket operation callback
     *
     * @param result Socket library result
     * @param arg User callback argument
     */
    typedef void (*Callback)(SOResult result, void* arg);

    /**
     * @brief Connection accept callback
     *
     * @param result Socket library result
     * @param peer Peer socket object
     * @param addr Peer address
     * @param arg User callback argument
     */
    typedef void (*AcceptCallback)(SOResult result, SocketBase* peer,
                                   const SockAddrAny& addr, void* arg);

public:
    /**
     * @brief Gets the console's IP address
     *
     * @param addr[out] IPv4 address
     */
    static void GetHostAddr(SockAddr4& addr);

    /**
     * @brief Constructor
     *
     * @param family Socket protocol family
     * @param type Socket type
     */
    SocketBase(SOProtoFamily family, SOSockType type);

    /**
     * @brief Destructor
     */
    virtual ~SocketBase();

    /**
     * @brief Tests whether the socket has a valid descriptor
     */
    bool IsOpen() const {
        return mHandle >= 0;
    }

    /**
     * @brief Connects to a peer
     *
     * @param addr Remote address
     * @param callback Connection callback
     * @param arg Callback user argument
     * @return Success
     */
    virtual bool Connect(const SockAddrAny& addr, Callback callback = NULL,
                         void* arg = NULL) = 0;

    /**
     * @brief Accepts a peer connection over a new socket
     *
     * @param callback Acceptance callback
     * @param arg Callback user argument
     * @return New socket
     */
    virtual SocketBase* Accept(AcceptCallback callback = NULL,
                               void* arg = NULL) = 0;

    /**
     * @brief Binds socket to local address
     * @note Bind to port zero for a random port (written out)
     *
     * @param addr[in,out] Local address
     * @return Success
     */
    bool Bind(SockAddrAny& addr = SockAddr4()) const;

    /**
     * @brief Listens for incoming connections
     *
     * @param backlog Maximum pending connections
     * @return Success
     */
    bool Listen(s32 backlog = SOMAXCONN) const;

    /**
     * @brief Tests whether socket is blocking
     */
    bool IsBlocking() const;

    /**
     * @brief Toggles socket blocking
     *
     * @param enable Whether to enable blocking
     * @return Success
     */
    bool SetBlocking(bool enable) const;

    /**
     * @brief Toggles port reuse
     *
     * @param enable Whether to enable port reuse
     * @return Success
     */
    bool SetReuseAddr(bool enable) const;

    /**
     * @brief Stops socket from reading/writing
     *
     * @param how How to shutdown connection
     * @return Success
     */
    bool Shutdown(SOShutdownType how) const;

    /**
     * @brief Closes socket
     *
     * @return Success
     */
    bool Close();

    /**
     * @brief Gets endpoint of socket
     *
     * @param[out] addr Socket address
     * @return Success
     */
    bool GetSocketAddr(SockAddrAny& addr) const;
    /**
     * @brief Gets endpoint of peer
     *
     * @param[out] addr Peer address
     * @return Success
     */
    bool GetPeerAddr(SockAddrAny& addr) const;

    /**
     * @brief Tests whether socket can receive data
     */
    bool CanRecv() const;
    /**
     * @brief Tests whether socket can send data
     */
    bool CanSend() const;

    /**
     * @name Receive data
     */
    /**@{*/
    /**
     * @brief Receives bytes from bound connection
     *
     * @param dst Destination buffer
     * @param len Buffer size
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes received
     */
    Optional<u32> RecvBytes(void* dst, u32 len, Callback callback = NULL,
                            void* arg = NULL);
    /**
     * @brief Receives bytes and records sender address
     *
     * @param dst Destination buffer
     * @param len Buffer size
     * @param addr[out] Sender address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes received
     */
    Optional<u32> RecvBytesFrom(void* dst, u32 len, SockAddrAny& addr,
                                Callback callback = NULL, void* arg = NULL);

    /**
     * @brief Receives object from bound connection
     *
     * @param dst Destination object
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes received
     */
    template <typename T>
    Optional<u32> Recv(T& dst, Callback callback = NULL, void* arg = NULL) {
        return RecvBytes(&dst, sizeof(T), callback, arg);
    }
    /**
     * @brief Receives object and records sender address
     *
     * @param dst Destination object
     * @param addr[out] Sender address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes received
     */
    template <typename T>
    Optional<u32> RecvFrom(T& dst, SockAddrAny& addr, Callback callback = NULL,
                           void* arg = NULL) {
        return RecvBytesFrom(&dst, sizeof(T), addr, callback, arg);
    }
    /**@}*/

    /**
     * @name Send data
     */
    /**@{*/
    /**
     * @brief Sends bytes to bound connection
     *
     * @param src Source buffer
     * @param len Buffer size
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    Optional<u32> SendBytes(const void* src, u32 len, Callback callback = NULL,
                            void* arg = NULL);
    /**
     * @brief Sends bytes to specified connection
     *
     * @param src Source buffer
     * @param len Buffer size
     * @param addr Destination address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    Optional<u32> SendBytesTo(const void* src, u32 len, const SockAddrAny& addr,
                              Callback callback = NULL, void* arg = NULL);

    /**
     * @brief Sends object to bound connection
     *
     * @param src Source object
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    template <typename T>
    Optional<u32> Send(const T& src, Callback callback = NULL,
                       void* arg = NULL) {
        return SendBytes(&src, sizeof(T), callback, arg);
    }
    /**
     * @brief Sends object to specified connection
     *
     * @param src Source object
     * @param addr Destination address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    template <typename T>
    Optional<u32> SendTo(const T& src, const SockAddrAny& addr,
                         Callback callback = NULL, void* arg = NULL) {
        return SendBytesTo(&src, sizeof(T), addr, callback, arg);
    }

    /**
     * @brief Sends string to bound connection
     *
     * @param src Source string
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    template <typename T>
    Optional<u32> Send(const StringImpl<T>& src, Callback callback = NULL,
                       void* arg = NULL) {
        return SendBytes(src.CStr(), src.Length() * sizeof(T), callback, arg);
    }
    /**
     * @brief Sends string to specified connection
     *
     * @param src Source string
     * @param addr Destination address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Number of bytes sent
     */
    template <typename T>
    Optional<u32> SendTo(const StringImpl<T>& src, const SockAddrAny& addr,
                         Callback callback = NULL, void* arg = NULL) {
        return SendBytesTo(src.CStr(), src.Length() * sizeof(T), addr, callback,
                           arg);
    }
    /**@}*/

protected:
    /**
     * @brief Constructor
     *
     * @param socket Socket file descriptor
     * @param type Socket protocol family
     * @param type Socket type
     */
    SocketBase(SOSocket socket, SOProtoFamily family, SOSockType type);

private:
    /**
     * @brief Receives data and records sender address (internal implementation)
     *
     * @param dst Destination buffer
     * @param len Buffer size
     * @param[out] nrecv Number of bytes received
     * @param[out] addr Sender address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Socket library result
     */
    virtual SOResult RecvImpl(void* dst, u32 len, u32& nrecv, SockAddrAny* addr,
                              Callback callback, void* arg) = 0;

    /**
     * @brief Sends data to specified connection (internal implementation)
     *
     * @param src Source buffer
     * @param len Buffer size
     * @param[out] nsend Number of bytes sent
     * @param addr Sender address
     * @param callback Completion callback
     * @param arg Callback user argument
     * @return Socket library result
     */
    virtual SOResult SendImpl(const void* src, u32 len, u32& nsend,
                              const SockAddrAny* addr, Callback callback,
                              void* arg) = 0;

protected:
    SOSocket mHandle;      // File descriptor
    SOProtoFamily mFamily; // Protocol family
    SOSockType mType;      // Socket type
};

} // namespace kiwi

#endif
