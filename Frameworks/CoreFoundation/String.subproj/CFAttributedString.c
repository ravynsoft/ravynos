/*	    CFAttributedString.c
	Copyright (c) 2004-2019, Apple Inc. All rights reserved.
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
        Original Author: Ali Ozer
	    Responsibility: Ali Ozer
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFAttributedString.h>
#include "CFRunArray.h"
#include <CoreFoundation/ForFoundationOnly.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

#if (TARGET_OS_MAC || TARGET_OS_WIN32) && DEPLOYMENT_RUNTIME_OBJC
#include <Foundation/NSAttributedString.h>
@interface NSAttributedString (NSPrivate)
- (NSAttributedString *)_createAttributedSubstringWithRange:(NSRange)range NS_RETURNS_RETAINED;
@end
#endif

struct __CFAttributedString {
    CFRuntimeBase base;
    CFStringRef string;
    CFRunArrayRef attributeArray;
};

/* Mutability is determined by a bit in the CF base. Mutable if bit 0 is 0.  So by default freshly created attributed strings are mutable.  Don't change mutability once the object has been created and initialized!
*/
CF_INLINE Boolean __CFAttributedStringIsMutable(CFAttributedStringRef attrStr) {
    // The first bit is 'immutable', so flip the sense
    return !__CFRuntimeGetFlag(attrStr, 0);
}

CF_INLINE void __CFAttributedStringSetMutable(CFAttributedStringRef attrStr, Boolean flag) {
    // The first bit is 'immutable', so flip the sense
    __CFRuntimeSetFlag(attrStr, 0, !flag);
}


/* Assertions
*/
#define __CFAssertIsAttributedString(cf) __CFGenericValidateType(cf, CFAttributedStringGetTypeID())
#define __CFAssertIndexIsInBounds(cf, idx) CFAssert3((idx) >= 0 && (idx) < CFAttributedStringGetLength(cf), __kCFLogAssertion, "%s(): index %ld out of bounds (length %ld)", __PRETTY_FUNCTION__, idx, CFAttributedStringGetLength(cf))
#define __CFAssertRangeIsInBounds(cf, idx, count) CFAssert4((idx) >= 0 && (idx + count) <= CFAttributedStringGetLength(cf), __kCFLogAssertion, "%s(): range %ld,%ld out of bounds (length %ld)", __PRETTY_FUNCTION__, idx, count, CFAttributedStringGetLength(cf))
#define __CFAssertRangeIsWithinLength(len, idx, count) CFAssert4((idx) >= 0 && (idx + count) <= len, __kCFLogAssertion, "%s(): range %ld,%ld out of bounds (length %ld)", __PRETTY_FUNCTION__, idx, count, len)
#define __CFAssertIsAttributedStringAndMutable(cf) CFAssert1((CFGetTypeID(cf) == CFAttributedStringGetTypeID()) && __CFAttributedStringIsMutable(cf), __kCFLogAssertion, "%s(): argument not a CFMutableAttributedString", __PRETTY_FUNCTION__)


/*** "Polymorphic" functions ***/

static Boolean __CFAttributedStringEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFAttributedStringRef attrStr = (CFAttributedStringRef)cf1;
    CFAttributedStringRef otherAttrStr = (CFAttributedStringRef)cf2;

    if (!CFEqual(attrStr->string, CFAttributedStringGetString(otherAttrStr))) return false;
        
    CFIndex len = CFStringGetLength(attrStr->string);
    CFIndex curLoc = 0;
    CFRange myRange = {0, 0};
    CFRange otherRange = {0, 0};
    CFDictionaryRef myAttrs = NULL;
    CFDictionaryRef otherAttrs = NULL;
    
    while (curLoc < len) {	
	if (curLoc >= myRange.length + myRange.location) myAttrs = CFAttributedStringGetAttributes(attrStr, curLoc, &myRange);
	if (curLoc >= otherRange.length + otherRange.location) otherAttrs = CFAttributedStringGetAttributes(otherAttrStr, curLoc, &otherRange);
	if (!CFEqual(myAttrs, otherAttrs)) return false;
	curLoc = otherRange.length + otherRange.location;
	if (myRange.length + myRange.location < curLoc) curLoc = myRange.length + myRange.location;
    }        
    return true;
}

static CFHashCode __CFAttributedStringHash(CFTypeRef cf) {
    CFAttributedStringRef attrStr = (CFAttributedStringRef)cf;
    return CFHash(attrStr->string);
}

// Create a stack or malloc'ed array of CFTypeRef

#define localArrayStackSize 256

