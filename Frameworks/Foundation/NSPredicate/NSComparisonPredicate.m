/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSComparisonPredicate.h>
#import <Foundation/NSExpression.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSString.h>
#import "NSExpression_operator.h"

@implementation NSComparisonPredicate

-initWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right modifier:(NSComparisonPredicateModifier)modifier type:(NSPredicateOperatorType)type options:(NSUInteger)options {
   _left=[left retain];
   _right=[right retain];
   _modifier=modifier;
   _type=type;
   _options=options;
   _customSelector=NULL;
   return self;
}

-initWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right customSelector:(SEL)selector {
   _left=[left retain];
   _right=[right retain];
   _modifier=NSDirectPredicateModifier;
   _type=NSCustomSelectorPredicateOperatorType;
   _options=0;
   _customSelector=selector;
   return self;
}

-(void)dealloc {
   [_left release];
   [_right release];
   [super dealloc];
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

+(NSPredicate *)predicateWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right modifier:(NSComparisonPredicateModifier)modifier type:(NSPredicateOperatorType)type options:(NSUInteger)options {
   return [[[self alloc] initWithLeftExpression:left rightExpression:right modifier:modifier type:type options:options] autorelease];
}

+(NSPredicate *)predicateWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right customSelector:(SEL)selector {
   return [[[self alloc] initWithLeftExpression:left rightExpression:right customSelector:selector] autorelease];
}

-(NSString *)predicateFormat {
   NSMutableString *result=[NSMutableString string];
   NSString        *operator=nil;
   NSString        *options;
   
   switch(_modifier){
    case NSDirectPredicateModifier:
     break;

    case NSAllPredicateModifier:
     [result appendFormat:@"ALL "];
     break;

    case NSAnyPredicateModifier:
     [result appendFormat:@"ANY "];
     break;
   }
   
   if(_options&NSCaseInsensitivePredicateOption)
    if(_options&NSDiacriticInsensitivePredicateOption)
     options=@"[cd]";
    else
     options=@"[c]";
   else if(_options&NSDiacriticInsensitivePredicateOption)
    options=@"[d]";
   else
    options=@"";
       
   switch(_type){
   
    case NSLessThanPredicateOperatorType:
     operator=@"<";
     break;
     
    case NSLessThanOrEqualToPredicateOperatorType:
     operator=@"<=";
     break;
     
    case NSGreaterThanPredicateOperatorType:
     operator=@">";
     break;
     
    case NSGreaterThanOrEqualToPredicateOperatorType:
     operator=@">=";
     break;
     
    case NSEqualToPredicateOperatorType:
     operator=@"==";
     break;
     
    case NSNotEqualToPredicateOperatorType:
     operator=@"!=";
     break;
     
    case NSMatchesPredicateOperatorType:
     operator=@"MATCHES";
     break;
     
    case NSLikePredicateOperatorType:
     operator=@"LIKE";
     break;
     
    case NSBeginsWithPredicateOperatorType:
     operator=@"BEGINSWITH";
     break;
     
    case NSEndsWithPredicateOperatorType:
     operator=@"ENDSWITH";
     break;
     
    case NSInPredicateOperatorType:
     operator=@"IN";
     break;
    
    // FIX, not right
    case NSCustomSelectorPredicateOperatorType:
     operator=[NSString stringWithFormat:@"@selector(%s)",sel_getName(_customSelector)];
     break;
     
   }
   
   [result appendFormat:@"%@ %@%@ %@",_left,operator,options,_right];
     
   return result;
}

-(NSPredicate *)predicateWithSubstitutionVariables:(NSDictionary *)variables {
   NSExpression *left=[_left _expressionWithSubstitutionVariables:variables];
   NSExpression *right=[_right _expressionWithSubstitutionVariables:variables];
   
   if(_type!=NSCustomSelectorPredicateOperatorType)
    return [NSComparisonPredicate predicateWithLeftExpression:left rightExpression:right modifier:_modifier type:_type options:_options];
   else
    return [NSComparisonPredicate predicateWithLeftExpression:left rightExpression:right customSelector:_customSelector];
}

-(NSExpression *)leftExpression {
   return _left;
}

-(NSExpression *)rightExpression {
   return _right;
}

-(NSPredicateOperatorType)predicateOperatorType {
   return _type;
}

-(NSComparisonPredicateModifier)comparisonPredicateModifier {
   return _modifier;
}

-(NSUInteger)options {
   return _options;
}

-(SEL)customSelector {
   return _customSelector;
}

-(BOOL)_evaluateValue:leftResult withObject:object {
   id rightResult=[_right expressionValueWithObject:object context:nil];
   NSUInteger compareOptions=0;
   
   BOOL selfIsNil = (leftResult == nil || [leftResult isEqual:[NSNull null]]);
   BOOL objectIsNil = (rightResult == nil || [rightResult isEqual:[NSNull null]]);
	
   if (selfIsNil || objectIsNil)
    return (selfIsNil == objectIsNil && _type == NSEqualToPredicateOperatorType);

   if(!(_options & NSDiacriticInsensitivePredicateOption))
     compareOptions |= NSLiteralSearch;
   if(_options & NSCaseInsensitivePredicateOption)
     compareOptions |= NSCaseInsensitiveSearch;

   switch(_type){
   
    case NSLessThanPredicateOperatorType:
     return ([leftResult compare:rightResult]==NSOrderedAscending)?YES:NO;
     
    case NSLessThanOrEqualToPredicateOperatorType:{
      NSComparisonResult check=[leftResult compare:rightResult];
      
      return (check==NSOrderedAscending || check==NSOrderedSame)?YES:NO;
     }
     
    case NSGreaterThanPredicateOperatorType:
     return ([leftResult compare:rightResult]==NSOrderedDescending)?YES:NO;
     
    case NSGreaterThanOrEqualToPredicateOperatorType:{
      NSComparisonResult check=[leftResult compare:rightResult];
      
      return (check==NSOrderedDescending || check==NSOrderedSame)?YES:NO;
     }
     
    case NSEqualToPredicateOperatorType:
     return [leftResult isEqual:rightResult];
     
    case NSNotEqualToPredicateOperatorType:
     return ![leftResult isEqual:rightResult];
     
    case NSMatchesPredicateOperatorType:
     NSUnimplementedMethod();
     return YES;
     
    case NSLikePredicateOperatorType:
     NSUnimplementedMethod();
     return YES;
     
    case NSBeginsWithPredicateOperatorType:{
      NSRange range = NSMakeRange(0,[rightResult length]);
      return ([leftResult compare:rightResult options:compareOptions range:range]==NSOrderedSame)?YES:NO;
     }
     
    case NSEndsWithPredicateOperatorType:{
      NSRange range = NSMakeRange([leftResult length] - [rightResult length],[rightResult length]);
      
      return ([leftResult compare:rightResult options:compareOptions range:range]==NSOrderedSame)?YES:NO;
     }
     
    case NSInPredicateOperatorType:
     return ([leftResult rangeOfString:rightResult options:compareOptions].location!=NSNotFound)?YES:NO;
     
    case NSCustomSelectorPredicateOperatorType:{
      BOOL (*function)(id,SEL,id)=(BOOL (*)(id,SEL,id))[leftResult methodForSelector:_customSelector];
      
      return function(leftResult,_customSelector,rightResult);
     }
    default:
     return NO;
   }
}

-(BOOL)evaluateWithObject:(id)object{
   NSMutableArray *values = [NSMutableArray array];
   NSComparisonPredicateModifier modifier = [self comparisonPredicateModifier];
   id leftValue = [[self leftExpression] expressionValueWithObject:object context:NULL];
	
   if(modifier==NSDirectPredicateModifier){
/* It is possible for an expression to return nil (constant or keypath for example), comparisons consider
   NSNull and nil equal (right?), so we just use NSNull here since you can use nil in an array */
   
    if(leftValue==nil)
     leftValue=[NSNull null];
     
    [values addObject:leftValue];
   }
   else{		
    if ([[self leftExpression] expressionType] != NSKeyPathExpressionType || !([leftValue isKindOfClass:[NSArray class]] || [leftValue isKindOfClass:[NSSet class]]))
     [NSException raise:NSInvalidArgumentException format:@"The left hand side for an ALL or ANY operator must be either an NSArray or an NSSet"];
    [values addObjectsFromArray:leftValue];
   }
	
   BOOL result = (modifier == NSAllPredicateModifier);
   NSEnumerator *e = [values objectEnumerator];
   id value;
   while ((value = [e nextObject])!=nil) {
    BOOL eval = [self _evaluateValue:value withObject:(id)object];
    
    if (eval == (modifier != NSAllPredicateModifier))
     return eval;		
   }

   return result;
}


@end
