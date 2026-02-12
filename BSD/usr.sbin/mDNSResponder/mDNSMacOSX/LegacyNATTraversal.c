/*
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
 */

#ifdef _LEGACY_NAT_TRAVERSAL_

#include "stdlib.h"         // For strtol()
#include "string.h"         // For strlcpy(), For strncpy(), strncasecmp()
#include "assert.h"         // For assert()

#if defined( WIN32 )
#   include "CommonServices.h"
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   define strcasecmp   _stricmp
#   define strncasecmp  _strnicmp

static int
inet_pton( int family, const char * addr, void * dst )
{
    struct sockaddr_storage ss;
    int sslen = sizeof( ss );

    ZeroMemory( &ss, sizeof( ss ) );
    ss.ss_family = (ADDRESS_FAMILY)family;

    if ( WSAStringToAddressA( (LPSTR)addr, family, NULL, ( struct sockaddr* ) &ss, &sslen ) == 0 )
    {
        if ( family == AF_INET ) { memcpy( dst, &( ( struct sockaddr_in* ) &ss)->sin_addr, sizeof( IN_ADDR ) ); return 1; }
        else if ( family == AF_INET6 ) { memcpy( dst, &( ( struct sockaddr_in6* ) &ss)->sin6_addr, sizeof( IN6_ADDR ) ); return 1; }
        else return 0;
    }
    else return 0;
}
#else
#   include <arpa/inet.h>       // For inet_pton()
#endif

#include "mDNSEmbeddedAPI.h"
#include "uDNS.h"           // For natTraversalHandleAddressReply() etc.

// used to format SOAP port mapping arguments
typedef struct Property_struct
{
    char *name;
    char *type;
    char *value;
} Property;

// All of the text parsing in this file is intentionally transparent so that we know exactly
// what's being done to the text, with an eye towards preventing security problems.

// This is an evolving list of useful acronyms to know. Please add to it at will.
// ST      Service Type
// NT      Notification Type
// USN     Unique Service Name
// UDN     Unique Device Name
// UUID    Universally Unique Identifier
// URN/urn Universal Resource Name

// Forward declaration because of circular reference:
// SendPortMapRequest -> SendSOAPMsgControlAction -> MakeTCPConnection -> tcpConnectionCallback -> handleLNTPortMappingResponse
// In the event of a port conflict, handleLNTPortMappingResponse then increments tcpInfo->retries and calls back to SendPortMapRequest to try again
mDNSlocal mStatus SendPortMapRequest(mDNS *m, NATTraversalInfo *n);

#define RequestedPortNum(n) (mDNSVal16(mDNSIPPortIsZero((n)->RequestedPort) ? (n)->IntPort : (n)->RequestedPort) + (mDNSu16)(n)->tcpInfo.retries)

// Note that this function assumes src is already NULL terminated
mDNSlocal void AllocAndCopy(char **const dst, const char *const src)
{
    size_t srcLen;
    if (src == mDNSNULL) return;
    srcLen = strlen(src) + 1;
    if ((srcLen > UINT32_MAX) || ((*dst = mDNSPlatformMemAllocate((mDNSu32)srcLen)) == mDNSNULL))
    {
        LogMsg("AllocAndCopy: can't allocate string");
        return;
    }
    memcpy(*dst, src, srcLen);
}

// This function does a simple parse of an HTTP URL that may include a hostname, port, and path
// If found in the URL, addressAndPort and path out params will point to newly allocated space (and will leak if they were previously pointing at allocated space)
mDNSlocal mStatus ParseHttpUrl(const mDNSu8 *ptr, const mDNSu8 *const end, char **const addressAndPort, mDNSIPPort *const port, char **const path)
{
    // if the data begins with "http://", we assume there is a hostname and possibly a port number
    if (end - ptr >= 7 && strncasecmp((char*)ptr, "http://", 7) == 0)
    {
        int i;
        const mDNSu8 *stop = end;
        const mDNSu8 *addrPtr = mDNSNULL;

        ptr += 7; //skip over "http://"
        if (ptr >= end) { LogInfo("ParseHttpUrl: past end of buffer parsing host:port"); return mStatus_BadParamErr; }

        // find the end of the host:port
        addrPtr = ptr;
        for (i = 0; addrPtr && addrPtr != end; i++, addrPtr++) if (*addrPtr == '/') break;

        // allocate the buffer (len i+1 so we have space to terminate the string)
        if ((*addressAndPort = mDNSPlatformMemAllocate(i+1)) == mDNSNULL)
        { LogMsg("ParseHttpUrl: can't allocate address string"); return mStatus_NoMemoryErr; }
        strncpy((char*)*addressAndPort, (char*)ptr, i);
        (*addressAndPort)[i] = '\0';

        // find the port number in the string, by looking backwards for the ':'
        stop = ptr;    // can't go back farther than the original start
        ptr = addrPtr; // move ptr to the path part

        for (addrPtr--; addrPtr>stop; addrPtr--)
        {
            if (*addrPtr == ':')
            {
                addrPtr++; // skip over ':'
                *port = mDNSOpaque16fromIntVal((mDNSu16)strtol((char*)addrPtr, mDNSNULL, 10)); // store it properly converted
                break;
            }
        }
    }

    // ptr should now point to the first character we haven't yet processed
    // everything that remains is the path
    if (path && ptr < end)
    {
        if ((*path = mDNSPlatformMemAllocate((mDNSu32)(end - ptr) + 1)) == mDNSNULL)
        { LogMsg("ParseHttpUrl: can't mDNSPlatformMemAllocate path"); return mStatus_NoMemoryErr; }
        strncpy((char*)*path, (char*)ptr, end - ptr);
        (*path)[end - ptr] = '\0';
    }

    return mStatus_NoError;
}

enum
{
    HTTPCode_NeedMoreData = -1, // No code found in stream
    HTTPCode_Other        = -2, // Valid code other than those below found in stream
    HTTPCode_Bad          = -3,
    HTTPCode_200          = 200,
    HTTPCode_404          = 404,
    HTTPCode_500          = 500,
};

