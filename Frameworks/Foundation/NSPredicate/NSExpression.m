/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSExpression.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSDictionary.h>
#import "NSExpression_constant.h"
#import "NSExpression_self.h"
#import "NSExpression_function.h"
#import "NSExpression_variable.h"
#import "NSExpression_keypath.h"

@implementation NSExpression

-initWithExpressionType:(NSExpressionType)type {
   _type=type;
   return self;
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

+(NSExpression *)expressionForConstantValue:value {
   return [[[NSExpression_constant allocWithZone:NULL] initWithValue:value] autorelease];
}

+(NSExpression *)expressionForEvaluatedObject {
   return [[[NSExpression_self allocWithZone:NULL] init] autorelease];
}

+(NSExpression *)expressionForVariable:(NSString *)string {
   return [[[NSExpression_variable allocWithZone:NULL] initWithVariable:string] autorelease];
}

+(NSExpression *)expressionForKeyPath:(NSString *)keyPath {
   return [[[NSExpression_keypath allocWithZone:NULL] initWithKeyPath:keyPath] autorelease];
}

+(NSExpression *)expressionForFunction:(NSString *)name arguments:(NSArray *)arguments {
   return [[[NSExpression_function allocWithZone:NULL] initWithName:name arguments:arguments] autorelease];
}

+(NSExpression *)expressionForKeyPathLeft:(NSExpression *)left right:(NSExpression *)right {
   return nil;
}

-(NSExpressionType)expressionType {
   return _type;
}

-constantValue {
   [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not of NSConstantValueExpressionType",isa,sel_getName(_cmd)];
   return nil;
}

-(NSString *)variable {
   [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not of NSVariableExpressionType",isa,sel_getName(_cmd)];
   return nil;
}

-(NSString *)keyPath {
   [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not of NSKeyPathExpressionType",isa,sel_getName(_cmd)];
   return nil;
}

-(NSString *)function {
   [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not of NSFunctionExpressionType",isa,sel_getName(_cmd)];
   return nil;
}

-(NSArray *)arguments {
   [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not of NSFunctionExpressionType",isa,sel_getName(_cmd)];
   return nil;
}

-(NSExpression *)operand {
   NSUnimplementedMethod();
   return nil;
}

-expressionValueWithObject:object context:(NSMutableDictionary *)context {
    NSUnimplementedMethod();
    return nil;
}

-(NSExpression *)_expressionWithSubstitutionVariables:(NSDictionary *)variables {
   return self;
}

@end
