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

/*	CFUtilities.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Tony Parker
*/

#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include "CFLocaleInternal.h"
#include <CoreFoundation/CFPriv.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <CoreFoundation/CFBundle.h>
#endif
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFPropertyList.h>
#if DEPLOYMENT_TARGET_WINDOWS
#include <process.h>
#endif
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <asl.h>
#else
#define ASL_LEVEL_EMERG 0
#define ASL_LEVEL_DEBUG 7
#endif


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <unistd.h>
#include <sys/uio.h>
#include <mach/mach.h>
#include <pthread.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <crt_externs.h>
#include <dlfcn.h>
#include <vproc.h>
#include <vproc_priv.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/errno.h>
#include <mach/mach_time.h>
#include <Block.h>
#endif
#if DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#endif

/* Comparator is passed the address of the values. */
/* Binary searches a sorted-increasing array of some type.
   Return value is either 1) the index of the element desired,
   if the target value exists in the list, 2) greater than or
   equal to count, if the element is greater than all the values
   in the list, or 3) the index of the element greater than the
   target value.

   For example, a search in the list of integers:
	2 3 5 7 11 13 17

   For...		Will Return...
	2		    0
   	5		    2
	23		    7
	1		    0
	9		    4

   For instance, if you just care about found/not found:
   index = CFBSearch(list, count, elem);
   if (count <= index || list[index] != elem) {
   	* Not found *
   } else {
   	* Found *
   }
   
*/
CF_PRIVATE CFIndex CFBSearch(const void *element, CFIndex elementSize, const void *list, CFIndex count, CFComparatorFunction comparator, void *context) {
    const char *ptr = (const char *)list;
    while (0 < count) {
        CFIndex half = count / 2;
        const char *probe = ptr + elementSize * half;
        CFComparisonResult cr = comparator(element, probe, context);
	if (0 == cr) return (probe - (const char *)list) / elementSize;
        ptr = (cr < 0) ? ptr : probe + elementSize;
        count = (cr < 0) ? half : (half + (count & 1) - 1);
    }
    return (ptr - (const char *)list) / elementSize;
}


#define ELF_STEP(B) T1 = (H << 4) + B; T2 = T1 & 0xF0000000; if (T2) T1 ^= (T2 >> 24); T1 &= (~T2); H = T1;

CFHashCode CFHashBytes(uint8_t *bytes, CFIndex length) {
    /* The ELF hash algorithm, used in the ELF object file format */
    UInt32 H = 0, T1, T2;
    SInt32 rem = length;
    while (3 < rem) {
	ELF_STEP(bytes[length - rem]);
	ELF_STEP(bytes[length - rem + 1]);
	ELF_STEP(bytes[length - rem + 2]);
	ELF_STEP(bytes[length - rem + 3]);
	rem -= 4;
    }
    switch (rem) {
    case 3:  ELF_STEP(bytes[length - 3]);
    case 2:  ELF_STEP(bytes[length - 2]);
    case 1:  ELF_STEP(bytes[length - 1]);
    case 0:  ;
    }
    return H;
}

#undef ELF_STEP


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
CF_PRIVATE uintptr_t __CFFindPointer(uintptr_t ptr, uintptr_t start) {
    vm_map_t task = mach_task_self();
    mach_vm_address_t address = start;
    for (;;) {
	mach_vm_size_t size = 0;
	vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
	mach_port_t object_name;
        kern_return_t ret = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &count, &object_name);
        if (KERN_SUCCESS != ret) break;
	boolean_t scan = (info.protection & VM_PROT_WRITE) ? 1 : 0;
	if (scan) {
	    uintptr_t *addr = (uintptr_t *)((uintptr_t)address);
	    uintptr_t *end = (uintptr_t *)((uintptr_t)address + (uintptr_t)size);
	    while (addr < end) {
	        if ((uintptr_t *)start <= addr && *addr == ptr) {
		    return (uintptr_t)addr;
	        }
	        addr++;
	    }
	}
        address += size;
    }
    return 0;
}

CF_PRIVATE void __CFDumpAllPointerLocations(uintptr_t ptr) {
    uintptr_t addr = 0;
    do {
        addr = __CFFindPointer(ptr, sizeof(void *) + addr);
        printf("%p\n", (void *)addr);
    } while (addr != 0);
}
#endif

// Looks for localized version of "nonLocalized" in the SystemVersion bundle
// If not found, and returnNonLocalizedFlag == true, will return the non localized string (retained of course), otherwise NULL
// If bundlePtr != NULL, will use *bundlePtr and will return the bundle in there; otherwise bundle is created and released
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
static CFStringRef _CFCopyLocalizedVersionKey(CFBundleRef *bundlePtr, CFStringRef nonLocalized) {
    CFStringRef localized = NULL;
    CFBundleRef locBundle = bundlePtr ? *bundlePtr : NULL;
    if (!locBundle) {
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, CFSTR("/System/Library/CoreServices/SystemVersion.bundle"), kCFURLPOSIXPathStyle, false);
        if (url) {
            locBundle = CFBundleCreate(kCFAllocatorSystemDefault, url);
            CFRelease(url);
        }
    }
    if (locBundle) {
	localized = CFBundleCopyLocalizedString(locBundle, nonLocalized, nonLocalized, CFSTR("SystemVersion"));
	if (bundlePtr) *bundlePtr = locBundle; else CFRelease(locBundle);
    }
    return localized ? localized : (CFStringRef)CFRetain(nonLocalized);
}
#endif