mDNSlocal mDNSs16 ParseHTTPResponseCode(const mDNSu8 **const data, const mDNSu8 *const end)
{
    const mDNSu8 *ptr = *data;
    const mDNSu8 *code;

    if (end - ptr < 5) return HTTPCode_NeedMoreData;
    if (strncasecmp((char*)ptr, "HTTP/", 5) != 0) return HTTPCode_Bad;
    ptr += 5;
    // should we care about the HTTP protocol version?

    // look for first space, which must come before first LF
    while (ptr && ptr != end)
    {
        if (*ptr == '\n') return HTTPCode_Bad;
        if (*ptr == ' ') break;
        ptr++;
    }
    if (ptr == end) return HTTPCode_NeedMoreData;
    ptr++;

    if (end - ptr < 3) return HTTPCode_NeedMoreData;

    code = ptr;
    ptr += 3;
    while (ptr && ptr != end)
    {
        if (*ptr == '\n') break;
        ptr++;
    }
    if (ptr == end) return HTTPCode_NeedMoreData;
    *data = ++ptr;

    if (memcmp((char*)code, "200", 3) == 0) return HTTPCode_200;
    if (memcmp((char*)code, "404", 3) == 0) return HTTPCode_404;
    if (memcmp((char*)code, "500", 3) == 0) return HTTPCode_500;

    LogInfo("ParseHTTPResponseCode found unexpected result code: %c%c%c", code[0], code[1], code[2]);
    return HTTPCode_Other;
}

// This function parses the xml body of the device description response from the router. Basically, we look to
// make sure this is a response referencing a service we care about (WANIPConnection or WANPPPConnection),
// look for the "controlURL" header immediately following, and copy the addressing and URL info we need
mDNSlocal void handleLNTDeviceDescriptionResponse(tcpLNTInfo *tcpInfo)
{
    mDNS    *m    = tcpInfo->m;
    const mDNSu8 *ptr  = tcpInfo->Reply;
    const mDNSu8 *end  = tcpInfo->Reply + tcpInfo->nread;
    const mDNSu8 *stop;
    mDNSs16 http_result;

    if (!mDNSIPPortIsZero(m->UPnPSOAPPort)) return; // already have the info we need

    http_result = ParseHTTPResponseCode(&ptr, end); // Note: modifies ptr
    if (http_result == HTTPCode_404) LNT_ClearState(m);
    if (http_result != HTTPCode_200)
    {
        LogInfo("handleLNTDeviceDescriptionResponse: HTTP Result code: %d", http_result);
        return;
    }

    // Always reset our flag to use WANIPConnection.  We'll use WANPPPConnection if we find it and don't find WANIPConnection.
    m->UPnPWANPPPConnection = mDNSfalse;

    // find either service we care about
    while (ptr && ptr < end)
    {
        if ((*ptr & 0xDF) == 'W' && (strncasecmp((char*)ptr, "WANIPConnection:1", 17) == 0)) break;
        ptr++;
    }
    if (ptr == end)
    {
        ptr = tcpInfo->Reply;
        while (ptr && ptr < end)
        {
            if ((*ptr & 0xDF) == 'W' && (strncasecmp((char*)ptr, "WANPPPConnection:1", 18) == 0))
            {
                m->UPnPWANPPPConnection = mDNStrue;
                break;
            }
            ptr++;
        }
    }
    if (ptr == mDNSNULL || ptr == end) { LogInfo("handleLNTDeviceDescriptionResponse: didn't find WANIPConnection:1 or WANPPPConnection:1 string"); return; }

    // find "controlURL", starting from where we left off
    while (ptr && ptr < end)
    {
        if ((*ptr & 0xDF) == 'C' && (strncasecmp((char*)ptr, "controlURL", 10) == 0)) break;            // find the first 'c'; is this controlURL? if not, keep looking
        ptr++;
    }
    if (ptr == mDNSNULL || ptr == end) { LogInfo("handleLNTDeviceDescriptionResponse: didn't find controlURL string"); return; }
    ptr += 11;                          // skip over "controlURL>"
    if (ptr >= end) { LogInfo("handleLNTDeviceDescriptionResponse: past end of buffer and no body!"); return; } // check ptr again in case we skipped over the end of the buffer

    // find the end of the controlURL element
    for (stop = ptr; stop < end; stop++) { if (*stop == '<') { end = stop; break; } }

    // fill in default port
    m->UPnPSOAPPort = m->UPnPRouterPort;

    // free string pointers and set to NULL
    if (m->UPnPSOAPAddressString != mDNSNULL)
    {
        mDNSPlatformMemFree(m->UPnPSOAPAddressString);
        m->UPnPSOAPAddressString = mDNSNULL;
    }
    if (m->UPnPSOAPURL != mDNSNULL)
    {
        mDNSPlatformMemFree(m->UPnPSOAPURL);
        m->UPnPSOAPURL = mDNSNULL;
    }

    if (ParseHttpUrl(ptr, end, &m->UPnPSOAPAddressString, &m->UPnPSOAPPort, &m->UPnPSOAPURL) != mStatus_NoError) return;
    // the SOAPURL should look something like "/uuid:0013-108c-4b3f0000f3dc"

    if (m->UPnPSOAPAddressString == mDNSNULL)
    {
        ptr = tcpInfo->Reply;
        while (ptr && ptr < end)
        {
            if ((*ptr & 0xDF) == 'U' && (strncasecmp((char*)ptr, "URLBase", 7) == 0)) break;
            ptr++;
        }

        if (ptr < end)      // found URLBase
        {
            LogInfo("handleLNTDeviceDescriptionResponse: found URLBase");
            ptr += 8; // skip over "URLBase>"
            // find the end of the URLBase element
            for (stop = ptr; stop < end; stop++) { if (stop && *stop == '<') { end = stop; break; } }
            if (ParseHttpUrl(ptr, end, &m->UPnPSOAPAddressString, &m->UPnPSOAPPort, mDNSNULL) != mStatus_NoError)
            {
                LogInfo("handleLNTDeviceDescriptionResponse: failed to parse URLBase");
            }
        }

        // if all else fails, use the router address string
        if (m->UPnPSOAPAddressString == mDNSNULL) AllocAndCopy(&m->UPnPSOAPAddressString, m->UPnPRouterAddressString);
    }
    if (m->UPnPSOAPAddressString == mDNSNULL) LogMsg("handleLNTDeviceDescriptionResponse: UPnPSOAPAddressString is NULL");
    else LogInfo("handleLNTDeviceDescriptionResponse: SOAP address string [%s]", m->UPnPSOAPAddressString);

    if (m->UPnPSOAPURL == mDNSNULL) AllocAndCopy(&m->UPnPSOAPURL, m->UPnPRouterURL);
    if (m->UPnPSOAPURL == mDNSNULL) LogMsg("handleLNTDeviceDescriptionResponse: UPnPSOAPURL is NULL");
    else LogInfo("handleLNTDeviceDescriptionResponse: SOAP URL [%s]", m->UPnPSOAPURL);
}

