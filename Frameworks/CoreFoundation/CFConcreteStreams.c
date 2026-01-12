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

/*	CFConcreteStreams.c
	Copyright (c) 2000-2014, Apple Inc. All rights reserved.
	Responsibility: John Iarocci
*/

#include "CFStreamInternal.h"
#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFNumber.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <sys/time.h>
#include <unistd.h>
#endif


#define SCHEDULE_AFTER_WRITE  (0)
#define SCHEDULE_AFTER_READ   (1)
#define APPEND                (3)
#define AT_EOF                (4)
#define USE_RUNLOOP_ARRAY     (5)


/* File callbacks */
typedef struct {
    CFURLRef url;
    int fd;
#ifdef REAL_FILE_SCHEDULING
    union {
        CFFileDescriptorRef cffd;	// ref created once we open and have an fd
        CFMutableArrayRef rlArray;	// scheduling information prior to open
    } rlInfo; // If fd > 0, cffd exists.  Otherwise, rlArray.
#else
    uint16_t scheduled;	// ref count of how many times we've been scheduled
#endif
    CFOptionFlags flags;    
    off_t offset;
} _CFFileStreamContext;


CONST_STRING_DECL(kCFStreamPropertyFileCurrentOffset, "kCFStreamPropertyFileCurrentOffset");
#if DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
CONST_STRING_DECL(_kCFStreamPropertyFileNativeHandle, "_kCFStreamPropertyFileNativeHandle");
#endif

#ifdef REAL_FILE_SCHEDULING
extern void _CFFileDescriptorInduceFakeReadCallBack(CFFileDescriptorRef);
static void fileCallBack(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info);

static void constructCFFD(_CFFileStreamContext *fileStream, Boolean forRead, struct _CFStream *stream) {
    CFFileDescriptorContext context = {0, stream, NULL, NULL, (void *)CFCopyDescription};
    CFFileDescriptorRef cffd = CFFileDescriptorCreate(CFGetAllocator(stream), fileStream->fd, false, fileCallBack, &context);
    CFFileDescriptorEnableCallBacks(cffd, forRead ? kCFFileDescriptorReadCallBack : kCFFileDescriptorWriteCallBack);
    if (fileStream->rlInfo.rlArray) {
        CFIndex i, c = CFArrayGetCount(fileStream->rlInfo.rlArray);
        CFRunLoopSourceRef src = CFFileDescriptorCreateRunLoopSource(CFGetAllocator(stream), cffd, 0);
        for (i = 0; i+1 < c; i += 2) {
            CFRunLoopRef rl = (CFRunLoopRef)CFArrayGetValueAtIndex(fileStream->rlInfo.rlArray, i);
            CFStringRef mode = CFArrayGetValueAtIndex(fileStream->rlInfo.rlArray, i+1);
            CFRunLoopAddSource(rl, src, mode);
        }
        CFRelease(fileStream->rlInfo.rlArray);
        CFRelease(src);
    }    
    fileStream->rlInfo.cffd = cffd;
}
#endif

static Boolean constructFD(_CFFileStreamContext *fileStream, CFStreamError *error, Boolean forRead, struct _CFStream *stream) {
    int flags = forRead ? O_RDONLY : (O_CREAT | O_TRUNC | O_WRONLY);
#if DEPLOYMENT_TARGET_WINDOWS
    wchar_t path[CFMaxPathSize];
    flags |= (_O_BINARY|_O_NOINHERIT);
    if (_CFURLGetWideFileSystemRepresentation(fileStream->url, TRUE, path, CFMaxPathSize) == FALSE)
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    char path[CFMaxPathSize];
    if (CFURLGetFileSystemRepresentation(fileStream->url, TRUE, (UInt8 *)path, CFMaxPathSize) == FALSE)
#endif
     {
        error->error = ENOENT;
        error->domain = kCFStreamErrorDomainPOSIX;
        return FALSE;
    }
    if (__CFBitIsSet(fileStream->flags, APPEND)) {
        flags |= O_APPEND;
        flags &= ~O_TRUNC;
    }
    
    do {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        fileStream->fd = open((const char *)path, flags, 0666);
#elif DEPLOYMENT_TARGET_WINDOWS
	fileStream->fd = _wopen(path, flags, 0666);
#endif
        if (fileStream->fd < 0)
            break;
        
        if ((fileStream->offset != -1) && (lseek(fileStream->fd, fileStream->offset, SEEK_SET) == -1))
            break;

#ifdef REAL_FILE_SCHEDULING
        if (fileStream->rlInfo.rlArray != NULL) {
            constructCFFD(fileStream, forRead, stream);
        }
#endif

        return TRUE;
    } while (1);

    __CFBitSet(fileStream->flags, USE_RUNLOOP_ARRAY);
    error->error = errno;
    error->domain = kCFStreamErrorDomainPOSIX;

    return FALSE;
}

