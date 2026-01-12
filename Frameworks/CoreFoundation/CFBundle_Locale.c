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

/*      CFBundle_Locale.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"

#include <CoreFoundation/CFPreferences.h>

#include <unicode/ualoc.h>
#include <ctype.h>

static CFStringRef _CFBundleCopyLanguageFoundInLocalizations(CFArrayRef localizations, CFStringRef language);

#pragma mark -
#pragma mark Mixed Localizations

// This helper function checks for various permutations of the ways people put boolean values in Info.plist dictionaries
static Boolean _CFBundleGetInfoDictionaryBoolean(CFStringRef key) {
    Boolean result = false;
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFDictionaryRef infoDict = mainBundle ? CFBundleGetInfoDictionary(mainBundle) : NULL;
    CFTypeRef infoDictValue = infoDict ? CFDictionaryGetValue(infoDict, key) : NULL;
    if (infoDictValue) {
        CFTypeID typeID = CFGetTypeID(infoDictValue);
        if (typeID == CFBooleanGetTypeID()) {
            result = CFBooleanGetValue((CFBooleanRef)infoDictValue);
        } else if (typeID == CFStringGetTypeID()) {
            result = (CFStringCompare((CFStringRef)infoDictValue, CFSTR("true"), kCFCompareCaseInsensitive) == kCFCompareEqualTo || CFStringCompare((CFStringRef)infoDictValue, CFSTR("YES"), kCFCompareCaseInsensitive) == kCFCompareEqualTo);
        } else if (typeID == CFNumberGetTypeID()) {
            SInt32 val = 0;
            if (CFNumberGetValue((CFNumberRef)infoDictValue, kCFNumberSInt32Type, &val)) result = (val != 0);
        }
    }
    return result;
}

CF_PRIVATE Boolean CFBundleAllowMixedLocalizations(void) {
    static Boolean allowMixed = false;
    static dispatch_once_t once = 0;
    dispatch_once(&once, ^{
        allowMixed = _CFBundleGetInfoDictionaryBoolean(_kCFBundleAllowMixedLocalizationsKey);
    });
    return allowMixed;
}

static Boolean CFBundleFollowParentLocalization(void) {
    static Boolean followParent = false;
    static dispatch_once_t once = 0;
    dispatch_once(&once, ^{
        followParent = _CFBundleGetInfoDictionaryBoolean(CFSTR("CFBundleFollowParentLocalization"));
    });
    return followParent;

}

#pragma mark -
#pragma mark Language and Locale Codes

// string, with groups of 6 characters being 1 element in the array of locale abbreviations
const char * __CFBundleLocaleAbbreviationsArray =
    "en_US\0"      "fr_FR\0"      "en_GB\0"      "de_DE\0"      "it_IT\0"      "nl_NL\0"      "nl_BE\0"      "sv_SE\0"
    "es_ES\0"      "da_DK\0"      "pt_PT\0"      "fr_CA\0"      "nb_NO\0"      "he_IL\0"      "ja_JP\0"      "en_AU\0"
    "ar\0\0\0\0"   "fi_FI\0"      "fr_CH\0"      "de_CH\0"      "el_GR\0"      "is_IS\0"      "mt_MT\0"      "el_CY\0"
    "tr_TR\0"      "hr_HR\0"      "nl_NL\0"      "nl_BE\0"      "en_CA\0"      "en_CA\0"      "pt_PT\0"      "nb_NO\0"
    "da_DK\0"      "hi_IN\0"      "ur_PK\0"      "tr_TR\0"      "it_CH\0"      "en\0\0\0\0"   "\0\0\0\0\0\0" "ro_RO\0"
    "grc\0\0\0"    "lt_LT\0"      "pl_PL\0"      "hu_HU\0"      "et_EE\0"      "lv_LV\0"      "se\0\0\0\0"   "fo_FO\0"
    "fa_IR\0"      "ru_RU\0"      "ga_IE\0"      "ko_KR\0"      "zh_CN\0"      "zh_TW\0"      "th_TH\0"      "\0\0\0\0\0\0"
    "cs_CZ\0"      "sk_SK\0"      "\0\0\0\0\0\0" "hu_HU\0"      "bn\0\0\0\0"   "be_BY\0"      "uk_UA\0"      "\0\0\0\0\0\0"
    "el_GR\0"      "sr_CS\0"      "sl_SI\0"      "mk_MK\0"      "hr_HR\0"      "\0\0\0\0\0\0" "de_DE\0"      "pt_BR\0"
    "bg_BG\0"      "ca_ES\0"      "\0\0\0\0\0\0" "gd\0\0\0\0"   "gv\0\0\0\0"   "br\0\0\0\0"   "iu_CA\0"      "cy\0\0\0\0"
    "en_CA\0"      "ga_IE\0"      "en_CA\0"      "dz_BT\0"      "hy_AM\0"      "ka_GE\0"      "es_XL\0"      "es_ES\0"
    "to_TO\0"      "pl_PL\0"      "ca_ES\0"      "fr\0\0\0\0"   "de_AT\0"      "es_XL\0"      "gu_IN\0"      "pa\0\0\0\0"
    "ur_IN\0"      "vi_VN\0"      "fr_BE\0"      "uz_UZ\0"      "en_SG\0"      "nn_NO\0"      "af_ZA\0"      "eo\0\0\0\0"
    "mr_IN\0"      "bo\0\0\0\0"   "ne_NP\0"      "kl\0\0\0\0"   "en_IE\0";

#define NUM_LOCALE_ABBREVIATIONS        109
#define LOCALE_ABBREVIATION_LENGTH      6

static const char * const __CFBundleLanguageNamesArray[] = {
    "English",      "French",       "German",       "Italian",      "Dutch",        "Swedish",      "Spanish",      "Danish",
    "Portuguese",   "Norwegian",    "Hebrew",       "Japanese",     "Arabic",       "Finnish",      "Greek",        "Icelandic",
    "Maltese",      "Turkish",      "Croatian",     "Chinese",      "Urdu",         "Hindi",        "Thai",         "Korean",
    "Lithuanian",   "Polish",       "Hungarian",    "Estonian",     "Latvian",      "Sami",         "Faroese",      "Farsi",
    "Russian",      "Chinese",      "Dutch",        "Irish",        "Albanian",     "Romanian",     "Czech",        "Slovak",
    "Slovenian",    "Yiddish",      "Serbian",      "Macedonian",   "Bulgarian",    "Ukrainian",    "Byelorussian", "Uzbek",
    "Kazakh",       "Azerbaijani",  "Azerbaijani",  "Armenian",     "Georgian",     "Moldavian",    "Kirghiz",      "Tajiki",
    "Turkmen",      "Mongolian",    "Mongolian",    "Pashto",       "Kurdish",      "Kashmiri",     "Sindhi",       "Tibetan",
    "Nepali",       "Sanskrit",     "Marathi",      "Bengali",      "Assamese",     "Gujarati",     "Punjabi",      "Oriya",
    "Malayalam",    "Kannada",      "Tamil",        "Telugu",       "Sinhalese",    "Burmese",      "Khmer",        "Lao",
    "Vietnamese",   "Indonesian",   "Tagalog",      "Malay",        "Malay",        "Amharic",      "Tigrinya",     "Oromo",
    "Somali",       "Swahili",      "Kinyarwanda",  "Rundi",        "Nyanja",       "Malagasy",     "Esperanto",    "",
    "",             "",             "",             "",             "",             "",             "",             "",
    "",             "",             "",             "",             "",             "",             "",             "",
    "",             "",             "",             "",             "",             "",             "",             "",
    "",             "",             "",             "",             "",             "",             "",             "",
    "Welsh",        "Basque",       "Catalan",      "Latin",        "Quechua",      "Guarani",      "Aymara",       "Tatar",
    "Uighur",       "Dzongkha",     "Javanese",     "Sundanese",    "Galician",     "Afrikaans",    "Breton",       "Inuktitut",
    "Scottish",     "Manx",         "Irish",        "Tongan",       "Greek",        "Greenlandic",  "Azerbaijani",  "Nynorsk"
};

#define NUM_LANGUAGE_NAMES      152
#define LANGUAGE_NAME_LENGTH    13

// string, with groups of 3 characters being 1 element in the array of abbreviations
const char * __CFBundleLanguageAbbreviationsArray =
    "en\0"   "fr\0"   "de\0"   "it\0"   "nl\0"   "sv\0"   "es\0"   "da\0"
    "pt\0"   "nb\0"   "he\0"   "ja\0"   "ar\0"   "fi\0"   "el\0"   "is\0"
    "mt\0"   "tr\0"   "hr\0"   "zh\0"   "ur\0"   "hi\0"   "th\0"   "ko\0"
    "lt\0"   "pl\0"   "hu\0"   "et\0"   "lv\0"   "se\0"   "fo\0"   "fa\0"
    "ru\0"   "zh\0"   "nl\0"   "ga\0"   "sq\0"   "ro\0"   "cs\0"   "sk\0"
    "sl\0"   "yi\0"   "sr\0"   "mk\0"   "bg\0"   "uk\0"   "be\0"   "uz\0"
    "kk\0"   "az\0"   "az\0"   "hy\0"   "ka\0"   "mo\0"   "ky\0"   "tg\0"
    "tk\0"   "mn\0"   "mn\0"   "ps\0"   "ku\0"   "ks\0"   "sd\0"   "bo\0"
    "ne\0"   "sa\0"   "mr\0"   "bn\0"   "as\0"   "gu\0"   "pa\0"   "or\0"
    "ml\0"   "kn\0"   "ta\0"   "te\0"   "si\0"   "my\0"   "km\0"   "lo\0"
    "vi\0"   "id\0"   "tl\0"   "ms\0"   "ms\0"   "am\0"   "ti\0"   "om\0"
    "so\0"   "sw\0"   "rw\0"   "rn\0"   "\0\0\0" "mg\0"   "eo\0"   "\0\0\0"
    "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0"
    "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0"
    "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0"
    "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0" "\0\0\0"
    "cy\0"   "eu\0"   "ca\0"   "la\0"   "qu\0"   "gn\0"   "ay\0"   "tt\0"
    "ug\0"   "dz\0"   "jv\0"   "su\0"   "gl\0"   "af\0"   "br\0"   "iu\0"
    "gd\0"   "gv\0"   "ga\0"   "to\0"   "el\0"   "kl\0"   "az\0"   "nn\0";

#define NUM_LANGUAGE_ABBREVIATIONS      152
#define LANGUAGE_ABBREVIATION_LENGTH    3

static CFStringRef _CFBundleGetAlternateNameForLanguage(CFStringRef language) {
    // These are not necessarily common localizations per se, but localizations for which the full language name is still in common use.
    // These are used to provide a fast path for it (other localizations usually use the abbreviation, which is even faster).
    static CFStringRef const __CFBundleCommonLanguageNamesArray[] = {CFSTR("English"), CFSTR("French"), CFSTR("German"), CFSTR("Italian"), CFSTR("Dutch"), CFSTR("Spanish"), CFSTR("Japanese")};
    static CFStringRef const __CFBundleCommonLanguageAbbreviationsArray[] = {CFSTR("en"), CFSTR("fr"), CFSTR("de"), CFSTR("it"), CFSTR("nl"), CFSTR("es"), CFSTR("ja")};
    
    for (CFIndex idx = 0; idx < sizeof(__CFBundleCommonLanguageNamesArray) / sizeof(CFStringRef); idx++) {
        if (CFEqual(language, __CFBundleCommonLanguageAbbreviationsArray[idx])) {
            return __CFBundleCommonLanguageNamesArray[idx];
        } else if (CFEqual(language, __CFBundleCommonLanguageNamesArray[idx])) {
            return __CFBundleCommonLanguageAbbreviationsArray[idx];
        }
    }

    return NULL;
}

static const SInt32 __CFBundleScriptCodesArray[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  1,  4,  0,  6,  0,
    0,  0,  0,  2,  4,  9, 21,  3, 29, 29, 29, 29, 29,  0,  0,  4,
    7, 25,  0,  0,  0,  0, 29, 29,  0,  5,  7,  7,  7,  7,  7,  7,
    7,  7,  4, 24, 23,  7,  7,  7,  7, 27,  7,  4,  4,  4,  4, 26,
    9,  9,  9, 13, 13, 11, 10, 12, 17, 16, 14, 15, 18, 19, 20, 22,
    30,  0,  0,  0,  4, 28, 28, 28,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  7,  4, 26,  0,  0,  0,  0,  0, 28,
    0,  0,  0,  0,  6,  0,  0,  0
};

static const CFStringEncoding __CFBundleStringEncodingsArray[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  1,  4,  0,  6, 37,
    0, 35, 36,  2,  4,  9, 21,  3, 29, 29, 29, 29, 29,  0, 37, 0x8C,
    7, 25,  0, 39,  0, 38, 29, 29, 36,  5,  7,  7,  7, 0x98,  7,  7,
    7,  7,  4, 24, 23,  7,  7,  7,  7, 27,  7,  4,  4,  4,  4, 26,
    9,  9,  9, 13, 13, 11, 10, 12, 17, 16, 14, 15, 18, 19, 20, 22,
    30,  0,  0,  0,  4, 28, 28, 28,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    39,  0,  0,  0,  0,  0,  0,  7,  4, 26,  0,  0,  0,  0, 39, 0xEC,
    39, 39, 40,  0,  6,  0,  0,  0
};

static SInt32 _CFBundleGetLanguageCodeForLocalization(CFStringRef localizationName) {
    SInt32 result = -1, i;
    char buff[256];
    CFIndex length = CFStringGetLength(localizationName);
    if (length >= LANGUAGE_ABBREVIATION_LENGTH - 1 && length <= 255 && CFStringGetCString(localizationName, buff, 255, kCFStringEncodingASCII)) {
        buff[255] = '\0';
        for (i = 0; -1 == result && i < NUM_LANGUAGE_NAMES; i++) {
            if (0 == strcmp(buff, __CFBundleLanguageNamesArray[i])) result = i;
        }
        if (0 == strcmp(buff, "zh_TW") || 0 == strcmp(buff, "zh-Hant")) result = 19; else if (0 == strcmp(buff, "zh_CN") || 0 == strcmp(buff, "zh-Hans")) result = 33; // hack for mixed-up Chinese language codes
        if (-1 == result && (length == LANGUAGE_ABBREVIATION_LENGTH - 1 || !isalpha(buff[LANGUAGE_ABBREVIATION_LENGTH - 1]))) {
            buff[LANGUAGE_ABBREVIATION_LENGTH - 1] = '\0';
            if ('n' == buff[0] && 'o' == buff[1]) result = 9;  // hack for Norwegian
            for (i = 0; -1 == result && i < NUM_LANGUAGE_ABBREVIATIONS * LANGUAGE_ABBREVIATION_LENGTH; i += LANGUAGE_ABBREVIATION_LENGTH) {
                if (buff[0] == *(__CFBundleLanguageAbbreviationsArray + i + 0) && buff[1] == *(__CFBundleLanguageAbbreviationsArray + i + 1)) result = i / LANGUAGE_ABBREVIATION_LENGTH;
            }
        }
    }
    return result;
}

static CFStringRef _CFBundleCopyLanguageAbbreviationForLanguageCode(SInt32 languageCode) {
    CFStringRef result = NULL;
    if (0 <= languageCode && languageCode < NUM_LANGUAGE_ABBREVIATIONS) {
        const char *languageAbbreviation = __CFBundleLanguageAbbreviationsArray + languageCode * LANGUAGE_ABBREVIATION_LENGTH;
        if (languageAbbreviation && *languageAbbreviation != '\0') result = CFStringCreateWithCStringNoCopy(kCFAllocatorSystemDefault, languageAbbreviation, kCFStringEncodingASCII, kCFAllocatorNull);
    }
    return result;
}

// Swaps - for _ and _ for - in a localization name
static CFStringRef _CFBundleCopyModifiedLocalization(CFStringRef localizationName) {
    CFMutableStringRef result = NULL;
    CFIndex length = CFStringGetLength(localizationName);
    if (length >= 4) {
        UniChar c = CFStringGetCharacterAtIndex(localizationName, 2);
        if ('-' == c || '_' == c) {
            result = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, length, localizationName);
            CFStringReplace(result, CFRangeMake(2, 1), ('-' == c) ? CFSTR("_") : CFSTR("-"));
        }
    }
    return result;
}

static SInt32 _CFBundleGetLanguageCodeForRegionCode(SInt32 regionCode) {
    SInt32 result = -1, i;
    if (52 == regionCode) {     // hack for mixed-up Chinese language codes
        result = 33;
    } else if (0 <= regionCode && regionCode < NUM_LOCALE_ABBREVIATIONS) {
        const char *localeAbbreviation = __CFBundleLocaleAbbreviationsArray + regionCode * LOCALE_ABBREVIATION_LENGTH;
        if (localeAbbreviation && *localeAbbreviation != '\0') {
            for (i = 0; -1 == result && i < NUM_LANGUAGE_ABBREVIATIONS * LANGUAGE_ABBREVIATION_LENGTH; i += LANGUAGE_ABBREVIATION_LENGTH) {
                if (localeAbbreviation[0] == *(__CFBundleLanguageAbbreviationsArray + i + 0) && localeAbbreviation[1] == *(__CFBundleLanguageAbbreviationsArray + i + 1)) result = i / LANGUAGE_ABBREVIATION_LENGTH;
            }
        }
    }
    return result;
}

static SInt32 _CFBundleGetRegionCodeForLanguageCode(SInt32 languageCode) {
    SInt32 result = -1, i;
    if (19 == languageCode) {   // hack for mixed-up Chinese language codes
        result = 53;
    } else if (0 <= languageCode && languageCode < NUM_LANGUAGE_ABBREVIATIONS) {
        const char *languageAbbreviation = __CFBundleLanguageAbbreviationsArray + languageCode * LANGUAGE_ABBREVIATION_LENGTH;
        if (languageAbbreviation && *languageAbbreviation != '\0') {
            for (i = 0; -1 == result && i < NUM_LOCALE_ABBREVIATIONS * LOCALE_ABBREVIATION_LENGTH; i += LOCALE_ABBREVIATION_LENGTH) {
                if (*(__CFBundleLocaleAbbreviationsArray + i + 0) == languageAbbreviation[0] && *(__CFBundleLocaleAbbreviationsArray + i + 1) == languageAbbreviation[1]) result = i / LOCALE_ABBREVIATION_LENGTH;
            }
        }
    }
    if (25 == result) result = 68;
    if (28 == result) result = 82;
    return result;
}

static SInt32 _CFBundleGetRegionCodeForLocalization(CFStringRef localizationName) {
    SInt32 result = -1, i;
    char buff[LOCALE_ABBREVIATION_LENGTH];
    CFIndex length = CFStringGetLength(localizationName);
    if (length >= LANGUAGE_ABBREVIATION_LENGTH - 1 && length <= LOCALE_ABBREVIATION_LENGTH - 1 && CFStringGetCString(localizationName, buff, LOCALE_ABBREVIATION_LENGTH, kCFStringEncodingASCII)) {
        buff[LOCALE_ABBREVIATION_LENGTH - 1] = '\0';
        for (i = 0; -1 == result && i < NUM_LOCALE_ABBREVIATIONS * LOCALE_ABBREVIATION_LENGTH; i += LOCALE_ABBREVIATION_LENGTH) {
            if (0 == strcmp(buff, __CFBundleLocaleAbbreviationsArray + i)) result = i / LOCALE_ABBREVIATION_LENGTH;
        }
    }
    if (25 == result) result = 68;
    if (28 == result) result = 82;
    if (37 == result) result = 0;
    if (-1 == result) {
        SInt32 languageCode = _CFBundleGetLanguageCodeForLocalization(localizationName);
        result = _CFBundleGetRegionCodeForLanguageCode(languageCode);
    }
    return result;
}

CF_PRIVATE CFStringRef _CFBundleCopyLocaleAbbreviationForRegionCode(SInt32 regionCode) {
    CFStringRef result = NULL;
    if (0 <= regionCode && regionCode < NUM_LOCALE_ABBREVIATIONS) {
        const char *localeAbbreviation = __CFBundleLocaleAbbreviationsArray + regionCode * LOCALE_ABBREVIATION_LENGTH;
        if (localeAbbreviation && *localeAbbreviation != '\0') {
            result = CFStringCreateWithCStringNoCopy(kCFAllocatorSystemDefault, localeAbbreviation, kCFStringEncodingASCII, kCFAllocatorNull);
        }
    }
    return result;
}

CF_EXPORT Boolean CFBundleGetLocalizationInfoForLocalization(CFStringRef localizationName, SInt32 *languageCode, SInt32 *regionCode, SInt32 *scriptCode, CFStringEncoding *stringEncoding) {
    Boolean retval = false;
    SInt32 language = -1, region = -1, script = 0;
    CFStringEncoding encoding = kCFStringEncodingMacRoman;
    if (!localizationName) {
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFArrayRef languages = NULL;
        if (mainBundle) {
            languages = _CFBundleCopyLanguageSearchListInBundle(mainBundle);
        }
        if (!languages) languages = _CFBundleCopyUserLanguages();
        if (languages && CFArrayGetCount(languages) > 0) localizationName = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
    }
    if (localizationName) {
        LangCode langCode = -1;
        RegionCode regCode = -1;
        ScriptCode scrCode = 0;
        CFStringEncoding enc = kCFStringEncodingMacRoman;
        retval = CFLocaleGetLanguageRegionEncodingForLocaleIdentifier(localizationName, &langCode, &regCode, &scrCode, &enc);
        if (retval) {
            language = langCode;
            region = regCode;
            script = scrCode;
            encoding = enc;
        }
    }
    if (!retval) {
        if (localizationName) {
            language = _CFBundleGetLanguageCodeForLocalization(localizationName);
            region = _CFBundleGetRegionCodeForLocalization(localizationName);
        } else {
            _CFBundleGetLanguageAndRegionCodes(&language, &region);
        }
        if ((language < 0 || language > (int)(sizeof(__CFBundleScriptCodesArray)/sizeof(SInt32))) && region != -1) language = _CFBundleGetLanguageCodeForRegionCode(region);
        if (region == -1 && language != -1) region = _CFBundleGetRegionCodeForLanguageCode(language);
        if (language >= 0 && language < (int)(sizeof(__CFBundleScriptCodesArray)/sizeof(SInt32))) {
            script = __CFBundleScriptCodesArray[language];
        }
        if (language >= 0 && language < (int)(sizeof(__CFBundleStringEncodingsArray)/sizeof(CFStringEncoding))) {
            encoding = __CFBundleStringEncodingsArray[language];
        }
        retval = (language != -1 || region != -1);
    }
    if (languageCode) *languageCode = language;
    if (regionCode) *regionCode = region;
    if (scriptCode) *scriptCode = script;
    if (stringEncoding) *stringEncoding = encoding;
    return retval;
}

CFStringRef CFBundleCopyLocalizationForLocalizationInfo(SInt32 languageCode, SInt32 regionCode, SInt32 scriptCode, CFStringEncoding stringEncoding) {
    CFStringRef localizationName = NULL;
    if (!localizationName) localizationName = _CFBundleCopyLocaleAbbreviationForRegionCode(regionCode);
#if DEPLOYMENT_TARGET_MACOSX
    if (!localizationName && 0 <= languageCode && languageCode < SHRT_MAX) localizationName = CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes(kCFAllocatorSystemDefault, (LangCode)languageCode, (RegionCode)-1);
#endif
    if (!localizationName) localizationName = _CFBundleCopyLanguageAbbreviationForLanguageCode(languageCode);
    if (!localizationName) {
        SInt32 language = -1, scriptLanguage = -1, encodingLanguage = -1;
        unsigned int i;
        for (i = 0; language == -1 && i < (sizeof(__CFBundleScriptCodesArray)/sizeof(SInt32)); i++) {
            if (__CFBundleScriptCodesArray[i] == scriptCode && __CFBundleStringEncodingsArray[i] == stringEncoding) language = i;
        }
        for (i = 0; scriptLanguage == -1 && i < (sizeof(__CFBundleScriptCodesArray)/sizeof(SInt32)); i++) {
            if (__CFBundleScriptCodesArray[i] == scriptCode) scriptLanguage = i;
        }
        for (i = 0; encodingLanguage == -1 && i < (sizeof(__CFBundleStringEncodingsArray)/sizeof(CFStringEncoding)); i++) {
            if (__CFBundleStringEncodingsArray[i] == stringEncoding) encodingLanguage = i;
        }
        localizationName = _CFBundleCopyLanguageAbbreviationForLanguageCode(language);
        if (!localizationName) localizationName = _CFBundleCopyLanguageAbbreviationForLanguageCode(encodingLanguage);
        if (!localizationName) localizationName = _CFBundleCopyLanguageAbbreviationForLanguageCode(scriptLanguage);
    }
    return localizationName;
}

#pragma mark -

// Get a list of lproj directories for a particular resource directory URL. Uncached. Does not include any predefined localizations from an Info.plist. This function does make any attempt to localize or canonicalize the results.
static CFArrayRef _CFBundleCopyLProjDirectoriesForURL(CFAllocatorRef allocator, CFURLRef url) {
    __block CFMutableArrayRef result = NULL;
    CFURLRef absoluteURL = CFURLCopyAbsoluteURL(url);
    CFStringRef directoryPath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
    CFRelease(absoluteURL);
    
    CFStringRef lproj = _CFBundleLprojExtensionWithDot;
    CFIndex lprojLen = CFStringGetLength(lproj);
    
    _CFIterateDirectory(directoryPath, ^Boolean(CFStringRef fileName, uint8_t fileType) {
        // See if the fileName ends in .lproj
        // The comparison starts at the end of the fileName, backed up by the length of .lproj
        CFIndex fileNameLen = CFStringGetLength(fileName);
        if (fileNameLen > lprojLen && CFStringCompareWithOptions(fileName, lproj, CFRangeMake(fileNameLen - lprojLen, lprojLen), 0) == kCFCompareEqualTo) {
            // Chop off the .lproj part before creating a string
            CFStringRef lprojDirectoryName = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fileName, CFRangeMake(0, fileNameLen - lprojLen));
            if (!result) result = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
            CFArrayAppendValue(result, lprojDirectoryName);
            CFRelease(lprojDirectoryName);
        }
        return true;
    });
    
    CFRelease(directoryPath);
    return (CFArrayRef)result;
}

/* This function returns:
    1. The predefined localizations in the Info.plist (CFBundleLocalizations)
    2. Additionally, the .lproj directories inside the bundle
    3. Additionally, the development region of the bundle (CFBundleDevelopmentRegion) -- although if it's already in #1, or #2, we don't append it again
    4. As an ultimate fallback, an empty array
 
 This doesn't attempt to include a list of localizations supported by a bundle by way of a fallback path; e.g., if the bundle has en_GB then we do not include en_IN (which falls back to en_GB if not present).

 Since the result of this is "typically passed as a parameter to either the CFBundleCopyPreferredLocalizationsFromArray or CFBundleCopyLocalizationsForPreferences function", those other functions will take into account the user prefs and pick the right lproj.
*/
CF_EXPORT CFArrayRef CFBundleCopyBundleLocalizations(CFBundleRef bundle) {
    CFArrayRef result = NULL;
    
    __CFLock(&bundle->_lock);
    if (bundle->_lookedForLocalizations) {
        result = (CFArrayRef)CFRetain(bundle->_localizations);
        __CFUnlock(&bundle->_lock);
        return result;
    }
    __CFUnlock(&bundle->_lock);
    
    CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
    if (infoDict) {
        CFArrayRef predefinedLocalizations = (CFArrayRef)CFDictionaryGetValue(infoDict, kCFBundleLocalizationsKey);
        if (predefinedLocalizations && CFGetTypeID(predefinedLocalizations) == CFArrayGetTypeID()) {
            // <rdar://problem/14255685> Some people put bad things inside this array =(
            CFMutableArrayRef realPredefinedLocalizations = CFArrayCreateMutable(CFGetAllocator(bundle), CFArrayGetCount(predefinedLocalizations), &kCFTypeArrayCallBacks);
            for (CFIndex i = 0; i < CFArrayGetCount(predefinedLocalizations); i++) {
                CFStringRef oneEntry = CFArrayGetValueAtIndex(predefinedLocalizations, i);
                if (CFGetTypeID(oneEntry) == CFStringGetTypeID() && CFStringGetLength(oneEntry) > 0) {
                    CFArrayAppendValue(realPredefinedLocalizations, oneEntry);
                }
            }
            result = CFArrayCreateCopy(CFGetAllocator(bundle), realPredefinedLocalizations);
            CFRelease(realPredefinedLocalizations);
        }
    }
    
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);
    if (resourcesURL) {
        CFArrayRef lprojDirectoriesInResources = _CFBundleCopyLProjDirectoriesForURL(CFGetAllocator(bundle), resourcesURL);
        if (lprojDirectoriesInResources) {
            if (result) {
                // Append the lproj result to the predefined localization array
                CFMutableArrayRef newResult = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, result);
                CFRelease(result);
                CFArrayAppendArray(newResult, lprojDirectoriesInResources, CFRangeMake(0, CFArrayGetCount(lprojDirectoriesInResources)));
                CFRelease(lprojDirectoriesInResources);
                result = newResult;
            } else {
                result = lprojDirectoriesInResources;
            }
        }
        CFRelease(resourcesURL);
    }
    
    CFStringRef developmentLocalization = CFBundleGetDevelopmentRegion(bundle);
    if (result) {
        if (developmentLocalization) {
            CFRange entireRange = CFRangeMake(0, CFArrayGetCount(result));
            if (CFArrayContainsValue(result, entireRange, _CFBundleBaseDirectory)) {
                // Base.lproj contains localizations for the development region. Insert the development region into the existing array if there isn't already a match so that resource lookup doesn't default to another language.
                // We need to make sure that we don't add "en" if "English" exists. (14006652)
                CFStringRef foundInLocalizations = _CFBundleCopyLanguageFoundInLocalizations(result, developmentLocalization);
                if (!foundInLocalizations) {
                    CFMutableArrayRef newResult = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, result);
                    CFRelease(result);
                    CFArrayAppendValue(newResult, developmentLocalization);
                    result = newResult;
                } else {
                    // The development localization was in the result, but in some other form. We don't need to add it again.
                    CFRelease(foundInLocalizations);
                }
            }
        }
    } else {
        if (developmentLocalization) {
            result = CFArrayCreate(CFGetAllocator(bundle), (const void **)&developmentLocalization, 1, &kCFTypeArrayCallBacks);
        } else {
            result = CFArrayCreate(CFGetAllocator(bundle), NULL, 0, &kCFTypeArrayCallBacks);
        }
    }
    
    // Cache the result.
    __CFLock(&bundle->_lock);
    if (bundle->_lookedForLocalizations && result) {
        // Another thread beat us to it. Release our result and return the existing answer.
        CFRelease(result);
        result = (CFArrayRef)CFRetain(bundle->_localizations);
    } else {
        bundle->_localizations = (CFArrayRef)CFRetain(result);
        bundle->_lookedForLocalizations = true;
    }
    __CFUnlock(&bundle->_lock);
    
    return result;
}