mDNSlocal void handleLNTGetExternalAddressResponse(tcpLNTInfo *tcpInfo)
{
    mDNS       *m = tcpInfo->m;
    mDNSu16 err = NATErr_None;
    mDNSv4Addr ExtAddr;
    const mDNSu8 *ptr = tcpInfo->Reply;
    const mDNSu8 *end = tcpInfo->Reply + tcpInfo->nread;
    mDNSu8       *addrend;
    static char tagname[20] = { 'N','e','w','E','x','t','e','r','n','a','l','I','P','A','d','d','r','e','s','s' };
    // Array NOT including a terminating nul

//	LogInfo("handleLNTGetExternalAddressResponse: %s", ptr);

    mDNSs16 http_result = ParseHTTPResponseCode(&ptr, end); // Note: modifies ptr
    if (http_result == HTTPCode_404) LNT_ClearState(m);
    if (http_result != HTTPCode_200)
    {
        LogInfo("handleLNTGetExternalAddressResponse: HTTP Result code: %d", http_result);
        return;
    }

    while (ptr < end && strncasecmp((char*)ptr, tagname, sizeof(tagname))) ptr++;
    ptr += sizeof(tagname);                     // Skip over "NewExternalIPAddress"
    while (ptr < end && *ptr != '>') ptr++;
    ptr += 1;                                   // Skip over ">"

    // Find the end of the address and terminate the string so inet_pton() can convert it
    // (Might be better to copy this to a local string here -- this is overwriting tcpInfo->Reply in-place
    addrend = (mDNSu8*)ptr;
    while (addrend < end && (mDNSIsDigit(*addrend) || *addrend == '.')) addrend++;
    if (addrend >= end) return;
    *addrend = 0;

    if (inet_pton(AF_INET, (char*)ptr, &ExtAddr) <= 0)
    {
        LogMsg("handleLNTGetExternalAddressResponse: Router returned bad address %s", ptr);
        err = NATErr_NetFail;
        ExtAddr = zerov4Addr;
    }
    if (!err) LogInfo("handleLNTGetExternalAddressResponse: External IP address is %.4a", &ExtAddr);

    natTraversalHandleAddressReply(m, err, ExtAddr);
}

mDNSlocal void handleLNTPortMappingResponse(tcpLNTInfo *tcpInfo)
{
    mDNS             *m         = tcpInfo->m;
    mDNSIPPort extport   = zeroIPPort;
    const mDNSu8     *ptr       = tcpInfo->Reply;
    const mDNSu8     *const end = tcpInfo->Reply + tcpInfo->nread;
    NATTraversalInfo *natInfo;
    mDNSs16 http_result;

    for (natInfo = m->NATTraversals; natInfo; natInfo=natInfo->next) { if (natInfo == tcpInfo->parentNATInfo) break;}

    if (!natInfo) { LogInfo("handleLNTPortMappingResponse: can't find matching tcpInfo in NATTraversals!"); return; }

    http_result = ParseHTTPResponseCode(&ptr, end); // Note: modifies ptr
    if (http_result == HTTPCode_200)
    {
        LogInfo("handleLNTPortMappingResponse: got a valid response, sending reply to natTraversalHandlePortMapReply(internal %d external %d retries %d)",
                mDNSVal16(natInfo->IntPort), RequestedPortNum(natInfo), tcpInfo->retries);

        // Make sure to compute extport *before* we zero tcpInfo->retries
        extport = mDNSOpaque16fromIntVal(RequestedPortNum(natInfo));
        tcpInfo->retries = 0;
        natTraversalHandlePortMapReply(m, natInfo, m->UPnPInterfaceID, mStatus_NoError, extport, NATMAP_DEFAULT_LEASE, NATTProtocolUPNPIGD);
    }
    else if (http_result == HTTPCode_500)
    {
        while (ptr && ptr != end)
        {
            if (((*ptr & 0xDF) == 'C' && end - ptr >= 8 && strncasecmp((char*)ptr, "Conflict", 8) == 0) ||
                (*ptr == '>' && end - ptr >= 15 && strncasecmp((char*)ptr, ">718</errorCode", 15) == 0))
            {
                if (tcpInfo->retries < 100)
                {
                    tcpInfo->retries++; SendPortMapRequest(tcpInfo->m, natInfo);
                    LogInfo("handleLNTPortMappingResponse: Conflict retry %d", tcpInfo->retries);
                }
                else
                {
                    LogMsg("handleLNTPortMappingResponse too many conflict retries %d %d", mDNSVal16(natInfo->IntPort), mDNSVal16(natInfo->RequestedPort));
                    natTraversalHandlePortMapReply(m, natInfo, m->UPnPInterfaceID, NATErr_Res, zeroIPPort, 0, NATTProtocolUPNPIGD);
                }
                return;
            }
            ptr++;
        }
    }
    else if (http_result == HTTPCode_Bad) LogMsg("handleLNTPortMappingResponse got data that was not a valid HTTP response");
    else if (http_result == HTTPCode_Other) LogMsg("handleLNTPortMappingResponse got unexpected response code");
    else if (http_result == HTTPCode_404) LNT_ClearState(m);
    if (http_result != HTTPCode_200 && http_result != HTTPCode_500)
        LogInfo("handleLNTPortMappingResponse: HTTP Result code: %d", http_result);
}

