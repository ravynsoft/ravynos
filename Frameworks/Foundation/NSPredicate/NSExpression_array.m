/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSExpression_array.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

@implementation NSExpression_array

-initWithArray:(NSArray *)array {
   _array=[array retain];
   return self;
}

+(NSExpression *)expressionForArray:(NSArray *)array {
   return [[[self alloc] initWithArray:array] autorelease];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   NSInteger        i,count=[_array count];
   
   [result appendString:@"{"];
   for(i=0;i<count;i++)
    [result appendFormat:@"%@%@",[_array objectAtIndex:i],(i+1<count)?@",":@""];
   [result appendString:@"}"];
   return result;
}

-(NSExpression *)_expressionWithSubstitutionVariables:(NSDictionary *)variables {
   NSMutableArray *array=[NSMutableArray array];
   NSInteger       i,count=[_array count];
      
   for(i=0;i<count;i++)
    [array addObject:[[_array objectAtIndex:i] _expressionWithSubstitutionVariables:variables]];

   return [[[NSExpression_array allocWithZone:NULL] initWithArray:array] autorelease];
}

@end
