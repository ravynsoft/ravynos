/* Interface for NSPredicate for GNUStep
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Dr. H. Nikolaus Schaller
   Created: 2005
   Modifications: Fred Kiefer <FredKiefer@gmx.de>
   Date: May 2007
   Modifications: Richard Frith-Macdoanld <rfm@gnu.org>
   Date: June 2007
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */ 

#import "common.h"

#define	EXPOSE_NSComparisonPredicate_IVARS	1
#define	EXPOSE_NSCompoundPredicate_IVARS	1
#define	EXPOSE_NSExpression_IVARS	1

#import "Foundation/NSComparisonPredicate.h"
#import "Foundation/NSCompoundPredicate.h"
#import "Foundation/NSExpression.h"
#import "Foundation/NSPredicate.h"

#import "Foundation/NSArray.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSScanner.h"
#import "Foundation/NSValue.h"

#import "GSPrivate.h"

// For pow()
#include <math.h>

#if     defined(HAVE_UNICODE_UREGEX_H)
#include <unicode/uregex.h>
#endif

/* Object to represent the expression beign evaluated.
 */
static NSExpression	*evaluatedObjectExpression = nil;

extern void     GSPropertyListMake(id,NSDictionary*,BOOL,BOOL,unsigned,id*);

@interface GSPredicateScanner : NSScanner
{
  NSEnumerator	*_args;		// Not retained.
  unsigned	_retrieved;
}

- (id) initWithString: (NSString*)format
		 args: (NSArray*)args;
- (id) nextArg;
- (BOOL) scanPredicateKeyword: (NSString *) key;
- (NSPredicate *) parse;
- (NSPredicate *) parsePredicate;
- (NSPredicate *) parseAnd;
- (NSPredicate *) parseNot;
- (NSPredicate *) parseOr;
- (NSPredicate *) parseComparison;
- (NSExpression *) parseExpression;
- (NSExpression *) parseFunctionalExpression;
- (NSExpression *) parsePowerExpression;
- (NSExpression *) parseMultiplicationExpression;
- (NSExpression *) parseAdditionExpression;
- (NSExpression *) parseBinaryExpression;
- (NSExpression *) parseSimpleExpression;

@end

@interface GSTruePredicate : NSPredicate
@end

@interface GSFalsePredicate : NSPredicate
@end

@interface GSAndCompoundPredicate : NSCompoundPredicate
@end

@interface GSOrCompoundPredicate : NSCompoundPredicate
@end

@interface GSNotCompoundPredicate : NSCompoundPredicate
@end

@interface NSExpression (Private)
- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables;
@end

@interface GSConstantValueExpression : NSExpression
{
  @public
  id	_obj;
}
@end

@interface GSEvaluatedObjectExpression : NSExpression
@end

@interface GSVariableExpression : NSExpression
{
  @public
  NSString	*_variable;
}
@end

@interface GSKeyPathExpression : NSExpression
{
  @public
  NSString	*_keyPath;
}
@end

@interface GSFunctionExpression : NSExpression
{
  @public
  NSString		*_function;
  NSArray		*_args;
  unsigned int		_argc;
  SEL                   _selector;
  NSString              *_op;        // Not retained;
}
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
@interface GSBlockPredicate : NSPredicate
{
  GSBlockPredicateBlock _block;
}

- (instancetype) initWithBlock: (GSBlockPredicateBlock)block;
@end


@interface GSBoundBlockPredicate : GSBlockPredicate
{
  GS_GENERIC_CLASS(NSDictionary,NSString*,id)* _bindings;
}
- (instancetype) initWithBlock: (GSBlockPredicateBlock)block
                      bindings: (GS_GENERIC_CLASS(NSDictionary,NSString*,id)*)bindings;
@end
#endif

@implementation NSPredicate

+ (NSPredicate *) predicateWithFormat: (NSString *) format, ...
{
  NSPredicate	*p;
  va_list	va;

  va_start(va, format);
  p = [self predicateWithFormat: format arguments: va];
  va_end(va);
  return p;
}

+ (NSPredicate *) predicateWithFormat: (NSString *)format
                        argumentArray: (NSArray *)args
{
  GSPredicateScanner	*s;
  NSPredicate		*p;

  s = [[GSPredicateScanner alloc] initWithString: format
                                            args: args];
  p = [s parse];
  RELEASE(s);
  return p;
}

+ (NSPredicate *) predicateWithFormat: (NSString *)format
                            arguments: (va_list)args
{
  GSPredicateScanner	*s;
  NSPredicate		*p;
  const char            *ptr = [format UTF8String];
  NSMutableArray        *arr = [NSMutableArray arrayWithCapacity: 10];

  while (*ptr != 0)
    {
      char      c = *ptr++;

      if (c == '%')
        {
          c = *ptr;
          switch (c)
            {
              case '%':
                ptr++;
                break;

              case 'K':
              case '@':
                ptr++;
                [arr addObject: va_arg(args, id)];
                break;

              case 'c':
                ptr++;
                [arr addObject: [NSNumber numberWithChar:
                  (char)va_arg(args, NSInteger)]];
                break;

              case 'C':
                ptr++;
                [arr addObject: [NSNumber numberWithShort:
                  (short)va_arg(args, NSInteger)]];
                break;

              case 'd':
              case 'D':
              case 'i':
                ptr++;
                [arr addObject: [NSNumber numberWithInt:
                  va_arg(args, int)]];
                break;

              case 'o':
              case 'O':
              case 'u':
              case 'U':
              case 'x':
              case 'X':
                ptr++;
                [arr addObject: [NSNumber numberWithUnsignedInt:
                  va_arg(args, unsigned)]];
                break;

              case 'e':
              case 'E':
              case 'f':
              case 'g':
              case 'G':
                ptr++;
                [arr addObject: [NSNumber numberWithDouble:
                  va_arg(args, double)]];
                break;

              case 'h':
                ptr++;
                if (*ptr != 0)
                  {
                    c = *ptr;
                    if (c == 'i')
                      {
                        [arr addObject: [NSNumber numberWithShort:
                          (short)va_arg(args, NSInteger)]];
                      }
                    if (c == 'u')
                      {
                        [arr addObject: [NSNumber numberWithUnsignedShort:
                          (unsigned short)va_arg(args, NSInteger)]];
                      }
                  }
                break;

              case 'q':
                ptr++;
                if (*ptr != 0)
                  {
                    c = *ptr;
                    if (c == 'i')
                      {
                        [arr addObject: [NSNumber numberWithLongLong:
                          va_arg(args, long long)]];
                      }
                    if (c == 'u' || c == 'x' || c == 'X')
                      {
                        [arr addObject: [NSNumber numberWithUnsignedLongLong:
                          va_arg(args, unsigned long long)]];
                      }
                  }
                break;
            }
        }
      else if (c == '\'')
        {
          while (*ptr != 0)
            {
              if (*ptr++ == '\'')
                {
                  break;
                }
            }
        }
      else if (c == '"')
        {
          while (*ptr != 0)
            {
              if (*ptr++ == '"')
                {
                  break;
                }
            }
        }
    }
  s = [[GSPredicateScanner alloc] initWithString: format
                                            args: arr];
  p = [s parse];
  RELEASE(s);
  return p;
}

+ (NSPredicate *) predicateWithValue: (BOOL)value
{
  if (value)
    {
      return AUTORELEASE([GSTruePredicate new]);
    }
  else
    {
      return AUTORELEASE([GSFalsePredicate new]);
    }
}

// we don't ever instantiate NSPredicate

- (id) copyWithZone: (NSZone *)z
{
  return NSCopyObject(self, 0, z);
}

- (BOOL) evaluateWithObject: (id)object
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (NSString *) description
{
  return [self predicateFormat];
}

- (NSString *) predicateFormat
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSPredicate *) predicateWithSubstitutionVariables: (NSDictionary *)variables
{
  return AUTORELEASE([self copy]);  
}

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) evaluateWithObject: (id)object
      substitutionVariables: (GS_GENERIC_CLASS(NSDictionary, NSString*, id)*)variables
{
  return [[self predicateWithSubstitutionVariables: variables]
                                evaluateWithObject: object];
}
#endif
- (Class) classForCoder
{
  return [NSPredicate class];
}

- (void) encodeWithCoder: (NSCoder *) coder;
{
  // FIXME
  [self subclassResponsibility: _cmd];
}

