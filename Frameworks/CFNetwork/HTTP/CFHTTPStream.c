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
/* CFHTTPStream.c
   Copyright 1998-2003, Apple, Inc. All rights reserved.
   Responsibility: Becky Willrich
*/
#include <CFNetwork/CFHTTPStreamPriv.h>
#include <CFNetwork/CFHTTPMessagePriv.h>
#include "CFNetConnection.h"
#include "CFNetworkInternal.h"
#include "CFHTTPInternal.h"
#include <CFNetwork/CFHTTPConnectionPriv.h>
#include <CoreFoundation/CFStreamPriv.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <CFNetwork/CFSocketStreamPriv.h>
#if defined(__MACH__)
#include <SystemConfiguration/SCSchemaDefinitions.h> /* For the HTTP proxy keys */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

//#define LOG_REQUESTS 1
//#define NO_PIPELINING 1

#if defined(__WIN32__)
// Always sleep at least one millisec
#define usleep(usec)   Sleep(1.0 > rintf(((float)(usec))/1000.0)) ? 1.0 : rintf(((float)(usec))/1000.0)))

#include <winsock2.h>
#define ECONNRESET WSAECONNRESET

#endif

#define BUF_SIZE (2048)

#define HAVE_SENT_REQUEST_HEADERS (0)
#define HAVE_SENT_REQUEST_PAYLOAD (1)
#define HAVE_CHECKED_RESPONSE_HEADERS (2)
// Used to force end-of-stream when we know from the request/response that no data should come (like for a HEAD request)
#define FORCE_EOF (3)
#define AUTOREDIRECT (4)
#define PAYLOAD_IS_DATA (5)
#define OPEN_SIGNALLED (6)
#define IS_PERSISTENT (7)
#define MIN_STATE_BIT (8)
#define MAX_STATE_BIT (11)
#define HAS_PAYLOAD (12)
#define IS_ZOMBIE (14)
#define IN_READ_CALLBACK (16)
#define CUSTOM_STREAMS (17)
#define WAITING_FOR_PROXY_STREAM (18)
// Used when we wish to guarantee that haveBeenOrphaned not reattempt this transaction, usually because we are the current request/response and have just detected an error.  As the originator of the error, we should not reattempt.
#define DO_NOT_REATTEMPT (19)
/* A bit of a hack to cover the fact that we may become the current response well before the net connection signals 
    us with stateChanged to prepareReception.  The problem is that once we become the current response, we get the 
    response stream callbacks, which may include the mark (actually intended for the prior response) - we don't want
    that to cause us to send an endEncountered event.  Now, if we receive such an event and we have not yet read the mark,
    we simply do so and continue.  */
#define HAVE_READ_MARK (20)

typedef struct _CFHTTPRequest {
    CFOptionFlags flags;
    CFHTTPMessageRef originalRequest, currentRequest;
    CFHTTPMessageRef responseHeaders; // The response headers, so we can still get them once we're dequeued from the connection cuz the response is complete
    CFReadStreamRef requestPayload; // The request payload; may be NULL
    CFDataRef requestFragment; // Fragmentary data read from requestPayload but not yet written to the connection's request stream
    long long requestBytesWritten;
    CFReadStreamRef responseStream; // The stream we returned for this request
    
    // proxyDict is the dictionary with all the info about proxies in general.  proxyList is the list of proxies to be tried (in order) with the current URL.  proxyStream is the stream being used to load the proxy information; it's mutually exclusive with proxyList.  One day, they should be a union, but I don't want to deal with the complexity now.
    CFDictionaryRef proxyDict;
    CFMutableArrayRef proxyList;
    CFReadStreamRef proxyStream;
    
    CFMutableArrayRef redirectedURLs; // NULL unless automatic redirection is requested
    CFHTTPMessageRef firstRedirection; // Only non-NULL if a redirection occurs; private support for Foundation
    
    _CFNetConnectionRef conn;  // The connection we are scheduled on; consult OWNS_CONNECTION flag bit to determine whether we own the connection or the connection owns us
    CFRunLoopSourceRef stateChangeSource; // This source is used when we need to wait on an outside state change - either for bytes to come in on the connection, or for some request upstream of us to progress.
    CFMutableDictionaryRef connProps;
	CFArrayRef peerCertificates;
} _CFHTTPRequest;

struct _CFHTTPTestSOCKSContext {
    CFDictionaryRef dict;
    Boolean isGood;
};

static _CFNetConnectionCacheKey nextConnectionCacheKeyFromProxyArray(_CFHTTPRequest *http, CFMutableArrayRef proxyArray, CFURLRef targetURL, CFDictionaryRef connProperties);
static void advanceToNextProxyFromProxyArray(CFMutableArrayRef proxyArray);

// number of properties that can occur in a socks proxy dict
#define NUM_SOCKS_PROPS  7

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPStreamFTPScheme					CFSTR("ftp")
#define _kCFHTTPStreamFTPSScheme				CFSTR("ftps")
#define _kCFHTTPStreamHTTPScheme				CFSTR("http")
#define _kCFHTTPStreamHTTPSScheme				CFSTR("https")
#define _kCFHTTPStreamSOCKS4Scheme				CFSTR("socks4")
#define _kCFHTTPStreamSOCKS5Scheme				CFSTR("socks5")
#define _kCFHTTPStreamUserAgentHeader			CFSTR("User-Agent")
#define _kCFHTTPStreamProxyAuthorizationHeader	CFSTR("Proxy-Authorization")
#define _kCFHTTPStreamDescribeFormat			CFSTR("<HTTP request stream %p>{url = %@, state = %d, flags=%d}")
#define _kCFHTTPStreamContentLengthHeader		CFSTR("Content-Length")
#define _kCFHTTPStreamContentLengthFormat		CFSTR("%d")
#define _kCFHTTPStreamConnectionHeader			CFSTR("Connection")
#define _kCFHTTPStreamProxyConnectionHeader		CFSTR("Proxy-Connection")
#define _kCFHTTPStreamConnectionKeepAlive		CFSTR("keep-alive")
#define _kCFHTTPStreamConnectionClose			CFSTR("close")
#define _kCFHTTPStreamConnectionSeparator		CFSTR(",")
#define _kCFHTTPStreamHostHeader				CFSTR("Host")
#define _kCFHTTPStreamHostFormat				CFSTR("%@:%d")
#define _kCFHTTPStreamLocationHeader			CFSTR("Location")
#define _kCFHTTPStreamLocationSeparator			CFSTR(", ")
#define _kCFHTTPStreamHEADMethod				CFSTR("HEAD")
#define _kCFStreamSocketCreatedCallBack			CFSTR("_kCFStreamSocketCreatedCallBack")
#define _kCFHTTPStreamPrivateRunLoopMode		CFSTR("_kCFHTTPStreamPrivateRunLoopMode")
#define _kCFNTLMMethod							CFSTR("NTLM")
#else
static CONST_STRING_DECL(_kCFHTTPStreamFTPScheme, "ftp")
static CONST_STRING_DECL(_kCFHTTPStreamFTPSScheme, "ftps")
static CONST_STRING_DECL(_kCFHTTPStreamHTTPScheme, "http")
static CONST_STRING_DECL(_kCFHTTPStreamHTTPSScheme, "https")
static CONST_STRING_DECL(_kCFHTTPStreamSOCKS4Scheme, "socks4")
static CONST_STRING_DECL(_kCFHTTPStreamSOCKS5Scheme, "socks5")
static CONST_STRING_DECL(_kCFHTTPStreamUserAgentHeader, "User-Agent")
static CONST_STRING_DECL(_kCFHTTPStreamProxyAuthorizationHeader, "Proxy-Authorization")
static CONST_STRING_DECL(_kCFHTTPStreamDescribeFormat, "<HTTP request stream %p>{url = %@, state = %d, flags=%d}")
static CONST_STRING_DECL(_kCFHTTPStreamContentLengthHeader, "Content-Length")
static CONST_STRING_DECL(_kCFHTTPStreamContentLengthFormat, "%d")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionHeader, "Connection")
static CONST_STRING_DECL(_kCFHTTPStreamProxyConnectionHeader, "Proxy-Connection")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionKeepAlive, "keep-alive")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionClose, "close")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionSeparator, ",")
static CONST_STRING_DECL(_kCFHTTPStreamHostHeader, "Host")
static CONST_STRING_DECL(_kCFHTTPStreamHostFormat, "%@:%d")
static CONST_STRING_DECL(_kCFHTTPStreamLocationHeader, "Location")
static CONST_STRING_DECL(_kCFHTTPStreamLocationSeparator, ", ")
static CONST_STRING_DECL(_kCFHTTPStreamHEADMethod, "HEAD")
static CONST_STRING_DECL(_kCFStreamSocketCreatedCallBack, "_kCFStreamSocketCreatedCallBack")
static CONST_STRING_DECL(_kCFHTTPStreamPrivateRunLoopMode, "_kCFHTTPStreamPrivateRunLoopMode")
static CONST_STRING_DECL(_kCFNTLMMethod, "NTLM")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Connection cache management; the cache is created and accessed in getConnectionForRequest

static CFSpinLock_t cacheInitLock = 0;
static CFNetConnectionCacheRef httpConnectionCache = NULL;

static void *httpRequestCreate(CFReadStreamRef stream, void *info);
static void httpRequestFinalize(CFReadStreamRef stream, void *info);
static CFStringRef httpRequestDescription(CFReadStreamRef stream, void *info);
static Boolean httpRequestOpen(CFReadStreamRef stream, CFStreamError *error, Boolean *openComplete, void *info);
static Boolean httpRequestOpenCompleted(CFReadStreamRef stream, CFStreamError *error, void *info);
static CFIndex httpRequestRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info);
static Boolean httpRequestCanRead(CFReadStreamRef stream, void *info);
static void httpRequestClose(CFReadStreamRef stream, void *info);
static CFTypeRef httpRequestCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info);
static Boolean httpRequestSetProperty(CFReadStreamRef stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info);
static void httpRequestSchedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
static void httpRequestUnschedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);

static const CFReadStreamCallBacks _CFHTTPQueuedResponseStreamCallBacks = {
    1,
    httpRequestCreate,
    httpRequestFinalize,
    httpRequestDescription,
    httpRequestOpen,
    httpRequestOpenCompleted,
    httpRequestRead,
    NULL, /*getBuffer*/
    httpRequestCanRead,
    httpRequestClose,
    httpRequestCopyProperty,
    httpRequestSetProperty,
    NULL, // requestEvents
    httpRequestSchedule,
    httpRequestUnschedule
};

static CFStreamError httpCreateConnectionStreams(CFAllocatorRef allocator, const void *info, CFWriteStreamRef *requestStreams, CFReadStreamRef *responseStreams);
static void httpRequestStateChanged(void *request, int newState, CFStreamError *err, _CFNetConnectionRef connection, const void*);
static void httpTransmitRequest(void *request, _CFNetConnectionRef connection, const void*);
static void httpReceiveResponse(void *request, _CFNetConnectionRef connection, const void*);
static void httpResponseStreamCallBack(void *request, CFReadStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, const void*);
static void httpRequestStreamCallBack(void *request, CFWriteStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, const void*);
static CFArrayRef httpRunLoopArrayForRequest(void *request, _CFNetConnectionRef conn, const void* info);

static const _CFNetConnectionCallBacks httpConnectionCallBacks = {
    0,
    connCacheKeyRetain,
    connCacheKeyRelease,
    httpCreateConnectionStreams,
    httpRequestStateChanged,
    httpTransmitRequest,
    httpReceiveResponse,
    httpResponseStreamCallBack,
    httpRequestStreamCallBack,
    httpRunLoopArrayForRequest
};

static void requestPayloadCallBack(CFReadStreamRef stream, CFStreamEventType type, void *info);
static Boolean resetForRequest(CFHTTPMessageRef newRequest, _CFHTTPRequest *http, CFStreamError *error);
static Boolean checkHeaders(_CFHTTPRequest *http, CFReadStreamRef responseStream, CFStreamError *error, Boolean *connectionStaysPersistent);
static void httpRequestDestroy(CFAllocatorRef alloc, _CFHTTPRequest *req);
static void addAuthenticationInfoToResponse1(_CFHTTPRequest *http);
static CFHTTPAuthenticationRef connectionOrientedAuth(_CFHTTPRequest *req, Boolean forProxy);

extern void _CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
									  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream);

static void buildDirectDescription(CFHTTPMessageRef request, CFStringRef *host, SInt32 *port, UInt32 *type, CFDictionaryRef *streamProperties) {
    CFURLRef targetURL = CFHTTPMessageCopyRequestURL(request);
    CFStringRef scheme = CFURLCopyScheme(targetURL);
    if (CFEqual(scheme, _kCFHTTPStreamHTTPSScheme)) {
        *type = kHTTPS;
    } else {
        *type = kHTTP;
    }
    CFRelease(scheme);
    *host = CFURLCopyHostName(targetURL);
    *port = CFURLGetPortNumber(targetURL);
    if (*port == -1) {
        *port = (*type == kHTTP) ? 80 : 443;
    }
    *streamProperties = NULL;
    CFRelease(targetURL);
}


