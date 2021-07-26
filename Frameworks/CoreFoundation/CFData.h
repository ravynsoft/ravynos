/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef const struct __CFMutableData *CFDataRef;
typedef struct __CFMutableData *CFMutableDataRef;

#import <CoreFoundation/CFBase.h>

COREFOUNDATION_EXPORT CFTypeID CFDataGetTypeID(void);

COREFOUNDATION_EXPORT CFDataRef CFDataCreate(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length);
COREFOUNDATION_EXPORT CFDataRef CFDataCreateWithBytesNoCopy(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFAllocatorRef bytesDeallocator);

COREFOUNDATION_EXPORT CFDataRef CFDataCreateCopy(CFAllocatorRef allocator, CFDataRef self);

COREFOUNDATION_EXPORT CFIndex CFDataGetLength(CFDataRef self);
COREFOUNDATION_EXPORT const uint8_t *CFDataGetBytePtr(CFDataRef self);
COREFOUNDATION_EXPORT void CFDataGetBytes(CFDataRef self, CFRange range, uint8_t *bytes);

// mutable

COREFOUNDATION_EXPORT CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity);

COREFOUNDATION_EXPORT CFMutableDataRef CFDataCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDataRef other);

COREFOUNDATION_EXPORT uint8_t *CFDataGetMutableBytePtr(CFMutableDataRef self);

COREFOUNDATION_EXPORT void CFDataSetLength(CFMutableDataRef self, CFIndex length);
COREFOUNDATION_EXPORT void CFDataAppendBytes(CFMutableDataRef self, const uint8_t *bytes, CFIndex length);
COREFOUNDATION_EXPORT void CFDataDeleteBytes(CFMutableDataRef self, CFRange range);
COREFOUNDATION_EXPORT void CFDataIncreaseLength(CFMutableDataRef self, CFIndex delta);
COREFOUNDATION_EXPORT void CFDataReplaceBytes(CFMutableDataRef self, CFRange range, const uint8_t *bytes, CFIndex length);
