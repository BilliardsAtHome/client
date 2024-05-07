#include <libkiwi.h>

namespace kiwi {
namespace {

const String sMethodNames[HttpRequest::EMethod_Max] = {"GET", "POST"};
const String sProtocolVer = "1.1";

} // namespace

/**
 * @brief Constructor
 *
 * @param host Server hostname
 */
HttpRequest::HttpRequest(const String& host)
    : mHostName(host),
      mURI("/"),
      mpSocket(NULL),
      mpResponse(NULL),
      mpResponseCallback(NULL),
      mpResponseCallbackArg(NULL) {
    mpSocket = new SyncSocket(SO_PF_INET, SO_SOCK_STREAM);
    K_ASSERT(mpSocket != NULL);

    // Bind to any local port
    bool success = mpSocket->Bind();
    K_ASSERT(success);

    // Hostname required by HTTP 1.1
    mHeader["Host"] = host;
    // Identify libkiwi requests by user agent
    mHeader["User-Agent"] = "libkiwi";
}

/**
 * @brief Send request synchronously
 *
 * @param method Request method
 * @return Server response
 */
const Optional<HttpResponse>& HttpRequest::Send(EMethod method) {
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != NULL);

    mMethod = method;
    mpResponseCallback = NULL;
    mpResponseCallbackArg = NULL;

    // Call on this thread
    SendImpl();

    K_ASSERT(mpResponse != NULL);
    return *mpResponse;
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

    mpResponse = new HttpResponse();
    K_ASSERT(mpResponse != NULL);

    // Establish connection with server
    SockAddr4 addr(mHostName, 80);
    bool success = mpSocket->Connect(addr);

    // Send request, receive server's response
    if (success) {
        success = success && Request();
        success = success && Receive();
    }

    // User callback
    if (mpResponseCallback != NULL) {
        mpResponseCallback(success ? MakeOptional(*mpResponse) : kiwi::nullopt,
                           mpResponseCallbackArg);
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
    K_ASSERT(mpResponse != NULL);

    /**
     * Receive response headers
     */

    // Need non-blocking because we greedily receive data
    bool success = mpSocket->SetBlocking(false);
    K_ASSERT(success);

    return true;

    // Read header string (ends in double newline)
    size_t end;
    String work = "";
    while ((end = work.Find("\r\n\r\n")) == String::npos) {
        char buffer[512 + 1] = "";
        Optional<u32> nrecv = mpSocket->RecvBytes(buffer, sizeof(buffer) - 1);

        // Don't append buffer if it's empty
        if (nrecv) {
            work += buffer;
        }
    }

    // Point index at end of sequence instead of start
    end += sizeof("\r\n\r\n") - 1;
    String headers = work.SubStr(0, end);

    /**
     * Build header dictionary
     */

    // Must at least have one line (status code)
    TVector<String> lines = headers.Split("\r\n");
    if (lines.Size() < 1) {
        return false;
    }

    // Extract status code
    K_ASSERT(lines[0].StartsWith("HTTP/1.1"));
    int num = std::sscanf(lines[0], "HTTP/1.1 %d", &mpResponse->status);
    K_ASSERT(num == 1);

    // Other lines contain key/value pairs
    for (int i = 1; i < lines.Size(); i++) {
        // Use Find over Split in case the value also contains a colon
        u32 pos = lines[i].Find(": ");
        u32 after = pos + sizeof(": ") - 1;

        // Malformed line (or part of \r\n\r\n ending)
        if (pos == String::npos) {
            K_ASSERT_EX(lines[i] == "", "Malformed response header");
            continue;
        }

        // Create key/value pair
        String key = lines[i].SubStr(0, pos);
        String value = lines[i].SubStr(after);
        mpResponse->header.Insert(key, value);
    }

    return true;

    /**
     * Receive response body
     */

    // We may have read *some* of it earlier
    if (end != headers.Length()) {
        mpResponse->body = work.SubStr(end);

        // Try to complete what we read earlier
        while (true) {
            char buffer[512 + 1] = "";
            Optional<u32> nrecv =
                mpSocket->RecvBytes(buffer, sizeof(buffer) - 1);

            // This is likely the end of the body (rather than server stall)
            if (nrecv.ValueOr(0)) {
                // If we were given the length, we can be 100% sure
                if (mpResponse->header.Contains("Content-Length")) {
                    u32 length = ksl::strtoul(
                        *mpResponse->header.Find("Content-Length"));

                    // Yep, we really did read all of it
                    if (mpResponse->body.Length() >= length) {
                        break;
                    }
                }
            }

            mpResponse->body += buffer;
        }
    }

    K_LOG_EX("BODY = %s\n", mpResponse->body.CStr());
    return true;
}

} // namespace kiwi