static CFDictionaryRef newConnPropsForSOCKSProxy(CFAllocatorRef alloc, CFURLRef proxyURL) {
    CFStringRef scheme;
    SInt32 port;
    CFStringRef user;
    
    CFStringRef keys[5];
    CFTypeRef values[5];
    CFDictionaryRef socksProxyDict;
    CFDictionaryRef newConnProps;

    keys[0] = kCFStreamPropertySOCKSProxyHost;
    values[0] = CFURLCopyHostName(proxyURL);

    keys[1] = kCFStreamPropertySOCKSProxyPort;
    port = CFURLGetPortNumber(proxyURL);
    values[1] = CFNumberCreate(alloc, kCFNumberSInt32Type, &port);
    
    keys[2] = kCFStreamPropertySOCKSVersion;
    scheme = CFURLCopyScheme(proxyURL);
    if (CFStringCompare(scheme, _kCFHTTPStreamSOCKS4Scheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        values[2] = kCFStreamSocketSOCKSVersion4;
    } else {
        values[2] = kCFStreamSocketSOCKSVersion5;
    }
    CFRelease(scheme);

    user = CFURLCopyUserName(proxyURL);
    if (user) {
        keys[3] = kCFStreamPropertySOCKSUser;
        values[3] = user;
        keys[4] = kCFStreamPropertySOCKSPassword;
        values[4] = CFURLCopyPassword(proxyURL);
    }
    socksProxyDict = CFDictionaryCreate(alloc, (const void **)keys, values, user ? 5 : 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);
    if (user) {
        CFRelease(user);
        CFRelease(values[4]);
    }

    keys[0] = kCFStreamPropertySOCKSProxy;
    values[0] = socksProxyDict;
    newConnProps = CFDictionaryCreate(alloc, (const void **)keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(socksProxyDict);
    return newConnProps;
}

static CFDictionaryRef newConnPropsForHTTPSProxy(CFAllocatorRef alloc, CFHTTPMessageRef req, CFURLRef proxyURL) {

    SInt32 port;
    
    CFStringRef keys[3];
    CFTypeRef values[3];
    CFDictionaryRef headers;
    CFDictionaryRef proxyDict;
    CFDictionaryRef newConnProps;

    keys[0] = _kCFHTTPStreamUserAgentHeader;
    values[0] = CFHTTPMessageCopyHeaderFieldValue(req, _kCFHTTPStreamUserAgentHeader);
    if (!values[0])
        values[0] = CFRetain( _CFNetworkUserAgentString() );

    keys[1] = _kCFHTTPStreamProxyAuthorizationHeader;
    values[1] = CFHTTPMessageCopyHeaderFieldValue(req, _kCFHTTPStreamProxyAuthorizationHeader);

    headers = CFDictionaryCreate(alloc, (const void **)keys, values, values[1] ? 2 : 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    if (values[1])
        CFRelease(values[1]);

    keys[0] = kCFStreamPropertyCONNECTProxyHost;
    values[0] = CFURLCopyHostName(proxyURL);

    keys[1] = kCFStreamPropertyCONNECTProxyPort;
    port = CFURLGetPortNumber(proxyURL);
    values[1] = CFNumberCreate(alloc, kCFNumberSInt32Type, &port);

    keys[2] = kCFStreamPropertyCONNECTAdditionalHeaders;
    values[2] = headers;

    proxyDict = CFDictionaryCreate(alloc, (const void **)keys, values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);
    CFRelease(values[2]);

    keys[0] = kCFStreamPropertyCONNECTProxy;
    values[0] = proxyDict;
    newConnProps = CFDictionaryCreate(alloc, (const void **)keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(proxyDict);
    return newConnProps;
}

CF_EXPORT void _CFHTTPGetConnectionInfoForProxyURL(CFURLRef proxyURL, CFHTTPMessageRef request, CFStringRef *host, SInt32 *port, UInt32 *type, CFDictionaryRef *streamProperties) {
    if ((CFTypeRef)proxyURL == kCFNull) {
        buildDirectDescription(request, host, port, type, streamProperties);
    } else {
        CFStringRef scheme = CFURLCopyScheme(proxyURL);
        if (CFStringCompare(scheme, _kCFHTTPStreamHTTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            *type = kHTTPProxy;
            *host = CFURLCopyHostName(proxyURL);
            *port = CFURLGetPortNumber(proxyURL);
            *streamProperties = NULL;
        } else if (CFStringCompare(scheme, _kCFHTTPStreamHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            CFURLRef targetURL = CFHTTPMessageCopyRequestURL(request);
            *type = kHTTPSProxy;
            *host = CFURLCopyHostName(targetURL);
            *port = CFURLGetPortNumber(targetURL);
            if (*port == -1) {
                CFStringRef scheme = CFURLCopyScheme(targetURL);
                if (scheme && (CFStringCompare(scheme, _kCFHTTPStreamHTTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
                    *port = 80;
                else
                    *port = 443;
                if (scheme) CFRelease(scheme);
            }
            *streamProperties = newConnPropsForHTTPSProxy(CFGetAllocator(request), request, proxyURL);
            CFRelease(targetURL);
        } else if ((CFStringCompare(scheme, _kCFHTTPStreamSOCKS4Scheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) || (CFStringCompare(scheme, _kCFHTTPStreamSOCKS5Scheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
            CFURLRef targetURL = CFHTTPMessageCopyRequestURL(request);
            CFStringRef targetScheme = CFURLCopyScheme(targetURL);
            
            if (CFStringCompare(targetScheme, _kCFHTTPStreamHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                *type = kHTTPS;
            } else {
                *type = kHTTP;
            }
            CFRelease(targetScheme);
            
            *host = CFURLCopyHostName(targetURL);

            *port = CFURLGetPortNumber(targetURL);
            if (*port == -1) {
                *port = (*type == kHTTP) ? 80 : 443;
            }
            *streamProperties = newConnPropsForSOCKSProxy(CFGetAllocator(request), proxyURL);
        }
        CFRelease(scheme);
    }
}

// This is currently only set up for HTTP/HTTPS queries.
static _CFNetConnectionCacheKey nextConnectionCacheKeyFromProxyArray(_CFHTTPRequest *http, CFMutableArrayRef proxyArray, CFURLRef targetURL, CFDictionaryRef connProperties) {
    _CFNetConnectionCacheKey key = NULL;
    CFStringRef host;
    SInt32 port;
    UInt32 type;
    CFDictionaryRef additionalProperties, props;
    CFMutableDictionaryRef newProps = NULL;

    CFURLRef proxyURL = CFArrayGetValueAtIndex(proxyArray, 0);

    _CFHTTPGetConnectionInfoForProxyURL(proxyURL, http->currentRequest ? http->currentRequest : http->originalRequest, &host, &port, &type, &additionalProperties);

    if (additionalProperties) {
        if (!http->connProps) {
            props = additionalProperties;
        } else {
            CFAllocatorRef alloc = CFGetAllocator(http->originalRequest);
            CFIndex index, count = CFDictionaryGetCount(additionalProperties);
            CFTypeRef *keys = CFAllocatorAllocate(alloc, count * 2 * sizeof(CFTypeRef), 0);
            CFTypeRef *values = keys + count;
            CFDictionaryGetKeysAndValues(additionalProperties, keys, values);
            newProps = CFDictionaryCreateMutableCopy(alloc, CFDictionaryGetCount(http->connProps) + count, http->connProps);
            for (index = 0; index < count; index ++) {
                CFDictionarySetValue(newProps, keys[index], values[index]);
            }
            CFAllocatorDeallocate(alloc, keys);
            props = newProps;
        }
    } else if (http->connProps) {
        props = http->connProps;
    } else {
        props = NULL;
    }
    
    key = createConnectionCacheKey(host, port, type, props);
    if (host) CFRelease(host);
    if (additionalProperties) CFRelease(additionalProperties);
    if (newProps) CFRelease(newProps);
    return key;
}

static void advanceToNextProxyFromProxyArray(CFMutableArrayRef proxyArray) {
    if (CFArrayGetCount(proxyArray) > 0) {
        CFArrayRemoveValueAtIndex(proxyArray, 0);
    }
}


static inline void _CFHTTPRequestSetState(_CFHTTPRequest *req, int newState) {
    __CFBitfieldSetValue(req->flags, MAX_STATE_BIT, MIN_STATE_BIT, newState);
}

static inline int _CFHTTPRequestGetState(_CFHTTPRequest *req) {
    return __CFBitfieldGetValue(req->flags, MAX_STATE_BIT, MIN_STATE_BIT);
}

#if defined(LOG_REQUESTS)
// For debugging
CF_EXPORT int CFHTTPRequestGetState(_CFHTTPRequest *req);
CF_EXPORT
int CFHTTPRequestGetState(_CFHTTPRequest *req) {
    return _CFHTTPRequestGetState(req);
}
#endif

static inline Boolean isPersistent(_CFHTTPRequest *http) {
    return __CFBitIsSet(http->flags, IS_PERSISTENT);
}

static inline Boolean haveCheckedHeaders(_CFHTTPRequest *req) {
    return __CFBitIsSet(req->flags, HAVE_CHECKED_RESPONSE_HEADERS);
}

static inline Boolean requestHasBeenTransmitted(_CFHTTPRequest *http) {
    return __CFBitIsSet(http->flags, HAVE_SENT_REQUEST_PAYLOAD);
}

// Utility to pull the SOCKS from a dictionary.  Returns the number keys found.
static int extractSocksProperties(CFDictionaryRef dict, CFTypeRef *outKeys, CFTypeRef *outValues) {

	const CFStringRef keys[] = {
        kCFStreamPropertySOCKSProxyHost,
		kCFStreamPropertySOCKSProxyPort,
        kCFStreamPropertySOCKSVersion,
        kCFStreamPropertySOCKSUser,
        kCFStreamPropertySOCKSPassword,
        kCFStreamPropertyProxyExceptionsList,
#if defined(__MACH__)
        kSCPropNetProxiesSOCKSEnable
#endif
    };
	
    int i, numFound = 0;
	
	for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
		
        CFTypeRef value = CFDictionaryGetValue(dict, keys[i]);
		
		if (value) {
			outKeys[numFound] = keys[i];
			outValues[numFound] = value;
			numFound++;
		}
	}
	
    return numFound;
}

static void *httpRequestCreate(CFReadStreamRef stream, void *info) {
    _CFHTTPRequest *newReq, *oldReq = (_CFHTTPRequest *)info;
    CFAllocatorRef alloc = CFGetAllocator(stream);
    newReq = CFAllocatorAllocate(alloc, sizeof(_CFHTTPRequest), 0);
    if (!newReq) return NULL;
    newReq->flags = 0;
    newReq->connProps = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    //CFDictionarySetValue(newReq->connProps, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, kCFBooleanTrue);
    _CFHTTPRequestSetState(newReq, kNotQueued);
    CFRetain(oldReq->originalRequest);
    newReq->originalRequest = oldReq->originalRequest;
    newReq->currentRequest = NULL;
    newReq->responseHeaders = NULL;
    if (oldReq->requestPayload) {
        __CFBitSet(newReq->flags, HAS_PAYLOAD);
        CFRetain(oldReq->requestPayload);
        newReq->requestPayload = oldReq->requestPayload;
    } else {
        CFDataRef body = CFHTTPMessageCopyBody(newReq->originalRequest);
        if (body) {
            __CFBitSet(newReq->flags, HAS_PAYLOAD);
            __CFBitSet(newReq->flags, PAYLOAD_IS_DATA);
            CFRelease(body);
        }
        newReq->requestPayload = NULL;
    }
	newReq->peerCertificates = NULL;
    newReq->requestFragment = NULL;
    newReq->requestBytesWritten = 0;
    newReq->responseStream = stream; // Do not retain.
    newReq->proxyDict = NULL;
    newReq->proxyList = NULL;
    newReq->proxyStream = NULL;
    newReq->redirectedURLs = NULL;
    newReq->firstRedirection = NULL;
    newReq->conn = NULL;
    newReq->stateChangeSource = NULL;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "Created request 0x%x\n", (int)newReq);
#endif
    return newReq;
}

static
_CFHTTPRequest *createZombieDouble1(CFAllocatorRef alloc, _CFHTTPRequest *orig, _CFNetConnectionRef conn) {
    _CFHTTPRequest *zombie;
    CFArrayRef origRLArray;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "substituteZombieDouble(0x%x, 0x%x, 0x%x) -", (int)alloc, (int)orig, (int)(conn));
#endif
    zombie = CFAllocatorAllocate(alloc, sizeof(_CFHTTPRequest), 0);
    if (!zombie) return NULL;
    zombie->flags = orig->flags;
    __CFBitSet(zombie->flags, IS_ZOMBIE);
    __CFBitClear(zombie->flags, AUTOREDIRECT);
    zombie->conn = conn;
    CFRetain(conn);
    zombie->connProps = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks); // Our code relies on connProps never being NULL
    zombie->firstRedirection = NULL;
    zombie->redirectedURLs = NULL;
    zombie->proxyDict = NULL;
    zombie->proxyList = NULL;
    zombie->proxyStream = NULL;
    zombie->responseHeaders = NULL;
    zombie->requestBytesWritten = orig->requestBytesWritten;
    zombie->stateChangeSource = NULL;
	zombie->peerCertificates = NULL;
    // Sadly, the zombie needs the original request in case there was auth on it; we may need to advance the state of the auth token when our response comes in.
    zombie->originalRequest = orig->originalRequest;
    CFRetain(zombie->originalRequest);
    zombie->currentRequest = orig->currentRequest;
    CFRetain(zombie->currentRequest);
    
    // For both of these, we want to transfer ownership to the zombie.  The original will have to deal without.
    zombie->requestFragment = orig->requestFragment;
    if (zombie->requestFragment) orig->requestFragment = NULL;
    if (orig->requestPayload) {
        CFStreamClientContext ctxt = {0, zombie, NULL, NULL, NULL};
        zombie->requestPayload = orig->requestPayload;
        orig->requestPayload = NULL;
        CFReadStreamSetClient(zombie->requestPayload, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, requestPayloadCallBack, &ctxt);
    } else {
        zombie->requestPayload = NULL;
    }
    
    // This is kinda ugly, but the zombie needs to know where it should schedule/unschedule, and the usual
    // way to do that is to look at its response stream.  So, we create a dummy response stream and schedule
    // it wherever orig->responseStream is scheduled.  Since we never open the stream, life should be good....
    zombie->responseStream = CFReadStreamCreateWithBytesNoCopy(alloc, (const UInt8*)"dummy zombie stream", strlen("dummy zombie stream"), kCFAllocatorNull);
    origRLArray = _CFReadStreamGetRunLoopsAndModes(orig->responseStream);
    if (origRLArray) {
        CFIndex i, c = CFArrayGetCount(origRLArray);
        for (i = 0; i + 1 < c; i += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(origRLArray, i);
            CFStringRef mode = CFArrayGetValueAtIndex(origRLArray, i + 1);
            CFReadStreamScheduleWithRunLoop(zombie->responseStream, rl, mode);
        }
    }
#if defined(LOG_REQUESTS)
    fprintf(stderr, " returned zombie 0x%x\n", (int)zombie);
#endif
    return zombie;
}

extern Boolean _CFHTTPAuthenticationConnectionAuthenticated(CFHTTPAuthenticationRef auth, const void* connection);

static void dequeueFromConnection1(_CFHTTPRequest *req) {
    // Guard against re-entrancy; CFHTTPConnectionDequeue may end up re-entering us and we don't want to to attempt multiple dequeues from the same connection.  Hence the shuffle below with req->conn and conn.
#if defined(LOG_REQUESTS)
    fprintf(stderr, " dequeueFromConnection(0x%x)\n", (int)req);
#endif
    if (req->conn) {
        _CFNetConnectionRef conn = req->conn;
        req->conn = NULL;
        if (!__CFBitIsSet(req->flags, IS_PERSISTENT)) {
            // We own this connection, just destroy it outright
            _CFNetConnectionSetAllowsNewRequests(conn, FALSE);
            CFRelease(conn);
        } else {
            if (!_CFNetConnectionDequeue(conn, req)) {
                _CFHTTPRequest *zombie = createZombieDouble1(CFGetAllocator(conn), req, conn);
                if (!zombie) {
                    // We're doomed....  We can't dequeue, and we can't replace ourselves....
                    req->conn = conn;
                    __CFBitSet(req->flags, IS_ZOMBIE);
                    return;
                } else {
                    _CFNetConnectionReplaceRequest(conn, req, zombie);
                }
            } else {
				
				int i, bad = 0;
				CFHTTPAuthenticationRef auth[2];
				Boolean isPersistent = _CFNetConnectionWillEnqueueRequests(conn);
				Boolean empty = _CFNetConnectionIsEmpty(conn);
				Boolean hasAuth = FALSE;
				Boolean authComplete = TRUE;
				
				auth[0] = connectionOrientedAuth(req, FALSE);
				auth[1] = connectionOrientedAuth(req, TRUE);
					
				for (i = 0; i < (sizeof(auth) / sizeof(auth[0])); i++) {
					
					if (!auth[i])
						continue;
					
					hasAuth = TRUE;
					
					if (!isPersistent || !CFHTTPAuthenticationIsValid(auth[i], NULL)) {
						bad++;
						_CFHTTPAuthenticationDisassociateConnection(auth[i], conn);
					}
					else
						authComplete = authComplete && _CFHTTPAuthenticationConnectionAuthenticated(auth[i], conn);
				}
				
				/*
				** There are four reasons for which to pull the connection from the cache:
				**
				**	1. If the connection isn't persistent, make sure it's not in the cache.
				**
				**	2. If there is not connection-based authentication and the queue for the
				**		connection is empty, remove the connection from the cache.
				**
				**	3. If there is authentication and one has gone bad, remove the connection
				**		from the cache.  This could actually be made slightly better if it
				**		paid attention to the proxy authentication going bad versus the server
				**		authentication going bad.  Something to add in the future.
				**
				**	4. If there is authentication and it has gone to completion, treat the
				**		connection like there is no authentication.  This means that as soon
				**		as the connection has gone empty, it can be removed.
				*/
				if (!isPersistent ||
					(!hasAuth && empty) ||
					(hasAuth && bad) ||
					(hasAuth && authComplete && empty))
				{
					_CFNetConnectionSetAllowsNewRequests(conn, FALSE);
					removeFromConnectionCache(httpConnectionCache, conn, (_CFNetConnectionCacheKey)_CFNetConnectionGetInfoPointer(conn));
				}
            }
            CFRelease(conn);
        }
    }
}

static void httpRequestDestroy(CFAllocatorRef alloc, _CFHTTPRequest *req) {
    if (req->conn) dequeueFromConnection1(req);
    CFRelease(req->originalRequest);
    if (req->currentRequest) CFRelease(req->currentRequest);
    if (req->responseHeaders) CFRelease(req->responseHeaders);
    if (req->requestPayload) {
        CFReadStreamClose(req->requestPayload);
        CFReadStreamSetClient(req->requestPayload, 0, NULL, NULL);
        CFRelease(req->requestPayload);
    }
    // Do NOT release req->responseStream unless we are a zombie; we don't have a reference
    if (__CFBitIsSet(req->flags, IS_ZOMBIE) && req->responseStream) CFRelease(req->responseStream);
    if (req->requestFragment) CFRelease(req->requestFragment);
    if (req->proxyDict) CFRelease(req->proxyDict);
    if (req->proxyList) CFRelease(req->proxyList);
    if (req->proxyStream) CFRelease(req->proxyStream);
    if (req->redirectedURLs) CFRelease(req->redirectedURLs);
    if (req->firstRedirection) CFRelease(req->firstRedirection);
    if (req->connProps) CFRelease(req->connProps);
    if (req->stateChangeSource) CFRelease(req->stateChangeSource);
	if (req->peerCertificates) CFRelease(req->peerCertificates);
    
    CFAllocatorDeallocate(alloc, req); 
}

static void httpRequestFinalize(CFReadStreamRef stream, void *info) {
#if defined(LOG_REQUESTS)
    fprintf(stderr, " httpRequestFinalize(0x%x, 0x%x)\n", (int)stream, (int)info);
#endif
    httpRequestDestroy(CFGetAllocator(stream), (_CFHTTPRequest *)info);
}

static CFStringRef httpRequestDescription(CFReadStreamRef stream, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
    CFURLRef url = CFHTTPMessageCopyRequestURL(req->originalRequest);
    CFStringRef str = CFStringCreateWithFormat(stream ? CFGetAllocator(stream) : NULL, NULL, _kCFHTTPStreamDescribeFormat, req, url, _CFHTTPRequestGetState(req), req->flags);
    CFRelease(url);
    return str;
}

// Need to clean up requestPayload, requestFragment.
static void closeRequestResources1(_CFHTTPRequest *req) {
    if (req->requestPayload) {
        CFReadStreamClose(req->requestPayload);
        CFRelease(req->requestPayload);
        req->requestPayload = NULL;
    }
    if (req->requestFragment) {
        CFRelease(req->requestFragment);
        req->requestFragment = NULL;
    }
}

extern CFStringRef _CFNetworkUserAgentString(void) {
    static CFStringRef userAgentString = NULL;
    if (!userAgentString) {
        CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork"));
        if (bundle) {
            CFMutableStringRef mutableString = CFStringCreateMutable(NULL, 0);
            CFStringAppendCString(mutableString, "CFNetwork/", kCFStringEncodingASCII);
            CFStringAppend(mutableString, CFBundleGetValueForInfoDictionaryKey(bundle, _kCFBundleShortVersionStringKey));
            userAgentString = CFStringCreateCopy(NULL, mutableString);
            CFRelease(mutableString);
        } else {
            userAgentString = CFSTR("CFNetwork (unknown version)");
        }
    }
    return userAgentString;
}

extern void cleanUpRequest(CFHTTPMessageRef req, int length, Boolean forPersistentConnection, Boolean forProxy) {
    // Perform basic house-keeping on the request: make sure a valid user-agent is set, make sure the Host: parameter is set, and make sure the Content-Length is set if there is data.
    CFURLRef dest;
    CFStringRef host;
    CFStringRef val;
    val = CFHTTPMessageCopyHeaderFieldValue(req, _kCFHTTPStreamUserAgentHeader);
    if (val == NULL) {
        // Some servers require that the User-Agent be listed first.
        _CFHTTPMessageSetHeader(req, _kCFHTTPStreamUserAgentHeader, _CFNetworkUserAgentString(), 0);
    } else {
        CFRelease(val);
    }
    
    if (length > -1 && !_CFHTTPMessageIsGetMethod(req)) {
        CFStringRef lenStr = CFStringCreateWithFormat(CFGetAllocator(req), NULL, _kCFHTTPStreamContentLengthFormat, length);
        CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamContentLengthHeader, lenStr);
        CFRelease(lenStr);
    }
    
    if (forPersistentConnection) {
        CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamConnectionHeader, _kCFHTTPStreamConnectionKeepAlive);
		if (forProxy) {
			CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamProxyConnectionHeader, _kCFHTTPStreamConnectionKeepAlive);
        }
    } else {
        CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamConnectionHeader, _kCFHTTPStreamConnectionClose);
		if (forProxy) {
			CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamProxyConnectionHeader, _kCFHTTPStreamConnectionClose);
        }
    }

    dest = CFHTTPMessageCopyRequestURL(req);
    host = dest ? CFURLCopyHostName(dest) : NULL;
    if (host) {
        CFStringRef scheme = CFURLCopyScheme(dest);
        SInt32 port = CFURLGetPortNumber(dest);
        if (port == -1) {
            CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamHostHeader, host);
        } else {
            CFStringRef hostStr = CFStringCreateWithFormat(CFGetAllocator(req), NULL, _kCFHTTPStreamHostFormat, host, port);
            if (hostStr) {
                CFHTTPMessageSetHeaderFieldValue(req, _kCFHTTPStreamHostHeader, hostStr);
                CFRelease(hostStr);
            }
        }
        CFRelease(host);
        if (scheme) CFRelease(scheme);
    }
    if (dest) CFRelease(dest);
}

static inline Boolean isConnectionToProxy(_CFNetConnectionRef conn) {
    _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)_CFNetConnectionGetInfoPointer(conn);
    CFStringRef host;
    SInt32 port;
    UInt32 type;
    CFDictionaryRef props;
    
    getValuesFromKey(key, &host, &port, &type, &props);
    return (type == kHTTPProxy) || (type == kHTTPSProxy);
    
}

static void prepareTransmission1(_CFHTTPRequest *req, CFWriteStreamRef requestStream, _CFNetConnectionRef conn) {
    // req->responseStream should never be NULL at this point; that can only happen if req is a zombie, and zombies are only created for requests whose transmission has already begun
    Boolean reqIsPersistent = isPersistent(req);
    CFStreamClientContext ctxt = {0, req, NULL, NULL, NULL};
    CFDataRef payload = NULL;
    Boolean forProxy = isConnectionToProxy(conn);
    
    
    // Set requestPayload properly; clean up the request
    if (__CFBitIsSet(req->flags, PAYLOAD_IS_DATA) && (payload = CFHTTPMessageCopyBody(req->originalRequest)) != NULL) {
        CFIndex length = CFDataGetLength(payload);
        if (req->requestPayload) {
            CFReadStreamSetClient(req->requestPayload, kCFStreamEventNone, NULL, NULL);
            CFReadStreamClose(req->requestPayload);
            CFRelease(req->requestPayload);
        }
        
        if (length) {
            req->requestPayload = CFReadStreamCreateWithBytesNoCopy(CFGetAllocator(payload), CFDataGetBytePtr(payload), length, kCFAllocatorNull);
        }
        else
            req->requestPayload = NULL;
            
        CFRelease(payload); // originalRequest is holding it for us
        cleanUpRequest(req->currentRequest, length, reqIsPersistent, forProxy);
    } else if (!req->requestPayload) {
        cleanUpRequest(req->currentRequest, 0, reqIsPersistent, forProxy);
    } else {
        cleanUpRequest(req->currentRequest, -1, reqIsPersistent, forProxy);
    }
    
    // Set client on both streams and schedule.  Open payload (requestStream is already open)
    if (req->requestPayload) {
        CFArrayRef rlArray;
        CFReadStreamSetClient(req->requestPayload, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, requestPayloadCallBack, &ctxt);
        rlArray = _CFReadStreamGetRunLoopsAndModes(req->responseStream);
        if (rlArray) {
            int i, c = CFArrayGetCount(rlArray);
            for (i = 0; i + 1 < c; i += 2) {
                CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(rlArray, i);
                CFStringRef mode = CFArrayGetValueAtIndex(rlArray, i + 1);
                if (req->requestPayload) {
                    CFReadStreamScheduleWithRunLoop(req->requestPayload, rl, mode);
                }
            }
        }
        CFReadStreamOpen(req->requestPayload);
    }
    CFWriteStreamSetProperty(requestStream, _kCFStreamPropertyHTTPNewHeader, req->currentRequest);
    if (!__CFBitIsSet(req->flags, OPEN_SIGNALLED)) {
        CFReadStreamSignalEvent(req->responseStream, kCFStreamEventOpenCompleted, NULL);
        __CFBitSet(req->flags, OPEN_SIGNALLED);
    }
    if (CFWriteStreamCanAcceptBytes(requestStream)) {
        _CFWriteStreamSignalEventDelayed(requestStream, kCFStreamEventCanAcceptBytes, NULL);
    }
}

static void concludeTransmission1(_CFHTTPRequest *req, CFWriteStreamRef requestStream) {
    closeRequestResources1(req);
    _CFHTTPWriteStreamWriteMark(requestStream);
}

static void prepareReception1(_CFHTTPRequest *req, CFReadStreamRef responseStream) {
    CFStringRef cmd = NULL;
    if (__CFBitIsSet(req->flags, HAVE_READ_MARK)) return;
    __CFBitSet(req->flags, HAVE_READ_MARK);
    _CFHTTPReadStreamReadMark(responseStream);
    if (req->originalRequest && (cmd = CFHTTPMessageCopyRequestMethod(req->originalRequest)) && CFEqual(cmd, _kCFHTTPStreamHEADMethod)) {
        CFReadStreamSetProperty(responseStream, _kCFStreamPropertyHTTPZeroLengthResponseExpected, kCFBooleanTrue);
    }
    if (cmd) CFRelease(cmd);
    if (CFReadStreamHasBytesAvailable(responseStream)) {
        _CFReadStreamSignalEventDelayed(responseStream, kCFStreamEventHasBytesAvailable, NULL);
    } else if (_CFHTTPReadStreamIsAtMark(responseStream)) {
        _CFReadStreamSignalEventDelayed(responseStream, kCFStreamEventMarkEncountered, NULL);
    }
}

static void concludeReception1(_CFHTTPRequest *req, CFReadStreamRef responseStream, Boolean *haveBeenDealloced) {
    Boolean readFromThisStream = TRUE;
    CFStreamError err = {0, 0};

    if (__CFBitIsSet(req->flags, IS_ZOMBIE)) {
        CFAllocatorRef alloc = CFGetAllocator(req->conn);
        dequeueFromConnection1(req); 
        httpRequestDestroy(alloc, req);
        *haveBeenDealloced = TRUE;
    } else {
        if (!haveCheckedHeaders(req)) {
            // This is our last chance; we're about to be disconnected from the connection
            Boolean persistentOK;
            readFromThisStream = checkHeaders(req, responseStream, &err, &persistentOK);
            if (isPersistent(req)) {
                if (!persistentOK) {
                    _CFNetConnectionLost(req->conn);
#if !defined(NO_PIPELINING)
                } else {
                    _CFNetConnectionSetShouldPipeline(req->conn, TRUE);
#endif
                }
            }
        }
        if (err.error != 0) {
            _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventErrorOccurred, &err);
        } else if (readFromThisStream) {
            _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventEndEncountered, NULL);
        }
    }
}

