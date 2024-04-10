#ifndef LIBKIWI_NET_HTTP_REQUEST_H
#define LIBKIWI_NET_HTTP_REQUEST_H
#include <libkiwi/kernel/kiwiAssert.h>
#include <libkiwi/net/kiwiSyncSocket.h>
#include <libkiwi/prim/kiwiHashMap.h>
#include <libkiwi/prim/kiwiString.h>
#include <types.h>

namespace kiwi {

/**
 * @brief HTTP Status code
 */
enum EHttpStatus {
    EHttpStatus_LibkiwiErr = -1, // Internal library error
    EHttpStatus_Dummy = 0,       // Uninitialized

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

    /**
     * @brief Destructor
     */
    ~HttpResponse() {
        delete body;
    }

    EHttpStatus status;          // Status code
    TMap<String, String> header; // Response header
    const char* body;            // Response body/payload
};

/**
 * @brief HTTP 1.1 request wrapper
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
     * @param response Request response
     * @param arg Callback user argument
     */
    typedef void (*ResponseCallback)(const HttpResponse& response, void* arg);

public:
    /**
     * @brief Constructor
     *
     * @param host Server hostname
     */
    HttpRequest(const String& host)
        : mHostName(host),
          mURI("/"),
          mpSocket(NULL),
          mpResponseCallback(NULL),
          mpResponseCallbackArg(NULL) {
        mpSocket = new SyncSocket(SO_PF_INET, SO_SOCK_STREAM);
        K_ASSERT(mpSocket != NULL);

        bool success = mpSocket->Bind();
        K_ASSERT(success);

        // Hostname required by HTTP 1.1
        mHeader["Host"] = host;
        // Identify libkiwi requests by user agent
        mHeader["User-Agent"] = "libkiwi";
    }

    /**
     * @brief Destructor
     */
    ~HttpRequest() {
        delete mpSocket;
    }

    /**
     * @brief Add/update a request header field
     *
     * @param name Field name
     * @param value Field value
     */
    void SetHeaderField(const String& name, const String& value) {
        mHeader.Insert(name, value);
    }

    /**
     * @brief Add/update a URL parameter
     *
     * @param name Parameter name
     * @param value Parameter value
     */
    void SetParameter(const String& name, const String& value) {
        mParams.Insert(name, value);
    }

    /**
     * @brief Change the requested resource
     *
     * @param uri URI value
     */
    void SetURI(const String& uri) {
        mURI = uri;
    }

    const HttpResponse& Send(EMethod method = EMethod_GET);
    void SendAsync(ResponseCallback callback, void* arg = NULL,
                   EMethod method = EMethod_GET);

private:
    typedef TMap<String, String>::ConstIterator ParamIterator;
    typedef TMap<String, String>::ConstIterator HeaderIterator;

private:
    void SendImpl();

    bool Request();
    bool Receive();

private:
    EMethod mMethod;      // Request method
    String mHostName;     // Server host name
    String mURI;          // Requested resource
    SyncSocket* mpSocket; // Connection to server

    TMap<String, String> mParams; // URL parameters
    TMap<String, String> mHeader; // Header fields

    HttpResponse mResponse;              // Server response
    ResponseCallback mpResponseCallback; // Response callback
    void* mpResponseCallbackArg;         // Callback user argument
};

} // namespace kiwi

#endif