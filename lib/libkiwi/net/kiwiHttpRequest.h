#ifndef LIBKIWI_NET_HTTP_REQUEST_H
#define LIBKIWI_NET_HTTP_REQUEST_H
#include <libkiwi/kernel/kiwiAssert.h>
#include <libkiwi/net/kiwiSyncSocket.h>
#include <libkiwi/prim/kiwiHashMap.h>
#include <libkiwi/prim/kiwiString.h>
#include <types.h>

namespace kiwi {

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
     * @brief Status code
     */
    enum EStatus {
        EStatus_LibkiwiErr = -1, // Internal library error
        EStatus_Dummy = 0,       // Uninitialized

        // TODO: Determine which additional codes are useful here

        // Successful
        EStatus_OK = 200, // OK
        EStatus_Created,  // Created
        EStatus_Accepted, // Accepted

        // Client error
        EStatus_BadReq = 400, // Bad Request
        EStatus_NotAuth,      // Unauthorized
        EStatus_Payment,      // Payment Required
        EStatus_Forbidden,    // Forbidden
        EStatus_NotFound,     // Not Found
        EStatus_Method,       // Method Not Allowed

        // Server error
        EStatus_ServErr = 500, // Internal Server Error
        EStatus_NotImpl,       // Not Implemented
        EStatus_BadGateway,    // Bad Gateway
        EStatus_ServDown,      // Service Unavailable
    };

    /**
     * @brief Request response
     */
    struct Response {
        /**
         * @brief Constructor
         */
        Response() : status(EStatus_OK) {}

        /**
         * @brief Destructor
         */
        ~Response() {
            delete body;
        }

        EStatus status;              // Status code
        TMap<String, String> header; // Response header
        const char* body;            // Response body/payload
    };

    /**
     * @brief Request response callback
     *
     * @param response Request response
     * @param arg Callback user argument
     */
    typedef void (*ResponseCallback)(const Response& response, void* arg);

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

    const Response& Send(EMethod method = EMethod_GET);
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

    Response mResponse;                  // Server response
    ResponseCallback mpResponseCallback; // Response callback
    void* mpResponseCallbackArg;         // Callback user argument
};

} // namespace kiwi

#endif