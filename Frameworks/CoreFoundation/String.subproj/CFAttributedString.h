// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

/*	CFAttributedString.h
	Copyright (c) 2004-2019, Apple Inc. All rights reserved.
*/

/*! @header CFAttributedString
Instances of CFAttributedString manage character strings and associated sets of attributes (for example, font and kerning) that apply to individual characters or ranges of characters in the string. CFAttributedString as defined in CoreFoundation provides the basic container functionality, while higher levels provide definitions for standard attributes, their values, and additional behaviors involving these. 

CFAttributedString is not a "subclass" of CFString; that is, it does not respond to CFString function calls. CFAttributedString conceptually contains a CFString to which it applies attributes. This protects users of attributed strings from ambiguities caused by the semantic differences between simple and attributed strings.

Attributes are identified by key/value pairs stored in CFDictionaryRefs. Keys must be CFStrings, while the values are arbitrary CFTypeRefs.
*/

#if !defined(__COREFOUNDATION_CFATTRIBUTEDSTRING__)
#define __COREFOUNDATION_CFATTRIBUTEDSTRING__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN


/* CFAttributedString comes in immutable and mutable flavors.
*/
typedef const struct CF_BRIDGED_TYPE(NSAttributedString) __CFAttributedString *CFAttributedStringRef;
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMutableAttributedString) __CFAttributedString *CFMutableAttributedStringRef;

/*! @function CFAttributedStringGetTypeID
Returns the type identifier of all CFAttributedString instances.
*/
CF_EXPORT CFTypeID CFAttributedStringGetTypeID(void);



/*** CFAttributedString ***/

/*! @function CFAttributedStringCreate
Creates an attributed string with the specified string and attributes (both copied).
*/
CF_EXPORT CFAttributedStringRef CFAttributedStringCreate(CFAllocatorRef alloc, CFStringRef str, CFDictionaryRef attributes);

/*! @function CFAttributedStringCreateWithSubstring
Creates a sub-attributed string from the specified range. It's a programming error for range to specify characters outside the bounds of aStr.
*/
CF_EXPORT CFAttributedStringRef CFAttributedStringCreateWithSubstring(CFAllocatorRef alloc, CFAttributedStringRef aStr, CFRange range);

/*! @function CFAttributedStringCreateCopy
Creates an immutable attributed string copy.
*/
CF_EXPORT CFAttributedStringRef CFAttributedStringCreateCopy(CFAllocatorRef alloc, CFAttributedStringRef aStr);

/*! @function CFAttributedStringGetString
Returns the string for the attributed string. For performance reasons, this will often point at the backing store of the attributed string, and it might change if the attributed string is edited.  However, this is an implementation detail, and definitely not something that should be counted on.
*/
CF_EXPORT CFStringRef CFAttributedStringGetString(CFAttributedStringRef aStr);

/*! @function CFAttributedStringGetLength
Returns the length of the attributed string in characters; same as CFStringGetLength(CFAttributedStringGetString(aStr))
*/
CF_EXPORT CFIndex CFAttributedStringGetLength(CFAttributedStringRef aStr);

/*! @function CFAttributedStringGetAttributes
Returns the attributes at the specified location. If effectiveRange is not NULL, upon return *effectiveRange contains a range over which the exact same set of attributes apply. Note that for performance reasons, the returned effectiveRange is not necessarily the maximal range - for that, use CFAttributedStringGetAttributesAndLongestEffectiveRange().  It's a programming error for loc to specify a location outside the bounds of the attributed string.

Note that the returned attribute dictionary might change in unpredictable ways from under the caller if the attributed string is edited after this call. If you wish to hang on to the dictionary long-term, you should make an actual copy of it rather than just retaining it.  Also, no assumptions should be made about the relationship of the actual CFDictionaryRef returned by this call and the dictionary originally used to set the attributes, other than the fact that the values stored in the dictionary will be identical (that is, ==) to those originally specified.
*/
CF_EXPORT CFDictionaryRef CFAttributedStringGetAttributes(CFAttributedStringRef aStr, CFIndex loc, CFRange *effectiveRange);

/*! @function CFAttributedStringGetAttribute
Returns the value of a single attribute at the specified location. If the specified attribute doesn't exist at the location, returns NULL. If effectiveRange is not NULL, upon return *effectiveRange contains a range over which the exact same attribute value applies. Note that for performance reasons, the returned effectiveRange is not necessarily the maximal range - for that, use CFAttributedStringGetAttributeAndLongestEffectiveRange(). It's a programming error for loc to specify a location outside the bounds of the attributed string.
*/
CF_EXPORT CFTypeRef CFAttributedStringGetAttribute(CFAttributedStringRef aStr, CFIndex loc, CFStringRef attrName, CFRange *effectiveRange);

/*! @function CFAttributedStringGetAttributesAndLongestEffectiveRange
Returns the attributes at the specified location. If longestEffectiveRange is not NULL, upon return *longestEffectiveRange contains the maximal range within inRange over which the exact same set of attributes apply. The returned range is clipped to inRange. It's a programming error for loc or inRange to specify locations outside the bounds of the attributed string.
*/
CF_EXPORT CFDictionaryRef CFAttributedStringGetAttributesAndLongestEffectiveRange(CFAttributedStringRef aStr, CFIndex loc, CFRange inRange, CFRange *longestEffectiveRange);

