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
#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFHTTPConnectionPriv.h>
#include "CFNetConnection.h"

struct _CFHTTPConnectionInfo {
    CFStringRef host;
    SInt32 port;
    UInt32 type;
    CFDictionaryRef streamProperties;
	CFMutableSetRef	authentications;
};
typedef struct _CFHTTPConnectionInfo _CFHTTPConnectionInfo;

#define HAVE_SENT_REQUEST_HEADERS (0)
#define HAVE_SENT_REQUEST_PAYLOAD (1)
#define HAVE_CHECKED_RESPONSE_HEADERS (2)
// Used to force end-of-stream when we know from the request/response that no data should come (like for a HEAD request)
#define FORCE_EOF (3)
#define PAYLOAD_IS_DATA (4)
#define OPEN_SIGNALLED (5)
#define HAS_PAYLOAD (6)
#define IS_ZOMBIE (7)
#define MIN_STATE_BIT (8)
#define MAX_STATE_BIT (11)
#define IN_READ_CALLBACK (12)
/* A bit of a hack to cover the fact that we may become the current response well before the net connection signals 
    us with stateChanged to prepareReception.  The problem is that once we become the current response, we get the 
    response stream callbacks, which may include the mark (actually intended for the prior response) - we don't want
    that to cause us to send an endEncountered event.  Now, if we receive such an event and we have not yet read the mark,
    we simply do so and continue.  */
#define HAVE_READ_MARK (13)

struct _CFHTTPStreamInfo {
    CFOptionFlags flags;
    CFHTTPMessageRef request;
    CFHTTPMessageRef responseHeaders; 
    CFReadStreamRef requestPayload; // May be NULL
    CFDataRef requestFragment; // Fragmentary data read from requestPayload but not yet written
    long long requestBytesWritten;
	CFArrayRef peerCertificates; // Certificates received from peer
	CFArrayRef clientCertificates; // Client certificate chain sent to peer
	CFNumberRef clientCertificateState; // Holds a SSLClientCertificateState value; see <Security/SecureTransport.h>
    
    _CFNetConnectionRef conn;
    CFReadStreamRef stream; // The stream we returned for this request
    CFRunLoopSourceRef stateChangeSource; // This source is used when we need to wait on an outside state change - either for bytes to come in on the connection, or for some request upstream of us to progress.
};
typedef struct _CFHTTPStreamInfo _CFHTTPStreamInfo;

/* Callbacks for _CFNetConnection */

static const void *httpConnectionCreate(CFAllocatorRef alloc, const void *info);
static void httpConnectionFinalize(CFAllocatorRef alloc, const void *info);
static CFStreamError httpConnectionCreateStreams(CFAllocatorRef allocator, const void *info, CFWriteStreamRef *requestStream, CFReadStreamRef *responseStream);
static void httpConnectionStateChanged(void *request, int newState, CFStreamError *err, _CFNetConnectionRef connection, const void *info);
static void httpConnectionTransmitRequest(void *request, _CFNetConnectionRef connection, const void *info);
static void httpConnectionReceiveResponse(void *request, _CFNetConnectionRef connection, const void *info);
static void httpConnectionResponseStreamCB(void *request, CFReadStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info);
static void httpConnectionRequestStreamCB(void *request, CFWriteStreamRef stream, CFStreamEventType eventType, _CFNetConnectionRef conn, const void *info);
static CFArrayRef httpConnectionRLArrayForRequest(void *request, _CFNetConnectionRef conn, const void *info);

/* Callbacks for the read streams we return */

static void *httpStreamCreate(CFReadStreamRef stream, void *info);
static void httpStreamFinalize(CFReadStreamRef stream, void *info);
static CFStringRef httpStreamCopyDescription(CFReadStreamRef stream, void *info);
static Boolean httpStreamOpen(CFReadStreamRef stream, CFStreamError *error, Boolean *openComplete, void *info);
static Boolean httpStreamOpenCompleted(CFReadStreamRef stream, CFStreamError *error, void *info);
static CFIndex httpStreamRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info);
static Boolean httpStreamCanRead(CFReadStreamRef stream, void *info);
static void httpStreamClose(CFReadStreamRef stream, void *info);
static CFTypeRef httpStreamCopyProperty(CFReadStreamRef stream, CFStringRef propertyName, void *info);
static Boolean httpStreamSetProperty(CFReadStreamRef stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info);
static void httpStreamSchedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
static void httpStreamUnschedule(CFReadStreamRef stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);

/* Other function prototypes */
static void httpRequestPayloadCallBack(CFReadStreamRef stream, CFStreamEventType type, void *info);
static void dequeueFromConnection(_CFHTTPStreamInfo *streamInfo);
static void destroyStreamInfo(CFAllocatorRef alloc, _CFHTTPStreamInfo *streamInfo);
static _CFHTTPStreamInfo *createZombieDouble(CFAllocatorRef alloc, _CFHTTPStreamInfo *orig, _CFNetConnectionRef conn);

