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
#include <CoreFoundation/CFNumber.h>
#include <CFNetwork/CFHTTPMessage.h>
#include <CFNetwork/CFHTTPStream.h>
#include <CFNetwork/CFHTTPStreamPriv.h>
#include <CFNetwork/CFSocketStream.h>
#include <CoreFoundation/CFStreamPriv.h>
#include <CFNetwork/CFSocketStreamPriv.h>
#include "CFHTTPInternal.h"
#include "CFNetworkInternal.h"
#include <stdlib.h>
#include <string.h>

extern CFDataRef _CFHTTPMessageCopySerializedHeaders(CFHTTPMessageRef msg, Boolean forProxy);

const SInt32 kCFStreamErrorDomainHTTP = 4;
const SInt32 _kCFStreamErrorHTTPStreamAtMark = -2095; // Randomly chosen value....
const SInt32 _kCFStreamErrorHTTPSProxyFailure = -2096;

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPFilterCommaSeparator					CFSTR(",")
#define _kCFHTTPFilterContentLengthHeader				CFSTR("Content-Length")
#define _kCFHTTPFilterTransferEncodingHeader			CFSTR("Transfer-Encoding")
#define _kCFHTTPFilterTransferEncodingChunked			CFSTR("chunked")
#define _kCFHTTPFilterTransferEncodingChunked2			CFSTR("Chunked")
#define _kCFHTTPFilterTransferEncodingChunkedSeparator	CFSTR(", Chunked")
#define _kCFHTTPFilterTransferEncodingIdentity			CFSTR("Identity")
#define _kCFHTTPFilterTrailingHeadersFormat				CFSTR("%@, %@")
#define _kCFHTTPFilterProxyAuthorizationHeader			CFSTR("Proxy-Authorization")
#define _kCFHTTPFilterHTTPSScheme						CFSTR("https")
#define _kCFHTTPStreamConnectionHeader					CFSTR("Connection")
#define _kCFHTTPStreamProxyConnectionHeader				CFSTR("Proxy-Connection")
#define _kCFHTTPStreamConnectionKeepAlive				CFSTR("keep-alive")
#define _kCFHTTPStreamConnectionClose					CFSTR("close")
#else
static CONST_STRING_DECL(_kCFHTTPFilterCommaSeparator, ",")
static CONST_STRING_DECL(_kCFHTTPFilterContentLengthHeader, "Content-Length")
static CONST_STRING_DECL(_kCFHTTPFilterTransferEncodingHeader, "Transfer-Encoding")
static CONST_STRING_DECL(_kCFHTTPFilterTransferEncodingChunked, "chunked")
static CONST_STRING_DECL(_kCFHTTPFilterTransferEncodingChunked2, "Chunked")
static CONST_STRING_DECL(_kCFHTTPFilterTransferEncodingChunkedSeparator, ", Chunked")
static CONST_STRING_DECL(_kCFHTTPFilterTransferEncodingIdentity, "Identity")
static CONST_STRING_DECL(_kCFHTTPFilterTrailingHeadersFormat, "%@, %@")
static CONST_STRING_DECL(_kCFHTTPFilterProxyAuthorizationHeader, "Proxy-Authorization")
static CONST_STRING_DECL(_kCFHTTPFilterHTTPSScheme, "https")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionHeader, "Connection")
static CONST_STRING_DECL(_kCFHTTPStreamProxyConnectionHeader, "Proxy-Connection")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionKeepAlive, "keep-alive")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionClose, "close")
#endif	/* __CONSTANT_CFSTRINGS__ */

CONST_STRING_DECL(_kCFStreamPropertyHTTPSProxyHoldYourFire, "_kCFStreamPropertyHTTPSProxyHoldYourFire")

// For now, don't allow this -- REW, 4/24/01
//static const UInt8 *httpFilteredGetBuffer(CFReadStreamRef stream, CFIndex maxBytesToRead, CFIndex *numBytesRead, CFStreamError *error, Boolean *atEOF, void *info);

static CFIndex httpRdFilterRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info);
static Boolean httpRdFilterCanRead(CFReadStreamRef stream, void *info);
static CFTypeRef httpRdFilterCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info);
static Boolean httpWrFilterCanWrite(CFWriteStreamRef stream, void *info);

//#define DEBUG_FILTER 1
//#define LOG_FILTER 1

typedef struct {
    CFHTTPMessageRef header;
    UInt32 flags;
	CFSpinLock_t lock;
    long long expectedBytes;  // Number of bytes expected; if we are chunked, this value is only for the current chunk
    long long processedBytes;  // Number of bytes thusfar returned; if we are chunked, this value is only for the current chunk, and does not include the chunk header bytes themselves.
    CFMutableDataRef _data;
    union {
        CFReadStreamRef r;
        CFWriteStreamRef w;
    } socketStream;
    union {
        CFReadStreamRef r;
        CFWriteStreamRef w;
    } filteredStream;
    CFDataRef customSSLContext;
#if defined(DEBUG_FILTER)
    CFMutableDataRef _allData;
#endif    
} _CFHTTPFilter;

static Boolean httpRdFilterCanReadNoSignal(CFReadStreamRef stream, _CFHTTPFilter *httpFilter, CFStreamError *err);

/* flag bits */

/* Shared flag bits - 0-7 */
#define IS_CHUNKED (0)
#define DATA_IS_MUTABLE (1)
#define MARK_ENABLED (2)
#define AT_MARK (3)
#define FIRST_CHUNK (4)
#define MARK_SIGNALLED (5)

/* For read streams  - 8-15 */
#define PARSE_FAILED (8)
#define CONNECTION_LOST (9)
#define LAST_CHUNK (10)
#define ZERO_LENGTH_RESPONSE_EXPECTED (11)
#define LAX_PARSING (12)

/* For write streams - 16-31 */
#define HEADER_TRANSMITTED (16)
#define FIRST_HEADER_SEEN (17)
#define IS_PROXY (18)
#define IS_HTTPS_PROXY (19)
#define HTTPS_PROXY_FAILURE (20)
#define STRIP_PROXY_AUTH (21)

/* special values for expectedBytes for read streams*/
#define MID_CHUNK_HEADER_PARSE  (-3)
#define HEADERS_NOT_YET_CHECKED (-2)
#define WAIT_FOR_END_OF_STREAM  (-1)

CF_INLINE void setConnectionLost(_CFHTTPFilter *filter, CFStreamError *error) {
    error->error = kCFStreamErrorHTTPConnectionLost;
    error->domain = kCFStreamErrorDomainHTTP;
    __CFBitSet(filter->flags, CONNECTION_LOST);
}

CF_INLINE void setParseFailure(_CFHTTPFilter *filter, CFStreamError *error) {
    error->error = kCFStreamErrorHTTPParseFailure;
    error->domain = kCFStreamErrorDomainHTTP;
    __CFBitSet(filter->flags, PARSE_FAILED);
}

CF_INLINE Boolean httpRdFilterAtMark(_CFHTTPFilter *httpFilter) {
    if (__CFBitIsSet(httpFilter->flags, MARK_ENABLED) && __CFBitIsSet(httpFilter->flags, AT_MARK)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void *httpRdFilterCreate(CFReadStreamRef stream, void *info) {
    _CFHTTPFilter *oldFilter = (_CFHTTPFilter *)info;
    _CFHTTPFilter *filter = (_CFHTTPFilter *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFHTTPFilter), 0);
    filter->header = oldFilter->header;
    CFRetain(filter->header);
	filter->lock = 0;
    filter->flags = oldFilter->flags;
    filter->expectedBytes = HEADERS_NOT_YET_CHECKED;
    filter->processedBytes = 0;
    filter->_data = NULL;
#if defined(DEBUG_FILTER)
    filter->_allData = CFDataCreateMutable(NULL, 0);
#endif    
    filter->socketStream.r = oldFilter->socketStream.r;
    CFRetain(filter->socketStream.r);
    filter->filteredStream.r = stream; // Do not retain; that will introduce a retain loop.
    filter->customSSLContext = NULL;
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: Creating read filter 0x%x\n", (unsigned)filter);
#endif
    return filter;
}

static void httpRdFilterDealloc(CFReadStreamRef stream, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: Destroying read filter 0x%x\n", (unsigned)filter);
#endif
	__CFSpinLock(&filter->lock);
    if (filter->header) CFRelease(filter->header);
    if (filter->_data) CFRelease(filter->_data);
#if defined(DEBUG_FILTER)
    filter->_allData = CFDataCreateMutable(NULL, 0);
#endif    
    CFReadStreamClose(filter->socketStream.r);
    CFReadStreamSetClient(filter->socketStream.r, kCFStreamEventNone, NULL, NULL);
    CFRelease(filter->socketStream.r);
    CFAllocatorDeallocate(CFGetAllocator(stream), filter);
}

// stream is filter->socketStream; clientCallBackInfo is the stream whose info pointer is filter.
static void httpRdFilterStreamCallBack(CFReadStreamRef stream, CFStreamEventType event, void *clientCallBackInfo) {
    CFReadStreamRef filterStream = (CFReadStreamRef)clientCallBackInfo;
    _CFHTTPFilter *filter = (_CFHTTPFilter *)CFReadStreamGetInfoPointer(filterStream);
	
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterStreamCallBack(stream = 0x%x, event = %d, filter = 0x%x)\n", (unsigned)stream, event, (unsigned)filter);
#endif
    switch (event) {
    case kCFStreamEventHasBytesAvailable:
		{
			CFStreamEventType event = kCFStreamEventNone;
			if (httpRdFilterCanRead(filterStream, filter)) {
				event = kCFStreamEventHasBytesAvailable;
			} else {
				
				// 3784921 Check to see if the call to httpRdFilterCanRead has
				// actually pushed the stream to or beyond the end.
				CFStreamStatus status = CFReadStreamGetStatus(stream);
				if ((status != kCFStreamStatusAtEnd) &&
					(status != kCFStreamStatusClosed) &&
					(status != kCFStreamStatusError))
				{
					__CFSpinLock(&filter->lock);
					if (httpRdFilterAtMark(filter) && !__CFBitIsSet(filter->flags, MARK_SIGNALLED)) {
						__CFBitSet(filter->flags, MARK_SIGNALLED);
						event = kCFStreamEventMarkEncountered;
					}
					__CFSpinUnlock(&filter->lock);
				}
			}
			
			if (event != kCFStreamEventNone)
				CFReadStreamSignalEvent(filterStream, event, NULL);
		}
        break;
    case kCFStreamEventErrorOccurred: {
        CFStreamError error = CFReadStreamGetError(stream);
		if (error.domain == kCFStreamErrorDomainPOSIX && (error.error == ECONNRESET || error.error == EPIPE) && _CFHTTPMessageIsEmpty(filter->header)) {
			error.domain = kCFStreamErrorDomainHTTP;
			error.error = kCFStreamErrorHTTPConnectionLost;
		}
        CFReadStreamSignalEvent(filterStream, kCFStreamEventErrorOccurred, &error);
        break;
    }
    default:
        CFReadStreamSignalEvent(filterStream, event, NULL);        
    }
}

static Boolean httpRdFilterOpen(CFReadStreamRef stream, CFStreamError *errorCode, Boolean *openComplete, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
    CFStreamClientContext clientContext = {0, stream, NULL, NULL, (CFStringRef(*)(void *))CFCopyDescription}; // Do not use CFRetain/CFRelease; they introduce a retain loop
    Boolean result;
    CFStreamStatus status;
	__CFSpinLock(&filter->lock);
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterOpen(stream = 0x%x, filter = 0x%x)\n", (unsigned)stream, (unsigned)filter);
#endif
    CFReadStreamSetClient(filter->socketStream.r, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, (CFReadStreamClientCallBack)httpRdFilterStreamCallBack, &clientContext);
    status = CFReadStreamGetStatus(filter->socketStream.r);
    if (status == kCFStreamStatusNotOpen) {
        result = CFReadStreamOpen(filter->socketStream.r); 
    } else {
        result = TRUE;
    }
    if (result) {
        errorCode->error = 0;
        *openComplete = TRUE;
    } else {
        *openComplete = TRUE;
        *errorCode = CFReadStreamGetError(filter->socketStream.r);
    }
	
	__CFSpinUnlock(&filter->lock);

	return result;
}

static void httpRdFilterClose(CFReadStreamRef stream, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterClose(stream = 0x%x, filter = 0x%x)\n", (unsigned)stream, (unsigned)filter);
#endif
    CFReadStreamClose(filter->socketStream.r);
	__CFSpinUnlock(&filter->lock);
}

static void httpRdFilterSchedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterSchedule(stream = 0x%x, runLoop = 0x%x, mode = 0x%x, filter = 0x%x)\n", (unsigned)stream, (unsigned)runLoop, (unsigned)runLoopMode, (unsigned)filter);
#endif
    CFReadStreamScheduleWithRunLoop(filter->socketStream.r, runLoop, runLoopMode);
	__CFSpinUnlock(&filter->lock);
}

