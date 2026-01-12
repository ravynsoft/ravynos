/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFCharacterSetPriv.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFCHARACTERSETPRIV__)
#define __COREFOUNDATION_CFCHARACTERSETPRIV__ 1

#include <CoreFoundation/CFCharacterSet.h>

CF_EXTERN_C_BEGIN

/*!
	@function CFCharacterSetIsSurrogateHighCharacter
	Reports whether or not the character is a high surrogate.
	@param character  The character to be checked.
	@result true, if character is a high surrogate, otherwise false.
*/
CF_INLINE Boolean CFCharacterSetIsSurrogateHighCharacter(UniChar character) {
    return ((character >= 0xD800UL) && (character <= 0xDBFFUL) ? true : false);
}

/*!
	@function CFCharacterSetIsSurrogateLowCharacter
	Reports whether or not the character is a low surrogate.
	@param character  The character to be checked.
	@result true, if character is a low surrogate, otherwise false.
*/
CF_INLINE Boolean CFCharacterSetIsSurrogateLowCharacter(UniChar character) {
    return ((character >= 0xDC00UL) && (character <= 0xDFFFUL) ? true : false);
}

/*!
	@function CFCharacterSetGetLongCharacterForSurrogatePair
	Returns the UTF-32 value corresponding to the surrogate pair passed in.
	@param surrogateHigh  The high surrogate character.  If this parameter
			is not a valid high surrogate character, the behavior is undefined.
	@param surrogateLow  The low surrogate character.  If this parameter
			is not a valid low surrogate character, the behavior is undefined.
	@result The UTF-32 value for the surrogate pair.
*/
CF_INLINE UTF32Char CFCharacterSetGetLongCharacterForSurrogatePair(UniChar surrogateHigh, UniChar surrogateLow) {
    return (UTF32Char)(((surrogateHigh - 0xD800UL) << 10) + (surrogateLow - 0xDC00UL) + 0x0010000UL);
}

/* Check to see if the character represented by the surrogate pair surrogateHigh & surrogateLow is in the chraracter set */
CF_EXPORT Boolean CFCharacterSetIsSurrogatePairMember(CFCharacterSetRef theSet, UniChar surrogateHigh, UniChar surrogateLow) ;

/* Keyed-coding support
*/
typedef CF_ENUM(CFIndex, CFCharacterSetKeyedCodingType) {
    kCFCharacterSetKeyedCodingTypeBitmap = 1,
    kCFCharacterSetKeyedCodingTypeBuiltin = 2,
    kCFCharacterSetKeyedCodingTypeRange = 3,
    kCFCharacterSetKeyedCodingTypeString = 4,
    kCFCharacterSetKeyedCodingTypeBuiltinAndBitmap = 5
};

CF_EXPORT CFCharacterSetKeyedCodingType _CFCharacterSetGetKeyedCodingType(CFCharacterSetRef cset);
CF_EXPORT CFCharacterSetPredefinedSet _CFCharacterSetGetKeyedCodingBuiltinType(CFCharacterSetRef cset);
CF_EXPORT CFRange _CFCharacterSetGetKeyedCodingRange(CFCharacterSetRef cset);
CF_EXPORT CFStringRef _CFCharacterSetCreateKeyedCodingString(CFCharacterSetRef cset);
CF_EXPORT bool _CFCharacterSetIsInverted(CFCharacterSetRef cset);
CF_EXPORT void _CFCharacterSetSetIsInverted(CFCharacterSetRef cset, bool flag);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFCHARACTERSETPRIV__ */

