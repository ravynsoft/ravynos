/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __CFStream *CFStreamRef;
typedef struct __CFReadStream *CFReadStreamRef;
typedef struct __CFWriteStream *CFWriteStreamRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFRunLoop.h>
#import <CoreFoundation/CFSocket.h>
#import <CoreFoundation/CFURL.h>
#import <CoreFoundation/CFError.h>

// These values must match NSStream
typedef enum {
    kCFStreamEventNone = 0,
    kCFStreamEventOpenCompleted = (1 << 0),
    kCFStreamEventHasBytesAvailable = (1 << 1),
    kCFStreamEventCanAcceptBytes = (1 << 2),
    kCFStreamEventErrorOccurred = (1 << 3),
    kCFStreamEventEndEncountered = (1 << 4),
} CFStreamEventType;

typedef enum {
    kCFStreamErrorDomainCustom = -1,
    kCFStreamErrorDomainPOSIX = 1,
    kCFStreamErrorDomainMacOSStatus = 2,
} CFStreamErrorDomain;

typedef void (*CFReadStreamClientCallBack)(CFReadStreamRef self, CFStreamEventType event, void *info);
typedef void (*CFWriteStreamClientCallBack)(CFWriteStreamRef self, CFStreamEventType event, void *info);

typedef struct {
    CFStreamErrorDomain domain;
    CFInteger error;
} CFStreamError;

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFStreamClientContext;

typedef enum {
    kCFStreamStatusNotOpen = 0,
    kCFStreamStatusOpening = 1,
    kCFStreamStatusOpen = 2,
    kCFStreamStatusReading = 3,
    kCFStreamStatusWriting = 4,
    kCFStreamStatusAtEnd = 5,
    kCFStreamStatusClosed = 6,
    kCFStreamStatusError = 7,
} CFStreamStatus;

COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainNetDB;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainNetServices;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainMach;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainFTP;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainHTTP;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainSOCKS;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainSystemConfiguration;
COREFOUNDATION_EXPORT const CFInteger kCFStreamErrorDomainSSL;

COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertyFileCurrentOffset;
COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertyAppendToFile;
COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertyDataWritten;
COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertySocketNativeHandle;
COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertySocketRemoteHostName;
COREFOUNDATION_EXPORT const CFStringRef kCFStreamPropertySocketRemotePortNumber;

COREFOUNDATION_EXPORT void CFStreamCreateBoundPair(CFAllocatorRef allocator, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream, CFIndex bufferSize);
COREFOUNDATION_EXPORT void CFStreamCreatePairWithPeerSocketSignature(CFAllocatorRef allocator, const CFSocketSignature *signature, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);
COREFOUNDATION_EXPORT void CFStreamCreatePairWithSocket(CFAllocatorRef allocator, CFSocketNativeHandle sock, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);
COREFOUNDATION_EXPORT void CFStreamCreatePairWithSocketToHost(CFAllocatorRef allocator, CFStringRef host, CFUInteger port, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);
// ReadStream

COREFOUNDATION_EXPORT CFTypeID CFReadStreamGetTypeID(void);

COREFOUNDATION_EXPORT CFReadStreamRef CFReadStreamCreateWithBytesNoCopy(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFAllocatorRef bytesDeallocator);
COREFOUNDATION_EXPORT CFReadStreamRef CFReadStreamCreateWithFile(CFAllocatorRef allocator, CFURLRef url);

COREFOUNDATION_EXPORT Boolean CFReadStreamSetClient(CFReadStreamRef self, CFOptionFlags events, CFReadStreamClientCallBack callback, CFStreamClientContext *context);

COREFOUNDATION_EXPORT CFTypeRef CFReadStreamCopyProperty(CFReadStreamRef self, CFStringRef key);
COREFOUNDATION_EXPORT Boolean CFReadStreamSetProperty(CFReadStreamRef self, CFStringRef key, CFTypeRef value);

COREFOUNDATION_EXPORT const uint8_t *CFReadStreamGetBuffer(CFReadStreamRef self, CFIndex limit, CFIndex *available);
COREFOUNDATION_EXPORT Boolean CFReadStreamOpen(CFReadStreamRef self);
COREFOUNDATION_EXPORT void CFReadStreamClose(CFReadStreamRef self);
COREFOUNDATION_EXPORT Boolean CFReadStreamHasBytesAvailable(CFReadStreamRef self);
COREFOUNDATION_EXPORT CFIndex CFReadStreamRead(CFReadStreamRef self, uint8_t *bytes, CFIndex length);

COREFOUNDATION_EXPORT CFErrorRef CFReadStreamCopyError(CFReadStreamRef self);
COREFOUNDATION_EXPORT CFStreamError CFReadStreamGetError(CFReadStreamRef self);
COREFOUNDATION_EXPORT CFStreamStatus CFReadStreamGetStatus(CFReadStreamRef self);

COREFOUNDATION_EXPORT void CFReadStreamScheduleWithRunLoop(CFReadStreamRef self, CFRunLoopRef runLoop, CFStringRef mode);
COREFOUNDATION_EXPORT void CFReadStreamUnscheduleFromRunLoop(CFReadStreamRef self, CFRunLoopRef runLoop, CFStringRef mode);

// Write Stream

COREFOUNDATION_EXPORT CFTypeID CFWriteStreamGetTypeID(void);

COREFOUNDATION_EXPORT CFWriteStreamRef CFWriteStreamCreateWithAllocatedBuffers(CFAllocatorRef allocator, CFAllocatorRef bufferAllocator);
COREFOUNDATION_EXPORT CFWriteStreamRef CFWriteStreamCreateWithBuffer(CFAllocatorRef allocator, uint8_t *bytes, CFIndex capacity);
COREFOUNDATION_EXPORT CFWriteStreamRef CFWriteStreamCreateWithFile(CFAllocatorRef allocator, CFURLRef url);

COREFOUNDATION_EXPORT Boolean CFWriteStreamSetClient(CFWriteStreamRef self, CFOptionFlags events, CFWriteStreamClientCallBack callback, CFStreamClientContext *context);

COREFOUNDATION_EXPORT CFTypeRef CFWriteStreamCopyProperty(CFWriteStreamRef self, CFStringRef key);
COREFOUNDATION_EXPORT Boolean CFWriteStreamSetProperty(CFWriteStreamRef self, CFStringRef key, CFTypeRef value);

COREFOUNDATION_EXPORT Boolean CFWriteStreamOpen(CFWriteStreamRef self);
COREFOUNDATION_EXPORT void CFWriteStreamClose(CFWriteStreamRef self);
COREFOUNDATION_EXPORT Boolean CFWriteStreamCanAcceptBytes(CFWriteStreamRef self);
COREFOUNDATION_EXPORT CFIndex CFWriteStreamWrite(CFWriteStreamRef self, const uint8_t *bytes, CFIndex length);

COREFOUNDATION_EXPORT CFErrorRef CFWriteStreamCopyError(CFReadStreamRef self);
COREFOUNDATION_EXPORT CFStreamError CFWriteStreamGetError(CFWriteStreamRef self);
COREFOUNDATION_EXPORT CFStreamStatus CFWriteStreamGetStatus(CFWriteStreamRef self);

COREFOUNDATION_EXPORT void CFWriteStreamScheduleWithRunLoop(CFWriteStreamRef self, CFRunLoopRef runLoop, CFStringRef mode);
COREFOUNDATION_EXPORT void CFWriteStreamUnscheduleFromRunLoop(CFWriteStreamRef self, CFRunLoopRef runLoop, CFStringRef mode);
