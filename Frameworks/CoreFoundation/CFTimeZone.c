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

/*	CFTimeZone.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/


#include <CoreFoundation/CFTimeZone.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFDateFormatter.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <math.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unicode/ucal.h>
#include <CoreFoundation/CFDateFormatter.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_LINUX
#include <dirent.h>
#include <unistd.h>
#include <sys/fcntl.h>
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <tzfile.h>
#elif DEPLOYMENT_TARGET_LINUX
#ifndef TZDIR
#define TZDIR	"/usr/share/zoneinfo" /* Time zone object file directory */
#endif /* !defined TZDIR */

#ifndef TZDEFAULT
#define TZDEFAULT	"/etc/localtime"
#endif /* !defined TZDEFAULT */

struct tzhead {
    char	tzh_magic[4];		/* TZ_MAGIC */
    char	tzh_reserved[16];	/* reserved for future use */
    char	tzh_ttisgmtcnt[4];	/* coded number of trans. time flags */
    char	tzh_ttisstdcnt[4];	/* coded number of trans. time flags */
    char	tzh_leapcnt[4];		/* coded number of leap seconds */
    char	tzh_timecnt[4];		/* coded number of transition times */
    char	tzh_typecnt[4];		/* coded number of local time types */
    char	tzh_charcnt[4];		/* coded number of abbr. chars */
};
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_LINUX
#define TZZONELINK	TZDEFAULT
#define TZZONEINFO	TZDIR "/"
#elif DEPLOYMENT_TARGET_WINDOWS
static CFStringRef __tzZoneInfo = NULL;
static char *__tzDir = NULL;
static void __InitTZStrings(void);
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

#if DEPLOYMENT_TARGET_LINUX
// Symbol aliases
CF_EXPORT CFStringRef const kCFDateFormatterTimeZone __attribute__((weak, alias ("kCFDateFormatterTimeZoneKey")));
#endif

CONST_STRING_DECL(kCFTimeZoneSystemTimeZoneDidChangeNotification, "kCFTimeZoneSystemTimeZoneDidChangeNotification")

static CFTimeZoneRef __CFTimeZoneSystem = NULL;
static CFTimeZoneRef __CFTimeZoneDefault = NULL;
static CFDictionaryRef __CFTimeZoneAbbreviationDict = NULL;
static CFLock_t __CFTimeZoneAbbreviationLock = CFLockInit;
static CFMutableDictionaryRef __CFTimeZoneCompatibilityMappingDict = NULL;
static CFLock_t __CFTimeZoneCompatibilityMappingLock = CFLockInit;
static CFArrayRef __CFKnownTimeZoneList = NULL;
static CFMutableDictionaryRef __CFTimeZoneCache = NULL;
static CFLock_t __CFTimeZoneGlobalLock = CFLockInit;

#if DEPLOYMENT_TARGET_WINDOWS
static CFDictionaryRef __CFTimeZoneWinToOlsonDict = NULL;
static CFLock_t __CFTimeZoneWinToOlsonLock = CFLockInit;
#endif

CF_INLINE void __CFTimeZoneLockGlobal(void) {
    __CFLock(&__CFTimeZoneGlobalLock);
}

CF_INLINE void __CFTimeZoneUnlockGlobal(void) {
    __CFUnlock(&__CFTimeZoneGlobalLock);
}

CF_INLINE void __CFTimeZoneLockAbbreviations(void) {
    __CFLock(&__CFTimeZoneAbbreviationLock);
}

CF_INLINE void __CFTimeZoneUnlockAbbreviations(void) {
    __CFUnlock(&__CFTimeZoneAbbreviationLock);
}

CF_INLINE void __CFTimeZoneLockCompatibilityMapping(void) {
    __CFLock(&__CFTimeZoneCompatibilityMappingLock);
}

CF_INLINE void __CFTimeZoneUnlockCompatibilityMapping(void) {
    __CFUnlock(&__CFTimeZoneCompatibilityMappingLock);
}

#if DEPLOYMENT_TARGET_WINDOWS
/* This function should be used for WIN32 instead of
 * __CFCopyRecursiveDirectoryList function.
 * It takes TimeZone names from the registry
 * (Aleksey Dukhnyakov)
 */
static CFMutableArrayRef __CFCopyWindowsTimeZoneList() {
    CFMutableArrayRef result = NULL;
    HKEY hkResult;
    TCHAR lpName[MAX_PATH+1];
    DWORD dwIndex, retCode;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,_T(TZZONEINFO),&hkResult) !=
        ERROR_SUCCESS )
        return NULL;

    result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    for (dwIndex=0; (retCode = RegEnumKey(hkResult,dwIndex,lpName,MAX_PATH)) != ERROR_NO_MORE_ITEMS ; dwIndex++) {
        if (retCode != ERROR_SUCCESS) {
            RegCloseKey(hkResult);
            CFRelease(result);
            return NULL;
        } else {
#if defined(UNICODE)
	    CFStringRef string = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const UInt8 *)lpName, (_tcslen(lpName) * sizeof(UniChar)), kCFStringEncodingUnicode, false);
#else
	    CFStringRef string = CFStringCreateWithBytes(kCFAllocatorSystemDefault, lpName, _tcslen(lpName), CFStringGetSystemEncoding(), false);
#endif
	    CFArrayAppendValue(result, string);
	    CFRelease(string);
        }
    }

    RegCloseKey(hkResult);
    return result;
}
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
static CFMutableArrayRef __CFCopyRecursiveDirectoryList() {
    CFMutableArrayRef result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
#if DEPLOYMENT_TARGET_WINDOWS
    if (!__tzDir) __InitTZStrings();
    if (!__tzDir) return result;
    int fd = open(__tzDir, O_RDONLY);
#else
    int fd = open(TZDIR "/zone.tab", O_RDONLY);
#endif

    for (; 0 <= fd;) {
        uint8_t buffer[4096];
        ssize_t len = read(fd, buffer, sizeof(buffer));
        if (len <= 0) break;
	if (len < sizeof(buffer)) {
	    // assumes that partial read only occurs at the end of the file
	    buffer[len] = '\n';
	    len++;
	}
        const uint8_t *bytes = buffer;
        for (;;) {
	    const uint8_t *nextl = memchr(bytes, '\n', len);
	    if (!nextl) break;
	    nextl++;
	    if ('#' == *bytes) {
		len -= (nextl - bytes);
		bytes = nextl;
		continue;
	    }
	    const uint8_t *tab1 = memchr(bytes, '\t', (nextl - bytes));
	    if (!tab1) {
		len -= (nextl - bytes);
		bytes = nextl;
		continue;
	    }
	    tab1++;
	    len -= (tab1 - bytes);
	    bytes = tab1; 
	    const uint8_t *tab2 = memchr(bytes, '\t', (nextl - bytes));
	    if (!tab2) {
		len -= (nextl - bytes);
		bytes = nextl;
		continue;
	    }
	    tab2++;
	    len -= (tab2 - bytes);
	    bytes = tab2; 
	    const uint8_t *tab3 = memchr(bytes, '\t', (nextl - bytes));
	    int nmlen = tab3 ? (tab3 - bytes) : (nextl - 1 - bytes);
	    CFStringRef string = CFStringCreateWithBytes(kCFAllocatorSystemDefault, bytes, nmlen, kCFStringEncodingUTF8, false);
	    CFArrayAppendValue(result, string);
	    CFRelease(string);
	    len -= (nextl - bytes);
	    bytes = nextl;
        }
        lseek(fd, -len, SEEK_CUR);
    }
    close(fd);
    return result;
}
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

typedef struct _CFTZPeriod {
    int32_t startSec;
    CFStringRef abbrev;
    uint32_t info;
} CFTZPeriod;

struct __CFTimeZone {
    CFRuntimeBase _base;
    CFStringRef _name;		/* immutable */
    CFDataRef _data;		/* immutable */
    CFTZPeriod *_periods;	/* immutable */
    int32_t _periodCnt;		/* immutable */
};

/* startSec is the whole integer seconds from a CFAbsoluteTime, giving dates
 * between 1933 and 2069; info outside these years is discarded on read-in */