static CFDictionaryRef _CFCopyVersionDictionary(CFStringRef path) {
    CFPropertyListRef plist = NULL;
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    CFDataRef data;
    CFURLRef url;
    
    url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, path, kCFURLPOSIXPathStyle, false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    if (url && CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, url, &data, NULL, NULL, NULL)) {
#pragma GCC diagnostic pop
        plist = CFPropertyListCreateWithData(kCFAllocatorSystemDefault, data, kCFPropertyListMutableContainers, NULL, NULL);
	CFRelease(data);
    }
    if (url) CFRelease(url);

    if (plist) {
#if DEPLOYMENT_TARGET_EMBEDDED_MINI
	CFStringRef fullVersion, vers, versExtra, build;
	CFStringRef versionString = CFRetain(_kCFSystemVersionProductVersionStringKey);
	CFStringRef buildString = CFRetain(_kCFSystemVersionBuildStringKey);
	CFStringRef fullVersionString = CFRetain(CFSTR("FullVersionString"));
#else
	CFBundleRef locBundle = NULL;
	CFStringRef fullVersion, vers, versExtra, build;
	CFStringRef versionString = _CFCopyLocalizedVersionKey(&locBundle, _kCFSystemVersionProductVersionStringKey);
	CFStringRef buildString = _CFCopyLocalizedVersionKey(&locBundle, _kCFSystemVersionBuildStringKey);
	CFStringRef fullVersionString = _CFCopyLocalizedVersionKey(&locBundle, CFSTR("FullVersionString"));
	if (locBundle) CFRelease(locBundle);
#endif

        // Now build the full version string
        if (CFEqual(fullVersionString, CFSTR("FullVersionString"))) {
            CFRelease(fullVersionString);
            fullVersionString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@ %%@ (%@ %%@)"), versionString, buildString);
        }
        vers = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)plist, _kCFSystemVersionProductVersionKey);
        versExtra = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)plist, _kCFSystemVersionProductVersionExtraKey);
        if (vers && versExtra) vers = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@ %@"), vers, versExtra);
        build = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)plist, _kCFSystemVersionBuildVersionKey);
        fullVersion = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, fullVersionString, (vers ? vers : CFSTR("?")), build ? build : CFSTR("?"));
        if (vers && versExtra) CFRelease(vers);
        
	CFDictionarySetValue((CFMutableDictionaryRef)plist, _kCFSystemVersionProductVersionStringKey, versionString);
	CFDictionarySetValue((CFMutableDictionaryRef)plist, _kCFSystemVersionBuildStringKey, buildString);
	CFDictionarySetValue((CFMutableDictionaryRef)plist, CFSTR("FullVersionString"), fullVersion);
 	CFRelease(versionString);
	CFRelease(buildString);
	CFRelease(fullVersionString);
        CFRelease(fullVersion);
    }
#elif DEPLOYMENT_TARGET_WINDOWS
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    BOOL result = GetVersionEx((OSVERSIONINFO *)&osvi);
    if (!result) return NULL;

    plist = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 10, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    // e.g. 10.7
    CFStringRef versionString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%ld.%ld(%ld,%ld)"), osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.wServicePackMinor);
    
    // e.g. 11A508
    CFStringRef buildString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%ld"), osvi.dwBuildNumber);
        
    CFDictionarySetValue((CFMutableDictionaryRef)plist, _kCFSystemVersionProductVersionKey, versionString);
    CFDictionarySetValue((CFMutableDictionaryRef)plist, _kCFSystemVersionBuildVersionKey, buildString);    
    CFDictionarySetValue((CFMutableDictionaryRef)plist, _kCFSystemVersionProductNameKey, CFSTR("Windows")); // hard coded for now
    
    CFRelease(versionString);
    CFRelease(buildString);
#endif
    return (CFDictionaryRef)plist;
}

CFStringRef CFCopySystemVersionString(void) {
    CFStringRef versionString;
    CFDictionaryRef dict = _CFCopyServerVersionDictionary();
    if (!dict) dict = _CFCopySystemVersionDictionary();
    if (!dict) return NULL;
    versionString = (CFStringRef)CFDictionaryGetValue(dict, CFSTR("FullVersionString"));
    if (versionString) CFRetain(versionString);
    CFRelease(dict);
    return versionString;
}

// Obsolete: These two functions cache the dictionaries to avoid calling _CFCopyVersionDictionary() more than once per dict desired
// In fact, they do not cache any more, because the file can change after
// apps are running in some situations, and apps need the new info.
// Proper caching and testing to see if the file has changed, without race
// conditions, would require semi-convoluted use of fstat().

static CFStringRef copySystemVersionPath(CFStringRef suffix) {
#if TARGET_IPHONE_SIMULATOR
    const char *simulatorRoot = getenv("IPHONE_SIMULATOR_ROOT");
    if (!simulatorRoot) simulatorRoot = getenv("CFFIXED_USER_HOME");
    if (!simulatorRoot) simulatorRoot = "/";
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%s%@"), simulatorRoot, suffix);
#else
    return suffix;
#endif
}


CFDictionaryRef _CFCopySystemVersionDictionary(void) {
    CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/SystemVersion.plist"));
    CFPropertyListRef plist = _CFCopyVersionDictionary(path);
    CFRelease(path);
    return (CFDictionaryRef)plist;
}

