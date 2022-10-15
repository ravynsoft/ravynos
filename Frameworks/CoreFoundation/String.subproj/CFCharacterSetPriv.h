/*	CFCharacterSetPriv.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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

/* For use ONLY by CoreText, contact the Foundation team before using.
*/
CF_EXPORT void _CFCharacterSetCompact(CFMutableCharacterSetRef cset);
CF_EXPORT void _CFCharacterSetFast(CFMutableCharacterSetRef cset);

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFCHARACTERSETPRIV__ */

