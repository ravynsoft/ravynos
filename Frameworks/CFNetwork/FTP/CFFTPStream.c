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
/* CFFTPStream.c
   Copyright 2000, Apple, Inc.  All rights reserved.
*/

#if 0
#pragma mark Includes
#endif
#include <CFNetwork/CFNetwork.h>
#include "CFNetworkInternal.h"			// for _CFNetConnectionCacheKey and friends

#include <CoreFoundation/CFStreamPriv.h>
#include "CFNetConnection.h"
#include <CFNetwork/CFFTPStream.h>
#include <CFNetwork/CFFTPStreamPriv.h>
#include <CoreFoundation/CFPriv.h>
#include <CFNetwork/CFSocketStreamPriv.h>
#include <CoreFoundation/CFPriv.h>
#include <CFNetwork/CFHTTPConnectionPriv.h>  // for the asynchronous proxy lookup
#include "CFNetworkSchedule.h"
#include <SystemConfiguration/SystemConfiguration.h>


#if 0
#pragma mark *Win32 Specifics
#endif
#if defined(__WIN32__)
#include <sys/param.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define SOCK_MAXADDRLEN 255

// Sockets and fds are not interchangeable on Win32, and have different error codes.
// These redefines assumes that in this file we only apply this error constant to socket ops.
#undef ENOTCONN
#define ENOTCONN WSAENOTCONN

#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL

#define     DT_UNKNOWN       0
#define     DT_FIFO          1
#define     DT_CHR           2
#define     DT_DIR           4
#define     DT_BLK           6
#define     DT_REG           8
#define     DT_LNK          10
#define     DT_SOCK         12
#define     DT_WHT          14

#else

#if 0
#pragma mark *Mach Specifics
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/dirent.h>
#include <netinet/tcp.h>
#endif


#if 0
#pragma mark -
#pragma mark Constants
#endif

/* extern */ const SInt32 kCFStreamErrorDomainFTP = 6;


#if 0
#pragma mark -
#pragma mark Constant Strings
#pragma mark *Stream Property Keys
#endif

// The following properties when set effect the key created for the
// connection cache, so their values should be stored in the "properties"
// dictionary on the ftp stream context.
CONST_STRING_DECL(kCFStreamPropertyFTPUserName, "kCFStreamPropertyFTPUserName")
CONST_STRING_DECL(kCFStreamPropertyFTPPassword, "kCFStreamPropertyFTPPassword")
CONST_STRING_DECL(kCFStreamPropertyFTPProxy, "kCFStreamPropertyFTPProxy")
CONST_STRING_DECL(kCFStreamPropertyFTPAttemptPersistentConnection, "kCFStreamPropertyFTPAttemptPersistentConnection")


// The following properties when set should NOT effect the key created
// for the connection cache, therefore their values are stored as bits
// or as ivars on the ftp stream context.
CONST_STRING_DECL(kCFStreamPropertyFTPUsePassiveMode, "kCFStreamPropertyFTPUsePassiveMode")
CONST_STRING_DECL(kCFStreamPropertyFTPFetchResourceInfo, "kCFStreamPropertyFTPFetchResourceInfo")
CONST_STRING_DECL(kCFStreamPropertyFTPFileTransferOffset, "kCFStreamPropertyFTPFileTransferOffset")
CONST_STRING_DECL(_kCFStreamPropertyFTPLogInOnly, "_kCFStreamPropertyFTPLogInOnly")  // SPI for connecting and logging in only
CONST_STRING_DECL(_kCFStreamPropertyFTPRemoveResource, "_kCFStreamPropertyFTPRemoveResource")  // SPI for removing the specified URL
CONST_STRING_DECL(_kCFStreamPropertyFTPNewResourceName, "_kCFStreamPropertyFTPNewResourceName")  // SPI for creating the specified URL
#ifdef __CONSTANT_CFSTRINGS__
#define kCFStreamPropertyFTPFetchNameList	CFSTR("kCFStreamPropertyFTPFetchNameList")
#else
static CONST_STRING_DECL(kCFStreamPropertyFTPFetchNameList, "kCFStreamPropertyFTPFetchNameList")  // SPI property key to perform a NLST instead of a LIST
#endif	/* __CONSTANT_CFSTRINGS__ */

// The following properties are copy only.
CONST_STRING_DECL(kCFStreamPropertyFTPResourceSize, "kCFStreamPropertyFTPResourceSize")

// The following property is internal to FTP and is used to cary over
// 407 responses in order to apply authentication to the new request.
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamPropertyFTPLastHTTPResponse	CFSTR("_kCFStreamPropertyFTPLastHTTPResponse")
#else
static CONST_STRING_DECL(_kCFStreamPropertyFTPLastHTTPResponse, "_kCFStreamPropertyFTPLastHTTPResponse")
#endif	/* __CONSTANT_CFSTRINGS__ */

#if 0
#pragma mark *Various Dictionary Keys
#endif

// Proxy dictionary keys
CONST_STRING_DECL(kCFStreamPropertyFTPProxyHost, "FTPProxy")
CONST_STRING_DECL(kCFStreamPropertyFTPProxyPort, "FTPPort")
CONST_STRING_DECL(kCFStreamPropertyFTPProxyPassword, "kCFStreamPropertyFTPProxyPassword")
CONST_STRING_DECL(kCFStreamPropertyFTPProxyUser, "kCFStreamPropertyFTPProxyUser")


// Resource info dictionary keys.
#define kResourceInfoItemCount 8L
CONST_STRING_DECL(kCFFTPResourceMode, "kCFFTPResourceMode")
CONST_STRING_DECL(kCFFTPResourceName, "kCFFTPResourceName")
CONST_STRING_DECL(kCFFTPResourceOwner, "kCFFTPResourceOwner")
CONST_STRING_DECL(kCFFTPResourceGroup, "kCFFTPResourceGroup")
CONST_STRING_DECL(kCFFTPResourceLink, "kCFFTPResourceLink")
CONST_STRING_DECL(kCFFTPResourceSize, "kCFFTPResourceSize")
CONST_STRING_DECL(kCFFTPResourceType, "kCFFTPResourceType")
CONST_STRING_DECL(kCFFTPResourceModDate, "kCFFTPResourceModDate")


#if 0
#pragma mark *Other Strings
#endif

// Scheme strings used for comparison in order to set up default
// information (e.g. port).
#ifdef __CONSTANT_CFSTRINGS__
#define kFTPSchemeString	CFSTR("ftp")
#define kFTPSSchemeString	CFSTR("ftps")
#define kSOCKS4SchemeString	CFSTR("socks4")
#define kSOCKS5SchemeString	CFSTR("socsk5")
#define kHTTPSchemeString	CFSTR("http")
#define kHTTPSSchemeString	CFSTR("https")
#else
static CONST_STRING_DECL(kFTPSchemeString, "ftp")
static CONST_STRING_DECL(kFTPSSchemeString, "ftps")
static CONST_STRING_DECL(kSOCKS4SchemeString, "socks4")
static CONST_STRING_DECL(kSOCKS5SchemeString, "socsk5")
static CONST_STRING_DECL(kHTTPSchemeString, "http")
static CONST_STRING_DECL(kHTTPSSchemeString, "https")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Anonymous username and password
#ifdef __CONSTANT_CFSTRINGS__
#define kAnonymousUserString		CFSTR("anonymous")
#define kAnonymousPasswordString	CFSTR("cfnetwork@apple.com")
#else
static CONST_STRING_DECL(kAnonymousUserString, "anonymous")
static CONST_STRING_DECL(kAnonymousPasswordString, "cfnetwork@apple.com")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Format for producing ftp proxy "username"
#ifdef __CONSTANT_CFSTRINGS__
#define kFTPProxyFormat			CFSTR("%@@%@")
#define kFTPProxyWithPortFormat	CFSTR("%@@%@:%ld")
#else
static CONST_STRING_DECL(kFTPProxyFormat, "%@@%@")
static CONST_STRING_DECL(kFTPProxyWithPortFormat, "%@@%@:%ld")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Method used when downloading over http
#ifdef __CONSTANT_CFSTRINGS__
#define kHTTPGETMethod	CFSTR("GET")
#else
static CONST_STRING_DECL(kHTTPGETMethod, "GET")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Used to strip HTML tags from directory listings.
#ifdef __CONSTANT_CFSTRINGS__
#define kHTMLTagOpen	CFSTR("<")
#define kHTMLTagClose	CFSTR(">")
#else
static CONST_STRING_DECL(kHTMLTagOpen, "<")
static CONST_STRING_DECL(kHTMLTagClose, ">")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Command format strings
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPUSERCommandString				CFSTR("USER %@\r\n")
#define kCFFTPPASSCommandString				CFSTR("PASS %@\r\n")
#define kCFFTPSYSTCommandString				CFSTR("SYST\r\n")
#define kCFFTPSITEDIRSTYLECommandString		CFSTR("SITE DIRSTYLE\r\n")
#define kCFFTPSITETRUTHCommandString		CFSTR("SITE TRUTH ON\r\n")
#define kCFFTPPWDCommandString				CFSTR("PWD\r\n")
#define kCFFTPTYPECommandString				CFSTR("TYPE I\r\n")
#define kCFFTPPASVCommandString				CFSTR("PASV\r\n")
#define kCFFTPEPSVCommandString				CFSTR("EPSV\r\n")
#define kCFFTPPORTCommandString				CFSTR("PORT %lu,%lu,%lu,%lu,%lu,%lu\r\n")
#define kCFFTPEPRTCommandString				CFSTR("EPRT |2|%x:%x:%x:%x:%x:%x:%x:%x|%lu|\r\n")
#define kCFFTPRESTCommandString				CFSTR("REST %lld\r\n")
#define kCFFTPSTATCommandString				CFSTR("STAT %@\r\n")
#define kCFFTPSIZECommandString				CFSTR("SIZE %@\r\n")
#define kCFFTPRETRCommandString				CFSTR("RETR %@\r\n")
#define kCFFTPNLSTCommandString				CFSTR("NLST %@\r\n")
#define kCFFTPCWDCommandString				CFSTR("CWD %@\r\n")
#define kCFFTPLISTCommandString				CFSTR("LIST\r\n")
#define kCFFTPSTORCommandString				CFSTR("STOR %@\r\n")
#define kCFFTPMKDCommandString				CFSTR("MKD %@\r\n")
#define kCFFTPRMDCommandString				CFSTR("RMD %@\r\n")
#define kCFFTPDELECommandString				CFSTR("DELE %@\r\n")
#define kCFFTPRNFRCommandString				CFSTR("RNFR %@\r\n")
#define kCFFTPRNTOCommandString				CFSTR("RNTO %@\r\n")
#else
static CONST_STRING_DECL(kCFFTPUSERCommandString, "USER %@\r\n")
static CONST_STRING_DECL(kCFFTPPASSCommandString, "PASS %@\r\n")
static CONST_STRING_DECL(kCFFTPSYSTCommandString, "SYST\r\n")
static CONST_STRING_DECL(kCFFTPSITEDIRSTYLECommandString, "SITE DIRSTYLE\r\n")
static CONST_STRING_DECL(kCFFTPSITETRUTHCommandString, "SITE TRUTH ON\r\n")
static CONST_STRING_DECL(kCFFTPPWDCommandString, "PWD\r\n")
static CONST_STRING_DECL(kCFFTPTYPECommandString, "TYPE I\r\n")
static CONST_STRING_DECL(kCFFTPPASVCommandString, "PASV\r\n")
static CONST_STRING_DECL(kCFFTPEPSVCommandString, "EPSV\r\n")
static CONST_STRING_DECL(kCFFTPPORTCommandString, "PORT %lu,%lu,%lu,%lu,%lu,%lu\r\n")
static CONST_STRING_DECL(kCFFTPEPRTCommandString, "EPRT |2|%x:%x:%x:%x:%x:%x:%x:%x|%lu|\r\n")
static CONST_STRING_DECL(kCFFTPRESTCommandString, "REST %lld\r\n")
static CONST_STRING_DECL(kCFFTPSTATCommandString, "STAT %@\r\n")
static CONST_STRING_DECL(kCFFTPSIZECommandString, "SIZE %@\r\n")
static CONST_STRING_DECL(kCFFTPRETRCommandString, "RETR %@\r\n")
static CONST_STRING_DECL(kCFFTPNLSTCommandString, "NLST %@\r\n")
static CONST_STRING_DECL(kCFFTPCWDCommandString, "CWD %@\r\n")
static CONST_STRING_DECL(kCFFTPLISTCommandString, "LIST\r\n")
static CONST_STRING_DECL(kCFFTPSTORCommandString, "STOR %@\r\n")
static CONST_STRING_DECL(kCFFTPMKDCommandString, "MKD %@\r\n")
static CONST_STRING_DECL(kCFFTPRMDCommandString, "RMD %@\r\n")
static CONST_STRING_DECL(kCFFTPDELECommandString, "DELE %@\r\n")
static CONST_STRING_DECL(kCFFTPRNFRCommandString, "RNFR %@\r\n")
static CONST_STRING_DECL(kCFFTPRNTOCommandString, "RNTO %@\r\n")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Path format for combining root with url path
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPPathFormatString	CFSTR("%@%@")
#else
static CONST_STRING_DECL(kCFFTPPathFormatString, "%@%@")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Path for when only a host is given
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPRootPathString	CFSTR("/")
#else
static CONST_STRING_DECL(kCFFTPRootPathString, "/")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Path prefix indicating full path (no root)
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPForcedRootPathPrefix	CFSTR("//")
#else
static CONST_STRING_DECL(kCFFTPForcedRootPathPrefix, "//");
#endif	/* __CONSTANT_CFSTRINGS__ */

// Comparison strings for determining DIRSTYLE
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPWindowsNTSystemString		CFSTR("Windows_NT")
#define kCFFTPMSDOSSystemString			CFSTR("MSDOS-like directory output is on")
#else
static CONST_STRING_DECL(kCFFTPWindowsNTSystemString, "Windows_NT")
static CONST_STRING_DECL(kCFFTPMSDOSSystemString, "MSDOS-like directory output is on")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Comparison string for determining TRUTH
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPOSXSystemString	CFSTR("Mac OS X Server")
#else
static CONST_STRING_DECL(kCFFTPOSXSystemString, "Mac OS X Server")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Run loop mode for waiting for stream to open
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPStreamOpenCompleted	CFSTR("_FTPStreamOpenCompleted")
#else
static CONST_STRING_DECL(kCFFTPStreamOpenCompleted, "_FTPStreamOpenCompleted")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Strings used for CopyDescription function
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPStreamDescriptionFormat	CFSTR("<FTPStream %p>{%@, url = %@, flags = 0x%x }")
#define kCFFTPStreamUploadDescription	CFSTR("upload")
#define kCFFTPStreamDownloadDescription	CFSTR("download")
#else
#error crap
static CONST_STRING_DECL(kCFFTPStreamDescriptionFormat, "<FTPStream %p>{%@, url = %@, flags = 0x%x }")
static CONST_STRING_DECL(kCFFTPStreamUploadDescription, "upload")
static CONST_STRING_DECL(kCFFTPStreamDownloadDescription, "download")
#endif	/* __CONSTANT_CFSTRINGS__ */

// It's sad that this is really needed.  Used for escape sequences in URL's.
#ifdef __CONSTANT_CFSTRINGS__
#define kCFFTPStreamEmptyString	CFSTR("")
#else
static CONST_STRING_DECL(kCFFTPStreamEmptyString, "")
#endif	/* __CONSTANT_CFSTRINGS__ */


#if 0
#pragma mark -
#pragma mark Enum Values
#endif

// State machine states.  These really indicate the command
// on which the state machine is waiting for a reply.
// NOTE:  THESE ARE ORDER DEPENDENT!!!
//	See _AdvanceStateMachine and _FTPConnectionRequestStateChanged
typedef enum {
    kFTPStateConnect = 0,
    kFTPStateUSER,
    kFTPStatePASS,
    kFTPStateSYST,
    kFTPStateSITEDIRSTYLE,
    kFTPStateSITETRUTH,
    kFTPStatePWD,
    kFTPStateTYPE,
    kFTPStateIdle,
    kFTPStateCWD,
    kFTPStatePASV,
    kFTPStatePORT,
    kFTPStateSTAT,
    kFTPStateSIZE,
    kFTPStateREST,
    kFTPStateRETR,		// The following must be last.
    kFTPStateNLST,
    kFTPStateLIST,
    kFTPStateSTOR,
    kFTPStateMKD,
    kFTPStateRMD,
    kFTPStateDELE,
    kFTPStateRNFR,
    kFTPStateRNTO
} _CFFTPStreamState;

enum {
    // _CFFTPStreamContext flags
    kFlagBitPerformPASV = 0,	// Passive is on by default
	kFlagBitDidSetPassiveBit,	// Set if SetProperty of the passive property
    kFlagBitPerformSTAT,	// Get the size of the target object
    kFlagBitPerformNLST,	// Perform NLIST instead of LIST command
    kFlagBitIsHTTPRequest,	// Indicates that the given request is ftp over http
    kFlagBitReadHTTPResponse,	// Already consulted the http response from server
    kFlagBit407TriedOnce,	// Already attempted a 407 retry
    kFlagBitPerformUpload,	// Indicates a write stream
    kFlagBitRemoveResource,	// Used to remove the resource pointed to by the url
    kFlagBitLogInOnly,		// Used by csmount to "test" connect
    kFlagBitGotError,		// Used to protect when dequeueing as a result of an
                       		// error and trying to requeue orphaned items.
    
    // _CFFTPNetConnection flags
    kFlagBitMultiline	= 0,	// In the process of a multiline response
    kFlagBitReturnToIdle,	// Return back to the idle state before proceeding to next request
    kFlagBitIsXServer,		// This connection is to an OS X server
    kFlagBitLeftForDead,	// Connection has no pending requests but is still in cache
    kFlagBitHTTPLitmus,		// Indicates having sent the early HTTP test
    
    // Other constants
    kBufferGrowthSize	= 2048,	// Growth factor used for reading responses to commands
    kFTPTimeoutInSeconds = 180,	// Timeout for stale connections sitting in the connection cache
    
    // CFNetConnection types for cache key
    kCFNetConnectionTypeFTP,
    kCFNetConnectionTypeFTPS,
    kCFNetConnectionTypeFTPProxy,
    kCFNetConnectionTypeFTPSProxy
};


#if 0
#pragma mark -
#pragma mark CFStream Context
#endif

typedef struct {
    
    UInt32			_flags;
    
    CFURLRef			_url;
    CFURLRef			_newUrl;
    CFTypeRef			_dataStream;
    CFTypeRef			_userStream;	// CFReadStreamRef if GET; CFWriteStreamRef if PUT
    
    CFStreamError		_error;		// Currently just use for fallback from proxy to proxy
    
    CFSocketRef			_server;
    
    CFDictionaryRef		_attributes;
    long long			_offset;
    
    CFMutableArrayRef		_runloops;
    
    CFMutableDictionaryRef	_properties;

    CFReadStreamRef		_proxyStream;
    CFArrayRef			_proxies;
    CFIndex			_current;
    
    _CFNetConnectionRef		_connection;
    
} _CFFTPStreamContext;


#if 0
#pragma mark -
#pragma mark CFNetConnection Context
#endif

// This information needs to be carried over from request to request on a connection.	
typedef struct {
    
    UInt32			_flags;
    
    _CFNetConnectionCacheKey	_key;		// Needed for stream creation
    
    UInt32			_result;
    _CFFTPStreamState		_state;
    CFStringRef			_root;		// This is the root directory (base url)
    
    CFIndex			_recvCount;	// Number of relevant bytes in the receive buffer
    CFIndex			_sendCount;	// Number of relevant bytes in the send buffer
    
    CFMutableDataRef		_recvBuffer;	// Used for leftovers (buffer can be larger than byte count)
    CFMutableDataRef		_sendBuffer;	// Used for leftovers (buffer can be larger than byte count)
    
} _CFFTPNetConnectionContext;


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _FTPStreamFinalize(CFTypeRef stream, _CFFTPStreamContext* ctxt);
static CFStringRef _FTPStreamCopyDescription(CFTypeRef stream, _CFFTPStreamContext* ctxt);
static Boolean _FTPStreamOpen(CFTypeRef stream, CFStreamError* error, Boolean* openComplete, _CFFTPStreamContext* ctxt);
static Boolean _FTPStreamOpenCompleted(CFTypeRef stream, CFStreamError* error, _CFFTPStreamContext* ctxt);
static CFIndex _FTPStreamRead(CFReadStreamRef stream, UInt8* buffer, CFIndex bufferLength, CFStreamError* error, Boolean* atEOF, _CFFTPStreamContext* ctxt);
static Boolean _FTPStreamCanRead(CFReadStreamRef stream, _CFFTPStreamContext* ctxt);
static CFIndex _FTPStreamWrite(CFWriteStreamRef stream, const UInt8* buffer, CFIndex bufferLength, CFStreamError* error, _CFFTPStreamContext* ctxt);
static Boolean _FTPStreamCanWrite(CFWriteStreamRef stream, _CFFTPStreamContext* ctxt);
static void _FTPStreamClose(CFTypeRef stream, _CFFTPStreamContext* ctxt);
static CFTypeRef _FTPStreamCopyProperty(CFTypeRef stream, CFStringRef propertyName, _CFFTPStreamContext* ctxt);
static Boolean _FTPStreamSetProperty(CFTypeRef stream, CFStringRef propertyName, CFTypeRef propertyValue, _CFFTPStreamContext* ctxt);
static void _FTPStreamSchedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFFTPStreamContext* ctxt);
static void _FTPStreamUnschedule(CFTypeRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, _CFFTPStreamContext* ctxt);


