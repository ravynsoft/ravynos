/*
 * Foundation/NSObject.h
 * Copyright (c) 2026 Zoe Knox. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _FOUNDATION_NSOBJECT_H_
#define _FOUNDATION_NSOBJECT_H_

#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>

typedef enum NSComparisonResult {
    NSOrderedAscending = -1,
    NSOrderedSame = 0,
    NSOrderedDescending = 1
} NSComparisonResult;

#define NSNotFound NSIntegerMax

#include <Foundation/NSObjCRuntime.h>
#include <objc/NSObject.h>

@interface NSObject(ravyn)
+(NSInteger)version;
+(void)setVersion:(NSInteger)version;
+(void)load;
+(void)initialize;
@end

@protocol NSCopying
- copyWithZone:(struct _NSZone *) __unused zone;
@end

@protocol NSMutableCopying
- mutableCopyWithZone:(struct _NSZone *) __unused zone;
@end

@class NSCoder;
@protocol NSCoding
- initWithCoder:(NSCoder *)coder;
- (void)encodeWithCoder:(NSCoder *)coder;
@end

void NSLog(NSString *format,...);
void NSLogv(NSString *format,va_list arguments);

#define NS_RETURNS_INNER_POINTER __attribute__((objc_returns_inner_pointer))
#endif