- (id) initWithCoder: (NSCoder *) coder;
{
  // FIXME
  [self subclassResponsibility: _cmd];
  return self;
}

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
+ (NSPredicate*)predicateWithBlock: (GSBlockPredicateBlock)block
{
  return [[[GSBlockPredicate alloc] initWithBlock: block] autorelease];
}
#endif
@end

@implementation GSTruePredicate

- (id) copyWithZone: (NSZone *)z
{
  return RETAIN(self);
}

- (BOOL) evaluateWithObject: (id)object
{
  return YES;
}

- (NSString *) predicateFormat
{
  return @"TRUEPREDICATE";
}

@end

@implementation GSFalsePredicate

- (id) copyWithZone: (NSZone *)z
{
  return RETAIN(self);
}

- (BOOL) evaluateWithObject: (id)object
{
  return NO;
}

- (NSString *) predicateFormat
{
  return @"FALSEPREDICATE";
}

@end

@implementation NSCompoundPredicate

+ (NSPredicate *) andPredicateWithSubpredicates: (NSArray *)list
{
  return AUTORELEASE([[GSAndCompoundPredicate alloc]
    initWithType: NSAndPredicateType subpredicates: list]);
}

+ (NSPredicate *) notPredicateWithSubpredicate: (NSPredicate *)predicate
{
  NSArray       *list;

  list = [NSArray arrayWithObject: predicate];
  return AUTORELEASE([[GSNotCompoundPredicate alloc] 
    initWithType: NSNotPredicateType subpredicates: list]);
}

+ (NSPredicate *) orPredicateWithSubpredicates: (NSArray *)list
{
  return AUTORELEASE([[GSOrCompoundPredicate alloc]
    initWithType: NSOrPredicateType subpredicates: list]);
}

- (NSCompoundPredicateType) compoundPredicateType
{
  return _type;
}

- (id) initWithType: (NSCompoundPredicateType)type
      subpredicates: (NSArray *)list
{
  if ((self = [super init]) != nil)
    {
      _type = type;
      ASSIGNCOPY(_subs, list);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_subs);
  [super dealloc];
}

- (id) copyWithZone: (NSZone *)z
{
  return [[[self class] alloc] initWithType: _type subpredicates: _subs];
}

- (NSArray *) subpredicates
{
  return _subs;
}

- (NSPredicate *) predicateWithSubstitutionVariables: (NSDictionary *)variables
{
  unsigned int count = [_subs count];
  NSMutableArray *esubs = [NSMutableArray arrayWithCapacity: count];
  unsigned int i;
  NSPredicate  *p;

  for (i = 0; i < count; i++)
    {
      [esubs addObject: [[_subs objectAtIndex: i] 
                            predicateWithSubstitutionVariables: variables]];
    }

  p = [[[self class] alloc] initWithType: _type subpredicates: esubs];
  return AUTORELEASE(p);
}

- (Class) classForCoder
{
  return [NSCompoundPredicate class];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
}

- (id) initWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
  return self;
}

@end

@implementation GSAndCompoundPredicate

- (BOOL) evaluateWithObject: (id) object
{
  NSEnumerator	*e = [_subs objectEnumerator];
  NSPredicate	*p;

  while ((p = [e nextObject]) != nil)
    {
      if ([p evaluateWithObject: object] == NO)
        {
          return NO;  // any NO returns NO
        }
    }
  return YES;  // all are true
}

- (NSString *) predicateFormat
{
  NSString	*fmt = @"";
  NSEnumerator	*e = [_subs objectEnumerator];
  NSPredicate	*sub;
  unsigned	cnt = 0;

  while ((sub = [e nextObject]) != nil)
    {
      // when to add ()? -> if sub is compound and of type "or"
      if (cnt == 0)
        {
          fmt = [sub predicateFormat];  // first
        }
      else
        {
          if (cnt == 1
            && [[_subs objectAtIndex: 0]
              isKindOfClass: [NSCompoundPredicate class]]
            && [(NSCompoundPredicate *)[_subs objectAtIndex: 0]
              compoundPredicateType] == NSOrPredicateType)
            {
              // we need () around first OR on left side
              fmt = [NSString stringWithFormat: @"(%@)", fmt]; 
            }
          if ([sub isKindOfClass: [NSCompoundPredicate class]]
              && [(NSCompoundPredicate *) sub compoundPredicateType]
              == NSOrPredicateType)
            {
              // we need () around right OR
              fmt = [NSString stringWithFormat: @"%@ AND (%@)",
                              fmt, [sub predicateFormat]];
            }
          else
            {
              fmt = [NSString stringWithFormat: @"%@ AND %@",
                              fmt, [sub predicateFormat]];
            }
        }
      cnt++;
    }
  return fmt;
}

@end

@implementation GSOrCompoundPredicate

- (BOOL) evaluateWithObject: (id)object
{
  NSEnumerator	*e = [_subs objectEnumerator];
  NSPredicate	*p;

  while ((p = [e nextObject]) != nil)
    {
      if ([p evaluateWithObject: object] == YES)
        {
          return YES;  // any YES returns YES
        }
    }
  return NO;  // none is true
}

- (NSString *) predicateFormat
{
  NSString	*fmt = @"";
  NSEnumerator	*e = [_subs objectEnumerator];
  NSPredicate	*sub;

  while ((sub = [e nextObject]) != nil)
    {
      if ([fmt length] > 0)
        {
          fmt = [NSString stringWithFormat: @"%@ OR %@",
                          fmt, [sub predicateFormat]];
        }
      else
        {
          fmt = [sub predicateFormat];  // first
        }
    }
  return fmt;
}

@end

@implementation GSNotCompoundPredicate

- (BOOL) evaluateWithObject: (id)object
{
  NSPredicate *sub = [_subs objectAtIndex: 0];

  return ![sub evaluateWithObject: object];
}

- (NSString *) predicateFormat
{
  NSPredicate *sub = [_subs objectAtIndex: 0];

  if ([sub isKindOfClass: [NSCompoundPredicate class]]
    && [(NSCompoundPredicate *)sub compoundPredicateType]
      != NSNotPredicateType)
    {
      return [NSString stringWithFormat: @"NOT(%@)", [sub predicateFormat]];
    }
  return [NSString stringWithFormat: @"NOT %@", [sub predicateFormat]];
}

@end

@implementation NSComparisonPredicate

+ (NSPredicate *) predicateWithLeftExpression: (NSExpression *)left
                              rightExpression: (NSExpression *)right
                               customSelector: (SEL) sel
{
  return AUTORELEASE([[self alloc] initWithLeftExpression: left
                                          rightExpression: right 
                                           customSelector: sel]);
}

+ (NSPredicate *) predicateWithLeftExpression: (NSExpression *)left
                              rightExpression: (NSExpression *)right
                                     modifier: (NSComparisonPredicateModifier)modifier
                                         type: (NSPredicateOperatorType)type
                                      options: (NSUInteger)opts
{
  return AUTORELEASE([[self alloc] initWithLeftExpression: left 
                                          rightExpression: right
                                                 modifier: modifier 
                                                     type: type 
                                                  options: opts]);
}

- (NSPredicate *) initWithLeftExpression: (NSExpression *)left
                         rightExpression: (NSExpression *)right
                          customSelector: (SEL)sel
{
  if ((self = [super init]) != nil)
    {
      ASSIGN(_left, left);
      ASSIGN(_right, right);
      _selector = sel;
      _type = NSCustomSelectorPredicateOperatorType;
    }
  return self;
}

- (id) initWithLeftExpression: (NSExpression *)left
              rightExpression: (NSExpression *)right
                     modifier: (NSComparisonPredicateModifier)modifier
                         type: (NSPredicateOperatorType)type
                      options: (NSUInteger)opts
{
  if ((self = [super init]) != nil)
    {
      ASSIGN(_left, left);
      ASSIGN(_right, right);
      _modifier = modifier;
      _type = type;
      _options = opts;
    }
  return self;
}

- (void) dealloc;
{
  RELEASE(_left);
  RELEASE(_right);
  [super dealloc];
}

- (NSComparisonPredicateModifier) comparisonPredicateModifier
{
  return _modifier;
}

- (SEL) customSelector
{
  return _selector;
}

- (NSExpression *) leftExpression
{
  return _left;
}

- (NSUInteger) options
{
  return _options;
}

- (NSPredicateOperatorType) predicateOperatorType
{
  return _type;
}

- (NSExpression *) rightExpression
{
  return _right;
}

