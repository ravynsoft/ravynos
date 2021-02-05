/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSNumber.h>
#import <Foundation/NSDecimal.h>

FOUNDATION_EXPORT NSString *const NSDecimalNumberDivideByZeroException;
FOUNDATION_EXPORT NSString *const NSDecimalNumberUnderflowException;
FOUNDATION_EXPORT NSString *const NSDecimalNumberOverflowException;
FOUNDATION_EXPORT NSString *const NSDecimalNumberExactnessException;

@protocol NSDecimalNumberBehaviors
@end

@interface NSDecimalNumber : NSNumber {
}

- initWithDecimal:(NSDecimal)decimal;
- initWithMantissa:(uint64_t)mantissa exponent:(int16_t)exponent isNegative:(BOOL)flag;
- initWithString:(NSString *)string;
- initWithString:(NSString *)string locale:(NSDictionary *)locale;

+ (NSDecimalNumber *)decimalNumberWithDecimal:(NSDecimal)decimal;
+ (NSDecimalNumber *)decimalNumberWithMantissa:(uint64_t)mantissa exponent:(int16_t)exponent isNegative:(BOOL)negative;
+ (NSDecimalNumber *)decimalNumberWithString:(NSString *)string;
+ (NSDecimalNumber *)decimalNumberWithString:(NSString *)string locale:(NSDictionary *)locale;

+ (NSDecimalNumber *)zero;
+ (NSDecimalNumber *)one;
+ (NSDecimalNumber *)minimumDecimalNumber;
+ (NSDecimalNumber *)maximumDecimalNumber;
+ (NSDecimalNumber *)notANumber;

+ (id<NSDecimalNumberBehaviors>)defaultBehavior;
+ (void)setDefaultBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSComparisonResult)compare:(NSNumber *)other;

- (double)doubleValue;
- (const char *)objCType;

- (NSDecimalNumber *)decimalNumberByRoundingAccordingToBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)other;
- (NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)other withBehavior:(id<NSDecimalNumberBehaviors>)behavior;
- (NSDecimalNumber *)decimalNumberBySubtracting:(NSDecimalNumber *)other;
- (NSDecimalNumber *)decimalNumberBySubtracting:(NSDecimalNumber *)other withBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)other;
- (NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)other withBehavior:(id<NSDecimalNumberBehaviors>)behavior;
- (NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)other;
- (NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)other withBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(int16_t)power;
- (NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(int16_t)power withBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power;
- (NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power withBehavior:(id<NSDecimalNumberBehaviors>)behavior;

- (NSString *)descriptionWithLocale:(NSDictionary *)locale;

@end