/* Bits 31-18 of the info are unused */
/* Bit 17 of the info is used for the is-DST state */
/* Bit 16 of the info is used for the sign of the offset (1 == negative) */
/* Bits 15-0 of the info are used for abs(offset) in seconds from GMT */

CF_INLINE void __CFTZPeriodInit(CFTZPeriod *period, int32_t startTime, CFStringRef abbrev, int32_t offset, Boolean isDST) {
    period->startSec = startTime;
    period->abbrev = abbrev ? (CFStringRef)CFRetain(abbrev) : NULL;
    __CFBitfieldSetValue(period->info, 15, 0, abs(offset));
    __CFBitfieldSetValue(period->info, 16, 16, (offset < 0 ? 1 : 0));
    __CFBitfieldSetValue(period->info, 17, 17, (isDST ? 1 : 0));
}

CF_INLINE int32_t __CFTZPeriodStartSeconds(const CFTZPeriod *period) {
    return period->startSec;
}

CF_INLINE CFStringRef __CFTZPeriodAbbreviation(const CFTZPeriod *period) {
    return period->abbrev;
}

CF_INLINE int32_t __CFTZPeriodGMTOffset(const CFTZPeriod *period) {
    int32_t v = __CFBitfieldGetValue(period->info, 15, 0);
    if (__CFBitfieldGetValue(period->info, 16, 16)) v = -v;
    return v;
}

CF_INLINE Boolean __CFTZPeriodIsDST(const CFTZPeriod *period) {
    return (Boolean)__CFBitfieldGetValue(period->info, 17, 17);
}

static CFComparisonResult __CFCompareTZPeriods(const void *val1, const void *val2, void *context) {
    CFTZPeriod *tzp1 = (CFTZPeriod *)val1;
    CFTZPeriod *tzp2 = (CFTZPeriod *)val2;
    // we treat equal as less than, as the code which uses the
    // result of the bsearch doesn't expect exact matches
    // (they're pretty rare, so no point in over-coding for them)
    if (__CFTZPeriodStartSeconds(tzp1) <= __CFTZPeriodStartSeconds(tzp2)) return kCFCompareLessThan;
    return kCFCompareGreaterThan;
}

static CFIndex __CFBSearchTZPeriods(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CFTZPeriod elem;
    __CFTZPeriodInit(&elem, (int32_t)floor(at + 1.0), NULL, 0, false);
    CFIndex idx = CFBSearch(&elem, sizeof(CFTZPeriod), tz->_periods, tz->_periodCnt, __CFCompareTZPeriods, NULL);
    if (tz->_periodCnt <= idx) {
	idx = tz->_periodCnt;
    } else if (0 == idx) {
	idx = 1;
    }
    return idx - 1;
}


CF_INLINE int32_t __CFDetzcode(const unsigned char *bufp) {
    int32_t result = (bufp[0] & 0x80) ? ~0L : 0L;
    result = (result << 8) | (bufp[0] & 0xff);
    result = (result << 8) | (bufp[1] & 0xff);
    result = (result << 8) | (bufp[2] & 0xff);
    result = (result << 8) | (bufp[3] & 0xff);
    return result;
}

CF_INLINE void __CFEntzcode(int32_t value, unsigned char *bufp) {
    bufp[0] = (value >> 24) & 0xff;
    bufp[1] = (value >> 16) & 0xff;
    bufp[2] = (value >> 8) & 0xff;
    bufp[3] = (value >> 0) & 0xff;
}

static Boolean __CFParseTimeZoneData(CFAllocatorRef allocator, CFDataRef data, CFTZPeriod **tzpp, CFIndex *cntp) {
    int32_t len, timecnt, typecnt, charcnt, idx, cnt;
    const uint8_t *p, *timep, *typep, *ttisp, *charp;
    CFStringRef *abbrs;
    Boolean result = true;

    p = CFDataGetBytePtr(data);
    len = CFDataGetLength(data);
    if (len < (int32_t)sizeof(struct tzhead)) {
	return false;
    }
    
    if (!(p[0] == 'T' && p[1] == 'Z' && p[2] == 'i' && p[3] == 'f')) return false;  /* Don't parse without TZif at head of file */
   
    p += 20 + 4 + 4 + 4;	/* skip reserved, ttisgmtcnt, ttisstdcnt, leapcnt */
    timecnt = __CFDetzcode(p);
    p += 4;
    typecnt = __CFDetzcode(p);
    p += 4;
    charcnt = __CFDetzcode(p);
    p += 4;
    if (typecnt <= 0 || timecnt < 0 || charcnt < 0) {
	return false;
    }
    if (1024 < timecnt || 32 < typecnt || 128 < charcnt) {
	// reject excessive timezones to avoid arithmetic overflows for
	// security reasons and to reject potentially corrupt files
	return false;
    }
    if (len - (int32_t)sizeof(struct tzhead) < (4 + 1) * timecnt + (4 + 1 + 1) * typecnt + charcnt) {
	return false;
    }
    timep = p;
    typep = timep + 4 * timecnt;
    ttisp = typep + timecnt;
    charp = ttisp + (4 + 1 + 1) * typecnt;
    cnt = (0 < timecnt) ? timecnt : 1;
    *tzpp = CFAllocatorAllocate(allocator, cnt * sizeof(CFTZPeriod), 0);
    if (__CFOASafe) __CFSetLastAllocationEventName(*tzpp, "CFTimeZone (store)");
    memset(*tzpp, 0, cnt * sizeof(CFTZPeriod));
    abbrs = CFAllocatorAllocate(allocator, (charcnt + 1) * sizeof(CFStringRef), 0);
    if (__CFOASafe) __CFSetLastAllocationEventName(abbrs, "CFTimeZone (temp)");
    for (idx = 0; idx < charcnt + 1; idx++) {
	abbrs[idx] = NULL;
    }
    for (idx = 0; idx < cnt; idx++) {
	CFAbsoluteTime at;
	int32_t itime, offset;
	uint8_t type, dst, abbridx;

	at = (CFAbsoluteTime)(__CFDetzcode(timep) + 0.0) - kCFAbsoluteTimeIntervalSince1970;
	if (0 == timecnt) itime = INT_MIN;
	else if (at < (CFAbsoluteTime)INT_MIN) itime = INT_MIN;
	else if ((CFAbsoluteTime)INT_MAX < at) itime = INT_MAX;
	else itime = (int32_t)at;
	timep += 4;	/* harmless if 0 == timecnt */
	type = (0 < timecnt) ? (uint8_t)*typep++ : 0;
	if (typecnt <= type) {
	    result = false;
	    break;
	}
	offset = __CFDetzcode(ttisp + 6 * type);
	dst = (uint8_t)*(ttisp + 6 * type + 4);
	if (0 != dst && 1 != dst) {
	    result = false;
	    break;
	}
	abbridx = (uint8_t)*(ttisp + 6 * type + 5);
	if (charcnt < abbridx) {
	    result = false;
	    break;
	}
	if (NULL == abbrs[abbridx]) {
	    abbrs[abbridx] = CFStringCreateWithCString(allocator, (char *)&charp[abbridx], kCFStringEncodingASCII);
	}
	__CFTZPeriodInit(*tzpp + idx, itime, abbrs[abbridx], offset, (dst ? true : false));
    }
    for (idx = 0; idx < charcnt + 1; idx++) {
	if (NULL != abbrs[idx]) {
	    CFRelease(abbrs[idx]);
	}
    }
    CFAllocatorDeallocate(allocator, abbrs);
    if (result) {
	// dump all but the last INT_MIN and the first INT_MAX
	for (idx = 0; idx < cnt; idx++) {
	    if (((*tzpp + idx)->startSec == INT_MIN) && (idx + 1 < cnt) && (((*tzpp + idx + 1)->startSec == INT_MIN))) {
		if (NULL != (*tzpp + idx)->abbrev) CFRelease((*tzpp + idx)->abbrev);
		cnt--;
		memmove((*tzpp + idx), (*tzpp + idx + 1), sizeof(CFTZPeriod) * (cnt - idx));
		idx--;
	    }
	}
	// Don't combine these loops!  Watch the idx decrementing...
	for (idx = 0; idx < cnt; idx++) {
	    if (((*tzpp + idx)->startSec == INT_MAX) && (0 < idx) && (((*tzpp + idx - 1)->startSec == INT_MAX))) {
		if (NULL != (*tzpp + idx)->abbrev) CFRelease((*tzpp + idx)->abbrev);
		cnt--;
		memmove((*tzpp + idx), (*tzpp + idx + 1), sizeof(CFTZPeriod) * (cnt - idx));
		idx--;
	    }
	}
	CFQSortArray(*tzpp, cnt, sizeof(CFTZPeriod), __CFCompareTZPeriods, NULL);
	// if the first period is in DST and there is more than one period, drop it
	if (1 < cnt && __CFTZPeriodIsDST(*tzpp + 0)) {
	    if (NULL != (*tzpp + 0)->abbrev) CFRelease((*tzpp + 0)->abbrev);
	    cnt--;
	    memmove((*tzpp + 0), (*tzpp + 0 + 1), sizeof(CFTZPeriod) * (cnt - 0));
	}
	*cntp = cnt;
    } else {
	CFAllocatorDeallocate(allocator, *tzpp);
	*tzpp = NULL;
    }
    return result;
}

static Boolean __CFTimeZoneEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFTimeZoneRef tz1 = (CFTimeZoneRef)cf1;
    CFTimeZoneRef tz2 = (CFTimeZoneRef)cf2;
    if (!CFEqual(CFTimeZoneGetName(tz1), CFTimeZoneGetName(tz2))) return false;
    if (!CFEqual(CFTimeZoneGetData(tz1), CFTimeZoneGetData(tz2))) return false;
    return true;
}

static CFHashCode __CFTimeZoneHash(CFTypeRef cf) {
    CFTimeZoneRef tz = (CFTimeZoneRef)cf;
    return CFHash(CFTimeZoneGetName(tz));
}

static CFStringRef __CFTimeZoneCopyDescription(CFTypeRef cf) {
    CFTimeZoneRef tz = (CFTimeZoneRef)cf;
    CFStringRef result, abbrev;
    CFAbsoluteTime at;
    at = CFAbsoluteTimeGetCurrent();
    abbrev = CFTimeZoneCopyAbbreviation(tz, at);
    result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFTimeZone %p [%p]>{name = %@; abbreviation = %@; GMT offset = %g; is DST = %s}"), cf, CFGetAllocator(tz), tz->_name, abbrev, CFTimeZoneGetSecondsFromGMT(tz, at), CFTimeZoneIsDaylightSavingTime(tz, at) ? "true" : "false");
    CFRelease(abbrev);
    return result;
}

static void __CFTimeZoneDeallocate(CFTypeRef cf) {
    CFTimeZoneRef tz = (CFTimeZoneRef)cf;
    CFAllocatorRef allocator = CFGetAllocator(tz);
    CFIndex idx;
    if (tz->_name) CFRelease(tz->_name);
    if (tz->_data) CFRelease(tz->_data);
    for (idx = 0; idx < tz->_periodCnt; idx++) {
	if (NULL != tz->_periods[idx].abbrev) CFRelease(tz->_periods[idx].abbrev);
    }
    if (NULL != tz->_periods) CFAllocatorDeallocate(allocator, tz->_periods);
}

static CFTypeID __kCFTimeZoneTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFTimeZoneClass = {
    0,
    "CFTimeZone",
    NULL,	// init
    NULL,	// copy
    __CFTimeZoneDeallocate,
    __CFTimeZoneEqual,
    __CFTimeZoneHash,
    NULL,	//
    __CFTimeZoneCopyDescription
};

CFTypeID CFTimeZoneGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFTimeZoneTypeID = _CFRuntimeRegisterClass(&__CFTimeZoneClass); });
    return __kCFTimeZoneTypeID;
}

