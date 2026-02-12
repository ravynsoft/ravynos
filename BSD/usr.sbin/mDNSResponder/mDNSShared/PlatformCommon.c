/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil; -*-
 *
 * Copyright (c) 2004-2019 Apple Inc. All rights reserved.
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
 * This file defines functions that are common to platforms with Posix APIs.
 * Current examples are mDNSMacOSX and mDNSPosix.
 */

#include <stdio.h>              // Needed for fopen() etc.
#include <unistd.h>             // Needed for close()
#include <stdlib.h>             // Needed for malloc()
#include <string.h>             // Needed for strlen() etc.
#include <errno.h>              // Needed for errno etc.
#include <sys/socket.h>         // Needed for socket() etc.
#include <netinet/in.h>         // Needed for sockaddr_in
#include <syslog.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <assert.h>

#if APPLE_OSX_mDNSResponder
#include <os/log.h>
#endif 

#include "mDNSEmbeddedAPI.h"    // Defines the interface provided to the client layer above
#include "DNSCommon.h"
#include "PlatformCommon.h"

#ifdef NOT_HAVE_SOCKLEN_T
typedef unsigned int socklen_t;
#endif

#if MDNS_MALLOC_DEBUGGING
// We ONLY want this for malloc debugging--on a running production system we want to deal with
// malloc failures, not just die.   There is a small performance penalty for enabling these options
// as well, so they are all only appropriate for debugging.   The flags mean:
//
// A = warnings are errors
// X = abort on failure
// Z = sets J & R
// J = allocated memory is initialized to a pattern
// R causes realloc to always reallocate even if not needed

char _malloc_options[] = "AXZ";

mDNSlocal mDNSListValidator *listValidators;

mDNSexport void mDNSPlatformAddListValidator(mDNSListValidator *lv, mDNSListValidationFunction *lvf,
                                             const char *lvfName, void *context)
{
    mDNSPlatformMemZero(lv, sizeof *lv);
    lv->validator = lvf;
    lv->validationFunctionName = lvfName;
    lv->context = context;
    lv->next = listValidators;
    listValidators = lv;
}

mDNSlocal void validateLists(void)
{
    mDNSListValidator *vfp;
    // Check Unix Domain Socket client lists (uds_daemon.c)
    for (vfp = listValidators; vfp; vfp = vfp->next)
    {
        vfp->validator(vfp->context);
    }

    mDNSPlatformValidateLists();
}

#define kAllocMagic     0xDEAD1234
#define kGuardMagic     0xDEAD1234
#define kFreeMagic      0xDEADDEAD
#define kAllocLargeSize 32768

mDNSexport void *mallocL(const char *msg, mDNSu32 size)
{
    // Allocate space for two words of sanity checking data before the requested block and two words after.
    // Adjust the length for alignment.
    mDNSu32 *mem = malloc(sizeof(mDNSu32) * 4 + size);
    mDNSu32 guard[2];
    if (!mem)
    { LogMsg("malloc( %s : %u ) failed", msg, size); return(NULL); }
    else
    {
        mDNSu32 *after = (mDNSu32 *)((mDNSu8 *)(mem + 2) + size);
        if      (size > kAllocLargeSize)      LogMsg("malloc( %s : %lu ) @ %p suspiciously large", msg, size, &mem[2]);
        else if (MDNS_MALLOC_DEBUGGING >= 2)  LogMsg("malloc( %s : %lu ) @ %p",                    msg, size, &mem[2]);
        mem[  0] = kAllocMagic;
        guard[0] = kGuardMagic;
        mem[  1] = size;
        guard[1] = size;
        memcpy(after, &guard, sizeof guard);
        memset(&mem[2], 0xFF, size);
        validateLists();
        return(&mem[2]);
    }
}

mDNSexport void *callocL(const char *msg, mDNSu32 size)
{
    mDNSu32 guard[2];
    const mDNSu32 headerSize = 4 * sizeof(mDNSu32);
    
    // Allocate space for two words of sanity checking data before the requested block and two words after.
    // Adjust the length for alignment.
    mDNSu32 *mem = (mDNSu32 *)calloc(1, headerSize + size);
    if (!mem)
    { LogMsg("calloc( %s : %u ) failed", msg, size); return(NULL); }
    else
    {
        mDNSu32 *after = (mDNSu32 *)((mDNSu8 *)(mem + 2) + size);
        if      (size > kAllocLargeSize)     LogMsg("calloc( %s : %lu ) @ %p suspiciously large", msg, size, &mem[2]);
        else if (MDNS_MALLOC_DEBUGGING >= 2) LogMsg("calloc( %s : %lu ) @ %p",                    msg, size, &mem[2]);
        mem[  0] = kAllocMagic;
        guard[0] = kGuardMagic;
        mem[  1] = size;
        guard[1] = size;
        memcpy(after, guard, sizeof guard);
        validateLists();
        return(&mem[2]);
    }
}

mDNSexport void freeL(const char *msg, void *x)
{
    if (!x)
        LogMsg("free( %s @ NULL )!", msg);
    else
    {
        mDNSu32 *mem = ((mDNSu32 *)x) - 2;
        if      (mem[0] == kFreeMagic)  { LogMemCorruption("free( %s : %lu @ %p ) !!!! ALREADY DISPOSED !!!!", msg, mem[1], &mem[2]); return; }
        if      (mem[0] != kAllocMagic) { LogMemCorruption("free( %s : %lu @ %p ) !!!! NEVER ALLOCATED !!!!",  msg, mem[1], &mem[2]); return; }
        if      (mem[1] > kAllocLargeSize)          LogMsg("free( %s : %lu @ %p) suspiciously large",          msg, mem[1], &mem[2]);
        else if (MDNS_MALLOC_DEBUGGING >= 2)        LogMsg("free( %s : %ld @ %p)",                             msg, mem[1], &mem[2]);
        mDNSu32 *after = (mDNSu32 *)((mDNSu8 *)x + mem[1]);
        mDNSu32 guard[2];

        memcpy(guard, after, sizeof guard);
        if (guard[0] != kGuardMagic)    { LogMemCorruption("free( %s : %lu @ %p ) !!!! END GUARD OVERWRITE !!!!",
                                                           msg, mem[1], &mem[2]); return; }
        if (guard[1] != mem[1])         { LogMemCorruption("free( %s : %lu @ %p ) !!!! LENGTH MISMATCH !!!!",
                                                           msg, mem[1], &mem[2]); return; }
        mem[0] = kFreeMagic;
        memset(mem + 2, 0xFF, mem[1] + 2 * sizeof(mDNSu32));
        validateLists();
        free(mem);
    }
}

#endif

// Bind a UDP socket to find the source address to a destination
mDNSexport void mDNSPlatformSourceAddrForDest(mDNSAddr *const src, const mDNSAddr *const dst)
{
    union { struct sockaddr s; struct sockaddr_in a4; struct sockaddr_in6 a6; } addr;
    socklen_t len = sizeof(addr);
    socklen_t inner_len = 0;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    src->type = mDNSAddrType_None;
    if (sock == -1) return;
    if (dst->type == mDNSAddrType_IPv4)
    {
        inner_len = sizeof(addr.a4);
        #ifndef NOT_HAVE_SA_LEN
        addr.a4.sin_len         = inner_len;
        #endif
        addr.a4.sin_family      = AF_INET;
        addr.a4.sin_port        = 1;    // Not important, any port will do
        addr.a4.sin_addr.s_addr = dst->ip.v4.NotAnInteger;
    }
    else if (dst->type == mDNSAddrType_IPv6)
    {
        inner_len = sizeof(addr.a6);
        #ifndef NOT_HAVE_SA_LEN
        addr.a6.sin6_len      = inner_len;
        #endif
        addr.a6.sin6_family   = AF_INET6;
        addr.a6.sin6_flowinfo = 0;
        addr.a6.sin6_port     = 1;  // Not important, any port will do
        addr.a6.sin6_addr     = *(struct in6_addr*)&dst->ip.v6;
        addr.a6.sin6_scope_id = 0;
    }
    else return;

    if ((connect(sock, &addr.s, inner_len)) < 0)
    { LogMsg("mDNSPlatformSourceAddrForDest: connect %#a failed errno %d (%s)", dst, errno, strerror(errno)); goto exit; }

    if ((getsockname(sock, &addr.s, &len)) < 0)
    { LogMsg("mDNSPlatformSourceAddrForDest: getsockname failed errno %d (%s)", errno, strerror(errno)); goto exit; }

    src->type = dst->type;
    if (dst->type == mDNSAddrType_IPv4) src->ip.v4.NotAnInteger = addr.a4.sin_addr.s_addr;
    else src->ip.v6 = *(mDNSv6Addr*)&addr.a6.sin6_addr;
exit:
    close(sock);
}

// dst must be at least MAX_ESCAPED_DOMAIN_NAME bytes, and option must be less than 32 bytes in length
mDNSlocal mDNSBool GetConfigOption(char *dst, const char *option, FILE *f)
{
    char buf[32+1+MAX_ESCAPED_DOMAIN_NAME]; // Option name, one space, option value
    unsigned int len = strlen(option);
    if (len + 1 + MAX_ESCAPED_DOMAIN_NAME > sizeof(buf)-1) { LogMsg("GetConfigOption: option %s too long", option); return mDNSfalse; }
    fseek(f, 0, SEEK_SET);  // set position to beginning of stream
    while (fgets(buf, sizeof(buf), f))      // Read at most sizeof(buf)-1 bytes from file, and append '\0' C-string terminator
    {
        if (!strncmp(buf, option, len))
        {
            strncpy(dst, buf + len + 1, MAX_ESCAPED_DOMAIN_NAME-1);
            if (dst[MAX_ESCAPED_DOMAIN_NAME-1]) dst[MAX_ESCAPED_DOMAIN_NAME-1] = '\0';
            len = strlen(dst);
            if (len && dst[len-1] == '\n') dst[len-1] = '\0';  // chop newline
            return mDNStrue;
        }
    }
    debugf("Option %s not set", option);
    return mDNSfalse;
}

mDNSexport void ReadDDNSSettingsFromConfFile(mDNS *const m, const char *const filename, domainname *const hostname, domainname *const domain, mDNSBool *DomainDiscoveryDisabled)
{
    char buf[MAX_ESCAPED_DOMAIN_NAME] = "";
    mStatus err;
    FILE *f = fopen(filename, "r");

    if (hostname) hostname->c[0] = 0;
    if (domain) domain->c[0] = 0;
    if (DomainDiscoveryDisabled) *DomainDiscoveryDisabled = mDNSfalse;

    if (f)
    {
        if (DomainDiscoveryDisabled && GetConfigOption(buf, "DomainDiscoveryDisabled", f) && !strcasecmp(buf, "true")) *DomainDiscoveryDisabled = mDNStrue;
        if (hostname && GetConfigOption(buf, "hostname", f) && !MakeDomainNameFromDNSNameString(hostname, buf)) goto badf;
        if (domain && GetConfigOption(buf, "zone", f) && !MakeDomainNameFromDNSNameString(domain, buf)) goto badf;
        buf[0] = 0;
        GetConfigOption(buf, "secret-64", f);  // failure means no authentication
        fclose(f);
        f = NULL;
    }
    else
    {
        if (errno != ENOENT) LogMsg("ERROR: Config file exists, but cannot be opened.");
        return;
    }

    if (domain && domain->c[0] && buf[0])
    {
        DomainAuthInfo *info = (DomainAuthInfo*) mDNSPlatformMemAllocateClear(sizeof(*info));
        // for now we assume keyname = service reg domain and we use same key for service and hostname registration
        err = mDNS_SetSecretForDomain(m, info, domain, domain, buf, NULL, 0);
        if (err) LogMsg("ERROR: mDNS_SetSecretForDomain returned %d for domain %##s", err, domain->c);
    }

    return;

badf:
    LogMsg("ERROR: malformatted config file");
    if (f) fclose(f);
}