mDNSlocal void DisposeInfoFromUnmapList(mDNS *m, tcpLNTInfo *tcpInfo)
{
    tcpLNTInfo **ptr = &m->tcpInfoUnmapList;
    while (*ptr && *ptr != tcpInfo) ptr = &(*ptr)->next;
    if (*ptr) { *ptr = (*ptr)->next; mDNSPlatformMemFree(tcpInfo); }    // If we found it, cut it from our list and free the memory
}

mDNSlocal void tcpConnectionCallback(TCPSocket *sock, void *context, mDNSBool ConnectionEstablished, mStatus err)
{
    mStatus status  = mStatus_NoError;
    tcpLNTInfo *tcpInfo = (tcpLNTInfo *)context;
    mDNSBool closed  = mDNSfalse;
    long n       = 0;
    long nsent   = 0;
    static mDNSu32 LNTERRORcount = 0;

    if (tcpInfo->sock != sock)
    {
        LogMsg("tcpConnectionCallback: WARNING- tcpInfo->sock(%p) != sock(%p) !!! Printing tcpInfo struct", tcpInfo->sock, sock);
        LogMsg("tcpConnectionCallback: tcpInfo->Address:Port [%#a:%d] tcpInfo->op[%d] tcpInfo->retries[%d] tcpInfo->Request[%s] tcpInfo->Reply[%s]", 
                &tcpInfo->Address, mDNSVal16(tcpInfo->Port), tcpInfo->op, tcpInfo->retries, tcpInfo->Request, tcpInfo->Reply);  
    }
        
    // The handlers below expect to be called with the lock held
    mDNS_Lock(tcpInfo->m);

    if (err) { LogInfo("tcpConnectionCallback: received error"); goto exit; }

    if (ConnectionEstablished)      // connection is established - send the message
    {
        LogInfo("tcpConnectionCallback: connection established, sending message");
        nsent = mDNSPlatformWriteTCP(sock, (char*)tcpInfo->Request, tcpInfo->requestLen);
        if (nsent != (long)tcpInfo->requestLen) { LogMsg("tcpConnectionCallback: error writing"); status = mStatus_UnknownErr; goto exit; }
    }
    else
    {
        n = mDNSPlatformReadTCP(sock, (char*)tcpInfo->Reply + tcpInfo->nread, tcpInfo->replyLen - tcpInfo->nread, &closed);
        LogInfo("tcpConnectionCallback: mDNSPlatformReadTCP read %d bytes", n);

        if      (n < 0)  { LogInfo("tcpConnectionCallback - read returned %d", n);                           status = mStatus_ConnFailed; goto exit; }
        else if (closed) { LogInfo("tcpConnectionCallback: socket closed by remote end %d", tcpInfo->nread); status = mStatus_ConnFailed; goto exit; }

        tcpInfo->nread += n;
        LogInfo("tcpConnectionCallback tcpInfo->nread %d", tcpInfo->nread);
        if (tcpInfo->nread > LNT_MAXBUFSIZE)
        {
            LogInfo("result truncated...");
            tcpInfo->nread = LNT_MAXBUFSIZE;
        }

        switch (tcpInfo->op)
        {
        case LNTDiscoveryOp:     handleLNTDeviceDescriptionResponse (tcpInfo); break;
        case LNTExternalAddrOp:  handleLNTGetExternalAddressResponse(tcpInfo); break;
        case LNTPortMapOp:       handleLNTPortMappingResponse       (tcpInfo); break;
        case LNTPortMapDeleteOp: status = mStatus_ConfigChanged;               break;
        default: LogMsg("tcpConnectionCallback: bad tcp operation! %d", tcpInfo->op); status = mStatus_Invalid; break;
        }
    }
exit:
    if (err || status)
    {
        mDNS *const m = tcpInfo->m;
        static mDNSs32 lastErrorTime = 0;

        if ((LNTERRORcount > 0) && (((mDNSu32)(m->timenow - lastErrorTime)) >= ((mDNSu32)mDNSPlatformOneSecond)))
        {
            LNTERRORcount = 0;
        }
        lastErrorTime = m->timenow;
        if ((++LNTERRORcount % 1000) == 0)
        {   
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                "ERROR: tcpconnectioncallback -> got error status %u times", LNTERRORcount);
            assert(LNTERRORcount < 1000);
            // Recovery Mechanism to bail mDNSResponder out of trouble: It has been seen that we can get into 
            // this loop: [tcpKQSocketCallback()--> doTcpSocketCallback()-->tcpconnectionCallback()-->mDNSASLLog()],
            // if mDNSPlatformTCPCloseConnection() does not close the TCPSocket. Instead of calling mDNSASLLog()
            // repeatedly and logging the same error msg causing 100% CPU usage, we 
            // crash mDNSResponder using assert() and restart fresh. See advantages below:
            // 1.Better User Experience 
            // 2.CrashLogs frequency can be monitored 
            // 3.StackTrace can be used for more info 
        }   

        switch (tcpInfo->op)
        {
        case LNTDiscoveryOp:
            LogInfo("tcpConnectionCallback: DeviceDescription SOAP address %s SOAP path %s",
                m->UPnPSOAPAddressString ? m->UPnPSOAPAddressString : "NULL", m->UPnPSOAPURL ? m->UPnPSOAPURL : "NULL");
            break;
        case LNTExternalAddrOp:
            LogInfo("tcpConnectionCallback: AddressRequest %s", mDNSIPv4AddressIsZero(m->ExtAddress) ? "failure" : "success");
            break;
        case LNTPortMapOp:
            if (tcpInfo->parentNATInfo)
                LogInfo("tcpConnectionCallback: PortMapRequest %s result %d",
                    (tcpInfo->parentNATInfo->Result) ? "failure" : "success", tcpInfo->parentNATInfo->Result);
            break;
        case LNTPortMapDeleteOp: break;
        default:                 break;
        }

        mDNSPlatformTCPCloseConnection(sock);
        tcpInfo->sock = mDNSNULL;
        if (tcpInfo->Request) { mDNSPlatformMemFree(tcpInfo->Request); tcpInfo->Request = mDNSNULL; }
        if (tcpInfo->Reply  ) { mDNSPlatformMemFree(tcpInfo->Reply);   tcpInfo->Reply   = mDNSNULL; }
    }
    else
    {
        LNTERRORcount = 0;  // clear LNTERRORcount
    }

    if (tcpInfo) mDNS_Unlock(tcpInfo->m);

    if (status == mStatus_ConfigChanged) DisposeInfoFromUnmapList(tcpInfo->m, tcpInfo);
}

