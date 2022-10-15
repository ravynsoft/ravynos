/*	CFStreamInternal.h
	Copyright (c) 2000-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */
#if !defined(__COREFOUNDATION_CFSTREAMINTERNAL__)
#define __COREFOUNDATION_CFSTREAMINTERNAL__ 1

#include <CoreFoundation/CFStreamAbstract.h>
#include <CoreFoundation/CFStreamPriv.h>
#include <CoreFoundation/CFBase.h>
#include "CFInternal.h"
#include <CoreFoundation/CFRuntime.h>

CF_EXTERN_C_BEGIN

struct _CFStream {
    CFRuntimeBase _cfBase;
    CFOptionFlags flags;
    CFErrorRef error; // if callBacks->version < 2, this is actually a pointer to a CFStreamError
    struct _CFStreamClient *client;
    void *info; /* callBacks info */
    const struct _CFStreamCallBacks *callBacks;  // This will not exist (will not be allocated) if the callbacks are from our known, "blessed" set.
    
    CFLock_t streamLock;
    CFArrayRef previousRunloopsAndModes;
#if __HAS_DISPATCH__
    dispatch_queue_t queue;
#endif
    Boolean pendingEventsToDeliver;
};

#ifndef _CFSTREAM_SIZE
#define _CFSTREAM_SIZE  (sizeof(struct _CFStream) - sizeof(CFRuntimeBase))
#endif

// Older versions of the callbacks; v0 callbacks match v1 callbacks, except that create, finalize, and copyDescription are missing.
typedef Boolean (*_CFStreamCBOpenV1)(struct _CFStream *stream, CFStreamError *error, Boolean *openComplete, void *info);
typedef Boolean (*_CFStreamCBOpenCompletedV1)(struct _CFStream *stream, CFStreamError *error, void *info);
typedef CFIndex (*_CFStreamCBReadV1)(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info);
typedef const UInt8 *(*_CFStreamCBGetBufferV1)(CFReadStreamRef sream, CFIndex maxBytesToRead, CFIndex *numBytesRead, CFStreamError *error, Boolean *atEOF, void *info);
typedef Boolean (*_CFStreamCBCanReadV1)(CFReadStreamRef, void *info);
typedef CFIndex (*_CFStreamCBWriteV1)(CFWriteStreamRef, const UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, void *info);
typedef Boolean (*_CFStreamCBCanWriteV1)(CFWriteStreamRef, void *info);

struct _CFStreamCallBacksV1 {
    CFIndex version;
    void *(*create)(struct _CFStream *stream, void *info);
    void (*finalize)(struct _CFStream *stream, void *info);
    CFStringRef (*copyDescription)(struct _CFStream *stream, void *info);

    _CFStreamCBOpenV1 open;
    _CFStreamCBOpenCompletedV1 openCompleted;
    _CFStreamCBReadV1 read;
    _CFStreamCBGetBufferV1 getBuffer;
    _CFStreamCBCanReadV1 canRead;
    _CFStreamCBWriteV1 write;
    _CFStreamCBCanWriteV1 canWrite;
    void (*close)(struct _CFStream *stream, void *info);

    CFTypeRef (*copyProperty)(struct _CFStream *stream, CFStringRef propertyName, void *info);
    Boolean (*setProperty)(struct _CFStream *stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info);
    void (*requestEvents)(struct _CFStream *stream, CFOptionFlags events, void *info);
    void (*schedule)(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
    void (*unschedule)(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
};

// These two are defined in CFSocketStream.c because that's where the glue for CFNetwork is.
CF_PRIVATE CFErrorRef _CFStreamCreateErrorFromStreamError(CFAllocatorRef alloc, CFStreamError *err);
CF_PRIVATE CFStreamError _CFStreamErrorFromError(CFErrorRef error);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFSTREAMINTERNAL__ */


