/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSExpression_operator.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

@implementation NSExpression_operator

-initWithOperator:(NSExpressionOperator)operator arguments:(NSArray *)arguments {
   _operator=operator;
   _arguments=[arguments retain];
   return self;
}

+(NSExpression *)expressionForOperator:(NSExpressionOperator)operator arguments:(NSArray *)arguments {
   return [[[self alloc] initWithOperator:operator arguments:arguments] autorelease];
}

-(NSArray *)arguments {
   return _arguments;
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   NSMutableArray  *args=[NSMutableArray array];
   NSInteger        i,count=[_arguments count];
   
   for(i=0;i<count;i++){
    NSExpression *check=[_arguments objectAtIndex:i];
    NSString     *precedence=[check description];
    
    if([check isKindOfClass:[NSExpression_operator class]])
     precedence=[NSString stringWithFormat:@"(%@)",precedence];

    [args addObject:precedence];
   }
   
   switch(_operator){

    case NSExpressionOperatorNegate:
     [result appendFormat:@"-%@",[args objectAtIndex:0]];
     break;

    case NSExpressionOperatorAdd:
     [result appendFormat:@"%@ + %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorSubtract:
     [result appendFormat:@"%@ - %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorMultiply:
     [result appendFormat:@"%@ * %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorDivide:
     [result appendFormat:@"%@ / %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorExp:
     [result appendFormat:@"%@ ** %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorAssign:
     [result appendFormat:@"%@ := %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorKeypath:
     [result appendFormat:@"%@.%@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorIndex:
     [result appendFormat:@"%@[%@]",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSExpressionOperatorIndexFirst:
     [result appendFormat:@"%@[FIRST]",[args objectAtIndex:0]];
     break;

    case NSExpressionOperatorIndexLast:
     [result appendFormat:@"%@[LAST]",[args objectAtIndex:0]];
     break;

    case NSExpressionOperatorIndexSize:
     [result appendFormat:@"%@[SIZE]",[args objectAtIndex:0]];
     break;
   }
   
   return result;
}

-(NSExpression *)_expressionWithSubstitutionVariables:(NSDictionary *)variables {
   NSMutableArray *array=[NSMutableArray array];
   NSInteger       i,count=[_arguments count];
      
   for(i=0;i<count;i++)
    [array addObject:[[_arguments objectAtIndex:i] _expressionWithSubstitutionVariables:variables]];

   return [NSExpression_operator expressionForOperator:_operator arguments:array];
}

@end
