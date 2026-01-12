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

/*
    CFLocaleIdentifier.c
	Copyright (c) 2002-2014, Apple Inc. All rights reserved.
    Responsibility: David Smith
    
    CFLocaleIdentifier.c defines
    - enum value kLocaleIdentifierCStringMax
    - structs KeyStringToResultString, SpecialCaseUpdates
    and provides the following data for the functions
    CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes,
    CFLocaleCreateCanonicalLocaleIdentifierFromString
    CFLocaleCreateCanonicalLanguageIdentifierFromString

    1. static const char * regionCodeToLocaleString[]; enum kNumRegionCodeToLocaleString;
        map RegionCode 0..kNumRegionCodeToLocaleString-1 to canonical locale string

    2. static const char * langCodeToLocaleString[]; enum kNumLangCodeToLocaleString;
        map LangCode 0..kNumLangCodeToLocaleString-1 to canonical locale string

    3. static const KeyStringToResultString oldAppleLocaleToCanonical[]; enum kNumOldAppleLocaleToCanonical;
        map old Apple string        oldAppleLocaleToCanonical[n].key
        to canonical locale string  oldAppleLocaleToCanonical[n].result
        for n = 0..kNumOldAppleLocaleToCanonical-1

    4. static const KeyStringToResultString localeStringPrefixToCanonical[]; enum kNumLocaleStringPrefixToCanonical;
        map non-canonical language prefix (3-letter, obsolete)  localeStringPrefixToCanonical[].key
        to updated replacement                                  localeStringPrefixToCanonical[].result
        for n = 0..kNumLocaleStringPrefixToCanonical-1

    5. static const SpecialCaseUpdates specialCases[];
        various special cases for updating region codes, or for updating language codes based on region codes

    6. static const KeyStringToResultString localeStringRegionToDefaults[]; enum kNumLocaleStringRegionToDefaults;
        map locale string region tag    localeStringRegionToDefaults[n].key
        to default substrings to delete localeStringRegionToDefaults[n].result
        for n = 0..kNumLocaleStringRegionToDefaults-1

    7. static const KeyStringToResultString localeStringPrefixToDefaults[]; enum kNumLocaleStringPrefixToDefaults;
        map locale string initial part  localeStringPrefixToDefaults[n].key
        to default substrings to delete localeStringPrefixToDefaults[n].result
        for n = 0..kNumLocaleStringPrefixToDefaults-1
    
    8. static const KeyStringToResultString appleLocaleToLanguageString[]; enum kNumAppleLocaleToLanguageString;
        map Apple locale string         appleLocaleToLanguageString[].key
        to equivalent language string   appleLocaleToLanguageString[].result
        for n = 0..kNumAppleLocaleToLanguageString-1
    
*/

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFCalendar.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
#include <unicode/uloc.h>
#else
#define ULOC_KEYWORD_SEPARATOR '@'
#define ULOC_FULLNAME_CAPACITY 56
#define ULOC_KEYWORD_AND_VALUES_CAPACITY 100
#endif
#include "CFInternal.h"
#include "CFLocaleInternal.h"

// Max byte length of locale identifier (ASCII) as C string, including terminating null byte
enum {
    kLocaleIdentifierCStringMax = ULOC_FULLNAME_CAPACITY + ULOC_KEYWORD_AND_VALUES_CAPACITY	// currently 56 + 100
};

// KeyStringToResultString struct used in data tables for CFLocaleCreateCanonicalLocaleIdentifierFromString
struct KeyStringToResultString {
    const char *    key;
    const char *    result;
};
typedef struct KeyStringToResultString KeyStringToResultString;

// SpecialCaseUpdates struct used in data tables for CFLocaleCreateCanonicalLocaleIdentifierFromString
struct SpecialCaseUpdates {
    const char *    lang;
    const char *    reg1;
    const char *    update1;
    const char *    reg2;
    const char *    update2;
};
typedef struct SpecialCaseUpdates SpecialCaseUpdates;


static const char * const regionCodeToLocaleString[] = {
// map RegionCode (array index) to canonical locale string
//
//  canon. string      region code;             language code;      [comment]       [   # __CFBundleLocaleAbbreviationsArray
//  --------           ------------             ------------------  ------------        --------     string, if different ]
    "en_US",        //   0 verUS;                 0 langEnglish;
    "fr_FR",        //   1 verFrance;             1 langFrench;
    "en_GB",        //   2 verBritain;            0 langEnglish;
    "de_DE",        //   3 verGermany;            2 langGerman;
    "it_IT",        //   4 verItaly;              3 langItalian;
    "nl_NL",        //   5 verNetherlands;        4 langDutch;
    "nl_BE",        //   6 verFlemish;           34 langFlemish (redundant, =Dutch);
    "sv_SE",        //   7 verSweden;             5 langSwedish;
    "es_ES",        //   8 verSpain;              6 langSpanish;
    "da_DK",        //   9 verDenmark;            7 langDanish;
    "pt_PT",        //  10 verPortugal;           8 langPortuguese;
    "fr_CA",        //  11 verFrCanada;           1 langFrench;
    "nb_NO",        //  12 verNorway;             9 langNorwegian (Bokmal);             # "no_NO"
    "he_IL",        //  13 verIsrael;            10 langHebrew;
    "ja_JP",        //  14 verJapan;             11 langJapanese;
    "en_AU",        //  15 verAustralia;          0 langEnglish;
    "ar",           //  16 verArabic;            12 langArabic;
    "fi_FI",        //  17 verFinland;           13 langFinnish;
    "fr_CH",        //  18 verFrSwiss;            1 langFrench;
    "de_CH",        //  19 verGrSwiss;            2 langGerman;
    "el_GR",        //  20 verGreece;            14 langGreek (modern)-Grek-mono;
    "is_IS",        //  21 verIceland;           15 langIcelandic;
    "mt_MT",        //  22 verMalta;             16 langMaltese;
    "el_CY",        //  23 verCyprus;            14 langGreek?;     el or tr? guess el  # ""
    "tr_TR",        //  24 verTurkey;            17 langTurkish;
    "hr_HR",        //  25 verYugoCroatian;      18 langCroatian;   * one-way mapping -> verCroatia
    "nl_NL",        //  26 KCHR, Netherlands;     4 langDutch;      * one-way mapping
    "nl_BE",        //  27 KCHR, verFlemish;     34 langFlemish;    * one-way mapping
    "_CA",          //  28 KCHR, Canada-en/fr?;  -1 none;           * one-way mapping   # "en_CA"
    "_CA",          //  29 KCHR, Canada-en/fr?;  -1 none;           * one-way mapping   # "en_CA"
    "pt_PT",        //  30 KCHR, Portugal;        8 langPortuguese; * one-way mapping
    "nb_NO",        //  31 KCHR, Norway;          9 langNorwegian (Bokmal); * one-way mapping   # "no_NO"
    "da_DK",        //  32 KCHR, Denmark;         7 langDanish;     * one-way mapping
    "hi_IN",        //  33 verIndiaHindi;        21 langHindi;
    "ur_PK",        //  34 verPakistanUrdu;      20 langUrdu;
    "tr_TR",        //  35 verTurkishModified;   17 langTurkish;    * one-way mapping
    "it_CH",        //  36 verItalianSwiss;       3 langItalian;
    "en_001",       //  37 verInternational;      0 langEnglish; ASCII only             # "en"
    NULL,           //  38 *unassigned;          -1 none;           * one-way mapping   # ""
    "ro_RO",        //  39 verRomania;           37 langRomanian;
    "grc",          //  40 verGreekAncient;     148 langGreekAncient -Grek-poly;        # "el_GR"
    "lt_LT",        //  41 verLithuania;         24 langLithuanian;
    "pl_PL",        //  42 verPoland;            25 langPolish;
    "hu_HU",        //  43 verHungary;           26 langHungarian;
    "et_EE",        //  44 verEstonia;           27 langEstonian;
    "lv_LV",        //  45 verLatvia;            28 langLatvian;
    "se",           //  46 verSami;              29 langSami;
    "fo_FO",        //  47 verFaroeIsl;          30 langFaroese;
    "fa_IR",        //  48 verIran;              31 langFarsi/Persian;
    "ru_RU",        //  49 verRussia;            32 langRussian;
    "ga_IE",        //  50 verIreland;           35 langIrishGaelic (no dots);
    "ko_KR",        //  51 verKorea;             23 langKorean;
    "zh_CN",        //  52 verChina;             33 langSimpChinese;
    "zh_TW",        //  53 verTaiwan;            19 langTradChinese;
    "th_TH",        //  54 verThailand;          22 langThai;
    "und",          //  55 verScriptGeneric;     -1 none;                               # ""        // <1.9>
    "cs_CZ",        //  56 verCzech;             38 langCzech;
    "sk_SK",        //  57 verSlovak;            39 langSlovak;
    "und",          //  58 verEastAsiaGeneric;   -1 none;           * one-way mapping   # ""        // <1.9>
    "hu_HU",        //  59 verMagyar;            26 langHungarian;  * one-way mapping -> verHungary
    "bn",           //  60 verBengali;           67 langBengali;    _IN or _BD? guess generic
    "be_BY",        //  61 verBelarus;           46 langBelorussian;
    "uk_UA",        //  62 verUkraine;           45 langUkrainian;
    NULL,           //  63 *unused;              -1 none;           * one-way mapping   # ""
    "el_GR",        //  64 verGreeceAlt;         14 langGreek (modern)-Grek-mono;   * one-way mapping
    "sr_RS",        //  65 verSerbian;           42 langSerbian -Cyrl;								// <1.18>
    "sl_SI",        //  66 verSlovenian;         40 langSlovenian;
    "mk_MK",        //  67 verMacedonian;        43 langMacedonian;
    "hr_HR",        //  68 verCroatia;           18 langCroatian;
    NULL,           //  69 *unused;              -1 none;           * one-way mapping   # ""
    "de-1996",      //  70 verGermanReformed;     2 langGerman;     1996 orthogr.       # "de_DE"
    "pt_BR",        //  71 verBrazil;             8 langPortuguese;
    "bg_BG",        //  72 verBulgaria;          44 langBulgarian;
    "ca_ES",        //  73 verCatalonia;        130 langCatalan;
    "mul",          //  74 verMultilingual;      -1 none;                               # ""
    "gd",           //  75 verScottishGaelic;   144 langScottishGaelic;
    "gv",           //  76 verManxGaelic;       145 langManxGaelic;
    "br",           //  77 verBreton;           142 langBreton;
    "iu_CA",        //  78 verNunavut;          143 langInuktitut -Cans;
    "cy",           //  79 verWelsh;            128 langWelsh;
    "_CA",          //  80 KCHR, Canada-en/fr?;  -1 none;           * one-way mapping   # "en_CA"
    "ga-Latg_IE",   //  81 verIrishGaelicScrip; 146 langIrishGaelicScript -dots;        # "ga_IE"   // <xx>
    "en_CA",        //  82 verEngCanada;          0 langEnglish;
    "dz_BT",        //  83 verBhutan;           137 langDzongkha;
    "hy_AM",        //  84 verArmenian;          51 langArmenian;
    "ka_GE",        //  85 verGeorgian;          52 langGeorgian;
    "es_419",       //  86 verSpLatinAmerica;     6 langSpanish;                        # "es"
    "es_ES",        //  87 KCHR, Spain;           6 langSpanish;    * one-way mapping
    "to_TO",        //  88 verTonga;            147 langTongan;
    "pl_PL",        //  89 KCHR, Poland;         25 langPolish;     * one-way mapping
    "ca_ES",        //  90 KCHR, Catalonia;     130 langCatalan;    * one-way mapping
    "fr_001",       //  91 verFrenchUniversal;    1 langFrench;
    "de_AT",        //  92 verAustria;            2 langGerman;
    "es_419",       //  93 > verSpLatinAmerica;   6 langSpanish;    * one-way mapping   # "es"
    "gu_IN",        //  94 verGujarati;          69 langGujarati;
    "pa",           //  95 verPunjabi;           70 langPunjabi;    _IN or _PK? guess generic
    "ur_IN",        //  96 verIndiaUrdu;         20 langUrdu;
    "vi_VN",        //  97 verVietnam;           80 langVietnamese;
    "fr_BE",        //  98 verFrBelgium;          1 langFrench;
    "uz_UZ",        //  99 verUzbek;             47 langUzbek;
    "en_SG",        // 100 verSingapore;          0 langEnglish?; en, zh, or ms? guess en   # ""
    "nn_NO",        // 101 verNynorsk;          151 langNynorsk;                        # ""
    "af_ZA",        // 102 verAfrikaans;        141 langAfrikaans;
    "eo",           // 103 verEsperanto;         94 langEsperanto;
    "mr_IN",        // 104 verMarathi;           66 langMarathi;
    "bo",           // 105 verTibetan;           63 langTibetan;
    "ne_NP",        // 106 verNepal;             64 langNepali;
    "kl",           // 107 verGreenland;        149 langGreenlandic;
    "en_IE",        // 108 verIrelandEnglish;     0 langEnglish;                        # (no entry)
};
enum {
    kNumRegionCodeToLocaleString = sizeof(regionCodeToLocaleString)/sizeof(char *)
};

static const char * const langCodeToLocaleString[] = {
// map LangCode (array index) to canonical locale string
//
//  canon. string   language code;                  [ comment]  [   # __CFBundleLanguageAbbreviationsArray
//  --------        --------------                  ----------      --------    string, if different ]
    "en",       //   0 langEnglish;
    "fr",       //   1 langFrench;
    "de",       //   2 langGerman;
    "it",       //   3 langItalian;
    "nl",       //   4 langDutch;
    "sv",       //   5 langSwedish;
    "es",       //   6 langSpanish;
    "da",       //   7 langDanish;
    "pt",       //   8 langPortuguese;
    "nb",       //   9 langNorwegian (Bokmal);                      # "no"
    "he",       //  10 langHebrew -Hebr;
    "ja",       //  11 langJapanese -Jpan;
    "ar",       //  12 langArabic -Arab;
    "fi",       //  13 langFinnish;
    "el",       //  14 langGreek (modern)-Grek-mono;
    "is",       //  15 langIcelandic;
    "mt",       //  16 langMaltese -Latn;
    "tr",       //  17 langTurkish -Latn;
    "hr",       //  18 langCroatian;
    "zh-Hant",  //  19 langTradChinese;                             # "zh"
    "ur",       //  20 langUrdu -Arab;
    "hi",       //  21 langHindi -Deva;
    "th",       //  22 langThai -Thai;
    "ko",       //  23 langKorean -Hang;
    "lt",       //  24 langLithuanian;
    "pl",       //  25 langPolish;
    "hu",       //  26 langHungarian;
    "et",       //  27 langEstonian;
    "lv",       //  28 langLatvian;
    "se",       //  29 langSami;
    "fo",       //  30 langFaroese;
    "fa",       //  31 langFarsi/Persian -Arab;
    "ru",       //  32 langRussian -Cyrl;
    "zh-Hans",  //  33 langSimpChinese;                             # "zh"
    "nl-BE",    //  34 langFlemish (redundant, =Dutch);             # "nl"
    "ga",       //  35 langIrishGaelic (no dots);
    "sq",       //  36 langAlbanian;                no region codes
    "ro",       //  37 langRomanian;
    "cs",       //  38 langCzech;
    "sk",       //  39 langSlovak;
    "sl",       //  40 langSlovenian;
    "yi",       //  41 langYiddish -Hebr;           no region codes
    "sr",       //  42 langSerbian -Cyrl;
    "mk",       //  43 langMacedonian -Cyrl;
    "bg",       //  44 langBulgarian -Cyrl;
    "uk",       //  45 langUkrainian -Cyrl;
    "be",       //  46 langBelorussian -Cyrl;
    "uz",       //  47 langUzbek -Cyrl;             also -Latn, -Arab
    "kk",       //  48 langKazakh -Cyrl;            no region codes; also -Latn, -Arab
    "az-Cyrl",  //  49 langAzerbaijani -Cyrl;       no region codes # "az"
    "az-Arab",  //  50 langAzerbaijanAr -Arab;      no region codes # "az"
    "hy",       //  51 langArmenian -Armn;
    "ka",       //  52 langGeorgian -Geor;
    "mo",       //  53 langMoldavian -Cyrl;         no region codes
    "ky",       //  54 langKirghiz -Cyrl;           no region codes; also -Latn, -Arab
    "tg",       //  55 langTajiki -Cyrl;            no region codes; also -Latn, -Arab
    "tk-Cyrl",  //  56 langTurkmen -Cyrl;           no region codes; also -Latn, -Arab
    "mn-Mong",  //  57 langMongolian -Mong;         no region codes # "mn"
    "mn",       //  58 langMongolianCyr -Cyrl;      no region codes # "mn"
    "ps",       //  59 langPashto -Arab;            no region codes
    "ku",       //  60 langKurdish -Arab;           no region codes
    "ks",       //  61 langKashmiri -Arab;          no region codes
    "sd",       //  62 langSindhi -Arab;            no region codes
    "bo",       //  63 langTibetan -Tibt;
    "ne",       //  64 langNepali -Deva;
    "sa",       //  65 langSanskrit -Deva;          no region codes
    "mr",       //  66 langMarathi -Deva;
    "bn",       //  67 langBengali -Beng;
    "as",       //  68 langAssamese -Beng;          no region codes
    "gu",       //  69 langGujarati -Gujr;
    "pa",       //  70 langPunjabi -Guru;
    "or",       //  71 langOriya -Orya;             no region codes
    "ml",       //  72 langMalayalam -Mlym;         no region codes
    "kn",       //  73 langKannada -Knda;           no region codes
    "ta",       //  74 langTamil -Taml;             no region codes
    "te",       //  75 langTelugu -Telu;            no region codes
    "si",       //  76 langSinhalese -Sinh;         no region codes
    "my",       //  77 langBurmese -Mymr;           no region codes
    "km",       //  78 langKhmer -Khmr;             no region codes
    "lo",       //  79 langLao -Laoo;               no region codes
    "vi",       //  80 langVietnamese -Latn;
    "id",       //  81 langIndonesian -Latn;        no region codes
    "fil",      //  82 langTagalog -Latn;           no region codes
    "ms",       //  83 langMalayRoman -Latn;        no region codes # "ms"
    "ms-Arab",  //  84 langMalayArabic -Arab;       no region codes # "ms"
    "am",       //  85 langAmharic -Ethi;           no region codes
    "ti",       //  86 langTigrinya -Ethi;          no region codes
    "om",       //  87 langOromo -Ethi;             no region codes
    "so",       //  88 langSomali -Latn;            no region codes
    "sw",       //  89 langSwahili -Latn;           no region codes
    "rw",       //  90 langKinyarwanda -Latn;       no region codes
    "rn",       //  91 langRundi -Latn;             no region codes
    "ny",       //  92 langNyanja/Chewa -Latn;      no region codes # ""
    "mg",       //  93 langMalagasy -Latn;          no region codes
    "eo",       //  94 langEsperanto -Latn;
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, //  95 to 105 (gap)
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, // 106 to 116 (gap)
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, // 107 to 117 (gap)
    "cy",       // 128 langWelsh -Latn;
    "eu",       // 129 langBasque -Latn;            no region codes
    "ca",       // 130 langCatalan -Latn;
    "la",       // 131 langLatin -Latn;             no region codes
    "qu",       // 132 langQuechua -Latn;           no region codes
    "gn",       // 133 langGuarani -Latn;           no region codes
    "ay",       // 134 langAymara -Latn;            no region codes
    "tt-Cyrl",  // 135 langTatar -Cyrl;             no region codes
    "ug",       // 136 langUighur -Arab;            no region codes
    "dz",       // 137 langDzongkha -Tibt;
    "jv",       // 138 langJavaneseRom -Latn;       no region codes
    "su",       // 139 langSundaneseRom -Latn;      no region codes
    "gl",       // 140 langGalician -Latn;          no region codes
    "af",       // 141 langAfrikaans -Latn;
    "br",       // 142 langBreton -Latn;
    "iu",       // 143 langInuktitut -Cans;
    "gd",       // 144 langScottishGaelic;
    "gv",       // 145 langManxGaelic -Latn;
    "ga-Latg",  // 146 langIrishGaelicScript  -Latn-dots;           # "ga"                                      // <xx>
    "to",       // 147 langTongan -Latn;
    "grc",      // 148 langGreekAncient -Grek-poly;                 # "el"
    "kl",       // 149 langGreenlandic -Latn;
    "az",       // 150 langAzerbaijanRoman -Latn;   no region codes # "az"
    "nn",       // 151 langNynorsk -Latn;                           # (no entry)
};
enum {
    kNumLangCodeToLocaleString = sizeof(langCodeToLocaleString)/sizeof(char *)
};

