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
 *  CFSocketStream.c
 *  
 *
 *  Created by Jeremy Wyld on Mon Apr 26 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#if 0
#pragma mark Includes
#endif

#include "CFNetworkInternal.h"
#include "CFNetworkSchedule.h"
// For _CFNetworkUserAgentString()
#include "CFHTTPInternal.h" 

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <CoreFoundation/CFStreamPriv.h>
#include <CFNetwork/CFSocketStreamPriv.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <Security/Security.h>
#include <Security/SecureTransportPriv.h>

#if 0
#pragma mark -
#pragma mark Constants
#endif

#define kSocketEvents ((CFOptionFlags)(kCFSocketReadCallBack | kCFSocketConnectCallBack | kCFSocketWriteCallBack))
#define kReadWriteTimeoutInterval ((CFTimeInterval)75.0)
#define kRecvBufferSize		((CFIndex)(32768L));
#define kSecurityBufferSize	((CFIndex)(32768L));

#ifndef __MACH__
const int kCFStreamErrorDomainSOCKS = 5;	/* On Mach this lives in CF for historical reasons, even though it is declared in CFNetwork */
#endif


#if 0
#pragma mark *Constant Strings
#pragma mark **Stream Property Keys
#endif

/* Properties made available as API */
CONST_STRING_DECL(kCFStreamPropertySocketRemoteHost, "kCFStreamPropertySocketRemoteHost")
CONST_STRING_DECL(kCFStreamPropertySocketRemoteNetService, "kCFStreamPropertySocketRemoteNetService")
CONST_STRING_DECL(kCFStreamPropertyShouldCloseNativeSocket, "kCFStreamPropertyShouldCloseNativeSocket")

CONST_STRING_DECL(_kCFStreamPropertySocketPeerName, "_kCFStreamPropertySocketPeerName")

CONST_STRING_DECL(kCFStreamPropertySSLPeerCertificates, "kCFStreamPropertySSLPeerCertificates")
CONST_STRING_DECL(_kCFStreamPropertySSLClientCertificates, "_kCFStreamPropertySSLClientCertificates")
CONST_STRING_DECL(_kCFStreamPropertySSLClientCertificateState, "_kCFStreamPropertySSLClientCertificateState")
CONST_STRING_DECL(kCFStreamPropertySSLSettings, "kCFStreamPropertySSLSettings")
CONST_STRING_DECL(kCFStreamSSLAllowsAnyRoot, "kCFStreamSSLAllowsAnyRoot")
CONST_STRING_DECL(kCFStreamSSLAllowsExpiredCertificates, "kCFStreamSSLAllowsExpiredCertificates")
CONST_STRING_DECL(kCFStreamSSLAllowsExpiredRoots, "kCFStreamSSLAllowsExpiredRoots")
CONST_STRING_DECL(kCFStreamSSLCertificates, "kCFStreamSSLCertificates")
CONST_STRING_DECL(kCFStreamSSLIsServer, "kCFStreamSSLIsServer")
CONST_STRING_DECL(kCFStreamSSLLevel, "kCFStreamSSLLevel")
CONST_STRING_DECL(kCFStreamSSLPeerName, "kCFStreamSSLPeerName")
CONST_STRING_DECL(kCFStreamSSLValidatesCertificateChain, "kCFStreamSSLValidatesCertificateChain")
CONST_STRING_DECL(kCFStreamSocketSecurityLevelTLSv1SSLv3, "kCFStreamSocketSecurityLevelTLSv1SSLv3")

/* Properties made available as SPI */
CONST_STRING_DECL(kCFStreamPropertyUseAddressCache, "kCFStreamPropertyUseAddressCache")
CONST_STRING_DECL(_kCFStreamSocketIChatWantsSubNet, "_kCFStreamSocketIChatWantsSubNet")
CONST_STRING_DECL(_kCFStreamSocketCreatedCallBack, "_kCFStreamSocketCreatedCallBack")
CONST_STRING_DECL(kCFStreamPropertyProxyExceptionsList, "ExceptionsList")
CONST_STRING_DECL(kCFStreamPropertyProxyLocalBypass, "ExcludeSimpleHostnames");

/* CONNECT tunnel properties.  Still SPI. */
CONST_STRING_DECL(kCFStreamPropertyCONNECTProxy, "kCFStreamPropertyCONNECTProxy")
CONST_STRING_DECL(kCFStreamPropertyCONNECTProxyHost, "kCFStreamPropertyCONNECTProxyHost")
CONST_STRING_DECL(kCFStreamPropertyCONNECTProxyPort, "kCFStreamPropertyCONNECTProxyPort")
CONST_STRING_DECL(kCFStreamPropertyCONNECTVersion, "kCFStreamPropertyCONNECTVersion")
CONST_STRING_DECL(kCFStreamPropertyCONNECTAdditionalHeaders, "kCFStreamPropertyCONNECTAdditionalHeaders")
CONST_STRING_DECL(kCFStreamPropertyCONNECTResponse, "kCFStreamPropertyCONNECTResponse")
CONST_STRING_DECL(kCFStreamPropertyPreviousCONNECTResponse, "kCFStreamPropertyPreviousCONNECTResponse")

/* Properties used internally to CFSocketStream */
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamProxySettingSOCKSEnable			CFSTR("SOCKSEnable")
#define _kCFStreamPropertySocketRemotePort			CFSTR("_kCFStreamPropertySocketRemotePort")
#define _kCFStreamPropertySocketAddressAttempt		CFSTR("_kCFStreamPropertySocketAddressAttempt")
#define _kCFStreamPropertySocketFamilyTypeProtocol	CFSTR("_kCFStreamPropertySocketFamilyTypeProtocol")
#define _kCFStreamPropertyHostForOpen				CFSTR("_kCFStreamPropertyHostForOpen")
#define _kCFStreamPropertyNetworkReachability		CFSTR("_kCFStreamPropertyNetworkReachability")
#define _kCFStreamPropertyRecvBuffer				CFSTR("_kCFStreamPropertyRecvBuffer")
#define _kCFStreamPropertyRecvBufferCount			CFSTR("_kCFStreamPropertyRecvBufferCount")
#define _kCFStreamPropertyRecvBufferSize			CFSTR("_kCFStreamPropertyRecvBufferSize")
#define _kCFStreamPropertySecurityRecvBuffer		CFSTR("_kCFStreamPropertySecurityRecvBuffer")
#define _kCFStreamPropertySecurityRecvBufferSize	CFSTR("_kCFStreamPropertySecurityRecvBufferSize")
#define _kCFStreamPropertySecurityRecvBufferCount	CFSTR("_kCFStreamPropertySecurityRecvBufferCount")
#define _kCFStreamPropertyHandshakes				CFSTR("_kCFStreamPropertyHandshakes")
#define _kCFStreamPropertyCONNECTSendBuffer			CFSTR("_kCFStreamPropertyCONNECTSendBuffer")
#define _kCFStreamPropertySOCKSSendBuffer			CFSTR("_kCFStreamPropertySOCKSSendBuffer")
#define _kCFStreamPropertySOCKSRecvBuffer			CFSTR("_kCFStreamPropertySOCKSRecvBuffer")
#define _kCFStreamPropertyReadTimeout				CFSTR("_kCFStreamPropertyReadTimeout")
#define _kCFStreamPropertyWriteTimeout				CFSTR("_kCFStreamPropertyWriteTimeout")
#define _kCFStreamPropertyReadCancel				CFSTR("_kCFStreamPropertyReadCancel")
#define _kCFStreamPropertyWriteCancel				CFSTR("_kCFStreamPropertyWriteCancel")
#else
static CONST_STRING_DECL(_kCFStreamProxySettingSOCKSEnable, "SOCKSEnable")
static CONST_STRING_DECL(_kCFStreamPropertySocketRemotePort, "_kCFStreamPropertySocketRemotePort")
static CONST_STRING_DECL(_kCFStreamPropertySocketAddressAttempt, "_kCFStreamPropertySocketAddressAttempt")
static CONST_STRING_DECL(_kCFStreamPropertySocketFamilyTypeProtocol, "_kCFStreamPropertySocketFamilyTypeProtocol")
static CONST_STRING_DECL(_kCFStreamPropertyHostForOpen, "_kCFStreamPropertyHostForOpen")
static CONST_STRING_DECL(_kCFStreamPropertyNetworkReachability, "_kCFStreamPropertyNetworkReachability")
static CONST_STRING_DECL(_kCFStreamPropertyRecvBuffer, "_kCFStreamPropertyRecvBuffer")
static CONST_STRING_DECL(_kCFStreamPropertyRecvBufferCount, "_kCFStreamPropertyRecvBufferCount")
static CONST_STRING_DECL(_kCFStreamPropertyRecvBufferSize, "_kCFStreamPropertyRecvBufferSize")
static CONST_STRING_DECL(_kCFStreamPropertySecurityRecvBuffer, "_kCFStreamPropertySecurityRecvBuffer")
static CONST_STRING_DECL(_kCFStreamPropertySecurityRecvBufferSize, "_kCFStreamPropertySecurityRecvBufferSize")
static CONST_STRING_DECL(_kCFStreamPropertySecurityRecvBufferCount, "_kCFStreamPropertySecurityRecvBufferCount")
static CONST_STRING_DECL(_kCFStreamPropertyHandshakes, "_kCFStreamPropertyHandshakes")
static CONST_STRING_DECL(_kCFStreamPropertyCONNECTSendBuffer, "_kCFStreamPropertyCONNECTSendBuffer")
static CONST_STRING_DECL(_kCFStreamPropertySOCKSSendBuffer, "_kCFStreamPropertySOCKSSendBuffer")
static CONST_STRING_DECL(_kCFStreamPropertySOCKSRecvBuffer, "_kCFStreamPropertySOCKSRecvBuffer")
static CONST_STRING_DECL(_kCFStreamPropertyReadCancel, "_kCFStreamPropertyReadCancel")
static CONST_STRING_DECL(_kCFStreamPropertyWriteCancel, "_kCFStreamPropertyWriteCancel")
#endif	/* __CONSTANT_CFSTRINGS__ */

#ifdef __MACH__
extern const CFStringRef kCFStreamPropertyAutoErrorOnSystemChange;
CONST_STRING_DECL(kCFStreamPropertySocketSSLContext, "kCFStreamPropertySocketSSLContext")
CONST_STRING_DECL(_kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, "_kCFStreamPropertySocketSecurityAuthenticatesServerCertificate")
#else
/* On Mach these live in CF for historical reasons, even though they are declared in CFNetwork */
CONST_STRING_DECL(kCFStreamPropertySOCKSProxy, "kCFStreamPropertySOCKSProxy")
CONST_STRING_DECL(kCFStreamPropertySOCKSProxyHost, "SOCKSProxy")
CONST_STRING_DECL(kCFStreamPropertySOCKSProxyPort, "SOCKSPort")
CONST_STRING_DECL(kCFStreamPropertySOCKSVersion, "kCFStreamPropertySOCKSVersion")
CONST_STRING_DECL(kCFStreamSocketSOCKSVersion4, "kCFStreamSocketSOCKSVersion4")
CONST_STRING_DECL(kCFStreamSocketSOCKSVersion5, "kCFStreamSocketSOCKSVersion5")
CONST_STRING_DECL(kCFStreamPropertySOCKSUser, "kCFStreamPropertySOCKSUser")
CONST_STRING_DECL(kCFStreamPropertySOCKSPassword, "kCFStreamPropertySOCKSPassword")

CONST_STRING_DECL(kCFStreamPropertyAutoErrorOnSystemChange, "kCFStreamPropertyAutoErrorOnSystemChange")
#endif


#if 0
#pragma mark **Other Strings
#endif

/* Keys for _kCFStreamPropertySocketFamilyTypeProtocol dictionary */
#ifdef __CONSTANT_CFSTRINGS__
#define	_kCFStreamSocketFamily		CFSTR("_kCFStreamSocketFamily")
#define	_kCFStreamSocketType		CFSTR("_kCFStreamSocketType")
#define	_kCFStreamSocketProtocol	CFSTR("_kCFStreamSocketProtocol")
#else
static CONST_STRING_DECL(_kCFStreamSocketFamily, "_kCFStreamSocketFamily")
static CONST_STRING_DECL(_kCFStreamSocketType, "_kCFStreamSocketType")
static CONST_STRING_DECL(_kCFStreamSocketProtocol, "_kCFStreamSocketProtocol")
#endif	/* __CONSTANT_CFSTRINGS__ */

/*
** Private modes for run loop polling.  These need to each be unique
** because one half should not affect the others.  E.g. one half
** polling in write should not directly conflict with the other
** half scheduled for read.
*/
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamSocketOpenCompletedPrivateMode	CFSTR("_kCFStreamSocketOpenCompletedPrivateMode")
#define _kCFStreamSocketReadPrivateMode				CFSTR("_kCFStreamSocketReadPrivateMode")
#define _kCFStreamSocketCanReadPrivateMode			CFSTR("_kCFStreamSocketCanReadPrivateMode")
#define _kCFStreamSocketWritePrivateMode			CFSTR("_kCFStreamSocketWritePrivateMode")
#define _kCFStreamSocketCanWritePrivateMode			CFSTR("_kCFStreamSocketCanWritePrivateMode")
#define _kCFStreamSocketSecurityClosePrivateMode	CFSTR("_kCFStreamSocketSecurityClosePrivateMode")
#define _kCFStreamSocketBogusPrivateMode			CFSTR("_kCFStreamSocketBogusPrivateMode")
#define _kCFStreamPropertyBogusRunLoop				CFSTR("_kCFStreamPropertyBogusRunLoop")
#else
static CONST_STRING_DECL(_kCFStreamSocketOpenCompletedPrivateMode, "_kCFStreamSocketOpenCompletedPrivateMode")
static CONST_STRING_DECL(_kCFStreamSocketReadPrivateMode, "_kCFStreamSocketReadPrivateMode")
static CONST_STRING_DECL(_kCFStreamSocketCanReadPrivateMode, "_kCFStreamSocketCanReadPrivateMode")
static CONST_STRING_DECL(_kCFStreamSocketWritePrivateMode, "_kCFStreamSocketWritePrivateMode")
static CONST_STRING_DECL(_kCFStreamSocketCanWritePrivateMode, "_kCFStreamSocketCanWritePrivateMode")
static CONST_STRING_DECL(_kCFStreamSocketSecurityClosePrivateMode, "_kCFStreamSocketSecurityClosePrivateMode")

static CONST_STRING_DECL(_kCFStreamSocketBogusPrivateMode, "_kCFStreamSocketBogusPrivateMode")
static CONST_STRING_DECL(_kCFStreamPropertyBogusRunLoop, "_kCFStreamPropertyBogusRunLoop")
#endif	/* __CONSTANT_CFSTRINGS__ */

/* Special strings and formats for performing CONNECT. */
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamCONNECTURLFormat	CFSTR("%@:%d")
#define _kCFStreamCONNECTMethod		CFSTR("CONNECT")
#define _kCFStreamUserAgentHeader	CFSTR("User-Agent")
#define _kCFStreamHostHeader		CFSTR("Host")
#else
static CONST_STRING_DECL(_kCFStreamCONNECTURLFormat, "%@:%d")
static CONST_STRING_DECL(_kCFStreamCONNECTMethod, "CONNECT")
static CONST_STRING_DECL(_kCFStreamUserAgentHeader, "User-Agent")
static CONST_STRING_DECL(_kCFStreamHostHeader, "Host")
#endif	/* __CONSTANT_CFSTRINGS__ */

/* AutoVPN strings */
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamAutoHostName					CFSTR("OnDemandHostName")
#define _kCFStreamPropertyAutoConnectPriority	CFSTR("OnDemandPriority")
#define _kCFStreamAutoVPNPriorityDefault		CFSTR("Default")
#else
static CONST_STRING_DECL(_kCFStreamAutoHostName, "OnDemandHostName")					/* **FIXME** Remove after PPPControllerPriv.h comes back */
static CONST_STRING_DECL(_kCFStreamPropertyAutoConnectPriority, "OnDemandPriority")	/* **FIXME** Ditto. */
static CONST_STRING_DECL(_kCFStreamAutoVPNPriorityDefault, "Default")
#endif	/* __CONSTANT_CFSTRINGS__ */

/* String used for CopyDescription function */
#ifdef __CONSTANT_CFSTRINGS__
#define kCFSocketStreamDescriptionFormat	CFSTR("<SocketStream %p>{flags = 0x%08x, read = %p, write = %p, socket = %@, properties = %p }")
#else
static CONST_STRING_DECL(kCFSocketStreamDescriptionFormat, "<SocketStream %p>{flags = 0x%08x, read = %p, write = %p, socket = %@, properties = %p }");
#endif	/* __CONSTANT_CFSTRINGS__ */


#if 0
#pragma mark -
#pragma mark Enum Values
#endif

enum {
    /* _CFSocketStreamContext flags */
	kFlagBitOpenStarted = 0,
	
	/* NOTE that the following three need to be kept in order */
    kFlagBitOpenComplete,		/* CFSocket is open and connected */
	kFlagBitCanRead,
	kFlagBitCanWrite,
	
	/* NOTE that the following three need to be kept in order (and ordered relative to the previous three) */
	kFlagBitPollOpen,			/* If the stream is event based and this bit is not set, OpenCompleted returns "no" immediately. */
	kFlagBitPollRead,			/* If the stream is event based and this bit is not set, CanRead returns "no" immediately. */
	kFlagBitPollWrite,			/* If the stream is event based and this bit is not set, CanWrite returns "no" immediately. */
	
	kFlagBitShared,				/* Indicates the stream structure is shared (read and write) */
	kFlagBitCreatedNative,		/* Stream(s) were created from a native socket handle. */
	kFlagBitReadStreamOpened,   /* Client has called open on the read stream */
	kFlagBitWriteStreamOpened,  /* Client has called open on the write stream */
	kFlagBitUseSSL,				/* Used for quickly determining SSL code paths. */
	kFlagBitClosed,				/* Signals that a close has been received on the buffered stream */
	kFlagBitTriedVPN,			/* Marked if an attempt at AutoVPN has been made */
	kFlagBitHasHandshakes,		/* Performance check for handshakes. */
	kFlagBitIsBuffered,			/* Performance check for using buffered reads. */
	kFlagBitRecvdRead,			/* On buffered streams, indicates that a read event has been received but buffer was full. */
	
	kFlagBitReadHasCancel,		/* Performance check for detecting run loop source for canceling synchronous read. */
	kFlagBitWriteHasCancel,		/* Performance check for detecting run loop source for canceling synchronous write. */
	
	/*
	** These flag bits are used to count the number of runs through the run loop short circuit
	** code at the end of read and write.  CFSocketStream is willing to run kMaximumNumberLoopAttempts
	** times through the run loop without forcing CFSocket to signal.  If kMaximumNumberLoopAttempts
	** attempts have not occurred, CFSocketStream will short circuit by selecting on the socket and
	** automatically marking the stream as readable or writable.
	*/
	kFlagBitMinLoops,
	kFlagBitMaxLoops = kFlagBitMinLoops + 3,
	kMaximumNumberLoopAttempts = (1 << (kFlagBitMaxLoops - kFlagBitMinLoops)),
	
	kSelectModeRead = 1,
	kSelectModeWrite = 2,
	kSelectModeExcept = 4
};


#if 0
#pragma mark -
#pragma mark Type Declarations
#pragma mark *CFStream Context
#endif

typedef struct {
	
	CFSpinLock_t				_lock;				/* Protection for read-half versus write-half */
	
	UInt32						_flags;
	CFStreamError				_error;
	
	CFReadStreamRef				_clientReadStream;
	CFWriteStreamRef			_clientWriteStream;
	
	CFSocketRef					_socket;			/* Actual underlying CFSocket */
	
    CFMutableArrayRef			_readloops;
    CFMutableArrayRef			_writeloops;
    CFMutableArrayRef			_sharedloops;
	
	CFMutableArrayRef			_schedulables;		/* Items to be scheduled (i.e. socket, reachability, host, etc.) */
		
	CFMutableDictionaryRef		_properties;		/* Host and port and reachability should be here too. */
	
} _CFSocketStreamContext;

#if 0
#pragma mark *Other Types
#endif

typedef void (*_CFSocketStreamSocketCreatedCallBack)(CFSocketNativeHandle s, void* info);
typedef void (*_CFSocketStreamPerformHandshakeCallBack)(_CFSocketStreamContext* ctxt);


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#pragma mark *Stream Callbacks
#endif

static void _SocketStreamFinalize(CFTypeRef stream, _CFSocketStreamContext* ctxt);
static CFStringRef _SocketStreamCopyDescription(CFTypeRef stream, _CFSocketStreamContext* ctxt);
static Boolean _SocketStreamOpen(CFTypeRef stream, CFStreamError* error, Boolean* openComplete, _CFSocketStreamContext* ctxt);
static Boolean _SocketStreamOpenCompleted(CFTypeRef stream, CFStreamError* error, _CFSocketStreamContext* ctxt);
static CFIndex _SocketStreamRead(CFReadStreamRef stream, UInt8* buffer, CFIndex bufferLength, CFStreamError* error, Boolean* atEOF, _CFSocketStreamContext* ctxt);
static Boolean _SocketStreamCanRead(CFReadStreamRef stream, _CFSocketStreamContext* ctxt);
static CFIndex _SocketStreamWrite(CFWriteStreamRef stream, const UInt8* buffer, CFIndex bufferLength, CFStreamError* error, _CFSocketStreamContext* ctxt);
static Boolean _SocketStreamCanWrite(CFWriteStreamRef stream, _CFSocketStreamContext* ctxt);
static void _SocketStreamClose(CFTypeRef stream, _CFSocketStreamContext* ctxt);
static CFTypeRef _SocketStreamCopyProperty(CFTypeRef stream, CFStringRef propertyName, _CFSocketStreamContext* ctxt);
static Boolean _SocketStreamSetProperty(CFTypeRef stream, CFStringRef propertyName, CFTypeRef propertyValue, _CFSocketStreamContext* ctxt);
static void _SocketStreamSchedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFSocketStreamContext* ctxt);
static void _SocketStreamUnschedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFSocketStreamContext* ctxt);

#if 0
#pragma mark *Utility Functions
#endif

static void _SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, _CFSocketStreamContext*info);
static void _HostCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError* error, _CFSocketStreamContext* info);
static void _NetServiceCallBack(CFNetServiceRef theService, CFStreamError* error, _CFSocketStreamContext* info);
static void _SocksHostCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError* error, _CFSocketStreamContext* info);
static void _ReachabilityCallBack(SCNetworkReachabilityRef target, const SCNetworkConnectionFlags flags, _CFSocketStreamContext* ctxt);
static void _NetworkConnectionCallBack(SCNetworkConnectionRef conn, SCNetworkConnectionStatus status, _CFSocketStreamContext* ctxt);

static Boolean _SchedulablesAdd(CFMutableArrayRef schedulables, CFTypeRef addition);
static Boolean _SchedulablesRemove(CFMutableArrayRef schedulables, CFTypeRef removal);
static void _SchedulablesScheduleApplierFunction(CFTypeRef obj, CFTypeRef loopAndMode[]);
static void _SchedulablesUnscheduleApplierFunction(CFTypeRef obj, CFTypeRef loopAndMode[]);
static void _SchedulablesUnscheduleFromAllApplierFunction(CFTypeRef obj, CFArrayRef schedules);
static void _SchedulablesInvalidateApplierFunction(CFTypeRef obj, void* context);
static void _SocketStreamSchedule_NoLock(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFSocketStreamContext* ctxt);
static void _SocketStreamUnschedule_NoLock(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFSocketStreamContext* ctxt);

static CFNumberRef _CFNumberCopyPortForOpen(CFDictionaryRef properties);
static CFDataRef _CFDataCopyAddressByInjectingPort(CFDataRef address, CFNumberRef port);
static Boolean _ScheduleAndStartLookup(CFTypeRef lookup, CFArrayRef* schedules, CFStreamError* error, const void* cb, void* info);

static CFIndex _CFSocketRecv(CFSocketRef s, UInt8* buffer, CFIndex length, CFStreamError* error);
static CFIndex _CFSocketSend(CFSocketRef s, const UInt8* buffer, CFIndex length, CFStreamError* error);
static Boolean _CFSocketCan(CFSocketRef s, int mode);

static _CFSocketStreamContext* _SocketStreamCreateContext(CFAllocatorRef alloc);
static void _SocketStreamDestroyContext_NoLock(CFAllocatorRef alloc, _CFSocketStreamContext* ctxt);

static Boolean _SocketStreamStartLookupForOpen_NoLock(_CFSocketStreamContext* ctxt);
static Boolean _SocketStreamCreateSocket_NoLock(_CFSocketStreamContext* ctxt, CFDataRef address);
static Boolean _SocketStreamConnect_NoLock(_CFSocketStreamContext* ctxt, CFDataRef address);
static Boolean _SocketStreamAttemptNextConnection_NoLock(_CFSocketStreamContext* ctxt);

static Boolean _SocketStreamCan(_CFSocketStreamContext* ctxt, CFTypeRef stream, int test, CFStringRef mode, CFStreamError* error);

static void _SocketStreamAddReachability_NoLock(_CFSocketStreamContext* ctxt);
static void _SocketStreamRemoveReachability_NoLock(_CFSocketStreamContext* ctxt);

static CFComparisonResult _OrderHandshakes(_CFSocketStreamPerformHandshakeCallBack fn1, _CFSocketStreamPerformHandshakeCallBack fn2, void* context);
static Boolean _SocketStreamAddHandshake_NoLock(_CFSocketStreamContext* ctxt, _CFSocketStreamPerformHandshakeCallBack fn);
static void _SocketStreamRemoveHandshake_NoLock(_CFSocketStreamContext* ctxt, _CFSocketStreamPerformHandshakeCallBack fn);

static void _SocketStreamAttemptAutoVPN_NoLock(_CFSocketStreamContext* ctxt, CFStringRef name);

static CFIndex _SocketStreamBufferedRead_NoLock(_CFSocketStreamContext* ctxt, UInt8* buffer, CFIndex length);
static void _SocketStreamBufferedSocketRead_NoLock(_CFSocketStreamContext* ctxt);

static void _SocketStreamPerformCancel(void* info);

CF_INLINE SInt32 _LastError(CFStreamError* error) {
	
	error->domain = _kCFStreamErrorDomainNativeSockets;
	
#if defined(__WIN32__)
	error->error = WSAGetLastError();
	if (!error->error) {
		error->error = errno;
		error->domain = kCFStreamErrorDomainPOSIX;
	}
#else
	error->error = errno;
#endif	/* __WIN32__ */
	
	return error->error;
}


#if 0
#pragma mark *SOCKS Support
#endif

static void _PerformSOCKSv5Handshake_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformSOCKSv5PostambleHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformSOCKSv5UserPassHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformSOCKSv4Handshake_NoLock(_CFSocketStreamContext* ctxt);
static Boolean _SOCKSSetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings);

static void _SocketStreamSOCKSHandleLookup_NoLock(_CFSocketStreamContext* ctxt, CFHostRef lookup);

CF_INLINE CFStringRef _GetSOCKSVersion(CFDictionaryRef settings) {
	
	CFStringRef result = (CFStringRef)CFDictionaryGetValue(settings, kCFStreamPropertySOCKSVersion);
	if (!result)
		result = kCFStreamSocketSOCKSVersion5;
	
	return result;
}


#if 0
#pragma mark *CONNECT Support
#endif

static void _CreateNameAndPortForCONNECTProxy(CFDictionaryRef properties, CFStringRef* name, CFNumberRef* port, CFStreamError* error);
static void _PerformCONNECTHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformCONNECTHaltHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _CONNECTHeaderApplier(CFStringRef key, CFStringRef value, CFHTTPMessageRef request);
static Boolean _CONNECTSetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings);


#if 0
#pragma mark *SSL Support
#endif

static OSStatus _SecurityReadFunc_NoLock(_CFSocketStreamContext* ctxt, void* data, UInt32* dataLength);
static OSStatus _SecurityWriteFunc_NoLock(_CFSocketStreamContext* ctxt, const void* data, UInt32* dataLength);
static CFIndex _SocketStreamSecuritySend_NoLock(_CFSocketStreamContext* ctxt, const UInt8* buffer, CFIndex length);
static void _SocketStreamSecurityBufferedRead_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformSecurityHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _PerformSecuritySendHandshake_NoLock(_CFSocketStreamContext* ctxt);
static void _SocketStreamSecurityClose_NoLock(_CFSocketStreamContext* ctxt);
static Boolean _SocketStreamSecuritySetContext_NoLock(_CFSocketStreamContext *ctxt, CFDataRef value);
static Boolean _SocketStreamSecuritySetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings);
static Boolean _SocketStreamSecuritySetAuthenticatesServerCertificates_NoLock(_CFSocketStreamContext* ctxt, CFBooleanRef authenticates);
static CFStringRef _SecurityGetProtocol(SSLContextRef security);
static SSLSessionState _SocketStreamSecurityGetSessionState_NoLock(_CFSocketStreamContext* ctxt);


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern void _CFStreamCreatePairWithCFSocketSignaturePieces(CFAllocatorRef alloc, SInt32 protocolFamily, SInt32 socketType,
														   SInt32 protocol, CFDataRef address, CFReadStreamRef* readStream,
														   CFWriteStreamRef* writeStream);
