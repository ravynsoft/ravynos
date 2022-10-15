// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

/*  CFStringTransform.c
    Copyright (c) 2004-2015, Apple Inc. All rights reserved.
        
!!! For performance reasons, it's important that all functions marked CF_INLINE in this file are inlined.
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <unicode/utrans.h>

static const char *__CFStringTransformGetICUIdentifier(CFStringRef identifier);

#define INSTRUMENT_CACHE 0

#if INSTRUMENT_CACHE
#include <stdio.h>
#endif

#if TARGET_OS_IPHONE
#define LITE_CACHE 1
#endif

#if LITE_CACHE
static CFStringRef __CFStringTransformCacheID = NULL;
static UTransliterator *__CFStringTransformCacheTL = NULL;
static Boolean __CFStringTransformCacheRev = false;
#else
static CFMutableDictionaryRef __CFStringTransformCache = NULL;
#endif
static CFLock_t __CFStringTransformGlobalLock = CFLockInit;

#define kCFStringTransformStackBufferSize 1024

CF_INLINE void __CFStringTransformLockGlobal(void) {
    __CFLock(&__CFStringTransformGlobalLock);
}

CF_INLINE void __CFStringTransformUnlockGlobal(void) {
    __CFUnlock(&__CFStringTransformGlobalLock);
}

#if !LITE_CACHE
struct transform_element {
    struct transform_element *_next;
    UTransliterator *_transliterator;
};

struct transform_head {
    struct transform_element *_forward_q;
    struct transform_element *_reverse_q;
};
#endif

/* Create an element for the transform cache, including the ICU transliterator
*/

#if LITE_CACHE
static UTransliterator *
#else
static struct transform_element *
#endif
__CFStringTransformCreate(CFStringRef identifier, bool reverse) {
#if INSTRUMENT_CACHE
    fprintf(stderr, "Create %s transform ", reverse?"reverse":"forward");
    CFShow(identifier);
#endif
    UTransliterator *made = NULL;
    UErrorCode icuStatus = U_ZERO_ERROR;

    const char *known = __CFStringTransformGetICUIdentifier(identifier);
    if (known) {
	UniChar buff[kCFStringTransformStackBufferSize];
	CFIndex len = strlen(known);
	CFIndex idx;
	for (idx = 0; idx < len; idx++) {
	    buff[idx] = known[idx];
	}
	made = utrans_openU((const UChar *)buff, len, reverse?UTRANS_REVERSE:UTRANS_FORWARD, NULL, 0, NULL, &icuStatus);
    } else {
	CFIndex len = CFStringGetLength(identifier);
	const UniChar *ucp = CFStringGetCharactersPtr(identifier);
	if (ucp != NULL) {
	    made = utrans_openU((const UChar *)ucp, len, reverse?UTRANS_REVERSE:UTRANS_FORWARD, NULL, 0, NULL, &icuStatus);
	} else if (len <= kCFStringTransformStackBufferSize) {
	    UniChar buff[kCFStringTransformStackBufferSize];
	    CFStringGetCharacters(identifier, CFRangeMake(0, len), buff);
	    made = utrans_openU((const UChar *)buff, len, reverse?UTRANS_REVERSE:UTRANS_FORWARD, NULL, 0, NULL, &icuStatus);
	} else {
	    UniChar *buff = (UniChar *) CFAllocatorAllocate(kCFAllocatorSystemDefault, len * sizeof(UniChar), 0);
	    if (buff == NULL)
		return NULL;
	    CFStringGetCharacters(identifier, CFRangeMake(0, len), buff);
	    made = utrans_openU((const UChar *)buff, len, reverse?UTRANS_REVERSE:UTRANS_FORWARD, NULL, 0, NULL, &icuStatus);
	    CFAllocatorDeallocate(kCFAllocatorSystemDefault, buff);
	}
    }

    if (U_FAILURE(icuStatus)) {
        if (made)
            utrans_close(made);
        return NULL;
    }

#if LITE_CACHE
    return made;
#else
    struct transform_element *result = NULL;
    result = (struct transform_element *) CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(struct transform_element), 0);
    if (result == NULL) {
        utrans_close(made);
        return NULL;
    }
    
    result->_next = NULL;
    result->_transliterator = made;
#if INSTRUMENT_CACHE
    fprintf(stderr, "Created\n");
#endif
    return result;
#endif
}

/* Acquire a transform element from the cache
*/

#if LITE_CACHE
static UTransliterator *
#else
static struct transform_element *
#endif
__CFStringTransformAcquire(CFStringRef identifier, bool reverse) {
#if LITE_CACHE
    __CFStringTransformLockGlobal();
    if (__CFStringTransformCacheID && (reverse == __CFStringTransformCacheRev) && (identifier == __CFStringTransformCacheID || CFEqual(identifier, __CFStringTransformCacheID))) {
        CFRelease(__CFStringTransformCacheID);
        __CFStringTransformCacheID = NULL;
        UTransliterator *cached = __CFStringTransformCacheTL;
	__CFStringTransformCacheTL = NULL;
        __CFStringTransformUnlockGlobal();
        return cached;
    }
    __CFStringTransformUnlockGlobal();

    UTransliterator *made = __CFStringTransformCreate(identifier, reverse);
    return made;
#else
    struct transform_head *head = NULL, *second_head = NULL;
    struct transform_element *element = NULL;
    CFMutableDictionaryRef cache = NULL;
#if INSTRUMENT_CACHE
    fprintf(stderr, "Acquire %s transform ", reverse?"reverse":"forward");
    CFShow(identifier);
#endif
    __CFStringTransformLockGlobal();
    
    /* Fetch the cache, creating it if necessary
    */
    if (__CFStringTransformCache == NULL) {
        __CFStringTransformUnlockGlobal();
#if INSTRUMENT_CACHE
        fprintf(stderr, "Create cache\n");
#endif
        cache = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, NULL);
        if (cache == NULL)
            return NULL;
        __CFStringTransformLockGlobal();
        if (__CFStringTransformCache == NULL) {
            __CFStringTransformCache = cache;
        }
        else {
#if INSTRUMENT_CACHE
            fprintf(stderr, "Redundant cache created\n");
#endif
            CFRelease(cache);
        }
    }
    cache = __CFStringTransformCache;
    
    /* Now we have the cache. Fetch the head element. Create it if necessary.
    */
    head = (struct transform_head *) CFDictionaryGetValue(cache, identifier);
    if (head == NULL) {
        __CFStringTransformUnlockGlobal();

#if INSTRUMENT_CACHE
        fprintf(stderr, "Cache entry head not found\n");
#endif
        /* Make the element first in case it's a bogus transform
        */
        element = __CFStringTransformCreate(identifier, reverse);
        
        if (element == NULL)
            return NULL;

        /* Create a new head element
        */
        head = (struct transform_head *) CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(struct transform_head), 0);
        if (head == NULL) {
            utrans_close(element->_transliterator);
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, element);
            return NULL;
        }
        head->_forward_q = NULL;
        head->_reverse_q = NULL;

#if INSTRUMENT_CACHE
        fprintf(stderr, "Cache head element created\n");
#endif
        
        __CFStringTransformLockGlobal();
        
        /* Check the head element again in case someone else made it first.
        */
        second_head = (struct transform_head *) CFDictionaryGetValue(cache, identifier);
        if (second_head == NULL) {
            CFDictionaryAddValue(cache, identifier, head);
        }
    }
    
    /* If we don't already have a transliterator, get one from the queue or create one.
    */
    if (element == NULL) {      // if element is NULL head cannot be NULL
        if (reverse) {
            element = head->_reverse_q;
            if (element != NULL) {
                head->_reverse_q = element->_next;
            }
        }
        else {
            element = head->_forward_q;
            if (element != NULL) {
                head->_forward_q = element->_next;
            }
        }
    }

    __CFStringTransformUnlockGlobal();
    
    /* If we made a head and someone else did too, dump ours.
    */
    if (second_head != NULL) {
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, head);
#if INSTRUMENT_CACHE
        fprintf(stderr, "Cache head element was redundant\n");
#endif
    }
    
    /* Did we get something? If not, make a new one. It will go on the queue later.
    */
    if (element == NULL) {
        element = __CFStringTransformCreate(identifier, reverse);
    }