static Boolean fileOpen(struct _CFStream *stream, CFStreamError *errorCode, Boolean *openComplete, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    Boolean forRead = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
    *openComplete = TRUE;
    if (ctxt->url) {
        if (constructFD(ctxt, errorCode, forRead, stream)) {
#ifndef REAL_FILE_SCHEDULING
            if (ctxt->scheduled > 0) {
                if (forRead)
                    CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
                else
                    CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
            }
#endif
            return TRUE;
        } else {
            return FALSE;
        }
#ifdef REAL_FILE_SCHEDULING
    } else if (ctxt->rlInfo.rlArray != NULL) {
        constructCFFD(ctxt, forRead, stream);
#endif
    }
    return TRUE;
}

CF_PRIVATE CFIndex fdRead(int fd, UInt8 *buffer, CFIndex bufferLength, CFStreamError *errorCode, Boolean *atEOF) {
    CFIndex bytesRead = read(fd, buffer, bufferLength);
    if (bytesRead < 0) {
        errorCode->error = errno;
        errorCode->domain = kCFStreamErrorDomainPOSIX;
        return -1;
    } else {
        *atEOF = (bytesRead == 0) ? TRUE : FALSE;
        errorCode->error = 0;
        return bytesRead;
    }
}

static CFIndex fileRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *errorCode, Boolean *atEOF, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    CFIndex result;
    result = fdRead(ctxt->fd, buffer, bufferLength, errorCode, atEOF);
#ifdef REAL_FILE_SCHEDULING
    if (__CFBitIsSet(ctxt->flags, SCHEDULE_AFTER_READ)) {
        __CFBitClear(ctxt->flags, SCHEDULE_AFTER_READ);
        if (!*atEOF && ctxt->rlInfo.cffd) {
            struct stat statbuf;
            int ret = fstat(ctxt->fd, &statbuf);
            if (0 <= ret && (S_IFREG == (statbuf.st_mode & S_IFMT))) {
                off_t offset = lseek(ctxt->fd, 0, SEEK_CUR);
                if (statbuf.st_size == offset) {
                    _CFFileDescriptorInduceFakeReadCallBack(ctxt->rlInfo.cffd);
                }
            }
        }
        if (ctxt->rlInfo.cffd) {
            CFFileDescriptorEnableCallBacks(ctxt->rlInfo.cffd, kCFFileDescriptorReadCallBack);
        }
    }
#else
    if (*atEOF)
        __CFBitSet(ctxt->flags, AT_EOF);
    if (ctxt->scheduled > 0 && !*atEOF) {
        CFReadStreamSignalEvent(stream, kCFStreamEventHasBytesAvailable, NULL);
    }
#endif
    return result;
}

#ifdef REAL_FILE_SCHEDULING
CF_PRIVATE Boolean fdCanRead(int fd) {
    struct timeval timeout = {0, 0};
    fd_set *readSetPtr;
    fd_set readSet;
    Boolean result;
// fd_set is not a mask in Win32, so checking for an fd that's too big is not relevant
    if (fd < FD_SETSIZE) {
        FD_ZERO(&readSet);
        readSetPtr = &readSet;
    } else {
        int size = howmany(fd+1, NFDBITS) * sizeof(uint32_t);
        uint32_t *fds_bits = (uint32_t *)malloc(size);
        memset(fds_bits, 0, size);
        readSetPtr = (fd_set *)fds_bits;
    }
    FD_SET(fd, readSetPtr);
    result = (select(fd + 1, readSetPtr, NULL, NULL, &timeout) == 1) ? TRUE : FALSE;
    if (readSetPtr != &readSet) {
        free(readSetPtr);
    }
    return result;
}
#endif

static Boolean fileCanRead(CFReadStreamRef stream, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
#ifdef REAL_FILE_SCHEDULING
    return fdCanRead(ctxt->fd);
#else
    return !__CFBitIsSet(ctxt->flags, AT_EOF);
#endif
}

CF_PRIVATE CFIndex fdWrite(int fd, const UInt8 *buffer, CFIndex bufferLength, CFStreamError *errorCode) {
    CFIndex bytesWritten = write(fd, buffer, bufferLength);
    if (bytesWritten < 0) {
        errorCode->error = errno;
        errorCode->domain = kCFStreamErrorDomainPOSIX;
        return -1;
    } else {
        errorCode->error = 0;
        return bytesWritten;
    }
}

static CFIndex fileWrite(CFWriteStreamRef stream, const UInt8 *buffer, CFIndex bufferLength, CFStreamError *errorCode, void *info) {
    _CFFileStreamContext *fileStream = ((_CFFileStreamContext *)info);
    CFIndex result = fdWrite(fileStream->fd, buffer, bufferLength, errorCode);
#ifdef REAL_FILE_SCHEDULING
    if (__CFBitIsSet(fileStream->flags, SCHEDULE_AFTER_WRITE)) {
        __CFBitClear(fileStream->flags, SCHEDULE_AFTER_WRITE);
        if (fileStream->rlInfo.cffd) {
            CFFileDescriptorEnableCallBacks(fileStream->rlInfo.cffd, kCFFileDescriptorWriteCallBack);
        }
    }
#else
    if (fileStream->scheduled > 0) {
        CFWriteStreamSignalEvent(stream, kCFStreamEventCanAcceptBytes, NULL);
    }
#endif
    return result;
}