- (NSString *) predicateFormat
{
  NSString	*modi = @"";
  NSString	*comp = @"?comparison?";
  NSString	*opt = @"";

  switch (_modifier)
    {
      case NSDirectPredicateModifier:
        break;
      case NSAnyPredicateModifier:
        modi = @"ANY "; 
        break;
      case NSAllPredicateModifier:
        modi = @"ALL"; 
        break;
      default:
        modi = @"?modifier?";
        break;
    }
  switch (_type)
    {
      case NSLessThanPredicateOperatorType:
        comp = @"<";
        break;
      case NSLessThanOrEqualToPredicateOperatorType:
        comp = @"<=";
        break;
      case NSGreaterThanPredicateOperatorType:
        comp = @">=";
        break;
      case NSGreaterThanOrEqualToPredicateOperatorType:
        comp = @">";
        break;
      case NSEqualToPredicateOperatorType:
        comp = @"=";
        break;
      case NSNotEqualToPredicateOperatorType:
        comp = @"!=";
        break;
      case NSMatchesPredicateOperatorType:
        comp = @"MATCHES";
        break;
      case NSLikePredicateOperatorType:
        comp = @"LIKE";
        break;
      case NSBeginsWithPredicateOperatorType:
        comp = @"BEGINSWITH";
        break;
      case NSEndsWithPredicateOperatorType:
        comp = @"ENDSWITH";
        break;
      case NSInPredicateOperatorType:
        comp = @"IN";
        break;
      case NSCustomSelectorPredicateOperatorType: 
        comp = NSStringFromSelector(_selector);
        break;
      case NSContainsPredicateOperatorType: 
        comp = @"CONTAINS";
        break;
      case NSBetweenPredicateOperatorType: 
        comp = @"BETWEEN";
        break;
    }
  switch (_options)
    {
      case NSCaseInsensitivePredicateOption:
        opt = @"[c]";
        break;
      case NSDiacriticInsensitivePredicateOption:
        opt = @"[d]";
        break;
      case NSCaseInsensitivePredicateOption
        | NSDiacriticInsensitivePredicateOption:
        opt = @"[cd]";
        break;
      default:
        //opt = @"[?options?]";
        break;
    }
  return [NSString stringWithFormat: @"%@%@ %@%@ %@",
           modi, _left, comp, opt, _right];
}

- (NSPredicate *) predicateWithSubstitutionVariables: (NSDictionary *)variables
{
  NSExpression *left;
  NSExpression *right;
   
  left = [_left _expressionWithSubstitutionVariables: variables];
  right = [_right _expressionWithSubstitutionVariables: variables];
  if (_type == NSCustomSelectorPredicateOperatorType)
    {
      return [NSComparisonPredicate predicateWithLeftExpression: left 
                                                rightExpression: right 
                                                 customSelector: _selector];
    }
  else
    {
      return [NSComparisonPredicate predicateWithLeftExpression: left 
                                                rightExpression: right 
                                                       modifier: _modifier 
                                                           type: _type 
                                                        options: _options];
    }
}

#if	GS_USE_ICU == 1
static BOOL
GSICUStringMatchesRegex(NSString *string, NSString *regex, NSStringCompareOptions opts)
{
  BOOL result = NO;
  UErrorCode error = 0;
  uint32_t flags = 0;
  NSUInteger stringLength = [string length];
  NSUInteger regexLength = [regex length];
  unichar *stringBuffer;
  unichar *regexBuffer;
  URegularExpression *icuregex = NULL;  

  stringBuffer = malloc(stringLength * sizeof(unichar));
  if (NULL == stringBuffer) { return NO; }
  regexBuffer = malloc(regexLength * sizeof(unichar));
  if (NULL == regexBuffer) { free(stringBuffer); return NO; }

  [string getCharacters: stringBuffer range: NSMakeRange(0, stringLength)];
  [regex getCharacters: regexBuffer range: NSMakeRange(0, regexLength)];

  flags |= UREGEX_DOTALL; // . is supposed to recognize newlines
  if ((opts & NSCaseInsensitiveSearch) != 0) { flags |= UREGEX_CASE_INSENSITIVE; }

  icuregex = uregex_open(regexBuffer, regexLength, flags, NULL, &error);
  if (icuregex != NULL && U_SUCCESS(error))
    {
      uregex_setText(icuregex, stringBuffer, stringLength, &error);
      result = uregex_matches(icuregex, 0, &error);
    }
  uregex_close(icuregex);

  free(stringBuffer);
  free(regexBuffer);

  return result;
}
#endif

- (double) doubleValueFor: (id)value
{
  if ([value isKindOfClass: [NSDate class]])
    {
      return [(NSDate*)value timeIntervalSinceReferenceDate];
    }
  else
    {
      return [value doubleValue];
    }
}

- (BOOL) _evaluateLeftValue: (id)leftResult
		 rightValue: (id)rightResult
		     object: (id)object
{
  unsigned compareOptions = 0;
  BOOL leftIsNil;
  BOOL rightIsNil;

  leftIsNil = (leftResult == nil || [leftResult isEqual: [NSNull null]]);
  rightIsNil = (rightResult == nil || [rightResult isEqual: [NSNull null]]);
  if (leftIsNil || rightIsNil)
    {
      if (leftIsNil == rightIsNil)
        {
          /* Both of the values are nil.
           * The result is YES if equality is requested.
           */
          if (NSEqualToPredicateOperatorType == _type
            || NSLessThanOrEqualToPredicateOperatorType == _type
            || NSGreaterThanOrEqualToPredicateOperatorType == _type)
            {
              return YES;
            }
        }
      else if (NSNotEqualToPredicateOperatorType == _type)
        {
          /* One, but not both of the values are nil.
           * The result is YES if inequality is requested.
           */
          return YES;
        }
      return NO;
    }

  // Change predicate options into string options.
  if (!(_options & NSDiacriticInsensitivePredicateOption))
    {
      compareOptions |= NSLiteralSearch;
    }
  if (_options & NSCaseInsensitivePredicateOption)
    {
      compareOptions |= NSCaseInsensitiveSearch;
    }

  /* This is a very optimistic implementation,
   * hoping that the values are of the right type.
   */
  switch (_type)
    {
      case NSLessThanPredicateOperatorType:
        {
          double ld = [self doubleValueFor: leftResult];
          double rd = [self doubleValueFor: rightResult];
          return (ld < rd) ? YES : NO;
        }
      case NSLessThanOrEqualToPredicateOperatorType:
        {
          double ld = [self doubleValueFor: leftResult];
          double rd = [self doubleValueFor: rightResult];
          return (ld <= rd) ? YES : NO;
        }
      case NSGreaterThanPredicateOperatorType:
        {
          double ld = [self doubleValueFor: leftResult];
          double rd = [self doubleValueFor: rightResult];
          return (ld > rd) ? YES : NO;
        }
      case NSGreaterThanOrEqualToPredicateOperatorType:
        {
          double ld = [self doubleValueFor: leftResult];
          double rd = [self doubleValueFor: rightResult];
          return (ld >= rd) ? YES : NO;
        }
      case NSEqualToPredicateOperatorType:
	return [leftResult isEqual: rightResult];
      case NSNotEqualToPredicateOperatorType:
	return ![leftResult isEqual: rightResult];
      case NSMatchesPredicateOperatorType:
#if	GS_USE_ICU == 1
	return GSICUStringMatchesRegex(leftResult, rightResult, compareOptions);
#else
	return [leftResult compare: rightResult options: compareOptions]
	  == NSOrderedSame ? YES : NO;
#endif
      case NSLikePredicateOperatorType:
#if	GS_USE_ICU == 1
	{
	  NSString *regex;

	  /* The right hand is a pattern with '?' meaning match one character,
	   * and '*' meaning match zero or more characters, so translate that
	   * into a regex.
	   */
	  regex = [rightResult stringByReplacingOccurrencesOfString: @"*"
							 withString: @".*"];
	  regex = [regex stringByReplacingOccurrencesOfString: @"?"
						   withString: @".?"];
	  regex = [NSString stringWithFormat: @"^%@$", regex];
	  return GSICUStringMatchesRegex(leftResult, regex, compareOptions);
	}
#else
	return [leftResult compare: rightResult options: compareOptions]
	  == NSOrderedSame ? YES : NO;
#endif
      case NSBeginsWithPredicateOperatorType:
	{
	  NSRange range = NSMakeRange(0, [rightResult length]);

	  return ([leftResult compare: rightResult
			      options: compareOptions
				range: range] == NSOrderedSame ? YES : NO);
	}
      case NSEndsWithPredicateOperatorType:
	{
	  NSRange range;
          NSUInteger    ll = [leftResult length];
          NSUInteger    rl = [rightResult length];

          if (ll < rl)
            {
              return NO;
            }
	  range = NSMakeRange(ll - rl, rl);
	  return ([leftResult compare: rightResult
			      options: compareOptions
				range: range] == NSOrderedSame ? YES : NO);
	}
      case NSInPredicateOperatorType:
	/* Handle special case where rightResult is a collection
	 * and leftResult an element of it.
	 */
	if (![rightResult isKindOfClass: [NSString class]])
	  {
	    NSEnumerator *e;
	    id value;

	    if (![rightResult respondsToSelector: @selector(objectEnumerator)])
	      {
		[NSException raise: NSInvalidArgumentException 
			    format: @"The right hand side for an IN operator "
		  @"must be a collection"];
	      }

	    e = [rightResult objectEnumerator];
	    while ((value = [e nextObject]))
	      {
		if ([value isEqual: leftResult]) 
		  return YES;		
	      }

	    return NO;
	  }
	return ([rightResult rangeOfString: leftResult
				   options: compareOptions].location
	  != NSNotFound ? YES : NO);
      case NSCustomSelectorPredicateOperatorType:
	{
	  BOOL (*function)(id,SEL,id)
            = (BOOL (*)(id,SEL,id))[leftResult methodForSelector: _selector];
	  return function(leftResult, _selector, rightResult);
	}
      default:
	return NO;
    }
}