CFDictionaryRef _CFCopyServerVersionDictionary(void) {
    CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/ServerVersion.plist"));
    CFPropertyListRef plist = _CFCopyVersionDictionary(path);
    CFRelease(path);
    return (CFDictionaryRef)plist;
}

CONST_STRING_DECL(_kCFSystemVersionProductNameKey, "ProductName")
CONST_STRING_DECL(_kCFSystemVersionProductCopyrightKey, "ProductCopyright")
CONST_STRING_DECL(_kCFSystemVersionProductVersionKey, "ProductVersion")
CONST_STRING_DECL(_kCFSystemVersionProductVersionExtraKey, "ProductVersionExtra")
CONST_STRING_DECL(_kCFSystemVersionProductUserVisibleVersionKey, "ProductUserVisibleVersion")
CONST_STRING_DECL(_kCFSystemVersionBuildVersionKey, "ProductBuildVersion")
CONST_STRING_DECL(_kCFSystemVersionProductVersionStringKey, "Version")
CONST_STRING_DECL(_kCFSystemVersionBuildStringKey, "Build")


CF_EXPORT Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version) {
    return true;
}




#if DEPLOYMENT_TARGET_MACOSX
CF_PRIVATE void *__CFLookupCarbonCoreFunction(const char *name) {
    static void *image = NULL;
    if (NULL == image) {
	image = dlopen("/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/CarbonCore", RTLD_LAZY | RTLD_LOCAL);
    }
    void *dyfunc = NULL;
    if (image) {
	dyfunc = dlsym(image, name);
    }
    return dyfunc;
}
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
CF_PRIVATE void *__CFLookupCoreServicesInternalFunction(const char *name) {
    static void *image = NULL;
    if (NULL == image) {
        image = dlopen("/System/Library/PrivateFrameworks/CoreServicesInternal.framework/CoreServicesInternal", RTLD_LAZY | RTLD_LOCAL);
    }
    void *dyfunc = NULL;
    if (image) {
        dyfunc = dlsym(image, name);
    }
    return dyfunc;
}

CF_PRIVATE void *__CFLookupCFNetworkFunction(const char *name) {
    static void *image = NULL;
    if (NULL == image) {
	const char *path = NULL;
        if (!__CFProcessIsRestricted()) {
	    path = __CFgetenv("CFNETWORK_LIBRARY_PATH");
	}
	if (!path) {
	    path = "/System/Library/Frameworks/CFNetwork.framework/CFNetwork";
	}
	image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    }
    void *dyfunc = NULL;
    if (image) {
	dyfunc = dlsym(image, name);
    }
    return dyfunc;
}
#endif

CF_PRIVATE CFIndex __CFActiveProcessorCount() {
    int32_t pcnt;
#if DEPLOYMENT_TARGET_WINDOWS
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD_PTR activeProcessorMask = sysInfo.dwActiveProcessorMask;
    // assumes sizeof(DWORD_PTR) is 64 bits or less
    uint64_t v = activeProcessorMask;
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0xf0f0f0f0f0f0f0fULL;
    pcnt = (v * 0x0101010101010101ULL) >> ((sizeof(v) - 1) * 8);
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    int32_t mib[] = {CTL_HW, HW_AVAILCPU};
    size_t len = sizeof(pcnt);
    int32_t result = sysctl(mib, sizeof(mib) / sizeof(int32_t), &pcnt, &len, NULL, 0);
    if (result != 0) {
        pcnt = 0;
    }
#else
    // Assume the worst
    pcnt = 1;
#endif
    return pcnt;
}

CF_PRIVATE void __CFGetUGIDs(uid_t *euid, gid_t *egid) {
#if 1 && (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI)
    uid_t uid;
    gid_t gid;
    if (0 == pthread_getugid_np(&uid, &gid)) {
        if (euid) *euid = uid;
        if (egid) *egid = gid;
    } else
#endif
    {
        if (euid) *euid = geteuid();
        if (egid) *egid = getegid();
    }
}

const char *_CFPrintForDebugger(const void *obj) {
	static char *result = NULL;
	CFStringRef str;
	CFIndex cnt = 0;

	free(result);	// Let go of result from previous call.
	result = NULL;
	if (obj) {
		if (CFGetTypeID(obj) == CFStringGetTypeID()) {
			// Makes Ali marginally happier
			str = __CFCopyFormattingDescription(obj, NULL);
			if (!str) str = CFCopyDescription(obj);
		} else {
			str = CFCopyDescription(obj);
		}
	} else {
		str = (CFStringRef)CFRetain(CFSTR("(null)"));
	}
	
	if (str != NULL) {
		CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, FALSE, NULL, 0, &cnt);
	}
	result = (char *) malloc(cnt + 2);	// 1 for '\0', 1 for an optional '\n'
	if (str != NULL) {
		CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, FALSE, (UInt8 *) result, cnt, &cnt);
	}
	result[cnt] = '\0';

	if (str) CFRelease(str);
	return result;
}

