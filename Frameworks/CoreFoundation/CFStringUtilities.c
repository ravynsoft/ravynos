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

/*	CFStringUtilities.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFInternal.h"
#include <CoreFoundation/CFStringEncodingConverterExt.h>
#include <CoreFoundation/CFUniChar.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include "CFStringEncodingDatabase.h"
#include "CFICUConverters.h"
#include <limits.h>
#include <stdlib.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#include <unicode/ucol.h>
#include <unicode/ucoleitr.h>
#endif
#include <string.h>

#if  DEPLOYMENT_TARGET_WINDOWS
#include <tchar.h>
#endif


Boolean CFStringIsEncodingAvailable(CFStringEncoding theEncoding) {
    switch (theEncoding) {
        case kCFStringEncodingASCII: // Built-in encodings
        case kCFStringEncodingMacRoman:
        case kCFStringEncodingUTF8:
        case kCFStringEncodingNonLossyASCII:
        case kCFStringEncodingWindowsLatin1:
        case kCFStringEncodingNextStepLatin:
        case kCFStringEncodingUTF16:
        case kCFStringEncodingUTF16BE:
        case kCFStringEncodingUTF16LE:
        case kCFStringEncodingUTF32:
        case kCFStringEncodingUTF32BE:
        case kCFStringEncodingUTF32LE:
            return true;

        default:
            return CFStringEncodingIsValidEncoding(theEncoding);
    }
}

const CFStringEncoding* CFStringGetListOfAvailableEncodings() {
    return (const CFStringEncoding *)CFStringEncodingListOfAvailableEncodings();
}

CFStringRef CFStringGetNameOfEncoding(CFStringEncoding theEncoding) {
    static CFMutableDictionaryRef mappingTable = NULL;
    static OSSpinLock mappingTableLock = OS_SPINLOCK_INIT;

    CFStringRef theName = NULL;

    if (mappingTable) {
        OSSpinLockLock(&mappingTableLock);
        theName = (CFStringRef)CFDictionaryGetValue(mappingTable, (const void*)(uintptr_t)theEncoding);
        OSSpinLockUnlock(&mappingTableLock);
    }

    if (!theName) {
        const char *encodingName = __CFStringEncodingGetName(theEncoding);
        
        if (encodingName) {
            theName = CFStringCreateWithCString(kCFAllocatorSystemDefault, encodingName, kCFStringEncodingASCII);
        }
        
        if (theName) {
            OSSpinLockLock(&mappingTableLock);

            CFStringRef result = NULL;
            if (!mappingTable) {
                mappingTable = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, (const CFDictionaryKeyCallBacks *)NULL, &kCFTypeDictionaryValueCallBacks);
            } else {    // Check to see if this got in the dictionary in the meantime
                result = (CFStringRef)CFDictionaryGetValue(mappingTable, (const void*)(uintptr_t)theEncoding);
            }
            if (!result) {  // If not, add it in
                CFDictionaryAddValue(mappingTable, (const void*)(uintptr_t)theEncoding, (const void*)theName);
                OSSpinLockUnlock(&mappingTableLock);
                CFRelease(theName);
            } else {        // Otherwise use the one already in there
                OSSpinLockUnlock(&mappingTableLock);
                CFRelease(theName);
                theName = result;
            }
        }
    }

    return theName;
}

CFStringEncoding CFStringConvertIANACharSetNameToEncoding(CFStringRef charsetName) {
    CFStringEncoding encoding = kCFStringEncodingInvalidId;
#define BUFFER_SIZE (100)
    char buffer[BUFFER_SIZE];
    const char *name = CFStringGetCStringPtr(charsetName, __CFStringGetEightBitStringEncoding());

    if (NULL == name) {
        if (false == CFStringGetCString(charsetName, buffer, BUFFER_SIZE, __CFStringGetEightBitStringEncoding())) return kCFStringEncodingInvalidId;

        name = buffer;
    }

    encoding = __CFStringEncodingGetFromCanonicalName(name);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (kCFStringEncodingInvalidId == encoding) encoding = __CFStringEncodingGetFromICUName(name);
#endif
    

    // handling Java name variant for MS codepages
    if ((kCFStringEncodingInvalidId == encoding) && !strncasecmp(name, "ms950", strlen("ms950"))) { // <rdar://problem/12903398> “MS950” is not recognized
        encoding = __CFStringEncodingGetFromCanonicalName("cp950");
    }
    
    return encoding;
}

CFStringRef CFStringConvertEncodingToIANACharSetName(CFStringEncoding encoding) {
    CFStringRef name = NULL;
    CFIndex value = encoding;
    static CFMutableDictionaryRef mappingTable = NULL;
    static CFLock_t lock = CFLockInit;

    __CFLock(&lock);
    name = ((NULL == mappingTable) ? NULL : (CFStringRef)CFDictionaryGetValue(mappingTable, (const void*)value));

    if (NULL == name) {
#define STACK_BUFFER_SIZE (100)
        char buffer[STACK_BUFFER_SIZE];

        if (__CFStringEncodingGetCanonicalName(encoding, buffer, STACK_BUFFER_SIZE)) name = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingASCII);


        if (NULL != name) {
            CFIndex value = encoding;

            if (NULL == mappingTable) mappingTable = CFDictionaryCreateMutable(NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks);

            CFDictionaryAddValue(mappingTable, (const void*)value, (const void*)name);
            CFRelease(name);
        }
    }
    __CFUnlock(&lock);

    return name;
}

enum {
    NSASCIIStringEncoding = 1,		/* 0..127 only */
    NSNEXTSTEPStringEncoding = 2,
    NSJapaneseEUCStringEncoding = 3,
    NSUTF8StringEncoding = 4,
    NSISOLatin1StringEncoding = 5,
    NSSymbolStringEncoding = 6,
    NSNonLossyASCIIStringEncoding = 7,
    NSShiftJISStringEncoding = 8,
    NSISOLatin2StringEncoding = 9,
    NSUnicodeStringEncoding = 10,
    NSWindowsCP1251StringEncoding = 11,    /* Cyrillic; same as AdobeStandardCyrillic */
    NSWindowsCP1252StringEncoding = 12,    /* WinLatin1 */
    NSWindowsCP1253StringEncoding = 13,    /* Greek */
    NSWindowsCP1254StringEncoding = 14,    /* Turkish */
    NSWindowsCP1250StringEncoding = 15,    /* WinLatin2 */
    NSISO2022JPStringEncoding = 21,         /* ISO 2022 Japanese encoding for e-mail */
    NSMacOSRomanStringEncoding = 30,

    NSProprietaryStringEncoding = 65536    /* Installation-specific encoding */
};