static const void* _CFFTPNetConnectionContextCreate(CFAllocatorRef alloc, const _CFFTPNetConnectionContext* template);
static void _CFFTPNetConnectionContextFinalize(CFAllocatorRef alloc, const _CFFTPNetConnectionContext* ctxt);
static CFStreamError _FTPConnectionCreateStreams(CFAllocatorRef alloc, const _CFFTPNetConnectionContext* key, CFWriteStreamRef* requestStream, CFReadStreamRef* responseStream);
static void _FTPConnectionRequestStateChanged(_CFFTPStreamContext* ctxt, int newState, CFStreamError *err, _CFNetConnectionRef connection, _CFFTPNetConnectionContext* netCtxt);
static void _FTPConnectionTransmitRequest(_CFFTPStreamContext* ctxt, _CFNetConnectionRef connection, _CFFTPNetConnectionContext* netCtxt);
static void _FTPConnectionReceiveResponse(_CFFTPStreamContext* ctxt, _CFNetConnectionRef connection, _CFFTPNetConnectionContext* netCtxt);
static void _FTPResponseStreamCallBack(_CFFTPStreamContext* ctxt, CFReadStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, _CFFTPNetConnectionContext* netCtxt);
static void _FTPRequestStreamCallBack(_CFFTPStreamContext* ctxt, CFWriteStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, _CFFTPNetConnectionContext* netCtxt);
static CFArrayRef _FTPRunLoopArrayCallBack(_CFFTPStreamContext *ctxt, _CFNetConnectionRef conn, _CFFTPNetConnectionContext *netCtxt);

