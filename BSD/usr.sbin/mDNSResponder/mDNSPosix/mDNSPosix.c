/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil; -*-
 *
 * Copyright (c) 2002-2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mDNSEmbeddedAPI.h"           // Defines the interface provided to the client layer above
#include "DNSCommon.h"
#include "mDNSPosix.h"               // Defines the specific types needed to run mDNS on this platform
#include "PlatformCommon.h"
#include "dns_sd.h"
#include "dnssec.h"
#include "nsec.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>                   // platform support for UTC time
#include <ifaddrs.h>

#if USES_NETLINK
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#else // USES_NETLINK
#include <net/route.h>
#include <net/if.h>
#endif // USES_NETLINK

#include "mDNSUNP.h"
#include "GenLinkedList.h"
#include "dnsproxy.h"

// ***************************************************************************
// Structures

// Context record for interface change callback
struct IfChangeRec
{
    int NotifySD;
    mDNS *mDNS;
};
typedef struct IfChangeRec IfChangeRec;

// Note that static data is initialized to zero in (modern) C.
static PosixEventSource *gEventSources;             // linked list of PosixEventSource's
static sigset_t gEventSignalSet;                // Signals which event loop listens for
static sigset_t gEventSignals;                  // Signals which were received while inside loop

static PosixNetworkInterface *gRecentInterfaces;

// ***************************************************************************
// Globals (for debugging)

static int num_registered_interfaces = 0;
static int num_pkts_accepted = 0;
static int num_pkts_rejected = 0;

// ***************************************************************************
// Locals
mDNSlocal void requestReadEvents(PosixEventSource *eventSource,
                                    const char *taskName, mDNSPosixEventCallback callback, void *context);
mDNSlocal mStatus stopReadOrWriteEvents(int fd, mDNSBool freeSource, mDNSBool removeSource, int flags);
mDNSlocal void requestWriteEvents(PosixEventSource *eventSource,
                                     const char *taskName, mDNSPosixEventCallback callback, void *context);
// ***************************************************************************
// Functions

#if MDNS_MALLOC_DEBUGGING
mDNSexport void mDNSPlatformValidateLists(void)
{
    // This should validate gEventSources and any other Posix-specific stuff that gets allocated.
}
#endif

int gMDNSPlatformPosixVerboseLevel = 0;

#define PosixErrorToStatus(errNum) ((errNum) == 0 ? mStatus_NoError : mStatus_UnknownErr)

mDNSlocal void SockAddrTomDNSAddr(const struct sockaddr *const sa, mDNSAddr *ipAddr, mDNSIPPort *ipPort)
{
    switch (sa->sa_family)
    {
    case AF_INET:
    {
        struct sockaddr_in *sin          = (struct sockaddr_in*)sa;
        ipAddr->type                     = mDNSAddrType_IPv4;
        ipAddr->ip.v4.NotAnInteger       = sin->sin_addr.s_addr;
        if (ipPort) ipPort->NotAnInteger = sin->sin_port;
        break;
    }

#if HAVE_IPV6
    case AF_INET6:
    {
        struct sockaddr_in6 *sin6        = (struct sockaddr_in6*)sa;
#ifndef NOT_HAVE_SA_LEN
        assert(sin6->sin6_len == sizeof(*sin6));
#endif
        ipAddr->type                     = mDNSAddrType_IPv6;
        ipAddr->ip.v6                    = *(mDNSv6Addr*)&sin6->sin6_addr;
        if (ipPort) ipPort->NotAnInteger = sin6->sin6_port;
        break;
    }
#endif

    default:
        verbosedebugf("SockAddrTomDNSAddr: Uknown address family %d\n", sa->sa_family);
        ipAddr->type = mDNSAddrType_None;
        if (ipPort) ipPort->NotAnInteger = 0;
        break;
    }
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Send and Receive
#endif

// mDNS core calls this routine when it needs to send a packet.
mDNSexport mStatus mDNSPlatformSendUDP(const mDNS *const m, const void *const msg, const mDNSu8 *const end,
                                       mDNSInterfaceID InterfaceID, UDPSocket *src, const mDNSAddr *dst,
                                       mDNSIPPort dstPort, mDNSBool useBackgroundTrafficClass)
{
    int err = 0;
    struct sockaddr_storage to;
    PosixNetworkInterface * thisIntf = (PosixNetworkInterface *)(InterfaceID);
    int sendingsocket = -1;

    (void)src;  // Will need to use this parameter once we implement mDNSPlatformUDPSocket/mDNSPlatformUDPClose
    (void) useBackgroundTrafficClass;

    assert(m != NULL);
    assert(msg != NULL);
    assert(end != NULL);
    assert((((char *) end) - ((char *) msg)) > 0);

    if (dstPort.NotAnInteger == 0)
    {
        LogMsg("mDNSPlatformSendUDP: Invalid argument -dstPort is set to 0");
        return PosixErrorToStatus(EINVAL);
    }
    if (dst->type == mDNSAddrType_IPv4)
    {
        struct sockaddr_in *sin = (struct sockaddr_in*)&to;
#ifndef NOT_HAVE_SA_LEN
        sin->sin_len            = sizeof(*sin);
#endif
        sin->sin_family         = AF_INET;
        sin->sin_port           = dstPort.NotAnInteger;
        sin->sin_addr.s_addr    = dst->ip.v4.NotAnInteger;
        sendingsocket           = thisIntf ? thisIntf->multicastSocket4 : m->p->unicastSocket4;
    }

#if HAVE_IPV6
    else if (dst->type == mDNSAddrType_IPv6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&to;
        mDNSPlatformMemZero(sin6, sizeof(*sin6));
#ifndef NOT_HAVE_SA_LEN
        sin6->sin6_len            = sizeof(*sin6);
#endif
        sin6->sin6_family         = AF_INET6;
        sin6->sin6_port           = dstPort.NotAnInteger;
        sin6->sin6_addr           = *(struct in6_addr*)&dst->ip.v6;
        sendingsocket             = thisIntf ? thisIntf->multicastSocket6 : m->p->unicastSocket6;
    }
#endif

    if (sendingsocket >= 0)
        err = sendto(sendingsocket, msg, (char*)end - (char*)msg, 0, (struct sockaddr *)&to, GET_SA_LEN(to));

    if      (err > 0) err = 0;
    else if (err < 0)
    {
        static int MessageCount = 0;
        // Don't report EHOSTDOWN (i.e. ARP failure), ENETDOWN, or no route to host for unicast destinations
        if (!mDNSAddressIsAllDNSLinkGroup(dst))
            if (errno == EHOSTDOWN || errno == ENETDOWN || errno == EHOSTUNREACH || errno == ENETUNREACH) return(mStatus_TransientErr);

        if (MessageCount < 1000)
        {
            MessageCount++;
            if (thisIntf)
                LogMsg("mDNSPlatformSendUDP got error %d (%s) sending packet to %#a on interface %#a/%s/%d",
                       errno, strerror(errno), dst, &thisIntf->coreIntf.ip, thisIntf->intfName, thisIntf->index);
            else
                LogMsg("mDNSPlatformSendUDP got error %d (%s) sending packet to %#a", errno, strerror(errno), dst);
        }
    }

    return PosixErrorToStatus(err);
}

mDNSlocal void TCPReadCallback(int fd, void *context)
{
    TCPSocket *sock = context;
    (void)fd;
    
    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
        // implement
    }
    else
    {
        sock->callback(sock, sock->context, mDNSfalse, sock->err);
    }
}

mDNSlocal void tcpConnectCallback(int fd, void *context)
{
    TCPSocket *sock = context;
    mDNSBool c = !sock->connected;
    int result;
    socklen_t len = sizeof result;

    sock->connected = mDNStrue;

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &len) < 0)
    {
        LogInfo("ERROR: TCPConnectCallback - unable to get connect error: socket %d: Error %d (%s)",
               sock->events.fd, result, strerror(result));
        sock->err = mStatus_ConnFailed;
    }
    else
    {
        if (result != 0)
        {
            sock->err = mStatus_ConnFailed;
            if (result == EHOSTUNREACH || result == EADDRNOTAVAIL || result == ENETDOWN)
            {
                LogInfo("ERROR: TCPConnectCallback - connect failed: socket %d: Error %d (%s)",
                        sock->events.fd, result, strerror(result));
            }
            else
            { 
                LogMsg("ERROR: TCPConnectCallback - connect failed: socket %d: Error %d (%s)",
                       sock->events.fd, result, strerror(result));
            }
        }
        else
        {
            // The connection succeeded.
            sock->connected = mDNStrue;
            // Select for read events.
            sock->events.fd = fd;
            requestReadEvents(&sock->events, "mDNSPosix::tcpConnectCallback", TCPReadCallback, sock);
        }
    }

    if (sock->callback)
    {
        sock->callback(sock, sock->context, c, sock->err);
        // Here sock must be assumed to be invalid, in case the callback freed it.
        return;
    }
}

