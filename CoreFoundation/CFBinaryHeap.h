/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct CFBinaryHeap *CFBinaryHeapRef;

typedef const void *(*CFBinaryHeapRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void (*CFBinaryHeapReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFAllocatorCopyDescriptionCallBack CFBinaryHeapCopyDescriptionCallBack;
typedef CFComparisonResult (*CFBinaryHeapCompareCallBack)(const void *value, const void *other, void *info);

typedef struct {
    CFIndex version;
    CFBinaryHeapRetainCallBack retain;
    CFBinaryHeapReleaseCallBack release;
    CFBinaryHeapCopyDescriptionCallBack copyDescription;
    CFBinaryHeapCompareCallBack compare;
} CFBinaryHeapCallBacks;

typedef struct {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFBinaryHeapCompareContext;

typedef void (*CFBinaryHeapApplierFunction)(const void *value, void *context);

COREFOUNDATION_EXPORT const CFBinaryHeapCallBacks kCFStringBinaryHeapCallBacks;

COREFOUNDATION_EXPORT CFTypeID CFBinaryHeapGetTypeID(void);

COREFOUNDATION_EXPORT void CFBinaryHeapAddValue(CFBinaryHeapRef self, const void *value);
COREFOUNDATION_EXPORT void CFBinaryHeapApplyFunction(CFBinaryHeapRef self, CFBinaryHeapApplierFunction function, void *context);
COREFOUNDATION_EXPORT Boolean CFBinaryHeapContainsValue(CFBinaryHeapRef self, const void *value);
COREFOUNDATION_EXPORT CFBinaryHeapRef CFBinaryHeapCreate(CFAllocatorRef allocator, CFIndex capacity, const CFBinaryHeapCallBacks *callbacks, const CFBinaryHeapCompareContext *context);
COREFOUNDATION_EXPORT CFBinaryHeapRef CFBinaryHeapCreateCopy(CFAllocatorRef allocator, CFIndex capacity, CFBinaryHeapRef self);
COREFOUNDATION_EXPORT CFIndex CFBinaryHeapGetCount(CFBinaryHeapRef self);
COREFOUNDATION_EXPORT CFIndex CFBinaryHeapGetCountOfValue(CFBinaryHeapRef self, const void *value);
COREFOUNDATION_EXPORT const void *CFBinaryHeapGetMinimum(CFBinaryHeapRef self);
COREFOUNDATION_EXPORT Boolean CFBinaryHeapGetMinimumIfPresent(CFBinaryHeapRef self, const void **valuep);
COREFOUNDATION_EXPORT void CFBinaryHeapGetValues(CFBinaryHeapRef self, const void **values);
COREFOUNDATION_EXPORT void CFBinaryHeapRemoveAllValues(CFBinaryHeapRef self);
COREFOUNDATION_EXPORT void CFBinaryHeapRemoveMinimumValue(CFBinaryHeapRef self);
