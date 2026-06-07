/*
*****************************************************************************************
* Copyright (C) 2014-2019 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#define DEBUG_UALOC 0
#if DEBUG_UALOC
#include <stdio.h>
#endif
#include <string.h>
#include <ctype.h>
#include "unicode/utypes.h"
#include "unicode/ualoc.h"
#include "unicode/uloc.h"
#include "unicode/ures.h"
#include "unicode/putil.h"
#include "unicode/ustring.h"
#include "cstring.h"
#include "cmemory.h"
#include "uhash.h"
#include "umutex.h"
#include "ucln_cmn.h"
// the following has replacements for some math.h funcs etc
#include "putilimp.h"


// The numeric values in territoryInfo are in "IntF" format from LDML2ICUConverter.
// From its docs (adapted): [IntF is] a special integer that represents the number in
// normalized scientific notation.
// Resultant integers are in the form -?xxyyyyyy, where xx is the exponent
// offset by 50 and yyyyyy is the coefficient to 5 decimal places (range 1.0 to 9.99999), e.g.
// 14660000000000 -> 1.46600E13 -> 63146600
// 0.0001 -> 1.00000E-4 -> 46100000
// -123.456 -> -1.23456E-2 -> -48123456
//
// Here to avoid an extra division we have the max coefficient as 999999 (instead of
// 9.99999) and instead offset the exponent by -55.
//
static double doubleFromIntF(int32_t intF) {
    double coefficient = (double)(intF % 1000000);
    int32_t exponent = (intF / 1000000) - 55;
    return coefficient * uprv_pow10(exponent);
}

static int compareLangEntries(const void * entry1, const void * entry2) {
    double fraction1 = ((const UALanguageEntry *)entry1)->userFraction;
    double fraction2 = ((const UALanguageEntry *)entry2)->userFraction;
    // want descending order
    if (fraction1 > fraction2) return -1;
    if (fraction1 < fraction2) return 1;
    // userFractions the same, sort by languageCode
    return uprv_strcmp(((const UALanguageEntry *)entry1)->languageCode,((const UALanguageEntry *)entry2)->languageCode);
}

// language codes to version with default script
// must be sorted by language code
static const char * langToDefaultScript[] = {
    "az",   "az_Latn",
    "bm",   "bm_Latn",  // <rdar://problem/47494729> added
    "bs",   "bs_Latn",
    "byn",  "byn_Ethi", // <rdar://problem/47494729> added
    "cu",   "cu_Cyrl",  // <rdar://problem/47494729> added
    "ff",   "ff_Latn",  // <rdar://problem/47494729> added
    "ha",   "ha_Latn",  // <rdar://problem/47494729> added
    "iu",   "iu_Cans",
    "kk",   "kk_Cyrl",  // <rdar://problem/47494729> changed from _Arab
    "ks",   "ks_Arab",  // unnecessary?
    "ku",   "ku_Latn",
    "ky",   "ky_Cyrl",
    "mn",   "mn_Cyrl",
    "ms",   "ms_Latn",
    "pa",   "pa_Guru",
    "rif",  "rif_Tfng", // unnecessary? no locale support anyway
    "sd",   "sd_Arab",  // <rdar://problem/47494729> added
    "shi",  "shi_Tfng",
    "sr",   "sr_Cyrl",
    "tg",   "tg_Cyrl",
    "tk",   "tk_Latn",  // unnecessary?
    "ug",   "ug_Arab",
    "uz",   "uz_Latn",
    "vai",  "vai_Vaii",
    "yue",  "yue_Hant", // to match CLDR data, not Apple default
    "zh",   "zh_Hans",
    NULL
};

static const char * langCodeWithScriptIfAmbig(const char * langCode) {
	const char ** langToDefScriptPtr = langToDefaultScript;
	const char * testCurLoc;
	while ( (testCurLoc = *langToDefScriptPtr++) != NULL ) {
		int cmp = uprv_strcmp(langCode, testCurLoc);
		if (cmp <= 0) {
			if (cmp == 0) {
			    return *langToDefScriptPtr;
			}
			break;
		}
		langToDefScriptPtr++;
	}
    return langCode;
}

static const UChar ustrLangStatusDefacto[]  = {0x64,0x65,0x5F,0x66,0x61,0x63,0x74,0x6F,0x5F,0x6F,0x66,0x66,0x69,0x63,0x69,0x61,0x6C,0}; //"de_facto_official"
static const UChar ustrLangStatusOfficial[] = {0x6F,0x66,0x66,0x69,0x63,0x69,0x61,0x6C,0}; //"official"
static const UChar ustrLangStatusRegional[] = {0x6F,0x66,0x66,0x69,0x63,0x69,0x61,0x6C,0x5F,0x72,0x65,0x67,0x69,0x6F,0x6E,0x61,0x6C,0}; //"official_regional"

enum {
    kLocalLangEntriesMax = 26, // enough for most regions to minimumFraction 0.001 except India
    kLangEntriesFactor = 3     // if we have to allocate, multiply existing size by this
};

U_CAPI int32_t U_EXPORT2
ualoc_getLanguagesForRegion(const char *regionID, double minimumFraction,
                            UALanguageEntry *entries, int32_t entriesCapacity,
                            UErrorCode *err)
{
    if (U_FAILURE(*err)) {
        return 0;
    }
    if ( regionID == NULL || minimumFraction < 0.0 || minimumFraction > 1.0 ||
        ((entries==NULL)? entriesCapacity!=0: entriesCapacity<0) ) {
        *err = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    UResourceBundle *rb = ures_openDirect(NULL, "supplementalData", err);
    rb = ures_getByKey(rb, "territoryInfo", rb, err);
    rb = ures_getByKey(rb, regionID, rb, err);
    if (U_FAILURE(*err)) {
        ures_close(rb);
        return 0;
    }

    int32_t entryCount = 0;
    UResourceBundle *langBund = NULL;
    int32_t lbIdx, lbCount = ures_getSize(rb);
    UALanguageEntry localLangEntries[kLocalLangEntriesMax];
    UALanguageEntry * langEntries = localLangEntries;
    int32_t langEntriesMax = kLocalLangEntriesMax;

    for (lbIdx = 0; lbIdx < lbCount; lbIdx++) {
        langBund = ures_getByIndex(rb, lbIdx, langBund, err);
        if (U_FAILURE(*err)) {
            break;
        }
        const char * langCode = ures_getKey(langBund);
        if (uprv_strcmp(langCode,"territoryF") == 0) {
            continue;
        }
        if (strnlen(langCode, UALANGDATA_CODELEN+1) > UALANGDATA_CODELEN) { // no uprv_strnlen
            continue; // a code we cannot handle
        }

        UErrorCode localErr = U_ZERO_ERROR;
        double userFraction = 0.0;
        UResourceBundle *itemBund = ures_getByKey(langBund, "populationShareF", NULL, &localErr);
        if (U_SUCCESS(localErr)) {
            int32_t intF = ures_getInt(itemBund, &localErr);
            if (U_SUCCESS(localErr)) {
                userFraction = doubleFromIntF(intF);
            }
            ures_close(itemBund);
        }
        if (userFraction < minimumFraction) {
            continue;
        }
        if (entries != NULL) {
            localErr = U_ZERO_ERROR;
            UALanguageStatus langStatus = UALANGSTATUS_UNSPECIFIED;
            int32_t ulen;
            const UChar * ustrLangStatus = ures_getStringByKey(langBund, "officialStatus", &ulen, &localErr);
            if (U_SUCCESS(localErr)) {
                int32_t cmp = u_strcmp(ustrLangStatus, ustrLangStatusOfficial);
                if (cmp == 0) {
                    langStatus = UALANGSTATUS_OFFICIAL;
                } else if (cmp < 0 && u_strcmp(ustrLangStatus, ustrLangStatusDefacto) == 0) {
                    langStatus = UALANGSTATUS_DEFACTO_OFFICIAL;
                } else if (u_strcmp(ustrLangStatus, ustrLangStatusRegional) == 0) {
                    langStatus = UALANGSTATUS_REGIONAL_OFFICIAL;
                }
            }
            // Now we have all of the info for our next entry
            if (entryCount >= langEntriesMax) {
                int32_t newMax = langEntriesMax * kLangEntriesFactor;
                if (langEntries == localLangEntries) {
                    // first allocation, copy from local buf
                    langEntries = (UALanguageEntry*)uprv_malloc(newMax*sizeof(UALanguageEntry));
                    if (langEntries == NULL) {
                        *err = U_MEMORY_ALLOCATION_ERROR;
                        break;
                    }
                    uprv_memcpy(langEntries, localLangEntries, entryCount*sizeof(UALanguageEntry));
                } else {
                    langEntries = (UALanguageEntry*)uprv_realloc(langEntries, newMax*sizeof(UALanguageEntry));
                    if (langEntries == NULL) {
                        *err = U_MEMORY_ALLOCATION_ERROR;
                        break;
                    }
                }
                langEntriesMax = newMax;
            }
            uprv_strcpy(langEntries[entryCount].languageCode, langCodeWithScriptIfAmbig(langCode));
            langEntries[entryCount].userFraction = userFraction;
            langEntries[entryCount].status = langStatus;
        }
        entryCount++;
    }
    ures_close(langBund);
    ures_close(rb);
    if (U_FAILURE(*err)) {
        if (langEntries != localLangEntries) {
            free(langEntries);
        }
        return 0;
    }
    if (entries != NULL) {
        // sort langEntries, copy entries that fit to provided array
        qsort(langEntries, entryCount, sizeof(UALanguageEntry), compareLangEntries);
        if (entryCount > entriesCapacity) {
            entryCount = entriesCapacity;
        }
        uprv_memcpy(entries, langEntries, entryCount*sizeof(UALanguageEntry));
        if (langEntries != localLangEntries) {
            free(langEntries);
        }
    }
    return entryCount;
}

static const char * forceParent[] = {
    "en_150",  "en_GB",  // en for Europe
    "en_AU",   "en_GB",
    "en_BD",   "en_GB",  // en for Bangladesh
    "en_BE",   "en_150", // en for Belgium goes to en for Europe
    "en_DG",   "en_GB",
    "en_FK",   "en_GB",
    "en_GG",   "en_GB",
    "en_GI",   "en_GB",
    "en_HK",   "en_GB",  // en for Hong Kong
    "en_IE",   "en_GB",
    "en_IM",   "en_GB",
    "en_IN",   "en_GB",
    "en_IO",   "en_GB",
    "en_JE",   "en_GB",
    "en_JM",   "en_GB",
    "en_LK",   "en_GB",
    "en_MO",   "en_GB",
    "en_MT",   "en_GB",
    "en_MV",   "en_GB",  // for Maldives
    "en_MY",   "en_GB",  // en for Malaysia
    "en_NZ",   "en_AU",
    "en_PK",   "en_GB",  // en for Pakistan
    "en_SG",   "en_GB",
    "en_SH",   "en_GB",
    "en_VG",   "en_GB",
    "yue",     "yue_CN", // yue_CN has 71M users (5.2% of 1.37G), yue_HK has 6.5M (90% of 7.17M)
    "yue_CN",  "root",
    "yue_HK",  "root",
    "yue_Hans","yue_CN",
    "yue_Hant","yue_HK",
    "zh",      "zh_CN",
    "zh_CN",   "root",
    "zh_Hant", "zh_TW",
    "zh_TW",   "root",
    NULL
};

enum { kLocBaseNameMax = 16 };

U_CAPI int32_t U_EXPORT2
ualoc_getAppleParent(const char* localeID,
                     char * parent,
                     int32_t parentCapacity,
                     UErrorCode* err)
{
    UResourceBundle *rb;
    int32_t len;
    UErrorCode tempStatus;
    char locbuf[ULOC_FULLNAME_CAPACITY+1];
    char * foundDoubleUnderscore;

    if (U_FAILURE(*err)) {
        return 0;
    }
    if ( (parent==NULL)? parentCapacity!=0: parentCapacity<0 ) {
        *err = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    len = uloc_getBaseName(localeID, locbuf, ULOC_FULLNAME_CAPACITY, err); /* canonicalize and strip keywords */
    if (U_FAILURE(*err)) {
        return 0;
    }
    if (*err == U_STRING_NOT_TERMINATED_WARNING) {
        locbuf[ULOC_FULLNAME_CAPACITY] = 0;
        *err = U_ZERO_ERROR;
    }
    foundDoubleUnderscore = uprv_strstr(locbuf, "__"); /* __ comes from bad/missing subtag or variant */
    if (foundDoubleUnderscore != NULL) {
        *foundDoubleUnderscore = 0; /* terminate at the __ */
        len = uprv_strlen(locbuf);
    }
    if (len >= 2 && (uprv_strncmp(locbuf, "en", 2) == 0 || uprv_strncmp(locbuf, "zh", 2) == 0)) {
        const char ** forceParentPtr = forceParent;
        const char * testCurLoc;
        while ( (testCurLoc = *forceParentPtr++) != NULL ) {
            int cmp = uprv_strcmp(locbuf, testCurLoc);
            if (cmp <= 0) {
                if (cmp == 0) {
                    len = uprv_strlen(*forceParentPtr);
                    if (len < parentCapacity) {
                        uprv_strcpy(parent, *forceParentPtr);
                    } else {
                        *err = U_BUFFER_OVERFLOW_ERROR;
                    }
                    return len;
                }
                break;
            }
            forceParentPtr++;
        }
    }
    tempStatus = U_ZERO_ERROR;
    rb = ures_openDirect(NULL, locbuf, &tempStatus);
    if (U_SUCCESS(tempStatus)) {
        const char * actualLocale = ures_getLocaleByType(rb, ULOC_ACTUAL_LOCALE, &tempStatus);
        ures_close(rb);
        if (U_SUCCESS(tempStatus) && uprv_strcmp(locbuf, actualLocale) != 0) {
            // we have followed an alias
            len = uprv_strlen(actualLocale);
            if (len < parentCapacity) {
                uprv_strcpy(parent, actualLocale);
            } else {
                *err = U_BUFFER_OVERFLOW_ERROR;
            }
            return len;
        }
    }
    tempStatus = U_ZERO_ERROR;
    rb = ures_openDirect(NULL, "supplementalData", &tempStatus);
    rb = ures_getByKey(rb, "parentLocales", rb, &tempStatus);
    if (U_SUCCESS(tempStatus)) {
        UResourceBundle * parentMapBundle = NULL;
        int32_t childLen = 0;
        while (childLen == 0) {
            tempStatus = U_ZERO_ERROR;
            parentMapBundle = ures_getNextResource(rb, parentMapBundle, &tempStatus);
            if (U_FAILURE(tempStatus)) {
                break; // no more parent bundles, normal exit
            }
            char childName[kLocBaseNameMax + 1];
            childName[kLocBaseNameMax] = 0;
            const char * childPtr = NULL;
            if (ures_getType(parentMapBundle) == URES_STRING) {
                childLen = kLocBaseNameMax;
                childPtr = ures_getUTF8String(parentMapBundle, childName, &childLen, FALSE, &tempStatus);
                if (U_FAILURE(tempStatus) || uprv_strncmp(locbuf, childPtr, kLocBaseNameMax) != 0) {
                    childLen = 0;
                }
            } else { // should be URES_ARRAY
                int32_t childCur, childCount = ures_getSize(parentMapBundle);
                for (childCur = 0; childCur < childCount && childLen == 0; childCur++) {
                    tempStatus = U_ZERO_ERROR;
                    childLen = kLocBaseNameMax;
                    childPtr = ures_getUTF8StringByIndex(parentMapBundle, childCur, childName, &childLen, FALSE, &tempStatus);
                    if (U_FAILURE(tempStatus) || uprv_strncmp(locbuf, childPtr, kLocBaseNameMax) != 0) {
                        childLen = 0;
                    }
                }
            }
        }
        ures_close(rb);
        if (childLen > 0) {
            // parentMapBundle key is the parent we are looking for
            const char * keyStr = ures_getKey(parentMapBundle);
            len = uprv_strlen(keyStr);
            if (len < parentCapacity) {
                 uprv_strcpy(parent, keyStr);
            } else {
                *err = U_BUFFER_OVERFLOW_ERROR;
            }
            ures_close(parentMapBundle);
            return len;
        }
        ures_close(parentMapBundle);
    }
    
    len = uloc_getParent(locbuf, parent, parentCapacity, err);
    if (U_SUCCESS(*err) && len == 0) {
        len = 4;
        if (len < parentCapacity) {
            uprv_strcpy(parent, "root");
        } else {
            *err = U_BUFFER_OVERFLOW_ERROR;
        }
    }
    return len;
}