#if INSTRUMENT_CACHE
    else {
        fprintf(stderr, "Transform reused from queue\n");
    }

    if (element) {
        fprintf(stderr, "Transform acquired\n");
    }
#endif
    
    return element;
#endif
}

/* Release a transform element back to the cache
*/

static void __CFStringTransformRelease(CFStringRef identifier, bool reverse,
#if LITE_CACHE
 UTransliterator *tl
#else
 struct transform_element *element
#endif
) {
#if DISABLE_CACHE

        utrans_close(element->_transliterator);
	CFAllocatorDeallocate(kCFAllocatorSystemDefault, element);
	return;
#endif

#if LITE_CACHE
    __CFStringTransformLockGlobal();
    if (__CFStringTransformCacheID) {
        CFRelease(__CFStringTransformCacheID);
        utrans_close(__CFStringTransformCacheTL);
    }
    __CFStringTransformCacheID = CFStringCreateCopy(kCFAllocatorSystemDefault, identifier);
    __CFStringTransformCacheTL = tl;
    __CFStringTransformCacheRev = reverse;
    __CFStringTransformUnlockGlobal();
    return;
#else

#if INSTRUMENT_CACHE
    fprintf(stderr, "Releasing %s transform element ", reverse?"reverse":"forward");
    CFShow(identifier);
#endif
    __CFStringTransformLockGlobal();
    
    /* We cannot have a transform_element if there wasn't a head created for it, so we don't need to check that here. We also don't need to check for the cache being present.
    */
    struct transform_head *head = (struct transform_head *)CFDictionaryGetValue(__CFStringTransformCache, identifier);
    if (reverse) {
        element->_next = head->_reverse_q;
        head->_reverse_q = element;
    }
    else {
        element->_next = head->_forward_q;
        head->_forward_q = element;
    }
    
    __CFStringTransformUnlockGlobal();
#endif
}

/* Callback functions for UReplaceable and CFMutableStringRef
*/

typedef struct {
    CFMutableStringRef _source;
    CFIndex _sourceLength;
    CFStringInlineBuffer _buffer;
    CFMutableStringRef _externalMutable;
} _CFStringUReplaceable;

static int32_t __CFStringReplaceableLength(const UReplaceable *rep)
{
    return (int32_t) ((const _CFStringUReplaceable *)rep)->_sourceLength;
}

static UChar __CFStringReplaceableCharAt(const UReplaceable* rep, int32_t offset)
{
    
    _CFStringUReplaceable *myrep = (_CFStringUReplaceable *)rep;
    if (offset >= myrep->_sourceLength)
        return 0xFFFF;
    return (UChar) CFStringGetCharacterFromInlineBuffer(&myrep->_buffer, (CFIndex)offset);
}

static UChar32 __CFStringReplaceableChar32At(const UReplaceable *rep, int32_t offset)
{
    _CFStringUReplaceable *myrep = (_CFStringUReplaceable *)rep;
    if (offset >= myrep->_sourceLength)
        return 0xFFFF;
    UniChar character = CFStringGetCharacterFromInlineBuffer(&myrep->_buffer, (CFIndex)offset);
    if (!CFUniCharIsSurrogateHighCharacter(character) || offset >= myrep->_sourceLength-1)
        return (UChar32) character;

    UTF32Char result = character;
    UniChar second = CFStringGetCharacterFromInlineBuffer(&myrep->_buffer, (CFIndex)offset+1);
    if (CFUniCharIsSurrogateLowCharacter(second)) {
        result = CFUniCharGetLongCharacterForSurrogatePair(character, second);
    }
    
    return (UChar32) result;
}

static void __CFStringReplaceableReplace(UReplaceable *rep, int32_t start, int32_t limit, const UChar *text, int32_t textLimit)
{
    /* We're using a mutable string here so we can reuse it. We will never actually change the characters themselves, despite casting away const.
    */
    _CFStringUReplaceable *myrep = (_CFStringUReplaceable *)rep;
    if (myrep->_externalMutable == NULL) {
        myrep->_externalMutable = CFStringCreateMutableWithExternalCharactersNoCopy(kCFAllocatorSystemDefault, (UniChar *)text, (CFIndex) textLimit, (CFIndex) textLimit, kCFAllocatorNull);
    }
    else {
        CFStringSetExternalCharactersNoCopy(myrep->_externalMutable, (UniChar *)text, (CFIndex) textLimit, (CFIndex) textLimit);
    }
    CFStringReplace(myrep->_source, CFRangeMake((CFIndex)start, (CFIndex)(limit-start)), myrep->_externalMutable);
    myrep->_sourceLength = CFStringGetLength(myrep->_source);
    CFStringInitInlineBuffer(myrep->_source, &myrep->_buffer, CFRangeMake(0, myrep->_sourceLength));
}