static const KeyStringToResultString oldAppleLocaleToCanonical[] = {
// Map obsolete/old-style Apple strings to canonical
// Must be sorted according to how strcmp compares the strings in the first column
//
//    non-canonical             canonical       [  comment ]            # source/reason for non-canonical string
//    string                    string
//    -------------             ---------
    { "Afrikaans",              "af"        },  //                      # __CFBundleLanguageNamesArray
    { "Albanian",               "sq"        },  //                      # __CFBundleLanguageNamesArray
    { "Amharic",                "am"        },  //                      # __CFBundleLanguageNamesArray
    { "Arabic",                 "ar"        },  //                      # __CFBundleLanguageNamesArray
    { "Armenian",               "hy"        },  //                      # __CFBundleLanguageNamesArray
    { "Assamese",               "as"        },  //                      # __CFBundleLanguageNamesArray
    { "Aymara",                 "ay"        },  //                      # __CFBundleLanguageNamesArray
    { "Azerbaijani",            "az"        },  // -Arab,-Cyrl,-Latn?   # __CFBundleLanguageNamesArray (had 3 entries "Azerbaijani" for "az-Arab", "az-Cyrl", "az-Latn")
    { "Basque",                 "eu"        },  //                      # __CFBundleLanguageNamesArray
    { "Belarusian",             "be"        },  //                      # handle other names
    { "Belorussian",            "be"        },  //                      # handle other names
    { "Bengali",                "bn"        },  //                      # __CFBundleLanguageNamesArray
    { "Brazilian Portugese",    "pt-BR"     },  //                      # from Installer.app Info.plist IFLanguages key, misspelled
    { "Brazilian Portuguese",   "pt-BR"     },  //                      # correct spelling for above
    { "Breton",                 "br"        },  //                      # __CFBundleLanguageNamesArray
    { "Bulgarian",              "bg"        },  //                      # __CFBundleLanguageNamesArray
    { "Burmese",                "my"        },  //                      # __CFBundleLanguageNamesArray
    { "Byelorussian",           "be"        },  //                      # __CFBundleLanguageNamesArray
    { "Catalan",                "ca"        },  //                      # __CFBundleLanguageNamesArray
    { "Chewa",                  "ny"        },  //                      # handle other names
    { "Chichewa",               "ny"        },  //                      # handle other names
    { "Chinese",                "zh"        },  // -Hans,-Hant?         # __CFBundleLanguageNamesArray (had 2 entries "Chinese" for "zh-Hant", "zh-Hans")
    { "Chinese, Simplified",    "zh-Hans"   },  //                      # from Installer.app Info.plist IFLanguages key
    { "Chinese, Traditional",   "zh-Hant"   },  //                      # correct spelling for below
    { "Chinese, Tradtional",    "zh-Hant"   },  //                      # from Installer.app Info.plist IFLanguages key, misspelled
    { "Croatian",               "hr"        },  //                      # __CFBundleLanguageNamesArray
    { "Czech",                  "cs"        },  //                      # __CFBundleLanguageNamesArray
    { "Danish",                 "da"        },  //                      # __CFBundleLanguageNamesArray
    { "Dutch",                  "nl"        },  //                      # __CFBundleLanguageNamesArray (had 2 entries "Dutch" for "nl", "nl-BE")
    { "Dzongkha",               "dz"        },  //                      # __CFBundleLanguageNamesArray
    { "English",                "en"        },  //                      # __CFBundleLanguageNamesArray
    { "Esperanto",              "eo"        },  //                      # __CFBundleLanguageNamesArray
    { "Estonian",               "et"        },  //                      # __CFBundleLanguageNamesArray
    { "Faroese",                "fo"        },  //                      # __CFBundleLanguageNamesArray
    { "Farsi",                  "fa"        },  //                      # __CFBundleLanguageNamesArray
    { "Finnish",                "fi"        },  //                      # __CFBundleLanguageNamesArray
    { "Flemish",                "nl-BE"     },  //                      # handle other names
    { "French",                 "fr"        },  //                      # __CFBundleLanguageNamesArray
    { "Galician",               "gl"        },  //                      # __CFBundleLanguageNamesArray
    { "Gallegan",               "gl"        },  //                      # handle other names
    { "Georgian",               "ka"        },  //                      # __CFBundleLanguageNamesArray
    { "German",                 "de"        },  //                      # __CFBundleLanguageNamesArray
    { "Greek",                  "el"        },  //                      # __CFBundleLanguageNamesArray (had 2 entries "Greek" for "el", "grc")
    { "Greenlandic",            "kl"        },  //                      # __CFBundleLanguageNamesArray
    { "Guarani",                "gn"        },  //                      # __CFBundleLanguageNamesArray
    { "Gujarati",               "gu"        },  //                      # __CFBundleLanguageNamesArray
    { "Hawaiian",               "haw"       },  //                      # handle new languages
    { "Hebrew",                 "he"        },  //                      # __CFBundleLanguageNamesArray
    { "Hindi",                  "hi"        },  //                      # __CFBundleLanguageNamesArray
    { "Hungarian",              "hu"        },  //                      # __CFBundleLanguageNamesArray
    { "Icelandic",              "is"        },  //                      # __CFBundleLanguageNamesArray
    { "Indonesian",             "id"        },  //                      # __CFBundleLanguageNamesArray
    { "Inuktitut",              "iu"        },  //                      # __CFBundleLanguageNamesArray
    { "Irish",                  "ga"        },  //                      # __CFBundleLanguageNamesArray (had 2 entries "Irish" for "ga", "ga-dots")
    { "Italian",                "it"        },  //                      # __CFBundleLanguageNamesArray
    { "Japanese",               "ja"        },  //                      # __CFBundleLanguageNamesArray
    { "Javanese",               "jv"        },  //                      # __CFBundleLanguageNamesArray
    { "Kalaallisut",            "kl"        },  //                      # handle other names
    { "Kannada",                "kn"        },  //                      # __CFBundleLanguageNamesArray
    { "Kashmiri",               "ks"        },  //                      # __CFBundleLanguageNamesArray
    { "Kazakh",                 "kk"        },  //                      # __CFBundleLanguageNamesArray
    { "Khmer",                  "km"        },  //                      # __CFBundleLanguageNamesArray
    { "Kinyarwanda",            "rw"        },  //                      # __CFBundleLanguageNamesArray
    { "Kirghiz",                "ky"        },  //                      # __CFBundleLanguageNamesArray
    { "Korean",                 "ko"        },  //                      # __CFBundleLanguageNamesArray
    { "Kurdish",                "ku"        },  //                      # __CFBundleLanguageNamesArray
    { "Lao",                    "lo"        },  //                      # __CFBundleLanguageNamesArray
    { "Latin",                  "la"        },  //                      # __CFBundleLanguageNamesArray
    { "Latvian",                "lv"        },  //                      # __CFBundleLanguageNamesArray
    { "Lithuanian",             "lt"        },  //                      # __CFBundleLanguageNamesArray
    { "Macedonian",             "mk"        },  //                      # __CFBundleLanguageNamesArray
    { "Malagasy",               "mg"        },  //                      # __CFBundleLanguageNamesArray
    { "Malay",                  "ms"        },  // -Latn,-Arab?         # __CFBundleLanguageNamesArray (had 2 entries "Malay" for "ms-Latn", "ms-Arab")
    { "Malayalam",              "ml"        },  //                      # __CFBundleLanguageNamesArray
    { "Maltese",                "mt"        },  //                      # __CFBundleLanguageNamesArray
    { "Manx",                   "gv"        },  //                      # __CFBundleLanguageNamesArray
    { "Marathi",                "mr"        },  //                      # __CFBundleLanguageNamesArray
    { "Moldavian",              "mo"        },  //                      # __CFBundleLanguageNamesArray
    { "Mongolian",              "mn"        },  // -Mong,-Cyrl?         # __CFBundleLanguageNamesArray (had 2 entries "Mongolian" for "mn-Mong", "mn-Cyrl")
    { "Nepali",                 "ne"        },  //                      # __CFBundleLanguageNamesArray
    { "Norwegian",              "nb"        },  //                      # __CFBundleLanguageNamesArray (had "Norwegian" mapping to "no")
    { "Nyanja",                 "ny"        },  //                      # __CFBundleLanguageNamesArray
    { "Nynorsk",                "nn"        },  //                      # handle other names (no entry in __CFBundleLanguageNamesArray)
    { "Oriya",                  "or"        },  //                      # __CFBundleLanguageNamesArray
    { "Oromo",                  "om"        },  //                      # __CFBundleLanguageNamesArray
    { "Panjabi",                "pa"        },  //                      # handle other names
    { "Pashto",                 "ps"        },  //                      # __CFBundleLanguageNamesArray
    { "Persian",                "fa"        },  //                      # handle other names
    { "Polish",                 "pl"        },  //                      # __CFBundleLanguageNamesArray
    { "Portuguese",             "pt"        },  //                      # __CFBundleLanguageNamesArray
    { "Portuguese, Brazilian",  "pt-BR"     },  //                      # handle other names
    { "Punjabi",                "pa"        },  //                      # __CFBundleLanguageNamesArray
    { "Pushto",                 "ps"        },  //                      # handle other names
    { "Quechua",                "qu"        },  //                      # __CFBundleLanguageNamesArray
    { "Romanian",               "ro"        },  //                      # __CFBundleLanguageNamesArray
    { "Ruanda",                 "rw"        },  //                      # handle other names
    { "Rundi",                  "rn"        },  //                      # __CFBundleLanguageNamesArray
    { "Russian",                "ru"        },  //                      # __CFBundleLanguageNamesArray
    { "Sami",                   "se"        },  //                      # __CFBundleLanguageNamesArray
    { "Sanskrit",               "sa"        },  //                      # __CFBundleLanguageNamesArray
    { "Scottish",               "gd"        },  //                      # __CFBundleLanguageNamesArray
    { "Serbian",                "sr"        },  //                      # __CFBundleLanguageNamesArray
    { "Simplified Chinese",     "zh-Hans"   },  //                      # handle other names
    { "Sindhi",                 "sd"        },  //                      # __CFBundleLanguageNamesArray
    { "Sinhalese",              "si"        },  //                      # __CFBundleLanguageNamesArray
    { "Slovak",                 "sk"        },  //                      # __CFBundleLanguageNamesArray
    { "Slovenian",              "sl"        },  //                      # __CFBundleLanguageNamesArray
    { "Somali",                 "so"        },  //                      # __CFBundleLanguageNamesArray
    { "Spanish",                "es"        },  //                      # __CFBundleLanguageNamesArray
    { "Sundanese",              "su"        },  //                      # __CFBundleLanguageNamesArray
    { "Swahili",                "sw"        },  //                      # __CFBundleLanguageNamesArray
    { "Swedish",                "sv"        },  //                      # __CFBundleLanguageNamesArray
    { "Tagalog",                "fil"       },  //                      # __CFBundleLanguageNamesArray
    { "Tajik",                  "tg"        },  //                      # handle other names
    { "Tajiki",                 "tg"        },  //                      # __CFBundleLanguageNamesArray
    { "Tamil",                  "ta"        },  //                      # __CFBundleLanguageNamesArray
    { "Tatar",                  "tt"        },  //                      # __CFBundleLanguageNamesArray
    { "Telugu",                 "te"        },  //                      # __CFBundleLanguageNamesArray
    { "Thai",                   "th"        },  //                      # __CFBundleLanguageNamesArray
    { "Tibetan",                "bo"        },  //                      # __CFBundleLanguageNamesArray
    { "Tigrinya",               "ti"        },  //                      # __CFBundleLanguageNamesArray
    { "Tongan",                 "to"        },  //                      # __CFBundleLanguageNamesArray
    { "Traditional Chinese",    "zh-Hant"   },  //                      # handle other names
    { "Turkish",                "tr"        },  //                      # __CFBundleLanguageNamesArray
    { "Turkmen",                "tk"        },  //                      # __CFBundleLanguageNamesArray
    { "Uighur",                 "ug"        },  //                      # __CFBundleLanguageNamesArray
    { "Ukrainian",              "uk"        },  //                      # __CFBundleLanguageNamesArray
    { "Urdu",                   "ur"        },  //                      # __CFBundleLanguageNamesArray
    { "Uzbek",                  "uz"        },  //                      # __CFBundleLanguageNamesArray
    { "Vietnamese",             "vi"        },  //                      # __CFBundleLanguageNamesArray
    { "Welsh",                  "cy"        },  //                      # __CFBundleLanguageNamesArray
    { "Yiddish",                "yi"        },  //                      # __CFBundleLanguageNamesArray
    { "ar_??",                  "ar"        },  //                      # from old MapScriptInfoAndISOCodes
    { "az.Ar",                  "az-Arab"   },  //                      # from old LocaleRefGetPartString
    { "az.Cy",                  "az-Cyrl"   },  //                      # from old LocaleRefGetPartString
    { "az.La",                  "az"        },  //                      # from old LocaleRefGetPartString
    { "be_??",                  "be_BY"     },  //                      # from old MapScriptInfoAndISOCodes
    { "bn_??",                  "bn"        },  //                      # from old LocaleRefGetPartString
    { "bo_??",                  "bo"        },  //                      # from old MapScriptInfoAndISOCodes
    { "br_??",                  "br"        },  //                      # from old MapScriptInfoAndISOCodes
    { "cy_??",                  "cy"        },  //                      # from old MapScriptInfoAndISOCodes
    { "de-96",                  "de-1996"   },  //                      # from old MapScriptInfoAndISOCodes                     // <1.9>
    { "de_96",                  "de-1996"   },  //                      # from old MapScriptInfoAndISOCodes                     // <1.9>
    { "de_??",                  "de-1996"   },  //                      # from old MapScriptInfoAndISOCodes
    { "el.El-P",                "grc"       },  //                      # from old LocaleRefGetPartString
    { "en-ascii",               "en_001"    },  //                      # from earlier version of tables in this file!
    { "en_??",                  "en_001"    },  //                      # from old MapScriptInfoAndISOCodes
    { "eo_??",                  "eo"        },  //                      # from old MapScriptInfoAndISOCodes
    { "es_??",                  "es_419"    },  //                      # from old MapScriptInfoAndISOCodes
    { "es_XL",                  "es_419"    },  //                      # from earlier version of tables in this file!
    { "fr_??",                  "fr_001"    },  //                      # from old MapScriptInfoAndISOCodes
    { "ga-dots",                "ga-Latg"   },  //                      # from earlier version of tables in this file!          // <1.8>
    { "ga-dots_IE",             "ga-Latg_IE" }, //                      # from earlier version of tables in this file!          // <1.8>
    { "ga.Lg",                  "ga-Latg"   },  //                      # from old LocaleRefGetPartString                       // <1.8>
    { "ga.Lg_IE",               "ga-Latg_IE" }, //                      # from old LocaleRefGetPartString                       // <1.8>
    { "gd_??",                  "gd"        },  //                      # from old MapScriptInfoAndISOCodes
    { "gv_??",                  "gv"        },  //                      # from old MapScriptInfoAndISOCodes
    { "jv.La",                  "jv"        },  //                      # logical extension                                     // <1.9>
    { "jw.La",                  "jv"        },  //                      # from old LocaleRefGetPartString
    { "kk.Cy",                  "kk"        },  //                      # from old LocaleRefGetPartString
    { "kl.La",                  "kl"        },  //                      # from old LocaleRefGetPartString
    { "kl.La_GL",               "kl_GL"     },  //                      # from old LocaleRefGetPartString                       // <1.9>
    { "lp_??",                  "se"        },  //                      # from old MapScriptInfoAndISOCodes
    { "mk_??",                  "mk_MK"     },  //                      # from old MapScriptInfoAndISOCodes
    { "mn.Cy",                  "mn"        },  //                      # from old LocaleRefGetPartString
    { "mn.Mn",                  "mn-Mong"   },  //                      # from old LocaleRefGetPartString
    { "ms.Ar",                  "ms-Arab"   },  //                      # from old LocaleRefGetPartString
    { "ms.La",                  "ms"        },  //                      # from old LocaleRefGetPartString
    { "nl-be",                  "nl-BE"     },  //                      # from old LocaleRefGetPartString
    { "nl-be_BE",               "nl_BE"     },  //                      # from old LocaleRefGetPartString
    { "no-NO",					"nb-NO"     },  //                      # not handled by localeStringPrefixToCanonical
    { "no-NO_NO",				"nb-NO_NO"  },  //                      # not handled by localeStringPrefixToCanonical
//  { "no-bok_NO",              "nb_NO"     },  //                      # from old LocaleRefGetPartString - handled by localeStringPrefixToCanonical
//  { "no-nyn_NO",              "nn_NO"     },  //                      # from old LocaleRefGetPartString - handled by localeStringPrefixToCanonical
//  { "nya",                    "ny"        },  //                      # from old LocaleRefGetPartString - handled by localeStringPrefixToCanonical
    { "pa_??",                  "pa"        },  //                      # from old LocaleRefGetPartString
    { "sa.Dv",                  "sa"        },  //                      # from old LocaleRefGetPartString
    { "sl_??",                  "sl_SI"     },  //                      # from old MapScriptInfoAndISOCodes
    { "sr_??",                  "sr_RS"     },  //                      # from old MapScriptInfoAndISOCodes						// <1.18>
    { "su.La",                  "su"        },  //                      # from old LocaleRefGetPartString
    { "yi.He",                  "yi"        },  //                      # from old LocaleRefGetPartString
    { "zh-simp",                "zh-Hans"   },  //                      # from earlier version of tables in this file!
    { "zh-trad",                "zh-Hant"   },  //                      # from earlier version of tables in this file!
    { "zh.Ha-S",                "zh-Hans"   },  //                      # from old LocaleRefGetPartString
    { "zh.Ha-S_CN",             "zh_CN"     },  //                      # from old LocaleRefGetPartString
    { "zh.Ha-T",                "zh-Hant"   },  //                      # from old LocaleRefGetPartString
    { "zh.Ha-T_TW",             "zh_TW"     },  //                      # from old LocaleRefGetPartString
};
enum {
    kNumOldAppleLocaleToCanonical = sizeof(oldAppleLocaleToCanonical)/sizeof(KeyStringToResultString)
};

