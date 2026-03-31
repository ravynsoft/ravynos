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
/*	CFHTTPMessage.c
	Copyright 1998-2002, Apple, Inc. All rights reserved.
	Responsibility: Becky Willrich
*/
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFPriv.h>
#include <CFNetwork/CFHTTPMessage.h>
#include <CFNetwork/CFHTTPMessagePriv.h>
#include <CFNetwork/CFHTTPStreamPriv.h>
#include "CFNetworkInternal.h"
#include "CFHTTPInternal.h"
#include <CFNetwork/CFHTTPStream.h>

#if defined(__MACH__)
#include <Security/cssm.h>
#include <mach-o/dyld.h>

#include "spnegoBlob.h"
//#include "spnegoDER.h"
#include "spnegoKrb.h"
#endif // __MACH__


/* To do - add in asserts/argument checking */

struct __CFHTTPMessage {
    CFRuntimeBase _cfBase;

    CFStringRef _firstLine; // This is the request line for HTTP requests; the status line for HTTP responses
    CFStringRef _method;
    CFURLRef _url;
    CFMutableDictionaryRef _headers;
    CFMutableArrayRef _headerOrder;
    CFStringRef	_lastKey;	// This is the last key that was parsed in _parseHeadersFromData.
    CFDataRef _data;
	CFHTTPAuthenticationRef _auth;
	CFHTTPAuthenticationRef _proxyAuth;
    UInt32 _flags;
};

/* To do - convert this ot the CFBit family of functions */
// Flag bitfields/masks
#define STATUS_MASK			0x000003FF

// The HTTP spec requires a delimiter of CRLF, but there are some servers out there that Do Evil, so we check
#define DELIMITER_MASK		0x00000C00
#define DELIMITER(flags)	(((flags) & DELIMITER_MASK) >> 10)
#define DELIM_UNKNOWN		0
#define DELIM_CRLF			1
#define DELIM_CR			2
#define DELIM_LF			3

#define IS_RESPONSE			0x00001000
#define HEADERS_COMPLETE	0x00002000
#define MUTABLE_DATA		0x00004000
#define LAX_PARSING			0x00008000

#define IS_GET_METHOD		0x00010000

// table used in message header parsing
struct MessageHeaderMap {
    const char			_header[19];
    int					_length;
};

static const struct MessageHeaderMap kHTTPMessageHeaderMap[] = {
    {"Accept-Ranges",		13},
    {"Cache-Control",		13},
    {"Connection",			10},
    {"Content-Language",	16},
    {"Content-Length",		14},
    {"Content-Location",	16},
    {"Content-Type",		12},
    {"Date",				 4},
    {"Etag",				 4},
    {"Expires",				 7},
    {"Last-Modified",		13},
    {"Location",			 8},
    {"Proxy-Authenticate",	18},
    {"Server",				 6},
    {"Set-Cookie",			10}
};

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPMessageAcceptRangesHeader		CFSTR("Accept-Ranges")
#define _kCFHTTPMessageCacheControlHeader		CFSTR("Cache-Control")
#define _kCFHTTPMessageConnectHeader			CFSTR("Connection")
#define _kCFHTTPMessageContentLanguageHeader	CFSTR("Content-Language")
#define _kCFHTTPMessageContentLengthHeader		CFSTR("Content-Length")
#define _kCFHTTPMessageContentLocationHeader	CFSTR("Content-Location")
#define _kCFHTTPMessageContentTypeHeader		CFSTR("Content-Type")
#define _kCFHTTPMessageDateHeader				CFSTR("Date")
#define _kCFHTTPMessageEtagHeader				CFSTR("Etag")
#define _kCFHTTPMessageExpiresHeader			CFSTR("Expires")
#define _kCFHTTPMessageLastModifiedHeader		CFSTR("Last-Modified")
#define _kCFHTTPMessageLocationHeader			CFSTR("Location")
#define _kCFHTTPMessageProxyAuthenticateHeader	CFSTR("Proxy-Authenticate")
#define _kCFHTTPMessageServerHeader				CFSTR("Server")
#define _kCFHTTPMessageSetCookieHeader			CFSTR("Set-Cookie")
#else
static CONST_STRING_DECL(_kCFHTTPMessageAcceptRangesHeader, "Accept-Ranges")
static CONST_STRING_DECL(_kCFHTTPMessageCacheControlHeader, "Cache-Control")
static CONST_STRING_DECL(_kCFHTTPMessageConnectHeader, "Connection")
static CONST_STRING_DECL(_kCFHTTPMessageContentLanguageHeader, "Content-Language")
static CONST_STRING_DECL(_kCFHTTPMessageContentLengthHeader, "Content-Length")
static CONST_STRING_DECL(_kCFHTTPMessageContentLocationHeader, "Content-Location")
static CONST_STRING_DECL(_kCFHTTPMessageContentTypeHeader, "Content-Type")
static CONST_STRING_DECL(_kCFHTTPMessageDateHeader, "Date")
static CONST_STRING_DECL(_kCFHTTPMessageEtagHeader, "Etag")
static CONST_STRING_DECL(_kCFHTTPMessageExpiresHeader, "Expires")
static CONST_STRING_DECL(_kCFHTTPMessageLastModifiedHeader, "Last-Modified")
static CONST_STRING_DECL(_kCFHTTPMessageLocationHeader, "Location")
static CONST_STRING_DECL(_kCFHTTPMessageProxyAuthenticateHeader, "Proxy-Authenticate")
static CONST_STRING_DECL(_kCFHTTPMessageServerHeader, "Server")
static CONST_STRING_DECL(_kCFHTTPMessageSetCookieHeader, "Set-Cookie")
#endif	/* __CONSTANT_CFSTRINGS__ */

static const CFStringRef kHTTPMessageHeaderMap2[] = {
	_kCFHTTPMessageAcceptRangesHeader,
	_kCFHTTPMessageCacheControlHeader,
	_kCFHTTPMessageConnectHeader,
	_kCFHTTPMessageContentLanguageHeader,
	_kCFHTTPMessageContentLengthHeader,
	_kCFHTTPMessageContentLocationHeader,
	_kCFHTTPMessageContentTypeHeader,
	_kCFHTTPMessageDateHeader,
	_kCFHTTPMessageEtagHeader,
	_kCFHTTPMessageExpiresHeader,
	_kCFHTTPMessageLastModifiedHeader,
	_kCFHTTPMessageLocationHeader,
	_kCFHTTPMessageProxyAuthenticateHeader,
	_kCFHTTPMessageServerHeader,
	_kCFHTTPMessageSetCookieHeader
};