#ifdef REAL_FILE_SCHEDULING
CF_PRIVATE Boolean fdCanWrite(int fd) {
    struct timeval timeout = {0, 0};
    fd_set *writeSetPtr;
    fd_set writeSet;
    Boolean result;
    if (fd < FD_SETSIZE) {
        FD_ZERO(&writeSet);
        writeSetPtr = &writeSet;
    } else {
        int size = howmany(fd+1, NFDBITS) * sizeof(uint32_t);
        uint32_t *fds_bits = (uint32_t *)malloc(size);
        memset(fds_bits, 0, size);
        writeSetPtr = (fd_set *)fds_bits;
    }
    FD_SET(fd, writeSetPtr);
    result = (select(fd + 1, NULL, writeSetPtr, NULL, &timeout) == 1) ? TRUE : FALSE;
    if (writeSetPtr != &writeSet) {
        free(writeSetPtr);
    }
    return result;
}
#endif

static Boolean fileCanWrite(CFWriteStreamRef stream, void *info) {
#ifdef REAL_FILE_SCHEDULING
    return fdCanWrite(((_CFFileStreamContext *)info)->fd);
#else
    return TRUE;
#endif
}

static void fileClose(struct _CFStream *stream, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    if (ctxt->fd >= 0) {
        close(ctxt->fd);
        ctxt->fd = -1;
#ifdef REAL_FILE_SCHEDULING
        if (ctxt->rlInfo.cffd) {
            CFFileDescriptorInvalidate(ctxt->rlInfo.cffd);
            CFRelease(ctxt->rlInfo.cffd);
            ctxt->rlInfo.cffd = NULL;
        }
    } else if (ctxt->rlInfo.rlArray) {
        CFRelease(ctxt->rlInfo.rlArray);
        ctxt->rlInfo.rlArray = NULL;
#endif
    }
}

#ifdef REAL_FILE_SCHEDULING
static void fileCallBack(CFFileDescriptorRef f, CFOptionFlags type, void *info) {
    struct _CFStream *stream = (struct _CFStream *)info;
    Boolean isReadStream = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
    _CFFileStreamContext *fileStream = isReadStream ? CFReadStreamGetInfoPointer((CFReadStreamRef)stream) : CFWriteStreamGetInfoPointer((CFWriteStreamRef)stream);
    if (type == kCFFileDescriptorWriteCallBack) {
        __CFBitSet(fileStream->flags, SCHEDULE_AFTER_WRITE);
        CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
    } else {
        __CFBitSet(fileStream->flags, SCHEDULE_AFTER_READ);
        CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
    }
}
#endif

static void fileSchedule(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFFileStreamContext *fileStream = (_CFFileStreamContext *)info;
    Boolean isReadStream = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
    CFStreamStatus status = isReadStream ? CFReadStreamGetStatus((CFReadStreamRef)stream) : CFWriteStreamGetStatus((CFWriteStreamRef)stream);
    if (fileStream->fd < 0 && status != kCFStreamStatusNotOpen) {
        // Stream's already closed or error-ed out 
        return;
    }
#ifdef REAL_FILE_SCHEDULING
    if (status == kCFStreamStatusNotOpen) {
        if (!fileStream->rlInfo.rlArray) {
            fileStream->rlInfo.rlArray = CFArrayCreateMutable(CFGetAllocator(stream), 0, &kCFTypeArrayCallBacks);
        }
        CFArrayAppendValue(fileStream->rlInfo.rlArray, runLoop);
        CFArrayAppendValue(fileStream->rlInfo.rlArray, runLoopMode);
    } else {
        CFRunLoopSourceRef rlSrc;
        if (!fileStream->rlInfo.cffd) {
            constructCFFD(fileStream, isReadStream, stream);
        }
        rlSrc = CFFileDescriptorCreateRunLoopSource(CFGetAllocator(stream), fileStream->rlInfo.cffd, 0);
        CFRunLoopAddSource(runLoop, rlSrc, runLoopMode);
        CFRelease(rlSrc);
    }
#else
    fileStream->scheduled++;
    if (fileStream->scheduled == 1 && fileStream->fd > 0 && status == kCFStreamStatusOpen) {
        if (isReadStream)
            CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
        else
            CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
    }
#endif
}

static void fileUnschedule(struct _CFStream *stream, CFRunLoopRef runLoop, CFStringRef runLoopMode, void *info) {
    _CFFileStreamContext *fileStream = (_CFFileStreamContext *)info;
#ifdef REAL_FILE_SCHEDULING
    Boolean isReadStream = (CFGetTypeID(stream) == CFReadStreamGetTypeID());
    CFStreamStatus status = isReadStream ? CFReadStreamGetStatus((CFReadStreamRef)stream) : CFWriteStreamGetStatus((CFWriteStreamRef)stream);
    if (status == kCFStreamStatusNotOpen) {
        // Not opened yet
        if (fileStream->rlInfo.rlArray) {
            CFMutableArrayRef runloops = fileStream->rlInfo.rlArray;
            CFIndex i, c;
            for (i = 0, c = CFArrayGetCount(runloops); i+1 < c; i += 2) {
                if (CFEqual(CFArrayGetValueAtIndex(runloops, i), runLoop) && CFEqual(CFArrayGetValueAtIndex(runloops, i+1), runLoopMode)) {
                    CFArrayRemoveValueAtIndex(runloops, i);
                    CFArrayRemoveValueAtIndex(runloops, i);
                    break;
                }
            }
        }
    } else if (fileStream->rlInfo.cffd) {
		if (__CFBitIsSet(fileStream->flags, USE_RUNLOOP_ARRAY)) {
			// we know that fileStream->rlInfo.rlArray is non-NULL because it is in a union with fileStream->rlInfo.cffd
            CFMutableArrayRef runloops = fileStream->rlInfo.rlArray;
            CFIndex i, c;
            for (i = 0, c = CFArrayGetCount(runloops); i+1 < c; i += 2) {
                if (CFEqual(CFArrayGetValueAtIndex(runloops, i), runLoop) && CFEqual(CFArrayGetValueAtIndex(runloops, i+1), runLoopMode)) {
                    CFArrayRemoveValueAtIndex(runloops, i);
                    CFArrayRemoveValueAtIndex(runloops, i);
                    break;
                }
            }
        } else {
			CFRunLoopSourceRef rlSrc = CFFileDescriptorCreateRunLoopSource(CFGetAllocator(stream), fileStream->rlInfo.cffd, 0);
			CFRunLoopRemoveSource(runLoop, rlSrc, runLoopMode);
			CFRelease(rlSrc);
		}
    }
#else
    if (fileStream->scheduled > 0)
        fileStream->scheduled--;
#endif
}

static CFTypeRef fileCopyProperty(struct _CFStream *stream, CFStringRef propertyName, void *info) {
    
    CFTypeRef result = NULL;
    _CFFileStreamContext *fileStream = (_CFFileStreamContext *)info;

    if (CFEqual(propertyName, kCFStreamPropertyFileCurrentOffset)) {
        
        // NOTE that this does a lseek of 0 from the current location in
        // order to populate the offset field which will then be used to
        // create the resulting value.
        if (!__CFBitIsSet(fileStream->flags, APPEND) && fileStream->fd != -1) {
            fileStream->offset = lseek(fileStream->fd, 0, SEEK_CUR);
        }
        
        if (fileStream->offset != -1) {
            result = CFNumberCreate(CFGetAllocator((CFTypeRef)stream), kCFNumberSInt64Type, &(fileStream->offset));
        }
#if DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    } else if (CFEqual(propertyName, _kCFStreamPropertyFileNativeHandle)) {
		int fd = fileStream->fd;
		if (fd != -1) {
			result = CFDataCreate(CFGetAllocator((CFTypeRef) stream), (const uint8_t *)&fd, sizeof(fd));
		}
#endif
	}

    return result;
}

static Boolean fileSetProperty(struct _CFStream *stream, CFStringRef prop, CFTypeRef val, void *info) {
    
    Boolean result = FALSE;
    _CFFileStreamContext *fileStream = (_CFFileStreamContext *)info;

    if (CFEqual(prop, kCFStreamPropertyAppendToFile) && CFGetTypeID(stream) == CFWriteStreamGetTypeID() &&
        CFWriteStreamGetStatus((CFWriteStreamRef)stream) == kCFStreamStatusNotOpen)
    {
        if (val == kCFBooleanTrue) {
            __CFBitSet(fileStream->flags, APPEND);
            fileStream->offset = -1;				// Can't offset and append on the stream
        } else {
            __CFBitClear(fileStream->flags, APPEND);
        }
        result = TRUE;
    }
    
    else if (CFEqual(prop, kCFStreamPropertyFileCurrentOffset)) {
        
        if (!__CFBitIsSet(fileStream->flags, APPEND))
        {
            result = CFNumberGetValue((CFNumberRef)val, kCFNumberSInt64Type, &(fileStream->offset));
        }
        
        if ((fileStream->fd != -1) && (lseek(fileStream->fd, fileStream->offset, SEEK_SET) == -1)) {
            result = FALSE;
        }
    }
    
    return result;
}

static void *fileCreate(struct _CFStream *stream, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    _CFFileStreamContext *newCtxt = (_CFFileStreamContext *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFFileStreamContext), 0);
    if (!newCtxt) return NULL;
    newCtxt->url = ctxt->url;
    if (newCtxt->url) {
        CFRetain(newCtxt->url);
    }
    newCtxt->fd = ctxt->fd;
#ifdef REAL_FILE_SCHEDULING
    newCtxt->rlInfo.cffd = NULL;
#else
    newCtxt->scheduled = 0;