// =================
// Data and related functions for ualoc_localizationsToUse
// =================

static const char * appleAliasMap[][2] = {
    // names are lowercase here because they are looked up after being processed by uloc_getBaseName
    { "arabic",     "ar"      },    // T2
    { "chinese",    "zh_Hans" },    // T0
    { "danish",     "da"      },    // T2
    { "dutch",      "nl"      },    // T1, still in use
    { "english",    "en"      },    // T0, still in use
    { "finnish",    "fi"      },    // T2
    { "french",     "fr"      },    // T0, still in use
    { "german",     "de"      },    // T0, still in use
    { "italian",    "it"      },    // T1, still in use
    { "japanese",   "ja"      },    // T0, still in use
    { "korean",     "ko"      },    // T1
    { "no_NO",      "nb_NO"   },    // special
    { "norwegian",  "nb"      },    // T2
    { "polish",     "pl"      },    // T2
    { "portuguese", "pt"      },    // T2
    { "russian",    "ru"      },    // T2
    { "spanish",    "es"      },    // T1, still in use
    { "swedish",    "sv"      },    // T2
    { "thai",       "th"      },    // T2
    { "turkish",    "tr"      },    // T2
    { "yue",        "yue_Hans"},    // special
    { "zh",         "zh_Hans" },    // special
};
enum { kAppleAliasMapCount = UPRV_LENGTHOF(appleAliasMap) };

