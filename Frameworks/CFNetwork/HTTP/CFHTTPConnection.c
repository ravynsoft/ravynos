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
#include "CFHTTPConnectionInternal.h"
#include "CFHTTPInternal.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

const SInt32 kCFStreamErrorHTTPConnectionLost = -4;

extern void _CFSocketStreamCreatePair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle s,
									  const CFSocketSignature* sig, CFReadStreamRef* readStream, CFWriteStreamRef* writeStream);

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFStreamSocketCreatedCallBack			CFSTR("_kCFStreamSocketCreatedCallBack")
#define _kCFHTTPConnectionHEADMethod			CFSTR("HEAD")
#define _kCFHTTPConnectionDescribeFormat		CFSTR("<HTTP stream context 0x%x>{url = %@, state = %d, conn=0x%x}")
#define _kCFHTTPConnectionPrivateRunLoopMode	CFSTR("_kCFHTTPConnectionPrivateRunLoopMode")
#define _kCFHTTPSScheme							CFSTR("https")
#define _kCFNTLMMethod							CFSTR("NTLM")
#define _kCFHTTPStreamProxyAuthorizationHeader	CFSTR("Proxy-Authorization")
#define _kCFHTTPStreamProxyConnectionHeader		CFSTR("Proxy-Connection")
#define _kCFHTTPStreamConnectionKeepAlive		CFSTR("keep-alive")
#else
static CONST_STRING_DECL(_kCFStreamSocketCreatedCallBack, "_kCFStreamSocketCreatedCallBack")
static CONST_STRING_DECL(_kCFHTTPConnectionHEADMethod, "HEAD")
static CONST_STRING_DECL(_kCFHTTPConnectionDescribeFormat, "<HTTP stream context 0x%x>{url = %@, state = %d, conn=0x%x}")
static CONST_STRING_DECL(_kCFHTTPConnectionPrivateRunLoopMode, "_kCFHTTPConnectionPrivateRunLoopMode")
static CONST_STRING_DECL(_kCFHTTPSScheme, "https")
static CONST_STRING_DECL(_kCFNTLMMethod, "NTLM")
static CONST_STRING_DECL(_kCFHTTPStreamProxyAuthorizationHeader, "Proxy-Authorization")
static CONST_STRING_DECL(_kCFHTTPStreamProxyConnectionHeader, "Proxy-Connection")
static CONST_STRING_DECL(_kCFHTTPStreamConnectionKeepAlive, "keep-alive")
#endif	/* __CONSTANT_CFSTRINGS__ */


static inline int _CFHTTPStreamInfoGetState(_CFHTTPStreamInfo *info) {
    return __CFBitfieldGetValue(info->flags, MAX_STATE_BIT, MIN_STATE_BIT);
}

static void dequeueFromConnection(_CFHTTPStreamInfo *streamInfo) {
    // Guard against re-entrancy; _CFNetConnectionDequeue may end up re-entering us and we don't want to to attempt multiple dequeues from the same connection.  Hence the shuffle below with req->conn and conn.
    if (streamInfo->conn) {
        _CFNetConnectionRef conn = streamInfo->conn;
        streamInfo->conn = NULL;
        if (!_CFNetConnectionDequeue(conn, streamInfo) && _CFNetConnectionWillEnqueueRequests(conn)) {
            _CFHTTPStreamInfo *zombie = createZombieDouble(CFGetAllocator(conn), streamInfo, conn);
            if (!zombie) {
                // We're doomed....  We can't dequeue, and we can't replace ourselves....
                streamInfo->conn = conn;
                __CFBitSet(streamInfo->flags, IS_ZOMBIE);
                return;
            } else {
                _CFNetConnectionReplaceRequest(conn, streamInfo, zombie);
            }
        }
        CFRelease(conn);
    }
}

static void destroyStreamInfo(CFAllocatorRef alloc, _CFHTTPStreamInfo *streamInfo) {
    if (streamInfo->conn) dequeueFromConnection(streamInfo);
    CFRelease(streamInfo->request);
    if (streamInfo->responseHeaders) CFRelease(streamInfo->responseHeaders);
    if (streamInfo->requestPayload) {
        CFReadStreamClose(streamInfo->requestPayload);
        CFReadStreamSetClient(streamInfo->requestPayload, 0, NULL, NULL);
        CFRelease(streamInfo->requestPayload);
    }
	if (streamInfo->peerCertificates)
		CFRelease(streamInfo->peerCertificates);
	if (streamInfo->clientCertificates)
		CFRelease(streamInfo->clientCertificates);
	if (streamInfo->clientCertificateState)
		CFRelease(streamInfo->clientCertificateState);
    // Do NOT release streamInfo->stream unless we are a zombie; we don't have a reference
    if (__CFBitIsSet(streamInfo->flags, IS_ZOMBIE) && streamInfo->stream) CFRelease(streamInfo->stream);
    if (streamInfo->requestFragment) CFRelease(streamInfo->requestFragment);
    if (streamInfo->stateChangeSource) CFRelease(streamInfo->stateChangeSource);
    
    CFAllocatorDeallocate(alloc, streamInfo); 
}

static _CFHTTPStreamInfo *createZombieDouble(CFAllocatorRef alloc, _CFHTTPStreamInfo *orig, _CFNetConnectionRef conn) {
    _CFHTTPStreamInfo *zombie;
    CFArrayRef origRLArray;
    zombie = CFAllocatorAllocate(alloc, sizeof(_CFHTTPStreamInfo), 0);
    if (!zombie) return NULL;
    zombie->flags = orig->flags;
    __CFBitSet(zombie->flags, IS_ZOMBIE);
    zombie->conn = conn;
    CFRetain(conn);
    zombie->responseHeaders = NULL;
    zombie->requestBytesWritten = orig->requestBytesWritten;
    zombie->stateChangeSource = NULL;
	zombie->peerCertificates = NULL;
	zombie->clientCertificates = NULL;
	zombie->clientCertificateState = NULL;
    // Sadly, the zombie needs the original request in case there was auth on it; we may need to advance the state of the auth token when our response comes in.
    zombie->request = orig->request;
    CFRetain(zombie->request);
	
    // For both of these, we want to transfer ownership to the zombie.  The original will have to deal without.
    zombie->requestFragment = orig->requestFragment;
    if (zombie->requestFragment) orig->requestFragment = NULL;
    if (orig->requestPayload) {
        CFStreamClientContext ctxt = {0, zombie, NULL, NULL, NULL};
        zombie->requestPayload = orig->requestPayload;
        orig->requestPayload = NULL;
        CFReadStreamSetClient(zombie->requestPayload, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, httpRequestPayloadCallBack, &ctxt);
    } else {
        zombie->requestPayload = NULL;
    }
    
    // This is kinda ugly, but the zombie needs to know where it should schedule/unschedule, and the usual
    // way to do that is to look at its response stream.  So, we create a dummy response stream and schedule
    // it wherever orig->responseStream is scheduled.  Since we never open the stream, life should be good....
    zombie->stream = CFReadStreamCreateWithBytesNoCopy(alloc, (const UInt8*)"dummy zombie stream", strlen("dummy zombie stream"), kCFAllocatorNull);
    origRLArray = _CFReadStreamGetRunLoopsAndModes(orig->stream);
    if (origRLArray) {
        CFIndex i, c = CFArrayGetCount(origRLArray);
        for (i = 0; i + 1 < c; i += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(origRLArray, i);
            CFStringRef mode = CFArrayGetValueAtIndex(origRLArray, i + 1);
            CFReadStreamScheduleWithRunLoop(zombie->stream, rl, mode);
        }
    }
    return zombie;
}