- (BOOL) evaluateWithObject: (id)object
{
  id leftValue = [_left expressionValueWithObject: object context: nil];
  id rightValue = [_right expressionValueWithObject: object context: nil];

  if (_modifier == NSDirectPredicateModifier)
    {
      return [self _evaluateLeftValue: leftValue
			   rightValue: rightValue
			       object: object];
    }
  else
    {		
      BOOL result = (_modifier == NSAllPredicateModifier);
      NSEnumerator *e;
      id value;

      if (![leftValue respondsToSelector: @selector(objectEnumerator)])
        {
          [NSException raise: NSInvalidArgumentException 
                      format: @"The left hand side for an ALL or ANY operator must be a collection"];
        }

      e = [leftValue objectEnumerator];
      while ((value = [e nextObject]))
        {
          BOOL eval = [self _evaluateLeftValue: value
				    rightValue: rightValue
					object: object];
          if (eval != result) 
            return eval;		
        }

      return result;
    }
}

- (id) copyWithZone: (NSZone *)z
{
  NSComparisonPredicate *copy;

  copy = (NSComparisonPredicate *)NSCopyObject(self, 0, z);
  copy->_left = [_left copyWithZone: z];
  copy->_right = [_right copyWithZone: z];
  return copy;
}

- (Class) classForCoder
{
  return [NSComparisonPredicate class];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
}

- (id) initWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
  return self;
}

@end



@implementation NSExpression

+ (void) initialize
{
  if (self == [NSExpression class] && nil == evaluatedObjectExpression)
    {
      evaluatedObjectExpression = [GSEvaluatedObjectExpression new];
    }
}

+ (NSExpression *) expressionForConstantValue: (id)obj
{
  GSConstantValueExpression *e;

  e = [[GSConstantValueExpression alloc] 
          initWithExpressionType: NSConstantValueExpressionType];
  ASSIGN(e->_obj, obj);
  return AUTORELEASE(e);
}

+ (NSExpression *) expressionForEvaluatedObject
{
  return evaluatedObjectExpression;
}

+ (NSExpression *) expressionForFunction: (NSString *)name
                               arguments: (NSArray *)args
{
  GSFunctionExpression	*e;
  NSString		*s;

  e = [[GSFunctionExpression alloc]
    initWithExpressionType: NSFunctionExpressionType];
  s = [NSString stringWithFormat: @"_eval_%@:", name];
  e->_selector = NSSelectorFromString(s);
  if (![e respondsToSelector: e->_selector])
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Unknown function implementation: %@", name];
    }
  ASSIGN(e->_function, name);
  e->_argc = [args count];
  ASSIGN(e->_args, args);
  if ([name isEqualToString: @"_add"]) e->_op = @"+";
  else if ([name isEqualToString: @"_sub"]) e->_op = @"-";
  else if ([name isEqualToString: @"_mul"]) e->_op = @"*";
  else if ([name isEqualToString: @"_div"]) e->_op = @"/";
  else if ([name isEqualToString: @"_pow"]) e->_op = @"**";
  return AUTORELEASE(e);
}

+ (NSExpression *) expressionForKeyPath: (NSString *)path
{
  GSKeyPathExpression *e;

  if (![path isKindOfClass: [NSString class]])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Keypath is not NSString: %@", path];
    }
  e = [[GSKeyPathExpression alloc] 
          initWithExpressionType: NSKeyPathExpressionType];
  ASSIGN(e->_keyPath, path);
  return AUTORELEASE(e);
}

+ (NSExpression *) expressionForVariable: (NSString *)string
{
  GSVariableExpression *e;

  e = [[GSVariableExpression alloc] 
          initWithExpressionType: NSVariableExpressionType];
  ASSIGN(e->_variable, string);
  return AUTORELEASE(e);
}

- (id) initWithExpressionType: (NSExpressionType)type
{
  if ((self = [super init]) != nil)
    {
      _type = type;
    }
  return self;
}

- (id) copyWithZone: (NSZone *)z
{
  return NSCopyObject(self, 0, z);
}

- (NSArray *) arguments
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) constantValue
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSString *) description
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSExpressionType) expressionType
{
  return _type;
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSString *) function
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSString *) keyPath
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSExpression *) operand
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSString *) variable
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (Class) classForCoder
{
  return [NSExpression class];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
}

- (id) initWithCoder: (NSCoder *)coder
{
  // FIXME
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  [self subclassResponsibility: _cmd];
  return nil;
}

@end

@implementation GSConstantValueExpression

- (id) constantValue
{
  return _obj;
}