static Boolean _IsRoot(CFURLRef url);
static void _FTPConnectionCacheCreate(void);
static void _FTPConnectionCacheExpiration(_CFNetConnectionRef conn, CFDateRef expiration, CFMutableArrayRef list);
static void _SetSOCKS4ProxyInformation(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFURLRef proxyUrl);
static void _SetSOCKS5ProxyInformation(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFURLRef proxyUrl);
static void _StartHTTPRequest(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFStreamError* error, CFURLRef proxyUrl);
static Boolean _ProcessHTTPResponse(_CFFTPStreamContext* ctxt, CFStreamError* error);
static void _RollOverHTTPRequest(_CFFTPStreamContext* ctxt, CFStreamError* error);
static void _CFStreamSocketCreatedCallBack(int fd, void* ctxt);
static void _DataStreamCallBack(CFTypeRef stream, CFStreamEventType type, _CFFTPStreamContext* ctxt);
static void _ReleaseDataReadStream(_CFFTPStreamContext* ctxt);
static void _SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, _CFFTPStreamContext* ctxt);
static void _StreamPropertyApplier(CFTypeRef key, CFTypeRef value, CFTypeRef stream);
static Boolean _PASVAddressParser(const UInt8* buffer, struct sockaddr_in* saddr);
static Boolean _EPSVPortParser(const UInt8* buffer, struct sockaddr_in6* saddr);
static u_char _GetProtocolFamily(_CFFTPStreamContext* ctxt, UInt8* buffer);
static Boolean _CreateListenerForContext(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt);
static void _StartTransfer(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _InvalidateServer(_CFFTPStreamContext* ctxt);
static CFStringRef _CreatePathForContext(CFAllocatorRef alloc, _CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);

static void _ReportError(_CFFTPStreamContext* ctxt, CFStreamError* error);
static void _ConnectionComplete(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _WriteCommand(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, CFStringRef cmd);
static void _HandleResponse(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static CFURLRef _ConvertToCFFTPHappyURL(CFURLRef url);
static Boolean _ReadModeBits(const UInt8* str, int* mode);
static Boolean _ReadSize(const UInt8* str, UInt64* size);
static CFStringRef _CFStringCreateCopyWithStrippedHTML(CFAllocatorRef alloc, CFStringRef theString);
static const UInt8* _CFFTPGetDateTimeFunc(CFAllocatorRef alloc, const UInt8* str, CFIndex length, CFDateRef* date);
static void _ProxyStreamCallBack(CFReadStreamRef proxyStream, _CFFTPStreamContext* ctxt);
static CFIndex _FindLine(const UInt8 *buffer, CFIndex bufferLength, const UInt8** start, const UInt8** end);

static void _AdvanceStateMachine(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length, Boolean isMultiLine);
static void _HandleConnect(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandleUsername(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandlePassword(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleSystem(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandleSiteDirStyle(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandleSiteTruth(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandlePrintWorkingDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandleType(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleChangeDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandlePassive(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandlePort(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleRestart(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleStat(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length, Boolean isMultiLine);
static void _HandleSize(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length);
static void _HandleRetrieve(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleNameList(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleList(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleStore(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleMakeDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleRemoveDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleDelete(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleRenameFrom(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _HandleRenameTo(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);
static void _StartProcess(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt);


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern void _CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
									  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream);


#if 0
#pragma mark -
#pragma mark Globals
#endif

static CFMutableDictionaryRef gFTPConnectionTimeouts = NULL;
static CFNetConnectionCacheRef gFTPConnectionCache = NULL;
static CFSpinLock_t gFTPSpinLock = 0;
static _CFNetConnectionCallBacks* _kFTPConnectionCallBacks = NULL;


#if 0
#pragma mark -
#pragma mark CFStream Callback Functions
#endif


/* static */ void
_FTPStreamFinalize(CFTypeRef stream, _CFFTPStreamContext* ctxt) {

    _FTPStreamClose(stream, ctxt);

    CFRelease(ctxt->_url);

    if (ctxt->_newUrl)
        CFRelease(ctxt->_newUrl);

    
    
    CFRelease(ctxt->_runloops);
    CFRelease(ctxt->_properties);

    if (ctxt->_proxies)
        CFRelease(ctxt->_proxies);
    
    if (ctxt->_attributes)
        CFRelease(ctxt->_attributes);

    CFAllocatorDeallocate(CFGetAllocator(stream), ctxt);
}


/* static */ CFStringRef
_FTPStreamCopyDescription(CFTypeRef stream, _CFFTPStreamContext* ctxt) {

    // **FIXME** Should display whether it's reading or writing.
    // **FIXME** Should display URL.

    return CFStringCreateWithFormat(CFGetAllocator(stream),
                                    NULL,
                                    kCFFTPStreamDescriptionFormat,
                                    (int)stream,
                                    __CFBitIsSet(ctxt->_flags, kFlagBitPerformUpload) ? kCFFTPStreamUploadDescription : kCFFTPStreamDownloadDescription,
                                    ctxt->_url,
                                    ctxt->_flags);
}


/* static */ Boolean
_FTPStreamOpen(CFTypeRef stream, CFStreamError* error, Boolean* openComplete,
			   _CFFTPStreamContext* ctxt)
{
    CFAllocatorRef alloc = CFGetAllocator(stream);
    
    UInt32 type = kCFNetConnectionTypeFTP;
    
    _CFFTPNetConnectionContext template;

    CFTypeRef proxyUrl = NULL;
    CFStringRef proxyHost = NULL;

    CFBooleanRef persistent = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties,
                                                                kCFStreamPropertyFTPAttemptPersistentConnection);
    Boolean usePersistent = (persistent && CFEqual(persistent, kCFBooleanFalse)) ? FALSE : TRUE;

    CFStringRef scheme = CFURLCopyScheme(ctxt->_url);
    SInt32 port = CFURLGetPortNumber(ctxt->_url);
    CFStringRef host = CFURLCopyHostName(ctxt->_url);    

    if (!ctxt->_proxies) {

        
        CFDictionaryRef info = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxy);
        if (info) {
			CFRetain(info);
            CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyFTPProxy);
		}
        else {
            info = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
            if (info) {
				CFRetain(info);
                CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
			}
        }

        ctxt->_proxies = _CFNetworkFindProxyForURLAsync(NULL,			// Take the scheme from the url
                                                        ctxt->_url,
                                                        NULL,	 		// Take the host from the url
                                                        info,			// Proxy dictionary.
                                                        (_CFProxyStreamCallBack)_ProxyStreamCallBack,
                                                        ctxt,
                                                        &ctxt->_proxyStream);
        
		if (info)
			CFRelease(info);
		
        ctxt->_current = 0;

        // If ctxt->_proxies is NULL, there has been an error and
        // need to error out the stream now.
        if (!ctxt->_proxies) {
            
            CFRelease(scheme);
            CFRelease(host);

            if (ctxt->_proxyStream) {
                
				_CFTypeScheduleOnMultipleRunLoops(ctxt->_proxyStream, ctxt->_runloops);

                *openComplete = FALSE;
                return TRUE;
            }
            else {
                *openComplete = TRUE;
		error->domain = _kCFStreamErrorDomainNativeSockets;
                error->error = ENOTCONN;
                return FALSE;
            }
        }
    }

    // If ctxt->_current is beyond the end,  there has been an
    // error and must error out the stream now.
    if (ctxt->_current == CFArrayGetCount(ctxt->_proxies)) {
        CFRelease(scheme);
        CFRelease(host);
        *openComplete = TRUE;
        if (ctxt->_error.error)
            *error = ctxt->_error;
        else {
            error->domain = _kCFStreamErrorDomainNativeSockets;
            error->error = ENOTCONN;
        }
        return FALSE;
    }

    ctxt->_error.domain = 0;
    ctxt->_error.error = 0;
    
    CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyFTPProxy);
    CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
    __CFBitClear(ctxt->_flags, kFlagBitReadHTTPResponse);
    __CFBitClear(ctxt->_flags, kFlagBitIsHTTPRequest);
    
    proxyUrl = (CFTypeRef)CFArrayGetValueAtIndex(ctxt->_proxies, ctxt->_current);
    
    if (!CFEqual(proxyUrl, kCFNull)) {

        CFStringRef pScheme = CFURLCopyScheme(proxyUrl);

        if (CFEqual(pScheme, kFTPSchemeString)) {

            CFStringRef pHost = CFURLCopyHostName(proxyUrl);
            SInt32 p = CFURLGetPortNumber(proxyUrl);
            CFNumberRef pPort = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
            CFMutableDictionaryRef pInfo = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            
            CFDictionaryAddValue(pInfo, kCFStreamPropertyFTPProxyHost, pHost);
            CFDictionaryAddValue(pInfo, kCFStreamPropertyFTPProxyPort, pPort);
            
            CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertyFTPProxy, pInfo);
            CFRelease(pInfo);
            
            proxyHost = pHost;
            
            CFRelease(pHost);
            CFRelease(pPort);
        }

        else if (CFEqual(pScheme, kSOCKS4SchemeString))
            _SetSOCKS4ProxyInformation(alloc, ctxt, proxyUrl);

        else if (CFEqual(pScheme, kSOCKS5SchemeString))
            _SetSOCKS5ProxyInformation(alloc, ctxt, proxyUrl);
		
        else if ((CFStringCompare(pScheme, kHTTPSchemeString, kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
                 (CFStringCompare(pScheme, kHTTPSSchemeString, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
        {
            CFRelease(pScheme);
            CFRelease(scheme);

            // **FIXME** Upload through an HTTP proxy is not supported.
            if (__CFBitIsSet(ctxt->_flags, kFlagBitPerformUpload)) {
                ctxt->_current++;
                return _FTPStreamOpen(stream, error, openComplete, ctxt);
            }

            _StartHTTPRequest(alloc, ctxt, error, proxyUrl);
            if (error->error) {
                *openComplete = TRUE;
                return FALSE;
            }

            return TRUE;
        }

        CFRelease(pScheme);
    }
    
    if (CFStringCompare(scheme, kFTPSchemeString, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        if (port == -1)
            port = 21;
            
        if (proxyHost)
            type = kCFNetConnectionTypeFTPProxy;
    }
    else {
        
        CFTypeRef ssl = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertySocketSecurityLevel);
        
        if (!ssl)
            _FTPStreamSetProperty(stream, kCFStreamPropertySocketSecurityLevel, kCFStreamSocketSecurityLevelNegotiatedSSL, ctxt);
            
        if (port == -1)
            port = 990;
            
        if (proxyHost)
            type = kCFNetConnectionTypeFTPSProxy;
        else
            type = kCFNetConnectionTypeFTPS;
    }

    CFRelease(scheme);
                
    *openComplete = FALSE;
    memset(error, 0, sizeof(error[0]));
    
    if (!ctxt->_connection) {
        
        CFMutableArrayRef expired = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
        _CFNetConnectionCacheKey key = createConnectionCacheKey(host, port, type, ctxt->_properties);
    
        memset(&template, 0, sizeof(template));
        template._state = kFTPStateConnect;
        template._key = key;

        __CFSpinLock(&gFTPSpinLock);
        if (gFTPConnectionCache == NULL)
            _FTPConnectionCacheCreate();
        __CFSpinUnlock(&gFTPSpinLock);

        lockConnectionCache(gFTPConnectionCache);
        CFDictionaryApplyFunction( gFTPConnectionTimeouts, (CFDictionaryApplierFunction)_FTPConnectionCacheExpiration, expired);
        unlockConnectionCache(gFTPConnectionCache);
        
        ctxt->_connection = findOrCreateNetConnection(gFTPConnectionCache,
                                                      alloc,
                                                      _kFTPConnectionCallBacks,
                                                      &template,
                                                      key,
                                                      usePersistent,
                                                      ctxt->_properties);
        
        lockConnectionCache(gFTPConnectionCache);
        CFDictionaryRemoveValue(gFTPConnectionTimeouts, ctxt->_connection);
        if (CFArrayGetCount(expired)) {
            CFIndex i;
            for (i = CFArrayGetCount(expired) - 1; i >= 0; i--) {

                _CFNetConnectionRef conn = (_CFNetConnectionRef)CFArrayGetValueAtIndex(expired, i);
                CFDictionaryRemoveValue(gFTPConnectionTimeouts, conn);
                if (conn != ctxt->_connection)
                    _CFNetConnectionSetAllowsNewRequests(conn, FALSE);
            }
        }
        CFRelease(expired);
        unlockConnectionCache(gFTPConnectionCache);
        
        releaseConnectionCacheKey(key);
    }
    
    CFRelease(host);
    
    if (!ctxt->_connection) {
        *openComplete = TRUE;
        error->error = errno;
        if (!error->error)
            error->error = ENOMEM;
        error->domain = kCFStreamErrorDomainPOSIX;
        return FALSE;
    }

    // Detect failed enque and handle properly.
    if (!_CFNetConnectionEnqueue(ctxt->_connection, ctxt)) {
        *openComplete = TRUE;
        error->error = errno;
        if (!error->error)
            error->error = ENOMEM;
        error->domain = kCFStreamErrorDomainPOSIX;
        return FALSE;
    } else if (!usePersistent) {
        _CFNetConnectionSetAllowsNewRequests(ctxt->_connection, FALSE);
    }

    return TRUE;
}


/* static */ Boolean
_FTPStreamOpenCompleted(CFTypeRef stream, CFStreamError* error, _CFFTPStreamContext* ctxt) {

    Boolean result = FALSE;
    memset(error, 0, sizeof(error[0]));

    if (ctxt->_proxyStream)
        _ProxyStreamCallBack(ctxt->_proxyStream, ctxt);
    
    if (ctxt->_dataStream) {
    
        CFStreamStatus status;
        CFTypeID i = CFReadStreamGetTypeID();
        
        if (CFGetTypeID(ctxt->_dataStream) == i)
            status = CFReadStreamGetStatus((CFReadStreamRef)ctxt->_dataStream);
        else
            status = CFWriteStreamGetStatus((CFWriteStreamRef)ctxt->_dataStream);
        
        switch (status) {
            
            case kCFStreamStatusNotOpen:
            case kCFStreamStatusOpening:
                break;
                
            case kCFStreamStatusError:
                if (CFGetTypeID(stream) == i)
                    *error = CFReadStreamGetError((CFReadStreamRef)stream);
                else
                    *error = CFWriteStreamGetError((CFWriteStreamRef)stream);
                return TRUE;
                
            default:
                return TRUE;
        }
    }

    if (ctxt->_connection) {
        _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);
        if (ctxt->_connection &&
            (_CFNetConnectionGetCurrentRequest(ctxt->_connection) == ctxt))
        {
            CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(ctxt->_connection);
            CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(ctxt->_connection);

            memset(error, 0, sizeof(error[0]));

            if (rStream && (CFReadStreamGetStatus(rStream) == kCFStreamStatusError)) {
                *error = CFReadStreamGetError(rStream);
                if (ctxt->_connection &&
                    (((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
                    (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
                {
                    ctxt->_current++;
                    ctxt->_error = *error;
                    _CFNetConnectionErrorOccurred(ctxt->_connection, error);
                    return result;
                }
                else {
                    result = TRUE;
                    _ReportError(ctxt, error);
                }
            }
            else if (wStream && (CFWriteStreamGetStatus(wStream) == kCFStreamStatusError)) {
                *error = CFWriteStreamGetError(wStream);
                if (ctxt->_connection &&
                    (((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
                    (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
                {
                    ctxt->_current++;
                    ctxt->_error = *error;
                    _CFNetConnectionErrorOccurred(ctxt->_connection, error);
                    return result;
                }
                else {
                    result = TRUE;
                    _ReportError(ctxt, error);
                }
            }
        }
    }

    if (ctxt->_server) {
        CFRunLoopSourceRef src = CFSocketCreateRunLoopSource(CFGetAllocator(ctxt->_server), ctxt->_server, 0);

        if (src) {
            CFRunLoopRef rl = CFRunLoopGetCurrent();
            
            CFRunLoopAddSource(rl, src, kCFFTPStreamOpenCompleted);
            
            CFRunLoopRunInMode(kCFFTPStreamOpenCompleted, 0.0, TRUE);
            
            CFRunLoopRemoveSource(rl, src, kCFFTPStreamOpenCompleted);
            
            CFRelease(src);
        }
    }
    
    return result;
}


/* static */ CFIndex
_FTPStreamRead(CFReadStreamRef stream, UInt8* buffer, CFIndex bufferLength, CFStreamError* error,
			   Boolean* atEOF, _CFFTPStreamContext* ctxt)
{
	CFIndex result = 0;
	
	*atEOF = FALSE;
	memset(error, 0, sizeof(error[0]));

        if (ctxt->_proxyStream) {

            CFRunLoopRef rl = CFRunLoopGetCurrent();
            CFReadStreamRef s = (CFReadStreamRef)CFRetain(ctxt->_proxyStream);

            CFReadStreamScheduleWithRunLoop(s, rl, kCFFTPStreamOpenCompleted);

            do {
                CFRunLoopRunInMode(kCFFTPStreamOpenCompleted, 1e+20, TRUE);
            } while (ctxt->_proxyStream);

            CFReadStreamUnscheduleFromRunLoop(s, rl, kCFFTPStreamOpenCompleted);
            CFRelease(s);
        }
        
	while (ctxt->_connection && (!ctxt->_dataStream || !CFReadStreamHasBytesAvailable((CFReadStreamRef)ctxt->_dataStream))) {
    
        CFWriteStreamRef requestStreams;
        CFReadStreamRef responseStreams;
    
        _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);
    
        if (!ctxt->_connection) {
            *error = CFReadStreamGetError((CFReadStreamRef)ctxt->_userStream);
            if (error->error) {
                *atEOF = TRUE;
                result = -1;
            }
            break;
        }

        else {
			
            requestStreams = _CFNetConnectionGetRequestStream(ctxt->_connection);
            responseStreams = _CFNetConnectionGetResponseStream(ctxt->_connection);
        
            if (responseStreams) {
                *error = CFReadStreamGetError(responseStreams);
            }
            
            if (!error->error && requestStreams) {
                *error = CFWriteStreamGetError(requestStreams);
            }
            
            if (error->error) {

                if ((((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
                    (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
                {
                    ctxt->_current++;
                    ctxt->_error = *error;
                    _CFNetConnectionErrorOccurred(ctxt->_connection, error);
                    continue;
                }

                else {
                    result = -1;
                    *atEOF = TRUE;
                    break;
                }
            }
        }
    }
		
    if (ctxt->_dataStream) {

        result = CFReadStreamRead((CFReadStreamRef)ctxt->_dataStream, buffer, bufferLength);

        if (__CFBitIsSet(ctxt->_flags, kFlagBitIsHTTPRequest) && !__CFBitIsSet(ctxt->_flags, kFlagBitReadHTTPResponse)) {

            if ((result >= 0) && _ProcessHTTPResponse(ctxt, error)) {

                if (error->error) {
                    *atEOF = TRUE;	// Return here so error isn't retrieved from the stream.
                    return result;	// HTTP streams don't report HTTP responses as errors (FTP does).
                }
                else if (__CFBitIsSet(ctxt->_flags, kFlagBit407TriedOnce)) {

                    // Had to re-open a new connection in order to do the read.  Return the results of the
                    // read on the new connection.
                    return _FTPStreamRead((CFReadStreamRef)ctxt->_userStream, buffer, bufferLength, error, atEOF, ctxt);
                }
            }
            else if (ctxt->_current < CFArrayGetCount(ctxt->_proxies)) {

                _RollOverHTTPRequest(ctxt, error);

                if (error->error) {
                    *atEOF = TRUE;	// Return here so error isn't retrieved from the stream.
                    return result;	// HTTP streams don't report HTTP responses as errors (FTP does).
                }
                else {

                    // Return the results of the read on the new connection.
                    return _FTPStreamRead((CFReadStreamRef)ctxt->_userStream, buffer, bufferLength, error, atEOF, ctxt);
                }
            }
        }

        if (result <= 0) {

            // **FIXME** This is not 100% correct.  If the data stream has zero bytes,
            // the result should actually be read from the control stream and any error
            // should be processed there.  This should probably call CFConnectionGetState
            // and pump along the state machine until connection is done.  The problem
            // with that solution is that a blocking situation occurs.

            *atEOF = TRUE;
            *error = CFReadStreamGetError((CFReadStreamRef)ctxt->_dataStream);
        }
    }
    
    return result;
}


/* static */ Boolean
_FTPStreamCanRead(CFReadStreamRef stream, _CFFTPStreamContext* ctxt) {

    Boolean result = FALSE;

    if (ctxt->_proxyStream)
        _ProxyStreamCallBack(ctxt->_proxyStream, ctxt);
    
    if (ctxt->_connection) {

        _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);

        if (!ctxt->_connection) {
            CFStreamError error = CFReadStreamGetError((CFReadStreamRef)ctxt->_userStream);
            if (error.error) {
                CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);
                result = TRUE;
            }
        }

        else {

            CFStreamError error;
            CFWriteStreamRef requestStreams = _CFNetConnectionGetRequestStream(ctxt->_connection);
            CFReadStreamRef responseStreams = _CFNetConnectionGetResponseStream(ctxt->_connection);

            if (responseStreams) {
                error = CFReadStreamGetError(responseStreams);
            }

            if (!error.error && requestStreams) {
                error = CFWriteStreamGetError(requestStreams);
            }

            if (error.error) {

                if ((((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
                    (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
                {
                    ctxt->_current++;
                    ctxt->_error = error;
                    _CFNetConnectionErrorOccurred(ctxt->_connection, &error);
                    result = FALSE;
                }

                else {
                    CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);
                    result = TRUE;
                }
            }
        }
    }
	
    if (ctxt->_dataStream) {

        result = CFReadStreamHasBytesAvailable((CFReadStreamRef)ctxt->_dataStream);

        // **FIXME** Catch situation where CFReadStreamHasBytesAvailable returns FALSE
        // and HTTP stream is at the end.
        if (!result && CFReadStreamGetStatus((CFReadStreamRef)ctxt->_dataStream) == kCFStreamStatusAtEnd)
            result = TRUE;
		
        if (result && __CFBitIsSet(ctxt->_flags, kFlagBitIsHTTPRequest) && !__CFBitIsSet(ctxt->_flags, kFlagBitReadHTTPResponse)) {

            CFStreamError error;

            if (_ProcessHTTPResponse(ctxt, &error)) {

                if (error.error)
                    CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);
                else
                    result = FALSE;
            }
            else if (ctxt->_current < CFArrayGetCount(ctxt->_proxies)) {

                _RollOverHTTPRequest(ctxt, &error);

                if (error.error)
                    CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);

                else
                    result = FALSE;
            }
        }
    }

    return result;
}


/* static */ CFIndex
_FTPStreamWrite(CFWriteStreamRef stream, const UInt8* buffer, CFIndex bufferLength,
                CFStreamError* error, _CFFTPStreamContext* ctxt)
{
	CFIndex result = 0;
	memset(error, 0, sizeof(error[0]));

        if (ctxt->_proxyStream) {

            CFRunLoopRef rl = CFRunLoopGetCurrent();
            CFReadStreamRef s = (CFReadStreamRef)CFRetain(ctxt->_proxyStream);

            CFReadStreamScheduleWithRunLoop(s, rl, kCFFTPStreamOpenCompleted);

            do {
                CFRunLoopRunInMode(kCFFTPStreamOpenCompleted, 1e+20, TRUE);
            } while (ctxt->_proxyStream);

            CFReadStreamUnscheduleFromRunLoop(s, rl, kCFFTPStreamOpenCompleted);
            CFRelease(s);
        }
        
	while (ctxt->_connection && (!ctxt->_dataStream || !CFWriteStreamCanAcceptBytes((CFWriteStreamRef)ctxt->_dataStream))) {
    
        CFWriteStreamRef requestStreams;
        CFReadStreamRef responseStreams;
    
        _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);
    
        if (!ctxt->_connection) {
            *error = CFWriteStreamGetError((CFWriteStreamRef)ctxt->_userStream);
            if (error->error)
                result = -1;
            break;
        }

        else {
            requestStreams = _CFNetConnectionGetRequestStream(ctxt->_connection);
            responseStreams = _CFNetConnectionGetResponseStream(ctxt->_connection);
        
            if (responseStreams) {
                *error = CFReadStreamGetError(responseStreams);
            }
            
            if (!error->error && requestStreams) {
                *error = CFWriteStreamGetError(requestStreams);
            }
            
            if (error->error) {
                result = -1;
                break;
            }
        }
    }

    if (ctxt->_dataStream) {

        result = CFWriteStreamWrite((CFWriteStreamRef)ctxt->_dataStream, buffer, bufferLength);

        if (result <= 0) {

            // **FIXME** This is not 100% correct.  If the data stream has zero bytes,
            // the result should actually be read from the control stream and any error
            // should be processed there.  This should probably call CFConnectionGetState
            // and pump along the state machine until connection is done.  The problem
            // with that solution is that a blocking situation occurs.

            *error = CFWriteStreamGetError((CFWriteStreamRef)ctxt->_dataStream);
        }
    }
    
    return result;
}


/* static */ Boolean
_FTPStreamCanWrite(CFWriteStreamRef stream, _CFFTPStreamContext* ctxt) {

    Boolean result = FALSE;

    if (ctxt->_proxyStream)
        _ProxyStreamCallBack(ctxt->_proxyStream, ctxt);
    
    if (ctxt->_connection) {

        _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);

        if (!ctxt->_connection) {
            CFStreamError error = CFWriteStreamGetError((CFWriteStreamRef)ctxt->_userStream);
            if (error.error) {
                CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);
                result = TRUE;
            }
        }

        else {

            CFStreamError error;
            CFWriteStreamRef requestStreams = _CFNetConnectionGetRequestStream(ctxt->_connection);
            CFReadStreamRef responseStreams = _CFNetConnectionGetResponseStream(ctxt->_connection);

            if (responseStreams) {
                error = CFReadStreamGetError(responseStreams);
            }

            if (!error.error && requestStreams) {
                error = CFWriteStreamGetError(requestStreams);
            }

            if (error.error) {

                if ((((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
                    (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
                {
                    ctxt->_current++;
                    ctxt->_error = error;
                    _CFNetConnectionErrorOccurred(ctxt->_connection, &error);
                    result = FALSE;
                }

                else {
                    CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, &error);
                    result = TRUE;
                }
            }
        }
    }
        
    if (ctxt->_dataStream)
        return CFWriteStreamCanAcceptBytes((CFWriteStreamRef)ctxt->_dataStream);

    return FALSE;
}


/* static */ void
_FTPStreamClose(CFTypeRef stream, _CFFTPStreamContext* ctxt) {

    _InvalidateServer(ctxt);

    if (ctxt->_proxyStream) {
	
		_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_proxyStream, ctxt->_runloops);

        CFReadStreamClose(ctxt->_proxyStream);
        CFRelease(ctxt->_proxyStream);
        ctxt->_proxyStream = NULL;
    }
    
    if (ctxt->_dataStream) {

        if (CFGetTypeID(ctxt->_dataStream) == CFReadStreamGetTypeID())
            _ReleaseDataReadStream(ctxt);

        else {

			_CFTypeInvalidate(ctxt->_dataStream);
			_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_dataStream, ctxt->_runloops);
            
            CFWriteStreamClose((CFWriteStreamRef)(ctxt->_dataStream));
            CFRelease(ctxt->_dataStream);
            ctxt->_dataStream = NULL;
        }
    }
	
    if (ctxt->_connection) {
        
        int state = _CFNetConnectionGetState(ctxt->_connection, FALSE, ctxt);
        
        if (state != kTransmittingRequest)
            _CFNetConnectionDequeue(ctxt->_connection, ctxt);
        else {
            CFArrayRef a = ctxt->_runloops;
            int i, count = CFArrayGetCount(a);
            for (i = 0; i < count; i += 2) {
                _CFNetConnectionUnschedule(ctxt->_connection,
                                           ctxt,
                                           (CFRunLoopRef)CFArrayGetValueAtIndex(a, i),
                                           (CFStringRef)CFArrayGetValueAtIndex(a, i + 1));
            }
            
            _CFNetConnectionRequestIsComplete(ctxt->_connection, ctxt);
            _CFNetConnectionResponseIsComplete(ctxt->_connection, ctxt);
        }
    }
}


/* static */ CFTypeRef
_FTPStreamCopyProperty(CFTypeRef stream, CFStringRef propertyName, _CFFTPStreamContext* ctxt) {

    CFTypeRef value = NULL;
    
    if (CFEqual(propertyName, kCFStreamPropertyFTPUsePassiveMode)) {
        value = CFRetain(__CFBitIsSet(ctxt->_flags, kFlagBitPerformPASV) ? kCFBooleanTrue : kCFBooleanFalse);
    }
	
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFetchResourceInfo)) {
        value = CFRetain(__CFBitIsSet(ctxt->_flags, kFlagBitPerformSTAT) ? kCFBooleanTrue : kCFBooleanFalse);
    }
	
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFetchNameList)) {
        value = CFRetain(__CFBitIsSet(ctxt->_flags, kFlagBitPerformNLST) ? kCFBooleanTrue : kCFBooleanFalse);
    }
	
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFileTransferOffset)) {
        value = CFNumberCreate(CFGetAllocator(ctxt->_properties), kCFNumberLongLongType, &ctxt->_offset);
    }
	
    else if (CFEqual(propertyName, kCFStreamPropertyFTPResourceSize)) {
        if (ctxt->_attributes) {
            CFNumberRef original = CFDictionaryGetValue(ctxt->_attributes, kCFFTPResourceSize);
            if (original)
                value = CFRetain(original);		// Becky's sez that CFNumber is immutable so retaining is fine.
        }
    }

    else if (CFEqual(propertyName, kCFStreamPropertyFTPAttemptPersistentConnection)) {
        CFBooleanRef contains = CFDictionaryGetValue(ctxt->_properties, propertyName);
        value = contains ? CFRetain(contains) : CFRetain(kCFBooleanTrue);
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPLogInOnly)) {
        value = CFRetain(__CFBitIsSet(ctxt->_flags, kFlagBitLogInOnly) ? kCFBooleanTrue : kCFBooleanFalse);
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPRemoveResource)) {
        value = CFRetain(__CFBitIsSet(ctxt->_flags, kFlagBitRemoveResource) ? kCFBooleanTrue : kCFBooleanFalse);
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPNewResourceName)) {
        value = ctxt->_newUrl ? CFRetain(ctxt->_newUrl) : NULL;
    }

    if (!value && ctxt->_dataStream) {
        if (CFGetTypeID(ctxt->_dataStream) == CFReadStreamGetTypeID())
            value = CFReadStreamCopyProperty((CFReadStreamRef)ctxt->_dataStream, propertyName);
        else
            value = CFWriteStreamCopyProperty((CFWriteStreamRef)ctxt->_dataStream, propertyName);
    }

    // If there is a control stream, check it for the property.
    if (!value && ctxt->_connection &&
        (_CFNetConnectionGetCurrentRequest(ctxt->_connection) == ctxt))
    {
        CFTypeRef s = _CFNetConnectionGetResponseStream(ctxt->_connection);
        
        if (s)
            value = CFReadStreamCopyProperty((CFReadStreamRef)s, propertyName);
            
        if (!value) {
            s = _CFNetConnectionGetRequestStream(ctxt->_connection);
            if (s)
                value = CFWriteStreamCopyProperty((CFWriteStreamRef)s, propertyName);
        }
    }

    // If every avenue has been tried, try grabbing from the local properties.
    if (!value) {
    
        CFTypeRef orig = CFDictionaryGetValue(ctxt->_properties, propertyName);
        
        if (orig) {
            CFTypeID i = CFGetTypeID(orig);
            if (i == CFStringGetTypeID())
                value = CFStringCreateCopy(CFGetAllocator(ctxt->_properties), orig);
            else if (i == CFDataGetTypeID())
                value = CFDataCreateCopy(CFGetAllocator(ctxt->_properties), orig);
            else if (i == CFDictionaryGetTypeID())
                value = CFDictionaryCreateCopy(CFGetAllocator(ctxt->_properties), orig);
            else if (i == CFArrayGetTypeID())
                value = CFArrayCreateCopy(CFGetAllocator(ctxt->_properties), orig);
        }
    }

    return value;
}


/* static */ Boolean
_FTPStreamSetProperty(CFTypeRef stream, CFStringRef propertyName,
					  CFTypeRef propertyValue, _CFFTPStreamContext* ctxt)
{
    Boolean result = FALSE;
	
    // **FIXME** Flow-changing properties should check to see if the
    // state machine is beyond their respective command before setting
    // successfully.
    
    if (CFEqual(propertyName, kCFStreamPropertyFTPProxy)) {
        
        if (!ctxt->_connection && !ctxt->_dataStream) {
        
            if (!propertyValue) {
                CFDictionaryRemoveValue(ctxt->_properties, propertyName);
                result = TRUE;
            }
            
            else if (CFGetTypeID(propertyValue) == CFDictionaryGetTypeID()) {

				// Attempt to set the passive bit based upon proxy dictionary from SC.
				CFTypeRef p = CFDictionaryGetValue(propertyValue, kSCPropNetProxiesFTPPassive);
				
                CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertySOCKSProxy);
                CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
				
				// Only set the bit if it wasn't set explicitly and there is a value.
				if (!__CFBitIsSet(ctxt->_flags, kFlagBitDidSetPassiveBit) && p) {
				
					// If it's a number, set it based upon zero or non-zero.
					if (CFGetTypeID(p) == CFNumberGetTypeID()) {
						
						SInt32 val;
						
						CFNumberGetValue(p, kCFNumberSInt32Type, &val);
						
						if (val)
							__CFBitSet(ctxt->_flags, kFlagBitPerformPASV);
						else
							__CFBitClear(ctxt->_flags, kFlagBitPerformPASV);
					}
					
					else if (p == kCFBooleanFalse) {
						__CFBitClear(ctxt->_flags, kFlagBitPerformPASV);
					}
				}
				
                result = TRUE;
            }
        }
    }
    
    else if (CFEqual(propertyName, kCFStreamPropertySOCKSProxy)) {
        
        if (!ctxt->_connection && !ctxt->_dataStream) {

            if (!propertyValue) {
                CFDictionaryRemoveValue(ctxt->_properties, propertyName);
                result = TRUE;
            }
            
            else if (CFGetTypeID(propertyValue) == CFDictionaryGetTypeID()) {
                
                if (!CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxy)) {

                    CFDictionaryRemoveValue(ctxt->_properties, kCFStreamPropertyFTPProxy);
                    CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
                    result = TRUE;
                }
            }
        }
    }
    
    else if (CFEqual(propertyName, kCFStreamPropertyFTPUsePassiveMode)) {
        
        // Default mode (NULL or true) indicate passive use
        if (propertyValue && CFEqual(propertyValue, kCFBooleanFalse))
            __CFBitClear(ctxt->_flags, kFlagBitPerformPASV);
        else
            __CFBitSet(ctxt->_flags, kFlagBitPerformPASV);
            
		__CFBitSet(ctxt->_flags, kFlagBitDidSetPassiveBit);

        result = TRUE;
    }
    
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFetchResourceInfo)) {
    
        // Default is not to perform the STAT command
        if (propertyValue && CFEqual(propertyValue, kCFBooleanTrue))
            __CFBitSet(ctxt->_flags, kFlagBitPerformSTAT);
        else
            __CFBitClear(ctxt->_flags, kFlagBitPerformSTAT);
            
        result = TRUE;
    }
    
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFetchNameList)) {
    
        // Default is not to perform the NLST command
        if (propertyValue && CFEqual(propertyValue, kCFBooleanTrue))
            __CFBitSet(ctxt->_flags, kFlagBitPerformNLST);
        else
            __CFBitClear(ctxt->_flags, kFlagBitPerformNLST);
            
        result = TRUE;
    }
    
    else if (CFEqual(propertyName, kCFStreamPropertyFTPFileTransferOffset)) {
    
        // Default is to not perform the offset (or offset to zero)
        if (!propertyValue)
            ctxt->_offset = 0;
        else
            CFNumberGetValue(propertyValue, kCFNumberLongLongType, &ctxt->_offset);
                    
        result = TRUE;
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPLogInOnly)) {
        if (propertyValue && CFEqual(propertyValue, kCFBooleanTrue))
            __CFBitSet(ctxt->_flags, kFlagBitLogInOnly);
        else
            __CFBitClear(ctxt->_flags, kFlagBitLogInOnly);
        
        result = TRUE;
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPRemoveResource)) {
        if (propertyValue && CFEqual(propertyValue, kCFBooleanTrue))
            __CFBitSet(ctxt->_flags, kFlagBitRemoveResource);
        else
            __CFBitClear(ctxt->_flags, kFlagBitRemoveResource);
            
        result = TRUE;
    }
    
    else if (CFEqual(propertyName, _kCFStreamPropertyFTPNewResourceName)) {
    
        CFURLRef temp = ctxt->_newUrl;
        ctxt->_newUrl = propertyValue ? CFURLCopyAbsoluteURL(propertyValue) : NULL;
        if (temp)
            CFRelease(temp);
        
        result = TRUE;
    }
    
    // kCFStreamPropertyFTPResourceSize can not be set.
    else if (CFEqual(propertyName, kCFStreamPropertyFTPResourceSize))
        result = FALSE;
        
    else if (CFEqual(propertyName, kCFStreamPropertyFTPAttemptPersistentConnection)) {
        
        if (!propertyValue || !CFEqual(propertyValue, kCFBooleanFalse))
            CFDictionaryRemoveValue(ctxt->_properties, propertyName);
        else
            CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
    }
        
    else {
    
        if (ctxt->_dataStream) {
            if (CFGetTypeID(ctxt->_dataStream) == CFReadStreamGetTypeID())
                result = CFReadStreamSetProperty((CFReadStreamRef)ctxt->_dataStream, propertyName, propertyValue);
            else
                result = CFWriteStreamSetProperty((CFWriteStreamRef)ctxt->_dataStream, propertyName, propertyValue);
        }
        
        // **FIXME** Set the property on the control stream if there is one.  It should also
        // orphan the remaining queued items.
        
        if (!result) {
            if (propertyValue)
                CFDictionarySetValue(ctxt->_properties, propertyName, propertyValue);
            else
                CFDictionaryRemoveValue(ctxt->_properties, propertyName);
            result = TRUE;
        }
    }

    return result;
}


/* static */ void
_FTPStreamSchedule(CFTypeRef stream, CFRunLoopRef runLoop,
				   CFStringRef runLoopMode, _CFFTPStreamContext* ctxt)
{
	if (_SchedulesAddRunLoopAndMode(ctxt->_runloops, runLoop, runLoopMode)) {

		if (ctxt->_proxyStream)
			CFReadStreamScheduleWithRunLoop(ctxt->_proxyStream, runLoop, runLoopMode);
		
		if (ctxt->_server)
			_CFTypeScheduleOnRunLoop(ctxt->_server, runLoop, runLoopMode);

		if (ctxt->_dataStream)
			_CFTypeScheduleOnRunLoop(ctxt->_dataStream, runLoop, runLoopMode);
			
		if (ctxt->_connection)
			_CFNetConnectionSchedule(ctxt->_connection, ctxt, runLoop, runLoopMode);
	}
}


/* static */ void
_FTPStreamUnschedule(CFTypeRef stream, CFRunLoopRef runLoop,
					 CFStringRef runLoopMode, _CFFTPStreamContext* ctxt)
{
	if (_SchedulesRemoveRunLoopAndMode(ctxt->_runloops, runLoop, runLoopMode)) {

		if (ctxt->_proxyStream)
			CFReadStreamUnscheduleFromRunLoop(ctxt->_proxyStream, runLoop, runLoopMode);
		
		if (ctxt->_server)
			_CFTypeUnscheduleFromRunLoop(ctxt->_server, runLoop, runLoopMode);

		if (ctxt->_dataStream)
			_CFTypeUnscheduleFromRunLoop(ctxt->_dataStream, runLoop, runLoopMode);
			
		if (ctxt->_connection)
			_CFNetConnectionUnschedule(ctxt->_connection, ctxt, runLoop, runLoopMode);
    }
}


#if 0
#pragma mark -
#pragma mark CFNetConnection Callback Functions
#endif


/* static */ const void*
_CFFTPNetConnectionContextCreate(CFAllocatorRef alloc, const _CFFTPNetConnectionContext* template) {

    _CFFTPNetConnectionContext* ctxt = (_CFFTPNetConnectionContext*)CFAllocatorAllocate(alloc,
                                                                                        sizeof(ctxt[0]),
                                                                                        0);
                                                                                          
    memmove(ctxt, template, sizeof(ctxt[0]));
    ctxt->_key = (_CFNetConnectionCacheKey)connCacheKeyRetain(alloc, template->_key);
	
    if (!ctxt->_recvBuffer)
        ctxt->_recvBuffer = CFDataCreateMutable(alloc, 0);
        
    if (!ctxt->_sendBuffer)
        ctxt->_sendBuffer = CFDataCreateMutable(alloc, 0);
    __CFBitClear(ctxt->_flags, kFlagBitIsXServer);
    
    return (const void*)ctxt;
}


/* static */ void
_CFFTPNetConnectionContextFinalize(CFAllocatorRef alloc, const _CFFTPNetConnectionContext* ctxt) {
    
    connCacheKeyRelease(alloc, ctxt->_key);
    
    if (ctxt->_root)
        CFRelease(ctxt->_root);
    
    if (ctxt->_recvBuffer)
        CFRelease(ctxt->_recvBuffer);
        
    if (ctxt->_sendBuffer)
        CFRelease(ctxt->_sendBuffer);

    CFAllocatorDeallocate(alloc, (_CFFTPNetConnectionContext*)ctxt);
}


/* static */ CFStreamError
_FTPConnectionCreateStreams(CFAllocatorRef alloc,
                            const _CFFTPNetConnectionContext* ctxt,
                            CFWriteStreamRef* requestStream,
                            CFReadStreamRef* responseStream)
{
    CFStreamError result = {0, 0};
    
    UInt32 type;
    SInt32 port;
    CFStringRef host;
    CFDictionaryRef properties;
    
    getValuesFromKey(ctxt->_key, &host, &port, &type, &properties);

    *requestStream = NULL;
    *responseStream = NULL;
	
    // Get the correct host and port if using proxy.
    if ((type == kCFNetConnectionTypeFTPProxy) || (type == kCFNetConnectionTypeFTPSProxy)) {
        
        CFDictionaryRef proxy = CFDictionaryGetValue(properties, kCFStreamPropertyFTPProxy);
        CFNumberRef cfport = CFDictionaryGetValue(proxy, kCFStreamPropertyFTPProxyPort);
    
        host = CFDictionaryGetValue(proxy, kCFStreamPropertyFTPProxyHost);
        if (cfport)
            CFNumberGetValue(cfport, kCFNumberSInt32Type, &port);
        else if (type == kCFNetConnectionTypeFTPProxy)
            port = 21;
        else
            port = 990;
    }

	_CFSocketStreamCreatePair(alloc, host, port, 0, NULL, responseStream, requestStream);

    if (*responseStream && *requestStream) {

		CFArrayCallBacks cb = {0, NULL, NULL, NULL, NULL};
		const void* values[2] = {_CFStreamSocketCreatedCallBack, NULL};
		CFArrayRef callback = CFArrayCreate(alloc, values, sizeof(values) / sizeof(values[0]), &cb);
		
		if (callback) {
			CFWriteStreamSetProperty(*requestStream, CFSTR("_kCFStreamSocketCreatedCallBack"), callback);
			CFRelease(callback);
		}
	
        CFDictionaryApplyFunction(properties, (CFDictionaryApplierFunction)_StreamPropertyApplier, *responseStream);
        CFDictionaryApplyFunction(properties, (CFDictionaryApplierFunction)_StreamPropertyApplier, *requestStream);
    }
    else {
        result.domain = kCFStreamErrorDomainPOSIX;
        result.error = errno;
#if defined(__WIN32__)
        if (!result.error) {
            result.error = WSAGetLastError();
            if (result.error)
                result.domain = kCFStreamErrorDomainWinSock;
        }
#endif
        if (!result.error)
            result.error = ENOMEM;
    }

    return result;
}


/* static */ void
_FTPConnectionRequestStateChanged(_CFFTPStreamContext* ctxt, int newState,
                                  CFStreamError *err, _CFNetConnectionRef connection,
                                  _CFFTPNetConnectionContext* netCtxt)
{
    switch (newState) {

        case kQueued:
            ctxt->_connection = connection;
            break;
        
        case kTransmittingRequest:
            {
                CFArrayRef a = ctxt->_runloops;
                int i, count = CFArrayGetCount(a);
                
                for (i = 0; i < count; i += 2) {
                    _CFNetConnectionSchedule(connection,
                                             ctxt,
                                             (CFRunLoopRef)CFArrayGetValueAtIndex(a, i),
                                             (CFStringRef)CFArrayGetValueAtIndex(a, i + 1));
                }
                
                if (netCtxt->_state == kFTPStateIdle)
                    _StartProcess(netCtxt, ctxt);
                else if (netCtxt->_state > kFTPStateIdle) {
                    __CFBitSet(netCtxt->_flags, kFlagBitReturnToIdle);
                    _CFNetConnectionGetState(ctxt->_connection, TRUE, ctxt);
                }
            }
            break;
            
        case kFinished:
//        case kCancelled:
            {
                CFBooleanRef persistent = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties,
                                                                             kCFStreamPropertyFTPAttemptPersistentConnection);
                Boolean usePersistent = (persistent && CFEqual(persistent, kCFBooleanFalse)) ? FALSE : TRUE;
                __CFBitSet(netCtxt->_flags, kFlagBitLeftForDead);
                if (usePersistent)
                    CFDictionarySetValue(gFTPConnectionTimeouts, ctxt->_connection, CFDateCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + kFTPTimeoutInSeconds));
            }
            // NOTE that this falls through to kOrphaned on purpose.
            
        case kOrphaned:
            {
                CFArrayRef a = ctxt->_runloops;
                int i, count = CFArrayGetCount(a);

                _CFNetConnectionDequeue(connection, ctxt);
                
                for (i = 0; i < count; i += 2) {
                    _CFNetConnectionUnschedule(connection,
                                               ctxt,
                                               (CFRunLoopRef)CFArrayGetValueAtIndex(a, i),
                                               (CFStringRef)CFArrayGetValueAtIndex(a, i + 1));
                }
                
                CFRelease(ctxt->_connection);
                ctxt->_connection = NULL;
            
                if ((newState == kOrphaned) && !__CFBitIsSet(ctxt->_flags, kFlagBitGotError)) {
                    
                    CFStreamError error;
                    Boolean open;
                    
                    _FTPStreamOpen(ctxt->_userStream, &error, &open, ctxt);
                    
                    if (open) {
                        
                        CFStreamEventType event = kCFStreamEventErrorOccurred;
                        if (!error.error)
                            event = kCFStreamEventOpenCompleted;

                        if (CFGetTypeID(ctxt->_userStream) == CFReadStreamGetTypeID())
                            CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, event, &error);
                        else
                            CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, event, &error);
                    }
                }
            }
            
            break;
                
        default:
            break;
    }
}


/* static */ void
_FTPConnectionTransmitRequest(_CFFTPStreamContext* ctxt, _CFNetConnectionRef connection, _CFFTPNetConnectionContext* netCtxt) {

    CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(connection);
	CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(connection);
	
    if (CFWriteStreamCanAcceptBytes(wStream)) {
        if (__CFBitIsSet(netCtxt->_flags, kFlagBitHTTPLitmus))
            _FTPRequestStreamCallBack(ctxt, wStream, kCFStreamEventCanAcceptBytes, connection, netCtxt);
        else {
            CFStringRef cmd;
            CFStringRef user = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPUserName);

            CFStringRef host;
            SInt32 port;
            UInt32 type;
            CFDictionaryRef properties;

            CFAllocatorRef alloc = CFGetAllocator(ctxt->_runloops);

            getValuesFromKey(netCtxt->_key, &host, &port, &type, &properties);

            if (user)
                CFRetain(user);
            else {
                user = CFURLCopyUserName(ctxt->_url);
                if (!user)
                    user = CFRetain(kAnonymousUserString);
            }

            if ((type == kCFNetConnectionTypeFTPProxy) || (type == kCFNetConnectionTypeFTPSProxy)) {

                CFStringRef newUser;

                // **FIXME** This may not be kosher.  Is the URL passed into the proxy
                // allowed to have a ':' and port appended to it?
                if (((type == kCFNetConnectionTypeFTPProxy) && (port == 21)) ||
                    ((type == kCFNetConnectionTypeFTPSProxy) && (port == 990)))
                {
                    newUser = CFStringCreateWithFormat(alloc, NULL, kFTPProxyFormat, user, host);
                }
                else
                    newUser = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@@%@:%ld"), user, host, port);

                CFRelease(user);
                user = newUser;
            }

            cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPUSERCommandString, user);

            CFRelease(user);
            __CFBitSet(netCtxt->_flags, kFlagBitHTTPLitmus);	// NOTE that this is set before the call to WriteCommand
    
            if (cmd) {
                _WriteCommand(netCtxt, ctxt, cmd);
                CFRelease(cmd);
            }
            else {
                CFStreamError error = {kCFStreamErrorDomainPOSIX, errno};

                if (!error.error)
                    error.error = ENOMEM;
                _ReportError(ctxt, &error);
            }
        }
    }
	
    if (ctxt->_connection && CFReadStreamHasBytesAvailable(rStream))
		_FTPConnectionReceiveResponse(ctxt, connection, netCtxt);
}


/* static */ void
_FTPConnectionReceiveResponse(_CFFTPStreamContext* ctxt, _CFNetConnectionRef connection, _CFFTPNetConnectionContext* netCtxt) {

    CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(connection);
    CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(connection);

    if (CFReadStreamHasBytesAvailable(rStream))
            _FTPResponseStreamCallBack(ctxt, rStream, kCFStreamEventHasBytesAvailable, connection, netCtxt);
	
    if (ctxt->_connection && CFWriteStreamCanAcceptBytes(wStream))
		_FTPConnectionTransmitRequest(ctxt, connection, netCtxt);
}


/* static */ void
_FTPResponseStreamCallBack(_CFFTPStreamContext* ctxt, CFReadStreamRef stream,
                           CFStreamEventType type, _CFNetConnectionRef conn,
                           _CFFTPNetConnectionContext* netCtxt)
{
    switch (type) {

        case kCFStreamEventHasBytesAvailable:
            {
                CFIndex i, canRead = CFDataGetLength(netCtxt->_recvBuffer) - netCtxt->_recvCount;
                
                // **FIXME** There are false positives coming through in
                // heavily threaded tests.
                if (!CFReadStreamHasBytesAvailable(stream))
                    return;
                
                if (canRead < kBufferGrowthSize) {
                    CFDataSetLength(netCtxt->_recvBuffer, netCtxt->_recvCount + kBufferGrowthSize);
                    if (CFDataGetLength(netCtxt->_recvBuffer) < (netCtxt->_recvCount + kBufferGrowthSize)) {
                        CFStreamError error = {kCFStreamErrorDomainPOSIX, ENOMEM};
                        _ReportError(ctxt, &error);
                        return;
                    }
                    canRead = kBufferGrowthSize;
                }

                i = CFReadStreamRead(stream,
                                     CFDataGetMutableBytePtr(netCtxt->_recvBuffer) + netCtxt->_recvCount,
                                     canRead);

                if (i < 0)
                    _FTPResponseStreamCallBack(ctxt, stream, kCFStreamEventErrorOccurred, conn, netCtxt);
                else if (!i) {
                    if (__CFBitIsSet(netCtxt->_flags, kFlagBitLeftForDead)) {

                        CFStreamError error;
                        Boolean openComplete;

                        _CFNetConnectionLost(ctxt->_connection);
                        _CFNetConnectionDequeue(ctxt->_connection, ctxt);
						
						CFRelease(ctxt->_connection);
						ctxt->_connection = NULL;
						
                        _FTPStreamOpen(ctxt->_userStream, &error, &openComplete, ctxt);
                    }
                    else
                        _FTPResponseStreamCallBack(ctxt, stream, kCFStreamEventEndEncountered, conn, netCtxt);
                }
                else {						

                    netCtxt->_recvCount += i;

                    _HandleResponse(netCtxt, ctxt);
                }
            }
            break;

        case kCFStreamEventErrorOccurred:
            {
                CFStreamError error = CFReadStreamGetError(stream);
                _ReportError(ctxt, &error);
            }
            break;

        case kCFStreamEventEndEncountered:
            {
                // **FIXME** Deal with cases where an end is okay.
                CFStreamError error = {_kCFStreamErrorDomainNativeSockets, ENOTCONN};
                _ReportError(ctxt, &error);
            }
            break;
			
        default:
            break;
    }
}


/* static */ void
_FTPRequestStreamCallBack(_CFFTPStreamContext* ctxt, CFWriteStreamRef stream,
                          CFStreamEventType type, _CFNetConnectionRef conn,
                          _CFFTPNetConnectionContext* netCtxt)
{
    switch (type) {

        case kCFStreamEventCanAcceptBytes:
            {
                if (!__CFBitIsSet(netCtxt->_flags, kFlagBitHTTPLitmus)) {
                    _FTPConnectionTransmitRequest(ctxt, conn, netCtxt);		// NOTE that this is a re-entrant call
                    break;
                }

                if (netCtxt->_sendCount) {
                    UInt8* buffer = CFDataGetMutableBytePtr(netCtxt->_sendBuffer);
                    CFIndex i = CFWriteStreamWrite(stream, buffer, netCtxt->_sendCount);

                    if (i < 0)
                        _FTPRequestStreamCallBack(ctxt, stream, kCFStreamEventErrorOccurred, conn, netCtxt);
                    else if (!i)
                        _FTPRequestStreamCallBack(ctxt, stream, kCFStreamEventEndEncountered, conn, netCtxt);
                    else {
                        netCtxt->_sendCount -= i;
                        memmove(buffer, buffer + i, netCtxt->_sendCount);
                    }
                }
            }
            break;

        case kCFStreamEventErrorOccurred:
            {
                CFStreamError error = CFWriteStreamGetError(stream);
                _ReportError(ctxt, &error);
            }
            break;

        case kCFStreamEventEndEncountered:
            {
                // **FIXME** Deal with cases where an end is okay.
                CFStreamError error = {_kCFStreamErrorDomainNativeSockets, ENOTCONN};
                _ReportError(ctxt, &error);
            }
            break;
			
        default:
            break;
    }
}

/* static */ CFArrayRef
_FTPRunLoopArrayCallBack(_CFFTPStreamContext *ctxt, _CFNetConnectionRef conn, _CFFTPNetConnectionContext *netCtxt) {
    return ctxt->_runloops;
}


#if 0
#pragma mark -
#pragma mark Utility Functions
#endif

/* static */ Boolean
_IsRoot(CFURLRef url) {
    Boolean isAbsolute;  
    CFStringRef strictPath, resourceSpecifier;
    strictPath = CFURLCopyStrictPath(url, &isAbsolute);
    resourceSpecifier = CFURLCopyResourceSpecifier(url);
    if (!strictPath && !resourceSpecifier)  return TRUE;
    if (strictPath) CFRelease(strictPath);
    if (resourceSpecifier) CFRelease(resourceSpecifier);
    return FALSE;     
}

/* static */ void
_FTPConnectionCacheCreate(void) {
	
	if (!_kFTPConnectionCallBacks) {
		
		_kFTPConnectionCallBacks = (_CFNetConnectionCallBacks*)CFAllocatorAllocate(kCFAllocatorDefault, sizeof(_kFTPConnectionCallBacks[0]), 0);
		
		assert(_kFTPConnectionCallBacks != NULL);
		
		_kFTPConnectionCallBacks->version = 0;
		_kFTPConnectionCallBacks->create = (const void* (*)(CFAllocatorRef, const void*))_CFFTPNetConnectionContextCreate;
		_kFTPConnectionCallBacks->finalize = (void (*)(CFAllocatorRef, const void*))_CFFTPNetConnectionContextFinalize;
		_kFTPConnectionCallBacks->createConnectionStreams = (CFStreamError (*)(CFAllocatorRef, const void*, CFWriteStreamRef*, CFReadStreamRef*))_FTPConnectionCreateStreams;
		_kFTPConnectionCallBacks->requestStateChanged = (void (*)(void*, int, CFStreamError*, _CFNetConnectionRef, const void*))_FTPConnectionRequestStateChanged;
		_kFTPConnectionCallBacks->transmitRequest = (void (*)(void*, _CFNetConnectionRef, const void*))_FTPConnectionTransmitRequest;
		_kFTPConnectionCallBacks->receiveResponse = (void (*)(void*, _CFNetConnectionRef, const void*))_FTPConnectionReceiveResponse;
		_kFTPConnectionCallBacks->responseStreamCallBack = (void (*)(void*, CFReadStreamRef, CFStreamEventType, _CFNetConnectionRef, const void*))_FTPResponseStreamCallBack;
		_kFTPConnectionCallBacks->requestStreamCallBack = (void (*)(void*, CFWriteStreamRef, CFStreamEventType, _CFNetConnectionRef, const void*))_FTPRequestStreamCallBack;
		_kFTPConnectionCallBacks->runLoopAndModesArrayForRequest = (CFArrayRef (*)(void *, _CFNetConnectionRef, const void*))_FTPRunLoopArrayCallBack;
	}
	
    gFTPConnectionTimeouts = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    gFTPConnectionCache = createConnectionCache();
}

/* static */ void
_FTPConnectionCacheExpiration(_CFNetConnectionRef conn, CFDateRef expiration, CFMutableArrayRef list) {

    if (CFAbsoluteTimeGetCurrent() >= CFDateGetAbsoluteTime(expiration))
        CFArrayAppendValue(list, conn);
}


/* static */ void
_SetSOCKS4ProxyInformation(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFURLRef proxyUrl) {

    CFStringRef pHost = CFURLCopyHostName(proxyUrl);
    SInt32 p = CFURLGetPortNumber(proxyUrl);
    CFNumberRef pPort = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
    CFStringRef pUser = CFURLCopyUserName(proxyUrl);
    CFStringRef pPass = CFURLCopyPassword(proxyUrl);
    CFMutableDictionaryRef pInfo = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSProxyHost, pHost);
    CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSProxyPort, pPort);

    CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSVersion, kCFStreamSocketSOCKSVersion4);

    if (pUser) {
        CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSUser, pUser);
        CFRelease(pUser);
    }

    if (pPass) {
        CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSPassword, pPass);
        CFRelease(pPass);
    }

    CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySOCKSProxy, pInfo);
    CFRelease(pInfo);

    CFRelease(pHost);
    CFRelease(pPort);
}


/* static */ void
_SetSOCKS5ProxyInformation(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFURLRef proxyUrl) {

    CFStringRef pHost = CFURLCopyHostName(proxyUrl);
    SInt32 p = CFURLGetPortNumber(proxyUrl);
    CFNumberRef pPort = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
    CFStringRef pUser = CFURLCopyUserName(proxyUrl);
    CFStringRef pPass = CFURLCopyPassword(proxyUrl);
    CFMutableDictionaryRef pInfo = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSProxyHost, pHost);
    CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSProxyPort, pPort);

    if (pUser) {
        CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSUser, pUser);
        CFRelease(pUser);
    }

    if (pPass) {
        CFDictionaryAddValue(pInfo, kCFStreamPropertySOCKSPassword, pPass);
        CFRelease(pPass);
    }

    CFDictionaryAddValue(ctxt->_properties, kCFStreamPropertySOCKSProxy, pInfo);
    CFRelease(pInfo);

    CFRelease(pHost);
    CFRelease(pPort);
}


