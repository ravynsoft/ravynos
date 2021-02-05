/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSValue.h>
#import <Foundation/NSDecimal.h>

@class NSDictionary;

@interface NSNumber : NSValue

- initWithBool:(BOOL)value;
- initWithChar:(char)value;
- initWithShort:(short)value;
- initWithInt:(int)value;
- initWithLong:(long)value;
- initWithLongLong:(long long)value;
- initWithFloat:(float)value;
- initWithDouble:(double)value;
- initWithInteger:(NSInteger)value;

- initWithUnsignedChar:(unsigned char)value;
- initWithUnsignedShort:(unsigned short)value;
- initWithUnsignedInt:(unsigned int)value;
- initWithUnsignedLong:(unsigned long)value;
- initWithUnsignedLongLong:(unsigned long long)value;
- initWithUnsignedInteger:(NSUInteger)value;

+ (NSNumber *)numberWithBool:(BOOL)value;
+ (NSNumber *)numberWithChar:(char)value;
+ (NSNumber *)numberWithShort:(short)value;
+ (NSNumber *)numberWithInt:(int)value;
+ (NSNumber *)numberWithLong:(long)value;
+ (NSNumber *)numberWithLongLong:(long long)value;
+ (NSNumber *)numberWithFloat:(float)value;
+ (NSNumber *)numberWithDouble:(double)value;
+ (NSNumber *)numberWithInteger:(NSInteger)value;

+ (NSNumber *)numberWithUnsignedChar:(unsigned char)value;
+ (NSNumber *)numberWithUnsignedShort:(unsigned short)value;
+ (NSNumber *)numberWithUnsignedInt:(unsigned int)value;
+ (NSNumber *)numberWithUnsignedLong:(unsigned long)value;
+ (NSNumber *)numberWithUnsignedLongLong:(unsigned long long)value;
+ (NSNumber *)numberWithUnsignedInteger:(NSUInteger)value;

- (NSComparisonResult)compare:(NSNumber *)other;

- (BOOL)isEqualToNumber:(NSNumber *)other;

- (char)charValue;
- (short)shortValue;
- (int)intValue;
- (long)longValue;
- (long long)longLongValue;
- (float)floatValue;
- (double)doubleValue;
- (BOOL)boolValue;
- (NSDecimal)decimalValue;
- (NSInteger)integerValue;
- (unsigned char)unsignedCharValue;
- (unsigned short)unsignedShortValue;
- (unsigned int)unsignedIntValue;
- (unsigned long)unsignedLongValue;
- (unsigned long long)unsignedLongLongValue;
- (NSUInteger)unsignedIntegerValue;

- (NSString *)stringValue;

- (NSString *)descriptionWithLocale:(NSDictionary *)locale;

@end
