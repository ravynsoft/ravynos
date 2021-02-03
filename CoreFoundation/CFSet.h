/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct __NSMutableSet *CFSetRef;
typedef struct __NSMutableSet *CFMutableSetRef;

typedef const void *(*CFSetRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void (*CFSetReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFAllocatorCopyDescriptionCallBack CFSetCopyDescriptionCallBack;
typedef Boolean (*CFSetEqualCallBack)(const void *value, const void *other);
typedef CFHashCode (*CFSetHashCallBack)(const void *value);

typedef struct {
    CFIndex version;
    CFSetRetainCallBack retain;
    CFSetReleaseCallBack release;
    CFSetCopyDescriptionCallBack copyDescription;
    CFSetEqualCallBack equal;
    CFSetHashCallBack hash;
} CFSetCallBacks;

typedef void (*CFSetApplierFunction)(const void *value, void *context);

COREFOUNDATION_EXPORT const CFSetCallBacks kCFTypeSetCallBacks;
COREFOUNDATION_EXPORT const CFSetCallBacks kCFCopyStringSetCallBacks;

COREFOUNDATION_EXPORT CFTypeID CFSetGetTypeID(void);

COREFOUNDATION_EXPORT CFSetRef CFSetCreate(CFAllocatorRef allocator, const void **values, CFIndex count, const CFSetCallBacks *callbacks);

COREFOUNDATION_EXPORT CFSetRef CFSetCreateCopy(CFAllocatorRef allocator, CFSetRef self);

COREFOUNDATION_EXPORT CFIndex CFSetGetCount(CFSetRef self);
COREFOUNDATION_EXPORT const void *CFSetGetValue(CFSetRef self, const void *value);
COREFOUNDATION_EXPORT void CFSetGetValues(CFSetRef self, const void **values);

COREFOUNDATION_EXPORT void CFSetApplyFunction(CFSetRef self, CFSetApplierFunction function, void *context);
COREFOUNDATION_EXPORT Boolean CFSetContainsValue(CFSetRef self, const void *value);
COREFOUNDATION_EXPORT CFIndex CFSetGetCountOfValue(CFSetRef self, const void *value);
COREFOUNDATION_EXPORT Boolean CFSetGetValueIfPresent(CFSetRef self, const void *candidate, const void **value);

// mutable

COREFOUNDATION_EXPORT CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFSetCallBacks *callbacks);

COREFOUNDATION_EXPORT CFMutableSetRef CFSetCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFSetRef self);

COREFOUNDATION_EXPORT void CFSetAddValue(CFMutableSetRef self, const void *value);
COREFOUNDATION_EXPORT void CFSetRemoveAllValues(CFMutableSetRef self);
COREFOUNDATION_EXPORT void CFSetRemoveValue(CFMutableSetRef self, const void *value);
COREFOUNDATION_EXPORT void CFSetReplaceValue(CFMutableSetRef self, const void *value);
COREFOUNDATION_EXPORT void CFSetSetValue(CFMutableSetRef self, const void *value);
