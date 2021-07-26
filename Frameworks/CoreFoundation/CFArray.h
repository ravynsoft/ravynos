/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __CFArray *CFArrayRef;
typedef struct __CFArray *CFMutableArrayRef;

#import <CoreFoundation/CFBase.h>

typedef const void *(*CFArrayRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void (*CFArrayReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFStringRef (*CFArrayCopyDescriptionCallBack)(const void *value);
typedef Boolean (*CFArrayEqualCallBack)(const void *value, const void *other);

typedef struct {
    CFIndex version;
    CFArrayRetainCallBack retain;
    CFArrayReleaseCallBack release;
    CFArrayCopyDescriptionCallBack copyDescription;
    CFArrayEqualCallBack equal;
} CFArrayCallBacks;

typedef void (*CFArrayApplierFunction)(const void *value, void *context);

COREFOUNDATION_EXPORT const CFArrayCallBacks kCFTypeArrayCallBacks;

COREFOUNDATION_EXPORT CFTypeID CFArrayGetTypeID(void);

COREFOUNDATION_EXPORT CFArrayRef CFArrayCreate(CFAllocatorRef allocator, const void **values, CFIndex count, const CFArrayCallBacks *callbacks);

COREFOUNDATION_EXPORT CFArrayRef CFArrayCreateCopy(CFAllocatorRef allocator, CFArrayRef self);

COREFOUNDATION_EXPORT CFIndex CFArrayGetCount(CFArrayRef self);
COREFOUNDATION_EXPORT const void *CFArrayGetValueAtIndex(CFArrayRef self, CFIndex index);
COREFOUNDATION_EXPORT void CFArrayGetValues(CFArrayRef self, CFRange range, const void **values);
COREFOUNDATION_EXPORT Boolean CFArrayContainsValue(CFArrayRef self, CFRange range, const void *value);
COREFOUNDATION_EXPORT CFIndex CFArrayGetFirstIndexOfValue(CFArrayRef self, CFRange range, const void *value);
COREFOUNDATION_EXPORT CFIndex CFArrayGetLastIndexOfValue(CFArrayRef self, CFRange range, const void *value);
COREFOUNDATION_EXPORT CFIndex CFArrayGetCountOfValue(CFArrayRef self, CFRange range, const void *value);
COREFOUNDATION_EXPORT void CFArrayApplyFunction(CFArrayRef self, CFRange range, CFArrayApplierFunction function, void *context);
COREFOUNDATION_EXPORT CFIndex CFArrayBSearchValues(CFArrayRef self, CFRange range, const void *value, CFComparatorFunction function, void *context);

// mutable

COREFOUNDATION_EXPORT CFMutableArrayRef CFArrayCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callbacks);

COREFOUNDATION_EXPORT CFMutableArrayRef CFArrayCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFArrayRef self);

COREFOUNDATION_EXPORT void CFArrayAppendValue(CFMutableArrayRef self, const void *value);
COREFOUNDATION_EXPORT void CFArrayAppendArray(CFMutableArrayRef self, CFArrayRef other, CFRange range);
COREFOUNDATION_EXPORT void CFArrayRemoveValueAtIndex(CFMutableArrayRef self, CFIndex index);
COREFOUNDATION_EXPORT void CFArrayRemoveAllValues(CFMutableArrayRef self);
COREFOUNDATION_EXPORT void CFArrayInsertValueAtIndex(CFMutableArrayRef self, CFIndex index, const void *value);
COREFOUNDATION_EXPORT void CFArraySetValueAtIndex(CFMutableArrayRef self, CFIndex index, const void *value);
COREFOUNDATION_EXPORT void CFArrayReplaceValues(CFMutableArrayRef self, CFRange range, const void **values, CFIndex count);
COREFOUNDATION_EXPORT void CFArrayExchangeValuesAtIndices(CFMutableArrayRef self, CFIndex index, CFIndex other);
COREFOUNDATION_EXPORT void CFArraySortValues(CFMutableArrayRef self, CFRange range, CFComparatorFunction function, void *context);
