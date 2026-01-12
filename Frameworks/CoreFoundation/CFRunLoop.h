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

/*	CFRunLoop.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFRUNLOOP__)
#define __COREFOUNDATION_CFRUNLOOP__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFString.h>
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
#include <mach/port.h>
#endif

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct __CFRunLoop * CFRunLoopRef;

typedef struct __CFRunLoopSource * CFRunLoopSourceRef;

typedef struct __CFRunLoopObserver * CFRunLoopObserverRef;

typedef struct CF_BRIDGED_MUTABLE_TYPE(NSTimer) __CFRunLoopTimer * CFRunLoopTimerRef;

/* Reasons for CFRunLoopRunInMode() to Return */
enum {
    kCFRunLoopRunFinished = 1,
    kCFRunLoopRunStopped = 2,
    kCFRunLoopRunTimedOut = 3,
    kCFRunLoopRunHandledSource = 4
};

/* Run Loop Observer Activities */
typedef CF_OPTIONS(CFOptionFlags, CFRunLoopActivity) {
    kCFRunLoopEntry = (1UL << 0),
    kCFRunLoopBeforeTimers = (1UL << 1),
    kCFRunLoopBeforeSources = (1UL << 2),
    kCFRunLoopBeforeWaiting = (1UL << 5),
    kCFRunLoopAfterWaiting = (1UL << 6),
    kCFRunLoopExit = (1UL << 7),
    kCFRunLoopAllActivities = 0x0FFFFFFFU
};

CF_EXPORT const CFStringRef kCFRunLoopDefaultMode;
CF_EXPORT const CFStringRef kCFRunLoopCommonModes;

CF_EXPORT CFTypeID CFRunLoopGetTypeID(void);

CF_EXPORT CFRunLoopRef CFRunLoopGetCurrent(void);
CF_EXPORT CFRunLoopRef CFRunLoopGetMain(void);

CF_EXPORT CFStringRef CFRunLoopCopyCurrentMode(CFRunLoopRef rl);

CF_EXPORT CFArrayRef CFRunLoopCopyAllModes(CFRunLoopRef rl);

CF_EXPORT void CFRunLoopAddCommonMode(CFRunLoopRef rl, CFStringRef mode);

CF_EXPORT CFAbsoluteTime CFRunLoopGetNextTimerFireDate(CFRunLoopRef rl, CFStringRef mode);

CF_EXPORT void CFRunLoopRun(void);
CF_EXPORT SInt32 CFRunLoopRunInMode(CFStringRef mode, CFTimeInterval seconds, Boolean returnAfterSourceHandled);
CF_EXPORT Boolean CFRunLoopIsWaiting(CFRunLoopRef rl);
CF_EXPORT void CFRunLoopWakeUp(CFRunLoopRef rl);
CF_EXPORT void CFRunLoopStop(CFRunLoopRef rl);

#if __BLOCKS__
CF_EXPORT void CFRunLoopPerformBlock(CFRunLoopRef rl, CFTypeRef mode, void (^block)(void)) CF_AVAILABLE(10_6, 4_0); 
#endif

CF_EXPORT Boolean CFRunLoopContainsSource(CFRunLoopRef rl, CFRunLoopSourceRef source, CFStringRef mode);
CF_EXPORT void CFRunLoopAddSource(CFRunLoopRef rl, CFRunLoopSourceRef source, CFStringRef mode);
CF_EXPORT void CFRunLoopRemoveSource(CFRunLoopRef rl, CFRunLoopSourceRef source, CFStringRef mode);

CF_EXPORT Boolean CFRunLoopContainsObserver(CFRunLoopRef rl, CFRunLoopObserverRef observer, CFStringRef mode);
CF_EXPORT void CFRunLoopAddObserver(CFRunLoopRef rl, CFRunLoopObserverRef observer, CFStringRef mode);
CF_EXPORT void CFRunLoopRemoveObserver(CFRunLoopRef rl, CFRunLoopObserverRef observer, CFStringRef mode);

CF_EXPORT Boolean CFRunLoopContainsTimer(CFRunLoopRef rl, CFRunLoopTimerRef timer, CFStringRef mode);
CF_EXPORT void CFRunLoopAddTimer(CFRunLoopRef rl, CFRunLoopTimerRef timer, CFStringRef mode);
CF_EXPORT void CFRunLoopRemoveTimer(CFRunLoopRef rl, CFRunLoopTimerRef timer, CFStringRef mode);

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
    Boolean	(*equal)(const void *info1, const void *info2);
    CFHashCode	(*hash)(const void *info);
    void	(*schedule)(void *info, CFRunLoopRef rl, CFStringRef mode);
    void	(*cancel)(void *info, CFRunLoopRef rl, CFStringRef mode);
    void	(*perform)(void *info);
} CFRunLoopSourceContext;

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
    Boolean	(*equal)(const void *info1, const void *info2);
    CFHashCode	(*hash)(const void *info);
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
    mach_port_t	(*getPort)(void *info);
    void *	(*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info);
#else
    void *	(*getPort)(void *info);
    void	(*perform)(void *info);
#endif
} CFRunLoopSourceContext1;

CF_EXPORT CFTypeID CFRunLoopSourceGetTypeID(void);

CF_EXPORT CFRunLoopSourceRef CFRunLoopSourceCreate(CFAllocatorRef allocator, CFIndex order, CFRunLoopSourceContext *context);