#endif
    newCtxt->flags = 0;
    newCtxt->offset = -1;
    return newCtxt;
}

static void	fileFinalize(struct _CFStream *stream, void *info) {
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    if (ctxt->fd > 0) {
#ifdef REAL_FILE_SCHEDULING
        if (ctxt->rlInfo.cffd) {
            CFFileDescriptorInvalidate(ctxt->rlInfo.cffd); 
            CFRelease(ctxt->rlInfo.cffd);
            ctxt->rlInfo.cffd = NULL;
        }
#endif
        close(ctxt->fd);
#ifdef REAL_FILE_SCHEDULING
    } else if (ctxt->rlInfo.rlArray) {
        CFRelease(ctxt->rlInfo.rlArray);
#endif
    }
    if (ctxt->url) {
        CFRelease(ctxt->url);
    }
    CFAllocatorDeallocate(CFGetAllocator(stream), ctxt);
}

static CFStringRef fileCopyDescription(struct _CFStream *stream, void *info) {
    // This needs work
    _CFFileStreamContext *ctxt = (_CFFileStreamContext *)info;
    if (ctxt->url) {
        return CFCopyDescription(ctxt->url);
    } else {
        return CFStringCreateWithFormat(CFGetAllocator(stream), NULL, CFSTR("fd = %d"), ctxt->fd);
    }
}

/* CFData stream callbacks */
typedef struct {
    CFDataRef data; // Mutable if the stream was constructed writable
    const UInt8 *loc; // Current location in the file
    Boolean scheduled;
    char _padding[3];
} _CFReadDataStreamContext;

#define BUF_SIZE 1024
typedef struct _CFStreamByteBuffer {
    UInt8 *bytes;
    CFIndex capacity, length;
    struct _CFStreamByteBuffer *next;
} _CFStreamByteBuffer;

typedef struct {
    _CFStreamByteBuffer *firstBuf, *currentBuf;
    CFAllocatorRef bufferAllocator;
    Boolean scheduled;
    char _padding[3];
} _CFWriteDataStreamContext;

static Boolean readDataOpen(struct _CFStream *stream, CFStreamError *errorCode, Boolean *openComplete, void *info) {
    _CFReadDataStreamContext *dataStream = (_CFReadDataStreamContext *)info;
    if (dataStream->scheduled) {
        if (CFDataGetLength(dataStream->data) != 0) {
            CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
        } else {
            CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventEndEncountered, NULL);
        }
    }
    errorCode->error = 0;
    *openComplete = TRUE;
    return TRUE;
}

static void readDataSchedule(struct _CFStream *stream, CFRunLoopRef rl, CFStringRef rlMode, void *info) {
    _CFReadDataStreamContext *dataStream = (_CFReadDataStreamContext *)info;
    if (dataStream->scheduled == FALSE) {
        dataStream->scheduled = TRUE;
		if (CFReadStreamGetStatus((CFReadStreamRef)stream) != kCFStreamStatusOpen)
			return;
        if (CFDataGetBytePtr(dataStream->data) + CFDataGetLength(dataStream->data) > dataStream->loc) {
            CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventHasBytesAvailable, NULL);
        } else {
            CFReadStreamSignalEvent((CFReadStreamRef)stream, kCFStreamEventEndEncountered, NULL);
        }
    }
}

static CFIndex dataRead(CFReadStreamRef stream, UInt8 *buffer, CFIndex bufferLength, CFStreamError *error, Boolean *atEOF, void *info) {
    _CFReadDataStreamContext *dataCtxt = (_CFReadDataStreamContext *)info;
    const UInt8 *bytePtr = CFDataGetBytePtr(dataCtxt->data);
    CFIndex length = CFDataGetLength(dataCtxt->data);
    CFIndex bytesToCopy = bytePtr + length - dataCtxt->loc;
    if (bytesToCopy > bufferLength) {
        bytesToCopy = bufferLength;
    }
    if (bytesToCopy < 0) {
        bytesToCopy = 0;
    }
    if (bytesToCopy != 0) {
        memmove(buffer, dataCtxt->loc, bytesToCopy);
        dataCtxt->loc += bytesToCopy;
    }
    error->error = 0;
    *atEOF = (dataCtxt->loc < bytePtr + length) ? FALSE : TRUE;
    if (dataCtxt->scheduled && !*atEOF) {
        CFReadStreamSignalEvent(stream, kCFStreamEventHasBytesAvailable, NULL);
    }
    return bytesToCopy;
}

static const UInt8 *dataGetBuffer(CFReadStreamRef stream, CFIndex maxBytesToRead, CFIndex *numBytesRead, CFStreamError *error, Boolean *atEOF, void *info) {
    _CFReadDataStreamContext *dataCtxt = (_CFReadDataStreamContext *)info;
    const UInt8 *bytes = CFDataGetBytePtr(dataCtxt->data);
    if (dataCtxt->loc - bytes > maxBytesToRead) {
        *numBytesRead = maxBytesToRead;
        *atEOF = FALSE;
    } else {
        *numBytesRead = dataCtxt->loc - bytes;
        *atEOF = TRUE;
    }
    error->error = 0;
    bytes = dataCtxt->loc;
    dataCtxt->loc += *numBytesRead;
    if (dataCtxt->scheduled && !*atEOF) {
        CFReadStreamSignalEvent(stream, kCFStreamEventHasBytesAvailable, NULL);
    }
    return bytes;
}

