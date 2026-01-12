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

/*	CFStringEncodingDatabase.c
	Copyright (c) 2005-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFInternal.h"
#include <CoreFoundation/CFStringEncodingExt.h>
#include "CFStringEncodingConverterPriv.h"
#include "CFStringEncodingDatabase.h"
#include <stdio.h>

#define ISO8859CODEPAGE_BASE (28590)

static const uint16_t __CFKnownEncodingList[] = {
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacChineseTrad,
    kCFStringEncodingMacKorean,
    kCFStringEncodingMacArabic,
    kCFStringEncodingMacHebrew,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacCyrillic,
    kCFStringEncodingMacDevanagari,
    kCFStringEncodingMacGurmukhi,
    kCFStringEncodingMacGujarati,
    kCFStringEncodingMacOriya,
    kCFStringEncodingMacBengali,
    kCFStringEncodingMacTamil,
    kCFStringEncodingMacTelugu,
    kCFStringEncodingMacKannada,
    kCFStringEncodingMacMalayalam,
    kCFStringEncodingMacSinhalese,
    kCFStringEncodingMacBurmese,
    kCFStringEncodingMacKhmer,
    kCFStringEncodingMacThai,
    kCFStringEncodingMacLaotian,
    kCFStringEncodingMacGeorgian,
    kCFStringEncodingMacArmenian,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacTibetan,
    kCFStringEncodingMacMongolian,
    kCFStringEncodingMacEthiopic,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacVietnamese,
    kCFStringEncodingMacSymbol,
    kCFStringEncodingMacDingbats,
    kCFStringEncodingMacTurkish,
    kCFStringEncodingMacCroatian,
    kCFStringEncodingMacIcelandic,
    kCFStringEncodingMacRomanian,
    kCFStringEncodingMacCeltic,
    kCFStringEncodingMacGaelic,
    kCFStringEncodingMacFarsi,
    kCFStringEncodingMacUkrainian,
    kCFStringEncodingMacInuit,
    
    kCFStringEncodingDOSLatinUS,
    kCFStringEncodingDOSGreek,
    kCFStringEncodingDOSBalticRim,
    kCFStringEncodingDOSLatin1,
    kCFStringEncodingDOSGreek1,
    kCFStringEncodingDOSLatin2,
    kCFStringEncodingDOSCyrillic,
    kCFStringEncodingDOSTurkish,
    kCFStringEncodingDOSPortuguese,
    kCFStringEncodingDOSIcelandic,
    kCFStringEncodingDOSHebrew,
    kCFStringEncodingDOSCanadianFrench,
    kCFStringEncodingDOSArabic,
    kCFStringEncodingDOSNordic,
    kCFStringEncodingDOSRussian,
    kCFStringEncodingDOSGreek2,
    kCFStringEncodingDOSThai,
    kCFStringEncodingDOSJapanese,
    kCFStringEncodingDOSChineseSimplif,
    kCFStringEncodingDOSKorean,
    kCFStringEncodingDOSChineseTrad,
    
    kCFStringEncodingWindowsLatin1,
    kCFStringEncodingWindowsLatin2,
    kCFStringEncodingWindowsCyrillic,
    kCFStringEncodingWindowsGreek,
    kCFStringEncodingWindowsLatin5,
    kCFStringEncodingWindowsHebrew,
    kCFStringEncodingWindowsArabic,
    kCFStringEncodingWindowsBalticRim,
    kCFStringEncodingWindowsVietnamese,
    kCFStringEncodingWindowsKoreanJohab,
    kCFStringEncodingASCII,
    
    kCFStringEncodingShiftJIS_X0213,
    kCFStringEncodingGB_18030_2000,
    
    kCFStringEncodingISO_2022_JP,
    kCFStringEncodingISO_2022_JP_2,
    kCFStringEncodingISO_2022_JP_1,
    kCFStringEncodingISO_2022_JP_3,
    kCFStringEncodingISO_2022_CN,
    kCFStringEncodingISO_2022_CN_EXT,
    kCFStringEncodingISO_2022_KR,
    kCFStringEncodingEUC_JP,
    kCFStringEncodingEUC_CN,
    kCFStringEncodingEUC_TW,
    kCFStringEncodingEUC_KR,
    
    kCFStringEncodingShiftJIS,

    kCFStringEncodingKOI8_R,

    kCFStringEncodingBig5,

    kCFStringEncodingMacRomanLatin1,
    kCFStringEncodingHZ_GB_2312,
    kCFStringEncodingBig5_HKSCS_1999,
    kCFStringEncodingVISCII,
    kCFStringEncodingKOI8_U,
    kCFStringEncodingBig5_E,
    kCFStringEncodingUTF7_IMAP,
    
    kCFStringEncodingNextStepLatin,
    
    kCFStringEncodingEBCDIC_CP037
};

// Windows codepage mapping
static const uint16_t __CFWindowsCPList[] = {
    10000,
    10001,
    10002,
    10003,
    10004,
    10005,
    10006,
    10007,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    10021,
    0,
    0,
    0,
    10008,
    0,
    0,
    0,
    10029,
    0,
    0,
    0,
    10081,
    10082,
    10079,
    10010,
    0,
    0,
    0,
    10017,
    0,
    
    437,
    737,
    775,
    850,
    851,
    852,
    855,
    857,
    860,
    861,
    862,
    863,
    864,
    865,
    866,
    869,
    874,
    932,
    936,
    949,
    950,
    
    1252,
    1250,
    1251,
    1253,
    1254,
    1255,
    1256,
    1257,
    1258,
    1361,

    20127,
    
    0,
    54936,
    
    50221, // we prefere this over 50220/50221 since that's what CF coverter generates
    0,
    0,
    0,
    50227,
    0,
    50225,
    
    51932,
    51936,
    51950,
    51949,
    
    0,

    20866,

    0,

    0,
    52936,
    0,
    0,
    21866,
    0,
    0,
    
    0,
    
    37
};

// Canonical name
static const char *__CFCanonicalNameList[] = {
    "macintosh",
    "japanese",
    "trad-chinese",
    "korean",
    "arabic",
    "hebrew",
    "greek",
    "cyrillic",
    "devanagari",
    "gurmukhi",
    "gujarati",
    "oriya",
    "bengali",
    "tamil",
    "telugu",
    "kannada",
    "malayalam",
    "sinhalese",
    "burmese",
    "khmer",
    "thai",
    "laotian",
    "georgian",
    "armenian",
    "simp-chinese",
    "tibetan",
    "mongolian",
    "ethiopic",
    "centraleurroman",
    "vietnamese",
    "symbol",
    "dingbats",
    "turkish",
    "croatian",
    "icelandic",
    "romanian",
    "celtic",
    "gaelic",
    "farsi",
    "ukrainian",
    "inuit",
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    
    "us-ascii",
    
    NULL,
    "gb18030",
    
    "iso-2022-jp",
    "iso-2022-jp-2",
    "iso-2022-jp-1",
    "iso-2022-jp-3",
    "iso-2022-cn",
    "iso-2022-cn-ext",
    "iso-2022-kr",
    "euc-jp",
    "gb2312",
    "euc-tw",
    "euc-kr",
    
    "shift_jis",

    "koi8-r",

    "big5",

    "roman-latin1",
    "hz-gb-2312",
    "big5-hkscs",
    "viscii",
    "koi8-u",
    NULL,
    "utf7-imap",
    
    "x-nextstep",
    
    "ibm037",
};

static inline CFIndex __CFGetEncodingIndex(CFStringEncoding encoding) {
    const uint16_t *head = __CFKnownEncodingList;
    const uint16_t *tail = head + ((sizeof(__CFKnownEncodingList) / sizeof(*__CFKnownEncodingList)) - 1);
    const uint16_t *middle;

    encoding &= 0x0FFF;
    while (head <= tail) {
        middle = head + ((tail - head) >> 1);

        if (encoding == *middle) {
            return middle - __CFKnownEncodingList;
        } else if (encoding < *middle) {
            tail = middle - 1;
        } else {
            head = middle + 1;
        }
    }

    return kCFNotFound;
}

CF_PRIVATE uint16_t __CFStringEncodingGetWindowsCodePage(CFStringEncoding encoding) {
    CFStringEncoding encodingBase = encoding & 0x0F00;

    if (0x0100 == encodingBase) { // UTF
        switch (encoding) {
            case kCFStringEncodingUTF7: return 65000;
            case kCFStringEncodingUTF8: return 65001;
            case kCFStringEncodingUTF16: return 1200;
            case kCFStringEncodingUTF16BE: return 1201;
            case kCFStringEncodingUTF32: return 65005;
            case kCFStringEncodingUTF32BE: return 65006;
        }        
    } else if (0x0200 == encodingBase) { // ISO 8859 range
        return ISO8859CODEPAGE_BASE + (encoding & 0xFF);
    } else { // others
        CFIndex index = __CFGetEncodingIndex(encoding);

        if (kCFNotFound != index) return __CFWindowsCPList[index];
    }

    return 0;
}

CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromWindowsCodePage(uint16_t codepage) {
    switch (codepage) {
        case 65001: return kCFStringEncodingUTF8;
        case 1200: return kCFStringEncodingUTF16;
        case 0: return kCFStringEncodingInvalidId;
        case 1201: return kCFStringEncodingUTF16BE;
        case 65005: return kCFStringEncodingUTF32;
        case 65006: return kCFStringEncodingUTF32BE;
        case 65000: return kCFStringEncodingUTF7;
    }

    if ((codepage > ISO8859CODEPAGE_BASE) && (codepage <= (ISO8859CODEPAGE_BASE + 16))) {
        return (codepage - ISO8859CODEPAGE_BASE) + 0x0200;
    } else {
        static CFMutableDictionaryRef mappingTable = NULL;
        static CFLock_t lock = CFLockInit;
        uintptr_t value;

        __CFLock(&lock);
        if (NULL == mappingTable) {
            CFIndex index, count = sizeof(__CFKnownEncodingList) / sizeof(*__CFKnownEncodingList);
            
            mappingTable = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

            for (index = 0;index < count;index++) {
                if (0 != __CFWindowsCPList[index]) CFDictionarySetValue(mappingTable, (const void *)(uintptr_t)__CFWindowsCPList[index], (const void *)(uintptr_t)__CFKnownEncodingList[index]);
            }
        }
        __CFUnlock(&lock);

        if (CFDictionaryGetValueIfPresent(mappingTable, (const void *)(uintptr_t)codepage, (const void **)&value)) return (CFStringEncoding)value;
    }


    return kCFStringEncodingInvalidId;
}

CF_PRIVATE bool __CFStringEncodingGetCanonicalName(CFStringEncoding encoding, char *buffer, CFIndex bufferSize) {
    const char *format = "%s";
    const char *name = NULL;
    uint32_t value = 0;
    CFIndex index;

    switch (encoding & 0x0F00) {
        case 0x0100: // UTF range
            switch (encoding) {
                case kCFStringEncodingUTF7: name = "utf-7"; break;
                case kCFStringEncodingUTF8: name = "utf-8"; break;
                case kCFStringEncodingUTF16: name = "utf-16"; break;
                case kCFStringEncodingUTF16BE: name = "utf-16be"; break;
                case kCFStringEncodingUTF16LE: name = "utf-16le"; break;
                case kCFStringEncodingUTF32: name = "utf-32"; break;
                case kCFStringEncodingUTF32BE: name = "utf-32be"; break;
                case kCFStringEncodingUTF32LE: name = "utf-32le"; break;
            }
            break;

        case 0x0200: // ISO 8859 range
            format = "iso-8859-%d";
            value = (encoding & 0xFF);
            break;

        case 0x0400: // DOS code page range
        case 0x0500: // Windows code page range
            index = __CFGetEncodingIndex(encoding);
            
            if (kCFNotFound != index) {
                value = __CFWindowsCPList[index];
                if (0 != value) format = ((0x0400 == (encoding & 0x0F00)) ? "cp%d" : "windows-%d");
            }
            break;

        default: // others
            index = __CFGetEncodingIndex(encoding);

            if (kCFNotFound != index) {
                if (((0 == (encoding & 0x0F00)) && (kCFStringEncodingMacRoman != encoding)) || (kCFStringEncodingMacRomanLatin1 == encoding)) format = "x-mac-%s";
                name = (const char *)__CFCanonicalNameList[index];
            }
            break;
    }

    if ((0 == value) && (NULL == name)) {
        return false;
    } else if (0 != value) {
        return ((snprintf(buffer, bufferSize, format, value) < bufferSize) ? true : false);
    } else {
        return ((snprintf(buffer, bufferSize, format, name) < bufferSize) ? true : false);
    }
}

#define LENGTH_LIMIT (256)
static Boolean __CFCanonicalNameCompare(const void *value1, const void *value2) { return ((0 == strncasecmp_l((const char *)value1, (const char *)value2, LENGTH_LIMIT, NULL)) ? true : false); }

static CFHashCode __CFCanonicalNameHash(const void *value) {
    const char *name = (const char *)value;
    CFHashCode code = 0;

    while ((0 != *name) && ((name - (const char *)value) < LENGTH_LIMIT)) {
        char character = *(name++);

        code += (character + (((character >= 'A') && (character <= 'Z')) ? 'a' - 'A' : 0));
    }

    return code * (name - (const char *)value);
}

CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromCanonicalName(const char *canonicalName) {
    CFStringEncoding encoding;
    CFIndex prefixLength;
    static CFMutableDictionaryRef mappingTable = NULL;
    static CFLock_t lock = CFLockInit;

    prefixLength = strlen("iso-8859-");
    if (0 == strncasecmp_l(canonicalName, "iso-8859-", prefixLength, NULL)) {// do ISO
        encoding = strtol(canonicalName + prefixLength, NULL, 10);

        return (((0 == encoding) || (encoding > 16)) ? kCFStringEncodingInvalidId : encoding + 0x0200);
    }

    prefixLength = strlen("cp");
    if (0 == strncasecmp_l(canonicalName, "cp", prefixLength, NULL)) {// do DOS
        encoding = strtol(canonicalName + prefixLength, NULL, 10);

        return __CFStringEncodingGetFromWindowsCodePage(encoding);
    }

    prefixLength = strlen("windows-");
    if (0 == strncasecmp_l(canonicalName, "windows-", prefixLength, NULL)) {// do DOS
        encoding = strtol(canonicalName + prefixLength, NULL, 10);
        
        return __CFStringEncodingGetFromWindowsCodePage(encoding);
    }
    
    __CFLock(&lock);
    if (NULL == mappingTable) {
        CFIndex index, count = sizeof(__CFKnownEncodingList) / sizeof(*__CFKnownEncodingList);

        CFDictionaryKeyCallBacks keys = {
            0, NULL, NULL, NULL, &__CFCanonicalNameCompare, &__CFCanonicalNameHash
        };

        mappingTable = CFDictionaryCreateMutable(NULL, 0, &keys, NULL);

        // Add UTFs
        CFDictionarySetValue(mappingTable, "utf-7", (const void *)kCFStringEncodingUTF7);
        CFDictionarySetValue(mappingTable, "utf-8", (const void *)kCFStringEncodingUTF8);
        CFDictionarySetValue(mappingTable, "utf-16", (const void *)kCFStringEncodingUTF16);
        CFDictionarySetValue(mappingTable, "utf-16be", (const void *)kCFStringEncodingUTF16BE);
        CFDictionarySetValue(mappingTable, "utf-16le", (const void *)kCFStringEncodingUTF16LE);
        CFDictionarySetValue(mappingTable, "utf-32", (const void *)kCFStringEncodingUTF32);
        CFDictionarySetValue(mappingTable, "utf-32be", (const void *)kCFStringEncodingUTF32BE);
        CFDictionarySetValue(mappingTable, "utf-32le", (const void *)kCFStringEncodingUTF32LE);

        for (index = 0;index < count;index++) {
            if (NULL != __CFCanonicalNameList[index]) CFDictionarySetValue(mappingTable, (const void *)(uintptr_t)__CFCanonicalNameList[index], (const void *)(uintptr_t)__CFKnownEncodingList[index]);
        }
    }
    __CFUnlock(&lock);

    if (0 == strncasecmp_l(canonicalName, "macintosh", sizeof("macintosh") - 1, NULL)) return kCFStringEncodingMacRoman;

    
    prefixLength = strlen("x-mac-");
    encoding = (CFStringEncoding)(CFIndex)CFDictionaryGetValue(mappingTable, canonicalName + ((0 == strncasecmp_l(canonicalName, "x-mac-", prefixLength, NULL)) ? prefixLength : 0));

    return ((0 == encoding) ? kCFStringEncodingInvalidId : encoding);
}
#undef LENGTH_LIMIT

#if DEPLOYMENT_TARGET_MACOSX
// This list indexes from DOS range
static uint16_t __CFISO8859SimilarScriptList[] = {
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacCyrillic,
    kCFStringEncodingMacArabic,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacHebrew,
    kCFStringEncodingMacTurkish,
    kCFStringEncodingMacInuit,
    kCFStringEncodingMacThai,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacCeltic,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacRomanian};

static uint16_t __CFOtherSimilarScriptList[] = {
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacCyrillic,
    kCFStringEncodingMacTurkish,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacIcelandic,
    kCFStringEncodingMacHebrew,
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacArabic,
    kCFStringEncodingMacInuit,
    kCFStringEncodingMacCyrillic,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacThai,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacKorean,
    kCFStringEncodingMacChineseTrad,
    
    kCFStringEncodingMacRoman,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacCyrillic,
    kCFStringEncodingMacGreek,
    kCFStringEncodingMacTurkish,
    kCFStringEncodingMacHebrew,
    kCFStringEncodingMacArabic,
    kCFStringEncodingMacCentralEurRoman,
    kCFStringEncodingMacVietnamese,
    kCFStringEncodingMacKorean,

    kCFStringEncodingMacRoman,

    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacChineseSimp,
    
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacKorean,
    kCFStringEncodingMacJapanese,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacChineseTrad,
    kCFStringEncodingMacKorean,

    kCFStringEncodingMacJapanese,

    kCFStringEncodingMacCyrillic,

    kCFStringEncodingMacChineseTrad,

    kCFStringEncodingMacRoman,
    kCFStringEncodingMacChineseSimp,
    kCFStringEncodingMacChineseTrad,
    kCFStringEncodingMacVietnamese,
    kCFStringEncodingMacUkrainian,
    kCFStringEncodingMacChineseTrad,
    kCFStringEncodingMacRoman,
    
    kCFStringEncodingMacRoman,
    
    kCFStringEncodingMacRoman
};

static const char *__CFISONameList[] = {
    "Western (ISO Latin 1)",
    "Central European (ISO Latin 2)",
    "Western (ISO Latin 3)",
    "Central European (ISO Latin 4)",
    "Cyrillic (ISO 8859-5)",
    "Arabic (ISO 8859-6)",
    "Greek (ISO 8859-7)",
    "Hebrew (ISO 8859-8)",
    "Turkish (ISO Latin 5)",
    "Nordic (ISO Latin 6)",
    "Thai (ISO 8859-11)",
    NULL,
    "Baltic (ISO Latin 7)",
    "Celtic (ISO Latin 8)",
    "Western (ISO Latin 9)",
    "Romanian (ISO Latin 10)",
};

static const char *__CFOtherNameList[] = {
    "Western (Mac OS Roman)",
    "Japanese (Mac OS)",
    "Traditional Chinese (Mac OS)",
    "Korean (Mac OS)",
    "Arabic (Mac OS)",
    "Hebrew (Mac OS)",
    "Greek (Mac OS)",
    "Cyrillic (Mac OS)",
    "Devanagari (Mac OS)",
    "Gurmukhi (Mac OS)",
    "Gujarati (Mac OS)",
    "Oriya (Mac OS)",
    "Bengali (Mac OS)",
    "Tamil (Mac OS)",
    "Telugu (Mac OS)",
    "Kannada (Mac OS)",
    "Malayalam (Mac OS)",
    "Sinhalese (Mac OS)",
    "Burmese (Mac OS)",
    "Khmer (Mac OS)",
    "Thai (Mac OS)",
    "Laotian (Mac OS)",
    "Georgian (Mac OS)",
    "Armenian (Mac OS)",
    "Simplified Chinese (Mac OS)",
    "Tibetan (Mac OS)",
    "Mongolian (Mac OS)",
    "Ethiopic (Mac OS)",
    "Central European (Mac OS)",
    "Vietnamese (Mac OS)",
    "Symbol (Mac OS)",
    "Dingbats (Mac OS)",
    "Turkish (Mac OS)",
    "Croatian (Mac OS)",
    "Icelandic (Mac OS)",
    "Romanian (Mac OS)",
    "Celtic (Mac OS)",
    "Gaelic (Mac OS)",
    "Farsi (Mac OS)",
    "Cyrillic (Mac OS Ukrainian)",
    "Inuit (Mac OS)",
    "Latin-US (DOS)",
    "Greek (DOS)",
    "Baltic (DOS)",
    "Western (DOS Latin 1)",
    "Greek (DOS Greek 1)",
    "Central European (DOS Latin 2)",
    "Cyrillic (DOS)",
    "Turkish (DOS)",
    "Portuguese (DOS)",
    "Icelandic (DOS)",
    "Hebrew (DOS)",
    "Canadian French (DOS)",
    "Arabic (DOS)",
    "Nordic (DOS)",
    "Russian (DOS)",
    "Greek (DOS Greek 2)",
    "Thai (Windows, DOS)",
    "Japanese (Windows, DOS)",
    "Simplified Chinese (Windows, DOS)",
    "Korean (Windows, DOS)",
    "Traditional Chinese (Windows, DOS)",
    "Western (Windows Latin 1)",
    "Central European (Windows Latin 2)",
    "Cyrillic (Windows)",
    "Greek (Windows)",
    "Turkish (Windows Latin 5)",
    "Hebrew (Windows)",
    "Arabic (Windows)",
    "Baltic (Windows)",
    "Vietnamese (Windows)",
    "Korean (Windows Johab)",
    "Western (ASCII)",
    "Japanese (Shift JIS X0213)",
    "Chinese (GB 18030)",
    "Japanese (ISO 2022-JP)",
    "Japanese (ISO 2022-JP-2)",
    "Japanese (ISO 2022-JP-1)",
    "Japanese (ISO 2022-JP-3)",
    "Chinese (ISO 2022-CN)",
    "Chinese (ISO 2022-CN-EXT)",
    "Korean (ISO 2022-KR)",
    "Japanese (EUC)",
    "Simplified Chinese (GB 2312)",
    "Traditional Chinese (EUC)",
    "Korean (EUC)",
    "Japanese (Shift JIS)",
    "Cyrillic (KOI8-R)",
    "Traditional Chinese (Big 5)",
    "Western (Mac Mail)",
    "Simplified Chinese (HZ GB 2312)",
    "Traditional Chinese (Big 5 HKSCS)",
    NULL,
    "Ukrainian (KOI8-U)",
    "Traditional Chinese (Big 5-E)",
    NULL,
    "Western (NextStep)",
    "Western (EBCDIC Latin 1)",
};
#endif /* DEPLOYMENT_TARGET_MACOSX */

CF_PRIVATE CFStringEncoding __CFStringEncodingGetMostCompatibleMacScript(CFStringEncoding encoding) {
#if DEPLOYMENT_TARGET_MACOSX
    switch (encoding & 0x0F00) {
        case 0: return encoding & 0xFF; break; // Mac scripts

        case 0x0100: return kCFStringEncodingUnicode; break; // Unicode

        case 0x200: // ISO 8859
            return (((encoding & 0xFF) <= (sizeof(__CFISO8859SimilarScriptList) / sizeof(*__CFISO8859SimilarScriptList))) ? __CFISO8859SimilarScriptList[(encoding & 0xFF) - 1] : kCFStringEncodingInvalidId);
            break;

        default: {
            CFIndex index = __CFGetEncodingIndex(encoding);
            
            if (kCFNotFound != index) {
                index -= __CFGetEncodingIndex(kCFStringEncodingDOSLatinUS);
                return __CFOtherSimilarScriptList[index];
            }
        }
    }
#endif /* DEPLOYMENT_TARGET_MACOSX */

    return kCFStringEncodingInvalidId;
}

CF_PRIVATE const char *__CFStringEncodingGetName(CFStringEncoding encoding) {
    switch (encoding) {
        case kCFStringEncodingUTF8: return "Unicode (UTF-8)"; break;
        case kCFStringEncodingUTF16: return "Unicode (UTF-16)"; break;
        case kCFStringEncodingUTF16BE: return "Unicode (UTF-16BE)"; break;
        case kCFStringEncodingUTF16LE: return "Unicode (UTF-16LE)"; break;
        case kCFStringEncodingUTF32: return "Unicode (UTF-32)"; break;
        case kCFStringEncodingUTF32BE: return "Unicode (UTF-32BE)"; break;
        case kCFStringEncodingUTF32LE: return "Unicode (UTF-32LE)"; break;
        case kCFStringEncodingNonLossyASCII: return "Non-lossy ASCII"; break;
        case kCFStringEncodingUTF7: return "Unicode (UTF-7)"; break;
    }

#if DEPLOYMENT_TARGET_MACOSX
    if (0x0200 == (encoding & 0x0F00)) {
        encoding &= 0x00FF;

        if (encoding <= (sizeof(__CFISONameList) / sizeof(*__CFISONameList))) return __CFISONameList[encoding - 1];
    } else {
        CFIndex index = __CFGetEncodingIndex(encoding);

        if (kCFNotFound != index) return __CFOtherNameList[index];
    }
#endif /* DEPLOYMENT_TARGET_MACOSX */
    
    return NULL;
}