static void _CFShowToFile(FILE *file, Boolean flush, const void *obj) {
     CFStringRef str;
     CFIndex idx, cnt;
     CFStringInlineBuffer buffer;
     bool lastNL = false;

     if (obj) {
	if (CFGetTypeID(obj) == CFStringGetTypeID()) {
	    // Makes Ali marginally happier
	    str = __CFCopyFormattingDescription(obj, NULL);
	    if (!str) str = CFCopyDescription(obj);
	} else {
	    str = CFCopyDescription(obj);
	}
     } else {
	str = (CFStringRef)CFRetain(CFSTR("(null)"));
     }
     cnt = CFStringGetLength(str);

#if DEPLOYMENT_TARGET_WINDOWS
    UniChar *ptr = (UniChar *)CFStringGetCharactersPtr(str);
    BOOL freePtr = false;
    if (!ptr) {
	CFIndex strLen = CFStringGetLength(str);
	// +2, 1 for newline, 1 for null
	CFIndex bufSize = sizeof(UniChar *) * (CFStringGetMaximumSizeForEncoding(strLen, kCFStringEncodingUnicode) + 2);
	CFIndex bytesUsed = 0;
	ptr = (UniChar *)malloc(bufSize);
	CFStringGetCharacters(str, CFRangeMake(0, strLen), ptr);
	ptr[strLen] = L'\n';
	ptr[strLen+1] = 0;
	freePtr = true;
    }
    OutputDebugStringW((wchar_t *)ptr);
    if (freePtr) free(ptr);
#else
     CFStringInitInlineBuffer(str, &buffer, CFRangeMake(0, cnt));
     for (idx = 0; idx < cnt; idx++) {
         UniChar ch = __CFStringGetCharacterFromInlineBufferQuick(&buffer, idx);
         if (ch < 128) {
             fprintf_l(file, NULL, "%c", ch);
             lastNL = (ch == '\n');
         } else {
             fprintf_l(file, NULL, "\\u%04x", ch);
         }
     }
     if (!lastNL) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
         fprintf_l(file, NULL, "\n");
#else
         fprintf(file, NULL, "\n");
#endif
         if (flush) fflush(file);
     }
#endif

     if (str) CFRelease(str);
}

void CFShow(const void *obj) {
     _CFShowToFile(stderr, true, obj);
}


// message must be a UTF8-encoded, null-terminated, byte buffer with at least length bytes
typedef void (*CFLogFunc)(int32_t lev, const char *message, size_t length, char withBanner);

static Boolean also_do_stderr() {
#if DEPLOYMENT_TARGET_EMBEDDED_MINI
    // just log to stderr, other logging facilities are out
    return true;
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    if (!issetugid() && __CFgetenv("CFLOG_FORCE_STDERR")) {
	return true;
    }
    struct stat sb;
    int ret = fstat(STDERR_FILENO, &sb);
    if (ret < 0) return false;
    mode_t m = sb.st_mode & S_IFMT;
    if (S_IFREG == m || S_IFSOCK == m) return true;
    if (!(S_IFIFO == m || S_IFCHR == m)) return false; // disallow any whacky stuff
#if 0 // launchd no longer repeats everything it hears
    // if it could be a pipe back to launchd, fail
    int64_t val = 0;
    // assumes val is not written to on error
    vproc_swap_integer(NULL, VPROC_GSK_IS_MANAGED, NULL, &val);
    if (val) return false;
#endif
#endif
    return true;
}

extern char *__CFBundleMainID;

static void __CFLogCString(int32_t lev, const char *message, size_t length, char withBanner) {
    char *banner = NULL;
    char *time = NULL;
    char *thread = NULL;
    char *uid = NULL;
    int bannerLen;
    bannerLen = 0;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS || DEPLOYMENT_TARGET_LINUX
    // The banner path may use CF functions, but the rest of this function should not. It may be called at times when CF is not fully setup or torn down.
    if (withBanner) {
	double dummy;
	CFAbsoluteTime at = CFAbsoluteTimeGetCurrent();
        time_t tv = floor(at + kCFAbsoluteTimeIntervalSince1970);
        struct tm mine;
        localtime_r(&tv, &mine);
        int32_t year = mine.tm_year + 1900;
        int32_t month = mine.tm_mon + 1;
        int32_t day = mine.tm_mday;
        int32_t hour = mine.tm_hour;
        int32_t minute = mine.tm_min;
        int32_t second = mine.tm_sec;
	int32_t ms = (int32_t)floor(1000.0 * modf(at, &dummy));
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        uint64_t tid = 0;
        if (0 != pthread_threadid_np(NULL, &tid)) tid = pthread_mach_thread_np(pthread_self());
        asprintf(&banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%llu] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), tid);
	asprintf(&thread, "%x", pthread_mach_thread_np(pthread_self()));
#elif DEPLOYMENT_TARGET_WINDOWS
	bannerLen = asprintf(&banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%x] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), GetCurrentThreadId());
	asprintf(&thread, "%x", GetCurrentThreadId());
#else
	bannerLen = asprintf(&banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%x] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), (unsigned int)pthread_self());
	asprintf(&thread, "%lx", pthread_self());