static Boolean dataCanRead(CFReadStreamRef stream, void *info) {
    _CFReadDataStreamContext *dataCtxt = (_CFReadDataStreamContext *)info;
    return (CFDataGetBytePtr(dataCtxt->data) + CFDataGetLength(dataCtxt->data) > dataCtxt->loc) ? TRUE : FALSE;
}

static Boolean writeDataOpen(struct _CFStream *stream, CFStreamError *errorCode, Boolean *openComplete, void *info) {
    _CFWriteDataStreamContext *dataStream = (_CFWriteDataStreamContext *)info;
    if (dataStream->scheduled) {
        if (dataStream->bufferAllocator != kCFAllocatorNull || dataStream->currentBuf->capacity > dataStream->currentBuf->length) {
            CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
        } else {
            CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventEndEncountered, NULL);
        }
    }
    errorCode->error = 0;
    *openComplete = TRUE;
    return TRUE;
}

static void writeDataSchedule(struct _CFStream *stream, CFRunLoopRef rl, CFStringRef rlMode, void *info) {
    _CFWriteDataStreamContext *dataStream = (_CFWriteDataStreamContext *)info;
    if (dataStream->scheduled == FALSE) {
        dataStream->scheduled = TRUE;
		if (CFWriteStreamGetStatus((CFWriteStreamRef)stream) != kCFStreamStatusOpen)
			return;
        if (dataStream->bufferAllocator != kCFAllocatorNull || dataStream->currentBuf->capacity > dataStream->currentBuf->length) {
            CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventCanAcceptBytes, NULL);
        } else {
            CFWriteStreamSignalEvent((CFWriteStreamRef)stream, kCFStreamEventEndEncountered, NULL);
        }
    }
}

static CFIndex dataWrite(CFWriteStreamRef stream, const UInt8 *buffer, CFIndex bufferLength, CFStreamError *errorCode, void *info) {
    _CFWriteDataStreamContext *dataStream = (_CFWriteDataStreamContext *)info;
    CFIndex result;
    CFIndex freeSpace = dataStream->currentBuf->capacity - dataStream->currentBuf->length;
    if (dataStream->bufferAllocator == kCFAllocatorNull && bufferLength > freeSpace) {
        errorCode->error = ENOMEM;
        errorCode->domain = kCFStreamErrorDomainPOSIX;
        return -1;
    } else {
        result = bufferLength;
        while (bufferLength > 0) {
            CFIndex amountToCopy = (bufferLength > freeSpace) ? freeSpace : bufferLength;
            if (freeSpace > 0) {
                memmove(dataStream->currentBuf->bytes + dataStream->currentBuf->length, buffer, amountToCopy);
                buffer += amountToCopy;
                bufferLength -= amountToCopy;
                dataStream->currentBuf->length += amountToCopy;
            }
            if (bufferLength > 0) {
                CFIndex bufSize = BUF_SIZE > bufferLength ? BUF_SIZE : bufferLength;
                _CFStreamByteBuffer *newBuf = (_CFStreamByteBuffer *)CFAllocatorAllocate(dataStream->bufferAllocator, sizeof(_CFStreamByteBuffer) + bufSize, 0);
                if (newBuf == NULL) {
                    errorCode->error = ENOMEM;
                    errorCode->domain = kCFStreamErrorDomainPOSIX;
                    return -1;
                } else {
                    newBuf->bytes = (UInt8 *)(newBuf + 1);
                    newBuf->capacity = bufSize;
                    newBuf->length = 0;
                    newBuf->next = NULL;
                    dataStream->currentBuf->next = newBuf;
                    dataStream->currentBuf = newBuf;
                    freeSpace = bufSize;
                }
            }
        }
        errorCode->error = 0;
    }
    if (dataStream->scheduled && (dataStream->bufferAllocator != kCFAllocatorNull || dataStream->currentBuf->capacity > dataStream->currentBuf->length)) {
        CFWriteStreamSignalEvent(stream, kCFStreamEventCanAcceptBytes, NULL);
    }
    return result;
}

static  Boolean dataCanWrite(CFWriteStreamRef stream, void *info) {
    _CFWriteDataStreamContext *dataStream = (_CFWriteDataStreamContext *)info;
    if (dataStream->bufferAllocator != kCFAllocatorNull) return TRUE;
    if (dataStream->currentBuf->capacity  > dataStream->currentBuf->length) return TRUE;
    return FALSE;
}