CF_EXPORT CFArrayRef CFBundleCopyLocalizationsForURL(CFURLRef url) {
    CFArrayRef result = NULL;
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, url);
    CFStringRef devLang = NULL;
    if (bundle) {
        result = CFBundleCopyBundleLocalizations(bundle);
        CFRelease(bundle);
    } else {
        CFDictionaryRef infoDict = _CFBundleCopyInfoDictionaryInExecutable(url);
        if (infoDict) {
            CFArrayRef predefinedLocalizations = (CFArrayRef)CFDictionaryGetValue(infoDict, kCFBundleLocalizationsKey);
            if (predefinedLocalizations && CFGetTypeID(predefinedLocalizations) == CFArrayGetTypeID()) {
                result = (CFArrayRef)CFRetain(predefinedLocalizations);
            }
            if (!result) {
                devLang = (CFStringRef)CFDictionaryGetValue(infoDict, kCFBundleDevelopmentRegionKey);
                if (devLang && (CFGetTypeID(devLang) == CFStringGetTypeID() && CFStringGetLength(devLang) > 0)) {
                    result = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)&devLang, 1, &kCFTypeArrayCallBacks);
                }
            }
            CFRelease(infoDict);
        }
    }
    return result;
}

extern void *__CFAppleLanguages;




CF_PRIVATE CFArrayRef _CFBundleCopyUserLanguages() {
    static CFArrayRef _CFBundleUserLanguages = NULL;
    static dispatch_once_t once = 0;
    dispatch_once(&once, ^{
        CFArrayRef preferencesArray = NULL;
        if (__CFAppleLanguages) {
            CFDataRef data;
            CFIndex length = strlen((const char *)__CFAppleLanguages);
            if (length > 0) {
                data = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, (const UInt8 *)__CFAppleLanguages, length, kCFAllocatorNull);
                if (data) {
                    _CFBundleUserLanguages = (CFArrayRef)CFPropertyListCreateWithData(kCFAllocatorSystemDefault, data, kCFPropertyListImmutable, NULL, NULL);
                    CFRelease(data);
                }
            }
        }
        if (!_CFBundleUserLanguages && preferencesArray) _CFBundleUserLanguages = (CFArrayRef)CFRetain(preferencesArray);
        Boolean useEnglishAsBackstop = true;
        // could perhaps read out of LANG environment variable
        if (useEnglishAsBackstop && !_CFBundleUserLanguages) {
            CFStringRef english = CFSTR("en");
            _CFBundleUserLanguages = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)&english, 1, &kCFTypeArrayCallBacks);
        }
        if (_CFBundleUserLanguages && CFGetTypeID(_CFBundleUserLanguages) != CFArrayGetTypeID()) {
            CFRelease(_CFBundleUserLanguages);
            _CFBundleUserLanguages = NULL;
        }
        if (preferencesArray) CFRelease(preferencesArray);
    });
    
    if (_CFBundleUserLanguages) {
        CFRetain(_CFBundleUserLanguages);
        return _CFBundleUserLanguages;
    } else {
        return NULL;
    }
}

CF_EXPORT void _CFBundleGetLanguageAndRegionCodes(SInt32 *languageCode, SInt32 *regionCode) {
    // an attempt to answer the question, "what language are we running in?"
    // note that the question cannot be answered fully since it may depend on the bundle
    SInt32 language = -1, region = -1;
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFArrayRef languages = NULL;
    if (mainBundle) {
        languages = _CFBundleCopyLanguageSearchListInBundle(mainBundle);
    }
    if (!languages) languages = _CFBundleCopyUserLanguages();
    if (languages && CFArrayGetCount(languages) > 0) {
        CFStringRef localizationName = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
        Boolean retval = false;
        LangCode langCode = -1;
        RegionCode regCode = -1;
        retval = CFLocaleGetLanguageRegionEncodingForLocaleIdentifier(localizationName, &langCode, &regCode, NULL, NULL);
        if (retval) {
            language = langCode;
            region = regCode;
        }
        if (!retval) {
            language = _CFBundleGetLanguageCodeForLocalization(localizationName);
            region = _CFBundleGetRegionCodeForLocalization(localizationName);
        }
    } else {
        language = 0;
        region = 0;
    }
    if (language == -1 && region != -1) language = _CFBundleGetLanguageCodeForRegionCode(region);
    if (region == -1 && language != -1) region = _CFBundleGetRegionCodeForLanguageCode(language);
    if (languages) CFRelease(languages);
    if (languageCode) *languageCode = language;
    if (regionCode) *regionCode = region;
}

