/*      CFBundle_DebugStrings.c
 Copyright (c) 1999-2017, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2017, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Sean Stewart <sean_stewart@apple.com>
 */

#include <assert.h>
#include "CFBundle_Internal.h"

// Directionality formatting codepoints
#define UTF16_RIGHT_TO_LEFT_OVERRIDE 0x202e
#define UTF16_POP_DIRECTIONAL_FORMATTING 0x202c

// Combining accent codepoints
#define UTF16_COMBINING_GRAVE_ACCENT 0x0300
#define UTF16_COMBINING_CEDILLA 0x0327
#define UTF16_COMBINING_TILDE 0x0303
#define UTF16_COMBINING_RING_BELOW 0x0325
#define UTF16_COMBINING_RING_ABOVE 0x030A
#define UTF16_COMBINING_DIAERESIS 0x0308

CF_EXPORT CFStringRef _CFDoubledStringCreate(CFStringRef theString) {
    CFMutableStringRef _Nonnull doubledString = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppend(doubledString, theString);
    // Prevent doubling any format string specifiers that may be present
    CFStringFindAndReplace(doubledString, CFSTR("%"), CFSTR(""), CFRangeMake(0, CFStringGetLength(doubledString)), 0);
    CFStringAppendFormat(doubledString, NULL, CFSTR(" %@"), theString);
    return (CFStringRef)doubledString;
}

static CFStringRef __CFAccentuatedStringCreateWithAcceptableAccentChars(CFStringRef inString, UniChar const *acceptableAccentChars, CFIndex acceptableCharLength) {
    // Copy the incoming string
    CFMutableStringRef _Nonnull workingString = CFStringCreateMutableCopy(kCFAllocatorDefault, CFStringGetLength(inString), inString);

    // Create UniChar (UTF-16) buffer from original string
    CFIndex const workingStringLength = CFStringGetLength(workingString);
    CFStringInlineBuffer originalStringInlineBuffer;
    CFStringInitInlineBuffer(workingString, &originalStringInlineBuffer, CFRangeMake(0, workingStringLength));

    // Create new double-length UniChar (UTF-16) buffer for the new string
    // We will trim the size later, but worst case is that there will be no skipped whitespace characters
    UniChar *_Nonnull const accentedStringBuffer = malloc((workingStringLength * 2 + 1) * sizeof(UniChar)); // +1 for null char
    CFIndex accentedStringCursor = 0;

    // Create CFCharacterSets that contain characters we wish to skip over during the accentuation loop
    static CFCharacterSetRef _Nullable charsToSkip = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        CFMutableCharacterSetRef _Nonnull charsToSkipMutable = CFCharacterSetCreateMutable(kCFAllocatorDefault);
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetControl));
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline));
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetPunctuation));
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetSymbol));
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit));
        CFCharacterSetUnion(charsToSkipMutable, CFCharacterSetGetPredefined(kCFCharacterSetNonBase));
        charsToSkip = CFCharacterSetCreateCopy(kCFAllocatorDefault, charsToSkipMutable);
        CFRelease(charsToSkipMutable);
    });

    // For each non-skipped character, add a combining accent after the codepoint
    for (CFIndex i = 0; i < workingStringLength;) { // inner loop increments i
        CFRange const composedCharRange = CFStringGetRangeOfComposedCharactersAtIndex(workingString, i); // range length will always be >=1
        UniChar c;
        // Enqueue the characters within the composedCharRange
        for (CFIndex j = 0; j < composedCharRange.length; j++) {
            c = CFStringGetCharacterFromInlineBuffer(&originalStringInlineBuffer, i++);
            // Defend against CFStringGetCharacterFromInlineBuffer edge case listed in documentation
            if (c == 0) assert("UniChar at specified index could not be accessed as it is outside the original range specified when initializing the CFStringInlineBuffer");
            accentedStringBuffer[accentedStringCursor++] = c;
        }
        // Don't sprinkle accents onto composed char sequences and characters belonging to certain ranges
        // The goal here is to _never_ ever accentuate composed char sequences (like emoji)
        if ((composedCharRange.length == 1) && !CFCharacterSetIsCharacterMember(charsToSkip, c)) {
            // Deterministically pair a combining accent
            CFIndex const combiningAccentIndex = c % acceptableCharLength;
            UniChar const accentChar = acceptableAccentChars[combiningAccentIndex];
            accentedStringBuffer[accentedStringCursor++] = accentChar;
        }
    }
    // Terminate the UniChar buffer with a null char
    accentedStringBuffer[accentedStringCursor] = (UniChar)0x0;

    // Create a mutable string ref from the new char buffer (dealloc using kCFAllocatorNull and free the char buffer later ourselves)
    CFMutableStringRef _Nonnull const accentedStringFromCharBuffer = CFStringCreateMutableWithExternalCharactersNoCopy(NULL, accentedStringBuffer, accentedStringCursor, accentedStringCursor, kCFAllocatorNull);

    // Make a copy of the string so that CFString can optimize itself
    // CFStrings bound to client-owned buffers do not benefit from CFString-internal optimizations
    CFStringRef _Nonnull accentedString = CFStringCreateCopy(kCFAllocatorDefault, accentedStringFromCharBuffer);

    // Cleanup
    CFRelease(workingString);
    CFRelease(accentedStringFromCharBuffer);
    free(accentedStringBuffer);
    return (CFStringRef)accentedString;
}

CF_EXPORT CFStringRef _CFAccentuatedStringCreate(CFStringRef theString) {
    /* Default set of "acceptable" Unicode combining accent codepoints
     * These are hand-selected from the superset of combining accents because this subset is commonly used on Latin characters
     * Additionally they're visually obvious and all look unique (that's why we choose to use only the GRAVE and not GRAVE & ACUTE accents due to similarity)
     */
    static UniChar const acceptableAccentChars[] = {
        UTF16_COMBINING_GRAVE_ACCENT,
        UTF16_COMBINING_CEDILLA,
        UTF16_COMBINING_TILDE,
        UTF16_COMBINING_RING_BELOW,
        UTF16_COMBINING_RING_ABOVE,
        UTF16_COMBINING_DIAERESIS
    };
    static CFIndex const len = sizeof(acceptableAccentChars)/sizeof(UniChar);
    return __CFAccentuatedStringCreateWithAcceptableAccentChars(theString, acceptableAccentChars, len);
}

CF_EXPORT CFStringRef _CFAffixedStringCreate(CFStringRef theString, CFStringRef prefix, CFStringRef suffix) {
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@%@%@"), prefix, theString, suffix);
}

CF_EXPORT CFStringRef _CFRLORightToLeftStringCreate(CFStringRef theString) {
    CFStringRef rtlString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%C%@%C"), UTF16_RIGHT_TO_LEFT_OVERRIDE, theString, UTF16_POP_DIRECTIONAL_FORMATTING);
    return rtlString;
}