// This routine is called when the main loop detects that data is available on a socket.
mDNSlocal void SocketDataReady(mDNS *const m, PosixNetworkInterface *intf, int skt)
{
    mDNSAddr senderAddr, destAddr;
    mDNSIPPort senderPort;
    ssize_t packetLen;
    DNSMessage packet;
    struct my_in_pktinfo packetInfo;
    struct sockaddr_storage from;
    socklen_t fromLen;
    int flags;
    mDNSu8 ttl;
    mDNSBool reject;
    const mDNSInterfaceID InterfaceID = intf ? intf->coreIntf.InterfaceID : NULL;

    assert(m    != NULL);
    assert(skt  >= 0);

    fromLen = sizeof(from);
    flags   = 0;
    packetLen = recvfrom_flags(skt, &packet, sizeof(packet), &flags, (struct sockaddr *) &from, &fromLen, &packetInfo, &ttl);

    if (packetLen >= 0)
    {
        SockAddrTomDNSAddr((struct sockaddr*)&from, &senderAddr, &senderPort);
        SockAddrTomDNSAddr((struct sockaddr*)&packetInfo.ipi_addr, &destAddr, NULL);

        // If we have broken IP_RECVDSTADDR functionality (so far
        // I've only seen this on OpenBSD) then apply a hack to
        // convince mDNS Core that this isn't a spoof packet.
        // Basically what we do is check to see whether the
        // packet arrived as a multicast and, if so, set its
        // destAddr to the mDNS address.
        //
        // I must admit that I could just be doing something
        // wrong on OpenBSD and hence triggering this problem
        // but I'm at a loss as to how.
        //
        // If this platform doesn't have IP_PKTINFO or IP_RECVDSTADDR, then we have
        // no way to tell the destination address or interface this packet arrived on,
        // so all we can do is just assume it's a multicast

        #if HAVE_BROKEN_RECVDSTADDR || (!defined(IP_PKTINFO) && !defined(IP_RECVDSTADDR))
        if ((destAddr.NotAnInteger == 0) && (flags & MSG_MCAST))
        {
            destAddr.type = senderAddr.type;
            if      (senderAddr.type == mDNSAddrType_IPv4) destAddr.ip.v4 = AllDNSLinkGroup_v4.ip.v4;
            else if (senderAddr.type == mDNSAddrType_IPv6) destAddr.ip.v6 = AllDNSLinkGroup_v6.ip.v6;
        }
        #endif

        // We only accept the packet if the interface on which it came
        // in matches the interface associated with this socket.
        // We do this match by name or by index, depending on which
        // information is available.  recvfrom_flags sets the name
        // to "" if the name isn't available, or the index to -1
        // if the index is available.  This accomodates the various
        // different capabilities of our target platforms.

        reject = mDNSfalse;
        if (!intf)
        {
            // Ignore multicasts accidentally delivered to our unicast receiving socket
            if (mDNSAddrIsDNSMulticast(&destAddr)) packetLen = -1;
        }
        else
        {
            if      (packetInfo.ipi_ifname[0] != 0) reject = (strcmp(packetInfo.ipi_ifname, intf->intfName) != 0);
            else if (packetInfo.ipi_ifindex != -1) reject = (packetInfo.ipi_ifindex != intf->index);

            if (reject)
            {
                verbosedebugf("SocketDataReady ignored a packet from %#a to %#a on interface %s/%d expecting %#a/%s/%d/%d",
                              &senderAddr, &destAddr, packetInfo.ipi_ifname, packetInfo.ipi_ifindex,
                              &intf->coreIntf.ip, intf->intfName, intf->index, skt);
                packetLen = -1;
                num_pkts_rejected++;
                if (num_pkts_rejected > (num_pkts_accepted + 1) * (num_registered_interfaces + 1) * 2)
                {
                    fprintf(stderr,
                            "*** WARNING: Received %d packets; Accepted %d packets; Rejected %d packets because of interface mismatch\n",
                            num_pkts_accepted + num_pkts_rejected, num_pkts_accepted, num_pkts_rejected);
                    num_pkts_accepted = 0;
                    num_pkts_rejected = 0;
                }
            }
            else
            {
                verbosedebugf("SocketDataReady got a packet from %#a to %#a on interface %#a/%s/%d/%d",
                              &senderAddr, &destAddr, &intf->coreIntf.ip, intf->intfName, intf->index, skt);
                num_pkts_accepted++;
            }
        }
    }

    if (packetLen >= 0)
        mDNSCoreReceive(m, &packet, (mDNSu8 *)&packet + packetLen,
                        &senderAddr, senderPort, &destAddr, MulticastDNSPort, InterfaceID);
}

mDNSexport TCPSocket *mDNSPlatformTCPSocket(TCPSocketFlags flags, mDNSAddr_Type addrType, mDNSIPPort * port,
                                            domainname *hostname, mDNSBool useBackgroundTrafficClass)
{
    TCPSocket *sock;
    int len = sizeof (TCPSocket);
    
    (void)useBackgroundTrafficClass;

    if (hostname)
    {
        len += sizeof (domainname);
    }
    sock = malloc(len); 

    if (sock == NULL)
    {
        LogMsg("mDNSPlatformTCPSocket: no memory for socket");
        return NULL;
    }
    memset(sock, 0, sizeof *sock);
    
    if (hostname)
    {
        sock->hostname = (domainname *)(sock + 1);
        LogMsg("mDNSPlatformTCPSocket: hostname %##s", hostname->c);
        AssignDomainName(sock->hostname, hostname);
    }
    
    sock->events.fd = -1;
    if (!mDNSPosixTCPSocketSetup(&sock->events.fd, addrType, port, &sock->port))
    {
      if (sock->events.fd != -1) close(sock->events.fd);
      free(sock);
      return mDNSNULL;
    }

    // Set up the other fields in the structure.
    sock->flags = flags;
    sock->err = mStatus_NoError;
    sock->setup = mDNSfalse;
    sock->connected = mDNSfalse;
    return sock;
}

mDNSexport mStatus mDNSPlatformTCPSocketSetCallback(TCPSocket *sock, TCPConnectionCallback callback, void *context)
{
    sock->callback = callback;
    sock->context = context;
    return mStatus_NoError;
}

mDNSexport TCPSocket *mDNSPlatformTCPAccept(TCPSocketFlags flags, int fd)
{
    TCPSocket *sock;

    // XXX Add!
    if (flags & kTCPSocketFlags_UseTLS)
    {
    return mDNSNULL; // not supported yet.
    }

    sock = (TCPSocket *) mDNSPlatformMemAllocateClear(sizeof *sock);
    if (!sock)
    {
        return mDNSNULL;
    }

    sock->events.fd = fd;
    sock->flags = flags;
    sock->connected = mDNStrue;
    return sock;
}


mDNSlocal void tcpListenCallback(int fd, void *context)
{
    TCPListener *listener = context;
    TCPSocket *sock;
    
    sock = mDNSPosixDoTCPListenCallback(fd, listener->addressType, listener->socketFlags,
                                 listener->callback, listener->context);
    if (sock != NULL)
    {
        requestReadEvents(&sock->events, "mDNSPosix::tcpListenCallback", TCPReadCallback, sock);
    }
}

mDNSexport TCPListener *mDNSPlatformTCPListen(mDNSAddr_Type addrType, mDNSIPPort *port, mDNSAddr *addr,
                                              TCPSocketFlags socketFlags, mDNSBool reuseAddr, int queueLength,
                                              TCPAcceptedCallback callback, void *context)
{
    TCPListener *ret;
    int fd = -1;

    if (!mDNSPosixTCPListen(&fd, addrType, port, addr, reuseAddr, queueLength))
    {
        if (fd != -1)
        {
            close(fd);
        }
        return mDNSNULL;
    }
    
    // Allocate a listener structure
    ret = (TCPListener *) mDNSPlatformMemAllocateClear(sizeof *ret);
    if (ret == NULL)
    {
        LogMsg("mDNSPlatformTCPListen: no memory for TCPListener struct.");
        close(fd);
        return mDNSNULL;
    }
    ret->events.fd = fd;
    ret->callback = callback;
    ret->context = context;
    ret->addressType = addrType;
    ret->socketFlags = socketFlags;

    // When we get a connection, mDNSPosixListenCallback will be called, and it will invoke the
    // callback we were passed.
    requestReadEvents(&ret->events, "tcpListenCallback", tcpListenCallback, ret);
    return ret;
}

mDNSexport int mDNSPlatformTCPGetFD(TCPSocket *sock)
{
    return sock->events.fd;
}

mDNSexport mStatus mDNSPlatformTCPConnect(TCPSocket *sock, const mDNSAddr *dst, mDNSOpaque16 dstport,
                                          mDNSInterfaceID InterfaceID, TCPConnectionCallback callback, void *context)
{
    int result;
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } addr;
    socklen_t len;

    sock->callback = callback;
    sock->context = context;
    sock->setup = mDNSfalse;
    sock->connected = mDNSfalse;
    sock->err = mStatus_NoError;

    result = fcntl(sock->events.fd, F_GETFL, 0);
    if (result < 0)
    {
        LogMsg("mDNSPlatformTCPConnect: F_GETFL failed: %s", strerror(errno));
        return mStatus_UnknownErr;
    }

    result = fcntl(sock->events.fd, F_SETFL, result | O_NONBLOCK);
    if (result < 0)
    {
        LogMsg("mDNSPlatformTCPConnect: F_SETFL failed: %s", strerror(errno));
        return mStatus_UnknownErr;
    }

    // If we've been asked to bind to a single interface, do it.  See comment in mDNSMacOSX.c for more info.
    if (InterfaceID)
    {
        PosixNetworkInterface *iface = (PosixNetworkInterface *)InterfaceID;
#if defined(SO_BINDTODEVICE)
        result = setsockopt(sock->events.fd,
                            SOL_SOCKET, SO_BINDTODEVICE, iface->intfName, strlen(iface->intfName));
        if (result < 0)
        {
            LogMsg("mDNSPlatformTCPConnect: SO_BINDTODEVICE failed on %s: %s", iface->intfName, strerror(errno));
            return mStatus_BadParamErr;
        }
#else
        if (dst->type == mDNSAddrType_IPv4)
        {
#if defined(IP_BOUND_IF)
            result = setsockopt(sock->events.fd, IPPROTO_IP, IP_BOUND_IF, &iface->index, sizeof iface->index);
            if (result < 0)
            {
                LogMsg("mDNSPlatformTCPConnect: IP_BOUND_IF failed on %s (%d): %s",
                       iface->intfName, iface->index, strerror(errno));
                return mStatus_BadParamErr;
            }
#else
            (void)iface;
#endif // IP_BOUND_IF
        }
        else
        { // IPv6
#if defined(IPV6_BOUND_IF)
            result = setsockopt(sock->events.fd, IPPROTO_IPV6, IPV6_BOUND_IF, &iface->index, sizeof iface->index);
            if (result < 0)
            {
                LogMsg("mDNSPlatformTCPConnect: IP_BOUND_IF failed on %s (%d): %s",
                       iface->intfName, iface->index, strerror(errno));
                return mStatus_BadParamErr;
            }
#else
            (void)iface;
#endif // IPV6_BOUND_IF
        }           
#endif // SO_BINDTODEVICE
    }

    memset(&addr, 0, sizeof addr);
    if (dst->type == mDNSAddrType_IPv4)
    {
        addr.sa.sa_family = AF_INET;
        addr.sin.sin_port = dstport.NotAnInteger;
        len = sizeof (struct sockaddr_in);
        addr.sin.sin_addr.s_addr = dst->ip.v4.NotAnInteger;
    }
    else
    {
        addr.sa.sa_family = AF_INET6;
        len = sizeof (struct sockaddr_in6);
        addr.sin6.sin6_port = dstport.NotAnInteger;
        memcpy(&addr.sin6.sin6_addr.s6_addr, &dst->ip.v6, sizeof addr.sin6.sin6_addr.s6_addr);
    }
#ifndef NOT_HAVE_SA_LEN
    addr.sa.sa_len = len;
#endif

    result = connect(sock->events.fd, (struct sockaddr *)&addr, len);
    if (result < 0)
    {
        if (errno == EINPROGRESS)
        {
            requestWriteEvents(&sock->events, "mDNSPlatformConnect", tcpConnectCallback, sock);
            return mStatus_ConnPending;
        }
        if (errno == EHOSTUNREACH || errno == EADDRNOTAVAIL || errno == ENETDOWN)
        {
            LogInfo("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s)",
                    sock->events.fd, errno, strerror(errno));
        }
        else
        { 
            LogMsg("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s) length %d",
                   sock->events.fd, errno, strerror(errno), len);
        }
        return mStatus_ConnFailed;
    }       

    LogMsg("NOTE: mDNSPlatformTCPConnect completed synchronously");
    return mStatus_NoError;
}