#define createLocalArray(array, count) \
    CFTypeRef array ## Buf[localArrayStackSize];  \
    if (!(count < LONG_MAX / sizeof(CFTypeRef))) { CRSetCrashLogMessage("Ridiculous size count"); HALT; } \
    CFTypeRef *array = (count <= localArrayStackSize) ? (array ## Buf) : malloc(count * sizeof(CFTypeRef));

#define freeLocalArray(array) \
    if (array != array ## Buf) free(array);

static CFStringRef __CFAttributedStringCopyDescription(CFTypeRef cf) {
    CFMutableStringRef result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFAttributedStringRef attrStr = (CFAttributedStringRef)cf;
    CFIndex len = CFStringGetLength(attrStr->string);
    CFRange range = {0, 0};
    while (range.location < len) {
	CFDictionaryRef attrs = CFAttributedStringGetAttributes(attrStr, range.location, &range);
	CFStringRef substring = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, attrStr->string, range);
	CFStringAppend(result, substring);
	CFRelease(substring);
	CFStringAppendFormat(result, NULL, CFSTR(" %p {"), attrs);
        CFIndex cnt = CFDictionaryGetCount(attrs);
        createLocalArray(keys, cnt);
        createLocalArray(values, cnt);
	CFDictionaryGetKeysAndValues(attrs, keys, values);
	while (cnt--) CFStringAppendFormat(result, NULL, CFSTR("%@=%@%s"), keys[cnt], values[cnt], ((cnt == 0) ? "" : ", "));
	CFStringAppendFormat(result, NULL, CFSTR("} Len %ld\n"), (long)range.length);
	range.location += range.length;
        freeLocalArray(keys);
        freeLocalArray(values);
    }
    return result;
}

static void __CFAttributedStringDeallocate(CFTypeRef cf) {
    CFAttributedStringRef attrStr = (CFAttributedStringRef)cf;
    CFRelease(attrStr->string);
    CFRelease(attrStr->attributeArray);
}

const CFRuntimeClass __CFAttributedStringClass = {
    0,
    "CFAttributedString",
    NULL,	// init
    NULL,	// copy
    __CFAttributedStringDeallocate,
    __CFAttributedStringEqual,
    __CFAttributedStringHash,
    NULL,	// 
    __CFAttributedStringCopyDescription
};

CFTypeID CFAttributedStringGetTypeID(void) {
    return _kCFRuntimeIDCFAttributedString;
}


/*** Creation and Copy routines ***/

/* Create an "internal" attributes dictionary
*/
static CFMutableDictionaryRef __CFAttributedStringCreateAttributesDictionary(CFAllocatorRef alloc, CFDictionaryRef attrs) {
    if (attrs) {
        return CFDictionaryCreateMutableCopy(alloc, 0, attrs);
    } else {
        return CFDictionaryCreateMutable(alloc, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
}

/* Does no argument checking; doesn't shortcut to doing a copy if the range is the whole string. (This is used by the other functions to create copies). 
*/
CFMutableAttributedStringRef __CFAttributedStringCreateMutableWithSubstring(CFAllocatorRef alloc, CFAttributedStringRef attrStr, CFRange range) {
    CFMutableAttributedStringRef newAttrStr = CFAttributedStringCreateMutable(alloc, 0);

    // Initialize the string (!!! this should be done more efficiently!) 
    CFStringRef str = CFStringCreateWithSubstring(alloc, attrStr->string, range); 
    CFAttributedStringReplaceString(newAttrStr, CFRangeMake(0, 0), str);
    CFRelease(str);
    
    CFIndex curLoc = range.location;
    CFIndex endLoc = range.location + range.length;
  
    while (curLoc < endLoc) {
	CFRange effectiveRange;
	CFDictionaryRef attrs = CFAttributedStringGetAttributes(attrStr, curLoc, &effectiveRange);
	if (curLoc != effectiveRange.location) effectiveRange.length -= (curLoc - effectiveRange.location);
	if (curLoc + effectiveRange.length > endLoc) effectiveRange.length = endLoc - curLoc;
        CFAttributedStringSetAttributes(newAttrStr, CFRangeMake(curLoc - range.location, effectiveRange.length), attrs, true);
	curLoc += effectiveRange.length;
    }    

    return newAttrStr;
}


CFAttributedStringRef CFAttributedStringCreate(CFAllocatorRef alloc, CFStringRef str, CFDictionaryRef attributes) {
    CFIndex size = sizeof(struct __CFAttributedString) - sizeof(CFRuntimeBase);
    struct __CFAttributedString *newAttrStr = (struct __CFAttributedString *)_CFRuntimeCreateInstance(alloc, CFAttributedStringGetTypeID(), size, NULL);

    if (newAttrStr == NULL) return NULL;

    newAttrStr->string = CFStringCreateCopy(alloc, str);
    newAttrStr->attributeArray = CFRunArrayCreate(alloc);
    
    CFIndex len = CFStringGetLength(newAttrStr->string);
    if (len) {
	CFMutableDictionaryRef attrs = __CFAttributedStringCreateAttributesDictionary(alloc, attributes);
	CFRunArrayInsert(newAttrStr->attributeArray, CFRangeMake(0, len), attrs);
	CFRelease(attrs);
    }

    __CFAttributedStringSetMutable(newAttrStr, false);     // Make it immutable
    
    return (CFAttributedStringRef)newAttrStr;
}

CFAttributedStringRef CFAttributedStringCreateWithSubstring(CFAllocatorRef alloc, CFAttributedStringRef attrStr, CFRange range) {
//  CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFAttributedStringRef, (NSAttributedString *)attrStr, _createAttributedSubstringWithRange:NSMakeRange(range.location, range.length));

    __CFAssertIsAttributedString(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);

    CFIndex len = CFAttributedStringGetLength(attrStr);
    
    // !!! Bounds check on range here

    if (range.location == 0 && range.length == len) {
        return CFAttributedStringCreateCopy(alloc, attrStr);  /* This in turn might take a retain shortcut */
    } else {
        struct __CFAttributedString *newAttrStr = __CFAttributedStringCreateMutableWithSubstring(alloc, attrStr, range);
        __CFAttributedStringSetMutable(newAttrStr, false);
        return (CFAttributedStringRef)newAttrStr;
    }
}

/* CFAttributedStringCreateCopy() will do a refcount bump if the argument is immutable and has the same allocator
*/
CFAttributedStringRef CFAttributedStringCreateCopy(CFAllocatorRef alloc, CFAttributedStringRef attrStr) {
//  CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFAttributedStringRef, (NSAttributedString *)attrStr, copy);
    
    __CFAssertIsAttributedString(attrStr);
    if (!__CFAttributedStringIsMutable(attrStr) &&                                      // If the string is not mutable
        ((alloc ? alloc : __CFGetDefaultAllocator()) == __CFGetAllocator(attrStr))) {   //  and it has the same allocator as the one we're using
	CFRetain(attrStr);                                                              // Then just retain instead of making a true copy
	return attrStr;
    } else {
        CFIndex len = CFAttributedStringGetLength(attrStr);
        CFMutableAttributedStringRef newAttrStr =__CFAttributedStringCreateMutableWithSubstring(alloc, attrStr, CFRangeMake(0, len));
        __CFAttributedStringSetMutable(newAttrStr, false);     // Make it immutable
        return (CFAttributedStringRef)newAttrStr;
    }
}

/* CFAttributedStringCreateWithRuns() creates an attributed string from the specified string and a list of sparse attribute dictionaries.  See CFAttributedStringPriv.h for info.
*/
CFAttributedStringRef _CFAttributedStringCreateWithRuns(CFAllocatorRef alloc, CFStringRef str, const CFDictionaryRef *attrDictionaries, const CFRange *runRanges, CFIndex numRuns) {
    CFIndex size = sizeof(struct __CFAttributedString) - sizeof(CFRuntimeBase);
    struct __CFAttributedString *newAttrStr = (struct __CFAttributedString *)_CFRuntimeCreateInstance(alloc, CFAttributedStringGetTypeID(), size, NULL);

    if (newAttrStr == NULL) return NULL;

    newAttrStr->string = CFStringCreateCopy(alloc, str);
    newAttrStr->attributeArray = CFRunArrayCreate(alloc);

    // First fill attributed string with empty attribute dictionary
    CFIndex len = CFStringGetLength(newAttrStr->string);
    if (len) {
	CFMutableDictionaryRef attrs = __CFAttributedStringCreateAttributesDictionary(alloc, NULL);
	CFRunArrayInsert(newAttrStr->attributeArray, CFRangeMake(0, len), attrs);
	CFRelease(attrs);
    }

    // Now set the various ranges    
    CFIndex cnt;
    for (cnt = 0; cnt < numRuns; cnt++) {
	CFMutableDictionaryRef attrs = __CFAttributedStringCreateAttributesDictionary(alloc, attrDictionaries[cnt]);
	__CFAssertRangeIsWithinLength(len, runRanges[cnt].location, runRanges[cnt].length);
	CFRunArrayReplace(newAttrStr->attributeArray, runRanges[cnt], attrs, runRanges[cnt].length);
	CFRelease(attrs);
    }

    __CFAttributedStringSetMutable(newAttrStr, false);     // Make it immutable
    
    return (CFAttributedStringRef)newAttrStr;
}


CFMutableAttributedStringRef CFAttributedStringCreateMutable(CFAllocatorRef alloc, CFIndex maxLength) {
    CFIndex size = sizeof(struct __CFAttributedString) - sizeof(CFRuntimeBase);
    struct __CFAttributedString *newAttrStr = (CFMutableAttributedStringRef)_CFRuntimeCreateInstance(alloc, CFAttributedStringGetTypeID(), size, NULL);

    if (newAttrStr == NULL) return NULL;

    newAttrStr->string = CFStringCreateMutable(alloc, maxLength);
    newAttrStr->attributeArray = CFRunArrayCreate(alloc);

    return (CFMutableAttributedStringRef)newAttrStr;

}

CFMutableAttributedStringRef CFAttributedStringCreateMutableCopy(CFAllocatorRef alloc, CFIndex maxLength, CFAttributedStringRef attrStr) {
//  CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFMutableAttributedStringRef, (NSAttributedString *)attrStr, mutableCopy);
    
    __CFAssertIsAttributedString(attrStr);

    // !!! Need to deal with maxLength
    return __CFAttributedStringCreateMutableWithSubstring(alloc, attrStr, CFRangeMake(0, CFStringGetLength(attrStr->string)));
}



/*** CFAttributedString functionality ***/

CFStringRef CFAttributedStringGetString(CFAttributedStringRef attrStr) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFStringRef, (NSAttributedString *)attrStr, string);
    __CFAssertIsAttributedString(attrStr);
    
    return attrStr->string;
}

CFIndex CFAttributedStringGetLength(CFAttributedStringRef attrStr) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFIndex, (NSAttributedString *)attrStr, length);
    __CFAssertIsAttributedString(attrStr);
    
    return CFStringGetLength(attrStr->string);
}