static void httpRdFilterUnschedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterUnschedule(stream = 0x%x, runLoop = 0x%x, mode = 0x%x, filter = 0x%x)\n", (unsigned)stream, (unsigned)runLoop, (unsigned)runLoopMode, (unsigned)filter);
#endif
    CFReadStreamUnscheduleFromRunLoop(filter->socketStream.r, runLoop, runLoopMode);
	__CFSpinUnlock(&filter->lock);
}


static long long expectedSizeFromHeaders(CFHTTPMessageRef responseHeader) {
    CFStringRef contentLength;
    Boolean gotGoodContentLength = FALSE;
    
    // Check to see if this is a request and if the request method is "GET"; if so, set the expected content length to 0 regardless of anything else (some servers get upset if you explicitly set the content length to 0 for GETs, even though that should work fine by the spec).
    if (CFHTTPMessageIsRequest(responseHeader) && _CFHTTPMessageIsGetMethod(responseHeader)) {
		return 0; // GET requests send no message body
    }
   
    contentLength = CFHTTPMessageCopyHeaderFieldValue(responseHeader, _kCFHTTPFilterContentLengthHeader);
    if (contentLength) {
        CFIndex charIndex = 0, length = CFStringGetLength(contentLength);
        UniChar ch;
        long long expectedBytes = 0;
        while (charIndex < length) {
            ch = CFStringGetCharacterAtIndex(contentLength, charIndex);
            if (ch != ' ' && ch != '\t') break;
            charIndex ++;
        }
		
		/* 3687749 Mark an empty content length as zero */
		if (charIndex >= length) gotGoodContentLength = TRUE;
		
        while (charIndex < length) {
            ch = CFStringGetCharacterAtIndex(contentLength, charIndex);
            charIndex ++;
            if (ch >= '0' && ch <= '9') {
                long long newVal = expectedBytes * 10 + ch - '0';
                if (charIndex < length && newVal > (LLONG_MAX/10LL - 10LL)) {
                    // Protect against overflow;
                    gotGoodContentLength = FALSE;
                    break;
                } else {
                    gotGoodContentLength = TRUE;
                    expectedBytes = newVal;
                }
            } else {
                break;
            }
        }
        CFRelease(contentLength);
        if (gotGoodContentLength) {
            return expectedBytes;
        }
    }
    if (!CFHTTPMessageIsRequest(responseHeader)) {
        int status = CFHTTPMessageGetResponseStatusCode(responseHeader);
        if (status == 204 || status == 304) {
            // No content, per the HTTP/1.1 spec
            return 0;
        }
    }
    return WAIT_FOR_END_OF_STREAM;
}

static Boolean readHeaderBytes(_CFHTTPFilter *httpFilter, Boolean toCompletion, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error) {
    Boolean parseSucceeded = TRUE;
    Boolean connectionLost = FALSE;
    CFStringRef headerString;
    CFReadStreamRef stream = httpFilter->socketStream.r;
    int status;
    	
    if (httpFilter->_data) {
        CFIndex length = CFDataGetLength(httpFilter->_data);
        const UInt8 *bytes = CFDataGetBytePtr(httpFilter->_data);
        parseSucceeded = CFHTTPMessageAppendBytes(httpFilter->header, bytes, length);
        // httpFilter->_header is holding all bytes for us now
        CFRelease(httpFilter->_data);
        httpFilter->_data = NULL;
    }
	else {
		CFHTTPMessageRef proxyResponse = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyPreviousCONNECTResponse);
		if (!proxyResponse)
			proxyResponse = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyCONNECTResponse);
		if (proxyResponse) {
			int status = CFHTTPMessageGetResponseStatusCode(proxyResponse);
			if (status == 200)
				CFRelease(proxyResponse);
			else {
				CFStringRef connectionHeader = CFHTTPMessageCopyHeaderFieldValue(proxyResponse, _kCFHTTPStreamProxyConnectionHeader);
				if (!connectionHeader || (CFStringCompare(connectionHeader, _kCFHTTPStreamConnectionKeepAlive, kCFCompareCaseInsensitive) != kCFCompareEqualTo))
					CFHTTPMessageSetHeaderFieldValue(proxyResponse, _kCFHTTPStreamConnectionHeader, _kCFHTTPStreamConnectionClose);
				if (connectionHeader) CFRelease(connectionHeader);
				CFRelease(httpFilter->header);
				httpFilter->header = proxyResponse;
				
			}
		}
	}
	
    while (parseSucceeded && !CFHTTPMessageIsHeaderComplete(httpFilter->header) && (toCompletion || CFReadStreamHasBytesAvailable(stream))) {
        CFIndex bytesRead = CFReadStreamRead(stream, buffer, bufferLength);
        if (bytesRead == 0) {
            // ???? EOF isn't legal at this point; can we convert to an 0.9 response?
            if (_CFHTTPMessageIsEmpty(httpFilter->header)) {
                parseSucceeded = FALSE;
                connectionLost = TRUE;
            } else if (!_CFHTTPMessageConvertToDataOnlyResponse(httpFilter->header)) {
				parseSucceeded = _CFHTTPMessageCanStandAlone(httpFilter->header) ? TRUE : FALSE;
            } else {
                // The call to ConvertToDataOnlyResponse will cause CFHTTPMessageIsHeaderComplete() to return TRUE
                break;
            }
        } else if (bytesRead < 0) {
            break;
        } else {
#if defined(DEBUG_FILTER)
            CFDataAppendBytes(httpFilter->_allData, buffer, bytesRead);
#endif
            parseSucceeded = CFHTTPMessageAppendBytes(httpFilter->header, buffer, bytesRead);
        }
    }
    if (!parseSucceeded) {
        if (connectionLost) {
            setConnectionLost(httpFilter, error);
        } else {
            setParseFailure(httpFilter, error);
        }
        return FALSE;
    } else if (CFReadStreamGetStatus(stream) == kCFStreamStatusError) {
        *error = CFReadStreamGetError(stream);
		if (error->domain == kCFStreamErrorDomainPOSIX && (error->error == ECONNRESET || error->error == EPIPE) && _CFHTTPMessageIsEmpty(httpFilter->header)) {
            setConnectionLost(httpFilter, error);
		}
        return FALSE;
    } else if (!CFHTTPMessageIsHeaderComplete(httpFilter->header)) {
        // Didn't get enough bytes, and we were asked not to block
        return TRUE;
    }

	if (httpFilter->_data)
		CFRelease(httpFilter->_data);
	
    httpFilter->_data = (CFMutableDataRef)_CFHTTPMessageGetBody(httpFilter->header);
	if (httpFilter->_data) {
		CFRetain(httpFilter->_data);
		CFHTTPMessageSetBody(httpFilter->header, NULL);
	}
    __CFBitClear(httpFilter->flags, DATA_IS_MUTABLE);

    // See if this is a 10x response; if it is, we swallow it and start a new header immediately.  10x responses cannot carry bodies, so there cannot be any further bytes.
    status = CFHTTPMessageGetResponseStatusCode(httpFilter->header);
    if (status >= 100 && status < 200) {
        CFHTTPMessageRef newHeader = CFHTTPMessageCreateEmpty(CFGetAllocator(httpFilter->header), FALSE);
        if (__CFBitIsSet(httpFilter->flags, LAX_PARSING)) {
            _CFHTTPMessageSetLaxParsing(newHeader, TRUE);
        }
        CFRelease(httpFilter->header);
        httpFilter->header = newHeader;
        return readHeaderBytes(httpFilter, toCompletion, buffer, bufferLength, error);
    }
    
    if (__CFBitIsSet(httpFilter->flags, ZERO_LENGTH_RESPONSE_EXPECTED)) {
        __CFBitClear(httpFilter->flags, ZERO_LENGTH_RESPONSE_EXPECTED);
        httpFilter->expectedBytes = 0;
        return TRUE;
    }
    
    // See if we're chunked or not
    headerString = CFHTTPMessageCopyHeaderFieldValue(httpFilter->header, _kCFHTTPFilterTransferEncodingHeader);
    if (headerString) {
        CFArrayRef encodingsArray = CFStringCreateArrayBySeparatingStrings(CFGetAllocator(headerString), headerString, _kCFHTTPFilterCommaSeparator);
        CFIndex i, c = CFArrayGetCount(encodingsArray);
        for (i = 0; i < c; i ++) {
            CFStringRef encoding = CFArrayGetValueAtIndex(encodingsArray, i);
            if (CFStringFindWithOptions(encoding, _kCFHTTPFilterTransferEncodingChunked, CFRangeMake(0, CFStringGetLength(encoding)), kCFCompareCaseInsensitive, NULL)) {
                break;
            }
        }
        if (i < c) {
            CFStringRef newString;
            if (c == 1) {
                newString = CFRetain(_kCFHTTPFilterTransferEncodingIdentity);
            } else {
                CFMutableArrayRef mArray = CFArrayCreateMutableCopy(CFGetAllocator(headerString), c, encodingsArray);
                CFArrayRemoveValueAtIndex(mArray, i);
                newString = CFStringCreateByCombiningStrings(CFGetAllocator(headerString), encodingsArray, _kCFHTTPFilterCommaSeparator);
                CFRelease(mArray);
            }
            CFHTTPMessageSetHeaderFieldValue(httpFilter->header, _kCFHTTPFilterTransferEncodingHeader, newString);
            CFRelease(newString);
            __CFBitSet(httpFilter->flags, IS_CHUNKED);
        }
        CFRelease(encodingsArray);
        CFRelease(headerString);
    }
    
    if (__CFBitIsSet(httpFilter->flags, IS_CHUNKED)) {
        // Mark it as at the end of a chunk so the read routine will start with a new chunk
        __CFBitSet(httpFilter->flags, FIRST_CHUNK);
        httpFilter->expectedBytes = 0;
    } else {
        // See if we have a valid content-length
        httpFilter->expectedBytes = expectedSizeFromHeaders(httpFilter->header);
		
		if ((httpFilter->expectedBytes == WAIT_FOR_END_OF_STREAM) && (CFReadStreamGetStatus(stream) == kCFStreamStatusAtEnd))
			httpFilter->expectedBytes = 0;
    }
    return TRUE;
}