mDNSexport void mDNSPlatformTCPCloseConnection(TCPSocket *sock)
{
    if (sock)
    { // can sock really be NULL when this is called?
        shutdown(sock->events.fd, SHUT_RDWR);
        stopReadOrWriteEvents(sock->events.fd, mDNSfalse, mDNStrue,
                              PosixEventFlag_Read | PosixEventFlag_Write);
        close(sock->events.fd);
        free(sock);
    }
}

mDNSexport long mDNSPlatformReadTCP(TCPSocket *sock, void *buf, unsigned long buflen, mDNSBool * closed)
{
    ssize_t nread;
    
    *closed = mDNSfalse;
    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
        // Implement...
        nread = -1;
        *closed = mDNStrue;
    } else {
        nread = mDNSPosixReadTCP(sock->events.fd, buf, buflen, closed);
    }
    return nread;
}

mDNSexport mDNSBool mDNSPlatformTCPWritable(TCPSocket *sock)
{
    fd_set w;
    int nfds = sock->events.fd + 1;
    int count;
    struct timeval tv;

    if (nfds > FD_SETSIZE)
    {
        LogMsg("ERROR: mDNSPlatformTCPWritable called on an fd that won't fit in an fd_set.");
        return mDNStrue; // hope for the best?
    }
    FD_SET(sock->events.fd, &w);
    tv.tv_sec = tv.tv_usec = 0;
    count = select(nfds, NULL, &w, NULL, &tv);
    if (count > 0)
    {
        return mDNStrue;
    }
    return mDNSfalse;
}

mDNSexport long mDNSPlatformWriteTCP(TCPSocket *sock, const char *msg, unsigned long len)
{
    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
        // implement
        return -1;
    }
    else
    {
        return mDNSPosixWriteTCP(sock->events.fd, msg, len);
    }
}

mDNSexport UDPSocket *mDNSPlatformUDPSocket(mDNSIPPort port)
{
    (void)port;         // Unused
    return NULL;
}

mDNSexport void           mDNSPlatformUDPClose(UDPSocket *sock)
{
    (void)sock;         // Unused
}

mDNSexport void mDNSPlatformUpdateProxyList(const mDNSInterfaceID InterfaceID)
{
    (void)InterfaceID;          // Unused
}

mDNSexport void mDNSPlatformSendRawPacket(const void *const msg, const mDNSu8 *const end, mDNSInterfaceID InterfaceID)
{
    (void)msg;          // Unused
    (void)end;          // Unused
    (void)InterfaceID;          // Unused
}

mDNSexport void mDNSPlatformSetLocalAddressCacheEntry(const mDNSAddr *const tpa, const mDNSEthAddr *const tha, mDNSInterfaceID InterfaceID)
{
    (void)tpa;          // Unused
    (void)tha;          // Unused
    (void)InterfaceID;          // Unused
}

mDNSexport mStatus mDNSPlatformTLSSetupCerts(void)
{
    return(mStatus_UnsupportedErr);
}

mDNSexport void mDNSPlatformTLSTearDownCerts(void)
{
}

mDNSexport void mDNSPlatformSetAllowSleep(mDNSBool allowSleep, const char *reason)
{
    (void) allowSleep;
    (void) reason;
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - /etc/hosts support
#endif

mDNSexport void FreeEtcHosts(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)m;  // unused
    (void)rr;
    (void)result;
}


#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** DDNS Config Platform Functions
#endif

mDNSexport mDNSBool mDNSPlatformSetDNSConfig(mDNSBool setservers, mDNSBool setsearch, domainname *const fqdn, DNameListElem **RegDomains,
    DNameListElem **BrowseDomains, mDNSBool ackConfig)
{
    (void) setservers;
    (void) setsearch;
    (void) ackConfig;

    if (fqdn         ) fqdn->c[0]      = 0;
    if (RegDomains   ) *RegDomains     = NULL;
    if (BrowseDomains) *BrowseDomains  = NULL;

    return mDNStrue;
}

mDNSexport mStatus mDNSPlatformGetPrimaryInterface(mDNSAddr * v4, mDNSAddr * v6, mDNSAddr * router)
{
    (void) v4;
    (void) v6;
    (void) router;

    return mStatus_UnsupportedErr;
}

mDNSexport void mDNSPlatformDynDNSHostNameStatusChanged(const domainname *const dname, const mStatus status)
{
    (void) dname;
    (void) status;
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Init and Term
#endif

// This gets the current hostname, truncating it at the first dot if necessary
mDNSlocal void GetUserSpecifiedRFC1034ComputerName(domainlabel *const namelabel)
{
    int len = 0;
    gethostname((char *)(&namelabel->c[1]), MAX_DOMAIN_LABEL);
    while (len < MAX_DOMAIN_LABEL && namelabel->c[len+1] && namelabel->c[len+1] != '.') len++;
    namelabel->c[0] = len;
}

// On OS X this gets the text of the field labelled "Computer Name" in the Sharing Prefs Control Panel
// Other platforms can either get the information from the appropriate place,
// or they can alternatively just require all registering services to provide an explicit name
mDNSlocal void GetUserSpecifiedFriendlyComputerName(domainlabel *const namelabel)
{
    // On Unix we have no better name than the host name, so we just use that.
    GetUserSpecifiedRFC1034ComputerName(namelabel);
}

mDNSexport int ParseDNSServers(mDNS *m, const char *filePath)
{
    char line[256];
    char nameserver[16];
    char keyword[11];
    int numOfServers = 0;
    FILE *fp = fopen(filePath, "r");
    if (fp == NULL) return -1;
    while (fgets(line,sizeof(line),fp))
    {
        struct in_addr ina;
        line[255]='\0';     // just to be safe
        if (sscanf(line,"%10s %15s", keyword, nameserver) != 2) continue;   // it will skip whitespaces
        if (strncasecmp(keyword,"nameserver",10)) continue;
        if (inet_aton(nameserver, (struct in_addr *)&ina) != 0)
        {
            mDNSAddr DNSAddr;
            DNSAddr.type = mDNSAddrType_IPv4;
            DNSAddr.ip.v4.NotAnInteger = ina.s_addr;
            mDNS_AddDNSServer(m, NULL, mDNSInterface_Any, 0, &DNSAddr, UnicastDNSPort, kScopeNone, 0, mDNSfalse, mDNSfalse, mDNSfalse, mDNSfalse, 0, mDNStrue, mDNStrue, mDNSfalse);
            numOfServers++;
        }
    }
    fclose(fp);
    return (numOfServers > 0) ? 0 : -1;
}

// Searches the interface list looking for the named interface.
// Returns a pointer to if it found, or NULL otherwise.
mDNSlocal PosixNetworkInterface *SearchForInterfaceByName(mDNS *const m, const char *intfName)
{
    PosixNetworkInterface *intf;

    assert(m != NULL);
    assert(intfName != NULL);

    intf = (PosixNetworkInterface*)(m->HostInterfaces);
    while ((intf != NULL) && (strcmp(intf->intfName, intfName) != 0))
        intf = (PosixNetworkInterface *)(intf->coreIntf.next);

    return intf;
}

mDNSexport mDNSInterfaceID mDNSPlatformInterfaceIDfromInterfaceIndex(mDNS *const m, mDNSu32 index)
{
    PosixNetworkInterface *intf;

    assert(m != NULL);

    if (index == kDNSServiceInterfaceIndexLocalOnly) return(mDNSInterface_LocalOnly);
    if (index == kDNSServiceInterfaceIndexP2P      ) return(mDNSInterface_P2P);
    if (index == kDNSServiceInterfaceIndexAny      ) return(mDNSInterface_Any);

    intf = (PosixNetworkInterface*)(m->HostInterfaces);
    while ((intf != NULL) && (mDNSu32) intf->index != index)
        intf = (PosixNetworkInterface *)(intf->coreIntf.next);

    return (mDNSInterfaceID) intf;
}

mDNSexport mDNSu32 mDNSPlatformInterfaceIndexfromInterfaceID(mDNS *const m, mDNSInterfaceID id, mDNSBool suppressNetworkChange)
{
    PosixNetworkInterface *intf;
    (void) suppressNetworkChange; // Unused

    assert(m != NULL);

    if (id == mDNSInterface_LocalOnly) return(kDNSServiceInterfaceIndexLocalOnly);
    if (id == mDNSInterface_P2P      ) return(kDNSServiceInterfaceIndexP2P);
    if (id == mDNSInterface_Any      ) return(kDNSServiceInterfaceIndexAny);

    intf = (PosixNetworkInterface*)(m->HostInterfaces);
    while ((intf != NULL) && (mDNSInterfaceID) intf != id)
        intf = (PosixNetworkInterface *)(intf->coreIntf.next);

    if (intf) return intf->index;

    // If we didn't find the interface, check the RecentInterfaces list as well
    intf = gRecentInterfaces;
    while ((intf != NULL) && (mDNSInterfaceID) intf != id)
        intf = (PosixNetworkInterface *)(intf->coreIntf.next);

    return intf ? intf->index : 0;
}

// Frees the specified PosixNetworkInterface structure. The underlying
// interface must have already been deregistered with the mDNS core.
mDNSlocal void FreePosixNetworkInterface(PosixNetworkInterface *intf)
{
    int rv;
    assert(intf != NULL);
    if (intf->intfName != NULL) free((void *)intf->intfName);
    if (intf->multicastSocket4 != -1)
    {
        rv = close(intf->multicastSocket4);
        assert(rv == 0);
    }
#if HAVE_IPV6
    if (intf->multicastSocket6 != -1)
    {
        rv = close(intf->multicastSocket6);
        assert(rv == 0);
    }
#endif

    // Move interface to the RecentInterfaces list for a minute
    intf->LastSeen = mDNSPlatformUTC();
    intf->coreIntf.next = &gRecentInterfaces->coreIntf;
    gRecentInterfaces = intf;
}

// Grab the first interface, deregister it, free it, and repeat until done.
mDNSlocal void ClearInterfaceList(mDNS *const m)
{
    assert(m != NULL);

    while (m->HostInterfaces)
    {
        PosixNetworkInterface *intf = (PosixNetworkInterface*)(m->HostInterfaces);
        mDNS_DeregisterInterface(m, &intf->coreIntf, NormalActivation);
        if (gMDNSPlatformPosixVerboseLevel > 0) fprintf(stderr, "Deregistered interface %s\n", intf->intfName);
        FreePosixNetworkInterface(intf);
    }
    num_registered_interfaces = 0;
    num_pkts_accepted = 0;
    num_pkts_rejected = 0;
}

// Sets up a send/receive socket.
// If mDNSIPPort port is non-zero, then it's a multicast socket on the specified interface
// If mDNSIPPort port is zero, then it's a randomly assigned port number, used for sending unicast queries
mDNSlocal int SetupSocket(struct sockaddr *intfAddr, mDNSIPPort port, int interfaceIndex, int *sktPtr)
{
    int err = 0;
    static const int kOn = 1;
    static const int kIntTwoFiveFive = 255;
    static const unsigned char kByteTwoFiveFive = 255;
    const mDNSBool JoinMulticastGroup = (port.NotAnInteger != 0);

    (void) interfaceIndex;  // This parameter unused on plaforms that don't have IPv6
    assert(intfAddr != NULL);
    assert(sktPtr != NULL);
    assert(*sktPtr == -1);

    // Open the socket...
    if      (intfAddr->sa_family == AF_INET) *sktPtr = socket(PF_INET,  SOCK_DGRAM, IPPROTO_UDP);
#if HAVE_IPV6
    else if (intfAddr->sa_family == AF_INET6) *sktPtr = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
#endif
    else return EINVAL;

    if (*sktPtr < 0) { err = errno; perror((intfAddr->sa_family == AF_INET) ? "socket AF_INET" : "socket AF_INET6"); }

    // ... with a shared UDP port, if it's for multicast receiving
    if (err == 0 && port.NotAnInteger)
    {
        // <rdar://problem/20946253> Suggestions from Jonny Törnbom at Axis Communications
        // We test for SO_REUSEADDR first, as suggested by Jonny Törnbom from Axis Communications
        // Linux kernel versions 3.9 introduces support for socket option
        // SO_REUSEPORT, however this is not implemented the same as on *BSD
        // systems. Linux version implements a "port hijacking" prevention
        // mechanism, limiting processes wanting to bind to an already existing
        // addr:port to have the same effective UID as the first who bound it. What
        // this meant for us was that the daemon ran as one user and when for
        // instance mDNSClientPosix was executed by another user, it wasn't allowed
        // to bind to the socket. Our suggestion was to switch the order in which
        // SO_REUSEPORT and SO_REUSEADDR was tested so that SO_REUSEADDR stays on
        // top and SO_REUSEPORT to be used only if SO_REUSEADDR doesn't exist.
        #if defined(SO_REUSEADDR) && !defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
        err = setsockopt(*sktPtr, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
        #elif defined(SO_REUSEPORT)
        err = setsockopt(*sktPtr, SOL_SOCKET, SO_REUSEPORT, &kOn, sizeof(kOn));
        #else
            #error This platform has no way to avoid address busy errors on multicast.
        #endif
        if (err < 0) { err = errno; perror("setsockopt - SO_REUSExxxx"); }

#if TARGET_OS_MAC
        // Enable inbound packets on IFEF_AWDL interface.
        // Only done for multicast sockets, since we don't expect unicast socket operations
        // on the IFEF_AWDL interface. Operation is a no-op for other interface types.
        #ifndef SO_RECV_ANYIF
        #define SO_RECV_ANYIF   0x1104      /* unrestricted inbound processing */
        #endif
        if (setsockopt(*sktPtr, SOL_SOCKET, SO_RECV_ANYIF, &kOn, sizeof(kOn)) < 0) perror("setsockopt - SO_RECV_ANYIF");
#endif
    }

    // We want to receive destination addresses and interface identifiers.
    if (intfAddr->sa_family == AF_INET)
    {
        struct ip_mreq imr;
        struct sockaddr_in bindAddr;
        if (err == 0)
        {
            #if defined(IP_PKTINFO)                                 // Linux
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_PKTINFO, &kOn, sizeof(kOn));
            if (err < 0) { err = errno; perror("setsockopt - IP_PKTINFO"); }
            #elif defined(IP_RECVDSTADDR) || defined(IP_RECVIF)     // BSD and Solaris
                #if defined(IP_RECVDSTADDR)
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_RECVDSTADDR, &kOn, sizeof(kOn));
            if (err < 0) { err = errno; perror("setsockopt - IP_RECVDSTADDR"); }
                #endif
                #if defined(IP_RECVIF)
            if (err == 0)
            {
                err = setsockopt(*sktPtr, IPPROTO_IP, IP_RECVIF, &kOn, sizeof(kOn));
                if (err < 0) { err = errno; perror("setsockopt - IP_RECVIF"); }
            }
                #endif
            #else
                #warning This platform has no way to get the destination interface information -- will only work for single-homed hosts
            #endif
        }
    #if defined(IP_RECVTTL)                                 // Linux
        if (err == 0)
        {
            setsockopt(*sktPtr, IPPROTO_IP, IP_RECVTTL, &kOn, sizeof(kOn));
            // We no longer depend on being able to get the received TTL, so don't worry if the option fails
        }
    #endif

        // Add multicast group membership on this interface
        if (err == 0 && JoinMulticastGroup)
        {
            imr.imr_multiaddr.s_addr = AllDNSLinkGroup_v4.ip.v4.NotAnInteger;
            imr.imr_interface        = ((struct sockaddr_in*)intfAddr)->sin_addr;
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr));
            if (err < 0) { err = errno; perror("setsockopt - IP_ADD_MEMBERSHIP"); }
        }

        // Specify outgoing interface too
        if (err == 0 && JoinMulticastGroup)
        {
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_MULTICAST_IF, &((struct sockaddr_in*)intfAddr)->sin_addr, sizeof(struct in_addr));
            if (err < 0) { err = errno; perror("setsockopt - IP_MULTICAST_IF"); }
        }

        // Per the mDNS spec, send unicast packets with TTL 255
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_TTL, &kIntTwoFiveFive, sizeof(kIntTwoFiveFive));
            if (err < 0) { err = errno; perror("setsockopt - IP_TTL"); }
        }

        // and multicast packets with TTL 255 too
        // There's some debate as to whether IP_MULTICAST_TTL is an int or a byte so we just try both.
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IP, IP_MULTICAST_TTL, &kByteTwoFiveFive, sizeof(kByteTwoFiveFive));
            if (err < 0 && errno == EINVAL)
                err = setsockopt(*sktPtr, IPPROTO_IP, IP_MULTICAST_TTL, &kIntTwoFiveFive, sizeof(kIntTwoFiveFive));
            if (err < 0) { err = errno; perror("setsockopt - IP_MULTICAST_TTL"); }
        }

        // And start listening for packets
        if (err == 0)
        {
            bindAddr.sin_family      = AF_INET;
            bindAddr.sin_port        = port.NotAnInteger;
            bindAddr.sin_addr.s_addr = INADDR_ANY; // Want to receive multicasts AND unicasts on this socket
            err = bind(*sktPtr, (struct sockaddr *) &bindAddr, sizeof(bindAddr));
            if (err < 0) { err = errno; perror("bind"); fflush(stderr); }
        }
    }     // endif (intfAddr->sa_family == AF_INET)

#if HAVE_IPV6
    else if (intfAddr->sa_family == AF_INET6)
    {
        struct ipv6_mreq imr6;
        struct sockaddr_in6 bindAddr6;
    #if defined(IPV6_PKTINFO)
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_2292_PKTINFO, &kOn, sizeof(kOn));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_PKTINFO"); }
        }
    #else
        #warning This platform has no way to get the destination interface information for IPv6 -- will only work for single-homed hosts
    #endif
    #if defined(IPV6_HOPLIMIT)
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_2292_HOPLIMIT, &kOn, sizeof(kOn));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_HOPLIMIT"); }
        }
    #endif

        // Add multicast group membership on this interface
        if (err == 0 && JoinMulticastGroup)
        {
            imr6.ipv6mr_multiaddr       = *(const struct in6_addr*)&AllDNSLinkGroup_v6.ip.v6;
            imr6.ipv6mr_interface       = interfaceIndex;
            //LogMsg("Joining %.16a on %d", &imr6.ipv6mr_multiaddr, imr6.ipv6mr_interface);
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_JOIN_GROUP, &imr6, sizeof(imr6));
            if (err < 0)
            {
                err = errno;
                verbosedebugf("IPV6_JOIN_GROUP %.16a on %d failed.\n", &imr6.ipv6mr_multiaddr, imr6.ipv6mr_interface);
                perror("setsockopt - IPV6_JOIN_GROUP");
            }
        }

        // Specify outgoing interface too
        if (err == 0 && JoinMulticastGroup)
        {
            u_int multicast_if = interfaceIndex;
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_MULTICAST_IF, &multicast_if, sizeof(multicast_if));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_MULTICAST_IF"); }
        }

        // We want to receive only IPv6 packets on this socket.
        // Without this option, we may get IPv4 addresses as mapped addresses.
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_V6ONLY, &kOn, sizeof(kOn));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_V6ONLY"); }
        }

        // Per the mDNS spec, send unicast packets with TTL 255
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &kIntTwoFiveFive, sizeof(kIntTwoFiveFive));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_UNICAST_HOPS"); }
        }

        // and multicast packets with TTL 255 too
        // There's some debate as to whether IPV6_MULTICAST_HOPS is an int or a byte so we just try both.
        if (err == 0)
        {
            err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &kByteTwoFiveFive, sizeof(kByteTwoFiveFive));
            if (err < 0 && errno == EINVAL)
                err = setsockopt(*sktPtr, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &kIntTwoFiveFive, sizeof(kIntTwoFiveFive));
            if (err < 0) { err = errno; perror("setsockopt - IPV6_MULTICAST_HOPS"); }
        }

        // And start listening for packets
        if (err == 0)
        {
            mDNSPlatformMemZero(&bindAddr6, sizeof(bindAddr6));
#ifndef NOT_HAVE_SA_LEN
            bindAddr6.sin6_len         = sizeof(bindAddr6);
#endif
            bindAddr6.sin6_family      = AF_INET6;
            bindAddr6.sin6_port        = port.NotAnInteger;
            bindAddr6.sin6_flowinfo    = 0;
            bindAddr6.sin6_addr        = in6addr_any; // Want to receive multicasts AND unicasts on this socket
            bindAddr6.sin6_scope_id    = 0;
            err = bind(*sktPtr, (struct sockaddr *) &bindAddr6, sizeof(bindAddr6));
            if (err < 0) { err = errno; perror("bind"); fflush(stderr); }
        }
    }     // endif (intfAddr->sa_family == AF_INET6)
#endif

    // Set the socket to non-blocking.
    if (err == 0)
    {
        err = fcntl(*sktPtr, F_GETFL, 0);
        if (err < 0) err = errno;
        else
        {
            err = fcntl(*sktPtr, F_SETFL, err | O_NONBLOCK);
            if (err < 0) err = errno;
        }
    }

    // Clean up
    if (err != 0 && *sktPtr != -1)
    {
        int rv;
        rv = close(*sktPtr);
        assert(rv == 0);
        *sktPtr = -1;
    }
    assert((err == 0) == (*sktPtr != -1));
    return err;
}