CFDictionaryRef CFAttributedStringGetAttributes(CFAttributedStringRef attrStr, CFIndex loc, CFRange *effectiveRange) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFDictionaryRef, (NSAttributedString *)attrStr, attributesAtIndex:(NSUInteger)loc effectiveRange:(NSRange *)effectiveRange);
    __CFAssertIsAttributedString(attrStr);
    __CFAssertIndexIsInBounds(attrStr, loc);
    
    return (CFDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, loc, effectiveRange, NULL);
}

CFTypeRef CFAttributedStringGetAttribute(CFAttributedStringRef attrStr, CFIndex loc, CFStringRef attrName, CFRange *effectiveRange) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFTypeRef, (NSAttributedString *)attrStr, attribute:(NSString *)attrName atIndex:(NSUInteger)loc effectiveRange:(NSRange *)effectiveRange);
    __CFAssertIsAttributedString(attrStr);
    __CFAssertIndexIsInBounds(attrStr, loc);

    CFDictionaryRef attrs = (CFDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, loc, effectiveRange, NULL);
    return CFDictionaryGetValue(attrs, attrName);
}

CFDictionaryRef CFAttributedStringGetAttributesAndLongestEffectiveRange(CFAttributedStringRef attrStr, CFIndex location, CFRange rangeLimit, CFRange *longestEffectiveRange) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFDictionaryRef, (NSAttributedString *)attrStr, attributesAtIndex:(NSUInteger)location longestEffectiveRange:(NSRange *)longestEffectiveRange inRange:NSMakeRange(rangeLimit.location, rangeLimit.length));
    __CFAssertIsAttributedString(attrStr);
    __CFAssertRangeIsInBounds(attrStr, rangeLimit.location, rangeLimit.length);
    __CFAssertIndexIsInBounds(attrStr, location);

    CFDictionaryRef attrs = CFAttributedStringGetAttributes(attrStr, location, longestEffectiveRange);
    
    if (longestEffectiveRange) {    // No need to do any of this if the range isn't desired
        CFIndex leftEnd, rightEnd;
	CFDictionaryRef other;
        CFRange tmpRange;

        // First go right and set rightEnd pointing past the end
	tmpRange = *longestEffectiveRange;
        do {
            if ((rightEnd = tmpRange.location + tmpRange.length) >= rangeLimit.location + rangeLimit.length) break;
            other = CFAttributedStringGetAttributes(attrStr, rightEnd, &tmpRange);
        } while (CFEqual(other, attrs));

	// Now go back and set leftEnd
	tmpRange = *longestEffectiveRange;
        do {
	    if ((leftEnd = tmpRange.location) <= rangeLimit.location) break;
            other = CFAttributedStringGetAttributes(attrStr, leftEnd - 1, &tmpRange);
        } while (CFEqual(other, attrs));

        // This basically intersects the rangeLimit with leftEnd,rightEnd
        longestEffectiveRange->location = (leftEnd < rangeLimit.location) ? rangeLimit.location : leftEnd;
        CFIndex rangeEnd = rangeLimit.location + rangeLimit.length;
        longestEffectiveRange->length = ((rangeEnd < rightEnd) ? rangeEnd : rightEnd) - longestEffectiveRange->location;
    }
    return attrs;
}

