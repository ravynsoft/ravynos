/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDecimalNumber.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>

NSString * const NSDecimalNumberDivideByZeroException=@"NSDecimalNumberDivideByZeroException";
NSString * const NSDecimalNumberUnderflowException=@"NSDecimalNumberUnderflowException";
NSString * const NSDecimalNumberOverflowException=@"NSDecimalNumberOverflowException";
NSString * const NSDecimalNumberExactnessException=@"NSDecimalNumberExactnessException";

@implementation NSDecimalNumber

-initWithDecimal:(NSDecimal)decimal {
   NSUnimplementedMethod();
   return nil;
}

-initWithMantissa:(uint64_t)mantissa exponent:(int16_t)exponent isNegative:(BOOL)flag {
   NSUnimplementedMethod();
   return nil;
}

-initWithString:(NSString *)string {
   NSUnimplementedMethod();
   return nil;
}

-initWithString:(NSString *)string locale:(NSDictionary *)locale {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)decimalNumberWithDecimal:(NSDecimal)decimal {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)decimalNumberWithMantissa:(uint64_t)mantissa exponent:(int16_t)exponent isNegative:(BOOL)negative {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)decimalNumberWithString:(NSString *)string {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)decimalNumberWithString:(NSString *)string locale:(NSDictionary *)locale {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)zero {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)one {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)minimumDecimalNumber {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)maximumDecimalNumber {
   NSUnimplementedMethod();
   return nil;
}

+(NSDecimalNumber *)notANumber {
   NSUnimplementedMethod();
   return nil;
}

+(id <NSDecimalNumberBehaviors>)defaultBehavior {
   NSUnimplementedMethod();
   return nil;
}

+(void)setDefaultBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
}

-(NSComparisonResult)compare:(NSNumber *)other {
   NSUnimplementedMethod();
   return 0;
}

-(double)doubleValue {
   NSUnimplementedMethod();
   return 0;
}

-(const char *)objCType {
   return @encode(double);
}

-(NSDecimalNumber *)decimalNumberByRoundingAccordingToBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)other {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByAdding:(NSDecimalNumber *)other withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberBySubtracting:(NSDecimalNumber *)other {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberBySubtracting:(NSDecimalNumber *)other withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)other {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByMultiplyingBy:(NSDecimalNumber *)other withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)other {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByDividingBy:(NSDecimalNumber *)other withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(int16_t)power {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByMultiplyingByPowerOf10:(int16_t)power withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power {
   NSUnimplementedMethod();
   return nil;
}

-(NSDecimalNumber *)decimalNumberByRaisingToPower:(NSUInteger)power withBehavior:(id <NSDecimalNumberBehaviors>)behavior {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
   NSUnimplementedMethod();
   return nil;
}

@end
