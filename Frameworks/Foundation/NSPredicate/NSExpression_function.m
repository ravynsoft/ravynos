/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSExpression_function.h"
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>

@implementation NSExpression_function

-initWithName:(NSString *)name arguments:(NSArray *)arguments {
   [super initWithExpressionType:NSFunctionExpressionType];
   _name=[name copy];
   _arguments=[arguments retain];
   return self;
}

-(void)dealloc {
   [_name release];
   [_arguments release];
   [super dealloc];
}

-(NSString *)function {
   return _name;
}

-(NSArray *)arguments {
   return _arguments;
}

-expressionValueWithObject:object context:(NSMutableDictionary *)context {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   NSInteger        i,count=[_arguments count];
      
   [result appendFormat:@"%@(",_name];
   for(i=0;i<count;i++)
    [result appendFormat:@"%@%s",[_arguments objectAtIndex:i],(i+1<count)?",":""];
   [result appendFormat:@")"];
   
   return result;
}

-(NSExpression *)_expressionWithSubstitutionVariables:(NSDictionary *)variables {
   NSMutableArray *array=[NSMutableArray array];
   NSInteger       i,count=[_arguments count];
      
   for(i=0;i<count;i++)
    [array addObject:[[_arguments objectAtIndex:i] _expressionWithSubstitutionVariables:variables]];

   return [NSExpression expressionForFunction:_name arguments:array];
}
@end