static const KeyStringToResultString localeStringPrefixToCanonical[] = {
// Map 3-letter & obsolete ISO 639 codes, plus obsolete RFC 3066 codes, to 2-letter ISO 639 code.
// (special cases for 'sh' handled separately)
// First column must be all lowercase; must be sorted according to how strcmp compares the strings in the first column.
//
//    non-canonical canonical       [  comment ]                # source/reason for non-canonical string
//    prefix        prefix
//    ------------- ---------

    { "aar",        "aa"        },  // Afar
 //   { "aa_SAAHO",   "ssy"       },  // Saho                       # deprecated/grandfathered, handled as a special case
    { "abk",        "ab"        },  // Abkhazian
    { "afr",        "af"        },  // Afrikaans
    { "aju",        "jrb"       },  // Moroccan Judeo-Arabic -> Judeo-Arabic (macrolang.)
    { "aka",        "ak"        },  // Akan
    { "alb",        "sq"        },  // Albanian
    { "als",        "sq"        },  // Tosk Albanian -> Albanian (macrolang.)
    { "amh",        "am"        },  // Amharic
    { "ara",        "ar"        },  // Arabic
    { "arb",        "ar"        },  // Std Arabic -> Arabic (macrolang.)
    { "arg",        "an"        },  // Aragonese
    { "arm",        "hy"        },  // Armenian
    { "art-lojban", "jbo"       },  // Lojban                     # deprecated/grandfathered
    { "asm",        "as"        },  // Assamese
    { "ava",        "av"        },  // Avaric
    { "ave",        "ae"        },  // Avestan
    { "aym",        "ay"        },  // Aymara
    { "ayr",        "ay"        },  // Central Aymara -> Aymara (macrolang.)
    { "aze",        "az"        },  // Azerbaijani
    { "azj",        "az"        },  // N.Azerbaijani -> Azerbaijani (macrolang.)
    { "bak",        "ba"        },  // Bashkir
    { "bam",        "bm"        },  // Bambara
    { "baq",        "eu"        },  // Basque
    { "bcc",        "bal"       },  // Balochi, Southern -> Baluchi (macrolang.)
    { "bcl",        "bik"       },  // Bicolano, Central -> Bikol (macrolang.)
    { "bel",        "be"        },  // Belarusian
    { "ben",        "bn"        },  // Bengali
    { "bih",        "bh"        },  // Bihari
    { "bis",        "bi"        },  // Bislama
    { "bod",        "bo"        },  // Tibetan
    { "bos",        "bs"        },  // Bosnian
    { "bre",        "br"        },  // Breton
    { "bul",        "bg"        },  // Bulgarian
    { "bur",        "my"        },  // Burmese
    { "bxk",        "luy"       },  // Lubukusu -> Luyia (macrolang.)
    { "bxr",        "bua"       },  // Buriat, Russia -> Buriat (macrolang.)
    { "cat",        "ca"        },  // Catalan
    { "ces",        "cs"        },  // Czech
    { "cha",        "ch"        },  // Chamorro
    { "che",        "ce"        },  // Chechen
    { "chi",        "zh"        },  // Chinese
    { "chu",        "cu"        },  // Church Slavic, Church Slavonic, Old Bulgarian, Old Church Slavonic, Old Slavonic
    { "chv",        "cv"        },  // Chuvash
    { "cld",        "syr"       },  // Chaldean Neo-Aramaic -> Syriac (macrolang.)
    { "cmn",        "zh"        },  // Mandarin -> Chinese (macrolang.)
    { "cor",        "kw"        },  // Cornish
    { "cos",        "co"        },  // Corsican
    { "cre",        "cr"        },  // Cree
    { "cwd",        "cr"        },  // Cree, Woods -> Cree (macrolang.)
    { "cym",        "cy"        },  // Welsh
    { "cze",        "cs"        },  // Czech
    { "dan",        "da"        },  // Danish
    { "deu",        "de"        },  // German
    { "dgo",        "doi"       },  // Dogri -> Dogri (macrolang.)
    { "dhd",        "mwr"       },  // Dhundari -> Marwari (macrolang.)
    { "dik",        "din"       },  // Southwestern Dinka -> Dinka (macrolang.)
    { "diq",        "zza"       },  // Dimli -> Zaza (macrolang.)
    { "div",        "dv"        },  // Dhivehi, Divehi, Maldivian
    { "dut",        "nl"        },  // Dutch
    { "dzo",        "dz"        },  // Dzongkha
    { "ekk",        "et"        },  // Std Estonian -> Estonian (macrolang.)
    { "ell",        "el"        },  // Greek, Modern (1453-)
    { "emk",        "man"       },  // Maninkakan, Eastern -> Mandingo (macrolang.)
    { "eng",        "en"        },  // English
    { "epo",        "eo"        },  // Esperanto
    { "esk",        "ik"        },  // Northwest Alaska Inupiatun -> Inupiaq (macrolang.)
    { "est",        "et"        },  // Estonian
    { "eus",        "eu"        },  // Basque
    { "ewe",        "ee"        },  // Ewe
    { "fao",        "fo"        },  // Faroese
    { "fas",        "fa"        },  // Persian
    { "fat",        "ak"        },  // Fanti -> Akan (macrolang.)
    { "fij",        "fj"        },  // Fijian
    { "fin",        "fi"        },  // Finnish
    { "fra",        "fr"        },  // French
    { "fre",        "fr"        },  // French
    { "fry",        "fy"        },  // Western Frisian
    { "fuc",        "ff"        },  // Pular -> Fulah (macrolang.)
    { "ful",        "ff"        },  // Fulah
    { "gaz",        "om"        },  // W.Central Oromo -> Oromo (macrolang.)
    { "gbo",        "grb"       },  // Northern Grebo -> Grebo (macrolang.)
    { "geo",        "ka"        },  // Georgian
    { "ger",        "de"        },  // German
    { "gla",        "gd"        },  // Gaelic,Scottish
    { "gle",        "ga"        },  // Irish
    { "glg",        "gl"        },  // Gallegan
    { "glv",        "gv"        },  // Manx
    { "gno",        "gon"       },  // Northern Gondi -> Gondi (macrolang.)
    { "gre",        "el"        },  // Greek, Modern (1453-)
    { "grn",        "gn"        },  // Guarani
    { "gug",        "gn"        },  // Paraguayan Guarani -> Guarani (macrolang.)
    { "guj",        "gu"        },  // Gujarati
    { "gya",        "gba"       },  // Northwest Gbaya -> Gbaya (Cent. Afr. Rep.) (macrolang.)
    { "hat",        "ht"        },  // Haitian, Haitian Creole
    { "hau",        "ha"        },  // Hausa
    { "hbs",        "sr_Latn"   },  // Serbo-Croatian
    { "hdn",        "hai"       },  // Northern Haida -> Haida (macrolang.)
    { "hea",        "hmn"       },  // Northern Qiandong Miao -> Hmong (macrolang.)
    { "heb",        "he"        },  // Hebrew
    { "her",        "hz"        },  // Herero
    { "him",        "srx"       },  // Himachali -> Sirmauri (= Pahari, Himachali) (macrolang.)
    { "hin",        "hi"        },  // Hindi
    { "hmo",        "ho"        },  // Hiri Motu
    { "hrv",        "hr"        },  // Croatian
    { "hun",        "hu"        },  // Hungarian
    { "hye",        "hy"        },  // Armenian
    { "i-ami",      "ami"       },  // Amis                       # deprecated/grandfathered
    { "i-bnn",      "bnn"       },  // Bunun                      # deprecated/grandfathered
    { "i-hak",      "hak"       },  // Hakka                    # deprecated RFC 3066
    { "i-klingon",  "tlh"       },  // Klingon                    # deprecated/grandfathered
    { "i-lux",      "lb"        },  // Luxembourgish            # deprecated RFC 3066
    { "i-navajo",   "nv"        },  // Navajo                   # deprecated RFC 3066
    { "i-pwn",      "pwn"       },  // Paiwan                     # deprecated/grandfathered
    { "i-tao",      "tao"       },  // Tao                        # deprecated/grandfathered
    { "i-tay",      "tay"       },  // Tayal                      # deprecated/grandfathered
    { "i-tsu",      "tsu"       },  // Tsou                       # deprecated/grandfathered
    { "ibo",        "ig"        },  // Igbo
    { "ice",        "is"        },  // Icelandic
    { "ido",        "io"        },  // Ido
    { "iii",        "ii"        },  // Sichuan Yi, Nuosu
    { "ike",        "iu"        },  // E.Canada Inuktitut -> Inuktitut (macrolang.)
    { "iku",        "iu"        },  // Inuktitut
    { "ile",        "ie"        },  // Interlingue
    { "in",         "id"        },  // Indonesian               # deprecated 639 code in -> id (1989)
    { "ina",        "ia"        },  // Interlingua
    { "ind",        "id"        },  // Indonesian
    { "ipk",        "ik"        },  // Inupiaq
    { "isl",        "is"        },  // Icelandic
    { "ita",        "it"        },  // Italian
    { "iw",         "he"        },  // Hebrew                   # deprecated 639 code iw -> he (1989)
    { "jav",        "jv"        },  // Javanese
    { "jaw",        "jv"        },  // Javanese                 # deprecated 639 code jaw -> jv (2001)
    { "ji",         "yi"        },  // Yiddish                  # deprecated 639 code ji -> yi (1989)
    { "jpn",        "ja"        },  // Japanese
    { "jw",         "jv"        },  // Javanese                 # deprecated
    { "kal",        "kl"        },  // Kalaallisut
    { "kan",        "kn"        },  // Kannada
    { "kas",        "ks"        },  // Kashmiri
    { "kat",        "ka"        },  // Georgian
    { "kau",        "kr"        },  // Kanuri
    { "kaz",        "kk"        },  // Kazakh
    { "khk",        "mn"        },  // Halh Mongolian [mainly Cyrl] -> Mongolian (macrolang.)
    { "khm",        "km"        },  // Khmer
    { "kik",        "ki"        },  // Kikuyu, Gikuyu
    { "kin",        "rw"        },  // Kinyarwanda
    { "kir",        "ky"        },  // Kirghiz
    { "kmr",        "ku"        },  // Northern Kurdish -> Kurdish (macrolang.)
    { "knc",        "kr"        },  // Central Kanuri -> Kanuri (macrolang.)
    { "kng",        "kg"        },  // Koongo -> Kongo (macrolang.)
    { "knn",        "kok"       },  // Konkani (individ.lang) -> Konkani (macrolang.)
    { "kom",        "kv"        },  // Komi
    { "kon",        "kg"        },  // Kongo
    { "kor",        "ko"        },  // Korean
    { "kpv",        "kv"        },  // Komi-Zyrian -> Komi (macrolang.)
    { "kua",        "kj"        },  // Kuanyama, Kwanyama
    { "kur",        "ku"        },  // Kurdish
    { "lao",        "lo"        },  // Lao
    { "lat",        "la"        },  // Latin
    { "lav",        "lv"        },  // Latvian
    { "lbk",        "bnc"       },  // Central Bontok -> Bontok (macrolang.)
    { "lim",        "li"        },  // Limburgan, Limburger, Limburgish
    { "lin",        "ln"        },  // Lingala
    { "lit",        "lt"        },  // Lithuanian
    { "ltz",        "lb"        },  // Letzeburgesch
    { "lub",        "lu"        },  // Luba-Katanga
    { "lug",        "lg"        },  // Ganda
    { "lvs",        "lv"        },  // Std Latvian -> Latvian (macrolang.)
    { "mac",        "mk"        },  // Macedonian
    { "mal",        "ml"        },  // Malayalam
    { "mar",        "mr"        },  // Marathi
    { "may",        "ms"        },  // Malay
    { "mhr",        "chm"       },  // Mari, Eastern -> Mari (Russia) (macrolang.)
    { "mkd",        "mk"        },  // Macedonian
    { "mlg",        "mg"        },  // Malagasy
    { "mlt",        "mt"        },  // Maltese
    { "mol",        "mo"        },  // Moldavian
    { "mon",        "mn"        },  // Mongolian
    { "msa",        "ms"        },  // Malay
    { "mup",        "raj"       },  // Malvi -> Rajasthani (macrolang.)
    { "mya",        "my"        },  // Burmese
    { "nau",        "na"        },  // Nauru
    { "nav",        "nv"        },  // Navajo, Navaho
    { "nbl",        "nr"        },  // South Ndebele
    { "nde",        "nd"        },  // North Ndebele
    { "ndo",        "ng"        },  // Ndonga
    { "nep",        "ne"        },  // Nepali
    { "nld",        "nl"        },  // Dutch
    { "nno",        "nn"        },  // Norwegian Nynorsk
    { "no",         "nb"        },  // Norwegian generic        # ambiguous 639 code no -> nb
    { "no-bok",     "nb"        },  // Norwegian Bokmal         # deprecated RFC 3066 tag - used in old LocaleRefGetPartString
    { "no-nyn",     "nn"        },  // Norwegian Nynorsk        # deprecated RFC 3066 tag - used in old LocaleRefGetPartString
    { "nob",        "nb"        },  // Norwegian Bokmal
    { "nor",        "nb"        },  // Norwegian generic        # ambiguous 639 code nor -> nb
  //  { "no_BOKMAL",  "nb"        },  // Norwegian Bokmal           # deprecated/grandfathered, handled as a special case
  //  { "no_NYNORSK", "nn"        },  // Norwegian Nynorsk          # deprecated/grandfathered, handled as a special case
    { "nya",        "ny"        },  // Nyanja/Chewa/Chichewa    # 3-letter code used in old LocaleRefGetPartString
    { "oci",        "oc"        },  // Occitan/Provencal
    { "ojg",        "oj"        },  // Ojibwa, Eastern -> Ojibwa (macrolang.)
    { "oji",        "oj"        },  // Ojibwa
    { "ori",        "or"        },  // Oriya
    { "orm",        "om"        },  // Oromo,Galla
    { "oss",        "os"        },  // Ossetian, Ossetic
    { "pan",        "pa"        },  // Panjabi
    { "pbu",        "ps"        },  // N.Pashto, -> Pushto (macrolang.)
    { "per",        "fa"        },  // Persian
    { "pes",        "fa"        },  // W.Farsi -> Persian (macrolang.)
    { "pli",        "pi"        },  // Pali
    { "plt",        "mg"        },  // Plateau Malagasy -> Malagasy (macrolang.)
    { "pnb",        "lah"       },  // W.Panjabi -> Lahnda (macrolang.)
    { "pol",        "pl"        },  // Polish
    { "por",        "pt"        },  // Portuguese
    { "pus",        "ps"        },  // Pushto
    { "que",        "qu"        },  // Quechua
    { "qxp",        "qu"        },  // Puno Quechua -> Quechua (macrolang.)
    { "rmy",        "rom"       },  // Vlax Romani -> Romany (macrolang.)
    { "roh",        "rm"        },  // Raeto-Romance
    { "ron",        "ro"        },  // Romanian
    { "rum",        "ro"        },  // Romanian
    { "run",        "rn"        },  // Rundi
    { "rus",        "ru"        },  // Russian
    { "sag",        "sg"        },  // Sango
    { "san",        "sa"        },  // Sanskrit
    { "scc",        "sr"        },  // Serbian
    { "scr",        "hr"        },  // Croatian
    { "sgn-be-fr",  "sfb"       },  // Belgian-French Sign Lang.  # deprecated/grandfathered
    { "sgn-be-nl",  "vgt"       },  // Belgian-Flemish Sign Lang. # deprecated/grandfathered
    { "sgn-ch-de",  "sgg"       },  // Swiss German Sign Lang.    # deprecated/grandfathered
    { "sin",        "si"        },  // Sinhalese
    { "slk",        "sk"        },  // Slovak
    { "slo",        "sk"        },  // Slovak
    { "slv",        "sl"        },  // Slovenian
    { "sme",        "se"        },  // Sami,Northern
    { "smo",        "sm"        },  // Samoan
    { "sna",        "sn"        },  // Shona
    { "snd",        "sd"        },  // Sindhi
    { "som",        "so"        },  // Somali
    { "sot",        "st"        },  // Southern Sotho
    { "spa",        "es"        },  // Spanish
    { "spy",        "kln"       },  // Sabaot -> Kalenjin (macrolang.)
    { "sqi",        "sq"        },  // Albanian
    { "src",        "sc"        },  // Sardinian, Logudorese -> Sardinian (macrolang.)
    { "srd",        "sc"        },  // Sardinian
    { "srp",        "sr"        },  // Serbian
    { "ssw",        "ss"        },  // Swati
    { "sun",        "su"        },  // Sundanese
    { "swa",        "sw"        },  // Swahili
    { "swe",        "sv"        },  // Swedish
    { "swh",        "sw"        },  // Swahili (individ.lang) -> Swahili (macrolang.)
    { "tah",        "ty"        },  // Tahitian
    { "tam",        "ta"        },  // Tamil
    { "tat",        "tt"        },  // Tatar
    { "tel",        "te"        },  // Telugu
    { "tgk",        "tg"        },  // Tajik
    { "tgl",        "fil"       },  // Tagalog
    { "tha",        "th"        },  // Thai
    { "tib",        "bo"        },  // Tibetan
    { "tir",        "ti"        },  // Tigrinya
    { "tl",         "fil"       },  // Tagalog                  # legacy
    { "ton",        "to"        },  // Tongan
    { "tsn",        "tn"        },  // Tswana
    { "tso",        "ts"        },  // Tsonga
    { "ttq",        "tmh"       },  // Tamajaq, Tawallammat -> Tamashek (macrolang.)
    { "tuk",        "tk"        },  // Turkmen
    { "tur",        "tr"        },  // Turkish
    { "tw",         "ak"        },  // Twi -> Akan (macrolang.)
    { "twi",        "ak"        },  // Twi
    { "uig",        "ug"        },  // Uighur
    { "ukr",        "uk"        },  // Ukrainian
    { "umu",        "del"       },  // Munsee -> Delaware (macrolang.)
    { "urd",        "ur"        },  // Urdu
    { "uzb",        "uz"        },  // Uzbek
    { "uzn",        "uz"        },  // N. Uzbek -> Uzbek (macrolang.)
    { "ven",        "ve"        },  // Venda
    { "vie",        "vi"        },  // Vietnamese
    { "vol",        "vo"        },  // Volap√ºk
    { "wel",        "cy"        },  // Welsh
    { "wln",        "wa"        },  // Walloon
    { "wol",        "wo"        },  // Wolof
    { "xho",        "xh"        },  // Xhosa
    { "xpe",        "kpe"       },  // Kpelle, Liberia -> Kpelle (macrolang.)
    { "xsl",        "den"       },  // Slavey, South -> Slave (Athapascan) (macrolang.)
    { "ydd",        "yi"        },  // Yiddish,E. -> Yiddish (macrolang.)
    { "yid",        "yi"        },  // Yiddish
    { "yor",        "yo"        },  // Yoruba
    { "zai",        "zap"       },  // Zapotec, Isthmus -> Zapotec (macrolang.)
    { "zh-cdo",     "cdo"       },  // Chinese, Min Dong        # extlang
    { "zh-cjy",     "cjy"       },  // Chinese, Jinyu           # extlang
    { "zh-cmn",     "zh"        },  // Chinese, Mandarin        # extlang
    { "zh-cpx",     "cpx"       },  // Chinese, Pu-Xian         # extlang
    { "zh-czh",     "czh"       },  // Chinese, Huizhou         # extlang
    { "zh-czo",     "czo"       },  // Chinese, Min Zhong       # extlang
    { "zh-gan",     "gan"       },  // Chinese, Gan             # extlang
    { "zh-guoyu",   "zh"        },  // Mandarin/Std Chinese     # deprecated
    { "zh-hak",     "hak"       },  // Chinese, Hakka           # extlang
    { "zh-hakka",   "hak"       },  // Hakka                    # deprecated
    { "zh-hsn",     "hsn"       },  // Chinese, Xiang           # extlang
    { "zh-min-nan", "nan"       },  // Minnan,Hokkien,Taiwanese,So. Fujian # deprecated
    { "zh-mnp",     "mnp"       },  // Chinese, Min Bei         # extlang
    { "zh-nan",     "nan"       },  // Chinese, Min Nan         # extlang
    { "zh-wuu",     "wuu"       },  // Chinese, Wu              # extlang
    { "zh-xiang",   "hsn"       },  // Xiang/Hunanese           # deprecated
    { "zh-yue",     "yue"       },  // Chinese, Yue             # extlang
    { "zha",        "za"        },  // Zhuang, Chuang
    { "zho",        "zh"        },  // Chinese
    { "zsm",        "ms"        },  // Std Malay -> Malay (macrolang.)
    { "zul",        "zu"        },  // Zulu
    { "zyb",        "za"        },  // Yongbei Zhuang -> Zhuang (macrolang.)
};
enum {
    kNumLocaleStringPrefixToCanonical = sizeof(localeStringPrefixToCanonical)/sizeof(KeyStringToResultString)
};