/*! @function CFAttributedStringGetAttributeAndLongestEffectiveRange
Returns the value of a single attribute at the specified location. If longestEffectiveRange is not NULL, upon return *longestEffectiveRange contains the maximal range within inRange over which the exact same attribute value applies. The returned range is clipped to inRange. It's a programming error for loc or inRange to specify locations outside the bounds of the attributed string.
*/
CF_EXPORT CFTypeRef CFAttributedStringGetAttributeAndLongestEffectiveRange(CFAttributedStringRef aStr, CFIndex loc, CFStringRef attrName, CFRange inRange, CFRange *longestEffectiveRange);



/*** CFMutableAttributedString ***/

/*! @function CFAttributedStringCreateMutableCopy
Creates a mutable attributed string copy. maxLength, if not 0, is a hard bound on the length of the attributed string; exceeding this size limit during any editing operation is a programming error. If 0, there is no limit on the length.
*/
CF_EXPORT CFMutableAttributedStringRef CFAttributedStringCreateMutableCopy(CFAllocatorRef alloc, CFIndex maxLength, CFAttributedStringRef aStr);

/*! @function CFAttributedStringCreateMutable
Creates a mutable empty attributed string. maxLength, if not 0, is a hard bound on the length of the attributed string; exceeding this size limit during any editing operation is a programming error. If 0, there is no limit on the length.
*/
CF_EXPORT CFMutableAttributedStringRef CFAttributedStringCreateMutable(CFAllocatorRef alloc, CFIndex maxLength);

/*! @function CFAttributedStringReplaceString
Modifies the string for the attributed string, much like CFStringReplace().  It's an error for range to specify characters outside the bounds of aStr. 

(Note: This function is a convenience on CFAttributedStringGetMutableString(); however, until CFAttributedStringGetMutableString() is implemented, it remains the only way to edit the string of the attributed string.)
*/
CF_EXPORT void CFAttributedStringReplaceString(CFMutableAttributedStringRef aStr, CFRange range, CFStringRef replacement);

/*! @function CFAttributedStringGetMutableString
Gets the string for the attributed string as a mutable string, allowing editing the character contents of the string as if it were an CFMutableString. Attributes corresponding to the edited range are appropriately modified. If, as a result of the edit, new characters are introduced into the string, they inherit the attributes of the first replaced character from range. If no existing characters are replaced by the edit, the new characters inherit the attributes of the character preceding range if it has any, otherwise of the character following range. If the initial string is empty, the attributes for the new characters are also empty.

(Note: This function is not yet implemented and will return NULL except for toll-free bridged instances.)
*/
CF_EXPORT CFMutableStringRef CFAttributedStringGetMutableString(CFMutableAttributedStringRef aStr);

/*! @function CFAttributedStringSetAttributes
Sets the value of multiple attributes over the specified range, which should be valid. If clearOtherAttributes is false, existing attributes (which aren't being replaced) are left alone; otherwise they are cleared. The dictionary should be setup for "usual" CF type usage --- CFString keys, and arbitrary CFType values. Note that after this call, further mutations to the replacement dictionary argument by the caller will not affect the contents of the attributed string.
*/
CF_EXPORT void CFAttributedStringSetAttributes(CFMutableAttributedStringRef aStr, CFRange range, CFDictionaryRef replacement, Boolean clearOtherAttributes);

/*! @function CFAttributedStringSetAttribute
Sets the value of a single attribute over the specified range, which should be valid. value should not be NULL. 
*/
CF_EXPORT void CFAttributedStringSetAttribute(CFMutableAttributedStringRef aStr, CFRange range, CFStringRef attrName, CFTypeRef value);

/*! @function CFAttributedStringRemoveAttribute
Removes the value of a single attribute over the specified range, which should be valid. It's OK for the attribute not the exist over the specified range.
*/
CF_EXPORT void CFAttributedStringRemoveAttribute(CFMutableAttributedStringRef aStr, CFRange range, CFStringRef attrName);

/*! @function CFAttributedStringReplaceAttributedString
Replaces the attributed substring over the specified range with the attributed string specified in replacement. range should be valid. To delete a range of the attributed string, call CFAttributedStringReplaceString() with empty string and specified range. 
*/
CF_EXPORT void CFAttributedStringReplaceAttributedString(CFMutableAttributedStringRef aStr, CFRange range, CFAttributedStringRef replacement);

/*! @function CFAttributedStringBeginEditing
In cases where attributed string might do a bunch of work to assure self-consistency, CFAttributedStringBeginEditing/CFAttributedStringEndEditing allow disabling that to allow deferring and coalescing any work. It's a good idea to call these around a set of related mutation calls which don't require the string to be in consistent state in between. These calls can be nested. 
*/
CF_EXPORT void CFAttributedStringBeginEditing(CFMutableAttributedStringRef aStr);

/*! @function CFAttributedStringEndEditing
In cases where attributed string might do a bunch of work to assure self-consistency, CFAttributedStringBeginEditing/CFAttributedStringEndEditing allow disabling that to allow deferring and coalescing any work. It's a good idea to call these around a set of related mutation calls which don't require the string to be in consistent state in between. These calls can be nested. 
*/
CF_EXPORT void CFAttributedStringEndEditing(CFMutableAttributedStringRef aStr);


CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFATTRIBUTEDSTRING__ */

