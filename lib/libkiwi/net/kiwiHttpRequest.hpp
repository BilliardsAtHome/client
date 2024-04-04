#ifndef LIBKIWI_NET_HTTP_REQUEST_H
#define LIBKIWI_NET_HTTP_REQUEST_H
#include <libkiwi/net/kiwiSyncSocket.hpp>
#include <libkiwi/prim/kiwiHashMap.hpp>
#include <libkiwi/prim/kiwiString.hpp>
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
          mpSocket(NULL),
          mpResponseCallback(NULL),
          mpResponseCallbackArg(NULL),
          mAsyncState(EState_None) {
        mpSocket = new SyncSocket(SO_PF_INET, SO_SOCK_STREAM);
        K_ASSERT(mpSocket != NULL);

        bool success = mpSocket->Bind();
        K_ASSERT(success);
    }

    /**
     * @brief Destructor
     */
    ~HttpRequest() {
        delete mpSocket;
    }

    /**
     * @brief Access a header field by name
     * @note Creates field if it does not already exist
     *
     * @param key Key
     * @return Existing field, or new field
     */
    String& operator[](const String& key) {
        return mHeader[key];
    }

    Response Send(EMethod method = EMethod_GET);

    void SendAsync(ResponseCallback callback, void* arg = NULL,
                   EMethod method = EMethod_GET);

private:
    /**
     * @brief Asynchronous state
     */
    enum EState {
        EState_None,
        EState_Connecting,
        EState_Sending,
        EState_Receiving
    };

private:
    String mHostName;             // Server host name
    SyncSocket* mpSocket;         // Connection to server
    TMap<String, String> mHeader; // Header fields

    ResponseCallback mpResponseCallback; // Response callback
    void* mpResponseCallbackArg;         // Callback user argument

    EState mAsyncState; // Current state (for async only)
};

} // namespace kiwi

#endif