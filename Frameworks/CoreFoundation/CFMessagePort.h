/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFRunLoop.h>

typedef struct __NSMessagePort *CFMessagePortRef;

typedef void (*CFMessagePortInvalidationCallBack)(CFMessagePortRef self, void *info);

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFMessagePortContext;

enum {
    kCFMessagePortSuccess = 0,
    kCFMessagePortSendTimeout = -1,
    kCFMessagePortReceiveTimeout = -2,
    kCFMessagePortIsInvalid = -3,
    kCFMessagePortTransportError = -4,
};

typedef CFDataRef (*CFMessagePortCallBack)(CFMessagePortRef local, CFInteger msgId, CFDataRef data, void *info);

COREFOUNDATION_EXPORT CFTypeID CFMessagePortGetTypeID(void);

COREFOUNDATION_EXPORT CFMessagePortRef CFMessagePortCreateLocal(CFAllocatorRef allocator, CFStringRef name, CFMessagePortCallBack callback, CFMessagePortContext *context, Boolean *callerFreeInfo);
COREFOUNDATION_EXPORT CFMessagePortRef CFMessagePortCreateRemote(CFAllocatorRef allocator, CFStringRef name);

COREFOUNDATION_EXPORT CFRunLoopSourceRef CFMessagePortCreateRunLoopSource(CFAllocatorRef allocator, CFMessagePortRef self, CFIndex order);
COREFOUNDATION_EXPORT void CFMessagePortGetContext(CFMessagePortRef self, CFMessagePortContext *context);
COREFOUNDATION_EXPORT CFMessagePortInvalidationCallBack CFMessagePortGetInvalidationCallBack(CFMessagePortRef self);
COREFOUNDATION_EXPORT CFStringRef CFMessagePortGetName(CFMessagePortRef self);
COREFOUNDATION_EXPORT void CFMessagePortInvalidate(CFMessagePortRef self);
COREFOUNDATION_EXPORT Boolean CFMessagePortIsRemote(CFMessagePortRef self);
COREFOUNDATION_EXPORT Boolean CFMessagePortIsValid(CFMessagePortRef self);
COREFOUNDATION_EXPORT CFInteger CFMessagePortSendRequest(CFMessagePortRef self, CFInteger msgId, CFDataRef data, CFTimeInterval sendTimeout, CFTimeInterval receiveTimeout, CFStringRef replyMode, CFDataRef *replyData);
COREFOUNDATION_EXPORT void CFMessagePortSetInvalidationCallBack(CFMessagePortRef self, CFMessagePortInvalidationCallBack callback);
COREFOUNDATION_EXPORT Boolean CFMessagePortSetName(CFMessagePortRef self, CFStringRef name);
