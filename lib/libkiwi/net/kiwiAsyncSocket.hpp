#ifndef LIBKIWI_NET_ASYNC_SOCKET_H
#define LIBKIWI_NET_ASYNC_SOCKET_H
#include <libkiwi/net/kiwiPacket.hpp>
#include <libkiwi/net/kiwiSocketBase.hpp>
#include <libkiwi/prim/kiwiLinkList.hpp>
#include <revolution/OS.h>
#include <types.h>

namespace kiwi {

/**
 * Asynchronous (non-blocking) socket
 */
class AsyncSocket : public SocketBase {
public:
    /**
     * Socket connect callback
     *
     * @param s32 SOConnect result
     * @param arg User callback argument
     */
    typedef void (*ConnectCallback)(s32 result, void* arg);

    /**
     * Socket accept callback
     *
     * @param socket Peer socket object
     * @param peer Peer address
     * @param arg User callback argument
     */
    typedef void (*AcceptCallback)(SocketBase* socket, const SOSockAddr& peer,
                                   void* arg);

    /**
     * Socket receive callback
     *
     * @param socket Socket which received data
     * @param peer Peer address (NULL if Recv instead of RecvFrom)
     * @param packet Packet data
     * @param size Packet data size
     * @param arg User callback argument
     */
    typedef void (*ReceiveCallback)(SocketBase* socket, const SOSockAddr* peer,
                                    const void* packet, u32 size, void* arg);

private:
    /**
     * Socket async task
     */
    enum ETask { ETask_Thinking, ETask_Connecting, ETask_Accepting };

public:
    AsyncSocket(SOProtoFamily family, SOSockType type);
    virtual ~AsyncSocket();

    virtual bool Connect(const SOSockAddr& addr);
    virtual AsyncSocket* Accept();

    /**
     * Set async connect callback
     *
     * @param callback Callback function
     * @param arg Callback function argument
     */
    void SetConnectCallback(ConnectCallback callback, void* arg = NULL) {
        mpConnectCallback = callback;
        mpConnectCallbackArg = arg;
    }

    /**
     * Set async accept callback
     *
     * @param callback Callback function
     * @param arg Callback function argument
     */
    void SetAcceptCallback(AcceptCallback callback, void* arg = NULL) {
        mpAcceptCallback = callback;
        mpAcceptCallbackArg = arg;
    }

    /**
     * Set async receive callback
     *
     * @param callback Callback function
     * @param arg Callback function argument
     */
    void SetReceiveCallback(ReceiveCallback callback, void* arg = NULL) {
        mpReceiveCallback = callback;
        mpReceiveCallbackArg = arg;
    }

private:
    static void* ThreadFunc(void* arg);

    AsyncSocket(SOSocket socket, SOProtoFamily family, SOSockType type);

    void Initialize();
    void Calc();
    void CalcRecv();
    void CalcSend();

    Packet* FindPacketForRecv();

    virtual bool RecvImpl(void* dst, u32 len, SOSockAddr* addr, u32& nrecv);
    virtual bool SendImpl(const void* src, u32 len, const SOSockAddr* addr,
                          u32& nsend);

private:
    static const u32 THREAD_STACK_SIZE = 0x4000;

    // Current async task
    ETask mTask;
    // Peer address
    SOSockAddr mPeer;

    // Active receive operations
    TList<Packet> mRecvPackets;
    // Active send operations
    TList<Packet> mSendPackets;

    // Connect callback
    ConnectCallback mpConnectCallback;
    void* mpConnectCallbackArg;

    // Accept callback
    AcceptCallback mpAcceptCallback;
    void* mpAcceptCallbackArg;

    // Receive callback
    ReceiveCallback mpReceiveCallback;
    void* mpReceiveCallbackArg;

    // Thread for async socket operation
    static OSThread sSocketThread;
    static bool sSocketThreadCreated;
    static u8 sSocketThreadStack[THREAD_STACK_SIZE];

    // Active async sockets
    static TList<AsyncSocket> sSocketList;
};

} // namespace kiwi

#endif