static Boolean shouldReattemptRequest(_CFHTTPRequest *req, CFStreamError *err, int oldState, _CFNetConnectionRef conn, Boolean *advanceToNextProxy) {
    *advanceToNextProxy = FALSE;
    
    if (__CFBitIsSet(req->flags, IS_ZOMBIE)) {
        return FALSE;
    } else if (!isPersistent(req) || __CFBitIsSet(req->flags, DO_NOT_REATTEMPT)) {
        // Under these circumstances, we should never reattempt with the current connection (or another connection just like it).
        // See if we should reattempt after advancing to the next proxy; if not, we just error out at this point.
        
        // test that we have a proxy to advance to, and that the prior state was kTransmittingRequest
        if (oldState == kTransmittingRequest && req->proxyList && CFArrayGetCount(req->proxyList) > 1 && (!__CFBitIsSet(req->flags, HAS_PAYLOAD) || __CFBitIsSet(req->flags, PAYLOAD_IS_DATA))) {
            *advanceToNextProxy = TRUE;
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (oldState == kNotQueued || oldState == kQueued) {
        return TRUE;
    } else if (__CFBitIsSet(req->flags, HAS_PAYLOAD) && !__CFBitIsSet(req->flags, PAYLOAD_IS_DATA)) {
        return FALSE;
    } else if (err->domain == kCFStreamErrorDomainHTTP && err->error == kCFStreamErrorHTTPConnectionLost) {
        // Somewhere upstream, the server decided to stop processing further pipelined requests
        return TRUE;
    } else if (oldState == kTransmittingRequest) {
        if (req->proxyList && CFArrayGetCount(req->proxyList) > 1) {
            *advanceToNextProxy = TRUE;
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        // oldState == kWaitingForResponse
        return TRUE;
    }
}

static Boolean performReattempt(_CFHTTPRequest *req, Boolean advanceToNextProxy) {
    CFStreamError dummy; // Caller never cares about the error when resetting...
        dequeueFromConnection1(req);
        closeRequestResources1(req);
        if (advanceToNextProxy) {
            advanceToNextProxyFromProxyArray(req->proxyList);
        }
    return resetForRequest(req->currentRequest, req, &dummy);
}

static void haveBeenOrphaned1(_CFHTTPRequest *req, int oldState, CFStreamError *err, _CFNetConnectionRef conn, Boolean *haveBeenDealloced) {
    Boolean shouldReattempt;
    Boolean advanceToNextProxy;

    *haveBeenDealloced = FALSE;
    shouldReattempt = shouldReattemptRequest(req, err, oldState, conn, &advanceToNextProxy);
    
    if (shouldReattempt) {
        if (!performReattempt(req, advanceToNextProxy)) {
            // report the error that caused the reattempt, not the new error
            if (err->domain == kCFStreamErrorDomainHTTP && err->error == kCFStreamErrorHTTPConnectionLost) {
                err->domain = _kCFStreamErrorDomainNativeSockets;
                err->error = ECONNRESET;
            }
            _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventErrorOccurred, err);
        }
    } else {
        if (req->requestPayload) {
            CFReadStreamClose(req->requestPayload);
            CFRelease(req->requestPayload);
            req->requestPayload = NULL;
        }
        if (req->requestFragment) {
            CFRelease(req->requestFragment);
            req->requestFragment = NULL;
        }
        if (!__CFBitIsSet(req->flags, IS_ZOMBIE)) {
            if (err->domain == kCFStreamErrorDomainHTTP && err->error == _kCFStreamErrorHTTPSProxyFailure) {
                    req->responseHeaders = (CFHTTPMessageRef)CFWriteStreamCopyProperty(_CFNetConnectionGetRequestStream(conn), kCFStreamPropertyCONNECTResponse);
                    addAuthenticationInfoToResponse1(req);
                    _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventEndEncountered, NULL);
            } else {
                if (err->domain == kCFStreamErrorDomainHTTP && err->error == kCFStreamErrorHTTPConnectionLost) {
                    err->domain = _kCFStreamErrorDomainNativeSockets;
                    err->error = ECONNRESET;
                }
                _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventErrorOccurred, err);
            }
            dequeueFromConnection1(req);
        } else {
            // We're a zombie; allocator is available from the old connection
            dequeueFromConnection1(req);
            httpRequestDestroy(CFGetAllocator(conn), req);
            *haveBeenDealloced = TRUE;
        }
    }
}