/* From RFC 2616 (HTTP/1.1), section 19.4.6
    A process for decoding the "chunked" transfer-coding (section 3.6) can be represented in pseudo-code as:
        length := 0
        read chunk-size, chunk-extension (if any) and CRLF
        while (chunk-size > 0) {
            read chunk-data and CRLF
            append chunk-data to entity-body
            length := length + chunk-size
            read chunk-size and CRLF
        }
        read entity-header
        while (entity-header not empty) {
            append entity-header to existing header fields
            read entity-header
        }
        Content-Length := length
        Remove "chunked" from Transfer-Encoding
        
And from section 3.6.1, the productions for the chunked terms
    Chunked-Body = *chunk
                    last-chunk
                    trailer
                    CRLF
    chunk = chunk-size [ chunk-extension ] CRLF
            chunk-data CRLF
    chunk-size = 1*HEX
    last-chunk = 1*("0") [ chunk-extension ] CRLF
    chunk-extension = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
    chunk-ext-name = token
    chunk-ext-val = token | quoted-string
    chunk-data = chunk-size(OCTET)
    trailer = *(entity-header CRLF)
*/

static inline Boolean isToken(UInt8 ch) {
    if (ch < 32 || ch == 127) return FALSE;
    if (ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '@') return FALSE;
    if (ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '\"') return FALSE;
    if (ch == '/' || ch == '[' || ch == ']' || ch == '?' || ch == '=') return FALSE;
    if (ch == '{' || ch == '}' || ch == ' ' || ch == '\t') return FALSE;
    return TRUE;
}

static inline const UInt8 *skipLWS(const UInt8 *current, const UInt8 *end) {
    while (current < end) {
        if (*current != ' ' && *current != '\t') break;
        current ++;
    }
    return current;
}

static inline const UInt8 *skipToken(const UInt8 *curr, const UInt8 *end) {
    while (curr < end && isToken(*curr)) {
        curr ++;
    }
    return curr;
}

static inline const UInt8 *skipQuotedString(const UInt8 *curr, const UInt8 *end) {
    curr ++; // Get past the initial quote; the caller should have checked this
    while (curr < end) {
        if (*curr == '\"') {
            curr ++;
            return curr;
        } else if (*curr == '\\') {
            curr ++;
        }
        curr ++;
    }
    return curr;
}

/* Return the size of the chunk, -1 on parse failure, or MID_CHUNK_HEADER_PARSE if the buffer doesn't provide enough data */
static CFIndex parseChunkedHeader(const UInt8 *buffer, CFIndex bufferLength, Boolean isFirstChunk, CFIndex *firstDataByte) {
    const UInt8 *curr = buffer, *end = buffer + bufferLength;
    CFIndex size = 0;
    if (!isFirstChunk) {
		while (curr != end) {
			UInt8 c = *curr;
			if ((c == '\r') || (c == '\n') || (c == ' ') || (c == '\t'))
				curr++;
			else
				break;
		}
		if (curr == end) return MID_CHUNK_HEADER_PARSE;
    }
    for (; curr < end; curr ++) {
        int digit;
        if (*curr >= '0' && *curr <= '9') {
            digit = *curr - '0';
        } else if (*curr >= 'A' && *curr <= 'F') {
            digit = *curr - 'A' + 10;
        } else  if (*curr >= 'a' && *curr <= 'f') {
            digit = *curr - 'a' + 10;
        } else {
            break;
        }
        size = (size << 4) + digit;
    }
    curr = skipLWS(curr, end);
    if (curr == end) return MID_CHUNK_HEADER_PARSE;
    while (*curr == ';') {
		curr++;						// Skip the semi-colon
        curr = skipLWS(curr, end);
        if (curr == end) return MID_CHUNK_HEADER_PARSE;
        UInt8 c = *curr;
        if (c == '\r' || c == '\n') {
            // malformed chunk
            // semicolon without chunk extension
            // just move on
            break;
        }
        else {
            // Advance past extensions
            curr = skipToken(curr, end);
            curr = skipLWS(curr, end);
            if (curr == end) return MID_CHUNK_HEADER_PARSE;
            if (*curr == '=') {
                curr = skipLWS(curr, end);
                if (curr == end) return MID_CHUNK_HEADER_PARSE;
                if (*curr == '\"') {
                    curr = skipQuotedString(curr, end);
                } else {
                    curr = skipToken(curr, end);
                }
                curr = skipLWS(curr, end);
                if (curr == end) return MID_CHUNK_HEADER_PARSE;
            }
        }
    }
    if (*curr == '\r') {
        if (curr + 1 == end) {
            return MID_CHUNK_HEADER_PARSE;
        } else if (*(curr + 1) == '\n') {
            *firstDataByte = curr + 2 - buffer;
            return size;
        } else {
            // Parse failure
            return -1;
        }
    }
    return -1;    
}

// Returns TRUE if and only if there were enough bytes in headerBytes to get a definitive result; otherwise, returns FALSE leaving httpFilter unmodified.  if TRUE is returned, firstDataByte is set to point to the first byte that should be reported back to the client (and httpFilter->expectedBytes and httpFilter->processedBytes is set accordingly), or -1 if the parse failed.  Regardless, error is set accordingly.
static Boolean setFilterForChunkedHeaderBytes(_CFHTTPFilter *httpFilter, const UInt8 *headerBytes, CFIndex headerByteLength, CFIndex *firstDataByte, CFStreamError *error) {
    CFIndex chunkSize;
    if (__CFBitIsSet(httpFilter->flags, FIRST_CHUNK)) {
        chunkSize = parseChunkedHeader(headerBytes, headerByteLength, TRUE, firstDataByte);
        if (chunkSize != MID_CHUNK_HEADER_PARSE) {
            __CFBitClear(httpFilter->flags, FIRST_CHUNK);
        }
    } else {
        chunkSize = parseChunkedHeader(headerBytes, headerByteLength, FALSE, firstDataByte);
    }
    if (chunkSize == -1) {
        // parse failure
        *firstDataByte = -1;
        setParseFailure(httpFilter, error);
        return TRUE;
    } else if (chunkSize == MID_CHUNK_HEADER_PARSE) {
        return FALSE;
    } else {
        // parse success
        error->error = 0;
        httpFilter->expectedBytes = chunkSize;
        httpFilter->processedBytes = 0;
        if (chunkSize == 0) {
            __CFBitSet(httpFilter->flags, LAST_CHUNK);
        }
        return TRUE;
    }
}

static inline void setDataForRange(_CFHTTPFilter *filter, CFRange newRange) {
    if (newRange.length <= 0) {
        CFRelease(filter->_data);
        filter->_data = NULL;
    } else if (__CFBitIsSet(filter->flags, DATA_IS_MUTABLE)) {
        if (newRange.location > 0) {
            CFDataDeleteBytes(filter->_data, CFRangeMake(0, newRange.location));
        }
        CFDataSetLength(filter->_data, newRange.length);
    } else {
        CFMutableDataRef newData = CFDataCreateMutable(CFGetAllocator(filter->_data), 0);
        CFDataAppendBytes(newData, CFDataGetBytePtr(filter->_data)+newRange.location, newRange.length);
        CFRelease(filter->_data);
        filter->_data = newData;
        __CFBitSet(filter->flags, DATA_IS_MUTABLE);
    }
}

static inline void setDataForBytes(_CFHTTPFilter *filter, const UInt8 *bytes, CFIndex length) {
    if (filter->_data) CFRelease(filter->_data);
    if (length == 0) {
        filter->_data = NULL;
    } else {
        filter->_data = CFDataCreateMutable(CFGetAllocator(filter->header), 0);
        CFDataAppendBytes(filter->_data, bytes, length);
        __CFBitSet(filter->flags, DATA_IS_MUTABLE);
    }
}