#if DEPLOYMENT_TARGET_WINDOWS
static const char *__CFTimeZoneWinToOlsonDefaults =
/* Mappings to time zones in Windows Registry are best-guess */
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
" <!DOCTYPE plist SYSTEM \"file://localhost/System/Library/DTDs/PropertyList.dtd\">"
" <plist version=\"1.0\">"
" <dict>"
"    <key>Afghanistan</key>                 <string>Asia/Kabul</string>"
"    <key>Afghanistan Standard Time</key>   <string>Asia/Kabul</string>"
"    <key>Alaskan</key>                     <string>America/Anchorage</string>"
"    <key>Alaskan Standard Time</key>       <string>America/Anchorage</string>"
"    <key>Arab</key>                        <string>Asia/Riyadh</string>"
"    <key>Arab Standard Time</key>          <string>Asia/Riyadh</string>"
"    <key>Arabian</key>                     <string>Asia/Muscat</string>"
"    <key>Arabian Standard Time</key>       <string>Asia/Muscat</string>"
"    <key>Arabic Standard Time</key>        <string>Asia/Baghdad</string>"
"    <key>Atlantic</key>                    <string>America/Halifax</string>"
"    <key>Atlantic Standard Time</key>      <string>America/Halifax</string>"
"    <key>AUS Central</key>                 <string>Australia/Darwin</string>"
"    <key>AUS Central Standard Time</key>	<string>Australia/Darwin</string>"
"    <key>AUS Eastern</key>                 <string>Australia/Sydney</string>"
"    <key>AUS Eastern Standard Time</key>	<string>Australia/Sydney</string>"
"    <key>Azerbaijan Standard Time</key>	<string>Asia/Baku</string>"
"    <key>Azores</key>                      <string>Atlantic/Azores</string>"
"    <key>Azores Standard Time</key>        <string>Atlantic/Azores</string>"
"    <key>Bangkok</key>                     <string>Asia/Bangkok</string>"
"    <key>Bangkok Standard Time</key>       <string>Asia/Bangkok</string>"
"    <key>Beijing</key>                     <string>Asia/Shanghai</string>"
"    <key>Canada Central</key>              <string>America/Regina</string>"
"    <key>Canada Central Standard Time</key> <string>America/Regina</string>"
"    <key>Cape Verde Standard Time</key>    <string>Atlantic/Cape_Verde</string>"
"    <key>Caucasus</key>                    <string>Asia/Yerevan</string>"
"    <key>Caucasus Standard Time</key>      <string>Asia/Yerevan</string>"
"    <key>Cen. Australia</key>              <string>Australia/Adelaide</string>"
"    <key>Cen. Australia Standard Time</key> <string>Australia/Adelaide</string>"
"    <key>Central</key>                     <string>America/Chicago</string>"
"    <key>Central America Standard Time</key> <string>America/Regina</string>"
"    <key>Central Asia</key>                <string>Asia/Dhaka</string>"
"    <key>Central Asia Standard Time</key>	<string>Asia/Dhaka</string>"
"    <key>Central Brazilian Standard Time</key>	<string>America/Manaus</string>"
"    <key>Central Europe</key>              <string>Europe/Prague</string>"
"    <key>Central Europe Standard Time</key> <string>Europe/Prague</string>"
"    <key>Central European</key>            <string>Europe/Belgrade</string>"
"    <key>Central European Standard Time</key>	<string>Europe/Belgrade</string>"
"    <key>Central Pacific</key>             <string>Pacific/Guadalcanal</string>"
"    <key>Central Pacific Standard Time</key>	<string>Pacific/Guadalcanal</string>"
"    <key>Central Standard Time</key>       <string>America/Chicago</string>"
"    <key>Central Standard Time (Mexico)</key> <string>America/Mexico_City</string>"
"    <key>China</key>                       <string>Asia/Shanghai</string>"
"    <key>China Standard Time</key>         <string>Asia/Shanghai</string>"
"    <key>Dateline</key>                    <string>GMT-1200</string>"
"    <key>Dateline Standard Time</key>      <string>GMT-1200</string>"
"    <key>E. Africa</key>                   <string>Africa/Nairobi</string>"
"    <key>E. Africa Standard Time</key>     <string>Africa/Nairobi</string>"
"    <key>E. Australia</key>                <string>Australia/Brisbane</string>"
"    <key>E. Australia Standard Time</key>	<string>Australia/Brisbane</string>"
"    <key>E. Europe</key>                   <string>Europe/Minsk</string>"
"    <key>E. Europe Standard Time</key>     <string>Europe/Minsk</string>"
"    <key>E. South America</key>            <string>America/Sao_Paulo</string>"
"    <key>E. South America Standard Time</key>	<string>America/Sao_Paulo</string>"
"    <key>Eastern</key>                     <string>America/New_York</string>"
"    <key>Eastern Standard Time</key>       <string>America/New_York</string>"
"    <key>Egypt</key>       <string>Africa/Cairo</string>"
"    <key>Egypt Standard Time</key> <string>Africa/Cairo</string>"
"    <key>Ekaterinburg</key>                <string>Asia/Yekaterinburg</string>"
"    <key>Ekaterinburg Standard Time</key>	<string>Asia/Yekaterinburg</string>"
"    <key>Fiji</key>	<string>Pacific/Fiji</string>"
"    <key>Fiji Standard Time</key>	<string>Pacific/Fiji</string>"
"    <key>FLE</key>	<string>Europe/Helsinki</string>"
"    <key>FLE Standard Time</key>	<string>Europe/Helsinki</string>"
"    <key>Georgian Standard Time</key>	<string>Asia/Tbilisi</string>"
"    <key>GFT</key>	<string>Europe/Athens</string>"
"    <key>GFT Standard Time</key>	<string>Europe/Athens</string>"
"    <key>GMT</key>	<string>Europe/London</string>"
"    <key>GMT Standard Time</key>	<string>Europe/London</string>"
"    <key>Greenland Standard Time</key>	<string>America/Godthab</string>"
"    <key>Greenwich</key>	<string>GMT</string>"
"    <key>Greenwich Standard Time</key>	<string>GMT</string>"
"    <key>GTB</key>	<string>Europe/Athens</string>"
"    <key>GTB Standard Time</key>	<string>Europe/Athens</string>"
"    <key>Hawaiian</key>	<string>Pacific/Honolulu</string>"
"    <key>Hawaiian Standard Time</key>	<string>Pacific/Honolulu</string>"
"    <key>India</key>	<string>Asia/Calcutta</string>"
"    <key>India Standard Time</key>	<string>Asia/Calcutta</string>"
"    <key>Iran</key>	<string>Asia/Tehran</string>"
"    <key>Iran Standard Time</key>	<string>Asia/Tehran</string>"
"    <key>Israel</key>	<string>Asia/Jerusalem</string>"
"    <key>Israel Standard Time</key>	<string>Asia/Jerusalem</string>"
"    <key>Jordan Standard Time</key>	<string>Asia/Amman</string>"
"    <key>Korea</key>	<string>Asia/Seoul</string>"
"    <key>Korea Standard Time</key>	<string>Asia/Seoul</string>"
"    <key>Mexico</key>	<string>America/Mexico_City</string>"
"    <key>Mexico Standard Time</key>	<string>America/Mexico_City</string>"
"    <key>Mexico Standard Time 2</key>	<string>America/Chihuahua</string>"
"    <key>Mid-Atlantic</key>	<string>Atlantic/South_Georgia</string>"
"    <key>Mid-Atlantic Standard Time</key>	<string>Atlantic/South_Georgia</string>"
"    <key>Middle East Standard Time</key>	<string>Asia/Beirut</string>"
"    <key>Mountain</key>	<string>America/Denver</string>"
"    <key>Mountain Standard Time</key>	<string>America/Denver</string>"
"    <key>Mountain Standard Time (Mexico)</key>	<string>America/Chihuahua</string>"
"    <key>Myanmar Standard Time</key>	<string>Asia/Rangoon</string>"
"    <key>N. Central Asia Standard Time</key>   <string>Asia/Novosibirsk</string>"
"    <key>Namibia Standard Time</key>   <string>Africa/Windhoek</string>"
"    <key>Nepal Standard Time</key>	<string>Asia/Katmandu</string>"
"    <key>New Zealand</key>	<string>Pacific/Auckland</string>"
"    <key>New Zealand Standard Time</key>	<string>Pacific/Auckland</string>"
"    <key>Newfoundland</key>	<string>America/St_Johns</string>"
"    <key>Newfoundland Standard Time</key>	<string>America/St_Johns</string>"
"    <key>North Asia East Standard Time</key>	<string>Asia/Ulaanbaatar</string>"
"    <key>North Asia Standard Time</key>	<string>Asia/Krasnoyarsk</string>"
"    <key>Pacific</key>	<string>America/Los_Angeles</string>"
"    <key>Pacific SA</key>	<string>America/Santiago</string>"
"    <key>Pacific SA Standard Time</key>	<string>America/Santiago</string>"
"    <key>Pacific Standard Time</key>	<string>America/Los_Angeles</string>"
"    <key>Pacific Standard Time (Mexico)</key>	<string>America/Tijuana</string>"
"    <key>Prague Bratislava</key>	<string>Europe/Prague</string>"
"    <key>Romance</key>	<string>Europe/Paris</string>"
"    <key>Romance Standard Time</key>	<string>Europe/Paris</string>"
"    <key>Russian</key>	<string>Europe/Moscow</string>"
"    <key>Russian Standard Time</key>	<string>Europe/Moscow</string>"
"    <key>SA Eastern</key>	<string>America/Buenos_Aires</string>"
"    <key>SA Eastern Standard Time</key>	<string>America/Buenos_Aires</string>"
"    <key>SA Pacific</key>	<string>America/Bogota</string>"
"    <key>SA Pacific Standard Time</key>	<string>America/Bogota</string>"
"    <key>SA Western</key>	<string>America/Caracas</string>"
"    <key>SA Western Standard Time</key>	<string>America/Caracas</string>"
"    <key>Samoa</key>	<string>Pacific/Apia</string>"
"    <key>Samoa Standard Time</key>	<string>Pacific/Apia</string>"
"    <key>Saudi Arabia</key>	<string>Asia/Riyadh</string>"
"    <key>Saudi Arabia Standard Time</key>	<string>Asia/Riyadh</string>"
"    <key>SE Asia Standard Time</key>	<string>Asia/Bangkok</string>"
"    <key>Singapore</key>	<string>Asia/Singapore</string>"
"    <key>Singapore Standard Time</key>	<string>Asia/Singapore</string>"
"    <key>South Africa</key>	<string>Africa/Harare</string>"
"    <key>South Africa Standard Time</key>	<string>Africa/Harare</string>"
"    <key>Sri Lanka</key>	<string>Asia/Colombo</string>"
"    <key>Sri Lanka Standard Time</key>	<string>Asia/Colombo</string>"
"    <key>Sydney Standard Time</key>	<string>Australia/Sydney</string>"
"    <key>Taipei</key>	<string>Asia/Taipei</string>"
"    <key>Taipei Standard Time</key>	<string>Asia/Taipei</string>"
"    <key>Tasmania</key>	<string>Australia/Hobart</string>"
"    <key>Tasmania Standard Time</key>	<string>Australia/Hobart</string>"
"    <key>Tasmania Standard Time</key>	<string>Australia/Hobart</string>"
"    <key>Tokyo</key>	<string>Asia/Tokyo</string>"
"    <key>Tokyo Standard Time</key>	<string>Asia/Tokyo</string>"
"    <key>Tonga Standard Time</key>	<string>Pacific/Tongatapu</string>"
"    <key>US Eastern</key>	<string>America/Indianapolis</string>"
"    <key>US Eastern Standard Time</key>	<string>America/Indianapolis</string>"
"    <key>US Mountain</key>	<string>America/Phoenix</string>"
"    <key>US Mountain Standard Time</key>	<string>America/Phoenix</string>"
"    <key>Vladivostok</key>	<string>Asia/Vladivostok</string>"
"    <key>Vladivostok Standard Time</key>	<string>Asia/Vladivostok</string>"
"    <key>W. Australia</key>	<string>Australia/Perth</string>"
"    <key>W. Australia Standard Time</key>	<string>Australia/Perth</string>"
"    <key>W. Central Africa Standard Time</key>	<string>Africa/Luanda</string>"
"    <key>W. Europe</key>	<string>Europe/Berlin</string>"
"    <key>W. Europe Standard Time</key>	<string>Europe/Berlin</string>"
"    <key>Warsaw</key>	<string>Europe/Warsaw</string>"
"    <key>West Asia</key>	<string>Asia/Karachi</string>"
"    <key>West Asia Standard Time</key>	<string>Asia/Karachi</string>"
"    <key>West Pacific</key>	<string>Pacific/Guam</string>"
"    <key>West Pacific Standard Time</key>	<string>Pacific/Guam</string>"
"    <key>Western Brazilian Standard Time</key>	<string>America/Rio_Branco</string>"
"    <key>Yakutsk</key>	<string>Asia/Yakutsk</string>"
" </dict>"
" </plist>";