#define NSENCODING_MASK (1 << 31)

unsigned long CFStringConvertEncodingToNSStringEncoding(CFStringEncoding theEncoding) {
    switch (theEncoding & 0xFFF) {
        case kCFStringEncodingUnicode:
            if (theEncoding == kCFStringEncodingUTF16) return NSUnicodeStringEncoding;
            else if (theEncoding == kCFStringEncodingUTF8) return NSUTF8StringEncoding;
            break;

        case kCFStringEncodingWindowsLatin1: return NSWindowsCP1252StringEncoding;
        case kCFStringEncodingMacRoman: return NSMacOSRomanStringEncoding;

        case kCFStringEncodingASCII: return NSASCIIStringEncoding;

        case kCFStringEncodingDOSJapanese: return NSShiftJISStringEncoding;
        case kCFStringEncodingWindowsCyrillic: return NSWindowsCP1251StringEncoding;
        case kCFStringEncodingWindowsGreek: return NSWindowsCP1253StringEncoding;
        case kCFStringEncodingWindowsLatin5: return NSWindowsCP1254StringEncoding;
        case kCFStringEncodingWindowsLatin2: return NSWindowsCP1250StringEncoding;
        case kCFStringEncodingISOLatin1: return NSISOLatin1StringEncoding;

        case kCFStringEncodingNonLossyASCII: return NSNonLossyASCIIStringEncoding;
        case kCFStringEncodingEUC_JP: return NSJapaneseEUCStringEncoding;
        case kCFStringEncodingMacSymbol: return NSSymbolStringEncoding;
        case kCFStringEncodingISOLatin2: return NSISOLatin2StringEncoding;
        case kCFStringEncodingISO_2022_JP: return NSISO2022JPStringEncoding;
        case kCFStringEncodingNextStepLatin: return NSNEXTSTEPStringEncoding;
    }

    return NSENCODING_MASK | theEncoding;
}