static const _CFNetConnectionCallBacks HTTPConnectionCallBacks =  {
  0,
  httpConnectionCreate,
  httpConnectionFinalize,
  httpConnectionCreateStreams,
  httpConnectionStateChanged,
  httpConnectionTransmitRequest,
  httpConnectionReceiveResponse,
  httpConnectionResponseStreamCB,
  httpConnectionRequestStreamCB,
  httpConnectionRLArrayForRequest
};

static const CFReadStreamCallBacks HTTPStreamCallBacks = {
    1, 
    httpStreamCreate,
    httpStreamFinalize,
    httpStreamCopyDescription,
    httpStreamOpen,
    httpStreamOpenCompleted,
    httpStreamRead,
    NULL,
    httpStreamCanRead,
    httpStreamClose,
    httpStreamCopyProperty,
    httpStreamSetProperty,
    NULL,
    httpStreamSchedule,
    httpStreamUnschedule
};


CFHTTPConnectionRef CFHTTPConnectionCreate(CFAllocatorRef alloc, CFStringRef host, SInt32 port, UInt32 connectionType, CFDictionaryRef streamProperties) {
    _CFHTTPConnectionInfo info = {host, port, connectionType, streamProperties}; 
    _CFNetConnectionRef conn = _CFNetConnectionCreate(alloc, &info, &HTTPConnectionCallBacks, FALSE);
    return conn;
}

void CFHTTPConnectionSetShouldPipeline(CFHTTPConnectionRef conn, Boolean shouldPipeline) {
    _CFNetConnectionSetShouldPipeline((_CFNetConnectionRef)conn, shouldPipeline);
}

void CFHTTPConnectionLost(CFHTTPConnectionRef conn) {
    _CFNetConnectionLost((_CFNetConnectionRef)conn);
}

void CFHTTPConnectionInvalidate(CFHTTPConnectionRef conn, CFStreamError *err) {
    _CFNetConnectionErrorOccurred((_CFNetConnectionRef)conn, err);
}

CFAbsoluteTime CFHTTPConnectionGetLastAccessTime(CFHTTPConnectionRef connection) {
    return _CFNetConnectionGetLastAccessTime((_CFNetConnectionRef)connection);
}

Boolean CFHTTPConnectionAcceptsRequests(CFHTTPConnectionRef conn) {
    return _CFNetConnectionWillEnqueueRequests((_CFNetConnectionRef)conn);
}

int CFHTTPConnectionGetQueueDepth(CFHTTPConnectionRef conn) {
    return _CFNetConnectionGetQueueDepth((_CFNetConnectionRef)conn);
}

static const void *httpConnectionCreate(CFAllocatorRef alloc, const void *info) {
    _CFHTTPConnectionInfo *newInfo = CFAllocatorAllocate(alloc, sizeof(_CFHTTPConnectionInfo), 0);
    _CFHTTPConnectionInfo *oldInfo = (_CFHTTPConnectionInfo *)info;
    newInfo->host = oldInfo->host;
    CFRetain(newInfo->host);
    newInfo->port = oldInfo->port;
    newInfo->type = oldInfo->type;
    newInfo->streamProperties = oldInfo->streamProperties;
    if (newInfo->streamProperties)  CFRetain(newInfo->streamProperties);
	newInfo->authentications = CFSetCreateMutable(alloc, 0, &kCFTypeSetCallBacks);
    return newInfo;
}

static void httpConnectionFinalize(CFAllocatorRef alloc, const void *info) {
    _CFHTTPConnectionInfo *connInfo = (_CFHTTPConnectionInfo *)info;
    CFRelease(connInfo->host);
    if (connInfo->streamProperties) CFRelease(connInfo->streamProperties);
	if (connInfo->authentications) {
		CFSetApplyFunction(connInfo->authentications,
						   (CFSetApplierFunction)_CFHTTPAuthenticationDisassociateConnection,
						   connInfo);
		
		CFRelease(connInfo->authentications);
	}
    CFAllocatorDeallocate(alloc, connInfo);
}

static void _CFStreamSocketCreatedCallBack(int fd, void* ctxt) {
	
	int yes = 1;
	
	(void)ctxt;		/* unused */
	
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(yes));
}

static CFStreamError httpConnectionCreateStreams(CFAllocatorRef allocator, const void *info, CFWriteStreamRef *requestStream, CFReadStreamRef *responseStream) {
    _CFHTTPConnectionInfo *connInfo = (_CFHTTPConnectionInfo *)info;
    CFIndex count;
    
    CFReadStreamRef rStream;
    CFWriteStreamRef wStream;
	CFArrayRef callback;
	CFArrayCallBacks cb = {0, NULL, NULL, NULL, NULL};
	const void* values[2] = {_CFStreamSocketCreatedCallBack, NULL};

    CFStreamError err = {0, 0};
    
    // Create the socket streams ourselves
	_CFSocketStreamCreatePair(allocator, connInfo->host, connInfo->port, 0, NULL, &rStream, &wStream);
    if (!rStream) {
        err.domain = kCFStreamErrorDomainPOSIX;
        err.error = ENOMEM;
        return err;
    }
	
	callback = CFArrayCreate(allocator, values, sizeof(values) / sizeof(values[0]), &cb);
	
	if (callback) {
		CFWriteStreamSetProperty(wStream, _kCFStreamSocketCreatedCallBack, callback);
		CFRelease(callback);
	}

    if (connInfo->type == kHTTPS) {
        CFReadStreamSetProperty(rStream, kCFStreamPropertySocketSecurityLevel, kCFStreamSocketSecurityLevelNegotiatedSSL);
    }

    *requestStream = CFWriteStreamCreateHTTPStream(allocator, NULL, (connInfo->type == kHTTPProxy || connInfo->type == kHTTPSProxy), wStream);
    CFWriteStreamSetProperty(*requestStream, _kCFStreamPropertyHTTPPersistent, kCFBooleanTrue);
    CFRelease(wStream);
    *responseStream = CFReadStreamCreateHTTPStream(allocator, rStream, TRUE);
    CFReadStreamSetProperty(*responseStream, _kCFStreamPropertyHTTPPersistent, kCFBooleanTrue);
    CFRelease(rStream);
    
    if (connInfo->streamProperties && (count = CFDictionaryGetCount(connInfo->streamProperties)) > 0) {
        CFStringRef *keys = CFAllocatorAllocate(allocator, sizeof(CFStringRef)*count*2, 0);
        CFTypeRef *values = (CFTypeRef *)(keys + count);
        CFIndex index;
        CFDictionaryGetKeysAndValues(connInfo->streamProperties, (const void **)keys, (const void **)values);
        for (index = 0; index < count; index ++) {
            CFReadStreamSetProperty(*responseStream, keys[index], values[index]);
            CFWriteStreamSetProperty(*requestStream, keys[index], values[index]);
        }
        CFAllocatorDeallocate(allocator, keys);
    }

    return err;
}

static void prepareTransmission(_CFHTTPStreamInfo *streamInfo, CFWriteStreamRef requestStream) {

	int i;
    CFHTTPAuthenticationRef auth[2];
	_CFNetConnectionRef conn = streamInfo->conn;
	Boolean persistent = _CFNetConnectionWillEnqueueRequests(conn);
	_CFHTTPConnectionInfo* identifier = (_CFHTTPConnectionInfo*)_CFNetConnectionGetInfoPointer(conn);
        Boolean forProxy = (identifier->type == kHTTPProxy) || (identifier->type == kHTTPSProxy);
	
    CFStreamClientContext ctxt = {0, streamInfo, NULL, NULL, NULL};
    CFDataRef payload = NULL;
    
    // Set requestPayload properly; clean up the request
    if (__CFBitIsSet(streamInfo->flags, PAYLOAD_IS_DATA) && (payload = CFHTTPMessageCopyBody(streamInfo->request)) != NULL) {
        CFIndex length = CFDataGetLength(payload);
        if (streamInfo->requestPayload) {
            CFReadStreamSetClient(streamInfo->requestPayload, kCFStreamEventNone, NULL, NULL);
            CFReadStreamClose(streamInfo->requestPayload);
            CFRelease(streamInfo->requestPayload);
        }
        
        if (length) {
            streamInfo->requestPayload = CFReadStreamCreateWithBytesNoCopy(CFGetAllocator(payload), CFDataGetBytePtr(payload), length, kCFAllocatorNull);
        } else {
            streamInfo->requestPayload = NULL;
        }
            
        CFRelease(payload); // originalRequest is holding it for us
        cleanUpRequest(streamInfo->request, length, TRUE, forProxy);
    } else if (!streamInfo->requestPayload) {
        cleanUpRequest(streamInfo->request, 0, TRUE, forProxy);
    } else {
        cleanUpRequest(streamInfo->request, -1, TRUE, forProxy);
    }
    
	
	auth[0] = _CFHTTPMessageGetAuthentication(streamInfo->request, FALSE);
	auth[1] = _CFHTTPMessageGetAuthentication(streamInfo->request, TRUE);
	
	for (i = 0; (i < (sizeof(auth) / sizeof(auth[0]))); i++) {
		
		if (!auth[i])
			continue;
		
		if (!persistent) {
			_CFHTTPAuthenticationDisassociateConnection(auth[i], identifier);
			CFSetRemoveValue(identifier->authentications, auth[i]);
		}
		
		else {
			CFStreamError error = _CFHTTPAuthenticationApplyHeaderToRequest(auth[i], streamInfo->request, identifier);
			
			if (!error.error)
				CFSetSetValue(identifier->authentications, auth[i]);
		}
	}
	
    // Set client on payload stream and schedule
    if (streamInfo->requestPayload) {
        CFArrayRef rlArray;
        CFReadStreamSetClient(streamInfo->requestPayload, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, httpRequestPayloadCallBack, &ctxt);
        rlArray = _CFReadStreamGetRunLoopsAndModes(streamInfo->stream);
        if (rlArray) {
            int i, c = CFArrayGetCount(rlArray);
            for (i = 0; i + 1 < c; i += 2) {
                CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(rlArray, i);
                CFStringRef mode = CFArrayGetValueAtIndex(rlArray, i + 1);
                if (streamInfo->requestPayload) {
                    CFReadStreamScheduleWithRunLoop(streamInfo->requestPayload, rl, mode);
                }
            }
        }
        CFReadStreamOpen(streamInfo->requestPayload);
    }
    CFWriteStreamSetProperty(requestStream, _kCFStreamPropertyHTTPNewHeader, streamInfo->request);
    if (!__CFBitIsSet(streamInfo->flags, OPEN_SIGNALLED)) {
        CFReadStreamSignalEvent(streamInfo->stream, kCFStreamEventOpenCompleted, NULL);
        __CFBitSet(streamInfo->flags, OPEN_SIGNALLED);
    }
    if (CFWriteStreamCanAcceptBytes(requestStream)) {
        _CFWriteStreamSignalEventDelayed(requestStream, kCFStreamEventCanAcceptBytes, NULL);
    }
}

static void closeRequestResources(_CFHTTPStreamInfo *streamInfo) {
    if (streamInfo->requestPayload) {
        CFReadStreamClose(streamInfo->requestPayload);
        CFRelease(streamInfo->requestPayload);
        streamInfo->requestPayload = NULL;
    }
    if (streamInfo->requestFragment) {
        CFRelease(streamInfo->requestFragment);
        streamInfo->requestFragment = NULL;
    }
}

static void concludeTransmission(_CFHTTPStreamInfo *streamInfo, CFWriteStreamRef requestStream) {
    closeRequestResources(streamInfo);
    _CFHTTPWriteStreamWriteMark(requestStream);
}

static void prepareReception(_CFHTTPStreamInfo *streamInfo, CFReadStreamRef responseStream) {
    CFStringRef cmd = NULL;
    if (__CFBitIsSet(streamInfo->flags, HAVE_READ_MARK)) return;
    __CFBitSet(streamInfo->flags, HAVE_READ_MARK);
    _CFHTTPReadStreamReadMark(responseStream);
    if ((cmd = CFHTTPMessageCopyRequestMethod(streamInfo->request)) && CFEqual(cmd, _kCFHTTPConnectionHEADMethod)) {
        CFReadStreamSetProperty(responseStream, _kCFStreamPropertyHTTPZeroLengthResponseExpected, kCFBooleanTrue);
    }
    if (cmd) CFRelease(cmd);
    if (CFReadStreamHasBytesAvailable(responseStream)) {
        _CFReadStreamSignalEventDelayed(responseStream, kCFStreamEventHasBytesAvailable, NULL);
    } else if (_CFHTTPReadStreamIsAtMark(responseStream)) {
        _CFReadStreamSignalEventDelayed(responseStream, kCFStreamEventMarkEncountered, NULL);
    }
}

