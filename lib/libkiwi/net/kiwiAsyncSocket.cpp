#include <libkiwi.h>

namespace kiwi {

OSThread AsyncSocket::sSocketThread;
bool AsyncSocket::sSocketThreadCreated = false;
u8 AsyncSocket::sSocketThreadStack[THREAD_STACK_SIZE];
TList<AsyncSocket> AsyncSocket::sSocketList;

/**
 * Socket thread function
 */
void* AsyncSocket::ThreadFunc(void* arg) {
#pragma unused(arg)
    s32 result;

    // Operate all open sockets
    while (true) {
        for (TList<AsyncSocket>::Iterator it = sSocketList.Begin();
             it != sSocketList.End(); it++) {
            K_ASSERT_EX(it->IsOpen(),
                        "Closed socket shouldn't be in the active list");

            it->Calc();
        }
    }

    return NULL;
}

/**
 * Constructor
 *
 * @param family Socket protocol family
 * @param type Socket type
 */
AsyncSocket::AsyncSocket(SOProtoFamily family, SOSockType type)
    : SocketBase(family, type),
      mTask(ETask_Thinking),
      mpConnectCallback(NULL),
      mpConnectCallbackArg(NULL),
      mpAcceptCallback(NULL),
      mpAcceptCallbackArg(NULL),
      mpReceiveCallback(NULL),
      mpReceiveCallbackArg(NULL) {
    Initialize();
}

/**
 * Constructor
 *
 * @param socket Socket file descriptor
 * @param type Socket protocol family
 * @param type Socket type
 */
AsyncSocket::AsyncSocket(SOSocket socket, SOProtoFamily family, SOSockType type)
    : SocketBase(socket, family, type),
      mTask(ETask_Thinking),
      mpConnectCallback(NULL),
      mpConnectCallbackArg(NULL),
      mpAcceptCallback(NULL),
      mpAcceptCallbackArg(NULL),
      mpReceiveCallback(NULL),
      mpReceiveCallbackArg(NULL) {
    Initialize();
}

/**
 * Prepares socket for async operation
 */
void AsyncSocket::Initialize() {
    // Make socket non-blocking
    bool success = SetBlocking(false);
    K_ASSERT(success);

    // Thread needs to see this socket
    sSocketList.PushBack(this);

    // Thread must exist if there is an open socket
    if (!sSocketThreadCreated) {
        OSCreateThread(&sSocketThread, ThreadFunc, NULL,
                       sSocketThreadStack + sizeof(sSocketThreadStack),
                       sizeof(sSocketThreadStack), OS_PRIORITY_MAX, 0);

        sSocketThreadCreated = true;
        OSResumeThread(&sSocketThread);
    }
}

/**
 * Destructor
 */
AsyncSocket::~AsyncSocket() {
    sSocketList.Remove(this);
}

/**
 * Connects to another socket
 *
 * @param addr Remote address
 * @return Success
 */
bool AsyncSocket::Connect(const SOSockAddr& addr) {
    mPeer = addr;
    mTask = ETask_Connecting;
    // Connect doesn't actually happen on this thread
    return false;
}

/**
 * Accepts remote connection
 *
 * @return New socket
 */
AsyncSocket* AsyncSocket::Accept() {
    mTask = ETask_Accepting;
    // Accept doesn't actually happen on this thread
    return NULL;
}

/**
 * Receives data from specified connection
 *
 * @param dst Destination buffer
 * @param len Buffer size
 * @param[out] addr Sender address
 * @param[out] nrecv Number of bytes received
 * @return Success (or blocking)
 */
bool AsyncSocket::RecvImpl(void* dst, u32 len, SOSockAddr* addr, u32& nrecv) {
    K_ASSERT(len > 0);
    K_ASSERT_EX(mpReceiveCallback != NULL,
                "Please register the async receive callback");

    // Bad design, I know. But I can't think of a better way....
    K_WARN(dst != NULL, "Async receive will not write to this parameter.\n"
                        "Please use the receive callback instead.");

    // Queue receive task
    Packet* packet = new Packet(len);
    K_ASSERT(packet != NULL);
    mRecvPackets.PushBack(packet);

    // Prevent UB
    nrecv = 0;
    if (addr != NULL) {
        std::memset(addr, 0, sizeof(SOSockAddr));
    }

    // Receive doesn't actually happen on this thread
    return true;
}

/**
 * Sends data to specified connection
 *
 * @param src Source buffer
 * @param len Buffer size
 * @param addr Destination address
 * @param[out] nsend Number of bytes sent
 * @return Success (or blocking)
 */
bool AsyncSocket::SendImpl(const void* src, u32 len, const SOSockAddr* addr,
                           u32& nsend) {
    K_ASSERT(src != NULL);
    K_ASSERT(len > 0);

    Packet* packet = new Packet(len, addr);
    K_ASSERT(packet != NULL);

    // Store data inside packet
    u32 written = packet->Write(src, len);

    // Queue packet for sending
    mSendPackets.PushFront(packet);
    return written;
}

/**
 * Attempt to complete the pending task
 */
void AsyncSocket::Calc() {
    s32 result;

    switch (mTask) {
    case ETask_Thinking:
        CalcRecv();
        CalcSend();
        break;

    case ETask_Connecting:
        result = LibSO::Connect(mHandle, mPeer);

        // Report non-blocking results
        if (result != SO_EWOULDBLOCK || result != SO_EINPROGRESS) {
            mTask = ETask_Thinking;
            mpConnectCallback(result, mpConnectCallbackArg);
        }
        break;

    case ETask_Accepting:
        result = LibSO::Accept(mHandle, mPeer);

        // Report non-blocking results
        if (result != SO_EWOULDBLOCK) {
            // Peer connection
            AsyncSocket* socket = NULL;

            // Result code is the peer descriptor
            if (result >= 0) {
                AsyncSocket* socket = new AsyncSocket(result, mFamily, mType);
                K_ASSERT(socket != NULL);
            }

            mTask = ETask_Thinking;
            mpAcceptCallback(socket, mPeer, mpAcceptCallbackArg);
        }
        break;
    }
}

/**
 * Searches for the next packet which should be received from the socket
 */
Packet* AsyncSocket::FindPacketForRecv() {
    for (TList<Packet>::Iterator it = mRecvPackets.Begin();
         it != mRecvPackets.End(); it++) {
        // Incomplete packet means more data can be received
        if (!it->IsWriteComplete()) {
            return &*it;
        }
    }

    // All packets are complete
    return NULL;
}

/**
 * Receives packet data over socket
 */
void AsyncSocket::CalcRecv() {
    while (true) {
        // Find next incomplete packet
        Packet* packet = FindPacketForRecv();
        if (packet == NULL) {
            return;
        }

        // Attempt to complete packet
        packet->Recv(mHandle);

        // If complete, dispatch message handler
        if (packet->IsWriteComplete()) {
            K_ASSERT_EX(mpReceiveCallback != NULL,
                        "Please register the async receive callback");

            if (packet->HasPeer()) {
                SOSockAddr peer;
                packet->GetPeer(peer);
                mpReceiveCallback(this, &peer, packet->GetContent(),
                                  packet->GetContentSize(),
                                  mpReceiveCallbackArg);
            } else {
                mpReceiveCallback(this, NULL, packet->GetContent(),
                                  packet->GetContentSize(),
                                  mpReceiveCallbackArg);
            }
        }
    }
}

/**
 * Sends packet data over socket
 */
void AsyncSocket::CalcSend() {
    while (!mSendPackets.Empty()) {
        Packet& packet = mSendPackets.Back();
        packet.Send(mHandle);

        // Remove packet if completely sent
        if (packet.IsWriteComplete()) {
            mSendPackets.PopBack();
            delete &packet;
        }
    }
}

} // namespace kiwi