// Returns FALSE and sets error on parse/stream error.  Returns TRUE and sets httpFilter->expectedBytes to MID_CHUNK_HEADER_PARSE if there are insufficient bytes to parse the header, and we were requested to not block.  Otherwise, returns TRUE and sets expectedBytes accordingly.
static Boolean readChunkedHeader(_CFHTTPFilter *httpFilter, Boolean toCompletion, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error) {
    CFIndex dataLength = httpFilter->_data ? CFDataGetLength(httpFilter->_data) : 0;
    const UInt8 *dataBytes = dataLength != 0 ? CFDataGetBytePtr(httpFilter->_data) : NULL;
    Boolean result = TRUE, done = FALSE;
    CFReadStreamRef stream = httpFilter->socketStream.r;
    // First see if the bytes in data are sufficient 
    if (dataBytes) {
        CFIndex endOfHeader;
        if (setFilterForChunkedHeaderBytes(httpFilter, dataBytes, dataLength, &endOfHeader, error)) {
            done = TRUE;
            result = (endOfHeader != -1);
            // if the parse succeeded, we must deal with any fragmentary bytes.  It is possible that data should be unchanged (if the client has requested that we return the chunk headers to them), hence the second clause below
            if (result && endOfHeader != 0) {
                setDataForRange(httpFilter, CFRangeMake(endOfHeader, dataLength-endOfHeader));
            }
        }
    }
    // At this point, we'll have to read from the stream; if we can't do that, jump to the end.
    if (!done && !toCompletion && !CFReadStreamHasBytesAvailable(stream) && CFReadStreamGetStatus(stream) != kCFStreamStatusError) {
        httpFilter->expectedBytes = MID_CHUNK_HEADER_PARSE;
        done = TRUE;
        result = TRUE;
    }

    // Try and use buffer if possible
    if (!done && dataLength < bufferLength) {
        UInt8 *readBuffer = buffer;
        CFIndex readBufferLength = bufferLength;
        CFIndex bytesRead;
        if (dataBytes) {
            memmove(buffer, dataBytes, dataLength);
            readBuffer += dataLength;
            readBufferLength -= dataLength;
        }
        bytesRead = CFReadStreamRead(stream, readBuffer, readBufferLength);
        if (bytesRead < 0) {
            // stream error
            *error = CFReadStreamGetError(stream);
            done = TRUE;
            result = FALSE;
        } else if (bytesRead == 0) {
            // Premature end of stream.  However, some servers send simply CRLF for their last chunk instead of 0CRLF; be tolerant of this.
            if (readBuffer == buffer + 2 && buffer[0] == '\r' && buffer[1] == '\n') {
                httpFilter->expectedBytes = 0;
                httpFilter->processedBytes = 0;
                __CFBitSet(httpFilter->flags, LAST_CHUNK);
                result = TRUE;
            } else {
                setParseFailure(httpFilter, error);
                result = FALSE;
            }
            done = TRUE;
        } else {
            // Read was successful; the header bytes are now in buffer and have length dataLength + bytesRead
            CFIndex endOfHeader;
#if defined(DEBUG_FILTER)
            CFDataAppendBytes(httpFilter->_allData, readBuffer, bytesRead);
#endif
            if (setFilterForChunkedHeaderBytes(httpFilter, buffer, dataLength + bytesRead, &endOfHeader, error)) {
                done = TRUE;
                result = (endOfHeader != -1);
                if (result) {
                    // parse succeeded; figure out what to do with the excess bytes.  Note that endOfHeader must be interpretted relative to buffer.
                    setDataForBytes(httpFilter, buffer + endOfHeader, dataLength + bytesRead - endOfHeader);
                }
            } else {
                // Still don't have enough bytes.  Return the bytes we have thusfar read in httpFilter->header
                if (dataLength && __CFBitIsSet(httpFilter->flags, DATA_IS_MUTABLE)) {
                    CFDataAppendBytes(httpFilter->_data, buffer + dataLength, bytesRead);
                } else {
                    setDataForBytes(httpFilter, buffer, dataLength + bytesRead);
                }
            }
        }
    }
    
    // If all else fails, keep reading bytes into data until we have enough.  Hopefully we never reach this case (assuming we're being given a reasonably sized buffer, we shouldn't - chunk headers are in general very small), since it's somewhat expensive.
    if (httpFilter->_data == NULL || !__CFBitIsSet(httpFilter->flags, DATA_IS_MUTABLE)) {
        if (httpFilter->_data) {
            CFMutableDataRef newData = CFDataCreateMutableCopy(CFGetAllocator(httpFilter->_data), 0, httpFilter->_data);
            CFRelease(httpFilter->_data);
            httpFilter->_data = newData;
        } else {
            httpFilter->_data = CFDataCreateMutable(CFGetAllocator(httpFilter->header), 0);
        }
        __CFBitSet(httpFilter->flags, DATA_IS_MUTABLE);
    }

    while (!done && (toCompletion || CFReadStreamHasBytesAvailable(stream) || CFReadStreamGetStatus(stream) == kCFStreamStatusError)) {
        CFIndex bytesRead, oldLength = CFDataGetLength(httpFilter->_data);
        UInt8 *bytes;
        CFDataSetLength(httpFilter->_data, oldLength + bufferLength);
        bytes = CFDataGetMutableBytePtr(httpFilter->_data);
        bytesRead = CFReadStreamRead(stream, bytes + oldLength, bufferLength);
        if (bytesRead < 0) {
            // error
            done = TRUE;
            result = FALSE;
            *error = CFReadStreamGetError(stream);
        } else if (bytesRead == 0) {
            // premature end of stream
            done = TRUE;
            result = FALSE;
            setParseFailure(httpFilter, error);
        } else {
            // Append the read bytes to the header, then try and parse the header bytes
            CFIndex firstDataByte;
#if defined(DEBUG_FILTER)
            CFDataAppendBytes(httpFilter->_allData, bytes + oldLength, bytesRead);
#endif
            if (setFilterForChunkedHeaderBytes(httpFilter, bytes, oldLength + bytesRead, &firstDataByte, error)) {
                done = TRUE;
                result = (firstDataByte != -1);
                if (result && firstDataByte != 0) {
                    CFDataDeleteBytes(httpFilter->_data, CFRangeMake(0, firstDataByte));
                    CFDataSetLength(httpFilter->_data, oldLength + bytesRead - firstDataByte);
                }
            } else {
                CFDataSetLength(httpFilter->_data, oldLength + bytesRead);
            }
        }
    }
    if (!done) {
        // never could get enough bytes
        result = TRUE;
        httpFilter->expectedBytes = MID_CHUNK_HEADER_PARSE;
    }
    return result;
}


static void addTrailingHeader(CFStringRef key, CFStringRef value, CFHTTPMessageRef response) {
    
    CFStringRef original = CFHTTPMessageCopyHeaderFieldValue(response, key);
    if (original) {
        value = CFStringCreateWithFormat(CFGetAllocator(response), NULL, _kCFHTTPFilterTrailingHeadersFormat, original, value);
        CFRelease(original);
    }
    
    CFHTTPMessageSetHeaderFieldValue(response, key, value);
}

// Returns TRUE if parse succeeds; sets error and returns FALSE otherwise.  Right now, we don't try and support trailing headers.  We should add this at some point....
// This is annoyingly complex code to check for CRLF.  We really need a push-back stream.... -- REW, 7/16/2001
static Boolean readChunkedTrailers(_CFHTTPFilter *httpFilter, CFStreamError *error) {

    static const char bogus[] = "HTTP/1.0 200 OK\r\n";
    CFReadStreamRef stream = httpFilter->socketStream.r;
    Boolean result = TRUE;
    
    CFHTTPMessageRef hdrs = CFHTTPMessageCreateEmpty(CFGetAllocator(stream), FALSE);
    CFHTTPMessageAppendBytes(hdrs, (const UInt8*)bogus,  sizeof(bogus) - 1);
    
    if (httpFilter->_data) {
        CFIndex length = CFDataGetLength(httpFilter->_data);
        const UInt8 *bytes = CFDataGetBytePtr(httpFilter->_data);
        
        result = CFHTTPMessageAppendBytes(hdrs, bytes, length);
       
        CFRelease(httpFilter->_data);
        httpFilter->_data = NULL;
    }
    
    while (result && !CFHTTPMessageIsHeaderComplete(hdrs)) {
        
        UInt8 buffer[2048];
        CFIndex bytesRead = CFReadStreamRead(stream, buffer, sizeof(buffer));
        
        if (bytesRead > 0) {
#if defined(DEBUG_FILTER)
            CFDataAppendBytes(httpFilter->_allData, buffer, bytesRead);
#endif
            result = CFHTTPMessageAppendBytes(hdrs, buffer, bytesRead);
        }
        else if (bytesRead == 0) {
            setParseFailure(httpFilter, error);
            result = FALSE;
        }
        else {
            *error = CFReadStreamGetError(stream);
            result = FALSE;
        }
    }
    
    if (result) {
        CFDictionaryRef fields;
        
        CFDataRef leftovers = CFHTTPMessageCopyBody(hdrs);
        
        if (leftovers) {
            httpFilter->_data = (CFMutableDataRef)leftovers;
            __CFBitClear(httpFilter->flags, DATA_IS_MUTABLE);
        }
        
        fields = CFHTTPMessageCopyAllHeaderFields(hdrs);
        if (fields) {
            CFDictionaryApplyFunction(fields, (CFDictionaryApplierFunction)addTrailingHeader, httpFilter->header);
            CFRelease(fields);
        }
    }
    
    CFRelease(hdrs);
    
    return result;
}

static CFIndex doChunkedRead(_CFHTTPFilter *httpFilter, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF) {
    CFIndex lengthFilled = 0; // The number of bytes to actually report back; buffer is advance and bufferLength decremented as we get bytes from the various sources.
    Boolean errorOccurred = FALSE;
    CFReadStreamRef stream = httpFilter->socketStream.r;
    *atEOF = FALSE;
    error->error = 0;
    
    while (bufferLength > 0) {
        CFIndex bytesRemainingInChunk;
        
        if (__CFBitIsSet(httpFilter->flags, LAST_CHUNK)) {
            *atEOF = TRUE;
            errorOccurred = (readChunkedTrailers(httpFilter, error)) ? FALSE : TRUE;
            break;
        }
        
        // Read the header if necessary
        if (httpFilter->expectedBytes == MID_CHUNK_HEADER_PARSE || httpFilter->expectedBytes == httpFilter->processedBytes) {
            // Note that we don't want to block reading the chunk header if we've already got some bytes to return
            if (!readChunkedHeader(httpFilter, lengthFilled > 0 ? FALSE : TRUE, buffer, bufferLength, error)) {
                // Error occurred; error has already been set.
                errorOccurred = TRUE;
                break;
            } else if (httpFilter->expectedBytes == MID_CHUNK_HEADER_PARSE) {
                errorOccurred = FALSE;
                break;
            } else if (httpFilter->expectedBytes == 0) {
                // all done
                *atEOF = TRUE;
                errorOccurred = (readChunkedTrailers(httpFilter, error)) ? FALSE : TRUE;
                break;
            }
            // else we got a good chunk header; fall-through to start processing the actual bytes.
        }
        
        bytesRemainingInChunk = httpFilter->expectedBytes - httpFilter->processedBytes;
        // Exhaust any data we're holding
        if (httpFilter->_data) {
            const UInt8 *bytes = CFDataGetBytePtr(httpFilter->_data);
            CFIndex length = CFDataGetLength(httpFilter->_data);
            CFIndex numBytesToMove;
            if (length >= bytesRemainingInChunk) {
                // Data spans 2 chunks; do not take more than bytesRemainingInChunk
                if (bytesRemainingInChunk >= bufferLength) {
                    numBytesToMove = bufferLength;
                } else {
                    numBytesToMove = bytesRemainingInChunk;
                }
            } else {
                // Data's in a single chunk; can safely take it all
                if (length >= bufferLength) {
                    numBytesToMove = bufferLength;
                } else {
                    numBytesToMove = length;
                }
            }
            memmove(buffer, bytes, numBytesToMove);
            buffer += numBytesToMove;
            bufferLength -= numBytesToMove;
            lengthFilled += numBytesToMove;
            httpFilter->processedBytes += numBytesToMove;
            setDataForRange(httpFilter, CFRangeMake(numBytesToMove, length - numBytesToMove));
            bytesRemainingInChunk -= numBytesToMove;
        }
        if (bufferLength == 0) break;
        if (bytesRemainingInChunk == 0) continue;
        // Read up to the lesser of bytesRemainingInChunk and bufferLength directly from the stream.  Note that if we have already filled some bytes, we must not read from the stream unless it has bytes available; our contract is to not block if we have bytes on-hand.
        if (lengthFilled > 0 && !CFReadStreamHasBytesAvailable(stream) && CFReadStreamGetStatus(stream) != kCFStreamStatusError) {
            break;
        } else {
            CFIndex bytesToRead = bytesRemainingInChunk > bufferLength ? bufferLength : bytesRemainingInChunk;
            CFIndex numRead = CFReadStreamRead(stream, buffer, bytesToRead);
            if (numRead < 0) {
                *error = CFReadStreamGetError(stream);
                errorOccurred = TRUE;
                break;
            } else if (numRead == 0) {
                // Premature end-of-stream
                setParseFailure(httpFilter, error);
                errorOccurred = TRUE;
                break;
            } else {
#if defined(DEBUG_FILTER)
                CFDataAppendBytes(httpFilter->_allData, buffer, numRead);
#endif
                buffer += numRead;
                bufferLength -= numRead;
                lengthFilled += numRead;
                httpFilter->processedBytes += numRead;
            }
        }
    }
    if (errorOccurred) {
        return -1;
    } else {
        return lengthFilled;
    }
}