CFTypeRef CFAttributedStringGetAttributeAndLongestEffectiveRange(CFAttributedStringRef attrStr, CFIndex location, CFStringRef attrName, CFRange rangeLimit, CFRange *longestEffectiveRange) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFTypeRef, (NSAttributedString *)attrStr, attribute:(NSString *)attrName atIndex:(NSUInteger)location longestEffectiveRange:(NSRange *)longestEffectiveRange inRange: NSMakeRange(rangeLimit.location, rangeLimit.length));
    __CFAssertIsAttributedString(attrStr);
    __CFAssertRangeIsInBounds(attrStr, rangeLimit.location, rangeLimit.length);
    __CFAssertIndexIsInBounds(attrStr, location);

    CFTypeRef attr = CFAttributedStringGetAttribute(attrStr, location, attrName, longestEffectiveRange);

    if (longestEffectiveRange) {    // No need to do any of this if the range isn't desired
        CFIndex leftEnd, rightEnd;
	CFTypeRef other;
        CFRange tmpRange;

        // First go right and set rightEnd pointing past the end
	tmpRange = *longestEffectiveRange;
        do {
            if ((rightEnd = tmpRange.location + tmpRange.length) >= rangeLimit.location + rangeLimit.length) break;
            other = CFAttributedStringGetAttribute(attrStr, rightEnd, attrName, &tmpRange);
        } while ((other == attr) || (other && attr && CFEqual(other, attr)));  // The first check is to catch nil as well

	// Now go back and set leftEnd
	tmpRange = *longestEffectiveRange;
        do {
	    if ((leftEnd = tmpRange.location) <= rangeLimit.location) break;
            other = CFAttributedStringGetAttribute(attrStr, leftEnd - 1, attrName, &tmpRange);
        } while ((other == attr) || (other && attr && CFEqual(other, attr)));  // The first check is to catch nil as well

        // This basically intersects the rangeLimit with leftEnd,rightEnd
        longestEffectiveRange->location = (leftEnd < rangeLimit.location) ? rangeLimit.location : leftEnd;
        CFIndex rangeEnd = rangeLimit.location + rangeLimit.length;
        longestEffectiveRange->length = ((rangeEnd < rightEnd) ? rangeEnd : rightEnd) - longestEffectiveRange->location;
    }
    return attr;
}


