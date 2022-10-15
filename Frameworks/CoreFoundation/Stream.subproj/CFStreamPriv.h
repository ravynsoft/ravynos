/*	CFStreamPriv.h
	Copyright (c) 2000-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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

CF_EXPORT
CFReadStreamRef _CFReadStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);

CF_EXPORT
CFWriteStreamRef _CFWriteStreamCreateFromFileDescriptor(CFAllocatorRef alloc, int fd);



#define SECURITY_NONE   (0)
#define SECURITY_SSLv2  (1)
#define SECURITY_SSLv3  (2)
#define SECURITY_SSLv32 (3)
#define SECURITY_TLS    (4)    

#if TARGET_OS_OSX || TARGET_OS_IPHONE
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

/*
 *  kCFStreamPropertySocketSSLContext
 *
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFDataRef
 *    containing a reference to the SecureTransport SecureContext
 *    structure.
 *
 */
CF_EXPORT const CFStringRef kCFStreamPropertySocketSSLContext;

CF_EXTERN_C_END

/*
 * for CF{Read/Write}StreamCopyProperty created from a file.  The
 * result is a CFDataRef containing sizeof(int) bytes in machine byte
 * ordering representing the file descriptor of the underlying open
 * file.  If the underlying file descriptor is not open, the property
 * value will be NULL (as opposed to containing ((int) -1)).
 */
CF_EXPORT const CFStringRef _kCFStreamPropertyFileNativeHandle API_AVAILABLE(macosx(10.12.4), ios(5.0), watchos(2.0), tvos(9.0));

/*
 * SPI: The _kCFStreamPropertyHTTPTrailer property is a dictionary of HTTP headers & values that the caller
 *		wishes to have present in the trailer section of a chunked-encoded message body.  The caller is responsible
 *		for setting this property BEFORE the stream event NSStreamEventEndEncountered is signaled to the reader of the
 *		stream on which this property is set.
 */
CF_EXPORT const CFStringRef _kCFStreamPropertyHTTPTrailer API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));

#endif /* ! __COREFOUNDATION_CFSTREAMPRIV__ */

