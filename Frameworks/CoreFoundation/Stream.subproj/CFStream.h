/*	CFStream.h
	Copyright (c) 2000-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFSTREAM__)
#define __COREFOUNDATION_CFSTREAM__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFError.h>
#include <dispatch/dispatch.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct {
    CFIndex domain;
    SInt32 error;
} CFStreamError;

typedef CFStringRef CFStreamPropertyKey CF_EXTENSIBLE_STRING_ENUM;

typedef CF_ENUM(CFIndex, CFStreamStatus) {
    kCFStreamStatusNotOpen = 0,
    kCFStreamStatusOpening,  /* open is in-progress */
    kCFStreamStatusOpen,
    kCFStreamStatusReading,
    kCFStreamStatusWriting,
    kCFStreamStatusAtEnd,    /* no further bytes can be read/written */
    kCFStreamStatusClosed,
    kCFStreamStatusError
};

typedef CF_OPTIONS(CFOptionFlags, CFStreamEventType) {
    kCFStreamEventNone = 0,
    kCFStreamEventOpenCompleted = 1,
    kCFStreamEventHasBytesAvailable = 2,
    kCFStreamEventCanAcceptBytes = 4, 
    kCFStreamEventErrorOccurred = 8,
    kCFStreamEventEndEncountered = 16
};

typedef struct {
    CFIndex version;
    void * _Null_unspecified info;
    void *_Null_unspecified(* _Null_unspecified retain)(void * _Null_unspecified info);
    void (* _Null_unspecified release)(void * _Null_unspecified info);
    CFStringRef _Null_unspecified (* _Null_unspecified copyDescription)(void * _Null_unspecified info);
} CFStreamClientContext;

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSInputStream) __CFReadStream * CFReadStreamRef;
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSOutputStream) __CFWriteStream * CFWriteStreamRef;

typedef void (*CFReadStreamClientCallBack)(CFReadStreamRef _Null_unspecified stream, CFStreamEventType type, void * _Null_unspecified clientCallBackInfo);
typedef void (*CFWriteStreamClientCallBack)(CFWriteStreamRef _Null_unspecified stream, CFStreamEventType type, void * _Null_unspecified clientCallBackInfo);

CF_EXPORT
CFTypeID CFReadStreamGetTypeID(void);
CF_EXPORT
CFTypeID CFWriteStreamGetTypeID(void);

/* Memory streams */

/* Value will be a CFData containing all bytes thusfar written; used to recover the data written to a memory write stream. */
CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertyDataWritten;

/* Pass kCFAllocatorNull for bytesDeallocator to prevent CFReadStream from deallocating bytes; otherwise, CFReadStream will deallocate bytes when the stream is destroyed */
CF_EXPORT
CFReadStreamRef _Null_unspecified CFReadStreamCreateWithBytesNoCopy(CFAllocatorRef _Null_unspecified alloc, const UInt8 * _Null_unspecified bytes, CFIndex length, CFAllocatorRef _Null_unspecified bytesDeallocator);

/* The stream writes into the buffer given; when bufferCapacity is exhausted, the stream is exhausted (status becomes kCFStreamStatusAtEnd) */
CF_EXPORT
CFWriteStreamRef _Null_unspecified CFWriteStreamCreateWithBuffer(CFAllocatorRef _Null_unspecified alloc, UInt8 * _Null_unspecified buffer, CFIndex bufferCapacity);

/* New buffers are allocated from bufferAllocator as bytes are written to the stream.  At any point, you can recover the bytes thusfar written by asking for the property kCFStreamPropertyDataWritten, above */
CF_EXPORT
CFWriteStreamRef _Null_unspecified CFWriteStreamCreateWithAllocatedBuffers(CFAllocatorRef _Null_unspecified alloc, CFAllocatorRef _Null_unspecified bufferAllocator);