static CFIndex doPlainRead(_CFHTTPFilter *httpFilter, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF) {
    CFIndex result = 0;
    CFReadStreamRef stream = httpFilter->socketStream.r;
    *atEOF = FALSE;
    error->error = 0;
    if (httpFilter->expectedBytes != WAIT_FOR_END_OF_STREAM && bufferLength > (httpFilter->expectedBytes - httpFilter->processedBytes)) {
        bufferLength = httpFilter->expectedBytes - httpFilter->processedBytes;
    }
    if (bufferLength == 0) {
        // Two possibilities - either there are no more bytes to read, or the client passed in a zero-length buffer.  We don't worry about the latter case - the client shouldn't do that.
        *atEOF = TRUE;
        return 0;
    }
    if (httpFilter->_data) {
        const UInt8 *bytes = CFDataGetBytePtr(httpFilter->_data);
        CFIndex length = CFDataGetLength(httpFilter->_data);
        if (length > bufferLength) {
            memcpy(buffer, bytes, bufferLength);
            setDataForRange(httpFilter, CFRangeMake(bufferLength, length-bufferLength));
            result = bufferLength;
        } else {
            memcpy(buffer, bytes, length);
            CFRelease(httpFilter->_data);
            httpFilter->_data = NULL;
            result = length;
        }
    }
    if (result == 0 || (result < bufferLength && CFReadStreamHasBytesAvailable(stream))) {
        // Attempt to read from the stream; it is an error to block, reading from the stream, if the stream has no bytes currently available and we were able to retrieve at least one byte from our internal storage, above.
        CFIndex bytesRead;
        buffer += result;
        bufferLength -= result;
        bytesRead = CFReadStreamRead(stream, buffer, bufferLength);
        if (bytesRead < 0) {
            result = -1;
            *error = CFReadStreamGetError(stream);
            *atEOF = TRUE;
        } else if (bytesRead == 0) {
            *atEOF = TRUE;
        } else {
#if defined(DEBUG_FILTER)
            CFDataAppendBytes(httpFilter->_allData, buffer, bytesRead);
#endif
            result += bytesRead;
            if (CFReadStreamGetStatus(stream) == kCFStreamStatusAtEnd)  {
                *atEOF = TRUE;
            }
        }
    }
    if (result > 0) {
        // Now do the bookkeeping
        httpFilter->processedBytes += result;
        if (httpFilter->expectedBytes != WAIT_FOR_END_OF_STREAM && httpFilter->processedBytes >= httpFilter->expectedBytes) {
            *atEOF = TRUE;
        }
    }
    return result;
}

static CFIndex httpRdFilterRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info) {
    _CFHTTPFilter *httpFilter = (_CFHTTPFilter *)info;
    Boolean parseSucceeded = TRUE;
    int result;
	__CFSpinLock(&httpFilter->lock);
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterRead(stream = 0x%x, filter = 0x%x) - ", (unsigned)stream, (unsigned)httpFilter);
#endif

    if (__CFBitIsSet(httpFilter->flags, PARSE_FAILED)) {
        error->error = kCFStreamErrorHTTPParseFailure;
        error->domain = kCFStreamErrorDomainHTTP;
        *atEOF = TRUE;
#if defined(LOG_FILTER)
        fprintf(stderr, "returned -1 (error = {%d, %d})\n", (int)error->domain, (int)error->error);
#endif
		__CFSpinUnlock(&httpFilter->lock);
        return -1;
    } else if (__CFBitIsSet(httpFilter->flags, CONNECTION_LOST)) {
        error->error = kCFStreamErrorHTTPConnectionLost;
        error->domain = kCFStreamErrorDomainHTTP;
        *atEOF = TRUE;
#if defined(LOG_FILTER)
        fprintf(stderr, "returned -1 (error = {%d, %d})\n", (int)error->domain, (int)error->error);
#endif
		__CFSpinUnlock(&httpFilter->lock);
        return -1;
    } else if (__CFBitIsSet(httpFilter->flags, AT_MARK)) {
        error->error = _kCFStreamErrorHTTPStreamAtMark;
        error->domain = kCFStreamErrorDomainHTTP;
        *atEOF = TRUE;
#if defined(LOG_FILTER)
        fprintf(stderr, "returned -1 (error = {%d, %d})\n", (int)error->domain, (int)error->error);
#endif
		__CFSpinUnlock(&httpFilter->lock);
        return -1;
    }

    if (httpFilter->expectedBytes == HEADERS_NOT_YET_CHECKED) {
        // Parse the headers out if we haven't already; note that we're supposed to block until we have at least one data byte to return
        parseSucceeded = readHeaderBytes(httpFilter, TRUE, buffer, bufferLength, error);
    }
    if (!parseSucceeded) {
        *atEOF = TRUE;
#if defined(LOG_FILTER)
        fprintf(stderr, "returned -1 (error = {%d, %d})\n", (int)error->domain, (int)error->error);
#endif
		__CFSpinUnlock(&httpFilter->lock);
        return -1; // readHeaderBytes set our error code for us.
    }
    
    if (__CFBitIsSet(httpFilter->flags, IS_CHUNKED)) {
        result = doChunkedRead(httpFilter, buffer, bufferLength, error, atEOF);
    } else {
        result = doPlainRead(httpFilter, buffer, bufferLength, error, atEOF);
    }
    if (*atEOF && !error->error && __CFBitIsSet(httpFilter->flags, MARK_ENABLED)) {
        *atEOF = FALSE;
        __CFBitSet(httpFilter->flags, AT_MARK);
        __CFBitSet(httpFilter->flags, MARK_SIGNALLED);
        CFReadStreamSignalEvent(stream, kCFStreamEventMarkEncountered, NULL);
    } else {
        CFStreamError err = {0, 0};
        if (result > 0 && !*atEOF && httpRdFilterCanReadNoSignal(stream, httpFilter, &err)) {
            CFReadStreamSignalEvent(stream, kCFStreamEventHasBytesAvailable, NULL);
        }
        if (err.error) {
            CFReadStreamSignalEvent(stream, kCFStreamEventErrorOccurred, &err);
        }
    }
#if defined(LOG_FILTER)
        fprintf(stderr, "returned %d (error = {%d, %d})\n", result, (int)error->domain, (int)error->error);
#endif
	__CFSpinUnlock(&httpFilter->lock);
    return result;
}

#define BUFFER_LENGTH (4 * 1024)

static Boolean httpRdFilterCanReadNoSignal(CFReadStreamRef stream, _CFHTTPFilter *httpFilter, CFStreamError *err) {
    UInt8 buffer[BUFFER_LENGTH];
    err->error = 0;

	if (__CFBitIsSet(httpFilter->flags, AT_MARK)) {
        return FALSE;
    }
    if (httpFilter->expectedBytes == HEADERS_NOT_YET_CHECKED) {
        if (!readHeaderBytes(httpFilter, FALSE, buffer, BUFFER_LENGTH, err)) {
            return FALSE;
        }
        if (!CFHTTPMessageIsHeaderComplete(httpFilter->header)) {
            return FALSE;
        } 
    }
    
    // We are safely past the http header; now see if we need to pass a chunk header.  We cannot fold this into the code above because we may be between chunks
    if (__CFBitIsSet(httpFilter->flags, IS_CHUNKED) && (httpFilter->expectedBytes == httpFilter->processedBytes || httpFilter->expectedBytes == MID_CHUNK_HEADER_PARSE)) {
        if (__CFBitIsSet(httpFilter->flags, LAST_CHUNK)) {
            return TRUE;
        } else if (!readChunkedHeader(httpFilter, FALSE, buffer, BUFFER_LENGTH, err)) {
            return FALSE;
        } else if (httpFilter->expectedBytes == MID_CHUNK_HEADER_PARSE) {
            return FALSE;
        }
    }
    if (httpFilter->_data && CFDataGetLength(httpFilter->_data) != 0) {
        return TRUE;
    } else if (httpFilter->expectedBytes == httpFilter->processedBytes) {
        // At EOF
        if (__CFBitIsSet(httpFilter->flags, MARK_ENABLED)) {
            __CFBitSet(httpFilter->flags, AT_MARK);
            return FALSE;
        } else {
            return TRUE;
        }
    } else if (CFReadStreamHasBytesAvailable(httpFilter->socketStream.r)) {
        return TRUE;
    }
    return FALSE;
}

static Boolean httpRdFilterCanRead(CFReadStreamRef stream, void *info) {
    _CFHTTPFilter *httpFilter = (_CFHTTPFilter *)info;
    CFStreamError error;
	Boolean result;
	__CFSpinLock(&httpFilter->lock);
    result = httpRdFilterCanReadNoSignal(stream, httpFilter, &error);
	__CFSpinUnlock(&httpFilter->lock);
    if (error.error) {
        CFReadStreamSignalEvent(stream, kCFStreamEventErrorOccurred, &error);
    }
#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: httpRdFilterCanRead(stream = 0x%x, filter = 0x%x) - returning %s\n", (unsigned)stream, (unsigned)httpFilter, result ? "TRUE" : "FALSE");
#endif
    return result;
}

extern
Boolean _CFHTTPReadStreamIsAtMark(CFReadStreamRef filteredStream) {
    Boolean result;
	_CFHTTPFilter *filter = (_CFHTTPFilter *)CFReadStreamGetInfoPointer(filteredStream);
	__CFSpinLock(&filter->lock);
    result = httpRdFilterAtMark(filter);
	__CFSpinUnlock(&filter->lock);
	return result;
}