// Return a CFStringRef as it appears in the localizations, if it matches the input language in any form.
// For example, if the input language is 'en', and the array contains 'English', return 'English'.
// If the input language is 'en-US', and the array contains 'en_US', return 'en_US'.
// If the input language is 'froobleblax' and the array does not contain it, return NULL.
static CFStringRef _CFBundleCopyLanguageFoundInLocalizations(CFArrayRef localizations, CFStringRef language) {
    // Bail early on empty input
    if (!localizations || !language) {
        return NULL;
    }
    
    CFRange localizationsRange = CFRangeMake(0, CFArrayGetCount(localizations));
    
    // Does the array straight-up contain this language?
    if (CFArrayContainsValue(localizations, localizationsRange, language)) {
        return CFRetain(language);
    }
    
    // Does the array contain the alternate form of this language? (en -> English, or vice versa)
    CFStringRef altLangStr = _CFBundleGetAlternateNameForLanguage(language);
    if (altLangStr && CFArrayContainsValue(localizations, localizationsRange, altLangStr)) {
        return CFRetain(altLangStr);
    }
    
    // Does the array contain a modified form of this language? (en-US -> en_US, or vice versa)
    CFStringRef modifiedLangStr = _CFBundleCopyModifiedLocalization(language);
    if (modifiedLangStr) {
        if (CFArrayContainsValue(localizations, localizationsRange, modifiedLangStr)) {
            return modifiedLangStr;
        }
        CFRelease(modifiedLangStr);
    }

    // Does the array contain a canonical form of this language?
    CFStringRef canonicalLanguage = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, language);
    if (canonicalLanguage) {
        if (CFArrayContainsValue(localizations, localizationsRange, canonicalLanguage)) {
            return canonicalLanguage;
        }
        
        // Does the array converted to canonical forms match the canonical form of this language?
        for (CFIndex i = 0; i < localizationsRange.length; i++) {
            CFStringRef oneLanguage = (CFStringRef)CFArrayGetValueAtIndex(localizations, i);
            CFStringRef canonicalOneLanguage = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorSystemDefault, oneLanguage);
            
            if (canonicalOneLanguage) {
                if (CFEqual(canonicalOneLanguage, canonicalLanguage)) {
                    // oneLocalization is the same as the input language, even though they are in different forms
                    CFRelease(canonicalOneLanguage);
                    CFRelease(canonicalLanguage);
                    
                    return CFRetain(oneLanguage);
                }
                CFRelease(canonicalOneLanguage);
            }
        }
        
        CFRelease(canonicalLanguage);
    }
    
    // The language was not found in the array in any form
    return NULL;
}