static void httpRequestStateChanged(void *request, int newState, CFStreamError *err, _CFNetConnectionRef conn, const void* key) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)request;
    int oldState = _CFHTTPRequestGetState(req);
    Boolean haveBeenDealloced = FALSE;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestStateChanged(req = 0x%x, newState = %d, conn = 0x%x)\n", (int)req, newState, (int)conn);
#endif
    _CFHTTPRequestSetState(req, newState);

    switch (newState) {
    case kQueued:
        break;
    case kTransmittingRequest:
        prepareTransmission1(req, _CFNetConnectionGetRequestStream(conn), conn);
        break;
    case kWaitingForResponse:
        concludeTransmission1(req, _CFNetConnectionGetRequestStream(conn));
        break;
    case kReceivingResponse:
        prepareReception1(req, _CFNetConnectionGetResponseStream(conn));
        break;
    case kFinished:
        concludeReception1(req, _CFNetConnectionGetResponseStream(conn), &haveBeenDealloced);
        break;
    case kOrphaned: {
        haveBeenOrphaned1(req, oldState, err, conn, &haveBeenDealloced);
        break;
    }
    default:
        CFLog(0, CFSTR("Encountered unexpected state %d for request 0x%x"), newState, req);
    }
    if (!haveBeenDealloced && req->stateChangeSource) {
        CFRunLoopSourceSignal(req->stateChangeSource);
    }
}

// Returns whether the request has been fully transmitted
static Boolean transmitRequest1(_CFHTTPRequest *http, CFWriteStreamRef destStream, CFStreamError *error, Boolean blockOnce) {
    Boolean done = FALSE;
    UInt8 buf[BUF_SIZE];
    const UInt8 *bytes;
    error->error = 0;
    
    if (__CFBitIsSet(http->flags, HAVE_SENT_REQUEST_PAYLOAD)) return TRUE;
    
	if (CFWriteStreamCopyProperty(destStream, _kCFStreamPropertyHTTPSProxyHoldYourFire))
		return TRUE;
	
    // if http->requestPayload is NULL, we still need to wait until the write stream reports canAcceptBytes, because otherwise, our request header hasn't been sent.
    if (http->requestPayload == NULL) {
        if (CFWriteStreamCanAcceptBytes(destStream)) {
            done = TRUE;
        } else if (blockOnce) {
            CFStreamStatus status = CFWriteStreamGetStatus(destStream);
            while ((status == kCFStreamStatusOpening || status == kCFStreamStatusOpen) && !CFWriteStreamCanAcceptBytes(destStream)) {
//#warning GROSS!
                usleep(5); // Shouldn't need to wait long for this....
                status = CFWriteStreamGetStatus(destStream);
            }
            done = TRUE;
        } else if (CFWriteStreamGetStatus(destStream) == kCFStreamStatusError) {
            *error = CFWriteStreamGetError(destStream);
            done = TRUE;
        } else {
            return FALSE;
        }
    }

    if (!done) {
        CFStreamStatus status = CFReadStreamGetStatus(http->requestPayload);
        if (status == kCFStreamStatusError) {
            *error = CFReadStreamGetError(http->requestPayload);
            done = TRUE;
        } else {
            status = CFWriteStreamGetStatus(destStream);
            if (status == kCFStreamStatusError) {
                *error = CFWriteStreamGetError(destStream);
                done = TRUE;
            } else if (status == kCFStreamStatusAtEnd) {
                // Premature end-of-stream
                done = TRUE;
                error->domain = kCFStreamErrorDomainHTTP;
                error->error = kCFStreamErrorHTTPParseFailure;
            }
        }
    }

    while (!done && (http->requestFragment || blockOnce || (CFReadStreamHasBytesAvailable(http->requestPayload) && CFWriteStreamCanAcceptBytes(destStream)))) {
        CFIndex bytesRead, bytesWritten;
        if (http->requestFragment) {
            bytesRead = CFDataGetLength(http->requestFragment);
            bytes = CFDataGetBytePtr(http->requestFragment);
        } else {
            bytesRead = CFReadStreamRead(http->requestPayload, buf, BUF_SIZE);
            bytes = buf;
            if (bytesRead < 0) {
                *error = CFReadStreamGetError(http->requestPayload);
                done = TRUE;
            } else if (bytesRead == 0) {
                done = TRUE;
            }
        }
        while (!done && bytesRead > 0 && (blockOnce || CFWriteStreamCanAcceptBytes(destStream))) {
            bytesWritten = CFWriteStreamWrite(destStream, bytes, bytesRead);
            if (bytesWritten < 0) {
                *error = CFWriteStreamGetError(destStream);
                done = TRUE;
            } else if (bytesWritten == 0) {
                // Premature end-of-stream
                done = TRUE;
                error->domain = kCFStreamErrorDomainHTTP;
                error->error = kCFStreamErrorHTTPParseFailure;
            } else {
                bytesRead -= bytesWritten;
                bytes += bytesWritten;
                http->requestBytesWritten += bytesWritten;
            }
        }
        if (http->requestFragment) {
            if (done || bytesRead <= 0) {
                CFRelease(http->requestFragment);
                http->requestFragment = NULL;
            } else {
                CFDataRef newData = CFDataCreate(CFGetAllocator(http->originalRequest), bytes, bytesRead);
                CFRelease(http->requestFragment);
                http->requestFragment = newData;
            }
        } else if (bytesRead > 0) {
            http->requestFragment = CFDataCreate(CFGetAllocator(http->originalRequest), bytes, bytesRead);
        }
        blockOnce = FALSE;
    }
    if (!done && !http->requestFragment && CFReadStreamGetStatus(http->requestPayload) == kCFStreamStatusAtEnd) {
        done = TRUE;
    }
    if (done) {
        closeRequestResources1(http);
        __CFBitSet(http->flags, HAVE_SENT_REQUEST_PAYLOAD);
    }
    return done;
}

static
void httpTransmitRequest(void *request, _CFNetConnectionRef connection, const void* key) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)request;
    CFWriteStreamRef requestStream = _CFNetConnectionGetRequestStream(connection);
    CFStreamError error;
    Boolean requestTransmitted;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpTransmitRequest(req = 0x%x, requestStream = 0x%x, conn = 0x%x)\n", (int)req, (int)requestStream, (int)connection);
#endif
    requestTransmitted = transmitRequest1(req, requestStream, &error, FALSE);
    if (error.error != 0) {
        if (error.domain == kCFStreamErrorDomainHTTP && error.error == _kCFStreamErrorHTTPSProxyFailure) {
            req->responseHeaders = (CFHTTPMessageRef)CFWriteStreamCopyProperty(requestStream, kCFStreamPropertyCONNECTResponse);
            addAuthenticationInfoToResponse1(req);
            __CFBitSet(req->flags, HAVE_CHECKED_RESPONSE_HEADERS);
            _CFNetConnectionRequestIsComplete(connection, req);
            _CFNetConnectionLost(connection); // Do not let anyone else use this stream
        } else {
            // Something went wrong
            if (error.domain != kCFStreamErrorDomainHTTP || error.error != kCFStreamErrorHTTPConnectionLost) {
                __CFBitSet(req->flags, DO_NOT_REATTEMPT);
            }
            _CFNetConnectionErrorOccurred(connection, &error);
        }
    } else if (requestTransmitted) {
        // Request completed
        _CFNetConnectionRequestIsComplete(connection, req);
    }
}


// Move the response along as much as possible; this may not be very far if we're waiting for our client to read....
static void httpReceiveResponse(void *request, _CFNetConnectionRef conn, const void* key) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)request;
    CFReadStreamRef responseStream = _CFNetConnectionGetResponseStream(conn);
    Boolean useThisStream;
    int state;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestReceiveResponse(req = 0x%x, responseStream = 0x%x, conn = 0x%x)\n", (int)req, (int)responseStream, (int)(conn));
#endif
    if (__CFBitIsSet(req->flags, IN_READ_CALLBACK)) return; // We're here because the client is already reading; httpRequestRead will take care of this....
    // Check if there's anything for us to do....
    if (!responseStream) return;
    state = CFReadStreamGetStatus(responseStream);
    if (state != kCFStreamStatusAtEnd && state != kCFStreamStatusError && !CFReadStreamHasBytesAvailable(responseStream) && !_CFHTTPReadStreamIsAtMark(responseStream)) return;

    if (!haveCheckedHeaders(req)) {
        CFStreamError err;
        Boolean persistentOK;
        useThisStream = checkHeaders(req, responseStream, &err, &persistentOK);
        if (err.error != 0) {
            if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
                __CFBitSet(req->flags, DO_NOT_REATTEMPT);
            }
            _CFNetConnectionErrorOccurred(conn, &err);
        } else {
            if (!persistentOK) {
                _CFNetConnectionLost(conn);
            }
            if (__CFBitIsSet(req->flags, FORCE_EOF)) {
                // We know from the request/response that there will never be any data
                _CFNetConnectionResponseIsComplete(conn, req);
                useThisStream = FALSE;
            }
        }
    } else {
        useThisStream = TRUE;
    }
    if (useThisStream) {
        if (CFReadStreamHasBytesAvailable(responseStream)) {
            if (__CFBitIsSet(req->flags, IS_ZOMBIE)) {
                // Just plow through the bytes
                UInt8 buf[BUF_SIZE];
                CFIndex bytesRead;
                while (CFReadStreamHasBytesAvailable(responseStream)) {
                    bytesRead = CFReadStreamRead(responseStream, buf, BUF_SIZE);
                    if (bytesRead < 0) {
                        CFStreamError err = CFReadStreamGetError(responseStream);
                        __CFBitSet(req->flags, DO_NOT_REATTEMPT);
                        _CFNetConnectionErrorOccurred(conn, &err);
                        break;
                    } else if (bytesRead == 0) {
                        break;
                    }
                }
            } else {
                _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventHasBytesAvailable, NULL);
            }
        } else if (_CFHTTPReadStreamIsAtMark(responseStream)) {
            _CFNetConnectionResponseIsComplete(conn, req);
        } else if (CFReadStreamGetStatus(responseStream) == kCFStreamStatusAtEnd) {
            _CFNetConnectionLost(conn);
            _CFNetConnectionResponseIsComplete(conn, req);
        } else if (CFReadStreamGetStatus(responseStream) == kCFStreamStatusError) {
            CFStreamError err = CFReadStreamGetError(responseStream);
            if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
                __CFBitSet(req->flags, DO_NOT_REATTEMPT);
            }
            _CFNetConnectionErrorOccurred(conn, &err);
        }
    }
}

