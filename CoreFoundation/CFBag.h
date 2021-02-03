/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct CFMutableBag *CFBagRef;
typedef struct CFMutableBag *CFMutableBagRef;

typedef const void *(*CFBagRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void (*CFBagReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFAllocatorCopyDescriptionCallBack CFBagCopyDescriptionCallBack;
typedef Boolean (*CFBagEqualCallBack)(const void *value, const void *other);
typedef CFHashCode (*CFBagHashCallBack)(const void *value);

typedef struct {
    CFIndex version;
    CFBagRetainCallBack retain;
    CFBagReleaseCallBack release;
    CFBagCopyDescriptionCallBack copyDescription;
    CFBagEqualCallBack equal;
    CFBagHashCallBack hash;
} CFBagCallBacks;

typedef void (*CFBagApplierFunction)(const void *value, void *context);

COREFOUNDATION_EXPORT const CFBagCallBacks kCFTypeBagCallBacks;
COREFOUNDATION_EXPORT const CFBagCallBacks kCFCopyStringBagCallBacks;

COREFOUNDATION_EXPORT CFTypeID CFBagGetTypeID(void);

COREFOUNDATION_EXPORT void CFBagApplyFunction(CFBagRef self, CFBagApplierFunction function, void *context);
COREFOUNDATION_EXPORT Boolean CFBagContainsValue(CFBagRef self, const void *value);
COREFOUNDATION_EXPORT CFBagRef CFBagCreate(CFAllocatorRef allocator, const void **values, CFIndex count, const CFBagCallBacks *callbacks);
COREFOUNDATION_EXPORT CFBagRef CFBagCreateCopy(CFAllocatorRef allocator, CFBagRef self);
COREFOUNDATION_EXPORT CFIndex CFBagGetCount(CFBagRef self);
COREFOUNDATION_EXPORT CFIndex CFBagGetCountOfValue(CFBagRef self, const void *value);
COREFOUNDATION_EXPORT const void *CFBagGetValue(CFBagRef self, const void *value);
COREFOUNDATION_EXPORT Boolean CFBagGetValueIfPresent(CFBagRef self, const void *candidate, const void **value);
COREFOUNDATION_EXPORT void CFBagGetValues(CFBagRef self, const void **values);
// mutable

COREFOUNDATION_EXPORT void CFBagAddValue(CFMutableBagRef self, const void *value);
COREFOUNDATION_EXPORT CFMutableBagRef CFBagCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFBagCallBacks *callbacks);
COREFOUNDATION_EXPORT CFMutableBagRef CFBagCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFBagRef self);
COREFOUNDATION_EXPORT void CFBagRemoveAllValues(CFMutableBagRef self);
COREFOUNDATION_EXPORT void CFBagRemoveValue(CFMutableBagRef self, const void *value);
COREFOUNDATION_EXPORT void CFBagReplaceValue(CFMutableBagRef self, const void *value);
COREFOUNDATION_EXPORT void CFBagSetValue(CFMutableBagRef self, const void *value);