static const char * appleParentMap[][2] = {
    { "en_150",     "en_GB"   },    // Apple custom parent
    { "en_AD",      "en_150"  },    // Apple locale addition
    { "en_AG",      "en_GB"   },    // Antigua & Barbuda
    { "en_AI",      "en_GB"   },    // Anguilla
    { "en_AL",      "en_150"  },    // Apple locale addition
    { "en_AT",      "en_150"  },    // Apple locale addition
    { "en_AU",      "en_GB"   },    // Apple custom parent
    { "en_BA",      "en_150"  },    // Apple locale addition
    { "en_BB",      "en_GB"   },    // Barbados
    { "en_BD",      "en_GB"   },    // Apple custom parent
    { "en_BE",      "en_150"  },    // Apple custom parent
    { "en_BM",      "en_GB"   },    // Bermuda
    { "en_BS",      "en_GB"   },    // Bahamas
    { "en_BW",      "en_GB"   },    // Botswana
    { "en_BZ",      "en_GB"   },    // Belize
    { "en_CC",      "en_AU"   },    // Cocos (Keeling) Islands
    { "en_CH",      "en_150"  },    // Apple locale addition
    { "en_CK",      "en_AU"   },    // Cook Islands (maybe to en_NZ instead?)
    { "en_CX",      "en_AU"   },    // Christmas Island
    { "en_CY",      "en_150"  },    // Apple locale addition
    { "en_CZ",      "en_150"  },    // Apple locale addition
    { "en_DE",      "en_150"  },    // Apple locale addition
    { "en_DG",      "en_GB"   },
    { "en_DK",      "en_150"  },    // Apple locale addition
    { "en_DM",      "en_GB"   },    // Dominica
    { "en_EE",      "en_150"  },    // Apple locale addition
    { "en_ES",      "en_150"  },    // Apple locale addition
    { "en_FI",      "en_150"  },    // Apple locale addition
    { "en_FJ",      "en_GB"   },    // Fiji
    { "en_FK",      "en_GB"   },
    { "en_FR",      "en_150"  },    // Apple locale addition
    { "en_GD",      "en_GB"   },    // Grenada
    { "en_GG",      "en_GB"   },
    { "en_GH",      "en_GB"   },    // Ghana
    { "en_GI",      "en_GB"   },
    { "en_GM",      "en_GB"   },    // Gambia
    { "en_GR",      "en_150"  },    // Apple locale addition
    { "en_GY",      "en_GB"   },    // Guyana
    { "en_HK",      "en_GB"   },    // Apple custom parent
    { "en_HR",      "en_150"  },    // Apple locale addition
    { "en_HU",      "en_150"  },    // Apple locale addition
    { "en_IE",      "en_GB"   },
    { "en_IL",      "en_001"  },    // Apple locale addition
    { "en_IM",      "en_GB"   },
    { "en_IN",      "en_GB"   },    // Apple custom parent
    { "en_IO",      "en_GB"   },
    { "en_IS",      "en_150"  },    // Apple locale addition
    { "en_IT",      "en_150"  },    // Apple locale addition
    { "en_JE",      "en_GB"   },
    { "en_JM",      "en_GB"   },
    { "en_KE",      "en_GB"   },    // Kenya
    { "en_KI",      "en_GB"   },    // Kiribati
    { "en_KN",      "en_GB"   },    // St. Kitts & Nevis
    { "en_KY",      "en_GB"   },    // Cayman Islands
    { "en_LC",      "en_GB"   },    // St. Lucia
    { "en_LK",      "en_GB"   },    // Apple custom parent
    { "en_LS",      "en_GB"   },    // Lesotho
    { "en_LT",      "en_150"  },    // Apple locale addition
    { "en_LU",      "en_150"  },    // Apple locale addition
    { "en_LV",      "en_150"  },    // Apple locale addition
    { "en_ME",      "en_150"  },    // Apple locale addition
    { "en_MO",      "en_GB"   },
    { "en_MS",      "en_GB"   },    // Montserrat
    { "en_MT",      "en_GB"   },
    { "en_MU",      "en_GB"   },    // Mauritius
    { "en_MV",      "en_GB"   },
    { "en_MW",      "en_GB"   },    // Malawi
    { "en_MY",      "en_GB"   },    // Apple custom parent
    { "en_NA",      "en_GB"   },    // Namibia
    { "en_NF",      "en_AU"   },    // Norfolk Island
    { "en_NG",      "en_GB"   },    // Nigeria
    { "en_NL",      "en_150"  },    // Apple locale addition
    { "en_NO",      "en_150"  },    // Apple locale addition
    { "en_NR",      "en_AU"   },    // Nauru
    { "en_NU",      "en_AU"   },    // Niue (maybe to en_NZ instead?)
    { "en_NZ",      "en_AU"   },
    { "en_PG",      "en_AU"   },    // Papua New Guinea
    { "en_PK",      "en_GB"   },    // Apple custom parent
    { "en_PL",      "en_150"  },    // Apple locale addition
    { "en_PN",      "en_GB"   },    // Pitcairn Islands
    { "en_PT",      "en_150"  },    // Apple locale addition
    { "en_RO",      "en_150"  },    // Apple locale addition
    { "en_RS",      "en_150"  },    // Apple locale addition
    { "en_RU",      "en_150"  },    // Apple locale addition
    { "en_SB",      "en_GB"   },    // Solomon Islands
    { "en_SC",      "en_GB"   },    // Seychelles
    { "en_SD",      "en_GB"   },    // Sudan
    { "en_SE",      "en_150"  },    // Apple locale addition
    { "en_SG",      "en_GB"   },
    { "en_SH",      "en_GB"   },
    { "en_SI",      "en_150"  },    // Apple locale addition
    { "en_SK",      "en_150"  },    // Apple locale addition
    { "en_SL",      "en_GB"   },    // Sierra Leone
    { "en_SS",      "en_GB"   },    // South Sudan
    { "en_SZ",      "en_GB"   },    // Swaziland
    { "en_TC",      "en_GB"   },    // Tristan da Cunha
    { "en_TO",      "en_GB"   },    // Tonga
    { "en_TT",      "en_GB"   },    // Trinidad & Tobago
    { "en_TV",      "en_GB"   },    // Tuvalu
    { "en_TZ",      "en_GB"   },    // Tanzania
    { "en_UA",      "en_150"  },    // Apple locale addition
    { "en_UG",      "en_GB"   },    // Uganda
    { "en_VC",      "en_GB"   },    // St. Vincent & Grenadines
    { "en_VG",      "en_GB"   },
    { "en_VU",      "en_GB"   },    // Vanuatu
    { "en_WS",      "en_AU"   },    // Samoa (maybe to en_NZ instead?)
    { "en_ZA",      "en_GB"   },    // South Africa
    { "en_ZM",      "en_GB"   },    // Zambia
    { "en_ZW",      "en_GB"   },    // Zimbabwe
};
enum { kAppleParentMapCount = UPRV_LENGTHOF(appleParentMap) };