CF_EXPORT CFIndex CFRunLoopSourceGetOrder(CFRunLoopSourceRef source);
CF_EXPORT void CFRunLoopSourceInvalidate(CFRunLoopSourceRef source);
CF_EXPORT Boolean CFRunLoopSourceIsValid(CFRunLoopSourceRef source);
CF_EXPORT void CFRunLoopSourceGetContext(CFRunLoopSourceRef source, CFRunLoopSourceContext *context);
CF_EXPORT void CFRunLoopSourceSignal(CFRunLoopSourceRef source);

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFRunLoopObserverContext;

typedef void (*CFRunLoopObserverCallBack)(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info);

CF_EXPORT CFTypeID CFRunLoopObserverGetTypeID(void);

CF_EXPORT CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator, CFOptionFlags activities, Boolean repeats, CFIndex order, CFRunLoopObserverCallBack callout, CFRunLoopObserverContext *context);
#if __BLOCKS__
CF_EXPORT CFRunLoopObserverRef CFRunLoopObserverCreateWithHandler(CFAllocatorRef allocator, CFOptionFlags activities, Boolean repeats, CFIndex order, void (^block) (CFRunLoopObserverRef observer, CFRunLoopActivity activity)) CF_AVAILABLE(10_7, 5_0);
#endif

CF_EXPORT CFOptionFlags CFRunLoopObserverGetActivities(CFRunLoopObserverRef observer);
CF_EXPORT Boolean CFRunLoopObserverDoesRepeat(CFRunLoopObserverRef observer);
CF_EXPORT CFIndex CFRunLoopObserverGetOrder(CFRunLoopObserverRef observer);
CF_EXPORT void CFRunLoopObserverInvalidate(CFRunLoopObserverRef observer);
CF_EXPORT Boolean CFRunLoopObserverIsValid(CFRunLoopObserverRef observer);
CF_EXPORT void CFRunLoopObserverGetContext(CFRunLoopObserverRef observer, CFRunLoopObserverContext *context);

typedef struct {
    CFIndex	version;
    void *	info;
    const void *(*retain)(const void *info);
    void	(*release)(const void *info);
    CFStringRef	(*copyDescription)(const void *info);
} CFRunLoopTimerContext;

typedef void (*CFRunLoopTimerCallBack)(CFRunLoopTimerRef timer, void *info);

CF_EXPORT CFTypeID CFRunLoopTimerGetTypeID(void);

CF_EXPORT CFRunLoopTimerRef CFRunLoopTimerCreate(CFAllocatorRef allocator, CFAbsoluteTime fireDate, CFTimeInterval interval, CFOptionFlags flags, CFIndex order, CFRunLoopTimerCallBack callout, CFRunLoopTimerContext *context);
#if __BLOCKS__
CF_EXPORT CFRunLoopTimerRef CFRunLoopTimerCreateWithHandler(CFAllocatorRef allocator, CFAbsoluteTime fireDate, CFTimeInterval interval, CFOptionFlags flags, CFIndex order, void (^block) (CFRunLoopTimerRef timer)) CF_AVAILABLE(10_7, 5_0);
#endif

CF_EXPORT CFAbsoluteTime CFRunLoopTimerGetNextFireDate(CFRunLoopTimerRef timer);
CF_EXPORT void CFRunLoopTimerSetNextFireDate(CFRunLoopTimerRef timer, CFAbsoluteTime fireDate);
CF_EXPORT CFTimeInterval CFRunLoopTimerGetInterval(CFRunLoopTimerRef timer);
CF_EXPORT Boolean CFRunLoopTimerDoesRepeat(CFRunLoopTimerRef timer);
CF_EXPORT CFIndex CFRunLoopTimerGetOrder(CFRunLoopTimerRef timer);
CF_EXPORT void CFRunLoopTimerInvalidate(CFRunLoopTimerRef timer);
CF_EXPORT Boolean CFRunLoopTimerIsValid(CFRunLoopTimerRef timer);
CF_EXPORT void CFRunLoopTimerGetContext(CFRunLoopTimerRef timer, CFRunLoopTimerContext *context);

// Setting a tolerance for a timer allows it to fire later than the scheduled fire date, improving the ability of the system to optimize for increased power savings and responsiveness. The timer may fire at any time between its scheduled fire date and the scheduled fire date plus the tolerance. The timer will not fire before the scheduled fire date. For repeating timers, the next fire date is calculated from the original fire date regardless of tolerance applied at individual fire times, to avoid drift. The default value is zero, which means no additional tolerance is applied. The system reserves the right to apply a small amount of tolerance to certain timers regardless of the value of this property.
// As the user of the timer, you will have the best idea of what an appropriate tolerance for a timer may be. A general rule of thumb, though, is to set the tolerance to at least 10% of the interval, for a repeating timer. Even a small amount of tolerance will have a significant positive impact on the power usage of your application. The system may put a maximum value of the tolerance.
CF_EXPORT CFTimeInterval CFRunLoopTimerGetTolerance(CFRunLoopTimerRef timer) CF_AVAILABLE(10_9, 7_0);
CF_EXPORT void CFRunLoopTimerSetTolerance(CFRunLoopTimerRef timer, CFTimeInterval tolerance) CF_AVAILABLE(10_9, 7_0);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFRUNLOOP__ */