/* The next two functions are SPI which allow return bulk information about attributes.  See CFAttributedStringPriv.h for info.
*/
CFIndex _CFAttributedStringGetNumberOfRuns(CFAttributedStringRef attrStr, Boolean includeEmpty) {
    CFIndex numRuns = 0;
    CFRange remainingRange = CFRangeMake(0, CFAttributedStringGetLength(attrStr));
    while (remainingRange.length > 0) {
	CFRange thisRange;
	CFDictionaryRef dict = CFAttributedStringGetAttributesAndLongestEffectiveRange(attrStr, remainingRange.location, remainingRange, &thisRange);
	if (includeEmpty || (CFDictionaryGetCount(dict) > 0)) numRuns++;
	remainingRange.length -= thisRange.length;
	remainingRange.location += thisRange.length;
    }
    return numRuns;
}

void _CFAttributedStringGetRuns(CFAttributedStringRef attrStr, Boolean includeEmpty, CFDictionaryRef *attrDictionaries, CFRange *runRanges) {
    CFRange remainingRange = CFRangeMake(0, CFAttributedStringGetLength(attrStr));
    while (remainingRange.length > 0) {
	CFRange thisRange;
	CFDictionaryRef dict = CFAttributedStringGetAttributesAndLongestEffectiveRange(attrStr, remainingRange.location, remainingRange, &thisRange);
	if (includeEmpty || (CFDictionaryGetCount(dict) > 0)) {
	    if (attrDictionaries) *attrDictionaries++ = dict;
	    if (runRanges) *runRanges++ = thisRange;
	}
	remainingRange.length -= thisRange.length;
	remainingRange.location += thisRange.length;
    }
}


/*** CFMutableAttributedString functionality ***/

static void __CFDictionaryAddMultiple(CFMutableDictionaryRef dict, CFTypeRef *keys, CFTypeRef *values, CFIndex numAdditionalItems) {
    CFIndex cnt;
    for (cnt = 0; cnt < numAdditionalItems; cnt++) CFDictionarySetValue(dict, keys[cnt], values[cnt]);
}

/* ??? This is not properly implemented at this point! We need to return a proxy
*/
CFMutableStringRef CFAttributedStringGetMutableString(CFMutableAttributedStringRef attrStr) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), CFMutableStringRef, (NSMutableAttributedString *)attrStr, mutableString);
    return NULL;  /* (CFMutableStringRef)(attrStr->string); */
}