#define kHTTPMessageNumItems (sizeof(kHTTPMessageHeaderMap) / sizeof(kHTTPMessageHeaderMap[0]))

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPMessageDescribeFormat		CFSTR("<CFHTTPMessage 0x%x>{url = %@; %@ = %@}")
#define _kCFHTTPMessageDescribeRequest		CFSTR("request")
#define _kCFHTTPMessageDescribeStatus		CFSTR("status")
#define _kCFHTTPMessageGETMethod			CFSTR("GET")
#define _kCFHTTPMessageResponseLineFormat	CFSTR(" %d ")
#define _kCFHTTPMessageSpace				CFSTR(" ")
#define _kCFHTTPMessageEmptyString			CFSTR("")
#define _kCFHTTPMessageAppendHeaderFormat	CFSTR("%@, %@")
#else
static CONST_STRING_DECL(_kCFHTTPMessageDescribeFormat, "<CFHTTPMessage 0x%x>{url = %@; %@ = %@}")
static CONST_STRING_DECL(_kCFHTTPMessageDescribeRequest, "request")
static CONST_STRING_DECL(_kCFHTTPMessageDescribeStatus, "status")
static CONST_STRING_DECL(_kCFHTTPMessageGETMethod, "GET")
static CONST_STRING_DECL(_kCFHTTPMessageResponseLineFormat, " %d ")
static CONST_STRING_DECL(_kCFHTTPMessageSpace, " ")
static CONST_STRING_DECL(_kCFHTTPMessageEmptyString, "")
static CONST_STRING_DECL(_kCFHTTPMessageAppendHeaderFormat, "%@, %@")
#endif	/* __CONSTANT_CFSTRINGS__ */

static CFStringRef __CFHTTPMessageCopyDescription(CFTypeRef cf) {
    CFHTTPMessageRef msg = (CFHTTPMessageRef)cf;
    return CFStringCreateWithFormat(NULL, NULL, _kCFHTTPMessageDescribeFormat, msg, msg->_url, (msg->_flags & IS_RESPONSE) == 0 ? _kCFHTTPMessageDescribeRequest : _kCFHTTPMessageDescribeStatus, msg->_firstLine);
}

static void __CFHTTPMessageDeallocate(CFTypeRef cf) {
    CFHTTPMessageRef req = (CFHTTPMessageRef)cf;
    CFRelease(req->_headers);
    CFRelease(req->_headerOrder);
    if (req->_firstLine) CFRelease(req->_firstLine);
    if (req->_method) CFRelease(req->_method);
    if (req->_url) CFRelease(req->_url);
    if (req->_data) CFRelease(req->_data);
	if (req->_auth) CFRelease(req->_auth);
	if (req->_proxyAuth) CFRelease(req->_proxyAuth);
    if (req->_lastKey) CFRelease(req->_lastKey);
}

CONST_STRING_DECL(kCFHTTPVersion1_0, "HTTP/1.0")  
CONST_STRING_DECL(kCFHTTPVersion1_1, "HTTP/1.1")
CONST_STRING_DECL(kCFHTTPRedirectionResponse,"kCFHTTPRedirectionResponse")
CONST_STRING_DECL(kCFStreamPropertyHTTPRequest,"kCFStreamPropertyHTTPRequest")
CONST_STRING_DECL(kCFStreamPropertyHTTPResponseHeader,"kCFStreamPropertyHTTPResponseHeader")

CONST_STRING_DECL(_kCFStreamPropertyHTTPPersistent, "_kCFStreamPropertyHTTPPersistent")
CONST_STRING_DECL(_kCFStreamPropertyHTTPNewHeader, "_kCFStreamPropertyHTTPNewHeader")
CONST_STRING_DECL(_kCFStreamPropertyHTTPLaxParsing, "_kCFStreamPropertyHTTPLaxParsing")
CONST_STRING_DECL(kCFStreamPropertyHTTPProxy, "kCFStreamPropertyHTTPProxy")
CONST_STRING_DECL(kCFStreamPropertyHTTPProxyHost, "HTTPProxy")
CONST_STRING_DECL(kCFStreamPropertyHTTPProxyPort, "HTTPPort")
CONST_STRING_DECL(kCFStreamPropertyHTTPSProxyHost, "HTTPSProxy")
CONST_STRING_DECL(kCFStreamPropertyHTTPSProxyPort, "HTTPSPort")
CONST_STRING_DECL(kCFStreamPropertyHTTPShouldAutoredirect, "kCFStreamPropertyHTTPShouldAutoredirect")
CONST_STRING_DECL(kCFStreamPropertyHTTPFinalURL, "kCFStreamPropertyHTTPFinalURL")
CONST_STRING_DECL(kCFStreamPropertyHTTPAttemptPersistentConnection, "kCFStreamPropertyHTTPAttemptPersistentConnection")
CONST_STRING_DECL(kCFStreamPropertyHTTPRequestBytesWrittenCount, "kCFStreamPropertyHTTPRequestBytesWrittenCount")
CONST_STRING_DECL(_kCFStreamPropertyHTTPConnectionStreams, "_kCFStreamPropertyHTTPConnectionStreams")
CONST_STRING_DECL(_kCFStreamPropertyHTTPZeroLengthResponseExpected, "_kCFStreamPropertyHTTPZeroLengthResponseExpected")
CONST_STRING_DECL(_kCFStreamPropertyHTTPProxyProxyAutoConfigURLString, "ProxyAutoConfigURLString")
CONST_STRING_DECL(_kCFStreamPropertyHTTPProxyProxyAutoConfigEnable, "ProxyAutoConfigEnable")
CONST_STRING_DECL(_kCFStreamPropertyHTTPConnection, "_kCFStreamPropertyHTTPConnection")

static _CFOnceLock gHTTPMessageClassRegistration = _CFOnceInitializer;
static CFTypeID __kCFHTTPMessageTypeID = _kCFRuntimeNotATypeID;

static void
_HTTPMessageRegisterClass(void) {
	
	static const CFRuntimeClass __CFHTTPMessageClass = {
		0,
		"CFHTTPMessage",
		NULL,      // init
		NULL,      // copy
		__CFHTTPMessageDeallocate,
		NULL,      // equal
		NULL,      // hash
		NULL,      // 
		__CFHTTPMessageCopyDescription
	};
	
	// On Windows CFSTR is a function call, not compiler supported, so we init this table ourselves
#if defined(__WIN32__)
	// **FIXME** Broken with changes for shrinking DATA section
	int i;
	for (i = 0; i < kHTTPMessageNumItems; i++)
		kHTTPMessageHeaderMap[i]._constant = CFStringCreateWithCStringNoCopy(NULL, kHTTPMessageHeaderMap[i]._header, kCFStringEncodingASCII, kCFAllocatorNull);
#endif

    __kCFHTTPMessageTypeID = _CFRuntimeRegisterClass(&__CFHTTPMessageClass);
}