mDNSlocal mStatus MakeTCPConnection(mDNS *const m, tcpLNTInfo *info, const mDNSAddr *const Addr, const mDNSIPPort Port, LNTOp_t op)
{
    mStatus err = mStatus_NoError;
    mDNSIPPort srcport = zeroIPPort;

    if (mDNSIPv4AddressIsZero(Addr->ip.v4) || mDNSIPPortIsZero(Port))
    { LogMsg("LNT MakeTCPConnection: bad address/port %#a:%d", Addr, mDNSVal16(Port)); return(mStatus_Invalid); }
    info->m         = m;
    info->Address   = *Addr;
    info->Port      = Port;
    info->op        = op;
    info->nread     = 0;
    info->replyLen  = LNT_MAXBUFSIZE;
    if      (info->Reply != mDNSNULL) mDNSPlatformMemZero(info->Reply, LNT_MAXBUFSIZE);   // reuse previously allocated buffer
    else if ((info->Reply = mDNSPlatformMemAllocate(LNT_MAXBUFSIZE)) == mDNSNULL) { LogInfo("can't allocate reply buffer"); return (mStatus_NoMemoryErr); }

    if (info->sock) { LogInfo("MakeTCPConnection: closing previous open connection"); mDNSPlatformTCPCloseConnection(info->sock); info->sock = mDNSNULL; }
    info->sock = mDNSPlatformTCPSocket(kTCPSocketFlags_Zero, Addr->type, &srcport, mDNSNULL, mDNSfalse);
    if (!info->sock) { LogMsg("LNT MakeTCPConnection: unable to create TCP socket"); mDNSPlatformMemFree(info->Reply); info->Reply = mDNSNULL; return(mStatus_NoMemoryErr); }
    LogInfo("MakeTCPConnection: connecting to %#a:%d", &info->Address, mDNSVal16(info->Port));
    err = mDNSPlatformTCPConnect(info->sock, Addr, Port, 0, tcpConnectionCallback, info);

    if      (err == mStatus_ConnPending) err = mStatus_NoError;
    else if (err == mStatus_ConnEstablished)
    {
        mDNS_DropLockBeforeCallback();
        tcpConnectionCallback(info->sock, info, mDNStrue, mStatus_NoError);
        mDNS_ReclaimLockAfterCallback();
        err = mStatus_NoError;
    }
    else
    {
        // Don't need to log this in customer builds -- it happens quite often during sleep, wake, configuration changes, etc.
        LogInfo("LNT MakeTCPConnection: connection failed");
        mDNSPlatformTCPCloseConnection(info->sock); // Dispose the socket we created with mDNSPlatformTCPSocket() above
        info->sock = mDNSNULL;
        mDNSPlatformMemFree(info->Reply);
        info->Reply = mDNSNULL;
    }
    return(err);
}

mDNSlocal unsigned int AddSOAPArguments(char *const buf, const unsigned int maxlen, const int numArgs, const Property *const a)
{
    static const char f1[] = "<%s>%s</%s>";
    static const char f2[] = "<%s xmlns:dt=\"urn:schemas-microsoft-com:datatypes\" dt:dt=\"%s\">%s</%s>";
    int i, len = 0;
    *buf = 0;
    for (i = 0; i < numArgs; i++)
    {
        if (a[i].type) len += mDNS_snprintf(buf + len, maxlen - len, f2, a[i].name, a[i].type, a[i].value, a[i].name);
        else len += mDNS_snprintf(buf + len, maxlen - len, f1, a[i].name,            a[i].value, a[i].name);
    }
    return(len);
}