void CFAttributedStringReplaceString(CFMutableAttributedStringRef attrStr, CFRange range, CFStringRef replacement) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, replaceCharactersInRange:NSMakeRange(range.location, range.length) withString:(NSString *)replacement);
    __CFAssertIsAttributedStringAndMutable(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);
    
    CFIndex replacementLen = CFStringGetLength(replacement);
    CFMutableDictionaryRef attributesToBeUsed = NULL;
    if (replacementLen > 0) {
        // By default extend replaced attributes, or take them from the previous character
        if (range.length > 0) {
            attributesToBeUsed = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, range.location, NULL, NULL);
	    CFRetain(attributesToBeUsed);
        } else if (range.location > 0) {
            attributesToBeUsed = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, range.location - 1, NULL, NULL);
	    CFRetain(attributesToBeUsed);
        } else if (CFStringGetLength(attrStr->string) > 0) {
            attributesToBeUsed = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, 0, NULL, NULL);
	    CFRetain(attributesToBeUsed);
       } else {
            attributesToBeUsed = __CFAttributedStringCreateAttributesDictionary(CFGetAllocator(attrStr), NULL);
        }
    }
    if (range.length > 0) {
        CFRunArrayReplace(attrStr->attributeArray, range, attributesToBeUsed, replacementLen);
    } else if (replacementLen) {
        CFRunArrayInsert(attrStr->attributeArray, CFRangeMake(range.location, replacementLen), attributesToBeUsed);
    }
    if (attributesToBeUsed) CFRelease(attributesToBeUsed);
    CFStringReplace((CFMutableStringRef)(attrStr->string), range, replacement);
    // !!! [self edited:NSAttributedStringEditedCharacters range:range changeInLength:replacementLen - range.length];
}

void CFAttributedStringSetAttributes(CFMutableAttributedStringRef attrStr, CFRange range, CFDictionaryRef replacementAttrs, Boolean clearOtherAttributes) {
    if (clearOtherAttributes) {
        CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, setAttributes:(NSDictionary *)replacementAttrs range: NSMakeRange(range.location, range.length));
    } else {
        CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, addAttributes:(NSDictionary *)replacementAttrs range: NSMakeRange(range.location, range.length));
    }
    
    __CFAssertIsAttributedStringAndMutable(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);
    
    if (clearOtherAttributes) { // Just blast all attribute dictionaries in the specified range
        if (range.length) {
            CFMutableDictionaryRef attrs = __CFAttributedStringCreateAttributesDictionary(CFGetAllocator(attrStr), replacementAttrs);
            CFRunArrayReplace(attrStr->attributeArray, range, attrs, range.length);
            CFRelease(attrs);
            // !!! [self edited:NSAttributedStringEditedAttributes range:range changeInLength:0];
        }
    } else {    // More difficult --- set specified keys and values on the existing dictionaries in the specified range
        CFIndex numAdditionalItems = CFDictionaryGetCount(replacementAttrs);
        if (numAdditionalItems) {
            // Extract the new keys and values so we don't do it over and over for each range
            createLocalArray(additionalKeys, numAdditionalItems);
            createLocalArray(additionalValues, numAdditionalItems);
            CFDictionaryGetKeysAndValues(replacementAttrs, additionalKeys, additionalValues);
            
            // CFAttributedStringBeginEditing(attrStr);
            while (range.length) {
                CFRange effectiveRange;
                CFMutableDictionaryRef attrs = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, range.location, &effectiveRange, NULL);
#if !DEPLOYMENT_RUNTIME_SWIFT // We can't make assumptions of the retain count in Swift.
                if (effectiveRange.location == range.location && effectiveRange.length <= range.length && CFGetRetainCount(attrs) == 1) {
                    // !!! Retain count check cheesy; need to keep our own reference count
                    // The attributes dictionary can just be modified in place
                    // Here we are assuming that CFRunArray is not under-reporting the effectiveRange for a given item!
                    __CFDictionaryAddMultiple(attrs, additionalKeys, additionalValues, numAdditionalItems);
                } else
#endif // !DEPLOYMENT_RUNTIME_SWIFT
                {
                    // Intersect effectiveRange and range
                    if (effectiveRange.location < range.location) {
                        effectiveRange.length -= (range.location - effectiveRange.location);
                        effectiveRange.location = range.location;
                    }
                    if (effectiveRange.length > range.length) effectiveRange.length = range.length;
                    // We need to make a new copy
                    attrs = __CFAttributedStringCreateAttributesDictionary(CFGetAllocator(attrStr), attrs);
                    __CFDictionaryAddMultiple(attrs, additionalKeys, additionalValues, numAdditionalItems);
                    CFRunArrayReplace(attrStr->attributeArray, effectiveRange, attrs, effectiveRange.length);
                    CFRelease(attrs);
                }
                range.length -= effectiveRange.length;
                range.location += effectiveRange.length;
            }
            // CFAttributedStringEndEditing(attrStr);
            
            freeLocalArray(additionalKeys);
            freeLocalArray(additionalValues);
        }
    }
}