static CFHTTPAuthenticationRef connectionOrientedAuth(_CFHTTPRequest *http, Boolean forProxy) {
    CFHTTPAuthenticationRef auth = _CFHTTPMessageGetAuthentication(http->currentRequest, forProxy);
    CFHTTPAuthenticationRef connAuth = NULL;
    if (auth) {
        CFStringRef method = CFHTTPAuthenticationCopyMethod(auth);
        if (method) {
            if (method == kCFHTTPAuthenticationSchemeNegotiate || method == kCFHTTPAuthenticationSchemeNTLM) {
                connAuth = auth;
            }
            CFRelease(method);
        }
    }
    return connAuth;
}

static void setConnectionFromProxyStream(_CFHTTPRequest *http, CFStreamError *err) {
    Boolean isComplete;
    err->domain = 0;
    http->proxyList = _CFNetworkCopyProxyFromProxyStream(http->proxyStream, &isComplete);
    if (!isComplete) {
        return;
    }
    CFRelease(http->proxyStream);
    http->proxyStream = NULL;
    if (!http->proxyList || CFArrayGetCount(http->proxyList) == 0) {
        // Error 
        err->domain = kCFStreamErrorDomainHTTP;
        err->error = kCFStreamErrorHTTPParseFailure;
    } else {
        // Just advance to the next proxy
        CFHTTPAuthenticationRef auth = connectionOrientedAuth(http, FALSE);
		CFHTTPAuthenticationRef proxyAuth = connectionOrientedAuth(http, TRUE);

		if (auth || proxyAuth) {
			// Connection-oriented auth schemes must be persistent, since you're authenticating
			// the pipe and it might take >1 message to complete the protocol.
			__CFBitSet(http->flags, IS_PERSISTENT);
		}
		
		CFHTTPMessageRef request = http->currentRequest ? http->currentRequest : http->originalRequest;
        CFURLRef targetURL = CFHTTPMessageCopyRequestURL(request);
        _CFNetConnectionCacheKey key = nextConnectionCacheKeyFromProxyArray(http, http->proxyList, targetURL, http->connProps);
        CFRelease(targetURL);
		__CFSpinLock(&cacheInitLock);
        if (httpConnectionCache == NULL) {
            httpConnectionCache = createConnectionCache();
        }
		__CFSpinUnlock(&cacheInitLock);
        http->conn = findOrCreateNetConnection(httpConnectionCache, CFGetAllocator(http->responseStream), &httpConnectionCallBacks, key, key, isPersistent(http), http->connProps);
        releaseConnectionCacheKey(key);
		
		if ((auth || proxyAuth) && http->conn) {
			
			err->error = 0;
			err->domain = 0;
			
			if (auth)
				*err = _CFHTTPAuthenticationApplyHeaderToRequest(auth, request, http->conn);
			
			if (!err->error && proxyAuth)
				*err = _CFHTTPAuthenticationApplyHeaderToRequest(proxyAuth, request, http->conn);
			
			if (err->error) {
				CFRelease(http->conn);
				http->conn = NULL;
				
				return;		/* NOTE the early bail! */
			}
		}
		
        _CFNetConnectionEnqueue(http->conn, http);
        if (!isPersistent(http)) {
            _CFNetConnectionSetAllowsNewRequests(http->conn, FALSE);
        }
    }
}

static void proxyInfoAvailable(CFReadStreamRef proxyStream, void *clientInfo) {
    _CFHTTPRequest *http = (_CFHTTPRequest *)clientInfo;
    CFStreamError err;
    if (__CFBitIsSet(http->flags, WAITING_FOR_PROXY_STREAM)) {
        // Don't call setConnectionFromProxyStream; some upper function (probably httpRequestRead) is waiting for the proxyStream to empty.
        // That function needs to capture the error code returned by setConnectionFromProxyStream, so we just clear this bit so it knows
        // it can safely call that function now.
        __CFBitClear(http->flags, WAITING_FOR_PROXY_STREAM);
    } else {
        setConnectionFromProxyStream(http, &err);
        if (err.domain != 0) {
            CFReadStreamSignalEvent(http->responseStream, kCFStreamEventErrorOccurred, &err);
        }
    }
}