- (NSString *) description
{
  if ([_obj isKindOfClass: [NSString class]])
    {
      NSMutableString	*result = nil;

      /* Quote the result string as necessary.
       */
      GSPropertyListMake(_obj, nil, NO, YES, 2, &result);
      return result;
    }
  else if ([_obj isKindOfClass: [NSDate class]])
    {
      return [NSString stringWithFormat: @"CAST(%15.6f, \"NSDate\")",
                       [(NSDate*)_obj timeIntervalSinceReferenceDate]];
    }
  return [_obj description];
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{
  return _obj;
}

- (void) dealloc
{
  RELEASE(_obj);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  GSConstantValueExpression *copy;

  copy = (GSConstantValueExpression *)[super copyWithZone: zone];
  copy->_obj = [_obj copyWithZone: zone];
  return copy;
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  return self;
}

@end

@implementation GSEvaluatedObjectExpression

- (NSString *) description
{
  return @"SELF";
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{
  return object;
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  return self;
}

- (NSString *) keyPath
{
  return @"SELF";
}
@end

@implementation GSVariableExpression

- (NSString *) description
{
  return [NSString stringWithFormat: @"$%@", _variable];
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{
  return [context objectForKey: _variable];
}

- (NSString *) variable
{
  return _variable;
}

- (void) dealloc;
{
  RELEASE(_variable);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  GSVariableExpression *copy;

  copy = (GSVariableExpression *)[super copyWithZone: zone];
  copy->_variable = [_variable copyWithZone: zone];
  return copy;
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  id result = [variables objectForKey: _variable];

  if (result != nil)
    {
      return [NSExpression expressionForConstantValue: result];
    }
  else
    {
      return self;
    }
}

@end

@implementation GSKeyPathExpression

- (NSString *) description
{
  return _keyPath;
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{
  return [object valueForKeyPath: _keyPath];
}

- (NSString *) keyPath
{
  return _keyPath;
}

- (void) dealloc;
{
  RELEASE(_keyPath);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  GSKeyPathExpression *copy;

  copy = (GSKeyPathExpression *)[super copyWithZone: zone];
  copy->_keyPath = [_keyPath copyWithZone: zone];
  return copy;
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  return self;
}

@end

@implementation GSFunctionExpression

- (NSArray *) arguments
{
  return _args;
}

- (NSString *) description
{
  if (nil != _op && 1 == [_args count])
    {
      GSFunctionExpression      *a0 = [_args objectAtIndex: 0];

      if (YES == [a0 isKindOfClass: [self class]] && nil != a0->_op)
        {
          return [NSString stringWithFormat: @"%@(%@)", _op, a0];
        }
      return [NSString stringWithFormat: @"%@%@", _op, a0];
    }

  if (nil != _op)
    {
      GSFunctionExpression      *a0 = [_args objectAtIndex: 0];
      GSFunctionExpression      *a1 = [_args objectAtIndex: 1];

      if (YES == [a0 isKindOfClass: [self class]] && nil != a0->_op)
        {
          if (YES == [a1 isKindOfClass: [self class]] && nil != a1->_op)
            {
              return [NSString stringWithFormat: @"(%@) %@ (%@)", a0, _op, a1];
            }
          return [NSString stringWithFormat: @"(%@) %@ %@", a0, _op, a1];
        }

      if (YES == [a1 isKindOfClass: [self class]] && nil != a1->_op)
        {
          return [NSString stringWithFormat: @"%@ %@ (%@)", a0, _op, a1];
        }

      return [NSString stringWithFormat: @"%@ %@ %@", a0, _op, a1];
    }
  return [NSString stringWithFormat: @"%@(%@)", [self function], _args];
}

- (NSString *) function
{
  return _function;
}

- (NSString *) keyPath
{
  return nil;
}

- (id) expressionValueWithObject: (id)object
			 context: (NSMutableDictionary *)context
{ 
  // temporary space 
  NSMutableArray	*eargs = [NSMutableArray arrayWithCapacity: _argc];
  unsigned int i;

  for (i = 0; i < _argc; i++)
    {
      [eargs addObject: [[_args objectAtIndex: i] 
        expressionValueWithObject: object context: context]];
    }
  // apply method selector
  return [self performSelector: _selector
                    withObject: eargs];
}

- (void) dealloc;
{
  RELEASE(_args);
  RELEASE(_function);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  GSFunctionExpression *copy;

  copy = (GSFunctionExpression *)[super copyWithZone: zone];
  copy->_function = [_function copyWithZone: zone];
  copy->_args = [_args copyWithZone: zone];
  return copy;
}

- (NSEnumerator*) _enum: (NSArray *)expressions
{
  id    o;

  /* Check to see if this is aggregating over a collection.
   */
  if (1 == _argc && [(o = [expressions lastObject])
    respondsToSelector: @selector(objectEnumerator)])
    {
      return [o objectEnumerator];
    }
  return [expressions objectEnumerator];
}

- (id) _expressionWithSubstitutionVariables: (NSDictionary *)variables
{
  NSMutableArray *args = [NSMutableArray arrayWithCapacity: _argc];
  unsigned int i;
      
  for (i = 0; i < _argc; i++)
    {
      [args addObject: [[_args objectAtIndex: i] 
                           _expressionWithSubstitutionVariables: variables]];
    }

   return [NSExpression expressionForFunction: _function arguments: args];
}

- (id) _eval__chs: (NSArray *)expressions
{
  return [NSNumber numberWithInt: -[[expressions objectAtIndex: 0] intValue]];
}

- (id) _eval__first: (NSArray *)expressions
{
  return [[expressions objectAtIndex: 0] objectAtIndex: 0];
}

- (id) _eval__last: (NSArray *)expressions
{
  return [[expressions objectAtIndex: 0] lastObject];
}

- (id) _eval__index: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  if ([left isKindOfClass: [NSDictionary class]])
    {
      return [left objectForKey: right];
    }
  else
    {
      // raises exception if invalid
      return [left objectAtIndex: [right unsignedIntValue]];
    }
}

- (id) _eval__pow: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  return [NSNumber numberWithDouble:
    pow([left doubleValue], [right doubleValue])];
}

- (id) _eval__mul: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  return [NSNumber numberWithDouble: [left doubleValue] * [right doubleValue]];
}

- (id) _eval__div: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  return [NSNumber numberWithDouble: [left doubleValue] / [right doubleValue]];
}

- (id) _eval__add: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  return [NSNumber numberWithDouble: [left doubleValue] + [right doubleValue]];
}

- (id) _eval__sub: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  return [NSNumber numberWithDouble: [left doubleValue] - [right doubleValue]];
}

- (id) _eval_count: (NSArray *)expressions
{
  NSAssert(_argc == 1, NSInternalInconsistencyException);
  return [NSNumber numberWithUnsignedInt:
    [[expressions objectAtIndex: 0] count]];
}

- (id) _eval_avg: (NSArray *)expressions 
{
  NSEnumerator  *e = [self _enum: expressions];
  double        sum = 0.0;
  unsigned      count = 0;
  id            o;
    
  while (nil != (o = [e nextObject]))
    {
      sum += [o doubleValue];
      count++;
    }
  if (count == 0)
    {
      return [NSNumber numberWithDouble: 0.0];
    }
  return [NSNumber numberWithDouble: sum / count];
}

- (id) _eval_max: (NSArray *)expressions
{
  NSEnumerator  *e = [self _enum: expressions];
  id            o = [e nextObject];
  double        max = (nil == o) ? 0.0 : [o doubleValue];
  double        cur;
  
  while (nil != (o = [e nextObject]))
    {
      cur = [o doubleValue];
      if (max < cur)
        {
          max = cur;
        }
    }
  return [NSNumber numberWithDouble: max];
}

- (id) _eval_min: (NSArray *)expressions
{
  NSEnumerator  *e = [self _enum: expressions];
  id            o = [e nextObject];
  double        min = (nil == o ? 0.0 : [o doubleValue]);
  double        cur;

  while (nil != (o = [e nextObject]))
    {
      cur = [o doubleValue];
      if (min > cur)
        {
          min = cur;
        }
    }
  return [NSNumber numberWithDouble: min];
}

- (id) _eval_sum: (NSArray *)expressions
{
  NSEnumerator  *e = [self _enum: expressions];
  double        sum = 0.0;
  id            o;

  while (nil != (o = [e nextObject]))
    {
      sum += [o doubleValue];
    }
  return [NSNumber numberWithDouble: sum];
}

- (id) _eval_CAST: (NSArray *)expressions
{
  id left = [expressions objectAtIndex: 0];
  id right = [expressions objectAtIndex: 1];

  if ([right isEqualToString: @"NSDate"])
    {
      return [NSDate dateWithTimeIntervalSinceReferenceDate:
	[left doubleValue]];
    }

  NSLog(@"Cast to unknown type %@", right);
  return nil;
}

// add arithmetic functions: average, median, mode, stddev, sqrt, log, ln, exp, floor, ceiling, abs, trunc, random, randomn, now

@end



@implementation NSArray (NSPredicate)

- (NSArray *) filteredArrayUsingPredicate: (NSPredicate *)predicate
{
  NSMutableArray	*result;
  NSEnumerator		*e = [self objectEnumerator];
  id			object;

  result = [NSMutableArray arrayWithCapacity: [self count]];
  while ((object = [e nextObject]) != nil)
    {
      if ([predicate evaluateWithObject: object] == YES)
        {
          [result addObject: object];  // passes filter
        }
    }
  return GS_IMMUTABLE(result);
}

@end

@implementation NSMutableArray (NSPredicate)

- (void) filterUsingPredicate: (NSPredicate *)predicate
{	
  unsigned	count = [self count];

  while (count-- > 0)
    {
      id	object = [self objectAtIndex: count];
	
      if ([predicate evaluateWithObject: object] == NO)
        {
          [self removeObjectAtIndex: count];
        }
    }
}

@end

@implementation NSSet (NSPredicate)

- (NSSet *) filteredSetUsingPredicate: (NSPredicate *)predicate
{
  NSMutableSet	*result;
  NSEnumerator	*e = [self objectEnumerator];
  id		object;

  result = [NSMutableSet setWithCapacity: [self count]];
  while ((object = [e nextObject]) != nil)
    {
      if ([predicate evaluateWithObject: object] == YES)
        {
          [result addObject: object];  // passes filter
        }
    }
  return GS_IMMUTABLE(result);
}

@end

@implementation NSMutableSet (NSPredicate)

- (void) filterUsingPredicate: (NSPredicate *)predicate
{
  NSMutableSet	*rejected;
  NSEnumerator	*e = [self objectEnumerator];
  id		object;

  rejected = [NSMutableSet setWithCapacity: [self count]];
  while ((object = [e nextObject]) != nil)
    {
      if ([predicate evaluateWithObject: object] == NO)
        {
          [rejected addObject: object];
        }
    }
  [self minusSet: rejected];
}

@end



@implementation GSPredicateScanner

