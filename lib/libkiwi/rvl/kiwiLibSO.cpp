#include <cstdio>
#include <libkiwi.h>
#include <revolution/IPC.h>

namespace kiwi {

/**
 * IOS socket control codes
 */
enum {
    Ioctl_Accept = 1,
    Ioctl_Bind = 2,
    Ioctl_Close = 3,
    Ioctl_Connect = 4,
    Ioctl_Fcntl = 5,
    Ioctl_GetPeerName = 7,
    Ioctl_GetSocketName = 7,
    Ioctl_Listen = 10,
    Ioctl_Poll = 11,
    Ioctl_RecvFrom = 12,
    Ioctl_SendTo = 13,
    Ioctl_Shutdown = 14,
    Ioctl_Create = 15,
    Ioctl_GetHostID = 16,
};

s32 LibSO::sDeviceHandle = -1;
SOResult LibSO::sLastError = SO_SUCCESS;

/**
 * Accesses IOS IP device for socket operation
 *
 * @note Please call this before other LibSO functions
 */
void LibSO::Initialize() {
    if (sDeviceHandle >= 0) {
        return;
    }

    sDeviceHandle = IOS_Open("/dev/net/ip/top", IPC_OPEN_NONE);
    K_ASSERT_EX(sDeviceHandle >= 0, "Could not access IOS IP device");
}

/**
 * Determine the most recent error code
 */
SOResult LibSO::GetLastError() {
    return sLastError;
}

/**
 * Creates a socket
 *
 * @param family Protocol family
 * @param type Socket type
 * @return New socket descriptor, or IOS error code
 */
s32 LibSO::Socket(SOProtoFamily family, SOSockType type) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(family == SO_PF_INET || family == SO_PF_INET6,
                "Invalid protocol family (%d)", family);

    K_ASSERT_EX(type == SO_SOCK_STREAM || type == SO_SOCK_DGRAM,
                "Invalid socket type (%d)", type);

    struct Args {
        s32 family;   // at 0x0
        s32 type;     // at 0x4
        s32 protocol; // at 0x8
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->family = family;
    args->type = type;

    // IOS must auto-detect protocol
    args->protocol = SO_IPPROTO_IP;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Create, args, sizeof(Args), NULL, 0);

    if (result < 0) {
        sLastError = static_cast<SOResult>(result);
    }

    delete args;
    return result;
}

/**
 * Closes a socket descriptor
 *
 * @param socket Socket descriptor
 * @return IOS error code
 */
SOResult LibSO::Close(SOSocket socket) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    struct Args {
        s32 fd; // at 0x0
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Close, args, sizeof(Args), NULL, 0);

    delete args;

    sLastError = static_cast<SOResult>(result);
    return sLastError;
}

/**
 * Listens for socket connections
 *
 * @param socket Socket descriptor
 * @param backlog Maximum pending connections (default 5)
 * @return IOS error code
 */
SOResult LibSO::Listen(SOSocket socket, s32 backlog) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    struct Args {
        s32 fd;      // at 0x0
        s32 backlog; // at 0x4
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->backlog = backlog;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Listen, args, sizeof(Args), NULL, 0);

    delete args;

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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    struct Args {
        s32 fd; // at 0x0
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;

    // Must be in MEM2, and must be 32-byte aligned
    SockAddr* out = new (32, EMemory_MEM2) SockAddr();
    K_ASSERT(out != NULL);

    // Address length is specified by input
    out->len = addr.len;

    // Result >= 0 == peer descriptor
    s32 result = IOS_Ioctl(sDeviceHandle, Ioctl_Accept, args, sizeof(Args), out,
                           out->len);

    if (result >= 0) {
        std::memcpy(&addr, out, out->len);
    } else {
        sLastError = static_cast<SOResult>(result);
    }

    delete args;
    delete out;
    return result;
}

/**
 * Binds a name to a socket
 *
 * @param socket Socket descriptor
 * @param addr[in,out] Local address (zero for random port)
 * @return IOS error code
 */
SOResult LibSO::Bind(SOSocket socket, SockAddr& addr) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    // IOS doesn't allow specifying zero for a random port.
    // We must emulate this behavior at the user level
    if (addr.port == 0) {
        addr.port = Random().NextU32(49152, 65535);
    }

    struct Args {
        s32 fd;        // at 0x0
        BOOL hasDest;  // at 0x4
        SockAddr dest; // at 0x8
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->hasDest = TRUE;
    args->dest = addr;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Bind, args, sizeof(Args), NULL, 0);

    delete args;

    sLastError = static_cast<SOResult>(result);
    return sLastError;
}

/**
 * Connects a socket
 *
 * @param socket Socket descriptor
 * @param addr Remote address
 * @return IOS error code
 */