#if MDNS_DEBUGMSGS
mDNSexport void mDNSPlatformWriteDebugMsg(const char *msg)
{
    fprintf(stderr,"%s\n", msg);
    fflush(stderr);
}
#endif

#if !MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
mDNSexport void mDNSPlatformWriteLogMsg(const char *ident, const char *buffer, mDNSLogLevel_t loglevel)
{
#if APPLE_OSX_mDNSResponder && LogTimeStamps
    extern mDNS mDNSStorage;
    extern mDNSu32 mDNSPlatformClockDivisor;
    mDNSs32 t = mDNSStorage.timenow ? mDNSStorage.timenow : mDNSPlatformClockDivisor ? mDNS_TimeNow_NoLock(&mDNSStorage) : 0;
    int ms = ((t < 0) ? -t : t) % 1000;
#endif

    if (mDNS_DebugMode) // In debug mode we write to stderr
    {
#if APPLE_OSX_mDNSResponder && LogTimeStamps
        if (ident && ident[0] && mDNSPlatformClockDivisor)
            fprintf(stderr,"%8d.%03d: %s\n", (int)(t/1000), ms, buffer);
        else
#endif
        fprintf(stderr,"%s\n", buffer);
        fflush(stderr);
    }
    else                // else, in production mode, we write to syslog
    {
        static int log_inited = 0;

        int syslog_level;
        switch (loglevel)
        {
            case MDNS_LOG_FAULT:     syslog_level = LOG_ERR;     break;
            case MDNS_LOG_ERROR:     syslog_level = LOG_ERR;     break;
            case MDNS_LOG_WARNING:   syslog_level = LOG_WARNING; break;
            case MDNS_LOG_DEFAULT:   syslog_level = LOG_NOTICE;  break;
            case MDNS_LOG_INFO:      syslog_level = LOG_INFO;    break;
            case MDNS_LOG_DEBUG:     syslog_level = LOG_DEBUG;   break;
            default:                 syslog_level = LOG_NOTICE;  break;
        }

        if (!log_inited) { openlog(ident, LOG_CONS, LOG_DAEMON); log_inited++; }

#if APPLE_OSX_mDNSResponder && LogTimeStamps
        if (ident && ident[0] && mDNSPlatformClockDivisor)
            syslog(syslog_level, "%8d.%03d: %s", (int)(t/1000), ms, buffer);
        else
#endif
        {
            syslog(syslog_level, "%s", buffer);
        }
    }
}
#endif // !MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)

mDNSexport mDNSBool mDNSPosixTCPSocketSetup(int *fd, mDNSAddr_Type addrType, mDNSIPPort *port, mDNSIPPort *outTcpPort)
{
    int sa_family = (addrType == mDNSAddrType_IPv4) ? AF_INET : AF_INET6;
    int err;
    int sock;
    mDNSu32 lowWater = 15384;

    sock = socket(sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 3)
    {
        if (errno != EAFNOSUPPORT)
        {
            LogMsg("mDNSPosixTCPSocketSetup: socket error %d errno %d (%s)", sock, errno, strerror(errno));
        }
        return mDNStrue;
    }
    *fd = sock;

    union
    {
        struct sockaddr sa;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } addr;
    // If port is not NULL, bind to it.
    if (port != NULL)
    {
        socklen_t len = (sa_family == AF_INET) ? sizeof (struct sockaddr_in) : sizeof (struct sockaddr_in6);
        mDNSPlatformMemZero(&addr, sizeof addr);

        addr.sa.sa_family = sa_family;
#ifndef NOT_HAVE_SA_LEN
	addr.sa.sa_len = len;
#endif
        if (sa_family == AF_INET6)
        {
            addr.sin6.sin6_port = port->NotAnInteger;
        }
        else
        {
            addr.sin.sin_port = port->NotAnInteger;
        }
        err = bind(sock, &addr.sa, len);
        if (err < 0)
        {
            LogMsg("mDNSPosixTCPSocketSetup getsockname: %s", strerror(errno));
            return mDNSfalse;
        }
    }

    socklen_t addrlen = sizeof addr;
    err = getsockname(sock, (struct sockaddr *)&addr, &addrlen);
    if (err < 0)
    {
        LogMsg("mDNSPosixTCPSocketSetup getsockname: %s", strerror(errno));
        return mDNSfalse;
    }
    if (sa_family == AF_INET6)
    {
        outTcpPort->NotAnInteger = addr.sin6.sin6_port;

    } else
    {
        outTcpPort->NotAnInteger = addr.sin.sin_port;
    }
    if (port)
        port->NotAnInteger = outTcpPort->NotAnInteger;

    err = setsockopt(sock, IPPROTO_TCP, TCP_NOTSENT_LOWAT, &lowWater, sizeof lowWater);
    if (err < 0)
    {
        LogMsg("mDNSPosixTCPSocketSetup: TCP_NOTSENT_LOWAT failed: %s", strerror(errno));
        return mDNSfalse;
    }

    return mDNStrue;
}

