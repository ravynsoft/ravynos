/*	CFMachPort.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFMACHPORT__)
#define __COREFOUNDATION_CFMACHPORT__ 1

#include <CoreFoundation/CFRunLoop.h>
#if TARGET_OS_MAC
#include <mach/port.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMachPort) __CFMachPort * CFMachPortRef;

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFMachPortContext;

typedef void (*CFMachPortCallBack)(CFMachPortRef port, void *msg, CFIndex size, void *info);
typedef void (*CFMachPortInvalidationCallBack)(CFMachPortRef port, void *info);

CF_EXPORT CFTypeID	CFMachPortGetTypeID(void);

CF_EXPORT CFMachPortRef	CFMachPortCreate(CFAllocatorRef allocator, CFMachPortCallBack callout, CFMachPortContext *context, Boolean *shouldFreeInfo);
CF_EXPORT CFMachPortRef	CFMachPortCreateWithPort(CFAllocatorRef allocator, mach_port_t portNum, CFMachPortCallBack callout, CFMachPortContext *context, Boolean *shouldFreeInfo);

CF_EXPORT mach_port_t	CFMachPortGetPort(CFMachPortRef port);
CF_EXPORT void		CFMachPortGetContext(CFMachPortRef port, CFMachPortContext *context);
CF_EXPORT void		CFMachPortInvalidate(CFMachPortRef port);
CF_EXPORT Boolean	CFMachPortIsValid(CFMachPortRef port);
CF_EXPORT CFMachPortInvalidationCallBack CFMachPortGetInvalidationCallBack(CFMachPortRef port);
CF_EXPORT void		CFMachPortSetInvalidationCallBack(CFMachPortRef port, CFMachPortInvalidationCallBack callout);

CF_EXPORT CFRunLoopSourceRef	CFMachPortCreateRunLoopSource(CFAllocatorRef allocator, CFMachPortRef port, CFIndex order);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif

#endif /* ! __COREFOUNDATION_CFMACHPORT__ */