CF_INLINE void __CFTimeZoneLockWinToOlson(void) {
    __CFLock(&__CFTimeZoneWinToOlsonLock);
}

CF_INLINE void __CFTimeZoneUnlockWinToOlson(void) {
    __CFUnlock(&__CFTimeZoneWinToOlsonLock);
}

CFDictionaryRef CFTimeZoneCopyWinToOlsonDictionary(void) {
    CFDictionaryRef dict;
    __CFTimeZoneLockWinToOlson();
    if (NULL == __CFTimeZoneWinToOlsonDict) {
        CFDataRef data = CFDataCreate(kCFAllocatorSystemDefault, (uint8_t *)__CFTimeZoneWinToOlsonDefaults, strlen(__CFTimeZoneWinToOlsonDefaults));
        __CFTimeZoneWinToOlsonDict = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, data, kCFPropertyListImmutable, NULL);
        CFRelease(data);
    }
    if (NULL == __CFTimeZoneWinToOlsonDict) {
        __CFTimeZoneWinToOlsonDict = CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, NULL, NULL);
    }
    dict = __CFTimeZoneWinToOlsonDict ? (CFDictionaryRef)CFRetain(__CFTimeZoneWinToOlsonDict) : NULL;
    __CFTimeZoneUnlockWinToOlson();
    return dict;
}

void CFTimeZoneSetWinToOlsonDictionary(CFDictionaryRef dict) {
    __CFGenericValidateType(dict, CFDictionaryGetTypeID());
    __CFTimeZoneLockWinToOlson();
    if (dict != __CFTimeZoneWinToOlsonDict) {
        if (dict) CFRetain(dict);
        if (__CFTimeZoneWinToOlsonDict) CFRelease(__CFTimeZoneWinToOlsonDict);
        __CFTimeZoneWinToOlsonDict = dict;
    }
    __CFTimeZoneUnlockWinToOlson();
}

CFTimeZoneRef CFTimeZoneCreateWithWindowsName(CFAllocatorRef allocator, CFStringRef winName) {
    if (!winName) return NULL;
    
    CFDictionaryRef winToOlson = CFTimeZoneCopyWinToOlsonDictionary();
    if (!winToOlson) return NULL;
    
    CFStringRef olsonName = CFDictionaryGetValue(winToOlson, winName);
    CFTimeZoneRef retval = NULL;
    if (olsonName) {
         retval = CFTimeZoneCreateWithName(allocator, olsonName, false);
    }
    CFRelease(winToOlson);
    return retval;
}

extern CFStringRef _CFGetWindowsAppleSystemLibraryDirectory(void);
void __InitTZStrings(void) {
    static CFLock_t __CFTZDirLock = CFLockInit;
    __CFLock(&__CFTZDirLock);
    if (!__tzZoneInfo) {
        CFStringRef winDir = _CFGetWindowsAppleSystemLibraryDirectory();
        __tzZoneInfo = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@\\etc\\zoneinfo"), winDir);
    }   
    if (!__tzDir && __tzZoneInfo) {
        int length = CFStringGetLength(__tzZoneInfo) + sizeof("\\zone.tab") + 1;
        __tzDir = malloc(length); // If we don't use ascii, we'll need to malloc more space
        if (!__tzDir || !CFStringGetCString(__tzZoneInfo, __tzDir, length, kCFStringEncodingASCII)) {
            free(__tzDir);
        } else {
            strcat(__tzDir, "\\zone.tab");
        }
    }
    __CFUnlock(&__CFTZDirLock);
}
#endif

static CFTimeZoneRef __CFTimeZoneCreateSystem(void) {
    CFTimeZoneRef result = NULL;
    
#if DEPLOYMENT_TARGET_WINDOWS
    CFStringRef name = NULL;
    TIME_ZONE_INFORMATION tzi = { 0 };
    DWORD rval = GetTimeZoneInformation(&tzi);
    if (rval != TIME_ZONE_ID_INVALID) {
        LPWSTR standardName = (LPWSTR)&tzi.StandardName;
        CFStringRef cfStandardName = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (UInt8 *)standardName, wcslen(standardName)*sizeof(WCHAR), kCFStringEncodingUTF16LE, false);
        if (cfStandardName) {
            CFDictionaryRef winToOlson = CFTimeZoneCopyWinToOlsonDictionary();
            if (winToOlson) {
                name = CFDictionaryGetValue(winToOlson, cfStandardName);
                if (name) CFRetain(name);
                CFRelease(winToOlson);
            }
            CFRelease(cfStandardName);
        }
    } else {
        CFLog(kCFLogLevelError, CFSTR("Couldn't get time zone information error %d"), GetLastError());
    }
    if (name) {
#else
    const char *tzenv;
    int ret;
    char linkbuf[CFMaxPathSize];

    tzenv = __CFgetenv("TZFILE");
    if (NULL != tzenv) {
	CFStringRef name = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (uint8_t *)tzenv, strlen(tzenv), kCFStringEncodingUTF8, false);
	result = CFTimeZoneCreateWithName(kCFAllocatorSystemDefault, name, false);
	CFRelease(name);
	if (result) return result;
    }
    tzenv = __CFgetenv("TZ");
    if (NULL != tzenv) {
	CFStringRef name = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (uint8_t *)tzenv, strlen(tzenv), kCFStringEncodingUTF8, false);
	result = CFTimeZoneCreateWithName(kCFAllocatorSystemDefault, name, true);
	CFRelease(name);
	if (result) return result;
    }
    ret = readlink(TZZONELINK, linkbuf, sizeof(linkbuf));
    if (0 < ret) {
	CFStringRef name;
	linkbuf[ret] = '\0';
	if (strncmp(linkbuf, TZZONEINFO, sizeof(TZZONEINFO) - 1) == 0) {
	    name = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (uint8_t *)linkbuf + sizeof(TZZONEINFO) - 1, strlen(linkbuf) - sizeof(TZZONEINFO) + 1, kCFStringEncodingUTF8, false);
	} else {
	    name = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (uint8_t *)linkbuf, strlen(linkbuf), kCFStringEncodingUTF8, false);
	}
#endif

	result = CFTimeZoneCreateWithName(kCFAllocatorSystemDefault, name, false);
	CFRelease(name);
	if (result) return result;
    }
    return CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorSystemDefault, 0.0);
}

CFTimeZoneRef CFTimeZoneCopySystem(void) {
    CFTimeZoneRef tz;
    __CFTimeZoneLockGlobal();
    if (NULL == __CFTimeZoneSystem) {
	__CFTimeZoneUnlockGlobal();
	tz = __CFTimeZoneCreateSystem();
	__CFTimeZoneLockGlobal();
	if (NULL == __CFTimeZoneSystem) {
	    __CFTimeZoneSystem = tz;
	} else {
	    if (tz) CFRelease(tz);
	}
    }
    tz = __CFTimeZoneSystem ? (CFTimeZoneRef)CFRetain(__CFTimeZoneSystem) : NULL;
    __CFTimeZoneUnlockGlobal();
    return tz;
}

static CFIndex __noteCount = 0;

void CFTimeZoneResetSystem(void) {
    __CFTimeZoneLockGlobal();
    if (__CFTimeZoneDefault == __CFTimeZoneSystem) {
	if (__CFTimeZoneDefault) CFRelease(__CFTimeZoneDefault);
	__CFTimeZoneDefault = NULL;
    }
    CFTimeZoneRef tz = __CFTimeZoneSystem;
    __CFTimeZoneSystem = NULL;
    __CFTimeZoneUnlockGlobal();
    if (tz) CFRelease(tz);
}

CFIndex _CFTimeZoneGetNoteCount(void) {
    return __noteCount;
}