/* static */ void
_StartHTTPRequest(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt, CFStreamError* error, CFURLRef proxyUrl) {
	
    CFURLRef url;
    CFHTTPMessageRef msg;
    CFMutableDictionaryRef pInfo;
    CFURLComponentsRFC1808 comps;
    CFStreamClientContext streamCtxt = {0, ctxt, NULL, NULL, NULL};
    CFStringRef user = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPUserName);
    CFStringRef pass = CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPPassword);
    CFHTTPMessageRef resp = (CFHTTPMessageRef)CFDictionaryGetValue(ctxt->_properties, _kCFStreamPropertyFTPLastHTTPResponse);
    
    __CFBitSet(ctxt->_flags, kFlagBitIsHTTPRequest);

    memset(error, 0, sizeof(error[0]));

    if (user)
        CFRetain(user);
    else
        user = CFURLCopyUserName(ctxt->_url);

    if (pass)
        CFRetain(pass);
    else
        pass = CFURLCopyPassword(ctxt->_url);


    // **FIXME** Create new URL to use.  This should use the new range API's instead.
    // Remove #include <CoreFoundation/CFPriv.h> when switched.
    memset(&comps, 0, sizeof(comps));
    _CFURLCopyComponents(ctxt->_url, kCFURLComponentDecompositionRFC1808, &comps);

    if (!comps.user)
        comps.user = user ? CFURLCreateStringByAddingPercentEscapes(alloc, user, NULL, NULL, kCFStringEncodingUTF8) : NULL;

    if (!comps.password)
        comps.password = pass ? CFURLCreateStringByAddingPercentEscapes(alloc, pass, NULL, NULL, kCFStringEncodingUTF8) : NULL;

    if (comps.query) {
        CFRelease(comps.query);
        comps.query = NULL;
    }

    if (comps.fragment) {
        CFRelease(comps.fragment);
        comps.fragment = NULL;
    }

    if (comps.parameterString) {
        CFRelease(comps.parameterString);
        comps.parameterString = NULL;
    }

    if (user)
        CFRelease(user);

    if (pass)
        CFRelease(pass);

    url = _CFURLCreateFromComponents(alloc, kCFURLComponentDecompositionRFC1808, &comps);

    if (comps.scheme)
        CFRelease(comps.scheme);
    if (comps.user)
        CFRelease(comps.user);
    if (comps.password)
        CFRelease(comps.password);
    if (comps.host)
        CFRelease(comps.host);
    if (comps.pathComponents)
        CFRelease(comps.pathComponents);
    if (comps.baseURL)
        CFRelease(comps.baseURL);

    msg = CFHTTPMessageCreateRequest(alloc, kHTTPGETMethod, url ? url : ctxt->_url, kCFHTTPVersion1_1);

    if (url)
        CFRelease(url);

    if (resp) {
        CFHTTPMessageAddAuthentication(msg,
                                       resp,
                                       CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxyUser),
                                       CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxyPassword),
                                       NULL,
                                       TRUE);
    }

    ctxt->_dataStream = CFReadStreamCreateForHTTPRequest(alloc, msg);
    CFRelease(msg);
    
    if (!ctxt->_dataStream) {
        error->error = errno;
        error->domain = kCFStreamErrorDomainPOSIX;
#if defined(__WIN32__)
        if (!error->error) {
            error->error = WSAGetLastError();
            if (error->error)
                error->domain = kCFStreamErrorDomainWinSock;
        }
#endif
        if (!error->error)
            error->error = ENOMEM;
    }
    else {
        CFStringRef pScheme = CFURLCopyScheme(proxyUrl);
        CFStringRef pHost = CFURLCopyHostName(proxyUrl);
        SInt32 p = CFURLGetPortNumber(proxyUrl);
        CFNumberRef pPort = CFNumberCreate(alloc, kCFNumberSInt32Type, &p);
        CFBooleanRef persistent = (CFBooleanRef)CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPAttemptPersistentConnection);
        
        pInfo = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        if (CFStringCompare(pScheme, kHTTPSchemeString, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            CFDictionarySetValue(pInfo, kCFStreamPropertyHTTPProxyHost, pHost);
            CFDictionarySetValue(pInfo, kCFStreamPropertyHTTPProxyPort, pPort);
        }
        else {
            CFDictionarySetValue(pInfo, kCFStreamPropertyHTTPSProxyHost, pHost);
            CFDictionarySetValue(pInfo, kCFStreamPropertyHTTPSProxyPort, pPort);
        }
        CFReadStreamSetProperty((CFReadStreamRef)ctxt->_dataStream, kCFStreamPropertyHTTPProxy, pInfo);
        CFRelease(pInfo);
        
        if (!persistent || CFEqual(persistent, kCFBooleanTrue))
            CFReadStreamSetProperty((CFReadStreamRef)ctxt->_dataStream, kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue);
        
        CFDictionaryApplyFunction(ctxt->_properties, (CFDictionaryApplierFunction)_StreamPropertyApplier, (void*)ctxt->_dataStream);

        CFReadStreamSetClient((CFReadStreamRef)ctxt->_dataStream, ~0L, (CFReadStreamClientCallBack)_DataStreamCallBack, &streamCtxt);

		_CFTypeScheduleOnMultipleRunLoops(ctxt->_dataStream,ctxt->_runloops);

        CFReadStreamOpen((CFReadStreamRef)ctxt->_dataStream);

        CFRelease(pHost);
        CFRelease(pPort);
        CFRelease(pScheme);
    }
}