typedef struct {
    const char * locale;
    const char * parent;
    int8_t       distance;
} LocParentAndDistance;

static LocParentAndDistance locParentMap[] = {
    // The localizations listed in the first column are in
    // normalized form (e.g. zh_CN -> zh_Hans_CN, etc.).
    // The distance is a rough measure of distance from
    // the localization to its parent, used as a weight.
    { "de_DE",      "de",      0 },
    { "en_001",     "en",      2 },
    { "en_150",     "en_GB",   1 },
    { "en_AU",      "en_GB",   1 },
    { "en_GB",      "en_001",  0 },
    { "en_US",      "en",      0 },
    { "es_419",     "es",      2 },
    { "es_MX",      "es_419",  0 },
    { "fr_FR",      "fr",      0 },
    { "it_IT",      "it",      0 },
    { "pt_PT",      "pt",      2 },
    { "yue_Hans_CN","yue_Hans",0 },
    { "yue_Hant_HK","yue_Hant",0 },
    { "zh_Hans_CN", "zh_Hans", 0 },
    { "zh_Hant_HK", "zh_Hant", 1 },
    { "zh_Hant_TW", "zh_Hant", 0 },
};
enum { kLocParentMapCount = UPRV_LENGTHOF(locParentMap), kMaxParentDistance = 8 };

enum {
    kStringsAllocSize = 5280, // cannot expand; current actual usage 5259
    kParentMapInitCount = 272 // can expand; current actual usage 254
};

U_CDECL_BEGIN
static UBool U_CALLCONV ualocale_cleanup(void);
U_CDECL_END

U_NAMESPACE_BEGIN

static UInitOnce gUALocaleCacheInitOnce = U_INITONCE_INITIALIZER;

static int gMapDataState = 0; // 0 = not initialized, 1 = initialized, -1 = failure
static char* gStrings = NULL;
static UHashtable* gAliasMap = NULL;
static UHashtable* gParentMap = NULL;

U_NAMESPACE_END

U_CDECL_BEGIN

static UBool U_CALLCONV ualocale_cleanup(void)
{
    U_NAMESPACE_USE

    gUALocaleCacheInitOnce.reset();

    if (gMapDataState > 0) {
        uhash_close(gParentMap);
        gParentMap = NULL;
        uhash_close(gAliasMap);
        gAliasMap = NULL;
        uprv_free(gStrings);
        gStrings = NULL;
    }
    gMapDataState = 0;
    return TRUE;
}