static const SpecialCaseUpdates specialCases[] = {
// Data for special cases
// a) The 3166 code CS was used for Czechoslovakia until 1993, when that country split and the code was
// replaced by CZ and SK. Then in 2003-07, the code YU (formerly designating all of Yugoslavia, then after
// the 1990s breakup just designating what is now Serbia and Montenegro) was changed to CS! Then after
// Serbia and Montenegro split, the code CS was replaced in 2006-09 with separate codes RS and ME. If we
// see CS but a language of cs or sk, we change CS to CZ or SK. Otherwise, we change CS (and old YU) to RS.
// b) The 639 code sh for Serbo-Croatian was also replaced in the 1990s by separate codes hr and sr, and
// deprecated in 2000. We guess which one to map it to as follows: If there is a region tag of HR we use
// hr; if there is a region tag of (now) RS we use sr; else we do not change it (not enough info).
// c) There are other codes that have been updated without these issues (eg. TP to TL), plus among the
// "exceptionally reserved" codes some are just alternates for standard codes (eg. UK for GB).
    {   NULL,   "-UK",  "GB",   NULL,   NULL    },  // always change UK to GB (UK is "exceptionally reserved" to mean GB)
    {   NULL,   "-TP",  "TL",   NULL,   NULL    },  // always change TP to TL (East Timor, code changed 2002-05)
    {   "cs",   "-CS",  "CZ",   NULL,   NULL    },  // if language is cs, change CS (pre-1993 Czechoslovakia) to CZ (Czech Republic)
    {   "sk",   "-CS",  "SK",   NULL,   NULL    },  // if language is sk, change CS (pre-1993 Czechoslovakia) to SK (Slovakia)
    {   NULL,   "-CS",  "RS",   NULL,   NULL    },  // otherwise map CS (assume Serbia+Montenegro) to RS (Serbia)
    {   NULL,   "-YU",  "RS",   NULL,   NULL    },  // also map old YU (assume Serbia+Montenegro) to RS (Serbia)
    {   "sh",   "-HR",  "hr",   "-RS",  "sr"    },  // then if language is old 'sh' (SerboCroatian), change it to 'hr' (Croatian)
                                                    // if we find HR (Croatia) or to 'sr' (Serbian) if we find RS (Serbia).
                                                    // Note: Do this after changing YU/CS toRS as above.
    {   NULL,   NULL,   NULL,   NULL,   NULL    }   // terminator
};


static const KeyStringToResultString localeStringRegionToDefaults[] = {
// For some region-code suffixes, there are default substrings to strip off for canonical string.
// Must be sorted according to how strcmp compares the strings in the first column
//
//  region      default writing
//  suffix      system tags, strip     comment
//  --------    -------------          ---------
    { "_CN",    "-Hans"         },  // mainland China, default is simplified
    { "_HK",    "-Hant"         },  // Hong Kong, default is traditional
    { "_MO",    "-Hant"         },  // Macao, default is traditional
    { "_SG",    "-Hans"         },  // Singapore, default is simplified
    { "_TW",    "-Hant"         },  // Taiwan, default is traditional
};
enum {
    kNumLocaleStringRegionToDefaults = sizeof(localeStringRegionToDefaults)/sizeof(KeyStringToResultString)
};

static const KeyStringToResultString localeStringPrefixToDefaults[] = {
// For some initial portions of language tag, there are default substrings to strip off for canonical string.
// Must be sorted according to how strcmp compares the strings in the first column
//
//  language    default writing
//  tag prefix  system tags, strip     comment
//  --------    -------------          ---------
    { "ab-",    "-Cyrl"         },  // Abkhazian
    { "af-",    "-Latn"         },  // Afrikaans
    { "agq-",   "-Latn"         },  // Aghem
    { "ak-",    "-Latn"         },  // Akan
    { "am-",    "-Ethi"         },  // Amharic
    { "ar-",    "-Arab"         },  // Arabic
    { "as-",    "-Beng"         },  // Assamese
    { "asa-",   "-Latn"         },  // Asu
    { "ay-",    "-Latn"         },  // Aymara
    { "az-",    "-Latn"         },  // Azerbaijani
    { "bas-",   "-Latn"         },  // Basaa
    { "be-",    "-Cyrl"         },  // Belarusian
    { "bem-",   "-Latn"         },  // Bemba
    { "bez-",   "-Latn"         },  // Bena
    { "bg-",    "-Cyrl"         },  // Bulgarian
    { "bm-",    "-Latn"         },  // Bambara
    { "bn-",    "-Beng"         },  // Bengali
    { "bo-",    "-Tibt"         },  // Tibetan (? not Suppress-Script)
    { "br-",    "-Latn"         },  // Breton (? not Suppress-Script)
    { "brx-",   "-Deva"         },  // Bodo
    { "bs-",    "-Latn"         },  // Bosnian
    { "ca-",    "-Latn"         },  // Catalan
    { "cgg-",   "-Latn"         },  // Chiga
    { "chr-",   "-Cher"         },  // Cherokee
    { "cs-",    "-Latn"         },  // Czech
    { "cy-",    "-Latn"         },  // Welsh
    { "da-",    "-Latn"         },  // Danish
    { "dav-",   "-Latn"         },  // Taita
    { "de-",    "-Latn -1901"   },  // German, traditional orthography
    { "dje-",   "-Latn"         },  // Zarma
    { "dua-",   "-Latn"         },  // Duala
    { "dv-",    "-Thaa"         },  // Divehi/Maldivian
    { "dyo-",   "-Latn"         },  // Jola-Fonyi
    { "dz-",    "-Tibt"         },  // Dzongkha
    { "ebu-",   "-Latn"         },  // Embu
    { "ee-",    "-Latn"         },  // Ewe
    { "el-",    "-Grek"         },  // Greek (modern, monotonic)
    { "en-",    "-Latn"         },  // English
    { "eo-",    "-Latn"         },  // Esperanto
    { "es-",    "-Latn"         },  // Spanish
    { "et-",    "-Latn"         },  // Estonian
    { "eu-",    "-Latn"         },  // Basque
    { "ewo-",   "-Latn"         },  // Ewondo
    { "fa-",    "-Arab"         },  // Farsi
    { "ff-",    "-Latn"         },  // Fulah
    { "fi-",    "-Latn"         },  // Finnish
    { "fil-",   "-Latn"         },  // Tagalog
    { "fo-",    "-Latn"         },  // Faroese
    { "fr-",    "-Latn"         },  // French
    { "ga-",    "-Latn"         },  // Irish
    { "gd-",    "-Latn"         },  // Scottish Gaelic (? not Suppress-Script)
    { "gl-",    "-Latn"         },  // Galician
    { "gn-",    "-Latn"         },  // Guarani
    { "gsw-",   "-Latn"         },  // Swiss German
    { "gu-",    "-Gujr"         },  // Gujarati
    { "guz-",   "-Latn"         },  // Gusii
    { "gv-",    "-Latn"         },  // Manx
    { "ha-",    "-Latn"         },  // Hausa
    { "haw-",   "-Latn"         },  // Hawaiian (? not Suppress-Script)
    { "he-",    "-Hebr"         },  // Hebrew
    { "hi-",    "-Deva"         },  // Hindi
    { "hr-",    "-Latn"         },  // Croatian
    { "hu-",    "-Latn"         },  // Hungarian
    { "hy-",    "-Armn"         },  // Armenian
    { "id-",    "-Latn"         },  // Indonesian
    { "ig-",    "-Latn"         },  // Igbo
    { "ii-",    "-Yiii"         },  // Sichuan Yi
    { "is-",    "-Latn"         },  // Icelandic
    { "it-",    "-Latn"         },  // Italian
    { "ja-",    "-Jpan"         },  // Japanese
    { "jmc-",   "-Latn"         },  // Machame
    { "ka-",    "-Geor"         },  // Georgian
    { "kab-",   "-Latn"         },  // Kabyle
    { "kam-",   "-Latn"         },  // Kamba
    { "kde-",   "-Latn"         },  // Makonde
    { "kea-",   "-Latn"         },  // Kabuverdianu
    { "khq-",   "-Latn"         },  // Koyra Chiini
    { "ki-",    "-Latn"         },  // Kikuyu
    { "kk-",    "-Cyrl"         },  // Kazakh
    { "kl-",    "-Latn"         },  // Kalaallisut/Greenlandic
    { "km-",    "-Khmr"         },  // Central Khmer
    { "kn-",    "-Knda"         },  // Kannada
    { "ko-",    "-Hang"         },  // Korean (? not Suppress-Script)
    { "kok-",   "-Deva"         },  // Konkani
    { "ksb-",   "-Latn"         },  // Shambala
    { "ksf-",   "-Latn"         },  // Bafia
    { "kw-",    "-Latn"         },  // Cornish
    { "ky-",    "-Cyrl"         },  // Kirghiz
    { "la-",    "-Latn"         },  // Latin
    { "lag-",   "-Latn"         },  // Langi    
    { "lb-",    "-Latn"         },  // Luxembourgish
    { "lg-",    "-Latn"         },  // Ganda
    { "ln-",    "-Latn"         },  // Lingala
    { "lo-",    "-Laoo"         },  // Lao
    { "lt-",    "-Latn"         },  // Lithuanian
    { "lu-",    "-Latn"         },  // Luba-Katanga
    { "luo-",   "-Latn"         },  // Luo
    { "luy-",   "-Latn"         },  // Luyia
    { "lv-",    "-Latn"         },  // Latvian
    { "mas-",   "-Latn"         },  // Masai
    { "mer-",   "-Latn"         },  // Meru
    { "mfe-",   "-Latn"         },  // Morisyen
    { "mg-",    "-Latn"         },  // Malagasy
    { "mgh-",   "-Latn"         },  // Makhuwa-Meetto
    { "mk-",    "-Cyrl"         },  // Macedonian
    { "ml-",    "-Mlym"         },  // Malayalam
    { "mn-",    "-Cyrl"         },  // Mongolian
    { "mo-",    "-Latn"         },  // Moldavian
    { "mr-",    "-Deva"         },  // Marathi
    { "ms-",    "-Latn"         },  // Malay
    { "mt-",    "-Latn"         },  // Maltese
    { "mua-",   "-Latn"         },  // Mundang
    { "my-",    "-Mymr"         },  // Burmese/Myanmar
    { "naq-",   "-Latn"         },  // Nama
    { "nb-",    "-Latn"         },  // Norwegian Bokmal
    { "nd-",    "-Latn"         },  // North Ndebele
    { "ne-",    "-Deva"         },  // Nepali
    { "nl-",    "-Latn"         },  // Dutch
    { "nmg-",   "-Latn"         },  // Kwasio
    { "nn-",    "-Latn"         },  // Norwegian Nynorsk
    { "nus-",   "-Latn"         },  // Nuer
    { "ny-",    "-Latn"         },  // Chichewa/Nyanja
    { "nyn-",   "-Latn"         },  // Nyankole
    { "om-",    "-Latn"         },  // Oromo
    { "or-",    "-Orya"         },  // Oriya
    { "pa-",    "-Guru"         },  // Punjabi
    { "pl-",    "-Latn"         },  // Polish
    { "ps-",    "-Arab"         },  // Pushto
    { "pt-",    "-Latn"         },  // Portuguese
    { "qu-",    "-Latn"         },  // Quechua
    { "rm-",    "-Latn"         },  // Romansh
    { "rn-",    "-Latn"         },  // Rundi
    { "ro-",    "-Latn"         },  // Romanian
    { "rof-",   "-Latn"         },  // Rombo
    { "ru-",    "-Cyrl"         },  // Russian
    { "rw-",    "-Latn"         },  // Kinyarwanda
    { "rwk-",   "-Latn"         },  // Rwa
    { "sa-",    "-Deva"         },  // Sanskrit (? not Suppress-Script)
    { "saq-",   "-Latn"         },  // Samburu
    { "sbp-",   "-Latn"         },  // Sangu
    { "se-",    "-Latn"         },  // Sami (? not Suppress-Script)
    { "seh-",   "-Latn"         },  // Sena
    { "ses-",   "-Latn"         },  // Koyraboro Senni
    { "sg-",    "-Latn"         },  // Sango
    { "shi-",   "-Latn"         },  // Tachelhit
    { "si-",    "-Sinh"         },  // Sinhala
    { "sk-",    "-Latn"         },  // Slovak
    { "sl-",    "-Latn"         },  // Slovenian
    { "sn-",    "-Latn"         },  // Shona
    { "so-",    "-Latn"         },  // Somali
    { "sq-",    "-Latn"         },  // Albanian
    { "sr-",    "-Cyrl"         },  // Serbian
    { "sv-",    "-Latn"         },  // Swedish
    { "sw-",    "-Latn"         },  // Swahili
    { "swc-",   "-Latn"         },  // Congo Swahili
    { "ta-",    "-Taml"         },  // Tamil
    { "te-",    "-Telu"         },  // Telugu
    { "teo-",   "-Latn"         },  // Teso
    { "tg-",    "-Cyrl"         },  // Tajik
    { "th-",    "-Thai"         },  // Thai
    { "ti-",    "-Ethi"         },  // Tigrinya
    { "tk-",    "-Latn"         },  // Turkmen
    { "tn-",    "-Latn"         },  // Tswana
    { "to-",    "-Latn"         },  // Tonga of Tonga Islands
    { "tr-",    "-Latn"         },  // Turkish
    { "twq-",   "-Latn"         },  // Tasawaq
    { "tzm-",   "-Latn"         },  // Central Morocco Tamazight
    { "uk-",    "-Cyrl"         },  // Ukrainian
    { "ur-",    "-Arab"         },  // Urdu
    { "uz-",    "-Cyrl"         },  // Uzbek
    { "vai-",   "-Vaii"         },  // Vai
    { "vi-",    "-Latn"         },  // Vietnamese
    { "vun-",   "-Latn"         },  // Vunjo
    { "wo-",    "-Latn"         },  // Wolof
    { "xh-",    "-Latn"         },  // Xhosa
    { "xog-",   "-Latn"         },  // Soga
    { "yav-",   "-Latn"         },  // Yangben
    { "yi-",    "-Hebr"         },  // Yiddish
    { "yo-",    "-Latn"         },  // Yoruba
    { "zh-",    "-Hani"         },  // Chinese (? not Suppress-Script)
    { "zu-",    "-Latn"         },  // Zulu
};
enum {
    kNumLocaleStringPrefixToDefaults = sizeof(localeStringPrefixToDefaults)/sizeof(KeyStringToResultString)
};

