/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSData.h>

@interface NSData_concrete : NSData {
    NSUInteger _length;
    char *_bytes;
    BOOL _freeWhenDone;
}
@end

static inline void NSByteCopy(const void *src, void *dst, NSUInteger count) {
    const char *srcBytes = src;
    char *dstBytes = dst;
    NSUInteger i;

    for(i = 0; i < count; i++)
        dstBytes[i] = srcBytes[i];
}

static inline void NSByteZero(void *bytes, NSUInteger count) {
    char *zeroBytes = bytes;
    NSUInteger i;

    for(i = 0; i < count; i++)
        zeroBytes[i] = 0;
}

static inline void NSByteZeroRange(void *bytes, NSRange range) {
    char *zeroBytes = bytes;
    NSUInteger i;

    for(i = 0; i < range.length; i++)
        zeroBytes[i + range.location] = 0;
}

static inline BOOL NSBytesEqual(const void *src1, const void *src2, NSUInteger count) {
    const char *bytes1 = src1, *bytes2 = src2;
    NSUInteger i;

    for(i = 0; i < count; i++)
        if(bytes1[i] != bytes2[i])
            return NO;

    return YES;
}

void *NSBytesReplicate(const void *src, NSUInteger count, NSZone *zone);

NSData *NSData_concreteNew(NSZone *zone, const char *bytes, NSUInteger length);
NSData *NSData_concreteNewNoCopy(NSZone *zone, void *bytes, NSUInteger length);
