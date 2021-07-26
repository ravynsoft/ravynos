/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSExpression_assignment.h"
#import "NSExpression_operator.h"
#import <Foundation/NSString.h>

@implementation NSExpression_assignment

-initWithVariable:(NSExpression *)variable expression:(NSExpression *)expression {
   _variable=[variable retain];
   _expression=[expression retain];
   return self;
}

-(void)dealloc {
   [_variable release];
   [_expression release];
   [super dealloc];
}

+(NSExpression *)expressionWithVariable:(NSExpression *)variable expression:(NSExpression *)expression {
   return [[[self allocWithZone:NULL] initWithVariable:variable expression:expression] autorelease];
}

-(NSString *)description {
   NSString *pretty=[_expression description];
   
   if([_expression isKindOfClass:[NSExpression_operator class]])
    pretty=[NSString stringWithFormat:@"(%@)",pretty];
    
   return [NSString stringWithFormat:@"%@ := %@",_variable,pretty];
}

-(NSExpression *)_expressionWithSubstitutionVariables:(NSDictionary *)variables {
// FIX?
   return self;
}

@end
