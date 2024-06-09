#ifndef LIBKIWI_NET_HTTP_REQUEST_H
#define LIBKIWI_NET_HTTP_REQUEST_H
#include <libkiwi/debug/kiwiAssert.h>
#include <libkiwi/k_config.h>
#include <libkiwi/k_types.h>
#include <libkiwi/prim/kiwiHashMap.h>
#include <libkiwi/prim/kiwiString.h>

namespace kiwi {

// Forward declarations
class SyncSocket;

/**
 * @brief HTTP error
 */
enum EHttpErr {
    EHttpErr_Success, // OK

    EHttpErr_CantConnect, // Can't connect to the server
    EHttpErr_BadResponse, // Malformed server response
    EHttpErr_TimedOut,    // Connection timed out
    EHttpErr_Closed,      // Connection closed
    EHttpErr_Socket,      // Misc. socket error
};

/**
 * @brief HTTP status code
 */
enum EHttpStatus {
    // TODO: Determine which additional codes are useful here

    // Successful
    EHttpStatus_OK = 200, // OK
    EHttpStatus_Created,  // Created
    EHttpStatus_Accepted, // Accepted

    // Client error
    EHttpStatus_BadReq = 400, // Bad Request
    EHttpStatus_NotAuth,      // Unauthorized
    EHttpStatus_Payment,      // Payment Required
    EHttpStatus_Forbidden,    // Forbidden
    EHttpStatus_NotFound,     // Not Found
    EHttpStatus_Method,       // Method Not Allowed

    // Server error
    EHttpStatus_ServErr = 500, // Internal Server Error
    EHttpStatus_NotImpl,       // Not Implemented
    EHttpStatus_BadGateway,    // Bad Gateway
    EHttpStatus_ServDown,      // Service Unavailable
};

/**
 * @brief HTTP request response
 */
struct HttpResponse {
    /**
     * @brief Constructor
     */
    HttpResponse() : status(EHttpStatus_OK) {}

    EHttpErr error;              // Error code
    EHttpStatus status;          // Status code
    TMap<String, String> header; // Response header
    String body;                 // Response body/payload
};

/**
 * @brief HTTP (1.1) request wrapper
 */
class HttpRequest {
public:
    /**
     * @brief Request method
     */
    enum EMethod {
        // TODO: Should we support more?
        EMethod_GET,
        EMethod_POST,

        EMethod_Max
    };

    /**
     * @brief Request response callback
     *
     * @param resp Request response
     * @param arg Callback user argument
     */
    typedef void (*ResponseCallback)(const HttpResponse& resp, void* arg);

public:
    /**
     * @brief Constructor
     *
     * @param host Server hostname
     */
    explicit HttpRequest(const String& host);

    /**
     * @brief Destructor
     */
    ~HttpRequest() {
        K_ASSERT_EX(
            mpResponseCallback == NULL,
            "Don't destroy this object while async request is pending.");

        delete mpSocket;
    }

    /**
     * @brief Sets the maximum state duration before timeout
     *
     * @param timeOut Time-out period, in milliseconds
     */
    void SetTimeOut(u32 timeOut) {
        mTimeOut = OS_MSEC_TO_TICKS(timeOut);
    }

    /**
     * @brief Adds/updates a request header field
     *
     * @param name Field name
     * @param value Field value
     */
    void SetHeaderField(const String& name, const String& value) {
        mHeader.Insert(name, value);
    }

    /**
     * @brief Adds/updates a URL parameter
     *
     * @param name Parameter name
     * @param value Parameter value
     */
    template <typename T>
    void SetParameter(const String& name, const T& value) {
        SetParameter(name, kiwi::ToString(value));
    }

    /**
     * @brief Changes the requested resource
     *
     * @param uri URI value
     */
    void SetURI(const String& uri) {
        mURI = uri;
    }

    /**
     * @brief Sends request synchronously
     *
     * @param method Request method
     * @return Server response
     */
    const HttpResponse& Send(EMethod method = EMethod_GET);

    /**
     * @brief Sends request asynchronously
     *
     * @param callback Response callback
     * @param arg Callback user argument
     * @param method Request method
     */
    void SendAsync(ResponseCallback callback, void* arg = NULL,
                   EMethod method = EMethod_GET);

private:
    /**
     * @brief Sends request (internal implementation)
     */
    void SendImpl();

    /**
     * @brief Sends request data
     *
     * @return Success
     */
    bool Request();
    /**
     * @brief Receives response data
     *
     * @return Successs
     */
    bool Receive();

private:
    /**
     * @brief Default connection timeout, in milliseconds
     */
    static const u32 DEFAULT_TIMEOUT = 2000;

    /**
     * @brief HTTP request method names
     */
    static const char* sMethodNames[EMethod_Max];
    /**
     * @brief HTTP protocol version
     */
    static const char* sProtocolVer;

    EMethod mMethod;  // Request method
    String mHostName; // Server host name
    String mURI;      // Requested resource

    SyncSocket* mpSocket; // Connection to server
    u32 mTimeOut;         // Connection timeout

    TMap<String, String> mParams; // URL parameters
    TMap<String, String> mHeader; // Header fields

    HttpResponse mResponse;              // Server response
    ResponseCallback mpResponseCallback; // Response callback
    void* mpResponseCallbackArg;         // Callback user argument
};

} // namespace kiwi

#endif
