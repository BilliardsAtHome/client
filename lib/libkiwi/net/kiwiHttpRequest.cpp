#include <libkiwi.h>

namespace kiwi {
namespace {

const String sMethodNames[HttpRequest::EMethod_Max] = {"GET", "POST"};
const String sProtocolVer = "1.1";

} // namespace

/**
 * @brief Send request synchronously
 *
 * @param method Request method
 * @return Server response
 */
const HttpRequest::Response& HttpRequest::Send(EMethod method) {
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != NULL);
    K_ASSERT(mAsyncState == EState_Idle);

    // Wait on async version
    SendAsync(NULL, NULL, method);
    while (mAsyncState != EState_Finish) {
        ;
    }

    K_ASSERT(mResponse.status != EStatus_Dummy);
    return mResponse;
}

/**
 * @brief Send request asynchronously
 *
 * @param callback Response callback
 * @param arg Callback user argument
 * @param method Request method
 */
void HttpRequest::SendAsync(ResponseCallback callback, void* arg,
                            EMethod method) {
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != NULL);
    K_ASSERT_EX(mAsyncState == EState_Idle,
                "Please wait before calling this function");

    mMethod = method;
    mpResponseCallback = callback;
    mpResponseCallbackArg = arg;

    // Begin state machine
    mAsyncState = EState_Connecting;
    SocketCallback(SO_SUCCESS, this);
}

/**
 * @brief Async socket state machine
 *
 * @param result Socket library result
 * @param arg Callback user argument (this request)
 */
void HttpRequest::SocketCallback(SOResult result, void* arg) {
    K_ASSERT(arg != NULL);

    // Callback argument is the request object
    HttpRequest* self = static_cast<HttpRequest*>(arg);

    K_LOG_EX("Socket callback! State %d", self->mAsyncState);

    // Socket library failure
    if (result != SO_SUCCESS) {
        self->mResponse.status = EStatus_LibkiwiErr;
    }

    // Async operation is finished
    if (self->mAsyncState == EState_Finish) {
        K_ASSERT(self->mResponse.status != EStatus_Dummy);

        if (self->mpResponseCallback) {
            self->mpResponseCallback(self->mResponse,
                                     self->mpResponseCallbackArg);
        }

        return;
    }

    // State machine
    switch (self->mAsyncState) {
    case EState_Connecting:
        self->StateConnecting();
        self->mAsyncState = EState_Sending;
        break;
    case EState_Sending:
        self->StateSending();
        self->mAsyncState = EState_Receiving;
        break;
    case EState_Receiving:
        self->StateReceiving();
        self->mAsyncState = EState_Finish;
        break;
    default:
        K_ASSERT_EX(false, "Invalid state");
        self->mAsyncState = EState_Idle;
        break;
    }
}

/**
 * @brief Connect to server
 */
void HttpRequest::StateConnecting() {
    K_ASSERT(mpSocket != NULL);
    K_ASSERT(mAsyncState == EState_Connecting);

    // Establish connection with server
    SockAddr4 addr(mHostName, 80);
    mpSocket->Connect(addr, SocketCallback, this);
}

/**
 * @brief Send request data
 */
void HttpRequest::StateSending() {
    K_ASSERT(mpSocket != NULL);
    K_ASSERT(mAsyncState == EState_Sending);
    K_ASSERT(mMethod < EMethod_Max);

    Optional<u32> sent;
    String request;

    // Build URL parameter string
    request += mParams.Empty() ? "/" : "/?";
    for (TMap<String, String>::ConstIterator it = mParams.Begin();
         it != mParams.End(); ++it) {
        request += Format("&%s=%s", it.Key().CStr(), it.Value().CStr());
    }

    // Build request line
    request = Format("%s %s HTTP/%s\n", sMethodNames[mMethod].CStr(),
                     request.CStr(), sProtocolVer.CStr());

    // Build header fields
    for (TMap<String, String>::ConstIterator it = mHeader.Begin();
         it != mHeader.End(); ++it) {
        request += Format("%s: %s\n", it.Key().CStr(), it.Value().CStr());
    }

    // :D
    request += "User-Agent: libkiwi\n";
    // Request ends with double-line
    request += "\n\n";

    // Send request data
    mpSocket->Send(request, SocketCallback, this);
}

/**
 * @brief Receive response data
 */
void HttpRequest::StateReceiving() {
    K_ASSERT(mpSocket != NULL);
    K_ASSERT(mAsyncState == EState_Receiving);

    // TODO: Implement
    SocketCallback(SO_SUCCESS, this);
}

} // namespace kiwi