/* static */ Boolean
_ProcessHTTPResponse(_CFFTPStreamContext* ctxt, CFStreamError* error) {

    UInt32 code;
    CFHTTPMessageRef resp = (CFHTTPMessageRef)CFReadStreamCopyProperty((CFReadStreamRef)ctxt->_dataStream, kCFStreamPropertyHTTPResponseHeader);

    memset(error, 0, sizeof(error[0]));
    
    if (!resp)
        return FALSE;

    code = CFHTTPMessageGetResponseStatusCode(resp);
    
    if (code < 300)
        __CFBitSet(ctxt->_flags, kFlagBitReadHTTPResponse);
    
    else if (code == 407 && !__CFBitIsSet(ctxt->_flags, kFlagBit407TriedOnce) &&
             CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxyUser) &&
             CFDictionaryGetValue(ctxt->_properties, kCFStreamPropertyFTPProxyPassword))
    {
        Boolean openComplete = FALSE;

        __CFBitSet(ctxt->_flags, kFlagBit407TriedOnce);

        _ReleaseDataReadStream(ctxt);

        CFDictionarySetValue(ctxt->_properties, _kCFStreamPropertyFTPLastHTTPResponse, resp);
        _FTPStreamOpen(ctxt->_userStream, error, &openComplete, ctxt);
        CFDictionaryRemoveValue(ctxt->_properties, _kCFStreamPropertyFTPLastHTTPResponse);
    }
    else {
        error->domain = kCFStreamErrorDomainFTP;
        error->error = code;
    }
    
    CFRelease(resp);

    return TRUE;
}


/* static */ void
_RollOverHTTPRequest(_CFFTPStreamContext* ctxt, CFStreamError* error) {
    
    Boolean openComplete = FALSE;

    _ReleaseDataReadStream(ctxt);
    
    ctxt->_current++;
    ctxt->_error = *error;
    __CFBitClear(ctxt->_flags, kFlagBit407TriedOnce);

    _FTPStreamOpen(ctxt->_userStream, error, &openComplete, ctxt);
}


/* static */ void
_CFStreamSocketCreatedCallBack(int fd, void* ctxt) {

	int yes = 1;
	
	(void)ctxt;		/* unused */
	
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(yes));
}


/* static */ void
_DataStreamCallBack(CFTypeRef stream, CFStreamEventType type, _CFFTPStreamContext* ctxt) {

    if (__CFBitIsSet(ctxt->_flags, kFlagBitIsHTTPRequest) || type != kCFStreamEventEndEncountered) {
        
        CFStreamError error;
        CFTypeID i = CFReadStreamGetTypeID();
        
        if (CFGetTypeID(stream) == i)
            error = CFReadStreamGetError((CFReadStreamRef)stream);
        else 
            error = CFWriteStreamGetError((CFWriteStreamRef)stream);

        if (__CFBitIsSet(ctxt->_flags, kFlagBitIsHTTPRequest) && !__CFBitIsSet(ctxt->_flags, kFlagBitReadHTTPResponse))
        {
            if (type == kCFStreamEventHasBytesAvailable) {
                if (_ProcessHTTPResponse(ctxt, &error)) {

                    if (error.error)
                        type = kCFStreamEventErrorOccurred;
                    else if (__CFBitIsSet(ctxt->_flags, kFlagBit407TriedOnce))
                        return;		// Do not signal the event since having to retry.
                }
                else if (ctxt->_current < CFArrayGetCount(ctxt->_proxies)) {

                    _RollOverHTTPRequest(ctxt, &error);

                    if (error.error)
                        type = kCFStreamEventErrorOccurred;
                    else
                        return;		// Do not signal the event since having to retry.
                }
            }
            else if ((type == kCFStreamEventErrorOccurred) && (ctxt->_current < CFArrayGetCount(ctxt->_proxies))) {

                _RollOverHTTPRequest(ctxt, &error);

                if (error.error)
                    type = kCFStreamEventErrorOccurred;
                else
                    return;		// Do not signal the event since having to retry.
            }
        }

        if (CFGetTypeID(ctxt->_userStream) == i)
            CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, type, &error);
        else
            CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, type, &error);
    }
}


/* static */ void
_ReleaseDataReadStream(_CFFTPStreamContext* ctxt) {

    CFArrayRef a = ctxt->_runloops;
    CFReadStreamRef s = (CFReadStreamRef)ctxt->_dataStream;

    CFReadStreamSetClient(s, 0, NULL, NULL);
	
	_CFTypeUnscheduleFromMultipleRunLoops(s, a);

    CFReadStreamClose(s);
    CFRelease(s);
    ctxt->_dataStream = NULL;
}


/* static */ void
_SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address,
                const void *data, _CFFTPStreamContext* ctxt)
{
    CFStreamClientContext streamCtxt = {0, ctxt, NULL, NULL, NULL};
    CFAllocatorRef alloc = CFGetAllocator(ctxt->_properties);
    
    if (!data || (*((CFSocketNativeHandle*)data) == -1)) {
        CFStreamError error = {_kCFStreamErrorDomainNativeSockets, *((int*)data)};
        _ReportError(ctxt, &error);
    }
    
    if (type != kCFSocketAcceptCallBack)
        return;
    
    if (__CFBitIsSet(ctxt->_flags, kFlagBitPerformUpload))
		_CFSocketStreamCreatePair(alloc, NULL, 0, *((CFSocketNativeHandle*)data), NULL, NULL, (CFWriteStreamRef*)&ctxt->_dataStream);
    else
		_CFSocketStreamCreatePair(alloc, NULL, 0, *((CFSocketNativeHandle*)data), NULL, (CFReadStreamRef*)&ctxt->_dataStream, NULL);
    
    CFDictionaryApplyFunction(ctxt->_properties, (CFDictionaryApplierFunction)_StreamPropertyApplier, (void*)ctxt->_dataStream);
    
    if (CFGetTypeID(ctxt->_dataStream) == CFReadStreamGetTypeID()) {
    
        CFReadStreamSetClient((CFReadStreamRef)ctxt->_dataStream, ~0L, (CFReadStreamClientCallBack)_DataStreamCallBack, &streamCtxt);
        
		_CFTypeScheduleOnMultipleRunLoops(ctxt->_dataStream, ctxt->_runloops);
        
        CFReadStreamOpen((CFReadStreamRef)ctxt->_dataStream);
    }
    else {
    
        CFWriteStreamSetClient((CFWriteStreamRef)ctxt->_dataStream, ~0L, (CFWriteStreamClientCallBack)_DataStreamCallBack, &streamCtxt);
        
		_CFTypeScheduleOnMultipleRunLoops(ctxt->_dataStream, ctxt->_runloops);
        
        CFWriteStreamOpen((CFWriteStreamRef)ctxt->_dataStream);
    }
    
    _InvalidateServer(ctxt);
}


/* static */ void
_StreamPropertyApplier(CFTypeRef key, CFTypeRef value, CFTypeRef stream) {
    
    if (CFGetTypeID(stream) == CFReadStreamGetTypeID())
        CFReadStreamSetProperty((CFReadStreamRef)stream, key, value);
    else
        CFWriteStreamSetProperty((CFWriteStreamRef)stream, key, value);
}


/* static */ void
_ReportError(_CFFTPStreamContext* ctxt, CFStreamError* error) {

    if (ctxt->_connection &&
        (((_CFFTPNetConnectionContext*)_CFNetConnectionGetInfoPointer(ctxt->_connection))->_state == kFTPStateConnect) &&
        (ctxt->_current < CFArrayGetCount(ctxt->_proxies)))
    {
        ctxt->_current++;
        ctxt->_error = *error;
        _CFNetConnectionErrorOccurred(ctxt->_connection, error);
        return;
    }

    __CFBitSet(ctxt->_flags, kFlagBitGotError);

    if (ctxt->_connection)
        _CFNetConnectionErrorOccurred(ctxt->_connection, error);
    
    if (ctxt->_dataStream) {
		
        if (CFGetTypeID(ctxt->_dataStream) == CFReadStreamGetTypeID())
            _ReleaseDataReadStream(ctxt);
		
        else {
			
			_CFTypeInvalidate(ctxt->_dataStream);
			_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_dataStream, ctxt->_runloops);
            
            CFWriteStreamClose((CFWriteStreamRef)(ctxt->_dataStream));
            CFRelease(ctxt->_dataStream);
            ctxt->_dataStream = NULL;
        }
    }
	
    if (CFGetTypeID(ctxt->_userStream) == CFReadStreamGetTypeID())
        CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, error);
    else
        CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, kCFStreamEventErrorOccurred, error);
}


/* static */ void
_ConnectionComplete(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    CFArrayRef a = ftpCtxt->_runloops;
    int i, count = CFArrayGetCount(a);
    for (i = 0; i < count; i += 2) {
        _CFNetConnectionUnschedule(ftpCtxt->_connection,
                                   ftpCtxt,
                                   (CFRunLoopRef)CFArrayGetValueAtIndex(a, i),
                                   (CFStringRef)CFArrayGetValueAtIndex(a, i + 1));
    }
    
    ctxt->_state = kFTPStateIdle;
    
    _CFNetConnectionRequestIsComplete(ftpCtxt->_connection, ftpCtxt);
    _CFNetConnectionResponseIsComplete(ftpCtxt->_connection, ftpCtxt);

    // 3266164 Let the end event come from the data stream at this point.  It's
    // now properly unwound from the connection, so let things progress naturally.
    if (!ftpCtxt->_server && !ftpCtxt->_dataStream) {
        if (CFGetTypeID(ftpCtxt->_userStream) == CFReadStreamGetTypeID())
            CFReadStreamSignalEvent((CFReadStreamRef)ftpCtxt->_userStream, kCFStreamEventEndEncountered, NULL);
        else
            CFWriteStreamSignalEvent((CFWriteStreamRef)ftpCtxt->_userStream, kCFStreamEventEndEncountered, NULL);
    }
}


/* static */ Boolean
_PASVAddressParser(const UInt8* buffer, struct sockaddr_in* saddr)
{
    const UInt8* walk = buffer + 3;
    int byteCount = 0;
    u_long	host = 0;
    u_short	port = 0;
    memset(saddr, 0, sizeof(saddr[0]));

    while (*walk) {
        if (isdigit(*walk)) {
            unsigned	temp;

            sscanf((const char*)walk, "%ud", &temp);
            // WARNING: "clever" code follows..
            if (byteCount < 4) {
                host |= (temp << (24 - (8 * byteCount)));
            } else {
                port |= (temp << (8 - (8 * (byteCount - 4))));
            }

            // Break out when all the bytes have been retrieved.
            if (byteCount < 5) {
                byteCount ++;
            } else {
                host = ntohl(host);
                memmove(&saddr->sin_addr, &host, sizeof(u_long));
#if !defined(__WIN32__)
                saddr->sin_len = sizeof(saddr[0]);
#endif
                saddr->sin_family = AF_INET;
                saddr->sin_port = ntohs(port);
                return TRUE;
            }

            // step past the digit to get the next one.
            while (isdigit(*walk))
                walk++;
        }
        walk ++;
    }
    
    return FALSE;
}


/* static */ Boolean
_EPSVPortParser(const UInt8* buffer, struct sockaddr_in6* saddr)
{
    const UInt8* walk = buffer + 3;
    while (*walk) {
        if (!isdigit(*walk)) walk++;
        else {
            unsigned tmp;
            sscanf((const char*)walk, "%ud", &tmp);
            saddr->sin6_port = htons((tmp & 0x0000FFFF));
            return TRUE;
        }
    }
    return FALSE;
}


/* static */ u_char
_GetProtocolFamily(_CFFTPStreamContext* ctxt, UInt8* buffer)
{
    CFDataRef native = NULL;
    socklen_t addrlen = SOCK_MAXADDRLEN;
    struct sockaddr* addr = (struct sockaddr*)&(buffer[0]);
    u_char result = 255;
    
    CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(ctxt->_connection);
    native = CFWriteStreamCopyProperty(wStream, kCFStreamPropertySocketNativeHandle);
    if (native) {
        memset(buffer, 0, addrlen);
        if (!getpeername(*((int*)CFDataGetBytePtr(native)), addr, &addrlen))
            result = addr->sa_family;
        CFRelease(native);
    }
    return result;
}


/* static */ Boolean
_CreateListenerForContext(CFAllocatorRef alloc, _CFFTPStreamContext* ctxt) {

    CFDataRef native = NULL;
    CFDataRef address = NULL;
    CFRunLoopSourceRef src = NULL;
    
    do {
        int yes = 1;
        UInt8 buffer[SOCK_MAXADDRLEN];
        socklen_t addrlen = sizeof(buffer);
        struct sockaddr* addr = (struct sockaddr*)&(buffer[0]);
        struct sockaddr_in* addr4 = (struct sockaddr_in*)&(buffer[0]);
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&(buffer[0]);
        CFSocketContext socketCtxt = {0, ctxt, NULL, NULL, NULL};
    
        CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(ctxt->_connection);
    
        native = CFWriteStreamCopyProperty(wStream, kCFStreamPropertySocketNativeHandle);
        
        if (!native)
            break;
    
        memset(buffer, 0, sizeof(buffer));
            
        if (getsockname(*((int*)CFDataGetBytePtr(native)), addr, &addrlen))
            break;
    
        CFRelease(native);
        native = NULL;

        ctxt->_server = CFSocketCreate(alloc,
                                       addr->sa_family,
                                       SOCK_STREAM,
                                       IPPROTO_TCP,
                                       kCFSocketAcceptCallBack,
                                       (CFSocketCallBack)&_SocketCallBack,
                                       &socketCtxt);
                                                
        if (!ctxt->_server)
            break;
            
        setsockopt(CFSocketGetNative(ctxt->_server), SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes));
            
        if (addr->sa_family == AF_INET)
            addr4->sin_port = 0;
        else
            addr6->sin6_port = 0;

        // Wrap the native address structure for CFSocketCreate.
        address = CFDataCreateWithBytesNoCopy(alloc, (const UInt8*)addr, (addr->sa_family == AF_INET) ? sizeof(addr4[0]) : sizeof(addr6[0]), kCFAllocatorNull);
        
        if (!address)
            break;
			
        // Set the local binding which causes the socket to start listening.
        if (CFSocketSetAddress(ctxt->_server, address) != kCFSocketSuccess)
            break;

        CFRelease(address);
        address = NULL;

        if (!ctxt->_runloops)
            return TRUE;
            
        src = CFSocketCreateRunLoopSource(alloc, ctxt->_server, 0);
        if (!src)
            break;
        
		_CFTypeScheduleOnMultipleRunLoops(src, ctxt->_runloops);

        CFRelease(src);

        return TRUE;

    } while (0);
    
    if (native)
        CFRelease(native);
        
    if (address)
        CFRelease(address);
        
    if (src)
        CFRelease(src);
        
    _InvalidateServer(ctxt);
    
    return FALSE;
}


/* static */ void
_StartTransfer(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    CFStringRef cmd, target = CFURLCopyLastPathComponent(ftpCtxt->_url);
    CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
	
    if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformUpload)) {
        ctxt->_state = kFTPStateSTOR;
        cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPSTORCommandString, target);

        if (ftpCtxt->_dataStream)
            CFWriteStreamOpen((CFWriteStreamRef)ftpCtxt->_dataStream);
    }

    else {
        // Handle the retrieval of a directory listing
        if (CFURLHasDirectoryPath(ftpCtxt->_url) || _IsRoot(ftpCtxt->_url)) {
            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformNLST)) {
                cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPNLSTCommandString, target);
                ctxt->_state = kFTPStateNLST;

                if (ftpCtxt->_dataStream)
                    CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
            }
            else {
                cmd = CFRetain(kCFFTPLISTCommandString);
                ctxt->_state = kFTPStateLIST;

                if (ftpCtxt->_dataStream)
                    CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
            }
        }

        // Handle the retrieval of a file
        else {

            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformSTAT)) {
                ctxt->_state = kFTPStateSIZE;
                cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPSIZECommandString, target);
            }
            else {
                if (!ftpCtxt->_offset) {
                    cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRETRCommandString, target);
                    ctxt->_state = kFTPStateRETR;

                    if (ftpCtxt->_dataStream)
                        CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
                }
                else {
                    cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRESTCommandString, ftpCtxt->_offset);
                    ctxt->_state = kFTPStateREST;
                }
            }
        }
    }

    if (target) CFRelease(target);
    
    _WriteCommand(ctxt, ftpCtxt, cmd);
    CFRelease(cmd);
}