static _CFNetConnectionRef getConnectionForRequest(_CFHTTPRequest *req, Boolean *created, CFStreamError *error) {
    _CFNetConnectionRef conn = NULL;
    CFHTTPAuthenticationRef auth;
	CFHTTPAuthenticationRef proxyAuth;

    error->domain = 0;
    error->error = 0;
#if (DEBUG)
    if (__CFBitIsSet(req->flags, IS_ZOMBIE)) {
        CFLog(0, CFSTR("Asked to enqueue a zombie request <0x%x>"), req);
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = -1000; // ???
        return NULL;
    }
#endif
    auth = connectionOrientedAuth(req, FALSE);
	proxyAuth = connectionOrientedAuth(req, TRUE);
	
    if (auth || proxyAuth) {
        // Connection-oriented auth schemes must be persistent, since you're authenticating
		// the pipe and it might take >1 message to complete the protocol.
        __CFBitSet(req->flags, IS_PERSISTENT);
	}
		
    CFURLRef targetURL = CFHTTPMessageCopyRequestURL(req->currentRequest ? req->currentRequest : req->originalRequest);
    CFStringRef scheme = targetURL ? CFURLCopyScheme(targetURL) : NULL;
    if (!targetURL) {
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = kCFStreamErrorHTTPBadURL;
        conn = NULL;
    } else if (!req->proxyList && (scheme == NULL)) {
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = kCFStreamErrorHTTPBadURL;
        conn = NULL;
    } else {
        CFReadStreamRef proxyStream = NULL;
        if (!req->proxyList) {
            // Go construct the proxy list
            CFStringRef proxyScheme = NULL;
            if (CFStringCompare(scheme, _kCFHTTPStreamFTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxyScheme = _kCFHTTPStreamHTTPScheme;
            } else if (CFStringCompare(scheme, _kCFHTTPStreamFTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxyScheme = _kCFHTTPStreamHTTPSScheme;
            }
            req->proxyList = _CFNetworkFindProxyForURLAsync(proxyScheme, targetURL, NULL, req->proxyDict, proxyInfoAvailable, req, &proxyStream);
        }
    
        if (!req->proxyList) {
            CFArrayRef rlArray = _CFReadStreamGetRunLoopsAndModes(req->responseStream);
            CFIndex count;
            // asynchronous proxy lookup is underway
            req->proxyStream = proxyStream;
            conn = NULL;
            if (rlArray && (count = CFArrayGetCount(rlArray)) != 0) {
                CFIndex index;
                for (index = 0; index+1 < count; index += 2) {
                    CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(rlArray, index);
                    CFStringRef mode = CFArrayGetValueAtIndex(rlArray, index + 1);
                    CFReadStreamScheduleWithRunLoop(proxyStream, rl, mode);
                }
            }
        } else if (CFArrayGetCount(req->proxyList) == 0) {
            // Need a PAC error code here
            error->domain = kCFStreamErrorDomainHTTP;
            error->error = kCFStreamErrorHTTPParseFailure;
            conn = NULL;
        } else {
            // Just advance to the next proxy
            _CFNetConnectionCacheKey key = nextConnectionCacheKeyFromProxyArray(req, req->proxyList, targetURL, req->connProps);
			__CFSpinLock(&cacheInitLock);
			if (httpConnectionCache == NULL) {
                httpConnectionCache = createConnectionCache();
            }
			__CFSpinUnlock(&cacheInitLock);
            conn = findOrCreateNetConnection(httpConnectionCache, CFGetAllocator(req->responseStream), &httpConnectionCallBacks, key, key, isPersistent(req), req->connProps);
            releaseConnectionCacheKey(key);
        }
    }
	
    if ((auth || proxyAuth) && conn) {
		
		if (auth)
			*error = _CFHTTPAuthenticationApplyHeaderToRequest(auth, req->currentRequest, conn);
		
		if (!error->error && proxyAuth) {
			
			*error = _CFHTTPAuthenticationApplyHeaderToRequest(proxyAuth, req->currentRequest, conn);
			
			if (!error->error) {
				
				CFStringRef method = CFHTTPAuthenticationCopyMethod(proxyAuth);
				
				if (method && (CFStringCompare(method, _kCFNTLMMethod, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
					
					if (scheme && (CFStringCompare(scheme, _kCFHTTPStreamHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
						
						CFAllocatorRef alloc = CFGetAllocator(req->responseStream);
						CFMutableDictionaryRef new_value = NULL;
						CFStringRef header;
						CFDictionaryRef property = CFWriteStreamCopyProperty(_CFNetConnectionGetRequestStream(conn), kCFStreamPropertyCONNECTProxy);
						
						if (!property)
							property = CFDictionaryGetValue(req->connProps, kCFStreamPropertyCONNECTProxy);
						
						//_CFHTTPAuthenticationApplyHeaderToRequest(proxyAuth, req->currentRequest, conn);
						
						new_value = property ? CFDictionaryCreateMutableCopy(alloc, 0, property) : CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						if (property) CFRelease(property);
						
						header = CFHTTPMessageCopyHeaderFieldValue(req->currentRequest, _kCFHTTPStreamProxyAuthorizationHeader);
						if (header) {
							
							CFMutableDictionaryRef headers = NULL;
							
							property = CFDictionaryGetValue(new_value, kCFStreamPropertyCONNECTAdditionalHeaders);
							
							if (property)
								headers = CFDictionaryCreateMutableCopy(alloc, 0, property);
							else
								headers = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
							
							CFDictionarySetValue(headers, _kCFHTTPStreamProxyAuthorizationHeader, header);
							CFRelease(header);

							CFDictionarySetValue(headers, _kCFHTTPStreamProxyConnectionHeader, _kCFHTTPStreamConnectionKeepAlive);
							
							CFDictionarySetValue(new_value, kCFStreamPropertyCONNECTAdditionalHeaders, headers);
							CFRelease(headers);
						}
						
						CFWriteStreamSetProperty(_CFNetConnectionGetRequestStream(conn), kCFStreamPropertyCONNECTProxy, new_value);
						
						CFRelease(new_value);
					}
				}
				
				if (method) CFRelease(method);
			}
		}
		
		if (error->error) {
			CFRelease(conn);
			conn = NULL;
		}
    }
	
	if (scheme) CFRelease(scheme);
	
    if (targetURL) CFRelease(targetURL);

    return conn;
}

static void _CFStreamSocketCreatedCallBack(int fd, void* ctxt) {
	
	int yes = 1;
	
	(void)ctxt;		/* unused */
	
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(yes));
}

static CFStreamError httpCreateConnectionStreams(CFAllocatorRef alloc, const void *info, CFWriteStreamRef *requestStream, CFReadStreamRef *responseStream) {
    const _CFNetConnectionCacheKey key = (_CFNetConnectionCacheKey)info;
    
    CFReadStreamRef rStream;
    CFWriteStreamRef wStream;
    
    CFStreamError err = {0, 0};
    CFIndex count;
    CFArrayRef array;
    CFStringRef host;
    SInt32 port;
    UInt32 connType;
    CFDictionaryRef properties;
    
    getValuesFromKey(key, &host, &port, &connType, &properties);
    
    if (properties && (array = CFDictionaryGetValue(properties, _kCFStreamPropertyHTTPConnectionStreams)) != NULL) {
        rStream = (CFReadStreamRef)CFArrayGetValueAtIndex(array, 0);
        wStream = (CFWriteStreamRef)CFArrayGetValueAtIndex(array, 1);
        CFRetain(rStream);
        CFRetain(wStream);
    } else {
		CFArrayRef callback;
		CFArrayCallBacks cb = {0, NULL, NULL, NULL, NULL};
		const void* values[2] = {_CFStreamSocketCreatedCallBack, NULL};

        // Create the socket streams ourselves
		_CFSocketStreamCreatePair(alloc, host, port, 0, NULL, &rStream, &wStream);
        if (!rStream) {
            err.domain = kCFStreamErrorDomainPOSIX;
            err.error = ENOMEM;
            return err;
        }
		
		callback = CFArrayCreate(alloc, values, sizeof(values) / sizeof(values[0]), &cb);
		
		if (callback) {
			CFWriteStreamSetProperty(wStream, _kCFStreamSocketCreatedCallBack, callback);
			CFRelease(callback);
		}

        if (connType == kHTTPS) {
            CFReadStreamSetProperty(rStream, kCFStreamPropertySocketSecurityLevel, kCFStreamSocketSecurityLevelNegotiatedSSL);
        }
    }

    *requestStream = CFWriteStreamCreateHTTPStream(alloc, NULL, (connType == kHTTPProxy || connType == kHTTPSProxy), wStream);
    CFWriteStreamSetProperty(*requestStream, _kCFStreamPropertyHTTPPersistent, kCFBooleanTrue);
    CFRelease(wStream);
    *responseStream = CFReadStreamCreateHTTPStream(alloc, rStream, TRUE);
    CFReadStreamSetProperty(*responseStream, _kCFStreamPropertyHTTPPersistent, kCFBooleanTrue);
    CFRelease(rStream);
 
    if (properties && (count = CFDictionaryGetCount(properties)) > 0) {
        CFStringRef *keys = CFAllocatorAllocate(alloc, sizeof(CFStringRef)*count*2, 0);
        CFTypeRef *values = (CFTypeRef *)(keys + count);
        CFIndex index;
        CFDictionaryGetKeysAndValues(properties, (const void **)keys, (const void **)values);
        for (index = 0; index < count; index ++) {
            CFReadStreamSetProperty(*responseStream, keys[index], values[index]);
            CFWriteStreamSetProperty(*requestStream, keys[index], values[index]);
        }
        CFAllocatorDeallocate(alloc, keys);
    }
    
    return err;
}

static Boolean resetForRequest(CFHTTPMessageRef newRequest, _CFHTTPRequest *http, CFStreamError *error) {
    __CFBitClear(http->flags, HAVE_SENT_REQUEST_HEADERS);
    __CFBitClear(http->flags, HAVE_SENT_REQUEST_PAYLOAD);
    __CFBitClear(http->flags, HAVE_CHECKED_RESPONSE_HEADERS);
    __CFBitClear(http->flags, FORCE_EOF);
    __CFBitClear(http->flags, HAVE_READ_MARK);

    if (newRequest != http->currentRequest) {
        if (http->currentRequest) CFRelease(http->currentRequest);
        CFRetain(newRequest);
        http->currentRequest = newRequest;
        http->requestBytesWritten = 0;
    }
    // now set up the new connection
    if (http->conn && !_CFNetConnectionWillEnqueueRequests(http->conn)) {
        // The connection went dead between when we configured it and now
        dequeueFromConnection1(http);
    }
    if (!http->conn) {
        Boolean dummy;
        http->conn = getConnectionForRequest(http, &dummy, error);
    } 
    if (error->domain != 0) {
        return FALSE;
    } else if (!http->conn) {
        // Asynchronous discovery of the correct connection; getConnectionForRequest took care of setting everything up 
        return TRUE;
    } else {
        _CFNetConnectionEnqueue(http->conn, http);
        if (!isPersistent(http)) {
            _CFNetConnectionSetAllowsNewRequests(http->conn, FALSE);
        }
        return TRUE;
    }
}

static Boolean httpRequestOpen(CFReadStreamRef stream, CFStreamError *error, Boolean *openComplete, void *info) {
    _CFHTTPRequest *http = (_CFHTTPRequest *)info;
    CFHTTPMessageRef newRequest = CFHTTPMessageCreateCopy(CFGetAllocator(stream), http->originalRequest);
    Boolean result;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestOpen(req = 0x%x)\n", (int)http);
#endif
    if (!resetForRequest(newRequest, http, error)) {
        *openComplete = TRUE;
        result = FALSE;
    } else {
        *openComplete = http->proxyStream ? FALSE : (_CFHTTPRequestGetState(http) > kQueued);
        result = TRUE;
    }
    CFRelease(newRequest);
    return result;
}

static Boolean httpRequestOpenCompleted(CFReadStreamRef stream, CFStreamError *error, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
    int currentState;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestOpenCompleted(req = 0x%x)\n", (int)req);
#endif
    if (__CFBitIsSet(req->flags, OPEN_SIGNALLED)) return TRUE;
    if (req->proxyStream) {
        setConnectionFromProxyStream(req, error);
        if (error->domain != 0) return TRUE;
        else if (req->proxyStream) return FALSE;
    }
    if (req->conn) {
        currentState = _CFNetConnectionGetState(req->conn, TRUE, req); // This will drive the connection forward if necessary; it will also update our internal state, so we don't need to do that here.
    } else {
        currentState = _CFHTTPRequestGetState(req);
    }
    return (currentState > kQueued);
}


static CFHTTPMessageRef constructRedirectedRequest(CFURLRef newDest, CFHTTPMessageRef origRequest) {
    // This is awfully expensive; we might want to do something more straitforward in CFHTTPMessage in SPI.  Also, if origRequest is going to be discarded, this is really pricey.
    CFStringRef reqMethod = CFHTTPMessageCopyRequestMethod(origRequest);
    CFStringRef oldVersion = CFHTTPMessageCopyVersion(origRequest);
    CFHTTPMessageRef newRequest = CFHTTPMessageCreateRequest(CFGetAllocator(origRequest), reqMethod, newDest, oldVersion);
    CFDictionaryRef reqHeaders;
    CFRelease(reqMethod);
    CFRelease(oldVersion);
    if (!newRequest) return NULL;
    reqHeaders = CFHTTPMessageCopyAllHeaderFields(origRequest);
    if (reqHeaders) {
        // This loses the ordering on the headers; do we care?
        CFIndex count = CFDictionaryGetCount(reqHeaders);
        if (count > 0) {
            CFStringRef *keys, *values, *currentKey, *currentValue;
            CFAllocatorRef alloc = CFGetAllocator(origRequest);
            keys = CFAllocatorAllocate(alloc, sizeof(CFStringRef) * 2 * count, 0);
            values = keys + count;
            CFDictionaryGetKeysAndValues(reqHeaders, (const void **)keys, (const void **)values);
            for (currentKey = keys, currentValue = values; currentKey < values; currentValue ++, currentKey ++) {
                CFHTTPMessageSetHeaderFieldValue(newRequest, *currentKey, *currentValue);
            }
            CFAllocatorDeallocate(alloc, keys);
        }
        CFRelease(reqHeaders);
    }
    // Use false for isPersistent and forProxy flag because whatever header we need is already present on newRequest (copied from origRequest)
    cleanUpRequest(newRequest, -1, false, false);
    return newRequest;
}

// Codes chosen to mirror HTTP respones codes.

// DONE is returned when the result is OK, but we know there aren't any more bytes coming (no matter what the header said) - e.g. a HEAD request
#define DONE (-100)
// For use when we want proper handling of 100's
#define EXTRA_RESPONSE (100)
#define OK  (200)
#define REDIRECT (300)
#define AUTHENTICATE (401)
#define PROXY_AUTHENTICATE (407)

// If we return OK or DONE, we need to set the HAVE_CHECKED_RESPONSE_HEADERS bit
static int nextActionForHeaders(CFHTTPMessageRef headers, _CFHTTPRequest *http, CFURLRef *nextURL, Boolean *connectionStaysPersistent) {
    CFStringRef requestMethod = CFHTTPMessageCopyRequestMethod(http->originalRequest);
    int status = CFHTTPMessageGetResponseStatusCode(headers);
    int result;
    *connectionStaysPersistent = canKeepAlive(headers, http->currentRequest);
    if ((status >= 100 && status <200) || status == 204 || status == 304 || CFEqual(_kCFHTTPStreamHEADMethod, requestMethod)) {
        // These are the conditions under which the server must not send further information
        CFRelease(requestMethod);
        __CFBitSet(http->flags, HAVE_CHECKED_RESPONSE_HEADERS);
        return DONE;
    }
    CFRelease(requestMethod);
    if (__CFBitIsSet(http->flags, AUTOREDIRECT) && status >= 300 && status < 400) {
        // Fetch the nextURL
        CFStringRef newLocation = CFHTTPMessageCopyHeaderFieldValue(headers, _kCFHTTPStreamLocationHeader);
        CFURLRef lastURL = CFArrayGetValueAtIndex(http->redirectedURLs, CFArrayGetCount(http->redirectedURLs) - 1);
        if (newLocation) {
            CFAllocatorRef alloc = CFGetAllocator(headers);
            // Check if the server has for some reason returned multiple Location fields.  eBay does this
            CFRange commaRg = CFStringFind(newLocation, _kCFHTTPStreamLocationSeparator, 0);
            if (commaRg.location != kCFNotFound) {
                CFStringRef substr = CFStringCreateWithSubstring(alloc, newLocation, CFRangeMake(0, commaRg.location));
                *nextURL = CFURLCreateWithString(alloc, substr, lastURL);
                CFRelease(substr);
            } else {
                *nextURL = CFURLCreateWithString(alloc, newLocation, lastURL);
            }
            CFRelease(newLocation);
        } else {
            *nextURL = NULL;
        }
        if (*nextURL) {
            result = REDIRECT;
        } else {
            result = OK;
        }
    } else {
        result = OK;
    }
    if (result == OK) {
        __CFBitSet(http->flags, HAVE_CHECKED_RESPONSE_HEADERS);
    }
    *connectionStaysPersistent = canKeepAlive(headers, http->currentRequest);
    return result;
}

static void addAuthenticationInfoToResponse1(_CFHTTPRequest *http) {
    CFURLRef requestedURL;
	_CFNetConnectionRef conn = http->conn;
	Boolean persistent = _CFNetConnectionWillEnqueueRequests(conn);
    CFHTTPAuthenticationRef auth = NULL;
    if (__CFBitIsSet(http->flags, AUTOREDIRECT)) {
        requestedURL = CFArrayGetValueAtIndex(http->redirectedURLs, CFArrayGetCount(http->redirectedURLs) - 1);
        CFRetain(requestedURL);
    } else {
        requestedURL = CFHTTPMessageCopyRequestURL(http->originalRequest);
    }
    _CFHTTPMessageSetResponseURL(http->responseHeaders, requestedURL);
    CFRelease(requestedURL);
	
    // Update auth token if there is one
    auth = _CFHTTPMessageGetAuthentication(http->originalRequest, FALSE);
    if (auth) {
        _CFHTTPAuthenticationUpdateFromResponse(auth, http->responseHeaders, conn);
		if (!persistent)
			_CFHTTPAuthenticationDisassociateConnection(auth, conn);
    }
	
	// Update the proxy auth if there is one
    auth = _CFHTTPMessageGetAuthentication(http->originalRequest, TRUE);
    if (auth) {
        _CFHTTPAuthenticationUpdateFromResponse(auth, http->responseHeaders, conn);
		if (!persistent)
			_CFHTTPAuthenticationDisassociateConnection(auth, conn);
    }
}

// Our first peek at the headers; see if further action is required
// Returns true if the stream did not need to be reconfigured; false otherwise
static Boolean checkHeaders(_CFHTTPRequest *http, CFReadStreamRef stream, CFStreamError *error, Boolean *connectionStaysPersistent) {
    CFURLRef nextURL = NULL;
    Boolean result = TRUE;
    int nextAction;
    
    if (!stream) {
        CFLog(0, CFSTR("Internal consistency check error for http request 0x%x"), http);
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = -1;
        return TRUE;
    }

    error->error = 0;
    if (http->responseHeaders) CFRelease(http->responseHeaders);
    http->responseHeaders = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);

    if (!http->responseHeaders) {
        // 0.9 response
        nextAction = OK;
        *connectionStaysPersistent = FALSE;
        __CFBitSet(http->flags, HAVE_CHECKED_RESPONSE_HEADERS);
    } else {
        nextAction = nextActionForHeaders(http->responseHeaders, http, &nextURL, connectionStaysPersistent); // This routine guarantees haveCheckedHeaders() will return TRUE next time if nextAction is OK
    }
	
	/*
	 ** This was moved here fromhttpResponseStreamCallBack in order to properly mark
	 ** the state of the connection so NTLM fail-over works.
	 */
	if (!error->error && isPersistent(http)) {
		if (!*connectionStaysPersistent) {
			_CFNetConnectionLost(http->conn);
#if !defined(NO_PIPELINING)
		} else {
			_CFNetConnectionSetShouldPipeline(http->conn, TRUE);
#endif
		}
	}
	
	/* Make this last so state is good before carryover work for authentication. */
	if (http->responseHeaders)
		addAuthenticationInfoToResponse1(http);

    switch (nextAction) {
    case DONE:
        __CFBitSet(http->flags, FORCE_EOF);
        break;
    case REDIRECT:
        if (!http->firstRedirection) {
            http->firstRedirection = http->responseHeaders;
            CFRetain(http->firstRedirection);
        }
        if (!CFArrayContainsValue(http->redirectedURLs, CFRangeMake(0, CFArrayGetCount(http->redirectedURLs)), nextURL)) {
            CFHTTPMessageRef newRequest = constructRedirectedRequest(nextURL, http->currentRequest);
            CFArrayAppendValue(http->redirectedURLs, nextURL);
            dequeueFromConnection1(http);
            closeRequestResources1(http);
            
            // This forces the proxy list to be recomputed, which we need to do since currently it only holds the remaining rollover possibilities - REW
            CFRelease(http->proxyList);
            http->proxyList = NULL;
            if (resetForRequest(newRequest, http, error)) {
                result = FALSE;
            }
            CFRelease(newRequest);
        } else {
            error->domain = kCFStreamErrorDomainHTTP;
            error->error = kCFStreamErrorHTTPRedirectionLoop;
        }
        break;
    case AUTHENTICATE:
        // Not implemented
    case PROXY_AUTHENTICATE:
        // Not implemented
    case OK:
    default:
        // Just fall through....
        ;
    }
    if (nextURL) {
        CFRelease(nextURL);
    }

    return result;
}

static CFIndex readFromConnection(_CFHTTPRequest *req, UInt8 *buffer, CFIndex length, Boolean *atEOF, CFStreamError *error) {
    Boolean readFromThisStream = TRUE;  // We set this to FALSE if we had to shut down one read stream and start a new one
    CFReadStreamRef stream = _CFNetConnectionGetResponseStream(req->conn);
    _CFNetConnectionRef connWeAreDoneWith = NULL;
    CFIndex result;
    *atEOF = FALSE;
    error->error = 0;

    if (_CFHTTPReadStreamIsAtMark(stream)) {
        result = 0;
        *atEOF = TRUE;
        connWeAreDoneWith = req->conn;
        CFRetain(connWeAreDoneWith);
    } else {
        result = CFReadStreamRead(stream, buffer, length);
        if (result < 0) {
            *error = CFReadStreamGetError(stream);
            if (error->domain != kCFStreamErrorDomainHTTP || error->error != kCFStreamErrorHTTPConnectionLost) {
                __CFBitSet(req->flags, DO_NOT_REATTEMPT);
            }
            _CFNetConnectionErrorOccurred(req->conn, error);
            return -1;
        } else if (kCFStreamStatusAtEnd == CFReadStreamGetStatus(stream) || _CFHTTPReadStreamIsAtMark(stream)) {
            connWeAreDoneWith = req->conn;
            CFRetain(connWeAreDoneWith);
            *atEOF = TRUE;
        }
    }

    // If we've never looked at the headers, see if they're available now.  If they are, we may have to take extra action
    if (!haveCheckedHeaders(req)) {
        Boolean connectionStaysPersistent;
        readFromThisStream = checkHeaders(req, stream, error, &connectionStaysPersistent);
        if (isPersistent(req)) {
            if (!connectionStaysPersistent) {
                _CFNetConnectionLost(req->conn);
#if !defined(NO_PIPELINING)
            } else {
                _CFNetConnectionSetShouldPipeline(req->conn, TRUE);
#endif
            }
        }
    }
    if (error->error == 0) {
        if (!readFromThisStream) {
            // The work above changed the read stream; discard our current read bytes and perform the read all over again.
            result = httpRequestRead(req->responseStream, buffer, length, error, atEOF, req);
        } else if (__CFBitIsSet(req->flags, FORCE_EOF)) {
            // We're done, but the filtered stream can't know that (we used some knowledge from the request, like that this is a HEAD request, to determine that there's no content coming).  We must force the correct responses here.
            connWeAreDoneWith = req->conn;
            result = 0;
        }
    }
    if (connWeAreDoneWith) {
        _CFNetConnectionResponseIsComplete(connWeAreDoneWith, req);
        CFRelease(connWeAreDoneWith);
    }
    return result;
}

extern void emptyPerform(void *info) {
    // Just a stub function because we can't create a run loop source with a NULL perform function
}

static CFIndex httpRequestRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info) {
    CFIndex result;
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
    _CFNetConnectionRef oldConn;
    enum _CFNetConnectionState state;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestRead(req = 0x%x)\n", (int)req);
#endif

    if (req->proxyStream) {
        setConnectionFromProxyStream(req, error);
        if (error->domain != 0) {
            return -1;
        }
        if (req->proxyStream) {
            // Must wait for the proxy stream to empty
            CFRunLoopRef currentRL = CFRunLoopGetCurrent();
            CFStringRef mode = _kCFHTTPStreamPrivateRunLoopMode;
            CFReadStreamScheduleWithRunLoop(req->proxyStream, currentRL, mode);
            __CFBitSet(req->flags, WAITING_FOR_PROXY_STREAM);
            while (__CFBitIsSet(req->flags, WAITING_FOR_PROXY_STREAM)) {
                CFRunLoopRunInMode(mode, 1e+20, TRUE);
            }
            setConnectionFromProxyStream(req, error);
            if (error->domain != 0) { 
                return -1;
            }
        }
    }
    oldConn = req->conn; // We grab the old connection so we can detect it if the connection changes through GetState below.
    __CFBitSet(req->flags, IN_READ_CALLBACK);
    state = oldConn ? _CFNetConnectionGetState(oldConn, TRUE, req) : -1;

    if (state < kReceivingResponse) {
        CFRunLoopRef currentRL = CFRunLoopGetCurrent();
        CFStringRef mode = _kCFHTTPStreamPrivateRunLoopMode;
        CFReadStreamScheduleWithRunLoop(stream, currentRL, mode);
        if (req->requestPayload) {
            CFReadStreamScheduleWithRunLoop(req->requestPayload, currentRL, mode);
        }
        CFRetain(oldConn);
        if (!req->stateChangeSource) {
            CFRunLoopSourceContext rlsCtxt = {0, req, NULL, NULL, NULL, NULL, NULL, NULL, NULL, emptyPerform};
            req->stateChangeSource = CFRunLoopSourceCreate(CFGetAllocator(stream), 0, &rlsCtxt);
        }
        CFRunLoopAddSource(currentRL, req->stateChangeSource, mode);
    
        // Poll once more to make sure events didn't get dropped while we were scheduling
        state = _CFNetConnectionGetState(oldConn, TRUE, req);
        while ((oldConn == req->conn) && state < kReceivingResponse) {
            CFRunLoopRunInMode(mode, 1e+20, TRUE);
            //state = req->conn ? _CFNetConnectionGetState(req->conn, TRUE, req) : -1;
            state = _CFNetConnectionGetState(oldConn, TRUE, req);
            if (state >= kReceivingResponse && oldConn != req->conn && req->conn) {
                // We've moved to a new connection; we need to go through this loop once more, so we can get the correct state from the new connection
                CFRelease(oldConn);
                oldConn = req->conn;
                CFRetain(oldConn);
                state = kQueued;
            }
        }
        CFReadStreamUnscheduleFromRunLoop(stream, currentRL, mode);
        if (req->requestPayload) {
            CFReadStreamUnscheduleFromRunLoop(req->requestPayload, currentRL, mode);
        }
        CFRelease(oldConn);
        CFRunLoopRemoveSource(currentRL, req->stateChangeSource, mode);
    }

    __CFBitClear(req->flags, IN_READ_CALLBACK);
    if (state == kFinished) {
        error->error = 0;
        *atEOF = TRUE;
        result = 0;
    } else if (state == kOrphaned || !req->conn) {
        *error = CFReadStreamGetError(stream);
        if (error->error == 0) {
        error->error = ECONNRESET;
        error->domain = _kCFStreamErrorDomainNativeSockets;
        }
        result = -1;
    } else {
        oldConn = req->conn;
        result = readFromConnection(req, buffer, bufferLength, atEOF, error);
        if (result < 0 && req->conn != oldConn && req->conn != NULL) {
            result = httpRequestRead(stream, buffer, bufferLength, error, atEOF, info);
        }
    }
    return result;    
}

static Boolean httpRequestCanRead(CFReadStreamRef ourStream, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
    int state;
    CFReadStreamRef stream;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestCanRead(req = 0x%x)\n", (int)req);
#endif
    if (req->proxyStream) {
        // Attempt to get the proxy info
        CFStreamError err;
        setConnectionFromProxyStream(req, &err);
        if (err.domain != 0) {
            CFReadStreamSignalEvent(req->responseStream, kCFStreamEventErrorOccurred, &err);
            return FALSE;
        } 
    }
     
    if (req->proxyStream) {
        // proxyStream isn't done yet
        return FALSE;
    }

    if (req->conn) {
        state = _CFNetConnectionGetState(req->conn, TRUE, req); // This will drive the connection forward if necessary; it will also update our internal state, so we don't need to do that here.
        if (!req->conn && state <= kReceivingResponse) {
            // !!!! - We still don't know why this is necessary; sometimes CFHTTPConnectionGetState drives the current req all the way to completion, but does not return one of the "all done" states, which cause the code below to end up dereferencing req->conn, which has been set to NULL.... - REW, 12/5/2002
            CFLog(0, CFSTR("Detected bad return from CFNetConnectionGetState"));
            state = _CFHTTPRequestGetState(req);
        }
    } else {
        state = _CFHTTPRequestGetState(req);
    }
    if (state > kReceivingResponse) {
        // This stream's been emptied
        return TRUE;
    }  else if (state < kReceivingResponse) {
        return FALSE;
    }
    stream = _CFNetConnectionGetResponseStream(req->conn);
    if (!CFReadStreamHasBytesAvailable(stream)) {
        return FALSE;
    } else if (__CFBitIsSet(req->flags, HAVE_CHECKED_RESPONSE_HEADERS)) {
        return TRUE;
    } else {
        CFStreamError err = {0, 0};
        Boolean persistentOK;
        Boolean useThisStream = checkHeaders(req, stream, &err, &persistentOK);
        if (err.error != 0) {
            if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
                __CFBitSet(req->flags, DO_NOT_REATTEMPT);
            }
            _CFNetConnectionErrorOccurred(req->conn, &err);
        } else if (isPersistent(req)) {
            if (!persistentOK) {
                _CFNetConnectionLost(req->conn);
#if !defined(NO_PIPELINING)
            } else {
                _CFNetConnectionSetShouldPipeline(req->conn, TRUE);
#endif
            }
        }
        if (useThisStream) {
            return TRUE;
        }
    }
    return FALSE;
}

static void httpRequestClose(CFReadStreamRef stream, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestClose(req = 0x%x)\n", (int)req);
#endif
    if (req->conn) {
        dequeueFromConnection1(req);
    }
    if (req->proxyStream) {
        CFReadStreamClose(req->proxyStream);
        CFRelease(req->proxyStream);
        req->proxyStream = NULL;
    }
}

static CFTypeRef httpRequestCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
    CFTypeRef property = NULL;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestCopyProperty(req = 0x%x)\n", (int)req);
#endif
    if (CFEqual(propertyName, kCFStreamPropertyHTTPResponseHeader)) {
        property = req->responseHeaders;
        if (property) CFRetain(property);
	} else if (CFEqual(propertyName, kCFStreamPropertySSLPeerCertificates)) {
		if (req->peerCertificates)
			property = CFRetain(req->peerCertificates);
		else if (req->conn) {
			CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(req->conn);
			if (rStream) {
				property = CFReadStreamCopyProperty(rStream, propertyName);
			}
			if (!property) {
				CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(req->conn);
				if (wStream) {
					property = CFWriteStreamCopyProperty(wStream, propertyName);
				}
			}
		}
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPFinalURL)) {
        if (req->currentRequest) {
            property = CFHTTPMessageCopyRequestURL(req->currentRequest);
        } else {
            property = CFHTTPMessageCopyRequestURL(req->originalRequest);
        }
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPRequest)) {
        // Client wants the final, redirected request.
        static Boolean warnOnce = FALSE;
        if (!warnOnce) {
            CFLog(0, CFSTR("Use of kCFStreamPropertyHTTPRequest is deprecated; please use kCFStreamPropertyHTTPFinalURL instead"));
            warnOnce = TRUE;
        }
        property = req->currentRequest;
        if (property) CFRetain(property);
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPProxy)) {
        property = req->proxyDict;
        if (property) CFRetain(property);
    } else if (CFEqual(propertyName, kCFHTTPRedirectionResponse)) {
        property = req->firstRedirection;
        if (property) CFRetain(property);
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPRequestBytesWrittenCount)) {
        property = CFNumberCreate(CFGetAllocator(stream), kCFNumberLongLongType, &(req->requestBytesWritten));
    } else if (req->conn) {
        CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(req->conn);
        if (rStream) {
            property = CFReadStreamCopyProperty(rStream, propertyName);
        }
        if (!property) {
            CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(req->conn);
            if (wStream) {
                property = CFWriteStreamCopyProperty(wStream, propertyName);
            }
        }
    } else {
        property = NULL;
    }
    return property;
}