- (id) initWithString: (NSString*)format
                 args: (NSArray*)args
{
  self = [super initWithString: format];
  if (self != nil)
    {
      _args = [args objectEnumerator];
    }
  return self;
}

- (id) nextArg
{
  return [_args nextObject];
}

- (BOOL) scanPredicateKeyword: (NSString *)key
{
  // save to back up
  unsigned loc = [self scanLocation];
  unichar c;
  
  [self setCaseSensitive: NO];
  if (![self scanString: key intoString: NULL])
    {
      // no match
      return NO;
    }

  if ([self isAtEnd])
    {
       // ok
      return YES;
    }
  
  // Does the next character still belong to the token?
  c = [[self string] characterAtIndex: [self scanLocation]];
  if (![[NSCharacterSet alphanumericCharacterSet] characterIsMember: c])
    {
      // ok
      return YES;
    }

  // back up
  [self setScanLocation: loc];
  // no match
  return NO;
}

- (NSPredicate *) parse
{
  NSPredicate *r = nil;

  NS_DURING
    {
      r = [self parsePredicate];
    }
  NS_HANDLER
    {
      NSLog(@"Parsing failed for %@ with %@", [self string], localException);
      [localException raise];
    }
  NS_ENDHANDLER

  if (![self isAtEnd])
    {
      [NSException raise: NSInvalidArgumentException 
		  format: @"Format string contains extra characters: \"%@\"", 
		   [self string]];
    }
  return r;
}

- (NSPredicate *) parsePredicate
{
  return [self parseAnd];
}

- (NSPredicate *) parseAnd
{
  NSPredicate	*l = [self parseOr];

  while ([self scanPredicateKeyword: @"AND"]
    || [self scanPredicateKeyword: @"&&"])
    {
      NSPredicate	*r = [self parseOr];

      if ([r isKindOfClass: [NSCompoundPredicate class]]
        && [(NSCompoundPredicate *)r compoundPredicateType]
        == NSAndPredicateType)
        {
          NSCompoundPredicate   *right = (NSCompoundPredicate*)r;

          // merge
          if ([l isKindOfClass:[NSCompoundPredicate class]]
            && [(NSCompoundPredicate *)l compoundPredicateType]
            == NSAndPredicateType)
            {
              NSCompoundPredicate       *left;
              NSMutableArray            *subs;

              left = (NSCompoundPredicate*)l;
              subs = [[left subpredicates] mutableCopy];
              [subs addObjectsFromArray: [right subpredicates]];
              l = [NSCompoundPredicate andPredicateWithSubpredicates: subs];
              [subs release];
            }
          else
            {
              NSMutableArray            *subs;

              subs = [[right subpredicates] mutableCopy];
              [subs insertObject: l atIndex: 0];
              l = [NSCompoundPredicate andPredicateWithSubpredicates: subs];
              [subs release];
            }
        }
      else if ([l isKindOfClass: [NSCompoundPredicate class]]
        && [(NSCompoundPredicate *)l compoundPredicateType]
        == NSAndPredicateType)
        {
          NSCompoundPredicate   *left;
          NSMutableArray        *subs;

          left = (NSCompoundPredicate*)l;
          subs = [[left subpredicates] mutableCopy];
          [subs addObject: r];
          l = [NSCompoundPredicate andPredicateWithSubpredicates: subs];
          [subs release];
        }
      else
        {
          l = [NSCompoundPredicate andPredicateWithSubpredicates: 
            [NSArray arrayWithObjects: l, r, nil]];
        }
    }
  return l;
}

- (NSPredicate *) parseNot
{
  if ([self scanString: @"(" intoString: NULL])
    {
      NSPredicate *r = [self parsePredicate];
	
      if (![self scanString: @")" intoString: NULL])
        {
          [NSException raise: NSInvalidArgumentException 
                      format: @"Missing ) in compound predicate"];
        }
      return r;
    }

  if ([self scanPredicateKeyword: @"NOT"] || [self scanPredicateKeyword: @"!"])
    {
      // -> NOT NOT x or NOT (y)
      return [NSCompoundPredicate
                 notPredicateWithSubpredicate: [self parseNot]];
    }

  if ([self scanPredicateKeyword: @"TRUEPREDICATE"])
    {
      return [NSPredicate predicateWithValue: YES];
    }
  if ([self scanPredicateKeyword: @"FALSEPREDICATE"])
    {
      return [NSPredicate predicateWithValue: NO];
    }
  
  return [self parseComparison];
}

- (NSPredicate *) parseOr
{
  NSPredicate	*l = [self parseNot];

  while ([self scanPredicateKeyword: @"OR"]
    || [self scanPredicateKeyword: @"||"])
    {
      NSPredicate	*r = [self parseNot];

      if ([r isKindOfClass: [NSCompoundPredicate class]]
        && [(NSCompoundPredicate *)r compoundPredicateType]
        == NSOrPredicateType)
        {
          NSCompoundPredicate   *right = (NSCompoundPredicate*)r;

          // merge
          if ([l isKindOfClass: [NSCompoundPredicate class]]
            && [(NSCompoundPredicate *)l compoundPredicateType]
            == NSOrPredicateType)
            {
              NSCompoundPredicate       *left = (NSCompoundPredicate*)l;
              NSMutableArray            *subs;

              subs = [[left subpredicates] mutableCopy];
              [subs addObjectsFromArray: [right subpredicates]];
              l = [NSCompoundPredicate orPredicateWithSubpredicates: subs];
              [subs release];
            }
          else
            {
              NSMutableArray            *subs;

              subs = [[right subpredicates] mutableCopy];
              [subs insertObject: l atIndex: 0];
              l = [NSCompoundPredicate orPredicateWithSubpredicates: subs];
              [subs release];
            }
        }
      else if ([l isKindOfClass: [NSCompoundPredicate class]]
        && [(NSCompoundPredicate *)l compoundPredicateType]
        == NSOrPredicateType)
        {
          NSCompoundPredicate   *left = (NSCompoundPredicate*)l;
          NSMutableArray        *subs;

          subs = [[left subpredicates] mutableCopy];
          [subs addObject:r];
          l = [NSCompoundPredicate orPredicateWithSubpredicates: subs];
          [subs release];
        }
      else
        {
          l = [NSCompoundPredicate orPredicateWithSubpredicates: 
            [NSArray arrayWithObjects: l, r, nil]];
        }
    }
  return l;
}