static void addAuthenticationInfoToResponse(_CFHTTPStreamInfo *streamInfo) {
	
	int i;
    CFHTTPAuthenticationRef auth[2];
	_CFNetConnectionRef conn = streamInfo->conn;
	Boolean persistent = _CFNetConnectionWillEnqueueRequests(conn);
	_CFHTTPConnectionInfo* identifier = (_CFHTTPConnectionInfo*)_CFNetConnectionGetInfoPointer(conn);

    CFURLRef requestedURL = CFHTTPMessageCopyRequestURL(streamInfo->request);
    _CFHTTPMessageSetResponseURL(streamInfo->responseHeaders, requestedURL);
    CFRelease(requestedURL);

	auth[0] = _CFHTTPMessageGetAuthentication(streamInfo->request, FALSE);		/* Grab main authentication. */
	auth[1] = _CFHTTPMessageGetAuthentication(streamInfo->request, TRUE);		/* Grab proxy authentication. */
	
    /* Update authentication objects if there are some. */
	for (i = 0; i < (sizeof(auth) / sizeof(auth[0])); i++) {
		
		if (!auth[i])
			continue;
		
		if (!persistent) {
			_CFHTTPAuthenticationDisassociateConnection(auth[i], identifier);
			CFSetRemoveValue(identifier->authentications, auth[i]);
		}
		
		_CFHTTPAuthenticationUpdateFromResponse(auth[i], streamInfo->responseHeaders, identifier);
	}
}

static Boolean persistentIsOK(_CFHTTPStreamInfo *streamInfo, CFReadStreamRef stream) {
    Boolean persistentOK = TRUE;
    
    if (!stream) {
        CFLog(0, CFSTR("Internal consistency check error for http request 0x%x"), streamInfo);
        return TRUE;
    }

    if (streamInfo->responseHeaders) CFRelease(streamInfo->responseHeaders);
    streamInfo->responseHeaders = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);

    __CFBitSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS);
    if (!streamInfo->responseHeaders) {
        // 0.9 response
        return FALSE;
    } else {
        CFStringRef requestMethod = CFHTTPMessageCopyRequestMethod(streamInfo->request);
        int status = CFHTTPMessageGetResponseStatusCode(streamInfo->responseHeaders);
        if ((status >= 100 && status <200) || status == 204 || status == 304 || CFEqual(_kCFHTTPConnectionHEADMethod, requestMethod)) {
            // These are the conditions under which the server must not send further information
            CFRelease(requestMethod);
            __CFBitSet(streamInfo->flags, FORCE_EOF); 
        }
        CFRelease(requestMethod);
        persistentOK = canKeepAlive(streamInfo->responseHeaders, streamInfo->request);
        addAuthenticationInfoToResponse(streamInfo);
    }
    return persistentOK;
}

static void concludeReception(_CFHTTPStreamInfo *streamInfo, CFReadStreamRef responseStream) {
    if (__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
        CFAllocatorRef alloc = CFGetAllocator(streamInfo->stream);
        dequeueFromConnection(streamInfo);
        destroyStreamInfo(alloc, streamInfo);
    } else {
        if (!__CFBitIsSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS)) {
            // This is our last chance; we're about to be disconnected from the connection
            if (!persistentIsOK(streamInfo, responseStream)) {
                _CFNetConnectionLost(streamInfo->conn);
            }
        }
        _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventEndEncountered, NULL);
    }
}

static void haveBeenOrphaned(_CFHTTPStreamInfo *streamInfo, CFStreamError *err, CFHTTPConnectionRef conn) {
    closeRequestResources(streamInfo);
    if (!__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
        if (err->domain == kCFStreamErrorDomainHTTP && err->error == _kCFStreamErrorHTTPSProxyFailure) {
            streamInfo->responseHeaders = (CFHTTPMessageRef)CFWriteStreamCopyProperty(_CFNetConnectionGetRequestStream((_CFNetConnectionRef)conn), kCFStreamPropertyCONNECTResponse);
            addAuthenticationInfoToResponse(streamInfo);
            _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventEndEncountered, NULL);
        } else {
            _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventErrorOccurred, err);
//            fprintf(stderr, "kCFStreamEventErrorOccurred 1 - (%d, %d)\n", (int)err->domain, (int)err->error);
        }
        dequeueFromConnection(streamInfo);
    } else {
        // We're a zombie; allocator is available from the old connection
        dequeueFromConnection(streamInfo);
        destroyStreamInfo(CFGetAllocator(conn), streamInfo);
    }
}