static Boolean httpRequestSetProperty(CFReadStreamRef stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info) {
    _CFHTTPRequest *http = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestSetProperty(req = 0x%x)\n", (int)http);
#endif 
    if (CFReadStreamGetStatus(stream) > kCFStreamStatusNotOpen) return FALSE;
    if (CFEqual(propertyName, kCFStreamPropertyHTTPShouldAutoredirect)) {
        if (propertyValue == kCFBooleanTrue) {
            if (!http->redirectedURLs) {
                http->redirectedURLs = CFArrayCreateMutable(CFGetAllocator(stream),0, &kCFTypeArrayCallBacks);
                CFArrayAppendValue(http->redirectedURLs, CFHTTPMessageCopyRequestURL(http->originalRequest));
                // Ended up with an extra retain on the URL just added
                CFRelease((CFTypeRef)CFArrayGetValueAtIndex(http->redirectedURLs, 0));
            }
            __CFBitSet(http->flags, AUTOREDIRECT);
            return TRUE;
        } else if (propertyValue == kCFBooleanFalse) {
            if (http->redirectedURLs) {
                CFRelease(http->redirectedURLs);
                http->redirectedURLs = NULL;
            }
            __CFBitClear(http->flags, AUTOREDIRECT);
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPProxy)) {
        if (propertyValue == NULL) {
            if (http->proxyDict) {
                CFRelease(http->proxyDict);
                http->proxyDict = NULL;
            }
            return TRUE;
        } else if (CFGetTypeID(propertyValue) != CFDictionaryGetTypeID()) {
            return FALSE;
        } else {
            if (http->proxyDict) CFRelease(http->proxyDict);
            http->proxyDict = CFDictionaryCreateCopy(CFGetAllocator(stream), propertyValue);
            return TRUE;
        }
    } else if (CFEqual(propertyName, _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString)) {
        if (propertyValue == NULL) {
            if (http->proxyDict && CFDictionaryGetValue(http->proxyDict, _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString)) {
                CFMutableDictionaryRef mDict = CFDictionaryCreateMutableCopy(CFGetAllocator(stream), CFDictionaryGetCount(http->proxyDict), http->proxyDict);
                CFDictionaryRemoveValue(mDict, _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString);
                CFRelease(http->proxyDict);
                http->proxyDict = mDict;
            }
        } else {
            if (http->proxyDict) {
                CFMutableDictionaryRef mDict = CFDictionaryCreateMutableCopy(CFGetAllocator(stream), CFDictionaryGetCount(http->proxyDict) + 1, http->proxyDict);
                CFDictionarySetValue(mDict, _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString, propertyValue);
                CFRelease(http->proxyDict);
                http->proxyDict = mDict;
            } else {
                http->proxyDict = CFDictionaryCreate(CFGetAllocator(stream), (const void **)(&_kCFStreamPropertyHTTPProxyProxyAutoConfigURLString), &propertyValue, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            }
        }
        return TRUE;
    } else if (CFEqual(propertyName, kCFStreamPropertySOCKSProxy)) {
        if (!propertyValue) {
            if (http->proxyDict && CFDictionaryGetValue(http->proxyDict, kCFStreamPropertySOCKSProxyHost)) {
                CFMutableDictionaryRef mDict = CFDictionaryCreateMutableCopy(CFGetAllocator(stream), CFDictionaryGetCount(http->proxyDict), http->proxyDict);
                CFDictionaryRemoveValue(mDict, kCFStreamPropertySOCKSProxyHost);
                CFDictionaryRemoveValue(mDict, kCFStreamPropertySOCKSProxyPort);
                CFRelease(http->proxyDict);
                http->proxyDict = mDict;
            }
        } else if (CFGetTypeID(propertyValue) != CFDictionaryGetTypeID()) {
            return FALSE;
        } else {
            CFDictionaryRef dict = (CFDictionaryRef)propertyValue;
            CFTypeRef keys[NUM_SOCKS_PROPS], values[NUM_SOCKS_PROPS];
            CFIndex numEntries = extractSocksProperties(dict, keys, values);
            if (!http->proxyDict && numEntries) {
                http->proxyDict = CFDictionaryCreate(CFGetAllocator(stream), keys, values, numEntries, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            } else {
                CFMutableDictionaryRef mDict = CFDictionaryCreateMutableCopy(CFGetAllocator(stream), CFDictionaryGetCount(http->proxyDict) + 4, http->proxyDict);
                CFIndex i;
                for (i = 0; i < numEntries; i ++) {
                    CFDictionarySetValue(mDict, keys[i], values[i]);
                }
                CFRelease(http->proxyDict);
                http->proxyDict = mDict;
            }
        }        
        return TRUE;
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPAttemptPersistentConnection)) {
        if (propertyValue == kCFBooleanTrue) {
            if (!isPersistent(http)) __CFBitSet(http->flags, IS_PERSISTENT);
            return TRUE;
        } else if (propertyValue == kCFBooleanFalse) {
            if (isPersistent(http)) __CFBitClear(http->flags, IS_PERSISTENT);
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (CFEqual(propertyName, kCFStreamPropertySocketSecurityLevel) ||
               CFEqual(propertyName, kCFStreamPropertyShouldCloseNativeSocket)) {
        // We own these (socket) properties; prevent the client from setting them
        return FALSE;
    } else if (CFEqual(propertyName, _kCFStreamPropertyHTTPConnectionStreams)) {
        if (CFGetTypeID(propertyValue) != CFArrayGetTypeID() 
            || CFArrayGetCount(propertyValue) != 2 
            || CFGetTypeID(CFArrayGetValueAtIndex(propertyValue, 0)) != CFReadStreamGetTypeID() 
            || CFGetTypeID(CFArrayGetValueAtIndex(propertyValue, 1)) != CFWriteStreamGetTypeID()) {
            return FALSE;
        } else {
            __CFBitSet(http->flags, CUSTOM_STREAMS);
            CFDictionarySetValue(http->connProps, propertyName, propertyValue);
            return TRUE;
        }
    } else {
        if (propertyValue) {
            CFDictionarySetValue(http->connProps, propertyName, propertyValue);
        } else {
            CFDictionaryRemoveValue(http->connProps, propertyName);
        }
        return TRUE;
    }
}

static void httpRequestSchedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestSchedule(req = 0x%x)\n", (int)req);
#endif
    if (_CFHTTPRequestGetState(req) < kFinished) {
        if (req->conn) {
            _CFNetConnectionSchedule(req->conn, req, runLoop, runLoopMode);
        }
        if (req->proxyStream) {
            CFReadStreamScheduleWithRunLoop(req->proxyStream, runLoop, runLoopMode);
        }
    }
}

static void httpRequestUnschedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "httpRequestUnschedule(req = 0x%x)\n", (int)req);
#endif
     if (_CFHTTPRequestGetState(req) < kFinished) {
        if (req->conn) {
            _CFNetConnectionUnschedule(req->conn, req, runLoop, runLoopMode);
        }
        if (req->proxyStream) {
            CFReadStreamUnscheduleFromRunLoop(req->proxyStream, runLoop, runLoopMode);
        }
    }
}