extern
void _CFHTTPReadStreamReadMark(CFReadStreamRef fStream) {
    _CFHTTPFilter *httpFilter = (_CFHTTPFilter *)CFReadStreamGetInfoPointer(fStream);
    struct _CFStream *filteredStream = (struct _CFStream *)fStream;
    CFStreamError error = {0, 0};

	__CFSpinLock(&httpFilter->lock);

#if defined(LOG_FILTER)
    fprintf(stderr, "HTTPFilter: _CFHTTPReadStreamReadMark(stream = 0x%x, filter = 0x%x)\n ", (unsigned)fStream, (unsigned)httpFilter);
#endif
    if (__CFBitIsSet(httpFilter->flags, MARK_ENABLED) && __CFBitIsSet(httpFilter->flags, AT_MARK)) {
        CFHTTPMessageRef newHeader;
        CFAllocatorRef alloc = CFGetAllocator(httpFilter->header);
        newHeader = CFHTTPMessageCreateEmpty(alloc, CFHTTPMessageIsRequest(httpFilter->header));
        if (__CFBitIsSet(httpFilter->flags, LAX_PARSING)) {
            _CFHTTPMessageSetLaxParsing(newHeader, TRUE);
        }
        CFRelease(httpFilter->header);
        httpFilter->header = newHeader;
        httpFilter->expectedBytes = HEADERS_NOT_YET_CHECKED;
        httpFilter->processedBytes = 0;
        __CFBitClear(httpFilter->flags, AT_MARK);
        __CFBitClear(httpFilter->flags, MARK_SIGNALLED);
        __CFBitClear(httpFilter->flags, IS_CHUNKED);
        __CFBitClear(httpFilter->flags, FIRST_CHUNK);
        __CFBitClear(httpFilter->flags, LAST_CHUNK);
        __CFBitClear(httpFilter->flags, PARSE_FAILED);
        __CFBitClear(httpFilter->flags, CONNECTION_LOST);
        __CFBitClear(httpFilter->flags, ZERO_LENGTH_RESPONSE_EXPECTED);
#if defined(DEBUG_FILTER)
        CFRelease(httpFilter->_allData);
        httpFilter->_allData = CFDataCreateMutable(NULL, 0);
        if (httpFilter->_data) {
            CFDataAppendBytes(httpFilter->_allData, CFDataGetBytePtr(httpFilter->_data), CFDataGetLength(httpFilter->_data));
        }
#endif    
    }
    if (httpRdFilterCanReadNoSignal(fStream, httpFilter, &error)) {
        _CFReadStreamSignalEventDelayed(fStream, kCFStreamEventHasBytesAvailable, NULL);
    } else if (error.error) {
        if (error.domain == _kCFStreamErrorDomainNativeSockets && (error.error == ECONNRESET || error.error == EPIPE)) {
            error.domain= kCFStreamErrorDomainHTTP;
            error.error = kCFStreamErrorHTTPConnectionLost;
        }
        _CFReadStreamSignalEventDelayed(fStream, kCFStreamEventErrorOccurred, &error);
    } else if (CFReadStreamGetStatus(httpFilter->socketStream.r) == kCFStreamStatusAtEnd) {
        error.domain= kCFStreamErrorDomainHTTP;
        error.error = kCFStreamErrorHTTPConnectionLost;
        _CFReadStreamSignalEventDelayed(fStream, kCFStreamEventErrorOccurred, &error);        
    }

    if (filteredStream->client) {
        filteredStream->client->whatToSignal &= ~kCFStreamEventMarkEncountered;
    }
	
	__CFSpinUnlock(&httpFilter->lock);
}

static CFTypeRef httpRdFilterCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	CFTypeRef result = NULL;
	
	__CFSpinLock(&filter->lock);
	
    if (CFEqual(propertyName, _kCFStreamPropertyHTTPPersistent)) {
        result = (__CFBitIsSet(filter->flags, MARK_ENABLED)) ? kCFBooleanTrue : kCFBooleanFalse;
    } else if ((!CFHTTPMessageIsRequest(filter->header) && CFEqual(propertyName, kCFStreamPropertyHTTPResponseHeader)) || (CFHTTPMessageIsRequest(filter->header) && CFEqual(propertyName, kCFStreamPropertyHTTPRequest))) {
        CFHTTPMessageRef response = filter->header;
        if (CFHTTPMessageIsHeaderComplete(response)) {
            CFRetain(response);
        } else {
            response = NULL;
        }
        result = response;
    } else {
        result = CFReadStreamCopyProperty(filter->socketStream.r, propertyName);
    }
	
	__CFSpinUnlock(&filter->lock);

	return result;
}

static Boolean httpRdFilterSetProperty(CFReadStreamRef stream, CFStringRef propName, CFTypeRef propValue, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;

	__CFSpinLock(&filter->lock);

    if (CFEqual(propName, _kCFStreamPropertyHTTPPersistent)) {
        if (propValue == kCFBooleanTrue) {
            __CFBitSet(filter->flags, MARK_ENABLED);
        } else {
            __CFBitClear(filter->flags, MARK_ENABLED);
        }
		__CFSpinUnlock(&filter->lock);
        return TRUE;
    } else if (propName == _kCFStreamPropertyHTTPZeroLengthResponseExpected) {
        if (propValue == kCFBooleanTrue) {
            __CFBitSet(filter->flags, ZERO_LENGTH_RESPONSE_EXPECTED);
        }
		__CFSpinUnlock(&filter->lock);
        return TRUE;
    } else if (CFEqual(propName, _kCFStreamPropertyHTTPLaxParsing)) {
        if (propValue == kCFBooleanTrue) {
            __CFBitSet(filter->flags, LAX_PARSING);
            if (filter->header) _CFHTTPMessageSetLaxParsing(filter->header, TRUE);
        } else {
            __CFBitClear(filter->flags, LAX_PARSING);
            if (filter->header) _CFHTTPMessageSetLaxParsing(filter->header, FALSE);
        }
		__CFSpinUnlock(&filter->lock);
        return TRUE;
#if defined(__MACH__)
    } else if (CFEqual(propName, kCFStreamPropertySocketSSLContext)) {
        // This must be set on the write filter
		__CFSpinUnlock(&filter->lock);
        return FALSE;
#endif
    } else {
		Boolean result = CFReadStreamSetProperty(filter->socketStream.r, propName, propValue);
		__CFSpinUnlock(&filter->lock);
        return result;
    }
}

static const CFReadStreamCallBacks HTTPReadFilterCallBacks = {1, 
httpRdFilterCreate
, httpRdFilterDealloc
, NULL /*copyDesc*/, httpRdFilterOpen
, NULL /*openCompleted*/, httpRdFilterRead
, NULL /*getBuffer*/, httpRdFilterCanRead
, httpRdFilterClose
, httpRdFilterCopyProperty
, httpRdFilterSetProperty
, NULL /*atEnd*/, httpRdFilterSchedule
, httpRdFilterUnschedule};

CF_EXPORT
CFReadStreamRef CFReadStreamCreateHTTPStream(CFAllocatorRef alloc, CFReadStreamRef readStream, Boolean forResponse) {
    _CFHTTPFilter filter;
    CFReadStreamRef stream;
    memset(&filter, 0, sizeof(_CFHTTPFilter));
    filter.socketStream.r = readStream;
    if (forResponse) {
        filter.header = CFHTTPMessageCreateEmpty(alloc, FALSE);
    } else {
        filter.header = CFHTTPMessageCreateEmpty(alloc, TRUE);
    }
    stream = CFReadStreamCreate(alloc, &HTTPReadFilterCallBacks, &filter);
    CFRelease(filter.header);
    return stream;
}

/******************************/
/*    Write streams           */
/******************************/

static void *httpWrFilterCreate(CFWriteStreamRef stream, void *info) {
    _CFHTTPFilter *oldFilter = (_CFHTTPFilter *)info;
    _CFHTTPFilter *filter = (_CFHTTPFilter *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFHTTPFilter), 0);
	filter->lock = 0;
    filter->header = NULL;
    filter->flags = 0;
    if (__CFBitIsSet(oldFilter->flags, IS_PROXY)) {
        __CFBitSet(filter->flags, IS_PROXY);
    }
    __CFBitSet(filter->flags, FIRST_CHUNK);
    filter->expectedBytes = 0;
    filter->processedBytes = 0;
    filter->_data = NULL;
    filter->socketStream.w = oldFilter->socketStream.w;
    CFRetain(filter->socketStream.w);
    filter->filteredStream.w = stream; // Do not retain; that will introduce a retain loop.
    filter->customSSLContext = NULL;
    return filter;
}

static void httpWrFilterDealloc(CFWriteStreamRef stream, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    if (filter->header) CFRelease(filter->header);
    if (filter->_data) CFRelease(filter->_data);
    CFWriteStreamClose(filter->socketStream.w);
    CFWriteStreamSetClient(filter->socketStream.w, kCFStreamEventNone, NULL, NULL);
    CFRelease(filter->socketStream.w);
    if (filter->customSSLContext) CFRelease(filter->customSSLContext);
    CFAllocatorDeallocate(CFGetAllocator(stream), filter);
}

static void httpWrFilterStreamCallBack(CFWriteStreamRef stream, CFStreamEventType event, void *clientCallBackInfo) {
    CFWriteStreamRef filterStream = (CFWriteStreamRef)clientCallBackInfo;
    _CFHTTPFilter *filter = (_CFHTTPFilter *)CFWriteStreamGetInfoPointer(filterStream);
    switch (event) {
		case kCFStreamEventErrorOccurred: {
			CFStreamError error = CFWriteStreamGetError(stream);
			__CFSpinLock(&filter->lock);
			if (!__CFBitIsSet(filter->flags, HEADER_TRANSMITTED) && __CFBitIsSet(filter->flags, IS_HTTPS_PROXY)) {
				CFHTTPMessageRef proxyResponse = (CFHTTPMessageRef)CFWriteStreamCopyProperty(filter->socketStream.w, kCFStreamPropertyCONNECTResponse);
				int status = CFHTTPMessageGetResponseStatusCode(proxyResponse);
				CFRelease(proxyResponse);
				if (status != 200) {
					if (!filter->processedBytes) {
						if (filter->_data) CFRelease(filter->_data);
						filter->_data = NULL;
						__CFBitSet(filter->flags, HEADER_TRANSMITTED);
						__CFBitSet(filter->flags, HTTPS_PROXY_FAILURE);
					}
					else {
						error.domain = kCFStreamErrorDomainHTTP;
						error.error = _kCFStreamErrorHTTPSProxyFailure;
					}
				}
			}
			__CFSpinUnlock(&filter->lock);
			CFWriteStreamSignalEvent(filterStream, kCFStreamEventErrorOccurred, &error);
			break;
		}
		case kCFStreamEventCanAcceptBytes:
        if (httpWrFilterCanWrite(stream, filter)) {
            CFWriteStreamSignalEvent(filterStream, kCFStreamEventCanAcceptBytes, NULL);
        }
        break;
    default:
        CFWriteStreamSignalEvent(filterStream, event, NULL);
    }
}

static Boolean httpWrFilterOpen(CFWriteStreamRef stream, CFStreamError *errorCode, Boolean *openComplete, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
    CFStreamClientContext clientContext = {0, stream, NULL, NULL, (CFStringRef(*)(void *))CFCopyDescription}; // Do not use CFRetain/CFRelease; they introduce a retain loop
    Boolean result;
    CFStreamStatus status;
	__CFSpinLock(&filter->lock);
    CFWriteStreamSetClient(filter->socketStream.w, kCFStreamEventCanAcceptBytes | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, (CFWriteStreamClientCallBack)httpWrFilterStreamCallBack, &clientContext);
    status = CFWriteStreamGetStatus(filter->socketStream.w);
    if (status == kCFStreamStatusNotOpen) {
        result = CFWriteStreamOpen(filter->socketStream.w); 
    } else {
        result = TRUE;
    }
    if (result) {
        errorCode->error = 0;
        *openComplete = TRUE;
    } else {
        *openComplete = TRUE;
        *errorCode = CFWriteStreamGetError(filter->socketStream.w);
    }
	__CFSpinUnlock(&filter->lock);
	return result;
}

static void httpWrFilterClose(CFWriteStreamRef stream, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    CFWriteStreamClose(filter->socketStream.w);
	__CFSpinUnlock(&filter->lock);
}