static void httpConnectionStateChanged(void *request, int newState, CFStreamError *err, _CFNetConnectionRef conn, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
//    if (_CFHTTPStreamInfoGetState(streamInfo) != newState-1) {
//        fprintf(stderr, "streamInfo %x going from state %d to %d\n", (unsigned)request, _CFHTTPStreamInfoGetState(streamInfo), newState);
//    }
    __CFBitfieldSetValue(streamInfo->flags, MAX_STATE_BIT, MIN_STATE_BIT, newState);
    
    if (streamInfo->stateChangeSource) {
        CFRunLoopSourceSignal(streamInfo->stateChangeSource);
    }

    switch (newState) {
    case kQueued:
		{
			CFReadStreamRef stream = _CFNetConnectionGetResponseStream(conn);
			
			if (stream)
			{
				CFHTTPAuthenticationRef auth = _CFHTTPMessageGetAuthentication(streamInfo->request, TRUE);
				
				if (auth) {
					CFStringRef method = CFHTTPAuthenticationCopyMethod(auth);
					
					if (method && (CFStringCompare(method, _kCFNTLMMethod, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
						
						CFURLRef url = CFHTTPMessageCopyRequestURL(streamInfo->request);
						CFStringRef scheme = url ? CFURLCopyScheme(url) : NULL;
						
						if (scheme && (CFStringCompare(scheme, _kCFHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
							
							CFAllocatorRef alloc = CFGetAllocator(stream);
							CFMutableDictionaryRef new_value = NULL;
							CFStringRef header;
							CFDictionaryRef property = CFReadStreamCopyProperty(stream, kCFStreamPropertyCONNECTProxy);
							
							_CFHTTPAuthenticationApplyHeaderToRequest(auth, streamInfo->request, _CFNetConnectionGetInfoPointer(conn));
							
							new_value = CFDictionaryCreateMutableCopy(alloc, 0, property);
							CFRelease(property);
							
							header = CFHTTPMessageCopyHeaderFieldValue(streamInfo->request, _kCFHTTPStreamProxyAuthorizationHeader);
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
							
							CFReadStreamSetProperty(stream, kCFStreamPropertyCONNECTProxy, new_value);
							
							CFRelease(new_value);
						}
						
						if (url) CFRelease(url);
						if (scheme) CFRelease(scheme);
					}
					
					if (method) CFRelease(method);
				}
			}
		}
		break;
    case kTransmittingRequest:
        prepareTransmission(streamInfo, _CFNetConnectionGetRequestStream(conn));
        break;
    case kWaitingForResponse:
        concludeTransmission(streamInfo, _CFNetConnectionGetRequestStream(conn));
        break;
    case kReceivingResponse:
        prepareReception(streamInfo, _CFNetConnectionGetResponseStream(conn));
        break;
    case kFinished:
        concludeReception(streamInfo, _CFNetConnectionGetResponseStream(conn));
        break;
    case kOrphaned: {
        haveBeenOrphaned(streamInfo, err, conn);
        break;
    }
    default:
        CFLog(0, CFSTR("Encountered unexpected state %d for request 0x%x"), newState, streamInfo);
    }
}

#define BUF_SIZE (2048)
// Returns whether the request has been fully transmitted
static Boolean transmitRequest(_CFHTTPStreamInfo *streamInfo, CFWriteStreamRef destStream, CFStreamError *error, Boolean blockOnce) {
    Boolean done = FALSE;
    UInt8 buf[BUF_SIZE];
    const UInt8 *bytes;
    error->error = 0;
    
    if (__CFBitIsSet(streamInfo->flags, HAVE_SENT_REQUEST_PAYLOAD)) return TRUE;
    
	if (CFWriteStreamCopyProperty(destStream, _kCFStreamPropertyHTTPSProxyHoldYourFire))
		return TRUE;
    
    // if http->requestPayload is NULL, we still need to wait until the write stream reports canAcceptBytes, because otherwise, our request header hasn't been sent.
    if (streamInfo->requestPayload == NULL) {
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
        CFStreamStatus status = CFReadStreamGetStatus(streamInfo->requestPayload);
        if (status == kCFStreamStatusError) {
            *error = CFReadStreamGetError(streamInfo->requestPayload);
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

    while (!done && (streamInfo->requestFragment || blockOnce || (CFReadStreamHasBytesAvailable(streamInfo->requestPayload) && CFWriteStreamCanAcceptBytes(destStream)))) {
        CFIndex bytesRead, bytesWritten;
        if (streamInfo->requestFragment) {
            bytesRead = CFDataGetLength(streamInfo->requestFragment);
            bytes = CFDataGetBytePtr(streamInfo->requestFragment);
        } else {
            bytesRead = CFReadStreamRead(streamInfo->requestPayload, buf, BUF_SIZE);
            bytes = buf;
            if (bytesRead < 0) {
                *error = CFReadStreamGetError(streamInfo->requestPayload);
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
                streamInfo->requestBytesWritten += bytesWritten;
            }
        }
        if (streamInfo->requestFragment) {
            if (done || bytesRead <= 0) {
                CFRelease(streamInfo->requestFragment);
                streamInfo->requestFragment = NULL;
            } else {
                CFDataRef newData = CFDataCreate(CFGetAllocator(streamInfo->request), bytes, bytesRead);
                CFRelease(streamInfo->requestFragment);
                streamInfo->requestFragment = newData;
            }
        } else if (bytesRead > 0) {
            streamInfo->requestFragment = CFDataCreate(CFGetAllocator(streamInfo->request), bytes, bytesRead);
        }
        blockOnce = FALSE;
    }
    if (!done && !streamInfo->requestFragment && CFReadStreamGetStatus(streamInfo->requestPayload) == kCFStreamStatusAtEnd) {
        done = TRUE;
    }
    if (done) {
        closeRequestResources(streamInfo);
        __CFBitSet(streamInfo->flags, HAVE_SENT_REQUEST_PAYLOAD);
    }
    return done;
}

static void httpConnectionTransmitRequest(void *request, _CFNetConnectionRef connection, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
    CFWriteStreamRef requestStream = _CFNetConnectionGetRequestStream(connection);
    CFStreamError error;
    Boolean requestTransmitted = transmitRequest(streamInfo, requestStream, &error, FALSE);
    if (error.error != 0) {
        if (error.domain == kCFStreamErrorDomainHTTP && error.error == _kCFStreamErrorHTTPSProxyFailure) {
            streamInfo->responseHeaders = (CFHTTPMessageRef)CFWriteStreamCopyProperty(requestStream, kCFStreamPropertyCONNECTResponse);
            addAuthenticationInfoToResponse(streamInfo);
            __CFBitSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS);
            _CFNetConnectionRequestIsComplete(connection, streamInfo);
            _CFNetConnectionLost(connection); // Do not let anyone else use this stream
        } else {
            // Something went wrong
            _CFNetConnectionErrorOccurred(connection, &error);
        }
    } else if (requestTransmitted) {
        // Request completed
        _CFNetConnectionRequestIsComplete(connection, streamInfo);
    }
}

static void httpConnectionReceiveResponse(void *request, _CFNetConnectionRef connection, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
    CFReadStreamRef responseStream = _CFNetConnectionGetResponseStream(connection);
    Boolean done = FALSE;
    int state;

    if (__CFBitIsSet(streamInfo->flags, IN_READ_CALLBACK)) return; // We're here because the client is already reading; httpRequestRead will take care of this....
    // Check if there's anything for us to do....
    if (!responseStream) return;
    state = CFReadStreamGetStatus(responseStream);
    if (state != kCFStreamStatusAtEnd && state != kCFStreamStatusError && !CFReadStreamHasBytesAvailable(responseStream) && !_CFHTTPReadStreamIsAtMark(responseStream)) return;

    if (!__CFBitIsSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS)) {
        if (!persistentIsOK(streamInfo, responseStream)) {
            _CFNetConnectionLost(connection);
        }
        if (__CFBitIsSet(streamInfo->flags, FORCE_EOF)) {
            // We know from the request/response that there will never be any data
            _CFNetConnectionResponseIsComplete(connection, streamInfo);
            done = TRUE;
        }
    }
    if (!done) {
        if (CFReadStreamHasBytesAvailable(responseStream)) {
            if (__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
                // Just plow through the bytes
                UInt8 buf[BUF_SIZE];
                CFIndex bytesRead;
                while (CFReadStreamHasBytesAvailable(responseStream)) {
                    bytesRead = CFReadStreamRead(responseStream, buf, BUF_SIZE);
                    if (bytesRead < 0) {
                        CFStreamError err = CFReadStreamGetError(responseStream);
                        _CFNetConnectionErrorOccurred(connection, &err);
                        break;
                    } else if (bytesRead == 0) {
                        break;
                    }
                }
            } else {
                _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventHasBytesAvailable, NULL);
            }
        } else if (_CFHTTPReadStreamIsAtMark(responseStream)) {
            _CFNetConnectionResponseIsComplete(connection, streamInfo);
        } else if (CFReadStreamGetStatus(responseStream) == kCFStreamStatusAtEnd) {
            _CFNetConnectionLost(connection);
            _CFNetConnectionResponseIsComplete(connection, streamInfo);
        } else if (CFReadStreamGetStatus(responseStream) == kCFStreamStatusError) {
            CFStreamError err = CFReadStreamGetError(responseStream);
            _CFNetConnectionErrorOccurred(connection, &err);
        }
    }
}

static void grabReadStreamProperties(_CFHTTPStreamInfo *streamInfo, CFReadStreamRef stream) {
	if (!streamInfo->peerCertificates) {
		streamInfo->peerCertificates = (CFArrayRef)CFReadStreamCopyProperty(stream, kCFStreamPropertySSLPeerCertificates);
	}
	if (!streamInfo->clientCertificates) {
		streamInfo->clientCertificates = (CFArrayRef)CFReadStreamCopyProperty(stream, _kCFStreamPropertySSLClientCertificates);
	}
	if (!streamInfo->clientCertificateState) {
		streamInfo->clientCertificateState = (CFNumberRef)CFReadStreamCopyProperty(stream, _kCFStreamPropertySSLClientCertificateState);
	}
}

static void grabWriteStreamProperties(_CFHTTPStreamInfo *streamInfo, CFWriteStreamRef stream) {
	if (!streamInfo->peerCertificates) {
		streamInfo->peerCertificates = (CFArrayRef)CFWriteStreamCopyProperty(stream, kCFStreamPropertySSLPeerCertificates);
	}
	if (!streamInfo->clientCertificates) {
		streamInfo->clientCertificates = (CFArrayRef)CFWriteStreamCopyProperty(stream, _kCFStreamPropertySSLClientCertificates);
	}
	if (!streamInfo->clientCertificateState) {
		streamInfo->clientCertificateState = (CFNumberRef)CFWriteStreamCopyProperty(stream, _kCFStreamPropertySSLClientCertificateState);
	}
}

static void httpConnectionResponseStreamCB(void *request, CFReadStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
    Boolean justReadMark = FALSE;
    if (!__CFBitIsSet(streamInfo->flags, HAVE_READ_MARK)) {
        justReadMark = TRUE;
        prepareReception(streamInfo, stream);
    }
    switch (eventType) {
    case kCFStreamEventHasBytesAvailable:
        if (justReadMark) break; // prepareReception just queued another HasBytesAvailable event; wait and field that one instead.
        if (__CFBitIsSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS)) {
            if (!__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
                _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventHasBytesAvailable, NULL);
            } else {
                // Perform the read ourselves
                UInt8 buf[BUF_SIZE];
                while (CFReadStreamHasBytesAvailable(stream) > 0) {
                    CFReadStreamRead(stream, buf, BUF_SIZE);
                }
            }
        } else {
            if (!persistentIsOK(streamInfo, _CFNetConnectionGetResponseStream(conn))) {
                _CFNetConnectionLost(streamInfo->conn);
            }
            if (__CFBitIsSet(streamInfo->flags, FORCE_EOF)) {
                // We know from the request/response that there will never be any data
                _CFNetConnectionResponseIsComplete(streamInfo->conn, streamInfo);
            } else if (!__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
                _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventHasBytesAvailable, NULL);
            } else {
                // Perform the read ourselves
                UInt8 buf[BUF_SIZE];
                while (CFReadStreamHasBytesAvailable(stream)) {
                    CFReadStreamRead(stream, buf, BUF_SIZE);
                }
            }
        }
        break;
    case kCFStreamEventMarkEncountered: 
        if (!justReadMark && streamInfo->conn) {
			_CFNetConnectionResponseIsComplete(streamInfo->conn, streamInfo);
        }
        break;
    case kCFStreamEventEndEncountered:
        _CFNetConnectionLost(streamInfo->conn);
        if (!__CFBitIsSet(streamInfo->flags, IS_ZOMBIE)) {
            _CFReadStreamSignalEventDelayed(streamInfo->stream, kCFStreamEventEndEncountered, NULL);
        }
        break;
    case kCFStreamEventErrorOccurred: {
        // Error out the current response.  Remove from queue.  Restart from the current request
        CFStreamError err = CFReadStreamGetError(stream);
        grabReadStreamProperties(streamInfo, stream);
        _CFNetConnectionErrorOccurred(streamInfo->conn, &err);
        break;
    }
    default:
        ;
    }
}

