#include <libkiwi.h>

namespace kiwi {

/**
 * @brief HTTP request method names
 */
const String HttpRequest::METHOD_NAMES[EMethod_Max] = {"GET", "POST"};

/**
 * @brief HTTP protocol version
 */
const String HttpRequest::PROTOCOL_VERSION = "HTTP/1.1";

/**
 * @brief Constructor
 *
 * @param rHost Server hostname
 * @param port Connection port
 */
HttpRequest::HttpRequest(const String& rHost, u16 port) {
    // Socket is owned by this request
    mIsUserSocket = false;
    mpSocket = new SyncSocket(SO_PF_INET, SO_SOCK_STREAM);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    // Timeout requires non-blocking
    bool success = mpSocket->SetBlocking(false);
    K_ASSERT(success);

    // Any local port is fine
    success = mpSocket->Bind();
    K_ASSERT(success);

    mHost = rHost;
    mPort = port;

    Init();
}

/**
 * @brief Constructor
 *
 * @param pSocket Socket connected to the server
 */
HttpRequest::HttpRequest(SocketBase* pSocket) {
    // TODO: How can we test here that the socket is connected?
    K_ASSERT(pSocket != nullptr);
    K_ASSERT(pSocket->IsOpen());

    // Socket is owned by the caller
    mIsUserSocket = true;
    mpSocket = pSocket;

    // Timeout requires non-blocking
    K_WARN_EX(mpSocket->IsBlocking(),
              "Blocking will be disabled on this socket.\n");
    bool success = mpSocket->SetBlocking(false);
    K_ASSERT(success);

    // TODO: How can we get the hostname/port from the socket?
    mHost = "";
    mPort = 0;

    // TODO: Need state machine for request/receive (can reuse for synchronous)
    K_ASSERT_EX(false, "Not implemented.");
}

/**
 * @brief Destructor
 */
HttpRequest::~HttpRequest() {
    K_ASSERT_EX(mpCallback == nullptr,
                "Don't destroy this object while async request is pending.");

    // User-provided socket will outlive this request
    if (!mIsUserSocket) {
        delete mpSocket;
    }

    mpSocket = nullptr;
}

/**
 * @brief Performs common initialization
 */
void HttpRequest::Init() {
    mMethod = EMethod_Max;
    mResource = "/";
    mTimeOut = OS_MSEC_TO_TICKS(DEFAULT_TIMEOUT);

    mpCallback = nullptr;
    mpCallbackArg = nullptr;

    mHeader["Host"] = mHost;
    mHeader["User-Agent"] = "libkiwi";
}

/**
 * @brief Sends request synchronously
 *
 * @param method Request method
 * @return Server response
 */
const HttpResponse& HttpRequest::Send(EMethod method) {
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    mMethod = method;
    mpCallback = nullptr;
    mpCallbackArg = nullptr;

    SendImpl();
    return mResponse;
}

/**
 * @brief Sends request asynchronously
 *
 * @param pCallback Response callback
 * @param pArg Callback user argument
 * @param method Request method
 */
void HttpRequest::SendAsync(Callback pCallback, void* pArg, EMethod method) {
    K_ASSERT_EX(pCallback != nullptr, "You will lose the reponse!");
    K_ASSERT(method < EMethod_Max);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    mMethod = method;
    mpCallback = pCallback;
    mpCallbackArg = pArg;

    // TODO: Need state machine for request/receive (can reuse for synchronous)
    K_ASSERT_EX(false, "Not implemented.");
    // Thread t(SendImpl, *this);
}

/**
 * @brief Sends request (internal implementation)
 */
void HttpRequest::SendImpl() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    // Beginning timestamp
    Watch w;
    w.Start();

    // Establish connection with server
    while (true) {
        bool connected = true;

        // Request-owned sockets won't have a connection yet
        if (!mIsUserSocket) {
            connected = mpSocket->Connect(SockAddr4(mHost, mPort));
        }

        // After connection we can perform the request
        if (connected) {
            (void)Request();
            (void)Receive();
            break;
        }

        // Timeout while connecting
        if (w.Elapsed() >= mTimeOut) {
            mResponse.error = EHttpErr_CantConnect;
            break;
        }
    }

    // Dispatch user callback
    if (mpCallback != nullptr) {
        mpCallback(mResponse, mpCallbackArg);
    }

    // Signal to destructor
#ifndef NDEBUG
    mpCallback = nullptr;
#endif
}

/**
 * @brief Sends request data
 *
 * @return Success
 */