// Creates a PosixNetworkInterface for the interface whose IP address is
// intfAddr and whose name is intfName and registers it with mDNS core.
mDNSlocal int SetupOneInterface(mDNS *const m, struct sockaddr *intfAddr, struct sockaddr *intfMask, const char *intfName, int intfIndex)
{
    int err = 0;
    PosixNetworkInterface *intf;
    PosixNetworkInterface *alias = NULL;

    assert(m != NULL);
    assert(intfAddr != NULL);
    assert(intfName != NULL);
    assert(intfMask != NULL);

    // Allocate the interface structure itself.
    intf = (PosixNetworkInterface*)calloc(1, sizeof(*intf));
    if (intf == NULL) { assert(0); err = ENOMEM; }

    // And make a copy of the intfName.
    if (err == 0)
    {
#ifdef LINUX
        char *s;
        int len;
        s = strchr(intfName, ':');
        if (s != NULL)
        {
            len = (s - intfName) + 1;
        }
        else
        {
            len = strlen(intfName) + 1;
        }
        intf->intfName = malloc(len);
        if (intf->intfName == NULL) { assert(0); err = ENOMEM; }
        memcpy(intf->intfName, intfName, len - 1);
        intfName[len - 1] = 0;
#else
        intf->intfName = strdup(intfName);
        if (intf->intfName == NULL) { assert(0); err = ENOMEM; }
#endif
    }

    if (err == 0)
    {
        // Set up the fields required by the mDNS core.
        SockAddrTomDNSAddr(intfAddr, &intf->coreIntf.ip, NULL);
        SockAddrTomDNSAddr(intfMask, &intf->coreIntf.mask, NULL);

        //LogMsg("SetupOneInterface: %#a %#a",  &intf->coreIntf.ip,  &intf->coreIntf.mask);
        strncpy(intf->coreIntf.ifname, intfName, sizeof(intf->coreIntf.ifname));
        intf->coreIntf.ifname[sizeof(intf->coreIntf.ifname)-1] = 0;

        intf->coreIntf.Advertise = m->AdvertiseLocalAddresses;
        intf->coreIntf.McastTxRx = mDNStrue;

        // Set up the extra fields in PosixNetworkInterface.
        assert(intf->intfName != NULL);         // intf->intfName already set up above
        intf->index                = intfIndex;
        intf->multicastSocket4     = -1;
#if HAVE_IPV6
        intf->multicastSocket6     = -1;
#endif
        alias                      = SearchForInterfaceByName(m, intf->intfName);
        if (alias == NULL) alias   = intf;
        intf->coreIntf.InterfaceID = (mDNSInterfaceID)alias;

        if (alias != intf)
            debugf("SetupOneInterface: %s %#a is an alias of %#a", intfName, &intf->coreIntf.ip, &alias->coreIntf.ip);
    }

    // Set up the multicast socket
    if (err == 0)
    {
        if (alias->multicastSocket4 == -1 && intfAddr->sa_family == AF_INET)
            err = SetupSocket(intfAddr, MulticastDNSPort, intf->index, &alias->multicastSocket4);
#if HAVE_IPV6
        else if (alias->multicastSocket6 == -1 && intfAddr->sa_family == AF_INET6)
            err = SetupSocket(intfAddr, MulticastDNSPort, intf->index, &alias->multicastSocket6);
#endif
    }

    // If interface is a direct link, address record will be marked as kDNSRecordTypeKnownUnique
    // and skip the probe phase of the probe/announce packet sequence.
    intf->coreIntf.DirectLink = mDNSfalse;
#ifdef DIRECTLINK_INTERFACE_NAME
    if (strcmp(intfName, STRINGIFY(DIRECTLINK_INTERFACE_NAME)) == 0)
        intf->coreIntf.DirectLink = mDNStrue;
#endif
    intf->coreIntf.SupportsUnicastMDNSResponse = mDNStrue;

    // The interface is all ready to go, let's register it with the mDNS core.
    if (err == 0)
        err = mDNS_RegisterInterface(m, &intf->coreIntf, NormalActivation);

    // Clean up.
    if (err == 0)
    {
        num_registered_interfaces++;
        debugf("SetupOneInterface: %s %#a Registered", intf->intfName, &intf->coreIntf.ip);
        if (gMDNSPlatformPosixVerboseLevel > 0)
            fprintf(stderr, "Registered interface %s\n", intf->intfName);
    }
    else
    {
        // Use intfName instead of intf->intfName in the next line to avoid dereferencing NULL.
        debugf("SetupOneInterface: %s %#a failed to register %d", intfName, &intf->coreIntf.ip, err);
        if (intf) { FreePosixNetworkInterface(intf); intf = NULL; }
    }

    assert((err == 0) == (intf != NULL));

    return err;
}

// Call get_ifi_info() to obtain a list of active interfaces and call SetupOneInterface() on each one.
mDNSlocal int SetupInterfaceList(mDNS *const m)
{
    mDNSBool foundav4       = mDNSfalse;
    int err            = 0;
    struct ifaddrs *intfList;
    struct ifaddrs *firstLoopback = NULL;
    int firstLoopbackIndex = 0;

    assert(m != NULL);
    debugf("SetupInterfaceList");

    if (getifaddrs(&intfList) < 0)
    {
        err = errno;
    }
    if (intfList == NULL) err = ENOENT;

    if (err == 0)
    {
        struct ifaddrs *i = intfList;
        while (i)
        {
            if (     ((i->ifa_addr->sa_family == AF_INET)
#if HAVE_IPV6
                      || (i->ifa_addr->sa_family == AF_INET6)
#endif
                      ) &&  (i->ifa_flags & IFF_UP) && !(i->ifa_flags & IFF_POINTOPOINT))
            {
                int ifIndex = if_nametoindex(i->ifa_name);
                if (ifIndex == 0)
                {
                    continue;
                }
                if (i->ifa_flags & IFF_LOOPBACK)
                {
                    if (firstLoopback == NULL)
                    {
                        firstLoopback = i;
                        firstLoopbackIndex = ifIndex;
                    }
                }
                else
                {
                    if (SetupOneInterface(m, i->ifa_addr, i->ifa_netmask, i->ifa_name, ifIndex) == 0)
                    {
                        if (i->ifa_addr->sa_family == AF_INET)
                        {
                            foundav4 = mDNStrue;
                        }
                    }
                }
            }
            i = i->ifa_next;
        }

        // If we found no normal interfaces but we did find a loopback interface, register the
        // loopback interface.  This allows self-discovery if no interfaces are configured.
        // Temporary workaround: Multicast loopback on IPv6 interfaces appears not to work.
        // In the interim, we skip loopback interface only if we found at least one v4 interface to use
        // if ((m->HostInterfaces == NULL) && (firstLoopback != NULL))
        if (!foundav4 && firstLoopback)
        {
            (void)SetupOneInterface(m, firstLoopback->ifa_addr, firstLoopback->ifa_netmask, firstLoopback->ifa_name,
                                    firstLoopbackIndex);
        }
    }

    // Clean up.
    if (intfList != NULL) freeifaddrs(intfList);

    // Clean up any interfaces that have been hanging around on the RecentInterfaces list for more than a minute
    PosixNetworkInterface **ri = &gRecentInterfaces;
    const mDNSs32 utc = mDNSPlatformUTC();
    while (*ri)
    {
        PosixNetworkInterface *pi = *ri;
        if (utc - pi->LastSeen < 60) ri = (PosixNetworkInterface **)&pi->coreIntf.next;
        else { *ri = (PosixNetworkInterface *)pi->coreIntf.next; free(pi); }
    }

    return err;
}

#if USES_NETLINK

// See <http://www.faqs.org/rfcs/rfc3549.html> for a description of NetLink

// Open a socket that will receive interface change notifications
mDNSlocal mStatus OpenIfNotifySocket(int *pFD)
{
    mStatus err = mStatus_NoError;
    struct sockaddr_nl snl;
    int sock;
    int ret;

    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0)
        return errno;

    // Configure read to be non-blocking because inbound msg size is not known in advance
    (void) fcntl(sock, F_SETFL, O_NONBLOCK);

    /* Subscribe the socket to Link & IP addr notifications. */
    mDNSPlatformMemZero(&snl, sizeof snl);
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
    ret = bind(sock, (struct sockaddr *) &snl, sizeof snl);
    if (0 == ret)
        *pFD = sock;
    else
        err = errno;

    return err;
}

#if MDNS_DEBUGMSGS
mDNSlocal void      PrintNetLinkMsg(const struct nlmsghdr *pNLMsg)
{
    const char *kNLMsgTypes[] = { "", "NLMSG_NOOP", "NLMSG_ERROR", "NLMSG_DONE", "NLMSG_OVERRUN" };
    const char *kNLRtMsgTypes[] = { "RTM_NEWLINK", "RTM_DELLINK", "RTM_GETLINK", "RTM_NEWADDR", "RTM_DELADDR", "RTM_GETADDR" };

    printf("nlmsghdr len=%d, type=%s, flags=0x%x\n", pNLMsg->nlmsg_len,
           pNLMsg->nlmsg_type < RTM_BASE ? kNLMsgTypes[pNLMsg->nlmsg_type] : kNLRtMsgTypes[pNLMsg->nlmsg_type - RTM_BASE],
           pNLMsg->nlmsg_flags);

    if (RTM_NEWLINK <= pNLMsg->nlmsg_type && pNLMsg->nlmsg_type <= RTM_GETLINK)
    {
        struct ifinfomsg    *pIfInfo = (struct ifinfomsg*) NLMSG_DATA(pNLMsg);
        printf("ifinfomsg family=%d, type=%d, index=%d, flags=0x%x, change=0x%x\n", pIfInfo->ifi_family,
               pIfInfo->ifi_type, pIfInfo->ifi_index, pIfInfo->ifi_flags, pIfInfo->ifi_change);

    }
    else if (RTM_NEWADDR <= pNLMsg->nlmsg_type && pNLMsg->nlmsg_type <= RTM_GETADDR)
    {
        struct ifaddrmsg    *pIfAddr = (struct ifaddrmsg*) NLMSG_DATA(pNLMsg);
        printf("ifaddrmsg family=%d, index=%d, flags=0x%x\n", pIfAddr->ifa_family,
               pIfAddr->ifa_index, pIfAddr->ifa_flags);
    }
    printf("\n");
}
#endif

