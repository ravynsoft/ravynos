/* 
   NSAttributedString.h

   String class with attributes

   Copyright (C) 1997,1999 Free Software Foundation, Inc.

   Written by: ANOQ of the sun <anoq@vip.cybercity.dk>
   Date: November 1997
   Rewrite by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: April 1999
   
   This file is part of GNUStep-base

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   If you are interested in a warranty or support for this source code,
   contact Scott Christley <scottc@net-community.com> for more information.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

/* Warning -	[-initWithString:attributes:] is the designated initialiser,
 *		but it doesn't provide any way to perform the function of the
 *		[-initWithAttributedString:] initialiser.
 *		In order to work youd this, the string argument of the
 *		designated initialiser has been overloaded such that it
 *		is expected to accept an NSAttributedString here instead of
 *		a string.  If you create an NSAttributedString subclass, you
 *		must make sure that your implementation of the initialiser
 *		copes with either an NSString or an NSAttributedString.
 *		If it receives an NSAttributedString, it should ignore the
 *		attributes argument and use the values from the string.
 */


#ifndef __NSAttributedString_h_GNUSTEP_BASE_INCLUDE
#define __NSAttributedString_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#import	<Foundation/NSObject.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
#import	<Foundation/NSString.h>
#import	<Foundation/NSDictionary.h>
#import	<Foundation/NSArray.h>
#import	<Foundation/NSCoder.h>

GS_EXPORT_CLASS
@interface NSAttributedString : NSObject <NSCoding, NSCopying, NSMutableCopying>
{
}

//Creating an NSAttributedString
- (id) initWithString: (NSString*)aString;
- (id) initWithAttributedString: (NSAttributedString*)attributedString;
- (id) initWithString: (NSString*)aString attributes: (NSDictionary*)attributes;

//Retrieving character information
- (NSUInteger) length;
/** Returns the string content of the receiver.<br />
 * NB. this is actually a proxy to the internal content (which may change)
 * so if you need an immutable instance you should copy the returned value,
 * not just retain it.
 */
- (NSString*) string;					//Primitive method!

//Retrieving attribute information
- (NSDictionary*) attributesAtIndex: (NSUInteger)index
		     effectiveRange: (NSRange*)aRange;	//Primitive method!
- (NSDictionary*) attributesAtIndex: (NSUInteger)index
	      longestEffectiveRange: (NSRange*)aRange
			    inRange: (NSRange)rangeLimit;
- (id) attribute: (NSString*)attributeName
	 atIndex: (NSUInteger)index
  effectiveRange: (NSRange*)aRange;
- (id) attribute: (NSString*)attributeName atIndex: (NSUInteger)index
  longestEffectiveRange: (NSRange*)aRange inRange: (NSRange)rangeLimit;

//Comparing attributed strings
- (BOOL) isEqualToAttributedString: (NSAttributedString*)otherString;

//Extracting a substring
- (NSAttributedString*) attributedSubstringFromRange: (NSRange)aRange;

@end //NSAttributedString


GS_EXPORT_CLASS
@interface NSMutableAttributedString : NSAttributedString
{
}

//Retrieving character information
- (NSMutableString*) mutableString;

//Changing characters
- (void) deleteCharactersInRange: (NSRange)aRange;

//Changing attributes
- (void) setAttributes: (NSDictionary*)attributes
		 range: (NSRange)aRange;		//Primitive method!
- (void) addAttribute: (NSString*)name value: (id)value range: (NSRange)aRange;
- (void) addAttributes: (NSDictionary*)attributes range: (NSRange)aRange;
- (void) removeAttribute: (NSString*)name range: (NSRange)aRange;

//Changing characters and attributes
- (void) appendAttributedString: (NSAttributedString*)attributedString;
- (void) insertAttributedString: (NSAttributedString*)attributedString
			atIndex: (NSUInteger)index;
- (void) replaceCharactersInRange: (NSRange)aRange
	     withAttributedString: (NSAttributedString*)attributedString;
- (void) replaceCharactersInRange: (NSRange)aRange
		       withString: (NSString*)aString;	//Primitive method!
- (void) setAttributedString: (NSAttributedString*)attributedString;

//Grouping changes
- (void) beginEditing;
- (void) endEditing;

@end //NSMutableAttributedString

typedef NSString* NSAttributedStringKey;

#endif /* GS_API_MACOSX */

#if	defined(__cplusplus)
}
#endif

#if     !NO_GNUSTEP && !defined(GNUSTEP_BASE_INTERNAL)
#import <GNUstepBase/NSAttributedString+GNUstepBase.h>
#endif

#endif	/* __NSAttributedString_h_GNUSTEP_BASE_INCLUDE */