static void initializeMapData() {
    U_NAMESPACE_USE

    UResourceBundle * curBundle;
    char* stringsPtr;
    char* stringsEnd;
    UErrorCode status;
    int32_t entryIndex, icuEntryCount;

    ucln_common_registerCleanup(UCLN_COMMON_LOCALE, ualocale_cleanup);

    gStrings = (char*)uprv_malloc(kStringsAllocSize);
    if (gStrings) {
        stringsPtr = gStrings;
        stringsEnd = gStrings + kStringsAllocSize;
    }

    status = U_ZERO_ERROR;
    curBundle = NULL;
    icuEntryCount = 0;
    if (gStrings) {
        curBundle = ures_openDirect(NULL, "metadata", &status);
        curBundle = ures_getByKey(curBundle, "alias", curBundle, &status);
        curBundle = ures_getByKey(curBundle, "language", curBundle, &status); // language resource is URES_TABLE
        if (U_SUCCESS(status)) {
            icuEntryCount = ures_getSize(curBundle); // currently 331
        }
    }
    status = U_ZERO_ERROR;
    gAliasMap = uhash_openSize(uhash_hashIChars, uhash_compareIChars, uhash_compareIChars,
                                kAppleAliasMapCount + icuEntryCount, &status);
    // defaults to keyDeleter NULL
    if (U_SUCCESS(status)) {
        for (entryIndex = 0; entryIndex < kAppleAliasMapCount && U_SUCCESS(status); entryIndex++) {
            uhash_put(gAliasMap, (void*)appleAliasMap[entryIndex][0], (void*)appleAliasMap[entryIndex][1], &status);
#if DEBUG_UALOC
            if (U_FAILURE(status)) {
                printf("# uhash_put 1 fails %s\n", u_errorName(status));
            }
#endif
        }
        status = U_ZERO_ERROR;
        UResourceBundle * aliasMapBundle = NULL;
        for (entryIndex = 0; entryIndex < icuEntryCount && U_SUCCESS(status); entryIndex++) {
            aliasMapBundle = ures_getByIndex(curBundle, entryIndex, aliasMapBundle, &status);
            if (U_FAILURE(status)) {
                break; // error
            }
            const char * keyStr = ures_getKey(aliasMapBundle);
            int32_t len = uprv_strlen(keyStr);
            if (len >= stringsEnd - stringsPtr) {
                break; // error
            }
            uprv_strcpy(stringsPtr, keyStr);
            char * inLocStr = stringsPtr;
            stringsPtr += len + 1;

            len = stringsEnd - stringsPtr - 1;
            ures_getUTF8StringByKey(aliasMapBundle, "replacement", stringsPtr, &len, TRUE, &status);
            if (U_FAILURE(status)) {
                break; // error
            }
            stringsPtr[len] = 0;
            uhash_put(gAliasMap, inLocStr, stringsPtr, &status);
#if DEBUG_UALOC
            if (U_FAILURE(status)) {
                printf("# uhash_put 2 fails %s\n", u_errorName(status));
            }
#endif
            stringsPtr += len + 1;
        }
        ures_close(aliasMapBundle);
    } else {
        ures_close(curBundle);
        uprv_free(gStrings);
        gMapDataState = -1; // failure
        return;
    }
    ures_close(curBundle);

    status = U_ZERO_ERROR;
    gParentMap = uhash_openSize(uhash_hashIChars, uhash_compareIChars, uhash_compareIChars,
                                kParentMapInitCount, &status);
    // defaults to keyDeleter NULL
    if (U_SUCCESS(status)) {
        curBundle = ures_openDirect(NULL, "supplementalData", &status);
        curBundle = ures_getByKey(curBundle, "parentLocales", curBundle, &status); // parentLocales resource is URES_TABLE
        if (U_SUCCESS(status)) {
            UResourceBundle * parentMapBundle = NULL;
            while (TRUE) {
                parentMapBundle = ures_getNextResource(curBundle, parentMapBundle, &status);
                if (U_FAILURE(status)) {
                    break; // no more parent bundles, normal exit
                }
                const char * keyStr = ures_getKey(parentMapBundle);
                int32_t len = uprv_strlen(keyStr);
                if (len >= stringsEnd - stringsPtr) {
                    break; // error
                }
                uprv_strcpy(stringsPtr, keyStr);
                char * parentStr = stringsPtr;
                stringsPtr += len + 1;

                if (ures_getType(parentMapBundle) == URES_STRING) {
                    len = stringsEnd - stringsPtr - 1;
                    ures_getUTF8String(parentMapBundle, stringsPtr, &len, TRUE, &status);
                    if (U_FAILURE(status)) {
                        break; // error
                    }
                    stringsPtr[len] = 0;
                    uhash_put(gParentMap, stringsPtr, parentStr, &status);
                    stringsPtr += len + 1;
                } else {
                    // should be URES_ARRAY
                    icuEntryCount = ures_getSize(parentMapBundle);
                    for (entryIndex = 0; entryIndex < icuEntryCount && U_SUCCESS(status); entryIndex++) {
                        len = stringsEnd - stringsPtr - 1;
                        ures_getUTF8StringByIndex(parentMapBundle, entryIndex, stringsPtr, &len, TRUE, &status);
                        if (U_FAILURE(status)) {
                            break;
                        }
                        stringsPtr[len] = 0;
                        uhash_put(gParentMap, stringsPtr, parentStr, &status);
                        stringsPtr += len + 1;
                    }
                }
            }
            ures_close(parentMapBundle);
        }
        ures_close(curBundle);

        status = U_ZERO_ERROR;
        for (entryIndex = 0; entryIndex < kAppleParentMapCount && U_SUCCESS(status); entryIndex++) {
            uhash_put(gParentMap, (void*)appleParentMap[entryIndex][0], (void*)appleParentMap[entryIndex][1], &status);
        }
    } else {
        uhash_close(gAliasMap);
        gAliasMap = NULL;
        uprv_free(gStrings);
        gMapDataState = -1; // failure
        return;
    }

#if DEBUG_UALOC
    printf("# gStrings size %ld\n", stringsPtr - gStrings);
    printf("# gParentMap count %d\n", uhash_count(gParentMap));
#endif
    gMapDataState = 1;
}

U_CDECL_END

// The following maps aliases, etc. Ensures 0-termination if no error.
static void ualoc_normalize(const char *locale, char *normalized, int32_t normalizedCapacity, UErrorCode *status)
{
    if (U_FAILURE(*status)) {
        return;
    }
    // uloc_minimizeSubtags(locale, normalized, normalizedCapacity, status);

    const char *replacement =  NULL;
    if (icu::gMapDataState > 0) {
        replacement = (const char *)uhash_get(icu::gAliasMap, locale);
    }
    if (replacement == NULL) {
        replacement = locale;
    }
    int32_t len = strnlen(replacement, normalizedCapacity);
    if (len < normalizedCapacity) { // allow for 0 termination
        uprv_strcpy(normalized, replacement);
    } else {
        *status = U_BUFFER_OVERFLOW_ERROR;
    }
}

static void ualoc_getParent(const char *locale, char *parent, int32_t parentCapacity, UErrorCode *status)
{
    if (U_FAILURE(*status)) {
        return;
    }
    if (icu::gMapDataState > 0) {
        const char *replacement = (const char *)uhash_get(icu::gParentMap, locale);
        if (replacement) {
            int32_t len = uprv_strlen(replacement);
            if (len < parentCapacity) { // allow for 0 termination
                uprv_strcpy(parent, replacement);
#if DEBUG_UALOC
                printf("    # ualoc_getParent 1: locale %s -> parent %s\n", locale, parent);
#endif
            } else {
                *status = U_BUFFER_OVERFLOW_ERROR;
            }
            return;
        }
    }
    uloc_getParent(locale, parent, parentCapacity - 1, status);
#if DEBUG_UALOC
    printf("    # ualoc_getParent 2: locale %s -> parent %s\n", locale, parent);
#endif
    parent[parentCapacity - 1] = 0; // ensure 0 termination in case of U_STRING_NOT_TERMINATED_WARNING
}

// Might do something better for this, perhaps maximizing locales then stripping
static const char * getLocParent(const char *locale, int32_t* distance)
{
    int32_t locParentIndex;
    for (locParentIndex = 0; locParentIndex < kLocParentMapCount; locParentIndex++) {
        if (uprv_strcmp(locale, locParentMap[locParentIndex].locale) == 0) {
            *distance = locParentMap[locParentIndex].distance;
            return locParentMap[locParentIndex].parent;
        }
    }
    if (icu::gMapDataState > 0) {
        const char *replacement = (const char *)uhash_get(icu::gParentMap, locale);
        if (replacement) {
            *distance = 1;
            return replacement;
        }
    }
    return NULL;
}

// this just checks if the *pointer* value is already in the array
static UBool locInArray(const char* *localizationsToUse, int32_t locsToUseCount, const char *locToCheck)
{
    int32_t locIndex;
    for (locIndex = 0; locIndex < locsToUseCount; locIndex++) {
        if (locToCheck == localizationsToUse[locIndex]) {
            return TRUE;
        }
    }
    return FALSE;
}

enum { kLangScriptRegMaxLen = ULOC_LANG_CAPACITY + ULOC_SCRIPT_CAPACITY + ULOC_COUNTRY_CAPACITY }; // currently 22

int32_t
ualoc_localizationsToUse( const char* const *preferredLanguages,
                          int32_t preferredLanguagesCount,
                          const char* const *availableLocalizations,
                          int32_t availableLocalizationsCount,
                          const char* *localizationsToUse,
                          int32_t localizationsToUseCapacity,
                          UErrorCode *status )
{
    if (U_FAILURE(*status)) {
        return -1;
    }
    if (preferredLanguages == NULL || availableLocalizations == NULL || localizationsToUse == NULL) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return -1;
    }
    // get resource data, need to protect with mutex
    if (icu::gMapDataState == 0) {
        umtx_initOnce(icu::gUALocaleCacheInitOnce, initializeMapData);
    }
    int32_t locsToUseCount = 0;
    int32_t prefLangIndex, availLocIndex = 0;
    int32_t availLocIndexBackup = -1; // if >= 0, contains index of backup match
    int32_t foundMatchPrefLangIndex = 0, backupMatchPrefLangIndex = 0;
    char (*availLocBase)[kLangScriptRegMaxLen + 1] = NULL;
    char (*availLocNorm)[kLangScriptRegMaxLen + 1] = NULL;
    UBool foundMatch = FALSE;
    UBool backupMatchPrefLang_pt_PT = FALSE;