SOResult LibSO::Connect(SOSocket socket, const SockAddr& addr) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    struct Args {
        s32 fd;        // at 0x0
        BOOL hasDest;  // at 0x4
        SockAddr dest; // at 0x8
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->hasDest = TRUE;
    args->dest = addr;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Connect, args, sizeof(Args), NULL, 0);

    delete args;

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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    struct Args {
        s32 fd; // at 0x0
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;

    // Must be in MEM2, and must be 32-byte aligned
    SockAddr* self = new (32, EMemory_MEM2) SockAddr();
    K_ASSERT(self != NULL);

    self->len = addr.len;

    s32 result = IOS_Ioctl(sDeviceHandle, Ioctl_GetSocketName, args,
                           sizeof(Args), self, self->len);

    if (result >= 0) {
        std::memcpy(&addr, self, self->len);
    }

    delete args;
    delete self;

    sLastError = static_cast<SOResult>(result);
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    struct Args {
        s32 fd; // at 0x0
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;

    // Must be in MEM2, and must be 32-byte aligned
    SockAddr* peer = new (32, EMemory_MEM2) SockAddr();
    K_ASSERT(peer != NULL);

    peer->len = addr.len;

    s32 result = IOS_Ioctl(sDeviceHandle, Ioctl_GetPeerName, args, sizeof(Args),
                           peer, peer->len);

    if (result >= 0) {
        std::memcpy(&addr, peer, peer->len);
    }

    delete args;

    sLastError = static_cast<SOResult>(result);
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    K_ASSERT_EX(addr.len == sizeof(SockAddr4) || addr.len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr.len);

    return SendImpl(socket, src, len, flags, &addr);
}

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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
    K_ASSERT(dst != NULL);

    K_ASSERT_EX(addr == NULL || addr->len == sizeof(SockAddr4) ||
                    addr->len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr->len);

    enum {
        V_IARGS, // I/O vector for ioctl args
        V_OBUFF, // I/O vector for user buffer
        V_OADDR, // I/O vector for source address

        V_MAX, // Total I/O vector count

        V_NUM_IN = 1, // Total input I/O vector count
        V_NUM_OUT = 2 // Total output I/O vector count
    };

    // Must be in MEM2, and must be 32-byte aligned
    IPCIOVector* vectors = new (32, EMemory_MEM2) IPCIOVector[V_MAX];
    K_ASSERT(vectors != NULL);

    // Setup args for ioctl
    struct Args {
        s32 fd;    // at 0x0
        u32 flags; // at 0x4
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->flags = flags;

    // Must be in MEM2, and must be 32-byte aligned
    SockAddr* from = new (32, EMemory_MEM2) SockAddr();
    K_ASSERT(from != NULL);

    // Input vector 1: Ioctl args
    vectors[V_IARGS].base = args;
    vectors[V_IARGS].length = sizeof(Args);

    // Output vector 1: Destination buffer
    vectors[V_OBUFF].base = dst;
    vectors[V_OBUFF].length = len;

    // Output vector 2: Source address
    if (addr != NULL) {
        *from = *addr;
        vectors[V_OADDR].base = from;
        vectors[V_OADDR].length = from->len;
    } else {
        vectors[V_OADDR].base = NULL;
        vectors[V_OADDR].length = 0;
    }

    s32 result =
        IOS_Ioctlv(sDeviceHandle, Ioctl_RecvFrom, V_NUM_IN, V_NUM_OUT, vectors);

    if (result < 0) {
        sLastError = static_cast<SOResult>(result);
    }

    delete[] vectors;
    delete args;
    delete from;
    return result;
}

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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
    K_ASSERT(src != NULL);

    K_ASSERT_EX(addr == NULL || addr->len == sizeof(SockAddr4) ||
                    addr->len == sizeof(SockAddr6),
                "Invalid address length (%d)", addr->len);

    enum {
        V_IBUFF, // I/O vector for user buffer
        V_IARGS, // I/O vector for ioctl args

        V_MAX, // Total I/O vector count

        V_NUM_IN = 2, // Total input I/O vector count
    };

    // Must be in MEM2, and must be 32-byte aligned
    IPCIOVector* vectors = new (32, EMemory_MEM2) IPCIOVector[V_MAX];
    K_ASSERT(vectors != NULL);

    struct Args {
        s32 fd;        // at 0x0
        u32 flags;     // at 0x4
        BOOL hasDest;  // at 0x8
        SockAddr dest; // at 0xC
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->flags = flags;

    // Input vector 1: Source buffer
    vectors[V_IBUFF].base = const_cast<void*>(src);
    vectors[V_IBUFF].length = len;

    // Input vector 2: Ioctl args
    vectors[V_IARGS].base = &args;
    vectors[V_IARGS].length = sizeof(args);

    // Copy in destination address
    if (addr != NULL) {
        args->hasDest = TRUE;
        args->dest = *addr;
    } else {
        args->hasDest = FALSE;
    }

    // Request send
    s32 result = IOS_Ioctlv(sDeviceHandle, Ioctl_SendTo, V_NUM_IN, 0, vectors);

    if (result < 0) {
        sLastError = static_cast<SOResult>(result);
    }

    delete[] vectors;
    delete args;
    return result;
}

/**
 * Controls socket file descriptor
 *
 * @param socket Socket descriptor
 * @param cmd Command
 * @param ... Command argument
 * @return Command return value, or IOS error code
 */
s32 LibSO::Fcntl(SOSocket socket, SOFcntlCmd cmd, ...) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    std::va_list list;
    va_start(list, cmd);
    void* arg = va_arg(list, void*);
    va_end(list);

    struct Args {
        s32 fd;    // at 0x0
        s32 cmd;   // at 0x4
        void* arg; // at 0x8
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->cmd = cmd;
    args->arg = arg;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Fcntl, args, sizeof(Args), NULL, 0);

    if (result < 0) {
        sLastError = static_cast<SOResult>(result);
    }

    delete args;
    return result;
}