/* static */ void
_InvalidateServer(_CFFTPStreamContext* ctxt) {
        
    if (!ctxt->_server)
        return;
    
    if (ctxt->_runloops) {    
		_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_server, ctxt->_runloops);
    }
    
    CFSocketInvalidate(ctxt->_server);
    CFRelease(ctxt->_server);
    
    ctxt->_server = NULL;
}


/* static */ CFStringRef
_CreatePathForContext(CFAllocatorRef alloc, _CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    CFStringRef path = CFURLCopyFileSystemPath(ftpCtxt->_url, kCFURLPOSIXPathStyle);
    if (!CFStringGetLength(path)) {
        CFRelease(path);
        path = CFRetain(kCFFTPRootPathString);
    }

    // 3619570 path prefixed with "//" indicates root of "/".  This mimics
    // Jaguar and IE behavior.
	if (CFStringHasPrefix(path, kCFFTPForcedRootPathPrefix)) {
		CFStringRef temp = CFStringCreateWithSubstring(alloc, path, CFRangeMake(1, CFStringGetLength(path) - 1));
		if (temp) {
			CFRelease(path);
			path = temp;
		}
	}
	
    else if (ctxt->_root) {
        CFStringRef temp = CFStringCreateWithFormat(alloc, NULL, kCFFTPPathFormatString, ctxt->_root, path);
        if (temp) {
            CFRelease(path);
            path = temp;
        }
    }
    
    return path;
}


/* static */ void
_WriteCommand(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, CFStringRef cmd) {
	
    UInt8* buffer;
    CFIndex dlen = CFDataGetLength(ctxt->_sendBuffer);
    CFIndex slen = CFStringGetLength(cmd);
    CFIndex req = CFStringGetBytes(cmd, CFRangeMake(0, slen), kCFStringEncodingMacRoman, '_', FALSE, NULL, 0, NULL);
    
    CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(ftpCtxt->_connection);
	
    if ((dlen - ctxt->_sendCount) < req)
        CFDataSetLength(ctxt->_sendBuffer, dlen + req - (dlen - ctxt->_sendCount));
	
    buffer = CFDataGetMutableBytePtr(ctxt->_sendBuffer);
    
    CFStringGetBytes(cmd, CFRangeMake(0, slen), kCFStringEncodingMacRoman, '_', FALSE, buffer + ctxt->_sendCount, req, NULL);
    
    ctxt->_sendCount += req;
    
    if (ctxt->_sendCount && CFWriteStreamCanAcceptBytes(wStream)) {
        _FTPRequestStreamCallBack(ftpCtxt,
                                  wStream,
                                  kCFStreamEventCanAcceptBytes,
                                  ftpCtxt->_connection,
                                  ctxt);
    }
}


/* static */ void
_HandleResponse(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    CFIndex count = ctxt->_recvCount;

    while (count >= 4) {

        UInt32 result = -1;
        UInt8* buffer = (UInt8*)CFDataGetMutableBytePtr(ctxt->_recvBuffer);
        Boolean isMultiline = FALSE;
        
        UInt8* newline = memchr(buffer, '\n', count);
        if (!newline) break;

		if (isdigit(buffer[0]) && isdigit(buffer[1]) && isdigit(buffer[2])) {
			
            result = ((buffer[0] - '0') * 100) + ((buffer[1] - '0') * 10) + (buffer[2] - '0');
			
			if (buffer[3] == '-')
				isMultiline = TRUE;
			
			else if (buffer[3] != ' ')
				result = -1;
		}
            
        if (__CFBitIsSet(ctxt->_flags, kFlagBitMultiline)) {
                
            if ((result == ctxt->_result) && !isMultiline)
                __CFBitClear(ctxt->_flags, kFlagBitMultiline);
        }
        else {

            if (*(newline - 1) != '\r') break;
            
            ctxt->_result = result;
            
            if (!(ctxt->_result > 99) && (ctxt->_result < 600)) {
            
                CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
                _ReportError(ftpCtxt, &error);
                break;
            }
                
            if (isMultiline)
                __CFBitSet(ctxt->_flags, kFlagBitMultiline);
            else
                __CFBitClear(ctxt->_flags, kFlagBitMultiline);
        }
		
		_AdvanceStateMachine(ctxt, ftpCtxt, buffer, newline - buffer, __CFBitIsSet(ctxt->_flags, kFlagBitMultiline));

        newline++;

        count -= (newline - buffer);
        memmove(buffer, newline, count);
        ctxt->_recvCount = count;
    }
}


/* static */ CFURLRef
_ConvertToCFFTPHappyURL(CFURLRef url) {

    CFURLRef result = NULL;
    UInt8 stack_buffer[2048];
    UInt8* buffer = &stack_buffer[0];
    CFURLRef tmp = CFURLCopyAbsoluteURL(url);
    CFIndex length;

    if (!tmp)
        return NULL;
    url = tmp;
    
    length = CFURLGetBytes(url, buffer, sizeof(stack_buffer));

    if (-1 == length) {

        CFIndex req = CFURLGetBytes(url, NULL, 0);
        buffer = (UInt8*)malloc(req);
        if (!buffer) {
			CFRelease(tmp);
            return NULL;
		}

        length = CFURLGetBytes(url, buffer, req);
    }

    result = CFURLCreateAbsoluteURLWithBytes(CFGetAllocator(url), buffer, length, kCFStringEncodingMacRoman, NULL, FALSE);

    if (buffer != &stack_buffer[0])
        free(buffer);

	CFRelease(tmp);

    return result;
}


/* static */ Boolean
_ReadModeBits(const UInt8* str, int* mode) {

    // Function returns TRUE if the string appears to be
    // recognizable mode bits.  Returns false otherwise.
    Boolean result = TRUE;
    int i;
	
	for (i = 0; result && (i < 9); i += 3) {
		
		if (str[i] == 'r') *mode |= (1 << (8 - i));		// See if it's readable
		else if (str[i] != '-') result = FALSE;			// Make sure it's another valid character then
		
		if (str[i + 1] == 'w') *mode |= (1 << (7 - i));		// See if it's writable
		else if (str[i + 1] != '-') result = FALSE;			// Make sure it's another valid character then
		
		switch (str[i + 2]) {
			
			case 'x':
				*mode |= (1 << (6 - i));
				break;
				
			case 's':
				*mode |= (1 << (6 - i));
				if ((i / 3) == 0)
					*mode |= 04000;
				else if ((i / 3) == 1)
					*mode |= 02000;
				break;
				
			case 't':
				*mode |= (1 << (6 - i));
				if ((i / 3) == 2)
					*mode |= 01000;
				break;
				
			case '-':
				break;
				
			case 'S':
				if ((i / 3) == 0)
					*mode |= 04000;
				else if ((i / 3) == 1)
					*mode |= 02000;
				break;
				
			case 'T':
				if ((i / 3) == 2)
					*mode |= 01000;
				break;
				
			default:
				result = FALSE;
				break;				// Any other character is invalid
		}
	}

    return result;
}


/* static */ Boolean
_ReadSize(const UInt8* str, UInt64* size) {

    const UInt8* iter = str;

    *size = 0;

    if (!isdigit(*iter))
        return FALSE;

    while (isdigit(*iter))
        iter++;
    
    if (!isspace(*iter))
        return FALSE;

#if defined(__WIN32__)
    *size = _atoi64(str);
#else
    *size = strtouq((const char*)str, NULL, 10);
#endif

    return TRUE;
}


/* static */ CFStringRef
_CFStringCreateCopyWithStrippedHTML(CFAllocatorRef alloc, CFStringRef theString) {
	
	CFStringRef result = NULL;
	
	/* If it doesn't smell like html, return the retained argument. */
	if (!CFStringHasPrefix(theString, kHTMLTagOpen) || !CFStringHasSuffix(theString, kHTMLTagClose))
		result = CFStringCreateCopy(alloc, theString);

	/* Looks like it might be html. */
	else {
		
		/* Look forward for the close of the opening tag. */
		CFRange r1 = CFStringFind(theString, kHTMLTagClose, 0);
		
		/* Look backward for the open of the closing tag. */
		CFRange r2 = CFStringFind(theString, kHTMLTagOpen, kCFCompareBackwards);
		
		/* If there are no bytes between the two, just return the retained argument. */
		if (r1.location >= r2.location)
			result = CFStringCreateCopy(alloc, theString);
		
		/* Create a copy of the bytes between the two tags. */
		else {
			r1.length = (r2.location - r1.location) - 1;
			r1.location += 1;
			result = CFStringCreateWithSubstring(alloc, theString, r1);
		}
	}

	return result;
}


/* static */ const UInt8*
_CFFTPGetDateTimeFunc(CFAllocatorRef alloc, const UInt8* str, CFIndex length, CFDateRef* date) {

    static const char kMonthStrs[12][3] = {
		{'J', 'a', 'n'},
		{'F', 'e', 'b'},
		{'M', 'a', 'r'},
		{'A', 'p', 'r'},
		{'M', 'a', 'y'},
		{'J', 'u', 'n'},
		{'J', 'u', 'l'},
		{'A', 'u', 'g'},
		{'S', 'e', 'p'},
		{'O', 'c', 't'},
		{'N', 'o', 'v'},
		{'D', 'e', 'c'}
	};

    CFIndex i;
    Boolean hourIsYear = TRUE;
    SInt8 month, day, minute = 0;
    SInt32 hour = 0;

    *date = NULL;

    if (length < 9)
        return NULL;
    
    for (month = 0; month < (sizeof(kMonthStrs) / sizeof(kMonthStrs[0])); month++) {

        if (!memcmp(str, kMonthStrs[month], 3)) {
            break;
        }
    }

    if ((month == (sizeof(kMonthStrs) / sizeof(kMonthStrs[0]))) || !isspace(str[3]))
        return NULL;

    month++;
    
    i = 4;
    while ((i < length) && !isdigit(str[i]))
        i++;

    if (i == length)
        return NULL;

    day = (str[i++] - '0');
    if (i == length)
        return NULL;

    if (isdigit(str[i])) {
        day *= 10;
        day += (str[i++] - '0');
    }
        
    if ((i == length) || !isspace(str[i]))
        return NULL;

    while ((i < length) && !isdigit(str[i]))
        i++;

    while ((i < length) && isdigit(str[i])) {
        hour *= 10;
        hour += (str[i++] - '0');
    }

    if ((i < length) && (str[i] == ':')) {
        
        hourIsYear = FALSE;
        
        i++;
        
        while ((i < length) && isdigit(str[i])) {
            minute *= 10;
            minute += (str[i++] - '0');
        }
    }

    if ((i == length) || isspace(str[i])) {
        
		CFTimeZoneRef tz = CFTimeZoneCopyDefault();
        CFAbsoluteTime t = CFAbsoluteTimeGetCurrent();
        CFGregorianDate d = CFAbsoluteTimeGetGregorianDate(t, tz);
        
        if (hourIsYear) {
            
            if (hour < 100)
                d.year = 1900 + hour;
            else
                d.year = hour;
            
            d.hour = 0;
            d.minute = 0;
            d.second = 0;
			
			d.month = month;
			d.day = day;
        }
        
        else {
            CFAbsoluteTime t2;
			CFGregorianDate d2 = CFAbsoluteTimeGetGregorianDate((t + 86400.0), tz);
			
            d.hour = hour & 0xFF;
            d.minute = minute;
            d.second = 0;
			
			d.month = month;
			d.day = day;
			d.year = d2.year;
			
			t2 = CFGregorianDateGetAbsoluteTime(d, tz);
			if (t2 > (t + 86400.0))
				d.year--;
        }
		
        t = CFGregorianDateGetAbsoluteTime(d, tz);
		if (tz)
			CFRelease(tz);
        *date = CFDateCreate(alloc, t);

        return &str[i];
    }

    return NULL;
}


/* static */ void
_ProxyStreamCallBack(CFReadStreamRef proxyStream, _CFFTPStreamContext* ctxt) {

    Boolean complete = FALSE;
    
    ctxt->_proxies = _CFNetworkCopyProxyFromProxyStream(ctxt->_proxyStream, &complete);

    if (complete) {

        CFStreamError error = CFReadStreamGetError(ctxt->_proxyStream);
	
		_CFTypeUnscheduleFromMultipleRunLoops(ctxt->_proxyStream, ctxt->_runloops);
        
        CFRelease(ctxt->_proxyStream);
        ctxt->_proxyStream = NULL;

        if (ctxt->_proxies) {

            Boolean open;
    
            _FTPStreamOpen(ctxt->_userStream, &error, &open, ctxt);
    
            if (open) {
    
                CFStreamEventType event = kCFStreamEventErrorOccurred;
                if (!error.error)
                    event = kCFStreamEventOpenCompleted;
    
                if (CFGetTypeID(ctxt->_userStream) == CFReadStreamGetTypeID())
                    CFReadStreamSignalEvent((CFReadStreamRef)ctxt->_userStream, event, &error);
                else
                    CFWriteStreamSignalEvent((CFWriteStreamRef)ctxt->_userStream, event, &error);
            }
        }
        
        else {
            if (!error.error) {
                error.domain = kCFStreamErrorDomainPOSIX;
                error.error = errno;
                if (!error.error)
                    error.error = EIO;
            }
            _ReportError(ctxt, &error);
        }
    }
}


#if defined(__WIN32__)
extern void
_CFFTPCleanup(void) {

    __CFSpinLock(&gFTPSpinLock);
    if (gFTPConnectionCache != NULL) {
        releaseConnectionCache(gFTPConnectionCache);
        gFTPConnectionCache = NULL;
    }
    __CFSpinUnlock(&gFTPSpinLock);
}
#endif


/* static */ CFIndex
_FindLine(const UInt8 *buffer, CFIndex bufferLength, const UInt8** start, const UInt8** end) {
/*
 * This function finds lines delimited on either side by CR or LF characters.
*/
	CFIndex consumed;
	
	*start = NULL;
	*end = NULL;
	consumed = 0;
	
	if ( (buffer != NULL) && (bufferLength != 0) ) {
		const UInt8* lastBufChar;
		const UInt8* startOfLine;
		
		lastBufChar = buffer + bufferLength - 1;
		
		/* find the start of the line... the first non CR or LF character */
		startOfLine = buffer;
		while ( startOfLine <= lastBufChar ) {
			if ( *startOfLine != '\r' && *startOfLine != '\n' ) {
				break;
			}
			++startOfLine;
		}
		
		/* if there characters left, see if there's a line */
		if ( startOfLine <= lastBufChar ) {
			const UInt8* endOfLine;
			const UInt8* firstend = NULL;
			
			/* find the end of the line... the character before the next CR or LF character (if any) */
			endOfLine = startOfLine;
			while ( endOfLine <= lastBufChar ) {
				if ( *endOfLine == '\r' || *endOfLine == '\n' ) {
					break;
				}
				++endOfLine;
			}
			firstend = endOfLine;
			
			/* if endOfLine is still within buffer, we have a line */
			if ( endOfLine <= lastBufChar ) {
				const UInt8* lastend;
				
				/* return the first and last characters of the line */
				*start = startOfLine;
				*end = endOfLine;
				
				/* find the last CR or LF character after the line */
				lastend = firstend;
				while ( lastend <= lastBufChar ) {
					if ( *lastend != '\r' && *lastend != '\n' ) {
						break;
					}
					++lastend;
				}
				
				/* consume everthing up through the last CR or LF character */
				consumed = lastend - buffer;
			}
			else {
				/* no line -- just consume the CR and LF characters at the beginning of the buffer */
				consumed = startOfLine - buffer;
			}
		}
		else {
			/* the buffer is all CR or LF characters -- consume them */
			consumed = bufferLength;
		}
	}
	
	return ( consumed );
}


#if 0
#pragma mark -
#pragma mark State Machine
#endif

/* static */ void
_AdvanceStateMachine(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length, Boolean isMultiLine) {

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
        
        if (ctxt->_state <= kFTPStateIdle)
            __CFBitClear(ctxt->_flags, kFlagBitReturnToIdle);
        else if (ctxt->_state < kFTPStateRETR) {
            if (!isMultiLine) {
                ctxt->_state = kFTPStateIdle;
                _StartProcess(ctxt, ftpCtxt);
            }
            return;
        }
    }

    switch (ctxt->_state) {

        case kFTPStateConnect:
            if (!isMultiLine) _HandleConnect(ctxt, ftpCtxt, line, length);
            break;

        case kFTPStateUSER:
            if (!isMultiLine) _HandleUsername(ctxt, ftpCtxt);
            break;

        case kFTPStatePASS:
            if (!isMultiLine) _HandlePassword(ctxt, ftpCtxt);
            break;
			
        case kFTPStateSYST:
            if (!isMultiLine) _HandleSystem(ctxt, ftpCtxt, line, length);
            break;
            
        case kFTPStateSITEDIRSTYLE:
            if (!isMultiLine) _HandleSiteDirStyle(ctxt, ftpCtxt, line, length);
            break;
            
        case kFTPStateSITETRUTH:
            if (!isMultiLine) _HandleSiteTruth(ctxt, ftpCtxt);
            break;
            
        case kFTPStatePWD:
            if (!isMultiLine) _HandlePrintWorkingDirectory(ctxt, ftpCtxt, line, length);
            break;

        case kFTPStateTYPE:
            if (!isMultiLine) _HandleType(ctxt, ftpCtxt);
            break;

        case kFTPStatePASV:
            if (!isMultiLine) _HandlePassive(ctxt, ftpCtxt, line, length);
            break;

        case kFTPStatePORT:
            if (!isMultiLine) _HandlePort(ctxt, ftpCtxt);
            break;
			
        case kFTPStateSIZE:
            if (!isMultiLine) _HandleSize(ctxt, ftpCtxt, line, length);
            break;
			
        case kFTPStateSTAT:
            _HandleStat(ctxt, ftpCtxt, line, length, isMultiLine);
            break;
            
        case kFTPStateREST:
            if (!isMultiLine) _HandleRestart(ctxt, ftpCtxt);
            break;

        case kFTPStateRETR:
            if (!isMultiLine) _HandleRetrieve(ctxt, ftpCtxt);
            break;
		
        case kFTPStateNLST:
            if (!isMultiLine) _HandleNameList(ctxt, ftpCtxt);
            break;

        case kFTPStateCWD:
            if (!isMultiLine) _HandleChangeDirectory(ctxt, ftpCtxt);
            break;

        case kFTPStateLIST:
            if (!isMultiLine) _HandleList(ctxt, ftpCtxt);
            break;
            
        case kFTPStateSTOR:
            if (!isMultiLine) _HandleStore(ctxt, ftpCtxt);
            break;
            
        case kFTPStateMKD:
            if (!isMultiLine) _HandleMakeDirectory(ctxt, ftpCtxt);
            break;
            
        case kFTPStateRMD:
            if (!isMultiLine) _HandleRemoveDirectory(ctxt, ftpCtxt);
            break;
            
        case kFTPStateDELE:
            if (!isMultiLine) _HandleDelete(ctxt, ftpCtxt);
            break;
            
        case kFTPStateRNFR:
            if (!isMultiLine) _HandleRenameFrom(ctxt, ftpCtxt);
            break;
            
        case kFTPStateRNTO:
            if (!isMultiLine) _HandleRenameTo(ctxt, ftpCtxt);
            break;

        default:
            break;
    }
}


/* static */ void
_HandleConnect(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Valid returns for connect are:
    //		120, 220, 421

    if (ctxt->_result < 200)
        return;
            
    else if (ctxt->_result >= 300) {
            
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    else {
        
        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_runloops);
        
        CFStringRef system = CFStringCreateWithBytes(alloc,
                                                     line,
                                                     length,
                                                     kCFStringEncodingUTF8,
                                                     FALSE);
		
		if (!system) {
			system = CFStringCreateWithBytes(alloc,
											 line,
											 length,
											 kCFStringEncodingISOLatin1,
											 FALSE);
		}
		
		if (system) {
		
			CFRange range = CFRangeMake(0, CFStringGetLength(system));
			
			if (CFStringFindWithOptions(system, kCFFTPOSXSystemString, range, 0, NULL))
				__CFBitSet(ctxt->_flags, kFlagBitIsXServer);
			
			CFRelease(system);
		}
        
        ctxt->_state = kFTPStateUSER;
    }
}


/* static */ void
_HandleUsername(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for USER are:
    //		230, 331, 332, 421, 500, 501, 530

    if ((ctxt->_result < 200) || (ctxt->_result >= 400)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    
    else if (ctxt->_result >= 300) {

        CFStringRef cmd;
        CFStringRef pass = CFDictionaryGetValue(ftpCtxt->_properties, kCFStreamPropertyFTPPassword);

        if (pass)
            CFRetain(pass);
        else {
            pass = CFURLCopyPassword(ftpCtxt->_url);
            if (!pass)
                pass = CFRetain(kAnonymousPasswordString);
        }

        cmd = CFStringCreateWithFormat(CFGetAllocator(ftpCtxt->_runloops), NULL, kCFFTPPASSCommandString, pass);

        CFRelease(pass);

        if (cmd) {
            ctxt->_state = kFTPStatePASS;
            _WriteCommand(ctxt, ftpCtxt, cmd);
            CFRelease(cmd);
        }
        else {
            CFStreamError error = {kCFStreamErrorDomainPOSIX, errno};

            if (!error.error)
                error.error = ENOMEM;
            _ReportError(ftpCtxt, &error);
        }
    }

    else {
        ctxt->_state = kFTPStateSYST;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPSYSTCommandString);
    }
}