/* File streams */
CF_EXPORT
CFReadStreamRef _Null_unspecified CFReadStreamCreateWithFile(CFAllocatorRef _Null_unspecified alloc, CFURLRef  _Null_unspecified fileURL);
CF_EXPORT
CFWriteStreamRef _Null_unspecified CFWriteStreamCreateWithFile(CFAllocatorRef _Null_unspecified alloc, CFURLRef _Null_unspecified fileURL);
CF_IMPLICIT_BRIDGING_DISABLED
CF_EXPORT
void CFStreamCreateBoundPair(CFAllocatorRef  _Null_unspecified alloc, CFReadStreamRef _Null_unspecified * _Null_unspecified readStream, CFWriteStreamRef _Null_unspecified * _Null_unspecified writeStream, CFIndex transferBufferSize);
CF_IMPLICIT_BRIDGING_ENABLED

/* Property for file write streams; value should be a CFBoolean.  Set to TRUE to append to a file, rather than to replace its contents */
CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertyAppendToFile;

CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertyFileCurrentOffset;   // Value is a CFNumber


/* Socket stream properties */

/* Value will be a CFData containing the native handle */
CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertySocketNativeHandle;

/* Value will be a CFString, or NULL if unknown */
CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertySocketRemoteHostName;

/* Value will be a CFNumber, or NULL if unknown */
CF_EXPORT
const CFStreamPropertyKey _Null_unspecified kCFStreamPropertySocketRemotePortNumber;
/*
 *  kCFStreamErrorDomainSOCKS
 *
 *  Discussion:
 *    SOCKS proxy error domain.  Errors formulated using inlines below.
 *
 */
CF_EXPORT const int kCFStreamErrorDomainSOCKS CF_AVAILABLE(10_0, 2_0);

/*
 *  kCFStreamPropertySOCKSProxy
 *
 *  Discussion:
 *    Stream property key, for both set and copy operations.  To set a
 *    stream to use a SOCKS proxy, call CFReadStreamSetProperty or
 *    CFWriteStreamSetProperty with the property name set to
 *    kCFStreamPropertySOCKSProxy and the value being a dictionary with
 *    at least the following two keys: kCFStreamPropertySOCKSProxyHost
 *    and kCFStreamPropertySOCKSProxyPort.  The dictionary returned by
 *    SystemConfiguration for SOCKS proxies will work without
 *    alteration.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSProxy CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySOCKSProxyHost
 *
 *  Discussion:
 *    CFDictionary key for SOCKS proxy information.  The key
 *    kCFStreamPropertySOCKSProxyHost should contain a CFStringRef
 *    value representing the SOCKS proxy host.  Defined to match
 *    kSCPropNetProxiesSOCKSProxy
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSProxyHost CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySOCKSProxyPort
 *
 *  Discussion:
 *    CFDictionary key for SOCKS proxy information.  The key
 *    kCFStreamPropertySOCKSProxyPort should contain a CFNumberRef
 *    which itself is of type kCFNumberSInt32Type.  This value should
 *    represent the port on which the proxy is listening.  Defined to
 *    match kSCPropNetProxiesSOCKSPort
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSProxyPort CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySOCKSVersion
 *
 *  Discussion:
 *    CFDictionary key for SOCKS proxy information.  By default, SOCKS5
 *    will be used unless there is a kCFStreamPropertySOCKSVersion key
 *    in the dictionary.  Its value must be
 *    kCFStreamSocketSOCKSVersion4 or kCFStreamSocketSOCKSVersion5 to
 *    set SOCKS4 or SOCKS5, respectively.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSVersion CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamSocketSOCKSVersion4
 *
 *  Discussion:
 *    CFDictionary value for SOCKS proxy information.  Indcates that
 *    SOCKS will or is using version 4 of the SOCKS protocol.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSOCKSVersion4 CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamSocketSOCKSVersion5
 *
 *  Discussion:
 *    CFDictionary value for SOCKS proxy information.  Indcates that
 *    SOCKS will or is using version 5 of the SOCKS protocol.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSOCKSVersion5 CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySOCKSUser
 *
 *  Discussion:
 *    CFDictionary key for SOCKS proxy information.  To set a user name
 *    and/or password, if required, the dictionary must contain the
 *    key(s) kCFStreamPropertySOCKSUser and/or
 *    kCFStreamPropertySOCKSPassword with the value being the user's
 *    name as a CFStringRef and/or the user's password as a
 *    CFStringRef, respectively.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSUser CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySOCKSPassword
 *
 *  Discussion:
 *    CFDictionary key for SOCKS proxy information.  To set a user name
 *    and/or password, if required, the dictionary must contain the
 *    key(s) kCFStreamPropertySOCKSUser and/or
 *    kCFStreamPropertySOCKSPassword with the value being the user's
 *    name as a CFStringRef and/or the user's password as a
 *    CFStringRef, respectively.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySOCKSPassword CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamErrorDomainSSL
 *
 *  Discussion:
 *    Errors located in Security/SecureTransport.h
 *
 */