mDNSlocal mStatus SendSOAPMsgControlAction(mDNS *m, tcpLNTInfo *info, const char *const Action, const int numArgs, const Property *const Arguments, const LNTOp_t op)
{
    // SOAP message header format -
    //  - control URL
    //  - action (string)
    //  - router's host/port ("host:port")
    //  - content-length
    static const char header[] =
        "POST %s HTTP/1.1\r\n"
        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
        "SOAPAction: \"urn:schemas-upnp-org:service:WAN%sConnection:1#%s\"\r\n"
        "User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)\r\n"
        "Host: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Pragma: no-cache\r\n"
        "\r\n"
        "%s\r\n";

    static const char body1[] =
        "<?xml version=\"1.0\"?>\r\n"
        "<SOAP-ENV:Envelope"
        " xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<SOAP-ENV:Body>"
        "<m:%s xmlns:m=\"urn:schemas-upnp-org:service:WAN%sConnection:1\">";

    static const char body2[] =
        "</m:%s>"
        "</SOAP-ENV:Body>"
        "</SOAP-ENV:Envelope>\r\n";

    mStatus err;
    char   *body = (char*)&m->omsg;         // Typically requires 1110-1122 bytes; m->omsg is 8952 bytes, which is plenty
    int bodyLen;

    if (mDNSIPPortIsZero(m->UPnPSOAPPort) || m->UPnPSOAPURL == mDNSNULL || m->UPnPSOAPAddressString == mDNSNULL)    // if no SOAP URL or address exists get out here
    { LogInfo("SendSOAPMsgControlAction: no SOAP port, URL or address string"); return mStatus_Invalid; }

    // Create body
    bodyLen  = mDNS_snprintf   (body,           sizeof(m->omsg),           body1,   Action,   m->UPnPWANPPPConnection ? "PPP" : "IP");
    bodyLen += AddSOAPArguments(body + bodyLen, sizeof(m->omsg) - bodyLen, numArgs, Arguments);
    bodyLen += mDNS_snprintf   (body + bodyLen, sizeof(m->omsg) - bodyLen, body2,   Action);

    // Create info->Request; the header needs to contain the bodyLen in the "Content-Length" field
    if (!info->Request) info->Request = mDNSPlatformMemAllocate(LNT_MAXBUFSIZE);
    if (!info->Request) { LogMsg("SendSOAPMsgControlAction: Can't allocate info->Request"); return mStatus_NoMemoryErr; }
    info->requestLen = mDNS_snprintf((char*)info->Request, LNT_MAXBUFSIZE, header, m->UPnPSOAPURL, m->UPnPWANPPPConnection ? "PPP" : "IP", Action, m->UPnPSOAPAddressString, bodyLen, body);

    err = MakeTCPConnection(m, info, &m->Router, m->UPnPSOAPPort, op);
    if (err) { mDNSPlatformMemFree(info->Request); info->Request = mDNSNULL; }
    return err;
}

// Build port mapping request with new port (up to max) and send it
mDNSlocal mStatus SendPortMapRequest(mDNS *m, NATTraversalInfo *n)
{
    char externalPort[6];
    char internalPort[6];
    char localIPAddrString[30];
    char publicPortString[40];
    Property propArgs[8];
    mDNSu16 ReqPortNum = RequestedPortNum(n);
    NATTraversalInfo *n2 = m->NATTraversals;

    // Scan our m->NATTraversals list to make sure the external port we're requesting is locally unique.
    // UPnP gateways will report conflicts if different devices request the same external port, but if two
    // clients on the same device request the same external port the second one just stomps over the first.
    // One way this can happen is like this:
    // 1. Client A binds local port 80
    // 2. Client A requests external port 80 -> internal port 80
    // 3. UPnP NAT gateway refuses external port 80 (some other client already has it)
    // 4. Client A tries again, and successfully gets external port 80 -> internal port 81
    // 5. Client B on same machine tries to bind local port 80, and fails
    // 6. Client B tries again, and successfully binds local port 81
    // 7. Client B now requests external port 81 -> internal port 81
    // 8. UPnP NAT gateway allows this, stomping over Client A's existing mapping

    while (n2)
    {
        if (n2 == n || RequestedPortNum(n2) != ReqPortNum) n2=n2->next;
        else
        {
            if (n->tcpInfo.retries < 100)
            {
                n->tcpInfo.retries++;
                ReqPortNum = RequestedPortNum(n);   // Pick a new port number
                n2 = m->NATTraversals;              // And re-scan the list looking for conflicts
            }
            else
            {
                natTraversalHandlePortMapReply(m, n, m->UPnPInterfaceID, NATErr_Res, zeroIPPort, 0, NATTProtocolUPNPIGD);
                return mStatus_NoError;
            }
        }
    }

    // create strings to use in the message
    mDNS_snprintf(externalPort,      sizeof(externalPort),      "%u",   ReqPortNum);
    mDNS_snprintf(internalPort,      sizeof(internalPort),      "%u",   mDNSVal16(n->IntPort));
    mDNS_snprintf(publicPortString,  sizeof(publicPortString),  "iC%u", ReqPortNum);
    mDNS_snprintf(localIPAddrString, sizeof(localIPAddrString), "%u.%u.%u.%u",
                  m->AdvertisedV4.ip.v4.b[0], m->AdvertisedV4.ip.v4.b[1], m->AdvertisedV4.ip.v4.b[2], m->AdvertisedV4.ip.v4.b[3]);

    // build the message
    mDNSPlatformMemZero(propArgs, sizeof(propArgs));
    propArgs[0].name  = "NewRemoteHost";
    propArgs[0].type  = "string";
    propArgs[0].value = "";
    propArgs[1].name  = "NewExternalPort";
    propArgs[1].type  = "ui2";
    propArgs[1].value = externalPort;
    propArgs[2].name  = "NewProtocol";
    propArgs[2].type  = "string";
    propArgs[2].value = (n->Protocol == NATOp_MapUDP) ? "UDP" : "TCP";
    propArgs[3].name  = "NewInternalPort";
    propArgs[3].type  = "ui2";
    propArgs[3].value = internalPort;
    propArgs[4].name  = "NewInternalClient";
    propArgs[4].type  = "string";
    propArgs[4].value = localIPAddrString;
    propArgs[5].name  = "NewEnabled";
    propArgs[5].type  = "boolean";
    propArgs[5].value = "1";
    propArgs[6].name  = "NewPortMappingDescription";
    propArgs[6].type  = "string";
    propArgs[6].value = publicPortString;
    propArgs[7].name  = "NewLeaseDuration";
    propArgs[7].type  = "ui4";
    propArgs[7].value = "0";

    LogInfo("SendPortMapRequest: internal %u external %u", mDNSVal16(n->IntPort), ReqPortNum);
    return SendSOAPMsgControlAction(m, &n->tcpInfo, "AddPortMapping", 8, propArgs, LNTPortMapOp);
}

mDNSexport mStatus LNT_MapPort(mDNS *m, NATTraversalInfo *const n)
{
    LogInfo("LNT_MapPort");
    if (n->tcpInfo.sock) return(mStatus_NoError);   // If we already have a connection up don't make another request for the same thing
    n->tcpInfo.parentNATInfo = n;
    n->tcpInfo.retries       = 0;
    return SendPortMapRequest(m, n);
}

