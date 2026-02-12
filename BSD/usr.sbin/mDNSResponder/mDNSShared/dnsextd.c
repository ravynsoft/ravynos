/*
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
 */

#if __APPLE__
// In Mac OS X 10.5 and later trying to use the daemon function gives a “‘daemon’ is deprecated”
// error, which prevents compilation because we build with "-Werror".
// Since this is supposed to be portable cross-platform code, we don't care that daemon is
// deprecated on Mac OS X 10.5, so we use this preprocessor trick to eliminate the error message.
#define daemon yes_we_know_that_daemon_is_deprecated_in_os_x_10_5_thankyou
#endif

#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <errno.h>

#if __APPLE__
#undef daemon
extern int daemon(int, int);
#endif

// Solaris doesn't have daemon(), so we define it here
#ifdef NOT_HAVE_DAEMON
#include "../mDNSPosix/mDNSUNP.h"       // For daemon()
#endif // NOT_HAVE_DAEMON

#include "dnsextd.h"
#include "../mDNSShared/uds_daemon.h"
#include "../mDNSShared/dnssd_ipc.h"
#include "../mDNSCore/uDNS.h"
#include "../mDNSShared/DebugServices.h"

// Compatibility workaround
#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

//
// Constants
//
mDNSexport const char ProgramName[] = "dnsextd";

#define LOOPBACK                    "127.0.0.1"
#if !defined(LISTENQ)
#   define LISTENQ                  128                 // tcp connection backlog
#endif
#define RECV_BUFLEN                 9000
#define LEASETABLE_INIT_NBUCKETS    256                 // initial hashtable size (doubles as table fills)
#define EXPIRATION_INTERVAL         300                 // check for expired records every 5 minutes
#define SRV_TTL                     7200                // TTL For _dns-update SRV records
#define CONFIG_FILE                 "/etc/dnsextd.conf"
#define TCP_SOCKET_FLAGS            kTCPSocketFlags_UseTLS

// LLQ Lease bounds (seconds)
#define LLQ_MIN_LEASE (15 * 60)
#define LLQ_MAX_LEASE (120 * 60)
#define LLQ_LEASE_FUDGE 60

// LLQ SOA poll interval (microseconds)
#define LLQ_MONITOR_ERR_INTERVAL (60 * 1000000)
#define LLQ_MONITOR_INTERVAL 250000
#ifdef SIGINFO
#define INFO_SIGNAL SIGINFO
#else
#define INFO_SIGNAL SIGUSR1
#endif

#define SAME_INADDR(x,y) (*((mDNSu32 *)&x) == *((mDNSu32 *)&y))

//
// Data Structures
// Structs/fields that must be locked for thread safety are explicitly commented
//

// args passed to UDP request handler thread as void*

typedef struct
{
    PktMsg pkt;
    struct sockaddr_in cliaddr;
    DaemonInfo *d;
    int sd;
} UDPContext;

// args passed to TCP request handler thread as void*
typedef struct
{
    PktMsg pkt;
    struct sockaddr_in cliaddr;
    TCPSocket *sock;           // socket connected to client
    DaemonInfo *d;
} TCPContext;

// args passed to UpdateAnswerList thread as void*
typedef struct
{
    DaemonInfo *d;
    AnswerListElem *a;
} UpdateAnswerListArgs;

//
// Global Variables
//

// booleans to determine runtime output
// read-only after initialization (no mutex protection)
static mDNSBool foreground = 0;
static mDNSBool verbose = 0;

// globals set via signal handler (accessed exclusively by main select loop and signal handler)
static mDNSBool terminate = 0;
static mDNSBool dumptable = 0;
static mDNSBool hangup    = 0;

// global for config file location
static char *   cfgfile   = NULL;

//
// Logging Routines
// Log messages are delivered to syslog unless -f option specified
//

// common message logging subroutine
mDNSlocal void PrintLog(const char *buffer)
{
    if (foreground)
    {
        fprintf(stderr,"%s\n", buffer);
        fflush(stderr);
    }
    else
    {
        openlog("dnsextd", LOG_CONS, LOG_DAEMON);
        syslog(LOG_ERR, "%s", buffer);
        closelog();
    }
}

// Verbose Logging (conditional on -v option)
mDNSlocal void VLog(const char *format, ...)
{
    char buffer[512];
    va_list ptr;

    if (!verbose) return;
    va_start(ptr,format);
    buffer[mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr)] = 0;
    va_end(ptr);
    PrintLog(buffer);
}

// Unconditional Logging
mDNSlocal void Log(const char *format, ...)
{
    char buffer[512];
    va_list ptr;

    va_start(ptr,format);
    buffer[mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr)] = 0;
    va_end(ptr);
    PrintLog(buffer);
}

// Error Logging
// prints message "dnsextd <function>: <operation> - <error message>"
// must be compiled w/ -D_REENTRANT for thread-safe errno usage
mDNSlocal void LogErr(const char *fn, const char *operation)
{
    char buf[512], errbuf[256];
    strerror_r(errno, errbuf, sizeof(errbuf));
    snprintf(buf, sizeof(buf), "%s: %s - %s", fn, operation, errbuf);
    PrintLog(buf);
}

//
// Networking Utility Routines
//

// Convert DNS Message Header from Network to Host byte order
mDNSlocal void HdrNToH(PktMsg *pkt)
{
    // Read the integer parts which are in IETF byte-order (MSB first, LSB second)
    mDNSu8 *ptr = (mDNSu8 *)&pkt->msg.h.numQuestions;
    pkt->msg.h.numQuestions   = (mDNSu16)((mDNSu16)ptr[0] <<  8 | ptr[1]);
    pkt->msg.h.numAnswers     = (mDNSu16)((mDNSu16)ptr[2] <<  8 | ptr[3]);
    pkt->msg.h.numAuthorities = (mDNSu16)((mDNSu16)ptr[4] <<  8 | ptr[5]);
    pkt->msg.h.numAdditionals = (mDNSu16)((mDNSu16)ptr[6] <<  8 | ptr[7]);
}

// Convert DNS Message Header from Host to Network byte order
mDNSlocal void HdrHToN(PktMsg *pkt)
{
    mDNSu16 numQuestions   = pkt->msg.h.numQuestions;
    mDNSu16 numAnswers     = pkt->msg.h.numAnswers;
    mDNSu16 numAuthorities = pkt->msg.h.numAuthorities;
    mDNSu16 numAdditionals = pkt->msg.h.numAdditionals;
    mDNSu8 *ptr = (mDNSu8 *)&pkt->msg.h.numQuestions;

    // Put all the integer values in IETF byte-order (MSB first, LSB second)
    *ptr++ = (mDNSu8)(numQuestions   >> 8);
    *ptr++ = (mDNSu8)(numQuestions   &  0xFF);
    *ptr++ = (mDNSu8)(numAnswers     >> 8);
    *ptr++ = (mDNSu8)(numAnswers     &  0xFF);
    *ptr++ = (mDNSu8)(numAuthorities >> 8);
    *ptr++ = (mDNSu8)(numAuthorities &  0xFF);
    *ptr++ = (mDNSu8)(numAdditionals >> 8);
    *ptr++ = (mDNSu8)(numAdditionals &  0xFF);
}


// Add socket to event loop

mDNSlocal mStatus AddSourceToEventLoop( DaemonInfo * self, TCPSocket *sock, EventCallback callback, void *context )
{
    EventSource * newSource;
    mStatus err = mStatus_NoError;

    if ( self->eventSources.LinkOffset == 0 )
    {
        InitLinkedList( &self->eventSources, offsetof( EventSource, next));
    }

    newSource = ( EventSource*) malloc( sizeof *newSource );
    if ( newSource == NULL )
    {
        err = mStatus_NoMemoryErr;
        goto exit;
    }

    newSource->callback = callback;
    newSource->context = context;
    newSource->sock = sock;
    newSource->fd = mDNSPlatformTCPGetFD( sock );

    AddToTail( &self->eventSources, newSource );

exit:

    return err;
}


// Remove socket from event loop

mDNSlocal mStatus RemoveSourceFromEventLoop( DaemonInfo * self, TCPSocket *sock )
{
    EventSource *   source;
    mStatus err;

    for ( source = ( EventSource* ) self->eventSources.Head; source; source = source->next )
    {
        if ( source->sock == sock )
        {
            RemoveFromList( &self->eventSources, source );

            free( source );
            err = mStatus_NoError;
            goto exit;
        }
    }

    err = mStatus_NoSuchNameErr;

exit:

    return err;
}

// create a socket connected to nameserver
// caller terminates connection via close()
mDNSlocal TCPSocket *ConnectToServer(DaemonInfo *d)
{
    int ntries = 0, retry = 0;

    while (1)
    {
        mDNSIPPort port = zeroIPPort;
        int fd;

        TCPSocket *sock = mDNSPlatformTCPSocket(0, mDNSAddrType_IPv4, &port, NULL, mDNSfalse );
        if ( !sock ) { LogErr("ConnectToServer", "socket");  return NULL; }
        fd = mDNSPlatformTCPGetFD( sock );
        if (!connect( fd, (struct sockaddr *)&d->ns_addr, sizeof(d->ns_addr))) return sock;
        mDNSPlatformTCPCloseConnection( sock );
        if (++ntries < 10)
        {
            LogErr("ConnectToServer", "connect");
            Log("ConnectToServer - retrying connection");
            if (!retry) retry = 500000 + random() % 500000;
            usleep(retry);
            retry *= 2;
        }
        else { Log("ConnectToServer - %d failed attempts.  Aborting.", ntries); return NULL; }
    }
}

// send an entire block of data over a connected socket
mDNSlocal int MySend(TCPSocket *sock, const void *msg, int len)
{
    int selectval, n, nsent = 0;
    fd_set wset;
    struct timeval timeout = { 3, 0 };  // until we remove all calls from main thread, keep timeout short

    while (nsent < len)
    {
        int fd;

        FD_ZERO(&wset);

        fd = mDNSPlatformTCPGetFD( sock );

        FD_SET( fd, &wset );
        selectval = select( fd+1, NULL, &wset, NULL, &timeout);
        if (selectval < 0) { LogErr("MySend", "select");  return -1; }
        if (!selectval || !FD_ISSET(fd, &wset)) { Log("MySend - timeout"); return -1; }

        n = mDNSPlatformWriteTCP( sock, ( char* ) msg + nsent, len - nsent);

        if (n < 0) { LogErr("MySend", "send");  return -1; }
        nsent += n;
    }
    return 0;
}

// Transmit a DNS message, prefixed by its length, over TCP, blocking if necessary
mDNSlocal int SendPacket(TCPSocket *sock, PktMsg *pkt)
{
    // send the lenth, in network byte order
    mDNSu16 len = htons((mDNSu16)pkt->len);
    if (MySend(sock, &len, sizeof(len)) < 0) return -1;

    // send the message
    VLog("SendPacket Q:%d A:%d A:%d A:%d ",
         ntohs(pkt->msg.h.numQuestions),
         ntohs(pkt->msg.h.numAnswers),
         ntohs(pkt->msg.h.numAuthorities),
         ntohs(pkt->msg.h.numAdditionals));
    return MySend(sock, &pkt->msg, pkt->len);
}

// Receive len bytes, waiting until we have all of them.
// Returns number of bytes read (which should always be the number asked for).
static int my_recv(TCPSocket *sock, void *const buf, const int len, mDNSBool * closed)
{
    // Don't use "MSG_WAITALL"; it returns "Invalid argument" on some Linux versions;
    // use an explicit while() loop instead.
    // Also, don't try to do '+=' arithmetic on the original "void *" pointer --
    // arithmetic on "void *" pointers is compiler-dependent.

    fd_set rset;
    struct timeval timeout = { 3, 0 };  // until we remove all calls from main thread, keep timeout short
    int selectval, remaining = len;
    char *ptr = (char *)buf;
    ssize_t num_read;

    while (remaining)
    {
        int fd;

        fd = mDNSPlatformTCPGetFD( sock );

        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        selectval = select(fd+1, &rset, NULL, NULL, &timeout);
        if (selectval < 0) { LogErr("my_recv", "select");  return -1; }
        if (!selectval || !FD_ISSET(fd, &rset))
        {
            Log("my_recv - timeout");
            return -1;
        }

        num_read = mDNSPlatformReadTCP( sock, ptr, remaining, closed );

        if (((num_read == 0) && *closed) || (num_read < 0) || (num_read > remaining)) return -1;
        if (num_read == 0) return 0;
        ptr       += num_read;
        remaining -= num_read;
    }
    return(len);
}

// Return a DNS Message read off of a TCP socket, or NULL on failure
// If storage is non-null, result is placed in that buffer.  Otherwise,
// returned value is allocated with Malloc, and contains sufficient extra
// storage for a Lease OPT RR

mDNSlocal PktMsg*
RecvPacket
(
    TCPSocket * sock,
    PktMsg      *   storage,
    mDNSBool    *   closed
)
{
    int nread;
    int allocsize;
    mDNSu16 msglen = 0;
    PktMsg      *   pkt = NULL;
    unsigned int srclen;
    int fd;
    mStatus err = 0;

    fd = mDNSPlatformTCPGetFD( sock );

    nread = my_recv( sock, &msglen, sizeof( msglen ), closed );

    require_action_quiet( nread != -1, exit, err = mStatus_UnknownErr );
    require_action_quiet( nread > 0, exit, err = mStatus_NoError );

    msglen = ntohs( msglen );
    require_action_quiet( nread == sizeof( msglen ), exit, err = mStatus_UnknownErr; Log( "Could not read length field of message") );

    if ( storage )
    {
        require_action_quiet( msglen <= sizeof( storage->msg ), exit, err = mStatus_UnknownErr; Log( "RecvPacket: provided buffer too small." ) );
        pkt = storage;
    }
    else
    {
        // buffer extra space to add an OPT RR

        if ( msglen > sizeof(DNSMessage))
        {
            allocsize = sizeof(PktMsg) - sizeof(DNSMessage) + msglen;
        }
        else
        {
            allocsize = sizeof(PktMsg);
        }

        pkt = malloc(allocsize);
        require_action_quiet( pkt, exit, err = mStatus_NoMemoryErr; LogErr( "RecvPacket", "malloc" ) );
        mDNSPlatformMemZero( pkt, sizeof( *pkt ) );
    }

    pkt->len = msglen;
    srclen = sizeof(pkt->src);

    if ( getpeername( fd, ( struct sockaddr* ) &pkt->src, &srclen ) || ( srclen != sizeof( pkt->src ) ) )
    {
        LogErr("RecvPacket", "getpeername");
        mDNSPlatformMemZero(&pkt->src, sizeof(pkt->src));
    }

    nread = my_recv(sock, &pkt->msg, msglen, closed );
    require_action_quiet( nread >= 0, exit, err = mStatus_UnknownErr ; LogErr( "RecvPacket", "recv" ) );
    require_action_quiet( nread == msglen, exit, err = mStatus_UnknownErr ; Log( "Could not read entire message" ) );
    require_action_quiet( pkt->len >= sizeof( DNSMessageHeader ), exit, err = mStatus_UnknownErr ; Log( "RecvPacket: Message too short (%d bytes)", pkt->len ) );

exit:

    if ( err && pkt )
    {
        if ( pkt != storage )
        {
            free(pkt);
        }

        pkt = NULL;
    }

    return pkt;
}