CF_EXPORT const int kCFStreamErrorDomainSSL CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertySocketSecurityLevel
 *
 *  Discussion:
 *    Stream property key, for both set and copy operations. To set a
 *    stream to be secure, call CFReadStreamSetProperty or
 *    CFWriteStreamSetPropertywith the property name set to
 *    kCFStreamPropertySocketSecurityLevel and the value being one of
 *    the following values.  Streams may set a security level after
 *    open in order to allow on-the-fly securing of a stream.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertySocketSecurityLevel CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamSocketSecurityLevelNone
 *
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to use no security (default setting).
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSecurityLevelNone CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamSocketSecurityLevelSSLv2
 *
 *  Note: SSLv2 is DEPRECATED starting in OS X 10.12 and iOS 10.0.
 *
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to use SSLv2 security.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSecurityLevelSSLv2 CF_DEPRECATED(10_2, 10_12, 2_0, 10_0);

/*
 *  kCFStreamSocketSecurityLevelSSLv3
 *
 *  Note: SSLv3 is DEPRECATED starting in OS X 10.12 and iOS 10.0.
 *
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to use SSLv3 security.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSecurityLevelSSLv3 CF_DEPRECATED(10_2, 10_12, 2_0, 10_0);

/*
 *  kCFStreamSocketSecurityLevelTLSv1
 *
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to use TLSv1 security.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSecurityLevelTLSv1 CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamSocketSecurityLevelNegotiatedSSL
 *
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to use TLS or SSL with fallback to lower versions. This
 *    is what HTTPS does, for instance.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamSocketSecurityLevelNegotiatedSSL CF_AVAILABLE(10_2, 2_0);

/*
 *  kCFStreamPropertyShouldCloseNativeSocket
 *
 *  Discussion:
 *    Set the value to kCFBooleanTrue if the stream should close and
 *    release the underlying native socket when the stream is released.
 *     Set the value to kCFBooleanFalse to keep the native socket from
 *    closing and releasing when the stream is released. If the stream
 *    was created with a native socket, the default property setting on
 *    the stream is kCFBooleanFalse. The
 *    kCFStreamPropertyShouldCloseNativeSocket can be set through
 *    CFReadStreamSetProperty or CFWriteStreamSetProperty.  The
 *    property can be copied through CFReadStreamCopyProperty or
 *    CFWriteStreamCopyProperty.
 *
 */
CF_EXPORT const CFStringRef _Nonnull kCFStreamPropertyShouldCloseNativeSocket CF_AVAILABLE(10_2, 2_0);