mDNSexport mStatus LNT_UnmapPort(mDNS *m, NATTraversalInfo *const n)
{
    char externalPort[10];
    Property propArgs[3];
    tcpLNTInfo  *info;
    tcpLNTInfo  **infoPtr = &m->tcpInfoUnmapList;
    mStatus err;

    // If no NAT gateway to talk to, no need to do all this work for nothing
    if (mDNSIPPortIsZero(m->UPnPSOAPPort) || !m->UPnPSOAPURL || !m->UPnPSOAPAddressString) return mStatus_NoError;

    mDNS_snprintf(externalPort, sizeof(externalPort), "%u", mDNSVal16(mDNSIPPortIsZero(n->RequestedPort) ? n->IntPort : n->RequestedPort));

    mDNSPlatformMemZero(propArgs, sizeof(propArgs));
    propArgs[0].name  = "NewRemoteHost";
    propArgs[0].type  = "string";
    propArgs[0].value = "";
    propArgs[1].name  = "NewExternalPort";
    propArgs[1].type  = "ui2";
    propArgs[1].value = externalPort;
    propArgs[2].name  = "NewProtocol";
    propArgs[2].type  = "string";
    propArgs[2].value = (n->Protocol == NATOp_MapUDP) ? "UDP" : "TCP";

    n->tcpInfo.parentNATInfo = n;

    // clean up previous port mapping requests and allocations
    if (n->tcpInfo.sock) LogInfo("LNT_UnmapPort: closing previous open connection");
    if (n->tcpInfo.sock   ) { mDNSPlatformTCPCloseConnection(n->tcpInfo.sock); n->tcpInfo.sock    = mDNSNULL; }
    if (n->tcpInfo.Request) { mDNSPlatformMemFree(n->tcpInfo.Request);         n->tcpInfo.Request = mDNSNULL; }
    if (n->tcpInfo.Reply  ) { mDNSPlatformMemFree(n->tcpInfo.Reply);           n->tcpInfo.Reply   = mDNSNULL; }

    // make a copy of the tcpInfo that we can clean up later (the one passed in will be destroyed by the client as soon as this returns)
    if ((info = (tcpLNTInfo *) mDNSPlatformMemAllocate(sizeof(*info))) == mDNSNULL)
    { LogInfo("LNT_UnmapPort: can't allocate tcpInfo"); return(mStatus_NoMemoryErr); }
    *info = n->tcpInfo;

    while (*infoPtr) infoPtr = &(*infoPtr)->next;   // find the end of the list
    *infoPtr = info;    // append

    err = SendSOAPMsgControlAction(m, info, "DeletePortMapping", 3, propArgs, LNTPortMapDeleteOp);
    if (err) DisposeInfoFromUnmapList(m, info);
    return err;
}

mDNSexport mStatus LNT_GetExternalAddress(mDNS *m)
{
    return SendSOAPMsgControlAction(m, &m->tcpAddrInfo, "GetExternalIPAddress", 0, mDNSNULL, LNTExternalAddrOp);
}

mDNSlocal mStatus GetDeviceDescription(mDNS *m, tcpLNTInfo *info)
{
    // Device description format -
    //  - device description URL
    //  - host/port
    static const char szSSDPMsgDescribeDeviceFMT[] =
        "GET %s HTTP/1.1\r\n"
        "Accept: text/xml, application/xml\r\n"
        "User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows NT/5.1)\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n";

    if (!mDNSIPPortIsZero(m->UPnPSOAPPort)) return mStatus_NoError; // already have the info we need

    if (m->UPnPRouterURL == mDNSNULL || m->UPnPRouterAddressString == mDNSNULL) { LogInfo("GetDeviceDescription: no router URL or address string!"); return (mStatus_Invalid); }

    // build message
    if      (info->Request != mDNSNULL) mDNSPlatformMemZero(info->Request, LNT_MAXBUFSIZE); // reuse previously allocated buffer
    else if ((info->Request = mDNSPlatformMemAllocate(LNT_MAXBUFSIZE)) == mDNSNULL) { LogInfo("can't allocate send buffer for discovery"); return (mStatus_NoMemoryErr); }
    info->requestLen = mDNS_snprintf((char*)info->Request, LNT_MAXBUFSIZE, szSSDPMsgDescribeDeviceFMT, m->UPnPRouterURL, m->UPnPRouterAddressString);
    LogInfo("Describe Device: [%s]", info->Request);
    return MakeTCPConnection(m, info, &m->Router, m->UPnPRouterPort, LNTDiscoveryOp);
}