mDNSlocal DNSZone*
FindZone
(
    DaemonInfo  *   self,
    domainname  *   name
)
{
    DNSZone * zone;

    for ( zone = self->zones; zone; zone = zone->next )
    {
        if ( SameDomainName( &zone->name, name ) )
        {
            break;
        }
    }

    return zone;
}


mDNSlocal mDNSBool
ZoneHandlesName
(
    const domainname * zname,
    const domainname * dname
)
{
    mDNSu16 i = DomainNameLength( zname );
    mDNSu16 j = DomainNameLength( dname );

    if ( ( i == ( MAX_DOMAIN_NAME + 1 ) ) || ( j == ( MAX_DOMAIN_NAME + 1 ) ) || ( i > j )  || ( memcmp( zname->c, dname->c + ( j - i ), i ) != 0 ) )
    {
        return mDNSfalse;
    }

    return mDNStrue;
}


mDNSlocal mDNSBool IsQuery( PktMsg * pkt )
{
    return ( pkt->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask ) == (mDNSu8) ( kDNSFlag0_QR_Query | kDNSFlag0_OP_StdQuery );
}


mDNSlocal mDNSBool IsUpdate( PktMsg * pkt )
{
    return ( pkt->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask ) == (mDNSu8) ( kDNSFlag0_OP_Update );
}


mDNSlocal mDNSBool IsNotify(PktMsg *pkt)
{
    return ( pkt->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask ) == ( mDNSu8) ( kDNSFlag0_OP_Notify );
}


mDNSlocal mDNSBool IsLLQRequest(PktMsg *pkt)
{
    const mDNSu8 *ptr = NULL, *end = (mDNSu8 *)&pkt->msg + pkt->len;
    LargeCacheRecord lcr;
    int i;
    mDNSBool result = mDNSfalse;

    HdrNToH(pkt);
    if ((mDNSu8)(pkt->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask) != (mDNSu8)(kDNSFlag0_QR_Query | kDNSFlag0_OP_StdQuery)) goto end;

    if (!pkt->msg.h.numAdditionals) goto end;
    ptr = LocateAdditionals(&pkt->msg, end);
    if (!ptr) goto end;

    bzero(&lcr, sizeof(lcr));
    // find last Additional info.
    for (i = 0; i < pkt->msg.h.numAdditionals; i++)
    {
        ptr = GetLargeResourceRecord(NULL, &pkt->msg, ptr, end, 0, kDNSRecordTypePacketAdd, &lcr);
        if (!ptr) { Log("Unable to read additional record"); goto end; }
    }

    if ( lcr.r.resrec.rrtype == kDNSType_OPT && lcr.r.resrec.rdlength >= DNSOpt_LLQData_Space && lcr.r.resrec.rdata->u.opt[0].opt == kDNSOpt_LLQ )
    {
        result = mDNStrue;
    }

end:
    HdrHToN(pkt);
    return result;
}

// !!!KRS implement properly
mDNSlocal mDNSBool IsLLQAck(PktMsg *pkt)
{
    if ((pkt->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask ) == (mDNSu8) ( kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery ) &&
        pkt->msg.h.numQuestions && !pkt->msg.h.numAnswers && !pkt->msg.h.numAuthorities) return mDNStrue;
    return mDNSfalse;
}


mDNSlocal mDNSBool
IsPublicSRV
(
    DaemonInfo  *   self,
    DNSQuestion *   q
)
{
    DNameListElem   *   elem;
    mDNSBool ret     = mDNSfalse;
    int i       = ( int ) DomainNameLength( &q->qname ) - 1;

    for ( elem = self->public_names; elem; elem = elem->next )
    {
        int j = ( int ) DomainNameLength( &elem->name ) - 1;

        if ( i > j )
        {
            for ( ; i >= 0; i--, j-- )
            {
                if ( q->qname.c[ i ] != elem->name.c[ j ] )
                {
                    ret = mDNStrue;
                    goto exit;
                }
            }
        }
    }

exit:

    return ret;
}


mDNSlocal void
SetZone
(
    DaemonInfo  * self,
    PktMsg      * pkt
)
{
    domainname zname;
    const mDNSu8    *   ptr = pkt->msg.data;
    mDNSBool exception = mDNSfalse;

    // Initialize

    pkt->zone           = NULL;
    pkt->isZonePublic   = mDNStrue;
    zname.c[0]          = '\0';

    // Figure out what type of packet this is

    if ( IsQuery( pkt ) )
    {
        DNSQuestion question;

        // It's a query

        getQuestion( &pkt->msg, ptr, ( ( mDNSu8* ) &pkt->msg ) + pkt->len, NULL, &question );

        AppendDomainName( &zname, &question.qname );

        exception = ( ( question.qtype == kDNSType_SOA ) || ( question.qtype == kDNSType_NS ) || ( ( question.qtype == kDNSType_SRV ) && IsPublicSRV( self, &question ) ) );
    }
    else if ( IsUpdate( pkt ) )
    {
        DNSQuestion question;

        // It's an update.  The format of the zone section is the same as the format for the question section
        // according to RFC 2136, so we'll just treat this as a question so we can get at the zone.

        getQuestion( &pkt->msg, ptr, ( ( mDNSu8* ) &pkt->msg ) + pkt->len, NULL, &question );

        AppendDomainName( &zname, &question.qname );

        exception = mDNSfalse;
    }

    if ( zname.c[0] != '\0' )
    {
        // Find the right zone

        for ( pkt->zone = self->zones; pkt->zone; pkt->zone = pkt->zone->next )
        {
            if ( ZoneHandlesName( &pkt->zone->name, &zname ) )
            {
                VLog( "found correct zone %##s for query", pkt->zone->name.c );

                pkt->isZonePublic = ( ( pkt->zone->type == kDNSZonePublic ) || exception );

                VLog( "zone %##s is %s", pkt->zone->name.c, ( pkt->isZonePublic ) ? "public" : "private" );

                break;
            }
        }
    }
}


mDNSlocal int
UDPServerTransaction(const DaemonInfo *d, const PktMsg *request, PktMsg *reply, mDNSBool *trunc)
{
    fd_set rset;
    struct timeval timeout = { 3, 0 };   // until we remove all calls from main thread, keep timeout short
    int sd;
    int res;
    mStatus err = mStatus_NoError;

    // Initialize

    *trunc = mDNSfalse;

    // Create a socket

    sd = socket( AF_INET, SOCK_DGRAM, 0 );
    require_action( sd >= 0, exit, err = mStatus_UnknownErr; LogErr( "UDPServerTransaction", "socket" ) );

    // Send the packet to the nameserver

    VLog("UDPServerTransaction Q:%d A:%d A:%d A:%d ",
         ntohs(request->msg.h.numQuestions),
         ntohs(request->msg.h.numAnswers),
         ntohs(request->msg.h.numAuthorities),
         ntohs(request->msg.h.numAdditionals));
    res = sendto( sd, (char *)&request->msg, request->len, 0, ( struct sockaddr* ) &d->ns_addr, sizeof( d->ns_addr ) );
    require_action( res == (int) request->len, exit, err = mStatus_UnknownErr; LogErr( "UDPServerTransaction", "sendto" ) );

    // Wait for reply

    FD_ZERO( &rset );
    FD_SET( sd, &rset );
    res = select( sd + 1, &rset, NULL, NULL, &timeout );
    require_action( res >= 0, exit, err = mStatus_UnknownErr; LogErr( "UDPServerTransaction", "select" ) );
    require_action( ( res > 0 ) && FD_ISSET( sd, &rset ), exit, err = mStatus_UnknownErr; Log( "UDPServerTransaction - timeout" ) );

    // Receive reply

    reply->len = recvfrom( sd, &reply->msg, sizeof(reply->msg), 0, NULL, NULL );
    require_action( ( ( int ) reply->len ) >= 0, exit, err = mStatus_UnknownErr; LogErr( "UDPServerTransaction", "recvfrom" ) );
    require_action( reply->len >= sizeof( DNSMessageHeader ), exit, err = mStatus_UnknownErr; Log( "UDPServerTransaction - Message too short (%d bytes)", reply->len ) );

    // Check for truncation bit

    if ( reply->msg.h.flags.b[0] & kDNSFlag0_TC )
    {
        *trunc = mDNStrue;
    }

exit:

    if ( sd >= 0 )
    {
        close( sd );
    }

    return err;
}

//
// Dynamic Update Utility Routines
//

// check if a request and server response complete a successful dynamic update
mDNSlocal mDNSBool SuccessfulUpdateTransaction(PktMsg *request, PktMsg *reply)
{
    char buf[32];
    char *vlogmsg = NULL;

    // check messages
    if (!request || !reply) { vlogmsg = "NULL message"; goto failure; }
    if (request->len < sizeof(DNSMessageHeader) || reply->len < sizeof(DNSMessageHeader)) { vlogmsg = "Malformatted message"; goto failure; }

    // check request operation
    if ((request->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask) != (request->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask))
    { vlogmsg = "Request opcode not an update"; goto failure; }

    // check result
    if ((reply->msg.h.flags.b[1] & kDNSFlag1_RC_Mask)) { vlogmsg = "Reply contains non-zero rcode";  goto failure; }
    if ((reply->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask) != (kDNSFlag0_OP_Update | kDNSFlag0_QR_Response))
    { vlogmsg = "Reply opcode not an update response"; goto failure; }

    VLog("Successful update from %s", inet_ntop(AF_INET, &request->src.sin_addr, buf, 32));
    return mDNStrue;

failure:
    VLog("Request %s: %s", inet_ntop(AF_INET, &request->src.sin_addr, buf, 32), vlogmsg);
    return mDNSfalse;
}

// Allocate an appropriately sized CacheRecord and copy data from original.
// Name pointer in CacheRecord object is set to point to the name specified
//
mDNSlocal CacheRecord *CopyCacheRecord(const CacheRecord *orig, domainname *name)
{
    CacheRecord *cr;
    size_t size = sizeof(*cr);
    if (orig->resrec.rdlength > InlineCacheRDSize) size += orig->resrec.rdlength - InlineCacheRDSize;
    cr = malloc(size);
    if (!cr) { LogErr("CopyCacheRecord", "malloc"); return NULL; }
    memcpy(cr, orig, size);
    cr->resrec.rdata = (RData*)&cr->smallrdatastorage;
    cr->resrec.name = name;

    return cr;
}


//
// Lease Hashtable Utility Routines
//

// double hash table size
// caller must lock table prior to invocation
mDNSlocal void RehashTable(DaemonInfo *d)
{
    RRTableElem *ptr, *tmp, **new;
    int i, bucket, newnbuckets = d->nbuckets * 2;

    VLog("Rehashing lease table (new size %d buckets)", newnbuckets);
    new = malloc(sizeof(RRTableElem *) * newnbuckets);
    if (!new) { LogErr("RehashTable", "malloc");  return; }
    mDNSPlatformMemZero(new, newnbuckets * sizeof(RRTableElem *));

    for (i = 0; i < d->nbuckets; i++)
    {
        ptr = d->table[i];
        while (ptr)
        {
            bucket = ptr->rr.resrec.namehash % newnbuckets;
            tmp = ptr;
            ptr = ptr->next;
            tmp->next = new[bucket];
            new[bucket] = tmp;
        }
    }
    d->nbuckets = newnbuckets;
    free(d->table);
    d->table = new;
}

// print entire contents of hashtable, invoked via SIGINFO
mDNSlocal void PrintLeaseTable(DaemonInfo *d)
{
    int i;
    RRTableElem *ptr;
    char rrbuf[MaxMsg], addrbuf[16];
    struct timeval now;
    int hr, min, sec;

    if (gettimeofday(&now, NULL)) { LogErr("PrintTable", "gettimeofday"); return; }
    if (pthread_mutex_lock(&d->tablelock)) { LogErr("PrintTable", "pthread_mutex_lock"); return; }

    Log("Dumping Lease Table Contents (table contains %d resource records)", d->nelems);
    for (i = 0; i < d->nbuckets; i++)
    {
        for (ptr = d->table[i]; ptr; ptr = ptr->next)
        {
            hr = ((ptr->expire - now.tv_sec) / 60) / 60;
            min = ((ptr->expire - now.tv_sec) / 60) % 60;
            sec = (ptr->expire - now.tv_sec) % 60;
            Log("Update from %s, Expires in %d:%d:%d\n\t%s", inet_ntop(AF_INET, &ptr->cli.sin_addr, addrbuf, 16), hr, min, sec,
                GetRRDisplayString_rdb(&ptr->rr.resrec, &ptr->rr.resrec.rdata->u, rrbuf));
        }
    }
    pthread_mutex_unlock(&d->tablelock);
}

//
// Startup SRV Registration Routines
// Register _dns-update._udp/_tcp.<zone> SRV records indicating the port on which
// the daemon accepts requests
//

// delete all RRS of a given name/type
mDNSlocal mDNSu8 *putRRSetDeletion(DNSMessage *msg, mDNSu8 *ptr, mDNSu8 *limit,  ResourceRecord *rr)
{
    ptr = putDomainNameAsLabels(msg, ptr, limit, rr->name);
    if (!ptr || ptr + 10 >= limit) return NULL;  // out of space
    ptr[0] = (mDNSu8)(rr->rrtype  >> 8);
    ptr[1] = (mDNSu8)(rr->rrtype  &  0xFF);
    ptr[2] = (mDNSu8)((mDNSu16)kDNSQClass_ANY >> 8);
    ptr[3] = (mDNSu8)((mDNSu16)kDNSQClass_ANY &  0xFF);
    mDNSPlatformMemZero(ptr+4, sizeof(rr->rroriginalttl) + sizeof(rr->rdlength)); // zero ttl/rdata
    msg->h.mDNS_numUpdates++;
    return ptr + 10;
}