/**
 * Shuts down specified part of socket connection
 *
 * @param socket Socket descriptor
 * @param how How to shutdown connection
 * @return IOS error code
 */
SOResult LibSO::Shutdown(SOSocket socket, SOShutdownType how) {
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");

    struct Args {
        s32 fd;   // at 0x0
        s32 type; // at 0x4
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->fd = socket;
    args->type = how;

    s32 result =
        IOS_Ioctl(sDeviceHandle, Ioctl_Shutdown, args, sizeof(Args), NULL, 0);

    delete args;

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
    K_ASSERT_EX(sDeviceHandle >= 0, "Please call LibSO::Initialize");
    K_ASSERT(fds != NULL);
    K_ASSERT(numfds > 0);

    struct Args {
        s64 msec;
    };

    // Must be in MEM2, and must be 32-byte aligned
    Args* args = new (32, EMemory_MEM2) Args();
    K_ASSERT(args != NULL);

    args->msec = OS_TICKS_TO_MSEC(timeout);

    // Must be in MEM2, and must be 32-byte aligned
    SOPollFD* buffer = new (32) SOPollFD[numfds];
    K_ASSERT(buffer != NULL);

    std::memcpy(buffer, fds, numfds * sizeof(SOPollFD));

    s32 result = IOS_Ioctl(sDeviceHandle, Ioctl_Poll, args, sizeof(Args),
                           buffer, numfds * sizeof(SOPollFD));

    if (result >= 0) {
        std::memcpy(fds, buffer, numfds * sizeof(SOPollFD));
    }

    if (result < 0) {
        sLastError = static_cast<SOResult>(result);
    }

    delete args;
    delete buffer;
    return result;
}

/**
 * Converts a string to an IPv4 address
 *
 * @param str Address string
 * @param[out] addr Address binary
 * @return Success
 */
bool LibSO::INetPtoN(String str, SOInAddr& addr) {
    return std::sscanf(str, "%d.%d.%d.%d", &addr.octets[0], &addr.octets[1],
                       &addr.octets[2],
                       &addr.octets[3]) == LENGTHOF(addr.octets);
}

/**
 * Converts a string to an IPv6 address
 *
 * @param str Address string
 * @param[out] addr Address binary
 * @return Success
 */
bool LibSO::INetPtoN(String str, SOInAddr6& addr) {
    return std::sscanf(str, "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx", &addr.groups[0],
                       &addr.groups[1], &addr.groups[2], &addr.groups[3],
                       &addr.groups[4], &addr.groups[5], &addr.groups[6],
                       &addr.groups[7]) == LENGTHOF(addr.groups);
}

/**
 * Converts an IPv4 address to a string
 *
 * @param addr Address binary
 * @return Address string
 */
String LibSO::INetNtoP(const SOInAddr& addr) {
    return Format("%d.%d.%d.%d", addr.octets[0], addr.octets[1], addr.octets[2],
                  addr.octets[3]);
}

/**
 * Converts an IPv6 address to a string
 *
 * @param addr Address binary
 * @return Address string
 */
String LibSO::INetNtoP(const SOInAddr6& addr) {
    return Format("%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx", addr.groups[0],
                  addr.groups[1], addr.groups[2], addr.groups[3],
                  addr.groups[4], addr.groups[5], addr.groups[6],
                  addr.groups[7]);
}

/**
 * Gets the host machine's IPv4 address
 *
 * @param[out] addr Host address
 */
void LibSO::GetHostID(SOInAddr& addr) {
    s32 result = IOS_Ioctl(sDeviceHandle, Ioctl_GetHostID, NULL, 0, NULL, 0);
    addr.raw = static_cast<u32>(result);
}

} // namespace kiwi
