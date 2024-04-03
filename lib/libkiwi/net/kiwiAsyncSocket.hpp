#ifndef LIBKIWI_NET_ASYNC_SOCKET_H
#define LIBKIWI_NET_ASYNC_SOCKET_H
#include <libkiwi/net/kiwiPacket.hpp>
#include <libkiwi/net/kiwiSocketBase.hpp>
#include <libkiwi/prim/kiwiLinkList.hpp>
#include <libkiwi/prim/kiwiOptional.hpp>
#include <revolution/OS.h>
#include <types.h>

namespace kiwi {

/**
 * Asynchronous (non-blocking) socket
 */
class AsyncSocket : public SocketBase {
public:
    AsyncSocket(SOProtoFamily family, SOSockType type);
    virtual ~AsyncSocket();

    virtual bool Connect(const SockAddr& addr, ConnectCallback callback = NULL,
                         void* arg = NULL);
    virtual AsyncSocket* Accept(AcceptCallback callback = NULL,
                                void* arg = NULL);

private:
    /**
     * Async state
     */
    enum EState { EState_Thinking, EState_Connecting, EState_Accepting };

    /**
     * Async operation
     */
    struct Job {
        Job(Packet* _packet);
        ~Job();

        // Packet associated with this job
        Packet* packet;

        // Job completion callback
        union {
            ReceiveCallback onrecv;
            SendCallback onsend;
        };

        // Callback user argument
        void* arg;
    };

private:
    static void* ThreadFunc(void* arg);

    AsyncSocket(SOSocket socket, SOProtoFamily family, SOSockType type);

    void Initialize();
    void Calc();
    void CalcRecv();
    void CalcSend();

    virtual SOResult RecvImpl(void* dst, u32 len, u32& nrecv, SockAddr* addr,
                              ReceiveCallback callback, void* arg);
    virtual SOResult SendImpl(const void* src, u32 len, u32& nsend,
                              const SockAddr* addr, SendCallback callback,
                              void* arg);

private:
    static const u32 THREAD_STACK_SIZE = 0x4000;

    // Current async task
    EState mState;
    // Peer address
    SockAddr mPeer;

    // Active packet jobs
    TList<Job> mRecvJobs;
    TList<Job> mSendJobs;

    // Connect callback
    ConnectCallback mpConnectCallback;
    void* mpConnectCallbackArg;

    // Accept callback
    AcceptCallback mpAcceptCallback;
    void* mpAcceptCallbackArg;

    // Thread for async socket operation
    static OSThread sSocketThread;
    static bool sSocketThreadCreated;
    static u8 sSocketThreadStack[THREAD_STACK_SIZE];

    // Active async sockets
    static TList<AsyncSocket> sSocketList;
};

} // namespace kiwi

#endif