CF_IMPLICIT_BRIDGING_DISABLED
/* Socket streams; the returned streams are paired such that they use the same socket; pass NULL if you want only the read stream or the write stream */
CF_EXPORT
void CFStreamCreatePairWithSocket(CFAllocatorRef _Null_unspecified alloc, CFSocketNativeHandle sock, CFReadStreamRef _Null_unspecified *   _Null_unspecified readStream, CFWriteStreamRef _Null_unspecified * _Null_unspecified writeStream) API_DEPRECATED("Use nw_connection_t in Network framework instead", macos(10.1, API_TO_BE_DEPRECATED), ios(2.0, API_TO_BE_DEPRECATED), watchos(2.0, API_TO_BE_DEPRECATED), tvos(9.0, API_TO_BE_DEPRECATED));
CF_EXPORT
void CFStreamCreatePairWithSocketToHost(CFAllocatorRef _Null_unspecified alloc, CFStringRef _Null_unspecified host, UInt32 port, CFReadStreamRef _Null_unspecified * _Null_unspecified readStream, CFWriteStreamRef _Null_unspecified * _Null_unspecified writeStream) API_DEPRECATED("Use nw_connection_t in Network framework instead", macos(10.1, API_TO_BE_DEPRECATED), ios(2.0, API_TO_BE_DEPRECATED), watchos(2.0, API_TO_BE_DEPRECATED), tvos(9.0, API_TO_BE_DEPRECATED));
CF_EXPORT
void CFStreamCreatePairWithPeerSocketSignature(CFAllocatorRef _Null_unspecified alloc, const CFSocketSignature * _Null_unspecified signature, CFReadStreamRef _Null_unspecified * _Null_unspecified readStream, CFWriteStreamRef _Null_unspecified * _Null_unspecified writeStream) API_DEPRECATED("Use nw_connection_t in Network framework instead", macos(10.1, API_TO_BE_DEPRECATED), ios(2.0, API_TO_BE_DEPRECATED), watchos(2.0, API_TO_BE_DEPRECATED), tvos(9.0, API_TO_BE_DEPRECATED));
CF_IMPLICIT_BRIDGING_ENABLED


/* Returns the current state of the stream */
CF_EXPORT
CFStreamStatus CFReadStreamGetStatus(CFReadStreamRef _Null_unspecified stream);
CF_EXPORT
CFStreamStatus CFWriteStreamGetStatus(CFWriteStreamRef _Null_unspecified stream);

/* Returns NULL if no error has occurred; otherwise returns the error. */
CF_EXPORT
CFErrorRef _Null_unspecified CFReadStreamCopyError(CFReadStreamRef _Null_unspecified stream) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CF_EXPORT
CFErrorRef _Null_unspecified CFWriteStreamCopyError(CFWriteStreamRef _Null_unspecified stream) API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

/* Returns success/failure.  Opening a stream causes it to reserve all the system
   resources it requires.  If the stream can open non-blocking, this will always 
   return TRUE; listen to the run loop source to find out when the open completes
   and whether it was successful, or poll using CFRead/WriteStreamGetStatus(), waiting 
   for a status of kCFStreamStatusOpen or kCFStreamStatusError.  */
CF_EXPORT
Boolean CFReadStreamOpen(CFReadStreamRef _Null_unspecified stream);
CF_EXPORT
Boolean CFWriteStreamOpen(CFWriteStreamRef _Null_unspecified stream);

/* Terminates the flow of bytes; releases any system resources required by the 
   stream.  The stream may not fail to close.  You may call CFStreamClose() to 
   effectively abort a stream. */
CF_EXPORT
void CFReadStreamClose(CFReadStreamRef _Null_unspecified stream);
CF_EXPORT
void CFWriteStreamClose(CFWriteStreamRef _Null_unspecified stream);

/* Whether there is data currently available for reading; returns TRUE if it's 
   impossible to tell without trying */
CF_EXPORT
Boolean CFReadStreamHasBytesAvailable(CFReadStreamRef _Null_unspecified stream);

/* Returns the number of bytes read, or -1 if an error occurs preventing any 
   bytes from being read, or 0 if the stream's end was encountered.  
   It is an error to try and read from a stream that hasn't been opened first.  
   This call will block until at least one byte is available; it will NOT block
   until the entire buffer can be filled.  To avoid blocking, either poll using
   CFReadStreamHasBytesAvailable() or use the run loop and listen for the 
   kCFStreamEventHasBytesAvailable event for notification of data available. */
CF_EXPORT
CFIndex CFReadStreamRead(CFReadStreamRef _Null_unspecified stream, UInt8 * _Null_unspecified buffer, CFIndex bufferLength);

/* Returns a pointer to an internal buffer if possible (setting *numBytesRead
   to the length of the returned buffer), otherwise returns NULL; guaranteed
   to return in O(1).  Bytes returned in the buffer are considered read from
   the stream; if maxBytesToRead is greater than 0, not more than maxBytesToRead
   will be returned.  If maxBytesToRead is less than or equal to zero, as many bytes
   as are readily available will be returned.  The returned buffer is good only
   until the next stream operation called on the stream.  Caller should neither
   change the contents of the returned buffer nor attempt to deallocate the buffer;
   it is still owned by the stream. */
CF_EXPORT
const UInt8 * _Null_unspecified CFReadStreamGetBuffer(CFReadStreamRef _Null_unspecified stream, CFIndex maxBytesToRead, CFIndex * _Null_unspecified numBytesRead);

/* Whether the stream can currently be written to without blocking;
   returns TRUE if it's impossible to tell without trying */
CF_EXPORT
Boolean CFWriteStreamCanAcceptBytes(CFWriteStreamRef _Null_unspecified stream);

/* Returns the number of bytes successfully written, -1 if an error has
   occurred, or 0 if the stream has been filled to capacity (for fixed-length
   streams).  If the stream is not full, this call will block until at least
   one byte is written.  To avoid blocking, either poll via CFWriteStreamCanAcceptBytes
   or use the run loop and listen for the kCFStreamEventCanAcceptBytes event. */
CF_EXPORT
CFIndex CFWriteStreamWrite(CFWriteStreamRef _Null_unspecified stream, const UInt8 * _Null_unspecified buffer, CFIndex bufferLength);

/* Particular streams can name properties and assign meanings to them; you
   access these properties through the following calls.  A property is any interesting
   information about the stream other than the data being transmitted itself.
   Examples include the headers from an HTTP transmission, or the expected 
   number of bytes, or permission information, etc.  Properties that can be set
   configure the behavior of the stream, and may only be settable at particular times
   (like before the stream has been opened).  See the documentation for particular 
   properties to determine their get- and set-ability. */
CF_EXPORT
CFTypeRef  _Null_unspecified CFReadStreamCopyProperty(CFReadStreamRef _Null_unspecified stream, CFStreamPropertyKey _Null_unspecified propertyName);
CF_EXPORT
CFTypeRef _Null_unspecified CFWriteStreamCopyProperty(CFWriteStreamRef _Null_unspecified stream, CFStreamPropertyKey _Null_unspecified propertyName);

/* Returns TRUE if the stream recognizes and accepts the given property-value pair; 
   FALSE otherwise. */
CF_EXPORT
Boolean CFReadStreamSetProperty(CFReadStreamRef _Null_unspecified stream, CFStreamPropertyKey _Null_unspecified propertyName, CFTypeRef _Null_unspecified propertyValue);
CF_EXPORT
Boolean CFWriteStreamSetProperty(CFWriteStreamRef _Null_unspecified stream, CFStreamPropertyKey _Null_unspecified propertyName, CFTypeRef _Null_unspecified propertyValue);

/* Asynchronous processing - If you wish to neither poll nor block, you may register 
   a client to hear about interesting events that occur on a stream.  Only one client
   per stream is allowed; registering a new client replaces the previous one.
 
   Once you have set a client, the stream must be scheduled to provide the context in
   which the client will be called.  Streams may be scheduled on a single dispatch queue
   or on one or more run loops.  If scheduled on a run loop, it is the caller's responsibility
   to ensure that at least one of the scheduled run loops is being run.

   NOTE: Unlike other CoreFoundation APIs, pasing a NULL clientContext here will remove
   the client.  If you do not care about the client context (i.e. your only concern
   is that your callback be called), you should pass in a valid context where every
   entry is 0 or NULL.

*/