bool HttpRequest::Request() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    // Request line begins with the resource
    String request = mResource;

    // URL parameter string
    K_FOREACH(mParams) {
        // Parameters delimited by ampersand
        String fmt = it == mParams.Begin() ? "?%s=%s" : "&%s=%s";
        request += Format(fmt, it.Key().CStr(), it.Value().CStr());
    }

    // Finish request line
    request = Format("%s %s %s\n", METHOD_NAMES[mMethod].CStr(), request.CStr(),
                     PROTOCOL_VERSION.CStr());

    // Build header fields
    K_FOREACH(mHeader) {
        request += Format("%s: %s\n", it.Key().CStr(), it.Value().CStr());
    }

    // Request ends with extra newline
    request += "\n";

    // Send request data
    Optional<u32> sent = mpSocket->Send(request);
    bool success = sent && *sent == request.Length();

    // Record socket library error if it failed
    if (!success) {
        mResponse.error = EHttpErr_Socket;
        mResponse.exError = LibSO::GetLastError();
    }

    return success;
}

/**
 * @brief Receives response data
 *
 * @return Successs
 */
bool HttpRequest::Receive() {
    K_ASSERT(mMethod < EMethod_Max);
    K_ASSERT(mpSocket != nullptr);
    K_ASSERT(mpSocket->IsOpen());

    // Beginning timestamp
    Watch w;
    w.Start();

    /**
     * Receive response headers
     */
    String work = "";
    size_t end = String::npos;

    while (end == String::npos) {
        char buffer[TEMP_BUFFER_SIZE] = "";
        Optional<u32> nrecv = mpSocket->RecvBytes(buffer, TEMP_BUFFER_SIZE - 1);

        // Record socket library error if it failed
        if (!nrecv) {
            mResponse.error = EHttpErr_Socket;
            mResponse.exError = LibSO::GetLastError();
            return false;
        }

        if (nrecv) {
            // Continue to build string
            work += buffer;
            // Response header ends in double newline
            end = work.Find("\r\n\r\n");

            // Server has terminated the connection
            if (*nrecv == 0 && LibSO::GetLastError() != SO_EWOULDBLOCK) {
                mResponse.error = EHttpErr_Closed;
                mResponse.exError = 0;
                return false;
            }
        }

        if (w.Elapsed() >= mTimeOut) {
            mResponse.error = EHttpErr_TimedOut;
            mResponse.exError = 0;
            return false;
        }
    }

    // Point index at end of sequence instead of start
    end += sizeof("\r\n\r\n") - 1;
    String headers = work.SubStr(0, end);

    /**
     * Build header dictionary
     */
    TVector<String> lines = headers.Split("\r\n");

    // Needs at least one line (for status code)
    if (lines.Empty()) {
        mResponse.error = EHttpErr_BadResponse;
        mResponse.exError = 0;
        return false;
    }

    // Extract status code
    int num =
        std::sscanf(lines[0], PROTOCOL_VERSION + " %d", &mResponse.status);
    if (num != 1) {
        mResponse.error = EHttpErr_BadResponse;
        mResponse.exError = 0;
        return false;
    }

    // Other lines contain key/value pairs
    for (u32 i = 1; i < lines.Size(); i++) {
        // NOTE: Use Find over Split in case the value also contains a colon
        u32 pos = lines[i].Find(": ");
        u32 after = pos + sizeof(": ") - 1;

        // Malformed line (or part of \r\n\r\n ending)
        if (pos == String::npos) {
            // If this isn't one of the trailing newlines, we have a problem
            if (lines[i] != "\r\n") {
                mResponse.error = EHttpErr_BadResponse;
                mResponse.exError = 0;
                return false;
            }

            continue;
        }

        // Create key/value pair
        String key = lines[i].SubStr(0, pos);
        String value = lines[i].SubStr(after);
        mResponse.header.Insert(key, value);
    }

    /**
     * Receive response body
     */
    Optional<u32> len;
    if (mResponse.header.Contains("Content-Length")) {
        // If we were given the length, we can be 100% sure
        len = ksl::strtoul(*mResponse.header.Find("Content-Length"));
    }

    // We may have read some of the body earlier
    if (end != work.Length()) {
        mResponse.body = work.SubStr(end);
    }

    // Receive the rest of the body
    while (true) {
        char buffer[TEMP_BUFFER_SIZE] = "";
        Optional<u32> nrecv = mpSocket->RecvBytes(buffer, TEMP_BUFFER_SIZE - 1);

        // Record socket library error if it failed
        if (!nrecv) {
            mResponse.error = EHttpErr_Socket;
            mResponse.exError = LibSO::GetLastError();
            return false;
        }

        if (nrecv) {
            // Continue building string
            mResponse.body += buffer;

            // Server is likely done and has terminated the connection
            if (*nrecv == 0 && LibSO::GetLastError() != SO_EWOULDBLOCK) {
                // This is only okay if we've read enough of the body
                if (!len || mResponse.body.Length() >= *len) {
                    break;
                }

                mResponse.error = EHttpErr_Closed;
                mResponse.exError = 0;
                return false;
            }
        }

        // Timeout may be the only way to end the body, so not a failure
        if (w.Elapsed() >= mTimeOut) {
            break;
        }
    };

    mResponse.error = EHttpErr_Success;
    mResponse.exError = 0;
    return true;
}

} // namespace kiwi