mDNSlocal mDNSu32       ProcessRoutingNotification(int sd)
// Read through the messages on sd and if any indicate that any interface records should
// be torn down and rebuilt, return affected indices as a bitmask. Otherwise return 0.
{
    ssize_t readCount;
    char buff[4096];
    struct nlmsghdr         *pNLMsg = (struct nlmsghdr*) buff;
    mDNSu32 result = 0;

    // The structure here is more complex than it really ought to be because,
    // unfortunately, there's no good way to size a buffer in advance large
    // enough to hold all pending data and so avoid message fragmentation.
    // (Note that FIONREAD is not supported on AF_NETLINK.)

    readCount = read(sd, buff, sizeof buff);
    while (1)
    {
        // Make sure we've got an entire nlmsghdr in the buffer, and payload, too.
        // If not, discard already-processed messages in buffer and read more data.
        if (((char*) &pNLMsg[1] > (buff + readCount)) ||    // i.e. *pNLMsg extends off end of buffer
            ((char*) pNLMsg + pNLMsg->nlmsg_len > (buff + readCount)))
        {
            if (buff < (char*) pNLMsg)      // we have space to shuffle
            {
                // discard processed data
                readCount -= ((char*) pNLMsg - buff);
                memmove(buff, pNLMsg, readCount);
                pNLMsg = (struct nlmsghdr*) buff;

                // read more data
                readCount += read(sd, buff + readCount, sizeof buff - readCount);
                continue;                   // spin around and revalidate with new readCount
            }
            else
                break;  // Otherwise message does not fit in buffer
        }

#if MDNS_DEBUGMSGS
        PrintNetLinkMsg(pNLMsg);
#endif

        // Process the NetLink message
        if (pNLMsg->nlmsg_type == RTM_GETLINK || pNLMsg->nlmsg_type == RTM_NEWLINK)
            result |= 1 << ((struct ifinfomsg*) NLMSG_DATA(pNLMsg))->ifi_index;
        else if (pNLMsg->nlmsg_type == RTM_DELADDR || pNLMsg->nlmsg_type == RTM_NEWADDR)
            result |= 1 << ((struct ifaddrmsg*) NLMSG_DATA(pNLMsg))->ifa_index;

        // Advance pNLMsg to the next message in the buffer
        if ((pNLMsg->nlmsg_flags & NLM_F_MULTI) != 0 && pNLMsg->nlmsg_type != NLMSG_DONE)
        {
            ssize_t len = readCount - ((char*)pNLMsg - buff);
            pNLMsg = NLMSG_NEXT(pNLMsg, len);
        }
        else
            break;  // all done!
    }

    return result;
}

#else // USES_NETLINK

// Open a socket that will receive interface change notifications
mDNSlocal mStatus OpenIfNotifySocket(int *pFD)
{
    *pFD = socket(AF_ROUTE, SOCK_RAW, 0);

    if (*pFD < 0)
        return mStatus_UnknownErr;

    // Configure read to be non-blocking because inbound msg size is not known in advance
    (void) fcntl(*pFD, F_SETFL, O_NONBLOCK);

    return mStatus_NoError;
}

#if MDNS_DEBUGMSGS
mDNSlocal void      PrintRoutingSocketMsg(const struct ifa_msghdr *pRSMsg)
{
    const char *kRSMsgTypes[] = { "", "RTM_ADD", "RTM_DELETE", "RTM_CHANGE", "RTM_GET", "RTM_LOSING",
                                  "RTM_REDIRECT", "RTM_MISS", "RTM_LOCK", "RTM_OLDADD", "RTM_OLDDEL", "RTM_RESOLVE",
                                  "RTM_NEWADDR", "RTM_DELADDR", "RTM_IFINFO", "RTM_NEWMADDR", "RTM_DELMADDR" };

    int index = pRSMsg->ifam_type == RTM_IFINFO ? ((struct if_msghdr*) pRSMsg)->ifm_index : pRSMsg->ifam_index;

    printf("ifa_msghdr len=%d, type=%s, index=%d\n", pRSMsg->ifam_msglen, kRSMsgTypes[pRSMsg->ifam_type], index);
}
#endif

mDNSlocal mDNSu32       ProcessRoutingNotification(int sd)
// Read through the messages on sd and if any indicate that any interface records should
// be torn down and rebuilt, return affected indices as a bitmask. Otherwise return 0.
{
    ssize_t readCount;
    char buff[4096];
    struct ifa_msghdr       *pRSMsg = (struct ifa_msghdr*) buff;
    mDNSu32 result = 0;

    readCount = read(sd, buff, sizeof buff);
    if (readCount < (ssize_t) sizeof(struct ifa_msghdr))
        return mStatus_UnsupportedErr;      // cannot decipher message

#if MDNS_DEBUGMSGS
    PrintRoutingSocketMsg(pRSMsg);
#endif

    // Process the message
    if (pRSMsg->ifam_type == RTM_NEWADDR || pRSMsg->ifam_type == RTM_DELADDR ||
        pRSMsg->ifam_type == RTM_IFINFO)
    {
        if (pRSMsg->ifam_type == RTM_IFINFO)
            result |= 1 << ((struct if_msghdr*) pRSMsg)->ifm_index;
        else
            result |= 1 << pRSMsg->ifam_index;
    }

    return result;
}

#endif // USES_NETLINK

// Called when data appears on interface change notification socket
mDNSlocal void InterfaceChangeCallback(int fd, void *context)
{
    IfChangeRec     *pChgRec = (IfChangeRec*) context;
    fd_set readFDs;
    mDNSu32 changedInterfaces = 0;
    struct timeval zeroTimeout = { 0, 0 };

    (void)fd; // Unused

    FD_ZERO(&readFDs);
    FD_SET(pChgRec->NotifySD, &readFDs);

    do
    {
        changedInterfaces |= ProcessRoutingNotification(pChgRec->NotifySD);
    }
    while (0 < select(pChgRec->NotifySD + 1, &readFDs, (fd_set*) NULL, (fd_set*) NULL, &zeroTimeout));

    // Currently we rebuild the entire interface list whenever any interface change is
    // detected. If this ever proves to be a performance issue in a multi-homed
    // configuration, more care should be paid to changedInterfaces.
    if (changedInterfaces)
        mDNSPlatformPosixRefreshInterfaceList(pChgRec->mDNS);
}

// Register with either a Routing Socket or RtNetLink to listen for interface changes.
mDNSlocal mStatus WatchForInterfaceChange(mDNS *const m)
{
    mStatus err;
    IfChangeRec *pChgRec;

    pChgRec = (IfChangeRec*) mDNSPlatformMemAllocateClear(sizeof *pChgRec);
    if (pChgRec == NULL)
        return mStatus_NoMemoryErr;

    pChgRec->mDNS = m;
    err = OpenIfNotifySocket(&pChgRec->NotifySD);
    if (err == 0)
        err = mDNSPosixAddFDToEventLoop(pChgRec->NotifySD, InterfaceChangeCallback, pChgRec);
    if (err)
        mDNSPlatformMemFree(pChgRec);

    return err;
}

// Test to see if we're the first client running on UDP port 5353, by trying to bind to 5353 without using SO_REUSEPORT.
// If we fail, someone else got here first. That's not a big problem; we can share the port for multicast responses --
// we just need to be aware that we shouldn't expect to successfully receive unicast UDP responses.
mDNSlocal mDNSBool mDNSPlatformInit_CanReceiveUnicast(void)
{
    int err;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in s5353;
    s5353.sin_family      = AF_INET;
    s5353.sin_port        = MulticastDNSPort.NotAnInteger;
    s5353.sin_addr.s_addr = 0;
    err = bind(s, (struct sockaddr *)&s5353, sizeof(s5353));
    close(s);
    if (err) debugf("No unicast UDP responses");
    else debugf("Unicast UDP responses okay");
    return(err == 0);
}

// mDNS core calls this routine to initialise the platform-specific data.
mDNSexport mStatus mDNSPlatformInit(mDNS *const m)
{
    int err = 0;
    struct sockaddr sa;
    assert(m != NULL);

    if (mDNSPlatformInit_CanReceiveUnicast()) m->CanReceiveUnicastOn5353 = mDNStrue;

    // Tell mDNS core the names of this machine.

    // Set up the nice label
    m->nicelabel.c[0] = 0;
    GetUserSpecifiedFriendlyComputerName(&m->nicelabel);
    if (m->nicelabel.c[0] == 0) MakeDomainLabelFromLiteralString(&m->nicelabel, "Computer");

    // Set up the RFC 1034-compliant label
    m->hostlabel.c[0] = 0;
    GetUserSpecifiedRFC1034ComputerName(&m->hostlabel);
    if (m->hostlabel.c[0] == 0) MakeDomainLabelFromLiteralString(&m->hostlabel, "Computer");

    mDNS_SetFQDN(m);

    sa.sa_family = AF_INET;
    m->p->unicastSocket4 = -1;
    if (err == mStatus_NoError) err = SetupSocket(&sa, zeroIPPort, 0, &m->p->unicastSocket4);
#if HAVE_IPV6
    sa.sa_family = AF_INET6;
    m->p->unicastSocket6 = -1;
    if (err == mStatus_NoError) err = SetupSocket(&sa, zeroIPPort, 0, &m->p->unicastSocket6);
#endif

    // Tell mDNS core about the network interfaces on this machine.
    if (err == mStatus_NoError) err = SetupInterfaceList(m);

    // Tell mDNS core about DNS Servers
    mDNS_Lock(m);
    if (err == mStatus_NoError) ParseDNSServers(m, uDNS_SERVERS_FILE);
    mDNS_Unlock(m);

    if (err == mStatus_NoError)
    {
        err = WatchForInterfaceChange(m);
        // Failure to observe interface changes is non-fatal.
        if (err != mStatus_NoError)
        {
            fprintf(stderr, "mDNS(%d) WARNING: Unable to detect interface changes (%d).\n", getpid(), err);
            err = mStatus_NoError;
        }
    }

    // We don't do asynchronous initialization on the Posix platform, so by the time
    // we get here the setup will already have succeeded or failed.  If it succeeded,
    // we should just call mDNSCoreInitComplete() immediately.
    if (err == mStatus_NoError)
        mDNSCoreInitComplete(m, mStatus_NoError);

    return PosixErrorToStatus(err);
}

// mDNS core calls this routine to clean up the platform-specific data.
// In our case all we need to do is to tear down every network interface.
mDNSexport void mDNSPlatformClose(mDNS *const m)
{
    int rv;
    assert(m != NULL);
    ClearInterfaceList(m);
    if (m->p->unicastSocket4 != -1)
    {
        rv = close(m->p->unicastSocket4);
        assert(rv == 0);
    }
#if HAVE_IPV6
    if (m->p->unicastSocket6 != -1)
    {
        rv = close(m->p->unicastSocket6);
        assert(rv == 0);
    }
#endif
}