#if DEBUG_UALOC
    if (preferredLanguagesCount > 0 && availableLocalizationsCount > 0) {
        printf("\n # ualoc_localizationsToUse start, preferredLanguages %d: %s, ..., availableLocalizations %d: %s, ...\n",
                 preferredLanguagesCount, preferredLanguages[0], availableLocalizationsCount, availableLocalizations[0]);
    } else {
        printf("\n # ualoc_localizationsToUse start, preferredLanguages %d: ..., availableLocalizations %d: ...\n",
                 preferredLanguagesCount, availableLocalizationsCount);
    }
#endif

    // Part 1, find the best matching localization, if any
    for (prefLangIndex = 0; prefLangIndex < preferredLanguagesCount; prefLangIndex++) {
        char prefLangBaseName[kLangScriptRegMaxLen + 1];
        char prefLangNormName[kLangScriptRegMaxLen + 1];
        char prefLangParentName[kLangScriptRegMaxLen + 1];
        UErrorCode tmpStatus = U_ZERO_ERROR;

        if (preferredLanguages[prefLangIndex] == NULL) {
            continue; // skip NULL preferredLanguages entry, go to next one
        }
        // use underscores, fix bad capitalization, delete any keywords
        uloc_getBaseName(preferredLanguages[prefLangIndex], prefLangBaseName, kLangScriptRegMaxLen, &tmpStatus);
        if (U_FAILURE(tmpStatus) || prefLangBaseName[0] == 0 ||
                uprv_strcmp(prefLangBaseName, "root") == 0 || prefLangBaseName[0] == '_') {
            continue; // can't handle this preferredLanguages entry or it is invalid, go to next one
        }
        prefLangBaseName[kLangScriptRegMaxLen] = 0; // ensure 0 termination, could have U_STRING_NOT_TERMINATED_WARNING
#if DEBUG_UALOC
        printf("  # loop: try prefLangBaseName %s\n", prefLangBaseName);
#endif

        // if we have not already allocated and filled the array of
        // base availableLocalizations, do so now.
        if (availLocBase == NULL) {
            availLocBase = (char (*)[kLangScriptRegMaxLen + 1])uprv_malloc(availableLocalizationsCount * (kLangScriptRegMaxLen + 1));
            if (availLocBase == NULL) {
                continue; // cannot further check this preferredLanguages entry, go to next one
            }
#if DEBUG_UALOC
            printf("   # allocate & fill availLocBase\n");
#endif
            for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                tmpStatus = U_ZERO_ERROR;
                if (availableLocalizations[availLocIndex] == NULL) {
                    availLocBase[availLocIndex][0] = 0; // effectively remove this entry
                    continue;
                }
                uloc_getBaseName(availableLocalizations[availLocIndex], availLocBase[availLocIndex], kLangScriptRegMaxLen, &tmpStatus);
                if (U_FAILURE(tmpStatus) || uprv_strcmp(availLocBase[availLocIndex], "root") == 0 || availLocBase[availLocIndex][0] == '_') {
                    availLocBase[availLocIndex][0] = 0; // effectively remove this entry
                    continue;
                }
                availLocBase[availLocIndex][kLangScriptRegMaxLen] = 0; // ensure 0 termination, could have U_STRING_NOT_TERMINATED_WARNING
#if DEBUG_UALOC
                printf("    # add availLocBase %s\n", availLocBase[availLocIndex]);
#endif
            }
        }
        // first compare base preferredLanguage to base versions of availableLocalizations names
        for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
            if (uprv_strcmp(prefLangBaseName, availLocBase[availLocIndex]) == 0) {
                foundMatch = TRUE; // availLocIndex records where
                foundMatchPrefLangIndex = prefLangIndex;
#if DEBUG_UALOC
                printf("   # FOUND: matched availLocBase %s -> actualLoc %s\n", availLocBase[availLocIndex], availableLocalizations[availLocIndex]);
#endif
                break;
            }
        }
        if (foundMatch) {
            break; // found a loc for this preferredLanguages entry
        }

        // get normalized preferredLanguage
        tmpStatus = U_ZERO_ERROR;
        ualoc_normalize(prefLangBaseName, prefLangNormName, kLangScriptRegMaxLen + 1, &tmpStatus);
        if (U_FAILURE(tmpStatus)) {
            continue; // can't handle this preferredLanguages entry, go to next one
        }
#if DEBUG_UALOC
        printf("   # prefLangNormName %s\n", prefLangNormName);