CFTimeZoneRef CFTimeZoneCopyDefault(void) {
    CFTimeZoneRef tz;
    __CFTimeZoneLockGlobal();
    if (NULL == __CFTimeZoneDefault) {
	__CFTimeZoneUnlockGlobal();
	tz = CFTimeZoneCopySystem();
	__CFTimeZoneLockGlobal();
	if (NULL == __CFTimeZoneDefault) {
	    __CFTimeZoneDefault = tz;
	} else {
	    if (tz) CFRelease(tz);
	}
    }
    tz = __CFTimeZoneDefault ? (CFTimeZoneRef)CFRetain(__CFTimeZoneDefault) : NULL;
    __CFTimeZoneUnlockGlobal();
    return tz;
}

void CFTimeZoneSetDefault(CFTimeZoneRef tz) {
    if (tz) __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    __CFTimeZoneLockGlobal();
    if (tz != __CFTimeZoneDefault) {
	if (tz) CFRetain(tz);
	if (__CFTimeZoneDefault) CFRelease(__CFTimeZoneDefault);
	__CFTimeZoneDefault = tz;
    }
    __CFTimeZoneUnlockGlobal();
}

static CFDictionaryRef __CFTimeZoneCopyCompatibilityDictionary(void);

CFArrayRef CFTimeZoneCopyKnownNames(void) {
    CFArrayRef tzs;
    __CFTimeZoneLockGlobal();
    if (NULL == __CFKnownTimeZoneList) {
	CFMutableArrayRef list;
/* TimeZone information locate in the registry for Win32
 * (Aleksey Dukhnyakov)
 */
        list = __CFCopyRecursiveDirectoryList();
	// Remove undesirable ancient cruft
	CFDictionaryRef dict = __CFTimeZoneCopyCompatibilityDictionary();
	CFIndex idx;
	for (idx = CFArrayGetCount(list); idx--; ) {
	    CFStringRef item = (CFStringRef)CFArrayGetValueAtIndex(list, idx);
	    if (CFDictionaryContainsKey(dict, item)) {
		CFArrayRemoveValueAtIndex(list, idx);
	    }
	}
	__CFKnownTimeZoneList = CFArrayCreateCopy(kCFAllocatorSystemDefault, list);
	CFRelease(list);
    }
    tzs = __CFKnownTimeZoneList ? (CFArrayRef)CFRetain(__CFKnownTimeZoneList) : NULL;
    __CFTimeZoneUnlockGlobal();
    return tzs;
}

/* The criteria here are sort of: coverage for the U.S. and Europe,
 * large cities, abbreviation uniqueness, and perhaps a few others.
 * But do not make the list too large with obscure information.
 */
static const char *__CFTimeZoneAbbreviationDefaults =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
" <!DOCTYPE plist SYSTEM \"file://localhost/System/Library/DTDs/PropertyList.dtd\">"
" <plist version=\"1.0\">"
" <dict>"
"    <key>ADT</key>  <string>America/Halifax</string>"
"    <key>AKDT</key> <string>America/Juneau</string>"
"    <key>AKST</key> <string>America/Juneau</string>"
"    <key>ART</key>  <string>America/Argentina/Buenos_Aires</string>"
"    <key>AST</key>  <string>America/Halifax</string>"
"    <key>BDT</key>  <string>Asia/Dhaka</string>"
"    <key>BRST</key> <string>America/Sao_Paulo</string>"
"    <key>BRT</key>  <string>America/Sao_Paulo</string>"
"    <key>BST</key>  <string>Europe/London</string>"
"    <key>CAT</key>  <string>Africa/Harare</string>"
"    <key>CDT</key>  <string>America/Chicago</string>"
"    <key>CEST</key> <string>Europe/Paris</string>"
"    <key>CET</key>  <string>Europe/Paris</string>"
"    <key>CLST</key> <string>America/Santiago</string>"
"    <key>CLT</key>  <string>America/Santiago</string>"
"    <key>COT</key>  <string>America/Bogota</string>"
"    <key>CST</key>  <string>America/Chicago</string>"
"    <key>EAT</key>  <string>Africa/Addis_Ababa</string>"
"    <key>EDT</key>  <string>America/New_York</string>"
"    <key>EEST</key> <string>Europe/Istanbul</string>"
"    <key>EET</key>  <string>Europe/Istanbul</string>"
"    <key>EST</key>  <string>America/New_York</string>"
"    <key>GMT</key>  <string>GMT</string>"
"    <key>GST</key>  <string>Asia/Dubai</string>"
"    <key>HKT</key>  <string>Asia/Hong_Kong</string>"
"    <key>HST</key>  <string>Pacific/Honolulu</string>"
"    <key>ICT</key>  <string>Asia/Bangkok</string>"
"    <key>IRST</key> <string>Asia/Tehran</string>"
"    <key>IST</key>  <string>Asia/Calcutta</string>"
"    <key>JST</key>  <string>Asia/Tokyo</string>"
"    <key>KST</key>  <string>Asia/Seoul</string>"
"    <key>MDT</key>  <string>America/Denver</string>"
"    <key>MSD</key>  <string>Europe/Moscow</string>"
"    <key>MSK</key>  <string>Europe/Moscow</string>"
"    <key>MST</key>  <string>America/Denver</string>"
"    <key>NZDT</key> <string>Pacific/Auckland</string>"
"    <key>NZST</key> <string>Pacific/Auckland</string>"
"    <key>PDT</key>  <string>America/Los_Angeles</string>"
"    <key>PET</key>  <string>America/Lima</string>"
"    <key>PHT</key>  <string>Asia/Manila</string>"
"    <key>PKT</key>  <string>Asia/Karachi</string>"
"    <key>PST</key>  <string>America/Los_Angeles</string>"
"    <key>SGT</key>  <string>Asia/Singapore</string>"
"    <key>UTC</key>  <string>UTC</string>"
"    <key>WAT</key>  <string>Africa/Lagos</string>"
"    <key>WEST</key> <string>Europe/Lisbon</string>"
"    <key>WET</key>  <string>Europe/Lisbon</string>"
"    <key>WIT</key>  <string>Asia/Jakarta</string>"
" </dict>"
" </plist>";

CFDictionaryRef CFTimeZoneCopyAbbreviationDictionary(void) {
    CFDictionaryRef dict;
    __CFTimeZoneLockAbbreviations();
    if (NULL == __CFTimeZoneAbbreviationDict) {
	CFDataRef data = CFDataCreate(kCFAllocatorSystemDefault, (uint8_t *)__CFTimeZoneAbbreviationDefaults, strlen(__CFTimeZoneAbbreviationDefaults));
	__CFTimeZoneAbbreviationDict = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, data, kCFPropertyListImmutable, NULL);
	CFRelease(data);
    }
    if (NULL == __CFTimeZoneAbbreviationDict) {
	__CFTimeZoneAbbreviationDict = CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, NULL, NULL);
    }
    dict = __CFTimeZoneAbbreviationDict ? (CFDictionaryRef)CFRetain(__CFTimeZoneAbbreviationDict) : NULL;
    __CFTimeZoneUnlockAbbreviations();
    return dict;
}

void _removeFromCache(const void *key, const void *value, void *context) {
    CFDictionaryRemoveValue(__CFTimeZoneCache, (CFStringRef)key);
}

void CFTimeZoneSetAbbreviationDictionary(CFDictionaryRef dict) {
    __CFGenericValidateType(dict, CFDictionaryGetTypeID());
    __CFTimeZoneLockGlobal();
    if (dict != __CFTimeZoneAbbreviationDict) {
	if (dict) CFRetain(dict);
	if (__CFTimeZoneAbbreviationDict) {
	    CFDictionaryApplyFunction(__CFTimeZoneAbbreviationDict, _removeFromCache, NULL);
	    CFRelease(__CFTimeZoneAbbreviationDict);
	}
	__CFTimeZoneAbbreviationDict = dict;
    }
    __CFTimeZoneUnlockGlobal();
}

