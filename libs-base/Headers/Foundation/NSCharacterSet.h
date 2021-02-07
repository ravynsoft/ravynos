/* Interface for NSCharacterSet for GNUStep
   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: 1995
   
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

#ifndef __NSCharacterSet_h_GNUSTEP_BASE_INCLUDE
#define __NSCharacterSet_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSRange.h>
#import	<Foundation/NSString.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;

/**
 *  Represents a set of unicode characters.  Used by [NSScanner] and [NSString]
 *  for parsing-related methods.
 */
GS_EXPORT_CLASS
@interface NSCharacterSet : NSObject <NSCoding, NSCopying, NSMutableCopying>

/**
 *  Returns a character set containing letters, numbers, and diacritical
 *  marks.  Note that "letters" includes all alphabetic as well as Chinese
 *  characters, etc..
 */
+ (id) alphanumericCharacterSet;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 *  Returns a character set containing letters in the unicode
 *  Titlecase category.
 */
+ (id) capitalizedLetterCharacterSet;
#endif

/**
 * Returns a character set containing control and format characters.
 */
+ (id) controlCharacterSet;

/**
 * Returns a character set containing characters that represent
 * the decimal digits 0 through 9.
 */
+ (id) decimalDigitCharacterSet;

/**
 * Returns a character set containing individual characters that
 * can be represented also by a composed character sequence.
 */
+ (id) decomposableCharacterSet;

/**
 * Returns a character set containing unassigned and explicitly illegal
 * character values.
 */
+ (id) illegalCharacterSet;

/**
 *  Returns a character set containing letters, including all alphabetic as
 *  well as Chinese characters, etc..
 */
+ (id) letterCharacterSet;

/**
 * Returns a character set that contains the lowercase characters.
 * This set does not include caseless characters, only those that
 * have corresponding characters in uppercase and/or titlecase.
 */
+ (id) lowercaseLetterCharacterSet;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Returns a character set containing the newline characters, values 
 * 0x000A and 0x000D and nextline 0x0085 character.
 */
+ (id) newlineCharacterSet;

/**
 * Returns allowed characers for URL fragment component.
 */
+ (id) URLFragmentAllowedCharacterSet;

/**
 * Returns allowed characers for URL host component.
 */
+ (id) URLHostAllowedCharacterSet;

/**
 * Returns allowed characers for URL password component.
 */
+ (id) URLPasswordAllowedCharacterSet;

/**
 * Returns allowed characers for URL path component.
 */
+ (id) URLPathAllowedCharacterSet;

/**
 * Returns allowed characers for URL query component.
 */
+ (id) URLQueryAllowedCharacterSet;

/**
 * Returns allowed characers for URL USER component.
 */
+ (id) URLUserAllowedCharacterSet;
#endif

/**
 *  Returns a character set containing characters for diacritical marks, which
 *  are usually only rendered in conjunction with another character.
 */
+ (id) nonBaseCharacterSet;

/**
 *  Returns a character set containing punctuation marks.
 */
+ (id) punctuationCharacterSet;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 *  Returns a character set containing mathematical symbols, etc..
 */
+ (id) symbolCharacterSet;
#endif

/**
 * Returns a character set that contains the uppercase characters.
 * This set does not include caseless characters, only those that
 * have corresponding characters in lowercase and/or titlecase.
 */
+ (id) uppercaseLetterCharacterSet;

/**
 * Returns a character set that contains the whitespace characters,
 * plus the newline characters, values 0x000A and 0x000D and nextline
 * 0x0085 character.
 */
+ (id) whitespaceAndNewlineCharacterSet;

/**
 * Returns a character set that contains the whitespace characters.
 */
+ (id) whitespaceCharacterSet;

/**
 * Returns a character set containing characters as encoded in the
 * data object (8192 bytes)
 */
+ (id) characterSetWithBitmapRepresentation: (NSData*)data;

/**
 *  Returns set with characters in aString, or empty set for empty string.
 *  Raises an exception if given a nil string.
 */
+ (id) characterSetWithCharactersInString: (NSString*)aString;

/**
 *  Returns set containing unicode index range given by aRange.
 */
+ (id) characterSetWithRange: (NSRange)aRange;

#if OS_API_VERSION(GS_API_OPENSTEP, GS_API_MACOSX)
/**
 *  Initializes from a bitmap (8192 bytes representing 65536 values).<br />
 *  Each bit set in the bitmap represents the fact that a character at
 *  that position in the unicode base plane exists in the characterset.<br />
 *  File must have extension "<code>.bitmap</code>".
 *  To get around this load the file into data yourself and use
 *  [NSCharacterSet -characterSetWithBitmapRepresentation].
 */
+ (id) characterSetWithContentsOfFile: (NSString*)aFile;
#endif

/**
 * Returns a bitmap representation of the receiver's character set
 * (suitable for archiving or writing to a file), in an NSData object.<br />
 */
- (NSData*) bitmapRepresentation;

/**
 * Returns YES if the receiver contains <em>aCharacter</em>, NO if
 * it does not.
 */
- (BOOL) characterIsMember: (unichar)aCharacter;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Returns YES if the receiver contains at least one character in the
 * specified unicode plane.
 */
- (BOOL) hasMemberInPlane: (uint8_t)aPlane;
#endif

/**
 * Returns a character set containing only characters that the
 * receiver does not contain.
 */
- (NSCharacterSet*) invertedSet;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Returns YES is all the characters in aSet are present in the receiver.
 */
- (BOOL) isSupersetOfSet: (NSCharacterSet*)aSet;

/**
 * Returns YES is the specified 32-bit character is present in the receiver.
 */
- (BOOL) longCharacterIsMember: (UTF32Char)aCharacter;
#endif
@end

/**
 *  An [NSCharacterSet] that can be modified.
 */
GS_EXPORT_CLASS
@interface NSMutableCharacterSet : NSCharacterSet

/**
 *  Adds characters specified by unicode indices in aRange to set.
 */
- (void) addCharactersInRange: (NSRange)aRange;

/**
 *  Adds characters in aString to set.
 */
- (void) addCharactersInString: (NSString*)aString;

/**
 *  Set intersection of character sets.
 */
- (void) formIntersectionWithCharacterSet: (NSCharacterSet*)otherSet;

/**
 *  Set union of character sets.
 */
- (void) formUnionWithCharacterSet: (NSCharacterSet*)otherSet;

/**
 *  Drop given range of characters.  No error for characters not currently in
 *  set.
 */
- (void) removeCharactersInRange: (NSRange)aRange;

/**
 *  Drop characters in aString.  No error for characters not currently in
 *  set.
 */
- (void) removeCharactersInString: (NSString*)aString;

/**
 *  Remove all characters currently in set and add all other characters.
 */
- (void) invert;

@end

#if	defined(__cplusplus)
}
#endif

#endif /* __NSCharacterSet_h_GNUSTEP_BASE_INCLUDE*/