extern void _CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
									  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream);
extern CFDataRef _CFHTTPMessageCopySerializedHeaders(CFHTTPMessageRef msg, Boolean forProxy);


#if 0
#pragma mark -
#pragma mark Callback Structs
#pragma mark *CFReadStreamCallBacks
#endif

static const CFReadStreamCallBacks
kSocketReadStreamCallBacks = {
    1,										/* version */
    NULL,									/* create */
    (void (*)(CFReadStreamRef, void*))_SocketStreamFinalize,
    (CFStringRef (*)(CFReadStreamRef, void*))_SocketStreamCopyDescription,
    (Boolean (*)(CFReadStreamRef, CFStreamError*, Boolean*, void*))_SocketStreamOpen,
    (Boolean (*)(CFReadStreamRef, CFStreamError*, void*))_SocketStreamOpenCompleted,
    (CFIndex (*)(CFReadStreamRef, UInt8*, CFIndex, CFStreamError*, Boolean*, void*))_SocketStreamRead,
    NULL,									/* getbuffer */
    (Boolean (*)(CFReadStreamRef, void*))_SocketStreamCanRead,
    (void (*)(CFReadStreamRef, void*))_SocketStreamClose,
    (CFTypeRef (*)(CFReadStreamRef, CFStringRef, void*))_SocketStreamCopyProperty,
    (Boolean (*)(CFReadStreamRef, CFStringRef, CFTypeRef, void*))_SocketStreamSetProperty,
    NULL,									/* requestEvents */
    (void (*)(CFReadStreamRef, CFRunLoopRef, CFStringRef, void*))_SocketStreamSchedule,
    (void (*)(CFReadStreamRef, CFRunLoopRef, CFStringRef, void*))_SocketStreamUnschedule
};

#if 0
#pragma mark *CFWriteStreamCallBacks
#endif

static const CFWriteStreamCallBacks
kSocketWriteStreamCallBacks = {
    1,										/* version */
    NULL,									/* create */
    (void (*)(CFWriteStreamRef, void*))_SocketStreamFinalize,
    (CFStringRef (*)(CFWriteStreamRef, void*))_SocketStreamCopyDescription,
    (Boolean (*)(CFWriteStreamRef, CFStreamError*, Boolean*, void*))_SocketStreamOpen,
    (Boolean (*)(CFWriteStreamRef, CFStreamError*, void*))_SocketStreamOpenCompleted,
    (CFIndex (*)(CFWriteStreamRef, const UInt8*, CFIndex, CFStreamError*, void*))_SocketStreamWrite,
    (Boolean (*)(CFWriteStreamRef, void*))_SocketStreamCanWrite,
    (void (*)(CFWriteStreamRef, void*))_SocketStreamClose,
    (CFTypeRef (*)(CFWriteStreamRef, CFStringRef, void*))_SocketStreamCopyProperty,
    (Boolean (*)(CFWriteStreamRef, CFStringRef, CFTypeRef, void*))_SocketStreamSetProperty,
    NULL,									/* requestEvents */
    (void (*)(CFWriteStreamRef, CFRunLoopRef, CFStringRef, void*))_SocketStreamSchedule,
    (void (*)(CFWriteStreamRef, CFRunLoopRef, CFStringRef, void*))_SocketStreamUnschedule
};


#if 0
#pragma mark -
#pragma mark CFStream Callback Functions
#endif


/* static */ void
_SocketStreamFinalize(CFTypeRef stream, _CFSocketStreamContext* ctxt) {

	/* Make sure the stream is shutdown */
	_SocketStreamClose(stream, ctxt);
	
	/* Lock down the context so it doesn't get touched again */
	__CFSpinLock(&ctxt->_lock);
	
	/*
	** If the other half is still using the struct, simply
	** mark one half as gone.
	*/
	if (__CFBitIsSet(ctxt->_flags, kFlagBitShared)) {
		
		/* No longer shared by two halves */
		__CFBitClear(ctxt->_flags, kFlagBitShared);
		
		/* Unlock and proceed */
		__CFSpinUnlock(&ctxt->_lock);
	}
	
	else {
		
		/* Destroy the context now */
		_SocketStreamDestroyContext_NoLock(CFGetAllocator(stream), ctxt);
	}
}


/* static */ CFStringRef
_SocketStreamCopyDescription(CFTypeRef stream, _CFSocketStreamContext* ctxt) {
	
	return CFStringCreateWithFormat(CFGetAllocator(stream),
									NULL,
									kCFSocketStreamDescriptionFormat,
									stream,
									ctxt->_flags,
									ctxt->_clientReadStream,
									ctxt->_clientWriteStream,
									ctxt->_socket,
									ctxt->_properties);
}


/* static */ Boolean
_SocketStreamOpen(CFTypeRef stream, CFStreamError* error, Boolean* openComplete,
				  _CFSocketStreamContext* ctxt)
{
	Boolean result = TRUE;
	CFRunLoopRef rl = NULL;
	
	/* Assume success and no error. */
	memset(error, 0, sizeof(error[0]));
	
	/* Assume that open is not complete; why else would it be here? */
	*openComplete = FALSE;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);

	/* Mark the stream having been called for open. */
	__CFBitSet(ctxt->_flags, (stream == ctxt->_clientReadStream) ? kFlagBitReadStreamOpened : kFlagBitWriteStreamOpened);
	
	/* Workaround the fact that some objects don't signal events when not scheduled permanently. */
	rl = (CFRunLoopRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyBogusRunLoop);
	if (!rl) {
		rl = CFRunLoopGetCurrent();
		CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyBogusRunLoop, rl);
	}
	
	/* If the open has finished on the other half, mark as such. */
	if (__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete)) {
		
		/* Copy any error that may have occurred. */
		memmove(error, &ctxt->_error, sizeof(error[0]));
		
		/* Open is done */
		*openComplete = TRUE;
	}
	
	else if (!__CFBitIsSet(ctxt->_flags, kFlagBitOpenStarted)) {
		
		/* Mark as started */
		__CFBitSet(ctxt->_flags, kFlagBitOpenStarted);
		
		/* If there is a carryover error, mark as complete. */
		if (ctxt->_error.error) {
			__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);
			__CFBitClear(ctxt->_flags, kFlagBitPollOpen);
		}
		
		/* If a completed host has already been set, start using it. */
		else if (CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHostForOpen)) {
			_SocketStreamAttemptNextConnection_NoLock(ctxt);
		}

		else if (!_SocketStreamStartLookupForOpen_NoLock(ctxt)) {
			
			/*
			** If no lookup started and no error, everything must be
			** ready for a connect attempt.
			*/
			if (!ctxt->_error.error)
				_SocketStreamAttemptNextConnection_NoLock(ctxt);
		}
		
		/* Did connect actually progress all the way through? */
		if (__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete)) {
			
			/* Remove the "started" flag */
			__CFBitClear(ctxt->_flags, kFlagBitOpenStarted);
			
			/* Mark as complete */
			*openComplete = TRUE;
		}
		
		/* Copy the error if one occurred. */
		if (ctxt->_error.error)
			memmove(error, &ctxt->_error, sizeof(error[0]));
	}
	
	/* Take care of simple things if there was an error. */
	if (error->error) {
		
		/* Mark the open as being complete */
		__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);

		__CFBitClear(ctxt->_flags, kFlagBitPollOpen);

		/* It's complete now that there's a failure. */
		*openComplete = TRUE;
		
		/* Open failed. */
		result = FALSE;
	}
	
	/* Workaround the fact that some objects don't signal events when not scheduled permanently. */
	if (result && rl)
		_SocketStreamSchedule_NoLock(stream, rl, _kCFStreamSocketBogusPrivateMode, ctxt);
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	return result;
}


/* static */ Boolean
_SocketStreamOpenCompleted(CFTypeRef stream, CFStreamError* error, _CFSocketStreamContext* ctxt) {

	/* Find out if open (polling if necessary). */
	return _SocketStreamCan(ctxt, stream, kFlagBitOpenComplete, _kCFStreamSocketOpenCompletedPrivateMode, error);
}


/* static */ CFIndex
_SocketStreamRead(CFReadStreamRef stream, UInt8* buffer, CFIndex bufferLength,
				  CFStreamError* error, Boolean* atEOF, _CFSocketStreamContext* ctxt)
{
	CFIndex result = 0;
	CFStreamEventType event = kCFStreamEventNone;

	/* Set as no error to start. */
	memset(error, 0, sizeof(error[0]));

	/* Not at end yet. */
	*atEOF = FALSE;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	while (1) {
		
		/* Wasn't time to read, so run in private mode for timeout or ability to read. */
		if (!ctxt->_error.error && !__CFBitIsSet(ctxt->_flags, kFlagBitCanRead)) {
			
			CFRunLoopRef rl = CFRunLoopGetCurrent();
			CFRunLoopSourceContext src = {0, rl, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _SocketStreamPerformCancel};
			CFRunLoopSourceRef cancel = CFRunLoopSourceCreate(CFGetAllocator(stream), 0, &src);
			
			if (cancel) {
				CFAbsoluteTime later;
				CFTimeInterval interval;
				CFNumberRef value = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyReadTimeout);
				
				CFTypeRef loopAndMode[2] = {rl, _kCFStreamSocketReadPrivateMode};
				
				CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyReadCancel, cancel);
				CFRelease(cancel);
				
				if (!value || !CFNumberGetValue(value, kCFNumberDoubleType, &interval))
					interval = kReadWriteTimeoutInterval;
				
				later = (interval == 0.0) ? DBL_MAX : CFAbsoluteTimeGetCurrent() + interval;
				
				/* Add the current loop and the private mode to the list */
				_SchedulesAddRunLoopAndMode(ctxt->_readloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
				
				/* Make sure to schedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesScheduleApplierFunction,
									 loopAndMode);
				
				CFRunLoopAddSource((CFRunLoopRef)loopAndMode[0], cancel, (CFStringRef)loopAndMode[1]);
				
				__CFBitSet(ctxt->_flags, kFlagBitReadHasCancel);
				
				do {
					/* Unlock the context to allow things to fire */
					__CFSpinUnlock(&ctxt->_lock);
					
					/* Run the run loop for a poll (0.0) */
					CFRunLoopRunInMode(_kCFStreamSocketReadPrivateMode, interval, TRUE);
					
					/* Lock the context back up. */
					__CFSpinLock(&ctxt->_lock);
					
				} while (!ctxt->_error.error &&
						 !__CFBitIsSet(ctxt->_flags, kFlagBitCanRead) &&
						 (0 < (interval = (later - CFAbsoluteTimeGetCurrent()))));
				
				__CFBitClear(ctxt->_flags, kFlagBitReadHasCancel);
				
				CFRunLoopRemoveSource((CFRunLoopRef)loopAndMode[0], cancel, (CFStringRef)loopAndMode[1]);
				
				/* Make sure to unschedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesUnscheduleApplierFunction,
									 loopAndMode);
				
				/* Remove this loop and private mode from the list. */
				_SchedulesRemoveRunLoopAndMode(ctxt->_readloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
				
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyReadCancel);
				
				/* If fell out, still not time, and no error, set to timed out. */
				if (!__CFBitIsSet(ctxt->_flags, kFlagBitCanRead) && !ctxt->_error.error) {				
					ctxt->_error.error = ETIMEDOUT;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				}
			}
			else {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			}
		}
		
		/* Using buffered reads? */
		if (__CFBitIsSet(ctxt->_flags, kFlagBitIsBuffered)) {
			result = _SocketStreamBufferedRead_NoLock(ctxt, buffer, bufferLength);
		}
		
		/* If there's no error, try to read now. */
		else if (!ctxt->_error.error) {
			result = _CFSocketRecv(ctxt->_socket, buffer, bufferLength, &ctxt->_error);
		}
		
		/* Did a read, so the event is no longer good. */
		__CFBitClear(ctxt->_flags, kFlagBitCanRead);
		
		/* Got a "would block" error, so clear it and wait for time to read. */
		if ((ctxt->_error.error == EAGAIN) && (ctxt->_error.domain == _kCFStreamErrorDomainNativeSockets)) {
			memset(&ctxt->_error, 0, sizeof(ctxt->_error));
			continue;
		}
					
		break;
	}
	
	/*
	** 3863115 The following processing may seem backwards, but it is intentional.
	** The idea is to allow processing of buffered bytes even if there is an error
	** sitting on the stream's context.
	*/
	
	/* Make sure to set things up correctly as a result of success. */
	if (result > 0) {

		/* If handshakes are in play, don't even attempt further checks. */
		if (!__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes)) {

			/* Right now only SSL is using the buffered reads. */
			if (__CFBitIsSet(ctxt->_flags, kFlagBitIsBuffered)) {
				
				/* Attempt to get the count of buffered bytes. */
				CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
										
				/* Are there bytes or has SSL closed the connection? */
				if (__CFBitIsSet(ctxt->_flags, kFlagBitClosed) || (c && *((CFIndex*)CFDataGetBytePtr(c)))) {
					__CFBitSet(ctxt->_flags, kFlagBitCanRead);
					__CFBitClear(ctxt->_flags, kFlagBitPollRead);
					event = kCFStreamEventHasBytesAvailable;
				}			
			}
			
			/* Can still read?  If not, re-enable. */
			else if (!_CFSocketCan(ctxt->_socket, kSelectModeRead))
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
			
			/* Still can read, so signal the "has bytes" now. */
			else {
				event = kCFStreamEventHasBytesAvailable;
				__CFBitSet(ctxt->_flags, kFlagBitCanRead);
				__CFBitClear(ctxt->_flags, kFlagBitPollRead);
			}
		}
		
		else {
			CFMutableArrayRef handshakes = (CFMutableArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
			
			if (handshakes && 
				CFArrayGetCount(handshakes) &&
				(_PerformCONNECTHaltHandshake_NoLock == CFArrayGetValueAtIndex(handshakes, 0)))
			{
				/* Can still read?  If not, re-enable. */
				if (!_CFSocketCan(ctxt->_socket, kSelectModeRead))
					CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
				
				/* Still can read, so signal the "has bytes" now. */
				else {
					event = kCFStreamEventHasBytesAvailable;
					__CFBitSet(ctxt->_flags, kFlagBitCanRead);
					__CFBitClear(ctxt->_flags, kFlagBitPollRead);
				}
			}
		}
	}
	
	/* If there was an error, make sure to propagate and signal. */
	else if (ctxt->_error.error) {
		
		/* Copy the error for return */
		memmove(error, &ctxt->_error, sizeof(error[0]));
		
		/* If there is a client stream and it's been opened, signal the error. */
		if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
			_CFWriteStreamSignalEventDelayed(ctxt->_clientWriteStream, kCFStreamEventErrorOccurred, error);
		
		/* Mark as done and error the result. */
		*atEOF = TRUE;
		result = -1;
            
		/* Make sure the socket doesn't signal anymore. */
		CFSocketDisableCallBacks(ctxt->_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack);
	}
	
	/* A read of zero is EOF. */
	else if (!result) {
	
		*atEOF = TRUE;
		
		/* Make sure the socket doesn't signal anymore. */
		CFSocketDisableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
	}
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	if (event != kCFStreamEventNone)
        CFReadStreamSignalEvent(stream, event, NULL);
	
	return result;
}


/* static */ Boolean
_SocketStreamCanRead(CFReadStreamRef stream, _CFSocketStreamContext* ctxt) {
	
	CFStreamError error;
	Boolean result = FALSE;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	/* Right now only SSL is using the buffered reads. */
	if (!__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes) && __CFBitIsSet(ctxt->_flags, kFlagBitIsBuffered)) {
		
		/* Similar to the end of _SocketStreamRead. */
		CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
		
		/* Need to check for buffered bytes or EOF. */
		if (__CFBitIsSet(ctxt->_flags, kFlagBitClosed) || (c && *((CFIndex*)CFDataGetBytePtr(c)))) {
			
			result = TRUE;
						
			/* Unlock */
			__CFSpinUnlock(&ctxt->_lock);
		}
		
		/* If none there, check to see if there are encrypted bytes that are buffered. */
		else if (__CFBitIsSet(ctxt->_flags, kFlagBitUseSSL)) {
			
			_SocketStreamSecurityBufferedRead_NoLock(ctxt);
			result = __CFBitIsSet(ctxt->_flags, kFlagBitCanRead) || __CFBitIsSet(ctxt->_flags, kFlagBitClosed);
			
			/* Unlock */
			__CFSpinUnlock(&ctxt->_lock);
		}
		
		else {
			
			/* Unlock */
			__CFSpinUnlock(&ctxt->_lock);
		
			/* Find out if can read (polling if necessary). */
			result = _SocketStreamCan(ctxt, stream, kFlagBitCanRead, _kCFStreamSocketCanReadPrivateMode, &error);
		}
	}
	
	else {
	
		/* Unlock */
		__CFSpinUnlock(&ctxt->_lock);
			
		/* Find out if can read (polling if necessary). */
		result = _SocketStreamCan(ctxt, stream, kFlagBitCanRead, _kCFStreamSocketCanReadPrivateMode, &error);
	}
	
	return result;
}


/* static */ CFIndex
_SocketStreamWrite(CFWriteStreamRef stream, const UInt8* buffer, CFIndex bufferLength,
				   CFStreamError* error, _CFSocketStreamContext* ctxt)
{
	CFIndex result = 0;
	CFStreamEventType event = kCFStreamEventNone;
	
	/* Set as no error to start. */
	memset(error, 0, sizeof(error[0]));
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	while (1) {			
		/* Wasn't time to write, so run in private mode for timeout or ability to write. */
		if (!ctxt->_error.error && !__CFBitIsSet(ctxt->_flags, kFlagBitCanWrite)) {
			
			CFRunLoopRef rl = CFRunLoopGetCurrent();
			CFRunLoopSourceContext src = {0, rl, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _SocketStreamPerformCancel};
			CFRunLoopSourceRef cancel = CFRunLoopSourceCreate(CFGetAllocator(stream), 0, &src);
			
			if (cancel) {
					
				CFAbsoluteTime later;
				CFTimeInterval interval;
				CFNumberRef value = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyWriteTimeout);
				
				CFTypeRef loopAndMode[2] = {rl, _kCFStreamSocketWritePrivateMode};
				
				CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyWriteCancel, cancel);
				CFRelease(cancel);
				
				if (!value || !CFNumberGetValue(value, kCFNumberDoubleType, &interval))
					interval = kReadWriteTimeoutInterval;
				
				later = (interval == 0.0) ? DBL_MAX : CFAbsoluteTimeGetCurrent() + interval;
				
				/* Add the current loop and the private mode to the list */
				_SchedulesAddRunLoopAndMode(ctxt->_writeloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
				
				__CFBitSet(ctxt->_flags, kFlagBitWriteHasCancel);
				
				/* Make sure to schedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesScheduleApplierFunction,
									 loopAndMode);
				
				CFRunLoopAddSource((CFRunLoopRef)loopAndMode[0], cancel, (CFStringRef)loopAndMode[1]);
				
				do {
					/* Unlock the context to allow things to fire */
					__CFSpinUnlock(&ctxt->_lock);
					
					/* Run the run loop for a poll (0.0) */
					CFRunLoopRunInMode(_kCFStreamSocketWritePrivateMode, interval, FALSE);
					
					/* Lock the context back up. */
					__CFSpinLock(&ctxt->_lock);
					
				} while (!ctxt->_error.error &&
						 !__CFBitIsSet(ctxt->_flags, kFlagBitCanWrite) &&
						 (0 < (interval = (later - CFAbsoluteTimeGetCurrent()))));
				
				__CFBitClear(ctxt->_flags, kFlagBitWriteHasCancel);
				
				CFRunLoopRemoveSource((CFRunLoopRef)loopAndMode[0], cancel, (CFStringRef)loopAndMode[1]);
				
				/* Make sure to unschedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesUnscheduleApplierFunction,
									 loopAndMode);
				
				/* Remove this loop and private mode from the list. */
				_SchedulesRemoveRunLoopAndMode(ctxt->_writeloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
				
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyWriteCancel);
				
				/* If fell out, still not time, and no error, set to timed out. */
				if (!__CFBitIsSet(ctxt->_flags, kFlagBitCanWrite) && !ctxt->_error.error) {				
					ctxt->_error.error = ETIMEDOUT;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				}
			}
			else {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			}
		}
		
		/* If there's no error, try to write now. */
		if (!ctxt->_error.error) {
		
			if (__CFBitIsSet(ctxt->_flags, kFlagBitUseSSL))
				result = _SocketStreamSecuritySend_NoLock(ctxt, buffer, bufferLength);
			else
				result = _CFSocketSend(ctxt->_socket, buffer, bufferLength, &ctxt->_error);
		}
		
		/* Did a write, so the event is no longer good. */
		__CFBitClear(ctxt->_flags, kFlagBitCanWrite);
		
		/* Got a "would block" error, so clear it and wait for time to write. */
		if ((ctxt->_error.error == EAGAIN) && (ctxt->_error.domain == _kCFStreamErrorDomainNativeSockets)) {
			memset(&ctxt->_error, 0, sizeof(ctxt->_error));
			continue;
		}

		break;
	}
		
	/* If there was an error, make sure to propagate and signal. */
	if (ctxt->_error.error) {
	
		CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
		
		/* 3863115 Only signal the read error if it's not buffered and no bytes waiting. */
		if (!c || !*((CFIndex*)CFDataGetBytePtr(c))) {
			
			/* Copy the error for return */
			memmove(error, &ctxt->_error, sizeof(error[0]));
			
			/* If there is a client stream for the other half and it's been opened, signal the error. */
			if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
				_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventErrorOccurred, error);
		}
		
		/* Mark as done and error the result. */
		result = -1;
            
		/* Make sure the socket doesn't signal anymore. */
		CFSocketDisableCallBacks(ctxt->_socket, kCFSocketWriteCallBack | kCFSocketReadCallBack);
	}
	
	/* A write of zero is EOF. */
	else if (!result) {

		/* Make sure the socket doesn't signal anymore. */
		CFSocketDisableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
	}
		
	/* Make sure to set things up correctly as a result of success. */
	else {
		
		/* If handshakes are in progress, don't perform further checks. */
		if (!__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes)) {
			
			/* If the end, signal EOF. */
			if (__CFBitIsSet(ctxt->_flags, kFlagBitClosed)) {
				event = kCFStreamEventEndEncountered;
				__CFBitSet(ctxt->_flags, kFlagBitCanWrite);
				__CFBitClear(ctxt->_flags, kFlagBitPollWrite);
			}
			
			/* If can't write then enable CFSocket to tell when. */
			else if (!_CFSocketCan(ctxt->_socket, kSelectModeWrite))
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
			
			/* Can still write so signal right away. */
			else {
				event = kCFStreamEventCanAcceptBytes;
				__CFBitSet(ctxt->_flags, kFlagBitCanWrite);
				__CFBitClear(ctxt->_flags, kFlagBitPollWrite);
			}
		}
	}
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	if (event != kCFStreamEventNone)
        CFWriteStreamSignalEvent(stream, event, NULL);
	
	return result;
}


/* static */ Boolean
_SocketStreamCanWrite(CFWriteStreamRef stream, _CFSocketStreamContext* ctxt) {
	
	CFStreamError error;
	
	/* Find out if can write (polling if necessary). */
	return _SocketStreamCan(ctxt, stream, kFlagBitCanWrite, _kCFStreamSocketCanWritePrivateMode, &error);
}


/* static */ void
_SocketStreamClose(CFTypeRef stream, _CFSocketStreamContext* ctxt) {
	
	CFMutableArrayRef loops, otherloops;
	CFIndex count;
	CFRunLoopRef rl;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	/* Workaround the fact that some objects don't signal events when not scheduled permanently. */
	rl = (CFRunLoopRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyBogusRunLoop);
	if (rl) {
		_SocketStreamUnschedule_NoLock(stream, rl, _kCFStreamSocketBogusPrivateMode, ctxt);
	}
	
	/* Figure out the proper list of schedules on which to operate. */
	if (CFGetTypeID(stream) == CFReadStreamGetTypeID()) {
		ctxt->_clientReadStream = NULL;
		loops = ctxt->_readloops;
		otherloops = ctxt->_writeloops;
	}
	else {
		ctxt->_clientWriteStream = NULL;
		loops = ctxt->_writeloops;
		otherloops = ctxt->_readloops;
	}
	
	/* Unschedule the items that are scheduled only for this half. */
	CFArrayApplyFunction(ctxt->_schedulables,
						 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
						 (CFArrayApplierFunction)_SchedulablesUnscheduleFromAllApplierFunction,
						 loops);
	
	/* Remove the list of schedules from this half. */
	CFArrayRemoveAllValues(loops);

	/* Move the list of shared loops and modes over to the other half. */
	if ((count = CFArrayGetCount(ctxt->_sharedloops))) {
	
		/* Move the shared list to the other half. */
		CFArrayAppendArray(otherloops, ctxt->_sharedloops, CFRangeMake(0, count));
		
		/* Dump the shared list. */
		CFArrayRemoveAllValues(ctxt->_sharedloops);
	}
	
	if (!ctxt->_clientReadStream && !ctxt->_clientWriteStream) {
		
		CFRange r;
		
		if (CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext))
			_SocketStreamSecurityClose_NoLock(ctxt);
		
		r = CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables));
		
		/* Unschedule the items that are scheduled.  This shouldn't happen. */
		CFArrayApplyFunction(ctxt->_schedulables, r, (CFArrayApplierFunction)_SchedulablesUnscheduleFromAllApplierFunction, otherloops);
		
		/* Dump the final list of schedules. */
		CFArrayRemoveAllValues(otherloops);

		/* Make sure to invalidate all the schedulables. */
		CFArrayApplyFunction(ctxt->_schedulables, r, (CFArrayApplierFunction)_SchedulablesInvalidateApplierFunction, NULL);
		
		/* Unscheduled and invalidated, so let them go. */
		CFArrayRemoveAllValues(ctxt->_schedulables);
		
		/* Take care of the socket if there is one. */
		if (ctxt->_socket) {
			
			/* Make sure to invalidate the socket */
			CFSocketInvalidate(ctxt->_socket);
		
			/* Dump and forget it. */
			CFRelease(ctxt->_socket);
			ctxt->_socket = NULL;
		}
		
		/* No more schedules/unschedules, so get rid of this workaround. */
		CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyBogusRunLoop);
	}
		
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
}


/* static */ CFTypeRef
_SocketStreamCopyProperty(CFTypeRef stream, CFStringRef propertyName, _CFSocketStreamContext* ctxt) {
	
	CFTypeRef result = NULL;
	CFTypeRef property;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	/* Try to just get the property from the dictionary */
	property = CFDictionaryGetValue(ctxt->_properties, propertyName);
	
	/* Must be some other type that takes a little more work to produce. */
	if (!property) {
		
		/* Client wants the far end host's name. */
		if (CFEqual(kCFStreamPropertySocketRemoteHostName, propertyName)) {
			
			/* Attempt to get the CFHostRef used for connecting. */
			CFTypeRef host = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
			
			/* If got the host, need to go for the name. */
			if (host) {
				
				/* Get the list of names. */
				CFArrayRef list = CFHostGetNames((CFHostRef)host, NULL);
				
				/* If it has names, pull the first. */
				if (list && CFArrayGetCount(list))
					property = CFArrayGetValueAtIndex(list, 0);
			}
			
			/* Not a CFHostRef, so go for the CFNetService instead. */
			else {
			
				host = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService);
				
				/* CFNetService's have a target instead. */
				if (host)
					property = CFNetServiceGetTargetHost((CFNetServiceRef)host);
			}
		}

		/* Client wants the native socket, but make sure there is one first. */
		else if (CFEqual(kCFStreamPropertySocketNativeHandle, propertyName) && ctxt->_socket) {
			
			CFSocketNativeHandle s = CFSocketGetNative(ctxt->_socket);
			
			/* Create the return value */
			result = CFDataCreate(CFGetAllocator(stream), (const void*)(&s), sizeof(s));
		}
		
		/* Support for legacy ordering.  Response was available right away. */
		else if (CFEqual(kCFStreamPropertyCONNECTResponse, propertyName)) {

			if (CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy))
				result = CFHTTPMessageCreateEmpty(CFGetAllocator(stream), FALSE); 
		}
		
		else if (CFEqual(kCFStreamPropertySSLPeerCertificates, propertyName)) {
			
			CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
			if (wrapper) {
				if (SSLGetPeerCertificates(*((SSLContextRef*)CFDataGetBytePtr(wrapper)), (CFArrayRef*)&result) && result) {
					CFRelease(result);
					result = NULL;
				}
			}
		}

		else if (CFEqual(_kCFStreamPropertySSLClientCertificates, propertyName)) {
			
			CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
			if (wrapper) {
				if (SSLGetCertificate(*((SSLContextRef*)CFDataGetBytePtr(wrapper)), (CFArrayRef*)&result) && result) {
					// note: result of SSLGetCertificate is not retained
					result = NULL;
				} else if (result) {
					CFRetain(result);
				}
			}
		}

		else if (CFEqual(_kCFStreamPropertySSLClientCertificateState, propertyName)) {
			
			CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
			if (wrapper) {
				SSLClientCertificateState clientState = kSSLClientCertNone;
				if (SSLGetClientCertificateState(*((SSLContextRef*)CFDataGetBytePtr(wrapper)), &clientState)) {
					result = NULL;
				} else {
					result = CFNumberCreate(CFGetAllocator(ctxt->_properties), kCFNumberIntType, &clientState);
				}
			}
		}

	}
	
    if (CFEqual(propertyName, kCFStreamPropertySocketSecurityLevel)) {
		
		CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
		
		if (wrapper)
			property = _SecurityGetProtocol(*((SSLContextRef*)CFDataGetBytePtr(wrapper)));
    }
	
	/* Do whatever is needed to "copy" the type if found. */
	if (property) {
		
		CFTypeID type = CFGetTypeID(property);
		
		/* Create copies of host, services, dictionaries, arrays, and http messages. */
		if (CFHostGetTypeID() == type)
			result = CFHostCreateCopy(CFGetAllocator(stream), (CFHostRef)property);
		
		else if (CFNetServiceGetTypeID() == type)
			result = CFNetServiceCreateCopy(CFGetAllocator(stream), (CFNetServiceRef)property);
		
		else if (CFDictionaryGetTypeID() == type)
			result = CFDictionaryCreateCopy(CFGetAllocator(stream), (CFDictionaryRef)property);
		
		else if (CFArrayGetTypeID() == type)
			result = CFArrayCreateCopy(CFGetAllocator(stream), (CFArrayRef)property);
		
		else if (CFHTTPMessageGetTypeID() == type)
			result = CFHTTPMessageCreateCopy(CFGetAllocator(stream), (CFHTTPMessageRef)property);
		
		/* All other types are just retained. */
		else 
			result = CFRetain(property);
	}
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	return result;
}


/* static */ Boolean
_SocketStreamSetProperty(CFTypeRef stream, CFStringRef propertyName, CFTypeRef propertyValue,
						 _CFSocketStreamContext* ctxt)
{   
	Boolean result = FALSE;
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	if (CFEqual(propertyName, kCFStreamPropertyUseAddressCache) ||
		CFEqual(propertyName, _kCFStreamSocketIChatWantsSubNet))
	{
		
		if (propertyValue)
			CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
		else
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
		
		result = TRUE;
	}
	
	else if (CFEqual(propertyName, kCFStreamPropertyAutoErrorOnSystemChange)) {
		
		if (!propertyValue) {
			
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
			
			_SocketStreamAddReachability_NoLock(ctxt);
		}
		
		else {
			CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
			
			if (CFEqual(propertyValue, kCFBooleanFalse))
				_SocketStreamRemoveReachability_NoLock(ctxt);
			else
				_SocketStreamAddReachability_NoLock(ctxt);
		}
		
		result = TRUE;
	}
	
    else if (CFEqual(propertyName, _kCFStreamSocketCreatedCallBack)) {
		
		if (!propertyValue)
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
			
		else {
		
			CFArrayRef old = (CFArrayRef)CFDictionaryGetValue(ctxt->_properties, propertyName);
			
			if (!old || !CFEqual(old, propertyValue))
				CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
		}
		
        result = TRUE;
    }

	else if (CFEqual(propertyName, kCFStreamPropertyShouldCloseNativeSocket)) {
		
		if (propertyValue)
			CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
		else
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
		
		if (ctxt->_socket) {
			
			CFOptionFlags flags = CFSocketGetSocketFlags(ctxt->_socket);
			
			if (!propertyValue) {
				
				if (__CFBitIsSet(ctxt->_flags, kFlagBitCreatedNative))
					flags &= ~kCFSocketCloseOnInvalidate;
				else
					flags |= kCFSocketCloseOnInvalidate;
			}
			
			else if (propertyValue != kCFBooleanFalse)
				flags |= kCFSocketCloseOnInvalidate;
			
			else
				flags &= ~kCFSocketCloseOnInvalidate;
			
			CFSocketSetSocketFlags(ctxt->_socket, flags);			
		}
			
		result = TRUE;
	}
	
	else if (CFEqual(propertyName, kCFStreamPropertyCONNECTProxy))
		result = _CONNECTSetInfo_NoLock(ctxt, propertyValue);
	
	else if (CFEqual(propertyName, kCFStreamPropertySocketSSLContext))
		result = _SocketStreamSecuritySetContext_NoLock(ctxt, propertyValue);
	
    else if (CFEqual(propertyName, kCFStreamPropertySSLSettings)) {

        result = _SocketStreamSecuritySetInfo_NoLock(ctxt, propertyValue);
		
		if (result) {
			
			if (propertyValue)
				CFDictionarySetValue(ctxt->_properties, kCFStreamPropertySSLSettings, propertyValue);
			else
				CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySSLSettings);
		}
    }
	
	else if (CFEqual(propertyName, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate)) {
		
		result = TRUE;
		
		if (CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext) &&
			(_SocketStreamSecurityGetSessionState_NoLock(ctxt) == kSSLIdle))
		{
			result = _SocketStreamSecuritySetAuthenticatesServerCertificates_NoLock(ctxt, propertyValue ? propertyValue : kCFBooleanTrue);
		}
        
		if (result) {
			if (propertyValue)
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, propertyValue);
			else
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate);
		}
    }
	
    else if (CFEqual(propertyName, kCFStreamPropertySocketSecurityLevel)) {
		
        CFMutableDictionaryRef settings = CFDictionaryCreateMutable(CFGetAllocator(ctxt->_properties),
																	0,
																	&kCFTypeDictionaryKeyCallBacks,
																	&kCFTypeDictionaryValueCallBacks);
        
		if (settings) {
            CFDictionaryAddValue(settings, kCFStreamSSLLevel, propertyValue);
            result = _SocketStreamSecuritySetInfo_NoLock(ctxt, settings);
            CFRelease(settings);
			
			if (result) {
				if (propertyValue)
					CFDictionarySetValue(ctxt->_properties, kCFStreamPropertySocketSecurityLevel, propertyValue);
				else
					CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySocketSecurityLevel);
			}
		}
    }
	
	else if (CFEqual(propertyName, _kCFStreamPropertySocketPeerName)) {
		
		if (propertyValue)
			CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySocketPeerName, propertyValue);
		else
			CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySSLSettings);
		
		result = TRUE;
	}
	
    else if (CFEqual(propertyName, kCFStreamPropertySOCKSProxy))
		result = _SOCKSSetInfo_NoLock(ctxt, (CFDictionaryRef)propertyValue);
	
	else if (CFEqual(propertyName, _kCFStreamPropertyHostForOpen) ||
			 CFEqual(propertyName, _kCFStreamPropertyReadTimeout) ||
			 CFEqual(propertyName, _kCFStreamPropertyWriteTimeout) ||
			 CFEqual(propertyName, _kCFStreamPropertyAutoConnectPriority))
	{
		
		if (propertyValue)
			CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
		else
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
		
		result = TRUE;
	}
	
	else if (CFEqual(propertyName, _kCFStreamPropertyRecvBufferSize) &&
			 !__CFBitIsSet(ctxt->_flags, kFlagBitOpenStarted) &&
			 !__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete))
	{
		if (!propertyValue) {
			CFDictionaryRemoveValue(ctxt->_properties, propertyName);
			__CFBitClear(ctxt->_flags, kFlagBitIsBuffered);
		}
		else if (CFNumberGetByteSize(propertyValue) == sizeof(CFIndex)) {
			CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
			__CFBitSet(ctxt->_flags, kFlagBitIsBuffered);
		}
			
		result = TRUE;
	}
	
	/* 3800596 Need to signal errors if setting property caused one. */
	if (ctxt->_error.error) {

		/* Attempt to get the count of buffered bytes. */
		CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
		
		/*
		** 3863115 If there is a client stream and it's been opened, signal
		** the error, but only if there is no bytes sitting in the buffer.
		*/
		if ((!c || !*((CFIndex*)CFDataGetBytePtr(c))) &&
			(ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened)))
		{
			_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventErrorOccurred, &ctxt->_error);
		}
		
		/* If there is a client stream and it's been opened, signal the error. */
		if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
			_CFWriteStreamSignalEventDelayed(ctxt->_clientWriteStream, kCFStreamEventErrorOccurred, &ctxt->_error);
	}
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	return result;
}


/* static */ void
_SocketStreamSchedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode,
					  _CFSocketStreamContext* ctxt)
{
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	/* Now do the actual work */
	_SocketStreamSchedule_NoLock(stream, runLoop, runLoopMode, ctxt);
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
}


/* static */ void
_SocketStreamUnschedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode,
						_CFSocketStreamContext* ctxt)
{
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	/* Now do the actual work */
	_SocketStreamUnschedule_NoLock(stream, runLoop, runLoopMode, ctxt);
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
}



#if 0
#pragma mark -
#pragma mark Utility Functions
#endif

/* static */ void
_SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, _CFSocketStreamContext* ctxt) {

	CFReadStreamRef rStream = NULL;
	CFWriteStreamRef wStream = NULL;
	CFStreamEventType event = kCFStreamEventNone;
	CFStreamError error = {0, 0};
	
	__CFSpinLock(&ctxt->_lock);

	if (!ctxt->_error.error) {

		switch (type) {
			
			case kCFSocketConnectCallBack:
				
				if (!data) {
					
					/* See if the client has turned off the error detection. */
					CFBooleanRef reach = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyAutoErrorOnSystemChange);
				
					/* Mark as open. */
					__CFBitClear(ctxt->_flags, kFlagBitOpenStarted);
					__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);
					__CFBitClear(ctxt->_flags, kFlagBitPollOpen);
					
					/* Get the streams and event to signal. */
					event = kCFStreamEventOpenCompleted;
					
					rStream = ctxt->_clientReadStream;
					wStream = ctxt->_clientWriteStream;
					
					/* Create and schedule reachability on this socket. */
					if (!reach || (reach != kCFBooleanFalse))
						_SocketStreamAddReachability_NoLock(ctxt);
				}
				
				else {
					int i;
					CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};

					ctxt->_error.error = *((SInt32 *)data);
					ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;
					
					/* Remove the socket from the schedulables. */
					_SchedulablesRemove(ctxt->_schedulables, s);
					
					/* Unschedule the socket from all loops and modes */
					for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
						_CFTypeUnscheduleFromMultipleRunLoops(s, loops[i]);
					
					/* Invalidate the socket; never to be used again. */
					_CFTypeInvalidate(s);
					
					/* Release and forget the socket */
					CFRelease(s);
					ctxt->_socket = NULL;
					
					/* Try to start the next connection. */
					if (_SocketStreamAttemptNextConnection_NoLock(ctxt)) {
					
						/* Start fresh with no error again. */
						memset(&ctxt->_error, 0, sizeof(ctxt->_error));
					}
				}
							
				break;
			
			case kCFSocketReadCallBack:
				
				/* If handshakes in place, pump those along. */
				if (__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes)) {
					CFArrayRef handshakes = (CFArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
					((_CFSocketStreamPerformHandshakeCallBack)CFArrayGetValueAtIndex(handshakes, 0))(ctxt);	
				}
				
				/* Buffered reading has special code. */
				else if (__CFBitIsSet(ctxt->_flags, kFlagBitIsBuffered)) {
					
					/* Call the buffered read stuff for SSL */
					if (__CFBitIsSet(ctxt->_flags, kFlagBitUseSSL))
						_SocketStreamSecurityBufferedRead_NoLock(ctxt);
					else
						_SocketStreamBufferedSocketRead_NoLock(ctxt);
					
					/* If that set the "can read" bit, set the event. */
					if (__CFBitIsSet(ctxt->_flags, kFlagBitCanRead)) {
						 event = kCFStreamEventHasBytesAvailable;
						 rStream = ctxt->_clientReadStream;
					}
				}
				
				else {
					__CFBitSet(ctxt->_flags, kFlagBitCanRead);
					__CFBitClear(ctxt->_flags, kFlagBitPollRead);
					event = kCFStreamEventHasBytesAvailable;
					rStream = ctxt->_clientReadStream;
				}
				
				break;
				
			case kCFSocketWriteCallBack:
				
				/* If handshakes in place, pump those along. */
				if (__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes)) {
					CFArrayRef handshakes = (CFArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
					((_CFSocketStreamPerformHandshakeCallBack)CFArrayGetValueAtIndex(handshakes, 0))(ctxt);	
				}
				
				else {
					__CFBitSet(ctxt->_flags, kFlagBitCanWrite);
					__CFBitClear(ctxt->_flags, kFlagBitPollWrite);
					event = kCFStreamEventCanAcceptBytes;
					wStream = ctxt->_clientWriteStream;
				}
				break;
			
			default:
				break;
		}
	}

	/* Got an error during processing? */
	if (ctxt->_error.error) {
		
		/* Copy the error for call out. */
		memmove(&error, &ctxt->_error, sizeof(error));
		
		/* Set the event and streams for notification. */
		event = kCFStreamEventErrorOccurred;
		rStream = ctxt->_clientReadStream;
		wStream = ctxt->_clientWriteStream;
	}
	
	/* Only signal the read stream if it's been opened. */
	if (rStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
		CFRetain(rStream);
	else
		rStream = NULL;
	
	/* Same is true for the write stream */
	if (wStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
		CFRetain(wStream);
	else
		wStream = NULL;
	
	/* If there is an event to signal, do so. */
	if (event != kCFStreamEventNone) {
		
		CFRunLoopRef rrl = NULL, wrl = NULL;
		CFRunLoopSourceRef rsrc = __CFBitIsSet(ctxt->_flags, kFlagBitReadHasCancel) ?
			(CFRunLoopSourceRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyReadCancel) :
			NULL;
		CFRunLoopSourceRef wsrc = __CFBitIsSet(ctxt->_flags, kFlagBitWriteHasCancel) ?
			(CFRunLoopSourceRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyWriteCancel) :
			NULL;
		
		if (rsrc) {
			
			CFRunLoopSourceContext c = {0};
			
			CFRetain(rsrc);
			
			CFRunLoopSourceGetContext(rsrc, &c);
			rrl = (CFRunLoopRef)(c.info);
		}
		
		if (wsrc) {
			
			CFRunLoopSourceContext c = {0};
			
			CFRetain(wsrc);
			
			CFRunLoopSourceGetContext(wsrc, &c);
			wrl = (CFRunLoopRef)(c.info);
		}
		
		/* 3863115 Only signal the read error if it's not buffered and no bytes waiting. */
		if (rStream && (event == kCFStreamEventErrorOccurred)) {
			
			CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
			
			/* If there are bytes waiting, turn the event into a read event. */
			if (c && *((CFIndex*)CFDataGetBytePtr(c))) {
				event = kCFStreamEventHasBytesAvailable;
				memset(&error, 0, sizeof(error));
			}
		}

		__CFSpinUnlock(&ctxt->_lock);
		
		if (rStream) {
			
			if (!rsrc)
				CFReadStreamSignalEvent(rStream, event, &error);
			else {
				CFRunLoopSourceSignal(rsrc);
				CFRunLoopWakeUp(rrl);
			}
		}
		
		if (wStream) {	

			if (!wsrc)
				CFWriteStreamSignalEvent(wStream, event, &error);
			else {
				CFRunLoopSourceSignal(wsrc);
				CFRunLoopWakeUp(wrl);
			}
		}
		
		if (rsrc) CFRelease(rsrc);
		if (wsrc) CFRelease(wsrc);
	}
	else
		__CFSpinUnlock(&ctxt->_lock);
	
	if (rStream) CFRelease(rStream);
	if (wStream) CFRelease(wStream);
}


/* static */ void
_HostCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError* error, _CFSocketStreamContext* ctxt) {

	int i;
	CFArrayRef addresses;
	CFMutableArrayRef loops[3];
	CFStreamError err;

	/* Only set to non-NULL if there is an error. */
	CFReadStreamRef rStream = NULL;
	CFWriteStreamRef wStream = NULL;
	
	/* NOTE the early bail!  Only care about the address callback. */
	if (typeInfo != kCFHostAddresses) return;
	
	/* Lock down the context. */
	__CFSpinLock(&ctxt->_lock);
	
	/* Handle the error */
	if (error->error)
		memmove(&(ctxt->_error), error, sizeof(error[0]));
					
	/* Remove the host from the schedulables since it's done. */
	_SchedulablesRemove(ctxt->_schedulables, theHost);
	
	/* Invalidate it so no callbacks occur. */
	_CFTypeInvalidate(theHost);
	
	/* Grab the list of run loops and modes for unscheduling. */
	loops[0] = ctxt->_readloops;
	loops[1] = ctxt->_writeloops;
	loops[2] = ctxt->_sharedloops;
	
	/* Make sure to remove the host lookup from all loops and modes. */
	for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
		_CFTypeUnscheduleFromMultipleRunLoops(theHost, loops[i]);
	
	/* Cancel the resolution for good measure. */
	CFHostCancelInfoResolution(theHost, kCFHostAddresses);
	
	if (!error->error) {
		
		/* Get the list of addresses for verification. */
		addresses = CFHostGetAddressing(theHost, NULL);
		
		/* Only attempt to connect if there are addresses. */
		if (addresses && CFArrayGetCount(addresses))
			_SocketStreamAttemptNextConnection_NoLock(ctxt);
			
		/* Mark an error that states that the host has no addresses. */
		else {
			ctxt->_error.error = EAI_NODATA;
			ctxt->_error.domain = kCFStreamErrorDomainNetDB;
		}
	}
	
	if (ctxt->_error.error) {

		/* Check to see if there is another lookup beyond this one. */
		CFTypeRef extra = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
		
		/* If didn't find one or the found one is not the current, need to invalidate and such. */
		if (!extra || (extra != theHost)) {
			
			/* If didn't find a lookup, see if there is a CFNetService lookup. */
			if (!extra)
				extra = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService);
				
			/* If it's removed from the list, need to unschedule, invalidate, and cancel it. */
			if (extra && _SchedulablesRemove(ctxt->_schedulables, extra)) {
				
				/* Make sure to remove the lookup from all loops and modes. */
				for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
					_CFTypeUnscheduleFromMultipleRunLoops(theHost, loops[i]);
				
				/* Invalidate it so no callbacks occur. */
				_CFTypeInvalidate(theHost);
		
				/* Cancel the resolution. */
				if (CFGetTypeID(extra) == CFHostGetTypeID())
					CFHostCancelInfoResolution((CFHostRef)extra, kCFHostAddresses);
				else
					CFNetServiceCancel((CFNetServiceRef)extra);
			}
		}
		
		/* Attempt "Auto Connect" if certain netdb errors are hit. */
		if (ctxt->_error.domain == kCFStreamErrorDomainNetDB) {
			
			switch (ctxt->_error.error) {
				
				case EAI_NODATA:
//				case 0xFECEFECE:
					_SocketStreamAttemptAutoVPN_NoLock(ctxt, (CFStringRef)CFArrayGetValueAtIndex(CFHostGetNames(theHost, NULL), 0));
					break;
					
				default:
					break;
			}
		}

		/* If there was an error at some point, mark complete and prepare for failure. */
		if (ctxt->_error.error) {
			
			__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);
			__CFBitClear(ctxt->_flags, kFlagBitOpenStarted);
			__CFBitClear(ctxt->_flags, kFlagBitPollOpen);

			/* Copy the error for notification. */
			memmove(&err, &ctxt->_error, sizeof(err));
			
			/* Grab the client streams for error notification. */
			if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
				rStream = (CFReadStreamRef)CFRetain(ctxt->_clientReadStream);
				
			if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
				wStream = (CFWriteStreamRef)CFRetain(ctxt->_clientWriteStream);
		}
	}
	
	/* Unlock now. */
	__CFSpinUnlock(&ctxt->_lock);
	
	/* Signal the streams of the error event. */
	if (rStream) {
		CFReadStreamSignalEvent(rStream, kCFStreamEventErrorOccurred, &err);
		CFRelease(rStream);
	}
		
	if (wStream) {
		CFWriteStreamSignalEvent(wStream, kCFStreamEventErrorOccurred, &err);
		CFRelease(wStream);
	}
}


/* static */ void
_NetServiceCallBack(CFNetServiceRef theService, CFStreamError* error, _CFSocketStreamContext* ctxt) {

	int i;
	CFMutableArrayRef loops[3];
	CFArrayRef addresses;

	/* Only set to non-NULL if there is an error. */
	CFReadStreamRef rStream = NULL;
	CFWriteStreamRef wStream = NULL;
	
	/* Lock down the context. */
	__CFSpinLock(&ctxt->_lock);
	
	/* Copy the error into the context. */
	if (error->error)
		memmove(&(ctxt->_error), error, sizeof(error[0]));
	
	/* Remove the host from the schedulables since it's done. */
	_SchedulablesRemove(ctxt->_schedulables, theService);
	
	/* Invalidate it so no callbacks occur. */
	_CFTypeInvalidate(theService);
	
	/* Grab the list of run loops and modes for unscheduling. */
	loops[0] = ctxt->_readloops;
	loops[1] = ctxt->_writeloops;
	loops[2] = ctxt->_sharedloops;
	
	/* Make sure to remove the host lookup from all loops and modes. */
	for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
		_CFTypeUnscheduleFromMultipleRunLoops(theService, loops[i]);
	
	/* Cancel the resolution for good measure. */
	CFNetServiceCancel(theService);
		
	if (!error->error) {
	
		/* Get the list of addresses for verification. */
		addresses = CFNetServiceGetAddressing(theService);
		
		/* Only attempt to connect if there are addresses. */
		if (addresses && CFArrayGetCount(addresses))
			_SocketStreamAttemptNextConnection_NoLock(ctxt);
			
		/* Mark an error that states that the host has no addresses. */
		else {
			ctxt->_error.error = EAI_NODATA;
			ctxt->_error.domain = kCFStreamErrorDomainNetDB;
		}
	}
		
	if (ctxt->_error.error) {
		
		/* Copy the error for notification. */
		memmove(&ctxt->_error, error, sizeof(error));
		
		/* Grab the client streams for error notification. */
		if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
			rStream = (CFReadStreamRef)CFRetain(ctxt->_clientReadStream);
			
		if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
			wStream = (CFWriteStreamRef)CFRetain(ctxt->_clientWriteStream);
	}
	
	/* Unlock now. */
	__CFSpinUnlock(&ctxt->_lock);
	
	/* Signal the streams of the error event. */
	if (rStream) {
		CFReadStreamSignalEvent(rStream, kCFStreamEventErrorOccurred, (CFStreamError*)(&error));
		CFRelease(rStream);
	}
		
	if (wStream) {
		CFWriteStreamSignalEvent(wStream, kCFStreamEventErrorOccurred, (CFStreamError*)(&error));
		CFRelease(wStream);
	}
}


/* static */ void
_SocksHostCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError* error, _CFSocketStreamContext* ctxt) {

	CFStreamError err;
	
	/* Only set to non-NULL if there is an error. */
	CFReadStreamRef rStream = NULL;
	CFWriteStreamRef wStream = NULL;
	
	/* NOTE the early bail!  Only care about the address callback. */
	if (typeInfo != kCFHostAddresses) return;
	
	/* Lock down the context. */
	__CFSpinLock(&ctxt->_lock);
	
	/* Tell SOCKS to handle it. */
	_SocketStreamSOCKSHandleLookup_NoLock(ctxt, theHost);
	
	/*
	** Cancel the resolution for good measure.  Object should have
	** been unscheduled and invalidated in the SOCKS call.
	*/
	CFHostCancelInfoResolution(theHost, kCFHostAddresses);

	if (ctxt->_error.error) {
		
		__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);
		__CFBitClear(ctxt->_flags, kFlagBitOpenStarted);
		__CFBitClear(ctxt->_flags, kFlagBitPollOpen);

		/* Copy the error for notification. */
		memmove(&err, &ctxt->_error, sizeof(err));
		
		/* Grab the client streams for error notification. */
		if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
			rStream = (CFReadStreamRef)CFRetain(ctxt->_clientReadStream);
		
		if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
			wStream = (CFWriteStreamRef)CFRetain(ctxt->_clientWriteStream);
	}
	
	/* Unlock now. */
	__CFSpinUnlock(&ctxt->_lock);
	
	/* Signal the streams of the error event. */
	if (rStream) {
		CFReadStreamSignalEvent(rStream, kCFStreamEventErrorOccurred, &err);
		CFRelease(rStream);
	}
	
	if (wStream) {
		CFWriteStreamSignalEvent(wStream, kCFStreamEventErrorOccurred, &err);
		CFRelease(wStream);
	}
}


/* static */ void
_ReachabilityCallBack(SCNetworkReachabilityRef target, const SCNetworkConnectionFlags flags, _CFSocketStreamContext* ctxt) {

	CFDataRef c;
	
	/*
    ** 3483384 If the reachability callback fires, there was a change in
    ** routing for this pair and it should get an error.
	*/
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
    
    ctxt->_error.error = ENOTCONN;
    ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;

	/* Attempt to get the count of buffered bytes. */
	c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);

	/*
	** 3863115 If there is a client stream and it's been opened, signal the error, but
	** only if there are no bytes sitting in the buffer.
	*/
	if ((!c || !*((CFIndex*)CFDataGetBytePtr(c))) &&
		(ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened)))
	{
		_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventErrorOccurred, &ctxt->_error);
	}
	
	/* If there is a client stream and it's been opened, signal the error. */
	if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
		_CFWriteStreamSignalEventDelayed(ctxt->_clientWriteStream, kCFStreamEventErrorOccurred, &ctxt->_error);
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
}


/* static */ void
_NetworkConnectionCallBack(SCNetworkConnectionRef conn, SCNetworkConnectionStatus status, _CFSocketStreamContext* ctxt) {

	CFReadStreamRef rStream = NULL;
	CFWriteStreamRef wStream = NULL;
	CFStreamEventType event = kCFStreamEventNone;
	CFStreamError error = {0, 0};
	
	/* Only perform anything for the final states. */
	switch (status) {
		
		case kSCNetworkConnectionConnected:			
		case kSCNetworkConnectionDisconnected:
		case kSCNetworkConnectionInvalid:
			
		{
			int i;
			CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
			
			/* Lock down the context */
			__CFSpinLock(&ctxt->_lock);

			/* Unschedule the connection from all loops and modes */
			for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
				_CFTypeUnscheduleFromMultipleRunLoops(conn, loops[i]);
			
			/* Invalidate the connection; never to be used again. */
			_CFTypeInvalidate(conn);
			
			/* Remove the connection from the schedulables. */
			_SchedulablesRemove(ctxt->_schedulables, conn);
			
			if (!_SocketStreamStartLookupForOpen_NoLock(ctxt)) {
				
				/*
				 ** If no lookup started and no error, everything must be
				 ** ready for a connect attempt.
				 */
				if (!ctxt->_error.error)
					_SocketStreamAttemptNextConnection_NoLock(ctxt);
			}
			
			/* Did connect actually progress all the way through? */
			if (__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete)) {
				
				/* Remove the "started" flag */
				__CFBitClear(ctxt->_flags, kFlagBitOpenStarted);
				
				/* Get the streams and event to signal. */
				event = kCFStreamEventOpenCompleted;
				
				if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
					rStream = ctxt->_clientReadStream;
				
				if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
					wStream = ctxt->_clientWriteStream;
			}
			
			/* Copy the error if one occurred. */
			if (ctxt->_error.error) {
				memmove(&error, &ctxt->_error, sizeof(error));
				
				/* Set the event and streams for notification. */
				event = kCFStreamEventErrorOccurred;
				
				if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
					rStream = ctxt->_clientReadStream;
				
				if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
				wStream = ctxt->_clientWriteStream;
			}
			
			if (rStream) CFRetain(rStream);
			if (wStream) CFRetain(wStream);
					
			/* Unlock */
			__CFSpinUnlock(&ctxt->_lock);
			
			break;
		}
			
		default:
			break;
	}
	
	if (event != kCFStreamEventNone) {
		
		if (rStream)
			CFReadStreamSignalEvent(rStream, event, &error);
		
		if (wStream) 			
			CFWriteStreamSignalEvent(wStream, event, &error);
	}
	
	if (rStream) CFRelease(rStream);
	if (wStream) CFRelease(wStream);
}


/* static */ _CFSocketStreamContext*
_SocketStreamCreateContext(CFAllocatorRef alloc) {
	
	/* Allocate the base structure */
	_CFSocketStreamContext* ctxt = (_CFSocketStreamContext*)CFAllocatorAllocate(alloc,
																				sizeof(ctxt[0]),
																				0);
	
	/* Continue on if successful */
	if (ctxt) {
		
		/* Zero everything to start. */
        memset(ctxt, 0, sizeof(ctxt[0]));
		
		/* Create the arrays for run loops and modes. */
		ctxt->_readloops = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		ctxt->_writeloops = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		ctxt->_sharedloops = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		
		/*  Create the set for the list of schedulable items. */
		ctxt->_schedulables = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		
		/* Create a dictionary to hold the properties. */
        ctxt->_properties = CFDictionaryCreateMutable(alloc,
                                                      0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);

		/* If anything failed, need to cleanup and toss result */
		if (!ctxt->_readloops || !ctxt->_writeloops || !ctxt->_sharedloops ||
			!ctxt->_schedulables || !ctxt->_properties)
		{
			ctxt = NULL;
		}
	}
	
	return ctxt;
}


/* static */ void
_SocketStreamDestroyContext_NoLock(CFAllocatorRef alloc, _CFSocketStreamContext* ctxt) {

	int i;
	CFMutableArrayRef loops[] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};

	/* Make sure to unschedule all the schedulables on this loop and mode. */
	if (ctxt->_schedulables) {
		
		CFRange r = CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables));
		
		/* Unschedule the schedulables from all run loops and modes. */
		for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++) {
			if (loops[i])
				CFArrayApplyFunction(ctxt->_schedulables, r, (CFArrayApplierFunction)_SchedulablesUnscheduleFromAllApplierFunction, loops[i]);
		}
		
		/* Make sure to invalidate them all. */
		CFArrayApplyFunction(ctxt->_schedulables, r, (CFArrayApplierFunction)_SchedulablesInvalidateApplierFunction, NULL);
	
		/* Release them all now. */
		CFRelease(ctxt->_schedulables);
	}
	
	/* Get rid of the socket */
	if (ctxt->_socket) {
		
		/* Make sure to invalidate the socket */
		CFSocketInvalidate(ctxt->_socket);
		
		CFRelease(ctxt->_socket);
	}

	/* Release the lists of run loops and modes */
	for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
		if (loops[i]) CFRelease(loops[i]);
	
	/* Get rid of any properties */
	if (ctxt->_properties)
		CFRelease(ctxt->_properties);
	
	/* Toss the context */
	CFAllocatorDeallocate(alloc, ctxt);
}


/* static */ Boolean
_SchedulablesAdd(CFMutableArrayRef schedulables, CFTypeRef addition) {

	if (!CFArrayContainsValue(schedulables, CFRangeMake(0, CFArrayGetCount(schedulables)), addition)) {
		CFArrayAppendValue(schedulables, addition);
		return TRUE;
	}
	
	return FALSE;
}


/* static */ Boolean
_SchedulablesRemove(CFMutableArrayRef schedulables, CFTypeRef removal) {
	
	CFIndex i = CFArrayGetFirstIndexOfValue(schedulables, CFRangeMake(0, CFArrayGetCount(schedulables)), removal);
	
	if (i != kCFNotFound) {
		CFArrayRemoveValueAtIndex(schedulables, i);
		return TRUE;
	}
	
	return FALSE;
}


/* static */ void
_SchedulablesScheduleApplierFunction(CFTypeRef obj, CFTypeRef loopAndMode[]) {
	
	/* Schedule the object on the loop and mode */
	_CFTypeScheduleOnRunLoop(obj, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
}


/* static */ void
_SchedulablesUnscheduleApplierFunction(CFTypeRef obj, CFTypeRef loopAndMode[]) {
	
	/* Remove the object from the loop and mode */
	_CFTypeUnscheduleFromRunLoop(obj, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
}


/* static */ void
_SchedulablesUnscheduleFromAllApplierFunction(CFTypeRef obj, CFArrayRef schedules) {
	
	/* Remove the object from all the run loops and modes */
	_CFTypeUnscheduleFromMultipleRunLoops(obj, schedules);
}


/* static */ void
_SchedulablesInvalidateApplierFunction(CFTypeRef obj, void* context) {
	
	(void)context;  /* unused */
	
	CFTypeID type = CFGetTypeID(obj);
	
	/* Invalidate the process. */
	_CFTypeInvalidate(obj);
	
	/* For CFHost and CFNetService, make sure to cancel too. */
	if (CFHostGetTypeID() == type)
		CFHostCancelInfoResolution((CFHostRef)obj, kCFHostAddresses);
	else if (CFNetServiceGetTypeID() == type)
		CFNetServiceCancel((CFNetServiceRef)obj);
}


/* static */ void
_SocketStreamSchedule_NoLock(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode,
							 _CFSocketStreamContext* ctxt)
{
	CFMutableArrayRef loops, otherloops;
	Boolean isReadStream = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
	
	/*
	 ** Figure out the proper loops and modes to use.  loops refers to
	 ** the list of schedules for the stream half which was passed into
	 ** the function.  otherloops refers to the list of schedules for
	 ** the other half.
	 */
	if (isReadStream) {
		loops = ctxt->_readloops;
		otherloops = ctxt->_writeloops;
	}
	else {
		loops = ctxt->_writeloops;
		otherloops = ctxt->_readloops;
	}
	
	/*
	 ** If the loop and mode are already in the shared list or the current
	 ** half is already scheduled on this loop and mode, don't do anything.
	 */
	if ((kCFNotFound == _SchedulesFind(ctxt->_sharedloops, runLoop, runLoopMode)) &&
		(kCFNotFound == _SchedulesFind(loops, runLoop, runLoopMode)))
	{
		/* Different behavior if the other half is scheduled on this loop and mode */
		if (kCFNotFound == _SchedulesFind(otherloops, runLoop, runLoopMode)) {
			
			CFTypeRef loopAndMode[2] = {runLoop, runLoopMode};
			
			/* Other half not scheduled, so only schedule on this half. */
			_SchedulesAddRunLoopAndMode(loops, runLoop, runLoopMode);
			
			/* Make sure to schedule all the schedulables on this loop and mode. */
			CFArrayApplyFunction(ctxt->_schedulables,
								 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
								 (CFArrayApplierFunction)_SchedulablesScheduleApplierFunction,
								 loopAndMode);								 
		}
		
		else {
			
			/* Other half is scheduled already, so remove this schedule from the other half. */
			_SchedulesRemoveRunLoopAndMode(otherloops, runLoop, runLoopMode);
			
			/* Promote this schedule to being shared. */
			_SchedulesAddRunLoopAndMode(ctxt->_sharedloops, runLoop, runLoopMode);
			
			/* NOTE that the schedulables are not scheduled since they already have been. */
		}
		
		if (isReadStream) {
			if (__CFBitIsSet(ctxt->_flags, kFlagBitCanRead) &&
				(CFArrayGetCount(loops) + CFArrayGetCount(ctxt->_sharedloops) == 4))
			{
				CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
			}
		}
		
		else {
			if (__CFBitIsSet(ctxt->_flags, kFlagBitCanWrite) &&
				(CFArrayGetCount(loops) + CFArrayGetCount(ctxt->_sharedloops) == 4))
			{
				CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
			}
		}
	}
}


/* static */ void
_SocketStreamUnschedule_NoLock(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode,
							   _CFSocketStreamContext* ctxt)
{
	CFMutableArrayRef loops, otherloops;
	
	/*
	 ** Figure out the proper loops and modes to use.  loops refers to
	 ** the list of schedules for the stream half which was passed into
	 ** the function.  otherloops refers to the list of schedules for
	 ** the other half.
	 */
	if (CFGetTypeID(stream) == CFReadStreamGetTypeID()) {
		loops = ctxt->_readloops;
		otherloops = ctxt->_writeloops;
	}
	else {
		loops = ctxt->_writeloops;
		otherloops = ctxt->_readloops;
	}
	
	/* Remove the loop and mode from the shared list if there */
	if (_SchedulesRemoveRunLoopAndMode(ctxt->_sharedloops, runLoop, runLoopMode)) {
		
		/* Demote the schedule down to one half instead of shared. */
		_SchedulesAddRunLoopAndMode(otherloops, runLoop, runLoopMode);
		
		/*
		 ** NOTE that the schedulables are not unscheduled, since they're
		 ** still scheduled for the other half.
		 */
	}
	
	/* Wasn't in the shared list, so try removing it from the list for this half. */
	else if (_SchedulesRemoveRunLoopAndMode(loops, runLoop, runLoopMode)) {
		
		CFTypeRef loopAndMode[2] = {runLoop, runLoopMode};
		
		/* Make sure to unschedule all the schedulables on this loop and mode. */
		CFArrayApplyFunction(ctxt->_schedulables,
							 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
							 (CFArrayApplierFunction)_SchedulablesUnscheduleApplierFunction,
							 loopAndMode);
	}
}


/* static */ CFNumberRef
_CFNumberCopyPortForOpen(CFDictionaryRef properties) {
	
	CFNumberRef result = NULL;
	
	/* Attempt to grab the SOCKS proxy information */
	CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(properties, kCFStreamPropertySOCKSProxy);
	
	/* If SOCKS proxy is being used, need to go to it. */
	if (proxy) {
	
		/* Try to get the one that was passed in. */
		result = (CFNumberRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSProxyPort);
		
		if (result)
			CFRetain(result);
		
		/* If not one, create one as the default. */
		else {
		
			SInt32 default_port = 1080;		/* Default SOCKS port */
			
			/* Create the CFNumber from the default port */
			result = CFNumberCreate(CFGetAllocator(properties), kCFNumberSInt32Type, &default_port);
		}
	}
	
	/* If there is no SOCKS proxy, it could be a CONNECT proxy. */
	else if ((proxy = (CFDictionaryRef)CFDictionaryGetValue(properties, kCFStreamPropertyCONNECTProxy))) {
	
		/* Try to get the one that was passed in. */
		result = (CFNumberRef)CFDictionaryGetValue(proxy, kCFStreamPropertyCONNECTProxyPort);
		
		/* There is no default port for CONNECT tunneling. */
		
		if (result) CFRetain(result);
	}
	
	/* It's direct to the host */
	else {
		result = (CFNumberRef)CFDictionaryGetValue(properties, _kCFStreamPropertySocketRemotePort);
		
		if (result) CFRetain(result);
	}
	
	return result;
}


/* static */ CFDataRef
_CFDataCopyAddressByInjectingPort(CFDataRef address, CFNumberRef port) {
	
	/*
	** If there was no port given, assume the port is in the address
	** already and just give the address an extra retain.
	*/
	if (!port)
		CFRetain(address);
	
	/* There is a port to inject */
	else {
	
		SInt32 p;
		
		/* If the port can't be retrieved from the number, return no address. */
		if (!CFNumberGetValue(port, kCFNumberSInt32Type, &p))
			address = NULL;
		
		/* Need to inject the port value now. */
		else {
			
			/* Handle injection based upon address family */
			switch (((struct sockaddr*)CFDataGetBytePtr(address))->sa_family) {
				
				case AF_INET:
					/* Create a copy for injection. */
					address = CFDataCreateMutableCopy(CFGetAllocator(address),
													  0,
													  address);
					
					/* Only place it there if a copy was made. */
					if (address)
						((struct sockaddr_in*)(CFDataGetMutableBytePtr((CFMutableDataRef)address)))->sin_port = htons(0x0000FFFF & p);
					break;
					
				case AF_INET6:
					/* Create a copy for injection. */
					address = CFDataCreateMutableCopy(CFGetAllocator(address),
													  0,
													  address);
					
					/* Only place it there if a copy was made. */
					if (address)
						((struct sockaddr_in6*)(CFDataGetMutableBytePtr((CFMutableDataRef)address)))->sin6_port = htons(0x0000FFFF & p);
					break;
				
				/*
				** Fail for an address family that is not known and is supposed
				** to get an injected port value.
				*/
				default:
					address = NULL;
					break;
			}
		}
	}

	return address;
}


/* static */ Boolean
_ScheduleAndStartLookup(CFTypeRef lookup, CFArrayRef* schedules, CFStreamError* error, const void* cb, void* info) {

	do {
		int i;
		CFArrayRef addresses = NULL;
		CFTypeID lookup_type = CFGetTypeID(lookup);
		CFTypeID host_type = CFHostGetTypeID();

		/* Set to no error. */
		memset(error, 0, sizeof(error[0]));
		
		/* Get the list of addresses, if any, from the lookup. */
		if (lookup_type == host_type)
			addresses = CFHostGetAddressing((CFHostRef)lookup, NULL);
		else
			addresses = CFNetServiceGetAddressing((CFNetServiceRef)lookup);
		
		/* If there are existing addresses, try to use them. */
		if (addresses) {
			
			/* If there is at least one address, use it. */
			if (CFArrayGetCount(addresses))
				break;
			
			/* No addresses in the list */
			else {
				
				/* Mark an error that states that the host has no addresses. */
				error->error = EAI_NODATA;
				error->domain = kCFStreamErrorDomainNetDB;
				
				break;
			}
		}
		
		/* Set the stream as the client for callback. */
		if (lookup_type == host_type) {
			CFHostClientContext c = {0, info, NULL, NULL, NULL};			
			CFHostSetClient((CFHostRef)lookup, (CFHostClientCallBack)cb, &c);
		}
		else {
			CFNetServiceClientContext c = {0, info, NULL, NULL, NULL};
			CFNetServiceSetClient((CFNetServiceRef)lookup, (CFNetServiceClientCallBack)cb, &c);
		}
		
		/* Now schedule the lookup on all loops and modes */
		for (i = 0; schedules[i]; i++)
			_CFTypeScheduleOnMultipleRunLoops(lookup, schedules[i]);
		
		/* Start the lookup */
		if (lookup_type == host_type)
			CFHostStartInfoResolution((CFHostRef)lookup, kCFHostAddresses, error);
		else
			CFNetServiceResolveWithTimeout((CFNetServiceRef)lookup, 0.0, error);
		
		/* Verify that the lookup started. */
		if (error->error) {
			
			/* Remove it from the all schedules. */
			for (i = 0; schedules[i]; i++)
				_CFTypeUnscheduleFromMultipleRunLoops(lookup, schedules[i]);
			
			/* Invalidate the lookup; never to be used again. */
			_CFTypeInvalidate(lookup);
			
			break;
		}
		
		/* Did start a lookup. */
		return TRUE;
	}
	while (0);

	/* Did not start a lookup. */
	return FALSE;
}


/* static */ CFIndex
_CFSocketRecv(CFSocketRef s, UInt8* buffer, CFIndex length, CFStreamError* error) {
	
	CFIndex result = -1;
	
	/* Zero out the error (no error). */
	memset(error, 0, sizeof(error[0]));
	
	/* If the socket is invalid, return an EINVAL error. */
	if (!s || !CFSocketIsValid(s)) {
		error->error = EINVAL;
		error->domain = kCFStreamErrorDomainPOSIX;
	}
	
	else {		
		/* Try to read some bytes off the socket. */
		result = read(CFSocketGetNative(s), buffer, length);
		
		/* If recv returned an error, get the error and make sure to return -1. */
		if (result < 0) {
			_LastError(error);
			result = -1;
		}
	}
	
    return result;
}


/* static */ CFIndex
_CFSocketSend(CFSocketRef s, const UInt8* buffer, CFIndex length, CFStreamError* error) {
	
	CFIndex result = -1;
	
	/* Zero out the error (no error). */
	memset(error, 0, sizeof(error[0]));
	
	/* If the socket is invalid, return an EINVAL error. */
	if (!s || !CFSocketIsValid(s)) {
		error->error = EINVAL;
		error->domain = kCFStreamErrorDomainPOSIX;
	}
	
	else {		
		/* Try to read some bytes off the socket. */
		result = write(CFSocketGetNative(s), buffer, length);
		
		/* If recv returned an error, get the error and make sure to return -1. */
		if (result < 0) {
			_LastError(error);
			result = -1;
		}
	}
	
    return result;
}


/* static */ Boolean
_CFSocketCan(CFSocketRef s, int mode) {
    
	/*
	** Unfortunately, this function is required as a result of some odd behavior
	** code in CFSocket.  In cases where CFSocket is not scheduled but enable/
	** disable of the callbacks are called, they are not truly enabled/disabled.
	** Once the CFSocket is scheduled again, it enables all the original events
	** which is not necessarily CFSocketStream's intended state.  This code
	** is then used in order to double check the incoming event from CFSocket.
	*/ 
	
	/*
	** This code is also used as a performance win at the end of SocketStreamRead
	** and SocketStreamWrite.  It's cheaper to quickly poll the fd than it is
	** to return to the run loop wait for the event from CFSocket and then signal
	** the client that reading or writing can be performed.
	*/
	
	int val;
    fd_set	set;
    fd_set* setptr = &set;
    
    struct timeval timeout = {0, 0};
	
	int fd = CFSocketGetNative(s);
	
    FD_ZERO(setptr);
	
#if !defined __WIN32__
    /* Irrelevant on Win32, because they don't use a bitmask for select args */
    if (fd >= FD_SETSIZE) {
        
        val = howmany(fd + 1, NFDBITS) * sizeof(fd_mask);
        
        setptr = (fd_set*)malloc(val);
        bzero(setptr, val);
    }
#endif    
    
    FD_SET(fd, setptr);
	
    val = select(fd + 1,
				 (mode & kSelectModeRead ? setptr : NULL),
				 (mode & kSelectModeWrite ? setptr : NULL),
				 (mode & kSelectModeExcept ? setptr : NULL),
				 &timeout);
    
    if (setptr != &set)
        free(setptr);
	
    return (val > 0) ? TRUE : FALSE;
}


/* static */ Boolean
_SocketStreamStartLookupForOpen_NoLock(_CFSocketStreamContext* ctxt) {
	
	Boolean result = FALSE;
	
	do {
		CFTypeRef lookup = NULL;
		CFHostRef extra = NULL;			/* Used in the case of SOCKSv4 only */
		CFTypeID lookup_type, host_type = CFHostGetTypeID();
		CFArrayRef loops[4] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops, NULL};
		
		/* Attempt to grab the SOCKS proxy information */
		CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
		
		/* If SOCKS proxy is being used, need to go to it. */
		if (proxy) {
			
			/* Create the host from the host name. */
			lookup = CFHostCreateWithName(CFGetAllocator(ctxt->_properties),
										  (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSProxyHost));
			
			/*
			** If trying to do a SOCKSv4 connection, need to get the address.
			** If lookup fails, SOCKSv4a will be tried instread.
			*/
			if (lookup && CFEqual(_GetSOCKSVersion(proxy), kCFStreamSocketSOCKSVersion4)) {
				
				/* Grab the far end, intended host. */
				extra = (CFHostRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
				
				/* If one wasn't found, give an invalid argument error.  This should never occur. */
				if (!extra) {
					ctxt->_error.error = EINVAL;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Try to schedule the lookup. */
				else if (_ScheduleAndStartLookup(extra,
												 loops,
												 &ctxt->_error,
												 (const void*)_SocksHostCallBack,
												 ctxt))
				{					
					/* Add it to the list of schedulables for future scheduling calls. */
					_SchedulablesAdd(ctxt->_schedulables, extra);
				}
				
				/* If there was an error, bail.  If no error, it means there was an address already. */
				else if (ctxt->_error.error) {
					
					/* Not needed */
					CFRelease(lookup);
					break;
				}
			}
		}
		
		/* If there is no SOCKS proxy, it could be a CONNECT proxy. */
		else if ((proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy))) {
			
			/* Create the host from the host name */
			lookup = CFHostCreateWithName(CFGetAllocator(ctxt->_properties),
										  (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertyCONNECTProxyHost));
		}
		
		/* It's direct to the host */
		else {
			
			/* No proxies so go for the remote host. */
			lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
			
			/* If no host, then it's CFNetService based. */
			if (!lookup)
				lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService);
			
			/* There should always be a lookup of some sort, but just in case. */
			if (lookup) CFRetain(lookup);
		}
		
		/* If there is no host for lookup, specify an error. */
		if (!lookup) {
			
			/* If there is no socket already, there must be a lookup. */
			if (!ctxt->_socket && !CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketNativeHandle)) {
				
				/* Attempt to get the error from errno. */
				ctxt->_error.error = errno;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				
				/* If errno is not filled, assume no memory. */
				if (!ctxt->_error.error)
					ctxt->_error.error = ENOMEM;
			}
			
			break;
		}
		
		/* Get the type of lookup for type specific work. */
		lookup_type = CFGetTypeID(lookup);
		
		/* Given the lookup, try to kick it off */
		result = _ScheduleAndStartLookup(lookup,
										 loops,
										 &ctxt->_error,
										 ((lookup_type == host_type) ? (const void*)_HostCallBack : (const void*)_NetServiceCallBack),
										 ctxt);
		
		/* Add it to the list of schedulables for future scheduling calls. */
		if (result)
			_SchedulablesAdd(ctxt->_schedulables, lookup);
		
		/* Scheduling failed as a result of an error. */
		else if (ctxt->_error.error) {

			/* Release the lookup */
			CFRelease(lookup);
			
			/* Need to cancel and cleanup any lookup started as a result of socksv4. */
			if (extra) {
				
				int i;
				
				/* Remove the sockv4 lookup from the list of schedulables. */
				_SchedulablesRemove(ctxt->_schedulables, extra);
				
				/* Remove it from the list of all schedules. */
				for (i = 0; loops[i]; i++)
					_CFTypeUnscheduleFromMultipleRunLoops(extra, loops[i]);
				
				/* Invalidate the socksv4 lookup; never to be used again. */
				_CFTypeInvalidate(extra);
				
				/* Cancel any lookup that it was doing. */
				CFHostCancelInfoResolution((CFHostRef)extra, kCFHostAddresses);
			}
			
			break;
		}
		
		/* Save the lookup in the properties list for iteration and socket creation later. */
		CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyHostForOpen, lookup);
		
		/* Release the lookup now. */
		CFRelease(lookup);

	} while (0);
	
	return result;
}


/* static */ Boolean
_SocketStreamCreateSocket_NoLock(_CFSocketStreamContext* ctxt, CFDataRef address) {
	
	do {
		SInt32 protocolFamily = PF_INET;
		SInt32 socketType = SOCK_STREAM;
		SInt32 protocol = IPPROTO_TCP;
		
        int yes = 1;
		CFOptionFlags flags;
		CFSocketNativeHandle s;		
		CFSocketContext c = {0, ctxt, NULL, NULL, NULL};
		
		CFArrayRef callback;
		CFBooleanRef boolean;
		CFDictionaryRef info = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketFamilyTypeProtocol);
		
		/* If there was a dictionary for the CFSocketSignature-type values, get those values. */
		if (info) {
			
			if (CFDictionaryContainsValue(info, _kCFStreamSocketFamily)) {
				const void* tmp = CFDictionaryGetValue(info, _kCFStreamSocketFamily);
				protocolFamily = (SInt32)tmp;
			}
			
			if (CFDictionaryContainsValue(info, _kCFStreamSocketType)) {
				const void* tmp = CFDictionaryGetValue(info, _kCFStreamSocketType);
				socketType = (SInt32)tmp;
			}
			
			if (CFDictionaryContainsValue(info, _kCFStreamSocketProtocol)) {
				const void* tmp = CFDictionaryGetValue(info, _kCFStreamSocketProtocol);
				protocol = (SInt32)tmp;
			}
		}
		
		/*
		 ** Set the protocol family based upon the address family.  Do it after
		 ** setting from the signature values in order to guarantee things like
		 ** fallover from IPv4 to IPv6 succeed correctly.
		 */
		if (address)
			protocolFamily = ((struct sockaddr*)CFDataGetBytePtr(address))->sa_family;
		
		/* Attempt to create the socket */
		ctxt->_socket = CFSocketCreate(CFGetAllocator(ctxt->_properties),
									   protocolFamily,
									   socketType,
									   protocol,
									   kSocketEvents,
									   (CFSocketCallBack)_SocketCallBack, 
									   &c);
		
		if (!ctxt->_socket) {
			
			/*
			** Try to pull any error they may have just occurred.  If none,
			** assume an out of memory occurred.
			*/
			if (!_LastError(&ctxt->_error)) {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			}
			
			break;
		}
		
		/* Get the native socket for setting options. */
		s = CFSocketGetNative(ctxt->_socket);
		
		/* See if the client wishes to be informed of the new socket.  Call back if needed. */
		callback = (CFArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamSocketCreatedCallBack);
		if (callback)
			((_CFSocketStreamSocketCreatedCallBack)CFArrayGetValueAtIndex(callback, 0))(s, (void*)CFArrayGetValueAtIndex(callback, 1));
		
		/* Find out if the ttl is supposed to be set special so as to prevent traffic beyond the subnet. */
		boolean = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamSocketIChatWantsSubNet);
		if (boolean == kCFBooleanTrue) {
            int ttl = 255;
            setsockopt(s, IPPROTO_IP, IP_TTL, (void*)&ttl, sizeof(ttl));
		}
		
#if !defined(__WIN32)
        /* Turn off SIGPIPE on the socket (SIGPIPE doesn't exist on WIN32) */
        setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void*)&yes, sizeof(yes));
#endif
		
        /* Place the socket in nonblocking mode. */
        ioctl(s, FIONBIO, (void*)&yes);
		
		/* Get the current socket flags and turn off the auto re-enable for reads and writes. */
		flags = CFSocketGetSocketFlags(ctxt->_socket) &
			~kCFSocketAutomaticallyReenableReadCallBack &
			~kCFSocketAutomaticallyReenableWriteCallBack;
		
		/* Find out if CFSocket should close the socket on invalidation. */
		boolean = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyShouldCloseNativeSocket);
		
		/* Adjust the flags on the setting.  No value is the default which means to close. */
		if (!boolean || (boolean != kCFBooleanFalse))
			flags |= kCFSocketCloseOnInvalidate;
		else
			flags &= ~kCFSocketCloseOnInvalidate;
		
		/* Set up the correct flags and enable the callbacks. */
		CFSocketSetSocketFlags(ctxt->_socket, flags);
		//CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack);
		
		return TRUE;
		
	} while (0);
	
	return FALSE;
}


/* static */ Boolean
_SocketStreamConnect_NoLock(_CFSocketStreamContext* ctxt, CFDataRef address) {
	
	int i;
	Boolean result = FALSE;
	CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
	
	/* Now schedule the socket on all loops and modes */
	for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
		_CFTypeScheduleOnMultipleRunLoops(ctxt->_socket, loops[i]);
		
	/* Start the connect */
	if ((result = (CFSocketConnectToAddress(ctxt->_socket, address, -1.0) == kCFSocketSuccess))) {
		
		memset(&ctxt->_error, 0, sizeof(ctxt->_error));
		
		/* Succeeded so make sure the socket is in the list of schedulables for future. */
		_SchedulablesAdd(ctxt->_schedulables, ctxt->_socket);
	}
	
	else {
		
		/* Grab the error that occurred.  If no error, make one up. */
		if (!_LastError(&ctxt->_error)) {
			ctxt->_error.error = EINVAL;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
		}
		
		/* Remove the socket from all the schedules. */
		for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
			_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_socket, loops[i]);
		
		/* Invalidate the socket; never to be used again. */
		_CFTypeInvalidate(ctxt->_socket);
		
		/* Release and forget the socket */
		CFRelease(ctxt->_socket);
		ctxt->_socket = NULL;
	}
	
	return result;
}


/* static */ Boolean
_SocketStreamAttemptNextConnection_NoLock(_CFSocketStreamContext* ctxt) {
	
	do {
		/* Attempt to get the primary host for connecting */
		CFTypeRef lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHostForOpen);
		SInt32* attempt = NULL;

		/* If there was a host, there is more work to do */
		if (lookup) {

			CFIndex count;
			CFArrayRef list = NULL;
			CFDataRef address = NULL;
			CFNumberRef port = _CFNumberCopyPortForOpen(ctxt->_properties);
			CFMutableDataRef a = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketAddressAttempt);
			
			/* If there is an address attempt, point to the counter. */
			if (a)
				attempt = (SInt32*)CFDataGetMutableBytePtr(a);
			
			/* This is the first attempt so create and add the counter. */
			else {
				
				SInt32 i = 0;
				
				/* Create the counter. */
				a = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), sizeof(i));
				
				/* If it fails, set out of memory and bail. */
				if (!a) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					
					/* Not needed anymore. */
					if (port) CFRelease(port);

					break;
				}
				
				/* Add the attempt counter to the properties for later */
				CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySocketAddressAttempt, a);
				CFRelease(a);
				
				/* Point the attempt at the counter */
				attempt = (SInt32*)CFDataGetMutableBytePtr(a);
				
				/* Start counting at zero. */
				*attempt = 0;
			}
			
			/* Get the address list from the lookup. */
			if (CFGetTypeID(lookup) == CFHostGetTypeID())
				list = CFHostGetAddressing((CFHostRef)lookup, NULL);
			else
				list = CFNetServiceGetAddressing((CFNetServiceRef)lookup);
			
			/* If there were no addresses, return an error. */
			if (!list || (*attempt >= (count = CFArrayGetCount(list)))) {
			
				if (!ctxt->_error.error) {
					ctxt->_error.error = EAI_NODATA;
					ctxt->_error.domain = kCFStreamErrorDomainNetDB;
				}
				
				/* Not needed anymore. */
				if (port) CFRelease(port);
				
				break;
			}
			
			/* Go through the list until a usable address is found */
			do {
				/* Create the address for connecting. */
				address = _CFDataCopyAddressByInjectingPort((CFDataRef)CFArrayGetValueAtIndex(list, *attempt), port);
				
				/* The next attempt will be the next address in the list. */
				*attempt = *attempt + 1;
				
				/* Only try to connect if there is an address */
				if (address) {
					
					/* If there was a socket, only need to connect it. */
					if (ctxt->_socket) {
						
						/*
						 ** If a socket was created previously, there is only one attempt
						 ** since the required socket type and protocol aren't known.
						 */
						*attempt = count;
						
						/* Start the connection. */
						_SocketStreamConnect_NoLock(ctxt, address);			
					}
					
					/* Try to create and start connecting to the address */
					else if (_SocketStreamCreateSocket_NoLock(ctxt, address))
						_SocketStreamConnect_NoLock(ctxt, address);
					
					/* No longer need the address */
					CFRelease(address);
					
					/* If succeeded in starting connect, don't continue anymore. */
					if (!ctxt->_error.error) {
						
						/* Not needed anymore. */
						if (port) CFRelease(port);

						return TRUE;				/* NOTE the early return here. */
					}
				}
			
			/* Continue through the list until all are exhausted. */
			} while (*attempt < count);
			
			/* Not needed anymore. */
			if (port) CFRelease(port);
			
			/*
			 ** It's an error to get to this point.  It means that none
			 ** of the addresses were suitable or worked.
			 */

			if (!ctxt->_error.error) {
				ctxt->_error.error = EINVAL;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			}
				
			break;
		}
		
		/* If there is no lookup and no socket, something is bad. */
		else {
			
			int i;
			int yes = 1;
			CFOptionFlags flags;
			CFBooleanRef boolean;
			CFSocketNativeHandle s;		
			CFSocketContext c = {0, ctxt, NULL, NULL, NULL};
			CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
			
			if (!ctxt->_socket) {
				
				/* Try to get the native socket for creation. */
				CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketNativeHandle);
				
				if (!wrapper) {
					ctxt->_error.error = EINVAL;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Create the CFSocket for riding. */
				ctxt->_socket = CFSocketCreateWithNative(CFGetAllocator(ctxt->_properties),
														 *((CFSocketNativeHandle*)CFDataGetBytePtr(wrapper)),
														 kSocketEvents,
														 (CFSocketCallBack)_SocketCallBack,
														 &c);
				
				if (!ctxt->_socket) {
					
					/*
					 ** Try to pull any error that may have just occurred.  If none,
					 ** assume an out of memory occurred.
					 */
					if (!_LastError(&ctxt->_error)) {
						ctxt->_error.error = ENOMEM;
						ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					}
					
					break;
				}
				
				/* Remove the cached value so it's only created when the client asks for it. */
				CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySocketNativeHandle);
			}
			
			/*
			** No host lookup and a socket means that the streams were
			** created with a connected socket already.
			*/

			__CFBitSet(ctxt->_flags, kFlagBitOpenComplete);
			__CFBitClear(ctxt->_flags, kFlagBitPollOpen);

			/* Get the native socket for setting options. */
			s = CFSocketGetNative(ctxt->_socket);
			
#if !defined(__WIN32)
			/* Turn off SIGPIPE on the socket (SIGPIPE doesn't exist on WIN32) */
			setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void*)&yes, sizeof(yes));
#endif
			
			/* Place the socket in nonblocking mode. */
			ioctl(s, FIONBIO, (void*)&yes);
			
			/* Get the current socket flags and turn off the auto re-enable for reads and writes. */
			flags = CFSocketGetSocketFlags(ctxt->_socket) &
				~kCFSocketAutomaticallyReenableReadCallBack &
				~kCFSocketAutomaticallyReenableWriteCallBack;
			
			/* Find out if CFSocket should close the socket on invalidation. */
			boolean = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyShouldCloseNativeSocket);
			
			/* Adjust the flags on the setting.  No value is the default which means to close. */
			if (!boolean || (boolean != kCFBooleanFalse))
				flags |= kCFSocketCloseOnInvalidate;
			else
				flags &= ~kCFSocketCloseOnInvalidate;
			
			/* Set up the correct flags and enable the callbacks. */
			CFSocketSetSocketFlags(ctxt->_socket, flags);
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack);
			
			/* Now schedule the socket on all loops and modes */
			for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
				_CFTypeScheduleOnMultipleRunLoops(ctxt->_socket, loops[i]);
			
			/* Succeeded so make sure the socket is in the list of schedulables for future. */
			_SchedulablesAdd(ctxt->_schedulables, ctxt->_socket);
		}
		
		return TRUE;
		
	} while (0);
	
	return FALSE;
}


/* static */ Boolean
_SocketStreamCan(_CFSocketStreamContext* ctxt, CFTypeRef stream, int test, CFStringRef mode, CFStreamError* error) {
	
	Boolean result = TRUE;
	Boolean isRead;
	
	/* No error to  start. */
	memset(error, 0, sizeof(error[0]));
	
	/* Lock down the context */
	__CFSpinLock(&ctxt->_lock);
	
	isRead = CFReadStreamGetTypeID() == CFGetTypeID(stream);
	
	result = __CFBitIsSet(ctxt->_flags, test);
	
	/* If not already been signalled, need to find out. */
	if (!ctxt->_error.error && !result) {
		
		CFMutableArrayRef loops = isRead ? ctxt->_readloops : ctxt->_writeloops;
		
		if (!__CFBitIsSet(ctxt->_flags, test + kFlagBitPollOpen) &&
			((CFArrayGetCount(ctxt->_sharedloops) + CFArrayGetCount(loops)) > 2))
		{
			__CFBitSet(ctxt->_flags, test + kFlagBitPollOpen);
		}
		
		else {

			CFTypeRef loopAndMode[2] = {CFRunLoopGetCurrent(), mode};

			/* Add the current loop and the private mode to the list */
			_SchedulesAddRunLoopAndMode(loops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
			
			if (ctxt->_socket &&
				(CFArrayGetCount(ctxt->_schedulables) == 1) &&
				(ctxt->_socket == CFArrayGetValueAtIndex(ctxt->_schedulables, 0)))
			{
				CFRunLoopSourceRef src = CFSocketCreateRunLoopSource(CFGetAllocator(ctxt->_schedulables), ctxt->_socket, 0);
				if (src) {
					CFRunLoopAddSource((CFRunLoopRef)loopAndMode[0], src, (CFStringRef)loopAndMode[1]);
					CFRelease(src);
				}
			}
			
			else {
				/* Make sure to schedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesScheduleApplierFunction,
									 loopAndMode);
			}
			
			/* Unlock the context to allow things to fire */
			__CFSpinUnlock(&ctxt->_lock);
			
			/* Run the run loop for a poll (0.0) */
			CFRunLoopRunInMode(mode, 0.0, FALSE);
			
			/* Lock the context back up. */
			__CFSpinLock(&ctxt->_lock);
			
			if (ctxt->_socket &&
				(CFArrayGetCount(ctxt->_schedulables) == 1) &&
				(ctxt->_socket == CFArrayGetValueAtIndex(ctxt->_schedulables, 0)))
			{
				CFRunLoopSourceRef src = CFSocketCreateRunLoopSource(CFGetAllocator(ctxt->_schedulables), ctxt->_socket, 0);
				if (src) {
					CFRunLoopRemoveSource((CFRunLoopRef)loopAndMode[0], src, (CFStringRef)loopAndMode[1]);
					CFRelease(src);
				}
			}

			else {
				/* Make sure to unschedule all the schedulables on this loop and mode. */
				CFArrayApplyFunction(ctxt->_schedulables,
									 CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
									 (CFArrayApplierFunction)_SchedulablesUnscheduleApplierFunction,
									 loopAndMode);
			}
			
			/* Remove this loop and private mode from the list. */
			_SchedulesRemoveRunLoopAndMode(loops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
			
			result = __CFBitIsSet(ctxt->_flags, test);
		}
	}
	
	/* If there was an error, make sure to signal it. */
	if (ctxt->_error.error) {
		
		/* Attempt to get the count of buffered bytes. */
		CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
		
		/* Copy the error. */
		memmove(error, &ctxt->_error, sizeof(error[0]));
		
		/* It's set now. */
		__CFBitSet(ctxt->_flags, test);
		
		result = TRUE;
		
		/*
		** 2998408 Force async callback for errors so there is no worry about the
		** context going bad underneath callers of this function.
		*/
		
		/*
		** 3863115 If there is a client stream and it's been opened, signal the
		** error, but only if there are no bytes in the buffer.
		*/
		if ((!c || !*((CFIndex*)CFDataGetBytePtr(c))) &&
			(ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened)))
		{
			_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventErrorOccurred, error);
		}
		
		/* If there is a client stream and it's been opened, signal the error. */
		if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened))
			_CFWriteStreamSignalEventDelayed(ctxt->_clientWriteStream, kCFStreamEventErrorOccurred, error);
	}
	
	/* Unlock */
	__CFSpinUnlock(&ctxt->_lock);
	
	return result;
}


/* static */ void
_SocketStreamAddReachability_NoLock(_CFSocketStreamContext* ctxt) {
	
	SCNetworkReachabilityRef reachability = (SCNetworkReachabilityRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyNetworkReachability);
	
	/* There has to be an open socket and no reachibility item already. */
	if (ctxt->_socket && __CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete) && !reachability) {

		/* Copy the addresses for the pipe. */
		CFDataRef localAddr = CFSocketCopyAddress(ctxt->_socket);
		CFDataRef peerAddr = CFSocketCopyPeerAddress(ctxt->_socket);
		
		if (localAddr && peerAddr) {
			
			/* Create the reachability object to watch the route. */
			SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddressPair(CFGetAllocator(ctxt->_properties),
																							   (const struct sockaddr*)CFDataGetBytePtr(localAddr),
																							   (const struct sockaddr*)CFDataGetBytePtr(peerAddr));
																					
			if (reachability) {
				int i;
				SCNetworkReachabilityContext c = {0, ctxt, NULL, NULL, NULL};
				CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
				
				/* Add it to the properties. */
				CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyNetworkReachability, reachability);
				
				/* Set the callback */
				SCNetworkReachabilitySetCallback(reachability, (SCNetworkReachabilityCallBack)_ReachabilityCallBack, &c);
				
				/* Schedule it on all the loops and modes. */
				for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
					_CFTypeScheduleOnMultipleRunLoops(reachability, loops[i]);
				
				/* Add it to the schedulables. */
				_SchedulablesAdd(ctxt->_schedulables, reachability);
				
				/* Schedulables and properties hold it now. */
				CFRelease(reachability);
			}
		}
		
		/* Don't need these anymore. */
		if (localAddr) CFRelease(localAddr);
		if (peerAddr) CFRelease(peerAddr);
	}
}


/* static */ void
_SocketStreamRemoveReachability_NoLock(_CFSocketStreamContext* ctxt) {
	
	/* Find out if there is a reachability already. */
	SCNetworkReachabilityRef reachability = (SCNetworkReachabilityRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyNetworkReachability);

	if (reachability) {
	
		int i;
		CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
		
		/* Invalidate the reachability; never to be used again. */
		_CFTypeInvalidate(reachability);
		
		/* Unschedule it from all the loops and modes. */
		for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
			_CFTypeUnscheduleFromMultipleRunLoops(reachability, loops[i]);
		
		/* Remove the socket from the schedulables. */
		_SchedulablesRemove(ctxt->_schedulables, reachability);
		
		/* Remove it from the properties */
		CFDictionaryRemoveValue(ctxt->_properties, reachability);
	}
}


/* static */ CFIndex
_SocketStreamBufferedRead_NoLock(_CFSocketStreamContext* ctxt, UInt8* buffer, CFIndex length) {
	
	CFIndex result = 0;
	
	/* Different bits required for "buffered reading." */
	CFNumberRef s = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferSize);
	CFMutableDataRef b = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBuffer);
	CFMutableDataRef c = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
	
	/* All have to be availbe to read. */
	if (b && c && s) {
		
		CFIndex* i = (CFIndex*)CFDataGetMutableBytePtr(c);
		UInt8* ptr = (UInt8*)CFDataGetMutableBytePtr(b);
		CFIndex max;
		
		CFNumberGetValue(s, kCFNumberCFIndexType, &max);
		
		/* Either read all the bytes or just what the client asked. */
		result = (*i < length) ? *i : length;
		*i = *i - result;
		
		/* Copy the bytes into the client buffer */
		memmove(buffer, ptr, result);
		
		/* Move down the bytes in the local buffer. */
		memmove(ptr, ptr + result, *i);
		
		/* Zero the bytes at the end of the local buffer */
		memset(ptr + *i, 0, max - *i);
		
		/* If the local buffer is empty, pump SSL along. */
		if (__CFBitIsSet(ctxt->_flags, kFlagBitUseSSL) && (*i == 0)) {
			_SocketStreamSecurityBufferedRead_NoLock(ctxt);
		}
	}
	
	/* If no bytes read and the pipe isn't closed, constitute an EAGAIN */
	if (!result) {
		if (!ctxt->_error.error && !__CFBitIsSet(ctxt->_flags, kFlagBitClosed)) {
			ctxt->_error.error = EAGAIN;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
		}
	}
	else if (__CFBitIsSet(ctxt->_flags, kFlagBitRecvdRead)) {
		__CFBitClear(ctxt->_flags, kFlagBitRecvdRead);
		if (ctxt->_socket)
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
	}
	
	return result;
}


/* static */ void
_SocketStreamBufferedSocketRead_NoLock(_CFSocketStreamContext* ctxt) {
	
	CFIndex* i;
	CFIndex s = kRecvBufferSize;
	
	/* Get the bits required in order to work with the buffer. */
	CFNumberRef size = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferSize);
	CFMutableDataRef buffer = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBuffer);
	CFMutableDataRef count = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
	
	/* No buffer assumes all are missing. */
	if (!buffer) {
		
		CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
		
		/* If no size, assume a default.  Can be overridden by properties. */
		if (!size)
			size = CFNumberCreate(alloc, kCFNumberCFIndexType, &s);
		else
			CFNumberGetValue(size, kCFNumberCFIndexType, &s);

		/* Create the backing for the buffer and the counter. */
		if (size) {
			buffer = CFDataCreateMutable(alloc, s);
			count = CFDataCreateMutable(alloc, sizeof(CFIndex));
		}
		
		/* If anything failed, set out of memory and bail. */
		if (!buffer || !count || !size) {
			
			if (buffer) CFRelease(buffer);
			if (count) CFRelease(count);
			if (size) CFRelease(size);
			
			ctxt->_error.error = ENOMEM;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			
			return;								/* NOTE the eary return. */
		}
		
		/* Save the buffer information. */
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferSize, size);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBuffer, buffer);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount, count);
		
		CFRelease(size);
		CFRelease(buffer);
		CFRelease(count);
		
		/* Start with a zero byte count. */
		*((CFIndex*)CFDataGetMutableBytePtr(count)) = 0;
	}
	
	/* Get the count and size of the buffer, respectively. */
	i = (CFIndex*)CFDataGetMutableBytePtr(count);
	CFNumberGetValue(size, kCFNumberCFIndexType, &s);
	
	/* Only read if there is room in the buffer. */
	if (*i < s) {
		
		UInt8* ptr = (UInt8*)CFDataGetMutableBytePtr(buffer);		
		CFIndex bytesRead = _CFSocketRecv(ctxt->_socket, ptr + *i, s - *i, &ctxt->_error);

		__CFBitClear(ctxt->_flags, kFlagBitRecvdRead);

		/* If did read bytes, increase the count. */
		if (bytesRead > 0) {
			*i = *i + bytesRead;
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
			__CFBitSet(ctxt->_flags, kFlagBitCanRead);
			__CFBitClear(ctxt->_flags, kFlagBitPollRead);
		}
		
		else if (bytesRead == 0) {
			__CFBitSet(ctxt->_flags, kFlagBitClosed);
			__CFBitSet(ctxt->_flags, kFlagBitCanRead);
			__CFBitClear(ctxt->_flags, kFlagBitPollRead);
		}
	}
	else
		__CFBitSet(ctxt->_flags, kFlagBitRecvdRead);
}


/* static */ CFComparisonResult
_OrderHandshakes(_CFSocketStreamPerformHandshakeCallBack fn1, _CFSocketStreamPerformHandshakeCallBack fn2, void* context) {
	
	if (!fn1) return kCFCompareGreaterThan;
	if (!fn2) return kCFCompareLessThan;
	
	/*
	** Order of handshakes in increasing priority:
	**
	** 1) SOCKSv5 \__ Conceivably should not be set at the same time.
	** 2) SOCKSv4 /
	** 3) SOCKSv5 user/pass negotiation
	** 3) SOCKSv5 postamble
	** 4) CONNECT halt <- Used to halt the stream for another CONNECT
	** 5) CONNECT
	** 6) SSL send <- could be required for #4
	** 7) SSL
	*/
	
	if (*fn1 == _PerformSOCKSv5Handshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSOCKSv5Handshake_NoLock) return kCFCompareGreaterThan;
	
	if (*fn1 == _PerformSOCKSv5UserPassHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSOCKSv5UserPassHandshake_NoLock) return kCFCompareGreaterThan;

	if (*fn1 == _PerformSOCKSv5PostambleHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSOCKSv5PostambleHandshake_NoLock) return kCFCompareGreaterThan;

	if (*fn1 == _PerformSOCKSv4Handshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSOCKSv4Handshake_NoLock) return kCFCompareGreaterThan;
	
	if (*fn1 == _PerformCONNECTHaltHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformCONNECTHaltHandshake_NoLock) return kCFCompareGreaterThan;
	
	if (*fn1 == _PerformCONNECTHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformCONNECTHandshake_NoLock) return kCFCompareGreaterThan;
	
	if (*fn1 == _PerformSecuritySendHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSecuritySendHandshake_NoLock) return kCFCompareGreaterThan;
	
	if (*fn1 == _PerformSecurityHandshake_NoLock) return kCFCompareLessThan;
	if (*fn2 == _PerformSecurityHandshake_NoLock) return kCFCompareGreaterThan;
	
	return kCFCompareEqualTo;
}


/* static */ Boolean
_SocketStreamAddHandshake_NoLock(_CFSocketStreamContext* ctxt, _CFSocketStreamPerformHandshakeCallBack fn) {
	
	CFIndex i;
	
	/* Get the existing list of handshakes. */
	CFMutableArrayRef handshakes = (CFMutableArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
	
	/* If there is no list, need to create one. */
	if (!handshakes) {
		
		CFArrayCallBacks cb = {0, NULL, NULL, NULL, NULL};
		
		/* Create the list of handshakes. */
		handshakes = CFArrayCreateMutable(CFGetAllocator(ctxt->_properties), 0, &cb);
		
		if (!handshakes)
			return FALSE;
		
		/* Add the list to the properties for later work. */
		CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyHandshakes, handshakes);
		__CFBitSet(ctxt->_flags, kFlagBitHasHandshakes);
		
		CFRelease(handshakes);
	}
	
	/* Find out if the handshake is in the list already. */
	i = CFArrayGetFirstIndexOfValue(handshakes, CFRangeMake(0, CFArrayGetCount(handshakes)), fn);
	
	/* Need to add it? */
	if (i == kCFNotFound) {
		
		CFRange r;
		
		/* Add the new handshake to the list. */
		CFArrayAppendValue(handshakes, fn);
		
		r = CFRangeMake(0, CFArrayGetCount(handshakes));
		
		/* Make sure to order the list of handshakes correctly */
		CFArraySortValues(handshakes, r, (CFComparatorFunction)_OrderHandshakes, NULL);
		
		/* Update the location. */
		i = CFArrayGetFirstIndexOfValue(handshakes, r, fn);
	}
	
	if (!i) {
		
		__CFBitClear(ctxt->_flags, kFlagBitCanRead);
		__CFBitClear(ctxt->_flags, kFlagBitCanWrite);
		
		__CFBitClear(ctxt->_flags, kFlagBitRecvdRead);
		
		/* Make sure all callbacks are set up for handshaking. */
		if (ctxt->_socket && __CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete))
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack);
	}
		
	return TRUE;
}


/* static */ void
_SocketStreamRemoveHandshake_NoLock(_CFSocketStreamContext* ctxt, _CFSocketStreamPerformHandshakeCallBack fn) {

	/* Get the existing list of handshakes. */
	CFMutableArrayRef handshakes = (CFMutableArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);

	if (handshakes) {
		
		/* Find out if the handshake is in the list. */
		CFIndex i = CFArrayGetFirstIndexOfValue(handshakes, CFRangeMake(0, CFArrayGetCount(handshakes)), fn);
		
		/* If it exists, need to remove it. */
		if (i != kCFNotFound)
			CFArrayRemoveValueAtIndex(handshakes, i);
		
		if (!CFArrayGetCount(handshakes)) {
			CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
			__CFBitClear(ctxt->_flags, kFlagBitHasHandshakes);
		}
	}
	
	/* Need to reset the flags as a result of a handshake removal. */
	__CFBitClear(ctxt->_flags, kFlagBitCanRead);
	__CFBitClear(ctxt->_flags, kFlagBitCanWrite);
	
	__CFBitClear(ctxt->_flags, kFlagBitRecvdRead);
			
	/* Make sure all callbacks are reset if open. */
	if (ctxt->_socket && __CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete)) {
		
		if (!__CFBitIsSet(ctxt->_flags, kFlagBitHasHandshakes) && __CFBitIsSet(ctxt->_flags, kFlagBitIsBuffered)) {

			CFStreamError error = ctxt->_error;
			
			/* Similar to the end of _SocketStreamRead. */
			CFDataRef c = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
			Boolean buffered = (c && *((CFIndex*)CFDataGetBytePtr(c)));
			
			if (buffered)
				memset(&error, 0, sizeof(error));
			
			/* Need to check for buffered bytes or EOF. */
			if (__CFBitIsSet(ctxt->_flags, kFlagBitClosed) || buffered) {
				
				if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened))
					_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventHasBytesAvailable, &error);
			}
			
			/* If none there, check to see if there are encrypted bytes that are buffered. */
			else if (__CFBitIsSet(ctxt->_flags, kFlagBitUseSSL)) {
				
				_SocketStreamSecurityBufferedRead_NoLock(ctxt);
				
				if (__CFBitIsSet(ctxt->_flags, kFlagBitCanRead) || __CFBitIsSet(ctxt->_flags, kFlagBitClosed)) {
					
					if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened)) {
						
						if (c && *((CFIndex*)CFDataGetBytePtr(c)))
							memset(&error, 0, sizeof(error));
						
						_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventHasBytesAvailable, &error);
					}
				}
			}
		}
		
		CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack | kCFSocketWriteCallBack);
	}
}
	

/* static */ void
_SocketStreamAttemptAutoVPN_NoLock(_CFSocketStreamContext* ctxt, CFStringRef name) {
	
	if (!__CFBitIsSet(ctxt->_flags, kFlagBitTriedVPN)) {
		
		CFTypeRef values[2] = {name, NULL};
		const CFStringRef keys[2] = {_kCFStreamAutoHostName, _kCFStreamPropertyAutoConnectPriority};
		CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
		CFDictionaryRef options;
		
		/* Grab the intended value.  If none, give if "default." */
		values[1] = (CFStringRef)CFDictionaryGetValue(ctxt->_properties, keys[1]);
		if (!values[1])
			values[1] = _kCFStreamAutoVPNPriorityDefault;
		
		/* Create the dictionary of options for SC. */
		options = CFDictionaryCreate(alloc,
									 (const void **)keys,
									 (const void **)values,
									 sizeof(values) / sizeof(values[0]),
									 &kCFTypeDictionaryKeyCallBacks,
									 &kCFTypeDictionaryValueCallBacks);
		
		/* Mark the attempt, so another is not made. */
		__CFBitSet(ctxt->_flags, kFlagBitTriedVPN);
		
		/* No options = no memory. */
		if (!options) {
			ctxt->_error.error = ENOMEM;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
		}
		
		else {
			CFStringRef service_id = NULL;
			CFDictionaryRef user_options = NULL;

			/* Create the service id and settings. */
			if (SCNetworkConnectionCopyUserPreferences(options, &service_id, &user_options)) {
				
				SCNetworkConnectionContext c = {0, ctxt, NULL, NULL, NULL};
				
				/* Create the connection for auto connect. */
				SCNetworkConnectionRef conn = SCNetworkConnectionCreateWithServiceID(alloc,
																					 service_id,
																					 (SCNetworkConnectionCallBack)_NetworkConnectionCallBack,
																					 &c);
				
				/* Did it create? */
				if (conn) {

					int i;
					CFArrayRef loops[3] = {ctxt->_readloops, ctxt->_writeloops, ctxt->_sharedloops};
					
					/* Now schedule the connection on all loops and modes */
					for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
						_CFTypeScheduleOnMultipleRunLoops(conn, loops[i]);
					
					if (SCNetworkConnectionStart(conn, user_options, TRUE)) {
						
						memset(&ctxt->_error, 0, sizeof(ctxt->_error));
						
						/* Succeeded so make sure the connection is in the list of schedulables for future. */
						_SchedulablesAdd(ctxt->_schedulables, conn);
					}
					
					else {
						
						/* Remove the connection from all the schedules. */
						for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
							_CFTypeUnscheduleFromMultipleRunLoops(conn, loops[i]);
						
						/* Invalidate the connection; never to be used again. */
						_CFTypeInvalidate(conn);
					}
				}
				
				/* Clean up. */
				if (conn) CFRelease(conn);
			}
			
			/* Clean up. */
			CFRelease(options);
			if (service_id) CFRelease(service_id);
			if (user_options) CFRelease(user_options);
		}
	}
}


/* static */ void
_SocketStreamPerformCancel(void* info) {
	(void)info;  /* unused */
}


#if 0
#pragma mark *SOCKS Support
#endif

#define kSOCKSv4BufferMaximum	((CFIndex)(8L))
#define kSOCKSv5BufferMaximum	((CFIndex)(2L))

/* static */ void
_PerformSOCKSv5Handshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	do {
		/* Get the buffer of stuff to send */
		CFMutableDataRef to_send = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSSendBuffer);
		CFMutableDataRef to_recv = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSRecvBuffer);
		
		if (!to_recv) {
			
			CFStreamError error = {0, 0};
			CFIndex length, sent;
				
			if (!to_send) {
				
				UInt8* ptr;
				
				/* Get the user/pass to determine how many methods are supported. */
				CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
				CFStringRef user = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSUser);
				CFStringRef pass = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSPassword);
				
				/* Create the 4 byte buffer for the intial connect. */
				to_send = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 4);
				
				/* Couldn't create so error out on no memory. */
				if (!to_send) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer, to_send);
				CFRelease(to_send);
								
				/* Get the local pointer to set the values. */
				ptr = CFDataGetMutableBytePtr(to_send);
				CFDataSetLength(to_send, 4);
				
				/* By default, perform only 1 method (no authentication). */
				ptr[0] = 0x05; ptr[1] = 0x01;
				ptr[2] = 0x00; ptr[3] = 0x02;
		
				/* If there is a valid user and pass, indicate willing to do two methods. */
				if (user && CFStringGetLength(user) && pass && CFStringGetLength(pass))
					ptr[1] = 0x02;
				else
					CFDataSetLength(to_send, 3);
			}
			
			/* Try sending out the bytes. */
			length = CFDataGetLength(to_send);
			sent = _CFSocketSend(ctxt->_socket, CFDataGetBytePtr(to_send), length, &error);
			
			/* If sent everything, dump the buffer. */
			if (sent == length) {
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer);
				
				/* Create the buffer for receive. */
				to_recv = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 2);
				
				/* Fail so error on no memory. */
				if (!to_recv) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer, to_recv);
				CFRelease(to_recv);
			}
			
			/* If couldn't send everything, trim the buffer. */
			else if (sent > 0) {
				
				UInt8* ptr = CFDataGetMutableBytePtr(to_send);
				
				/* New length */
				length -= sent;
				
				/* Move the bytes down in the buffer. */
				memmove(ptr, ptr + sent, length);
				
				/* Trim it. */
				CFDataSetLength(to_send, length);
				
				/* Re-enable so the rest can be written later. */
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
			}
			
			/* If got an error other than EAGAIN, set the error in the context. */
			else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
				memmove(&ctxt->_error, &error, sizeof(error));
		}
		
		else {
			
			UInt8* ptr = CFDataGetMutableBytePtr(to_recv);
			CFIndex length = CFDataGetLength(to_recv);
			
			if (length != kSOCKSv5BufferMaximum) {
				
				CFStreamError error = {0, 0};
				CFIndex recvd = _CFSocketRecv(ctxt->_socket, ptr + length, kSOCKSv5BufferMaximum - length, &error);
				
				/* If read 0 bytes, this is an early close from the other side. */
				if (recvd == 0) {
					
					/* Mark as not connected. */
					ctxt->_error.error = ENOTCONN;
					ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;
				}
				
				/* Successfully read? */
				else if (recvd > 0) {
					
					UInt8 tmp[kSOCKSv5BufferMaximum];
					
					/* Set the length of the buffer. */
					length += recvd;
					
					/* CF is so kind as to zero the bytes on SetLength, even though it's a fixed capacity. */
					memmove(tmp, ptr, length);
					
					/* Set the length of the buffer. */
					CFDataSetLength(to_recv, length);
					
					/* Put the bytes back. */
					memmove(ptr, tmp, length);
					
					/* Re-enable after performing a successful read. */
					CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
				}
				
				/* If got an error other than EAGAIN, set the error in the context. */
				else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
					memmove(&ctxt->_error, &error, sizeof(error));
			}
			
			/* Is there enough now? */
			if (length == kSOCKSv5BufferMaximum) {
				
				switch (ptr[1]) {
					
					case 0x00:
						/* Don't need to do anything for "No Authentication Required." */
						CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer);
						_SocketStreamAddHandshake_NoLock(ctxt, _PerformSOCKSv5PostambleHandshake_NoLock);
						_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
						break;
						
						/* **FIXME** Add GSS API support (0x01) */
						
					case 0x02:
					{
						CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
						CFStringRef user = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSUser);
						CFStringRef pass = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSPassword);
						
						CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer);
						
						if (user && pass) {
							_SocketStreamAddHandshake_NoLock(ctxt, _PerformSOCKSv5UserPassHandshake_NoLock);
							_SocketStreamAddHandshake_NoLock(ctxt, _PerformSOCKSv5PostambleHandshake_NoLock);
							_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
						}
						
						else {
							ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
							ctxt->_error.error = ((kCFStreamErrorSOCKS5SubDomainMethod << 16) | (ptr[1] & 0x000000FF));
						}
						
						break;
					}
						
						/* **FIXME** Add CHAP support (0x03) */
						
					default:
						ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
						ctxt->_error.error = ((kCFStreamErrorSOCKS5SubDomainMethod << 16) | (ptr[1] & 0x000000FF));
						break;
				}
			}
		}
	} while (0);
	
	/* If there was an error remove the handshake. */
	if (ctxt->_error.error)
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
}


/* static */ void
_PerformSOCKSv5PostambleHandshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	do {
		/* Get the buffer of stuff to send */
		CFMutableDataRef to_send = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSSendBuffer);
		CFMutableDataRef to_recv = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSRecvBuffer);
		
		if (!to_recv) {
			
			CFStreamError error = {0, 0};
			CFIndex length, sent;
			
			if (!to_send) {
				
				UInt8* ptr;
				SInt32 value = 0;
				unsigned short prt;

				/* Try to get the name for sending in the CONNECT request. */
				CFHostRef host = (CFHostRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
				CFNumberRef port = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketRemotePort);
				CFArrayRef list = CFHostGetNames(host, NULL);
				CFStringRef name = (list && CFArrayGetCount(list)) ? (CFStringRef)CFArrayGetValueAtIndex(list, 0) : NULL;
				
				if (name)
					CFRetain(name);
				else {
					
					/* Is there one good address to create a name? */
					list = CFHostGetAddressing(host, NULL);
					if (list && CFArrayGetCount(list)) {
						name = _CFNetworkCFStringCreateWithCFDataAddress(CFGetAllocator(list),
																		 (CFDataRef)CFArrayGetValueAtIndex(list, 0));
					}
					
					/* Couldn't create so error out on no memory. */
					if (!name) {
						ctxt->_error.error = ENOMEM;
						ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
						break;
					}
				}
				
				/* Create the buffer for the largest possible CONNECT request. */
				to_send = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 262);

				/* Couldn't create so error out on no memory. */
				if (!to_send) {
				  ctxt->_error.error = ENOMEM;
				  ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				  break;
				}
				
				/* Extend it all the way out for now; shorten later. */
				CFDataSetLength(to_send, 262);
				
				/* Get the local pointer to set the values. */
				ptr = CFDataGetMutableBytePtr(to_send);
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer, to_send);
				CFRelease(to_send);
				
				/* Place the name into the buffer. */
				CFStringGetPascalString(name, &(ptr[4]), 256, kCFStringEncodingUTF8);
				CFRelease(name);
				
				/* Place the header bytes. */
				ptr[0] = 0x05; ptr[1] = 0x01;
				ptr[2] = 0x00; ptr[3] = 0x03;
				
				/* Get the port value to lay down. */
				CFNumberGetValue(port, kCFNumberSInt32Type, &value);
				
				/* Lay down the port. */
				prt = htons(value & 0x0000FFFF);
				*((unsigned short*)(&ptr[ptr[4] + 5])) = prt;
				
				/* Trim down the buffer to the correct size. */
				CFDataSetLength(to_send, 7 + ptr[4]);
			}
			
			/* Try sending out the bytes. */
			length = CFDataGetLength(to_send);
			sent = _CFSocketSend(ctxt->_socket, CFDataGetBytePtr(to_send), length, &error);
			
			/* If sent everything, dump the buffer. */
			if (sent == length) {
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer);
				
				/* Create the buffer for receive. */
				to_recv = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 262);
				
				/* Fail so error on no memory. */
				if (!to_recv) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer, to_recv);
				CFRelease(to_recv);
			}
			
			/* If couldn't send everything, trim the buffer. */
			else if (sent > 0) {
				
				UInt8* ptr = CFDataGetMutableBytePtr(to_send);
				
				/* New length */
				length -= sent;
				
				/* Move the bytes down in the buffer. */
				memmove(ptr, ptr + sent, length);
				
				/* Trim it. */
				CFDataSetLength(to_send, length);
				
				/* Re-enable so the rest can be written later. */
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
			}
			
			/* If got an error other than EAGAIN, set the error in the context. */
			else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
				memmove(&ctxt->_error, &error, sizeof(error));
		}
		
		else {
						
			CFStreamError error = {0, 0};
			UInt8* ptr = CFDataGetMutableBytePtr(to_recv);
			CFIndex length = CFDataGetLength(to_recv);
			CFIndex recvd = 0;
			
			/* Go for the initial return code. */
			if (length < 2)
				recvd = _CFSocketRecv(ctxt->_socket, ptr + length, 2 - length, &error);
					
			/* Add the read bytes if successful */
			length += (recvd > 0) ? recvd : 0;
			
			/* Continue on if things are good. */
			if (!error.error && (length >= 2)) {
				
				/* Make sure the header starts correctly.  Fail if not. */
				if ((ptr[0] != 5) || ptr[1]) {
					ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
					ctxt->_error.error = ((kCFStreamErrorSOCKS5SubDomainResponse << 16) | (((ptr[0] != 5) ? -1 : ptr[1]) & 0x000000FF));
					break;
				}
				
				else {
					
					/* Go for as many bytes as the smallest result packet. */
					if (length < 8)
						recvd = _CFSocketRecv(ctxt->_socket, ptr + length, 8 - length, &error);
					
					/* Add the read bytes if successful */
					length += (recvd > 0) ? recvd : 0;
					
					/* Can continue so long as result type and length byte are there. */
					if (!error.error && (length >= 5)) {
						
						CFIndex intended = 0;
						
						/* Check the address type. */
						switch (ptr[3]) {
							
							/* IPv4 type */
							case 0x01:
								/* Bail if 10 bytes have been read. */
								intended = 10;
								break;
								
								/* Domain name type */
							case 0x03:
								/* Bail if the 7 bytes plus domain name length have been read. */
								intended = 7 + ptr[4];
								break;
								
								/* IPv6 type */
							case 0x04:
								/* Bail if 22 bytes have been read. */
								intended = 22;
								break;
								
								/* Got crap data so bail.*/
							default:
								ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
								ctxt->_error.error = ((kCFStreamErrorSOCKS5SubDomainResponse << 16) | kCFStreamErrorSOCKS5BadResponseAddr);
								break;
						}
						
						/* Not an understood resut, so bail. */
						if (ctxt->_error.error)
							break;
						
						/* If haven't read all the result, continue reading more. */
						if (length < intended)
							recvd = _CFSocketRecv(ctxt->_socket, ptr + length, intended - length, &error);
						
						/* No error on the final read? */
						if (!error.error) {
							
							/* Add the read bytes. */
							length += recvd;
							
							/* If got them all, need to remove the handshake and let things fly. */
							if (length == intended) {

								/* There is no reason to have this. */
								CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer);

								/* Remove the handshake and get out. */
								_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5PostambleHandshake_NoLock);
								
								return;							/* NOTE the early return! */
							}
						}
					}
				}
			}
						
			/* If read 0 bytes, this is an early close from the other side. */
			if (recvd == 0) {
				
				/* Mark as not connected. */
				ctxt->_error.error = ENOTCONN;
				ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;
			}
			
			/* Successfully read? */
			else if (!error.error)
				break;
			
			/* Got a blocking error, so need to copy over all the bytes and buffer. */
			else if ((error.error == EAGAIN) || (error.domain == kCFStreamErrorDomainPOSIX)) {
				
				UInt8 tmp[262];
				
				/* Set the length of the buffer. */
				length += recvd;
				
				/* CF is so kind as to zero the bytes on SetLength, even though it's a fixed capacity. */
				memmove(tmp, ptr, length);
				
				/* Set the length of the buffer. */
				CFDataSetLength(to_recv, length);
				
				/* Put the bytes back. */
				memmove(ptr, tmp, length);
				
				/* Re-enable after performing a successful read. */
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
			}
			
			/* If got an error other than EAGAIN, set the error in the context. */
			else
				memmove(&ctxt->_error, &error, sizeof(error));
		}
	} while (0);
	
	/* If there was an error remove the handshake. */
	if (ctxt->_error.error)
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
}


/* static */ void
_PerformSOCKSv5UserPassHandshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	do {
		/* Get the buffer of stuff to send */
		CFMutableDataRef to_send = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSSendBuffer);
		CFMutableDataRef to_recv = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSRecvBuffer);
		
		if (!to_recv) {
			
			CFStreamError error = {0, 0};
			CFIndex length, sent;
			
			if (!to_send) {
				
				UInt8* ptr;
				
				/* Get the user/pass. */
				CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
				CFStringRef user = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSUser);
				CFStringRef pass = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertySOCKSPassword);
				
				/* Create the buffer for the maximum user/pass packet. */
				to_send = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 513);
				
				/* Couldn't create so error out on no memory. */
				if (!to_send) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer, to_send);
				CFRelease(to_send);
				
				/* Get the local pointer to set the values. */
				ptr = CFDataGetMutableBytePtr(to_send);
				CFDataSetLength(to_send, 513);

				/* Set version 1. */
				ptr[0] = 0x01;
				
				/* Place the user and pass into the buffer. */
				CFStringGetPascalString(user, &(ptr[1]), 256, kCFStringEncodingUTF8);
				CFStringGetPascalString(pass, &(ptr[2 + ptr[1]]), 256, kCFStringEncodingUTF8);
				
				/* Set the length. */
				CFDataSetLength(to_send, 3 + ptr[1] + ptr[2 + ptr[1]]);
			}
			
			/* Try sending out the bytes. */
			length = CFDataGetLength(to_send);
			sent = _CFSocketSend(ctxt->_socket, CFDataGetBytePtr(to_send), length, &error);
			
			/* If sent everything, dump the buffer. */
			if (sent == length) {
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer);
				
				/* Create the buffer for receive. */
				to_recv = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 2);
				
				/* Fail so error on no memory. */
				if (!to_recv) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					break;
				}
				
				/* Make sure to save the buffer for later. */
				CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer, to_recv);
				CFRelease(to_recv);
			}
			
			/* If couldn't send everything, trim the buffer. */
			else if (sent > 0) {
				
				UInt8* ptr = CFDataGetMutableBytePtr(to_send);
				
				/* New length */
				length -= sent;
				
				/* Move the bytes down in the buffer. */
				memmove(ptr, ptr + sent, length);
				
				/* Trim it. */
				CFDataSetLength(to_send, length);
				
				/* Re-enable so the rest can be written later. */
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
			}
			
			/* If got an error other than EAGAIN, set the error in the context. */
			else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
				memmove(&ctxt->_error, &error, sizeof(error));
		}
		
		else {
			
			UInt8* ptr = CFDataGetMutableBytePtr(to_recv);
			CFIndex length = CFDataGetLength(to_recv);
			
			if (length != kSOCKSv5BufferMaximum) {
				
				CFStreamError error = {0, 0};
				CFIndex recvd = _CFSocketRecv(ctxt->_socket, ptr + length, kSOCKSv5BufferMaximum - length, &error);
				
				/* If read 0 bytes, this is an early close from the other side. */
				if (recvd == 0) {
					
					/* Mark as not connected. */
					ctxt->_error.error = ENOTCONN;
					ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;
				}
				
				/* Successfully read? */
				else if (recvd > 0) {
					
					UInt8 tmp[kSOCKSv5BufferMaximum];
					
					/* Set the length of the buffer. */
					length += recvd;
					
					/* CF is so kind as to zero the bytes on SetLength, even though it's a fixed capacity. */
					memmove(tmp, ptr, length);
					
					/* Set the length of the buffer. */
					CFDataSetLength(to_recv, length);
					
					/* Put the bytes back. */
					memmove(ptr, tmp, length);
					
					/* Re-enable after performing a successful read. */
					CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
				}
				
				/* If got an error other than EAGAIN, set the error in the context. */
				else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
					memmove(&ctxt->_error, &error, sizeof(error));
			}
			
			/* Is there enough now? */
			if (length == kSOCKSv5BufferMaximum) {
				
				/* Status must be 0x00 for success. */
				if (ptr[1]) {
					ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
					ctxt->_error.error = ((kCFStreamErrorSOCKS5SubDomainUserPass << 16) | (ptr[1] & 0x000000FF));
					break;
				}

				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer);
				_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5UserPassHandshake_NoLock);
			}
		}
	} while (0);
	
	/* If there was an error remove the handshake. */
	if (ctxt->_error.error)
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
}


/* static */ void
_PerformSOCKSv4Handshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	do {
		/* Get the buffer of stuff to send */
		CFMutableDataRef to_send = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSSendBuffer);
		CFMutableDataRef to_recv = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties,
																		  _kCFStreamPropertySOCKSRecvBuffer);
		
		/* Send anything that is waiting. */
		if (to_send) {
			
			CFStreamError error = {0, 0};
			CFIndex length = CFDataGetLength(to_send);
			CFIndex sent = _CFSocketSend(ctxt->_socket, CFDataGetBytePtr(to_send), length, &error);
			
			/* If sent everything, dump the buffer. */
			if (sent == length)
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer);
				
			/* If couldn't send everything, trim the buffer. */
			else if (sent > 0) {
				
				UInt8* ptr = CFDataGetMutableBytePtr(to_send);
				
				/* New length */
				length -= sent;
				
				/* Move the bytes down in the buffer. */
				memmove(ptr, ptr + sent, length);
				
				/* Trim it. */
				CFDataSetLength(to_send, length);
				
				/* Re-enable so the rest can be written later. */
				CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
			}
			
			/* If got an error other than EAGAIN, set the error in the context. */
			else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
				memmove(&ctxt->_error, &error, sizeof(error));
		}
		
		else {
			
			UInt8* ptr;
			CFIndex length;
			
			/* If there is no receive buffer, need to create it. */
			if (!to_recv) {
				
				/* SOCKSv4 uses only an 8 byte buffer. */
				to_recv = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), kSOCKSv4BufferMaximum);
				
				/* Did it create? */
				if (!to_recv) {
					
					/* Set no memory error and bail. */
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
					
					/* Fail now. */
					break;
				}
				
				/* Add it to the properties. */
				CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer, to_recv);
				
				CFRelease(to_recv);
			}
			
			ptr = CFDataGetMutableBytePtr(to_recv);
			length = CFDataGetLength(to_recv);
			
			if (length != kSOCKSv4BufferMaximum) {
				
				CFStreamError error = {0, 0};
				CFIndex recvd = _CFSocketRecv(ctxt->_socket, ptr + length, kSOCKSv4BufferMaximum - length, &error);
				
				/* If read 0 bytes, this is an early close from the other side. */
				if (recvd == 0) {
					
					/* Mark as not connected. */
					ctxt->_error.error = ENOTCONN;
					ctxt->_error.domain = _kCFStreamErrorDomainNativeSockets;
				}
				
				/* Successfully read? */
				else if (recvd > 0) {
					
					UInt8 tmp[kSOCKSv4BufferMaximum];
					
					/* Set the length of the buffer. */
					length += recvd;
						
					/* CF is so kind as to zero the bytes on SetLength, even though it's a fixed capacity. */
					memmove(tmp, ptr, length);
					
					/* Set the length of the buffer. */
					CFDataSetLength(to_recv, length);
					
					/* Put the bytes back. */
					memmove(ptr, tmp, length);
					
					/* Re-enable after performing a successful read. */
					CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
				}
				
				/* If got an error other than EAGAIN, set the error in the context. */
				else if ((error.error != EAGAIN) || (error.domain != kCFStreamErrorDomainPOSIX))
					memmove(&ctxt->_error, &error, sizeof(error));
			}
			
			/* Is there enough now? */
			if (length == kSOCKSv4BufferMaximum) {
				
				/* If successful, remove the handshake. */
				if ((ptr[0] == 0) && (ptr[1] == 90))
					_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv4Handshake_NoLock);
				
				/* Set the error based upon the SOCKS result. */
				else {
					ctxt->_error.error = (kCFStreamErrorSOCKS4SubDomainResponse << 16) | (((ptr[0] == 0) ? ptr[1] : -1) & 0x0000FFFF);
					ctxt->_error.domain = kCFStreamErrorDomainSOCKS;
				}
				
				/* Toss this as it's not needed anymore. */
				CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSRecvBuffer);
			}
		}
	} while (0);
	
	/* If there was an error remove the handshake. */
	if (ctxt->_error.error)
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv4Handshake_NoLock);
}


/* static */ Boolean
_SOCKSSetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings) {
	
	CFDictionaryRef old = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
	
	/*
	** Up front check for correct settings type before tossing
	** out the existing SOCKS info.  Can only set SOCKS proxy
	** if not opened or opening.  Can't use SOCKS if created
	** with a connected socket.
	*/	
    if ((settings && (CFDictionaryGetTypeID() != CFGetTypeID(settings))) || 
        __CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete) ||
        __CFBitIsSet(ctxt->_flags, kFlagBitOpenStarted) ||
        __CFBitIsSet(ctxt->_flags, kFlagBitCreatedNative))
    {
        return FALSE;
    }
	
	if (!old || !CFEqual(old, settings)) {
		
		/* Removing the SOCKS proxy? */
		if (!settings) {
			
			/* Remove the settings. */
			CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
			
			/* Remove the handshake (removes both to make sure neither socks is set). */
			_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv4Handshake_NoLock);
			_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSOCKSv5Handshake_NoLock);
		}
		
		/* Client is setting the proxy. */
		else {
			
			CFStringRef name = NULL;
			CFStringRef user = (CFStringRef)CFDictionaryGetValue(settings, kCFStreamPropertySOCKSUser);
			CFStringRef pass = (CFStringRef)CFDictionaryGetValue(settings, kCFStreamPropertySOCKSPassword);
			CFStringRef version = _GetSOCKSVersion(settings);
			CFTypeRef lookup = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
			CFNumberRef enabled = CFDictionaryGetValue(settings,  _kCFStreamProxySettingSOCKSEnable);
			SInt32 enabled_value = 0;
			
			/* Verify that a valid version has been set.  No setting is SOCKSv5. */
			if (!CFEqual(version, kCFStreamSocketSOCKSVersion4) && !CFEqual(version, kCFStreamSocketSOCKSVersion5))
				return FALSE;
            
			/* See if this is being pulled directly out of SC and try to do the right thing. */
			if (enabled && CFNumberGetValue(enabled, kCFNumberSInt32Type, &enabled_value) && !enabled_value) {
				
				/* If it's not enabled, this means "don't set it." */
				CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
                
				return TRUE;
			}
			
			/* Is there far end information for setting up the tunnel? */
			if (lookup) {
				
				/* Get the list of names for setting up the tunnel */
				CFArrayRef list = CFHostGetNames((CFHostRef)lookup, NULL);
				
				/* Good with at least one name? */
				if (list && CFArrayGetCount(list))
					name = (CFStringRef)CFRetain((CFTypeRef)CFArrayGetValueAtIndex(list, 0));
				
				/* No name, but can create one with an IP. */
				else {
					
					/* Is there one good address to create a name? */
					list = CFHostGetAddressing((CFHostRef)lookup, NULL);
					if (list && CFArrayGetCount(list)) {
						name = _CFNetworkCFStringCreateWithCFDataAddress(CFGetAllocator(list),
																		 (CFDataRef)CFArrayGetValueAtIndex(list, 0));
					}
				}
			}
			
			/* No host, but is there a possible CFNetService? */
			else if ((lookup = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService))) {
				
				/* Can't perform SOCKS to a CFNetService. */
				return FALSE;
			}
			
			/* Fail if there is no way to put together a SOCKS request. */
			if (!name) {
				
				/* SOCKSv4 can attempt the resolve to get the name. */
				if (!CFEqual(version, kCFStreamSocketSOCKSVersion4))
					return FALSE;
			}
			
			else {
				/* The maximum hostname length to be packed is 255 in SOCKSv5 */
				CFIndex length = CFStringGetLength(name);
				
				/* Check to see if need this proxy for the intended host. */
				if (!_CFNetworkDoesNeedProxy(name,
											 CFDictionaryGetValue(settings, kCFStreamPropertyProxyExceptionsList),
											 CFDictionaryGetValue(settings, kCFStreamPropertyProxyLocalBypass)))
				{
					CFRelease(name);
					return FALSE;
				}
				
				CFRelease(name);
				
				/* SOCKSv5 requires that the host name be a maximum of 255. */
				if (CFEqual(version, kCFStreamSocketSOCKSVersion5) && ((length <= 0) || (length > 255)))
					return FALSE;
			}
			
			/* SOCKSv5 maximum password length if given is 255. */
			if (CFEqual(version, kCFStreamSocketSOCKSVersion5) && pass && (CFStringGetLength(pass) > 255))
				return FALSE;
			
			if (user) {
				
				/* SOCKSv4 maximum user name length is 512. */
				if (CFEqual(version, kCFStreamSocketSOCKSVersion4) && (CFStringGetLength(user) > 512))
					return FALSE;
				
				/* SOCKSv5 maximum user name length is 255. */
				else if (CFEqual(version, kCFStreamSocketSOCKSVersion5) && (CFStringGetLength(user) > 255))
					return FALSE;
			}
			
			/* Add the handshake to the list to perform. */
			if (!_SocketStreamAddHandshake_NoLock(ctxt, CFEqual(version, kCFStreamSocketSOCKSVersion4) ? _PerformSOCKSv4Handshake_NoLock : _PerformSOCKSv5Handshake_NoLock))
				return FALSE;
			
			/* Put the new setting in place, removing the old if previously set. */
			CFDictionarySetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy, settings);
		}
	}
	
	return TRUE;
}


/* static */ void
_SocketStreamSOCKSHandleLookup_NoLock(_CFSocketStreamContext* ctxt, CFHostRef lookup) {
	
	int i;
	CFArrayRef addresses;
	CFStringRef name = NULL;
	CFMutableArrayRef loops[3];
	CFIndex user_len = 0;
	CFIndex extra = 0;
	CFMutableDataRef buffer;
	CFDictionaryRef settings = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
	CFStringRef user = (CFStringRef)CFDictionaryGetValue(settings, kCFStreamPropertySOCKSUser);
	UInt8* ptr = NULL;
	
	CFNumberRef port = CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketRemotePort);
	
	/* Remove the lookup from the schedulables since it's done. */
	_SchedulablesRemove(ctxt->_schedulables, lookup);
	
	/* Invalidate it so no callbacks occur. */
	_CFTypeInvalidate(lookup);
	
	/* Grab the list of run loops and modes for unscheduling. */
	loops[0] = ctxt->_readloops;
	loops[1] = ctxt->_writeloops;
	loops[2] = ctxt->_sharedloops;
	
	/* Make sure to remove the lookup from all loops and modes. */
	for (i = 0; i < (sizeof(loops) / sizeof(loops[0])); i++)
		_CFTypeUnscheduleFromMultipleRunLoops(lookup, loops[i]);
	
	/* Get the list of addresses. */
	addresses = CFHostGetAddressing((CFHostRef)lookup, NULL);
	
	/* If no addresses, go for the name. */
	if (!addresses || !CFArrayGetCount(addresses))
		name = (CFStringRef)CFArrayGetValueAtIndex(CFHostGetNames((CFHostRef)lookup, NULL), 0);
	
	/* Find the overhead for the user name if one. */
	if (user) {
		user_len = CFStringGetBytes(user, CFRangeMake(0, CFStringGetLength(user)),
									kCFStringEncodingUTF8, 0, FALSE, NULL, 0, NULL);
	}
	
	/* Add for null termination. */
	user_len += 1;
	
	if (name) {
		
		/* What's the cost in bytes of the host name? */
		extra = CFStringGetBytes(name, CFRangeMake(0, CFStringGetLength(name)), kCFStringEncodingUTF8, 0, FALSE, NULL, 0, NULL);
		extra += 1;
	}
	
	/* Create the send buffer. */
	buffer = CFDataCreateMutable(CFGetAllocator(ctxt->_properties), 8 + extra + user_len);
	
	/* If failed, set the no memory error and return. */
	if (!buffer) {
		ctxt->_error.error = ENOMEM;
		ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
		
		return;						/* NOTE the early return. */
	}
	
	/* Extend out the length to the full capacity. */
	CFDataSetLength(buffer, 8 + extra + user_len);
	
	/* Add it to the properties for future. */
	CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer, buffer);
	CFRelease(buffer);
	
	/* Get the pointer for easier manipulation. */
	ptr = CFDataGetMutableBytePtr(buffer);
	
	/* Zero out even though SetLength probably did. */
	memset(ptr, 0, CFDataGetLength(buffer));
	
	/* If a name was set, there was no address. */
	if (name) {
		
		/* Copy the name into the buffer.  */
		CFStringGetBytes(name, CFRangeMake(0, CFStringGetLength(name)),
						 kCFStringEncodingUTF8, 0, FALSE, ptr + 8 + user_len - 1, extra, NULL);
		
		/* Cap with a null. */
		ptr[8 + user_len + extra - 1] = '\0';
	}
	
	/* Use an address instead of a name. */
	else {
		
		CFIndex i, count = CFArrayGetCount(addresses);
		
		/* Loop through looking for a valid IPv4 address.  SOCKSv4 doesn't do IPv6. */
		for (i = 0; i < count; i++) {
			
			struct sockaddr_in* sin = (struct sockaddr_in*)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, i));
			
			if (sin->sin_family == AF_INET) {
				
				/* Set the IP in the buffer. */
				memmove(&ptr[4], &sin->sin_addr, sizeof(sin->sin_addr));
				
				/* If there was no port, it must be in the address. */
				if (!port)
					memmove(&ptr[2], &sin->sin_port, sizeof(sin->sin_port));
				
				break;
			}
		}
		
		/* Went through all the addresses and found none suitable. */
		if (i == count) {
			
			/* Mark as an invalid parameter */
			ctxt->_error.error = EINVAL;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			
			/* Remove the buffer, 'cause this isn't going anywhere. */
			CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertySOCKSSendBuffer);
			
			return;					/* NOTE the early return. */
		}
	}
	
	/* Set the protocol version and "CONNECT" command. */
	ptr[0] = 0x04;
	ptr[1] = 0x01;
	
	/* If there was a port set, need to copy that. */
	if (port) {
		
		SInt32 value;
		
		/* Grab the real value. */
		CFNumberGetValue(port, kCFNumberSInt32Type, &value);
		
		/* Place the port into the buffer. */
		*((UInt16*)(&ptr[2])) = htons(value & 0x0000FFFF);
	}
	
	/* If there was a user name, need to grab its bytes into place. */
	if (user) {
		CFStringGetBytes(user, CFRangeMake(0, CFStringGetLength(user)),
						 kCFStringEncodingUTF8, 0, FALSE, ptr + 8, user_len - 1, NULL);
	}
	
	/* If open has already finished, need to pump this thing along. */
	if (__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete)) {
		
		CFArrayRef handshakes = (CFArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
		
		/* Only force the "pump" if SOCKS is the head. */
		if (handshakes && (CFArrayGetValueAtIndex(handshakes, 0) == _PerformSOCKSv4Handshake_NoLock))
			_PerformSOCKSv4Handshake_NoLock(ctxt);
	}
}


#if 0
#pragma mark *CONNECT Support
#endif

/* static */ void
_CreateNameAndPortForCONNECTProxy(CFDictionaryRef properties, CFStringRef* name, CFNumberRef* port, CFStreamError* error) {
		
	CFTypeRef lookup;
	CFDataRef addr = NULL;
	CFAllocatorRef alloc = CFGetAllocator(properties);
	
	/* NOTE that this function is used for setting the SSL peer ID for connections other than CONNECT tunnelling. */
	
	*name = NULL;
	*port = NULL;
	
	/* No error to start */
	memset(error, 0, sizeof(error[0]));
	
	/* Try to simply get the port from the properties. */
	*port = (CFNumberRef)CFDictionaryGetValue(properties, _kCFStreamPropertySocketRemotePort);
	
	/* Try to get the service for which the streams were created. */
	if ((lookup = (CFTypeRef)CFDictionaryGetValue(properties, kCFStreamPropertySocketRemoteNetService))) {
		
		/* Does it have a name?  These are checked at set, but just in case they go away. */
		*name = CFNetServiceGetTargetHost((CFNetServiceRef)lookup);
		
		/* If didn't get a port, need to go to the address to get it. */
		if (!*port) {
			
			/* Get the list of addresses from the service. */
			CFArrayRef list = CFNetServiceGetAddressing((CFNetServiceRef)lookup);
			
			/* Can only pull one out if it's been resolved. */
			if (list && CFArrayGetCount(list))
				addr = CFArrayGetValueAtIndex(list, 0);
		}
	}
	
	/* No service, so go for the host. */
	else if ((lookup = (CFTypeRef)CFDictionaryGetValue(properties, kCFStreamPropertySocketRemoteHost))) {
		
		/* Get the list of names in order to get one. */
		CFArrayRef list = CFHostGetNames((CFHostRef)lookup, NULL);
		
		/* Pull out the name */
		if (list && CFArrayGetCount(list))
			*name = (CFStringRef)CFArrayGetValueAtIndex(list, 0);
		
		else {
			
			/* No name, so get the address as a name instead. */
			list = CFHostGetAddressing((CFHostRef)lookup, NULL);
			
			/* Get the first address from the list. */
			if (list && CFArrayGetCount(list))
				addr = CFArrayGetValueAtIndex(list, 0);
		}
	}
	
	/* If there is no information at all, error and bail. */
	if (!*port && !*name && !addr) {
		error->error = EINVAL;
		error->domain = kCFStreamErrorDomainPOSIX;
		return;												/* NOTE the early return */
	}
	
	/* Got a port?  If so, just retain it. */
	if (*port)
		CFRetain(*port);
	
	else {
		
		SInt32 p;
		struct sockaddr* sa = (struct sockaddr*)CFDataGetBytePtr(addr);
		
		/* Need to go to the socket address in order to get the port value */
		switch (sa->sa_family) {
			
			case AF_INET:
				p = ntohs(((struct sockaddr_in*)sa)->sin_port);
				*port = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
				break;
				
			case AF_INET6:
				p = ntohs(((struct sockaddr_in6*)sa)->sin6_port);
				*port = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
				break;
				
			default:
				/* Not a known type.  Return an error. */
				error->error = EINVAL;
				error->domain = kCFStreamErrorDomainPOSIX;
				return;										/* NOTE the early return */
		}
	}
	
	/* Either retain the current name or create one from the address. */
	if (*name)
		CFRetain(*name);
	else
		*name = _CFNetworkCFStringCreateWithCFDataAddress(alloc, addr);
	
	/* If either failed, need to error out. */
	if (!*name || !*port) {
		
		/* Release anything that was created/retained. */
		if (*name) CFRelease(*name);
		if (*port) CFRelease(*port);
		
		/* Set the out of memory error. */
		error->error = ENOMEM;
		error->domain = kCFStreamErrorDomainPOSIX;
	}
}		


/* static */ void
_PerformCONNECTHandshake_NoLock(_CFSocketStreamContext* ctxt) {
    
    UInt8 buffer[2048];
	CFIndex count;
	CFStreamError error = {0, 0};
	CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
	CFHTTPMessageRef response = (CFHTTPMessageRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTResponse);
	
	/* NOTE, use the lack of response to mean that need to send first. */
	if (!response) {
		
		CFIndex length;
		CFDataRef left = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyCONNECTSendBuffer);
		
		/* If there is nothing waiting, haven't put anything together. */
		if (!left) {
			
			CFDictionaryRef headers;
			CFHTTPMessageRef request = NULL;
			CFStringRef version, name = NULL;
			CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy);
			
			/* Get the intended version of HTTP. */
			version = (CFStringRef)CFDictionaryGetValue(proxy, kCFStreamPropertyCONNECTVersion);
			
			do {
				SInt32 p;
				CFURLRef url;
				CFNumberRef port;
				CFStringRef urlstr;

				/* Figure out what the far end name and port are. */
				_CreateNameAndPortForCONNECTProxy(ctxt->_properties, &name, &port, &ctxt->_error);
			
				/* Got an error, so bail. */
				if (ctxt->_error.error) break;
				
				/* Get the real port value. */
				CFNumberGetValue(port, kCFNumberSInt32Type, &p);
			
				/* Produce the "CONNECT" url which is <host>:<port>. */
				urlstr = CFStringCreateWithFormat(alloc, NULL, _kCFStreamCONNECTURLFormat, name, p & 0x0000FFFF);
			
				/* Don't need it. */
				CFRelease(port);
				
				/* Make sure there's an url before continuing on. */
				if (!urlstr) break;
				
				/* Create the url based upon the string. */
				url = CFURLCreateWithString(alloc, urlstr, NULL);
				
				/* Don't need it. */
				CFRelease(urlstr);
				
				/* Must have an url to continue. */
				if (!url) break;
			
				/*
				** Create the "CONNECT" request.  Default HTTP version is 1.0 if none specified.
				** NOTE there are actually some servers which will fail if 1.1 is used.
				*/
				request = CFHTTPMessageCreateRequest(alloc, _kCFStreamCONNECTMethod, url, version ? version : kCFHTTPVersion1_0);
			
				CFRelease(url);
			
			} while (0);
			
			/* Fail if the request wasn't made. */
			if (!request) {
				
				if (name) CFRelease(name);
				
				/* If an error wasn't set, assume an out of memory error. */
				if (!ctxt->_error.error) {
					ctxt->_error.error = ENOMEM;
					ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				}
				
				return;												/* NOTE the early return */
			}

			/* Add the other headers */
			headers = CFDictionaryGetValue(proxy, kCFStreamPropertyCONNECTAdditionalHeaders);
			if (headers) {
				
				/* Check to see if "Host:" needs to be added and do so. */
				CFStringRef value = (CFStringRef)CFDictionaryGetValue(headers, _kCFStreamHostHeader);
				if (!value)
					CFHTTPMessageSetHeaderFieldValue(request, _kCFStreamHostHeader, name);
				
				/* Check to see if "User-Agent:" need to be added and do so. */
				value = (CFStringRef)CFDictionaryGetValue(headers, _kCFStreamUserAgentHeader);
				if (!value)
					CFHTTPMessageSetHeaderFieldValue(request, _kCFStreamUserAgentHeader, _CFNetworkUserAgentString());
				
				/* Add all the other headers. */
				CFDictionaryApplyFunction(headers, (CFDictionaryApplierFunction)_CONNECTHeaderApplier, request);
			}
			else {
				/* CONNECT must have "Host:" and "User-Agent:" headers. */
				CFHTTPMessageSetHeaderFieldValue(request, _kCFStreamHostHeader, name);
				CFHTTPMessageSetHeaderFieldValue(request, _kCFStreamUserAgentHeader, _CFNetworkUserAgentString());
			}
			
			CFRelease(name);
			
			/*
			** Flatten the request using this special version.  This will
			** flatten it for proxy usage (it will keep the full url).
			*/
			left = _CFHTTPMessageCopySerializedHeaders(request, TRUE);
			CFRelease(request);
			
			/* If failed to flatten, specify out of memory. */
			if (!left) {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				return;												/* NOTE the early return */
			}
			
			/* Add the buffer to the properties for future. */
			CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertyCONNECTSendBuffer, left);
			CFRelease(left);
		}
		
		/* Find out how much is left and try sending. */
		length = CFDataGetLength(left);
		count = _CFSocketSend(ctxt->_socket, CFDataGetBytePtr(left), length, &error);
		
		if (error.error) {
		
			/* EAGAIN is not really an error. */
			if ((error.error == EAGAIN) && (error.domain == _kCFStreamErrorDomainNativeSockets))
				count = 0;
			
			/* Other errors, copy into place and bail. */
			else {
				memmove(&ctxt->_error, &error, sizeof(error));
				return;												/* NOTE the early return */
			}
		}
		
		/* Shrink by the sent amount. */
		length -= count;
		
		/* Re-enable so can continue. */
		CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
		
		/* Did everything get written? */
		if (!length) {
			
			/* Toss the buffer, since it is done. */
			CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyCONNECTSendBuffer);
			
			/* Make sure there is a response for reading. */
			response = CFHTTPMessageCreateEmpty(alloc, FALSE);
			
			/* Bail if failed to create. */
			if (!response) {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				return;												/* NOTE the early return */
			}
			
			/* Save it as a property, since that's what it is. */
			CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertyCONNECTResponse, response);
			CFRelease(response);
		}
		
		/* If did read but not everything, need to trim the buffer. */
		else if (count) {
			
			/* Just create a copy of what's left. */
			left = CFDataCreate(alloc, ((UInt8*)CFDataGetBytePtr(left)) + count, length);
			
			/* Failed with an out of memory? */
			if (!left) {
				ctxt->_error.error = ENOMEM;
				ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
				return;												/* NOTE the early return */
			}
			
			/* Put it over top of the other. */
			CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyCONNECTSendBuffer, left);
			CFRelease(left);
		}
		
		/* Would block. */
		else
			return;												/* NOTE the early return */
	}
	
	/*
	** Peek at the socket buffer to get the bytes.  Since there is no capability
	** of pushing bytes back, need to peek to pull out the bytes.  Don't  want to
	** read bytes beyond the response bytes.
	*/
	if (-1 == (count = recv(CFSocketGetNative(ctxt->_socket), buffer, sizeof(buffer), MSG_PEEK))) {
		
		/* If it's an EAGAIN, re-enable the callback to come back later. */
		if ((_LastError(&error) == EAGAIN) && (error.domain == _kCFStreamErrorDomainNativeSockets)) {
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
			return;												/* NOTE the early return */
		}
		
		/* Some other error, so bail. */
		else if (error.error) {
			memmove(&ctxt->_error, &error, sizeof(error));
			return;												/* NOTE the early return */
		}
	}
	
	/* Add the bytes to the response.  Let it determine the end. */
	if (CFHTTPMessageAppendBytes(response, buffer, count)) {
		
		/*
		** Detect done if the head is complete.   For CONNECT,
		** all bytes after the headers belong to the client.
		*/
		if (CFHTTPMessageIsHeaderComplete(response)) {

			/* Get the result code for check later. */
			UInt32 code = CFHTTPMessageGetResponseStatusCode(response);

			/* If there were extra bytes read, need to remove those. */
			CFDataRef body = CFHTTPMessageCopyBody(response);
			if (body) {
				
				/* Reduce the count, so the correct byte count can get sucked out of the kernel. */
				count -= CFDataGetLength(body);
				CFRelease(body);
				
				/* Get rid of the body.  CONNECT responses have no body. */
				CFHTTPMessageSetBody(response, NULL);
			}
			
			/* If it wasn't a 200 series result, stall the stream for another CONNECT. */
			if ((code < 200) || (code > 299))
				_SocketStreamAddHandshake_NoLock(ctxt, _PerformCONNECTHaltHandshake_NoLock);
			
			/* Remove the handshake now that it is complete. */
			_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformCONNECTHandshake_NoLock);
			
			CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyPreviousCONNECTResponse);
		}
		
		/* Suck the bytes out of the kernel. */
		if (-1 == recv(CFSocketGetNative(ctxt->_socket), buffer, count, 0)) {
			
			/* Copy the last error. */
			_LastError(&ctxt->_error);
			
			return;												/* NOTE the early return */
		}
				
		/* Re-enable for future callbacks. */
		CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
	}
	
	/* Failed to append the bytes, so error as a HTTP parsing failure. */
	else {
		ctxt->_error.error = kCFStreamErrorHTTPParseFailure;
		ctxt->_error.domain = kCFStreamErrorDomainHTTP;
	}
}


/* static */ void
_PerformCONNECTHaltHandshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	(void)ctxt;		/* unused */
	
	/* Do nothing.  This will cause a stall until it's time to go again. */
	
	if (ctxt->_clientWriteStream && __CFBitIsSet(ctxt->_flags, kFlagBitWriteStreamOpened)) {
		__CFBitSet(ctxt->_flags, kFlagBitCanWrite);
		_CFWriteStreamSignalEventDelayed(ctxt->_clientWriteStream, kCFStreamEventCanAcceptBytes, NULL);
	}
	
	if (ctxt->_clientReadStream && __CFBitIsSet(ctxt->_flags, kFlagBitReadStreamOpened)) {
		__CFBitSet(ctxt->_flags, kFlagBitCanRead);
		_CFReadStreamSignalEventDelayed(ctxt->_clientReadStream, kCFStreamEventHasBytesAvailable, NULL);
	}
}


/* static */ void
_CONNECTHeaderApplier(CFStringRef key, CFStringRef value, CFHTTPMessageRef request) {
	
    CFHTTPMessageSetHeaderFieldValue(request, key, value);
}


/* static */ Boolean
_CONNECTSetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings) {
	
	Boolean resume = FALSE;
    CFStringRef server = settings ? CFDictionaryGetValue(settings, kCFStreamPropertyCONNECTProxyHost) : NULL;
	CFNumberRef port = settings ? CFDictionaryGetValue(settings, kCFStreamPropertyCONNECTProxyPort) : NULL;
	CFDictionaryRef old = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy);
	
	/*
    ** Up front check for server information before tossing out the
    ** existing CONNECT info.  Can't use CONNECT if created with a
	** connected socket.
	*/
    if ((settings && (!server || !port)) || 
        __CFBitIsSet(ctxt->_flags, kFlagBitCreatedNative))
    {
        return FALSE;
    }

	if (__CFBitIsSet(ctxt->_flags, kFlagBitOpenComplete) || __CFBitIsSet(ctxt->_flags, kFlagBitOpenStarted)) {
	
		CFMutableArrayRef handshakes = (CFMutableArrayRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyHandshakes);
		
		if (handshakes && 
			CFArrayGetCount(handshakes) &&
			(_PerformCONNECTHaltHandshake_NoLock == CFArrayGetValueAtIndex(handshakes, 0)))
		{
			resume = TRUE;
		}
		
		else
			return FALSE;
	}
		
	/* If setting the same setting, just return TRUE. */
	if (!old || !CFEqual(old, settings)) {
		
		/* Removing the CONNECT proxy? */
		if (!settings) {
			
			/* Remove the settings. */
			CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy);
			
			/* Remove the handshake. */
			_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformCONNECTHandshake_NoLock);
		}
		
		/* Client is setting the proxy. */
		else {
			
			Boolean hasName = FALSE;
			CFTypeRef lookup = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
			
			/* Is there far end information for setting up the tunnel? */
			if (lookup) {
				
				/* Get the list of names for setting up the tunnel */
				CFArrayRef list = CFHostGetNames((CFHostRef)lookup, NULL);
				
				/* Good with at least one name? */
				if (list && CFArrayGetCount(list))
					hasName = TRUE;
				
				/* No name, but can create one with an IP. */
				else {
					
					/* Is there one good address to create a name? */
					list = CFHostGetAddressing((CFHostRef)lookup, NULL);
					if (list && CFArrayGetCount(list))
						hasName = TRUE;
				}
			}
			
			/* No host, but is there a possible CFNetService with enough information? */
			else if ((lookup = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService))) {
				
				/* Has the far end server been resolved already? */
				if (CFNetServiceGetTargetHost((CFNetServiceRef)lookup))
					hasName = TRUE;
				
				/*
				** NOTE that the target host is resolved.  If that doesn't exist,
				** there is no reason to check addresses since those are resolved
				** too.  There is no way to create a CFNetServiceRef from an
				** address only.
				*/
			}
			
			/* Fail if there is no way to put together a CONNECT request. */
			if (!hasName)
				return FALSE;
			
			/* Add the handshake to the list to perform. */
			if (!_SocketStreamAddHandshake_NoLock(ctxt, _PerformCONNECTHandshake_NoLock)) {

				if (resume) {
					CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyCONNECTResponse);
					_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformCONNECTHaltHandshake_NoLock);
				}

				return FALSE;
			}
			
			if (resume) {
				CFTypeRef last = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTResponse);
				if (last)
					CFDictionarySetValue(ctxt->_properties, kCFStreamPropertyPreviousCONNECTResponse, last);
				CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyCONNECTResponse);
				_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformCONNECTHaltHandshake_NoLock);
			}
			
			/* Put the new setting in place, removing the old if previously set. */
			CFDictionarySetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy, settings);
		}
	}
	
	return TRUE;
}	


#if 0
#pragma mark *SSL Support
#endif

/* static */ OSStatus
_SecurityReadFunc_NoLock(_CFSocketStreamContext* ctxt, void* data, UInt32* dataLength) {
	
	/* This is the read function used by SecureTransport in order to get bytes off the wire. */
	
	/* NOTE that SSL reads bytes off the wire and into a buffer of encrypted bytes. */
	
	CFIndex i, s;
    UInt32 try = *dataLength;
	CFStreamError error = {0, 0};
	
	/* Bits required for buffered reading. */
	CFDataRef size = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBufferSize);
	CFMutableDataRef buffer = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBuffer);
	CFMutableDataRef count = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBufferCount);
	
	/* If missing the buffer, assume nothing has been created. */
	if (!buffer) {
		
		CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
		
		/* Start with a size.  This could be overridden from properties. */
		if (!size) {
			s = kSecurityBufferSize;
			size = CFDataCreate(alloc, (const UInt8*)&s, sizeof(s));
		}
		
		/* If have a size, create buffers for the count and the buffer itself. */
		if (size) {
			buffer = CFDataCreateMutable(alloc, *((CFIndex*)CFDataGetBytePtr(size)));
			count = CFDataCreateMutable(alloc, sizeof(CFIndex));
		}
		
		/* If anything failed, set out of memory and return error. */
		if (!buffer || !count || !size) {
			
			if (buffer) CFRelease(buffer);
			if (count) CFRelease(count);
			if (size) CFRelease(size);
			
			ctxt->_error.error = ENOMEM;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			
			return errSSLInternal;								/* NOTE the eary return. */
		}
		
		/* Place everything in the properties bucket for later. */
		CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBufferSize, size);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBuffer, buffer);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertySecurityRecvBufferCount, count);
		
		CFRelease(size);
		CFRelease(buffer);
		CFRelease(count);
		
		/* Set the intial count to zero. */
		*((CFIndex*)CFDataGetMutableBytePtr(count)) = 0;
	}
	
	/* Get the count and size, respectively. */
	i = *((CFIndex*)CFDataGetMutableBytePtr(count));
	s = *((CFIndex*)CFDataGetBytePtr(size));
	
	/* If the count is less than the request, go to the wire. */
	if (i < *dataLength) {
		
		/* Read the bytes off the wire into what's left of the buffer. */
		CFIndex r = _CFSocketRecv(ctxt->_socket, ((UInt8*)CFDataGetMutableBytePtr(buffer)) + i, s - i, &error);
		
		__CFBitClear(ctxt->_flags, kFlagBitRecvdRead);
		
		/* If no error occurred, add the read bytes to the count. */
		if (!error.error)
			i += r;
	}
	else
		__CFBitSet(ctxt->_flags, kFlagBitRecvdRead);
	
	/* If still no errr, continue on. */
	if (!error.error) {
		
		UInt8* ptr = (UInt8*)CFDataGetMutableBytePtr(buffer);
		
		/* Either read what the client asked or what is in the buffer. */
		*dataLength = (*dataLength <= i) ? *dataLength : i;
		
		/* Decrease the buffer count by the return value. */
		i -= *dataLength;
		
		/* Copy the bytes into the client's buffer. */
		memmove(data, ptr, *dataLength);
		
		/* Move the bytes in the buffer down by the read count. */
		memmove(ptr, ptr + *dataLength, i);
		
		/* Zero the bytes that are no longer part of the buffer count. */
		memset(ptr + i, 0, s - i);
		
		/* Make sure to set count again. */
		*((CFIndex*)CFDataGetMutableBytePtr(count)) = i;
		
		/* If something was read, re-enable the read callback. */
		if (*dataLength)
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
		
		/* If no bytes read, return closed.  If couldn't read all, return "would block." */
        return (!*dataLength ? errSSLClosedAbort : (try == *dataLength) ? 0 : errSSLWouldBlock);
	}
	
	/* Error condition means no bytes read. */
	*dataLength = 0;
	
	/* If it's a "would block," return such. */
	if ((error.domain == _kCFStreamErrorDomainNativeSockets) && (EAGAIN == error.error))
        return errSSLWouldBlock;
	
	/* It's a real error, so copy it into the context */
	memmove(&ctxt->_error, &error, sizeof(error));
	
	/* A bad error occurred. */
	return errSSLInternal;
}	


/* static */ OSStatus
_SecurityWriteFunc_NoLock(_CFSocketStreamContext* ctxt, const void* data, UInt32* dataLength) {
	
	/* This is the function used by SecureTransport to write bytes to the wire. */
	
	CFStreamError error = {0, 0};
    UInt32 try = *dataLength;
	
	/* Write what the ST tells. */
	*dataLength = _CFSocketSend(ctxt->_socket, data, *dataLength, &error);
	
	/* If no error, return noErr, or if couldn't write everything, return the "would block." */
	if (!error.error)
		return ((try == *dataLength) ? 0 : errSSLWouldBlock);
	
	/* Error condition means no bytes written. */
	*dataLength = 0;
	
	/* If it was a "would block," return such. */
	if ((error.domain == _kCFStreamErrorDomainNativeSockets) && (EAGAIN == error.error))
        return errSSLWouldBlock;
	
	/* Copy the real error into place. */
	memmove(&ctxt->_error, &error, sizeof(error));

	/* Something bad happened. */
	return errSSLInternal;
}


/* static */ CFIndex
_SocketStreamSecuritySend_NoLock(_CFSocketStreamContext* ctxt, const UInt8* buffer, CFIndex length) {
	
    CFIndex bytesWritten = 0;
	
	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
    /* Try to write bytes on the socket. */
	OSStatus result = SSLWrite(ssl, buffer, length, (size_t*)(&bytesWritten));
    
	/* Check to see if error was set during the write. */
    if (ctxt->_error.error)
        return -1;
    
	/* If the stream wrote bytes but then got a blocking error, pass it as success. */
    if ((result == errSSLWouldBlock) && bytesWritten) {
        _SocketStreamAddHandshake_NoLock(ctxt, _PerformSecuritySendHandshake_NoLock);
        result = noErr;
    }
    
    /* Deal with result. */
    switch (result) {
        case errSSLClosedGraceful:			/* Non-fatal error */
        case errSSLClosedAbort:				/* Assumed non-fatal error (but may not be) **FIXME** ?? */
			
			/* Mark SSL as closed.  There could still be bytes in the buffers. */
			__CFBitSet(ctxt->_flags, kFlagBitClosed);
			
			/* NOTE the fall through. */
			
        case noErr:
            break;
            
        default:
			ctxt->_error.error = result;
			ctxt->_error.domain = kCFStreamErrorDomainSSL;
            return -1;
    }
	
    return bytesWritten;
}


/* static */ void
_SocketStreamSecurityBufferedRead_NoLock(_CFSocketStreamContext* ctxt) {

	/*
	** This function is used to read bytes out of the encrypted buffer and
	** into the unencrypted buffer.
	*/
	
	CFIndex* i;
	CFIndex s = kRecvBufferSize;
	OSStatus status = noErr;
	
	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	/* Get the bits required in order to work with the buffer. */
	CFNumberRef size = (CFNumberRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferSize);
	CFMutableDataRef buffer = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBuffer);
	CFMutableDataRef count = (CFMutableDataRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount);
	
	/* No buffer assumes all are missing. */
	if (!buffer) {
		
		CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
		
		/* If no size, assume a default.  Can be overridden by properties. */
		if (!size)
			size = CFNumberCreate(alloc, kCFNumberCFIndexType, &s);
		else
			CFNumberGetValue(size, kCFNumberCFIndexType, &s);

		/* Create the backing for the buffer and the counter. */
		if (size) {
			buffer = CFDataCreateMutable(alloc, s);
			count = CFDataCreateMutable(alloc, sizeof(CFIndex));
		}
		
		/* If anything failed, set out of memory and bail. */
		if (!buffer || !count || !size) {
			
			if (buffer) CFRelease(buffer);
			if (count) CFRelease(count);
			if (size) CFRelease(size);
			
			ctxt->_error.error = ENOMEM;
			ctxt->_error.domain = kCFStreamErrorDomainPOSIX;
			
			return;								/* NOTE the eary return. */
		}
		
		/* Save the buffer information. */
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferSize, size);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBuffer, buffer);
		CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyRecvBufferCount, count);
		
		CFRelease(size);
		CFRelease(buffer);
		CFRelease(count);
		
		/* Start with a zero byte count. */
		*((CFIndex*)CFDataGetMutableBytePtr(count)) = 0;
	}
	
	/* Get the count and size of the buffer, respectively. */
	i = (CFIndex*)CFDataGetMutableBytePtr(count);
	CFNumberGetValue(size, kCFNumberCFIndexType, &s);
	
	/* Only read if there is room in the buffer. */
	if (*i < s) {
		
		CFIndex start = *i;
		UInt8* ptr = (UInt8*)CFDataGetMutableBytePtr(buffer);
		
		/* Keep reading out of the encrypted buffer until an error or full. */
		while (!status && (*i < s)) {
			
			CFIndex bytesRead = 0;
			
			/* Read out of the encrypted and into the unencrypted. */
			status = SSLRead(ssl, ptr + *i, s - *i, (size_t*)(&bytesRead));
		
			/* If did read bytes, increase the count. */
			if (bytesRead > 0)
				*i = *i + bytesRead;
		}
		
		/* If didn't read bytes and the buffer is empty but SSL hasn't closed, need read events again. */
		if ((*i == start) && (*i == 0) && !__CFBitIsSet(ctxt->_flags, kFlagBitClosed))
			CFSocketEnableCallBacks(ctxt->_socket, kCFSocketReadCallBack);
	}
	
	switch (status) {
		case errSSLClosedGraceful:			/* Non-fatal error */
		case errSSLClosedAbort:				/* Assumed non-fatal error (but may not be) **FIXME** ?? */
			
			/* SSL has closed, so mark as such. */
			__CFBitSet(ctxt->_flags, kFlagBitClosed);
			__CFBitSet(ctxt->_flags, kFlagBitCanRead);
			__CFBitClear(ctxt->_flags, kFlagBitPollRead);
			
			/* NOTE the fall through. */

		case noErr:
		case errSSLWouldBlock:
			
			/* If there are bytes in the buffer to be read, set the bit. */
			if (*i) {
				__CFBitSet(ctxt->_flags, kFlagBitCanRead);
				__CFBitClear(ctxt->_flags, kFlagBitPollRead);
			}
			break;
			
		default:
			if (!ctxt->_error.error) {
				ctxt->_error.error = status;
				ctxt->_error.domain = kCFStreamErrorDomainSSL;
			}
			break;
	}
}