/* static */ void
_HandlePassword(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for PASS are:
    //		230, 202, 332, 421, 500, 501, 503, 530

    if ((ctxt->_result < 200) || (ctxt->_result >= 400)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    
    else if (ctxt->_result >= 300) {
        // **FIXME** This is not a true failure case but an account
        // is required which is not supported yet.  This would require
        // an issue of the ACCT command.
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    
    else {
        ctxt->_state = kFTPStateSYST;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPSYSTCommandString);
    }
}


/* static */  void
_HandleSystem(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Valid returns for SYST are:
    //		215, 421, 500, 501, 502

    CFStringRef system = NULL;
    CFRange range;

    if ((ctxt->_result >= 200) && (ctxt->_result < 300)) {
        system = CFStringCreateWithBytes(CFGetAllocator(ftpCtxt->_properties),
                                         line,
                                         length,
                                         kCFStringEncodingUTF8,
                                         FALSE);
        range = CFRangeMake(0, CFStringGetLength(system));
    }
    
    if (system && CFStringFindWithOptions(system, kCFFTPWindowsNTSystemString, range, 0, NULL)) {
        ctxt->_state = kFTPStateSITEDIRSTYLE;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPSITEDIRSTYLECommandString);
    }
    else if( __CFBitIsSet(ctxt->_flags, kFlagBitIsXServer)) {
        ctxt->_state = kFTPStateSITETRUTH;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPSITETRUTHCommandString);
    } else { 
        ctxt->_state = kFTPStatePWD; 
        _WriteCommand(ctxt, ftpCtxt, kCFFTPPWDCommandString);
    }

    if (system)
        CFRelease(system);
}


/* static */ void
_HandleSiteDirStyle(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Ignore the results.  If it took it fine and if not, it'll
    // just have to do.
    if ((ctxt->_result >= 200) && (ctxt->_result < 300)) {
    
        CFStringRef system = CFStringCreateWithBytes(CFGetAllocator(ftpCtxt->_properties),
                                                     line,
                                                     length,
                                                     kCFStringEncodingUTF8,
                                                     FALSE);
        CFRange range = CFRangeMake(0, CFStringGetLength(system));

        if (CFStringFindWithOptions(system, kCFFTPMSDOSSystemString, range, 0, NULL)) {
            ctxt->_state = kFTPStateSITEDIRSTYLE;
            _WriteCommand(ctxt, ftpCtxt, kCFFTPSITEDIRSTYLECommandString);
            CFRelease(system);
            return;
        }
        
        CFRelease(system);
    }

    if( __CFBitIsSet(ctxt->_flags, kFlagBitIsXServer)) {
        ctxt->_state = kFTPStateSITETRUTH;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPSITETRUTHCommandString);
    } else { 
        ctxt->_state = kFTPStatePWD;
        _WriteCommand(ctxt, ftpCtxt, kCFFTPPWDCommandString); 
    }
}


/* static */ void
_HandleSiteTruth(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Ignore the results.  If it took it fine and if not, it'll
    // just have to do.

    ctxt->_state = kFTPStatePWD;
    _WriteCommand(ctxt, ftpCtxt, kCFFTPPWDCommandString);
}


/* static */ void
_HandlePrintWorkingDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Valid returns for PWD are:
    //		257, 421, 500, 501, 502, 550

    // Only care about trying to parse out the root directory
    // if PWD succeeded.
    if ((ctxt->_result >= 200) && (ctxt->_result < 300)) {
        
        const UInt8* first = memchr(line, '"', length);
        
        if (first) {
            const UInt8* last = line + length - 1;
            
            first++;
            
            while  ( last != first) {
                if (*last == '"') {
                    
                    if (*(last - 1) == '/')
                        last--;
                    
                    ctxt->_root = CFStringCreateWithBytes(CFGetAllocator(ftpCtxt->_properties),
                                                          first,
                                                          last - first,
                                                          kCFStringEncodingUTF8,
                                                          FALSE);
                    break;
                }
                
                last--;
            }
        }
    }

    ctxt->_state = kFTPStateTYPE;
    _WriteCommand(ctxt, ftpCtxt, kCFFTPTYPECommandString);
}


/* static */ void
_HandleType(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for TYPE are:
    //		200, 421, 501, 504, 530

    if ((ctxt->_result < 200) || (ctxt->_result >= 300)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }

    else {
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
}


/* static */ void
_HandleChangeDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for CWD are:
    //		250, 421, 500, 501, 502, 530, 550

    if (__CFBitIsSet(ctxt->_flags, kFlagBitLeftForDead) && (ctxt->_result == 421)) {
        
        CFStreamError error;
        Boolean openComplete;
        
        _CFNetConnectionLost(ftpCtxt->_connection);
        _CFNetConnectionDequeue(ftpCtxt->_connection, ftpCtxt);
        
        _FTPStreamOpen(ftpCtxt->_userStream, &error, &openComplete, ftpCtxt);
    }

    else if ((ctxt->_result >= 300) || (ctxt->_result < 200)){
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
	__CFBitClear(ctxt->_flags, kFlagBitLeftForDead);
        _ReportError(ftpCtxt, &error);
    }

    else {

        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
        __CFBitClear(ctxt->_flags, kFlagBitLeftForDead);
        
        // Only allow the special SPI stuff on the WriteStream for now
        if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformUpload)) {

            CFStringRef target = CFURLCopyLastPathComponent(ftpCtxt->_url);
			
            // Performing a RMD or DELE
            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitRemoveResource)) {

                CFStringRef cmd;

                if (CFURLHasDirectoryPath(ftpCtxt->_url) || _IsRoot(ftpCtxt->_url)) {
                    ctxt->_state = kFTPStateRMD;
                    cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRMDCommandString, target);
                }
                else {
                    ctxt->_state = kFTPStateDELE;
                    cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPDELECommandString, target);
                }

                _WriteCommand(ctxt, ftpCtxt, cmd);
                CFRelease(cmd);

                CFWriteStreamSignalEvent((CFWriteStreamRef)ftpCtxt->_userStream, kCFStreamEventOpenCompleted, NULL);
            }

            // Performing RNFR->RNTO
            else if (ftpCtxt->_newUrl) {
                CFStringRef cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRNFRCommandString, target);
                ctxt->_state = kFTPStateRNFR;
                _WriteCommand(ctxt, ftpCtxt, cmd);
                CFRelease(cmd);

                CFWriteStreamSignalEvent((CFWriteStreamRef)ftpCtxt->_userStream, kCFStreamEventOpenCompleted, NULL);
            }

            // Performing MKD
            else if (CFURLHasDirectoryPath(ftpCtxt->_url) || _IsRoot(ftpCtxt->_url)) {
                CFStringRef cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPMKDCommandString, target);
                ctxt->_state = kFTPStateMKD;
                _WriteCommand(ctxt, ftpCtxt, cmd);
                CFRelease(cmd);

                CFWriteStreamSignalEvent((CFWriteStreamRef)ftpCtxt->_userStream, kCFStreamEventOpenCompleted, NULL);
            }

            if (target) CFRelease(target);
        }

        if (ctxt->_state == kFTPStateCWD) {

            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformPASV)) {
                UInt8 buf[SOCK_MAXADDRLEN];
                u_char family = _GetProtocolFamily(ftpCtxt, buf);

                ctxt->_state = kFTPStatePASV;
                if (family == AF_INET)
                    _WriteCommand(ctxt, ftpCtxt, kCFFTPPASVCommandString);
                else if (family == AF_INET6)
                    _WriteCommand(ctxt, ftpCtxt, kCFFTPEPSVCommandString);
                else {
                    CFStreamError error = {kCFStreamErrorDomainFTP, 522}; //unkown protocol
                    _ReportError(ftpCtxt, &error);
                }
            }

            else if (_CreateListenerForContext(alloc, ftpCtxt)) {
                CFDataRef addr = CFSocketCopyAddress(ftpCtxt->_server);
                CFStringRef cmd = NULL;
                struct sockaddr_in* sa = (struct sockaddr_in*)CFDataGetBytePtr(addr);
                struct sockaddr_in6* sa6 = (struct sockaddr_in6*)sa;

                if (sa->sin_family == AF_INET) { //ipv4
                    cmd = CFStringCreateWithFormat(alloc,
                                                   NULL,
                                                   kCFFTPPORTCommandString,
                                                   (unsigned int)((UInt8*)(&(sa->sin_addr)))[0],
                                                   (unsigned int)((UInt8*)(&(sa->sin_addr)))[1],
                                                   (unsigned int)((UInt8*)(&(sa->sin_addr)))[2],
                                                   (unsigned int)((UInt8*)(&(sa->sin_addr)))[3],
                                                   (unsigned int)((UInt8*)(&(sa->sin_port)))[0],
                                                   (unsigned int)((UInt8*)(&(sa->sin_port)))[1]);
                } else if (sa->sin_family == AF_INET6) { //ipv6
                    cmd = CFStringCreateWithFormat(alloc,
                                                   NULL,
                                                   kCFFTPEPRTCommandString,
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[0]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[1]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[2]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[3]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[4]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[5]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[6]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_addr)))[7]),
                                                   (unsigned int)ntohs(((UInt16*)(&(sa6->sin6_port)))[0]));
                } else { // bail out for unknown protocol
                    CFStreamError error = {kCFStreamErrorDomainFTP, 522}; //unkown protocol
                    _ReportError(ftpCtxt, &error);
                }

                if (cmd) {
                    ctxt->_state = kFTPStatePORT;
                    _WriteCommand(ctxt, ftpCtxt, cmd);
                    CFRelease(cmd);
                }
            }
            else {
                CFStreamError error = {kCFStreamErrorDomainPOSIX, errno};
#if defined(__WIN32__)
		if (!error.error) {
		    error.error = WSAGetLastError();
		    if (error.error)
			error.domain = kCFStreamErrorDomainWinSock;
		}
#endif
                if (!error.error) {
                    error.error = ENOTCONN;
		    error.domain = _kCFStreamErrorDomainNativeSockets;
		}
                _ReportError(ftpCtxt, &error);
            }
        }
    }
}


/* static */ void
_HandlePassive(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Valid returns for PASV are:
    //		227, 421, 500, 501, 502, 530

    if ((ctxt->_result < 200) || (ctxt->_result >= 300)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    else {
        UInt8 buf[SOCK_MAXADDRLEN];
        u_char family = _GetProtocolFamily(ftpCtxt, buf);
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
                
        if (family == AF_INET6)
            memcpy(&addr6, buf, sizeof(struct sockaddr_in6));
            
        if ((family == AF_INET && !_PASVAddressParser(line, &addr4)) ||
            (family == AF_INET6 && !_EPSVPortParser(line, &addr6)))
        {
            CFStreamError error = {_kCFStreamErrorDomainNativeSockets, EADDRNOTAVAIL};
            _ReportError(ftpCtxt, &error);
        }
        else {
            CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
            CFStreamClientContext streamCtxt = {0, ftpCtxt, NULL, NULL, NULL};
            CFSocketSignature sig;
    
            sig.protocolFamily = family;
            sig.socketType = SOCK_STREAM;
            sig.protocol = IPPROTO_TCP;
            if (family == AF_INET)
                sig.address = CFDataCreate(alloc, (const UInt8*)&addr4, sizeof(addr4));
            else
                sig.address = CFDataCreate(alloc, (const UInt8*)&addr6, sizeof(addr6));

            if (!sig.address) {
                CFStreamError error = {kCFStreamErrorDomainPOSIX, ENOMEM};
                _ReportError(ftpCtxt, &error);
                return;
            }
            
            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformUpload))
				_CFSocketStreamCreatePair(alloc, NULL, 0, 0, &sig, NULL, (CFWriteStreamRef*)&ftpCtxt->_dataStream);
            else
				_CFSocketStreamCreatePair(alloc, NULL, 0, 0, &sig, (CFReadStreamRef*)&ftpCtxt->_dataStream, NULL);

            CFRelease(sig.address);

            if (!ftpCtxt->_dataStream) {
                CFStreamError error = {kCFStreamErrorDomainPOSIX, ENOMEM};
                _ReportError(ftpCtxt, &error);
                return;
            }

            if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformUpload))
                CFWriteStreamSetClient((CFWriteStreamRef)ftpCtxt->_dataStream, ~0L, (CFWriteStreamClientCallBack)_DataStreamCallBack, &streamCtxt); 
            else 
                CFReadStreamSetClient((CFReadStreamRef)ftpCtxt->_dataStream, ~0L, (CFReadStreamClientCallBack)_DataStreamCallBack, &streamCtxt); 
			
			_CFTypeScheduleOnMultipleRunLoops(ftpCtxt->_dataStream, ftpCtxt->_runloops);
            
            CFDictionaryApplyFunction(ftpCtxt->_properties, (CFDictionaryApplierFunction)_StreamPropertyApplier, (void*)ftpCtxt->_dataStream);

            _StartTransfer(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandlePort(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for PORT are:
    //		200, 421, 500, 501, 530
    
    if ((ctxt->_result < 200) || (ctxt->_result >= 300)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    else
        _StartTransfer(ctxt, ftpCtxt);
}


/* static */ void
_HandleStat(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length, Boolean isMultiLine) {

    // Valid returns for STAT are:
    //		211, 212, 213, 421, 450, 500, 501, 502, 530

    if (!ftpCtxt->_attributes) {

        CFFTPCreateParsedResourceListing(CFGetAllocator(ftpCtxt->_properties),
                                         line,
                                         length,
                                         &ftpCtxt->_attributes);
    }

    if (isMultiLine)
        return;

    if (isdigit(line[0])) {

        CFStringRef cmd;
        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
    
		if (!ftpCtxt->_offset) {
			CFStringRef path = _CreatePathForContext(alloc, ctxt, ftpCtxt);

			cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRETRCommandString, path);
			ctxt->_state = kFTPStateRETR;
			CFRelease(path);

			if (ftpCtxt->_dataStream)
				CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
		}
		else {
			cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRESTCommandString, ftpCtxt->_offset);
			ctxt->_state = kFTPStateREST;
		}

        _WriteCommand(ctxt, ftpCtxt, cmd);
        CFRelease(cmd);
    }
}


/* static */ void
_HandleSize(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt, const UInt8* line, CFIndex length) {

    // Valid returns for SIZE are (pulled from IETF working document, may not be set in stone):
    //		213, 500, 501, 502, 550

    CFStringRef cmd;
    CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
	
	// If SIZE failed, attempt to issue the STAT command instead.
	if ((ctxt->_result >= 500) && (ctxt->_result < 600)) {
		
		CFStringRef target = CFURLCopyLastPathComponent(ftpCtxt->_url);
		
		if (ftpCtxt->_attributes) {
			CFRelease(ftpCtxt->_attributes);
			ftpCtxt->_attributes = NULL;
		}
		
		ctxt->_state = kFTPStateSTAT;
		cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPSTATCommandString, target);
		if (target) CFRelease(target);
	}
    
    else {
		
		if (ctxt->_result == 213) {

			CFIndex i = 0;
			
			while ((i < length) && isdigit(line[i]))
				i++;

			while ((i < length) && !isdigit(line[i]))
				i++;

			if (i < length) {
				UInt8* end = NULL;
	#if defined(__WIN32__)
				long long s = _atoi64(&line[i]);
	#else
				long long s = strtouq((const char*)&line[i], (char**)&end, 0);
	#endif
				if (!(((s == ULLONG_MAX) && (errno)) || ((s == 0) && (end == &line[i])))) {
					const void *keys[1], *values[1];

					keys[0]   = kCFFTPResourceSize;
					values[0] = CFNumberCreate(alloc, kCFNumberLongLongType, &s);

					if (values[0]) {
						ftpCtxt->_attributes = CFDictionaryCreate(alloc, keys, values, 1,
																  &kCFTypeDictionaryKeyCallBacks,
																  &kCFTypeDictionaryValueCallBacks);
		
						CFRelease(values[0]);
					}
				}
			}
			
		}

		if (!ftpCtxt->_offset) {
			CFStringRef path = _CreatePathForContext(alloc, ctxt, ftpCtxt);

			cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRETRCommandString, path);
			ctxt->_state = kFTPStateRETR;
			CFRelease(path);

			if (ftpCtxt->_dataStream)
				CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
		}
		else {
			cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRESTCommandString, ftpCtxt->_offset);
			ctxt->_state = kFTPStateREST;
		}
	}

	_WriteCommand(ctxt, ftpCtxt, cmd);
	CFRelease(cmd);
}


/* static */ void
_HandleRestart(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for REST are:
    //		350, 421, 500, 501, 502, 530

    if ((ctxt->_result < 300) || (ctxt->_result >= 400)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    
    else {
        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
        CFStringRef cmd, path = _CreatePathForContext(alloc, ctxt, ftpCtxt);
    
        cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRETRCommandString, path);
        CFRelease(path);
        
        ctxt->_state = kFTPStateRETR;
                
        if (ftpCtxt->_dataStream)
            CFReadStreamOpen((CFReadStreamRef)ftpCtxt->_dataStream);
        
        _WriteCommand(ctxt, ftpCtxt, cmd);
        CFRelease(cmd);
    }
}