static void httpConnectionRequestStreamCB(void *request, CFWriteStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
    switch (eventType) {
    case kCFStreamEventCanAcceptBytes: {
        CFStreamError err;
        if (transmitRequest(streamInfo, stream, &err, FALSE)) {
            if (err.error == 0) {
                _CFNetConnectionRequestIsComplete(streamInfo->conn, streamInfo);
            } else {
                _CFNetConnectionErrorOccurred(streamInfo->conn, &err); // This should cause us to send our error when the conneciton turns around and orphans us.
            }
        }
        break;
    }
    case kCFStreamEventErrorOccurred: {
        CFStreamError err = CFWriteStreamGetError(stream);
        grabWriteStreamProperties(streamInfo, stream);
        _CFNetConnectionErrorOccurred(streamInfo->conn, &err);
        break;
    }
    case kCFStreamEventEndEncountered: {
        CFStreamError err = {_kCFStreamErrorDomainNativeSockets, ECONNRESET};
        _CFNetConnectionErrorOccurred(streamInfo->conn, &err);
        break;
    }
    default:
        ;
    }
}

static void httpRequestPayloadCallBack(CFReadStreamRef stream, CFStreamEventType type, void *info) { 
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    switch (type) {
    case kCFStreamEventEndEncountered:
    case kCFStreamEventHasBytesAvailable:
    {
        CFStreamError err;
        CFWriteStreamRef requestStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
        if (requestStream && transmitRequest(streamInfo, requestStream, &err, FALSE)) {
            if (err.error == 0) {
                _CFNetConnectionRequestIsComplete(streamInfo->conn, streamInfo);
            } else {
                _CFNetConnectionErrorOccurred(streamInfo->conn, &err); // This should cause us to send our error when the conneciton turns around and orphans us.
            }
        }
        break;
    }
    case kCFStreamEventErrorOccurred: {
        CFStreamError err = CFReadStreamGetError(stream);
        // theoretically, the connection could unwind and recover from this, but that would require a lot more code....
        _CFNetConnectionErrorOccurred(streamInfo->conn, &err);
        break;
    }
    default:
        ;
    }
}