/* static */ void
_PerformSecurityHandshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	OSStatus result;
	const void* peerid = NULL;
	size_t peeridlen;

	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	/* Make sure the peer id has been set for ST performance. */
	result = SSLGetPeerID(ssl, &peerid, &peeridlen);
	if (!result && !peerid) {
		
		Boolean set = FALSE;
		
		/* Check to see if going through a proxy.  A different ID is used in that case. */
		CFTypeRef value = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyCONNECTProxy);
		if (!value)
			value = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
		
		/* Use the name and port of the far end host. */
		if (value) {
			CFStringRef host = NULL;
			CFNumberRef port = NULL;
			CFStreamError error;
			
			/* Re-use some code to get the host and port of the far end. */
			_CreateNameAndPortForCONNECTProxy(ctxt->_properties, &host, &port, &error);
			
			/* Only do it if got the host and port, otherwise it'll fall through to the address. */
			if (host && port) {
				
				SInt32 p;
				CFStringRef peer;
				CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
				
				/* Get the real port value. */
				CFNumberGetValue(port, kCFNumberSInt32Type, &p);
				
				/* Produce the ID as <host>:<port>. */
				peer = CFStringCreateWithFormat(alloc, NULL, _kCFStreamCONNECTURLFormat, host, p & 0x0000FFFF);
				
				if (peer) {
					
					UInt8 static_buffer[1024];
					UInt8* buffer = &static_buffer[0];
					CFIndex buffer_size = sizeof(static_buffer);
					
					/* Get the raw bytes to use as the ID. */
					buffer = _CFStringGetOrCreateCString(alloc, peer, static_buffer, &buffer_size, kCFStringEncodingUTF8);
					
					CFRelease(peer);
					
					/* Set the peer ID. */
					SSLSetPeerID(ssl, buffer, buffer_size);
					
					/* Did it. */
					set = TRUE;
					
					/* Clean up the allocation if made. */
					if (buffer != &static_buffer[0])
						CFAllocatorDeallocate(alloc, buffer);
				}
			}
			
			if (host) CFRelease(host);
			if (port) CFRelease(port);
		}
		
		if (!set) {
			UInt8 static_buffer[SOCK_MAXADDRLEN];
			struct sockaddr* sa = (struct sockaddr*)&static_buffer[0];
			socklen_t addrlen = sizeof(static_buffer);
			
			if (!getpeername(CFSocketGetNative(ctxt->_socket), sa, &addrlen)) {
				if (sa->sa_family == AF_INET) {
					in_port_t port = ((struct sockaddr_in*)sa)->sin_port;
					memmove(static_buffer, &(((struct sockaddr_in*)sa)->sin_addr), sizeof(((struct sockaddr_in*)sa)->sin_addr));
					memmove(static_buffer + sizeof(((struct sockaddr_in*)sa)->sin_addr),  &port, sizeof(port));
					SSLSetPeerID(ssl, static_buffer, sizeof(((struct sockaddr_in*)sa)->sin_addr) + sizeof(port));
				}
				else if (sa->sa_family == AF_INET6) {
					in_port_t port = ((struct sockaddr_in6*)sa)->sin6_port;
					memmove(static_buffer, &(((struct sockaddr_in6*)sa)->sin6_addr), sizeof(((struct sockaddr_in6*)sa)->sin6_addr));
					memmove(static_buffer + sizeof(((struct sockaddr_in6*)sa)->sin6_addr),  &port, sizeof(port));
					SSLSetPeerID(ssl, static_buffer, sizeof(((struct sockaddr_in6*)sa)->sin6_addr) + sizeof(port));
				}
			}
		}
	}
	
	/* Perform the SSL handshake. */
	result = SSLHandshake(ssl);
	
	/* If not blocking, can do something. */
	if (result != errSSLWouldBlock) {
		
		/* Was it noErr? */
		if (result) {
			ctxt->_error.error = result;
			ctxt->_error.domain = kCFStreamErrorDomainSSL;
		}
		
		/* Either way, it's done.  Mark the SSL bit for performance checks. */
		__CFBitSet(ctxt->_flags, kFlagBitUseSSL);
		__CFBitSet(ctxt->_flags, kFlagBitIsBuffered);
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSecurityHandshake_NoLock);
	}
}


/* static */ void
_PerformSecuritySendHandshake_NoLock(_CFSocketStreamContext* ctxt) {
	
	OSStatus result;
	CFIndex bytesWritten;
	
	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	/* Attempt to write. */
	result = SSLWrite(ssl, NULL, 0, (size_t*)(&bytesWritten));
	
	/* If didn't get a block, can do something. */
	if (result == errSSLWouldBlock)
		CFSocketEnableCallBacks(ctxt->_socket, kCFSocketWriteCallBack);
	else {
		
		/* Remove the "send" handshake. */
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSecuritySendHandshake_NoLock);
			
		/* Need to save real errors. */
		if (result) {
			ctxt->_error.error = result;
			ctxt->_error.domain = kCFStreamErrorDomainSSL;
		}
	}
}


/* static */ void
_SocketStreamSecurityClose_NoLock(_CFSocketStreamContext* ctxt) {
	
	/*
	** This function is required during close in order to fully flush ST
	** and dump the SSLContextRef.
	*/
	
	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	/* Attempt to close and flush out any bytes. */
	while (!ctxt->_error.error && (errSSLWouldBlock == SSLClose(ssl))) {
		  
		CFTypeRef loopAndMode[2] = {CFRunLoopGetCurrent(), _kCFStreamSocketSecurityClosePrivateMode};

		/* Add the current loop and the private mode to the list */
		_SchedulesAddRunLoopAndMode(ctxt->_sharedloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);

		/* Make sure to schedule all the schedulables on this loop and mode. */
		CFArrayApplyFunction(ctxt->_schedulables,
						   CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
						   (CFArrayApplierFunction)_SchedulablesScheduleApplierFunction,
						   loopAndMode);

		/* Unlock the context to allow things to fire */
		__CFSpinUnlock(&ctxt->_lock);

		/* Run the run loop just waiting for the end. */
		CFRunLoopRunInMode(_kCFStreamSocketSecurityClosePrivateMode, 1e+20, TRUE);

		/* Lock the context back up. */
		__CFSpinLock(&ctxt->_lock);

		/* Make sure to unschedule all the schedulables on this loop and mode. */
		CFArrayApplyFunction(ctxt->_schedulables,
						   CFRangeMake(0, CFArrayGetCount(ctxt->_schedulables)),
						   (CFArrayApplierFunction)_SchedulablesUnscheduleApplierFunction,
						   loopAndMode);

		/* Remove this loop and private mode from the list. */
		_SchedulesRemoveRunLoopAndMode(ctxt->_sharedloops, (CFRunLoopRef)loopAndMode[0], (CFStringRef)loopAndMode[1]);
	}

	/* Destroy the SSLContext. */
	SSLDisposeContext(ssl);

	/* Remove it from the properties for no touch. */
	CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
}


/* static */ Boolean
_SocketStreamSecuritySetContext_NoLock(_CFSocketStreamContext *ctxt, CFDataRef value) {

	CFDataRef wrapper = (CFDataRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
	SSLContextRef old = wrapper ? *((SSLContextRef*)CFDataGetBytePtr(wrapper)) : NULL;
	SSLContextRef security = value ? *((SSLContextRef*)CFDataGetBytePtr(value)) : NULL;
	
	if (old) {
		
		/* Can only do something if idle (not opened). */
		if (_SocketStreamSecurityGetSessionState_NoLock(ctxt) != kSSLIdle)
			return FALSE;
        
		/* If not setting the same, destroy the old. */
        if (security != old)
            SSLDisposeContext(old);
            
		/* Remove the one that was there. */
		CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
	}
	
	/* If not setting a new one, remove what's there. */
	if (!security) {
		
		/* Remove the one that was there. */
		CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySocketSSLContext);
		
		/* Get rid of the SSL handshake. */
		_SocketStreamRemoveHandshake_NoLock(ctxt, _PerformSecurityHandshake_NoLock);
		
		return TRUE;
	}
	
	/* Set the read/write functions on the context and set the reference. */
	if ((!(ctxt->_error.error = SSLSetIOFuncs(security, (SSLReadFunc)_SecurityReadFunc_NoLock, (SSLWriteFunc)_SecurityWriteFunc_NoLock))) &&
		(!(ctxt->_error.error = SSLSetConnection(security, (SSLConnectionRef)ctxt))))
	{
		/* Add the handshake to the list of things to perform. */
		Boolean result = _SocketStreamAddHandshake_NoLock(ctxt, _PerformSecurityHandshake_NoLock);
		
		/* If added, save the context. */
		if (result)
			CFDictionarySetValue(ctxt->_properties, kCFStreamPropertySocketSSLContext, value);
		
        return result;
	}
	
	/* Error was set, so set the domain. */
	ctxt->_error.domain = kCFStreamErrorDomainSSL;

	return FALSE;
}


/* static */ Boolean
_SocketStreamSecuritySetInfo_NoLock(_CFSocketStreamContext* ctxt, CFDictionaryRef settings) {

    SSLContextRef security = NULL;
	CFDataRef wrapper = NULL;
	
	/* Try to clear the existing, if any. */
    if (!_SocketStreamSecuritySetContext_NoLock(ctxt, NULL))
        return FALSE;

	/* If no new settings, done. */
    if (!settings)
		return TRUE;

    do {
        CFTypeRef value = CFDictionaryGetValue(settings, kCFStreamSSLIsServer);
		CFBooleanRef check = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate);
		
		/* Create a new SSLContext. */
        if (SSLNewContext((value && CFEqual(value, kCFBooleanTrue)), &security))
            break;
		
		/* Figure out the correct security level to set and set it. */
        value = CFDictionaryGetValue(settings, kCFStreamSSLLevel);
        if ((!value || CFEqual(value, kCFStreamSocketSecurityLevelNegotiatedSSL)) && SSLSetProtocolVersion(security, kTLSProtocol1))
            break;

        else if (value) {
                
            if (CFEqual(value, kCFStreamSocketSecurityLevelNone)) {
                SSLDisposeContext(security);
                return TRUE;
            }
            
            else if (CFEqual(value, kCFStreamSocketSecurityLevelSSLv2) && SSLSetProtocolVersion(security, kSSLProtocol2))
                break;
    
            else if (CFEqual(value, kCFStreamSocketSecurityLevelSSLv3) && SSLSetProtocolVersion(security, kSSLProtocol3Only))
                break;
    
            else if (CFEqual(value, kCFStreamSocketSecurityLevelTLSv1) && SSLSetProtocolVersion(security, kTLSProtocol1Only))
                break;
    
            else if (CFEqual(value, kCFStreamSocketSecurityLevelTLSv1SSLv3) &&
                    (SSLSetProtocolVersion(security, kTLSProtocol1) || SSLSetProtocolVersionEnabled(security, kSSLProtocol2, FALSE)))
            {
                    break;
            }
        }
		
		/* If old property for cert auth was used, set lax now.  New settings override. */
        if (check && (check == kCFBooleanFalse)) {

            if (SSLSetAllowsExpiredRoots(security, TRUE))
                break;
            
            if (SSLSetAllowsAnyRoot(security, TRUE))
                break;
        }
		
		/* Set all the different properties based upon dictionary settings. */
        value = CFDictionaryGetValue(settings, kCFStreamSSLAllowsExpiredCertificates);
        if (value && CFEqual(value, kCFBooleanTrue) && SSLSetAllowsExpiredCerts(security, TRUE))
            break;

        value = CFDictionaryGetValue(settings, kCFStreamSSLAllowsExpiredRoots);
        if (value && CFEqual(value, kCFBooleanTrue) && SSLSetAllowsExpiredRoots(security, TRUE))
            break;

        value = CFDictionaryGetValue(settings, kCFStreamSSLAllowsAnyRoot);
        if (value && CFEqual(value, kCFBooleanTrue) && SSLSetAllowsAnyRoot(security, TRUE))
            break;

        value = CFDictionaryGetValue(settings, kCFStreamSSLValidatesCertificateChain);
        if (value && CFEqual(value, kCFBooleanFalse) && SSLSetEnableCertVerify(security, FALSE))
            break;

        value = CFDictionaryGetValue(settings, kCFStreamSSLCertificates);
        if (value && SSLSetCertificate(security, value))
			break;
		
		/* Get the peer name or figure out the peer name to use. */
        value = CFDictionaryGetValue(settings, kCFStreamSSLPeerName);
        if (!value) {
            
			CFStringRef name = (CFStringRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketPeerName);
			CFTypeRef lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
			
			/* Old property override. */
            if (check && (check == kCFBooleanFalse))
                value = kCFNull;
			
			/*
            ** **FIXME** Once the new improved CONNECT stuff is in, peer name override
            ** should go away.
			*/
            else if (name)
                value = name;
            
			/* Try to get the name of the host from the CFHost or CFNetService. */
            else if (lookup) {
                
                CFArrayRef names = CFHostGetNames((CFHostRef)lookup, NULL);
                if (names)
                    value = CFArrayGetValueAtIndex(names, 0);
            }
			
			else if ((lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService)))
				value = CFNetServiceGetTargetHost((CFNetServiceRef)lookup);
        }
        
		/* If got a name, try to set it. */
        if (value) {
			
			/* kCFNull value means don't check the peer name. */
            if (CFEqual(value, kCFNull)) {
				if (SSLSetPeerDomainName(security, NULL, 0))
					break;
			}
            
			/* Pull out the bytes and set 'cause ST doesn't do CFString's for this. */
            else {
                
                OSStatus err;
                UInt8 static_buffer[1024];
                UInt8* buffer = &static_buffer[0];
				CFIndex buffer_size = sizeof(static_buffer);
				CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
				
				buffer = _CFStringGetOrCreateCString(alloc, value, static_buffer, &buffer_size, kCFStringEncodingUTF8);
				
				/* After pulling out the bytes, set the peer name. */
                err = SSLSetPeerDomainName(security, (const char*)buffer, *((size_t*)(&buffer_size)));
    
				/* Clean up the allocation if made. */
                if (buffer != &static_buffer[0])
                    CFAllocatorDeallocate(alloc, buffer);
                
                if (err)
                    break;
            }
        }
        
		/* Wrap the SSLContextRef as a CFData. */
		wrapper = CFDataCreate(CFGetAllocator(ctxt->_properties), (void*)&security, sizeof(security));
		
		/* Set the property. */
        if (!wrapper || !_SocketStreamSecuritySetContext_NoLock(ctxt, wrapper))
            break;
        
		CFRelease(wrapper);
		
        return TRUE;
        
    } while(1);

	/* Clean up the SSLContextRef on failure. */
    if (security)
        SSLDisposeContext(security);
	
	if (wrapper)
		CFRelease(wrapper);
    
    return FALSE;
}


/* static */ Boolean
_SocketStreamSecuritySetAuthenticatesServerCertificates_NoLock(_CFSocketStreamContext* ctxt, CFBooleanRef authenticates) {
	
	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	do {
		CFTypeRef value = NULL;
		CFStringRef name = (CFStringRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertySocketPeerName);
		CFTypeRef lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost);
		
		/* Set lax if turning off */
		if (SSLSetAllowsExpiredRoots(ssl, authenticates ? FALSE : TRUE))
			break;
		
		/* Set lax if turning off */
		if (SSLSetAllowsAnyRoot(ssl, authenticates ? FALSE : TRUE))
			break;
		
		/* Set peer name as kCFNull if turning off. */
		if (authenticates == kCFBooleanFalse)
			value = kCFNull;
		
		/*
		** **FIXME** Once the new improved CONNECT stuff is in, peer name override
		** should go away.
		*/
		else if (name)
			value = name;
		
		/* Figure out the proper peer name for CFHost or CFNetService. */
		else if (lookup) {
			
			CFArrayRef names = CFHostGetNames((CFHostRef)lookup, NULL);
			if (names)
				value = CFArrayGetValueAtIndex(names, 0);
		}
		
		else if ((lookup = (CFTypeRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService)))
			value = CFNetServiceGetTargetHost((CFNetServiceRef)lookup);
		
		/* No value is a failure. */
		if (!value) break;
		
		/* kCFNull means no peer name check */
		if (CFEqual(value, kCFNull)) {
			if (SSLSetPeerDomainName(ssl, NULL, 0))
				break;
		}
		
		/* Pull out the bytes and set 'cause ST doesn't do CFString's for this. */
		else {
			
			OSStatus err;
			UInt8 static_buffer[1024];
			UInt8* buffer = &static_buffer[0];
			CFIndex buffer_size = sizeof(static_buffer);
			CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
				
			buffer = _CFStringGetOrCreateCString(alloc, value, static_buffer, &buffer_size, kCFStringEncodingUTF8);
			
			err = SSLSetPeerDomainName(ssl, (const char*)buffer, *((size_t*)(&buffer_size)));
			
			if (err)
				break;
		}
		
		return TRUE;
		
	} while (1);
	
	return FALSE;
}


/* static */ CFStringRef
_SecurityGetProtocol(SSLContextRef security) {

	SSLProtocol version = kSSLProtocolUnknown;
	
	/* Attempt to get the negotiated protocol */
	SSLGetNegotiatedProtocolVersion(security, &version);
	if (kSSLProtocolUnknown == version)
		SSLGetProtocolVersion(security, &version);
	
	/* Map the protocol from ST to CFSocketStream property values. */
	switch (version) {
	
		case kSSLProtocol2:
			return kCFStreamSocketSecurityLevelSSLv2;
			
		case kSSLProtocol3Only:
			return kCFStreamSocketSecurityLevelSSLv3;
			
		case kTLSProtocol1Only:
			return kCFStreamSocketSecurityLevelTLSv1;
			
		case kSSLProtocol3:
		case kTLSProtocol1:
		default:
        {
            Boolean enabled;
            
            if (!SSLGetProtocolVersionEnabled(security, kSSLProtocol2, &enabled) && !enabled)
                return kCFStreamSocketSecurityLevelTLSv1SSLv3;
            
            return kCFStreamSocketSecurityLevelNegotiatedSSL;
        }
	}
}


/* static */ SSLSessionState
_SocketStreamSecurityGetSessionState_NoLock(_CFSocketStreamContext* ctxt) {

	SSLContextRef ssl = *((SSLContextRef*)CFDataGetBytePtr((CFDataRef)CFDictionaryGetValue(ctxt->_properties,
																						   kCFStreamPropertySocketSSLContext)));
	
	/*
	** Slight fib to return Aborted if the SSL call fails, but if it does fail things are
	** quite hosed, so it's not such a lie.
	*/
	SSLSessionState state;
	return !SSLGetSessionState(ssl, &state) ? state : kSSLAborted;
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif

/* extern */ void 
CFStreamCreatePairWithSocketToCFHost(CFAllocatorRef alloc, CFHostRef host, UInt32 port,
									 CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
	_CFSocketStreamContext* ctxt;
	
	/* NULL off the read stream if given */
	if (readStream) *readStream = NULL;
	
	/* Do the same to the write stream */
	if (writeStream) *writeStream = NULL;
	
	/* Create the context for the new socket stream. */
	ctxt = _SocketStreamCreateContext(alloc);
	
	/* Set up the rest if successful. */
	if (ctxt) {
		
		CFNumberRef num;
		
		port = port & 0x0000FFFF;
		num = CFNumberCreate(alloc, kCFNumberSInt32Type, &port);
		
		/* If the port wasn't created, just kill everything. */
		if (!num)				
			_SocketStreamDestroyContext_NoLock(alloc, ctxt);
		
		else {
			
			/* Add the peer host and port for connecting later. */
			CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost, host);
			CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySocketRemotePort, num);
			
			/* Create the read stream if the client asked for it. */
			if (readStream) {
				*readStream = CFReadStreamCreate(alloc, &kSocketReadStreamCallBacks, ctxt);
				ctxt->_clientReadStream = *readStream;
			}
			
			/* Create the write stream if the client asked for it. */
			if (writeStream) {
				*writeStream = CFWriteStreamCreate(alloc, &kSocketWriteStreamCallBacks, ctxt);
				ctxt->_clientWriteStream = *writeStream;
			}
			
			if (readStream && *readStream && writeStream && *writeStream)
				__CFBitSet(ctxt->_flags, kFlagBitShared);
		}
		
		/* Release the port if it was created. */
		if (num) CFRelease(num);
	}
}


/* extern */ void 
CFStreamCreatePairWithSocketToNetService(CFAllocatorRef alloc, CFNetServiceRef service,
										 CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
	_CFSocketStreamContext* ctxt;
	
	/* NULL off the read stream if given */
	if (readStream) *readStream = NULL;
	
	/* Do the same to the write stream */
	if (writeStream) *writeStream = NULL;
	
	/* Create the context for the new socket stream. */
	ctxt = _SocketStreamCreateContext(alloc);
	
	/* Set up the rest if successful. */
	if (ctxt) {
		
		/* Add the peer service for connecting later. */
		CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySocketRemoteNetService, service);
		
		/* Create the read stream if the client asked for it. */
		if (readStream) {
			*readStream = CFReadStreamCreate(alloc, &kSocketReadStreamCallBacks, ctxt);
			ctxt->_clientReadStream = *readStream;
		}
		
		/* Create the write stream if the client asked for it. */
		if (writeStream) {
			*writeStream = CFWriteStreamCreate(alloc, &kSocketWriteStreamCallBacks, ctxt);
			ctxt->_clientWriteStream = *writeStream;
		}
		
		if (readStream && *readStream && writeStream && *writeStream)
			__CFBitSet(ctxt->_flags, kFlagBitShared);
	}
}


/* extern */ Boolean 
CFSocketStreamPairSetSecurityProtocol(CFReadStreamRef socketReadStream, CFWriteStreamRef socketWriteStream,
									  CFStreamSocketSecurityProtocol securityProtocol)
{
	Boolean result = FALSE;
	CFStringRef value = NULL;
	
	/* Map the old security levels to the new property values */
	switch (securityProtocol) {
	
		case kCFStreamSocketSecurityNone:
			value = kCFStreamSocketSecurityLevelNone;
			break;
			
		case kCFStreamSocketSecuritySSLv2:
			value = kCFStreamSocketSecurityLevelSSLv2;
			break;
			
		case kCFStreamSocketSecuritySSLv3:
			value = kCFStreamSocketSecurityLevelSSLv3;
			break;
			
		case kCFStreamSocketSecuritySSLv23:
			value = kCFStreamSocketSecurityLevelNegotiatedSSL;
			break;
			
		case kCFStreamSocketSecurityTLSv1:
			value = kCFStreamSocketSecurityLevelTLSv1;
			break;
			
		default:
			return result;  /* Early bail because of bad value */
	}
	
	/* Try setting on the read stream first */
	if (socketReadStream) {
	
		result = CFReadStreamSetProperty(socketReadStream,
										 kCFStreamPropertySocketSecurityLevel,
										 value);
	}
	
	/* If there was no read stream, try the write stream */
	else if (socketWriteStream) {
	
		result = CFWriteStreamSetProperty(socketWriteStream,
										  kCFStreamPropertySocketSecurityLevel,
										  value);
	}
	
	return result;
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (SPI)
#endif


extern void
_CFStreamCreatePairWithCFSocketSignaturePieces(CFAllocatorRef alloc, SInt32 protocolFamily, SInt32 socketType,
											   SInt32 protocol, CFDataRef address, CFReadStreamRef* readStream,
											   CFWriteStreamRef* writeStream)
{
	_CFSocketStreamContext* ctxt;

	/* NULL off the read stream if given */
	if (readStream) *readStream = NULL;
	
	/* Do the same to the write stream */
	if (writeStream) *writeStream = NULL;
	
	/* Create the context for the new socket stream. */
	ctxt = _SocketStreamCreateContext(alloc);
	
	/* Set up the rest if successful. */
	if (ctxt) {
		
		CFDictionaryValueCallBacks cb = {0, NULL, NULL, NULL, NULL};
		
		/* Create a host from the address in order to connect later */
		CFHostRef h = CFHostCreateWithAddress(alloc, address);
		
		CFStringRef keys[] = {
			_kCFStreamSocketFamily,
			_kCFStreamSocketType,
			_kCFStreamSocketProtocol
		};
		SInt32 values[] = {
			protocolFamily,
			socketType,
			protocol
		};
		CFDictionaryRef props = CFDictionaryCreate(alloc,
												   (const void**)keys,
												   (const void**)values,
												   (sizeof(values) / sizeof(values[0])),
												   &kCFTypeDictionaryKeyCallBacks,
												   &cb);
		
		/* If the socket properties or host wasn't created, just kill everything. */
		if (!props || !h)
			_SocketStreamDestroyContext_NoLock(alloc, ctxt);
		
		else {

			CFDictionaryAddValue(ctxt->_properties, _kCFStreamPropertySocketFamilyTypeProtocol, props);
			
			/* Add the host as the far end for connecting later. */
			CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySocketRemoteHost, h);
			
			/* Create the read stream if the client asked for it. */
			if (readStream) {
				*readStream = CFReadStreamCreate(alloc, &kSocketReadStreamCallBacks, ctxt);
				ctxt->_clientReadStream = *readStream;
			}
			
			/* Create the write stream if the client asked for it. */
			if (writeStream) {
				*writeStream = CFWriteStreamCreate(alloc, &kSocketWriteStreamCallBacks, ctxt);
				ctxt->_clientWriteStream = *writeStream;
			}
			
			if (readStream && *readStream && writeStream && *writeStream)
				__CFBitSet(ctxt->_flags, kFlagBitShared);
		}
		
		/* Release the host if it was created. */
		if (h) CFRelease(h);

		/* Release the socket properties if it was created. */
		if (props) CFRelease(props);
	}
}


/* extern */ void 
CFStreamCreatePairWithNetServicePieces(CFAllocatorRef alloc, CFStringRef domain, CFStringRef serviceType,
									   CFStringRef name, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
	/* Create a service to call directly over to the real API. */
	CFNetServiceRef service = CFNetServiceCreate(alloc, domain, serviceType, name, 0);
	
	/* NULL off the read stream if given */
	if (readStream) *readStream = NULL;
		
	/* Do the same to the write stream */
	if (writeStream) *writeStream = NULL;
	
	if (service) {
		
		/* Call the real API if there is a service */
		CFStreamCreatePairWithSocketToNetService(alloc, service, readStream, writeStream);
		
		/* No longer needed */
		CFRelease(service);
	}
}


/* extern */ void
_CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
						  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream)
{
	/* Protect against a bad entry. */
    if (!readStream && !writeStream) return;
	
	/* NULL off the read stream if given */
	if (readStream) *readStream = NULL;
	
	/* Do the same to the write stream */
	if (writeStream) *writeStream = NULL;
	
	/* Being created with a host? */
	if (host) {
		
		/* Create a CFHost wrapper for stream creation */
		CFHostRef h = CFHostCreateWithName(alloc, host);
		
		/* Only make the streams if created host */
		if (h) {
			
			/* Create the streams */
			CFStreamCreatePairWithSocketToCFHost(alloc, h, port, readStream, writeStream);
			
			/* Don't need the host anymore. */
			CFRelease(h);
		}
	}
	
	/* Being created with a CFSocketSignature? */
	else if (sig) {
	
		/* Create the streams from the pieces of the CFSocketSignature. */
		_CFStreamCreatePairWithCFSocketSignaturePieces(alloc,
													   sig->protocolFamily,
													   sig->socketType,
													   sig->protocol,
													   sig->address,
													   readStream,
													   writeStream);
	}
	
	else {
		
		/* Create the context for the new socket stream. */
		_CFSocketStreamContext* ctxt = _SocketStreamCreateContext(alloc);
		
		/* Set up the rest if successful. */
		if (ctxt) {
				
			/* Create the wrapper for the socket.  Can't create the CFSocket until open (3784821). */
			CFDataRef wrapper = CFDataCreate(alloc, (const void*)(&s), sizeof(s));
			
			/* Mark as coming from a native socket handle */
			__CFBitSet(ctxt->_flags, kFlagBitCreatedNative);
			
			/* If the socket wasn't created, just kill everything. */
			if (!wrapper)				
				_SocketStreamDestroyContext_NoLock(alloc, ctxt);
			
			else {
				
				/* Save the native socket handle until open. */
				CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySocketNativeHandle, wrapper);
				
				/* 3938584 Make sure to release the wrapper now that it's been retained by the properties. */
				CFRelease(wrapper);
				
				/* Streams created with a native socket don't automatically close the underlying native socket. */
				CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanFalse);
				
				/* Create the read stream if the client asked for it. */
				if (readStream) {
					*readStream = CFReadStreamCreate(alloc, &kSocketReadStreamCallBacks, ctxt);
					ctxt->_clientReadStream = *readStream;
				}
				
				/* Create the write stream if the client asked for it. */
				if (writeStream) {
					*writeStream = CFWriteStreamCreate(alloc, &kSocketWriteStreamCallBacks, ctxt);
					ctxt->_clientWriteStream = *writeStream;
				}
				
				if (readStream && *readStream && writeStream && *writeStream)
					__CFBitSet(ctxt->_flags, kFlagBitShared);
			}			
		}
	}
}