/* static */ void
_HandleRetrieve(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for RETR are:
    //		110, 125, 150, 226, 250, 421, 425, 426, 450, 451, 500, 501, 530, 550

    if (ctxt->_result < 200)
        return;

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
        
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
    
        if (ctxt->_result >= 300) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        
        else if (ctxt->_result >= 200) {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleNameList(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for NLST are:
    //		125, 150, 226, 250, 425, 426, 451, 450, 500, 501, 502, 421, 530
    
    if (ctxt->_result < 200)
        return;
        
    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
        
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if (ctxt->_result >= 300) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        
        else {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleList(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {
	
    // Valid returns for LIST are:
    //		125, 150, 226, 250, 421, 425, 426, 450, 451, 500, 501, 502, 530
	
    if (ctxt->_result < 200)
        return;
    
    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
        
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if (ctxt->_result >= 300) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else if (ctxt->_result < 300) {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleStore(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for STOR are:
    //		110, 125, 150, 226, 250, 421, 425, 426, 450, 451, 452,
    //		500, 501, 530, 532, 551, 552, 553

    if (ctxt->_result < 200)
        return;
    
    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
        
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if (ctxt->_result >= 300) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else if (ctxt->_result < 300) {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleMakeDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for MKD are:
    //		257, 421, 500, 501, 502, 530, 550

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
            
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if ((ctxt->_result >= 300) || (ctxt->_result < 200)) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleRemoveDirectory(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for RMD are:
    //		250, 421, 500, 501, 502, 530, 550

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
            
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if ((ctxt->_result >= 300) || (ctxt->_result < 200)) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleDelete(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for DELE are:
    //		250, 421, 450, 500, 501, 502, 530, 550

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
            
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if ((ctxt->_result >= 300) || (ctxt->_result < 200)) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_HandleRenameFrom(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

	// Valid returns for RNFR are:
	//		350, 421, 450, 500, 501, 502, 530, 550
    
    if ((ctxt->_result < 300) || (ctxt->_result >= 400)) {
        CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
        _ReportError(ftpCtxt, &error);
    }
    else {
        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
        CFStringRef cmd, path;
        CFURLRef url = ftpCtxt->_url;
        
        // **FIXME** Total hack for easily getting the path for the new URL.
        ftpCtxt->_url = ftpCtxt->_newUrl;
        path = _CreatePathForContext(alloc, ctxt, ftpCtxt);
        ftpCtxt->_url = url;
        
        ctxt->_state = kFTPStateRNTO;
        cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPRNTOCommandString, path);
        CFRelease(path);
        
        _WriteCommand(ctxt, ftpCtxt, cmd);
        CFRelease(cmd);
    }
}


/* static */ void
_HandleRenameTo(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {

    // Valid returns for RNTO are:
    //		250, 421, 500, 501, 502, 503, 530, 532, 553

    if (__CFBitIsSet(ctxt->_flags, kFlagBitReturnToIdle)) {
            
        ctxt->_state = kFTPStateIdle;
        _StartProcess(ctxt, ftpCtxt);
    }
    else {
        if ((ctxt->_result >= 300) || (ctxt->_result < 200)) {
            CFStreamError error = {kCFStreamErrorDomainFTP, ctxt->_result};
            _ReportError(ftpCtxt, &error);
        }
        else {
            _ConnectionComplete(ctxt, ftpCtxt);
        }
    }
}


/* static */ void
_StartProcess(_CFFTPNetConnectionContext* ctxt, _CFFTPStreamContext* ftpCtxt) {
    
    if (__CFBitIsSet(ftpCtxt->_flags, kFlagBitLogInOnly)) {
        if (CFGetTypeID(ftpCtxt->_userStream) == CFReadStreamGetTypeID())
            CFReadStreamSignalEvent((CFReadStreamRef)ftpCtxt->_userStream, kCFStreamEventOpenCompleted, NULL);
        else
            CFWriteStreamSignalEvent((CFWriteStreamRef)ftpCtxt->_userStream, kCFStreamEventOpenCompleted, NULL);
        _ConnectionComplete(ctxt, ftpCtxt);
    }
    else {

        CFStringRef path, cmd;
        CFAllocatorRef alloc = CFGetAllocator(ftpCtxt->_properties);
        
        __CFBitClear(ctxt->_flags, kFlagBitReturnToIdle);

        // Check if this request is for a directory listing.  If so, need to CWD
        // all the way to the give url.  All other requests CWD to the directory
        // above the last, path component.
        if (!__CFBitIsSet(ftpCtxt->_flags, kFlagBitPerformUpload) &&
            (CFURLHasDirectoryPath(ftpCtxt->_url) || _IsRoot(ftpCtxt->_url)))
        {
            path = _CreatePathForContext(alloc, ctxt, ftpCtxt);
        }
        else {
            // **FIXME** Total hack for easily getting the path without the last component.
            CFURLRef old = ftpCtxt->_url;
            ftpCtxt->_url = CFURLCreateCopyDeletingLastPathComponent(alloc, old);
            path = _CreatePathForContext(alloc, ctxt, ftpCtxt);
            CFRelease(ftpCtxt->_url);
            ftpCtxt->_url = old;
        }
        
        cmd = CFStringCreateWithFormat(alloc, NULL, kCFFTPCWDCommandString, path);
        CFRelease(path);
        
        ctxt->_state = kFTPStateCWD;
        _WriteCommand(ctxt, ftpCtxt, cmd);
        
        CFRelease(cmd);
    }
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif

/* CF_EXPORT */ CFWriteStreamRef
CFWriteStreamCreateWithFTPURL(CFAllocatorRef alloc, CFURLRef ftpURL) {
    
    CFWriteStreamRef result = NULL;
    CFStringRef temp;
    _CFFTPStreamContext* ctxt;

    if (!ftpURL || !(ftpURL = _ConvertToCFFTPHappyURL(ftpURL)))
        return result;
    
    temp = CFURLCopyScheme(ftpURL);
    if (!temp) {
        CFRelease(ftpURL);
        return result;
    }
        
    if ((CFStringCompare(temp, kFTPSchemeString, 0) != kCFCompareEqualTo) &&
        (CFStringCompare(temp, kFTPSSchemeString, 0) != kCFCompareEqualTo))
    {
        CFRelease(ftpURL);
        CFRelease(temp);
        return result;
    }
    
    CFRelease(temp);
    
    temp = CFURLCopyHostName(ftpURL);
    if (!temp) {
        CFRelease(ftpURL);
        return result;
    }
    
    CFRelease(temp);

    ctxt = (_CFFTPStreamContext*)CFAllocatorAllocate(alloc,
                                                     sizeof(ctxt[0]),
                                                     0);
    if (ctxt) {

        memset(ctxt, 0, sizeof(ctxt[0]));
        
        __CFBitSet(ctxt->_flags, kFlagBitPerformPASV);
        __CFBitSet(ctxt->_flags, kFlagBitPerformUpload);

        ctxt->_url = CFURLCopyAbsoluteURL(ftpURL);

        ctxt->_runloops = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
        ctxt->_properties = CFDictionaryCreateMutable(alloc,
                                                      0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);

        if (ctxt->_url && ctxt->_runloops && ctxt->_properties) {
			
			CFWriteStreamCallBacks _FTPWriteStreamCallBacks;

			memset(&_FTPWriteStreamCallBacks, 0, sizeof(_FTPWriteStreamCallBacks));
			
			_FTPWriteStreamCallBacks.version = 1;
			_FTPWriteStreamCallBacks.finalize = (void (*)(CFWriteStreamRef, void*))_FTPStreamFinalize;
			_FTPWriteStreamCallBacks.copyDescription = (CFStringRef (*)(CFWriteStreamRef, void*))_FTPStreamCopyDescription;
			_FTPWriteStreamCallBacks.open = (Boolean (*)(CFWriteStreamRef, CFStreamError*, Boolean*, void*))_FTPStreamOpen;
			_FTPWriteStreamCallBacks.openCompleted = (Boolean (*)(CFWriteStreamRef, CFStreamError*, void*))_FTPStreamOpenCompleted;
			_FTPWriteStreamCallBacks.write = (CFIndex (*)(CFWriteStreamRef, const UInt8*, CFIndex, CFStreamError*, void*))_FTPStreamWrite;
			_FTPWriteStreamCallBacks.canWrite = (Boolean (*)(CFWriteStreamRef, void*))_FTPStreamCanWrite;
			_FTPWriteStreamCallBacks.close = (void (*)(CFWriteStreamRef, void*))_FTPStreamClose;
			_FTPWriteStreamCallBacks.copyProperty = (CFTypeRef (*)(CFWriteStreamRef, CFStringRef, void*))_FTPStreamCopyProperty;
			_FTPWriteStreamCallBacks.setProperty = (Boolean (*)(CFWriteStreamRef, CFStringRef, CFTypeRef, void*))_FTPStreamSetProperty;
			_FTPWriteStreamCallBacks.schedule = (void (*)(CFWriteStreamRef, CFRunLoopRef, CFStringRef, void*))_FTPStreamSchedule;
			_FTPWriteStreamCallBacks.unschedule = (void (*)(CFWriteStreamRef, CFRunLoopRef, CFStringRef, void*))_FTPStreamUnschedule;
			
            result = CFWriteStreamCreate(alloc, &_FTPWriteStreamCallBacks, ctxt);
		}
    
        if (result) {

            ctxt->_userStream = result;		// Don't retain for fear of loop.
            
            temp = CFURLCopyUserName(ftpURL);
            if (temp) {
                CFWriteStreamSetProperty(result, kCFStreamPropertyFTPUserName, temp);
                CFRelease(temp);
            }
            
            temp = CFURLCopyPassword(ftpURL);
            if (temp) {
                CFWriteStreamSetProperty(result, kCFStreamPropertyFTPPassword, temp);
                CFRelease(temp);
            }
        }
        else {

            if (ctxt->_url)
                CFRelease(ctxt->_url);

            if (ctxt->_runloops)
                CFRelease(ctxt->_runloops);

            if (ctxt->_properties)
                CFRelease(ctxt->_properties);

            CFAllocatorDeallocate(alloc, ctxt);
        }
    }
    
    CFRelease(ftpURL);
    
    return result;
}


/* CF_EXPORT */ CFReadStreamRef
CFReadStreamCreateWithFTPURL(CFAllocatorRef alloc, CFURLRef ftpURL) {

    CFReadStreamRef result = NULL;
    CFStringRef temp;
    _CFFTPStreamContext* ctxt;

    if (!ftpURL || !(ftpURL = _ConvertToCFFTPHappyURL(ftpURL)))
        return result;
    
    temp = CFURLCopyScheme(ftpURL);
    if (!temp) {
        CFRelease(ftpURL);
        return result;
    }
    
    if ((CFStringCompare(temp, kFTPSchemeString, 0) != kCFCompareEqualTo) &&
        (CFStringCompare(temp, kFTPSSchemeString, 0) != kCFCompareEqualTo))
    {
        CFRelease(ftpURL);
        CFRelease(temp);
        return result;
    }
    
    CFRelease(temp);
    
    temp = CFURLCopyHostName(ftpURL);
    if (!temp) {
        CFRelease(ftpURL);
        return result;
    }
    
    CFRelease(temp);
    
    ctxt = (_CFFTPStreamContext*)CFAllocatorAllocate(alloc,
                                                     sizeof(ctxt[0]),
                                                     0);
    if (ctxt) {

        memset(ctxt, 0, sizeof(ctxt[0]));

        __CFBitSet(ctxt->_flags, kFlagBitPerformPASV);
        
        ctxt->_url = CFURLCopyAbsoluteURL(ftpURL);

        ctxt->_runloops = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
        ctxt->_properties = CFDictionaryCreateMutable(alloc,
                                                      0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);

        if (ctxt->_url && ctxt->_runloops && ctxt->_properties) {
			
			CFReadStreamCallBacks _FTPReadStreamCallBacks;

			memset(&_FTPReadStreamCallBacks, 0, sizeof(_FTPReadStreamCallBacks));
			
			_FTPReadStreamCallBacks.version = 1;
			_FTPReadStreamCallBacks.finalize = (void (*)(CFReadStreamRef, void*))_FTPStreamFinalize;
			_FTPReadStreamCallBacks.copyDescription = (CFStringRef (*)(CFReadStreamRef, void*))_FTPStreamCopyDescription;
			_FTPReadStreamCallBacks.open = (Boolean (*)(CFReadStreamRef, CFStreamError*, Boolean*, void*))_FTPStreamOpen;
			_FTPReadStreamCallBacks.openCompleted = (Boolean (*)(CFReadStreamRef, CFStreamError*, void*))_FTPStreamOpenCompleted;
			_FTPReadStreamCallBacks.read = (CFIndex (*)(CFReadStreamRef, UInt8*, CFIndex, CFStreamError*, Boolean*, void*))_FTPStreamRead;
			_FTPReadStreamCallBacks.canRead = (Boolean (*)(CFReadStreamRef, void*))_FTPStreamCanRead;
			_FTPReadStreamCallBacks.close = (void (*)(CFReadStreamRef, void*))_FTPStreamClose;
			_FTPReadStreamCallBacks.copyProperty = (CFTypeRef (*)(CFReadStreamRef, CFStringRef, void*))_FTPStreamCopyProperty;
			_FTPReadStreamCallBacks.setProperty = (Boolean (*)(CFReadStreamRef, CFStringRef, CFTypeRef, void*))_FTPStreamSetProperty;
			_FTPReadStreamCallBacks.schedule = (void (*)(CFReadStreamRef, CFRunLoopRef, CFStringRef, void*))_FTPStreamSchedule;
			_FTPReadStreamCallBacks.unschedule = (void (*)(CFReadStreamRef, CFRunLoopRef, CFStringRef, void*))_FTPStreamUnschedule;
			
            result = CFReadStreamCreate(alloc, &_FTPReadStreamCallBacks, ctxt);
		}
        
        if (result) {

            ctxt->_userStream = result;		// Don't retain for fear of loop.
            
            temp = CFURLCopyUserName(ftpURL);
            if (temp) {
                CFReadStreamSetProperty(result, kCFStreamPropertyFTPUserName, temp);
                CFRelease(temp);
            }
            
            temp = CFURLCopyPassword(ftpURL);
            if (temp) {
                CFReadStreamSetProperty(result, kCFStreamPropertyFTPPassword, temp);
                CFRelease(temp);
            }
        }
        else {
            
            if (ctxt->_url)
                CFRelease(ctxt->_url);

            if (ctxt->_runloops)
                CFRelease(ctxt->_runloops);
            
            if (ctxt->_properties)
                CFRelease(ctxt->_properties);

            CFAllocatorDeallocate(alloc, ctxt);
        }
    }

    CFRelease(ftpURL);
    
    return result;
}


/* CF_EXPORT */ CFIndex
CFFTPCreateParsedResourceListing(CFAllocatorRef alloc, const UInt8 *buffer, 
                                 CFIndex bufferLength, CFDictionaryRef *parsed)
{
	CFIndex totalConsumed;	// total characters consumed from buffer

    *parsed = NULL;
	totalConsumed = 0;

    // Bail if a null or empty buffer.
    if ( (buffer != NULL) && (bufferLength != 0) ) {
		const UInt8* scanStart;	// starting location to scan for line
		CFIndex scanLength;		// length to scan for line
		
		scanStart = buffer;
		scanLength = bufferLength;
		do {
			CFIndex consumed;	// number of characters consumed by _FindLine
			const UInt8* first;	// if not NULL, the beginning of the line to parse
			const UInt8* eol;	// if not NULL, the first EOL character after the line (more may have been consumed)
			
			/* find a line (if possible) and consume as many characters as possible */
			consumed = _FindLine(scanStart, scanLength, &first, &eol);
			totalConsumed += consumed;
			scanStart += consumed;
			scanLength -= consumed;
			
			if ( first == NULL ) {
				/* a line was not found so break */
				break;
			}
			
			// If it's not the summary line, parse it.
			if (memcmp("total ", first, 6)) {

				int count = 0;
				const UInt8* fields[16];

				// This is an example of the intended target listing:
				//    drwxrwxrwx  linkcount  user  group  size  month  day  yearOrTime  name

				memset(fields, 0, sizeof(fields));

				// Parse out each field.  If more than the number of fields
				// are parsed, assume they're all part of the name.
				while ((count < (sizeof(fields) / sizeof(fields[0]))) && (first < eol)) {

					// Skip leading space
					while ((first < eol) && isspace(*first))
						first++;

					// No more parsing if at the end of the line.
					if (first >= eol)
						break;

					// Save the location of the field.
					fields[count++] = first;

					// Skip over the field.
					while ((first < eol) && !isspace(*first))
						first++;
				}

				// If nothing parsed see if the next line does.
				if (count) {

					int type, mode;
					Boolean hadModeBits = TRUE;
					Boolean foundSize = FALSE;

					// Get the file type.
					switch (fields[0][0]) {
						case 'b': type = DT_BLK; break;		// Block special file.
						case 'c': type = DT_CHR; break;		// Character special file.
						case 'd': type = DT_DIR; break;		// Directory.
						case 'l': type = DT_LNK; break;		// Symbolic link.
						case 's': type = DT_SOCK; break;		// Socket link.
						case 'p': type = DT_FIFO; break;		// FIFO.
						case '-': type = DT_REG; break;		// Regular file.
						default: type = DT_UNKNOWN; break;
					}
		
					mode = 0;
		
					// Enough bytes to consider the mode field?
					if ((eol - fields[0]) < 11)
						hadModeBits = FALSE;
					
					else
						hadModeBits = _ReadModeBits(&fields[0][1], &mode);
					
					// Continue establishing the other information if room.  Start with date as the next anchor.
					if (fields[3] && fields[4]) {
						
						int i = 3;
						UInt64 size = 0;
						const UInt8* user = NULL;
						const UInt8* group = NULL;
						CFDateRef date = NULL;
						
						// Shoot to establish the next anchor, the date/time.
						while (fields[i]) {
							
							// Try to get the date/time.
							const UInt8* end = _CFFTPGetDateTimeFunc(alloc, fields[i], eol - fields[i], &date);
							
							// If built one, find out where it ended and the name begins.
							if (date) {
								
								int j = i - 1;
								
								// Walk backwards from the date to find the size.
								while (j >= 0) {
									
									// Allow "mode" field to be the size if no mode bits were there.
									if (!j && hadModeBits)
										break;
									
									// Try to convert to size.
									if (_ReadSize(fields[j], &size)) {
										
										foundSize = TRUE;
										
										j--;	// Assume the previous field to be group.
										
										// If it's not the first field or the mode bits
										// weren't in the first field, call it the group.
										if (j || !hadModeBits) {
											
											group = fields[j];
											
											j--;	// Assume the previous field to be user.
											
											// If there is another field and it's not the
											// mode bits, use it for the user field, otherwise
											// assume the user and group were a single field.
											if (!j && hadModeBits)
												user = group;
											
											else {
												
												UInt64 linkcount = 0;
												if (hadModeBits && (j == 1) && _ReadSize(fields[j], &linkcount))
													user = group;
												else
													user = fields[j];
											}
										}
										
										// Found size so break out of here.
										break;
									}
								}
								
								// Find out what the next field is.
								while (fields[i] && (end > fields[i]))
									i++;
								break;
							}
							
							// Try the next field as a date/time.
							i++;
						}
						
						// Found a date but is there a name field?
						if (fields[i] && date) {
							
							int j = 0;
							const UInt8* tmp = NULL;
							const UInt8* name = fields[i];
							const UInt8* link = NULL;
							const void *keys[kResourceInfoItemCount], *values[kResourceInfoItemCount];
							
							// If it's a link, find the link information.
							if (type == DT_LNK) {
								
								// Hunt for the "->" separator.
								while (fields[i]) {
									
									// If found, save the link.
									if (!memcmp(fields[i++], "->", 2)) {
										link = fields[i];
										break;
									}
								}
							}
							
							// If there were mode bits, save them in the values.
							if (hadModeBits) {
								keys[j] = kCFFTPResourceMode;
								values[j] = CFNumberCreate(alloc, kCFNumberSInt32Type, &mode);
								if (values[j]) j++;
							}
							
							// Save the name in the values.
							keys[j] = kCFFTPResourceName;						
							values[j] = CFStringCreateWithBytes(alloc, name, !link ? eol - name : (fields[i - 1] - 1) - name, kCFStringEncodingMacRoman, FALSE);
							if (values[j]) {
								CFStringRef temp = _CFStringCreateCopyWithStrippedHTML(alloc, values[j]);
								if (temp) {
									CFRelease(values[j]);
									values[j] = temp;
								}
								j++;
							}
							
							// If there was a link, save it.
							if (link) {
								keys[j]   = kCFFTPResourceLink;
								values[j] = CFStringCreateWithBytes(alloc, link, eol - link, kCFStringEncodingMacRoman, FALSE);
								if (values[j]) j++;
							}
							else {
								keys[j]   = kCFFTPResourceLink;
								values[j] = CFStringCreateWithCString(alloc, "", kCFStringEncodingUTF8);
								if (values[j]) j++;
							}
							
							// If the size was found, save the other bits.
							if (foundSize) {
								
								if (group && (group == user)) {
									
									const char kUserGroupSeparators[] = {'|', ':', '/', '\\'};
									const UInt8* sep = NULL;
									const UInt8* end = user;
									
									while (!isspace(*end))
										end++;
									
									for (i = 0; !sep && (i < (sizeof(kUserGroupSeparators) / sizeof(kUserGroupSeparators[0]))); i++)
										sep = memchr(user, kUserGroupSeparators[i], end - user);
									
									if (sep) {
										tmp = sep;		// Set tmp so length of user is properly calculated.
										group = sep + 1;
									}
									
									// NOTE that if no separator is found, user and group will get the
									// same value.
								}
								
								if (user) {
									
									if (!tmp) {
										for (tmp = user; !isspace(*tmp); tmp++)
											/* Do nothing. */ ;
									}
									
									// Save the owner.  There is only one if a size was found.
									keys[j]   = kCFFTPResourceOwner;
									values[j] = CFStringCreateWithBytes(alloc, user, tmp - user, kCFStringEncodingMacRoman, FALSE);
									if (values[j]) j++;
								}
								
								if (group) {
									
									for (tmp = group; !isspace(*tmp); tmp++)
										/* Do nothing. */ ;
									
									// Save the group.  There is only one if a size was found.
									keys[j]   = kCFFTPResourceGroup;
									values[j] = CFStringCreateWithBytes(alloc, group, tmp - group, kCFStringEncodingMacRoman, FALSE);
									if (values[j]) j++;
								}
								
								// Save the size.
								keys[j]   = kCFFTPResourceSize;
								values[j] = CFNumberCreate(alloc, kCFNumberLongLongType, &size);
								if (values[j]) j++;
							}
							
							// Save the file type.
							keys[j]   = kCFFTPResourceType;
							values[j] = CFNumberCreate(alloc, kCFNumberIntType, &type);
							if (values[j]) j++;
							
							// Save the date.
							keys[j]   = kCFFTPResourceModDate;
							values[j++] = CFRetain(date);		// Extra retain because it's released twice.
							
							// Create the dictionary of information for the user.
							*parsed = CFDictionaryCreate(alloc, keys, values, j, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
							
							// Release all the items that had been created.
							for (--j; j >= 0; j--)
								CFRelease(values[j]);
							
							// Did the parse, so bail.
							break;
						}
						
						// If date was allocated, release it.
						if (date)
							CFRelease(date);
					}
		
					// No mode bits, so deal with only a name.
					if (!hadModeBits) {
						const void *keys[1], *values[1];

						// Save the name in the values.
						keys[0] = kCFFTPResourceName;
						values[0] = CFStringCreateWithBytes(alloc, fields[0], eol - fields[0], kCFStringEncodingMacRoman, FALSE);
						

						// Create the dictionary of information for the user.
						if (values[0]) {
							
							if (!CFStringHasPrefix(values[0], kHTMLTagOpen) || !CFStringHasSuffix(values[0], kHTMLTagClose)) {
								
								*parsed = CFDictionaryCreate(alloc, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
								
								CFRelease(values[0]);
								
								// Did the parse, so bail.
								break;
							}
							
							CFRelease(values[0]);
						}
					}
				}
			}
			
			// Bail if at the end or beyond.
			if ( totalConsumed >= bufferLength ) {
				break;
			}
			
		} while (1);
	}
	
    // Return the number of bytes parsed.
    return ( totalConsumed );
}