- (NSPredicate *) parseComparison
{ 
  // there must always be a comparison
  NSComparisonPredicateModifier modifier = NSDirectPredicateModifier;
  NSPredicateOperatorType type = 0;
  unsigned opts = 0;
  NSExpression *left;
  NSExpression *right;
  NSPredicate *p;
  BOOL negate = NO;
  BOOL swap = NO;

  if ([self scanPredicateKeyword: @"ANY"])
    {
      modifier = NSAnyPredicateModifier;
    }
  else if ([self scanPredicateKeyword: @"ALL"])
    {
      modifier = NSAllPredicateModifier;
    }
  else if ([self scanPredicateKeyword: @"NONE"])
    {
      modifier = NSAnyPredicateModifier;
      negate = YES;
    }
  else if ([self scanPredicateKeyword: @"SOME"])
    {
      modifier = NSAllPredicateModifier;
      negate = YES;
    }

  left = [self parseExpression];
  if ([self scanString: @"!=" intoString: NULL]
    || [self scanString: @"<>" intoString: NULL])
    {
      type = NSNotEqualToPredicateOperatorType;
    }
  else if ([self scanString: @"<=" intoString: NULL]
    || [self scanString: @"=<" intoString: NULL])
    {
      type = NSLessThanOrEqualToPredicateOperatorType;
    }
  else if ([self scanString: @">=" intoString: NULL]
    || [self scanString: @"=>" intoString: NULL])
    {
      type = NSGreaterThanOrEqualToPredicateOperatorType;
    }
  else if ([self scanString: @"<" intoString: NULL])
    {
      type = NSLessThanPredicateOperatorType;
    }
  else if ([self scanString: @">" intoString: NULL])
    {
      type = NSGreaterThanPredicateOperatorType;
    }
  else if ([self scanString: @"==" intoString: NULL]
    || [self scanString: @"=" intoString: NULL])
    {
      type = NSEqualToPredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"MATCHES"])
    {
      type = NSMatchesPredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"LIKE"])
    {
      type = NSLikePredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"BEGINSWITH"])
    {
      type = NSBeginsWithPredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"ENDSWITH"])
    {
      type = NSEndsWithPredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"IN"])
    {
      type = NSInPredicateOperatorType;
    }
  else if ([self scanPredicateKeyword: @"CONTAINS"])
    {
      type = NSInPredicateOperatorType;
      swap = YES;
    }
  else if ([self scanPredicateKeyword: @"BETWEEN"])
    {
      // Requires special handling to transfer into AND of
      // two normal comparison predicates
      NSExpression *exp = [self parseSimpleExpression];
      NSArray *a = (NSArray *)[exp constantValue];
      NSNumber *lower, *upper;
      NSExpression *lexp, *uexp;
      NSPredicate *lp, *up;

      if (![a isKindOfClass: [NSArray class]])
        {
          [NSException raise: NSInvalidArgumentException
                       format: @"BETWEEN operator requires array argument"];
        }

      lower = [a objectAtIndex: 0];
      upper = [a objectAtIndex: 1];
      lexp = [NSExpression expressionForConstantValue: lower];
      uexp = [NSExpression expressionForConstantValue: upper];
      lp = [NSComparisonPredicate predicateWithLeftExpression: left 
                                  rightExpression: lexp
                                  modifier: modifier 
                                  type: NSGreaterThanPredicateOperatorType 
                                  options: opts];
      up = [NSComparisonPredicate predicateWithLeftExpression: left 
                                  rightExpression: uexp
                                  modifier: modifier 
                                  type: NSLessThanPredicateOperatorType 
                                  options: opts];
      return [NSCompoundPredicate andPredicateWithSubpredicates: 
                                       [NSArray arrayWithObjects: lp, up, nil]];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException 
                   format: @"Invalid comparison predicate: %@", 
		   [[self string] substringFromIndex: [self scanLocation]]];
    }
 
  if ([self scanString: @"[cd]" intoString: NULL])
    {
      opts = NSCaseInsensitivePredicateOption
        | NSDiacriticInsensitivePredicateOption;
    }
  else if ([self scanString: @"[c]" intoString: NULL])
    {
      opts = NSCaseInsensitivePredicateOption;
    }
  else if ([self scanString: @"[d]" intoString: NULL])
    {
      opts = NSDiacriticInsensitivePredicateOption;
    }

  right = [self parseExpression];
  if (swap == YES)
    {
      NSExpression      *tmp = left;

      left = right;
      right = tmp;
    }

  p = [NSComparisonPredicate predicateWithLeftExpression: left 
                             rightExpression: right
                             modifier: modifier 
                             type: type 
                             options: opts];

  return negate ? [NSCompoundPredicate notPredicateWithSubpredicate: p] : p;
}

- (NSExpression *) parseExpression
{
//  return [self parseAdditionExpression];
  return [self parseBinaryExpression];
}

- (NSExpression *) parseIdentifierExpression
{
  static NSCharacterSet *_identifier;
  NSString      *ident;

  // skip # as prefix if present (reserved words)
  (void)[self scanString: @"#" intoString: NULL];
  if (!_identifier)
    {
      ASSIGN(_identifier, [NSCharacterSet characterSetWithCharactersInString: 
	 @"_$abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"]);
    }

  if (![self scanCharactersFromSet: _identifier intoString: &ident])
    {
      [NSException raise: NSInvalidArgumentException 
                  format: @"Missing identifier: %@", 
                   [[self string] substringFromIndex: [self scanLocation]]];
    }

  return [NSExpression expressionForKeyPath: ident];
}

- (NSExpression *) parseSimpleExpression
{
  unsigned      location;
  double        dbl;

  if ([self scanDouble: &dbl])
    {
      return [NSExpression expressionForConstantValue: 
                               [NSNumber numberWithDouble: dbl]];
    }

  // FIXME: handle integer, hex constants, 0x 0o 0b
  if ([self scanString: @"-" intoString: NULL])
    {
      return [NSExpression expressionForFunction: @"_chs" 
        arguments: [NSArray arrayWithObject: [self parseExpression]]];
    }

  if ([self scanString: @"(" intoString: NULL])
    {
      NSExpression *arg = [self parseExpression];
      
      if (![self scanString: @")" intoString: NULL])
        {
          [NSException raise: NSInvalidArgumentException 
                       format: @"Missing ) in expression"];
        }
      return arg;
    }

  if ([self scanString: @"{" intoString: NULL])
    {
      NSMutableArray *a = [NSMutableArray arrayWithCapacity: 10];

      if ([self scanString: @"}" intoString: NULL])
        {
          // empty
          return [NSExpression expressionForConstantValue: a];
        }
      // first element
      [a addObject: [self parseExpression]];
      while ([self scanString: @"," intoString: NULL])
        {
          // more elements
          [a addObject: [self parseExpression]];
        }

      if (![self scanString: @"}" intoString: NULL])
        {
          [NSException raise: NSInvalidArgumentException 
                      format: @"Missing } in aggregate"];
        }
      return [NSExpression expressionForConstantValue: a];
    }

  if ([self scanPredicateKeyword: @"NULL"]
    || [self scanPredicateKeyword: @"NIL"])
    {
      return [NSExpression expressionForConstantValue: [NSNull null]];
    }
  if ([self scanPredicateKeyword: @"TRUE"]
    || [self scanPredicateKeyword: @"YES"])
    {
      return [NSExpression expressionForConstantValue: 
        [NSNumber numberWithBool: YES]];
    }
  if ([self scanPredicateKeyword: @"FALSE"]
    || [self scanPredicateKeyword: @"NO"])
    {
      return [NSExpression expressionForConstantValue: 
        [NSNumber numberWithBool: NO]];
    }
  if ([self scanPredicateKeyword: @"SELF"])
    {
      return [NSExpression expressionForEvaluatedObject];
    }
  if ([self scanString: @"$" intoString: NULL])
    {
      // variable
      NSExpression *var = [self parseIdentifierExpression];

      if (![var keyPath])
        {
          [NSException raise: NSInvalidArgumentException 
                      format: @"Invalid variable identifier: %@", var];
        }
      return [NSExpression expressionForVariable: [var keyPath]];
    }
	
  location = [self scanLocation];

  if ([self scanString: @"%" intoString: NULL])
    {
      if ([self isAtEnd] == NO)
        {
          unichar   c = [[self string] characterAtIndex: [self scanLocation]];

          switch (c)
            {
              case '%':                         // '%%' is treated as '%'
                location = [self scanLocation];
                break;

              case 'K':
                [self setScanLocation: [self scanLocation] + 1];
                return [NSExpression expressionForKeyPath:
                  [self nextArg]];

              case '@':
              case 'c':
              case 'C':
              case 'd':
              case 'D':
              case 'i':
              case 'o':
              case 'O':
              case 'u':
              case 'U':
              case 'x':
              case 'X':
              case 'e':
              case 'E':
              case 'f':
              case 'g':
              case 'G':
                [self setScanLocation: [self scanLocation] + 1];
                return [NSExpression expressionForConstantValue:
                  [self nextArg]];

              case 'h':
                (void)[self scanString: @"h" intoString: NULL];
                if ([self isAtEnd] == NO)
                  {
                    c = [[self string] characterAtIndex: [self scanLocation]];
                    if (c == 'i' || c == 'u')
                      {
                        [self setScanLocation: [self scanLocation] + 1];
                        return [NSExpression expressionForConstantValue:
                          [self nextArg]];
                      }
                  }
                break;

              case 'q':
                (void)[self scanString: @"q" intoString: NULL];
                if ([self isAtEnd] == NO)
                  {
                    c = [[self string] characterAtIndex: [self scanLocation]];
                    if (c == 'i' || c == 'u' || c == 'x' || c == 'X')
                      {
                        [self setScanLocation: [self scanLocation] + 1];
                        return [NSExpression expressionForConstantValue:
                          [self nextArg]];
                      }
                  }
                break;
            }
        }

      [self setScanLocation: location];
    }
	
  if ([self scanString: @"\"" intoString: NULL])
    {
      NSCharacterSet	*skip = [self charactersToBeSkipped];
      NSString *str = nil;

      [self setCharactersToBeSkipped: nil];
      if ([self scanUpToString: @"\"" intoString: &str] == NO)
	{
	  [self setCharactersToBeSkipped: skip];
          [NSException raise: NSInvalidArgumentException 
                      format: @"Invalid double quoted literal at %u", location];
	}
      [self setCharactersToBeSkipped: skip];
      if (NO == [self scanString: @"\"" intoString: NULL])
        {
          [NSException raise: NSInvalidArgumentException 
            format: @"Unterminated double quoted literal at %u", location];
        }
      return [NSExpression expressionForConstantValue: str];
    }
	
  if ([self scanString: @"'" intoString: NULL])
    {
      NSCharacterSet	*skip = [self charactersToBeSkipped];
      NSString *str = nil;

      [self setCharactersToBeSkipped: nil];
      if ([self scanUpToString: @"'" intoString: &str] == NO)
	{
	  [self setCharactersToBeSkipped: skip];
          [NSException raise: NSInvalidArgumentException 
                      format: @"Invalid single quoted literal at %u", location];
	}
      [self setCharactersToBeSkipped: skip];
      if (NO == [self scanString: @"'" intoString: NULL])
        {
          [NSException raise: NSInvalidArgumentException 
            format: @"Unterminated single quoted literal at %u", location];
        }
      return [NSExpression expressionForConstantValue: str];
    }

  if ([self scanString: @"@" intoString: NULL])
    {
      NSExpression *e = [self parseIdentifierExpression];

      if (![e keyPath])
        {
          [NSException raise: NSInvalidArgumentException 
                      format: @"Invalid keypath identifier: %@", e];
        }

      // prefix with keypath
      return [NSExpression expressionForKeyPath: 
        [NSString stringWithFormat: @"@%@", [e keyPath]]];
    }

  return [self parseIdentifierExpression];
}