// This is used internally by InterfaceChangeCallback.
// It's also exported so that the Standalone Responder (mDNSResponderPosix)
// can call it in response to a SIGHUP (mainly for debugging purposes).
mDNSexport mStatus mDNSPlatformPosixRefreshInterfaceList(mDNS *const m)
{
    int err;
    // This is a pretty heavyweight way to process interface changes --
    // destroying the entire interface list and then making fresh one from scratch.
    // We should make it like the OS X version, which leaves unchanged interfaces alone.
    ClearInterfaceList(m);
    err = SetupInterfaceList(m);
    return PosixErrorToStatus(err);
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Locking
#endif

// On the Posix platform, locking is a no-op because we only ever enter
// mDNS core on the main thread.

// mDNS core calls this routine when it wants to prevent
// the platform from reentering mDNS core code.
mDNSexport void    mDNSPlatformLock   (const mDNS *const m)
{
    (void) m;   // Unused
}

// mDNS core calls this routine when it release the lock taken by
// mDNSPlatformLock and allow the platform to reenter mDNS core code.
mDNSexport void    mDNSPlatformUnlock (const mDNS *const m)
{
    (void) m;   // Unused
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark ***** Strings
#endif

mDNSexport mDNSu32  mDNSPlatformStrLCopy(void *dst, const void *src, mDNSu32 len)
{
#if HAVE_STRLCPY
    return ((mDNSu32)strlcpy((char *)dst, (const char *)src, len));
#else
    size_t srcLen;

    srcLen = strlen((const char *)src);
    if (srcLen < len)
    {
        memcpy(dst, src, srcLen + 1);
    }
    else if (len > 0)
    {
        memcpy(dst, src, len - 1);
        ((char *)dst)[len - 1] = '\0';
    }

    return ((mDNSu32)srcLen);
#endif
}

// mDNS core calls this routine to get the length of a C string.
// On the Posix platform this maps directly to the ANSI C strlen.
mDNSexport mDNSu32  mDNSPlatformStrLen (const void *src)
{
    return strlen((const char*)src);
}

// mDNS core calls this routine to copy memory.
// On the Posix platform this maps directly to the ANSI C memcpy.
mDNSexport void    mDNSPlatformMemCopy(void *dst, const void *src, mDNSu32 len)
{
    memcpy(dst, src, len);
}

// mDNS core calls this routine to test whether blocks of memory are byte-for-byte
// identical. On the Posix platform this is a simple wrapper around ANSI C memcmp.
mDNSexport mDNSBool mDNSPlatformMemSame(const void *dst, const void *src, mDNSu32 len)
{
    return memcmp(dst, src, len) == 0;
}

// If the caller wants to know the exact return of memcmp, then use this instead
// of mDNSPlatformMemSame
mDNSexport int mDNSPlatformMemCmp(const void *dst, const void *src, mDNSu32 len)
{
    return (memcmp(dst, src, len));
}

mDNSexport void mDNSPlatformQsort(void *base, int nel, int width, int (*compar)(const void *, const void *))
{
    return (qsort(base, nel, width, compar));
}

// DNSSEC stub functions
mDNSexport void VerifySignature(mDNS *const m, DNSSECVerifier *dv, DNSQuestion *q)
{
    (void)m;
    (void)dv;
    (void)q;
}

mDNSexport mDNSBool AddNSECSForCacheRecord(mDNS *const m, CacheRecord *crlist, CacheRecord *negcr, mDNSu8 rcode)
{
    (void)m;
    (void)crlist;
    (void)negcr;
    (void)rcode;
    return mDNSfalse;
}

mDNSexport void BumpDNSSECStats(mDNS *const m, DNSSECStatsAction action, DNSSECStatsType type, mDNSu32 value)
{
    (void)m;
    (void)action;
    (void)type;
    (void)value;
}

// Proxy stub functions
mDNSexport mDNSu8 *DNSProxySetAttributes(DNSQuestion *q, DNSMessageHeader *h, DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *limit)
{
    (void) q;
    (void) h;
    (void) msg;
    (void) ptr;
    (void) limit;

    return ptr;
}

mDNSexport void DNSProxyInit(mDNSu32 IpIfArr[], mDNSu32 OpIf)
{
    (void) IpIfArr;
    (void) OpIf;
}

mDNSexport void DNSProxyTerminate(void)
{
}

// mDNS core calls this routine to clear blocks of memory.
// On the Posix platform this is a simple wrapper around ANSI C memset.
mDNSexport void  mDNSPlatformMemZero(void *dst, mDNSu32 len)
{
    memset(dst, 0, len);
}

#if !MDNS_MALLOC_DEBUGGING
mDNSexport void *mDNSPlatformMemAllocate(mDNSu32 len)      { return(mallocL("mDNSPlatformMemAllocate", len)); }
mDNSexport void *mDNSPlatformMemAllocateClear(mDNSu32 len) { return(callocL(name, len)); }
mDNSexport void  mDNSPlatformMemFree    (void *mem)        {          freeL("mDNSPlatformMemFree", mem); }
#endif

#if _PLATFORM_HAS_STRONG_PRNG_
mDNSexport mDNSu32 mDNSPlatformRandomNumber(void)
{
    return(arc4random());
}
#else
mDNSexport mDNSu32 mDNSPlatformRandomSeed(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return(tv.tv_usec);
}
#endif

mDNSexport mDNSs32 mDNSPlatformOneSecond = 1024;

mDNSexport mStatus mDNSPlatformTimeInit(void)
{
    // No special setup is required on Posix -- we just use gettimeofday();
    // This is not really safe, because gettimeofday can go backwards if the user manually changes the date or time
    // We should find a better way to do this
    return(mStatus_NoError);
}

mDNSexport mDNSs32  mDNSPlatformRawTime()
{
    struct timespec tm;
    int ret = clock_gettime(CLOCK_MONOTONIC, &tm);
    assert(ret == 0); // This call will only fail if the number of seconds does not fit in an object of type time_t.

    // tm.tv_sec is seconds since some unspecified starting point (it is usually the system start up time)
    // tm.tv_nsec is nanoseconds since the start of this second (i.e. values 0 to 999999999)
    // We use the lower 22 bits of tm.tv_sec for the top 22 bits of our result
    // and we multiply tm.tv_nsec by 2 / 1953125 to get a value in the range 0-1023 to go in the bottom 10 bits.
    // This gives us a proper modular (cyclic) counter that has a resolution of roughly 1ms (actually 1/1024 second)
    // and correctly cycles every 2^22 seconds (4194304 seconds = approx 48 days).

    return ((tm.tv_sec << 10) | (tm.tv_nsec * 2 / 1953125));
}

mDNSexport mDNSs32 mDNSPlatformUTC(void)
{
    return time(NULL);
}

mDNSexport void mDNSPlatformSendWakeupPacket(mDNSInterfaceID InterfaceID, char *EthAddr, char *IPAddr, int iteration)
{
    (void) InterfaceID;
    (void) EthAddr;
    (void) IPAddr;
    (void) iteration;
}

mDNSexport mDNSBool mDNSPlatformValidRecordForInterface(const AuthRecord *rr, mDNSInterfaceID InterfaceID)
{
    (void) rr;
    (void) InterfaceID;

    return 1;
}

mDNSexport mDNSBool mDNSPlatformValidQuestionForInterface(DNSQuestion *q, const NetworkInterfaceInfo *intf)
{
    (void) q;
    (void) intf;

    return 1;
}

// Used for debugging purposes. For now, just set the buffer to zero
mDNSexport void mDNSPlatformFormatTime(unsigned long te, mDNSu8 *buf, int bufsize)
{
    (void) te;
    if (bufsize) buf[0] = 0;
}

mDNSexport void mDNSPlatformSendKeepalive(mDNSAddr *sadd, mDNSAddr *dadd, mDNSIPPort *lport, mDNSIPPort *rport, mDNSu32 seq, mDNSu32 ack, mDNSu16 win)
{
    (void) sadd;    // Unused
    (void) dadd;    // Unused
    (void) lport;   // Unused
    (void) rport;   // Unused
    (void) seq;     // Unused
    (void) ack;     // Unused
    (void) win;     // Unused
}

mDNSexport mStatus mDNSPlatformRetrieveTCPInfo(mDNSAddr *laddr, mDNSIPPort *lport, mDNSAddr *raddr, mDNSIPPort *rport, mDNSTCPInfo *mti)
{
    (void) laddr;   // Unused
    (void) raddr;   // Unused
    (void) lport;   // Unused
    (void) rport;   // Unused
    (void) mti;     // Unused

    return mStatus_NoError;
}

mDNSexport mStatus mDNSPlatformGetRemoteMacAddr(mDNSAddr *raddr)
{
    (void) raddr; // Unused

    return mStatus_NoError;
}

mDNSexport mStatus    mDNSPlatformStoreSPSMACAddr(mDNSAddr *spsaddr, char *ifname)
{
    (void) spsaddr; // Unused
    (void) ifname;  // Unused

    return mStatus_NoError;
}

mDNSexport mStatus    mDNSPlatformClearSPSData(void)
{
    return mStatus_NoError;
}

mDNSexport mStatus mDNSPlatformStoreOwnerOptRecord(char *ifname, DNSMessage *msg, int length)
{
    (void) ifname; // Unused
    (void) msg;    // Unused
    (void) length; // Unused
    return mStatus_UnsupportedErr;
}

mDNSexport mDNSu16 mDNSPlatformGetUDPPort(UDPSocket *sock)
{
    (void) sock; // unused

    return (mDNSu16)-1;
}

mDNSexport mDNSBool mDNSPlatformInterfaceIsD2D(mDNSInterfaceID InterfaceID)
{
    (void) InterfaceID; // unused

    return mDNSfalse;
}

mDNSexport void mDNSPlatformSetSocktOpt(void *sock, mDNSTransport_Type transType, mDNSAddr_Type addrType, const DNSQuestion *q)
{
    (void) sock;
    (void) transType;
    (void) addrType;
    (void) q;
}

mDNSexport mDNSs32 mDNSPlatformGetPID()
{
    return 0;
}

mDNSlocal void mDNSPosixAddToFDSet(int *nfds, fd_set *readfds, int s)
{
    if (*nfds < s + 1) *nfds = s + 1;
    FD_SET(s, readfds);
}

mDNSexport void mDNSPosixGetFDSetForSelect(mDNS *m, int *nfds, fd_set *readfds, fd_set *writefds)
{
    int numFDs = *nfds;
    PosixEventSource *iSource;

    // 2. Build our list of active file descriptors
    PosixNetworkInterface *info = (PosixNetworkInterface *)(m->HostInterfaces);
    if (m->p->unicastSocket4 != -1) mDNSPosixAddToFDSet(&numFDs, readfds, m->p->unicastSocket4);
#if HAVE_IPV6
    if (m->p->unicastSocket6 != -1) mDNSPosixAddToFDSet(&numFDs, readfds, m->p->unicastSocket6);
#endif
    while (info)
    {
        if (info->multicastSocket4 != -1) mDNSPosixAddToFDSet(&numFDs, readfds, info->multicastSocket4);
#if HAVE_IPV6
        if (info->multicastSocket6 != -1) mDNSPosixAddToFDSet(&numFDs, readfds, info->multicastSocket6);
#endif
        info = (PosixNetworkInterface *)(info->coreIntf.next);
    }

    // Copy over the event fds.   We have to do it this way because client-provided event loops expect
    // to initialize their FD sets first and then call mDNSPosixGetFDSet()
    for (iSource = gEventSources; iSource; iSource = iSource->next)
    {
        if (iSource->readCallback != NULL)
            FD_SET(iSource->fd, readfds);
        if (iSource->writeCallback != NULL)
            FD_SET(iSource->fd, writefds);
        if (numFDs <= iSource->fd)
            numFDs = iSource->fd + 1;
    }
    *nfds = numFDs;
}

mDNSexport void mDNSPosixGetNextDNSEventTime(mDNS *m, struct timeval *timeout)
{
    mDNSs32 ticks;
    struct timeval interval;

    // 1. Call mDNS_Execute() to let mDNSCore do what it needs to do
    mDNSs32 nextevent = mDNS_Execute(m);

    // 3. Calculate the time remaining to the next scheduled event (in struct timeval format)
    ticks = nextevent - mDNS_TimeNow(m);
    if (ticks < 1) ticks = 1;
    interval.tv_sec  = ticks >> 10;                     // The high 22 bits are seconds
    interval.tv_usec = ((ticks & 0x3FF) * 15625) / 16;  // The low 10 bits are 1024ths

    // 4. If client's proposed timeout is more than what we want, then reduce it
    if (timeout->tv_sec > interval.tv_sec ||
        (timeout->tv_sec == interval.tv_sec && timeout->tv_usec > interval.tv_usec))
        *timeout = interval;
}

mDNSexport void mDNSPosixGetFDSet(mDNS *m, int *nfds, fd_set *readfds, fd_set *writefds, struct timeval *timeout)
{
    mDNSPosixGetNextDNSEventTime(m, timeout);
    mDNSPosixGetFDSetForSelect(m, nfds, readfds, writefds);
}

mDNSexport void mDNSPosixProcessFDSet(mDNS *const m, fd_set *readfds, fd_set *writefds)
{
    PosixNetworkInterface *info;
    PosixEventSource    *iSource;
    assert(m       != NULL);
    assert(readfds != NULL);
    info = (PosixNetworkInterface *)(m->HostInterfaces);

    if (m->p->unicastSocket4 != -1 && FD_ISSET(m->p->unicastSocket4, readfds))
    {
        FD_CLR(m->p->unicastSocket4, readfds);
        SocketDataReady(m, NULL, m->p->unicastSocket4);
    }
#if HAVE_IPV6
    if (m->p->unicastSocket6 != -1 && FD_ISSET(m->p->unicastSocket6, readfds))
    {
        FD_CLR(m->p->unicastSocket6, readfds);
        SocketDataReady(m, NULL, m->p->unicastSocket6);
    }
#endif

    while (info)
    {
        if (info->multicastSocket4 != -1 && FD_ISSET(info->multicastSocket4, readfds))
        {
            FD_CLR(info->multicastSocket4, readfds);
            SocketDataReady(m, info, info->multicastSocket4);
        }
#if HAVE_IPV6
        if (info->multicastSocket6 != -1 && FD_ISSET(info->multicastSocket6, readfds))
        {
            FD_CLR(info->multicastSocket6, readfds);
            SocketDataReady(m, info, info->multicastSocket6);
        }
#endif
        info = (PosixNetworkInterface *)(info->coreIntf.next);
    }

    // Now process routing socket events, discovery relay events and anything else of that ilk.
    for (iSource = gEventSources; iSource; iSource = iSource->next)
    {
        if (iSource->readCallback != NULL && FD_ISSET(iSource->fd, readfds))
        {
            iSource->readCallback(iSource->fd, iSource->readContext);
            break;  // in case callback removed elements from gEventSources
        }
        else if (iSource->writeCallback != NULL && FD_ISSET(iSource->fd, writefds))
        {
            mDNSPosixEventCallback writeCallback = iSource->writeCallback;
            // Write events are one-shot: to get another event, the consumer has to put in a new request.
            // We reset this before calling the callback just in case the callback requests another write
            // callback, or deletes the event context from the list.
            iSource->writeCallback = NULL;
            writeCallback(iSource->fd, iSource->writeContext);
            break;  // in case callback removed elements from gEventSources
        }
    }
}

mDNSu32 mDNSPlatformEventContextSize = sizeof (PosixEventSource);

mDNSlocal void requestIOEvents(PosixEventSource *newSource, const char *taskName,
                                  mDNSPosixEventCallback callback, void *context, int flag)
{
    PosixEventSource **epp = &gEventSources;

    if (newSource->fd >= (int) FD_SETSIZE || newSource->fd < 0)
    {
        LogMsg("requestIOEvents called with fd %d > FD_SETSIZE %d.", newSource->fd, FD_SETSIZE);
        assert(0);
    }
    if (callback == NULL)
    {
        LogMsg("requestIOEvents called no callback.", newSource->fd, FD_SETSIZE);
        assert(0);
    }

    // See if this event context is already on the list; if it is, no need to scan the list.
    if (!(newSource->flags & PosixEventFlag_OnList))
    {
        while (*epp)
        {
            // This should never happen.
            if (newSource == *epp)
            {
                LogMsg("Event context marked not on list but is on list.");
                assert(0);
            }
            epp = &(*epp)->next;
        }
        if (*epp == NULL)
        {
            *epp = newSource;
            newSource->next = NULL;
            newSource->flags = PosixEventFlag_OnList;
        }
    }

    if (flag & PosixEventFlag_Read)
    {
        newSource->readCallback = callback;
        newSource->readContext = context;
        newSource->flags |= PosixEventFlag_Read;
        newSource->readTaskName = taskName;
    }
    if (flag & PosixEventFlag_Write)
    {
        newSource->writeCallback = callback;
        newSource->writeContext = context;
        newSource->flags |= PosixEventFlag_Write;
        newSource->writeTaskName = taskName;
    }
}

mDNSlocal void requestReadEvents(PosixEventSource *eventSource,
                                    const char *taskName, mDNSPosixEventCallback callback, void *context)
{
    requestIOEvents(eventSource, taskName, callback, context, PosixEventFlag_Read);
}

mDNSlocal void requestWriteEvents(PosixEventSource *eventSource,
                                     const char *taskName, mDNSPosixEventCallback callback, void *context)
{
    requestIOEvents(eventSource, taskName, callback, context, PosixEventFlag_Write);
}

// Remove a file descriptor from the set that mDNSPosixRunEventLoopOnce() listens to.
mDNSlocal mStatus stopReadOrWriteEvents(int fd, mDNSBool freeContext, mDNSBool removeContext, int flags)
{
    PosixEventSource *iSource, **epp = &gEventSources;

    while (*epp)
    {
        iSource = *epp;
        if (fd == iSource->fd)
        {
            if (flags & PosixEventFlag_Read)
            {
                iSource->readCallback = NULL;
                iSource->readContext = NULL;
            }
            if (flags & PosixEventFlag_Write)
            {
                iSource->writeCallback = NULL;
                iSource->writeContext = NULL;
            }
            if (iSource->writeCallback == NULL && iSource->readCallback == NULL)
            {
                if (removeContext || freeContext)
                    *epp = iSource->next;
                if (freeContext)
                    free(iSource);
            }
            return mStatus_NoError;
        }
        epp = &(*epp)->next;
    }
    return mStatus_NoSuchNameErr;
}

// Some of the mDNSPosix client code relies on being able to add FDs to the event loop without
// providing storage for the event-related info.   mDNSPosixAddFDToEventLoop and
// mDNSPosixRemoveFDFromEventLoop handle the event structure storage automatically.
mStatus mDNSPosixAddFDToEventLoop(int fd, mDNSPosixEventCallback callback, void *context)
{
    PosixEventSource *newSource;

    newSource = (PosixEventSource*) malloc(sizeof *newSource);
    if (NULL == newSource)
        return mStatus_NoMemoryErr;
    memset(newSource, 0, sizeof *newSource);
    newSource->fd = fd;

    requestReadEvents(newSource, "mDNSPosixAddFDToEventLoop", callback, context);
    return mStatus_NoError;
}

mStatus mDNSPosixRemoveFDFromEventLoop(int fd)
{
    return stopReadOrWriteEvents(fd, mDNStrue, mDNStrue, PosixEventFlag_Read | PosixEventFlag_Write);
}

// Simply note the received signal in gEventSignals.
mDNSlocal void  NoteSignal(int signum)
{
    sigaddset(&gEventSignals, signum);
}

// Tell the event package to listen for signal and report it in mDNSPosixRunEventLoopOnce().
mStatus mDNSPosixListenForSignalInEventLoop(int signum)
{
    struct sigaction action;
    mStatus err;

    mDNSPlatformMemZero(&action, sizeof action);        // more portable than member-wise assignment
    action.sa_handler = NoteSignal;
    err = sigaction(signum, &action, (struct sigaction*) NULL);

    sigaddset(&gEventSignalSet, signum);

    return err;
}

// Tell the event package to stop listening for signal in mDNSPosixRunEventLoopOnce().
mStatus mDNSPosixIgnoreSignalInEventLoop(int signum)
{
    struct sigaction action;
    mStatus err;

    mDNSPlatformMemZero(&action, sizeof action);        // more portable than member-wise assignment
    action.sa_handler = SIG_DFL;
    err = sigaction(signum, &action, (struct sigaction*) NULL);

    sigdelset(&gEventSignalSet, signum);

    return err;
}

// Do a single pass through the attendent event sources and dispatch any found to their callbacks.
// Return as soon as internal timeout expires, or a signal we're listening for is received.
mStatus mDNSPosixRunEventLoopOnce(mDNS *m, const struct timeval *pTimeout,
                                  sigset_t *pSignalsReceived, mDNSBool *pDataDispatched)
{
    fd_set listenFDs;
    fd_set writeFDs;
    int numFDs = 0, numReady;
    struct timeval timeout = *pTimeout;

    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    FD_ZERO(&listenFDs);
    FD_ZERO(&writeFDs);

    // 2. Set up the timeout.
    mDNSPosixGetNextDNSEventTime(m, &timeout);

    // Include the sockets that are listening to the wire in our select() set
    mDNSPosixGetFDSetForSelect(m, &numFDs, &listenFDs, &writeFDs);
    numReady = select(numFDs, &listenFDs, &writeFDs, (fd_set*) NULL, &timeout);

    if (numReady > 0)
    {
        mDNSPosixProcessFDSet(m, &listenFDs, &writeFDs);
        *pDataDispatched = mDNStrue;
    }
    else if (numReady < 0)
    {
	if (errno != EINTR) {
            // This should never happen, represents a coding error, and is not recoverable, since
            // we'll just sit here spinning and never receive another event.   The usual reason for
            // it to happen is that an FD was closed but not removed from the event list.
            LogMsg("select failed: %s", strerror(errno));
            abort();
        }
    }
    else
        *pDataDispatched = mDNSfalse;

    (void) sigprocmask(SIG_BLOCK, &gEventSignalSet, (sigset_t*) NULL);
    *pSignalsReceived = gEventSignals;
    sigemptyset(&gEventSignals);
    (void) sigprocmask(SIG_UNBLOCK, &gEventSignalSet, (sigset_t*) NULL);

    return mStatus_NoError;
}