mDNSexport TCPSocket *mDNSPosixDoTCPListenCallback(int fd, mDNSAddr_Type addressType, TCPSocketFlags socketFlags,
                                             TCPAcceptedCallback callback, void *context)
{
    union
    {
        struct sockaddr_in6 sin6;
        struct sockaddr_in sin;
        struct sockaddr sa;
    } address;

    socklen_t slen = sizeof address;
    int remoteSock;
    mDNSAddr addr;
    mDNSIPPort port;
    TCPSocket *sock = mDNSNULL;
    int failed;
    char *nbp;
    int i;
    mDNSu32 lowWater = 16384;
    // When we remember our connection, we remember a name that we can print for logging.   But
    // since we are the listener in this case, we don't /have/ a name for it.   This buffer
    // is used to print the IP address into a human readable string which will serve that purpose
    // for this case.
    char namebuf[INET6_ADDRSTRLEN + 1 + 5 + 1];

    remoteSock = accept(fd, &address.sa, &slen);
    if (remoteSock < 0)
    {
        LogMsg("mDNSPosixDoTCPListenCallback: accept returned %d", remoteSock);
        goto out;
    }

    failed = fcntl(remoteSock, F_SETFL, O_NONBLOCK);
    if (failed < 0)
    {
        close(remoteSock);
        LogMsg("mDNSPosixDoTCPListenCallback: fcntl returned %d", errno);
        goto out;
    }

    failed = setsockopt(remoteSock, IPPROTO_TCP, TCP_NOTSENT_LOWAT,
                        &lowWater, sizeof lowWater);
    if (failed < 0)
    {
        close(remoteSock);
        LogMsg("mDNSPosixDoTCPListenCallback: TCP_NOTSENT_LOWAT returned %d", errno);
        goto out;
    }
    
    if (address.sa.sa_family == AF_INET6)
    {
        // If we are listening on an IPv4/IPv6 socket, the incoming address might be an IPv4-in-IPv6 address
        for (i = 0; i < 10; i++)
        {
            if (address.sin6.sin6_addr.s6_addr[i] != 0)
            {
                addr.type = mDNSAddrType_IPv6;
                goto nope;
            }
        }

        // a legit IPv4 address would be ::ffff:a.b.c.d; if there's no ::ffff bit, then it's an IPv6
        // address with a really weird prefix.
        if (address.sin6.sin6_addr.s6_addr[10] != 0xFF || address.sin6.sin6_addr.s6_addr[11] != 0xFF)
        {
            addr.type = mDNSAddrType_IPv6;
        } else if (addressType != mDNSAddrType_None)
        {
            if (inet_ntop(AF_INET, &address.sin6.sin6_addr.s6_addr[12], namebuf, INET6_ADDRSTRLEN + 1) == NULL)
            {
                strcpy(namebuf, ":unknown:");
            }
            LogMsg("mDNSPosixDoTCPListenCallback received an IPv4 connection from %s on an IPv6-only socket.",
                   namebuf);
            close(remoteSock);
            goto out;
        }
        else
        {
            addr.type = mDNSAddrType_IPv4;
        }
    nope:
        if (addr.type == mDNSAddrType_IPv6)
        {
            if (inet_ntop(address.sin6.sin6_family, &address.sin6.sin6_addr, namebuf, INET6_ADDRSTRLEN + 1) == NULL)
            {
                strcpy(namebuf, ":unknown:");
            }
            memcpy(&addr.ip.v6, &address.sin6.sin6_addr, sizeof addr.ip.v6);
        }
        else
        {
            if (inet_ntop(AF_INET, &address.sin6.sin6_addr.s6_addr[12], namebuf, INET6_ADDRSTRLEN + 1) == NULL)
            {
                strcpy(namebuf, ":unknown:");
            }
            memcpy(&addr.ip.v4, &address.sin6.sin6_addr.s6_addr[12], sizeof addr.ip.v4);
        }
        port.NotAnInteger = address.sin6.sin6_port;
    }
    else if (address.sa.sa_family == AF_INET)
    {
        addr.type = mDNSAddrType_IPv4;
        memcpy(&addr.ip.v4, &address.sin.sin_addr, sizeof addr.ip.v4);
        port.NotAnInteger = address.sin.sin_port;
        if (inet_ntop(AF_INET, &address.sin.sin_addr, namebuf, INET6_ADDRSTRLEN + 1) == NULL)
        {
            strcpy(namebuf, ":unknown:");
        }
    } else {
        LogMsg("mDNSPosixDoTCPListenCallback: connection from unknown address family %d", address.sa.sa_family);
        close(remoteSock);
        goto out;
    }
    nbp = namebuf + strlen(namebuf);
    *nbp++ = '%';
    snprintf(nbp, 6, "%u", ntohs(port.NotAnInteger));
             
    sock = mDNSPlatformTCPAccept(socketFlags, remoteSock);
    if (sock == NULL)
    {
        LogMsg("mDNSPosixDoTCPListenCallback: mDNSPlatformTCPAccept returned NULL; dropping connection from %s",
               namebuf);
        close(remoteSock);
        goto out;
    }
    callback(sock, &addr, &port, namebuf, context);
out:
    return sock;
}