// This function parses the response to our SSDP discovery message. Basically, we look to make sure this is a response
// referencing a service we care about (WANIPConnection or WANPPPConnection), then look for the "Location:" header and copy the addressing and
// URL info we need.
mDNSexport void LNT_ConfigureRouterInfo(mDNS *m, const mDNSInterfaceID InterfaceID, const mDNSu8 *const data, const mDNSu16 len)
{
    const mDNSu8 *ptr = data;
    const mDNSu8 *end = data + len;
    const mDNSu8 *stop;

    if (!mDNSIPPortIsZero(m->UPnPRouterPort)) return; // already have the info we need

    // The formatting of the HTTP header is not always the same when it comes to the placement of
    // the service and location strings, so we just look for each of them from the beginning for every response

    // figure out if this is a message from a service we care about
    while (ptr && ptr != end)
    {
        if ((*ptr & 0xDF) == 'W' && (strncasecmp((char*)ptr, "WANIPConnection:1", 17) == 0)) break;
        ptr++;
    }
    if (ptr == end)
    {
        ptr = data;
        while (ptr && ptr != end)
        {
            if ((*ptr & 0xDF) == 'W' && (strncasecmp((char*)ptr, "WANPPPConnection:1", 18) == 0)) break;
            ptr++;
        }
    }
    if (ptr == mDNSNULL || ptr == end) return;  // not a message we care about

    // find "Location:", starting from the beginning
    ptr = data;
    while (ptr && ptr != end)
    {
        if ((*ptr & 0xDF) == 'L' && (strncasecmp((char*)ptr, "Location:", 9) == 0)) break;          // find the first 'L'; is this Location? if not, keep looking
        ptr++;
    }
    if (ptr == mDNSNULL || ptr == end)
    {
        LogInfo("LNT_ConfigureRouterInfo: Location field not found");
        return; // not a message we care about
    }
    ptr += 9; //Skip over 'Location:'
    while (*ptr == ' ' && ptr < end) ptr++; // skip over spaces
    if (ptr >= end) return;

    // find the end of the line
    for (stop = ptr; stop != end; stop++) { if (*stop == '\r') { end = stop; break; } }

    // fill in default port
    m->UPnPRouterPort = mDNSOpaque16fromIntVal(80);

    // free string pointers and set to NULL
    if (m->UPnPRouterAddressString != mDNSNULL)
    {
        mDNSPlatformMemFree(m->UPnPRouterAddressString);
        m->UPnPRouterAddressString = mDNSNULL;
    }
    if (m->UPnPRouterURL != mDNSNULL)
    {
        mDNSPlatformMemFree(m->UPnPRouterURL);
        m->UPnPRouterURL = mDNSNULL;
    }

    // the Router URL should look something like "/dyndev/uuid:0013-108c-4b3f0000f3dc"
    if (ParseHttpUrl(ptr, end, &m->UPnPRouterAddressString, &m->UPnPRouterPort, &m->UPnPRouterURL) != mStatus_NoError)
    {
        LogInfo("LNT_ConfigureRouterInfo: Failed to parse URL");
        return;
    }

    m->UPnPInterfaceID = InterfaceID;

    if (m->UPnPRouterAddressString == mDNSNULL)
    {
        LogMsg("LNT_ConfigureRouterInfo: UPnPRouterAddressString is NULL");
    }
    else LogInfo("LNT_ConfigureRouterInfo: Router address string [%s]", m->UPnPRouterAddressString);

    if (m->UPnPRouterURL == mDNSNULL)
    {
        LogMsg("LNT_ConfigureRouterInfo: UPnPRouterURL is NULL");
    }
    else LogInfo("LNT_ConfigureRouterInfo: Router URL [%s]", m->UPnPRouterURL);

    LogInfo("LNT_ConfigureRouterInfo: Router port %d", mDNSVal16(m->UPnPRouterPort));
    LogInfo("LNT_ConfigureRouterInfo: Router interface %d", m->UPnPInterfaceID);

    // Don't need the SSDP socket anymore
    if (m->SSDPSocket) { debugf("LNT_ConfigureRouterInfo destroying SSDPSocket %p", &m->SSDPSocket); mDNSPlatformUDPClose(m->SSDPSocket); m->SSDPSocket = mDNSNULL; }

    // now send message to get the device description
    GetDeviceDescription(m, &m->tcpDeviceInfo);
}

mDNSexport void LNT_SendDiscoveryMsg(mDNS *m)
{
    static const char msg[] =
        "M-SEARCH * HTTP/1.1\r\n"
        "Host:239.255.255.250:1900\r\n"
        "ST:urn:schemas-upnp-org:service:WAN%sConnection:1\r\n"
        "Man:\"ssdp:discover\"\r\n"
        "MX:3\r\n\r\n";
    static const mDNSAddr multicastDest = { mDNSAddrType_IPv4, { { { 239, 255, 255, 250 } } } };

    mDNSu8 *const buf = (mDNSu8*)&m->omsg; //m->omsg is 8952 bytes, which is plenty
    unsigned int bufLen;

    if (m->SleepState != SleepState_Awake) return;
    if (!mDNSIPPortIsZero(m->UPnPRouterPort))
    {
        if (m->SSDPSocket) { debugf("LNT_SendDiscoveryMsg destroying SSDPSocket %p", &m->SSDPSocket); mDNSPlatformUDPClose(m->SSDPSocket); m->SSDPSocket = mDNSNULL; }
        if (mDNSIPPortIsZero(m->UPnPSOAPPort) && !m->tcpDeviceInfo.sock) GetDeviceDescription(m, &m->tcpDeviceInfo);
        return;
    }

    // Always query for WANIPConnection in the first SSDP packet
    if (m->retryIntervalGetAddr <= NATMAP_INIT_RETRY) m->SSDPWANPPPConnection = mDNSfalse;

    // Create message
    bufLen = mDNS_snprintf((char*)buf, sizeof(m->omsg), msg, m->SSDPWANPPPConnection ? "PPP" : "IP");

    debugf("LNT_SendDiscoveryMsg Router %.4a Current External Address %.4a", &m->Router.ip.v4, &m->ExtAddress);

    if (!mDNSIPv4AddressIsZero(m->Router.ip.v4))
    {
        if (!m->SSDPSocket) { m->SSDPSocket = mDNSPlatformUDPSocket(zeroIPPort); debugf("LNT_SendDiscoveryMsg created SSDPSocket %p", &m->SSDPSocket); }
        mDNSPlatformSendUDP(m, buf, buf + bufLen, 0, m->SSDPSocket, &m->Router,     SSDPPort, mDNSfalse);
        mDNSPlatformSendUDP(m, buf, buf + bufLen, 0, m->SSDPSocket, &multicastDest, SSDPPort, mDNSfalse);
    }

    m->SSDPWANPPPConnection = !m->SSDPWANPPPConnection;
}

mDNSexport void LNT_ClearState(mDNS *const m)
{
    if (m->tcpAddrInfo.sock)   { mDNSPlatformTCPCloseConnection(m->tcpAddrInfo.sock);   m->tcpAddrInfo.sock   = mDNSNULL; }
    if (m->tcpDeviceInfo.sock) { mDNSPlatformTCPCloseConnection(m->tcpDeviceInfo.sock); m->tcpDeviceInfo.sock = mDNSNULL; }
    m->UPnPSOAPPort = m->UPnPRouterPort = zeroIPPort;   // Reset UPnP ports
}

#endif /* _LEGACY_NAT_TRAVERSAL_ */