static CFPropertyListRef dataCopyProperty(struct _CFStream *stream, CFStringRef propertyName, void *info) {
    _CFWriteDataStreamContext *dataStream = (_CFWriteDataStreamContext *)info;
    CFIndex size = 0;
    _CFStreamByteBuffer *buf;
    CFAllocatorRef alloc;
    UInt8 *bytes, *currByte;
    if (!CFEqual(propertyName, kCFStreamPropertyDataWritten)) return NULL;
    if (dataStream->bufferAllocator == kCFAllocatorNull)  return NULL;
    alloc = dataStream->bufferAllocator;
    for (buf = dataStream->firstBuf; buf != NULL; buf = buf->next) {
        size += buf->length;
    }
    bytes = (UInt8 *)CFAllocatorAllocate(alloc, size, 0);
    currByte = bytes;
    for (buf = dataStream->firstBuf; buf != NULL; buf = buf->next) {
        memmove(currByte, buf->bytes, buf->length);
        currByte += buf->length;
    }
    return CFDataCreateWithBytesNoCopy(alloc, bytes, size, alloc);
}

static void *readDataCreate(struct _CFStream *stream, void *info) {
    _CFReadDataStreamContext *ctxt = (_CFReadDataStreamContext *)info;
    _CFReadDataStreamContext *newCtxt = (_CFReadDataStreamContext *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFReadDataStreamContext), 0);
    if (!newCtxt) return NULL;
    newCtxt->data = (CFDataRef)CFRetain(ctxt->data);
    newCtxt->loc = CFDataGetBytePtr(newCtxt->data);
    newCtxt->scheduled = FALSE;
    return (void *)newCtxt;
}

static void readDataFinalize(struct _CFStream *stream, void *info) {
    _CFReadDataStreamContext *ctxt = (_CFReadDataStreamContext *)info;
    CFRelease(ctxt->data);
    CFAllocatorDeallocate(CFGetAllocator(stream), ctxt);
}

static CFStringRef readDataCopyDescription(struct _CFStream *stream, void *info) {
    return CFCopyDescription(((_CFReadDataStreamContext *)info)->data);
}

static void *writeDataCreate(struct _CFStream *stream, void *info) {
    _CFWriteDataStreamContext *ctxt = (_CFWriteDataStreamContext *)info;
    _CFWriteDataStreamContext *newCtxt;
    if (ctxt->bufferAllocator != kCFAllocatorNull) {
        if (ctxt->bufferAllocator == NULL) ctxt->bufferAllocator = CFAllocatorGetDefault();
        CFRetain(ctxt->bufferAllocator);
        newCtxt = (_CFWriteDataStreamContext *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFWriteDataStreamContext) + sizeof(_CFStreamByteBuffer) + BUF_SIZE, 0);
        newCtxt->firstBuf = (_CFStreamByteBuffer *)(newCtxt + 1);
        newCtxt->firstBuf->bytes = (UInt8 *)(newCtxt->firstBuf + 1);
        newCtxt->firstBuf->capacity = BUF_SIZE;
        newCtxt->firstBuf->length = 0;
        newCtxt->firstBuf->next = NULL;
        newCtxt->currentBuf = newCtxt->firstBuf;
        newCtxt->bufferAllocator = ctxt->bufferAllocator;
        newCtxt->scheduled = FALSE;
    } else {
        newCtxt = (_CFWriteDataStreamContext *)CFAllocatorAllocate(CFGetAllocator(stream), sizeof(_CFWriteDataStreamContext) + sizeof(_CFStreamByteBuffer), 0);
        newCtxt->firstBuf = (_CFStreamByteBuffer *)(newCtxt+1);
        newCtxt->firstBuf->bytes = ctxt->firstBuf->bytes;
        newCtxt->firstBuf->capacity = ctxt->firstBuf->capacity;
        newCtxt->firstBuf->length = 0;
        newCtxt->firstBuf->next = NULL;
        newCtxt->currentBuf = newCtxt->firstBuf;
        newCtxt->bufferAllocator = kCFAllocatorNull;
        newCtxt->scheduled = FALSE;
    }
    return (void *)newCtxt;
}

static void writeDataFinalize(struct _CFStream *stream, void *info) {
    _CFWriteDataStreamContext *ctxt = (_CFWriteDataStreamContext *)info;
    if (ctxt->bufferAllocator != kCFAllocatorNull) {
        _CFStreamByteBuffer *buf = ctxt->firstBuf->next, *next;
        while (buf != NULL) {
            next = buf->next;
            CFAllocatorDeallocate(ctxt->bufferAllocator, buf);
            buf = next;
        }
        CFRelease(ctxt->bufferAllocator);
    }
    CFAllocatorDeallocate(CFGetAllocator(stream), ctxt);
}

static CFStringRef writeDataCopyDescription(struct _CFStream *stream, void *info) {
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFWriteDataContext %p>"), info);
}

static const struct _CFStreamCallBacksV1 fileCallBacks = {1, fileCreate, fileFinalize, fileCopyDescription, fileOpen, NULL, fileRead, NULL, fileCanRead, fileWrite, fileCanWrite, fileClose, fileCopyProperty, fileSetProperty, NULL, fileSchedule, fileUnschedule};

