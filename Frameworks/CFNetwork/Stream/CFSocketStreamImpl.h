/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
    CFSocketStreamImpl.h
    Copyright 1998-2003, Apple, Inc. All rights reserved.
    Responsibility: Jeremy Wyld

    Private header only for the set of files that implement CFSocketStream.
*/

#ifndef __CFSOCKETSTREAMIMPL__
#define __CFSOCKETSTREAMIMPL__

#include <CFNetwork/CFSocketStream.h>
#include "CFNetworkInternal.h"
#include <CFNetwork/CFHTTPStream.h>

#if defined(__MACH__)
#include <Security/SecureTransport.h>
#include <SystemConfiguration/SCNetworkReachability.h>
#endif

#if defined(__WIN32__)
#include <winsock2.h>
#include <ws2tcpip.h>	// for ipv6

// an alias for Win32 routines
#define ioctl(a, b, c)		ioctlsocket(a, b, c)

// Sockets and fds are not interchangeable on Win32, and have different error codes.
// These redefines assumes that in this file we only apply this error constant to socket ops.

#undef EAGAIN
#define EAGAIN WSAEWOULDBLOCK

#undef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED

#undef ENOTCONN
#define ENOTCONN WSAENOTCONN

#undef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED

#undef EBADF
#define EBADF WSAENOTSOCK

#undef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT

#endif

#if defined(__cplusplus)
extern "C" {
#endif


#if defined(__WIN32__)

typedef struct _SchannelState *SchannelState;

// Win32 doesn't have an equivalent, but we reuse this for porting ease
typedef enum {
    kSSLIdle = 1,
    kSSLHandshake,
    kSSLConnected,
    kSSLClosed,
    kSSLAborted
} SSLSessionState;

#endif

// The bits for a socket context's flags.  Note that these are the bit indices, not the bit masks themselves.
enum {
    SHARED	= 0,

    CREATED_WITH_SOCKET,

    OPEN_START,
    OPEN_COMPLETE,

    SHOULD_CLOSE,
    KILL_SOCKET_ON_CLOSE,

    SECURITY_CHECK_CERTIFICATE,

    VERIFIED_READ_EVENT,
    VERIFIED_WRITE_EVENT,

    WRITE_STREAM_OPENED,
    READ_STREAM_OPENED,

    USE_ADDR_CACHE,
    RECVD_READ,
    RECVD_WRITE,

    SELECT_READ,
    SELECT_WRITE,

    NO_REACHABILITY,

    ICHAT_SUBNET_SETTING = 31
};

typedef struct {
    CFMutableArrayRef	runloops;
    struct _CFStream	*stream;		// NOT retained; if it's retained, we introduce a retain loop!
} _CFSocketStreamRLSource;

typedef struct {
    UInt32			socks_flags;		// Flags used for performing SOCKS handshake.
    CFHostRef		host;				// SOCKS server
    UInt32			port;				// Port of the SOCKS server
    CFStringRef		user;				// user id for the SOCKS server
    CFStringRef		pass;				// password for the SOCKS server
    CFIndex			bytesInBuffer;		// Number of bytes in the buffer.
    UInt8*			buffer;				// Bytes read or waiting to be written.
} _SOCKSInfo;

typedef struct {
    CFHostRef			host;
    UInt32				port;
    CFDictionaryRef		settings;
    CFDataRef			request;
    CFIndex				left;
    CFHTTPMessageRef	response;
} _CONNECTInfo;

struct _CFSocketStreamContext;
typedef void (*handshakeFn)(struct _CFSocketStreamContext*);

typedef struct {
    CFSpinLock_t		lock;			// Used to lock access if two separate streams exist for the same socket.
    CFOptionFlags		flags;

    CFAllocatorRef		alloc;

    CFStreamError		error;			// Store the error code for the operation that failed

    CFTypeRef			lookup;			// async lookup; either a CFHost or a CFNetService
    CFIndex				attempt;		// current address index being attempted
    CFSocketRef			sock;			// underlying CFSocket

    CFArrayRef			cb;
#if defined(__MACH__)
    SCNetworkReachabilityRef reachability;
#endif
    union {
        UInt32			port;			// Port number if created with host/port
        int				sock;			// socket if created with a native socket
    } u;

    CFSocketSignature 	sig;			// Signature for a stream pair created with one

    CFMutableArrayRef	runloops;       // loop/mode pairs that are scheduled for
                                        // both read and write
    _CFSocketStreamRLSource		*readSource, *writeSource;

    handshakeFn			handshakes[4];	// Holds the functions to be performed as part of open.

    CFStringRef			peerName;		// if set, overrides peer name from the host we looked up
#if defined(__MACH__)
    SSLContextRef		security;
    UInt8*				sslBuffer;
    CFIndex				sslBufferCount;
#elif defined(__WIN32__)
    SchannelState       ssl;
#endif
    _SOCKSInfo*			socks_info;
    _CONNECTInfo*		connect_info;
} _CFSocketStreamContext;


// General routines used by the implementation files, implemented in CFSocketStream.c

extern CFIndex __fdRecv(int fd, UInt8* buffer, CFIndex bufferLength, CFStreamError* errorCode, Boolean *atEOF);
extern CFIndex __fdSend(int fd, const UInt8* buffer, CFIndex bufferLength, CFStreamError* errorCode);

extern char* __getServerName(_CFSocketStreamContext* ctxt, char *buffer, UInt32 *bufSize, CFAllocatorRef *allocator);

extern Boolean __AddHandshake_Unsafe(_CFSocketStreamContext* ctxt, handshakeFn fn);
extern void __RemoveHandshake_Unsafe(_CFSocketStreamContext* ctxt, handshakeFn fn);
extern void __WaitForHandshakeToComplete_Unsafe(_CFSocketStreamContext* ctxt);

#define __socketGetFD(ctxt)	((int)((__CFBitIsSet(ctxt->flags, CREATED_WITH_SOCKET)) ? ctxt->u.sock : (ctxt->sock ? CFSocketGetNative(ctxt->sock) : -1)))


// SSL routines used by CFSocketStream.c, implemented on both Mach and Win32

extern CFIndex sslRecv(_CFSocketStreamContext* ctxt, UInt8* buffer, CFIndex bufferLength, CFStreamError* errorCode, Boolean *atEOF);
extern CFIndex sslSend(_CFSocketStreamContext* ctxt, const UInt8* buffer, CFIndex bufferLength, CFStreamError* errorCode);
extern void sslClose(_CFSocketStreamContext* ctxt);
extern CFIndex sslBytesAvailableForRead(_CFSocketStreamContext* ctxt);

extern void performSSLHandshake(_CFSocketStreamContext* ctxt);
extern void performSSLSendHandshake(_CFSocketStreamContext* ctxt);

extern Boolean __SetSecuritySettings_Unsafe(_CFSocketStreamContext* ctxt, CFDictionaryRef settings);
extern CFStringRef __GetSSLProtocol(_CFSocketStreamContext* ctxt);
extern SSLSessionState __GetSSLSessionState(_CFSocketStreamContext* ctxt);

#if 0
#define SSL_LOG printf
#else
#define SSL_LOG while(0) printf
#endif

#if defined(__MACH__)

#define IS_SECURE(x)				((x)->security != NULL)
#define SSL_WOULD_BLOCK(error) ((error->domain == kCFStreamErrorDomainSSL) && (errSSLWouldBlock == error->error) )

#elif defined(__WIN32__)

#define IS_SECURE(ctxt)	((ctxt)->ssl != NULL)
// On Windows there is no SSL-specific way to represent this, it uses the normal EAGAIN
#define SSL_WOULD_BLOCK(error) (FALSE)

#endif  /* __WIN32__ */

#if defined(__cplusplus)
}
#endif

#endif	/* __CFSOCKETSTREAMIMPL__ */

