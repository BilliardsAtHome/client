#include <cstdio>
#include <libkiwi.h>
#include <revolution/IPC.h>

namespace kiwi {

/**
 * IOS I/O control codes
 */
enum {
    // dev/net/ip/top
    Ioctl_SOAccept = 1,
    Ioctl_SOBind = 2,
    Ioctl_SOClose = 3,
    Ioctl_SOConnect = 4,
    Ioctl_SOFcntl = 5,
    Ioctl_SOGetPeerName = 6,
    Ioctl_SOGetSocketName = 7,
    Ioctl_SOSetSockOpt = 9,
    Ioctl_SOListen = 10,
    Ioctl_SOPoll = 11,
    Ioctl_SORecvFrom = 12,
    Ioctl_SOSendTo = 13,
    Ioctl_SOShutdown = 14,
    Ioctl_SOCreate = 15,
    Ioctl_SOGetHostID = 16,
    Ioctl_SOINetAtoN = 21,
    Ioctl_SOStartup = 31,

    // dev/net/ncd/manage
    IoctlV_NCDGetLinkStatus = 7,

    // dev/net/kd/request
    Ioctl_NWC24iStartupSocket = 6,
};

struct NCDLinkStatus {
    /* 0x00 */ s32 state;
    /* 0x04 */ s32 linkState;
    /* 0x08 */ u8 padding[32 - 0x08];
};
struct NWC24Result {
    /* 0x00 */ s32 result;
    /* 0x04 */ s32 exResult;
    /* 0x08 */ u8 padding[32 - 0x08];
};

IosDevice LibSO::sIosDevice;
SOResult LibSO::sLastError = SO_SUCCESS;

/**
 * Accesses IOS IP device for socket operation
 *
 * @note Please call this before other LibSO functions
 */
void LibSO::Initialize() {
    // Prevent double initialization
    if (sIosDevice.IsOpen()) {
        return;
    }

    /**
     *
     * 1. Check network config link status
     *
     */
    {
        IosDevice ncd_manage;
        ncd_manage.Open("/dev/net/ncd/manage", 1000);
        K_ASSERT(ncd_manage.IsOpen());

        IosObject<NCDLinkStatus> linkStatus;
        s32 result =
            ncd_manage.IoctlV(IoctlV_NCDGetLinkStatus, NULL, &linkStatus);
        K_ASSERT_EX(result >= 0, "NCDGetLinkStatus failed (%d)", result);

        K_ASSERT(linkStatus->linkState >= 0);
    }

    /**
     *
     * 2. NWC24 socket startup
     *
     */
    {
        IosDevice kd_request;
        kd_request.Open("/dev/net/kd/request", 1000);
        K_ASSERT(kd_request.IsOpen());

        IosObject<NWC24Result> commonResult;
        s32 result =
            kd_request.Ioctl(Ioctl_NWC24iStartupSocket, NULL, &commonResult);
        K_ASSERT_EX(result >= 0, "NWC24iStartupSocket failed (%d)", result);

        K_ASSERT(commonResult->result >= 0);
        K_ASSERT(commonResult->exResult >= 0);
    }

    /**
     *
     * 3. SO startup
     *
     */
    {
        sIosDevice.Open("/dev/net/ip/top", IPC_OPEN_NONE);
        K_ASSERT_EX(sIosDevice.IsOpen(), "Couldn't open /dev/net/ip/top");

        s32 result = sIosDevice.Ioctl(Ioctl_SOStartup);
        K_ASSERT_EX(result >= 0, "SOStartup failed (%d)", result);
    }
}

/**
 * Determine the most recent error code
 */
SOResult LibSO::GetLastError() {
    return sLastError;
}

struct SOSocketArgs {
    /* 0x00 */ s32 family;
    /* 0x04 */ s32 type;
    /* 0x08 */ s32 protocol;
};
/**
 * Creates a socket
 *
 * @param family Protocol family
 * @param type Socket type
 * @return New socket descriptor, or IOS error code
 */
s32 LibSO::Socket(SOProtoFamily family, SOSockType type) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(family == SO_PF_INET || family == SO_PF_INET6,
                "Invalid protocol family (%d)", family);

    K_ASSERT_EX(type == SO_SOCK_STREAM || type == SO_SOCK_DGRAM,
                "Invalid socket type (%d)", type);

    IosObject<SOSocketArgs> args;
    args->family = family;
    args->type = type;