static CFStreamError transmitHeader(_CFHTTPFilter *filter, Boolean blockUntilDone) {
    CFStreamError err = {0, 0};
    long long length;
    const UInt8 *bytes;
    Boolean firstTime = TRUE;
    Boolean isFirstWriteOfHeader = FALSE;
	
    CFWriteStreamRef stream = filter->socketStream.w;
    if (__CFBitIsSet(filter->flags, HEADER_TRANSMITTED)) {
        return err;
    }

    if (!filter->header) {
        err.domain = kCFStreamErrorDomainHTTP;
        err.error = kCFStreamErrorHTTPParseFailure;
        return err;
    }

    if (__CFBitIsSet(filter->flags, IS_HTTPS_PROXY)) {
        CFHTTPMessageRef proxyResponse = (CFHTTPMessageRef)CFWriteStreamCopyProperty(filter->socketStream.w, kCFStreamPropertyCONNECTResponse);
        int status = CFHTTPMessageGetResponseStatusCode(proxyResponse);
        CFRelease(proxyResponse);
        if (status != 200) {
			filter->_data = NULL;
			filter->processedBytes = 0;
			__CFBitSet(filter->flags, HEADER_TRANSMITTED);
			__CFBitSet(filter->flags, HTTPS_PROXY_FAILURE);
            return err;
        } else {
            // We have a connection through the proxy, time to switch to SSL
            __CFBitClear(filter->flags, IS_HTTPS_PROXY); // from here out, communicate as if speaking to the remote server.
#if defined(__MACH__)
            if (filter->customSSLContext) {
                if (CFGetTypeID(filter->customSSLContext) == CFDataGetTypeID())
                    CFWriteStreamSetProperty(filter->socketStream.w, kCFStreamPropertySocketSSLContext, filter->customSSLContext);
                else
                    CFWriteStreamSetProperty(filter->socketStream.w, kCFStreamPropertySSLSettings, filter->customSSLContext);
                CFRelease(filter->customSSLContext);
                filter->customSSLContext = NULL;
            } else
#endif /* __MACH__ */
            {
                // Must set the real peer name before turning on SSL
                CFURLRef url = CFHTTPMessageCopyRequestURL(filter->header);
                CFStringRef host = CFURLCopyHostName(url);
                if (host) {
                    CFWriteStreamSetProperty(stream, _kCFStreamPropertySocketPeerName, host);
                    CFRelease(host);
                }
                CFRelease(url);

                CFWriteStreamSetProperty(stream, kCFStreamPropertySocketSecurityLevel, kCFStreamSocketSecurityLevelNegotiatedSSL);
            }
        }
    }

    if (!filter->_data) {
        length = expectedSizeFromHeaders(filter->header);
        if (length != WAIT_FOR_END_OF_STREAM) {
            filter->expectedBytes = length;
            __CFBitClear(filter->flags, IS_CHUNKED);
        } else {
            CFStringRef encodingString = CFHTTPMessageCopyHeaderFieldValue(filter->header, _kCFHTTPFilterTransferEncodingHeader);
            if (!encodingString) {
                CFHTTPMessageSetHeaderFieldValue(filter->header, _kCFHTTPFilterTransferEncodingHeader, _kCFHTTPFilterTransferEncodingChunked2);
            } else {
                if (!CFStringFindWithOptions(encodingString, _kCFHTTPFilterTransferEncodingChunked, CFRangeMake(0, CFStringGetLength(encodingString)), kCFCompareCaseInsensitive, NULL)) {
                    CFMutableStringRef newString = CFStringCreateMutableCopy(NULL, 0, encodingString);
                    CFStringAppend(newString, _kCFHTTPFilterTransferEncodingChunkedSeparator);
                    CFHTTPMessageSetHeaderFieldValue(filter->header, _kCFHTTPFilterTransferEncodingHeader, newString);
                    CFRelease(newString);
                }
                CFRelease(encodingString);
            }
            __CFBitSet(filter->flags, IS_CHUNKED);
            filter->expectedBytes = 0;
        }
        filter->_data = (CFMutableDataRef)_CFHTTPMessageCopySerializedHeaders(filter->header, __CFBitIsSet(filter->flags, IS_PROXY));  
        isFirstWriteOfHeader = TRUE;
        filter->processedBytes = 0;
    }

    if (!filter->_data) {
        err.domain = kCFStreamErrorDomainHTTP;
        err.error = kCFStreamErrorHTTPParseFailure;
        return err;
    }
    length = CFDataGetLength(filter->_data);
    bytes = CFDataGetBytePtr(filter->_data);
    while (filter->processedBytes < length && (blockUntilDone || CFWriteStreamCanAcceptBytes(stream))) {
        CFIndex bytesWritten = CFWriteStreamWrite(stream, bytes + filter->processedBytes, length - filter->processedBytes);
        if (bytesWritten < 0) {
            err = CFWriteStreamGetError(stream);
            if (isFirstWriteOfHeader && err.domain == _kCFStreamErrorDomainNativeSockets && (err.error == EPIPE || err.error == ECONNRESET)) {
                err.domain = kCFStreamErrorDomainHTTP;
                err.error = kCFStreamErrorHTTPConnectionLost;
            }
            break;
        }  else if (bytesWritten == 0) {
            // Premature end-of-stream
            if (isFirstWriteOfHeader) {
                err.domain = kCFStreamErrorDomainHTTP;
                err.error = kCFStreamErrorHTTPConnectionLost;
            } else {
                setParseFailure(filter, &err);
            }
            break;
        }
        filter->processedBytes += bytesWritten;
        firstTime = FALSE;
        isFirstWriteOfHeader = FALSE;
    }
    if (err.error != 0 || filter->processedBytes >= length) {
        CFRelease(filter->_data);
        filter->_data = NULL;
        filter->processedBytes = 0;
        __CFBitSet(filter->flags, HEADER_TRANSMITTED);
    }
    return err;
}

// CFIndex <= uint64 so no more than 16 characters to encode + 2 for CRLF + 2 for leading CRLF
#define MAX_CHUNK_HEADER_SIZE (20)
static void sendChunkHeader(CFWriteStreamRef stream, CFIndex chunkLength, Boolean firstChunk, CFStreamError *error) {
    // hex representation of chunkLength, followed by CRLF
    UInt8 writeBuffer[MAX_CHUNK_HEADER_SIZE];
    UInt8 *writeBase;
    CFIndex bytesWritten;
    error->error = 0;
    writeBuffer[MAX_CHUNK_HEADER_SIZE - 1] = '\n';
    writeBuffer[MAX_CHUNK_HEADER_SIZE - 2] = '\r';
    writeBase = &(writeBuffer[MAX_CHUNK_HEADER_SIZE-3]); 
    while (chunkLength > 0) {
        int nextDigit = chunkLength & 0xF;
        *writeBase = nextDigit < 10 ? '0' + nextDigit : 'A' + nextDigit - 10;
        writeBase --;
        chunkLength = chunkLength >> 4;
    }
    if (firstChunk) {
        writeBase ++;
    } else {
        *writeBase = '\n';
        writeBase --;
        *writeBase = '\r';
    }
    while (writeBase < writeBuffer + MAX_CHUNK_HEADER_SIZE) {
        bytesWritten = CFWriteStreamWrite(stream, writeBase, writeBuffer + MAX_CHUNK_HEADER_SIZE - writeBase);
        if (bytesWritten < 0) {
            *error = CFWriteStreamGetError(stream);
            break;
        } else if (bytesWritten == 0) {
            // Premature EOF; can we come up with a better error code?
            error->domain = kCFStreamErrorDomainHTTP;
            error->error = kCFStreamErrorHTTPParseFailure;
            break;
        } else {
            writeBase += bytesWritten;
        }
    }
}

static CFIndex doChunkedWrite(const UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, _CFHTTPFilter *filter) {
    CFIndex totalBytesWritten = 0;
    CFWriteStreamRef stream = filter->socketStream.w;
    if (filter->expectedBytes <= filter->processedBytes) {
        // At a chunk boundary; emit the header for this chunk
        sendChunkHeader(stream, bufferLength, __CFBitIsSet(filter->flags, FIRST_CHUNK), error);
        __CFBitClear(filter->flags, FIRST_CHUNK);
        if (error->error != 0) {
            // Error occurred while sending the header
            return -1;
        }
        filter->expectedBytes = bufferLength;
        filter->processedBytes = 0;
    }
    
    while (filter->expectedBytes > filter->processedBytes && (totalBytesWritten == 0 || CFWriteStreamCanAcceptBytes(stream))) {
        int bytesToWrite = filter->expectedBytes - filter->processedBytes > bufferLength ? bufferLength : filter->expectedBytes - filter->processedBytes;
        int bytesWritten = CFWriteStreamWrite(stream, buffer, bytesToWrite);
        if (bytesWritten < 0) {
            *error = CFWriteStreamGetError(stream);
            return -1;
        } else if (bytesWritten == 0) {
            // EOF on stream
            break;
        }
        filter->processedBytes += bytesWritten;
        totalBytesWritten += bytesWritten;
        buffer += bytesWritten;
        bufferLength -= bytesWritten;
    }
    if (bufferLength == 0 || !CFWriteStreamCanAcceptBytes(stream)) {
        return totalBytesWritten;
    } else {
        CFIndex newBytesWritten = doChunkedWrite(buffer, bufferLength, error, filter);
        if (newBytesWritten < 0) {
            // Error occurred; error has already been set
            return -1;
        } else {
            return newBytesWritten + totalBytesWritten;
        }
    }
    
}

static CFIndex httpWrFilterWrite(CFWriteStreamRef stream, const UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
    error->error = 0;
	__CFSpinLock(&filter->lock);
    if (__CFBitIsSet(filter->flags, HTTPS_PROXY_FAILURE)) {
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = _kCFStreamErrorHTTPSProxyFailure;
		__CFSpinUnlock(&filter->lock);
        return -1;
    }
    if (__CFBitIsSet(filter->flags, MARK_ENABLED) && __CFBitIsSet(filter->flags, AT_MARK)) {
        // it is an error to try to write after sending the mark, but before resetting the outgoing message. 
        error->error = _kCFStreamErrorHTTPStreamAtMark;
        error->domain = kCFStreamErrorDomainHTTP;
		__CFSpinUnlock(&filter->lock);
        return -1;
    }
    if (!__CFBitIsSet(filter->flags, HEADER_TRANSMITTED)) {
        *error = transmitHeader(filter, TRUE);
    }
    if (error->error != 0) {
		__CFSpinUnlock(&filter->lock);
        return -1;
    }
    if (__CFBitIsSet(filter->flags, IS_CHUNKED)) {
        CFIndex result = doChunkedWrite(buffer, bufferLength, error, filter);
		__CFSpinUnlock(&filter->lock);
		return result;
    } else if (filter->processedBytes >= filter->expectedBytes) {
		__CFSpinUnlock(&filter->lock);
        return 0;
    } else {
        CFIndex bytesWritten;
        if (bufferLength > filter->expectedBytes - filter->processedBytes) {
            // Do not allow more than the promised number of bytes to be written
            bufferLength = filter->expectedBytes - filter->processedBytes;
        }
        bytesWritten = CFWriteStreamWrite(filter->socketStream.w, buffer, bufferLength);
        if (bytesWritten < 0) {
            *error = CFWriteStreamGetError(filter->socketStream.w);
			__CFSpinUnlock(&filter->lock);
            return -1;
        } else {
            filter->processedBytes += bytesWritten;
			__CFSpinUnlock(&filter->lock);
            return bytesWritten;
        }
    }
}
 
