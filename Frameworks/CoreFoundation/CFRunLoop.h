/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFDate.h>

typedef struct __CFRunLoop *CFRunLoopRef;
typedef struct __NSTimer *CFRunLoopTimerRef;
typedef struct __NSInputSource *CFRunLoopSourceRef;
typedef struct CFRunLoopObserver *CFRunLoopObserverRef;

enum {
    kCFRunLoopRunFinished = 1,
    kCFRunLoopRunStopped = 2,
    kCFRunLoopRunTimedOut = 3,
    kCFRunLoopRunHandledSource = 4,
};

typedef enum {
    kCFRunLoopEntry = (1 << 0),
    kCFRunLoopBeforeTimers = (1 << 1),
    kCFRunLoopBeforeSources = (1 << 2),
    // skip <<3,4
    kCFRunLoopBeforeWaiting = (1 << 5),
    kCFRunLoopAfterWaiting = (1 << 6),
    kCFRunLoopExit = (1 << 7),
    kCFRunLoopAllActivities = 0xFFFFFFF
} CFRunLoopActivity;

COREFOUNDATION_EXPORT const CFStringRef kCFRunLoopCommonModes;

typedef void (*CFRunLoopCancelCallBack)(void *info, CFRunLoopRef self, CFStringRef mode);
typedef Boolean (*CFRunLoopEqualCallBack)(const void *info, const void *other);
typedef mach_port_t (*CFRunLoopGetPortCallBack)(void *info);
typedef CFHashCode (*CFRunLoopHashCallBack)(const void *info);
typedef void *(*CFRunLoopMachPerformCallBack)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info);
typedef void (*CFRunLoopPerformCallBack)(void *info);
typedef void (*CFRunLoopScheduleCallBack)(void *info, CFRunLoopRef self, CFStringRef mode);

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
    CFRunLoopEqualCallBack equal;
    CFRunLoopHashCallBack hash;
    CFRunLoopScheduleCallBack schedule;
    CFRunLoopCancelCallBack cancel;
    CFRunLoopPerformCallBack perform;
} CFRunLoopSourceContext;

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
    CFRunLoopEqualCallBack equal;
    CFRunLoopHashCallBack hash;
    CFRunLoopGetPortCallBack getPort;
    CFRunLoopMachPerformCallBack perform;
} CFRunLoopSourceContext1;

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFRunLoopObserverContext;

typedef void (*CFRunLoopObserverCallBack)(CFRunLoopObserverRef self, CFRunLoopActivity activity, void *info);

COREFOUNDATION_EXPORT CFTypeID CFRunLoopGetTypeID(void);

COREFOUNDATION_EXPORT CFRunLoopRef CFRunLoopGetCurrent(void);
COREFOUNDATION_EXPORT CFRunLoopRef CFRunLoopGetMain(void);
COREFOUNDATION_EXPORT void CFRunLoopRun(void);

COREFOUNDATION_EXPORT void CFRunLoopAddCommonMode(CFRunLoopRef self, CFStringRef mode);

COREFOUNDATION_EXPORT void CFRunLoopAddObserver(CFRunLoopRef self, CFRunLoopObserverRef observer, CFStringRef mode);
COREFOUNDATION_EXPORT void CFRunLoopRemoveObserver(CFRunLoopRef self, CFRunLoopObserverRef observer, CFStringRef mode);
COREFOUNDATION_EXPORT Boolean CFRunLoopContainsObserver(CFRunLoopRef self, CFRunLoopObserverRef observer, CFStringRef mode);

COREFOUNDATION_EXPORT void CFRunLoopAddSource(CFRunLoopRef self, CFRunLoopSourceRef source, CFStringRef mode);
COREFOUNDATION_EXPORT void CFRunLoopRemoveSource(CFRunLoopRef self, CFRunLoopSourceRef source, CFStringRef mode);
COREFOUNDATION_EXPORT Boolean CFRunLoopContainsSource(CFRunLoopRef self, CFRunLoopSourceRef source, CFStringRef mode);

COREFOUNDATION_EXPORT void CFRunLoopAddTimer(CFRunLoopRef self, CFRunLoopTimerRef timer, CFStringRef mode);
COREFOUNDATION_EXPORT void CFRunLoopRemoveTimer(CFRunLoopRef self, CFRunLoopTimerRef timer, CFStringRef mode);
COREFOUNDATION_EXPORT Boolean CFRunLoopContainsTimer(CFRunLoopRef self, CFRunLoopTimerRef timer, CFStringRef mode);

COREFOUNDATION_EXPORT CFStringRef CFRunLoopCopyCurrentMode(CFRunLoopRef self);
COREFOUNDATION_EXPORT CFArrayRef CFRunLoopCopyAllModes(CFRunLoopRef self);

COREFOUNDATION_EXPORT Boolean CFRunLoopIsWaiting(CFRunLoopRef self);
COREFOUNDATION_EXPORT CFAbsoluteTime CFRunLoopGetNextTimerFireDate(CFRunLoopRef self, CFStringRef mode);
COREFOUNDATION_EXPORT CFInteger CFRunLoopRunInMode(CFStringRef mode, CFTimeInterval seconds, Boolean returnAfterSourceHandled);
COREFOUNDATION_EXPORT void CFRunLoopStop(CFRunLoopRef self);
COREFOUNDATION_EXPORT void CFRunLoopWakeUp(CFRunLoopRef self);

// sources

COREFOUNDATION_EXPORT CFTypeID CFRunLoopSourceGetTypeID(void);

COREFOUNDATION_EXPORT CFRunLoopSourceRef CFRunLoopSourceCreate(CFAllocatorRef allocator, CFIndex order, CFRunLoopSourceContext *context);

COREFOUNDATION_EXPORT CFIndex CFRunLoopSourceGetOrder(CFRunLoopSourceRef self);
COREFOUNDATION_EXPORT void CFRunLoopSourceGetContext(CFRunLoopSourceRef self, CFRunLoopSourceContext *context);

COREFOUNDATION_EXPORT void CFRunLoopSourceInvalidate(CFRunLoopSourceRef self);
COREFOUNDATION_EXPORT Boolean CFRunLoopSourceIsValid(CFRunLoopSourceRef self);

COREFOUNDATION_EXPORT void CFRunLoopSourceSignal(CFRunLoopSourceRef self);

// observers

COREFOUNDATION_EXPORT CFTypeID CFRunLoopObserverGetTypeID(void);

COREFOUNDATION_EXPORT CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator, CFOptionFlags activities, Boolean repeats, CFIndex order, CFRunLoopObserverCallBack callback, CFRunLoopObserverContext *context);

COREFOUNDATION_EXPORT CFOptionFlags CFRunLoopObserverGetActivities(CFRunLoopObserverRef self);
COREFOUNDATION_EXPORT Boolean CFRunLoopObserverDoesRepeat(CFRunLoopObserverRef self);
COREFOUNDATION_EXPORT CFIndex CFRunLoopObserverGetOrder(CFRunLoopObserverRef self);
COREFOUNDATION_EXPORT void CFRunLoopObserverGetContext(CFRunLoopObserverRef self, CFRunLoopObserverContext *context);

COREFOUNDATION_EXPORT void CFRunLoopObserverInvalidate(CFRunLoopObserverRef self);
COREFOUNDATION_EXPORT Boolean CFRunLoopObserverIsValid(CFRunLoopObserverRef self);