CF_EXPORT
Boolean CFReadStreamSetClient(CFReadStreamRef _Null_unspecified stream, CFOptionFlags streamEvents, CFReadStreamClientCallBack _Null_unspecified clientCB, CFStreamClientContext * _Null_unspecified clientContext);
CF_EXPORT
Boolean CFWriteStreamSetClient(CFWriteStreamRef _Null_unspecified stream, CFOptionFlags streamEvents, CFWriteStreamClientCallBack _Null_unspecified clientCB, CFStreamClientContext * _Null_unspecified clientContext);

CF_EXPORT
void CFReadStreamScheduleWithRunLoop(CFReadStreamRef _Null_unspecified stream, CFRunLoopRef _Null_unspecified runLoop, CFRunLoopMode _Null_unspecified runLoopMode);
CF_EXPORT
void CFWriteStreamScheduleWithRunLoop(CFWriteStreamRef  _Null_unspecified stream, CFRunLoopRef _Null_unspecified runLoop, _Null_unspecified CFRunLoopMode runLoopMode);

CF_EXPORT
void CFReadStreamUnscheduleFromRunLoop(CFReadStreamRef _Null_unspecified stream, CFRunLoopRef _Null_unspecified runLoop, CFRunLoopMode _Null_unspecified runLoopMode);
CF_EXPORT
void CFWriteStreamUnscheduleFromRunLoop(CFWriteStreamRef _Null_unspecified stream, CFRunLoopRef _Null_unspecified runLoop, CFRunLoopMode _Null_unspecified runLoopMode);


/*
 * Specify the dispatch queue upon which the client callbacks will be invoked.
 * Passing NULL for the queue will prevent future callbacks from being invoked.
 * Specifying a dispatch queue using this API will unschedule the stream from
 * any run loops it had previously been scheduled upon - similarly, scheduling
 * with a runloop will disassociate the stream from any existing dispatch queue.
 */
CF_EXPORT
void CFReadStreamSetDispatchQueue(CFReadStreamRef _Null_unspecified stream, dispatch_queue_t _Null_unspecified q) API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));

CF_EXPORT
void CFWriteStreamSetDispatchQueue(CFWriteStreamRef _Null_unspecified stream, dispatch_queue_t _Null_unspecified q) API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));

/*
 * Returns the previously set dispatch queue with an incremented retain count.  
 * Note that the stream's queue may have been set to NULL if the stream was 
 * scheduled on a runloop subsequent to it having had a dispatch queue set.
 */
CF_EXPORT
dispatch_queue_t _Null_unspecified CFReadStreamCopyDispatchQueue(CFReadStreamRef _Null_unspecified stream) API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));

CF_EXPORT
dispatch_queue_t _Null_unspecified CFWriteStreamCopyDispatchQueue(CFWriteStreamRef _Null_unspecified stream) API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));


/* The following API is deprecated starting in 10.5; please use CFRead/WriteStreamCopyError(), above, instead */
typedef CF_ENUM(CFIndex, CFStreamErrorDomain) {
    kCFStreamErrorDomainCustom = -1L,      /* custom to the kind of stream in question */
    kCFStreamErrorDomainPOSIX = 1,        /* POSIX errno; interpret using <sys/errno.h> */
    kCFStreamErrorDomainMacOSStatus      /* OSStatus type from Carbon APIs; interpret using <MacTypes.h> */
};

CF_EXPORT
CFStreamError CFReadStreamGetError(CFReadStreamRef _Null_unspecified stream);
CF_EXPORT
CFStreamError CFWriteStreamGetError(CFWriteStreamRef _Null_unspecified stream);


CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFSTREAM__ */
