/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreFoundation/CFBase.h>

typedef const struct __CFBoolean *CFBooleanRef;

COREFOUNDATION_EXPORT const CFBooleanRef kCFBooleanTrue;
COREFOUNDATION_EXPORT const CFBooleanRef kCFBooleanFalse;

COREFOUNDATION_EXPORT CFTypeID CFBooleanGetTypeID(void);

COREFOUNDATION_EXPORT Boolean CFBooleanGetValue(CFBooleanRef boolean);

typedef const struct __NSNumber *CFNumberRef;

typedef enum {
    kCFNumberSInt8Type = 1,
    kCFNumberSInt16Type = 2,
    kCFNumberSInt32Type = 3,
    kCFNumberSInt64Type = 4,
    kCFNumberFloat32Type = 5,
    kCFNumberFloat64Type = 6,
    kCFNumberCharType = 7,
    kCFNumberShortType = 8,
    kCFNumberIntType = 9,
    kCFNumberLongType = 10,
    kCFNumberLongLongType = 11,
    kCFNumberFloatType = 12,
    kCFNumberDoubleType = 13,
    kCFNumberCFIndexType = 14,
    kCFNumberNSIntegerType = 15,
    kCFNumberCGFloatType = 16,
    kCFNumberMaxType = kCFNumberCGFloatType,
} CFNumberType;

COREFOUNDATION_EXPORT const CFNumberRef kCFNumberPositiveInfinity;
COREFOUNDATION_EXPORT const CFNumberRef kCFNumberNegativeInfinity;
COREFOUNDATION_EXPORT const CFNumberRef kCFNumberNaN;

COREFOUNDATION_EXPORT CFTypeID CFNumberGetTypeID(void);

COREFOUNDATION_EXPORT CFNumberRef CFNumberCreate(CFAllocatorRef allocator, CFNumberType type, const void *valuep);

COREFOUNDATION_EXPORT CFComparisonResult CFNumberCompare(CFNumberRef self, CFNumberRef other, void *context);
COREFOUNDATION_EXPORT CFIndex CFNumberGetByteSize(CFNumberRef self);
COREFOUNDATION_EXPORT CFNumberType CFNumberGetType(CFNumberRef self);
COREFOUNDATION_EXPORT Boolean CFNumberGetValue(CFNumberRef self, CFNumberType type, void *valuep);
COREFOUNDATION_EXPORT Boolean CFNumberIsFloatType(CFNumberRef self);