CFTimeZoneRef CFTimeZoneCreate(CFAllocatorRef allocator, CFStringRef name, CFDataRef data) {
// assert:    (NULL != name && NULL != data);
    CFTimeZoneRef memory;
    uint32_t size;
    CFTZPeriod *tzp = NULL;
    CFIndex idx, cnt = 0;

    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(name, CFStringGetTypeID());
    __CFGenericValidateType(data, CFDataGetTypeID());
    __CFTimeZoneLockGlobal();
    if (NULL != __CFTimeZoneCache && CFDictionaryGetValueIfPresent(__CFTimeZoneCache, name, (const void **)&memory)) {
	__CFTimeZoneUnlockGlobal();
	return (CFTimeZoneRef)CFRetain(memory);
    }
    if (!__CFParseTimeZoneData(allocator, data, &tzp, &cnt)) {
	__CFTimeZoneUnlockGlobal();
	return NULL;
    }
    size = sizeof(struct __CFTimeZone) - sizeof(CFRuntimeBase);
    memory = (CFTimeZoneRef)_CFRuntimeCreateInstance(allocator, CFTimeZoneGetTypeID(), size, NULL);
    if (NULL == memory) {
	__CFTimeZoneUnlockGlobal();
	for (idx = 0; idx < cnt; idx++) {
	    if (NULL != tzp[idx].abbrev) CFRelease(tzp[idx].abbrev);
	}
	if (NULL != tzp) CFAllocatorDeallocate(allocator, tzp);
        return NULL;
    }
    ((struct __CFTimeZone *)memory)->_name = (CFStringRef)CFStringCreateCopy(allocator, name);
    ((struct __CFTimeZone *)memory)->_data = CFDataCreateCopy(allocator, data);
    ((struct __CFTimeZone *)memory)->_periods = tzp;
    ((struct __CFTimeZone *)memory)->_periodCnt = cnt;
    if (NULL == __CFTimeZoneCache) {
	__CFTimeZoneCache = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    CFDictionaryAddValue(__CFTimeZoneCache, ((struct __CFTimeZone *)memory)->_name, memory);
    __CFTimeZoneUnlockGlobal();
    return memory;
}

static CFTimeZoneRef __CFTimeZoneCreateFixed(CFAllocatorRef allocator, int32_t seconds, CFStringRef name, int isDST) {
    CFTimeZoneRef result;
    CFDataRef data;
    int32_t nameLen = CFStringGetLength(name);
    unsigned char dataBytes[52 + nameLen + 1];
    memset(dataBytes, 0, sizeof(dataBytes));
    
    // Put in correct magic bytes for timezone structures
    dataBytes[0] = 'T';
    dataBytes[1] = 'Z';
    dataBytes[2] = 'i';
    dataBytes[3] = 'f';
    
    __CFEntzcode(1, dataBytes + 20);
    __CFEntzcode(1, dataBytes + 24);
    __CFEntzcode(1, dataBytes + 36);
    __CFEntzcode(nameLen + 1, dataBytes + 40);
    __CFEntzcode(seconds, dataBytes + 44);
    dataBytes[48] = isDST ? 1 : 0;
    CFStringGetCString(name, (char *)dataBytes + 50, nameLen + 1, kCFStringEncodingASCII);
    data = CFDataCreate(allocator, dataBytes, 52 + nameLen + 1);
    result = CFTimeZoneCreate(allocator, name, data);
    CFRelease(data);
    return result;
}


// rounds offset to nearest minute
CFTimeZoneRef CFTimeZoneCreateWithTimeIntervalFromGMT(CFAllocatorRef allocator, CFTimeInterval ti) {
    CFTimeZoneRef result;
    CFStringRef name;
    int32_t seconds, minute, hour;
    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    if (ti < -18.0 * 3600 || 18.0 * 3600 < ti) return NULL;
    ti = (ti < 0.0) ? ceil((ti / 60.0) - 0.5) * 60.0 : floor((ti / 60.0) + 0.5) * 60.0;
    seconds = (int32_t)ti;
    hour = (ti < 0) ? (-seconds / 3600) : (seconds / 3600);
    seconds -= ((ti < 0) ? -hour : hour) * 3600;
    minute = (ti < 0) ? (-seconds / 60) : (seconds / 60);
    if (fabs(ti) < 1.0) {
	name = (CFStringRef)CFRetain(CFSTR("GMT"));
    } else {
	name = CFStringCreateWithFormat(allocator, NULL, CFSTR("GMT%c%02d%02d"), (ti < 0.0 ? '-' : '+'), hour, minute);
    }
    result = __CFTimeZoneCreateFixed(allocator, (int32_t)ti, name, 0);
    CFRelease(name);
    return result;
}

CFTimeZoneRef CFTimeZoneCreateWithName(CFAllocatorRef allocator, CFStringRef name, Boolean tryAbbrev) {
    CFTimeZoneRef result = NULL;
    CFStringRef tzName = NULL;
    CFDataRef data = NULL;

    if (allocator == NULL) allocator = __CFGetDefaultAllocator();
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    __CFGenericValidateType(name, CFStringGetTypeID());
    if (CFEqual(CFSTR(""), name)) {
	// empty string is not a time zone name, just abort now,
	// following stuff will fail anyway
	return NULL;
    }
    __CFTimeZoneLockGlobal();
    if (NULL != __CFTimeZoneCache && CFDictionaryGetValueIfPresent(__CFTimeZoneCache, name, (const void **)&result)) {
	__CFTimeZoneUnlockGlobal();
	return (CFTimeZoneRef)CFRetain(result);
    }
    __CFTimeZoneUnlockGlobal();
    CFIndex len = CFStringGetLength(name);
    if (6 == len || 8 == len) {
	UniChar buffer[8];
	CFStringGetCharacters(name, CFRangeMake(0, len), buffer);
	if ('G' == buffer[0] && 'M' == buffer[1] && 'T' == buffer[2] && ('+' == buffer[3] || '-' == buffer[3])) {
	    if (('0' <= buffer[4] && buffer[4] <= '9') && ('0' <= buffer[5] && buffer[5] <= '9')) {
		int32_t hours = (buffer[4] - '0') * 10 + (buffer[5] - '0');
		if (-14 <= hours && hours <= 14) {
		    CFTimeInterval ti = hours * 3600.0;
		    if (6 == len) {
			return CFTimeZoneCreateWithTimeIntervalFromGMT(allocator, ('-' == buffer[3] ? -1.0 : 1.0) * ti);
		    } else {
			if (('0' <= buffer[6] && buffer[6] <= '9') && ('0' <= buffer[7] && buffer[7] <= '9')) {
			    int32_t minutes = (buffer[6] - '0') * 10 + (buffer[7] - '0');
			    if ((-14 == hours && 0 == minutes) || (14 == hours && 0 == minutes) || (0 <= minutes && minutes <= 59)) {
				ti = ti + minutes * 60.0;
				return CFTimeZoneCreateWithTimeIntervalFromGMT(allocator, ('-' == buffer[3] ? -1.0 : 1.0) * ti);
			    }
			}
		    }
		}
	    }
	}
    }
    CFURLRef baseURL, tempURL;
    void *bytes;
    CFIndex length;

#if DEPLOYMENT_TARGET_WINDOWS
    if (!__tzZoneInfo) __InitTZStrings();
    if (!__tzZoneInfo) return NULL;
    baseURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, __tzZoneInfo, kCFURLWindowsPathStyle, true);
#else
    baseURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, CFSTR(TZZONEINFO), kCFURLPOSIXPathStyle, true);
#endif
    if (tryAbbrev) {
	CFDictionaryRef abbrevs = CFTimeZoneCopyAbbreviationDictionary();
	tzName = CFDictionaryGetValue(abbrevs, name);
	if (NULL != tzName) {
	    tempURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, tzName, false);
	    if (NULL != tempURL) {
		if (_CFReadBytesFromFile(kCFAllocatorSystemDefault, tempURL, &bytes, &length, 0, 0)) {
		    data = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, bytes, length, kCFAllocatorSystemDefault);
		}
		CFRelease(tempURL);
	    }
	}
	CFRelease(abbrevs);
    }
    if (NULL == data) {
	CFDictionaryRef dict = __CFTimeZoneCopyCompatibilityDictionary();
	CFStringRef mapping = CFDictionaryGetValue(dict, name);
	if (mapping) {
	    name = mapping;
#if DEPLOYMENT_TARGET_WINDOWS
	} else if (CFStringHasPrefix(name, __tzZoneInfo)) {
	    CFMutableStringRef unprefixed = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, CFStringGetLength(name), name);
	    CFStringDelete(unprefixed, CFRangeMake(0, CFStringGetLength(__tzZoneInfo)));
#else
	} else if (CFStringHasPrefix(name, CFSTR(TZZONEINFO))) {
	    CFMutableStringRef unprefixed = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, CFStringGetLength(name), name);
	    CFStringDelete(unprefixed, CFRangeMake(0, sizeof(TZZONEINFO)));
