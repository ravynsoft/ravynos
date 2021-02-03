/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFRunLoop.h>

typedef struct CFFileDescriptor *CFFileDescriptorRef;

typedef int CFFileDescriptorNativeDescriptor;

enum {
    kCFFileDescriptorReadCallBack = (1 << 0),
    kCFFileDescriptorWriteCallBack = (1 << 1),
};

typedef void (*CFFileDescriptorCallBack)(CFFileDescriptorRef self, CFOptionFlags flags, void *info);

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFFileDescriptorContext;

COREFOUNDATION_EXPORT CFTypeID CFFileDescriptorGetTypeID(void);

COREFOUNDATION_EXPORT CFFileDescriptorRef CFFileDescriptorCreate(CFAllocatorRef allocator, CFFileDescriptorNativeDescriptor fd, Boolean closeOnInvalidate, CFFileDescriptorCallBack callback, const CFFileDescriptorContext *context);

COREFOUNDATION_EXPORT CFFileDescriptorNativeDescriptor CFFileDescriptorGetNativeDescriptor(CFFileDescriptorRef self);
COREFOUNDATION_EXPORT void CFFileDescriptorGetContext(CFFileDescriptorRef self, CFFileDescriptorContext *context);

COREFOUNDATION_EXPORT CFRunLoopSourceRef CFFileDescriptorCreateRunLoopSource(CFAllocatorRef allocator, CFFileDescriptorRef self, CFIndex order);
COREFOUNDATION_EXPORT void CFFileDescriptorDisableCallBacks(CFFileDescriptorRef self, CFOptionFlags flags);
COREFOUNDATION_EXPORT void CFFileDescriptorEnableCallBacks(CFFileDescriptorRef self, CFOptionFlags flags);
COREFOUNDATION_EXPORT void CFFileDescriptorInvalidate(CFFileDescriptorRef self);
COREFOUNDATION_EXPORT Boolean CFFileDescriptorIsValid(CFFileDescriptorRef self);