void CFAttributedStringSetAttribute(CFMutableAttributedStringRef attrStr, CFRange range, CFStringRef attrName, CFTypeRef value) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, addAttribute:(NSString *)attrName value:(id) value range:NSMakeRange(range.location, range.length));
    __CFAssertIsAttributedStringAndMutable(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);

    // CFAttributedStringBeginEditing(attrStr);
    while (range.length) {
        CFRange effectiveRange;
        // effectiveRange.location returned here may be equal to or smaller than range.location
        CFMutableDictionaryRef attrs = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, range.location, &effectiveRange, NULL);
#if !DEPLOYMENT_RUNTIME_SWIFT // We can't make assumptions of the retain count in Swift.
        if (effectiveRange.location == range.location && effectiveRange.length <= range.length && CFGetRetainCount(attrs) == 1) {
            // !!! Retain count check cheesy; need to keep our own reference count)
            // The attributes dictionary can just be modified in place
            // Here we are assuming that CFRunArray is not under-reporting the effectiveRange for a given item!
            CFDictionarySetValue(attrs, attrName, value);
        } else
#endif // !DEPLOYMENT_RUNTIME_SWIFT
        {
            // Intersect effectiveRange and range
            if (effectiveRange.location < range.location) {
                effectiveRange.length -= (range.location - effectiveRange.location);
                effectiveRange.location = range.location;	    
	     }
	     if (effectiveRange.length > range.length) effectiveRange.length = range.length;
            // First check to see if the same value already exists; this will avoid a copy
            CFTypeRef existingValue = CFDictionaryGetValue(attrs, attrName);
            if (!existingValue || !CFEqual(existingValue, value)) {
                // We need to make a new copy
                attrs = __CFAttributedStringCreateAttributesDictionary(CFGetAllocator(attrStr), attrs);
                CFDictionarySetValue(attrs, attrName, value);
                CFRunArrayReplace(attrStr->attributeArray, effectiveRange, attrs, effectiveRange.length);
                CFRelease(attrs);
            }
        }
        range.length -= effectiveRange.length;
        range.location += effectiveRange.length;
    }
    // CFAttributedStringEndEditing(attrStr);
}

void CFAttributedStringRemoveAttribute(CFMutableAttributedStringRef attrStr, CFRange range, CFStringRef attrName) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, removeAttribute:(NSString *)attrName range:NSMakeRange(range.location, range.length));
    __CFAssertIsAttributedStringAndMutable(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);

    // CFAttributedStringBeginEditing(attrStr);
    while (range.length) {
        CFRange effectiveRange;
        CFMutableDictionaryRef attrs = (CFMutableDictionaryRef)CFRunArrayGetValueAtIndex(attrStr->attributeArray, range.location, &effectiveRange, NULL);
#if !DEPLOYMENT_RUNTIME_SWIFT // We can't make assumptions of the retain count in Swift.
        if (effectiveRange.location == range.location && effectiveRange.length <= range.length && CFGetRetainCount(attrs) == 1) {
	    // !!! Retain count check cheesy; need to keep our own reference count
            // The attributes dictionary can just be modified in place
	    // Here we are assuming that CFRunArray is not under-reporting the effectiveRange for a given item!
            CFDictionaryRemoveValue(attrs, attrName);
        } else
#endif // !DEPLOYMENT_RUNTIME_SWIFT
        {
            // Intersect effectiveRange and range
            if (effectiveRange.location < range.location) {
                effectiveRange.length -= (range.location - effectiveRange.location);
                effectiveRange.location = range.location;	    
	     }
	     if (effectiveRange.length > range.length) effectiveRange.length = range.length;
            // First check to see if the value is not there; this will avoid a copy
            if (CFDictionaryContainsKey(attrs, attrName)) {
		// We need to make a new copy
		attrs = __CFAttributedStringCreateAttributesDictionary(CFGetAllocator(attrStr), attrs);
		CFDictionaryRemoveValue(attrs, attrName);
		CFRunArrayReplace(attrStr->attributeArray, effectiveRange, attrs, effectiveRange.length);
		CFRelease(attrs);
	    }
        }
        range.length -= effectiveRange.length;
        range.location += effectiveRange.length;
    }
    // CFAttributedStringEndEditing(attrStr);
}

