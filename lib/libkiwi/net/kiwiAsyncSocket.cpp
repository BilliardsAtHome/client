#include <climits>
#include <libkiwi.h>

namespace kiwi {

OSThread AsyncSocket::sSocketThread;
bool AsyncSocket::sSocketThreadCreated = false;
u8 AsyncSocket::sSocketThreadStack[THREAD_STACK_SIZE];
TList<AsyncSocket> AsyncSocket::sSocketList;

/**
 * Constructor
 *
 * @param _packet Packet for this job
 */
AsyncSocket::Job::Job(Packet* _packet)
    : packet(_packet), onrecv(NULL), arg(NULL) {
    K_ASSERT(packet != NULL);
}

/**
 * Destructor
 */
AsyncSocket::Job::~Job() {
    delete packet;
    packet = NULL;
}

/**
 * Socket thread function
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
 * Constructor
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
 * Constructor
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
 * @param callback Connection callback
 * @param arg Callback user argument
 * @return Success
 */
bool AsyncSocket::Connect(const SockAddr& addr, ConnectCallback callback,
                          void* arg) {
    K_ASSERT(IsOpen());

    mState = EState_Connecting;
    mPeer = addr;

    mpConnectCallback = callback;
    mpConnectCallbackArg = arg;

    // Connect doesn't actually happen on this thread
    return true;
}

/**
 * Accepts remote connection
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
SOResult AsyncSocket::RecvImpl(void* dst, u32 len, u32& nrecv, SockAddr* addr,
                               ReceiveCallback callback, void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(len > 0 && len < ULONG_MAX);
    K_ASSERT_EX(callback != NULL, "Please provide a receive callback");

    // Bad design, I know. But I can't think of a better way....
    K_WARN(dst != NULL, "Async receive will not write to this parameter.\n"
                        "Please use a receive callback instead.");

    // Packet to hold incoming data
    Packet* packet = new Packet(len);
    K_ASSERT(packet != NULL);

    // Asynchronous job
    Job* job = new Job(packet);
    K_ASSERT(job != NULL);

    // Queue up this receive
    job->onrecv = callback;
    job->arg = arg;
    mRecvJobs.PushBack(job);

    // Prevent UB
    if (addr != NULL) {
        std::memset(addr, 0, sizeof(SockAddr));
    }

    // Receive doesn't actually happen on this thread
    nrecv = 0;
    return SO_EWOULDBLOCK;
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
SOResult AsyncSocket::SendImpl(const void* src, u32 len, u32& nsend,
                               const SockAddr* addr, SendCallback callback,
                               void* arg) {
    K_ASSERT(IsOpen());
    K_ASSERT(src != NULL);
    K_ASSERT(len > 0 && len < ULONG_MAX);

    // Packet to hold incoming data
    Packet* packet = new Packet(len, addr);
    K_ASSERT(packet != NULL);

    // Store data inside packet
    packet->Write(src, len);

    // Asynchronous job
    Job* job = new Job(packet);
    K_ASSERT(job != NULL);

    // Queue up this receive
    job->onsend = callback;
    job->arg = arg;
    mSendJobs.PushBack(job);

    // Send doesn't actually happen on this thread
    nsend = 0;
    return SO_EWOULDBLOCK;
}

/**
 * Process pending tasks
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

        // Report non-blocking results
        if (result != SO_EWOULDBLOCK || result != SO_EINPROGRESS) {
            mState = EState_Thinking;
            mpConnectCallback(static_cast<SOResult>(result),
                              mpConnectCallbackArg);
        }
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
            mpAcceptCallback(socket, mPeer, mpAcceptCallbackArg);
        }
        break;
    }
}

/**
 * Receives packet data over socket
 */
void AsyncSocket::CalcRecv() {
    K_ASSERT(IsOpen());

    while (!mRecvJobs.Empty()) {
        // Find next incomplete job (FIFO)
        Job& job = mRecvJobs.Front();
        K_ASSERT_EX(job.packet != NULL, "Job has no packet?");
        K_ASSERT_EX(!job.packet->IsWriteComplete(),
                    "Completed job should be removed");

        // Attempt to complete job
        job.packet->Recv(mHandle);

        if (job.packet->IsWriteComplete()) {
            // Notify user
            if (job.onrecv != NULL) {
                SockAddr peer;
                job.packet->GetPeer(peer);

                // TODO: Maybe pass Packet instead, but that's not really useful
                job.onrecv(peer, job.packet->GetContent(),
                           job.packet->GetContentSize(), job.arg);
            }

            // Remove from queue
            mRecvJobs.PopFront();
            delete &job;
        }
    }
}

/**
 * Sends packet data over socket
 */
void AsyncSocket::CalcSend() {
    K_ASSERT(IsOpen());

    while (!mRecvJobs.Empty()) {
        // Find next incomplete job (FIFO)
        Job& job = mSendJobs.Front();
        K_ASSERT_EX(job.packet != NULL, "Job has no packet?");
        K_ASSERT_EX(!job.packet->IsReadComplete(),
                    "Completed job should be removed");

        // Attempt to complete job
        job.packet->Send(mHandle);

        if (job.packet->IsReadComplete()) {
            // Notify user
            if (job.onsend != NULL) {
                job.onsend(job.arg);
            }

            // Remove from queue
            mSendJobs.PopFront();
            delete &job;
        }
    }
}

} // namespace kiwi