#endif
	    mapping = CFDictionaryGetValue(dict, unprefixed);
	    if (mapping) {
		name = mapping;
	    }
	    CFRelease(unprefixed);
	}
	CFRelease(dict);
	if (CFEqual(CFSTR(""), name)) {
	    return NULL;
	}
    }
    if (NULL == data) {
       tzName = name;
       tempURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, tzName, false);
       if (NULL != tempURL) {
           if (_CFReadBytesFromFile(kCFAllocatorSystemDefault, tempURL, &bytes, &length, 0, 0)) {
               data = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, bytes, length, kCFAllocatorSystemDefault);
           }
           CFRelease(tempURL);
       }
    }
    CFRelease(baseURL);
    if (NULL != data) {
	result = CFTimeZoneCreate(allocator, tzName, data);
	if (name != tzName) {
	    CFStringRef nameCopy = (CFStringRef)CFStringCreateCopy(allocator, name);
	    __CFTimeZoneLockGlobal();
	    CFDictionaryAddValue(__CFTimeZoneCache, nameCopy, result);
	    __CFTimeZoneUnlockGlobal();
	    CFRelease(nameCopy);
	}
	CFRelease(data);
    }
    return result;
}

CFStringRef CFTimeZoneGetName(CFTimeZoneRef tz) {
    CF_OBJC_FUNCDISPATCHV(CFTimeZoneGetTypeID(), CFStringRef, (NSTimeZone *)tz, name);
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    return tz->_name;
}

CFDataRef CFTimeZoneGetData(CFTimeZoneRef tz) {
    CF_OBJC_FUNCDISPATCHV(CFTimeZoneGetTypeID(), CFDataRef, (NSTimeZone *)tz, data);
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    return tz->_data;
}

/* This function converts CFAbsoluteTime to (Win32) SYSTEMTIME
 * (Aleksey Dukhnyakov)
 */
#if DEPLOYMENT_TARGET_WINDOWS
BOOL __CFTimeZoneGetWin32SystemTime(SYSTEMTIME * sys_time, CFAbsoluteTime time)
{
    LONGLONG l;
    FILETIME * ftime=(FILETIME*)&l;

    /*  seconds between 1601 and 1970 : 11644473600,
     *  seconds between 1970 and 2001 : 978307200,
     *  FILETIME - number of 100-nanosecond intervals since January 1, 1601
     */
    l=(LONGLONG)(time+11644473600LL+978307200)*10000000;
    if (FileTimeToSystemTime(ftime,sys_time))
        return TRUE;
    else
        return FALSE;
}
#endif

CFTimeInterval CFTimeZoneGetSecondsFromGMT(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CFIndex idx;
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    idx = __CFBSearchTZPeriods(tz, at);
    return __CFTZPeriodGMTOffset(&(tz->_periods[idx]));
}

CFStringRef CFTimeZoneCopyAbbreviation(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CFStringRef result;
    CFIndex idx;
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    idx = __CFBSearchTZPeriods(tz, at);
    result = __CFTZPeriodAbbreviation(&(tz->_periods[idx]));
    return result ? (CFStringRef)CFRetain(result) : NULL;
}

Boolean CFTimeZoneIsDaylightSavingTime(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CFIndex idx;
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    idx = __CFBSearchTZPeriods(tz, at);
    return __CFTZPeriodIsDST(&(tz->_periods[idx]));
}

CFTimeInterval CFTimeZoneGetDaylightSavingTimeOffset(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CF_OBJC_FUNCDISPATCHV(CFTimeZoneGetTypeID(), CFTimeInterval, (NSTimeZone *)tz, _daylightSavingTimeOffsetForAbsoluteTime:at);
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    CFIndex idx = __CFBSearchTZPeriods(tz, at);
    if (__CFTZPeriodIsDST(&(tz->_periods[idx]))) {
	CFTimeInterval offset = __CFTZPeriodGMTOffset(&(tz->_periods[idx]));
	if (idx + 1 < tz->_periodCnt) {
	    return offset - __CFTZPeriodGMTOffset(&(tz->_periods[idx + 1]));
	} else if (0 < idx) {
            return offset - __CFTZPeriodGMTOffset(&(tz->_periods[idx - 1]));
	}
    }
    return 0.0;
}

CFAbsoluteTime CFTimeZoneGetNextDaylightSavingTimeTransition(CFTimeZoneRef tz, CFAbsoluteTime at) {
    CF_OBJC_FUNCDISPATCHV(CFTimeZoneGetTypeID(), CFTimeInterval, (NSTimeZone *)tz, _nextDaylightSavingTimeTransitionAfterAbsoluteTime:at);
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    CFIndex idx = __CFBSearchTZPeriods(tz, at);
    if (tz->_periodCnt <= idx + 1) {
        return 0.0;
    }
    return (CFAbsoluteTime)__CFTZPeriodStartSeconds(&(tz->_periods[idx + 1]));
}

extern UCalendar *__CFCalendarCreateUCalendar(CFStringRef calendarID, CFStringRef localeID, CFTimeZoneRef tz);

#define BUFFER_SIZE 768

CFStringRef CFTimeZoneCopyLocalizedName(CFTimeZoneRef tz, CFTimeZoneNameStyle style, CFLocaleRef locale) {
    CF_OBJC_FUNCDISPATCHV(CFTimeZoneGetTypeID(), CFStringRef, (NSTimeZone *)tz, localizedName:(NSTimeZoneNameStyle)style locale:(NSLocale *)locale);
    __CFGenericValidateType(tz, CFTimeZoneGetTypeID());
    __CFGenericValidateType(locale, CFLocaleGetTypeID());

    if (style == kCFTimeZoneNameStyleGeneric || style == kCFTimeZoneNameStyleShortGeneric) {
	CFDateFormatterRef df = CFDateFormatterCreate(kCFAllocatorSystemDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterNoStyle);
	CFDateFormatterSetProperty(df, kCFDateFormatterTimeZone, tz);
	CFDateFormatterSetFormat(df, (style == kCFTimeZoneNameStyleGeneric) ? CFSTR("vvvv") : CFSTR("v"));
	CFStringRef str = CFDateFormatterCreateStringWithAbsoluteTime(CFGetAllocator(tz), df, 0.0);
	CFRelease(df);
	return str;
    }

    CFStringRef localeID = CFLocaleGetIdentifier(locale);
    UCalendar *cal = __CFCalendarCreateUCalendar(NULL, localeID, tz);
    if (NULL == cal) {
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    const char *cstr = CFStringGetCStringPtr(localeID, kCFStringEncodingASCII);
    if (NULL == cstr) {
        if (CFStringGetCString(localeID, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) cstr = buffer;
    }
    if (NULL == cstr) {
	ucal_close(cal);
        return NULL;
    }

    UChar ubuffer[BUFFER_SIZE];
    UErrorCode status = U_ZERO_ERROR;
    int32_t cnt = ucal_getTimeZoneDisplayName(cal, (UCalendarDisplayNameType)style, cstr, ubuffer, BUFFER_SIZE, &status);
    ucal_close(cal);
    if (U_SUCCESS(status) && cnt <= BUFFER_SIZE) {
        return CFStringCreateWithCharacters(CFGetAllocator(tz), (const UniChar *)ubuffer, cnt);
    }
    return NULL;
}

static CFDictionaryRef __CFTimeZoneCopyCompatibilityDictionary(void) {
    CFDictionaryRef dict;
    __CFTimeZoneLockCompatibilityMapping();
    if (NULL == __CFTimeZoneCompatibilityMappingDict) {
	__CFTimeZoneCompatibilityMappingDict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 112, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	// Empty string means delete/ignore these 
    }
    dict = __CFTimeZoneCompatibilityMappingDict ? (CFDictionaryRef)CFRetain(__CFTimeZoneCompatibilityMappingDict) : NULL;
    __CFTimeZoneUnlockCompatibilityMapping();
    return dict;
}

#undef TZZONEINFO
#undef TZZONELINK

