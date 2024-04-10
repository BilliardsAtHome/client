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

    mMethod = method;
    mpResponseCallback = NULL;
    mpResponseCallbackArg = NULL;

    // Call on this thread
    SendImpl();

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
    K_ASSERT(callback != NULL);
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != NULL);

    mMethod = method;
    mpResponseCallback = callback;
    mpResponseCallbackArg = arg;

    // Call on new thread
    kiwi::Thread t(SendImpl, *this);
}

/**
 * @brief Common send implementation
 *
 * @return Success
 */
void HttpRequest::SendImpl() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != NULL);

    bool success = true;

    // Establish connection with server
    SockAddr4 addr(mHostName, 80);
    success = mpSocket->Connect(addr);
    K_ASSERT(success);

    // Send request, receive server's response
    success = success && Request();
    success = success && Receive();
    K_ASSERT(mResponse.status != EStatus_Dummy);

    // Internal error
    if (!success) {
        mResponse.status = EStatus_LibkiwiErr;
        K_ASSERT(false);
    }

    // User callback
    if (mpResponseCallback != NULL) {
        mpResponseCallback(mResponse, mpResponseCallbackArg);
    }
}

/**
 * @brief Send request data
 *
 * @return Success
 */
bool HttpRequest::Request() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != NULL);

    // Build URI & URL parameter string
    String request = mURI;
    for (ParamIterator it = mParams.Begin(); it != mParams.End(); ++it) {
        // Parameters delimited by ampersand
        String fmt = it == mParams.Begin() ? "?%s=%s" : "&%s=%s";
        request += Format(fmt, it.Key().CStr(), it.Value().CStr());
    }

    // Build request line
    request = Format("%s %s HTTP/%s\n", sMethodNames[mMethod].CStr(),
                     request.CStr(), sProtocolVer.CStr());

    // Build header fields
    for (HeaderIterator it = mHeader.Begin(); it != mHeader.End(); ++it) {
        request += Format("%s: %s\n", it.Key().CStr(), it.Value().CStr());
    }

    // Request ends with double-line
    request += "\n";

    // Send request data
    Optional<u32> sent = mpSocket->Send(request);
    return sent && sent.Value() == request.Length();
}

/**
 * @brief Receive response data
 *
 * @return Successs
 */
bool HttpRequest::Receive() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != NULL);

    // TODO: Implement
    mResponse.status = EStatus_OK;
    return true;
}

} // namespace kiwi