- (NSExpression *) parseFunctionalExpression
{
  NSExpression *left = [self parseSimpleExpression];
    
  while (YES)
    {
      if ([self scanString: @"(" intoString: NULL])
        { 
          // function - this parser allows for (max)(a, b, c) to be properly 
          // recognized and even (%K)(a, b, c) if %K evaluates to "max"
          NSMutableArray *args = [NSMutableArray arrayWithCapacity: 5];

          if (![left keyPath])
            {
              [NSException raise: NSInvalidArgumentException 
                          format: @"Invalid function identifier: %@", left];
            }

          if (![self scanString: @")" intoString: NULL])
            {
              // any arguments
              // first argument
              [args addObject: [self parseExpression]];
              while ([self scanString: @"," intoString: NULL])
                {
                  // more arguments
                  [args addObject: [self parseExpression]];
                }

              if (![self scanString: @")" intoString: NULL])
                {
                  [NSException raise: NSInvalidArgumentException 
                              format: @"Missing ) in function arguments"];
                }
            }
          left = [NSExpression expressionForFunction: [left keyPath] 
                                           arguments: args];
        }
      else if ([self scanString: @"[" intoString: NULL])
        {
          // index expression
          if ([self scanPredicateKeyword: @"FIRST"])
            {
              left = [NSExpression expressionForFunction: @"_first" 
                arguments: [NSArray arrayWithObject: [self parseExpression]]];
            }
          else if ([self scanPredicateKeyword: @"LAST"])
            {
              left = [NSExpression expressionForFunction: @"_last" 
                arguments: [NSArray arrayWithObject: [self parseExpression]]];
            }
          else if ([self scanPredicateKeyword: @"SIZE"])
            {
              left = [NSExpression expressionForFunction: @"count" 
                arguments: [NSArray arrayWithObject: [self parseExpression]]];
            }
          else
            {
              left = [NSExpression expressionForFunction: @"_index" 
                arguments: [NSArray arrayWithObjects: left,
                [self parseExpression], nil]];
            }
          if (![self scanString: @"]" intoString: NULL])
            {   
              [NSException raise: NSInvalidArgumentException 
                          format: @"Missing ] in index argument"];
            }
        }
      else if ([self scanString: @"." intoString: NULL])
        {
          // keypath - this parser allows for (a).(b.c)
          // to be properly recognized
          // and even %K.((%K)) if the first %K evaluates to "a" and the 
          // second %K to "b.c"
          NSExpression *right;
		
          if (![left keyPath])
            {
              [NSException raise: NSInvalidArgumentException 
                          format: @"Invalid left keypath: %@", left];
            }
          right = [self parseExpression];
          if (![right keyPath])
            {
              [NSException raise: NSInvalidArgumentException 
                          format: @"Invalid right keypath: %@", left];
            }

          if (evaluatedObjectExpression != left)
            {
              // concatenate
              left = [NSExpression expressionForKeyPath:
                        [NSString stringWithFormat: @"%@.%@",
                                  [left keyPath], [right keyPath]]];
            }
          else
            {
              left = [NSExpression expressionForKeyPath: [right keyPath]];
            }
        }
      else
        {
          // done with suffixes
          return left;
        }
    }
}

- (NSExpression *) parsePowerExpression
{
  NSExpression *left = [self parseFunctionalExpression];
  
  while (YES)
    {
      NSExpression *right;
	
      if ([self scanString: @"**" intoString: NULL])
        {
          right = [self parseFunctionalExpression];
          left = [NSExpression expressionForFunction: @"_pow" 
            arguments: [NSArray arrayWithObjects: left, right, nil]];
        }
      else
        {
          return left;
        }
    }
}

- (NSExpression *) parseMultiplicationExpression
{
  NSExpression *left = [self parsePowerExpression];
	
  while (YES)
    {
      NSExpression *right;
	
      if ([self scanString: @"*" intoString: NULL])
        {
          right = [self parsePowerExpression];
          left = [NSExpression expressionForFunction: @"_mul" 
            arguments: [NSArray arrayWithObjects: left, right, nil]];
        }
      else if ([self scanString: @"/" intoString: NULL])
        {
          right = [self parsePowerExpression];
          left = [NSExpression expressionForFunction: @"_div" 
            arguments: [NSArray arrayWithObjects: left, right, nil]];
        }
      else
        {
          return left;
        }
    }
}

- (NSExpression *) parseAdditionExpression
{
  NSExpression *left = [self parseMultiplicationExpression];
  
  while (YES)
    {
      NSExpression *right;
	
      if ([self scanString: @"+" intoString: NULL])
        {
          right = [self parseMultiplicationExpression];
          left = [NSExpression expressionForFunction: @"_add"
            arguments: [NSArray arrayWithObjects: left, right, nil]];
        }
      else if ([self scanString: @"-" intoString: NULL])
        {
          right = [self parseMultiplicationExpression];
          left = [NSExpression expressionForFunction: @"_sub"
            arguments: [NSArray arrayWithObjects: left, right, nil]];
        }
      else
        {
          return left;
        }
    }
}

- (NSExpression *) parseBinaryExpression
{
  NSExpression *left = [self parseAdditionExpression];
  
  while (YES)
    {
      NSExpression *right;

      if ([self scanString: @":=" intoString: NULL])	// assignment
        {
          // check left to be a variable?
          right = [self parseAdditionExpression];
          // FIXME
        }
      else
        {
          return left;
        }
    }
}
@end


#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)


@implementation GSBlockPredicate

- (instancetype) initWithBlock: (GSBlockPredicateBlock)block
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  _block = (GSBlockPredicateBlock)[(id)block retain];
  return self;
}

- (instancetype) predicateWithSubstitutionVariables: 
  (GS_GENERIC_CLASS(NSDictionary,NSString*,id)*)variables
{
  return [[[GSBoundBlockPredicate alloc] initWithBlock: _block
                                              bindings: variables] autorelease];
}

- (BOOL) evaluateWithObject: (id)object
      substitutionVariables: (GS_GENERIC_CLASS(NSDictionary,
                                               NSString*,id)*)variables
{
  return CALL_BLOCK(_block, object, variables);
}

- (BOOL) evaluateWithObject: (id)object
{
  return [self evaluateWithObject: object
            substitutionVariables: nil];
}

- (void) dealloc
{
  [(id)_block release];
  _block = NULL;
  [super dealloc];
}

- (NSString*) predicateFormat
{
  return [NSString stringWithFormat: @"BLOCKPREDICATE(%p)", (void*)_block];
}
@end

@implementation GSBoundBlockPredicate

- (instancetype) initWithBlock: (GSBlockPredicateBlock)block
                      bindings: (GS_GENERIC_CLASS(NSDictionary,
                                                   NSString*,id)*)bindings
{
  if (nil == (self = [super initWithBlock: block]))
    {
      return nil;
    }
  ASSIGN(_bindings, bindings);
  return self;
}

- (BOOL) evaluateWithObject: (id)object
{
  return [self evaluateWithObject: object
            substitutionVariables: _bindings];
}

- (void) dealloc
{
  DESTROY(_bindings);
  [super dealloc];
}
@end

#endif