CFStringEncoding CFStringConvertNSStringEncodingToEncoding(unsigned long theEncoding) {
    const uint16_t encodings[] = {
        kCFStringEncodingASCII,
        kCFStringEncodingNextStepLatin,
        kCFStringEncodingEUC_JP,
        0,
        kCFStringEncodingISOLatin1,
        kCFStringEncodingMacSymbol,
        kCFStringEncodingNonLossyASCII,
        kCFStringEncodingDOSJapanese,
        kCFStringEncodingISOLatin2,
        kCFStringEncodingUTF16,
        kCFStringEncodingWindowsCyrillic,
        kCFStringEncodingWindowsLatin1,
        kCFStringEncodingWindowsGreek,
        kCFStringEncodingWindowsLatin5,
        kCFStringEncodingWindowsLatin2
    };

    if (NSUTF8StringEncoding == theEncoding) return kCFStringEncodingUTF8;

    if ((theEncoding > 0) && (theEncoding <= NSWindowsCP1250StringEncoding)) return encodings[theEncoding - 1];

    switch (theEncoding) {
        case NSMacOSRomanStringEncoding: return kCFStringEncodingMacRoman;
        case NSISO2022JPStringEncoding: return kCFStringEncodingISO_2022_JP;

        default:
            return ((theEncoding & NSENCODING_MASK) ? theEncoding & ~NSENCODING_MASK : kCFStringEncodingInvalidId);
    }
}

UInt32 CFStringConvertEncodingToWindowsCodepage(CFStringEncoding theEncoding) {
    uint16_t codepage = __CFStringEncodingGetWindowsCodePage(theEncoding);

    return ((0 == codepage) ? kCFStringEncodingInvalidId : codepage);
}

CFStringEncoding CFStringConvertWindowsCodepageToEncoding(UInt32 theEncoding) {
    return __CFStringEncodingGetFromWindowsCodePage(theEncoding);
}

CFStringEncoding CFStringGetMostCompatibleMacStringEncoding(CFStringEncoding encoding) {
    CFStringEncoding macEncoding = __CFStringEncodingGetMostCompatibleMacScript(encoding);


    return macEncoding;
}

#define kCFStringCompareAllocationIncrement (128)

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX

// -------------------------------------------------------------------------------------------------
//	CompareSpecials - ignore case & diacritic differences
//
//	Decomposed have 2nd-4th chars of type Mn or Mc, or in range 1160-11FF (jamo)
//	Fullwidth & halfwidth are in range FF00-FFEF
//	Parenthesized & circled are in range 3200-32FF
// -------------------------------------------------------------------------------------------------

enum {
	kUpperCaseWeightMin	= 0x80 | 0x0F,
	kUpperCaseWeightMax	= 0x80 | 0x17,
	kUpperToLowerDelta	= 0x80 | 0x0A,	// 0x0A = 0x0F - 0x05
	kMaskPrimarySecondary	= 0xFFFFFF00,
	kMaskPrimaryOnly	= 0xFFFF0000,
	kMaskSecondaryOnly	= 0x0000FF00,
	kMaskCaseTertiary	= 0x000000FF	// 2 hi bits case, 6 lo bits tertiary
};

static SInt32 __CompareSpecials(const UCollator *collator, CFOptionFlags options, const UniChar *text1Ptr, UniCharCount text1Length, const UniChar *text2Ptr, UniCharCount text2Length) {
	UErrorCode icuStatus = U_ZERO_ERROR;
	SInt32	orderWidth = 0;
	SInt32	orderCompos = 0;

	UCollationElements * collElems1 = ucol_openElements(collator, (const UChar *)text1Ptr, text1Length, &icuStatus);
	UCollationElements * collElems2 = ucol_openElements(collator, (const UChar *)text2Ptr, text2Length, &icuStatus);
	if (U_SUCCESS(icuStatus)) {
		int32_t	startOffset1 = 0;
		int32_t	startOffset2 = 0;
		
		while (true) {
			int32_t	elemOrder1, elemOrder2;
			int32_t	offset1, offset2;
			
			elemOrder1 = ucol_next(collElems1, &icuStatus);
			elemOrder2 = ucol_next(collElems2, &icuStatus);
			if ( U_FAILURE(icuStatus) || elemOrder1 == (int32_t)UCOL_NULLORDER || elemOrder2 == (int32_t)UCOL_NULLORDER ) {
				break;
			}

			offset1 = ucol_getOffset(collElems1);
			offset2 = ucol_getOffset(collElems2);
			if ( (elemOrder1 & kMaskPrimarySecondary) == (elemOrder2 & kMaskPrimarySecondary) ) {
				if ( (elemOrder1 & kMaskPrimaryOnly) != 0 ) {
					// keys may differ in case, width, circling, etc.

					int32_t	tertiary1 = (elemOrder1 & kMaskCaseTertiary);
					int32_t tertiary2 = (elemOrder2 & kMaskCaseTertiary);
					// fold upper to lower case
					if (tertiary1 >= kUpperCaseWeightMin && tertiary1 <= kUpperCaseWeightMax) {
						tertiary1 -= kUpperToLowerDelta;
					}
					if (tertiary2 >= kUpperCaseWeightMin && tertiary2 <= kUpperCaseWeightMax) {
						tertiary2 -= kUpperToLowerDelta;
					}
					// now compare
					if (tertiary1 != tertiary2) {
						orderWidth = (tertiary1 < tertiary2)? -1: 1;
						break;
					}

				} else if ( (elemOrder1 & kMaskSecondaryOnly) != 0 ) {
					// primary weights are both zero, but secondaries are not.
					if ( orderCompos == 0 && (options & kCFCompareNonliteral) == 0 ) {
						// We have a code element which is a diacritic.
						// It may have come from a composed char or a combining char.
						// If it came from a combining char (longer element length) it sorts first.
						// This is only an approximation to what the Mac OS 9 code did, but this is an
						// unusual case anyway.
						int32_t	elem1Length = offset1 - startOffset1;
						int32_t	elem2Length = offset2 - startOffset2;
						if (elem1Length != elem2Length) {
							orderCompos = (elem1Length > elem2Length)? -1: 1;
						}
					}
				}
			}
			
			startOffset1 = offset1;
			startOffset2 = offset2;
		}
		ucol_closeElements(collElems1);
		ucol_closeElements(collElems2);
	}
	
	return (orderWidth != 0)? orderWidth: orderCompos;
}