mDNSlocal mDNSu8 *PutUpdateSRV(DaemonInfo *d, DNSZone * zone, PktMsg *pkt, mDNSu8 *ptr, char *regtype, mDNSIPPort port, mDNSBool registration)
{
    AuthRecord rr;
    char hostname[1024], buf[MaxMsg];
    mDNSu8 *end = (mDNSu8 *)&pkt->msg + sizeof(DNSMessage);

    ( void ) d;

    mDNS_SetupResourceRecord(&rr, NULL, 0, kDNSType_SRV, SRV_TTL, kDNSRecordTypeUnique, AuthRecordAny, NULL, NULL);
    rr.resrec.rrclass = kDNSClass_IN;
    rr.resrec.rdata->u.srv.priority = 0;
    rr.resrec.rdata->u.srv.weight   = 0;
    rr.resrec.rdata->u.srv.port     = port;
    if (gethostname(hostname, 1024) < 0 || !MakeDomainNameFromDNSNameString(&rr.resrec.rdata->u.srv.target, hostname))
        rr.resrec.rdata->u.srv.target.c[0] = '\0';

    MakeDomainNameFromDNSNameString(&rr.namestorage, regtype);
    AppendDomainName(&rr.namestorage, &zone->name);
    VLog("%s  %s", registration ? "Registering SRV record" : "Deleting existing RRSet",
         GetRRDisplayString_rdb(&rr.resrec, &rr.resrec.rdata->u, buf));
    if (registration) ptr = PutResourceRecord(&pkt->msg, ptr, &pkt->msg.h.mDNS_numUpdates, &rr.resrec);
    else ptr = putRRSetDeletion(&pkt->msg, ptr, end, &rr.resrec);
    return ptr;
}


// perform dynamic update.
// specify deletion by passing false for the register parameter, otherwise register the records.
mDNSlocal int UpdateSRV(DaemonInfo *d, mDNSBool registration)
{
    TCPSocket *sock = NULL;
    DNSZone * zone;
    int err = mStatus_NoError;

    sock = ConnectToServer( d );
    require_action( sock, exit, err = mStatus_UnknownErr; Log( "UpdateSRV: ConnectToServer failed" ) );

    for ( zone = d->zones; zone; zone = zone->next )
    {
        PktMsg pkt;
        mDNSu8 *ptr = pkt.msg.data;
        mDNSu8 *end = (mDNSu8 *)&pkt.msg + sizeof(DNSMessage);
        PktMsg *reply = NULL;
        mDNSBool closed;
        mDNSBool ok;

        // Initialize message
        InitializeDNSMessage(&pkt.msg.h, zeroID, UpdateReqFlags);
        pkt.src.sin_addr.s_addr = zerov4Addr.NotAnInteger; // address field set solely for verbose logging in subroutines
        pkt.src.sin_family = AF_INET;

        // format message body
        ptr = putZone(&pkt.msg, ptr, end, &zone->name, mDNSOpaque16fromIntVal(kDNSClass_IN));
        require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );

        if ( zone->type == kDNSZonePrivate )
        {
            ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-update-tls._tcp.", d->private_port, registration);
            require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
            ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-query-tls._tcp.", d->private_port, registration);
            require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
            ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-llq-tls._tcp.", d->private_port, registration);
            require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );

            if ( !registration )
            {
                ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-update._udp.", d->llq_port, registration);
                require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
                ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-llq._udp.", d->llq_port, registration);
                require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
            }
        }
        else
        {
            if ( !registration )
            {
                ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-update-tls.", d->private_port, registration);
                require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
                ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-query-tls.", d->private_port, registration);
                require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
                ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-llq-tls.", d->private_port, registration);
                require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
            }

            ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-update._udp.", d->llq_port, registration);
            require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
            ptr = PutUpdateSRV(d, zone, &pkt, ptr, "_dns-llq._udp.", d->llq_port, registration);
            require_action( ptr, exit, err = mStatus_UnknownErr; Log("UpdateSRV: Error constructing lease expiration update" ) );
        }

        HdrHToN(&pkt);

        if ( zone->updateKeys )
        {
            DNSDigest_SignMessage( &pkt.msg, &ptr, zone->updateKeys, 0 );
            require_action( ptr, exit, Log("UpdateSRV: Error constructing lease expiration update" ) );
        }

        pkt.len = ptr - (mDNSu8 *)&pkt.msg;

        // send message, receive reply

        err = SendPacket( sock, &pkt );
        require_action( !err, exit, Log( "UpdateSRV: SendPacket failed" ) );

        reply = RecvPacket( sock, NULL, &closed );
        require_action( reply, exit, err = mStatus_UnknownErr; Log( "UpdateSRV: RecvPacket returned NULL" ) );

        ok = SuccessfulUpdateTransaction( &pkt, reply );

        if ( !ok )
        {
            Log("SRV record registration failed with rcode %d", reply->msg.h.flags.b[1] & kDNSFlag1_RC_Mask);
        }

        free( reply );
    }

exit:

    if ( sock )
    {
        mDNSPlatformTCPCloseConnection( sock );
    }

    return err;
}

// wrapper routines/macros
#define ClearUpdateSRV(d) UpdateSRV(d, 0)

// clear any existing records prior to registration
mDNSlocal int SetUpdateSRV(DaemonInfo *d)
{
    int err;

    err = ClearUpdateSRV(d);         // clear any existing record
    if (!err) err = UpdateSRV(d, 1);
    return err;
}

//
// Argument Parsing and Configuration
//

mDNSlocal void PrintUsage(void)
{
    fprintf(stderr, "Usage: dnsextd [-f <config file>] [-vhd] ...\n"
            "Use \"dnsextd -h\" for help\n");
}

mDNSlocal void PrintHelp(void)
{
    fprintf(stderr, "\n\n");
    PrintUsage();

    fprintf(stderr,
            "dnsextd is a daemon that implements DNS extensions supporting Dynamic DNS Update Leases\n"
            "and Long Lived Queries, used in Wide-Area DNS Service Discovery, on behalf of name servers\n"
            "that do not natively support these extensions.  (See dns-sd.org for more info on DNS Service\n"
            "Discovery, Update Leases, and Long Lived Queries.)\n\n"

            "dnsextd requires one argument,the zone, which is the domain for which Update Leases\n"
            "and Long Lived Queries are to be administered.  dnsextd communicates directly with the\n"
            "primary master server for this zone.\n\n"

            "The options are as follows:\n\n"

            "-f    Specify configuration file. The default is /etc/dnsextd.conf.\n\n"

            "-d    Run daemon in foreground.\n\n"

            "-h    Print help.\n\n"

            "-v    Verbose output.\n\n"
            );
}


// Note: ProcessArgs called before process is daemonized, and therefore must open no descriptors
// returns 0 (success) if program is to continue execution
// output control arguments (-f, -v) do not affect this routine
mDNSlocal int ProcessArgs(int argc, char *argv[], DaemonInfo *d)
{
    DNSZone *   zone;
    int opt;
    int err = 0;

    cfgfile = strdup( CONFIG_FILE );
    require_action( cfgfile, arg_error, err = mStatus_NoMemoryErr );

    // defaults, may be overriden by command option

    // setup our sockaddr

    mDNSPlatformMemZero( &d->addr, sizeof( d->addr ) );
    d->addr.sin_addr.s_addr = zerov4Addr.NotAnInteger;
    d->addr.sin_port        = UnicastDNSPort.NotAnInteger;
    d->addr.sin_family      = AF_INET;
#ifndef NOT_HAVE_SA_LEN
    d->addr.sin_len         = sizeof( d->addr );
#endif

    // setup nameserver's sockaddr

    mDNSPlatformMemZero(&d->ns_addr, sizeof(d->ns_addr));
    d->ns_addr.sin_family   = AF_INET;
    inet_pton( AF_INET, LOOPBACK, &d->ns_addr.sin_addr );
    d->ns_addr.sin_port     = NSIPCPort.NotAnInteger;
#ifndef NOT_HAVE_SA_LEN
    d->ns_addr.sin_len      = sizeof( d->ns_addr );
#endif

    // setup our ports

    d->private_port = PrivateDNSPort;
    d->llq_port     = DNSEXTPort;

    while ((opt = getopt(argc, argv, "f:hdv")) != -1)
    {
        switch(opt)
        {
        case 'f': free( cfgfile ); cfgfile = strdup( optarg ); require_action( cfgfile, arg_error, err = mStatus_NoMemoryErr ); break;
        case 'h': PrintHelp();    return -1;
        case 'd': foreground = 1; break;            // Also used when launched via OS X's launchd mechanism
        case 'v': verbose = 1;    break;
        default:  goto arg_error;
        }
    }

    err = ParseConfig( d, cfgfile );
    require_noerr( err, arg_error );

    // Make sure we've specified some zones

    require_action( d->zones, arg_error, err = mStatus_UnknownErr );

    // if we have a shared secret, use it for the entire zone

    for ( zone = d->zones; zone; zone = zone->next )
    {
        if ( zone->updateKeys )
        {
            AssignDomainName( &zone->updateKeys->domain, &zone->name );
        }
    }

    return 0;

arg_error:

    PrintUsage();
    return -1;
}


//
// Initialization Routines
//

// Allocate memory, initialize locks and bookkeeping variables
mDNSlocal int InitLeaseTable(DaemonInfo *d)
{
    if (pthread_mutex_init(&d->tablelock, NULL)) { LogErr("InitLeaseTable", "pthread_mutex_init"); return -1; }
    d->nbuckets = LEASETABLE_INIT_NBUCKETS;
    d->nelems = 0;
    d->table = malloc(sizeof(RRTableElem *) * LEASETABLE_INIT_NBUCKETS);
    if (!d->table) { LogErr("InitLeaseTable", "malloc"); return -1; }
    mDNSPlatformMemZero(d->table, sizeof(RRTableElem *) * LEASETABLE_INIT_NBUCKETS);
    return 0;
}


mDNSlocal int
SetupSockets
(
    DaemonInfo * self
)
{
    static const int kOn = 1;
    int sockpair[2];
    mDNSBool private = mDNSfalse;
    struct sockaddr_in daddr;
    DNSZone         *   zone;
    mStatus err = 0;

    // set up sockets on which we all ns requests

    self->tcpsd = socket( AF_INET, SOCK_STREAM, 0 );
    require_action( dnssd_SocketValid(self->tcpsd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );

#if defined(SO_REUSEADDR)
    err = setsockopt(self->tcpsd, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
    require_action( !err, exit, LogErr( "SetupSockets", "SO_REUSEADDR self->tcpsd" ) );
#endif

    err = bind( self->tcpsd, ( struct sockaddr* ) &self->addr, sizeof( self->addr ) );
    require_action( !err, exit, LogErr( "SetupSockets", "bind self->tcpsd" ) );

    err = listen( self->tcpsd, LISTENQ );
    require_action( !err, exit, LogErr( "SetupSockets", "listen" ) );

    self->udpsd = socket( AF_INET, SOCK_DGRAM, 0 );
    require_action( dnssd_SocketValid(self->udpsd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );

#if defined(SO_REUSEADDR)
    err = setsockopt(self->udpsd, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
    require_action( !err, exit, LogErr( "SetupSockets", "SO_REUSEADDR self->udpsd" ) );
#endif

    err = bind( self->udpsd, ( struct sockaddr* ) &self->addr, sizeof( self->addr ) );
    require_action( !err, exit, LogErr( "SetupSockets", "bind self->udpsd" ) );

    // set up sockets on which we receive llq requests

    mDNSPlatformMemZero(&self->llq_addr, sizeof(self->llq_addr));
    self->llq_addr.sin_family       = AF_INET;
    self->llq_addr.sin_addr.s_addr  = zerov4Addr.NotAnInteger;
    self->llq_addr.sin_port         = ( self->llq_port.NotAnInteger ) ? self->llq_port.NotAnInteger : DNSEXTPort.NotAnInteger;

    if (self->llq_addr.sin_port == self->addr.sin_port)
    {
        self->llq_tcpsd = self->tcpsd;
        self->llq_udpsd = self->udpsd;
    }
    else
    {
        self->llq_tcpsd = socket( AF_INET, SOCK_STREAM, 0 );
        require_action( dnssd_SocketValid(self->llq_tcpsd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );

#if defined(SO_REUSEADDR)
        err = setsockopt(self->llq_tcpsd, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
        require_action( !err, exit, LogErr( "SetupSockets", "SO_REUSEADDR self->llq_tcpsd" ) );
#endif

        err = bind( self->llq_tcpsd, ( struct sockaddr* ) &self->llq_addr, sizeof( self->llq_addr ) );
        require_action( !err, exit, LogErr( "SetupSockets", "bind self->llq_tcpsd" ) );

        err = listen( self->llq_tcpsd, LISTENQ );
        require_action( !err, exit, LogErr( "SetupSockets", "listen" ) );

        self->llq_udpsd = socket( AF_INET, SOCK_DGRAM, 0 );
        require_action( dnssd_SocketValid(self->llq_udpsd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );

#if defined(SO_REUSEADDR)
        err = setsockopt(self->llq_udpsd, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
        require_action( !err, exit, LogErr( "SetupSockets", "SO_REUSEADDR self->llq_udpsd" ) );
#endif

        err = bind(self->llq_udpsd, ( struct sockaddr* ) &self->llq_addr, sizeof( self->llq_addr ) );
        require_action( !err, exit, LogErr( "SetupSockets", "bind self->llq_udpsd" ) );
    }

    // set up Unix domain socket pair for LLQ polling thread to signal main thread that a change to the zone occurred

    err = socketpair( AF_LOCAL, SOCK_STREAM, 0, sockpair );
    require_action( !err, exit, LogErr( "SetupSockets", "socketpair" ) );

    self->LLQEventListenSock = sockpair[0];
    self->LLQEventNotifySock = sockpair[1];

    // set up socket on which we receive private requests

    self->llq_tcpsd = socket( AF_INET, SOCK_STREAM, 0 );
    require_action( dnssd_SocketValid(self->tlssd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );
    mDNSPlatformMemZero(&daddr, sizeof(daddr));
    daddr.sin_family        = AF_INET;
    daddr.sin_addr.s_addr   = zerov4Addr.NotAnInteger;
    daddr.sin_port          = ( self->private_port.NotAnInteger ) ? self->private_port.NotAnInteger : PrivateDNSPort.NotAnInteger;

    self->tlssd = socket( AF_INET, SOCK_STREAM, 0 );
    require_action( dnssd_SocketValid(self->tlssd), exit, err = mStatus_UnknownErr; LogErr( "SetupSockets", "socket" ) );

#if defined(SO_REUSEADDR)
    err = setsockopt(self->tlssd, SOL_SOCKET, SO_REUSEADDR, &kOn, sizeof(kOn));
    require_action( !err, exit, LogErr( "SetupSockets", "SO_REUSEADDR self->tlssd" ) );
#endif

    err = bind( self->tlssd, ( struct sockaddr* ) &daddr, sizeof( daddr ) );
    require_action( !err, exit, LogErr( "SetupSockets", "bind self->tlssd" ) );

    err = listen( self->tlssd, LISTENQ );
    require_action( !err, exit, LogErr( "SetupSockets", "listen" ) );

    // Do we have any private zones?

    for ( zone = self->zones; zone; zone = zone->next )
    {
        if ( zone->type == kDNSZonePrivate )
        {
            private = mDNStrue;
            break;
        }
    }

    if ( private )
    {
        err = mDNSPlatformTLSSetupCerts();
        require_action( !err, exit, LogErr( "SetupSockets", "mDNSPlatformTLSSetupCerts" ) );
    }

exit:

    return err;
}

//
// periodic table updates
//

// Delete a resource record from the nameserver via a dynamic update
// sd is a socket already connected to the server
mDNSlocal void DeleteOneRecord(DaemonInfo *d, CacheRecord *rr, domainname *zname, TCPSocket *sock)
{
    DNSZone *   zone;
    PktMsg pkt;
    mDNSu8 *ptr = pkt.msg.data;
    mDNSu8 *end = (mDNSu8 *)&pkt.msg + sizeof(DNSMessage);
    char buf[MaxMsg];
    mDNSBool closed;
    PktMsg *reply = NULL;

    VLog("Expiring record %s", GetRRDisplayString_rdb(&rr->resrec, &rr->resrec.rdata->u, buf));

    InitializeDNSMessage(&pkt.msg.h, zeroID, UpdateReqFlags);

    ptr = putZone(&pkt.msg, ptr, end, zname, mDNSOpaque16fromIntVal(rr->resrec.rrclass));
    if (!ptr) goto end;
    ptr = putDeletionRecord(&pkt.msg, ptr, &rr->resrec);
    if (!ptr) goto end;

    HdrHToN(&pkt);

    zone = FindZone( d, zname );

    if ( zone && zone->updateKeys)
    {
        DNSDigest_SignMessage(&pkt.msg, &ptr, zone->updateKeys, 0 );
        if (!ptr) goto end;
    }

    pkt.len = ptr - (mDNSu8 *)&pkt.msg;
    pkt.src.sin_addr.s_addr = zerov4Addr.NotAnInteger; // address field set solely for verbose logging in subroutines
    pkt.src.sin_family = AF_INET;
    if (SendPacket( sock, &pkt)) { Log("DeleteOneRecord: SendPacket failed"); }
    reply = RecvPacket( sock, NULL, &closed );
    if (reply) HdrNToH(reply);
    require_action( reply, end, Log( "DeleteOneRecord: RecvPacket returned NULL" ) );

    if (!SuccessfulUpdateTransaction(&pkt, reply))
        Log("Expiration update failed with rcode %d", reply ? reply->msg.h.flags.b[1] & kDNSFlag1_RC_Mask : -1);

end:
    if (!ptr) { Log("DeleteOneRecord: Error constructing lease expiration update"); }
    if (reply) free(reply);
}

// iterate over table, deleting expired records (or all records if DeleteAll is true)
mDNSlocal void DeleteRecords(DaemonInfo *d, mDNSBool DeleteAll)
{
    struct timeval now;
    int i;
    TCPSocket *sock = ConnectToServer(d);
    if (!sock) { Log("DeleteRecords: ConnectToServer failed"); return; }
    if (gettimeofday(&now, NULL)) { LogErr("DeleteRecords ", "gettimeofday"); return; }
    if (pthread_mutex_lock(&d->tablelock)) { LogErr("DeleteRecords", "pthread_mutex_lock"); return; }

    for (i = 0; i < d->nbuckets; i++)
    {
        RRTableElem **ptr = &d->table[i];
        while (*ptr)
        {
            if (DeleteAll || (*ptr)->expire - now.tv_sec < 0)
            {
                RRTableElem *fptr;
                // delete record from server
                DeleteOneRecord(d, &(*ptr)->rr, &(*ptr)->zone, sock);
                fptr = *ptr;
                *ptr = (*ptr)->next;
                free(fptr);
                d->nelems--;
            }
            else ptr = &(*ptr)->next;
        }
    }
    pthread_mutex_unlock(&d->tablelock);
    mDNSPlatformTCPCloseConnection( sock );
}

//
// main update request handling
//

// Add, delete, or refresh records in table based on contents of a successfully completed dynamic update
mDNSlocal void UpdateLeaseTable(PktMsg *pkt, DaemonInfo *d, mDNSs32 lease)
{
    int i, allocsize;
    LargeCacheRecord lcr;
    ResourceRecord *rr = &lcr.r.resrec;
    const mDNSu8 *ptr, *end;
    struct timeval tv;
    DNSQuestion zone;
    char buf[MaxMsg];

    if (pthread_mutex_lock(&d->tablelock)) { LogErr("UpdateLeaseTable", "pthread_mutex_lock"); return; }
    HdrNToH(pkt);
    ptr = pkt->msg.data;
    end = (mDNSu8 *)&pkt->msg + pkt->len;
    ptr = getQuestion(&pkt->msg, ptr, end, 0, &zone);
    if (!ptr) { Log("UpdateLeaseTable: cannot read zone");  goto cleanup; }
    ptr = LocateAuthorities(&pkt->msg, end);
    if (!ptr) { Log("UpdateLeaseTable: Format error");  goto cleanup; }

    for (i = 0; i < pkt->msg.h.mDNS_numUpdates; i++)
    {
        mDNSBool DeleteAllRRSets = mDNSfalse, DeleteOneRRSet = mDNSfalse, DeleteOneRR = mDNSfalse;

        ptr = GetLargeResourceRecord(NULL, &pkt->msg, ptr, end, 0, kDNSRecordTypePacketAns, &lcr);
        if (!ptr || lcr.r.resrec.RecordType == kDNSRecordTypePacketNegative) { Log("UpdateLeaseTable: GetLargeResourceRecord failed"); goto cleanup; }
        int bucket = rr->namehash % d->nbuckets;
        RRTableElem *tmp, **rptr = &d->table[bucket];

        // handle deletions
        if (rr->rrtype == kDNSQType_ANY && !rr->rroriginalttl && rr->rrclass == kDNSQClass_ANY && !rr->rdlength)
            DeleteAllRRSets = mDNStrue; // delete all rrsets for a name
        else if (!rr->rroriginalttl && rr->rrclass == kDNSQClass_ANY && !rr->rdlength)
            DeleteOneRRSet = mDNStrue;
        else if (!rr->rroriginalttl && rr->rrclass == kDNSClass_NONE)
            DeleteOneRR = mDNStrue;

        if (DeleteAllRRSets || DeleteOneRRSet || DeleteOneRR)
        {
            while (*rptr)
            {
                if (SameDomainName((*rptr)->rr.resrec.name, rr->name) &&
                    (DeleteAllRRSets ||
                     (DeleteOneRRSet && (*rptr)->rr.resrec.rrtype == rr->rrtype) ||
                     (DeleteOneRR && IdenticalResourceRecord(&(*rptr)->rr.resrec, rr))))
                {
                    tmp = *rptr;
                    VLog("Received deletion update for %s", GetRRDisplayString_rdb(&tmp->rr.resrec, &tmp->rr.resrec.rdata->u, buf));
                    *rptr = (*rptr)->next;
                    free(tmp);
                    d->nelems--;
                }
                else rptr = &(*rptr)->next;
            }
        }
        else if (lease > 0)
        {
            // see if add or refresh
            while (*rptr && !IdenticalResourceRecord(&(*rptr)->rr.resrec, rr)) rptr = &(*rptr)->next;
            if (*rptr)
            {
                // refresh
                if (gettimeofday(&tv, NULL)) { LogErr("UpdateLeaseTable", "gettimeofday"); goto cleanup; }
                (*rptr)->expire = tv.tv_sec + (unsigned)lease;
                VLog("Refreshing lease for %s", GetRRDisplayString_rdb(&lcr.r.resrec, &lcr.r.resrec.rdata->u, buf));
            }
            else
            {
                // New record - add to table
                if (d->nelems > d->nbuckets)
                {
                    RehashTable(d);
                    bucket = rr->namehash % d->nbuckets;
                }
                if (gettimeofday(&tv, NULL)) { LogErr("UpdateLeaseTable", "gettimeofday"); goto cleanup; }
                allocsize = sizeof(RRTableElem);
                if (rr->rdlength > InlineCacheRDSize) allocsize += (rr->rdlength - InlineCacheRDSize);
                tmp = malloc(allocsize);
                if (!tmp) { LogErr("UpdateLeaseTable", "malloc"); goto cleanup; }
                memcpy(&tmp->rr, &lcr.r, sizeof(CacheRecord) + rr->rdlength - InlineCacheRDSize);
                tmp->rr.resrec.rdata = (RData *)&tmp->rr.smallrdatastorage;
                AssignDomainName(&tmp->name, rr->name);
                tmp->rr.resrec.name = &tmp->name;
                tmp->expire = tv.tv_sec + (unsigned)lease;
                tmp->cli.sin_addr = pkt->src.sin_addr;
                AssignDomainName(&tmp->zone, &zone.qname);
                tmp->next = d->table[bucket];
                d->table[bucket] = tmp;
                d->nelems++;
                VLog("Adding update for %s to lease table", GetRRDisplayString_rdb(&lcr.r.resrec, &lcr.r.resrec.rdata->u, buf));
            }
        }
    }

cleanup:
    pthread_mutex_unlock(&d->tablelock);
    HdrHToN(pkt);
}

// Given a successful reply from a server, create a new reply that contains lease information
// Replies are currently not signed !!!KRS change this
mDNSlocal PktMsg *FormatLeaseReply(DaemonInfo *d, PktMsg *orig, mDNSu32 lease)
{
    PktMsg *const reply = malloc(sizeof(*reply));
    mDNSu8 *ptr;
    mDNSOpaque16 flags;
    (void)d;  //unused
    
    if (!reply) { LogErr("FormatLeaseReply", "malloc"); return NULL; }
    flags.b[0] = kDNSFlag0_QR_Response | kDNSFlag0_OP_Update;
    flags.b[1] = 0;

    InitializeDNSMessage(&reply->msg.h, orig->msg.h.id, flags);
    reply->src.sin_addr.s_addr = zerov4Addr.NotAnInteger;            // unused except for log messages
    reply->src.sin_family = AF_INET;
    ptr = putUpdateLease(&reply->msg, reply->msg.data, lease);
    if (!ptr) { Log("FormatLeaseReply: putUpdateLease failed"); free(reply); return NULL; }
    reply->len = ptr - (mDNSu8 *)&reply->msg;
    HdrHToN(reply);
    return reply;
}


// pkt is thread-local, not requiring locking

mDNSlocal PktMsg*
HandleRequest
(
    DaemonInfo  *   self,
    PktMsg      *   request
)
{
    PktMsg      *   reply = NULL;
    PktMsg      *   leaseReply;
    PktMsg buf;
    char addrbuf[32];
    TCPSocket * sock = NULL;
    mStatus err;
    mDNSu32 lease = 0;
    mDNSBool gotlease;
    if ((request->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask) == kDNSFlag0_OP_Update)
    {
        int i, adds = 0, dels = 0;
        const mDNSu8 *ptr, *end = (mDNSu8 *)&request->msg + request->len;
        HdrNToH(request);
        gotlease = GetPktLease(&mDNSStorage, &request->msg, end, &lease);
        ptr = LocateAuthorities(&request->msg, end);
        for (i = 0; i < request->msg.h.mDNS_numUpdates; i++)
        {
            LargeCacheRecord lcr;
            ptr = GetLargeResourceRecord(NULL, &request->msg, ptr, end, 0, kDNSRecordTypePacketAns, &lcr);
            if (lcr.r.resrec.RecordType != kDNSRecordTypePacketNegative && lcr.r.resrec.rroriginalttl) adds++;else dels++;
        }
        HdrHToN(request);
        if (adds && !gotlease)
        {
            static const mDNSOpaque16 UpdateRefused = { { kDNSFlag0_QR_Response | kDNSFlag0_OP_Update, kDNSFlag1_RC_Refused } };
            Log("Rejecting Update Request with %d additions but no lease", adds);
            reply = malloc(sizeof(*reply));
            mDNSPlatformMemZero(&reply->src, sizeof(reply->src));
            reply->len = sizeof(DNSMessageHeader);
            reply->zone = NULL;
            reply->isZonePublic = 0;
            InitializeDNSMessage(&reply->msg.h, request->msg.h.id, UpdateRefused);
            return(reply);
        }
        if (lease > 7200)   // Don't allow lease greater than two hours; typically 90-minute renewal period
            lease = 7200;
    }
    // Send msg to server, read reply

    if ( request->len <= 512 )
    {
        mDNSBool trunc;

        if ( UDPServerTransaction( self, request, &buf, &trunc) < 0 )
        {
            Log("HandleRequest - UDPServerTransaction failed.  Trying TCP");
        }
        else if ( trunc )
        {
            VLog("HandleRequest - answer truncated.  Using TCP");
        }
        else
        {
            reply = &buf; // success
        }
    }

    if ( !reply )
    {
        mDNSBool closed;
        int res;

        sock = ConnectToServer( self );
        require_action_quiet( sock, exit, err = mStatus_UnknownErr ; Log( "Discarding request from %s due to connection errors", inet_ntop( AF_INET, &request->src.sin_addr, addrbuf, 32 ) ) );

        res = SendPacket( sock, request );
        require_action_quiet( res >= 0, exit, err = mStatus_UnknownErr ; Log( "Couldn't relay message from %s to server.  Discarding.", inet_ntop(AF_INET, &request->src.sin_addr, addrbuf, 32 ) ) );

        reply = RecvPacket( sock, &buf, &closed );
    }

    // IMPORTANT: reply is in network byte order at this point in the code
    // We keep it this way because we send it back to the client in the same form

    // Is it an update?

    if ( reply && ( ( reply->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask ) == ( kDNSFlag0_OP_Update | kDNSFlag0_QR_Response ) ) )
    {
        char pingmsg[4];
        mDNSBool ok = SuccessfulUpdateTransaction( request, reply );
        require_action( ok, exit, err = mStatus_UnknownErr; VLog( "Message from %s not a successful update.", inet_ntop(AF_INET, &request->src.sin_addr, addrbuf, 32 ) ) );

        UpdateLeaseTable( request, self, lease );

        if ( lease > 0 )
        {
            leaseReply = FormatLeaseReply( self, reply, lease );

            if ( !leaseReply )
            {
                Log("HandleRequest - unable to format lease reply");
            }

            // %%% Looks like a potential memory leak -- who frees the original reply?
            reply = leaseReply;
        }

        // tell the main thread there was an update so it can send LLQs

        if ( send( self->LLQEventNotifySock, pingmsg, sizeof( pingmsg ), 0 ) != sizeof( pingmsg ) )
        {
            LogErr("HandleRequest", "send");
        }
    }

exit:

    if ( sock )
    {
        mDNSPlatformTCPCloseConnection( sock );
    }

    if ( reply == &buf )
    {
        reply = malloc( sizeof( *reply ) );

        if ( reply )
        {
            reply->len = buf.len;
            memcpy(&reply->msg, &buf.msg, buf.len);
        }
        else
        {
            LogErr("HandleRequest", "malloc");
        }
    }

    return reply;
}


//
// LLQ Support Routines
//

// Set fields of an LLQ OPT Resource Record
mDNSlocal void FormatLLQOpt(AuthRecord *opt, int opcode, const mDNSOpaque64 *const id, mDNSs32 lease)
{
    mDNSPlatformMemZero(opt, sizeof(*opt));
    mDNS_SetupResourceRecord(opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
    opt->resrec.rrclass = NormalMaxDNSMessageData;
    opt->resrec.rdlength   = sizeof(rdataOPT);  // One option in this OPT record
    opt->resrec.rdestimate = sizeof(rdataOPT);
    opt->resrec.rdata->u.opt[0].opt = kDNSOpt_LLQ;
    opt->resrec.rdata->u.opt[0].u.llq.vers  = kLLQ_Vers;
    opt->resrec.rdata->u.opt[0].u.llq.llqOp = opcode;
    opt->resrec.rdata->u.opt[0].u.llq.err   = LLQErr_NoError;
    opt->resrec.rdata->u.opt[0].u.llq.id    = *id;
    opt->resrec.rdata->u.opt[0].u.llq.llqlease = lease;
}

// Calculate effective remaining lease of an LLQ
mDNSlocal mDNSu32 LLQLease(LLQEntry *e)
{
    struct timeval t;

    gettimeofday(&t, NULL);
    if (e->expire < t.tv_sec) return 0;
    else return e->expire - t.tv_sec;
}

mDNSlocal void DeleteLLQ(DaemonInfo *d, LLQEntry *e)
{
    int bucket = DomainNameHashValue(&e->qname) % LLQ_TABLESIZE;
    LLQEntry **ptr = &d->LLQTable[bucket];
    AnswerListElem *a = e->AnswerList;
    char addr[32];

    inet_ntop(AF_INET, &e->cli.sin_addr, addr, 32);
    VLog("Deleting LLQ table entry for %##s client %s", e->qname.c, addr);

    if (a && !(--a->refcount) && d->AnswerTableCount >= LLQ_TABLESIZE)
    {
        // currently, generating initial answers blocks the main thread, so we keep the answer list
        // even if the ref count drops to zero.  To prevent unbounded table growth, we free shared answers
        // if the ref count drops to zero AND there are more table elements than buckets
        // !!!KRS update this when we make the table dynamically growable

        CacheRecord *cr = a->KnownAnswers, *tmp;
        AnswerListElem **tbl = &d->AnswerTable[bucket];

        while (cr)
        {
            tmp = cr;
            cr = cr->next;
            free(tmp);
        }

        while (*tbl && *tbl != a) tbl = &(*tbl)->next;
        if (*tbl) { *tbl = (*tbl)->next; free(a); d->AnswerTableCount--; }
        else Log("Error: DeleteLLQ - AnswerList not found in table");
    }

    // remove LLQ from table, free memory
    while(*ptr && *ptr != e) ptr = &(*ptr)->next;
    if (!*ptr) { Log("Error: DeleteLLQ - LLQ not in table"); return; }
    *ptr = (*ptr)->next;
    free(e);
}

mDNSlocal int SendLLQ(DaemonInfo *d, PktMsg *pkt, struct sockaddr_in dst, TCPSocket *sock)
{
    char addr[32];
    int err = -1;

    HdrHToN(pkt);

    if ( sock )
    {
        if ( SendPacket( sock, pkt ) != 0 )
        {
            LogErr("DaemonInfo", "MySend");
            Log("Could not send response to client %s", inet_ntop(AF_INET, &dst.sin_addr, addr, 32));
        }
    }
    else
    {
        if (sendto(d->llq_udpsd, &pkt->msg, pkt->len, 0, (struct sockaddr *)&dst, sizeof(dst)) != (int)pkt->len)
        {
            LogErr("DaemonInfo", "sendto");
            Log("Could not send response to client %s", inet_ntop(AF_INET, &dst.sin_addr, addr, 32));
        }
    }

    err = 0;
    HdrNToH(pkt);
    return err;
}

mDNSlocal CacheRecord *AnswerQuestion(DaemonInfo *d, AnswerListElem *e)
{
    PktMsg q;
    int i;
    TCPSocket *sock = NULL;
    const mDNSu8 *ansptr;
    mDNSu8 *end = q.msg.data;
    PktMsg buf, *reply = NULL;
    LargeCacheRecord lcr;
    CacheRecord *AnswerList = NULL;
    mDNSu8 rcode;

    VLog("Querying server for %##s type %d", e->name.c, e->type);

    InitializeDNSMessage(&q.msg.h, zeroID, uQueryFlags);

    end = putQuestion(&q.msg, end, end + AbsoluteMaxDNSMessageData, &e->name, e->type, kDNSClass_IN);
    if (!end) { Log("Error: AnswerQuestion - putQuestion returned NULL"); goto end; }
    q.len = (int)(end - (mDNSu8 *)&q.msg);

    HdrHToN(&q);

    if (!e->UseTCP)
    {
        mDNSBool trunc;

        if (UDPServerTransaction(d, &q, &buf, &trunc) < 0)
            Log("AnswerQuestion %##s - UDPServerTransaction failed.  Trying TCP", e->name.c);
        else if (trunc)
        { VLog("AnswerQuestion %##s - answer truncated.  Using TCP", e->name.c); e->UseTCP = mDNStrue; }
        else reply = &buf;  // success
    }

    if (!reply)
    {
        mDNSBool closed;

        sock = ConnectToServer(d);
        if (!sock) { Log("AnswerQuestion: ConnectToServer failed"); goto end; }
        if (SendPacket( sock, &q)) { Log("AnswerQuestion: SendPacket failed"); mDNSPlatformTCPCloseConnection( sock ); goto end; }
        reply = RecvPacket( sock, NULL, &closed );
        mDNSPlatformTCPCloseConnection( sock );
        require_action( reply, end, Log( "AnswerQuestion: RecvPacket returned NULL" ) );
    }

    HdrNToH(&q);
    if (reply) HdrNToH(reply);

    if ((reply->msg.h.flags.b[0] & kDNSFlag0_QROP_Mask) != (kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery))
    { Log("AnswerQuestion: %##s type %d - Invalid response flags from server"); goto end; }
    rcode = (mDNSu8)(reply->msg.h.flags.b[1] & kDNSFlag1_RC_Mask);
    if (rcode && rcode != kDNSFlag1_RC_NXDomain) { Log("AnswerQuestion: %##s type %d - non-zero rcode %d from server", e->name.c, e->type, rcode); goto end; }

    end = (mDNSu8 *)&reply->msg + reply->len;
    ansptr = LocateAnswers(&reply->msg, end);
    if (!ansptr) { Log("Error: AnswerQuestion - LocateAnswers returned NULL"); goto end; }

    for (i = 0; i < reply->msg.h.numAnswers; i++)
    {
        ansptr = GetLargeResourceRecord(NULL, &reply->msg, ansptr, end, 0, kDNSRecordTypePacketAns, &lcr);
        if (!ansptr) { Log("AnswerQuestions: GetLargeResourceRecord returned NULL"); goto end; }
        if (lcr.r.resrec.RecordType != kDNSRecordTypePacketNegative)
        {
            if (lcr.r.resrec.rrtype != e->type || lcr.r.resrec.rrclass != kDNSClass_IN || !SameDomainName(lcr.r.resrec.name, &e->name))
            {
                Log("AnswerQuestion: response %##s type #d does not answer question %##s type #d.  Discarding",
                    lcr.r.resrec.name->c, lcr.r.resrec.rrtype, e->name.c, e->type);
            }
            else
            {
                CacheRecord *cr = CopyCacheRecord(&lcr.r, &e->name);
                if (!cr) { Log("Error: AnswerQuestion - CopyCacheRecord returned NULL"); goto end; }
                cr->next = AnswerList;
                AnswerList = cr;
            }
        }
    }

end:
    if (reply && reply != &buf) free(reply);
    return AnswerList;
}

// Routine forks a thread to set EventList to contain Add/Remove events, and deletes any removes from the KnownAnswer list
mDNSlocal void *UpdateAnswerList(void *args)
{
    CacheRecord *cr, *NewAnswers, **na, **ka; // "new answer", "known answer"
    DaemonInfo *d = ((UpdateAnswerListArgs *)args)->d;
    AnswerListElem *a = ((UpdateAnswerListArgs *)args)->a;

    free(args);
    args = NULL;

    // get up to date answers
    NewAnswers = AnswerQuestion(d, a);

    // first pass - mark all answers for deletion
    for (ka = &a->KnownAnswers; *ka; ka = &(*ka)->next)
        (*ka)->resrec.rroriginalttl = (unsigned)-1; // -1 means delete

    // second pass - mark answers pre-existent
    for (ka = &a->KnownAnswers; *ka; ka = &(*ka)->next)
    {
        for (na = &NewAnswers; *na; na = &(*na)->next)
        {
            if (IdenticalResourceRecord(&(*ka)->resrec, &(*na)->resrec))
            { (*ka)->resrec.rroriginalttl = 0; break; }     // 0 means no change
        }
    }

    // third pass - add new records to Event list
    na = &NewAnswers;
    while (*na)
    {
        for (ka = &a->KnownAnswers; *ka; ka = &(*ka)->next)
            if (IdenticalResourceRecord(&(*ka)->resrec, &(*na)->resrec)) break;
        if (!*ka)
        {
            // answer is not in list - splice from NewAnswers list, add to Event list
            cr = *na;
            *na = (*na)->next;        // splice from list
            cr->next = a->EventList;  // add spliced record to event list
            a->EventList = cr;
            cr->resrec.rroriginalttl = 1; // 1 means add
        }
        else na = &(*na)->next;
    }

    // move all the removes from the answer list to the event list
    ka = &a->KnownAnswers;
    while (*ka)
    {
        if ((*ka)->resrec.rroriginalttl == (unsigned)-1)
        {
            cr = *ka;
            *ka = (*ka)->next;
            cr->next = a->EventList;
            a->EventList = cr;
        }
        else ka = &(*ka)->next;
    }

    // lastly, free the remaining records (known answers) in NewAnswers list
    while (NewAnswers)
    {
        cr = NewAnswers;
        NewAnswers = NewAnswers->next;
        free(cr);
    }

    return NULL;
}

mDNSlocal void SendEvents(DaemonInfo *d, LLQEntry *e)
{
    PktMsg response;
    CacheRecord *cr;
    mDNSu8 *end = (mDNSu8 *)&response.msg.data;
    mDNSOpaque16 msgID;
    char rrbuf[MaxMsg], addrbuf[32];
    AuthRecord opt;

    // Should this really be random?  Do we use the msgID on the receiving end?
    msgID.NotAnInteger = random();
    if (verbose) inet_ntop(AF_INET, &e->cli.sin_addr, addrbuf, 32);
    InitializeDNSMessage(&response.msg.h, msgID, ResponseFlags);
    end = putQuestion(&response.msg, end, end + AbsoluteMaxDNSMessageData, &e->qname, e->qtype, kDNSClass_IN);
    if (!end) { Log("Error: SendEvents - putQuestion returned NULL"); return; }

    // put adds/removes in packet
    for (cr = e->AnswerList->EventList; cr; cr = cr->next)
    {
        if (verbose) GetRRDisplayString_rdb(&cr->resrec, &cr->resrec.rdata->u, rrbuf);
        VLog("%s (%s): %s", addrbuf, (mDNSs32)cr->resrec.rroriginalttl < 0 ? "Remove" : "Add", rrbuf);
        end = PutResourceRecordTTLJumbo(&response.msg, end, &response.msg.h.numAnswers, &cr->resrec, cr->resrec.rroriginalttl);
        if (!end) { Log("Error: SendEvents - PutResourceRecordTTLJumbo returned NULL"); return; }
    }

    FormatLLQOpt(&opt, kLLQOp_Event, &e->id, LLQLease(e));
    end = PutResourceRecordTTLJumbo(&response.msg, end, &response.msg.h.numAdditionals, &opt.resrec, 0);
    if (!end) { Log("Error: SendEvents - PutResourceRecordTTLJumbo"); return; }

    response.len = (int)(end - (mDNSu8 *)&response.msg);
    if (SendLLQ(d, &response, e->cli, NULL ) < 0) LogMsg("Error: SendEvents - SendLLQ");
}

mDNSlocal void PrintLLQAnswers(DaemonInfo *d)
{
    int i;
    char rrbuf[MaxMsg];

    Log("Printing LLQ Answer Table contents");

    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        AnswerListElem *a = d->AnswerTable[i];
        while(a)
        {
            int ancount = 0;
            const CacheRecord *rr = a->KnownAnswers;
            while (rr) { ancount++; rr = rr->next; }
            Log("%p : Question %##s;  type %d;  referenced by %d LLQs; %d answers:", a, a->name.c, a->type, a->refcount, ancount);
            for (rr = a->KnownAnswers; rr; rr = rr->next) Log("\t%s", GetRRDisplayString_rdb(&rr->resrec, &rr->resrec.rdata->u, rrbuf));
            a = a->next;
        }
    }
}

mDNSlocal void PrintLLQTable(DaemonInfo *d)
{
    LLQEntry *e;
    char addr[32];
    int i;

    Log("Printing LLQ table contents");

    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        e = d->LLQTable[i];
        while(e)
        {
            char *state;

            switch (e->state)
            {
            case RequestReceived: state = "RequestReceived"; break;
            case ChallengeSent:   state = "ChallengeSent";   break;
            case Established:     state = "Established";     break;
            default:              state = "unknown";
            }
            inet_ntop(AF_INET, &e->cli.sin_addr, addr, 32);

            Log("LLQ from %s in state %s; %##s; type %d; orig lease %d; remaining lease %d; AnswerList %p)",
                addr, state, e->qname.c, e->qtype, e->lease, LLQLease(e), e->AnswerList);
            e = e->next;
        }
    }
}

// Send events to clients as a result of a change in the zone
mDNSlocal void GenLLQEvents(DaemonInfo *d)
{
    LLQEntry **e;
    int i;
    struct timeval t;
    UpdateAnswerListArgs *args;

    VLog("Generating LLQ Events");

    gettimeofday(&t, NULL);

    // get all answers up to date
    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        AnswerListElem *a = d->AnswerTable[i];
        while(a)
        {
            args = malloc(sizeof(*args));
            if (!args) { LogErr("GenLLQEvents", "malloc"); return; }
            args->d = d;
            args->a = a;
            if (pthread_create(&a->tid, NULL, UpdateAnswerList, args) < 0) { LogErr("GenLLQEvents", "pthread_create"); return; }
            usleep(1);
            a = a->next;
        }
    }

    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        AnswerListElem *a = d->AnswerTable[i];
        while(a)
        {
            if (pthread_join(a->tid, NULL)) LogErr("GenLLQEvents", "pthread_join");
            a = a->next;
        }
    }

    // for each established LLQ, send events
    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        e = &d->LLQTable[i];
        while(*e)
        {
            if ((*e)->expire < t.tv_sec) DeleteLLQ(d, *e);
            else
            {
                if ((*e)->state == Established && (*e)->AnswerList->EventList) SendEvents(d, *e);
                e = &(*e)->next;
            }
        }
    }

    // now that all LLQs are updated, we move Add events from the Event list to the Known Answer list, and free Removes
    for (i = 0; i < LLQ_TABLESIZE; i++)
    {
        AnswerListElem *a = d->AnswerTable[i];
        while(a)
        {
            if (a->EventList)
            {
                CacheRecord *cr = a->EventList, *tmp;
                while (cr)
                {
                    tmp = cr;
                    cr = cr->next;
                    if ((signed)tmp->resrec.rroriginalttl < 0) free(tmp);
                    else
                    {
                        tmp->next = a->KnownAnswers;
                        a->KnownAnswers = tmp;
                        tmp->resrec.rroriginalttl = 0;
                    }
                }
                a->EventList = NULL;
            }
            a = a->next;
        }
    }
}

mDNSlocal void SetAnswerList(DaemonInfo *d, LLQEntry *e)
{
    int bucket = DomainNameHashValue(&e->qname) % LLQ_TABLESIZE;
    AnswerListElem *a = d->AnswerTable[bucket];
    while (a && (a->type != e->qtype ||!SameDomainName(&a->name, &e->qname))) a = a->next;
    if (!a)
    {
        a = malloc(sizeof(*a));
        if (!a) { LogErr("SetAnswerList", "malloc"); return; }
        AssignDomainName(&a->name, &e->qname);
        a->type = e->qtype;
        a->refcount = 0;
        a->EventList = NULL;
        a->UseTCP = mDNSfalse;
        a->next = d->AnswerTable[bucket];
        d->AnswerTable[bucket] = a;
        d->AnswerTableCount++;
        a->KnownAnswers = AnswerQuestion(d, a);
    }

    e->AnswerList = a;
    a->refcount++;
}

// Allocate LLQ entry, insert into table
mDNSlocal LLQEntry *NewLLQ(DaemonInfo *d, struct sockaddr_in cli, domainname *qname, mDNSu16 qtype, mDNSu32 lease )
{
    char addr[32];
    struct timeval t;
    int bucket = DomainNameHashValue(qname) % LLQ_TABLESIZE;
    LLQEntry *e;

    e = malloc(sizeof(*e));
    if (!e) { LogErr("NewLLQ", "malloc"); return NULL; }

    inet_ntop(AF_INET, &cli.sin_addr, addr, 32);
    VLog("Allocating LLQ entry for client %s question %##s type %d", addr, qname->c, qtype);

    // initialize structure
    e->cli = cli;
    AssignDomainName(&e->qname, qname);
    e->qtype = qtype;
    e->id    = zeroOpaque64;
    e->state = RequestReceived;
    e->AnswerList = NULL;

    if (lease < LLQ_MIN_LEASE) lease = LLQ_MIN_LEASE;
    else if (lease > LLQ_MAX_LEASE) lease = LLQ_MAX_LEASE;

    gettimeofday(&t, NULL);
    e->expire = t.tv_sec + (int)lease;
    e->lease = lease;

    // add to table
    e->next = d->LLQTable[bucket];
    d->LLQTable[bucket] = e;

    return e;
}

// Handle a refresh request from client
mDNSlocal void LLQRefresh(DaemonInfo *d, LLQEntry *e, LLQOptData *llq, mDNSOpaque16 msgID, TCPSocket *sock )
{
    AuthRecord opt;
    PktMsg ack;
    mDNSu8 *end = (mDNSu8 *)&ack.msg.data;
    char addr[32];

    inet_ntop(AF_INET, &e->cli.sin_addr, addr, 32);
    VLog("%s LLQ for %##s from %s", llq->llqlease ? "Refreshing" : "Deleting", e->qname.c, addr);

    if (llq->llqlease)
    {
        struct timeval t;
        if (llq->llqlease < LLQ_MIN_LEASE) llq->llqlease = LLQ_MIN_LEASE;
        else if (llq->llqlease > LLQ_MAX_LEASE) llq->llqlease = LLQ_MIN_LEASE;
        gettimeofday(&t, NULL);
        e->expire = t.tv_sec + llq->llqlease;
    }

    ack.src.sin_addr.s_addr = 0; // unused
    InitializeDNSMessage(&ack.msg.h, msgID, ResponseFlags);
    end = putQuestion(&ack.msg, end, end + AbsoluteMaxDNSMessageData, &e->qname, e->qtype, kDNSClass_IN);
    if (!end) { Log("Error: putQuestion"); return; }

    FormatLLQOpt(&opt, kLLQOp_Refresh, &e->id, llq->llqlease ? LLQLease(e) : 0);
    end = PutResourceRecordTTLJumbo(&ack.msg, end, &ack.msg.h.numAdditionals, &opt.resrec, 0);
    if (!end) { Log("Error: PutResourceRecordTTLJumbo"); return; }

    ack.len = (int)(end - (mDNSu8 *)&ack.msg);
    if (SendLLQ(d, &ack, e->cli, sock)) Log("Error: LLQRefresh");

    if (llq->llqlease) e->state = Established;
    else DeleteLLQ(d, e);
}

// Complete handshake with Ack an initial answers
mDNSlocal void LLQCompleteHandshake(DaemonInfo *d, LLQEntry *e, LLQOptData *llq, mDNSOpaque16 msgID, TCPSocket *sock)
{
    char addr[32];
    CacheRecord *ptr;
    AuthRecord opt;
    PktMsg ack;
    mDNSu8 *end = (mDNSu8 *)&ack.msg.data;
    char rrbuf[MaxMsg], addrbuf[32];

    inet_ntop(AF_INET, &e->cli.sin_addr, addr, 32);

    if (!mDNSSameOpaque64(&llq->id, &e->id) ||
        llq->vers  != kLLQ_Vers             ||
        llq->llqOp != kLLQOp_Setup          ||
        llq->err   != LLQErr_NoError        ||
        llq->llqlease > e->lease + LLQ_LEASE_FUDGE ||
        llq->llqlease < e->lease - LLQ_LEASE_FUDGE)
    {
        Log("Incorrect challenge response from %s", addr);
        return;
    }

    if (e->state == Established) VLog("Retransmitting LLQ ack + answers for %##s", e->qname.c);
    else VLog("Delivering LLQ ack + answers for %##s", e->qname.c);

    // format ack + answers
    ack.src.sin_addr.s_addr = 0; // unused
    InitializeDNSMessage(&ack.msg.h, msgID, ResponseFlags);
    end = putQuestion(&ack.msg, end, end + AbsoluteMaxDNSMessageData, &e->qname, e->qtype, kDNSClass_IN);
    if (!end) { Log("Error: putQuestion"); return; }

    if (e->state != Established) { SetAnswerList(d, e); e->state = Established; }

    if (verbose) inet_ntop(AF_INET, &e->cli.sin_addr, addrbuf, 32);
    for (ptr = e->AnswerList->KnownAnswers; ptr; ptr = ptr->next)
    {
        if (verbose) GetRRDisplayString_rdb(&ptr->resrec, &ptr->resrec.rdata->u, rrbuf);
        VLog("%s Intitial Answer - %s", addr, rrbuf);
        end = PutResourceRecordTTLJumbo(&ack.msg, end, &ack.msg.h.numAnswers, &ptr->resrec, 1);
        if (!end) { Log("Error: PutResourceRecordTTLJumbo"); return; }
    }

    FormatLLQOpt(&opt, kLLQOp_Setup, &e->id, LLQLease(e));
    end = PutResourceRecordTTLJumbo(&ack.msg, end, &ack.msg.h.numAdditionals, &opt.resrec, 0);
    if (!end) { Log("Error: PutResourceRecordTTLJumbo"); return; }

    ack.len = (int)(end - (mDNSu8 *)&ack.msg);
    if (SendLLQ(d, &ack, e->cli, sock)) Log("Error: LLQCompleteHandshake");
}

mDNSlocal void LLQSetupChallenge(DaemonInfo *d, LLQEntry *e, LLQOptData *llq, mDNSOpaque16 msgID)
{
    struct timeval t;
    PktMsg challenge;
    mDNSu8 *end = challenge.msg.data;
    AuthRecord opt;

    if (e->state == ChallengeSent) VLog("Retransmitting LLQ setup challenge for %##s", e->qname.c);
    else VLog("Sending LLQ setup challenge for %##s", e->qname.c);

    if (!mDNSOpaque64IsZero(&llq->id)) { Log("Error: LLQSetupChallenge - nonzero ID"); return; } // server bug
    if (llq->llqOp != kLLQOp_Setup) { Log("LLQSetupChallenge - incorrrect operation from client"); return; } // client error

    if (mDNSOpaque64IsZero(&e->id)) // don't regenerate random ID for retransmissions
    {
        // construct ID <time><random>
        gettimeofday(&t, NULL);
        e->id.l[0] = t.tv_sec;
        e->id.l[1] = random();
    }

    // format response (query + LLQ opt rr)
    challenge.src.sin_addr.s_addr = 0; // unused
    InitializeDNSMessage(&challenge.msg.h, msgID, ResponseFlags);
    end = putQuestion(&challenge.msg, end, end + AbsoluteMaxDNSMessageData, &e->qname, e->qtype, kDNSClass_IN);
    if (!end) { Log("Error: putQuestion"); return; }
    FormatLLQOpt(&opt, kLLQOp_Setup, &e->id, LLQLease(e));
    end = PutResourceRecordTTLJumbo(&challenge.msg, end, &challenge.msg.h.numAdditionals, &opt.resrec, 0);
    if (!end) { Log("Error: PutResourceRecordTTLJumbo"); return; }
    challenge.len = (int)(end - (mDNSu8 *)&challenge.msg);
    if (SendLLQ(d, &challenge, e->cli, NULL)) { Log("Error: LLQSetupChallenge"); return; }
    e->state = ChallengeSent;
}

// Take action on an LLQ message from client.  Entry must be initialized and in table
mDNSlocal void UpdateLLQ(DaemonInfo *d, LLQEntry *e, LLQOptData *llq, mDNSOpaque16 msgID, TCPSocket *sock )
{
    switch(e->state)
    {
    case RequestReceived:
        if ( sock )
        {
            struct timeval t;
            gettimeofday(&t, NULL);
            e->id.l[0] = t.tv_sec;      // construct ID <time><random>
            e->id.l[1] = random();
            llq->id = e->id;
            LLQCompleteHandshake( d, e, llq, msgID, sock );

            // Set the state to established because we've just set the LLQ up using TCP
            e->state = Established;
        }
        else
        {
            LLQSetupChallenge(d, e, llq, msgID);
        }
        return;
    case ChallengeSent:
        if (mDNSOpaque64IsZero(&llq->id)) LLQSetupChallenge(d, e, llq, msgID);     // challenge sent and lost
        else LLQCompleteHandshake(d, e, llq, msgID, sock );
        return;
    case Established:
        if (mDNSOpaque64IsZero(&llq->id))
        {
            // client started over.  reset state.
            LLQEntry *newe = NewLLQ(d, e->cli, &e->qname, e->qtype, llq->llqlease );
            if (!newe) return;
            DeleteLLQ(d, e);
            LLQSetupChallenge(d, newe, llq, msgID);
            return;
        }
        else if (llq->llqOp == kLLQOp_Setup)
        { LLQCompleteHandshake(d, e, llq, msgID, sock); return; }         // Ack lost
        else if (llq->llqOp == kLLQOp_Refresh)
        { LLQRefresh(d, e, llq, msgID, sock); return; }
        else { Log("Unhandled message for established LLQ"); return; }
    }
}

mDNSlocal LLQEntry *LookupLLQ(DaemonInfo *d, struct sockaddr_in cli, domainname *qname, mDNSu16 qtype, const mDNSOpaque64 *const id)
{
    int bucket = bucket = DomainNameHashValue(qname) % LLQ_TABLESIZE;
    LLQEntry *ptr = d->LLQTable[bucket];

    while(ptr)
    {
        if (((ptr->state == ChallengeSent && mDNSOpaque64IsZero(id) && (cli.sin_port == ptr->cli.sin_port)) || // zero-id due to packet loss OK in state ChallengeSent
             mDNSSameOpaque64(id, &ptr->id)) &&                        // id match
            (cli.sin_addr.s_addr == ptr->cli.sin_addr.s_addr) && (qtype == ptr->qtype) && SameDomainName(&ptr->qname, qname)) // same source, type, qname
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

mDNSlocal int
RecvNotify
(
    DaemonInfo  *   d,
    PktMsg      *   pkt
)
{
    int res;
    int err = 0;

    pkt->msg.h.flags.b[0] |= kDNSFlag0_QR_Response;

    res = sendto( d->udpsd, &pkt->msg, pkt->len, 0, ( struct sockaddr* ) &pkt->src, sizeof( pkt->src ) );
    require_action( res == ( int ) pkt->len, exit, err = mStatus_UnknownErr; LogErr( "RecvNotify", "sendto" ) );

exit:

    return err;
}


mDNSlocal int RecvLLQ( DaemonInfo *d, PktMsg *pkt, TCPSocket *sock )
{
    DNSQuestion q;
    LargeCacheRecord opt;
    unsigned int i;
    int err = -1;
    char addr[32];
    const mDNSu8 *qptr = pkt->msg.data;
    const mDNSu8 *end = (mDNSu8 *)&pkt->msg + pkt->len;
    const mDNSu8 *aptr;
    LLQOptData *llq = NULL;
    LLQEntry *e = NULL;

    HdrNToH(pkt);
    aptr = LocateAdditionals(&pkt->msg, end);   // Can't do this until after HdrNToH(pkt);
    inet_ntop(AF_INET, &pkt->src.sin_addr, addr, 32);

    VLog("Received LLQ msg from %s", addr);
    // sanity-check packet
    if (!pkt->msg.h.numQuestions || !pkt->msg.h.numAdditionals)
    {
        Log("Malformatted LLQ from %s with %d questions, %d additionals", addr, pkt->msg.h.numQuestions, pkt->msg.h.numAdditionals);
        goto end;
    }

    // Locate the OPT record.
    // According to RFC 2671, "One OPT pseudo-RR can be added to the additional data section of either a request or a response."
    // This implies that there may be *at most* one OPT record per DNS message, in the Additional Section,
    // but not necessarily the *last* entry in the Additional Section.
    for (i = 0; i < pkt->msg.h.numAdditionals; i++)
    {
        aptr = GetLargeResourceRecord(NULL, &pkt->msg, aptr, end, 0, kDNSRecordTypePacketAdd, &opt);
        if (!aptr) { Log("Malformatted LLQ from %s: could not get Additional record %u", addr, i); goto end; }
        if (opt.r.resrec.RecordType != kDNSRecordTypePacketNegative && opt.r.resrec.rrtype == kDNSType_OPT) break;
    }

    // validate OPT
    if (opt.r.resrec.rrtype != kDNSType_OPT) { Log("Malformatted LLQ from %s: last Additional not an OPT RR", addr); goto end; }
    if (opt.r.resrec.rdlength < pkt->msg.h.numQuestions * DNSOpt_LLQData_Space) { Log("Malformatted LLQ from %s: OPT RR to small (%d bytes for %d questions)", addr, opt.r.resrec.rdlength, pkt->msg.h.numQuestions); }

    // dispatch each question
    for (i = 0; i < pkt->msg.h.numQuestions; i++)
    {
        qptr = getQuestion(&pkt->msg, qptr, end, 0, &q);
        if (!qptr) { Log("Malformatted LLQ from %s: cannot read question %u", addr, i); goto end; }
        llq = &opt.r.resrec.rdata->u.opt[i].u.llq; // point into OptData at index i
        if (llq->vers != kLLQ_Vers) { Log("LLQ from %s contains bad version %d (expected %d)", addr, llq->vers, kLLQ_Vers); goto end; }

        e = LookupLLQ(d, pkt->src, &q.qname, q.qtype, &llq->id);
        if (!e)
        {
            // no entry - if zero ID, create new
            e = NewLLQ(d, pkt->src, &q.qname, q.qtype, llq->llqlease );
            if (!e) goto end;
        }
        UpdateLLQ(d, e, llq, pkt->msg.h.id, sock);
    }
    err = 0;

end:
    HdrHToN(pkt);
    return err;
}


mDNSlocal mDNSBool IsAuthorized( DaemonInfo * d, PktMsg * pkt, DomainAuthInfo ** key, mDNSu16 * rcode, mDNSu16 * tcode )
{
    const mDNSu8    *   lastPtr = NULL;
    const mDNSu8    *   ptr = NULL;
    DomainAuthInfo  *   keys;
    mDNSu8          *   end = ( mDNSu8* ) &pkt->msg + pkt->len;
    LargeCacheRecord lcr;
    mDNSBool hasTSIG = mDNSfalse;
    mDNSBool strip = mDNSfalse;
    mDNSBool ok = mDNSfalse;
    int i;

    // Unused parameters

    ( void ) d;

    HdrNToH(pkt);

    *key = NULL;
    bzero(&lcr, sizeof(lcr));

    if ( pkt->msg.h.numAdditionals )
    {
        ptr = LocateAdditionals(&pkt->msg, end);
        if (ptr)
        {
            for (i = 0; i < pkt->msg.h.numAdditionals; i++)
            {
                lastPtr = ptr;
                ptr = GetLargeResourceRecord(NULL, &pkt->msg, ptr, end, 0, kDNSRecordTypePacketAdd, &lcr);
                if (!ptr)
                {
                    Log("Unable to read additional record");
                    lastPtr = NULL;
                    break;
                }
            }

            hasTSIG = ( ptr && lcr.r.resrec.RecordType != kDNSRecordTypePacketNegative && lcr.r.resrec.rrtype == kDNSType_TSIG );
        }
        else
        {
            LogMsg( "IsAuthorized: unable to find Additional section" );
        }
    }

    // If we don't know what zone this is, then it's authorized.

    if ( !pkt->zone )
    {
        ok = mDNStrue;
        strip = mDNSfalse;
        goto exit;
    }

    if ( IsQuery( pkt ) )
    {
        keys = pkt->zone->queryKeys;
        strip = mDNStrue;
    }
    else if ( IsUpdate( pkt ) )
    {
        keys = pkt->zone->updateKeys;
        strip = mDNSfalse;
    }
    else
    {
        ok = mDNStrue;
        strip = mDNSfalse;
        goto exit;
    }

    if ( pkt->isZonePublic )
    {
        ok = mDNStrue;
        goto exit;
    }

    // If there are no keys, then we're authorized

    if ( ( hasTSIG && !keys ) || ( !hasTSIG && keys ) )
    {
        Log( "Invalid TSIG spec %##s for zone %##s", lcr.r.resrec.name->c, pkt->zone->name.c );
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadKey;
        strip = mDNStrue;
        ok = mDNSfalse;
        goto exit;
    }

    // Find the right key

    for ( *key = keys; *key; *key = (*key)->next )
    {
        if ( SameDomainName( lcr.r.resrec.name, &(*key)->keyname ) )
        {
            break;
        }
    }

    if ( !(*key) )
    {
        Log( "Invalid TSIG name %##s for zone %##s", lcr.r.resrec.name->c, pkt->zone->name.c );
        *rcode = kDNSFlag1_RC_NotAuth;
        *tcode = TSIG_ErrBadKey;
        strip = mDNStrue;
        ok = mDNSfalse;
        goto exit;
    }

    // Okay, we have the correct key and a TSIG record.  DNSDigest_VerifyMessage does the heavy
    // lifting of message verification

    pkt->msg.h.numAdditionals--;

    HdrHToN( pkt );

    ok = DNSDigest_VerifyMessage( &pkt->msg, ( mDNSu8* ) lastPtr, &lcr, (*key), rcode, tcode );

    HdrNToH( pkt );

    pkt->msg.h.numAdditionals++;

exit:

    if ( hasTSIG && strip )
    {
        // Strip the TSIG from the message

        pkt->msg.h.numAdditionals--;
        pkt->len = lastPtr - ( mDNSu8* ) ( &pkt->msg );
    }

    HdrHToN(pkt);

    return ok;
}

// request handler wrappers for TCP and UDP requests
// (read message off socket, fork thread that invokes main processing routine and handles cleanup)

mDNSlocal void*
UDPMessageHandler
(
    void * vptr
)
{
    UDPContext  *   context = ( UDPContext* ) vptr;
    PktMsg      *   reply   = NULL;
    int res;
    mStatus err;

    // !!!KRS strictly speaking, we shouldn't use TCP for a UDP request because the server
    // may give us a long answer that would require truncation for UDP delivery to client

    reply = HandleRequest( context->d, &context->pkt );
    require_action( reply, exit, err = mStatus_UnknownErr );

    res = sendto( context->sd, &reply->msg, reply->len, 0, ( struct sockaddr* ) &context->pkt.src, sizeof( context->pkt.src ) );
    require_action_quiet( res == ( int ) reply->len, exit, LogErr( "UDPMessageHandler", "sendto" ) );

exit:

    if ( reply )
    {
        free( reply );
    }

    free( context );

    pthread_exit( NULL );

    return NULL;
}


mDNSlocal int
RecvUDPMessage
(
    DaemonInfo  *   self,
    int sd
)
{
    UDPContext      *   context = NULL;
    pthread_t tid;
    mDNSu16 rcode;
    mDNSu16 tcode;
    DomainAuthInfo  *   key;
    unsigned int clisize = sizeof( context->cliaddr );
    int res;
    mStatus err = mStatus_NoError;

    context = malloc( sizeof( UDPContext ) );
    require_action( context, exit, err = mStatus_NoMemoryErr ; LogErr( "RecvUDPMessage", "malloc" ) );

    mDNSPlatformMemZero( context, sizeof( *context ) );
    context->d = self;
    context->sd = sd;

    res = recvfrom(sd, &context->pkt.msg, sizeof(context->pkt.msg), 0, (struct sockaddr *)&context->cliaddr, &clisize);

    require_action( res >= 0, exit, err = mStatus_UnknownErr ; LogErr( "RecvUDPMessage", "recvfrom" ) );
    context->pkt.len = res;
    require_action( clisize == sizeof( context->cliaddr ), exit, err = mStatus_UnknownErr ; Log( "Client address of unknown size %d", clisize ) );
    context->pkt.src = context->cliaddr;

    // Set the zone in the packet

    SetZone( context->d, &context->pkt );

    // Notify messages handled by main thread

    if ( IsNotify( &context->pkt ) )
    {
        int e = RecvNotify( self, &context->pkt );
        free(context);
        return e;
    }
    else if ( IsAuthorized( context->d, &context->pkt, &key, &rcode, &tcode ) )
    {
        if ( IsLLQRequest( &context->pkt ) )
        {
            // LLQ messages handled by main thread
            int e = RecvLLQ( self, &context->pkt, NULL );
            free(context);
            return e;
        }

        if ( IsLLQAck(&context->pkt ) )
        {
            // !!!KRS need to do acks + retrans

            free(context);
            return 0;
        }

        err = pthread_create( &tid, NULL, UDPMessageHandler, context );
        require_action( !err, exit, LogErr( "RecvUDPMessage", "pthread_create" ) );

        pthread_detach(tid);
    }
    else
    {
        PktMsg reply;
        int e;

        memcpy( &reply, &context->pkt, sizeof( PktMsg ) );

        reply.msg.h.flags.b[0]  =  kDNSFlag0_QR_Response | kDNSFlag0_AA | kDNSFlag0_RD;
        reply.msg.h.flags.b[1]  =  kDNSFlag1_RA | kDNSFlag1_RC_NXDomain;

        e = sendto( sd, &reply.msg, reply.len, 0, ( struct sockaddr* ) &context->pkt.src, sizeof( context->pkt.src ) );
        require_action_quiet( e == ( int ) reply.len, exit, LogErr( "RecvUDPMessage", "sendto" ) );

        err = mStatus_NoAuth;
    }

exit:

    if ( err && context )
    {
        free( context );
    }

    return err;
}


mDNSlocal void
FreeTCPContext
(
    TCPContext * context
)
{
    if ( context )
    {
        if ( context->sock )
        {
            mDNSPlatformTCPCloseConnection( context->sock );
        }

        free( context );
    }
}


mDNSlocal void*
TCPMessageHandler
(
    void * vptr
)
{
    TCPContext  *   context = ( TCPContext* ) vptr;
    PktMsg      *   reply = NULL;
    int res;
    char buf[32];

    //!!!KRS if this read blocks indefinitely, we can run out of threads
    // read the request

    reply = HandleRequest( context->d, &context->pkt );
    require_action_quiet( reply, exit, LogMsg( "TCPMessageHandler: No reply for client %s", inet_ntop( AF_INET, &context->cliaddr.sin_addr, buf, 32 ) ) );

    // deliver reply to client

    res = SendPacket( context->sock, reply );
    require_action( res >= 0, exit, LogMsg("TCPMessageHandler: Unable to send reply to client %s", inet_ntop(AF_INET, &context->cliaddr.sin_addr, buf, 32 ) ) );

exit:

    FreeTCPContext( context );

    if ( reply )
    {
        free( reply );
    }

    pthread_exit(NULL);
}


mDNSlocal void
RecvTCPMessage
(
    void * param
)
{
    TCPContext      *   context = ( TCPContext* ) param;
    mDNSu16 rcode;
    mDNSu16 tcode;
    pthread_t tid;
    DomainAuthInfo  *   key;
    PktMsg          *   pkt;
    mDNSBool closed;
    mDNSBool freeContext = mDNStrue;
    mStatus err = mStatus_NoError;

    // Receive a packet.  It's okay if we don't actually read a packet, as long as the closed flag is
    // set to false.  This is because SSL/TLS layer might gobble up the first packet that we read off the
    // wire.  We'll let it do that, and wait for the next packet which will be ours.

    pkt = RecvPacket( context->sock, &context->pkt, &closed );
    if (pkt) HdrNToH(pkt);
    require_action( pkt || !closed, exit, err = mStatus_UnknownErr; LogMsg( "client disconnected" ) );

    if ( pkt )
    {
        // Always do this, regardless of what kind of packet it is.  If we wanted LLQ events to be sent over TCP,
        // we would change this line of code.  As it is now, we will reply to an LLQ via TCP, but then events
        // are sent over UDP

        RemoveSourceFromEventLoop( context->d, context->sock );

        // Set's the DNS Zone that is associated with this message

        SetZone( context->d, &context->pkt );

        // IsAuthorized will make sure the message is authorized for the designated zone.
        // After verifying the signature, it will strip the TSIG from the message

        if ( IsAuthorized( context->d, &context->pkt, &key, &rcode, &tcode ) )
        {
            if ( IsLLQRequest( &context->pkt ) )
            {
                // LLQ messages handled by main thread
                RecvLLQ( context->d, &context->pkt, context->sock);
            }
            else
            {
                err = pthread_create( &tid, NULL, TCPMessageHandler, context );

                if ( err )
                {
                    LogErr( "RecvTCPMessage", "pthread_create" );
                    err = mStatus_NoError;
                    goto exit;
                }

                // Let the thread free the context

                freeContext = mDNSfalse;

                pthread_detach(tid);
            }
        }
        else
        {
            PktMsg reply;

            LogMsg( "Client %s Not authorized for zone %##s", inet_ntoa( context->pkt.src.sin_addr ), pkt->zone->name.c );

            memcpy( &reply, &context->pkt, sizeof( PktMsg ) );

            reply.msg.h.flags.b[0]  =  kDNSFlag0_QR_Response | kDNSFlag0_AA | kDNSFlag0_RD;
            reply.msg.h.flags.b[1]  =  kDNSFlag1_RA | kDNSFlag1_RC_Refused;

            SendPacket( context->sock, &reply );
        }
    }
    else
    {
        freeContext = mDNSfalse;
    }

exit:

    if ( err )
    {
        RemoveSourceFromEventLoop( context->d, context->sock );
    }

    if ( freeContext )
    {
        FreeTCPContext( context );
    }
}


mDNSlocal int
AcceptTCPConnection
(
    DaemonInfo      *   self,
    int sd,
    TCPSocketFlags flags
)
{
    TCPContext *    context = NULL;
    unsigned int clilen = sizeof( context->cliaddr);
    int newSock;
    mStatus err = mStatus_NoError;

    context = ( TCPContext* ) malloc( sizeof( TCPContext ) );
    require_action( context, exit, err = mStatus_NoMemoryErr; LogErr( "AcceptTCPConnection", "malloc" ) );
    mDNSPlatformMemZero( context, sizeof( sizeof( TCPContext ) ) );
    context->d       = self;
    newSock = accept( sd, ( struct sockaddr* ) &context->cliaddr, &clilen );
    require_action( newSock != -1, exit, err = mStatus_UnknownErr; LogErr( "AcceptTCPConnection", "accept" ) );

    context->sock = mDNSPlatformTCPAccept( flags, newSock );
    require_action( context->sock, exit, err = mStatus_UnknownErr; LogErr( "AcceptTCPConnection", "mDNSPlatformTCPAccept" ) );

    err = AddSourceToEventLoop( self, context->sock, RecvTCPMessage, context );
    require_action( !err, exit, LogErr( "AcceptTCPConnection", "AddSourceToEventLoop" ) );

exit:

    if ( err && context )
    {
        free( context );
        context = NULL;
    }

    return err;
}


// main event loop
// listen for incoming requests, periodically check table for expired records, respond to signals
mDNSlocal int Run(DaemonInfo *d)
{
    int staticMaxFD, nfds;
    fd_set rset;
    struct timeval timenow, timeout, EventTS, tablecheck = { 0, 0 };
    mDNSBool EventsPending = mDNSfalse;

    VLog("Listening for requests...");

    staticMaxFD = 0;

    if ( d->tcpsd + 1  > staticMaxFD ) staticMaxFD = d->tcpsd + 1;
    if ( d->udpsd + 1  > staticMaxFD ) staticMaxFD = d->udpsd + 1;
    if ( d->tlssd + 1  > staticMaxFD ) staticMaxFD = d->tlssd + 1;
    if ( d->llq_tcpsd + 1 > staticMaxFD ) staticMaxFD = d->llq_tcpsd + 1;
    if ( d->llq_udpsd + 1 > staticMaxFD ) staticMaxFD = d->llq_udpsd + 1;
    if ( d->LLQEventListenSock + 1 > staticMaxFD ) staticMaxFD = d->LLQEventListenSock + 1;

    while(1)
    {
        EventSource * source;
        int maxFD;

        // set timeout
        timeout.tv_sec = timeout.tv_usec = 0;
        if (gettimeofday(&timenow, NULL)) { LogErr("Run", "gettimeofday"); return -1; }

        if (EventsPending)
        {
            if (timenow.tv_sec - EventTS.tv_sec >= 5)           // if we've been waiting 5 seconds for a "quiet" period to send
            { GenLLQEvents(d); EventsPending = mDNSfalse; }     // events, we go ahead and do it now
            else timeout.tv_usec = 500000;                      // else do events after 1/2 second with no new events or LLQs
        }
        if (!EventsPending)
        {
            // if no pending events, timeout when we need to check for expired records
            if (tablecheck.tv_sec && timenow.tv_sec - tablecheck.tv_sec >= 0)
            { DeleteRecords(d, mDNSfalse); tablecheck.tv_sec = 0; }     // table check overdue
            if (!tablecheck.tv_sec) tablecheck.tv_sec = timenow.tv_sec + EXPIRATION_INTERVAL;
            timeout.tv_sec = tablecheck.tv_sec - timenow.tv_sec;
        }

        FD_ZERO(&rset);
        FD_SET( d->tcpsd, &rset );
        FD_SET( d->udpsd, &rset );
        FD_SET( d->tlssd, &rset );
        FD_SET( d->llq_tcpsd, &rset );
        FD_SET( d->llq_udpsd, &rset );
        FD_SET( d->LLQEventListenSock, &rset );

        maxFD = staticMaxFD;

        for ( source = ( EventSource* ) d->eventSources.Head; source; source = source->next )
        {
            FD_SET( source->fd, &rset );

            if ( source->fd > maxFD )
            {
                maxFD = source->fd;
            }
        }

        nfds = select( maxFD + 1, &rset, NULL, NULL, &timeout);
        if (nfds < 0)
        {
            if (errno == EINTR)
            {
                if (terminate)
                {
                    // close sockets to prevent clients from making new requests during shutdown
                    close( d->tcpsd );
                    close( d->udpsd );
                    close( d->tlssd );
                    close( d->llq_tcpsd );
                    close( d->llq_udpsd );
                    d->tcpsd = d->udpsd = d->tlssd = d->llq_tcpsd = d->llq_udpsd = -1;
                    DeleteRecords(d, mDNStrue);
                    return 0;
                }
                else if (dumptable)
                {
                    Log( "Received SIGINFO" );

                    PrintLeaseTable(d);
                    PrintLLQTable(d);
                    PrintLLQAnswers(d);
                    dumptable = 0;
                }
                else if (hangup)
                {
                    int err;

                    Log( "Received SIGHUP" );

                    err = ParseConfig( d, cfgfile );

                    if ( err )
                    {
                        LogErr( "Run", "ParseConfig" );
                        return -1;
                    }

                    hangup = 0;
                }
                else
                {
                    Log("Received unhandled signal - continuing");
                }
            }
            else
            {
                LogErr("Run", "select"); return -1;
            }
        }
        else if (nfds)
        {
            if (FD_ISSET(d->udpsd, &rset)) RecvUDPMessage( d, d->udpsd );
            if (FD_ISSET(d->llq_udpsd, &rset)) RecvUDPMessage( d, d->llq_udpsd );
            if (FD_ISSET(d->tcpsd, &rset)) AcceptTCPConnection( d, d->tcpsd, 0 );
            if (FD_ISSET(d->llq_tcpsd, &rset)) AcceptTCPConnection( d, d->llq_tcpsd, 0 );
            if (FD_ISSET(d->tlssd, &rset)) AcceptTCPConnection( d, d->tlssd, TCP_SOCKET_FLAGS );
            if (FD_ISSET(d->LLQEventListenSock, &rset))
            {
                // clear signalling data off socket
                char buf[256];
                recv(d->LLQEventListenSock, buf, 256, 0);
                if (!EventsPending)
                {
                    EventsPending = mDNStrue;
                    if (gettimeofday(&EventTS, NULL)) { LogErr("Run", "gettimeofday"); return -1; }
                }
            }

            for ( source = ( EventSource* ) d->eventSources.Head; source; source = source->next )
            {
                if ( FD_ISSET( source->fd, &rset ) )
                {
                    source->callback( source->context );
                    break;  // in case we removed this guy from the event loop
                }
            }
        }
        else
        {
            // timeout
            if (EventsPending) { GenLLQEvents(d); EventsPending = mDNSfalse; }
            else { DeleteRecords(d, mDNSfalse); tablecheck.tv_sec = 0; }
        }
    }
    return 0;
}

// signal handler sets global variables, which are inspected by main event loop
// (select automatically returns due to the handled signal)
mDNSlocal void HndlSignal(int sig)
{
    if (sig == SIGTERM || sig == SIGINT ) { terminate = 1; return; }
    if (sig == INFO_SIGNAL)               { dumptable = 1; return; }
    if (sig == SIGHUP)                    { hangup    = 1; return; }
}

mDNSlocal mStatus
SetPublicSRV
(
    DaemonInfo  *   d,
    const char  *   name
)
{
    DNameListElem * elem;
    mStatus err = mStatus_NoError;

    elem = ( DNameListElem* ) malloc( sizeof( DNameListElem ) );
    require_action( elem, exit, err = mStatus_NoMemoryErr );
    MakeDomainNameFromDNSNameString( &elem->name, name );
    elem->next = d->public_names;
    d->public_names = elem;

exit:

    return err;
}


int main(int argc, char *argv[])
{
    int started_via_launchd = 0;
    DaemonInfo *d;
    struct rlimit rlim;

    Log("dnsextd starting");

    d = malloc(sizeof(*d));
    if (!d) { LogErr("main", "malloc"); exit(1); }
    mDNSPlatformMemZero(d, sizeof(DaemonInfo));

    // Setup the public SRV record names

    SetPublicSRV(d, "_dns-update._udp.");
    SetPublicSRV(d, "_dns-llq._udp.");
    SetPublicSRV(d, "_dns-update-tls._tcp.");
    SetPublicSRV(d, "_dns-query-tls._tcp.");
    SetPublicSRV(d, "_dns-llq-tls._tcp.");

    // Setup signal handling

    if (signal(SIGHUP,      HndlSignal) == SIG_ERR) perror("Can't catch SIGHUP");
    if (signal(SIGTERM,     HndlSignal) == SIG_ERR) perror("Can't catch SIGTERM");
    if (signal(INFO_SIGNAL, HndlSignal) == SIG_ERR) perror("Can't catch SIGINFO");
    if (signal(SIGINT,      HndlSignal) == SIG_ERR) perror("Can't catch SIGINT");
    if (signal(SIGPIPE,     SIG_IGN  )  == SIG_ERR) perror("Can't ignore SIGPIPE");

    // remove open file limit
    rlim.rlim_max = RLIM_INFINITY;
    rlim.rlim_cur = RLIM_INFINITY;
    if (setrlimit(RLIMIT_NOFILE, &rlim) < 0)
    {
        LogErr("main", "setrlimit");
        Log("Using default file descriptor resource limit");
    }

    if (argc > 1 && !strcasecmp(argv[1], "-launchd"))
    {
        Log("started_via_launchd");
        started_via_launchd = 1;
        argv++;
        argc--;
    }
    if (ProcessArgs(argc, argv, d) < 0) { LogErr("main", "ProcessArgs"); exit(1); }

    if (!foreground && !started_via_launchd)
    {
        if (daemon(0,0))
        {
            LogErr("main", "daemon");
            foreground = 1;
        }
    }

    if (InitLeaseTable(d) < 0) { LogErr("main", "InitLeaseTable"); exit(1); }
    if (SetupSockets(d) < 0) { LogErr("main", "SetupSockets"); exit(1); }
    if (SetUpdateSRV(d) < 0) { LogErr("main", "SetUpdateSRV"); exit(1); }

    Run(d);

    Log("dnsextd stopping");

    if (ClearUpdateSRV(d) < 0) { LogErr("main", "ClearUpdateSRV"); exit(1); }  // clear update srv's even if Run or pthread_create returns an error
    free(d);
    exit(0);
}


// These are stubbed out implementations of up-call routines that the various platform support layers
// call.  These routines are fully implemented in both mDNS.c and uDNS.c, but dnsextd doesn't
// link this code in.
//
// It's an error for these routines to actually be called, so perhaps we should log any call
// to them.
void mDNSCoreInitComplete( mDNS * const m, mStatus result) { ( void ) m; ( void ) result; }
void mDNS_ConfigChanged(mDNS *const m)  { ( void ) m; }
void mDNSCoreMachineSleep(mDNS * const m, mDNSBool wake) { ( void ) m; ( void ) wake; }
void mDNSCoreReceive(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
                     const mDNSAddr *const srcaddr, const mDNSIPPort srcport,
                     const mDNSAddr *const dstaddr, const mDNSIPPort dstport, const mDNSInterfaceID iid)
{ ( void ) m; ( void ) msg; ( void ) end; ( void ) srcaddr; ( void ) srcport; ( void ) dstaddr; ( void ) dstport; ( void ) iid; }
DNSServer *mDNS_AddDNSServer(mDNS *const m, const domainname *d, const mDNSInterfaceID interface, const int serviceID, const mDNSAddr *addr, const mDNSIPPort port, 
                             mDNSu32 scopedType, mDNSu32 timeout, mDNSBool isCell, mDNSBool isExpensive, mDNSBool isConstrained,  mDNSBool isCLAT46, mDNSu32 resGroupID, mDNSBool reqA, mDNSBool reqAAAA, mDNSBool reqDO)
{ ( void ) m; ( void ) d; ( void ) interface; ( void ) serviceID; ( void ) addr; ( void ) port; ( void ) scopedType; ( void ) timeout; (void) isCell; (void) isExpensive; (void) isConstrained; (void) isCLAT46;
    (void) resGroupID; (void) reqA; (void) reqAAAA; (void) reqDO; return(NULL); }
void mDNS_AddSearchDomain(const domainname *const domain, mDNSInterfaceID InterfaceID) { (void)domain; (void) InterfaceID;}
void mDNS_AddDynDNSHostName(mDNS *m, const domainname *fqdn, mDNSRecordCallback *StatusCallback, const void *StatusContext)
{ ( void ) m; ( void ) fqdn; ( void ) StatusCallback; ( void ) StatusContext; }
mDNSs32 mDNS_Execute   (mDNS *const m) { ( void ) m; return 0; }
mDNSs32 mDNS_TimeNow(const mDNS *const m) { ( void ) m; return 0; }
mStatus mDNS_Deregister(mDNS *const m, AuthRecord *const rr) { ( void ) m; ( void ) rr; return 0; }
void mDNS_DeregisterInterface(mDNS *const m, NetworkInterfaceInfo *set, InterfaceActivationSpeed activationSpeed)
{ ( void ) m; ( void ) set; ( void ) activationSpeed; }
const char * const mDNS_DomainTypeNames[1] = {};
mStatus mDNS_GetDomains(mDNS *const m, DNSQuestion *const question, mDNS_DomainType DomainType, const domainname *dom,
                        const mDNSInterfaceID InterfaceID, mDNSQuestionCallback *Callback, void *Context)
{ ( void ) m; ( void ) question; ( void ) DomainType; ( void ) dom; ( void ) InterfaceID; ( void ) Callback; ( void ) Context; return 0; }
mStatus mDNS_Register(mDNS *const m, AuthRecord *const rr) { ( void ) m; ( void ) rr; return 0; }
mStatus mDNS_RegisterInterface(mDNS *const m, NetworkInterfaceInfo *set, InterfaceActivationSpeed activationSpeed)
{ ( void ) m; ( void ) set; ( void ) activationSpeed; return 0; }
void mDNS_RemoveDynDNSHostName(mDNS *m, const domainname *fqdn) { ( void ) m; ( void ) fqdn; }
void mDNS_SetFQDN(mDNS * const m) { ( void ) m; }
void mDNS_SetPrimaryInterfaceInfo(mDNS *m, const mDNSAddr *v4addr,  const mDNSAddr *v6addr, const mDNSAddr *router)
{ ( void ) m; ( void ) v4addr; ( void ) v6addr; ( void ) router; }
mStatus uDNS_SetupDNSConfig( mDNS *const m ) { ( void ) m; return 0; }
mStatus mDNS_SetSecretForDomain(mDNS *m, DomainAuthInfo *info,
                                const domainname *domain, const domainname *keyname, const char *b64keydata, const domainname *hostname, mDNSIPPort *port)
{ ( void ) m; ( void ) info; ( void ) domain; ( void ) keyname; ( void ) b64keydata; ( void ) hostname; (void) port; return 0; }
mStatus mDNS_StopQuery(mDNS *const m, DNSQuestion *const question) { ( void ) m; ( void ) question; return 0; }
void TriggerEventCompletion(void);
void TriggerEventCompletion() {}
mDNS mDNSStorage;


// For convenience when using the "strings" command, this is the last thing in the file
// The "@(#) " pattern is a special prefix the "what" command looks for
const char mDNSResponderVersionString_SCCS[] = "@(#) dnsextd " STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";

#if _BUILDING_XCODE_PROJECT_
// If the process crashes, then this string will be magically included in the automatically-generated crash log
const char *__crashreporter_info__ = mDNSResponderVersionString_SCCS + 5;
asm (".desc ___crashreporter_info__, 0x10");
#endif