static void requestPayloadCallBack(CFReadStreamRef stream, CFStreamEventType type, void *info) { 
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "requestPayloadCallBack(req = 0x%x, event = %d)\n", (int)req, type);
#endif
    switch (type) {
    case kCFStreamEventEndEncountered:
    case kCFStreamEventHasBytesAvailable:
    {
        CFStreamError err;
        CFWriteStreamRef requestStream = _CFNetConnectionGetRequestStream(req->conn);
        if (requestStream && transmitRequest1(req, requestStream, &err, FALSE)) {
            if (err.error == 0) {
                _CFNetConnectionRequestIsComplete(req->conn, req);
            } else {
                if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
                    __CFBitSet(req->flags, DO_NOT_REATTEMPT);
                }
                _CFNetConnectionErrorOccurred(req->conn, &err); // This should cause us to send our error when the conneciton turns around and orphans us.
            }
        }
        break;
    }
    case kCFStreamEventErrorOccurred: {
        CFStreamError err = CFReadStreamGetError(stream);
        // theoretically, the connection could unwind and recover from this, but that would require a lot more code....
        if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
            __CFBitSet(req->flags, DO_NOT_REATTEMPT);
        }
        _CFNetConnectionErrorOccurred(req->conn, &err);
        break;
    }
    default:
        ;
    }
}

static Boolean hasTokenInList(CFStringRef list, CFStringRef token) {
    CFIndex len = CFStringGetLength(list);
    CFRange rg = {0, len};
    while (rg.location < rg.length && CFStringFindWithOptions(list, token, rg, kCFCompareCaseInsensitive, &rg)) {
        CFIndex loc = rg.location-1;
        UniChar ch = '\0';
        while (loc >= 0 && (ch = CFStringGetCharacterAtIndex(list, loc)) && (ch == ' ' || ch == '\t')) {
            loc --;
        }
        if (loc == 0 || ch == ',') {
            loc = rg.location + rg.length;
            while (loc < len && ch == CFStringGetCharacterAtIndex(list, loc) && (ch == ' ' || ch == '\t')) {
                loc ++;
            }
            if (loc == len || ch == ',') {
                return TRUE;
            }
        }
        rg.location = rg.location + rg.length;
        rg.length = len - rg.location;
    }
    return FALSE;
}

extern Boolean canKeepAlive(CFHTTPMessageRef responseHeaders, CFHTTPMessageRef request) {
    CFStringRef connectionHeader;
    Boolean result;
    if (!responseHeaders) {
        // 0.9 server
        return FALSE;
    } 
    connectionHeader = CFHTTPMessageCopyHeaderFieldValue(responseHeaders, _kCFHTTPStreamProxyConnectionHeader);
	if (!connectionHeader) {
		connectionHeader = CFHTTPMessageCopyHeaderFieldValue(responseHeaders, _kCFHTTPStreamConnectionHeader);
    }
    if (connectionHeader) {
        // According to the HTTP/1.1 spec, this can actually be a comma-delimited list of values, specifying keep-alive or close, then a list of headers that should be removed when propagating the message across a proxy.  But I don't think anyone actually sets it to anything other than "keep-alive" or "close", so we check for those before doing the exhaustive case.
        if (CFStringCompare(connectionHeader, _kCFHTTPStreamConnectionClose, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            result = FALSE;
        } else if (CFStringCompare(connectionHeader, _kCFHTTPStreamConnectionKeepAlive, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            result = TRUE;
        } else if (CFStringFind(connectionHeader, _kCFHTTPStreamConnectionSeparator, 0).location == kCFNotFound) {
            result = FALSE;
        } else {
            // Sigh.  Do the full search.
            if (hasTokenInList(connectionHeader, _kCFHTTPStreamConnectionClose)) {
                result = FALSE;
            } else if (hasTokenInList(connectionHeader, _kCFHTTPStreamConnectionKeepAlive)) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        }
        CFRelease(connectionHeader);
    } else {
        CFStringRef responseVersion = CFHTTPMessageCopyVersion(responseHeaders);
        if (!responseVersion) {
            result = FALSE;
        } else {
            CFStringRef requestVersion = CFHTTPMessageCopyVersion(request);
            if (!requestVersion || CFEqual(responseVersion, kCFHTTPVersion1_0) || CFEqual(requestVersion, kCFHTTPVersion1_0)) {
                result = FALSE;
            } else if (CFEqual(responseVersion, kCFHTTPVersion1_1)) {
                result = TRUE;
            } else {
                int len = CFStringGetLength(responseVersion);
                if (len > 6) {
                    CFStringRef versNum = CFStringCreateWithSubstring(CFGetAllocator(responseVersion), responseVersion, CFRangeMake(5, len-5));
                    double versValue = CFStringGetDoubleValue(versNum);
                    result = (versValue > 1.1);
                    CFRelease(versNum);
                } else {
                    result = FALSE; // Malformed response from the server; better safe than sorry.
                }
            }
            CFRelease(responseVersion);
            if (requestVersion) CFRelease(requestVersion);
        }
    }
    return result;
}

/* Called while holding the lock */
void httpResponseStreamCallBack(void *theReq, CFReadStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, const void*  key) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)theReq;
    Boolean justReadMark = FALSE;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "responseStreamCallBack(req = 0x%x, event = %d)\n", (int)req, type);
#endif
    if (!__CFBitIsSet(req->flags, HAVE_READ_MARK)) {
        justReadMark = TRUE;
        prepareReception1(req, stream);
    }
    switch (type) {
    case kCFStreamEventHasBytesAvailable:
        if (justReadMark) break; // prepareReception just queued another HasBytesAvailable event; wait and field that one instead.
        if (haveCheckedHeaders(req)) {
            if (!__CFBitIsSet(req->flags, IS_ZOMBIE)) {
                _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventHasBytesAvailable, NULL);
            } else {
                // Perform the read ourselves
                UInt8 buf[BUF_SIZE];
                while (CFReadStreamHasBytesAvailable(stream) > 0) {
                    CFReadStreamRead(stream, buf, BUF_SIZE);
                }
            }
        } else {
            CFStreamError err;
            Boolean persistentOK;
            Boolean useThisStream = checkHeaders(req, stream, &err, &persistentOK);
            if (err.error != 0) {
                // Any error would be due to the new headers; at this point, the underlying HTTP stream should remain intact regardless.  Do not error out the connection.
                _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventErrorOccurred, &err);
            } else {
                if (useThisStream) {
                    // If we changed streams, we have nothing to report right now; we wait until the new stream sends us its event messages
                    if (__CFBitIsSet(req->flags, FORCE_EOF)) {
                        // We know from the request/response that there will never be any data
                        _CFNetConnectionResponseIsComplete(req->conn, req);
                    } else if (!__CFBitIsSet(req->flags, IS_ZOMBIE)) {
                        _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventHasBytesAvailable, NULL);
                    } else {
                        // Perform the read ourselves
                        UInt8 buf[BUF_SIZE];
                        while (CFReadStreamHasBytesAvailable(stream) > 0) {
                            CFReadStreamRead(stream, buf, BUF_SIZE);
                        }
                    }
                }
            }
        }
        break;
    case kCFStreamEventMarkEncountered: 
        if (!justReadMark && req->conn) {
            _CFNetConnectionResponseIsComplete(req->conn, req);
        }
        break;
    case kCFStreamEventEndEncountered:
        // This is an error for the connection if we do not own it and if we expected the connection to be persistent; otherwise it's o.k. and just means we're at the end of our response.  Regardless, we just signal EOF for us....
        if (isPersistent(req)) {
            _CFNetConnectionLost(req->conn); // This  will do the right thing if the connection had already been "lost" (marked as not persistent) once
        } 
        if (!__CFBitIsSet(req->flags, IS_ZOMBIE)) {
            _CFReadStreamSignalEventDelayed(req->responseStream, kCFStreamEventEndEncountered, NULL);
        }
        break;
    case kCFStreamEventErrorOccurred: {
        // Error out the current response.  Remove from queue.  Restart from the current request
        CFStreamError err = CFReadStreamGetError(stream);
        if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
            __CFBitSet(req->flags, DO_NOT_REATTEMPT);
        }
        if (!req->peerCertificates) {
            req->peerCertificates = (CFArrayRef)CFReadStreamCopyProperty(stream, kCFStreamPropertySSLPeerCertificates);
        }
        _CFNetConnectionErrorOccurred(req->conn, &err);
        break;
    }
    default:
        ;
    }
}

/* Called while holding the lock */
void httpRequestStreamCallBack(void *info, CFWriteStreamRef stream, CFStreamEventType type, _CFNetConnectionRef conn, const void* key) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)info;
#if defined(LOG_REQUESTS)
    fprintf(stderr, "requestStreamCallBack(req = 0x%x, event = %d)\n", (int)req, type);
#endif
    switch (type) {
    case kCFStreamEventCanAcceptBytes: {
        CFStreamError err;
        if (transmitRequest1(req, stream, &err, FALSE)) {
            if (err.error == 0) {
                _CFNetConnectionRequestIsComplete(req->conn, req);
            } else {
                if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
                    __CFBitSet(req->flags, DO_NOT_REATTEMPT);
                }
                _CFNetConnectionErrorOccurred(req->conn, &err); // This should cause us to send our error when the conneciton turns around and orphans us.
            }
        }
        break;
    }
    case kCFStreamEventErrorOccurred: {
        CFStreamError err = CFWriteStreamGetError(stream);
        if (err.domain != kCFStreamErrorDomainHTTP || err.error != kCFStreamErrorHTTPConnectionLost) {
            __CFBitSet(req->flags, DO_NOT_REATTEMPT);
        }
		if (!req->peerCertificates)
			req->peerCertificates = (CFArrayRef)CFWriteStreamCopyProperty(stream, kCFStreamPropertySSLPeerCertificates);
        _CFNetConnectionErrorOccurred(req->conn, &err);
        break;
    }
    case kCFStreamEventEndEncountered: {
        CFStreamError err = {_kCFStreamErrorDomainNativeSockets, ECONNRESET};
        __CFBitSet(req->flags, DO_NOT_REATTEMPT);
        _CFNetConnectionErrorOccurred(req->conn, &err);
        break;
    }
    default:
        ;
    }
}

static CFArrayRef httpRunLoopArrayForRequest(void *request, _CFNetConnectionRef conn, const void* info) {
    _CFHTTPRequest *req = (_CFHTTPRequest *)request;
    if (!req->responseStream) return NULL;
    return _CFReadStreamGetRunLoopsAndModes(req->responseStream);
}


CF_EXPORT
CFReadStreamRef CFReadStreamCreateForHTTPRequest(CFAllocatorRef alloc, CFHTTPMessageRef request) {
    _CFHTTPRequest httpContext;
    memset(&httpContext, 0, sizeof(_CFHTTPRequest));
    httpContext.originalRequest = request;
    httpContext.flags = 0;
    return CFReadStreamCreate(alloc, (void *)&_CFHTTPQueuedResponseStreamCallBacks, &httpContext);
}

CF_EXPORT
CFReadStreamRef CFReadStreamCreateForStreamedHTTPRequest(CFAllocatorRef alloc, CFHTTPMessageRef requestHeaders, CFReadStreamRef requestBody) {
    _CFHTTPRequest httpContext;
    memset(&httpContext, 0, sizeof(_CFHTTPRequest));
    httpContext.originalRequest = requestHeaders;
    httpContext.requestPayload = requestBody;
    return CFReadStreamCreate(alloc, (void *)&_CFHTTPQueuedResponseStreamCallBacks, &httpContext);
}


CF_EXPORT
void CFHTTPReadStreamSetRedirectsAutomatically(CFReadStreamRef stream, Boolean shouldAutoRedirect) {
    static Boolean warnOnce = FALSE;
    if (!warnOnce) {
        warnOnce = TRUE;
        CFLog(0, CFSTR("Usage of CFHTTPReadStreamSetRedirectsAutomatically is deprecated; call SetProperty(kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue/False) instead"));
    }
    CFReadStreamSetProperty(stream, kCFStreamPropertyHTTPShouldAutoredirect, shouldAutoRedirect ? kCFBooleanTrue : kCFBooleanFalse);
}

CF_EXPORT
void CFHTTPReadStreamSetProxy(CFReadStreamRef stream, CFStringRef proxyHost, CFIndex proxyPort) {
    CFAllocatorRef alloc = CFGetAllocator(stream);
    CFNumberRef num = CFNumberCreate(alloc, kCFNumberCFIndexType, &proxyPort);
    static Boolean warnOnce = FALSE;
    if (!warnOnce) {
        warnOnce = TRUE;
        CFLog(0, CFSTR("Usage of CFHTTPReadStreamSetProxy is deprecated; call SetProperty(kCFStreamPropertyHTTPProxy) instead"));
    }
    if (num) {
        const void *keys[2], *values[2];
        CFDictionaryRef dict;
        keys[0] = kCFStreamPropertyHTTPProxyHost;
        keys[1] = kCFStreamPropertyHTTPProxyPort;
        values[0] = proxyHost;
        values[1] = num;
        dict = CFDictionaryCreate(alloc, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFReadStreamSetProperty(stream, kCFStreamPropertyHTTPProxy, dict);
        CFRelease(dict);
        CFRelease(num);
    }
}

#if defined(__WIN32__)
extern void _CFHTTPStreamCleanup(void) {
    __CFSpinLock(&cacheInitLock);
    if (httpConnectionCache != NULL) {
        releaseConnectionCache(httpConnectionCache);
        httpConnectionCache = NULL;
    }
    __CFSpinUnlock(&cacheInitLock);
}
#endif