static void __CFStringReplaceableExtract(UReplaceable *rep, int32_t start, int32_t limit, UChar *dst)
{
    _CFStringUReplaceable *myrep = (_CFStringUReplaceable *)rep;
    CFStringGetCharactersFromInlineBuffer(&myrep->_buffer, CFRangeMake((CFIndex)start, (CFIndex)(limit-start)), (UniChar *)dst);
}

static void __CFStringReplaceableCopy(UReplaceable *rep, int32_t start, int32_t limit, int32_t dest)
{
    _CFStringUReplaceable *myrep = (_CFStringUReplaceable *)rep;
    CFStringRef sub = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, myrep->_source, CFRangeMake((CFIndex)start, (CFIndex)(limit-start)));
    CFStringInsert(myrep->_source, (CFIndex)dest, sub);
    CFRelease(sub);
    myrep->_sourceLength = CFStringGetLength(myrep->_source);
    CFStringInitInlineBuffer(myrep->_source, &myrep->_buffer, CFRangeMake(0, myrep->_sourceLength));
}

static UReplaceableCallbacks __CFStringReplaceableCallbacks = 
{
    __CFStringReplaceableLength,
    __CFStringReplaceableCharAt,
    __CFStringReplaceableChar32At,
    __CFStringReplaceableReplace,
    __CFStringReplaceableExtract,
    __CFStringReplaceableCopy
};

/* End UReplaceable CFMutableStringRef callbacks
*/


/* Constant strings for predefined transforms
*/

struct transform_map {
    const CFStringRef *from;
    const char *to;
};

/* Use leading ) to avoid collisions with legal ICU identifiers
*/
CONST_STRING_DECL(kCFStringTransformStripCombiningMarks, ")kCFStringTransformStripCombiningMarks")
CONST_STRING_DECL(kCFStringTransformToLatin, ")kCFStringTransformToLatin")
CONST_STRING_DECL(kCFStringTransformFullwidthHalfwidth, ")kCFStringTransformFullwidthHalfwidth")
CONST_STRING_DECL(kCFStringTransformLatinKatakana, ")kCFStringTransformLatinKatakana")
CONST_STRING_DECL(kCFStringTransformLatinHiragana, ")kCFStringTransformLatinHiragana")
CONST_STRING_DECL(kCFStringTransformHiraganaKatakana, ")kCFStringTransformHiraganaKatakana")
CONST_STRING_DECL(kCFStringTransformMandarinLatin, ")kCFStringTransformMandarinLatin")
CONST_STRING_DECL(kCFStringTransformLatinHangul, ")kCFStringTransformLatinHangul")
CONST_STRING_DECL(kCFStringTransformLatinArabic, ")kCFStringTransformLatinArabic")
CONST_STRING_DECL(kCFStringTransformLatinHebrew, ")kCFStringTransformLatinHebrew")
CONST_STRING_DECL(kCFStringTransformLatinThai, ")kCFStringTransformLatinThai")
CONST_STRING_DECL(kCFStringTransformLatinCyrillic, ")kCFStringTransformLatinCyrillic")
CONST_STRING_DECL(kCFStringTransformLatinGreek, ")kCFStringTransformLatinGreek")
CONST_STRING_DECL(kCFStringTransformToXMLHex, ")kCFStringTransformToXMLHex")
CONST_STRING_DECL(kCFStringTransformToUnicodeName, ")kCFStringTransformToUnicodeName")
CONST_STRING_DECL(kCFStringTransformStripDiacritics, ")kCFStringTransformStripDiacritics")