    // IOS must auto-detect protocol
    args->protocol = SO_IPPROTO_IP;

    s32 result = sIosDevice.Ioctl(Ioctl_SOCreate, &args, NULL);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    return result;
}

/**
 * Closes a socket descriptor
 *
 * @param socket Socket descriptor
 * @return IOS error code
 */
SOResult LibSO::Close(SOSocket socket) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    IosObject<s32> fd;
    *fd = socket;

    s32 result = sIosDevice.Ioctl(Ioctl_SOClose, &fd, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

struct SOListenArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ s32 backlog;
};
/**
 * Listens for socket connections
 *
 * @param socket Socket descriptor
 * @param backlog Maximum pending connections (default 5)
 * @return IOS error code
 */
SOResult LibSO::Listen(SOSocket socket, s32 backlog) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    IosObject<SOListenArgs> args;
    args->fd = socket;
    args->backlog = backlog;

    s32 result = sIosDevice.Ioctl(Ioctl_SOListen, &args, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

/**
 * Accepts a new connection on a socket
 *
 * @param socket Socket descriptor
 * @param[in,out] addr Remote address
 * @return Socket descriptor or IOS error code
 */
s32 LibSO::Accept(SOSocket socket, SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    IosObject<s32> fd;
    *fd = socket;

    // Address length is specified by input
    IosObject<SockAddr> out;
    out->len = addr.len;

    // Result >= 0 == peer descriptor
    s32 result = sIosDevice.Ioctl(Ioctl_SOAccept, &fd, &out);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    if (result >= 0) {
        std::memcpy(&addr, &*out, out->len);
    }

    return result;
}

struct SOBindArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ BOOL hasDest;
    /* 0x08 */ SockAddr dest;
};
/**
 * Binds a name to a socket
 *
 * @param socket Socket descriptor
 * @param addr[in,out] Local address (zero for random port)
 * @return IOS error code
 */
SOResult LibSO::Bind(SOSocket socket, SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    K_ASSERT_EX(addr.port != 0, "Port auto-detect not supported by IOS");

    IosObject<SOBindArgs> args;
    args->fd = socket;
    args->hasDest = TRUE;
    args->dest = addr;

    s32 result = sIosDevice.Ioctl(Ioctl_SOBind, &args, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

struct SOConnectArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ BOOL hasDest;
    /* 0x08 */ SockAddr dest;
};
/**
 * Connects a socket
 *
 * @param socket Socket descriptor
 * @param addr Remote address
 * @return IOS error code
 */
SOResult LibSO::Connect(SOSocket socket, const SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    IosObject<SOConnectArgs> args;
    args->fd = socket;
    args->hasDest = TRUE;
    args->dest = addr;

    s32 result = sIosDevice.Ioctl(Ioctl_SOConnect, &args, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

/**
 * Gets the socket name
 *
 * @param socket Socket descriptor
 * @param[in,out] addr Local address
 * @return IOS error code
 */
SOResult LibSO::GetSockName(SOSocket socket, SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    IosObject<s32> fd;
    *fd = socket;

    IosObject<SockAddr> self;
    self->len = addr.len;

    s32 result = sIosDevice.Ioctl(Ioctl_SOGetSocketName, &fd, &self);
    sLastError = static_cast<SOResult>(result);

    if (result >= 0) {
        std::memcpy(&addr, &*self, self->len);
    }

    return sLastError;
}

/**
 * Gets the peer socket name
 *
 * @param socket Socket descriptor
 * @param[in,out] addr Remote address
 * @return IOS error code
 */
SOResult LibSO::GetPeerName(SOSocket socket, SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    IosObject<s32> fd;
    *fd = socket;

    IosObject<SockAddr> peer;
    peer->len = addr.len;

    s32 result = sIosDevice.Ioctl(Ioctl_SOGetPeerName, &fd, &peer);
    sLastError = static_cast<SOResult>(result);

    if (result >= 0) {
        std::memcpy(&addr, &*peer, peer->len);
    }

    return sLastError;
}

/**
 * Receives a message from a connected socket
 *
 * @param socket Socket descriptor
 * @param dst Destination buffer
 * @param len Number of bytes to read
 * @return Number of bytes read, or IOS error code
 */
s32 LibSO::Read(SOSocket socket, void* dst, u32 len) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(dst != NULL);

    return Recv(socket, dst, len, 0);
}

/**
 * Receives a message from a connected socket
 *
 * @param socket Socket descriptor
 * @param dst Destination buffer
 * @param len Number of bytes to read
 * @param flags Operation flags
 * @return Number of bytes read, or IOS error code
 */
s32 LibSO::Recv(SOSocket socket, void* dst, u32 len, u32 flags) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(dst != NULL);

    return RecvImpl(socket, dst, len, flags, NULL);
}

/**
 * Receives a message from a specified address
 *
 * @param socket Socket descriptor
 * @param dst Destination buffer
 * @param len Number of bytes to read
 * @param flags Operation flags
 * @param[out] addr Sender address
 * @return Number of bytes read, or IOS error code
 */
s32 LibSO::RecvFrom(SOSocket socket, void* dst, u32 len, u32 flags,
                    SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(dst != NULL);

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    return RecvImpl(socket, dst, len, flags, &addr);
}

/**
 * Sends a message on a socket
 *
 * @param socket Socket descriptor
 * @param src Source buffer
 * @param len Number of bytes to write
 * @return Number of bytes written, or IOS error code
 */
s32 LibSO::Write(SOSocket socket, const void* src, u32 len) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    return Send(socket, src, len, 0);
}

/**
 * Sends a message on a socket
 *
 * @param socket Socket descriptor
 * @param src Source buffer
 * @param len Number of bytes to write
 * @param flags Operation flags
 * @return Number of bytes written, or IOS error code
 */
s32 LibSO::Send(SOSocket socket, const void* src, u32 len, u32 flags) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    return SendImpl(socket, src, len, flags, NULL);
}

/**
 * Sends a message to a specified address
 *
 * @param socket Socket descriptor
 * @param src Source buffer
 * @param len Number of bytes to write
 * @param flags Operation flags
 * @param addr[in] Destination address
 * @return Number of bytes written, or IOS error code
 */
s32 LibSO::SendTo(SOSocket socket, const void* src, u32 len, u32 flags,
                  const SockAddr& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    return SendImpl(socket, src, len, flags, &addr);
}

struct SORecvArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ u32 flags;
};
/**
 * Receives a message and records its sender
 *
 * @param socket Socket descriptor
 * @param dst Destination buffer
 * @param len Number of bytes to read
 * @param flags Operation flags
 * @param[out] addr Sender address
 * @return Number of bytes read, or IOS error code
 */
s32 LibSO::RecvImpl(SOSocket socket, void* dst, u32 len, u32 flags,
                    SockAddr* addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(dst != NULL);

    K_ASSERT_EX(addr == NULL || addr->len == sizeof(SockAddr4) ||
                    addr->len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr->len);

    // Input vectors
    enum {
        V_IARGS, // Input vector for ioctl args
        V_IMAX,  // Total input vector count
    };

    IosVectors input(V_IMAX);

    // Output vectors
    enum {
        V_OBUFF, // Output vector for user buffer
        V_OADDR, // Output vector for source address
        V_OMAX,  // Total output vector count
    };

    IosVectors output(V_OMAX);

    IosObject<SORecvArgs> args;
    args->fd = socket;
    args->flags = flags;

    // Input vector 1: Ioctl args
    input[V_IARGS].Set(args.Base(), args.Size());

    // Output vector 1: Destination buffer
    output[V_OBUFF].Set(dst, len);

    // Output vector 2: Source address
    IosObject<SockAddr> from;
    if (addr != NULL) {
        *from = *addr;
        output[V_OADDR].Set(from.Base(), from.Size());
    } else {
        output[V_OADDR].Clear();
    }

    s32 result = sIosDevice.IoctlV(Ioctl_SORecvFrom, &input, &output);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    return result;
}

struct SOSendArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ u32 flags;
    /* 0x08 */ BOOL hasDest;
    /* 0x0C */ SockAddr dest;
};
/**
 * Sends a message to a specified address
 *
 * @param socket Socket descriptor
 * @param src Source buffer
 * @param len Number of bytes to write
 * @param flags Operation flags
 * @param addr Recipient address
 * @return Number of bytes written, or IOS error code
 */
s32 LibSO::SendImpl(SOSocket socket, const void* src, u32 len, u32 flags,
                    const SockAddr* addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    K_ASSERT_EX(addr == NULL || addr->len == sizeof(SockAddr4) ||
                    addr->len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr->len);

    // Input vectors
    enum {
        V_IBUFF, // Input vector for user buffer
        V_IARGS, // Input vector for ioctl args
        V_IMAX,  // Total I/O vector count
    };

    IosVectors input(V_IMAX);

    IosObject<SOSendArgs> args;
    args->fd = socket;
    args->flags = flags;

    // Input vector 1: Source buffer
    input[V_IBUFF].Set(const_cast<void*>(src), len);
    // Input vector 2: Ioctl args
    input[V_IARGS].Set(args.Base(), args.Size());

    // Copy in destination address
    if (addr != NULL) {
        args->hasDest = TRUE;
        args->dest = *addr;
    } else {
        args->hasDest = FALSE;
    }

    // Request send
    s32 result = sIosDevice.IoctlV(Ioctl_SOSendTo, &input);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    return result;
}

struct SOFcntlArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ s32 cmd;
    /* 0x08 */ void* arg;
};
/**
 * Controls socket file descriptor
 *
 * @param socket Socket descriptor
 * @param cmd Command
 * @param ... Command argument
 * @return Command return value, or IOS error code
 */
s32 LibSO::Fcntl(SOSocket socket, SOFcntlCmd cmd, ...) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    std::va_list list;
    va_start(list, cmd);
    void* arg = va_arg(list, void*);
    va_end(list);

    IosObject<SOFcntlArgs> args;
    args->fd = socket;
    args->cmd = cmd;
    args->arg = arg;

    s32 result = sIosDevice.Ioctl(Ioctl_SOFcntl, &args, NULL);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    return result;
}

struct SOShutdownArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ s32 type;
};
/**
 * Shuts down specified part of socket connection
 *
 * @param socket Socket descriptor
 * @param how How to shutdown connection
 * @return IOS error code
 */
SOResult LibSO::Shutdown(SOSocket socket, SOShutdownType how) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    IosObject<SOShutdownArgs> args;
    args->fd = socket;
    args->type = how;

    s32 result = sIosDevice.Ioctl(Ioctl_SOShutdown, &args, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

/**
 * Wait for events on socket file descriptors
 *
 * @param[in,out] fds Socket descriptor array
 * @param numfds Socket descriptor array size
 * @param timeout Timeout for blocking
 * @return Number of socket results written out, or IOS error code
 */
s32 LibSO::Poll(SOPollFD fds[], u32 numfds, s64 timeout) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(fds != NULL);
    K_ASSERT(numfds > 0);

    SOPollFD* p = new (32, EMemory_MEM2) SOPollFD[numfds];

    IosObject<s64> msec;
    *msec = OS_TICKS_TO_MSEC(timeout);

    // Length known at runtime so cannot use IosObject
    IosVectors output(1);
    output[0].Set(p, numfds * sizeof(SOPollFD));

    // Input provides sockets to watch
    K_ASSERT(p != NULL);
    std::memcpy(p, fds, numfds * sizeof(SOPollFD));

    s32 result = sIosDevice.Ioctl(Ioctl_SOPoll, &msec, &output);
    sLastError = result >= 0 ? SO_SUCCESS : static_cast<SOResult>(result);

    // Output provides search results
    if (result >= 0) {
        std::memcpy(fds, p, numfds * sizeof(SOPollFD));
    }

    delete p;
    return result;
}

/**
 * @brief Convert hostname to IPv4 address
 *
 * @param name Hostname
 * @param[out] addr Socket address
 * @return IOS error code
 */
bool LibSO::INetAtoN(const String& name, SockAddr4& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    u32 len = name.Length();
    char* p = new (32, EMemory_MEM2) char[len + 1];

    // Length known at runtime so cannot use IosObject
    IosVectors input(1);
    input[0].Set(p, len);

    // String copy with MEM2 constraints
    K_ASSERT(p != NULL);
    std::strncpy(p, name, len);
    p[len] = '\0';

    IosObject<SOInAddr> output;

    s32 result = sIosDevice.Ioctl(Ioctl_SOINetAtoN, &input, &output);
    sLastError = result == 1 ? SO_SUCCESS : SO_EINVAL;

    if (result == 1) {
        addr.addr.raw = output->raw;
    }

    delete p;
    return result == 1;
}

/**
 * Converts a string to a socket address
 *
 * @param str Address string
 * @param[out] addr Socket address
 * @return Success
 */