static CFArrayRef httpConnectionRLArrayForRequest(void *request, _CFNetConnectionRef conn, const void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)request;
    if (!streamInfo->stream) return NULL;
    return _CFReadStreamGetRunLoopsAndModes(streamInfo->stream);
}

CFReadStreamRef CFHTTPConnectionEnqueue(CFHTTPConnectionRef connection, CFHTTPMessageRef request) {
    _CFHTTPStreamInfo info;
    info.flags = 0;
    info.request = request;
    info.responseHeaders = NULL;
    info.requestPayload = NULL;
    info.requestFragment = NULL;
    info.requestBytesWritten = 0;
    info.conn = (_CFNetConnectionRef)connection;
    info.stream = NULL;
    info.stateChangeSource = NULL;
    return CFReadStreamCreate(CFGetAllocator(connection), &HTTPStreamCallBacks, &info);
}

CFReadStreamRef CFHTTPConnectionEnqueueWithBodyStream(CFHTTPConnectionRef connection, CFHTTPMessageRef request, CFReadStreamRef bodyStream) {
    _CFHTTPStreamInfo info;
    info.flags = 0;
    info.request = request;
    info.responseHeaders = NULL;
    info.requestPayload = bodyStream;
    info.requestFragment = NULL;
    info.requestBytesWritten = 0;
    info.conn = (_CFNetConnectionRef)connection;
    info.stream = NULL;
    info.stateChangeSource = NULL;
    return CFReadStreamCreate(CFGetAllocator(connection), &HTTPStreamCallBacks, &info);
}

static void *httpStreamCreate(CFReadStreamRef stream, void *info) {
    _CFHTTPStreamInfo *newInfo, *oldInfo = (_CFHTTPStreamInfo *)info;
    CFAllocatorRef alloc = CFGetAllocator(stream);
    newInfo = CFAllocatorAllocate(alloc, sizeof(_CFHTTPStreamInfo), 0);
    if (!newInfo) return NULL;
    newInfo->flags = 0;
    __CFBitfieldSetValue(newInfo->flags, MAX_STATE_BIT, MIN_STATE_BIT, kNotQueued);
    CFRetain(oldInfo->request);
    newInfo->request = oldInfo->request;
    newInfo->responseHeaders = NULL;
    if (oldInfo->requestPayload) {
        __CFBitSet(newInfo->flags, HAS_PAYLOAD);
        CFRetain(oldInfo->requestPayload);
        newInfo->requestPayload = oldInfo->requestPayload;
    } else {
        CFDataRef body = CFHTTPMessageCopyBody(newInfo->request);
        if (body) {
            __CFBitSet(newInfo->flags, HAS_PAYLOAD);
            __CFBitSet(newInfo->flags, PAYLOAD_IS_DATA);
            CFRelease(body);
        }
        newInfo->requestPayload = NULL;
    }
	newInfo->peerCertificates = NULL;
	newInfo->clientCertificates = NULL;
	newInfo->clientCertificateState = NULL;
    newInfo->requestFragment = NULL;
    newInfo->requestBytesWritten = 0;
    newInfo->stream = stream; // Do not retain.
    newInfo->conn = oldInfo->conn;
    CFRetain(newInfo->conn);
    newInfo->stateChangeSource = NULL;
    return newInfo;
}

static void httpStreamFinalize(CFReadStreamRef stream, void *info) {
    destroyStreamInfo(CFGetAllocator(stream), (_CFHTTPStreamInfo *)info);
}

static CFStringRef httpStreamCopyDescription(CFReadStreamRef stream, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    CFURLRef url = CFHTTPMessageCopyRequestURL(streamInfo->request);
    CFStringRef str = CFStringCreateWithFormat(stream ? CFGetAllocator(stream) : NULL, NULL, _kCFHTTPConnectionDescribeFormat, (unsigned)streamInfo, url, _CFHTTPStreamInfoGetState(streamInfo), (unsigned)streamInfo->conn);
    CFRelease(url);
    return str;
}

static Boolean httpStreamOpen(CFReadStreamRef stream, CFStreamError *error, Boolean *openComplete, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    if (!_CFNetConnectionEnqueue(streamInfo->conn, streamInfo)) {
        *openComplete = TRUE;
        error->domain = kCFStreamErrorDomainHTTP;
        error->error = kCFStreamErrorHTTPConnectionLost;
        return FALSE;
    } else {
        *openComplete = _CFHTTPStreamInfoGetState(streamInfo) > kQueued;
        return TRUE;
    }
}

static Boolean httpStreamOpenCompleted(CFReadStreamRef stream, CFStreamError *error, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    int currentState;
    if (__CFBitIsSet(streamInfo->flags, OPEN_SIGNALLED)) return TRUE;
    currentState = _CFNetConnectionGetState(streamInfo->conn, TRUE, streamInfo);
    return (currentState > kQueued);
}

static CFIndex httpStreamRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info) {
    CFIndex result;
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    enum _CFNetConnectionState state;

    __CFBitSet(streamInfo->flags, IN_READ_CALLBACK);
    state = _CFNetConnectionGetState(streamInfo->conn, TRUE, streamInfo);

    if (state < kReceivingResponse) {
		_CFNetConnectionRef conn = (_CFNetConnectionRef)CFRetain(streamInfo->conn);
        CFRunLoopRef currentRL = CFRunLoopGetCurrent();
        CFStringRef mode = _kCFHTTPConnectionPrivateRunLoopMode;
        CFReadStreamScheduleWithRunLoop(stream, currentRL, mode);
        if (!streamInfo->stateChangeSource) {
            CFRunLoopSourceContext rlsCtxt = {0, streamInfo, NULL, NULL, NULL, NULL, NULL, NULL, NULL, emptyPerform};
            streamInfo->stateChangeSource = CFRunLoopSourceCreate(CFGetAllocator(stream), 0, &rlsCtxt);
        }
        CFRunLoopAddSource(currentRL, streamInfo->stateChangeSource, mode);
    
        while (state < kReceivingResponse) {
            CFRunLoopRunInMode(mode, 1e+20, TRUE);
			state = _CFNetConnectionGetState(conn, TRUE, streamInfo);
			if (!streamInfo->conn)
				break;
			else if (conn != streamInfo->conn) {
				CFRelease(conn);
				conn = (_CFNetConnectionRef)CFRetain(streamInfo->conn);
			}
        }
        CFReadStreamUnscheduleFromRunLoop(stream, currentRL, mode);
		CFRelease(conn);
        CFRunLoopRemoveSource(currentRL, streamInfo->stateChangeSource, mode);
    }

    __CFBitClear(streamInfo->flags, IN_READ_CALLBACK);
    if (state == kFinished) {
        error->error = 0;
        *atEOF = TRUE;
        result = 0;
    } else if (state == kOrphaned) {
        *error = CFReadStreamGetError(stream);
        if (error->error == 0) {
            error->error = ECONNRESET;
            error->domain = _kCFStreamErrorDomainNativeSockets;
        }
        result = -1;
    } else {
        CFReadStreamRef stream = _CFNetConnectionGetResponseStream(streamInfo->conn);
        error->error = 0;
        *atEOF = FALSE;

        if (_CFHTTPReadStreamIsAtMark(stream)) {
            _CFNetConnectionResponseIsComplete(streamInfo->conn, streamInfo);
            *atEOF = TRUE;
            result = 0;
        } else {
            result = CFReadStreamRead(stream, buffer, bufferLength);
            if (result < 0) {
                *error = CFReadStreamGetError(stream);
                _CFNetConnectionErrorOccurred(streamInfo->conn, error);
                return -1;
            } else if (kCFStreamStatusAtEnd == CFReadStreamGetStatus(stream) || _CFHTTPReadStreamIsAtMark(stream)) {
                _CFNetConnectionResponseIsComplete(streamInfo->conn, streamInfo);
                *atEOF = TRUE;
            }
        }

        // If we've never looked at the headers, see if they're available now.  If they are, we may have to take extra action
        if (!__CFBitIsSet(streamInfo->flags, HAVE_CHECKED_RESPONSE_HEADERS)) {
            if (!persistentIsOK(streamInfo, stream)) {
                _CFNetConnectionLost(streamInfo->conn);
            }
            if (__CFBitIsSet(streamInfo->flags, FORCE_EOF)) {
                _CFNetConnectionResponseIsComplete(streamInfo->conn, streamInfo);
                *atEOF = TRUE;
            }
        }
    }
    return result;    
}

