/* Interface for NSComparisonPredicate for GNUStep
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Dr. H. Nikolaus Schaller
   Created: 2005
   
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

#ifndef __NSComparisonPredicate_h_GNUSTEP_BASE_INCLUDE
#define __NSComparisonPredicate_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

#import	<Foundation/NSExpression.h>
#import	<Foundation/NSPredicate.h>

#if	defined(__cplusplus)
extern "C" {
#endif

typedef enum _NSComparisonPredicateModifier
{
  NSDirectPredicateModifier=0,
  NSAllPredicateModifier,
  NSAnyPredicateModifier
} NSComparisonPredicateModifier;

typedef enum _NSComparisonPredicateOptions
{
  NSCaseInsensitivePredicateOption=0x01,
  NSDiacriticInsensitivePredicateOption=0x02
} NSComparisonPredicateOptions;

typedef enum _NSPredicateOperatorType
{
  NSLessThanPredicateOperatorType = 0,
  NSLessThanOrEqualToPredicateOperatorType,
  NSGreaterThanPredicateOperatorType,
  NSGreaterThanOrEqualToPredicateOperatorType,
  NSEqualToPredicateOperatorType,
  NSNotEqualToPredicateOperatorType,
  NSMatchesPredicateOperatorType,
  NSLikePredicateOperatorType,
  NSBeginsWithPredicateOperatorType,
  NSEndsWithPredicateOperatorType,
  NSInPredicateOperatorType,
  NSCustomSelectorPredicateOperatorType
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
  ,
  NSContainsPredicateOperatorType = 99,
  NSBetweenPredicateOperatorType
#endif
} NSPredicateOperatorType;

GS_EXPORT_CLASS
@interface NSComparisonPredicate : NSPredicate
{
#if	GS_EXPOSE(NSComparisonPredicate)
  NSComparisonPredicateModifier	_modifier;
  SEL				_selector;
  NSUInteger			_options;
  NSPredicateOperatorType	_type;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
  @public
  NSExpression			*_left;
  NSExpression			*_right;
}

+ (NSPredicate *) predicateWithLeftExpression: (NSExpression *)left
			      rightExpression: (NSExpression *)right
			       customSelector: (SEL)sel;
+ (NSPredicate *) predicateWithLeftExpression: (NSExpression *)left
  rightExpression: (NSExpression *)right
  modifier: (NSComparisonPredicateModifier)modifier
  type: (NSPredicateOperatorType)type
  options: (NSUInteger) opts;

- (NSComparisonPredicateModifier) comparisonPredicateModifier;
- (SEL) customSelector;
- (NSPredicate *) initWithLeftExpression: (NSExpression *)left
			 rightExpression: (NSExpression *)right
			  customSelector: (SEL)sel;
- (id) initWithLeftExpression: (NSExpression *)left
	      rightExpression: (NSExpression *)right
		     modifier: (NSComparisonPredicateModifier)modifier
			 type: (NSPredicateOperatorType)type
		      options: (NSUInteger) opts;
- (NSExpression *) leftExpression;
- (NSUInteger) options;
- (NSPredicateOperatorType) predicateOperatorType;
- (NSExpression *) rightExpression;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* 100400 */
#endif