bool LibSO::INetPtoN(const String& str, SockAddr& addr) {
    bool success;

    switch (addr.len) {
    case sizeof(SockAddr4):
        success = std::sscanf(str, "%d.%d.%d.%d", &addr.in.addr.octets[0],
                              &addr.in.addr.octets[1], &addr.in.addr.octets[2],
                              &addr.in.addr.octets[3]) ==
                  LENGTHOF(addr.in.addr.octets);
        break;

    case sizeof(SockAddr6):
        success =
            std::sscanf(str, "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
                        &addr.in6.addr.groups[0], &addr.in6.addr.groups[1],
                        &addr.in6.addr.groups[2], &addr.in6.addr.groups[3],
                        &addr.in6.addr.groups[4], &addr.in6.addr.groups[5],
                        &addr.in6.addr.groups[6], &addr.in6.addr.groups[7]) ==
            LENGTHOF(addr.in6.addr.groups);
        break;

    default:
        success = false;
        K_ASSERT_EX(false, "Invalid SockAddr length (%d)", addr.len);
        break;
    }

    sLastError = success ? SO_SUCCESS : SO_EINVAL;
    return success;
}

/**
 * Converts a socket address to a string
 *
 * @param addr Socket address
 * @return Address string
 */
String LibSO::INetNtoP(const SockAddr& addr) {
    switch (addr.len) {
    case sizeof(SockAddr4):
        sLastError = SO_SUCCESS;
        return Format("%d.%d.%d.%d", addr.in.addr.octets[0],
                      addr.in.addr.octets[1], addr.in.addr.octets[2],
                      addr.in.addr.octets[3]);

    case sizeof(SockAddr6):
        sLastError = SO_SUCCESS;
        return Format("%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
                      addr.in6.addr.groups[0], addr.in6.addr.groups[1],
                      addr.in6.addr.groups[2], addr.in6.addr.groups[3],
                      addr.in6.addr.groups[4], addr.in6.addr.groups[5],
                      addr.in6.addr.groups[6], addr.in6.addr.groups[7]);

    default:
        sLastError = SO_EINVAL;
        K_ASSERT_EX(false, "Invalid SockAddr length (%d)", addr.len);
        return "";
    }
}

/**
 * Gets the host machine's IPv4 address
 *
 * @param[out] addr Host address
 */
void LibSO::GetHostID(SockAddr4& addr) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    s32 result = sIosDevice.Ioctl(Ioctl_SOGetHostID);
    addr.addr.raw = static_cast<u32>(result);
    addr.port = 0;

    sLastError = SO_SUCCESS;
}

/**
 * @brief Get socket option
 *
 * @param socket Socket descriptor
 * @param level Option level
 * @param opt Option name
 * @param[out] val Option value
 * @param len Buffer size
 * @return IOS error code
 */
SOResult LibSO::GetSockOpt(SOSocket socket, SOSockOptLevel level, SOSockOpt opt,
                           void* val, u32 len) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    K_ASSERT_EX(false, "Not implemented");
    sLastError = SO_SUCCESS;
    return SO_SUCCESS;
}

struct SOSetSockOptArgs {
    /* 0x00 */ s32 fd;
    /* 0x04 */ s32 level;
    /* 0x08 */ s32 opt;
    /* 0x0C */ const void* val;
    /* 0x10 */ u32 len;
};
/**
 * @brief Set socket option
 *
 * @param socket Socket descriptor
 * @param level Option level
 * @param opt Option name
 * @param val Option value
 * @param len Buffer size
 * @return IOS error code
 */
SOResult LibSO::SetSockOpt(SOSocket socket, SOSockOptLevel level, SOSockOpt opt,
                           const void* val, u32 len) {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");
    K_ASSERT(socket >= 0);
    K_ASSERT(val != NULL);
    K_ASSERT(len > 0);

    IosObject<SOSetSockOptArgs> args;
    args->fd = socket;
    args->level = level;
    args->opt = opt;
    args->val = val;
    args->len = len;

    s32 result = sIosDevice.Ioctl(Ioctl_SOSetSockOpt, &args, NULL);
    sLastError = static_cast<SOResult>(result);

    return sLastError;
}

/**
 * @brief Wait until the local IP address has been assigned
 */
void LibSO::WaitForDHCP() {
    K_ASSERT_EX(sIosDevice.IsOpen(), "Please call LibSO::Initialize");

    SockAddr4 addr;
    GetHostID(addr);

    while (addr.addr.raw == 0) {
        OSSleepTicks(OS_MSEC_TO_TICKS(10));
        GetHostID(addr);
    }

    sLastError = SO_SUCCESS;
}

} // namespace kiwi