static Boolean httpStreamCanRead(CFReadStreamRef myStream, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    int state;
    CFReadStreamRef stream;

    state = _CFNetConnectionGetState(streamInfo->conn, TRUE, streamInfo); // This will drive the connection forward if necessary; it will also update our internal state, so we don't need to do that here.
    if (state > kReceivingResponse) {
        // This stream's been emptied
        return TRUE;
    }  else if (state < kReceivingResponse) {
        return FALSE;
    }
    stream = _CFNetConnectionGetResponseStream(streamInfo->conn);
    if (!CFReadStreamHasBytesAvailable(stream)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void httpStreamClose(CFReadStreamRef stream, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    dequeueFromConnection(streamInfo);
}

static CFTypeRef httpStreamCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    CFTypeRef property = NULL;
    if (CFEqual(propertyName, kCFStreamPropertyHTTPResponseHeader)) {
        property = streamInfo->responseHeaders;
        if (property) CFRetain(property);
	} else if (CFEqual(propertyName, kCFStreamPropertySSLPeerCertificates)) {
		if (streamInfo->peerCertificates)
			property = CFRetain(streamInfo->peerCertificates);
		else if (streamInfo->conn) {
			CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(streamInfo->conn);
			if (rStream) {
				property = CFReadStreamCopyProperty(rStream, propertyName);
			}
			if (!property) {
				CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
				if (wStream) {
					property = CFWriteStreamCopyProperty(wStream, propertyName);
				}
			}
		}
	} else if (CFEqual(propertyName, _kCFStreamPropertySSLClientCertificates)) {
		if (streamInfo->clientCertificates)
			property = CFRetain(streamInfo->clientCertificates);
		else if (streamInfo->conn) {
			CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(streamInfo->conn);
			if (rStream) {
				property = CFReadStreamCopyProperty(rStream, propertyName);
			}
			if (!property) {
				CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
				if (wStream) {
					property = CFWriteStreamCopyProperty(wStream, propertyName);
				}
			}
		}
	} else if (CFEqual(propertyName, _kCFStreamPropertySSLClientCertificateState)) {
		if (streamInfo->clientCertificateState)
			property = CFRetain(streamInfo->clientCertificateState);
		else if (streamInfo->conn) {
			CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(streamInfo->conn);
			if (rStream) {
				property = CFReadStreamCopyProperty(rStream, propertyName);
			}
			if (!property) {
				CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
				if (wStream) {
					property = CFWriteStreamCopyProperty(wStream, propertyName);
				}
			}
		}
    } else if (CFEqual(propertyName, kCFStreamPropertyHTTPRequestBytesWrittenCount)) {
        property = CFNumberCreate(CFGetAllocator(stream), kCFNumberLongLongType, &(streamInfo->requestBytesWritten));
    } else if (CFEqual(propertyName, _kCFStreamPropertyHTTPConnection)) {
        property = streamInfo->conn;
        if (property) CFRetain(property);
    } else if (streamInfo->conn) {
        CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(streamInfo->conn);
        if (rStream) {
            property = CFReadStreamCopyProperty(rStream, propertyName);
        }
        if (!property) {
            CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
            if (wStream) {
                property = CFWriteStreamCopyProperty(wStream, propertyName);
            }
        }
    } else {
        property = NULL;
    }
    return property;
}

static Boolean httpStreamSetProperty(CFReadStreamRef stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    if (CFReadStreamGetStatus(stream) > kCFStreamStatusNotOpen) return FALSE;
    if (CFEqual(propertyName, kCFStreamPropertySocketSecurityLevel) ||
               CFEqual(propertyName, kCFStreamPropertyShouldCloseNativeSocket)) {
        // We own these (socket) properties; prevent the client from setting them
        return FALSE;
    } else if (streamInfo->conn) {
        CFReadStreamRef rStream = _CFNetConnectionGetResponseStream(streamInfo->conn);
        if (!rStream || !CFReadStreamSetProperty(rStream, propertyName, propertyValue)) {
            CFWriteStreamRef wStream = _CFNetConnectionGetRequestStream(streamInfo->conn);
            if (!wStream || !CFWriteStreamSetProperty(wStream, propertyName, propertyValue)) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

static void httpStreamSchedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
    if (_CFHTTPStreamInfoGetState(streamInfo) < kFinished) {
        _CFNetConnectionSchedule(streamInfo->conn, streamInfo, runLoop, runLoopMode);
    }
    if (streamInfo->requestPayload) {
        CFReadStreamScheduleWithRunLoop(streamInfo->requestPayload, runLoop, runLoopMode);
    }
}

static void httpStreamUnschedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFHTTPStreamInfo *streamInfo = (_CFHTTPStreamInfo *)info;
     if (_CFHTTPStreamInfoGetState(streamInfo) < kFinished) {
        _CFNetConnectionUnschedule(streamInfo->conn, streamInfo, runLoop, runLoopMode);
        if (streamInfo->requestPayload) {
            CFReadStreamUnscheduleFromRunLoop(streamInfo->requestPayload, runLoop, runLoopMode);
        }
    }
}