static SInt32 __CompareCodePoints(const UniChar *text1Ptr, UniCharCount text1Length, const UniChar *text2Ptr, UniCharCount text2Length ) {
	const UniChar *	text1P = text1Ptr;
	const UniChar *	text2P = text2Ptr;
	UInt32		textLimit = (text1Length <= text2Length)? text1Length: text2Length;
	UInt32		textCounter;
	SInt32		orderResult = 0;

	// Loop through either string...the first difference differentiates this.
	for (textCounter = 0; textCounter < textLimit && *text1P == *text2P; textCounter++) {
		text1P++;
		text2P++;
	}
	if (textCounter < textLimit) {
		// code point difference
		orderResult = (*text1P < *text2P) ? -1 : 1;
	} else if (text1Length != text2Length) {
		// one string has extra stuff at end
		orderResult = (text1Length < text2Length) ? -1 : 1;
	}
	return orderResult;
}


extern const CFStringRef __kCFLocaleCollatorID;

static UCollator *__CFStringCreateCollator(CFLocaleRef compareLocale) {
    CFStringRef canonLocaleCFStr = (CFStringRef)CFLocaleGetValue(compareLocale, __kCFLocaleCollatorID);
    char icuLocaleStr[128] = {0};
    CFStringGetCString(canonLocaleCFStr, icuLocaleStr, sizeof(icuLocaleStr), kCFStringEncodingASCII);
    UErrorCode icuStatus = U_ZERO_ERROR;
    UCollator * collator = ucol_open(icuLocaleStr, &icuStatus);
    ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_OFF, &icuStatus);
    ucol_setAttribute(collator, UCOL_ALTERNATE_HANDLING, UCOL_NON_IGNORABLE, &icuStatus);
    ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_PRIMARY, &icuStatus);
    ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_OFF, &icuStatus);
    ucol_setAttribute(collator, UCOL_NUMERIC_COLLATION, UCOL_OFF, &icuStatus);
    return collator;
}

#define kCFMaxCachedDefaultCollators (8)
static UCollator *__CFDefaultCollators[kCFMaxCachedDefaultCollators];
static CFIndex __CFDefaultCollatorsCount = 0;
static const void *__CFDefaultCollatorLocale = NULL;
static CFLock_t __CFDefaultCollatorLock = CFLockInit;