CFTypeID CFHTTPMessageGetTypeID(void) {
	
    _CFDoOnce(&gHTTPMessageClassRegistration, _HTTPMessageRegisterClass);
    
    return __kCFHTTPMessageTypeID;
}

#if defined(__WIN32__)
extern void _CFHTTPMessageCleanup(void) {
	
	// **FIXME** This is not protected versus the call to register.
	// **FIXME** Broken with changes for shrinking DATA section
	
    if (__kCFHTTPMessageTypeID != _kCFRuntimeNotATypeID) {
        int i;
        for (i = 0; i < kHTTPMessageNumItems; i++) {
            CFRelease(kHTTPMessageHeaderMap[i]._constant);
            kHTTPMessageHeaderMap[i]._constant = NULL;
        }
    }
}
#endif

// Caller's repsonsibility to properly initialize _flags
static CFHTTPMessageRef _CFHTTPMessageCreate(CFAllocatorRef allocator) {
    struct __CFHTTPMessage *newMsg;
    newMsg = (struct __CFHTTPMessage *)_CFRuntimeCreateInstance(allocator, CFHTTPMessageGetTypeID(), sizeof(struct __CFHTTPMessage) - sizeof(CFRuntimeBase), NULL);
    if (newMsg) {
        // Initialize fields other than _cfBase here.
        newMsg->_firstLine = NULL;
        newMsg->_method = NULL;
        newMsg->_url = NULL;
        newMsg->_headers = CFDictionaryCreateMutable(allocator, 17, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        newMsg->_headerOrder = CFArrayCreateMutable(allocator, 17, &kCFTypeArrayCallBacks);
        newMsg->_lastKey = NULL;
        newMsg->_data = NULL;
        newMsg->_auth = NULL;
        newMsg->_proxyAuth = NULL;
        newMsg->_flags = LAX_PARSING; // Turn on lax parsing by default.
    }
    return newMsg;
}

CFHTTPMessageRef CFHTTPMessageCreateCopy(CFAllocatorRef allocator, CFHTTPMessageRef msg) {
    struct __CFHTTPMessage *result;
    result = (struct __CFHTTPMessage *)_CFRuntimeCreateInstance(allocator, CFHTTPMessageGetTypeID(), sizeof(struct __CFHTTPMessage) - sizeof(CFRuntimeBase), NULL);
    if (result) {
        result->_firstLine = msg->_firstLine ? CFStringCreateCopy(allocator, msg->_firstLine) : NULL;
        result->_method = msg->_method ? CFRetain(msg->_method) : NULL;
        result->_url = msg->_url ? CFRetain(msg->_url) : NULL;
        result->_headers = CFDictionaryCreateMutableCopy(allocator, (CFDictionaryGetCount(msg->_headers) >= 16 ? 0 : 16), msg->_headers);
        result->_headerOrder = CFArrayCreateMutableCopy(allocator, (CFArrayGetCount(msg->_headerOrder) >= 16 ? 0 : 16), msg->_headerOrder);
        result->_flags = msg->_flags;
        result->_lastKey = msg->_lastKey ? CFRetain(msg->_lastKey) : NULL;
        if (msg->_data == NULL) {
            result->_data = NULL;
        } else if ((msg->_flags & MUTABLE_DATA) == 0) {
            result->_data = CFRetain(msg->_data);
        } else {
            result->_data = CFDataCreateMutableCopy(allocator, 0, msg->_data);
        }
        result->_auth = msg->_auth;
        result->_proxyAuth = msg->_proxyAuth;
        if (result->_auth)
            CFRetain(msg->_auth);
        if (result->_proxyAuth)
            CFRetain(msg->_proxyAuth);
    }
    return result;
}

UInt8 *_CFURLPortionForRequest(CFAllocatorRef alloc, CFURLRef url, Boolean useCompleteURL, UInt8 **buf, CFIndex bufLength, Boolean *deallocateBuffer) {
    CFURLRef absURL = CFURLCopyAbsoluteURL(url);
    UInt8 *urlBytes;
    CFIndex length;
    CFRange urlRg;
    CFRange fragRg;
    Boolean prependedSlash = FALSE;

    *deallocateBuffer = FALSE;
    // Reserve one byte at the beginning in case we must prepend a slash, and one byte at the end for null-termination
    length = CFURLGetBytes(absURL, (*buf) + 1, bufLength-2);
    if (length == -1) {
        *deallocateBuffer = TRUE;
        length = CFURLGetBytes(absURL, NULL, 0);
        *buf = CFAllocatorAllocate(alloc, length+2, 0);
        CFURLGetBytes(absURL, (*buf)+1, length);
    }
    urlBytes = (*buf)+1;
    
    if (!useCompleteURL) {
        // First byte is the byte of the path....
        CFRange pathWithSeparators;
        urlRg = CFURLGetByteRangeForComponent(absURL, kCFURLComponentPath, &pathWithSeparators);
        if (!urlRg.location == kCFNotFound || urlRg.length == 0) {
            *(urlBytes + pathWithSeparators.location - 1) = '/';
            prependedSlash = TRUE;
            urlRg = pathWithSeparators;
        }
    } else {
        urlRg.location = 0;
    }
    
    // Less the fragment. 3022146. - REW
    fragRg = CFURLGetByteRangeForComponent(absURL, kCFURLComponentFragment, NULL);
    if (fragRg.location == -1) {
        urlRg.length = length - urlRg.location;
    } else {
        urlRg.length = fragRg.location - 1 - urlRg.location;
    }

    *(urlBytes+urlRg.location+urlRg.length) = '\0';
    CFRelease(absURL);
    return prependedSlash ? urlBytes + urlRg.location - 1 : urlBytes + urlRg.location;
}

static CFStringRef createRequestLine(CFAllocatorRef alloc, CFStringRef method, CFURLRef url, CFStringRef version, Boolean useCompleteURL) {
    CFMutableStringRef line;
    UInt8 buf[512], *urlBytes = buf;
    UInt8 *urlPortion;
    Boolean freeBytes = FALSE;
    
    line = CFStringCreateMutableCopy(alloc, 0, method);
    CFStringAppendCString(line, " ", kCFStringEncodingASCII);

    urlPortion = _CFURLPortionForRequest(alloc, url, useCompleteURL, &urlBytes, sizeof(buf)/sizeof(UInt8), &freeBytes);
    CFStringAppendCString(line, (const char*)urlPortion, kCFStringEncodingISOLatin1);
    if (freeBytes) CFAllocatorDeallocate(alloc, urlBytes);

    CFStringAppendCString(line, " ", kCFStringEncodingASCII);
    CFStringAppend(line, version);
    return line;
}

CFHTTPMessageRef CFHTTPMessageCreateRequest(CFAllocatorRef allocator, CFStringRef requestMethod, CFURLRef url, CFStringRef httpVersion) {
    CFHTTPMessageRef newReq;
    if (!requestMethod || !url) {
        return NULL;
    }

    newReq = _CFHTTPMessageCreate(allocator);
    if (newReq) {
        newReq->_firstLine = createRequestLine(allocator, requestMethod, url, httpVersion, FALSE);
        newReq->_method = CFStringCreateCopy(allocator, requestMethod);
		if (CFStringCompare(requestMethod, _kCFHTTPMessageGETMethod, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			newReq->_flags |= IS_GET_METHOD;
        CFRetain(url);
        newReq->_url = url;
    }
    return newReq;
}

CF_INLINE const char *descriptionForResponseCode(int code) {
    switch (code) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested range not satisfiable";
    case 417: return "Expectation Failed";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Time-out";
    case 505: return "HTTP Version not supported";
    default:
        if (code < 200) {
            return "Continue";
        } else if (code < 300) {
            return "OK";
        } else if (code < 400) {
            return "Multiple Choices";
        } else if (code < 500) {
            return "Bad Request";
        } else {
            return "Internal Server Error";
        }
    }
}
static CFStringRef createResponseLine(CFAllocatorRef alloc, int code, CFStringRef desc, CFStringRef version) {
    CFMutableStringRef line = CFStringCreateMutableCopy(alloc, 0, version);
    CFStringAppendFormat(line, NULL,  _kCFHTTPMessageResponseLineFormat, code);
    if (desc) {
        CFStringAppend(line, desc);
    } else {
        CFStringAppendCString(line, descriptionForResponseCode(code), kCFStringEncodingASCII);
    }
    return line;
}

CFHTTPMessageRef CFHTTPMessageCreateResponse(CFAllocatorRef allocator, int statusCode, CFStringRef statusDescription, CFStringRef httpVersion) {
    CFHTTPMessageRef newResponse;

    newResponse = _CFHTTPMessageCreate(allocator);
    if (newResponse) {
        newResponse->_flags |= IS_RESPONSE;
        // record the status code, masking out any illegal bits in case the caller passed a status code too large
        newResponse->_flags = (newResponse->_flags & ~STATUS_MASK) | (statusCode & STATUS_MASK);	
        newResponse->_firstLine = createResponseLine(allocator, statusCode, statusDescription, httpVersion);
    }
    return newResponse;
}

CFHTTPMessageRef CFHTTPMessageCreateEmpty(CFAllocatorRef allocator, Boolean isRequest) {
    CFHTTPMessageRef newMsg = _CFHTTPMessageCreate(allocator);
    if (newMsg && !isRequest) {
        newMsg->_flags |= IS_RESPONSE;
    }
    return newMsg;
}

extern void _CFHTTPMessageSetLaxParsing(CFHTTPMessageRef msg, Boolean allowLaxParsing) {
    if (allowLaxParsing) {
        msg->_flags |= LAX_PARSING;
    } else {
        msg->_flags &= (~LAX_PARSING);
    }
}

Boolean CFHTTPMessageIsRequest(CFHTTPMessageRef message) {
    return (message->_flags & IS_RESPONSE) ? FALSE : TRUE;
}


/* extern */
CFHTTPAuthenticationRef _CFHTTPMessageGetAuthentication(CFHTTPMessageRef message, Boolean proxy) {
	
    return proxy ? message->_proxyAuth : message->_auth;
}


void _CFHTTPMessageSetAuthentication(CFHTTPMessageRef message, CFHTTPAuthenticationRef auth, Boolean proxy) {
	
	CFHTTPAuthenticationRef* ptr = proxy ? &(message->_proxyAuth) : &(message->_auth);
	
    CFRetain(auth);
	
	if (*ptr)
		CFRelease(*ptr);
	
	*ptr = auth;
}


CFStringRef CFHTTPMessageCopyVersion(CFHTTPMessageRef message) {
    CFRange rg = {0, 0};
    if (!message->_firstLine) {
        return NULL;
    }
    if (message->_flags & IS_RESPONSE) {
        if (CFStringFindWithOptions(message->_firstLine, _kCFHTTPMessageSpace, CFRangeMake(0, CFStringGetLength(message->_firstLine)), 0, &rg)) {
            rg.length = rg.location;
            rg.location = 0;
        } else {
            rg.location = 0;
            rg.length = 0;
        }
    } else {
        if (CFStringFindWithOptions(message->_firstLine, _kCFHTTPMessageSpace, CFRangeMake(0, CFStringGetLength(message->_firstLine)), kCFCompareBackwards, &rg)) {
            rg.location ++;
            rg.length = CFStringGetLength(message->_firstLine) - rg.location;
        } else {
            rg.location = 0;
            rg.length = 0;
        }
    }
    if (rg.length > 0) {
        return CFStringCreateWithSubstring(CFGetAllocator(message), message->_firstLine, rg);
    } else {
        return NULL;
    }
}

CFDataRef CFHTTPMessageCopyBody(CFHTTPMessageRef msg) {
    if (msg->_data) {
        if ((msg->_flags & MUTABLE_DATA) == 0) {
            CFRetain(msg->_data);
            return msg->_data;
        } else {
            return CFDataCreateCopy(CFGetAllocator(msg), msg->_data);
        }
    } else {
        return NULL;
    }
}

void CFHTTPMessageSetBody(CFHTTPMessageRef msg, CFDataRef data) {
    msg->_flags &= (~MUTABLE_DATA);
    if (data)  {
        data = CFDataCreateCopy(CFGetAllocator(msg), data);
    }
    if (msg->_data) {
        CFRelease(msg->_data);
    }
    msg->_data = data;
}

CFStringRef _CFCapitalizeHeader(CFStringRef headerString) {
    CFIndex i, len = CFStringGetLength(headerString);
    CFAllocatorRef alloc = CFGetAllocator(headerString);
    UInt8 *charPtr = NULL;
    UniChar *uniCharPtr = NULL;
    Boolean useUniCharPtr = FALSE;
    Boolean shouldCapitalize = TRUE;
    Boolean somethingChanged = FALSE;
    
    for (i = 0; i < len; i ++) {
        UniChar ch = CFStringGetCharacterAtIndex(headerString, i);
        Boolean replace = FALSE;
        if (shouldCapitalize && ch >= 'a' && ch <= 'z') {
            ch = ch + 'A' - 'a';
            replace = TRUE;
        } else if (!shouldCapitalize && ch >= 'A' && ch <= 'Z') {
            ch = ch + 'a' - 'A';
            replace = TRUE;
        }
        if (replace) {
            if (!somethingChanged) {
                
				CFIndex converted = 0;
				
				somethingChanged = TRUE;
				
				charPtr = _CFStringGetOrCreateCString(alloc, headerString, NULL, &converted, kCFStringEncodingISOLatin1);
				
                if (converted == len) {
                    // Can be encoded in ISOLatin1
                    useUniCharPtr = FALSE;
                } else {
                    useUniCharPtr = TRUE;
					
					CFAllocatorDeallocate(alloc, charPtr);
					
					uniCharPtr = (UniChar*)_CFStringGetOrCreateCString(alloc, headerString, NULL, &converted, kCFStringEncodingUnicode);
                }
            }
            if (useUniCharPtr) {
                uniCharPtr[i] = ch;
            } else {
                charPtr[i] = ch;
            }
        }
        if (ch == '-') {
            shouldCapitalize = TRUE;
        } else {
            shouldCapitalize = FALSE;
        }
    }
    if (somethingChanged) {
        if (useUniCharPtr) {
            return CFStringCreateWithCharactersNoCopy(alloc, uniCharPtr, len, alloc);
        } else {
            return CFStringCreateWithCStringNoCopy(alloc, (const char*)charPtr, kCFStringEncodingISOLatin1, alloc);
        }
    } else {
        CFRetain(headerString);
        return headerString;
    }
}

CFStringRef CFHTTPMessageCopyHeaderFieldValue(CFHTTPMessageRef msg, CFStringRef header) {
    CFStringRef lowerHeader = _CFCapitalizeHeader(header);
    CFStringRef result = CFDictionaryGetValue(msg->_headers, lowerHeader);
    CFRelease(lowerHeader);
    if (result) CFRetain(result);
    return result;
}

CFDictionaryRef CFHTTPMessageCopyAllHeaderFields(CFHTTPMessageRef msg) {
    CFRetain(msg->_headers);
    return msg->_headers;
}

extern void _CFHTTPMessageSetHeader(CFHTTPMessageRef msg, CFStringRef header, CFStringRef value, CFIndex position) {

    if (!value) {
        CFDictionaryRemoveValue(msg->_headers, header);
        CFArrayRemoveValueAtIndex(msg->_headerOrder,
                                  CFArrayGetFirstIndexOfValue(msg->_headerOrder, CFRangeMake(0, CFArrayGetCount(msg->_headerOrder)),
                                  header));
    }
    else {
        if (!CFDictionaryContainsKey(msg->_headers, header)) {
            
            CFIndex count = CFArrayGetCount(msg->_headerOrder);
            CFRange rg = CFRangeMake(((position >= 0) && (position < count)) ? position : count, 0);
            
            if (count == 16) {
                CFTypeRef temp = CFArrayCreateMutableCopy(CFGetAllocator(msg), 0, msg->_headerOrder);
                CFRelease(msg->_headerOrder);
                msg->_headerOrder = (CFMutableArrayRef)temp;
                temp = CFDictionaryCreateMutableCopy(CFGetAllocator(msg), 0, msg->_headers);
                CFRelease(msg->_headers);
                msg->_headers = (CFMutableDictionaryRef)temp;
            }
                
            
            CFArrayReplaceValues(msg->_headerOrder, rg, (const void**)(&header), 1);
        }
        
        CFDictionarySetValue(msg->_headers, header, value);
    }
}

void CFHTTPMessageSetHeaderFieldValue(CFHTTPMessageRef message, CFStringRef headerField, CFStringRef value) {
    CFStringRef header = _CFCapitalizeHeader(headerField);
    _CFHTTPMessageSetHeader(message, header, value, -1);
    CFRelease(header);
}

extern CFDataRef _CFHTTPMessageCopySerializedHeaders(CFHTTPMessageRef msg, Boolean forProxy);
extern CFDataRef _CFHTTPMessageCopySerializedHeaders(CFHTTPMessageRef msg, Boolean forProxy) {
    CFAllocatorRef allocator = CFGetAllocator(msg);
    CFMutableStringRef headers;
    CFDataRef result;
    unsigned i,c;
    
    if ((msg->_flags & IS_RESPONSE) != 0 || !forProxy) {
        headers = CFStringCreateMutableCopy(allocator, 0, msg->_firstLine);
    } else {
        CFStringRef method = CFHTTPMessageCopyRequestMethod(msg);
        CFStringRef version = CFHTTPMessageCopyVersion(msg);
        CFStringRef line = createRequestLine(allocator, method, msg->_url, version, forProxy);
        CFRelease(method);
        CFRelease(version);
        headers = CFStringCreateMutableCopy(allocator, 0, line);
        CFRelease(line);
    }
    CFStringAppendCString(headers, "\r\n", kCFStringEncodingASCII);
    for (i = 0, c = CFArrayGetCount(msg->_headerOrder); i < c; i ++) {
        CFStringRef header = CFArrayGetValueAtIndex(msg->_headerOrder, i);
        CFStringAppend(headers, header);
        CFStringAppendCString(headers, ": ", kCFStringEncodingASCII);
        CFStringAppend(headers, CFDictionaryGetValue(msg->_headers, header));
        CFStringAppendCString(headers, "\r\n", kCFStringEncodingASCII);
    }
    CFStringAppendCString(headers, "\r\n", kCFStringEncodingASCII);
    result = CFStringCreateExternalRepresentation(allocator, headers, kCFStringEncodingISOLatin1, '?');
    CFRelease(headers);
    return result;
}

extern CFDataRef _CFHTTPMessageCopySerializedMessage(CFHTTPMessageRef msg, Boolean forProxy);
extern CFDataRef _CFHTTPMessageCopySerializedMessage(CFHTTPMessageRef msg, Boolean forProxy) {
    CFDataRef result = _CFHTTPMessageCopySerializedHeaders(msg, forProxy);
    if (msg->_data) {
        CFMutableDataRef hdrData = CFDataCreateMutableCopy(CFGetAllocator(msg), CFDataGetLength(msg->_data) + CFDataGetLength(result), result);
        CFRelease(result);
        CFDataAppendBytes(hdrData, CFDataGetBytePtr(msg->_data), CFDataGetLength(msg->_data));
        result = hdrData;
    }
    return result;
}

CFDataRef CFHTTPMessageCopySerializedMessage(CFHTTPMessageRef msg) {
    return _CFHTTPMessageCopySerializedMessage(msg, FALSE);
}

Boolean CFHTTPMessageIsHeaderComplete(CFHTTPMessageRef message) {
    return (message->_flags & HEADERS_COMPLETE) == 0 ? FALSE : TRUE;
}



/*********************/
/* Request functions */
/*********************/

/* extern */ Boolean _CFHTTPMessageIsGetMethod(CFHTTPMessageRef msg) {
	
	if (!msg->_method) {
		
		CFStringRef method = CFHTTPMessageCopyRequestMethod(msg);
		if (method)
			CFRelease(method);
	}
	
	return (msg->_flags & IS_GET_METHOD) ? TRUE : FALSE;
}


CFStringRef CFHTTPMessageCopyRequestMethod(CFHTTPMessageRef request) {
//    __CFGenericValidateType(request, CFHTTPMessageGetTypeID());
//    CFAssert2(((request->_flags & IS_RESPONSE) == 0), __kCFLogAssertion, "%s(): message 0x%x is an HTTP response, not a request", __PRETTY_FUNCTION__, request);
    
    if (!request->_method) {
        CFRange rg;
        if (request->_firstLine && CFStringFindWithOptions(request->_firstLine, _kCFHTTPMessageSpace, CFRangeMake(0, CFStringGetLength(request->_firstLine)), 0, &rg)) {
            rg.length = rg.location;
            rg.location = 0;
            request->_method = CFStringCreateWithSubstring(CFGetAllocator(request), request->_firstLine, rg);
			if (request->_method && (CFStringCompare(request->_method, _kCFHTTPMessageGETMethod, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
				request->_flags |= IS_GET_METHOD;
        }
    }
    if (request->_method)
        return CFRetain(request->_method);
        
    return NULL;
}

CFURLRef CFHTTPMessageCopyRequestURL(CFHTTPMessageRef request) {
//    __CFGenericValidateType(request, CFHTTPMessageGetTypeID());
//    CFAssert2(((request->_flags & IS_RESPONSE) == 0), __kCFLogAssertion, "%s(): message 0x%x is an HTTP response, not a request", __PRETTY_FUNCTION__, request);
    CFRetain(request->_url);
    return request->_url;
}

/**********************/
/* Response functions */
/**********************/

extern
void _CFHTTPMessageSetResponseURL(CFHTTPMessageRef response, CFURLRef url) {
    CFRetain(url);
    if (response->_url) CFRelease(response->_url);
    response->_url = url;
}

// Assert if response is a request, not a response.  Return -1 if we haven't parsed a response code yet
UInt32 CFHTTPMessageGetResponseStatusCode(CFHTTPMessageRef response) {
//    __CFGenericValidateType(response, CFHTTPMessageGetTypeID());
//    CFAssert2(((response->_flags & IS_RESPONSE) != 0), __kCFLogAssertion, "%s(): message 0x%x is an HTTP request, not a response", __PRETTY_FUNCTION__, response);

    if (!response->_firstLine) {
        // Haven't paresd out the status line yet
        return -1;
    } else if (CFStringGetLength(response->_firstLine) == 0) {
        // We got a simple response - no headers.  We fake a status response of 200 (OK), since we are receiving data....
        return 200;
    } else {
        return (response->_flags & STATUS_MASK);
    }
}

CFStringRef CFHTTPMessageCopyResponseStatusLine(CFHTTPMessageRef response) {
    if (response->_firstLine) {
        CFRetain(response->_firstLine);
    }
    return response->_firstLine;
}

static const UInt8 *parseHTTPVersion(const UInt8 *bytes, CFIndex len, Boolean consumeSpaces) {
    Boolean sawDecimal = FALSE, sawOneDigit = FALSE;
    const UInt8 *currentByte, *lastByte = bytes + len;
    if (len < 8) {
        // Yes, we could do some checking here, but instead we choose to wait until we have at least 8 bytes to look at
        // However, we want to at least catch very small 0.9 responses (which don't have a header)
        if (len > 0 && bytes[0] != 'H') return NULL;
        return bytes;
    } else if (!(bytes[0] == 'H' && bytes[1] == 'T' && bytes[2] == 'T' && bytes[3] == 'P' &&  bytes[4] == '/')) {
        // Don't have the prefix "HTTP/"
        return NULL;
    } 
    for (currentByte = bytes+5; currentByte < lastByte; currentByte ++) {
        UInt8 ch = *currentByte;
        if (ch <= '9' && ch >= '0') {
            sawOneDigit = TRUE;
        } else if (ch == '.') {
            if (sawDecimal)  {
                return sawOneDigit ? currentByte : NULL;
            } else {
                sawDecimal = TRUE;
                sawOneDigit = FALSE;
            }
        } else {
            if (sawDecimal && sawOneDigit) {
                if (consumeSpaces) {
                    while (currentByte < lastByte && *currentByte == ' ') {
                        currentByte ++;
                    }
                }
                return currentByte;
            } else {
                return NULL;
            }
        }
    }
    return bytes;
}

/*
 Caller expects:
   _firstLine will be left NULL if we can't determine whether we have a valid header or not (in practice, means we have less than the first line available). _flags should be left unmodified.
   _firstLine to be set to CFSTR("") and the HEADERS_COMPLETE bit to be set if there is no HTTP header
   _firstline to be set to the full first (status) line if a valid HTTP header is found.  _flags must be updated with the status code, and the line delimiter.
 Return value should == bytes if no status line was found; otherwise it should point to the first byte beyond the status line extracted
*/
static const UInt8 *_extractResponseStatusLine(CFHTTPMessageRef response, const UInt8 *bytes, CFIndex len) {
    const UInt8 *currentByte = parseHTTPVersion(bytes, len, TRUE);
    const UInt8 *end = bytes + len;
    if (currentByte == bytes || currentByte + 3 >= end) { // insufficient bytes; we want 3 characters to be able grab the staus code
        return bytes;
    } else if (currentByte == NULL || *currentByte > '9' || *currentByte < '0' || currentByte[1] > '9' || currentByte[1] < '0' || currentByte[2] > '9' || currentByte[2] < '0') {
        // Something in the first bytes doesn't match the expected HTTP header.  Assume that we're receiving a header-less response
        response->_firstLine = CFRetain(_kCFHTTPMessageEmptyString);
        response->_flags |= HEADERS_COMPLETE;
        return bytes;
    } else {
        // O.k.; we've got a good HTTP header
        UInt32 delim = DELIM_UNKNOWN;
        UInt32 status = (currentByte[0] - '0')*100 + (currentByte[1] - '0')*10 + (currentByte[2] - '0');
        currentByte += 3;
        while (currentByte < end) {
            if (*currentByte == '\n' || *currentByte == '\r') {
                break;
            }
            currentByte ++;
        }
        if (currentByte < end) {
            if (*currentByte == '\n') {
                delim = DELIM_LF;
            } else if (currentByte+1 < end) {
                delim = (*(currentByte+1) == '\n') ? DELIM_CRLF : DELIM_CR;
            }
            // If neither of the clauses above is triggered, we need one more byte before we can figure this out.  Fall through and return the response unchanged, and we will try again when next new bytes arrive
        }
        if (delim == DELIM_UNKNOWN) {
            // Never found an EOL
            return bytes;
        } else {
            // Status code is in bytes 10 - 12
            response->_firstLine = CFStringCreateWithBytes(CFGetAllocator(response), bytes, currentByte - bytes, kCFStringEncodingISOLatin1, FALSE);
            response->_flags = (response->_flags & ~DELIMITER_MASK) | (delim << 10);
            response->_flags = (response->_flags & ~STATUS_MASK) | status;
            return  (delim == DELIM_CRLF) ? currentByte + 2 : currentByte + 1;
        }
    }
}

static const UInt8 *_extractRequestFirstLine(CFHTTPMessageRef request, const UInt8 *bytes, CFIndex len) {
    const UInt8 *end = bytes + len;
    const UInt8 *current;
    const UInt8 *methodEnd = NULL, *urlEnd = NULL, *versionEnd = NULL;
    Boolean fail = FALSE, incomplete = FALSE;
    UInt32 delim = DELIM_CRLF;
    
    // Look for the method
    for (current = bytes; current < end; current ++) {
        UInt8 ch = *current;
        if (ch > 126) break;
        if (ch < 32) break;
        if (ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@' || ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '\"' || ch == '/' || ch == '[' || ch == ']' || ch == '?' || ch == '=' || ch == '{' || ch == '}' ) break;
        if (ch == ' ') {
            methodEnd = current;
            current ++;
            break;
        }
    }
    if (methodEnd == NULL) {
        if (current == end) {  // Not enough bytes
            return bytes;
        } else {
            return NULL; // Formatting error
        }
    }

    while (current < end) {
        if (*current == ' ') break;
        current ++;
    }

    if (current < end) {
        urlEnd = current;
        current ++;
        request->_url = CFURLCreateWithBytes(CFGetAllocator(request), methodEnd+1, urlEnd - methodEnd - 1, kCFStringEncodingUTF8, NULL);
        if (!request->_url) {
            // parse error
            return NULL; 
        }
    } else {
        return bytes;
    }
    
    versionEnd = parseHTTPVersion(current, end-current, FALSE);
    if (!versionEnd) { // parse error
        fail = TRUE;
    } else if (versionEnd == current || versionEnd == end) { // insufficient bytes
        incomplete = TRUE;
    } else if (*versionEnd != '\r' && *versionEnd != '\n') {
        fail = TRUE;
    } else {
        if (*versionEnd == '\n') {
            delim = DELIM_LF;
        } else if (versionEnd < end) {
            if (versionEnd[1] == '\n') {
                delim = DELIM_CRLF;
            } else {
                delim = DELIM_CR;
            }
        } else {
            // Need just one more byte
            incomplete = TRUE;
        }
    }    
    if (fail || incomplete) {
        CFRelease(request->_url);
        request->_url = NULL;
        return fail ? NULL : bytes;
    } else {
        request->_firstLine = CFStringCreateWithBytes(CFGetAllocator(request), bytes, versionEnd - bytes, kCFStringEncodingISOLatin1, FALSE);
        request->_flags = (request->_flags & ~DELIMITER_MASK) | (delim << 10);
        return  (delim == DELIM_CRLF) ? versionEnd + 2 : versionEnd + 1;
    }
}

static inline const UInt8 *_findEOL(CFHTTPMessageRef response, const UInt8 *bytes, CFIndex len) {
    
    // According to the HTTP specification EOL is defined as
    // a CRLF pair.  Unfortunately, some servers will use LF
    // instead.  Worse yet, some servers will use a combination
    // of both (e.g. <headers>CRLFLF<body>), so findEOL needs
    // to be more forgiving.  It will now accept CRLF, LF, or
    // CR.
    //
    // It returns NULL if EOL is not found or it will return
    // a pointer to the first terminating character.
    
    const UInt8* result = memchr(bytes, '\n', len);
    if (!result)
        result = memchr(bytes, '\r', len - 1);  // NOTE (len - 1) in order to prevent spanning CRLF.
    
    return result;
}


// The data to be parsed is sitting in message->_data
static Boolean _parseHeadersFromData(CFHTTPMessageRef message) {

    Boolean result = TRUE;
    CFAllocatorRef alloc = CFGetAllocator(message);
    const UInt8* start = CFDataGetBytePtr(message->_data);
    const UInt8* end = start + CFDataGetLength(message->_data);

    if (!message->_firstLine) {
        
        const UInt8* newStart;
        
        // NOTE this is not using CFHTTPMessageIsRequest in order
        // to avoid the function dispatch.
        if (message->_flags & IS_RESPONSE)
            newStart = _extractResponseStatusLine(message, start, end - start);
        else
            newStart = _extractRequestFirstLine(message, start, end - start);
            
        if (newStart == start)
            return TRUE;
            
        if (!newStart)
            return FALSE;
            
        start = newStart;
    }
    
    while ((start != end) && !(message->_flags & HEADERS_COMPLETE)) {
        
        UInt8 c;
        const UInt8* eov;	// End of value?
        const UInt8* eol = _findEOL(message, start, end - start);
        
        if (!eol)
            break;
        
        // Make end-of-value point to the character just before
        // the first eol marker.
        eov = eol - 1;
        if ((*eov == '\r') && (*eol == '\n'))
            eov--;
        
        // Check if it's the empty line between head and body
        if (start >= eov) {
            start = eol + 1;
            message->_flags |= HEADERS_COMPLETE;
            if (message->_lastKey) {
                CFRelease(message->_lastKey);
                message->_lastKey = NULL;
            }
            break;
        }
        
        c = *start;
        
        // Check for continuation header
        if ((c == ' ') || (c == '\t')) {
            
            if (!message->_lastKey) {
                if (!(message->_flags & LAX_PARSING)) {
                    result = FALSE;
                    break;
                }
            } 
            else {
                CFMutableStringRef value = CFStringCreateMutableCopy(alloc, 0, CFDictionaryGetValue(message->_headers, message->_lastKey));
                CFStringRef str = CFStringCreateWithBytes(alloc, start, eov - start + 1, kCFStringEncodingISOLatin1, FALSE);
                CFStringAppend(value, str);
                CFRelease(str);
                CFHTTPMessageSetHeaderFieldValue(message, message->_lastKey, value);
                CFRelease(value);
            }
        }
        
        // It's a new header
        else {
        
            const UInt8* colon = memchr(start, ':', eol - start);
            
            if (!colon) {
                // Bad header; check to see if it's the IIS/eBay bug (second status
                // line being sent) before declaring it a parse error - 3140081

                // Expand to greater leniency - silently skip over any malformed line if LAX_PARSING is on.
                // Old constraint was:
                // if (CFHTTPMessageIsRequest(message) || !(message->_flags & LAX_PARSING) || !isResponseStatusLine(start, eol - start))
                if (!(message->_flags & LAX_PARSING))
                {
                    result = FALSE;
                    break;
                }
            }
            
            else {
                
                CFStringRef key = NULL;
                CFStringRef value, old;
                int i;
                
                for (i = 0; i < kHTTPMessageNumItems; i++) {
                    const struct MessageHeaderMap* map = &kHTTPMessageHeaderMap[i];
                    if (((colon - start) == map->_length) &&
                        !strncmp(map->_header, (const char*)start, map->_length))
                    {
                        key = CFRetain(kHTTPMessageHeaderMap2[i]);
                        break;
                    }
                }
                
                if (!key) {
                    CFStringRef temp;
                    key = CFStringCreateWithBytes(alloc, start, colon - start, kCFStringEncodingISOLatin1, FALSE);
                    temp = _CFCapitalizeHeader(key);
                    CFRelease(key);
                    key = temp;
                }
                
                if (message->_lastKey)
                    CFRelease(message->_lastKey);
                message->_lastKey = CFRetain(key);
                
                c = *++colon;
                while ((c == ' ') || (c == '\t'))
                    c = *++colon;
                    
                if (colon > eov)
                    value = CFRetain(_kCFHTTPMessageEmptyString);
                else
                    value = CFStringCreateWithBytes(alloc, colon, eov - colon + 1, kCFStringEncodingISOLatin1, FALSE);
                    
                old = CFDictionaryGetValue(message->_headers, key);
                if (old) {
                    CFStringRef newValue = CFStringCreateWithFormat(alloc, NULL, _kCFHTTPMessageAppendHeaderFormat, old, value);
                    CFRelease(value);
                    value = newValue;
                }
                
                _CFHTTPMessageSetHeader(message, key, value, -1);
                
                CFRelease(key);
                CFRelease(value);
            }
        }
            
        start = eol + 1;
    }
    
    if (start != CFDataGetBytePtr(message->_data)) {
    
        if (message->_flags & MUTABLE_DATA) {
            CFDataReplaceBytes((CFMutableDataRef)message->_data, CFRangeMake(0, start - CFDataGetBytePtr(message->_data)), NULL, 0);
        }
        else {
            message->_flags &= (~MUTABLE_DATA);
            CFRelease(message->_data);
            message->_data = CFDataCreate(alloc, start, end - start);
        }
    }

    return result;
}

Boolean CFHTTPMessageAppendBytes(CFHTTPMessageRef message, const UInt8 *newBytes, CFIndex numBytes) {
//    __CFGenericValidateType(response, CFHTTPMessageGetTypeID());
    if (numBytes == 0) {
        return TRUE;
    }

    // First append the data, then see if we have more header parsing to do
    if (message->_data == NULL) {
        message->_data = CFDataCreateMutable(CFGetAllocator(message), 0);
        message->_flags |= MUTABLE_DATA;
    } else if ((message->_flags & MUTABLE_DATA) == 0) {
        CFMutableDataRef newData = CFDataCreateMutableCopy(CFGetAllocator(message), 0, message->_data);
        CFRelease(message->_data);
        message->_data = newData;
        message->_flags |= MUTABLE_DATA;
    }
    CFDataAppendBytes((CFMutableDataRef)message->_data, newBytes, numBytes);

    if ((message->_flags & HEADERS_COMPLETE) == 0) {
        // Parse as much of the headers as possible
        return _parseHeadersFromData(message);
    }
    return TRUE;
}

extern Boolean _CFHTTPMessageConvertToDataOnlyResponse(CFHTTPMessageRef message) {
    if (message->_firstLine) return FALSE;
    if (!(message->_flags & IS_RESPONSE)) return FALSE;
    message->_firstLine = CFRetain(_kCFHTTPMessageEmptyString);
    message->_flags |= HEADERS_COMPLETE;
    return TRUE;
} 


/*
	This function should only be called after reading zero bytes on a stream
	and all bytes have already been appended to the message.  This is to catch
	those servers that choose not to send the empty line between headers and
	body when there is no body.  In this case, the CRLF is not truly required
	in order to make the message stand on its own.
 */
extern Boolean _CFHTTPMessageCanStandAlone(CFHTTPMessageRef message) {
	
    if (!(message->_flags & IS_RESPONSE)) return FALSE;
	if (message->_flags & HEADERS_COMPLETE) return TRUE;
	if (!message->_firstLine) return FALSE;
	if (message->_data && !CFDataGetLength(message->_data)) {
		message->_flags |= HEADERS_COMPLETE;
		return TRUE;
	}
	return FALSE;
}


/*
	This function is here in order to provide CFHTTPFilter
	a cheap and easy way of pulling the overflow from the
	read header bytes that ended up in the body.  These
	overflow bytes are supposed to be sent to the client
	and not left on the message.
 
	Today HTTPFilter performs a CopyBody and then a
	SetBody which causes an artificial balloon for a
	moment in time.  Simply getting, retaining, and then
	setting will get rid of the balloon effect.
*/
extern CFDataRef _CFHTTPMessageGetBody(CFHTTPMessageRef msg) {
	return msg->_data;
}


extern Boolean _CFHTTPMessageIsEmpty(CFHTTPMessageRef message) {
    if (message->_firstLine) return FALSE;
    if (message->_data && CFDataGetLength(message->_data)) return FALSE;
    return TRUE;
}