mDNSexport mDNSBool mDNSPosixTCPListen(int *fd, mDNSAddr_Type addrtype, mDNSIPPort *port, mDNSAddr *addr,
                                       mDNSBool reuseAddr, int queueLength)

{
    union
    {
        struct sockaddr_in6 sin6;
        struct sockaddr_in sin;
        struct sockaddr sa;
    } address;

    int failed;
    int sock;
    int one = 1;
    socklen_t sock_len;

    // We require an addrtype parameter because addr is allowed to be null, but they have to agree.
    if (addr != mDNSNULL && addr->type != addrtype)
    {
        LogMsg("mDNSPlatformTCPListen: address type conflict: %d:%d", addr->type, addrtype);
        return mDNSfalse;
    }
    if (port == mDNSNULL)
    {
        LogMsg("mDNSPlatformTCPListen: port must not be NULL");
        return mDNSfalse;
    }

    mDNSPlatformMemZero(&address, sizeof address);
    if (addrtype == mDNSAddrType_None || addrtype == mDNSAddrType_IPv6)
    {
        // Set up DNS listener socket
        if (addr != mDNSNULL)
        {
            memcpy(&address.sin6.sin6_addr.s6_addr, &addr->ip, sizeof address.sin6.sin6_addr.s6_addr);
        }
        address.sin6.sin6_port = port->NotAnInteger;

        sock_len = sizeof address.sin6;
        address.sin6.sin6_family = AF_INET6;
    }
    else if (addrtype == mDNSAddrType_IPv4)
    {
        if (addr != mDNSNULL)
        {
            memcpy(&address.sin.sin_addr.s_addr, &addr->ip, sizeof address.sin.sin_addr.s_addr);
        }
        address.sin.sin_port = port->NotAnInteger;
        sock_len = sizeof address.sin;
        address.sin.sin_family = AF_INET;
    }
    else
    {
        LogMsg("mDNSPlatformTCPListen: invalid address type: %d", addrtype);
        return mDNSfalse;
    }
#ifndef NOT_HAVE_SA_LEN
    address.sa.sa_len = sock_len;
#endif
    sock = socket(address.sa.sa_family, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0)
    {
        LogMsg("mDNSPlatformTCPListen: socket call failed: %s", strerror(errno));
        return mDNSfalse;
    }
    *fd = sock;

    // The reuseAddr flag is used to indicate that we want to listen on this port even if
    // there are still lingering sockets.   We will still fail if there is another listener.
    // Note that this requires SO_REUSEADDR, not SO_REUSEPORT, which does not have special
    // handling for lingering sockets.
    if (reuseAddr)
    {
        failed = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        if (failed < 0)
        {
            LogMsg("mDNSPlatformTCPListen: SO_REUSEADDR failed %s", strerror(errno));
            return mDNSfalse;
        }
    }

    // Bind to the port and (if provided) address
    failed = bind(sock, &address.sa, sock_len);
    if (failed < 0)
    {
        LogMsg("mDNSPlatformTCPListen: bind failed %s", strerror(errno));
        return mDNSfalse;
    }

    // If there was no specified listen port, we need to know what port we got.
    if (port->NotAnInteger == 0)
    {
        mDNSPlatformMemZero(&address, sizeof address);
        failed = getsockname(sock, &address.sa, &sock_len);
        if (failed < 0)
        {
            LogMsg("mDNSRelay: getsockname failed: %s", strerror(errno));
            return mDNSfalse;
        }
        if (address.sa.sa_family == AF_INET)
        {
            port->NotAnInteger = address.sin.sin_port;
        }
        else
        {
            port->NotAnInteger = address.sin6.sin6_port;
        }
    }

    failed = listen(sock, queueLength);
    if (failed < 0)
    {
        LogMsg("mDNSPlatformTCPListen: listen failed: %s", strerror(errno));
        return mDNSfalse;
    }
    return mDNStrue;
}