#endif
	asprintf(&time, "%04d-%02d-%02d %02d:%02d:%02d.%03d", year, month, day, hour, minute, second, ms);

    }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
    uid_t euid;
    __CFGetUGIDs(&euid, NULL);
    asprintf(&uid, "%d", euid);
    aslclient asl = asl_open(NULL, __CFBundleMainID[0] ? __CFBundleMainID : "com.apple.console", ASL_OPT_NO_DELAY);
    aslmsg msg = asl_new(ASL_TYPE_MSG);
    asl_set(msg, "CFLog Local Time", time); // not to be documented, not public API
    asl_set(msg, "CFLog Thread", thread);   // not to be documented, not public API
    asl_set(msg, "ReadUID", uid);
    static const char *levstr[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    asl_set(msg, ASL_KEY_LEVEL, levstr[lev]);
    asl_set(msg, ASL_KEY_MSG, message);
    asl_send(asl, msg);
    asl_free(msg);
    asl_close(asl);
#endif
#endif // DEPLOYMENT_TARGET

    if (also_do_stderr()) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
	struct iovec v[3];
	v[0].iov_base = banner;
	v[0].iov_len = banner ? strlen(banner) : 0;
	v[1].iov_base = (char *)message;
	v[1].iov_len = length;
	v[2].iov_base = "\n";
	v[2].iov_len = (message[length - 1] != '\n') ? 1 : 0;
	int nv = (v[0].iov_base ? 1 : 0) + 1 + (v[2].iov_len ? 1 : 0);
	static CFLock_t lock = CFLockInit;
	__CFLock(&lock);
	writev(STDERR_FILENO, v[0].iov_base ? v : v + 1, nv);
	__CFUnlock(&lock);
#elif DEPLOYMENT_TARGET_WINDOWS
        size_t bufLen = bannerLen + length + 1;
        char *buf = (char *)malloc(sizeof(char) * bufLen);
        if (banner) {
            // Copy the banner into the debug string
            memmove_s(buf, bufLen, banner, bannerLen);
            
            // Copy the message into the debug string
            strcpy_s(buf + bannerLen, bufLen - bannerLen, message);
        } else {
            strcpy_s(buf, bufLen, message);
        }
        buf[bufLen - 1] = '\0';
	fprintf_s(stderr, "%s\n", buf);
	// This Win32 API call only prints when a debugger is active
	// OutputDebugStringA(buf);
        free(buf);
#else
        size_t bufLen = bannerLen + length + 1;
        char *buf = (char *)malloc(sizeof(char) * bufLen);
        if (banner) {
            // Copy the banner into the debug string
            memmove(buf, banner, bannerLen);
            
            // Copy the message into the debug string
            strncpy(buf + bannerLen, message, bufLen - bannerLen);
        } else {
            strncpy(buf, message, bufLen);
        }
        buf[bufLen - 1] = '\0';
        fprintf(stderr, "%s\n", buf);
        free(buf);
#endif
    }
    
    if (thread) free(thread);
    if (time) free(time);
    if (banner) free(banner);
    if (uid) free(uid);
}
    
CF_EXPORT void _CFLogvEx2(CFLogFunc logit, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, int32_t lev, CFStringRef format, va_list args) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    uintptr_t val = (uintptr_t)_CFGetTSD(__CFTSDKeyIsInCFLog);
    if (3 < val) return; // allow up to 4 nested invocations
    _CFSetTSD(__CFTSDKeyIsInCFLog, (void *)(val + 1), NULL);
#endif
    CFStringRef str = format ? _CFStringCreateWithFormatAndArgumentsAux2(kCFAllocatorSystemDefault, copyDescFunc, contextDescFunc, formatOptions, (CFStringRef)format, args) : 0;
    CFIndex blen = str ? CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + 1 : 0;
    char *buf = str ? (char *)malloc(blen) : 0;
    if (str && buf) {
	Boolean converted = CFStringGetCString(str, buf, blen, kCFStringEncodingUTF8);
	size_t len = strlen(buf);
	// silently ignore 0-length or really large messages, and levels outside the valid range
	if (converted && !(len <= 0 || (1 << 24) < len) && !(lev < ASL_LEVEL_EMERG || ASL_LEVEL_DEBUG < lev)) {
	    (logit ? logit : __CFLogCString)(lev, buf, len, 1);
	}
    }
    if (buf) free(buf);
    if (str) CFRelease(str);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    _CFSetTSD(__CFTSDKeyIsInCFLog, (void *)val, NULL);
#endif
}
    
CF_EXPORT void _CFLogvEx(CFLogFunc logit, CFStringRef (*copyDescFunc)(void *, const void *), CFDictionaryRef formatOptions, int32_t lev, CFStringRef format, va_list args) {
    _CFLogvEx2(logit, copyDescFunc, NULL, formatOptions, lev, format, args);
}

// This CF-only log function uses no CF functionality, so it may be called anywhere within CF - including thread teardown or prior to full CF setup
CF_PRIVATE void _CFLogSimple(int32_t lev, char *format, ...) {
    va_list args;
    va_start(args, format);
    char formattedMessage[1024];
    int length = vsnprintf(formattedMessage, 1024, format, args);
    if (length > 0) {
        __CFLogCString(lev, formattedMessage, length, 0);
    }
    va_end(args);
}

void CFLog(int32_t lev, CFStringRef format, ...) {
    va_list args;
    va_start(args, format); 
    _CFLogvEx2(NULL, NULL, NULL, NULL, lev, format, args);
    va_end(args);
}



#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED

kern_return_t _CFDiscorporateMemoryAllocate(CFDiscorporateMemory *hm, size_t size, bool purgeable) {
    kern_return_t ret = KERN_SUCCESS;
    size = round_page(size);
    if (0 == size) size = vm_page_size;
    memset(hm, 0, sizeof(CFDiscorporateMemory));
    void *addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, VM_MAKE_TAG(0) | (purgeable ? VM_FLAGS_PURGABLE : 0), 0);
    if ((uintptr_t)addr == -1) {
        ret = KERN_NO_SPACE;
    }
    if (KERN_SUCCESS == ret) {
        hm->address = (mach_vm_address_t)(uintptr_t)addr;
        hm->size = (mach_vm_size_t)size;
        hm->port = MACH_PORT_NULL;
        hm->corporeal = true;
        hm->purgeable = purgeable;
    }
    if (KERN_SUCCESS == ret) ret = mach_make_memory_entry_64(mach_task_self(), &hm->size, hm->address, VM_PROT_DEFAULT, &hm->port, MACH_PORT_NULL);
    if (KERN_SUCCESS == ret) hm->corporeal = true;
    return ret;
}

kern_return_t _CFDiscorporateMemoryDeallocate(CFDiscorporateMemory *hm) {
    kern_return_t ret1 = KERN_SUCCESS, ret2 = KERN_SUCCESS;
    if (hm->corporeal) ret1 = mach_vm_deallocate(mach_task_self(), hm->address, hm->size);
    hm->address = MACH_VM_MIN_ADDRESS;
    hm->corporeal = false;
    ret2 = mach_port_deallocate(mach_task_self(), hm->port);
    hm->port = MACH_PORT_NULL;
    return ret1 != KERN_SUCCESS ? ret1 : ret2;
}

kern_return_t _CFDiscorporateMemoryDematerialize(CFDiscorporateMemory *hm) {
    kern_return_t ret = KERN_SUCCESS;
    if (!hm->corporeal) ret = KERN_INVALID_MEMORY_CONTROL;
    int state = VM_PURGABLE_VOLATILE;
    if (KERN_SUCCESS == ret) vm_purgable_control(mach_task_self(), (vm_address_t)hm->address, VM_PURGABLE_SET_STATE, &state);
    if (KERN_SUCCESS == ret) ret = mach_vm_deallocate(mach_task_self(), hm->address, hm->size);
    if (KERN_SUCCESS == ret) hm->address = MACH_VM_MIN_ADDRESS;
    if (KERN_SUCCESS == ret) hm->corporeal = false;
    return ret;
}

kern_return_t _CFDiscorporateMemoryMaterialize(CFDiscorporateMemory *hm) {
    kern_return_t ret = KERN_SUCCESS;
    if (hm->corporeal) ret = KERN_INVALID_MEMORY_CONTROL;
    if (KERN_SUCCESS == ret) ret = mach_vm_map(mach_task_self(), &hm->address, hm->size, 0, VM_FLAGS_ANYWHERE, hm->port, 0, FALSE, VM_PROT_DEFAULT, VM_PROT_DEFAULT, VM_INHERIT_DEFAULT);
    if (KERN_SUCCESS == ret) hm->corporeal = true;
    int state = VM_PURGABLE_NONVOLATILE;
    if (KERN_SUCCESS == ret) ret = vm_purgable_control(mach_task_self(), (vm_address_t)hm->address, VM_PURGABLE_SET_STATE, &state);
    if (KERN_SUCCESS == ret) if (VM_PURGABLE_EMPTY == state) ret = KERN_PROTECTION_FAILURE; // same as VM_PURGABLE_EMPTY
    return ret;
}

#endif

#if DEPLOYMENT_TARGET_MACOSX

#define SUDDEN_TERMINATION_ENABLE_VPROC 1

#if SUDDEN_TERMINATION_ENABLE_VPROC

static OSSpinLock __CFProcessKillingLock = OS_SPINLOCK_INIT;
static CFIndex __CFProcessKillingDisablingCount = 1;
static Boolean __CFProcessKillingWasTurnedOn = false;

void _CFSuddenTerminationDisable(void) {
    OSSpinLockLock(&__CFProcessKillingLock);
    __CFProcessKillingDisablingCount++;
    _vproc_transaction_begin();
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationEnable(void) {
    // In our model the first call of _CFSuddenTerminationEnable() that does not balance a previous call of _CFSuddenTerminationDisable() actually enables sudden termination so we have to keep a count that's almost redundant with vproc's.
    OSSpinLockLock(&__CFProcessKillingLock);
    __CFProcessKillingDisablingCount--;
    if (__CFProcessKillingDisablingCount==0 && !__CFProcessKillingWasTurnedOn) {
	_vproc_transactions_enable();
	__CFProcessKillingWasTurnedOn = true;
    } else {
	// Mail seems to have sudden termination disabling/enabling imbalance bugs that make _vproc_transaction_end() kill the app but we don't want that to prevent our submission of the fix 6382488.
	if (__CFProcessKillingDisablingCount>=0) {
	    _vproc_transaction_end();
	} else {
	    CFLog(kCFLogLevelError, CFSTR("-[NSProcessInfo enableSuddenTermination] has been invoked more times than necessary to balance invocations of -[NSProcessInfo disableSuddenTermination]. Ignoring."));
	}
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationExitIfTerminationEnabled(int exitStatus) {
    // This is for when the caller wants to try to exit quickly if possible but not automatically exit the process when it next becomes clean, because quitting might still be cancelled by the user.
    OSSpinLockLock(&__CFProcessKillingLock);
    // Check _vproc_transaction_count() because other code in the process might go straight to the vproc APIs but also check __CFProcessKillingWasTurnedOn because  _vproc_transaction_count() can return 0 when transactions didn't even get enabled.
    if (__CFProcessKillingWasTurnedOn) {
        _vproc_transaction_try_exit(exitStatus);
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationExitWhenTerminationEnabled(int exitStatus) {
    // The user has had their final opportunity to cancel quitting. Exit as soon as the process is clean. Same carefulness as in _CFSuddenTerminationExitIfTerminationEnabled().
    OSSpinLockLock(&__CFProcessKillingLock);
    if (__CFProcessKillingWasTurnedOn) {
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

size_t _CFSuddenTerminationDisablingCount(void) {
    // Until sudden termination has been really enabled vproc's notion of the count is off by one but we can't just return __CFProcessKillingDisablingCount() because that doesn't take into account stuff that calls the vproc_transaction functions behind our back.
    return _vproc_transaction_count() + (__CFProcessKillingWasTurnedOn ? 0 : 1);
}

#else

#warning Building with vproc sudden termination API disabled.

static OSSpinLockUnlock __CFProcessKillingLock = OS_SPINLOCK_INIT;
static size_t __CFProcessKillingDisablingCount = 1;
static Boolean __CFProcessExitNextTimeKillingIsEnabled = false;
static int32_t __CFProcessExitStatus = 0;
static int __CFProcessIsKillableNotifyToken;
static Boolean __CFProcessIsKillableNotifyTokenIsFigured = false;

CF_PRIVATE void _CFSetSuddenTerminationEnabled(Boolean isEnabled) {
    if (!__CFProcessIsKillableNotifyTokenIsFigured) {
        char *notificationName = NULL;
        asprintf(&notificationName, "com.apple.isKillable.%i", getpid());
        uint32_t notifyResult = notify_register_check(notificationName, &__CFProcessIsKillableNotifyToken);
        if (notifyResult != NOTIFY_STATUS_OK) {
            CFLog(kCFLogLevelError, CFSTR("%s: notify_register_check() returned %i."), __PRETTY_FUNCTION__, notifyResult);
        }
        free(notificationName);
        __CFProcessIsKillableNotifyTokenIsFigured = true;
    }
    uint32_t notifyResult = notify_set_state(__CFProcessIsKillableNotifyToken, isEnabled);
    if (notifyResult != NOTIFY_STATUS_OK) {
        CFLog(kCFLogLevelError, CFSTR("%s: notify_set_state() returned %i"), __PRETTY_FUNCTION__, notifyResult);
    }
}

void _CFSuddenTerminationDisable(void) {
    OSSpinLockLock(&__CFProcessKillingLock);
    if (__CFProcessKillingDisablingCount == 0) {
        _CFSetSuddenTerminationEnabled(false);
    }
    __CFProcessKillingDisablingCount++;
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationEnable(void) {
    OSSpinLockLock(&__CFProcessKillingLock);
    __CFProcessKillingDisablingCount--;
    if (__CFProcessKillingDisablingCount == 0) {
        if (__CFProcessExitNextTimeKillingIsEnabled) {
            _exit(__CFProcessExitStatus);
        } else {
            _CFSetSuddenTerminationEnabled(true);
        }
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationExitIfTerminationEnabled(int exitStatus) {
    OSSpinLockLock(&__CFProcessKillingLock);
    if (__CFProcessKillingDisablingCount == 0) {
        _exit(exitStatus);
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

void _CFSuddenTerminationExitWhenTerminationEnabled(int exitStatus) {
    OSSpinLockLock(&__CFProcessKillingLock);
    if (__CFProcessKillingDisablingCount == 0) {
        _exit(exitStatus);
    } else {
        __CFProcessExitNextTimeKillingIsEnabled = YES;
        __CFProcessExitStatus = exitStatus;
    }
    OSSpinLockUnlock(&__CFProcessKillingLock);
}

size_t _CFSuddenTerminationDisablingCount(void) {
    return __CFProcessKillingDisablingCount;
}

#endif

#endif

#if 0
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI

typedef void (^ThrottleTypeA)(void);		// allows calls per nanoseconds
typedef void (^ThrottleTypeB)(uint64_t amt);	// allows amount per nanoseconds

CF_PRIVATE ThrottleTypeA __CFCreateThrottleTypeA(uint16_t calls, uint64_t nanoseconds) {
   struct mach_timebase_info info;
   mach_timebase_info(&info);
   uint64_t period = nanoseconds / info.numer * info.denom;

   if (0 == calls || 0 == period) return NULL;

   __block OSSpinLock b_lock = OS_SPINLOCK_INIT;
   __block uint64_t b_values[calls];
   __block uint64_t *b_oldest = b_values;
   memset(b_values, 0, sizeof(b_values));

   return Block_copy(^{
               uint64_t curr_time = mach_absolute_time();
               OSSpinLockLock(&b_lock);
               uint64_t next_time = *b_oldest + period;
               *b_oldest = (curr_time < next_time) ? next_time : curr_time;
               b_oldest++;
               if (b_values + calls <= b_oldest) b_oldest = b_values;
               OSSpinLockUnlock(&b_lock);
               if (curr_time < next_time) {
                   mach_wait_until(next_time);
               }
           });
}

CF_PRIVATE ThrottleTypeB __CFCreateThrottleTypeB(uint64_t amount, uint64_t nanoseconds) {
   struct mach_timebase_info info;
   mach_timebase_info(&info);
   uint64_t period = nanoseconds / info.numer * info.denom;

   if (0 == amount || 0 == period) return NULL;

   __block OSSpinLock b_lock = OS_SPINLOCK_INIT;
   __block uint64_t b_sum = 0ULL;
   __block uint16_t b_num_values = 8;
   __block uint64_t *b_values = calloc(b_num_values, 2 * sizeof(uint64_t));
   __block uint64_t *b_oldest = b_values;

   return Block_copy(^(uint64_t amt){
               OSSpinLockLock(&b_lock);
// unimplemented
               OSSpinLockUnlock(&b_lock);
           });
}

#endif
#endif

#pragma mark File Reading
    
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#if DEPLOYMENT_TARGET_WINDOWS
#include <io.h>
#include <direct.h>
#define close _close
#define write _write
#define read _read
#define open _NS_open
#define stat _NS_stat
#define fstat _fstat
#define statinfo _stat
    
#define mach_task_self() 0

#else
#define statinfo stat
#endif
    
static CFErrorRef _CFErrorWithFilePathCodeDomain(CFStringRef domain, CFIndex code, CFStringRef path) {
    CFStringRef key = CFSTR("NSFilePath");
    CFDictionaryRef userInfo = CFDictionaryCreate(kCFAllocatorSystemDefault, (const void **)&key, (const void **)&path, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFErrorRef result = CFErrorCreate(kCFAllocatorSystemDefault, domain, code, userInfo);
    CFRelease(userInfo);
    return result;
}

// Caller is responsible for freeing memory. munmap() if map == true, else malloc().
CF_PRIVATE Boolean _CFReadMappedFromFile(CFStringRef path, Boolean map, Boolean uncached, void **outBytes, CFIndex *outLength, CFErrorRef *errorPtr) {
    void *bytes = NULL;
    unsigned long length;
    char cpath[CFMaxPathSize];
    if (!CFStringGetFileSystemRepresentation(path, cpath, CFMaxPathSize)) {
        // TODO: real error codes
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainCocoa, -1, path);
	return false;
    }

    struct statinfo statBuf;
    int32_t fd = -1;

    fd = open(cpath, O_RDONLY|CF_OPENFLGS, 0666);
    if (fd < 0) {
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, errno, path);
        return false;
    }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI    
    if (uncached) (void)fcntl(fd, F_NOCACHE, 1);  // Non-zero arg turns off caching; we ignore error as uncached is just a hint
#endif
    if (fstat(fd, &statBuf) < 0) {
        int32_t savederrno = errno;
        close(fd);
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, savederrno, path);
        return false;
    }
    if ((statBuf.st_mode & S_IFMT) != S_IFREG) {
        close(fd);
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, EACCES, path);
        return false;
    }
    if (statBuf.st_size < 0LL) {	// too small
        close(fd);
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, ENOMEM, path);
        return false;
    }
#if __LP64__
#else
    if (statBuf.st_size > (1LL << 31)) {	// refuse to do more than 2GB
        close(fd);
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, EFBIG, path);
        return false;
    }
#endif

    if (0LL == statBuf.st_size) {
        bytes = malloc(8); // don't return constant string -- it's freed!
	length = 0;
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
    } else if (map) {
        if((void *)-1 == (bytes = mmap(0, (size_t)statBuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {
	    int32_t savederrno = errno;
	    close(fd);
            if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, savederrno, path);
	    return false;
	}
	length = (unsigned long)statBuf.st_size;
    } else {
        bytes = malloc(statBuf.st_size);
        if (bytes == NULL) {
            close(fd);
            if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, ENOMEM, path);
            return false;
        }
	size_t numBytesRemaining = (size_t)statBuf.st_size;
	void *readLocation = bytes;
	while (numBytesRemaining > 0) {
	    size_t numBytesRequested = (numBytesRemaining < (1LL << 31)) ? numBytesRemaining : ((1LL << 31) - 1);	// This loop is basically a workaround for 4870206 
	    ssize_t numBytesRead = read(fd, readLocation, numBytesRequested);
	    if (numBytesRead <= 0) {
		if (numBytesRead < 0) {
		    int32_t savederrno = errno;
                    free(bytes);
		    close(fd);
                    if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, savederrno, path);
		    bytes = NULL;
		    return false;
		} else {
		    // This is a bizarre case; 0 bytes read. Might indicate end-of-file?
		    break;
		}
	    } else {
		readLocation += numBytesRead;
		numBytesRemaining -= numBytesRead;
	    }
	}
	length = (unsigned long)statBuf.st_size - numBytesRemaining;
    }
#elif DEPLOYMENT_TARGET_WINDOWS
    } else {
        bytes = malloc(statBuf.st_size);
        DWORD numBytesRead;
        if (!ReadFile((HANDLE)_get_osfhandle(fd), bytes, statBuf.st_size, &numBytesRead, NULL)) {
            DWORD lastError = GetLastError();
            if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, lastError, path);
	    free(bytes);
	    close(fd);
	    errno = lastError;
	    bytes = NULL;
	    return false;
        }
	length = numBytesRead;
    }
#endif
    close(fd);
    *outBytes = bytes;
    *outLength = length;
    return true;
}