static struct _CFStream *_CFStreamCreateWithFile(CFAllocatorRef alloc, CFURLRef fileURL, Boolean forReading) {
    _CFFileStreamContext fileContext;
    CFStringRef scheme = fileURL ? CFURLCopyScheme(fileURL) : NULL;
    if (!scheme || !CFEqual(scheme, CFSTR("file"))) {
        if (scheme) CFRelease(scheme);
        return NULL;
    }
    CFRelease(scheme);
    fileContext.url = fileURL;
    fileContext.fd = -1;
    return _CFStreamCreateWithConstantCallbacks(alloc, &fileContext, (struct _CFStreamCallBacks *)(&fileCallBacks), forReading);
}

CF_EXPORT CFReadStreamRef CFReadStreamCreateWithFile(CFAllocatorRef alloc, CFURLRef fileURL) {
    return (CFReadStreamRef)_CFStreamCreateWithFile(alloc, fileURL, TRUE);
}

CF_EXPORT CFWriteStreamRef CFWriteStreamCreateWithFile(CFAllocatorRef alloc, CFURLRef fileURL) {
    return (CFWriteStreamRef)_CFStreamCreateWithFile(alloc, fileURL, FALSE);
}

// CFReadStreamRef takes ownership of the fd, and will close() it
CFReadStreamRef _CFReadStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd) {
    _CFFileStreamContext fileContext;
    fileContext.url = NULL;
    fileContext.fd = fd;
    return (CFReadStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &fileContext, (struct _CFStreamCallBacks *)(&fileCallBacks), TRUE);
}

// CFWriteStreamRef takes ownership of the fd, and will close() it
CFWriteStreamRef _CFWriteStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd) {
    _CFFileStreamContext fileContext;
    fileContext.url = NULL;
    fileContext.fd = fd;
    return (CFWriteStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &fileContext, (struct _CFStreamCallBacks *)(&fileCallBacks), FALSE);
}



static const struct _CFStreamCallBacksV1 readDataCallBacks = {1, readDataCreate, readDataFinalize, readDataCopyDescription, readDataOpen, NULL, dataRead, dataGetBuffer, dataCanRead, NULL, NULL, NULL, NULL, NULL, NULL, readDataSchedule, NULL};
static const struct _CFStreamCallBacksV1 writeDataCallBacks = {1, writeDataCreate, writeDataFinalize, writeDataCopyDescription, writeDataOpen, NULL, NULL, NULL, NULL, dataWrite, dataCanWrite, NULL, dataCopyProperty, NULL, NULL, writeDataSchedule, NULL};

CF_EXPORT CFReadStreamRef CFReadStreamCreateWithBytesNoCopy(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex length, CFAllocatorRef bytesDeallocator) {
    _CFReadDataStreamContext ctxt;
    CFReadStreamRef result;
    ctxt.data = CFDataCreateWithBytesNoCopy(alloc, bytes, length, bytesDeallocator);
    result = (CFReadStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &ctxt, (struct _CFStreamCallBacks *)(&readDataCallBacks), TRUE);
    CFRelease(ctxt.data);
    return result;
}

/* This needs to be exported to make it callable from Foundation. */
CF_EXPORT CFReadStreamRef CFReadStreamCreateWithData(CFAllocatorRef alloc, CFDataRef data) {
    _CFReadDataStreamContext ctxt;
    CFReadStreamRef result = NULL;
    
    ctxt.data = (CFDataRef)CFRetain(data);
    result = (CFReadStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &ctxt, (struct _CFStreamCallBacks *)(&readDataCallBacks), TRUE);
    CFRelease(data);
    return result;
}

CFWriteStreamRef CFWriteStreamCreateWithBuffer(CFAllocatorRef alloc, UInt8 *buffer, CFIndex bufferCapacity) {
    _CFStreamByteBuffer buf;
    _CFWriteDataStreamContext ctxt;
    buf.bytes = buffer;
    buf.capacity = bufferCapacity;
    buf.length = 0;
    buf.next = NULL;
    ctxt.firstBuf = &buf;
    ctxt.currentBuf = ctxt.firstBuf;
    ctxt.bufferAllocator = kCFAllocatorNull;
    return (CFWriteStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &ctxt, (struct _CFStreamCallBacks *)(&writeDataCallBacks), FALSE);
}

CF_EXPORT CFWriteStreamRef CFWriteStreamCreateWithAllocatedBuffers(CFAllocatorRef alloc, CFAllocatorRef bufferAllocator) {
    _CFWriteDataStreamContext ctxt;
    ctxt.firstBuf = NULL;
    ctxt.currentBuf = NULL;
    ctxt.bufferAllocator = bufferAllocator;
    return (CFWriteStreamRef)_CFStreamCreateWithConstantCallbacks(alloc, &ctxt, (struct _CFStreamCallBacks *)(&writeDataCallBacks), FALSE);
}

#undef BUF_SIZE