mDNSexport long mDNSPosixReadTCP(int fd, void *buf, unsigned long buflen, mDNSBool *closed)
{
    static int CLOSEDcount = 0;
    static int EAGAINcount = 0;
    ssize_t nread = recv(fd, buf, buflen, 0);

    if (nread > 0)
    {
        CLOSEDcount = 0; 
        EAGAINcount = 0; 
    } // On success, clear our error counters
    else if (nread == 0)
    {
        *closed = mDNStrue;
        if ((++CLOSEDcount % 20) == 0)
        {
            LogMsg("ERROR: mDNSPosixReadFromSocket - recv %d got CLOSED %d times", fd, CLOSEDcount); 
            assert(CLOSEDcount < 1000);
            // Recovery Mechanism to bail mDNSResponder out of trouble: Instead of logging the same error
            // msg multiple times, crash mDNSResponder using assert() and restart fresh. See advantages
            // below:
            // 1.Better User Experience 
            // 2.CrashLogs frequency can be monitored 
            // 3.StackTrace can be used for more info
        }
    }
    // else nread is negative -- see what kind of error we got
    else if (errno == ECONNRESET)
    {
        nread = 0; *closed = mDNStrue;
    }
    else if (errno != EAGAIN)
    {
        LogMsg("ERROR: mDNSPosixReadFromSocket - recv: %d (%s)", errno, strerror(errno));
        nread = -1;
    }
    else
    { // errno is EAGAIN (EWOULDBLOCK) -- no data available
        nread = 0;
        if ((++EAGAINcount % 1000) == 0)
        {
            LogMsg("ERROR: mDNSPosixReadFromSocket - recv %d got EAGAIN %d times", fd, EAGAINcount);
            sleep(1);
        }
    }
    return nread;
}

mDNSexport long mDNSPosixWriteTCP(int fd, const char *msg, unsigned long len)
{
    ssize_t result;
    long nsent;

    result = write(fd, msg, len);
    if (result < 0)
    {
        if (errno == EAGAIN)
        {
            nsent = 0;
        }
        else
        {
            LogMsg("ERROR: mDNSPosixWriteTCP - send %s", strerror(errno)); nsent = -1;
        }
    }
    else
    {
        nsent = (long)result;
    }
    return nsent;
}
