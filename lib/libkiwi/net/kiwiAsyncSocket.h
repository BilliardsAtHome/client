#ifndef LIBKIWI_NET_ASYNC_SOCKET_H
#define LIBKIWI_NET_ASYNC_SOCKET_H
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/net/kiwiSocketBase.h>
#include <libkiwi/prim/kiwiLinkList.h>
#include <revolution/OS.h>

namespace kiwi {

/**
 * @brief Asynchronous (non-blocking) socket
 */
class AsyncSocket : public SocketBase {
public:
    /**
     * @brief Constructor
     *
     * @param family Socket protocol family
     * @param type Socket type
     */
    AsyncSocket(SOProtoFamily family, SOSockType type);

    /**
     * @brief Destructor
     */
    virtual ~AsyncSocket();

    /**
     * @brief Connects to a peer
     *
     * @param addr Remote address
     * @param callback Connection callback
     * @param arg Callback user argument
     * @return Success
     */
    virtual bool Connect(const SockAddrAny& addr, Callback callback, void* arg);

    /**
     * @brief Accepts a peer connection over a new socket
     *
     * @param callback Acceptance callback
     * @param arg Callback user argument
     * @return New socket
     */
    virtual AsyncSocket* Accept(AcceptCallback callback, void* arg);

private:
    /**
     * @brief Async state
     */
    enum EState { EState_Thinking, EState_Connecting, EState_Accepting };

    /**
     * @brief Async receive operation
     */
    class RecvJob;
    /**
     * @brief Async send operation
     */
    class SendJob;

private:
    /**
     * @brief Socket thread function
     */
    static void* ThreadFunc(void* arg);

    /**
     * @brief Constructor
     *
     * @param socket Socket file descriptor
     * @param type Socket protocol family
     * @param type Socket type
     */
    AsyncSocket(SOSocket socket, SOProtoFamily family, SOSockType type);

    /**
     * @brief Prepares socket for async operation
     */
    void Initialize();
    /**
     * @brief Processes pending socket tasks
     */
    void Calc();

    /**
     * @brief Receives packet data over socket
     */
    void CalcRecv();
    /**
     * @brief Sends packet data over socket
     */
    void CalcSend();

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
                              Callback callback, void* arg);

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
                              void* arg);

private:
    static const u32 THREAD_STACK_SIZE = 0x4000;

    volatile EState mState; // Current async task
    SockAddrAny mPeer;      // Peer address

    TList<RecvJob> mRecvJobs; // Active receive jobs
    TList<SendJob> mSendJobs; // Active send jobs

    Callback mpConnectCallback; // Connect callback
    void* mpConnectCallbackArg; // Connect callback user argument

    AcceptCallback mpAcceptCallback; // Accept callback
    void* mpAcceptCallbackArg;       // Accept callback user argument

    static OSThread sSocketThread;                   // Socket manager thread
    static bool sSocketThreadCreated;                // Thread guard
    static u8 sSocketThreadStack[THREAD_STACK_SIZE]; // Thread stack

    static TList<AsyncSocket> sSocketList; // Open async sockets
};

} // namespace kiwi

#endif
