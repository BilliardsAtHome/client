#include <libkiwi.h>

namespace kiwi {
namespace {

const String sMethodNames[HttpRequest::EMethod_Max] = {"GET", "POST"};

}

/**
 * @brief Send request synchronously
 *
 * @param method Request method
 * @return Response structure
 */
HttpRequest::Response HttpRequest::Send(EMethod method) {
    K_ASSERT(method < EMethod_Max);

    // HTTP always uses port 80
    SockAddr4 addr(mHostName, 80);
    bool success = mpSocket->Connect(addr);
    K_ASSERT(success);

    TMap<String, String>::ConstIterator it = mHeader.Begin();
    for (; it != mHeader.End(); ++it) {
    }
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
}

} // namespace kiwi