static const KeyStringToResultString appleLocaleToLanguageString[] = {
// Map locale strings that Apple uses as language IDs to real language strings.
// Must be sorted according to how strcmp compares the strings in the first column.
// Note: Now we remove all transforms of the form ll_RR -> ll-RR, they are now
// handled in the code. <1.19>
//
//    locale 			lang			[  comment ]
//    string			string
//    -------			-------
    { "en_US_POSIX",	"en-US-POSIX"	},  // POSIX locale, need as language string			// <1.17> [3840752]
    { "zh_CN",  		"zh-Hans"		},  // mainland China => simplified
    { "zh_HK",  		"zh-HK"		},  // Hong Kong => traditional, not currently used
    { "zh_MO",  		"zh-MO"		},  // Macao => traditional, not currently used
    { "zh_SG",  		"zh-SG"		},  // Singapore => simplified, not currently used
    { "zh_TW",  		"zh-Hant"		},  // Taiwan => traditional
};
enum {
    kNumAppleLocaleToLanguageString = sizeof(appleLocaleToLanguageString)/sizeof(KeyStringToResultString)
};

/*
static const KeyStringToResultString appleLocaleToLanguageStringForCFBundle[] = {
// Map locale strings that Apple uses as language IDs to real language strings.
// Must be sorted according to how strcmp compares the strings in the first column.
//
//    locale 			lang			[  comment ]
//    string			string
//    -------			-------
    { "de_AT",  		"de-AT"			},  // Austrian German
    { "de_CH",  		"de-CH"			},  // Swiss German
//  { "de_DE",  		"de-DE"			},  // German for Germany (default), not currently used
    { "en_AU", 			"en-AU"			},  // Australian English
    { "en_CA",  		"en-CA"			},  // Canadian English
    { "en_GB",  		"en-GB"			},  // British English
//  { "en_IE",  		"en-IE"			},  // Irish English, not currently used
    { "en_US",  		"en-US"			},  // U.S. English
    { "en_US_POSIX",	"en-US-POSIX"	},  // POSIX locale, need as language string			// <1.17> [3840752]
//  { "fr_BE",  		"fr-BE"			},  // Belgian French, not currently used
    { "fr_CA",  		"fr-CA"			},  // Canadian French
    { "fr_CH",  		"fr-CH"			},  // Swiss French
//  { "fr_FR",  		"fr-FR"			},  // French for France (default), not currently used
    { "nl_BE",  		"nl-BE"			},  // Flemish = Vlaams, Dutch for Belgium
//  { "nl_NL",  		"nl-NL"			},  // Dutch for Netherlands (default), not currently used
    { "pt_BR",  		"pt-BR"			},  // Brazilian Portuguese
    { "pt_PT",  		"pt-PT"     	},  // Portuguese for Portugal
    { "zh_CN",  		"zh-Hans"		},  // mainland China => simplified
    { "zh_HK",  		"zh-Hant"		},  // Hong Kong => traditional, not currently used
    { "zh_MO",  		"zh-Hant"		},  // Macao => traditional, not currently used
    { "zh_SG",  		"zh-Hans"		},  // Singapore => simplified, not currently used
    { "zh_TW",  		"zh-Hant"		},  // Taiwan => traditional
};
enum {
    kNumAppleLocaleToLanguageStringForCFBundle = sizeof(appleLocaleToLanguageStringForCFBundle)/sizeof(KeyStringToResultString)
};
 */

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX

struct LocaleToLegacyCodes {
    const char *        locale;	// reduced to language plus one other component (script, region, variant), separators normalized to'_'
    RegionCode		    regCode;
    LangCode		    langCode;
    CFStringEncoding    encoding;
};
typedef struct LocaleToLegacyCodes LocaleToLegacyCodes;

