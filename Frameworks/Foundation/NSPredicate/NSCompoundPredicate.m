/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCompoundPredicate.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

@implementation NSCompoundPredicate

-initWithType:(NSCompoundPredicateType)type subpredicates:(NSArray *)predicates {
   _type=type;
   _predicates=[predicates retain];
   return self;
}

-(void)dealloc {
   [_predicates release];
   [super dealloc];
}

+(NSPredicate *)notPredicateWithSubpredicate:(NSPredicate *)predicate {
   return [[[self alloc] initWithType:NSNotPredicateType subpredicates:[NSArray arrayWithObject:predicate]] autorelease];
}


+(NSPredicate *)andPredicateWithSubpredicates:(NSArray *)predicates {
   return [[[self alloc] initWithType:NSAndPredicateType subpredicates:predicates] autorelease];
}

+(NSPredicate *)orPredicateWithSubpredicates:(NSArray *)predicates {
   return [[[self alloc] initWithType:NSOrPredicateType subpredicates:predicates] autorelease];
}

-(NSString *)predicateFormat {
   NSMutableString *result=[NSMutableString string];
   NSMutableArray  *args=[NSMutableArray array];
   NSInteger              i,count=[_predicates count];
   
   for(i=0;i<count;i++){
    NSPredicate *check=[_predicates objectAtIndex:i];
    NSString    *precedence=[check predicateFormat];
    
    if([check isKindOfClass:[NSCompoundPredicate class]])
     if([(NSCompoundPredicate *)check compoundPredicateType]!=_type)
      precedence=[NSString stringWithFormat:@"(%@)",precedence];
     
    [args addObject:precedence];
   }
   
   switch(_type){
    case NSNotPredicateType:
     [result appendFormat:@"NOT %@",[args objectAtIndex:0]];
     break;
     
    case NSAndPredicateType:
     [result appendFormat:@"%@ AND %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;

    case NSOrPredicateType:
     [result appendFormat:@"%@ OR %@",[args objectAtIndex:0],[args objectAtIndex:1]];
     break;
    }
   
   return result;
}

-(NSCompoundPredicateType)compoundPredicateType {
   return _type;
}

-(NSArray *)subpredicates {
   return _predicates;
}

-(BOOL)evaluateWithObject:object {
   BOOL result=NO;
   NSInteger  i,count=[_predicates count];
   
   for(i=0;i<count;i++){
    NSPredicate *predicate=[_predicates objectAtIndex:i];
    
    switch(_type){
     case NSNotPredicateType:
      return ![predicate evaluateWithObject:object];

     case NSAndPredicateType:
      if(i==0)
       result=[predicate evaluateWithObject:object];
      else
       result=result && [predicate evaluateWithObject:object];
      break;
      
     case NSOrPredicateType:
      if([predicate evaluateWithObject:object])
       return YES;
      break;
    }
   }
   
   return result;
}

@end
