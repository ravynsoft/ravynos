/*	CFMessagePort.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFMESSAGEPORT__)
#define __COREFOUNDATION_CFMESSAGEPORT__ 1

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFData.h>
#include <dispatch/dispatch.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMessagePort) __CFMessagePort * CFMessagePortRef;

CF_ENUM(SInt32) {
    kCFMessagePortSuccess = 0,
    kCFMessagePortSendTimeout = -1,
    kCFMessagePortReceiveTimeout = -2,
    kCFMessagePortIsInvalid = -3,
    kCFMessagePortTransportError = -4,
    kCFMessagePortBecameInvalidError = -5
};

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFMessagePortContext;

typedef CFDataRef (*CFMessagePortCallBack)(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
/* If callout wants to keep a hold of the data past the return of the callout, it must COPY the data. This includes the case where the data is given to some routine which _might_ keep a hold of it; System will release returned CFData. */
typedef void (*CFMessagePortInvalidationCallBack)(CFMessagePortRef ms, void *info);

CF_EXPORT CFTypeID	CFMessagePortGetTypeID(void);

CF_EXPORT CFMessagePortRef	CFMessagePortCreateLocal(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callout, CFMessagePortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMessagePortRef	CFMessagePortCreateRemote(CFAllocatorRef allocator, CFStringRef name);

CF_EXPORT Boolean	CFMessagePortIsRemote(CFMessagePortRef ms);
CF_EXPORT CFStringRef	CFMessagePortGetName(CFMessagePortRef ms);
CF_EXPORT Boolean	CFMessagePortSetName(CFMessagePortRef ms, CFStringRef newName);
CF_EXPORT void		CFMessagePortGetContext(CFMessagePortRef ms, CFMessagePortContext *context);
CF_EXPORT void		CFMessagePortInvalidate(CFMessagePortRef ms);
CF_EXPORT Boolean	CFMessagePortIsValid(CFMessagePortRef ms);
CF_EXPORT CFMessagePortInvalidationCallBack CFMessagePortGetInvalidationCallBack(CFMessagePortRef ms);
CF_EXPORT void CFMessagePortSetInvalidationCallBack(CFMessagePortRef ms, CFMessagePortInvalidationCallBack callout);

/* NULL replyMode argument means no return value expected, don't wait for it */
CF_EXPORT SInt32	CFMessagePortSendRequest(CFMessagePortRef remote, SInt32 msgid, CFDataRef data, CFTimeInterval sendTimeout, CFTimeInterval rcvTimeout, CFStringRef replyMode, CFDataRef *returnData);

CF_EXPORT CFRunLoopSourceRef	CFMessagePortCreateRunLoopSource(CFAllocatorRef allocator, CFMessagePortRef local, CFIndex order);

CF_EXPORT void CFMessagePortSetDispatchQueue(CFMessagePortRef ms, dispatch_queue_t queue) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFMESSAGEPORT__ */

