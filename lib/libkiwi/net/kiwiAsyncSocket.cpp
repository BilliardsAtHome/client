#include <libkiwi.h>

namespace kiwi {

/**
 * @brief Socket manager thread
 */
OSThread AsyncSocket::sSocketThread;

/**
 * @brief Thread guard
 */
bool AsyncSocket::sSocketThreadCreated = false;

/**
 * @brief Thread stack
 */
u8 AsyncSocket::sSocketThreadStack[THREAD_STACK_SIZE];

/**
 * @brief Open async sockets
 */
TList<AsyncSocket> AsyncSocket::sSocketList;

/**
 * @brief Async receive operation
 */
class AsyncSocket::RecvJob {
public:
    /**
     * @brief Constructor
     *
     * @param _packet Packet for this job
     * @param _dst Destination address
     * @param[out] _peer Peer address
     * @param _callback Completion callback
     * @param _arg Callback user argument
     */
    RecvJob(Packet* _packet, void* _dst, SockAddrAny* _peer,
            Callback _callback = NULL, void* _arg = NULL)
        : packet(_packet), dst(_dst), callback(_callback), arg(_arg) {
        K_ASSERT(packet != NULL);
        K_ASSERT(dst != NULL);
    }

    /**
     * @brief Destructor
     */
    ~RecvJob() {
        delete packet;
    }

    /**
     * @brief Tests whether the receive operation is complete
     */
    bool IsComplete() const {
        K_ASSERT(packet != NULL);
        return packet->IsWriteComplete();
    }

    /**
     * @brief Updates job using the specified socket
     *
     * @param socket Socket descriptor
     * @return Whether the job is complete
     */
    bool Calc(SOSocket socket) {
        K_ASSERT(packet != NULL);
        K_ASSERT(socket >= 0);

        // Nothing left to do
        if (IsComplete()) {
            return true;
        }

        // Update
        packet->Recv(socket);
        bool done = IsComplete();

        // Write out data
        if (done) {
            K_ASSERT(dst != NULL);
            std::memcpy(dst, packet->GetContent(), packet->GetContentSize());

            // Write peer information
            if (peer != NULL) {
                *peer = packet->GetPeer();
            }

            // Notify user
            if (callback != NULL) {
                callback(LibSO::GetLastError(), arg);
            }
        }

        return done;
    }

private:
    Packet* packet;   // Packet to complete
    void* dst;        // Where to store packet data
    SOSockAddr* peer; // Where to store peer address

    Callback callback; // Completion callback
    void* arg;         // Completion callback user argument
};

/**
 * @brief Async send operation
 */
class AsyncSocket::SendJob {
public:
    /**
     * @brief Constructor
     *
     * @param _packet Packet for this job
     * @param _callback Completion callback
     * @param _arg Callback user argument
     */
    SendJob(Packet* _packet, Callback _callback = NULL, void* _arg = NULL)
        : packet(_packet), callback(_callback), arg(_arg) {
        K_ASSERT(packet != NULL);
    }

    /**
     * @brief Destructor
     */
    ~SendJob() {
        delete packet;
    }

    /**
     * @brief Tests whether the send operation is complete
     */
    bool IsComplete() const {
        K_ASSERT(packet != NULL);
        return packet->IsReadComplete();
    }

    /**
     * @brief Updates job using the specified socket
     *
     * @param socket Socket descriptor
     * @return Whether the job is complete
     */
    bool Calc(SOSocket socket) {
        K_ASSERT(packet != NULL);
        K_ASSERT(socket >= 0);

        // Nothing left to do
        if (IsComplete()) {
            return true;
        }

        // Update
        packet->Send(socket);
        bool done = IsComplete();

        // Fire callback
        if (done && callback != NULL) {
            callback(LibSO::GetLastError(), arg);
        }

        return done;
    }

private:
    Packet* packet; // Packet to complete