static const LocaleToLegacyCodes localeToLegacyCodes[] = {
	//	locale			RegionCode					LangCode						CFStringEncoding
    {   "af"/*ZA*/,     102/*verAfrikaans*/,        141/*langAfrikaans*/,            0/*Roman*/              },  // Latn
    {   "am",            -1,                         85/*langAmharic*/,             28/*Ethiopic*/           },  // Ethi
    {   "ar",            16/*verArabic*/,            12/*langArabic*/,               4/*Arabic*/             },  // Arab;
    {   "as",            -1,                         68/*langAssamese*/,            13/*Bengali*/            },  // Beng;
    {   "ay",            -1,                        134/*langAymara*/,               0/*Roman*/              },  // Latn;
    {   "az",            -1,                        150/*langAzerbaijanRoman*/,      0/*Roman*/              },  // "az" defaults to -Latn
    {   "az_Arab",       -1,                         50/*langAzerbaijanAr*/,         4/*Arabic*/             },  // Arab;
    {   "az_Cyrl",       -1,                         49/*langAzerbaijani*/,          7/*Cyrillic*/           },  // Cyrl;
    {   "az_Latn",       -1,                        150/*langAzerbaijanRoman*/,      0/*Roman*/              },  // Latn;
    {   "be"/*BY*/,      61/*verBelarus*/,           46/*langBelorussian*/,          7/*Cyrillic*/           },  // Cyrl;
    {   "bg"/*BG*/,      72/*verBulgaria*/,          44/*langBulgarian*/,            7/*Cyrillic*/           },  // Cyrl;
    {   "bn",            60/*verBengali*/,           67/*langBengali*/,             13/*Bengali*/            },  // Beng;
    {   "bo",           105/*verTibetan*/,           63/*langTibetan*/,             26/*Tibetan*/            },  // Tibt;
    {   "br",            77/*verBreton*/,           142/*langBreton*/,              39/*Celtic*/             },  // Latn;
    {   "ca"/*ES*/,      73/*verCatalonia*/,        130/*langCatalan*/,              0/*Roman*/              },  // Latn;
    {   "cs"/*CZ*/,      56/*verCzech*/,             38/*langCzech*/,               29/*CentralEurRoman*/    },  // Latn;
    {   "cy",            79/*verWelsh*/,            128/*langWelsh*/,               39/*Celtic*/             },  // Latn;
    {   "da"/*DK*/,       9/*verDenmark*/,            7/*langDanish*/,               0/*Roman*/              },  // Latn;
    {   "de",             3/*verGermany*/,            2/*langGerman*/,               0/*Roman*/              },  // assume "de" defaults to verGermany
    {   "de_1996",       70/*verGermanReformed*/,     2/*langGerman*/,               0/*Roman*/              },
    {   "de_AT",         92/*verAustria*/,            2/*langGerman*/,               0/*Roman*/              },
    {   "de_CH",         19/*verGrSwiss*/,            2/*langGerman*/,               0/*Roman*/              },
    {   "de_DE",          3/*verGermany*/,            2/*langGerman*/,               0/*Roman*/              },
    {   "dz"/*BT*/,      83/*verBhutan*/,           137/*langDzongkha*/,            26/*Tibetan*/            },  // Tibt;
    {   "el",            20/*verGreece*/,            14/*langGreek*/,                6/*Greek*/              },  // assume "el" defaults to verGreece
    {   "el_CY",         23/*verCyprus*/,            14/*langGreek*/,                6/*Greek*/              },
    {   "el_GR",         20/*verGreece*/,            14/*langGreek*/,                6/*Greek*/              },  // modern monotonic
    {   "en",             0/*verUS*/,                 0/*langEnglish*/,              0/*Roman*/              },  // "en" defaults to verUS (per Chris Hansten)
    {   "en_001",        37/*verInternational*/,      0/*langEnglish*/,              0/*Roman*/              },
    {   "en_AU",         15/*verAustralia*/,          0/*langEnglish*/,              0/*Roman*/              },
    {   "en_CA",         82/*verEngCanada*/,          0/*langEnglish*/,              0/*Roman*/              },
    {   "en_GB",          2/*verBritain*/,            0/*langEnglish*/,              0/*Roman*/              },
    {   "en_IE",        108/*verIrelandEnglish*/,     0/*langEnglish*/,              0/*Roman*/              },
    {   "en_SG",        100/*verSingapore*/,          0/*langEnglish*/,              0/*Roman*/              },
    {   "en_US",          0/*verUS*/,                 0/*langEnglish*/,              0/*Roman*/              },
    {   "eo",           103/*verEsperanto*/,         94/*langEsperanto*/,            0/*Roman*/              },  // Latn;
    {   "es",             8/*verSpain*/,              6/*langSpanish*/,              0/*Roman*/              },  // "es" defaults to verSpain (per Chris Hansten)
    {   "es_419",        86/*verSpLatinAmerica*/,     6/*langSpanish*/,              0/*Roman*/              },  // new BCP 47 tag
    {   "es_ES",          8/*verSpain*/,              6/*langSpanish*/,              0/*Roman*/              },
    {   "es_MX",         86/*verSpLatinAmerica*/,     6/*langSpanish*/,              0/*Roman*/              },
    {   "es_US",         86/*verSpLatinAmerica*/,     6/*langSpanish*/,              0/*Roman*/              },
    {   "et"/*EE*/,      44/*verEstonia*/,           27/*langEstonian*/,            29/*CentralEurRoman*/    },
    {   "eu",            -1,                        129/*langBasque*/,               0/*Roman*/              },  // Latn;
    {   "fa"/*IR*/,      48/*verIran*/,              31/*langFarsi/Persian*/,       0x8C/*Farsi*/            },  // Arab;
    {   "fi"/*FI*/,      17/*verFinland*/,           13/*langFinnish*/,              0/*Roman*/              },
    {   "fil",           -1,                         82/*langTagalog*/,              0/*Roman*/              },  // Latn;
    {   "fo"/*FO*/,      47/*verFaroeIsl*/,          30/*langFaroese*/,             37/*Icelandic*/          },
    {   "fr",             1/*verFrance*/,             1/*langFrench*/,               0/*Roman*/              },  // "fr" defaults to verFrance (per Chris Hansten)
    {   "fr_001",        91/*verFrenchUniversal*/,    1/*langFrench*/,               0/*Roman*/              },
    {   "fr_BE",         98/*verFrBelgium*/,          1/*langFrench*/,               0/*Roman*/              },
    {   "fr_CA",         11/*verFrCanada*/,           1/*langFrench*/,               0/*Roman*/              },
    {   "fr_CH",         18/*verFrSwiss*/,            1/*langFrench*/,               0/*Roman*/              },
    {   "fr_FR",          1/*verFrance*/,             1/*langFrench*/,               0/*Roman*/              },
    {   "ga"/*IE*/,      50/*verIreland*/,           35/*langIrishGaelic*/,          0/*Roman*/              },  // no dots (h after)
    {   "ga_Latg"/*IE*/, 81/*verIrishGaelicScrip*/, 146/*langIrishGaelicScript*/,   40/*Gaelic*/             },  // using dots
    {   "gd",            75/*verScottishGaelic*/,   144/*langScottishGaelic*/,      39/*Celtic*/             },
    {   "gl",            -1,                        140/*langGalician*/,             0/*Roman*/              },  // Latn;
    {   "gn",            -1,                        133/*langGuarani*/,              0/*Roman*/              },  // Latn;
    {   "grc",           40/*verGreekAncient*/,     148/*langGreekAncient*/,         6/*Greek*/              },  // polytonic (MacGreek doesn't actually support it)
    {   "gu"/*IN*/,      94/*verGujarati*/,          69/*langGujarati*/,            11/*Gujarati*/           },  // Gujr;
    {   "gv",            76/*verManxGaelic*/,       145/*langManxGaelic*/,          39/*Celtic*/             },  // Latn;
    {   "he"/*IL*/,      13/*verIsrael*/,            10/*langHebrew*/,               5/*Hebrew*/             },  // Hebr;
    {   "hi"/*IN*/,      33/*verIndiaHindi*/,        21/*langHindi*/,                9/*Devanagari*/         },  // Deva;
    {   "hr"/*HR*/,      68/*verCroatia*/,           18/*langCroatian*/,            36/*Croatian*/           },
    {   "hu"/*HU*/,      43/*verHungary*/,           26/*langHungarian*/,           29/*CentralEurRoman*/    },
    {   "hy"/*AM*/,      84/*verArmenian*/,          51/*langArmenian*/,            24/*Armenian*/           },  // Armn;
    {   "id",            -1,                         81/*langIndonesian*/,           0/*Roman*/              },  // Latn;
    {   "is"/*IS*/,      21/*verIceland*/,           15/*langIcelandic*/,           37/*Icelandic*/          },
    {   "it",             4/*verItaly*/,              3/*langItalian*/,              0/*Roman*/              },  // "it" defaults to verItaly
    {   "it_CH",         36/*verItalianSwiss*/,       3/*langItalian*/,              0/*Roman*/              },
    {   "it_IT",          4/*verItaly*/,              3/*langItalian*/,              0/*Roman*/              },
    {   "iu"/*CA*/,      78/*verNunavut*/,          143/*langInuktitut*/,           0xEC/*Inuit*/            },  // Cans;
    {   "ja"/*JP*/,      14/*verJapan*/,             11/*langJapanese*/,             1/*Japanese*/           },  // Jpan;
    {   "jv",            -1,                        138/*langJavaneseRom*/,          0/*Roman*/              },  // Latn;
    {   "ka"/*GE*/,      85/*verGeorgian*/,          52/*langGeorgian*/,            23/*Georgian*/           },  // Geor;
    {   "kk",            -1,                         48/*langKazakh*/,               7/*Cyrillic*/           },  // "kk" defaults to -Cyrl; also have -Latn, -Arab
    {   "kl",           107/*verGreenland*/,        149/*langGreenlandic*/,          0/*Roman*/              },  // Latn;
    {   "km",            -1,                         78/*langKhmer*/,               20/*Khmer*/              },  // Khmr;
    {   "kn",            -1,                         73/*langKannada*/,             16/*Kannada*/            },  // Knda;
    {   "ko"/*KR*/,      51/*verKorea*/,             23/*langKorean*/,               3/*Korean*/             },  // Hang;
    {   "ks",            -1,                         61/*langKashmiri*/,             4/*Arabic*/             },  // Arab;
    {   "ku",            -1,                         60/*langKurdish*/,              4/*Arabic*/             },  // Arab;
    {   "ky",            -1,                         54/*langKirghiz*/,              7/*Cyrillic*/           },  // Cyrl; also -Latn, -Arab
    {   "la",            -1,                        131/*langLatin*/,                0/*Roman*/              },  // Latn;
    {   "lo",            -1,                         79/*langLao*/,                 22/*Laotian*/            },  // Laoo;
    {   "lt"/*LT*/,      41/*verLithuania*/,         24/*langLithuanian*/,          29/*CentralEurRoman*/    },
    {   "lv"/*LV*/,      45/*verLatvia*/,            28/*langLatvian*/,             29/*CentralEurRoman*/    },
    {   "mg",            -1,                         93/*langMalagasy*/,             0/*Roman*/              },  // Latn;
    {   "mk"/*MK*/,      67/*verMacedonian*/,        43/*langMacedonian*/,           7/*Cyrillic*/           },  // Cyrl;
    {   "ml",            -1,                         72/*langMalayalam*/,           17/*Malayalam*/          },  // Mlym;
    {   "mn",            -1,                         58/*langMongolianCyr*/,         7/*Cyrillic*/           },  // "mn" defaults to -Cyrl
    {   "mn_Cyrl",       -1,                         58/*langMongolianCyr*/,         7/*Cyrillic*/           },  // Cyrl;
    {   "mn_Mong",       -1,                         57/*langMongolian*/,           27/*Mongolian*/          },  // Mong;
    {   "mo",            -1,                         53/*langMoldavian*/,            7/*Cyrillic*/           },  // Cyrl;
    {   "mr"/*IN*/,     104/*verMarathi*/,           66/*langMarathi*/,              9/*Devanagari*/         },  // Deva;
    {   "ms",            -1,                         83/*langMalayRoman*/,           0/*Roman*/              },  // "ms" defaults to -Latn;
    {   "ms_Arab",       -1,                         84/*langMalayArabic*/,          4/*Arabic*/             },  // Arab;
    {   "mt"/*MT*/,      22/*verMalta*/,             16/*langMaltese*/,              0/*Roman*/              },  // Latn;
    {   "mul",           74/*verMultilingual*/,      -1,                             0                       },
    {   "my",            -1,                         77/*langBurmese*/,             19/*Burmese*/            },  // Mymr;
    {   "nb"/*NO*/,      12/*verNorway*/,             9/*langNorwegian*/,            0/*Roman*/              },
    {   "ne"/*NP*/,     106/*verNepal*/,             64/*langNepali*/,               9/*Devanagari*/         },  // Deva;
    {   "nl",             5/*verNetherlands*/,        4/*langDutch*/,                0/*Roman*/              },  // "nl" defaults to verNetherlands
    {   "nl_BE",          6/*verFlemish*/,           34/*langFlemish*/,              0/*Roman*/              },
    {   "nl_NL",          5/*verNetherlands*/,        4/*langDutch*/,                0/*Roman*/              },
    {   "nn"/*NO*/,     101/*verNynorsk*/,          151/*langNynorsk*/,              0/*Roman*/              },
    {   "ny",            -1,                         92/*langNyanja/Chewa*/,         0/*Roman*/              },  // Latn;
    {   "om",            -1,                         87/*langOromo*/,               28/*Ethiopic*/           },  // Ethi;
    {   "or",            -1,                         71/*langOriya*/,               12/*Oriya*/              },  // Orya;
    {   "pa",            95/*verPunjabi*/,           70/*langPunjabi*/,             10/*Gurmukhi*/           },  // Guru;
    {   "pl"/*PL*/,      42/*verPoland*/,            25/*langPolish*/,              29/*CentralEurRoman*/    },
    {   "ps",            -1,                         59/*langPashto*/,              0x8C/*Farsi*/            },  // Arab;
    {   "pt",            71/*verBrazil*/,             8/*langPortuguese*/,           0/*Roman*/              },  // "pt" defaults to verBrazil (per Chris Hansten)
    {   "pt_BR",         71/*verBrazil*/,             8/*langPortuguese*/,           0/*Roman*/              },
    {   "pt_PT",         10/*verPortugal*/,           8/*langPortuguese*/,           0/*Roman*/              },
    {   "qu",            -1,                        132/*langQuechua*/,              0/*Roman*/              },  // Latn;
    {   "rn",            -1,                         91/*langRundi*/,                0/*Roman*/              },  // Latn;
    {   "ro"/*RO*/,      39/*verRomania*/,           37/*langRomanian*/,            38/*Romanian*/           },
    {   "ru"/*RU*/,      49/*verRussia*/,            32/*langRussian*/,              7/*Cyrillic*/           },  // Cyrl;
    {   "rw",            -1,                         90/*langKinyarwanda*/,          0/*Roman*/              },  // Latn;
    {   "sa",            -1,                         65/*langSanskrit*/,             9/*Devanagari*/         },  // Deva;
    {   "sd",            -1,                         62/*langSindhi*/,              0x8C/*Farsi*/            },  // Arab;
    {   "se",            46/*verSami*/,              29/*langSami*/,                 0/*Roman*/              },
    {   "si",            -1,                         76/*langSinhalese*/,           18/*Sinhalese*/          },  // Sinh;
    {   "sk"/*SK*/,      57/*verSlovak*/,            39/*langSlovak*/,              29/*CentralEurRoman*/    },
    {   "sl"/*SI*/,      66/*verSlovenian*/,         40/*langSlovenian*/,           36/*Croatian*/           },
    {   "so",            -1,                         88/*langSomali*/,               0/*Roman*/              },  // Latn;
    {   "sq",            -1,                         36/*langAlbanian*/,             0/*Roman*/              },
    {   "sr"/*CS,RS*/,   65/*verSerbian*/,           42/*langSerbian*/,              7/*Cyrillic*/           },  // Cyrl;
    {   "su",            -1,                        139/*langSundaneseRom*/,         0/*Roman*/              },  // Latn;
    {   "sv"/*SE*/,       7/*verSweden*/,             5/*langSwedish*/,              0/*Roman*/              },
    {   "sw",            -1,                         89/*langSwahili*/,              0/*Roman*/              },  // Latn;
    {   "ta",            -1,                         74/*langTamil*/,               14/*Tamil*/              },  // Taml;
    {   "te",            -1,                         75/*langTelugu*/,              15/*Telugu*/             },  // Telu
    {   "tg",            -1,                         55/*langTajiki*/,               7/*Cyrillic*/           },  // "tg" defaults to "Cyrl"
    {   "tg_Cyrl",       -1,                         55/*langTajiki*/,               7/*Cyrillic*/           },  // Cyrl; also -Latn, -Arab
    {   "th"/*TH*/,      54/*verThailand*/,          22/*langThai*/,                21/*Thai*/               },  // Thai;
    {   "ti",            -1,                         86/*langTigrinya*/,            28/*Ethiopic*/           },  // Ethi;
    {   "tk",            -1,                         56/*langTurkmen*/,              7/*Cyrillic*/           },  // "tk" defaults to Cyrl
    {   "tk_Cyrl",       -1,                         56/*langTurkmen*/,              7/*Cyrillic*/           },  // Cyrl; also -Latn, -Arab
    {   "tl",            -1,                         82/*langTagalog*/,              0/*Roman*/              },  // Latn;
    {   "to"/*TO*/,      88/*verTonga*/,            147/*langTongan*/,               0/*Roman*/              },  // Latn;
    {   "tr"/*TR*/,      24/*verTurkey*/,            17/*langTurkish*/,             35/*Turkish*/            },  // Latn;
    {   "tt",            -1,                        135/*langTatar*/,                7/*Cyrillic*/           },  // Cyrl;
    {   "tt_Cyrl",       -1,                        135/*langTatar*/,                7/*Cyrillic*/           },  // Cyrl;
    {   "ug",            -1,                        136/*langUighur*/,               4/*Arabic*/             },  // Arab;
    {   "uk"/*UA*/,      62/*verUkraine*/,           45/*langUkrainian*/,            7/*Cyrillic*/           },  // Cyrl;
    {   "und",           55/*verScriptGeneric*/,     -1,                             0                       },
    {   "ur",            34/*verPakistanUrdu*/,      20/*langUrdu*/,                0x8C/*Farsi*/            },  // "ur" defaults to verPakistanUrdu
    {   "ur_IN",         96/*verIndiaUrdu*/,         20/*langUrdu*/,                0x8C/*Farsi*/            },  // Arab
    {   "ur_PK",         34/*verPakistanUrdu*/,      20/*langUrdu*/,                0x8C/*Farsi*/            },  // Arab
    {   "uz"/*UZ*/,      99/*verUzbek*/,             47/*langUzbek*/,                7/*Cyrillic*/           },  // Cyrl; also -Latn, -Arab
    {   "uz_Cyrl",       99/*verUzbek*/,             47/*langUzbek*/,                7/*Cyrillic*/           },
    {   "vi"/*VN*/,      97/*verVietnam*/,           80/*langVietnamese*/,          30/*Vietnamese*/         },  // Latn
    {   "yi",            -1,                         41/*langYiddish*/,              5/*Hebrew*/             },  // Hebr;
    {   "zh",            52/*verChina*/,             33/*langSimpChinese*/,         25/*ChineseSimp*/        },  // "zh" defaults to verChina, langSimpChinese
    {   "zh_CN",         52/*verChina*/,             33/*langSimpChinese*/,         25/*ChineseSimp*/        },
    {   "zh_HK",         53/*verTaiwan*/,            19/*langTradChinese*/,          2/*ChineseTrad*/        },
    {   "zh_Hans",       52/*verChina*/,             33/*langSimpChinese*/,         25/*ChineseSimp*/        },
    {   "zh_Hant",       53/*verTaiwan*/,            19/*langTradChinese*/,          2/*ChineseTrad*/        },
    {   "zh_MO",         53/*verTaiwan*/,            19/*langTradChinese*/,          2/*ChineseTrad*/        },
    {   "zh_SG",         52/*verChina*/,             33/*langSimpChinese*/,         25/*ChineseSimp*/        },
    {   "zh_TW",         53/*verTaiwan*/,            19/*langTradChinese*/,          2/*ChineseTrad*/        },
};
enum {
    kNumLocaleToLegacyCodes = sizeof(localeToLegacyCodes)/sizeof(localeToLegacyCodes[0])
};

#endif

/*
	For reference here is a list of ICU locales with variants and how some
	of them are canonicalized with the ICU function uloc_canonicalize:
	
	ICU 3.0 has:
		en_US_POSIX			x	no change
		hy_AM_REVISED		x	no change
		ja_JP_TRADITIONAL	->	ja_JP@calendar=japanese
		th_TH_TRADITIONAL	->	th_TH@calendar=buddhist

	ICU 2.8 also had the following (now obsolete):
		ca_ES_PREEURO
		de__PHONEBOOK		->	de@collation=phonebook
		de_AT_PREEURO
		de_DE_PREEURO
		de_LU_PREEURO
		el_GR_PREEURO
		en_BE_PREEURO
		en_GB_EURO			->	en_GB@currency=EUR
		en_IE_PREEURO		->	en_IE@currency=IEP
		es__TRADITIONAL		->	es@collation=traditional
		es_ES_PREEURO
		eu_ES_PREEURO
		fi_FI_PREEURO
		fr_BE_PREEURO
		fr_FR_PREEURO		->	fr_FR@currency=FRF
		fr_LU_PREEURO
		ga_IE_PREEURO
		gl_ES_PREEURO
		hi__DIRECT			->	hi@collation=direct  
		it_IT_PREEURO
		nl_BE_PREEURO
		nl_NL_PREEURO
		pt_PT_PREEURO
		zh__PINYIN			->	zh@collation=pinyin
		zh_TW_STROKE		->	zh_TW@collation=stroke
	
*/

// _CompareTestEntryToTableEntryKey
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// comparison function for bsearch
static int _CompareTestEntryToTableEntryKey(const void *testEntryPtr, const void *tableEntryKeyPtr) {
    return strcmp( ((const KeyStringToResultString *)testEntryPtr)->key, ((const KeyStringToResultString *)tableEntryKeyPtr)->key );
}

// _CompareTestEntryPrefixToTableEntryKey
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// Comparison function for bsearch. Assumes prefix IS terminated with '-' or '_'.
// Do the following instead of strlen & strncmp so we don't walk tableEntry key twice.
static int _CompareTestEntryPrefixToTableEntryKey(const void *testEntryPtr, const void *tableEntryKeyPtr) {
    const char *    testPtr = ((const KeyStringToResultString *)testEntryPtr)->key;
    const char *    tablePtr = ((const KeyStringToResultString *)tableEntryKeyPtr)->key;
    
    while ( *testPtr == *tablePtr && *tablePtr != 0 ) {
        testPtr++; tablePtr++;
    }
    if ( *tablePtr != 0 ) {
        // strings are different, and the string in the table has not run out;
        // i.e. the table entry is not a prefix of the text string.
        return ( *testPtr < *tablePtr )? -1: 1;
    }
    return 0;
}

// _CompareLowerTestEntryPrefixToTableEntryKey
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// Comparison function for bsearch. Assumes prefix NOT terminated with '-' or '_'.
// Lowercases the test string before comparison (the table should already have lowercased entries).
static int _CompareLowerTestEntryPrefixToTableEntryKey(const void *testEntryPtr, const void *tableEntryKeyPtr) {
    const char *    testPtr = ((const KeyStringToResultString *)testEntryPtr)->key;
    const char *    tablePtr = ((const KeyStringToResultString *)tableEntryKeyPtr)->key;
    char            lowerTestChar;
    
    while ( (lowerTestChar = tolower(*testPtr)) == *tablePtr && *tablePtr != 0 && lowerTestChar != '_' ) {  // <1.9>
        testPtr++; tablePtr++;
    }
    if ( *tablePtr != 0 ) {
        // strings are different, and the string in the table has not run out;
        // i.e. the table entry is not a prefix of the text string.
        if (lowerTestChar == '_')                                                           // <1.9>
            return -1;                                                                      // <1.9>
        return ( lowerTestChar < *tablePtr )? -1: 1;
    }
    // The string in the table has run out. If the test string char is not alnum,
    // then the string matches, else the test string sorts after.
    return ( !isalnum(lowerTestChar) )? 0: 1;
}

// _DeleteCharsAtPointer
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// remove _length_ characters from the beginning of the string indicated by _stringPtr_
// (we know that the string has at least _length_ characters in it)
static void _DeleteCharsAtPointer(char *stringPtr, int length) {
    do {
        *stringPtr = stringPtr[length];
    } while (*stringPtr++ != 0);
}

// _CopyReplacementAtPointer
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// Copy replacement string (*excluding* terminating NULL byte) to the place indicated by stringPtr
static void _CopyReplacementAtPointer(char *stringPtr, const char *replacementPtr) {
    while (*replacementPtr != 0) {
        *stringPtr++ = *replacementPtr++;
    }
}

// _CheckForTag
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
static Boolean _CheckForTag(const char *localeStringPtr, const char *tagPtr, int tagLen) {
    return ( strncmp(localeStringPtr, tagPtr, tagLen) == 0 && !isalnum(localeStringPtr[tagLen]) );
}

// _ReplacePrefix
// Move this code from _UpdateFullLocaleString into separate function                       // <1.10>
static void _ReplacePrefix(char locString[], int locStringMaxLen, int oldPrefixLen, const char *newPrefix) {
    int newPrefixLen = strlen(newPrefix);
    int lengthDelta = newPrefixLen - oldPrefixLen;
    
    if (lengthDelta < 0) {
        // replacement is shorter, delete chars by shifting tail of string
        _DeleteCharsAtPointer(locString + newPrefixLen, -lengthDelta);
    } else if (lengthDelta > 0) {
        // replacement is longer...
        int stringLen = strlen(locString);
        
        if (stringLen + lengthDelta < locStringMaxLen) {
            // make room by shifting tail of string
            char *  tailShiftPtr = locString + stringLen;
            char *  tailStartPtr = locString + oldPrefixLen;    // pointer to tail of string to shift
            
            while (tailShiftPtr >= tailStartPtr) {
                tailShiftPtr[lengthDelta] = *tailShiftPtr;
                tailShiftPtr--;
            }
        } else {
            // no room, can't do substitution
            newPrefix = NULL;
        }
    }
    
    if (newPrefix) {
        // do the substitution
        _CopyReplacementAtPointer(locString, newPrefix);
    }
}

