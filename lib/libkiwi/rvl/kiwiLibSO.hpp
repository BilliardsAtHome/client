#ifndef LIBKIWI_RVL_LIBSO_HPP
#define LIBKIWI_RVL_LIBSO_HPP
#include <libkiwi/prim/kiwiString.hpp>
#include <revolution/SO.h>
#include <types.h>

namespace kiwi {

/**
 * SO library wrapper/extension
 */
class LibSO {
public:
    static void Initialize();

    static s32 Socket(SOProtoFamily family, SOSockType type);
    static s32 Close(SOSocket socket);
    static s32 Listen(SOSocket socket, s32 backlog = SOMAXCONN);
    static s32 Accept(SOSocket socket, SOSockAddr& addr);
    static s32 Bind(SOSocket socket, SOSockAddr& addr);
    static s32 Connect(SOSocket socket, const SOSockAddr& addr);
    static s32 GetSockName(SOSocket socket, SOSockAddr& addr);
    static s32 GetPeerName(SOSocket socket, SOSockAddr& addr);

    static s32 Read(SOSocket socket, void* dst, s32 n);
    static s32 Recv(SOSocket socket, void* dst, s32 n, u32 flags);
    static s32 RecvFrom(SOSocket socket, void* dst, s32 n, u32 flags,
                        SOSockAddr& addr);

    static s32 Write(SOSocket socket, const void* src, s32 n);
    static s32 Send(SOSocket socket, const void* src, s32 n, u32 flags);
    static s32 SendTo(SOSocket socket, const void* src, s32 n, u32 flags,
                      const SOSockAddr& addr);

    static s32 Fcntl(SOSocket socket, SOFcntlCmd cmd, ...);
    static s32 Shutdown(SOSocket socket, SOShutdownType how);
    static s32 Poll(SOPollFD fds[], u32 numfds, s64 timeout);

    static bool INetPtoN(String str, SOInAddr& addr);
    static bool INetPtoN(String str, SOInAddr6& addr);
    static String INetNtoP(const SOInAddr& addr);
    static String INetNtoP(const SOInAddr6& addr);

    static void GetHostID(SOInAddr& addr);

    static s32 GetSockOpt(SOSocket socket, SOSockOptLevel level, SOSockOpt opt,
                          void* val, u32 len);
    static s32 SetSockOpt(SOSocket socket, SOSockOptLevel level, SOSockOpt opt,
                          const void* val, u32 len);

private:
    static s32 RecvImpl(SOSocket socket, void* dst, s32 n, u32 flags,
                        SOSockAddr* addr);
    static s32 SendImpl(SOSocket socket, const void* src, s32 n, u32 flags,
                        const SOSockAddr* addr);

private:
    // IOS IP device handle
    static s32 sDeviceHandle;
};

/**
 * SO IPv4 address wrapper to simplify upcasting
 */
struct SockAddr4 : public SOSockAddrIn {
    operator SOSockAddr&() {
        return reinterpret_cast<SOSockAddr&>(*this);
    }
    operator const SOSockAddr&() const {
        return reinterpret_cast<const SOSockAddr&>(*this);
    }

    /**
     * @brief Constructor
     */
    SockAddr4() {
        len = sizeof(SOSockAddrIn);
        family = SO_AF_INET;
        port = 0;
        addr.raw = 0;
    }

    /**
     * @brief Constructor
     *
     * @param _addr IPv4 address (string)
     * @param _port Port
     */
    SockAddr4(String _addr, u16 _port) {
        len = sizeof(SOSockAddrIn);
        family = SO_AF_INET;
        port = _port;

        bool success = LibSO::INetPtoN(_addr, addr);
        K_ASSERT(success);
    }

    /**
     * @brief Constructor
     *
     * @param _addr IPv4 address
     * @param _port Port
     */
    SockAddr4(u32 _addr, u16 _port) {
        len = sizeof(SOSockAddrIn);
        family = SO_AF_INET;
        port = _port;
        addr.raw = _addr;
    }

    /**
     * @brief Constructor
     *
     * @param _port Port
     */
    SockAddr4(u16 _port) {
        len = sizeof(SOSockAddrIn);
        family = SO_AF_INET;
        port = _port;
        addr.raw = 0;
    }

    /**
     * @brief Convert IPv4 address to string
     */
    String ToString() const {
        return LibSO::INetNtoP(addr);
    }
};

/**
 * SO IPv6 address wrapper to simplify upcasting
 */
struct SockAddr6 : public SOSockAddrIn6 {
    operator SOSockAddr&() {
        return reinterpret_cast<SOSockAddr&>(*this);
    }
    operator const SOSockAddr&() const {
        return reinterpret_cast<const SOSockAddr&>(*this);
    }

    /**
     * @brief Constructor
     */
    SockAddr6() {
        len = sizeof(SOSockAddrIn6);
        family = SO_AF_INET6;
        port = 0;
        std::memset(&addr, 0, sizeof(SOInAddr6));
    }

    /**
     * @brief Constructor
     *
     * @param _addr IPv6 address (string)
     * @param _port Port
     */
    SockAddr6(String _addr, u16 _port) {
        len = sizeof(SOSockAddrIn6);
        family = SO_AF_INET6;
        port = _port;
        LibSO::INetPtoN(_addr, addr);
    }

    /**
     * @brief Constructor
     *
     * @param _port Port
     */
    SockAddr6(u16 _port) {
        len = sizeof(SOSockAddrIn6);
        family = SO_AF_INET6;
        port = _port;
        std::memset(&addr, 0, sizeof(SOInAddr6));
    }

    /**
     * @brief Convert IPv6 address to string
     */
    String ToString() const {
        return LibSO::INetNtoP(addr);
    }
};

// Should be SO types and nothing more
K_STATIC_ASSERT(sizeof(SockAddr4) == sizeof(SOSockAddrIn));
K_STATIC_ASSERT(sizeof(SockAddr6) == sizeof(SOSockAddrIn6));

} // namespace kiwi

#endif
