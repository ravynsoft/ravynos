/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFStreamPriv.h
	Copyright (c) 2000-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFSTREAMPRIV__)
#define __COREFOUNDATION_CFSTREAMPRIV__ 1

#include <CoreFoundation/CFStream.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFRuntime.h>

CF_EXTERN_C_BEGIN

struct _CFStream;
struct _CFStreamClient {
    CFStreamClientContext cbContext;
    void (*cb)(struct _CFStream *, CFStreamEventType, void *);
    CFOptionFlags when;
    CFRunLoopSourceRef rlSource;
    CFMutableArrayRef runLoopsAndModes;
    CFOptionFlags whatToSignal;
};

#define CFStreamCurrentVersion 2

// A unified set of callbacks so we can use a single structure for all struct _CFStreams.
struct _CFStreamCallBacks {
    CFIndex version;
    void *(*create)(struct _CFStream *stream, void *info);
    void (*finalize)(struct _CFStream *stream, void *info);
    CFStringRef (*copyDescription)(struct _CFStream *stream, void *info);

    Boolean (*open)(struct _CFStream *stream, CFErrorRef *error, Boolean *openComplete, void *info);
    Boolean (*openCompleted)(struct _CFStream *stream, CFErrorRef *error, void *info);
    CFIndex (*read)(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFErrorRef *error, Boolean *atEOF, void *info);
    const UInt8 *(*getBuffer)(CFReadStreamRef sream, CFIndex maxBytesToRead, CFIndex *numBytesRead, CFErrorRef *error, Boolean *atEOF, void *info);
    Boolean (*canRead)(CFReadStreamRef, CFErrorRef *error, void *info);
    CFIndex (*write)(CFWriteStreamRef, const UInt8 *buffer, CFIndex bufferLength, CFErrorRef *error, void *info);
    Boolean (*canWrite)(CFWriteStreamRef, CFErrorRef *error, void *info); 
    void (*close)(struct _CFStream *stream, void *info);

    CFTypeRef (*copyProperty)(struct _CFStream *stream, CFStringRef propertyName, void *info);
    Boolean (*setProperty)(struct _CFStream *stream, CFStringRef propertyName, CFTypeRef propertyValue, void *info);
    void (*requestEvents)(struct _CFStream *stream, CFOptionFlags events, void *info);
    void (*schedule)(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
    void (*unschedule)(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info);
};

struct _CFStream;

CF_EXPORT void* _CFStreamGetInfoPointer(struct _CFStream* stream);

// cb version must be > 0
CF_EXPORT struct _CFStream *_CFStreamCreateWithConstantCallbacks(CFAllocatorRef alloc, void *info, const struct _CFStreamCallBacks *cb, Boolean isReading);

// Only available for streams created with _CFStreamCreateWithConstantCallbacks, above. cb's version must be 1
CF_EXPORT void _CFStreamSetInfoPointer(struct _CFStream *stream, void *info, const struct _CFStreamCallBacks *cb);

/*
** _CFStreamSourceScheduleWithRunLoop
**
** Schedules the given run loop source on the given run loop and mode.  It then
** adds the loop and mode pair to the runLoopsAndModes list.  The list is
** simply a linear list of a loop reference followed by a mode reference.
**
** source Run loop source to be scheduled
**
** runLoopsAndModes List of run loop/mode pairs on which the source is scheduled
**
** runLoop Run loop on which the source is being scheduled
**
** runLoopMode Run loop mode on which the source is being scheduled
*/
CF_EXPORT
void _CFStreamSourceScheduleWithRunLoop(CFRunLoopSourceRef source, CFMutableArrayRef runLoopsAndModes, CFRunLoopRef runLoop, CFStringRef runLoopMode);


/*
** _CFStreamSourceUnscheduleFromRunLoop
**
** Unschedule the given source from the given run loop and mode.  It then will
** guarantee that the source remains scheduled on the list of run loop and mode
** pairs in the runLoopsAndModes list.  The list is simply a linear list of a
** loop reference followed by a mode reference.
**
** source Run loop source to be unscheduled
**
** runLoopsAndModes List of run loop/mode pairs on which the source is scheduled
**
** runLoop Run loop from which the source is being unscheduled
**
** runLoopMode Run loop mode from which the source is being unscheduled
*/
CF_EXPORT
void _CFStreamSourceUnscheduleFromRunLoop(CFRunLoopSourceRef source, CFMutableArrayRef runLoopsAndModes, CFRunLoopRef runLoop, CFStringRef runLoopMode);


/*
** _CFStreamSourceScheduleWithAllRunLoops
**
** Schedules the given run loop source on all the run loops and modes in the list.
** The list is simply a linear list of a loop reference followed by a mode reference.
**
** source Run loop source to be unscheduled
**
** runLoopsAndModes List of run loop/mode pairs on which the source is scheduled
*/
CF_EXPORT
void _CFStreamSourceScheduleWithAllRunLoops(CFRunLoopSourceRef source, CFArrayRef runLoopsAndModes);


/*
** _CFStreamSourceUnscheduleFromRunLoop
**
** Unschedule the given source from all the run loops and modes in the list.
** The list is simply a linear list of a loop reference followed by a mode
** reference.
**
** source Run loop source to be unscheduled
**
** runLoopsAndModes List of run loop/mode pairs on which the source is scheduled
*/
CF_EXPORT
void _CFStreamSourceUncheduleFromAllRunLoops(CFRunLoopSourceRef source, CFArrayRef runLoopsAndModes);

CF_EXPORT
CFReadStreamRef _CFReadStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);

CF_EXPORT
CFWriteStreamRef _CFWriteStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);



#define SECURITY_NONE   (0)
#define SECURITY_SSLv2  (1)
#define SECURITY_SSLv3  (2)
#define SECURITY_SSLv32 (3)
#define SECURITY_TLS    (4)    

#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
// This symbol is exported from CFNetwork (see CFSocketStream.i).  Only __MACH__ systems will
// get this symbol from CoreFoundation.
extern const int kCFStreamErrorDomainSSL;
#endif

/*
 * Additional SPI for CFNetwork for select side read buffering
 */
CF_EXPORT
Boolean __CFSocketGetBytesAvailable(CFSocketRef s, CFIndex* ctBytesAvailable);

CF_EXPORT
CFIndex __CFSocketRead(CFSocketRef s, UInt8* buffer, CFIndex length, int* error);

/*
 * This define can be removed once 6030579 is removed
 */
#define CFNETWORK_6030579	1

CF_EXPORT
void __CFSocketSetSocketReadBufferAttrs(CFSocketRef s, CFTimeInterval timeout, CFIndex length);

CF_EXTERN_C_END

/*
 * for CF{Read/Write}StreamCopyProperty created from a file.  The
 * result is a CFDataRef containing sizeof(int) bytes in machine byte
 * ordering representing the file descriptor of the underlying open
 * file.  If the underlying file descriptor is not open, the property
 * value will be NULL (as opposed to containing ((int) -1)).
 */
CF_EXPORT const CFStringRef _kCFStreamPropertyFileNativeHandle CF_AVAILABLE_IOS(5_0);

#endif /* ! __COREFOUNDATION_CFSTREAMPRIV__ */