// _UpdateFullLocaleString
// Given a locale string that uses standard codes (not a special old-style Apple string),
// update all the language codes and region codes to latest versions, map 3-letter
// language codes to 2-letter codes if possible, and normalize casing. If requested, return
// pointers to a language-region variant subtag (if present) and a region tag (if present).
// (add locStringMaxLen parameter)                                                          // <1.10>
static void _UpdateFullLocaleString(char inLocaleString[], int locStringMaxLen,
									char **langRegSubtagRef, char **regionTagRef,
									char varKeyValueString[])								// <1.17>
{
    KeyStringToResultString     testEntry;
    KeyStringToResultString *   foundEntry;
    const SpecialCaseUpdates *  specialCasePtr;
    char *      inLocalePtr;
    char *      subtagPtr;
    char *      langRegSubtag = NULL;
    char *      regionTag = NULL;
    char *		variantTag = NULL;
    Boolean     subtagHasDigits, pastPrimarySubtag, hadRegion;
    
    // 1. First replace any non-canonical prefix (case insensitive) with canonical
    // (change 3-letter ISO 639 code to 2-letter, update obsolete ISO 639 codes & RFC 3066 tags, etc.)
    
    testEntry.key = inLocaleString;
    foundEntry = (KeyStringToResultString *)bsearch( &testEntry, localeStringPrefixToCanonical, kNumLocaleStringPrefixToCanonical,
                                                    sizeof(KeyStringToResultString), _CompareLowerTestEntryPrefixToTableEntryKey );
    if (foundEntry) {
        // replace key (at beginning of string) with result
        _ReplacePrefix(inLocaleString, locStringMaxLen, strlen(foundEntry->key), foundEntry->result);   // <1.10>
    }
    
    // 2. Walk through input string, normalizing case & marking use of ISO 3166 codes
    
    inLocalePtr = inLocaleString;
    subtagPtr = inLocaleString;
    subtagHasDigits = false;
    pastPrimarySubtag = false;
    hadRegion = false;
    
    while ( true ) {
        if ( isalpha(*inLocalePtr) ) {
            // if not past a region tag, then lowercase, else uppercase
            *inLocalePtr = (!hadRegion)? tolower(*inLocalePtr): toupper(*inLocalePtr);
        } else if ( isdigit(*inLocalePtr) ) {
            subtagHasDigits = true;
        } else {
            
            if (!pastPrimarySubtag) {
                // may have a NULL primary subtag
                if (subtagHasDigits) {
                    break;
                }
                pastPrimarySubtag = true;
            } else if (!hadRegion) {
                // We are after any primary language subtag, but not past any region tag.
                // This subtag is preceded by '-' or '_'.
                int subtagLength = inLocalePtr - subtagPtr; // includes leading '-' or '_'
                
				if (subtagLength == 3 && !subtagHasDigits) {
					// potential ISO 3166 code for region or language variant; if so, needs uppercasing
					if (*subtagPtr == '_') {
						regionTag = subtagPtr;
						hadRegion = true;
						subtagPtr[1] = toupper(subtagPtr[1]);
						subtagPtr[2] = toupper(subtagPtr[2]);
					} else if (langRegSubtag == NULL) {
						langRegSubtag = subtagPtr;
						subtagPtr[1] = toupper(subtagPtr[1]);
						subtagPtr[2] = toupper(subtagPtr[2]);
					}
				} else if (subtagLength == 4 && subtagHasDigits) {
					// potential UN M.49 region code
					if (*subtagPtr == '_') {
						regionTag = subtagPtr;
						hadRegion = true;
					} else if (langRegSubtag == NULL) {
						langRegSubtag = subtagPtr;
					}
				} else if (subtagLength == 5 && !subtagHasDigits) {
					// ISO 15924 script code, uppercase just the first letter
					subtagPtr[1] = toupper(subtagPtr[1]);
				} else if (subtagLength == 1 && *subtagPtr == '_') {						// <1.17>
					hadRegion = true;
				}
                
                if (!hadRegion) {
                    // convert improper '_' to '-'
                    *subtagPtr = '-';
                }
            } else {
            	variantTag = subtagPtr;															// <1.17>
            }
            
            if (*inLocalePtr == '-' || *inLocalePtr == '_') {
                subtagPtr = inLocalePtr;
                subtagHasDigits = false;
            } else {
                break;
            }
        }
    
        inLocalePtr++;
    }
    
    // 3 If there is a variant tag, see if ICU canonicalizes it to keywords.					// <1.17> [3577669]
    // If so, copy the keywords to varKeyValueString and delete the variant tag
    // from the original string (but don't otherwise use the ICU canonicalization).
    varKeyValueString[0] = 0;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    if (variantTag) {
		UErrorCode	icuStatus;
		int			icuCanonStringLen;
		char * 		varKeyValueStringPtr = varKeyValueString;
		
		icuStatus = U_ZERO_ERROR;
		icuCanonStringLen = uloc_canonicalize( inLocaleString, varKeyValueString, locStringMaxLen, &icuStatus );
		if ( U_SUCCESS(icuStatus) ) {
			char *	icuCanonStringPtr = varKeyValueString;
			
			if (icuCanonStringLen >= locStringMaxLen)
				icuCanonStringLen = locStringMaxLen - 1;
			varKeyValueString[icuCanonStringLen] = 0;
			while (*icuCanonStringPtr != 0 && *icuCanonStringPtr != ULOC_KEYWORD_SEPARATOR)
				++icuCanonStringPtr;
			if (*icuCanonStringPtr != 0) {
				// the canonicalized string has keywords
				// delete the variant tag in the original string (and other trailing '_' or '-')
				*variantTag-- = 0;
				while (*variantTag == '_')
					*variantTag-- = 0;
				// delete all of the canonicalized string except the keywords
				while (*icuCanonStringPtr != 0)
					*varKeyValueStringPtr++ = *icuCanonStringPtr++;
			}
		*varKeyValueStringPtr = 0;
		}
    }
#endif
    
    // 4. Handle special cases of updating region codes, or updating language codes based on
    // region code.
    for (specialCasePtr = specialCases; specialCasePtr->reg1 != NULL; specialCasePtr++) {
        if ( specialCasePtr->lang == NULL || _CheckForTag(inLocaleString, specialCasePtr->lang, 2) ) {
            // OK, we matched any language specified. Now what needs updating?
            char * foundTag;
            
            if ( isupper(specialCasePtr->update1[0]) ) {
                // updating a region code
                if ( ( foundTag = strstr(inLocaleString, specialCasePtr->reg1) ) && !isalnum(foundTag[3]) ) {
                    _CopyReplacementAtPointer(foundTag+1, specialCasePtr->update1);
                }
                if ( regionTag && _CheckForTag(regionTag+1, specialCasePtr->reg1 + 1, 2) ) {
                    _CopyReplacementAtPointer(regionTag+1, specialCasePtr->update1);
                }
                
            } else {
                // updating the language, there will be two choices based on region
                if        ( ( regionTag && _CheckForTag(regionTag+1, specialCasePtr->reg1 + 1, 2) ) ||
                            ( ( foundTag = strstr(inLocaleString, specialCasePtr->reg1) ) && !isalnum(foundTag[3]) ) ) {
                    _CopyReplacementAtPointer(inLocaleString, specialCasePtr->update1);
                } else if ( ( regionTag && _CheckForTag(regionTag+1, specialCasePtr->reg2 + 1, 2) ) ||
                            ( ( foundTag = strstr(inLocaleString, specialCasePtr->reg2) ) && !isalnum(foundTag[3]) ) ) {
                    _CopyReplacementAtPointer(inLocaleString, specialCasePtr->update2);
                }
            }
        }
    }

    // 5. return pointers if requested.
    if (langRegSubtagRef != NULL) {
        *langRegSubtagRef = langRegSubtag;
    }
    if (regionTagRef != NULL) {
        *regionTagRef = regionTag;
    }
}


// _RemoveSubstringsIfPresent
// (Local function for CFLocaleCreateCanonicalLocaleIdentifierFromString)
// substringList is a list of space-separated substrings to strip if found in localeString
static void _RemoveSubstringsIfPresent(char *localeString, const char *substringList) {
    while (*substringList != 0) {
        char    currentSubstring[kLocaleIdentifierCStringMax];
        int     substringLength = 0;
        char *  foundSubstring;

        // copy current substring & get its length
        while ( isgraph(*substringList) ) {
            currentSubstring[substringLength++] = *substringList++;
        }
        // move to next substring
        while ( isspace(*substringList) ) {
            substringList++;
        }

        // search for current substring in locale string
        if (substringLength == 0)
            continue;
        currentSubstring[substringLength] = 0;
        foundSubstring = strstr(localeString, currentSubstring);

        // if substring is found, delete it
        if (foundSubstring) {
            _DeleteCharsAtPointer(foundSubstring, substringLength);
        }
    }
}


// _GetKeyValueString                                                                       // <1.10>
// Removes any key-value string from inLocaleString, puts canonized version in keyValueString

static void _GetKeyValueString(char inLocaleString[], char keyValueString[]) {
    char *  inLocalePtr = inLocaleString;
    
    while (*inLocalePtr != 0 && *inLocalePtr != ULOC_KEYWORD_SEPARATOR) {
        inLocalePtr++;
    }
    if (*inLocalePtr != 0) {    // we found a key-value section
        char *  keyValuePtr = keyValueString;
        
        *keyValuePtr = *inLocalePtr;
        *inLocalePtr = 0;
        do {
            if ( *(++inLocalePtr) != ' ' ) {
                *(++keyValuePtr) = *inLocalePtr;    // remove "tolower() for *inLocalePtr"  // <1.11>
            }
        } while (*inLocalePtr != 0);
    } else {
        keyValueString[0] = 0;
    }
}

static void _AppendKeyValueString(char inLocaleString[], int locStringMaxLen, char keyValueString[]) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
	if (keyValueString[0] != 0) {
		UErrorCode		uerr = U_ZERO_ERROR;
		UEnumeration *	uenum = uloc_openKeywords(keyValueString, &uerr);
		if ( uenum != NULL ) {
			const char *	keyword;
			int32_t			length;
			char			value[ULOC_KEYWORDS_CAPACITY];	// use as max for keyword value
			while ( U_SUCCESS(uerr) ) {
				keyword = uenum_next(uenum, &length, &uerr);
				if ( keyword == NULL ) {
					break;
				}
				length = uloc_getKeywordValue( keyValueString, keyword, value, sizeof(value), &uerr );
				length = uloc_setKeywordValue( keyword, value, inLocaleString, locStringMaxLen, &uerr );
			}
			uenum_close(uenum);
		}
	}
#endif
}

// __private_extern__ CFStringRef _CFLocaleCreateCanonicalLanguageIdentifierForCFBundle(CFAllocatorRef allocator, CFStringRef localeIdentifier) {}

CFStringRef CFLocaleCreateCanonicalLanguageIdentifierFromString(CFAllocatorRef allocator, CFStringRef localeIdentifier) {
    char            inLocaleString[kLocaleIdentifierCStringMax];
    CFStringRef     outStringRef = NULL;
    
    if ( localeIdentifier && CFStringGetCString(localeIdentifier, inLocaleString,  sizeof(inLocaleString), kCFStringEncodingASCII) ) {
        KeyStringToResultString     testEntry;
        KeyStringToResultString *   foundEntry;
        char                        keyValueString[sizeof(inLocaleString)];				// <1.10>
        char						varKeyValueString[sizeof(inLocaleString)];			// <1.17>

        _GetKeyValueString(inLocaleString, keyValueString);								// <1.10>
        testEntry.result = NULL;
        
        // A. Special case aa_SAAHO, no_BOKMAL, and no_NYNORSK since they are legacy identifiers that don't follow the normal rules (http://unicode.org/cldr/trac/browser/trunk/common/supplemental/supplementalMetadata.xml)
        
        testEntry.key = inLocaleString;
        KeyStringToResultString specialCase = testEntry;
        foundEntry = &specialCase;
        
        if (strncmp("aa_SAAHO", testEntry.key, strlen("aa_SAAHO")) == 0) {
            foundEntry->result = "ssy";
        } else if (strncmp("no_BOKMAL", testEntry.key, strlen("no_BOKMAL")) == 0) {
            foundEntry->result = "nb";
        } else if (strncmp("no_NYNORSK", testEntry.key, strlen("no_NYNORSK")) == 0) {
            foundEntry->result = "nn";
        } else {
            // B. First check if input string matches an old-style string that has a replacement
            // (do this before case normalization)
            foundEntry = (KeyStringToResultString *)bsearch( &testEntry, oldAppleLocaleToCanonical, kNumOldAppleLocaleToCanonical,
                                                            sizeof(KeyStringToResultString), _CompareTestEntryToTableEntryKey );
        }
        if (foundEntry) {
            // It does match, so replace old string with new
            strlcpy(inLocaleString, foundEntry->result, sizeof(inLocaleString));
             varKeyValueString[0] = 0;
        } else {
            char *      langRegSubtag = NULL;
            char *      regionTag = NULL;

            // C. No match with an old-style string, use input string but update codes, normalize case, etc.
            _UpdateFullLocaleString(inLocaleString, sizeof(inLocaleString), &langRegSubtag, &regionTag, varKeyValueString);   // <1.10><1.17><1.19>
            
            // if the language part already includes a regional variant, then delete any region tag. <1.19>
            if (langRegSubtag && regionTag)
            	*regionTag = 0;
        }
        
        // D. Now we have an up-to-date locale string, but we need to strip defaults and turn it into a language string
        
        // 1. Strip defaults in input string based on initial part of locale string
        // (mainly to strip default script tag for a language)
        testEntry.key = inLocaleString;
        foundEntry = (KeyStringToResultString *)bsearch( &testEntry, localeStringPrefixToDefaults, kNumLocaleStringPrefixToDefaults,
                                                        sizeof(KeyStringToResultString), _CompareTestEntryPrefixToTableEntryKey );
        if (foundEntry) {
            // The input string begins with a character sequence for which
            // there are default substrings which should be stripped if present
            _RemoveSubstringsIfPresent(inLocaleString, foundEntry->result);         
        }
        
        // 2. If the string matches a locale string used by Apple as a language string, turn it into a language string
        testEntry.key = inLocaleString;
        foundEntry = (KeyStringToResultString *)bsearch( &testEntry, appleLocaleToLanguageString, kNumAppleLocaleToLanguageString,
                                                        sizeof(KeyStringToResultString), _CompareTestEntryToTableEntryKey );
        if (foundEntry) {
            // it does match
            strlcpy(inLocaleString, foundEntry->result, sizeof(inLocaleString));
        } else {
            // skip to any region tag or java-type variant
            char *  inLocalePtr = inLocaleString;
            while (*inLocalePtr != 0 && *inLocalePtr != '_') {
                inLocalePtr++;
            }
            // if there is still a region tag, turn it into a language variant <1.19>
            if (*inLocalePtr == '_') {
            	// handle 3-digit regions in addition to 2-letter ones
            	char *	regionTag = inLocalePtr++;
            	long	expectedLength = 0;
            	if ( isalpha(*inLocalePtr) ) {
            		while ( isalpha(*(++inLocalePtr)) )
            			;
            		expectedLength = 3;
            	} else if ( isdigit(*inLocalePtr) ) {
            		while ( isdigit(*(++inLocalePtr)) )
            			;
            		expectedLength = 4;
            	}
            	*regionTag = (inLocalePtr - regionTag == expectedLength)? '-': 0;
             }
             // anything else at/after '_' just gets deleted
            *inLocalePtr = 0;
        }
        
        // E. Re-append any key-value strings, now canonical										// <1.10><1.17>
		_AppendKeyValueString( inLocaleString, sizeof(inLocaleString), varKeyValueString );
		_AppendKeyValueString( inLocaleString, sizeof(inLocaleString), keyValueString );
        
        // All done, return what we came up with.
        outStringRef = CFStringCreateWithCString(allocator, inLocaleString, kCFStringEncodingASCII);
    }

    return outStringRef;
}