static Boolean httpWrFilterCanWrite(CFWriteStreamRef stream, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    if (CFWriteStreamGetStatus(filter->socketStream.w) == kCFStreamStatusError) {
        CFStreamError err = CFWriteStreamGetError(filter->socketStream.w);
		__CFSpinUnlock(&filter->lock);
        CFWriteStreamSignalEvent(stream, kCFStreamEventErrorOccurred, &err);
        return FALSE;
    }
    if (!CFWriteStreamCanAcceptBytes(filter->socketStream.w)) {
		__CFSpinUnlock(&filter->lock);
		return FALSE;
	}
    if (__CFBitIsSet(filter->flags, MARK_ENABLED) && __CFBitIsSet(filter->flags, AT_MARK)) {
		__CFSpinUnlock(&filter->lock);
		return FALSE; // Refuse to write bytes until the header has been reset
	}
    if (!filter->header) {
		__CFSpinUnlock(&filter->lock);
		return FALSE; // Only way this can happen is if we've not yet been given the first request header; we refuse to write until the first request has been given to us and been sent along.
    }
	if (!__CFBitIsSet(filter->flags, HEADER_TRANSMITTED)) {
        CFStreamError error  = transmitHeader(filter, FALSE);
        if (error.error != 0) {
			__CFSpinUnlock(&filter->lock);
            CFWriteStreamSignalEvent(stream, kCFStreamEventErrorOccurred, &error);
            return FALSE;
        } 
    }
    if (!__CFBitIsSet(filter->flags, HEADER_TRANSMITTED) || !CFWriteStreamCanAcceptBytes(filter->socketStream.w)) {
		__CFSpinUnlock(&filter->lock);
        return FALSE;
    }
	__CFSpinUnlock(&filter->lock);
    return TRUE;
}

// !!!  This should take place at close!
static void writeChunkTrailer(CFWriteStreamRef stream, _CFHTTPFilter *filter) {
    CFWriteStreamWrite(stream, (const UInt8*)"\r\n0\r\n\r\n", 7);
}

extern
void _CFHTTPWriteStreamWriteMark(CFWriteStreamRef filteredStream) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)CFWriteStreamGetInfoPointer(filteredStream);

    if (__CFBitIsSet(filter->flags, MARK_ENABLED)) {
        __CFBitSet(filter->flags, AT_MARK);
        if (filter->header && __CFBitIsSet(filter->flags, IS_CHUNKED)) {
            // Finish off prior request
            writeChunkTrailer(filter->socketStream.w, filter);
        }
    }
    __CFBitSet(filter->flags, FIRST_CHUNK);

    if (httpWrFilterCanWrite(filteredStream, (void *)filter)) {
        _CFWriteStreamSignalEventDelayed(filteredStream, kCFStreamEventCanAcceptBytes, NULL);
    }
}

static CFTypeRef httpWrFilterCopyProperty(CFWriteStreamRef stream, CFStringRef propertyName, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	CFTypeRef result = NULL;
	
	__CFSpinLock(&filter->lock);
    
	// Pointer equality is fine on this one.
	if (propertyName == _kCFStreamPropertyHTTPSProxyHoldYourFire) {
		if (__CFBitIsSet(filter->flags, HTTPS_PROXY_FAILURE)) result = kCFBooleanTrue;
	} else if (CFEqual(propertyName, _kCFStreamPropertyHTTPPersistent)) {
        result = __CFBitIsSet(filter->flags, MARK_ENABLED) ? kCFBooleanTrue : kCFBooleanFalse;
    } else if (filter->header && ((CFHTTPMessageIsRequest(filter->header) && CFEqual(propertyName, kCFStreamPropertyHTTPRequest)) || (!CFHTTPMessageIsRequest(filter->header) && CFEqual(propertyName, kCFStreamPropertyHTTPResponseHeader)))) {
        CFRetain(filter->header);
        result = filter->header;
#if defined(__MACH__)
    } else if (CFEqual(propertyName, kCFStreamPropertySocketSSLContext)) {
        if (__CFBitIsSet(filter->flags, FIRST_HEADER_SEEN) && !__CFBitIsSet(filter->flags, IS_HTTPS_PROXY)) {
            result = CFWriteStreamCopyProperty(filter->socketStream.w, propertyName);
        } else {
            if (filter->customSSLContext) CFRetain(filter->customSSLContext);
            result = filter->customSSLContext;
        }
#endif
    } else {
        result = CFWriteStreamCopyProperty(filter->socketStream.w, propertyName);
    }
	__CFSpinUnlock(&filter->lock);
	return result;
}

static void prepareHTTPSProxy(_CFHTTPFilter *filter) {
    
    CFStringRef header = CFHTTPMessageCopyHeaderFieldValue(filter->header, _kCFHTTPFilterProxyAuthorizationHeader);
    if (header) {
        CFHTTPMessageSetHeaderFieldValue(filter->header, _kCFHTTPFilterProxyAuthorizationHeader, NULL);
        CFRelease(header);
    }

	if (!__CFBitIsSet(filter->flags, STRIP_PROXY_AUTH)) {
		 __CFBitClear(filter->flags, IS_PROXY);
		__CFBitSet(filter->flags, IS_HTTPS_PROXY);
		__CFBitSet(filter->flags, STRIP_PROXY_AUTH);
	}
}

static Boolean httpWrFilterSetProperty(CFWriteStreamRef stream, CFStringRef propName, CFTypeRef propValue, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    if (CFEqual(propName, _kCFStreamPropertyHTTPPersistent)) {
        if (propValue == kCFBooleanTrue) {
            __CFBitSet(filter->flags, MARK_ENABLED);
        } else {
            __CFBitClear(filter->flags, MARK_ENABLED);
        }
		__CFSpinUnlock(&filter->lock);
        return TRUE;
    } else if ((filter->header == NULL || (__CFBitIsSet(filter->flags, MARK_ENABLED) && __CFBitIsSet(filter->flags, AT_MARK))) && CFEqual(propName, _kCFStreamPropertyHTTPNewHeader) && CFGetTypeID(propValue) == CFHTTPMessageGetTypeID()) {
		CFHTTPMessageRef msg = (CFHTTPMessageRef)propValue;
		CFRetain(msg);
		if (filter->header) CFRelease(filter->header);
		filter->header = msg;
		filter->expectedBytes = 0;
		filter->processedBytes = 0;
		if (filter->_data) {
			CFRelease(filter->_data);
			filter->_data = NULL;
		}
		__CFBitClear(filter->flags, IS_CHUNKED);
		__CFBitClear(filter->flags, DATA_IS_MUTABLE);
		__CFBitClear(filter->flags, AT_MARK);
		__CFBitClear(filter->flags, HEADER_TRANSMITTED);
		__CFBitClear(filter->flags, HTTPS_PROXY_FAILURE);
		
		if ((__CFBitIsSet(filter->flags, STRIP_PROXY_AUTH) || __CFBitIsSet(filter->flags, IS_PROXY)) && CFHTTPMessageIsRequest(msg)) {
			// This is the first header; we need to check if we're talking to an HTTPS proxy
			CFURLRef url = CFHTTPMessageCopyRequestURL(msg);
			CFStringRef scheme = CFURLCopyScheme(url);
			CFRelease(url);
			if (CFEqual(scheme, _kCFHTTPFilterHTTPSScheme)) {
				prepareHTTPSProxy(filter);
			}
			CFRelease(scheme);
		}
		
		if (!__CFBitIsSet(filter->flags, FIRST_HEADER_SEEN)) {
			__CFBitSet(filter->flags, FIRST_HEADER_SEEN);
#if defined(__MACH__)
			if (filter->customSSLContext && !__CFBitIsSet(filter->flags, IS_HTTPS_PROXY)) {
				if (CFGetTypeID(filter->customSSLContext) == CFDataGetTypeID())
					CFWriteStreamSetProperty(filter->socketStream.w, kCFStreamPropertySocketSSLContext, filter->customSSLContext);
				else
					CFWriteStreamSetProperty(filter->socketStream.w, kCFStreamPropertySSLSettings, filter->customSSLContext);
				CFRelease(filter->customSSLContext);
				filter->customSSLContext = NULL;
			}
#endif
		}
		
		if (CFWriteStreamCanAcceptBytes(filter->socketStream.w)) {
			transmitHeader(filter, FALSE);
		}
		
		__CFSpinUnlock(&filter->lock);
		return TRUE;
#if defined(__MACH__)
    } else if ((CFEqual(propName, kCFStreamPropertySocketSSLContext) || CFEqual(propName, kCFStreamPropertySSLSettings)) && (!__CFBitIsSet(filter->flags, FIRST_HEADER_SEEN) || __CFBitIsSet(filter->flags, IS_HTTPS_PROXY))) {
        if (propValue) CFRetain(propValue);
        if (filter->customSSLContext) {
            CFRelease(filter->customSSLContext);
        }
        filter->customSSLContext = propValue;
		__CFSpinUnlock(&filter->lock);
        return TRUE;
#endif
    } else {
        Boolean result = CFWriteStreamSetProperty(filter->socketStream.w, propName, propValue);
		__CFSpinUnlock(&filter->lock);
		return result;
    }
}

static void httpWrFilterSchedule(CFWriteStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    CFWriteStreamScheduleWithRunLoop(filter->socketStream.w, runLoop, runLoopMode);
	__CFSpinUnlock(&filter->lock);
}

static void httpWrFilterUnschedule(CFWriteStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPFilter *filter = (_CFHTTPFilter *)info;
	__CFSpinLock(&filter->lock);
    CFWriteStreamUnscheduleFromRunLoop(filter->socketStream.w, runLoop, runLoopMode);
	__CFSpinUnlock(&filter->lock);
}

static const CFWriteStreamCallBacks httpFilteredStreamCBs = {1, 
httpWrFilterCreate, 
httpWrFilterDealloc, NULL /* copyDesc */, 
httpWrFilterOpen, NULL /* openCompleted */, 
httpWrFilterWrite, 
httpWrFilterCanWrite, 
httpWrFilterClose, 
httpWrFilterCopyProperty, 
httpWrFilterSetProperty, NULL, 
httpWrFilterSchedule, 
httpWrFilterUnschedule};

CF_EXPORT
CFWriteStreamRef CFWriteStreamCreateHTTPStream(CFAllocatorRef alloc, CFHTTPMessageRef header, Boolean isProxy, CFWriteStreamRef socketStream) {
    _CFHTTPFilter filter;
    CFWriteStreamRef stream;
    memset(&filter, 0, sizeof(_CFHTTPFilter));
    if (isProxy) {
        __CFBitSet(filter.flags, IS_PROXY);
    }
    filter.socketStream.w = socketStream;
    stream = CFWriteStreamCreate(alloc, &httpFilteredStreamCBs, &filter);
    if (header) {
        CFWriteStreamSetProperty(stream, _kCFStreamPropertyHTTPNewHeader, header);
    }
    return stream;
}