void CFAttributedStringReplaceAttributedString(CFMutableAttributedStringRef attrStr, CFRange range, CFAttributedStringRef replacement) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, replaceCharactersInRange:NSMakeRange(range.location, range.length) withAttributedString:(NSAttributedString *)replacement);
    __CFAssertIsAttributedStringAndMutable(attrStr);
    __CFAssertRangeIsInBounds(attrStr, range.location, range.length);

    CFStringRef otherStr = CFAttributedStringGetString(replacement);
    CFIndex stringLen = CFStringGetLength(otherStr);

    if (stringLen > 0) {
	CFAllocatorRef allocator = CFGetAllocator(attrStr);
	CFRange attrRange = {0, 0};
	while (attrRange.location < stringLen) {
	    CFDictionaryRef otherAttrs = CFAttributedStringGetAttributes(replacement, attrRange.location, &attrRange);
            CFMutableDictionaryRef attrs = __CFAttributedStringCreateAttributesDictionary(allocator, otherAttrs);
	    CFRunArrayInsert(attrStr->attributeArray, CFRangeMake(attrRange.location + range.location, attrRange.length), attrs);
	    CFRelease(attrs);
	    attrRange.location += attrRange.length;
	}
    }
    if (range.length > 0) CFRunArrayDelete(attrStr->attributeArray, CFRangeMake(range.location + stringLen, range.length));
    CFStringReplace((CFMutableStringRef)(attrStr->string), range, otherStr);
    // [self edited:NSAttributedStringEditedCharacters|NSAttributedStringEditedAttributes range:range changeInLength:stringLen - range.length];
}

void CFAttributedStringBeginEditing(CFMutableAttributedStringRef attrStr) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, beginEditing);
}

void CFAttributedStringEndEditing(CFMutableAttributedStringRef attrStr) {
    CF_OBJC_FUNCDISPATCHV(CFAttributedStringGetTypeID(), void, (NSMutableAttributedString *)attrStr, endEditing);
}



/*** Functions for NSCFAttributedString usage ***/


CFIndex _CFAttributedStringGetLength(CFAttributedStringRef attrStr) {
    return CFStringGetLength(attrStr->string);
}


int _CFAttributedStringCheckAndReplace(CFMutableAttributedStringRef attrStr, CFRange range, CFStringRef replacement) {
    CFIndex len = CFStringGetLength(attrStr->string);
    if (range.location > len || (range.location + range.length > len)) return _CFStringErrBounds;
    if (!__CFAttributedStringIsMutable(attrStr)) return _CFStringErrNotMutable;
    CFAttributedStringReplaceString(attrStr, range, replacement);   // ??? Do this faster!
    return _CFStringErrNone;
}

int _CFAttributedStringCheckAndReplaceAttributed(CFMutableAttributedStringRef attrStr, CFRange range, CFAttributedStringRef replacement) {
    CFIndex len = CFStringGetLength(attrStr->string);
    if (range.location > len || (range.location + range.length > len)) return _CFStringErrBounds;
    if (!__CFAttributedStringIsMutable(attrStr)) return _CFStringErrNotMutable;
    if (replacement) {  // Special: NULL means delete
	CFAttributedStringReplaceAttributedString(attrStr, range, replacement);   // ??? Do this faster!
    } else {
        CFAttributedStringReplaceString(attrStr, range, CFSTR(""));
    }
    return _CFStringErrNone;
}

int _CFAttributedStringCheckAndSetAttributes(CFMutableAttributedStringRef attrStr, CFRange range, CFTypeRef attrOrAttrs, Boolean clearOthers) {
    CFIndex len = CFStringGetLength(attrStr->string);
    if (range.location > len || (range.location + range.length > len)) return _CFStringErrBounds;
    if (!__CFAttributedStringIsMutable(attrStr)) return _CFStringErrNotMutable;
    CFAttributedStringSetAttributes(attrStr, range, (CFDictionaryRef)attrOrAttrs, clearOthers);  // ??? Do this faster!
    return _CFStringErrNone;
}

int _CFAttributedStringCheckAndSetAttribute(CFMutableAttributedStringRef attrStr, CFRange range, CFStringRef attrName, CFTypeRef attr) {
    CFIndex len = CFStringGetLength(attrStr->string);
    if (range.location > len || (range.location + range.length > len)) return _CFStringErrBounds;
    if (!__CFAttributedStringIsMutable(attrStr)) return _CFStringErrNotMutable;
    if (!attrName) return _CFStringErrNilArg;
    if (attr) {
	CFAttributedStringSetAttribute(attrStr, range, attrName, attr);  // ??? Do this faster!
    } else {
	CFAttributedStringRemoveAttribute(attrStr, range, attrName);    // ??? Do this faster!
    }
    return _CFStringErrNone;
}