// Given a list of localizations (e.g., provided as argument to API, or present as .lproj directories), return a mutable array of localizations in preferred order. Returns NULL if nothing is found.
static CFMutableArrayRef _CFBundleCreateMutableArrayOfFallbackLanguages(CFArrayRef localizations, CFStringRef language) {
    CFMutableArrayRef result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    
    // Check for the language itself (in whatever form)
    CFStringRef languageInLocalizations = _CFBundleCopyLanguageFoundInLocalizations(localizations, language);
    if (languageInLocalizations) {
        CFArrayAppendValue(result, languageInLocalizations);
        CFRelease(languageInLocalizations);
    }
    
    // Languages form a tree in ICU data. For example:
    
    //   en_IN -> en_GB -> en_001 -> en -> root
    // or
    //   zh_Hant_MO -> zh_Hant_HK -> zh_Hant -> zh_TW -> root
    // or
    //   zh_Hans -> zh -> zh_CN -> root
    
    // We will iterate through each element until we get to root, adding the entries to our resulting list of preferred languages as we go if they exist in the localizations list.
    
    char locale[128];
    if (CFStringGetCString(language, locale, 128, kCFStringEncodingUTF8)) {
        UErrorCode error = U_ZERO_ERROR;
        char parent[128];
        
        int counter = 0;
        while (1) {
            ualoc_getAppleParent(locale, parent, 128, &error);
            if (error != U_ZERO_ERROR) break;
            if (strncmp(parent, "root", 4) == 0) break;
            
            CFStringRef parentString = CFStringCreateWithCString(kCFAllocatorSystemDefault, parent, kCFStringEncodingUTF8);
            if (parentString) {
                CFStringRef parentInLocalizations = _CFBundleCopyLanguageFoundInLocalizations(localizations, parentString);
                if (parentInLocalizations) {
                    // Be sure not to add the same language to the result array twice. This can happen when the above function canonicalizes names (e.g., zh_TW -> zh_Hans).
                    if (!CFArrayContainsValue(result, CFRangeMake(0, CFArrayGetCount(result)), parentInLocalizations)) {
                        CFArrayAppendValue(result, parentInLocalizations);
                    }
                    CFRelease(parentInLocalizations);
                }
                CFRelease(parentString);
            }
            
            // The parent now becomes the locale we enter in the next loop
            if (strlcpy(locale, parent, 128) >= 128) break;
            
            // Just in case the call into ICU to get the parent locale results in a cycle, we need a fallback mechanism to break out of this while loop. Therefore we'll only fallback a max of 16 times.
            counter++;
            if (counter >= 16) break;
        }
    }
    
    if (CFArrayGetCount(result) == 0) {
        CFRelease(result);
        result = NULL;
    }
    
    return result;
}

/* 
    Funnel point for figuring out the language search order for resource lookup and other functions.
 
    The input to the search is the list of searchLanguages, a development language, and the list of user-preferred languages.
 
    The output is a mutable array. The result will add elements in order of the preferred languages specified. Returns an empty array if nothing is found.
 
    The users list can contain region names (like "en_US" for US English).  In this case, if the region lproj exists, it will be added, and, if the region's associated language lproj exists that will be added.
*/
static CFMutableArrayRef _CFBundleCopyPreferredLanguagesInList(CFArrayRef searchLanguages, CFStringRef devLang, CFArrayRef userLanguages, Boolean considerMain, CFURLRef bundleURL, CFBundleRef bundle) {
    CFMutableArrayRef result = NULL;
    CFArrayRef mainBundleLangs = NULL;
    
    // If CFBundleAllowMixedLocalizations is set, then we do not check the main bundle. If considerMain is not set (we've entered through the API that specifically ignores the main bundle), then do not check the main bundle.
    if (considerMain && !CFBundleAllowMixedLocalizations()) {
        if (CFBundleFollowParentLocalization()) {
        } else {
            CFBundleRef mainBundle = CFBundleGetMainBundle();
            if (mainBundle) {
                CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
                if (mainBundleURL) {
                    if (!bundleURL || !CFEqual(bundleURL, mainBundleURL)) {
                        // If there is a main bundle, and it isn't this one, try to use the language it prefers.
                        mainBundleLangs = _CFBundleCopyLanguageSearchListInBundle(mainBundle);
                    }
                    CFRelease(mainBundleURL);
                }
            }
        }
        
        if (mainBundleLangs) {
            if (CFArrayGetCount(mainBundleLangs) > 0) {
                result = _CFBundleCreateMutableArrayOfFallbackLanguages(searchLanguages, (CFStringRef)CFArrayGetValueAtIndex(mainBundleLangs, 0));
            }
        }
    }
    
    if (!result) {
        // If we didn't find the main bundle's preferred language, look at the users' prefs again and find the best one.
        if (userLanguages) {
            CFIndex count = CFArrayGetCount(userLanguages);
            for (CFIndex i = 0; i < count; i++) {
                CFStringRef curLangStr = (CFStringRef)CFArrayGetValueAtIndex(userLanguages, i);
                result = _CFBundleCreateMutableArrayOfFallbackLanguages(searchLanguages, curLangStr);
                
                if (result) break;
            }
        }
        
        // use development region and U.S. English as backstops
        if (!result && devLang) {
            result = _CFBundleCreateMutableArrayOfFallbackLanguages(searchLanguages, devLang);
        }
        
        if (!result) {
            result = _CFBundleCreateMutableArrayOfFallbackLanguages(searchLanguages, CFSTR("en_US"));
        }
    }
    
    if (!result) {
        result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    }
    
    
    if (mainBundleLangs) CFRelease(mainBundleLangs);
    
    return result;
}

// userLanguages must be non-NULL
static CFArrayRef _CFBundleCopyLocalizationsForPreferences(CFArrayRef localizations, CFArrayRef userLanguages, Boolean considerMain) {
    CFMutableArrayRef result = NULL;
    
    if (localizations && CFArrayGetCount(localizations) > 0) {
        result = _CFBundleCopyPreferredLanguagesInList(localizations, NULL, userLanguages, considerMain, NULL, NULL);
        
        // Additional backstop behavior: use first entry as backstop
        if (CFArrayGetCount(result) == 0 && CFArrayGetCount(localizations) > 0) {
            CFArrayAppendValue(result, CFArrayGetValueAtIndex(localizations, 0));
        } else if (CFArrayGetCount(result) == 0) {
            // Total backstop behavior to avoid having an empty array.
            CFArrayAppendValue(result, CFSTR("en"));
        }
    }

    if (!result) {
        // Total backstop behavior to avoid having an empty array.
        result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
        CFArrayAppendValue(result, CFSTR("en"));
    }
    
    return result;
}

CF_EXPORT CFArrayRef CFBundleCopyLocalizationsForPreferences(CFArrayRef localizations, CFArrayRef preferredLocalizations) {
    // NOTE: This function has an interesting side effect; passing NULL for preferredLocalizations will still use the user's current set of preferences, but it will ignore the main bundle for the purposes of matching languages. This is something that people rely upon.
    // Given an array of possible localizations, returns the one or more of them that CFBundle would use, without reference to the current application context, if the user's preferred localizations were given by prefArray. If prefArray is NULL, the current user's actual preferred localizations will be used. This is not the same as CFBundleCopyPreferredLocalizationsFromArray(), because that function takes the current application context into account. To determine the localizations that another application would use, apply this function to the result of CFBundleCopyBundleLocalizations().

    if (preferredLocalizations) {
        return _CFBundleCopyLocalizationsForPreferences(localizations, preferredLocalizations, false);
    } else {
        CFArrayRef defaultPreferredLocalizations = _CFBundleCopyUserLanguages();
        if (!defaultPreferredLocalizations) defaultPreferredLocalizations = CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
        CFArrayRef result = _CFBundleCopyLocalizationsForPreferences(localizations, defaultPreferredLocalizations, false);
        CFRelease(defaultPreferredLocalizations);
        return result;
    }
}

CF_EXPORT CFArrayRef CFBundleCopyPreferredLocalizationsFromArray(CFArrayRef localizations) {
    // Given an array of possible localizations, returns the one or more of them that CFBundle would use in the current application context. To determine the localizations that would be used for a particular bundle in the current application context, apply this function to the result of CFBundleCopyBundleLocalizations().
    // NOTE: Current application context refers to both using the main bundle and also using the preferred languages for the user
    CFArrayRef preferredLocalizations = _CFBundleCopyUserLanguages();
    if (!preferredLocalizations) preferredLocalizations = CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
    
    CFArrayRef result = _CFBundleCopyLocalizationsForPreferences(localizations, preferredLocalizations, true);
    
    CFRelease(preferredLocalizations);
    return result;
}

static CFStringRef _defaultLocalization = NULL;

CF_EXPORT void _CFBundleSetDefaultLocalization(CFStringRef localizationName) {
    CFStringRef newLocalization = localizationName ? (CFStringRef)CFStringCreateCopy(kCFAllocatorSystemDefault, localizationName) : NULL;
    if (_defaultLocalization) CFRelease(_defaultLocalization);
    _defaultLocalization = newLocalization;
}

#pragma mark -



// This is the funnel point for looking up languages for a particular bundle.
CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInBundle(CFBundleRef bundle) {
    if (!bundle->_searchLanguages) {
#if DEPLOYMENT_TARGET_WINDOWS
        if (_defaultLocalization) CFArrayAppendValue(langs, _defaultLocalization);
#endif
        // includes predefined localizations
        CFArrayRef localizationsForBundle = CFBundleCopyBundleLocalizations(bundle);
        CFArrayRef userLanguages = _CFBundleCopyUserLanguages();
        CFStringRef devLang = CFBundleGetDevelopmentRegion(bundle);
        
        CFMutableArrayRef result = _CFBundleCopyPreferredLanguagesInList(localizationsForBundle, devLang, userLanguages, true, bundle->_url, bundle);
        
        if (CFArrayGetCount(result) == 0) {
            // If the user does not prefer any of our languages, and devLang is not present, try English
            CFRelease(result);
            result = _CFBundleCopyPreferredLanguagesInList(localizationsForBundle, CFSTR("en_US"), userLanguages, true, bundle->_url, bundle);
        }
        
        // if none of the preferred localizations are present, fall back on a random localization that is present
        if (CFArrayGetCount(result) == 0 && localizationsForBundle && CFArrayGetCount(localizationsForBundle) > 0) {
            CFStringRef firstLocalization = (CFStringRef)CFArrayGetValueAtIndex(localizationsForBundle, 0);
            CFRelease(result);
            result = _CFBundleCopyPreferredLanguagesInList(localizationsForBundle, firstLocalization, userLanguages, true, bundle->_url, bundle);
        }
        
        if (userLanguages) CFRelease(userLanguages);
        
        if (devLang && !CFArrayContainsValue(result, CFRangeMake(0, CFArrayGetCount(result)), devLang)) {
            // Make sure that devLang is on the list as a fallback for individual resources that are not present
            CFArrayAppendValue(result, devLang);
        } else if (!devLang) {
            if (localizationsForBundle) {
                CFStringRef en_US = CFSTR("en_US"), en = CFSTR("en"), English = CFSTR("English");
                CFRange range = CFRangeMake(0, CFArrayGetCount(localizationsForBundle));
                if (CFArrayContainsValue(localizationsForBundle, range, en)) {
                    if (!CFArrayContainsValue(result, CFRangeMake(0, CFArrayGetCount(result)), en)) CFArrayAppendValue(result, en);
                } else if (CFArrayContainsValue(localizationsForBundle, range, English)) {
                    if (!CFArrayContainsValue(result, CFRangeMake(0, CFArrayGetCount(result)), English)) CFArrayAppendValue(result, English);
                } else if (CFArrayContainsValue(localizationsForBundle, range, en_US)) {
                    if (!CFArrayContainsValue(result, CFRangeMake(0, CFArrayGetCount(result)), en_US)) CFArrayAppendValue(result, en_US);
                }
            }
        }
        
        if (localizationsForBundle) CFRelease(localizationsForBundle);
        
        if (CFArrayGetCount(result) == 0) {
            // Total backstop behavior to avoid having an empty array.
            if (_defaultLocalization) {
                CFArrayAppendValue(result, _defaultLocalization);
            } else {
                CFArrayAppendValue(result, CFSTR("en"));
            }
        }
        
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, (void *)result, (void * volatile *)&(bundle->_searchLanguages))) {
            CFRelease(result);
        }
    }
    return (CFArrayRef)CFRetain(bundle->_searchLanguages);
}

// This is the funnel point for looking up languages for a particular directory.
CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInDirectory(CFURLRef url, uint8_t *version) {
    uint8_t localVersion = 0;
    CFDictionaryRef infoDict = _CFBundleCopyInfoDictionaryInDirectory(kCFAllocatorSystemDefault, url, &localVersion);
    
    CFArrayRef predefinedLocalizations = NULL;
    CFStringRef devLang = NULL;
    if (infoDict) {
        devLang = (CFStringRef)CFDictionaryGetValue(infoDict, kCFBundleDevelopmentRegionKey);
        if (devLang && (CFGetTypeID(devLang) != CFStringGetTypeID() || CFStringGetLength(devLang) == 0)) devLang = NULL;
        
        predefinedLocalizations = (CFArrayRef)CFDictionaryGetValue(infoDict, kCFBundleLocalizationsKey);
        if (predefinedLocalizations && CFGetTypeID(predefinedLocalizations) != CFArrayGetTypeID()) {
            predefinedLocalizations = NULL;
        }
    }
    
    CFURLRef resourcesURL = _CFBundleCopyResourcesDirectoryURLInDirectory(url, localVersion);
    CFArrayRef localizationsInDirectory = _CFBundleCopyLProjDirectoriesForURL(kCFAllocatorSystemDefault, resourcesURL);
    CFRelease(resourcesURL);
    
    if (predefinedLocalizations && localizationsInDirectory) {
        CFMutableArrayRef newLocalizations = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, 0, predefinedLocalizations);
        CFArrayAppendArray(newLocalizations, localizationsInDirectory, CFRangeMake(0, CFArrayGetCount(localizationsInDirectory)));
        CFRelease(localizationsInDirectory);
        localizationsInDirectory = (CFArrayRef)newLocalizations;
    } else if (predefinedLocalizations) {
        localizationsInDirectory = (CFArrayRef)CFRetain(predefinedLocalizations);
    } else if (!localizationsInDirectory) {
        localizationsInDirectory = CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
    }
    
    CFArrayRef userLanguages = _CFBundleCopyUserLanguages();
    CFMutableArrayRef result = _CFBundleCopyPreferredLanguagesInList(localizationsInDirectory, devLang, userLanguages, true, url, NULL);
    
    if (userLanguages) CFRelease(userLanguages);
    CFRelease(localizationsInDirectory);
    
    if (devLang && CFArrayGetFirstIndexOfValue(result, CFRangeMake(0, CFArrayGetCount(result)), devLang) < 0) CFArrayAppendValue(result, devLang);
    
    // Total backstop behavior to avoid having an empty array.
    if (CFArrayGetCount(result) == 0) CFArrayAppendValue(result, CFSTR("en"));
    
    if (infoDict) CFRelease(infoDict);
    if (version) *version = localVersion;
    return result;
}