#endif
        // if we have not already allocated and filled the array of
        // normalized availableLocalizations, do so now.
        // Note: ualoc_normalize turns "zh_TW" into "zh_Hant_TW", zh_HK" into "zh_Hant_HK",
        // and fixes deprecated codes "iw" > "he", "in" > "id" etc.
        if (availLocNorm == NULL) {
            availLocNorm = (char (*)[kLangScriptRegMaxLen + 1])uprv_malloc(availableLocalizationsCount * (kLangScriptRegMaxLen + 1));
            if (availLocNorm == NULL) {
                continue; // cannot further check this preferredLanguages entry, go to next one
            }
#if DEBUG_UALOC
            printf("   # allocate & fill availLocNorm\n");
#endif
            for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                tmpStatus = U_ZERO_ERROR;
                ualoc_normalize(availLocBase[availLocIndex], availLocNorm[availLocIndex], kLangScriptRegMaxLen + 1, &tmpStatus);
                if (U_FAILURE(tmpStatus)) {
                    availLocNorm[availLocIndex][0] = 0; // effectively remove this entry
#if DEBUG_UALOC
                } else {
                    printf("   # actualLoc %-11s -> norm %s\n", availableLocalizations[availLocIndex], availLocNorm[availLocIndex]);
#endif
                }
            }
        }
        // now compare normalized preferredLanguage to normalized localization names
        // if matches, copy *original* localization name
        for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
            if (uprv_strcmp(prefLangNormName, availLocNorm[availLocIndex]) == 0) {
                foundMatch = TRUE; // availLocIndex records where
                foundMatchPrefLangIndex = prefLangIndex;
#if DEBUG_UALOC
                printf("   # FOUND: matched availLocNorm %s -> actualLoc %s\n", availLocNorm[availLocIndex], availableLocalizations[availLocIndex]);
#endif
                break;
            }
        }
        if (foundMatch) {
            break; // found a loc for this preferredLanguages entry
        }

        // now walk up the parent chain for preferredLanguage
        // until we find a match or hit root
        uprv_strcpy(prefLangBaseName, prefLangNormName);
        while (!foundMatch) {
            tmpStatus = U_ZERO_ERROR;
            ualoc_getParent(prefLangBaseName, prefLangParentName, kLangScriptRegMaxLen + 1, &tmpStatus);
            if (U_FAILURE(tmpStatus) || uprv_strcmp(prefLangParentName, "root") == 0 || prefLangParentName[0] == 0) {
                break; // reached root or cannot proceed further
            }
#if DEBUG_UALOC
            printf("   # prefLangParentName %s\n", prefLangParentName);
#endif

            // now compare this preferredLanguage parent to normalized localization names
            // if matches, copy *original* localization name
            for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                if (uprv_strcmp(prefLangParentName, availLocNorm[availLocIndex]) == 0) {
                    foundMatch = TRUE; // availLocIndex records where
                    foundMatchPrefLangIndex = prefLangIndex;
#if DEBUG_UALOC
                    printf("   # FOUND: matched availLocNorm %s -> actualLoc %s\n", availLocNorm[availLocIndex], availableLocalizations[availLocIndex]);
#endif
                    break;
                }
            }
            uprv_strcpy(prefLangBaseName, prefLangParentName);
        }
        if (foundMatch) {
            break; // found a loc for this preferredLanguages entry
        }

        // last try, use parents of selected language to try for backup match
        // if we have not already found one
        if (availLocIndexBackup < 0) {
            // now walk up the parent chain for preferredLanguage again
            // checking against parents of selected availLocNorm entries
            // but this time start with current prefLangNormName
            uprv_strcpy(prefLangBaseName, prefLangNormName);
            int32_t minDistance = kMaxParentDistance;
            while (TRUE) {
                // now compare this preferredLanguage to normalized localization names
                // parent if have one for this;  if matches, copy *original* localization name
#if DEBUG_UALOC
                printf("   # BACKUP: trying prefLangBaseName %s\n", prefLangBaseName);
#endif
                for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                    char availLocMinOrParent[kLangScriptRegMaxLen + 1];
                    int32_t distance;
                    // first check for special Apple parents of availLocNorm; the number
                    // of locales with such parents is small.
                    // If no such parent, or if parent has an intermediate numeric region,
                    // then try stripping the original region.
                    int32_t availLocParentLen = 0;
                    const char *availLocParent = getLocParent(availLocNorm[availLocIndex], &distance);
                    if (availLocParent) {
#if DEBUG_UALOC
                        printf("    # availLocAppleParentName %s\n", availLocParent);
#endif
                        if (uprv_strcmp(prefLangBaseName, availLocParent) == 0 && distance < minDistance) {
                            availLocIndexBackup = availLocIndex; // records where the match occurred
                            backupMatchPrefLangIndex = prefLangIndex;
                            minDistance = distance;
#if DEBUG_UALOC
                            printf("    # BACKUP: LocAppleParent matched prefLangNormName with distance %d\n", distance);
#endif
                            continue;
                        }
                        availLocParentLen = uprv_strlen(availLocParent);
                    }
                    if (minDistance <= 1) {
                        continue; // we can't get any closer in the rest of this iteration
                    }
                    if (availLocParent == NULL || (availLocParentLen >= 6 && isdigit(availLocParent[availLocParentLen-1]))) {
                        tmpStatus = U_ZERO_ERROR;
                        int32_t regLen = uloc_getCountry(availLocNorm[availLocIndex], availLocMinOrParent, kLangScriptRegMaxLen, &tmpStatus);
                        if (U_SUCCESS(tmpStatus) && regLen > 1) {
                            uloc_addLikelySubtags(availLocNorm[availLocIndex], availLocMinOrParent, kLangScriptRegMaxLen, &tmpStatus);
                            if (U_SUCCESS(tmpStatus)) {
                                availLocMinOrParent[kLangScriptRegMaxLen] = 0; // ensure 0 termination, could have U_STRING_NOT_TERMINATED_WARNING
#if DEBUG_UALOC
                                printf("    # availLocRegMaxName %s\n", availLocMinOrParent);
#endif
                                char availLocTemp[kLangScriptRegMaxLen + 1];
                                uloc_getParent(availLocMinOrParent, availLocTemp, kLangScriptRegMaxLen, &tmpStatus);
                                if (U_SUCCESS(tmpStatus)) {
                                    availLocTemp[kLangScriptRegMaxLen] = 0;
                                    uloc_minimizeSubtags(availLocTemp, availLocMinOrParent, kLangScriptRegMaxLen, &tmpStatus);
                                    if (U_SUCCESS(tmpStatus)) {
                                        availLocMinOrParent[kLangScriptRegMaxLen] = 0; 
#if DEBUG_UALOC
                                        printf("    # availLocNoRegParentName %s\n", availLocMinOrParent);
#endif
                                        if (uprv_strcmp(prefLangBaseName, availLocMinOrParent) == 0) {
                                            availLocIndexBackup = availLocIndex; // records where the match occurred
                                            backupMatchPrefLangIndex = prefLangIndex;
                                            minDistance = 1;
                                            backupMatchPrefLang_pt_PT = (uprv_strcmp(prefLangNormName, "pt_PT") == 0);
#if DEBUG_UALOC
                                            printf("    # BACKUP: LocNoRegParent matched prefLangNormName with distance 1\n");
#endif
                                            continue;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    // then check against minimized version of availLocNorm
                    tmpStatus = U_ZERO_ERROR;
                    uloc_minimizeSubtags(availLocNorm[availLocIndex], availLocMinOrParent, kLangScriptRegMaxLen, &tmpStatus);
                    if (U_FAILURE(tmpStatus)) {
                        continue;
                    }
                    availLocMinOrParent[kLangScriptRegMaxLen] = 0; // ensure 0 termination, could have U_STRING_NOT_TERMINATED_WARNING
#if DEBUG_UALOC
                    printf("    # availLocMinimized %s\n", availLocMinOrParent);
#endif
                    if (uprv_strcmp(prefLangBaseName, availLocMinOrParent) == 0) {
                        availLocIndexBackup = availLocIndex; // records where the match occurred
                        backupMatchPrefLangIndex = prefLangIndex;
                        minDistance = 1;
#if DEBUG_UALOC
                        printf("    # BACKUP: LocMinimized matched prefLangNormName with distance 1\n");
#endif
                    }
                }
                if (availLocIndexBackup >= 0) {
                    break;
                }
                tmpStatus = U_ZERO_ERROR;
                ualoc_getParent(prefLangBaseName, prefLangParentName, kLangScriptRegMaxLen + 1, &tmpStatus);
                if (U_FAILURE(tmpStatus) || uprv_strcmp(prefLangParentName, "root") == 0 || prefLangParentName[0] == 0) {
                    break; // reached root or cannot proceed further
                }
                uprv_strcpy(prefLangBaseName, prefLangParentName);
            }
        }
    }
    // If we have a backup match, decide what to do
    if (availLocIndexBackup >= 0) {
        if (!foundMatch) {
            // no main match, just use the backup
            availLocIndex = availLocIndexBackup;
            foundMatch = TRUE;
#if DEBUG_UALOC
            printf(" # no main match, have backup => use availLocIndexBackup %d\n", availLocIndexBackup);
#endif
        } else if (backupMatchPrefLangIndex < foundMatchPrefLangIndex && (!backupMatchPrefLang_pt_PT || uprv_strcmp(availLocNorm[availLocIndexBackup], "pt_BR") != 0)) {
            // have a main match but backup match was higher in the prefs, use it if for a different language
#if DEBUG_UALOC
            printf(" # have backup match higher in prefs, comparing its language and script to main match\n");
#endif
            char mainLang[ULOC_LANG_CAPACITY + 1];
            char backupLang[ULOC_LANG_CAPACITY + 1];
            UErrorCode tmpStatus = U_ZERO_ERROR;
            uloc_getLanguage(availLocNorm[availLocIndex], mainLang, ULOC_LANG_CAPACITY, &tmpStatus);
            mainLang[ULOC_LANG_CAPACITY] = 0; // ensure zero termination
            uloc_getLanguage(availLocNorm[availLocIndexBackup], backupLang, ULOC_LANG_CAPACITY, &tmpStatus);
            backupLang[ULOC_LANG_CAPACITY] = 0; // ensure zero termination
            if (U_SUCCESS(tmpStatus)) {
                if (uprv_strncmp(mainLang, backupLang, ULOC_LANG_CAPACITY) != 0) {
                    // backup match has different language than main match
                    availLocIndex = availLocIndexBackup;
                    // foundMatch is already TRUE
#if DEBUG_UALOC
                    printf(" # main match but backup is for a different lang higher in prefs => use availLocIndexBackup %d\n", availLocIndexBackup);
#endif
                } else {
                    // backup match has same language as main match, check scripts too
                    char availLocMaximized[kLangScriptRegMaxLen + 1];

                    uloc_addLikelySubtags(availLocNorm[availLocIndex], availLocMaximized, kLangScriptRegMaxLen, &tmpStatus);
                    availLocMaximized[kLangScriptRegMaxLen] = 0;
                    uloc_getScript(availLocMaximized, mainLang, ULOC_LANG_CAPACITY, &tmpStatus);
                    mainLang[ULOC_LANG_CAPACITY] = 0;
 
                    uloc_addLikelySubtags(availLocNorm[availLocIndexBackup], availLocMaximized, kLangScriptRegMaxLen, &tmpStatus);
                    availLocMaximized[kLangScriptRegMaxLen] = 0;
                    uloc_getScript(availLocMaximized, backupLang, ULOC_LANG_CAPACITY, &tmpStatus);
                    backupLang[ULOC_LANG_CAPACITY] = 0;

                    if (U_SUCCESS(tmpStatus) && uprv_strncmp(mainLang, backupLang, ULOC_LANG_CAPACITY) != 0) {
                        // backup match has different script than main match
                        availLocIndex = availLocIndexBackup;
                        // foundMatch is already TRUE
#if DEBUG_UALOC
                        printf(" # main match but backup is for a different script higher in prefs => use availLocIndexBackup %d\n", availLocIndexBackup);
#endif
                    }
                 }
            }
        }
    }

    // Part 2, if we found a matching localization, then walk up its parent tree to find any fallback matches in availableLocalizations
    if (foundMatch) {
        // Here availLocIndex corresponds to the first matched localization
        UErrorCode tmpStatus = U_ZERO_ERROR;
        int32_t availLocMatchIndex = availLocIndex;
        if (locsToUseCount < localizationsToUseCapacity) {
            localizationsToUse[locsToUseCount++] = availableLocalizations[availLocMatchIndex];
        }
        // at this point we must have availLocBase, and minimally matched against that.
        // if we have not already allocated and filled the array of
        // normalized availableLocalizations, do so now, but don't require it
        if (availLocNorm == NULL) {
            availLocNorm = (char (*)[kLangScriptRegMaxLen + 1])uprv_malloc(availableLocalizationsCount * (kLangScriptRegMaxLen + 1));
            if (availLocNorm != NULL) {
                for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                    tmpStatus = U_ZERO_ERROR;
                    ualoc_normalize(availLocBase[availLocIndex], availLocNorm[availLocIndex], kLangScriptRegMaxLen + 1, &tmpStatus);
                    if (U_FAILURE(tmpStatus)) {
                        availLocNorm[availLocIndex][0] = 0; // effectively remove this entry
                    }
                }
            }
        }

       // add normalized form of matching loc, if different and in availLocBase
        if (locsToUseCount < localizationsToUseCapacity) {
            tmpStatus = U_ZERO_ERROR;
            char matchedLocNormName[kLangScriptRegMaxLen + 1];
            char matchedLocParentName[kLangScriptRegMaxLen + 1];
            // get normalized form of matching loc
            if (availLocNorm != NULL) {
                uprv_strcpy(matchedLocNormName, availLocNorm[availLocMatchIndex]);
            } else {
                ualoc_normalize(availLocBase[availLocMatchIndex], matchedLocNormName, kLangScriptRegMaxLen + 1, &tmpStatus);
            }
            if (U_SUCCESS(tmpStatus)) {
                // add normalized form of matching loc, if different and in availLocBase
                if (uprv_strcmp(matchedLocNormName, localizationsToUse[0]) != 0) {
                    // normalization of matched localization is different, see if we have the normalization in availableLocalizations
                    // from this point on, availLocIndex no longer corresponds to the matched localization.
                    for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                        if ( (uprv_strcmp(matchedLocNormName, availLocBase[availLocIndex]) == 0
                                || (availLocNorm != NULL && uprv_strcmp(matchedLocNormName, availLocNorm[availLocIndex]) == 0))
                                && !locInArray(localizationsToUse, locsToUseCount, availableLocalizations[availLocIndex])) {
                            localizationsToUse[locsToUseCount++] = availableLocalizations[availLocIndex];
                            break;
                        }
                    }
                }

                // now walk up the parent chain from matchedLocNormName, adding parents if they are in availLocBase
                while (locsToUseCount < localizationsToUseCapacity) {
                    ualoc_getParent(matchedLocNormName, matchedLocParentName, kLangScriptRegMaxLen + 1, &tmpStatus);
                    if (U_FAILURE(tmpStatus) || uprv_strcmp(matchedLocParentName, "root") == 0 || matchedLocParentName[0] == 0) {
                        break; // reached root or cannot proceed further
                    }

                    // now compare this matchedLocParentName parent to base localization names (and norm ones if we have them)
                    for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                        if ( (uprv_strcmp(matchedLocParentName, availLocBase[availLocIndex]) == 0
                                || (availLocNorm != NULL && uprv_strcmp(matchedLocParentName, availLocNorm[availLocIndex]) == 0))
                                && !locInArray(localizationsToUse, locsToUseCount, availableLocalizations[availLocIndex])) {
                            localizationsToUse[locsToUseCount++] = availableLocalizations[availLocIndex];
                            break;
                        }
                    }
                    uprv_strcpy(matchedLocNormName, matchedLocParentName);
                }

                // The above still fails to include "zh_TW" if it is in availLocBase and the matched localization
                // base name is "zh_HK" or "zh_MO". One option would be to walk up the parent chain from
                // matchedLocNormName again, comparing against parents of of selected availLocNorm entries.
                // But this picks up too many matches that are not parents of the matched localization. So
                // we just handle these specially.
                if ( locsToUseCount < localizationsToUseCapacity
                        && (uprv_strcmp(availLocBase[availLocMatchIndex], "zh_HK") == 0
                        || uprv_strcmp(availLocBase[availLocMatchIndex], "zh_MO") == 0) ) {
                    int32_t zhTW_matchIndex = -1;
                    UBool zhHant_found = FALSE;
                    for (availLocIndex = 0; availLocIndex < availableLocalizationsCount; availLocIndex++) {
                        if ( zhTW_matchIndex < 0 && uprv_strcmp("zh_TW", availLocBase[availLocIndex]) == 0 ) {
                            zhTW_matchIndex = availLocIndex;
                        }
                        if ( !zhHant_found && uprv_strcmp("zh_Hant", availLocBase[availLocIndex]) == 0 ) {
                            zhHant_found = TRUE;
                        }
                    }
                    if (zhTW_matchIndex >= 0 && !zhHant_found
                            && !locInArray(localizationsToUse, locsToUseCount, availableLocalizations[zhTW_matchIndex])) {
                        localizationsToUse[locsToUseCount++] = availableLocalizations[zhTW_matchIndex];
                    }
                }
            }
        }
    }

    uprv_free(availLocNorm);
    uprv_free(availLocBase);
    return locsToUseCount;
}