    Callback callback; // Completion callback
    void* arg;         // Completion callback user argument
};

/**
 * @brief Socket thread function
 */
void* AsyncSocket::ThreadFunc(void* arg) {
#pragma unused(arg)

    // Update all open sockets
    while (true) {
        for (TList<AsyncSocket>::Iterator it = sSocketList.Begin();
             it != sSocketList.End(); it++) {
            K_ASSERT(it->IsOpen());
            it->Calc();
        }
    }

    return NULL;
}

/**
 * @brief Constructor
 *
 * @param family Socket protocol family
 * @param type Socket type
 */
AsyncSocket::AsyncSocket(SOProtoFamily family, SOSockType type)
    : SocketBase(family, type),
      mState(EState_Thinking),
      mpConnectCallback(NULL),
      mpConnectCallbackArg(NULL),
      mpAcceptCallback(NULL),
      mpAcceptCallbackArg(NULL) {
    Initialize();
}

/**
 * @brief Constructor
 *
 * @param socket Socket file descriptor
 * @param type Socket protocol family
 * @param type Socket type
 */
AsyncSocket::AsyncSocket(SOSocket socket, SOProtoFamily family, SOSockType type)
    : SocketBase(socket, family, type),
      mState(EState_Thinking),
      mpConnectCallback(NULL),
      mpConnectCallbackArg(NULL),
      mpAcceptCallback(NULL),
      mpAcceptCallbackArg(NULL) {
    Initialize();
}

/**
 * @brief Destructor
 */
AsyncSocket::~AsyncSocket() {
    sSocketList.Remove(this);
}

/**
 * @brief Prepares socket for async operation
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
 * @brief Connects to another socket
 *
 * @param addr Remote address
 * @param callback Connection callback
 * @param arg Callback user argument
 * @return Success
 */
bool AsyncSocket::Connect(const SockAddrAny& addr, Callback callback,
                          void* arg) {
    K_ASSERT(IsOpen());

    mState = EState_Connecting;
    mPeer = addr;

    mpConnectCallback = callback;
    mpConnectCallbackArg = arg;

    // Connect doesn't actually happen on this thread
    return false;
}

/**
 * @brief Accepts remote connection
 *
 * @param callback Acceptance callback
 * @param arg Callback user argument
 * @return New socket
 */
AsyncSocket* AsyncSocket::Accept(AcceptCallback callback, void* arg) {
    K_ASSERT(IsOpen());

    mState = EState_Accepting;

    mpAcceptCallback = callback;
    mpAcceptCallbackArg = arg;

    // Accept doesn't actually happen on this thread
    return NULL;
}

/**
 * @brief Processes pending socket tasks
 */
void AsyncSocket::Calc() {
    s32 result;

    K_ASSERT(IsOpen());

    switch (mState) {
    case EState_Thinking:
        CalcRecv();
        CalcSend();
        break;

    case EState_Connecting:
        result = LibSO::Connect(mHandle, mPeer);

        // Blocking, try again
        if (result == SO_EINPROGRESS || result == SO_EALREADY) {
            break;
        }

        // Connection complete (looking for EISCONN here)
        mState = EState_Thinking;
        mpConnectCallback(result == SO_EISCONN ? SO_SUCCESS
                                               : LibSO::GetLastError(),
                          mpConnectCallbackArg);

        break;

    case EState_Accepting:
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

            mState = EState_Thinking;
            mpAcceptCallback(LibSO::GetLastError(), socket, mPeer,
                             mpAcceptCallbackArg);
        }
        break;
    }
}

/**
 * @brief Receives packet data over socket
 */
void AsyncSocket::CalcRecv() {
    K_ASSERT(IsOpen());

    // Nothing to do
    if (mRecvJobs.Empty()) {
        return;
    }

    // Find next incomplete job (FIFO)
    RecvJob& job = mRecvJobs.Front();
    K_ASSERT_EX(!job.IsComplete(), "Completed job should be removed");

    // Attempt to complete job
    if (job.Calc(mHandle)) {
        // Remove from queue
        mRecvJobs.PopFront();
        delete &job;
    }
}

/**
 * @brief Sends packet data over socket
 */
void AsyncSocket::CalcSend() {
    K_ASSERT(IsOpen());

    // Nothing to do
    if (mSendJobs.Empty()) {
        return;
    }

    // Find next incomplete job (FIFO)
    SendJob& job = mSendJobs.Front();
    K_ASSERT_EX(!job.IsComplete(), "Completed job should be removed");

    // Attempt to complete job
    if (job.Calc(mHandle)) {
        // Remove from queue
        mSendJobs.PopFront();
        delete &job;
    }
}

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
SOResult AsyncSocket::RecvImpl(void* dst, u32 len, u32& nrecv,
                               SockAddrAny* addr, Callback callback,
                               void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(dst != NULL);
    K_ASSERT_EX(!PtrUtil::IsStack(dst), "Don't use stack memory for async");

    // Packet to hold incoming data
    Packet* packet = new Packet(len);
    K_ASSERT(packet != NULL);

    // Asynchronous job
    RecvJob* job = new RecvJob(packet, dst, addr, callback, arg);
    K_ASSERT(job != NULL);
    mRecvJobs.PushBack(job);

    // Receive doesn't actually happen on this thread
    nrecv = 0;
    return SO_EWOULDBLOCK;
}

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
SOResult AsyncSocket::SendImpl(const void* src, u32 len, u32& nsend,
                               const SockAddrAny* addr, Callback callback,
                               void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(src != NULL);
    K_ASSERT_EX(!PtrUtil::IsStack(src), "Don't use stack memory for async");

    // Packet to hold incoming data
    Packet* packet = new Packet(len, addr);
    K_ASSERT(packet != NULL);

    // Store data inside packet
    packet->Write(src, len);

    // Asynchronous job
    SendJob* job = new SendJob(packet, callback, arg);
    K_ASSERT(job != NULL);
    mSendJobs.PushBack(job);

    // Send doesn't actually happen on this thread
    nsend = 0;
    return SO_EWOULDBLOCK;
}

} // namespace kiwi