static UCollator *__CFStringCopyDefaultCollator(CFLocaleRef compareLocale) {
    CFLocaleRef currentLocale = NULL;
    UCollator * collator = NULL;

    if (compareLocale != __CFDefaultCollatorLocale) {
        currentLocale = CFLocaleCopyCurrent();
        if (compareLocale != currentLocale) {
	    CFRelease(currentLocale);
	    return NULL;
	}
    }

    __CFLock(&__CFDefaultCollatorLock);
    if ((NULL != currentLocale) && (__CFDefaultCollatorLocale != currentLocale)) {
        while (__CFDefaultCollatorsCount > 0) ucol_close(__CFDefaultCollators[--__CFDefaultCollatorsCount]);
        __CFDefaultCollatorLocale = CFRetain(currentLocale);
    }

    if (__CFDefaultCollatorsCount > 0) collator = __CFDefaultCollators[--__CFDefaultCollatorsCount];
    __CFUnlock(&__CFDefaultCollatorLock);

    if (NULL == collator) {
	collator = __CFStringCreateCollator(compareLocale);
    }

    if (NULL != currentLocale) CFRelease(currentLocale);

    return collator;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
static void __collatorFinalize(UCollator *collator) {
    CFLocaleRef locale = _CFGetTSD(__CFTSDKeyCollatorLocale);
    _CFSetTSD(__CFTSDKeyCollatorUCollator, NULL, NULL);
    _CFSetTSD(__CFTSDKeyCollatorLocale, NULL, NULL);
    __CFLock(&__CFDefaultCollatorLock);
    if ((__CFDefaultCollatorLocale == locale) && (__CFDefaultCollatorsCount < kCFMaxCachedDefaultCollators)) {
        __CFDefaultCollators[__CFDefaultCollatorsCount++] = collator;
        collator = NULL;
    }
    __CFUnlock(&__CFDefaultCollatorLock);
    if (NULL != collator) ucol_close(collator);
    if (locale) CFRelease(locale);
}
#endif

// -------------------------------------------------------------------------------------------------
// __CompareTextDefault
// 
// A primary difference is denoted by values 2/-2 in orderP. Other differences are indicated with a -1/1.
// A negative value indicates that text1 sorts before text2.
// -------------------------------------------------------------------------------------------------
static OSStatus __CompareTextDefault(UCollator *collator, CFOptionFlags options, const UniChar *text1Ptr, UniCharCount text1Length, const UniChar *text2Ptr, UniCharCount text2Length, Boolean *equivalentP, SInt32 *orderP) {

	// collator must have default settings restored on exit from this function

	*equivalentP = true;
	*orderP = 0;	

	if (options & kCFCompareNumerically) {
	    UErrorCode icuStatus = U_ZERO_ERROR;	
	    ucol_setAttribute(collator, UCOL_NUMERIC_COLLATION, UCOL_ON, &icuStatus);
	}

	// Most string differences are Primary. Do a primary check first, then if there
	// are no differences do a comparison with the options in the collator.
	UCollationResult icuResult = ucol_strcoll(collator, (const UChar *)text1Ptr, text1Length, (const UChar *)text2Ptr, text2Length);
	if (icuResult != UCOL_EQUAL) {
		*orderP = (icuResult == UCOL_LESS) ? -2 : 2;
	}
	if (*orderP == 0) {
		UErrorCode icuStatus = U_ZERO_ERROR;	
                ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_ON, &icuStatus);
                ucol_setAttribute(collator, UCOL_STRENGTH, (options & kCFCompareDiacriticInsensitive) ? UCOL_PRIMARY : UCOL_SECONDARY, &icuStatus);
                ucol_setAttribute(collator, UCOL_CASE_LEVEL, (options & kCFCompareCaseInsensitive) ? UCOL_OFF : UCOL_ON, &icuStatus);
		if (!U_SUCCESS(icuStatus)) {
		    icuStatus = U_ZERO_ERROR;
		    ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_OFF, &icuStatus);
		    ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_PRIMARY, &icuStatus);
		    ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_OFF, &icuStatus);
		    ucol_setAttribute(collator, UCOL_NUMERIC_COLLATION, UCOL_OFF, &icuStatus);
		    return 666;
		}

		// We don't have a primary difference. Recompare with standard collator.
		icuResult = ucol_strcoll(collator, (const UChar *)text1Ptr, text1Length, (const UChar *)text2Ptr, text2Length);
		if (icuResult != UCOL_EQUAL) {
			*orderP = (icuResult == UCOL_LESS) ? -1 : 1;
		}
		icuStatus = U_ZERO_ERROR;
                ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_OFF, &icuStatus);
		ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_PRIMARY, &icuStatus);
		ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_OFF, &icuStatus);
	}
	if (*orderP == 0 && (options & kCFCompareNonliteral) == 0) {
		*orderP = __CompareSpecials(collator, options, text1Ptr, text1Length, text2Ptr, text2Length);
	}

	*equivalentP = (*orderP == 0);

	// If strings are equivalent but we care about order and have not yet checked
	// to the level of code point order, then do some more checks for order
	if (*orderP == 0) {
		UErrorCode icuStatus = U_ZERO_ERROR;	
		// First try to see if ICU can find any differences above code point level
                ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_ON, &icuStatus);
		ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_TERTIARY, &icuStatus);
		ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_ON, &icuStatus);
		if (!U_SUCCESS(icuStatus)) {
		    icuStatus = U_ZERO_ERROR;
		    ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_OFF, &icuStatus);
		    ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_PRIMARY, &icuStatus);
		    ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_OFF, &icuStatus);
		    ucol_setAttribute(collator, UCOL_NUMERIC_COLLATION, UCOL_OFF, &icuStatus);
		    return 666;
		}
		icuResult = ucol_strcoll(collator, (const UChar *)text1Ptr, text1Length, (const UChar *)text2Ptr, text2Length);
		if (icuResult != UCOL_EQUAL) {
			*orderP = (icuResult == UCOL_LESS) ? -1 : 1;
		} else {
			// no ICU differences above code point level, compare code points
			*orderP = __CompareCodePoints( text1Ptr, text1Length, text2Ptr, text2Length );
		}
		icuStatus = U_ZERO_ERROR;
                ucol_setAttribute(collator, UCOL_NORMALIZATION_MODE, UCOL_OFF, &icuStatus);
		ucol_setAttribute(collator, UCOL_STRENGTH, UCOL_PRIMARY, &icuStatus);
		ucol_setAttribute(collator, UCOL_CASE_LEVEL, UCOL_OFF, &icuStatus);
	}

	if (options & kCFCompareNumerically) {
	    UErrorCode icuStatus = U_ZERO_ERROR;	
	    ucol_setAttribute(collator, UCOL_NUMERIC_COLLATION, UCOL_OFF, &icuStatus);
	}
	return 0; // noErr
}

#endif // DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX

static inline CFIndex __extendLocationBackward(CFIndex location, CFStringInlineBuffer *str, const uint8_t *nonBaseBMP, const uint8_t *punctBMP) {
    while (location > 0) {
        UTF32Char ch = CFStringGetCharacterFromInlineBuffer(str, location);
        UTF32Char otherChar;
        if (CFUniCharIsSurrogateLowCharacter(ch) && CFUniCharIsSurrogateHighCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(str, location - 1)))) {
            ch = CFUniCharGetLongCharacterForSurrogatePair(ch, otherChar);
            uint8_t planeNo = (ch >> 16);
            if ((planeNo > 1) || (!CFUniCharIsMemberOfBitmap(ch, CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, planeNo)) && !CFUniCharIsMemberOfBitmap(ch, CFUniCharGetBitmapPtrForPlane(kCFUniCharPunctuationCharacterSet, planeNo)))) break;
            location -= 2;
        } else {
            if ((!CFUniCharIsMemberOfBitmap(ch, nonBaseBMP) && !CFUniCharIsMemberOfBitmap(ch, punctBMP)) || ((ch >= 0x2E80) && (ch < 0xAC00))) break;
            --location;
        }
    }

    return location;
}

static inline CFIndex __extendLocationForward(CFIndex location, CFStringInlineBuffer *str, const uint8_t *alnumBMP, const uint8_t *punctBMP, const uint8_t *controlBMP, CFIndex strMax) {
    do {
        UTF32Char ch = CFStringGetCharacterFromInlineBuffer(str, location);
        UTF32Char otherChar;
        if (CFUniCharIsSurrogateHighCharacter(ch) && CFUniCharIsSurrogateLowCharacter((otherChar = CFStringGetCharacterFromInlineBuffer(str, location + 1)))) {
            ch = CFUniCharGetLongCharacterForSurrogatePair(ch, otherChar);
            location += 2;
            uint8_t planeNo = (ch >> 16);
            if (!CFUniCharIsMemberOfBitmap(ch, CFUniCharGetBitmapPtrForPlane(kCFUniCharAlphaNumericCharacterSet, planeNo)) && !CFUniCharIsMemberOfBitmap(ch, CFUniCharGetBitmapPtrForPlane(kCFUniCharPunctuationCharacterSet, planeNo)) && !CFUniCharIsMemberOfBitmap(ch, CFUniCharGetBitmapPtrForPlane(kCFUniCharControlAndFormatterCharacterSet, planeNo))) break;
        } else {
            ++location;
            if ((!CFUniCharIsMemberOfBitmap(ch, alnumBMP) && !CFUniCharIsMemberOfBitmap(ch, punctBMP) && !CFUniCharIsMemberOfBitmap(ch, controlBMP)) || ((ch >= 0x2E80) && (ch < 0xAC00))) break;
        }
    } while (location < strMax);
    return location;
}

CF_PRIVATE CFComparisonResult _CFCompareStringsWithLocale(CFStringInlineBuffer *str1, CFRange str1Range, CFStringInlineBuffer *str2, CFRange str2Range, CFOptionFlags options, const void *compareLocale) {
    const UniChar *characters1;
    const UniChar *characters2;
    CFComparisonResult compResult = kCFCompareEqualTo;
    CFRange range1 = str1Range;
    CFRange range2 = str2Range;
    SInt32 order;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    Boolean isEqual;
    bool forcedOrdering = ((options & kCFCompareForcedOrdering) ? true : false);

    UCollator *collator = NULL;
    bool defaultCollator = true;
#endif
    static const uint8_t *alnumBMP = NULL;
    static const uint8_t *nonBaseBMP = NULL;
    static const uint8_t *punctBMP = NULL;
    static const uint8_t *controlBMP = NULL;
    
    if (NULL == alnumBMP) {
	alnumBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharAlphaNumericCharacterSet, 0);
	nonBaseBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, 0);
	punctBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharPunctuationCharacterSet, 0);
	controlBMP = CFUniCharGetBitmapPtrForPlane(kCFUniCharControlAndFormatterCharacterSet, 0);
    }
    
    // Determine the range of characters surrodiing the current index significant for localized comparison. The range is extended backward and forward as long as they are contextual. Contextual characters include all letters and punctuations. Since most control/format characters are ignorable in localized comparison, we also include them extending forward.
    
    range1.location = str1Range.location;
    range2.location = str2Range.location;
    
    // go backward
    // The characters upto the current index are already determined to be equal by the CFString's standard character folding algorithm. Extend as long as truly contextual (all letters and punctuations).
    if (range1.location > 0) {
	range1.location = __extendLocationBackward(range1.location - 1, str1, nonBaseBMP, punctBMP);
    }
    
    if (range2.location > 0) {
	range2.location = __extendLocationBackward(range2.location - 1, str2, nonBaseBMP, punctBMP);
    }
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    // First we try to use the last one used on this thread, if the locale is the same,
    // otherwise we try to check out a default one, or then we create one.
    UCollator *threadCollator = _CFGetTSD(__CFTSDKeyCollatorUCollator);
    CFLocaleRef threadLocale = _CFGetTSD(__CFTSDKeyCollatorLocale);
    if (compareLocale == threadLocale) {
	collator = threadCollator;
    } else {
#endif
	collator = __CFStringCopyDefaultCollator((CFLocaleRef)compareLocale);
	defaultCollator = true;
	if (NULL == collator) {
	    collator = __CFStringCreateCollator((CFLocaleRef)compareLocale);
	    defaultCollator = false;
	}
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    }
#endif
#endif
    
    characters1 = CFStringGetCharactersPtrFromInlineBuffer(str1, range1);
    characters2 = CFStringGetCharactersPtrFromInlineBuffer(str2, range2);

    if ((NULL != characters1) && (NULL != characters2)) { // do fast
	range1.length = (str1Range.location + str1Range.length) - range1.location;
	range2.length = (str2Range.location + str2Range.length) - range2.location;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
        if ((NULL != collator) && (__CompareTextDefault(collator, options, characters1, range1.length, characters2, range2.length, &isEqual, &order) == 0 /* noErr */)) {
            compResult = ((isEqual && !forcedOrdering) ? kCFCompareEqualTo : ((order < 0) ? kCFCompareLessThan : kCFCompareGreaterThan));
        } else 
#endif
        {
            compResult = ((memcmp(characters1, characters2, sizeof(UniChar) * range1.length) < 0) ? kCFCompareLessThan : kCFCompareGreaterThan);
        }
    } else {
        UniChar *buffer1 = NULL;
        UniChar *buffer2 = NULL;
        UTF16Char sBuffer1[kCFStringCompareAllocationIncrement];
        UTF16Char sBuffer2[kCFStringCompareAllocationIncrement];
        CFIndex buffer1Len = 0, buffer2Len = 0;
        CFIndex str1Max = str1Range.location + str1Range.length;
        CFIndex str2Max = str2Range.location + str2Range.length;
        CFIndex bufferSize;

        // Extend forward and compare until the result is deterministic. The result is indeterministic if the differences are weak and can be resolved by character folding. For example, comparision between "abc" and "ABC" is considered to be indeterministic.
        do {
            if (str1Range.location < str1Max) {
		str1Range.location = __extendLocationForward(str1Range.location, str1, alnumBMP, punctBMP, controlBMP, str1Max);
                range1.length = (str1Range.location - range1.location);
                characters1 = CFStringGetCharactersPtrFromInlineBuffer(str1, range1);

                if (NULL == characters1) {
                    if ((0 > buffer1Len) || (range1.length > kCFStringCompareAllocationIncrement)) {
                        if (buffer1Len < range1.length) {
                            bufferSize = range1.length + (kCFStringCompareAllocationIncrement - (range1.length % kCFStringCompareAllocationIncrement));
                            if (0 == buffer1Len) {
                                buffer1 = (UniChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UTF16Char) * bufferSize, 0);
                            } else if (buffer1Len < range1.length) {
                                buffer1 = (UniChar *)CFAllocatorReallocate(kCFAllocatorSystemDefault, buffer1, sizeof(UTF16Char) * bufferSize, 0);
                            }
                            buffer1Len = bufferSize;
                        }
                    } else {
                        buffer1 = sBuffer1;
                    }

                    CFStringGetCharactersFromInlineBuffer(str1, range1, buffer1);
                    characters1 = buffer1;
                }
            }

            if (str2Range.location < str2Max) {
		str2Range.location = __extendLocationForward(str2Range.location, str2, alnumBMP, punctBMP, controlBMP, str2Max);                
                range2.length = (str2Range.location - range2.location);
                characters2 = CFStringGetCharactersPtrFromInlineBuffer(str2, range2);
                
                if (NULL == characters2) {
                    if ((0 > buffer2Len) || (range2.length > kCFStringCompareAllocationIncrement)) {
                        if (buffer2Len < range2.length) {
                            bufferSize = range2.length + (kCFStringCompareAllocationIncrement - (range2.length % kCFStringCompareAllocationIncrement));
                            if (0 == buffer2Len) {
                                buffer2 = (UniChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(UTF16Char) * bufferSize, 0);
                            } else if (buffer2Len < range2.length) {
                                buffer2 = (UniChar *)CFAllocatorReallocate(kCFAllocatorSystemDefault, buffer2, sizeof(UTF16Char) * bufferSize, 0);
                            }
                            buffer2Len = bufferSize;
                        }
                    } else {
                        buffer2 = sBuffer2;
                    }

                    CFStringGetCharactersFromInlineBuffer(str2, range2, buffer2);
                    characters2 = buffer2;
                }
            }

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
            if ((NULL != collator) && (__CompareTextDefault(collator, options, characters1, range1.length, characters2, range2.length, &isEqual, &order) ==  0 /* noErr */)) {
                if (isEqual) {
                    if (forcedOrdering && (kCFCompareEqualTo == compResult) && (0 != order)) compResult = ((order < 0) ? kCFCompareLessThan : kCFCompareGreaterThan);
                    order = 0;
                }
            } else 
#endif
            {
                order = memcmp(characters1, characters2, sizeof(UTF16Char) * ((range1.length < range2.length) ? range1.length : range2.length));
                if (0 == order) {
                    if (range1.length < range2.length) {
                        order = -2;
                    } else if (range2.length < range1.length) {
                        order = 2;
                    }
                } else if (order < 0) {
                    --order;
                } else if (order > 0) {
                    ++order;
                }
            }

            if ((order < -1) || (order > 1)) break; // the result is deterministic

            if (0 == order) {
                range1.location = str1Range.location;
                range2.location = str2Range.location;
            }
        } while ((str1Range.location < str1Max) || (str2Range.location < str2Max));

        if (0 != order) compResult = ((order < 0) ? kCFCompareLessThan : kCFCompareGreaterThan);

        if (buffer1Len > 0) CFAllocatorDeallocate(kCFAllocatorSystemDefault, buffer1);
        if (buffer2Len > 0) CFAllocatorDeallocate(kCFAllocatorSystemDefault, buffer2);
    }

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if (collator == threadCollator) {
	// do nothing, already cached
    } else {
	if (threadLocale) __collatorFinalize((UCollator *)_CFGetTSD(__CFTSDKeyCollatorUCollator)); // need to dealloc collators

	_CFSetTSD(__CFTSDKeyCollatorUCollator, collator, (void *)__collatorFinalize);
	_CFSetTSD(__CFTSDKeyCollatorLocale, (void *)CFRetain(compareLocale), NULL);
    }
#endif
    
    return compResult;
}