CFStringRef CFLocaleCreateCanonicalLocaleIdentifierFromString(CFAllocatorRef allocator, CFStringRef localeIdentifier) {
    char            inLocaleString[kLocaleIdentifierCStringMax];
    CFStringRef     outStringRef = NULL;
    
    if ( localeIdentifier && CFStringGetCString(localeIdentifier, inLocaleString,  sizeof(inLocaleString), kCFStringEncodingASCII) ) {
        KeyStringToResultString     testEntry;
        KeyStringToResultString *   foundEntry;
        char                        keyValueString[sizeof(inLocaleString)];				// <1.10>
        char			    		varKeyValueString[sizeof(inLocaleString)];			// <1.17>

        _GetKeyValueString(inLocaleString, keyValueString);								// <1.10>
        testEntry.result = NULL;
        
        // A. First check if input string matches an old-style Apple string that has a replacement
        // (do this before case normalization)
        testEntry.key = inLocaleString;
        foundEntry = (KeyStringToResultString *)bsearch( &testEntry, oldAppleLocaleToCanonical, kNumOldAppleLocaleToCanonical,
                                                        sizeof(KeyStringToResultString), _CompareTestEntryToTableEntryKey );
        if (foundEntry) {
            // It does match, so replace old string with new                                // <1.10>
            strlcpy(inLocaleString, foundEntry->result, sizeof(inLocaleString));
            varKeyValueString[0] = 0;
        } else {
            char *      langRegSubtag = NULL;
            char *      regionTag = NULL;

            // B. No match with an old-style string, use input string but update codes, normalize case, etc.
            _UpdateFullLocaleString(inLocaleString, sizeof(inLocaleString), &langRegSubtag, &regionTag, varKeyValueString);   // <1.10><1.17>


            // C. Now strip defaults that are implied by other fields.

            // 1. If an ISO 3166 region tag matches an ISO 3166 regional language variant subtag, strip the latter.
            if ( langRegSubtag && regionTag && strncmp(langRegSubtag+1, regionTag+1, 2) == 0 ) {
                _DeleteCharsAtPointer(langRegSubtag, 3);
            }

            // 2. Strip defaults in input string based on final region tag in locale string
            // (mainly for Chinese, to strip -Hans for _CN/_SG, -Hant for _TW/_HK/_MO)
            if ( regionTag ) {
                testEntry.key = regionTag;
                foundEntry = (KeyStringToResultString *)bsearch( &testEntry, localeStringRegionToDefaults, kNumLocaleStringRegionToDefaults,
                                                                sizeof(KeyStringToResultString), _CompareTestEntryToTableEntryKey );
                if (foundEntry) {
                    _RemoveSubstringsIfPresent(inLocaleString, foundEntry->result);
                }
            }
            
            // 3. Strip defaults in input string based on initial part of locale string
            // (mainly to strip default script tag for a language)
            testEntry.key = inLocaleString;
            foundEntry = (KeyStringToResultString *)bsearch( &testEntry, localeStringPrefixToDefaults, kNumLocaleStringPrefixToDefaults,
                                                            sizeof(KeyStringToResultString), _CompareTestEntryPrefixToTableEntryKey );
            if (foundEntry) {
                // The input string begins with a character sequence for which
                // there are default substrings which should be stripped if present
                _RemoveSubstringsIfPresent(inLocaleString, foundEntry->result);         
            }
        }

        // D. Re-append any key-value strings, now canonical								// <1.10><1.17>
		_AppendKeyValueString( inLocaleString, sizeof(inLocaleString), varKeyValueString );
		_AppendKeyValueString( inLocaleString, sizeof(inLocaleString), keyValueString );
        
        // Now create the CFString (even if empty!)
        outStringRef = CFStringCreateWithCString(allocator, inLocaleString, kCFStringEncodingASCII);
    }

    return outStringRef;
}

// CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes, based on
// the first part of the SPI CFBundleCopyLocalizationForLocalizationInfo in CFBundle_Resources.c
CFStringRef CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes(CFAllocatorRef allocator, LangCode lcode, RegionCode rcode) {
    CFStringRef result = NULL;
    if (0 <= rcode && rcode < kNumRegionCodeToLocaleString) {
	const char *localeString = regionCodeToLocaleString[rcode];
	if (localeString != NULL && *localeString != '\0') {
	    result = CFStringCreateWithCStringNoCopy(allocator, localeString, kCFStringEncodingASCII, kCFAllocatorNull);
	}
    }
    if (result) return result;
    if (0 <= lcode && lcode < kNumLangCodeToLocaleString) {
	const char *localeString = langCodeToLocaleString[lcode];
	if (localeString != NULL && *localeString != '\0') {
	    result = CFStringCreateWithCStringNoCopy(allocator, localeString, kCFStringEncodingASCII, kCFAllocatorNull);
	}
    }
    return result;
}


/*
SPI:  CFLocaleGetLanguageRegionEncodingForLocaleIdentifier gets the appropriate language and region codes,
 and the default legacy script code and encoding, for the specified locale (or language) string.
 Returns false if CFLocale has no information about the given locale (in which case none of the by-reference return values are set);
 otherwise may set *langCode and/or *regCode to -1 if there is no appropriate legacy value for the locale.
 This is a replacement for the CFBundle SPI CFBundleGetLocalizationInfoForLocalization (which was intended to be temporary and transitional);
 this function is more up-to-date in its handling of locale strings, and is in CFLocale where this functionality should belong. Compared
 to CFBundleGetLocalizationInfoForLocalization, this function does not spcially interpret a NULL localeIdentifier to mean use the single most
 preferred localization in the current context (this function returns NO for a NULL localeIdentifier); and in this function
 langCode, regCode, and scriptCode are all SInt16* (not SInt32* like the equivalent parameters in CFBundleGetLocalizationInfoForLocalization).
*/
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
static int CompareLocaleToLegacyCodesEntries( const void *entry1, const void *entry2 );
#endif

Boolean CFLocaleGetLanguageRegionEncodingForLocaleIdentifier(CFStringRef localeIdentifier, LangCode *langCode, RegionCode *regCode, ScriptCode *scriptCode, CFStringEncoding *stringEncoding) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
	Boolean		returnValue = false;
	CFStringRef	canonicalIdentifier = CFLocaleCreateCanonicalLocaleIdentifierFromString(NULL, localeIdentifier);
	if (canonicalIdentifier) {
    	char	localeCString[kLocaleIdentifierCStringMax];
		if ( CFStringGetCString(canonicalIdentifier, localeCString,  sizeof(localeCString), kCFStringEncodingASCII) ) {
			UErrorCode	icuStatus = U_ZERO_ERROR;
			int32_t		languagelength;
			char		searchString[ULOC_LANG_CAPACITY + ULOC_FULLNAME_CAPACITY];
			
			languagelength = uloc_getLanguage( localeCString, searchString, ULOC_LANG_CAPACITY, &icuStatus );
			if ( U_SUCCESS(icuStatus) && languagelength > 0 ) {
				// OK, here we have at least a language code, check for other components in order
				LocaleToLegacyCodes			searchEntry = { (const char *)searchString, 0, 0, 0 };
				const LocaleToLegacyCodes *	foundEntryPtr;
				int32_t						componentLength;
				char						componentString[ULOC_FULLNAME_CAPACITY];
				
				languagelength = strlen(searchString);	// in case it got truncated
				icuStatus = U_ZERO_ERROR;
				componentLength = uloc_getScript( localeCString, componentString, sizeof(componentString), &icuStatus );
				if ( U_FAILURE(icuStatus) || componentLength == 0 ) {
					icuStatus = U_ZERO_ERROR;
					componentLength = uloc_getCountry( localeCString, componentString, sizeof(componentString), &icuStatus );
					if ( U_FAILURE(icuStatus) || componentLength == 0 ) {
						icuStatus = U_ZERO_ERROR;
						componentLength = uloc_getVariant( localeCString, componentString, sizeof(componentString), &icuStatus );
						if ( U_FAILURE(icuStatus) ) {
							componentLength = 0;
						}
					}
				}
				
				// Append whichever other component we first found
				if (componentLength > 0) {
					strlcat(searchString, "_", sizeof(searchString));
					strlcat(searchString, componentString, sizeof(searchString));
				}
				
				// Search
				foundEntryPtr = (const LocaleToLegacyCodes *)bsearch( &searchEntry, localeToLegacyCodes, kNumLocaleToLegacyCodes, sizeof(LocaleToLegacyCodes), CompareLocaleToLegacyCodesEntries );
				if (foundEntryPtr == NULL && (int32_t) strlen(searchString) > languagelength) {
					// truncate to language al;one and try again
					searchString[languagelength] = 0;
					foundEntryPtr = (const LocaleToLegacyCodes *)bsearch( &searchEntry, localeToLegacyCodes, kNumLocaleToLegacyCodes, sizeof(LocaleToLegacyCodes), CompareLocaleToLegacyCodesEntries );
				}

				// If found a matching entry, return requested values
				if (foundEntryPtr) {
					returnValue = true;
					if (langCode)		*langCode		= foundEntryPtr->langCode;
					if (regCode)		*regCode		= foundEntryPtr->regCode;
					if (stringEncoding)	*stringEncoding	= foundEntryPtr->encoding;
					if (scriptCode) {
						// map CFStringEncoding to ScriptCode
						if (foundEntryPtr->encoding < 33/*kCFStringEncodingMacSymbol*/) {
							*scriptCode	= foundEntryPtr->encoding;
						} else {
							switch (foundEntryPtr->encoding) {
								case 0x8C/*kCFStringEncodingMacFarsi*/:		*scriptCode	= 4/*smArabic*/; break;
								case 0x98/*kCFStringEncodingMacUkrainian*/:	*scriptCode	= 7/*smCyrillic*/; break;
								case 0xEC/*kCFStringEncodingMacInuit*/:		*scriptCode	= 28/*smEthiopic*/; break;
								case 0xFC/*kCFStringEncodingMacVT100*/:		*scriptCode	= 32/*smUninterp*/; break;
								default:									*scriptCode	= 0/*smRoman*/; break;
							}
						}
					}
				}
			}
		}
		CFRelease(canonicalIdentifier);
	}
	return returnValue;
#else
    return false;
#endif
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
static int CompareLocaleToLegacyCodesEntries( const void *entry1, const void *entry2 ) {
	const char *	localeString1 = ((const LocaleToLegacyCodes *)entry1)->locale;
	const char *	localeString2 = ((const LocaleToLegacyCodes *)entry2)->locale;
	return strcmp(localeString1, localeString2);
}
#endif

CFDictionaryRef CFLocaleCreateComponentsFromLocaleIdentifier(CFAllocatorRef allocator, CFStringRef localeID) {
    CFMutableDictionaryRef working = CFDictionaryCreateMutable(allocator, 10, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    char cLocaleID[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];
    char buffer[ULOC_FULLNAME_CAPACITY+ULOC_KEYWORD_AND_VALUES_CAPACITY];

    UErrorCode icuStatus = U_ZERO_ERROR;
    int32_t length = 0;

    if (!localeID) goto out;

    // Extract the C string locale ID, for ICU
    CFIndex outBytes = 0;
    CFStringGetBytes(localeID, CFRangeMake(0, CFStringGetLength(localeID)), kCFStringEncodingASCII, (UInt8) '?', true, (unsigned char *)cLocaleID, sizeof(cLocaleID)/sizeof(char) - 1, &outBytes);
    cLocaleID[outBytes] = '\0';

    // Get the components
    length = uloc_getLanguage(cLocaleID, buffer, sizeof(buffer)/sizeof(char), &icuStatus);
    if (U_SUCCESS(icuStatus) && length > 0)
    {
        CFStringRef string = CFStringCreateWithBytes(allocator, (UInt8 *)buffer, length, kCFStringEncodingASCII, true);
        CFDictionaryAddValue(working, kCFLocaleLanguageCodeKey, string);
        CFRelease(string);
    }
    icuStatus = U_ZERO_ERROR;
    
    length = uloc_getScript(cLocaleID, buffer, sizeof(buffer)/sizeof(char), &icuStatus);
    if (U_SUCCESS(icuStatus) && length > 0)
    {
        CFStringRef string = CFStringCreateWithBytes(allocator, (UInt8 *)buffer, length, kCFStringEncodingASCII, true);
        CFDictionaryAddValue(working, kCFLocaleScriptCodeKey, string);
        CFRelease(string);
    }
    icuStatus = U_ZERO_ERROR;
    
    length = uloc_getCountry(cLocaleID, buffer, sizeof(buffer)/sizeof(char), &icuStatus);
    if (U_SUCCESS(icuStatus) && length > 0)
    {
        CFStringRef string = CFStringCreateWithBytes(allocator, (UInt8 *)buffer, length, kCFStringEncodingASCII, true);
        CFDictionaryAddValue(working, kCFLocaleCountryCodeKey, string);
        CFRelease(string);
    }
    icuStatus = U_ZERO_ERROR;
    
    length = uloc_getVariant(cLocaleID, buffer, sizeof(buffer)/sizeof(char), &icuStatus);
    if (U_SUCCESS(icuStatus) && length > 0)
    {
        CFStringRef string = CFStringCreateWithBytes(allocator, (UInt8 *)buffer, length, kCFStringEncodingASCII, true);
        CFDictionaryAddValue(working, kCFLocaleVariantCodeKey, string);
        CFRelease(string);
    }
    icuStatus = U_ZERO_ERROR;
    
    // Now get the keywords; open an enumerator on them
    UEnumeration *iter = uloc_openKeywords(cLocaleID, &icuStatus);
    const char *locKey = NULL;
    int32_t locKeyLen = 0;
    while ((locKey = uenum_next(iter, &locKeyLen, &icuStatus)) && U_SUCCESS(icuStatus))
    {
        char locValue[ULOC_KEYWORD_AND_VALUES_CAPACITY];
    
        // Get the value for this keyword
        if (uloc_getKeywordValue(cLocaleID, locKey, locValue, sizeof(locValue)/sizeof(char), &icuStatus) > 0
            && U_SUCCESS(icuStatus))
        {
            CFStringRef key = CFStringCreateWithBytes(allocator, (UInt8 *)locKey, strlen(locKey), kCFStringEncodingASCII, true);
            CFStringRef value = CFStringCreateWithBytes(allocator, (UInt8 *)locValue, strlen(locValue), kCFStringEncodingASCII, true);
            if (key && value)
                CFDictionaryAddValue(working, key, value);
            if (key)
                CFRelease(key);
            if (value)
                CFRelease(value);
        }
    }
    uenum_close(iter);
    
    out:;
#endif
    // Convert to an immutable dictionary and return
    CFDictionaryRef result = CFDictionaryCreateCopy(allocator, working);
    CFRelease(working);
    return result;
}

static char *__CStringFromString(CFStringRef str) {
    if (!str) return NULL;
    CFRange rg = CFRangeMake(0, CFStringGetLength(str));
    CFIndex neededLength = 0;
    CFStringGetBytes(str, rg, kCFStringEncodingASCII, (UInt8)'?', false, NULL, 0, &neededLength);
    char *buf = (char *)malloc(neededLength + 1);
    CFStringGetBytes(str, rg, kCFStringEncodingASCII, (UInt8)'?', false, (uint8_t *)buf, neededLength, &neededLength);
    buf[neededLength] = '\0';
    return buf;
}

CFStringRef CFLocaleCreateLocaleIdentifierFromComponents(CFAllocatorRef allocator, CFDictionaryRef dictionary) {
    if (!dictionary) return NULL;

    CFIndex cnt = CFDictionaryGetCount(dictionary);
    STACK_BUFFER_DECL(CFStringRef, values, cnt);
    STACK_BUFFER_DECL(CFStringRef, keys, cnt);
    CFDictionaryGetKeysAndValues(dictionary, (const void **)keys, (const void **)values);

    char *language = NULL, *script = NULL, *country = NULL, *variant = NULL;
    for (CFIndex idx = 0; idx < cnt; idx++) {
	if (CFEqual(kCFLocaleLanguageCodeKey, keys[idx])) {
	    language = __CStringFromString(values[idx]);
	    keys[idx] = NULL;
	} else if (CFEqual(kCFLocaleScriptCodeKey, keys[idx])) {
	    script = __CStringFromString(values[idx]);
	    keys[idx] = NULL;
	} else if (CFEqual(kCFLocaleCountryCodeKey, keys[idx])) {
	    country = __CStringFromString(values[idx]);
	    keys[idx] = NULL;
	} else if (CFEqual(kCFLocaleVariantCodeKey, keys[idx])) {
	    variant = __CStringFromString(values[idx]);
	    keys[idx] = NULL;
	}
    }

    char *buf1 = NULL;	// (|L)(|_S)(|_C|_C_V|__V)
    asprintf(&buf1, "%s%s%s%s%s%s%s", language ? language : "", script ? "_" : "", script ? script : "", (country || variant ? "_" : ""), country ? country : "", variant ? "_" : "", variant ? variant : "");

    char cLocaleID[2 * ULOC_FULLNAME_CAPACITY + 2 * ULOC_KEYWORD_AND_VALUES_CAPACITY];
    strlcpy(cLocaleID, buf1, sizeof(cLocaleID));
    free(language);
    free(script);
    free(country);
    free(variant);
    free(buf1);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    for (CFIndex idx = 0; idx < cnt; idx++) {
	if (keys[idx]) {
	    char *key = __CStringFromString(keys[idx]);
	    char *value;
            if (0 == strcmp(key, "kCFLocaleCalendarKey")) {
                // For interchangeability convenience, we alternatively allow a
                // calendar object to be passed in, with the alternate key, and
                // we'll extract the identifier.
                CFCalendarRef cal = (CFCalendarRef)values[idx];
                CFStringRef ident = CFCalendarGetIdentifier(cal);
                value = __CStringFromString(ident);
                char *oldkey = key;
                key = strdup("calendar");
                free(oldkey);
            } else {
                value = __CStringFromString(values[idx]);
            }
	    UErrorCode status = U_ZERO_ERROR;
	    uloc_setKeywordValue(key, value, cLocaleID, sizeof(cLocaleID), &status);
	    free(key);
	    free(value);
	}
    }
#endif
    
    return CFStringCreateWithCString(allocator, cLocaleID, kCFStringEncodingASCII);
}