static struct transform_map __CFStringTransformNameMap[] = {
    {&kCFStringTransformStripCombiningMarks, "NFD; [:Mn:] Remove; NFC"},
    {&kCFStringTransformToLatin, "Any-Latin"},
    {&kCFStringTransformFullwidthHalfwidth, "Fullwidth-Halfwidth"},
    {&kCFStringTransformLatinKatakana, "Latin-Katakana"},
    {&kCFStringTransformLatinHiragana, "Latin-Hiragana"},
    {&kCFStringTransformHiraganaKatakana, "Hiragana-Katakana"},
    {&kCFStringTransformMandarinLatin, "Han-Latin"},
    {&kCFStringTransformLatinHangul, "Latin-Hangul"},
    {&kCFStringTransformLatinArabic, "Latin-Arabic"},
    {&kCFStringTransformLatinHebrew, "Latin-Hebrew"},
    {&kCFStringTransformLatinThai, "Latin-Thai"},
    {&kCFStringTransformLatinCyrillic, "Latin-Cyrillic"},
    {&kCFStringTransformLatinGreek, "Latin-Greek"},
    {&kCFStringTransformToXMLHex, "[^[:ASCII:]-[:C:]]; Any-Hex/XML"},
    {&kCFStringTransformToUnicodeName, "[^[:ASCII:]-[:C:]]; Any-Name"},
    {&kCFStringTransformStripDiacritics, "NFD; [[[:Mn:]&[:Diacritic:]]-[:Grapheme_Link:]-[:ccc=0:]-[:ccc=8:]-[\\u0338]] Remove; NFC"},
    {0,0}
};

/* Translation of constant strings to ICU transliterator IDs
*/

static const char *__CFStringTransformGetICUIdentifier(CFStringRef identifier) {
    if (CFStringGetLength(identifier) > 0 && CFStringGetCharacterAtIndex(identifier, 0) == ')') {
        CFIndex slot;
        for (slot = 0; __CFStringTransformNameMap[slot].from != NULL; ++slot) {
            if (*__CFStringTransformNameMap[slot].from == identifier) {
		return __CFStringTransformNameMap[slot].to;
            }
        }
        for (slot = 0; __CFStringTransformNameMap[slot].from != NULL; ++slot) {
            if (CFEqual(*__CFStringTransformNameMap[slot].from, identifier)) {
		return __CFStringTransformNameMap[slot].to;
            }
        }
    }
    return NULL;
}

/* Main entry point
*/

Boolean CFStringTransform(CFMutableStringRef string, CFRange *range, CFStringRef transform, Boolean reverse)
{
    Boolean result = false;
#if LITE_CACHE
    UTransliterator *tl = __CFStringTransformAcquire(transform, (reverse != 0));
    if (!tl) return false;
#else
    struct transform_element *element = __CFStringTransformAcquire(transform, (reverse != 0));
    if (element == NULL)
        return false;
    UTransliterator *tl = element->_transliterator;
#endif

    CFRange everything;
    if (range == NULL) {
        everything.location = 0;
        everything.length = CFStringGetLength(string);
        range = &everything;
    }
    
    /* Set up the UReplaceable
    */
    _CFStringUReplaceable replaceable;
    replaceable._source = string;
    replaceable._sourceLength = CFStringGetLength(string);
    CFStringInitInlineBuffer(string, &replaceable._buffer, CFRangeMake(0, replaceable._sourceLength));
    replaceable._externalMutable = NULL;
    
    /* Actually do the transform
    */
    UErrorCode icuStatus = U_ZERO_ERROR;
    int32_t limit = range->location + range->length;
    utrans_trans(tl, (UReplaceable *)&replaceable, &__CFStringReplaceableCallbacks, (int32_t) range->location, &limit, &icuStatus);

    /* Get rid of the utility CFMutableString if we allocated it.
    */
    if (replaceable._externalMutable != NULL) {
        CFRelease(replaceable._externalMutable);
    }

    if (U_SUCCESS(icuStatus)) {
        range->length = limit - range->location;
        result = true;
    }

#if LITE_CACHE
    __CFStringTransformRelease(transform, (reverse != 0), tl);
#else
    __CFStringTransformRelease(transform, (reverse != 0), element);
#endif
    
    return result;
}


