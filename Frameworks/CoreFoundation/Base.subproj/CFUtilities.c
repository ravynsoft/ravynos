/*	CFUtilities.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Tony Parker
*/

#if __has_include(<xpc/xpc_transaction_deprecate.h>)
#import <xpc/xpc_transaction_deprecate.h>
#undef XPC_TRANSACTION_DEPRECATED
#define XPC_TRANSACTION_DEPRECATED
#endif

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include "CFLocaleInternal.h"
#if !TARGET_OS_WASI
#include "CFBundle_Internal.h"
#endif
#include <CoreFoundation/CFPriv.h>
#if TARGET_OS_MAC || TARGET_OS_WIN32
#include <CoreFoundation/CFBundle.h>
#endif
#include <CoreFoundation/CFURLAccess.h>
#if !TARGET_OS_WASI
#include <CoreFoundation/CFPropertyList.h>
#endif
#if TARGET_OS_WIN32
#include <process.h>
#endif
#if TARGET_OS_ANDROID
#include <android/log.h>
#endif
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if TARGET_OS_MAC
#include <asl.h>
#else
#define ASL_LEVEL_EMERG 0
#define ASL_LEVEL_DEBUG 7
#endif


#if TARGET_OS_MAC && !__RAVYNOS__
#include <unistd.h>
#include <sys/uio.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <crt_externs.h>
#include <dlfcn.h>
#include <vproc.h>
#include <libproc.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/errno.h>
#include <mach/mach_time.h>
#include <Block.h>
#include <os/lock.h>
#elif __RAVYNOS__
#include <dlfcn.h>
#endif

#if TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI || __RAVYNOS__
#include <string.h>
#include <sys/mman.h>
#endif

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif
#if _POSIX_THREADS
#include <pthread.h>
#endif

#if TARGET_OS_LINUX
#ifdef HAVE_SCHED_GETAFFINITY
#include <sched.h>
#endif
#endif


#if !defined(CF_HAVE_HW_CONFIG_COMMPAGE) && defined(_COMM_PAGE_LOGICAL_CPUS) && defined(_COMM_PAGE_PHYSICAL_CPUS) && defined(_COMM_PAGE_ACTIVE_CPUS) && !__has_feature(address_sanitizer)
#define CF_HAVE_HW_CONFIG_COMMPAGE 1
#else
#define CF_HAVE_HW_CONFIG_COMMPAGE 0
#endif

CF_PRIVATE os_log_t _CFOSLog(void) {
    static os_log_t logger = NULL;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = os_log_create("com.apple.foundation", "general");
    });
    return logger;
}

CF_PRIVATE os_log_t _CFMethodSignatureROMLog(void) {
    static os_log_t logger;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = os_log_create("com.apple.foundation", "MethodSignatureROM");
    });
    return logger;
}

CF_PRIVATE os_log_t _CFRuntimeIssuesLog(void) {
    static os_log_t logger;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = os_log_create(OS_LOG_SUBSYSTEM_RUNTIME_ISSUES, "CoreFoundation");
    });
    return logger;
}

CF_PRIVATE os_log_t _CFFoundationRuntimeIssuesLog(void) {
    static os_log_t logger;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        logger = os_log_create(OS_LOG_SUBSYSTEM_RUNTIME_ISSUES, "Foundation");
    });
    return logger;
}

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


#if TARGET_OS_MAC && !__RAVYNOS__
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

CF_PRIVATE CFDataRef _CFDataCreateFromURL(CFURLRef resourceURL, CFErrorRef *error) {
    CFDataRef result = NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    SInt32 errorCode = 0;
    if (!CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, resourceURL, &result, NULL, NULL, &errorCode)) {
        if (error) {
            // This error domain is not quite right, but it's better than Cocoa
            *error = CFErrorCreate(kCFAllocatorSystemDefault, kCFErrorDomainOSStatus, errorCode, NULL);
            return NULL;
        }
    }
#pragma GCC diagnostic pop
    return result;
}

static CFStringRef copySystemVersionPath(CFStringRef suffix) {
#if TARGET_IPHONE_SIMULATOR
    const char *simulatorRoot = getenv("IPHONE_SIMULATOR_ROOT");
    if (!simulatorRoot) simulatorRoot = getenv("CFFIXED_USER_HOME");
    if (!simulatorRoot) simulatorRoot = "/";
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%s%@"), simulatorRoot, suffix);
#else
    return CFStringCreateCopy(kCFAllocatorSystemDefault, suffix);
#endif
}

// Looks for localized version of "nonLocalized" in the SystemVersion bundle
// If not found, and returnNonLocalizedFlag == true, will return the non localized string (retained of course), otherwise NULL
// If bundlePtr != NULL, will use *bundlePtr and will return the bundle in there; otherwise bundle is created and released
#if TARGET_OS_MAC
static CFStringRef _CFCopyLocalizedVersionKey(CFBundleRef *bundlePtr, CFStringRef nonLocalized) {
    CFStringRef localized = NULL;
    CFBundleRef locBundle = bundlePtr ? *bundlePtr : NULL;
    if (!locBundle) {
        CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/SystemVersion.bundle"));
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, path, kCFURLPOSIXPathStyle, false);
        CFRelease(path);
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

#if TARGET_OS_WIN32
static BOOL _CFGetWindowsSystemVersion(OSVERSIONINFOEX *osvi) {
    ZeroMemory(osvi, sizeof(OSVERSIONINFOEX));
    osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    return GetVersionEx((OSVERSIONINFO *)osvi);
}
#endif

static CFDictionaryRef _CFCopyVersionDictionary(CFStringRef path) {
    CFPropertyListRef plist = NULL;
    
#if TARGET_OS_MAC
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
	CFBundleRef locBundle = NULL;
	CFStringRef fullVersion, vers, versExtra, build;
	CFStringRef versionString = _CFCopyLocalizedVersionKey(&locBundle, _kCFSystemVersionProductVersionStringKey);
	CFStringRef buildString = _CFCopyLocalizedVersionKey(&locBundle, _kCFSystemVersionBuildStringKey);
	CFStringRef fullVersionString = _CFCopyLocalizedVersionKey(&locBundle, CFSTR("FullVersionString"));
	if (locBundle) CFRelease(locBundle);

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
#elif TARGET_OS_WIN32
    OSVERSIONINFOEX osvi;
    if (!_CFGetWindowsSystemVersion(&osvi)) return NULL;

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

#if !TARGET_OS_WASI
CFStringRef _CFCopySystemVersionDictionaryValue(CFStringRef key) {
    CFStringRef versionString;
    CFDictionaryRef dict = _CFCopyServerVersionDictionary();
    if (!dict) dict = _CFCopySystemVersionDictionary();
    if (!dict) return NULL;
    versionString = (CFStringRef)CFDictionaryGetValue(dict, key);
    if (versionString) CFRetain(versionString);
    CFRelease(dict);
    return versionString;
}

CFStringRef CFCopySystemVersionString(void) {
    return _CFCopySystemVersionDictionaryValue(CFSTR("FullVersionString"));
}

CFDictionaryRef _CFCopySystemVersionDictionary(void) {
    static dispatch_once_t onceToken;
    static CFDictionaryRef result = NULL;
    dispatch_once(&onceToken, ^{
        // TODO: Populate this dictionary differently on non-Darwin platforms
        CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/SystemVersion.plist"));
        CFPropertyListRef plist = _CFCopyVersionDictionary(path);
        CFRelease(path);
        result = (CFDictionaryRef)plist;
    });
    if (result) {
        return CFRetain(result);
    } else {
        return NULL;
    }
}

CFDictionaryRef _CFCopySystemVersionPlatformDictionary(void) {
    static dispatch_once_t onceToken;
    static CFDictionaryRef result = NULL;
    dispatch_once(&onceToken, ^{
        CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/.SystemVersionPlatform.plist"));
        CFPropertyListRef plist = _CFCopyVersionDictionary(path);
        CFRelease(path);
        if (!plist) {
            // Fall back to the normal one in case the .SystemVersionPlatform.plist symlink is missing
            plist = _CFCopySystemVersionDictionary();
        }
        result = (CFDictionaryRef)plist;
    });
    if (result) {
        return CFRetain(result);
    } else {
        return NULL;
    }
}

CFDictionaryRef _CFCopyServerVersionDictionary(void) {
    static dispatch_once_t onceToken;
    static CFDictionaryRef result = NULL;
    dispatch_once(&onceToken, ^{
        CFStringRef path = copySystemVersionPath(CFSTR("/System/Library/CoreServices/ServerVersion.plist"));
        CFPropertyListRef plist = _CFCopyVersionDictionary(path);
        CFRelease(path);
        result = (CFDictionaryRef)plist;
    });
    if (result) {
        return CFRetain(result);
    } else {
        return NULL;
    }
}

static CFOperatingSystemVersion _CFCalculateOSVersion(void) {
    CFOperatingSystemVersion versionStruct = {-1, 0, 0};
#if TARGET_OS_WIN32
    OSVERSIONINFOEX windowsVersion;
    if (_CFGetWindowsSystemVersion(&windowsVersion)) {
        versionStruct.majorVersion = windowsVersion.dwMajorVersion;
        versionStruct.minorVersion = windowsVersion.dwMinorVersion;
        versionStruct.patchVersion = windowsVersion.dwBuildNumber;
    }
#else
    CFStringRef resolvedProductVersionKey = _kCFSystemVersionProductVersionKey;
    CFStringRef productVersion = _CFCopySystemVersionDictionaryValue(resolvedProductVersionKey);
    if (productVersion) {
        CFArrayRef components = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, productVersion, CFSTR("."));
        if (components) {
            const CFIndex cnt = CFArrayGetCount(components);
            if (0 < cnt) {
                versionStruct.majorVersion = CFStringGetIntValue(CFArrayGetValueAtIndex(components, 0));
                if (1 < cnt) {
                    versionStruct.minorVersion = CFStringGetIntValue(CFArrayGetValueAtIndex(components, 1));
                    if (2 < cnt) {
                        versionStruct.patchVersion = CFStringGetIntValue(CFArrayGetValueAtIndex(components, 2));
                    }
                }
            }
            CFRelease(components);
        }
        CFRelease(productVersion);
    }
#endif
    return versionStruct;
}

CFOperatingSystemVersion _CFOperatingSystemVersionGetCurrent(void) {
    static CFOperatingSystemVersion version;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        version = _CFCalculateOSVersion();
    });
    return version;
}

Boolean _CFOperatingSystemVersionIsAtLeastVersion(CFOperatingSystemVersion version) {
    const CFOperatingSystemVersion ourVersion = _CFOperatingSystemVersionGetCurrent();
    if (ourVersion.majorVersion < version.majorVersion) return false;
    if (ourVersion.majorVersion > version.majorVersion) return true;
    if (ourVersion.minorVersion < version.minorVersion) return false;
    if (ourVersion.minorVersion > version.minorVersion) return true;
    if (ourVersion.patchVersion < version.patchVersion) return false;
    if (ourVersion.patchVersion > version.patchVersion) return true;
    return true;
}

CONST_STRING_DECL(_kCFSystemVersionProductNameKey, "ProductName")
CONST_STRING_DECL(_kCFSystemVersionProductCopyrightKey, "ProductCopyright")
CONST_STRING_DECL(_kCFSystemVersionProductVersionKey, "ProductVersion")
CONST_STRING_DECL(_kCFSystemVersionProductVersionExtraKey, "ProductVersionExtra")
CONST_STRING_DECL(_kCFSystemVersionProductUserVisibleVersionKey, "ProductUserVisibleVersion")
CONST_STRING_DECL(_kCFSystemVersionBuildVersionKey, "ProductBuildVersion")
CONST_STRING_DECL(_kCFSystemVersionProductVersionStringKey, "Version")
CONST_STRING_DECL(_kCFSystemVersionBuildStringKey, "Build")
#endif

CF_EXPORT Boolean _CFExecutableLinkedOnOrAfter(CFSystemVersion version) {
    return true;
}

#if TARGET_OS_OSX
CF_PRIVATE void *__CFLookupCarbonCoreFunction(const char *name) {
    static void *image = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        image = dlopen("/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/CarbonCore", RTLD_LAZY | RTLD_LOCAL);
    });
    void *dyfunc = NULL;
    if (image) {
	dyfunc = dlsym(image, name);
    }
    return dyfunc;
}
#endif

#if TARGET_OS_MAC && !__RAVYNOS__
CF_PRIVATE void *__CFLookupCoreServicesInternalFunction(const char *name) {
    static void *image = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        image = dlopen("/System/Library/PrivateFrameworks/CoreServicesInternal.framework/CoreServicesInternal", RTLD_LAZY | RTLD_LOCAL);
    });
    void *dyfunc = NULL;
    if (image) {
        dyfunc = dlsym(image, name);
    }
    return dyfunc;
}

CF_PRIVATE void *__CFLookupCFNetworkFunction(const char *name) {
    static void *image = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        const char *path = __CFgetenvIfNotRestricted("CFNETWORK_LIBRARY_PATH");
        if (!path) {
            path = "/System/Library/Frameworks/CFNetwork.framework/CFNetwork";
        }
        image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    });
    void *dyfunc = NULL;
    if (image) {
	dyfunc = dlsym(image, name);
    }
    return dyfunc;
}
#endif

// TSAN does not know about the commpage: [39153032]
__attribute__((no_sanitize_thread))
CF_PRIVATE CFIndex __CFActiveProcessorCount(void) {
#if CF_HAVE_HW_CONFIG_COMMPAGE
    static const uintptr_t p = _COMM_PAGE_ACTIVE_CPUS;
    return (CFIndex)(*(uint8_t*)p);
#else
    
    int32_t pcnt;
#if TARGET_OS_WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD_PTR activeProcessorMask = sysInfo.dwActiveProcessorMask;
    // assumes sizeof(DWORD_PTR) is 64 bits or less
    uint64_t v = activeProcessorMask;
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0xf0f0f0f0f0f0f0fULL;
    pcnt = (v * 0x0101010101010101ULL) >> ((sizeof(v) - 1) * 8);
#elif TARGET_OS_MAC && !__RAVYNOS__
    int32_t mib[] = {CTL_HW, HW_AVAILCPU};
    size_t len = sizeof(pcnt);
    int32_t result = sysctl(mib, sizeof(mib) / sizeof(int32_t), &pcnt, &len, NULL, 0);
    if (result != 0) {
        pcnt = 0;
    }
#elif TARGET_OS_LINUX || TARGET_OS_BSD || __RAVYNOS__

#ifdef HAVE_SCHED_GETAFFINITY
    cpu_set_t set;
    if (sched_getaffinity (getpid(), sizeof(set), &set) == 0) {
        return CPU_COUNT (&set);
    }
#endif

    pcnt = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // Assume the worst
    pcnt = 1;
#endif
    return pcnt;
#endif
}

CF_PRIVATE CFIndex __CFProcessorCount() {
    int32_t pcnt;
#if TARGET_OS_MAC && !__RAVYNOS__
    int32_t mib[] = {CTL_HW, HW_NCPU};
    size_t len = sizeof(pcnt);
    int32_t result = sysctl(mib, sizeof(mib) / sizeof(int32_t), &pcnt, &len, NULL, 0);
    if (result != 0) {
        pcnt = 0;
    }
#elif TARGET_OS_LINUX || TARGET_OS_BSD || __RAVYNOS__
    pcnt = sysconf(_SC_NPROCESSORS_CONF);
#else
    // Assume the worst
    pcnt = 1;
#endif
    return pcnt;
}

CF_PRIVATE uint64_t __CFMemorySize() {
    uint64_t memsize = 0;
#if TARGET_OS_MAC && !__RAVYNOS__
    int32_t mib[] = {CTL_HW, HW_MEMSIZE};
    size_t len = sizeof(memsize);
    int32_t result = sysctl(mib, sizeof(mib) / sizeof(int32_t), &memsize, &len, NULL, 0);
    if (result != 0) {
        memsize = 0;
    }
#elif TARGET_OS_LINUX || TARGET_OS_BSD || __RAVYNOS__
    memsize = sysconf(_SC_PHYS_PAGES);
    memsize *= sysconf(_SC_PAGESIZE);
#endif
    return memsize;
}

#if TARGET_OS_MAC && !__RAVYNOS__
static uid_t _CFGetSVUID(BOOL *successful) {
    uid_t uid = -1;
    struct kinfo_proc kinfo;
    u_int miblen = 4;
    size_t  len;
    int mib[miblen];
    int ret;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    len = sizeof(struct kinfo_proc);
    ret = sysctl(mib, miblen, &kinfo, &len, NULL, 0);
    if (ret != 0) {
        uid = -1;
        *successful = NO;
    } else {
        uid = kinfo.kp_eproc.e_pcred.p_svuid;
        *successful = YES;
    }
    return uid;
}
#endif

CF_INLINE BOOL _CFCanChangeEUIDs(void) {
#if TARGET_OS_MAC && !__RAVYNOS__
    static BOOL canChangeEUIDs;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        uid_t euid = geteuid();
        uid_t uid = getuid();
        BOOL gotSVUID = NO;
        uid_t svuid = _CFGetSVUID(&gotSVUID);
        canChangeEUIDs = (uid == 0 || uid != euid || svuid != euid || !gotSVUID);
    });
    return canChangeEUIDs;
#else
    return true;
#endif
}

#if !TARGET_OS_WASI
typedef struct _ugids {
    uid_t _euid;
    uid_t _egid;
} ugids;

// work around 33477678 by making these file-level globals
static ugids __CFGetUGIDs_cachedUGIDs = { -1, -1 };
static dispatch_once_t __CFGetUGIDs_onceToken;

CF_PRIVATE void __CFGetUGIDs(uid_t *euid, gid_t *egid) {
    ugids(^lookup)(void) = ^{
        ugids ids;
#if 1 && TARGET_OS_MAC && !__RAVYNOS__
        if (0 != pthread_getugid_np(&ids._euid, &ids._egid))
#endif
        {
            ids._euid = geteuid();
            ids._egid = getegid();
        }
        return ids;
    };
    
    ugids ids;
    
    if (_CFCanChangeEUIDs()) {
        ids = lookup();
    } else {
        dispatch_once(&__CFGetUGIDs_onceToken, ^{
            __CFGetUGIDs_cachedUGIDs = lookup();
        });
        ids = __CFGetUGIDs_cachedUGIDs;
    }
    
    if (euid) *euid = ids._euid;
    if (egid) *egid = ids._egid;
}
    
CF_EXPORT void _CFGetUGIDs(uid_t *euid, gid_t *egid) {
    __CFGetUGIDs(euid, egid);
}
    
CF_EXPORT uid_t _CFGetEUID(void) {
    uid_t euid;
    __CFGetUGIDs(&euid, NULL);
    return euid;
}

CF_EXPORT uid_t _CFGetEGID(void) {
    uid_t egid;
    __CFGetUGIDs(NULL, &egid);
    return egid;
}
#endif

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

#if TARGET_OS_WIN32
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
#if TARGET_OS_MAC
         fprintf_l(file, NULL, "\n");
#else
         fprintf(file, "\n");
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

#if TARGET_OS_MAC && !__RAVYNOS__
static Boolean debugger_attached() {
    BOOL debuggerAttached = NO;
    struct proc_bsdshortinfo info;
    const int size = proc_pidinfo(getpid(), PROC_PIDT_SHORTBSDINFO, 0, &info, sizeof(info));
    if (size == PROC_PIDT_SHORTBSDINFO_SIZE) {
        debuggerAttached = (info.pbsi_flags & PROC_FLAG_TRACED);
    }
    return debuggerAttached;
}
#endif

// We need to support both the 'modern' os_log focused approach to logging and the previous implementation. This enumeration gives us a way to distinguish between the two more verbosely than a boolean.
typedef enum {
    _cf_logging_style_os_log,
    _cf_logging_style_legacy
} _cf_logging_style;

static bool also_do_stderr(const _cf_logging_style style) {
    bool result = false;
#if TARGET_OS_LINUX || TARGET_OS_WIN32
    // just log to stderr, other logging facilities are out
    result = true;
#elif TARGET_OS_MAC && !__RAVYNOS__
    if (style == _cf_logging_style_os_log) {
        //
        // This might seem a bit odd, so an explanation is in order:
        //
        // If a debugger is attached we defer to os_log to do all logging for us. This
        // happens many frames up/sideways from here.
        //
        // Otherwise, we check for the force flag as we always have.
        //
        // That failing, we'll stat STDERR and see if its sxomething we like.
        //
        // NOTE: As nice as it would be to dispatch_once this, all of these things
        // can and do change at runtime under certain circumstances, so we need to
        // keep it dynamic.
        //
        if (debugger_attached() || __CFgetenv("OS_ACTIVITY_DT_MODE")) {
            return false;
        }

        if (!__CFProcessIsRestricted() && __CFgetenv("CFLOG_FORCE_STDERR")) {
            return true;
        }

        struct stat sb;
        int ret = fstat(STDERR_FILENO, &sb);
        if (ret < 0) {
            return false;
        }

        const mode_t m = sb.st_mode & S_IFMT;
        if (S_IFCHR == m || S_IFREG == m || S_IFIFO == m ||  S_IFSOCK == m) {
            return true;
        }
        // NOTE: we return false now, as the expectation is that os_log path owns all logging
        result = false;
    } else if (style == _cf_logging_style_legacy) {
        // compatibility path
        if (!issetugid() && __CFgetenv("CFLOG_FORCE_STDERR")) {
            return true;
        }
        struct stat sb;
        int ret = fstat(STDERR_FILENO, &sb);
        if (ret < 0) return false;
        mode_t m = sb.st_mode & S_IFMT;
        if (S_IFREG == m || S_IFSOCK == m) return true;
        if (!(S_IFIFO == m || S_IFCHR == m)) return false; // disallow any whacky stuff
        result = true;
    }
#else
    // just log to stderr, other logging facilities are out
    result = true;
#endif
    return result;
}

#if TARGET_OS_WIN32
static struct tm *localtime_r(time_t *tv, struct tm *result) {
  struct tm *tm = localtime(tv);
  if (tm) {
    *result = *tm;
  }
  return tm;
}
#endif

static void _populateBanner(char **banner, char **time, char **thread, int *bannerLen) {
    double dummy;
    CFAbsoluteTime at = CFAbsoluteTimeGetCurrent();
    time_t tv = (time_t)floor(at + kCFAbsoluteTimeIntervalSince1970);
    struct tm mine;
    localtime_r(&tv, &mine);
    int32_t year = mine.tm_year + 1900;
    int32_t month = mine.tm_mon + 1;
    int32_t day = mine.tm_mday;
    int32_t hour = mine.tm_hour;
    int32_t minute = mine.tm_min;
    int32_t second = mine.tm_sec;
    int32_t ms = (int32_t)floor(1000.0 * modf(at, &dummy));
#if TARGET_OS_MAC && !__RAVYNOS__
    uint64_t tid = 0;
    if (0 != pthread_threadid_np(NULL, &tid)) tid = pthread_mach_thread_np(pthread_self());
    asprintf(banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%llu] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), tid);
    asprintf(thread, "%x", pthread_mach_thread_np(pthread_self()));
#elif TARGET_OS_WIN32
    bannerLen = asprintf(banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%lx] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), GetCurrentThreadId());
    asprintf(thread, "%lx", GetCurrentThreadId());
#elif TARGET_OS_WASI
    bannerLen = asprintf(banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%x] ", year, month, day, hour, minute, second, ms, (unsigned int)pthread_self());
    asprintf(thread, "%lx", pthread_self());
#else
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
#pragma clang diagnostic ignored "-Wint-conversion"
#endif
    bannerLen = asprintf(banner, "%04d-%02d-%02d %02d:%02d:%02d.%03d %s[%d:%x] ", year, month, day, hour, minute, second, ms, *_CFGetProgname(), getpid(), (unsigned int)pthread_self());
    asprintf(thread, "%lx", pthread_self());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif
    asprintf(time, "%04d-%02d-%02d %02d:%02d:%02d.%03d", year, month, day, hour, minute, second, ms);
}

static void _logToStderr(char *banner, const char *message, size_t length) {
#if TARGET_OS_MAC
    struct iovec v[3];
    v[0].iov_base = banner;
    v[0].iov_len = banner ? strlen(banner) : 0;
    v[1].iov_base = (char *)message;
    v[1].iov_len = length;
    v[2].iov_base = "\n";
    v[2].iov_len = (message[length - 1] != '\n') ? 1 : 0;
    int nv = (v[0].iov_base ? 1 : 0) + 1 + (v[2].iov_len ? 1 : 0);
    static os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    os_unfair_lock_lock(&lock);
    writev(STDERR_FILENO, v[0].iov_base ? v : v + 1, nv);
    os_unfair_lock_unlock(&lock);
#elif TARGET_OS_WIN32
    size_t bannerLen = strlen(banner);
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
    size_t bannerLen = strlen(banner);
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

static void __CFLogCString(int32_t lev, const char *message, size_t length, char withBanner) {
    char *banner = NULL;
    char *time = NULL;
    char *thread = NULL;
    int bannerLen;
    bannerLen = 0;

    // The banner path may use CF functions, but the rest of this function should not. It may be called at times when CF is not fully setup or torn down.
    if (withBanner) {
        _populateBanner(&banner, &time, &thread, &bannerLen);
    }
    
    _logToStderr(banner, message, length);

    if (thread) free(thread);
    if (time) free(time);
    if (banner) free(banner);
}

static void __CFLogCStringLegacy(int32_t lev, const char *message, size_t length, char withBanner) {
    char *banner = NULL;
    char *time = NULL;
    char *uid = NULL;
    char *thread = NULL;
    int bannerLen;
    bannerLen = 0;
    
    // The banner path may use CF functions, but the rest of this function should not. It may be called at times when CF is not fully setup or torn down.
    if (withBanner) {
        _populateBanner(&banner, &time, &thread, &bannerLen);
    }
    
#if TARGET_OS_MAC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    uid_t euid;
    __CFGetUGIDs(&euid, NULL);
    asprintf(&uid, "%d", euid);
    aslclient asl = asl_open(NULL, (__CFBundleMainID && __CFBundleMainID[0]) ? __CFBundleMainID : "com.apple.console", ASL_OPT_NO_DELAY);
    aslmsg msg = asl_new(ASL_TYPE_MSG);
    asl_set(msg, "CFLog Local Time", time); // not to be documented, not public API
    asl_set(msg, "CFLog Thread", thread);   // not to be documented, not public API
    asl_set(msg, "ReadUID", uid);
    static const char * const levstr[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    asl_set(msg, ASL_KEY_LEVEL, levstr[lev]);
    asl_set(msg, ASL_KEY_MSG, message);
    asl_send(asl, msg);
    asl_free(msg);
    asl_close(asl);
#pragma GCC diagnostic pop
#endif // DEPLOYMENT_TARGET
    
    if (also_do_stderr(_cf_logging_style_legacy)) {
        _logToStderr(banner, message, length);
    }
    
    if (thread) free(thread);
    if (time) free(time);
    if (banner) free(banner);
    if (uid) free(uid);
}


static void _CFLogvEx2Predicate(CFLogFunc logit, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, int32_t lev, CFStringRef format, va_list args, _cf_logging_style loggingStyle) {
#if TARGET_OS_MAC
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
            if (logit) {
                logit(lev, buf, len, 1);
            }
            else if (loggingStyle == _cf_logging_style_os_log) {
                __CFLogCString(lev, buf, len, 1);
            }
            else if (loggingStyle == _cf_logging_style_legacy) {
                __CFLogCStringLegacy(lev, buf, len, 1);
            }
	}
    }
    if (buf) free(buf);
    if (str) CFRelease(str);
#if TARGET_OS_MAC
    _CFSetTSD(__CFTSDKeyIsInCFLog, (void *)val, NULL);
#endif
}
    
CF_EXPORT void _CFLogvEx2(CFLogFunc logit, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, int32_t lev, CFStringRef format, va_list args) {
    _CFLogvEx2Predicate(logit, copyDescFunc, contextDescFunc, formatOptions, lev, format, args, _cf_logging_style_legacy);
}
    
CF_EXPORT void _CFLogvEx3(CFLogFunc logit, CFStringRef (*copyDescFunc)(void *, const void *), CFStringRef (*contextDescFunc)(void *, const void *, const void *, bool, bool *), CFDictionaryRef formatOptions, int32_t lev, CFStringRef format, va_list args, void *addr) {
    _CFLogvEx2Predicate(logit, copyDescFunc, contextDescFunc, formatOptions, lev, format, args, _cf_logging_style_legacy);
    
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
        __CFLogCStringLegacy(lev, formattedMessage, length, 0);
    }
    va_end(args);
}

void CFLog(int32_t lev, CFStringRef format, ...) {
    va_list args;
    va_start(args, format); 
    
    #if !TARGET_OS_WASI
    _CFLogvEx3(NULL, NULL, NULL, NULL, lev, format, args, __builtin_return_address(0));
    #else
    _CFLogvEx3(NULL, NULL, NULL, NULL, lev, format, args, NULL);
    #endif
    va_end(args);
}
    
#if DEPLOYMENT_RUNTIME_SWIFT
// Temporary as Swift cannot import varag C functions
void CFLog1(CFLogLevel lev, CFStringRef message) {
#if TARGET_OS_ANDROID
    android_LogPriority priority = ANDROID_LOG_UNKNOWN;
    switch (lev) {
        case kCFLogLevelEmergency: priority = ANDROID_LOG_FATAL; break;
        case kCFLogLevelAlert:     priority = ANDROID_LOG_ERROR; break;
        case kCFLogLevelCritical:  priority = ANDROID_LOG_ERROR; break;
        case kCFLogLevelError:     priority = ANDROID_LOG_ERROR; break;
        case kCFLogLevelWarning:   priority = ANDROID_LOG_WARN;  break;
        case kCFLogLevelNotice:    priority = ANDROID_LOG_WARN;  break;
        case kCFLogLevelInfo:      priority = ANDROID_LOG_INFO;  break;
        case kCFLogLevelDebug:     priority = ANDROID_LOG_DEBUG; break;
    }
    
    if (message == NULL) message = CFSTR("NULL");
    
    char stack_buffer[1024] = { 0 };
    char *buffer = &stack_buffer[0];
    CFStringEncoding encoding = kCFStringEncodingUTF8;
    CFIndex maxLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(message), encoding) + 1;
    const char *tag = "Swift"; // process name not available from NDK
    
    if (maxLength > sizeof(stack_buffer) / sizeof(stack_buffer[0])) {
        buffer = calloc(sizeof(char), maxLength);
        if (!buffer) {
            maxLength = sizeof(stack_buffer) / sizeof(stack_buffer[0]);
            buffer = &stack_buffer[0];
            buffer[maxLength-1] = '\000';
        }
    }
    
    if (maxLength == 1) {
        // was crashing with zero length strings
        // https://bugs.swift.org/browse/SR-2666
        strcpy(buffer, " "); // log empty string
    }
    else
        CFStringGetCString(message, buffer, maxLength, encoding);
    
    __android_log_print(priority, tag, "%s", buffer);
    fprintf(stderr, "%s\n", buffer);
    
    if (buffer != &stack_buffer[0]) free(buffer);
#else
    CFLog(lev, CFSTR("%@"), message);
#endif
}
#endif



#if TARGET_OS_MAC && !__RAVYNOS__

kern_return_t _CFDiscorporateMemoryAllocate(CFDiscorporateMemory *hm, size_t size, bool purgeable) {
    kern_return_t ret = KERN_SUCCESS;
    size = round_page(size);
    if (0 == size) size = vm_page_size;
    memset(hm, 0, sizeof(CFDiscorporateMemory));

    ret = mach_vm_allocate(mach_task_self(), &hm->map_address, size, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_FOUNDATION) |  (purgeable ? VM_FLAGS_PURGABLE : 0));
    if (KERN_SUCCESS != ret) {
        hm->map_address = 0;
        return ret;
    }

    hm->address = hm->map_address;
    hm->size = (mach_vm_size_t)size;
    hm->purgeable = purgeable;
    return ret;
}

kern_return_t _CFDiscorporateMemoryDeallocate(CFDiscorporateMemory *hm) {
    kern_return_t ret = KERN_SUCCESS;
    if (hm->map_address != 0) ret = mach_vm_deallocate(mach_task_self(), hm->map_address, hm->size);
    if (KERN_SUCCESS != ret)
        CFLog(kCFLogLevelError, CFSTR("CFDiscorporateMemoryDeallocate: mach_vm_deallocate returned %s"), mach_error_string(ret));
    hm->address = 0;
    hm->size = 0;
    hm->map_address = 0;
    hm->purgeable = false;
    return ret;
}

kern_return_t _CFDiscorporateMemoryDematerialize(CFDiscorporateMemory *hm) {
    kern_return_t ret = KERN_SUCCESS;
    if (hm->address == 0) ret = KERN_INVALID_MEMORY_CONTROL;
    int state = VM_PURGABLE_VOLATILE;
    if (KERN_SUCCESS == ret) vm_purgable_control(mach_task_self(), (vm_address_t)hm->address, VM_PURGABLE_SET_STATE, &state);
    if (KERN_SUCCESS == ret) hm->address = 0;
    return ret;
}

kern_return_t _CFDiscorporateMemoryMaterialize(CFDiscorporateMemory *hm) {
    kern_return_t ret = KERN_SUCCESS;
    if (hm->address != 0) ret = KERN_INVALID_MEMORY_CONTROL;
    int state = VM_PURGABLE_NONVOLATILE;
    if (KERN_SUCCESS == ret) ret = vm_purgable_control(mach_task_self(), (vm_address_t)hm->map_address, VM_PURGABLE_SET_STATE, &state);
    if (KERN_SUCCESS == ret) hm->address = hm->map_address;
    if (KERN_SUCCESS == ret) if (VM_PURGABLE_EMPTY == state) ret = KERN_PROTECTION_FAILURE; // same as VM_PURGABLE_EMPTY
    return ret;
}

#endif

#if TARGET_OS_OSX
static os_unfair_lock __CFProcessKillingLock = OS_UNFAIR_LOCK_INIT;
static CFIndex __CFProcessKillingDisablingCount = 1;
static Boolean __CFProcessKillingWasTurnedOn = false;
#endif

void _CFSuddenTerminationDisable(void) {
#if TARGET_OS_OSX
    os_unfair_lock_lock(&__CFProcessKillingLock);
    __CFProcessKillingDisablingCount++;
    os_unfair_lock_unlock(&__CFProcessKillingLock);
#else
    // Do nothing
#endif
}

void _CFSuddenTerminationEnable(void) {
#if TARGET_OS_OSX
    // In our model the first call of _CFSuddenTerminationEnable() that does not balance a previous call of _CFSuddenTerminationDisable() actually enables sudden termination so we have to keep a count that's almost redundant with vproc's.
    os_unfair_lock_lock(&__CFProcessKillingLock);
    __CFProcessKillingDisablingCount--;
    if (__CFProcessKillingDisablingCount==0 && !__CFProcessKillingWasTurnedOn) {
	__CFProcessKillingWasTurnedOn = true;
    } else {
    }
    os_unfair_lock_unlock(&__CFProcessKillingLock);
#else
    // Do nothing
#endif
}

void _CFSuddenTerminationExitIfTerminationEnabled(int exitStatus) {
#if TARGET_OS_OSX
    // This is for when the caller wants to try to exit quickly if possible but not automatically exit the process when it next becomes clean, because quitting might still be cancelled by the user.
    os_unfair_lock_lock(&__CFProcessKillingLock);
    os_unfair_lock_unlock(&__CFProcessKillingLock);
#else
    // Elsewhere, sudden termination is always disabled
#endif
}

void _CFSuddenTerminationExitWhenTerminationEnabled(int exitStatus) {
#if TARGET_OS_OSX
    // The user has had their final opportunity to cancel quitting. Exit as soon as the process is clean. Same carefulness as in _CFSuddenTerminationExitIfTerminationEnabled().
    os_unfair_lock_lock(&__CFProcessKillingLock);
    if (__CFProcessKillingWasTurnedOn) {
    }
    os_unfair_lock_unlock(&__CFProcessKillingLock);
#else
    // Elsewhere, sudden termination is always disabled
#endif
}

size_t _CFSuddenTerminationDisablingCount(void) {
#if TARGET_OS_OSX
    return (__CFProcessKillingWasTurnedOn ? 0 : 1);
#else
    // Elsewhere, sudden termination is always disabled
    return 1;
#endif
}

#pragma mark File Reading
    
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#if TARGET_OS_WIN32
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
#if TARGET_OS_MAC && !__RAVYNOS__
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
#if TARGET_RT_64_BIT
#else
    if (statBuf.st_size > (1ull << 31)) {	// refuse to do more than 2GB
        close(fd);
        if (errorPtr) *errorPtr = _CFErrorWithFilePathCodeDomain(kCFErrorDomainPOSIX, EFBIG, path);
        return false;
    }
#endif

    if (0LL == statBuf.st_size) {
        bytes = malloc(8); // don't return constant string -- it's freed!
	length = 0;
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI
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
#elif TARGET_OS_WIN32
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

/* __CFStringGetCharacterFromInlineBufferQuickReverse is the same as __CFStringGetCharacterFromInlineBufferQuick(), but expects idx to be decremented
 */
CF_INLINE UniChar __CFStringGetCharacterFromInlineBufferQuickReverse(CFStringInlineBuffer *buf, CFIndex idx) {
    if (buf->directUniCharBuffer) return buf->directUniCharBuffer[idx + buf->rangeToBuffer.location];
    if (buf->directCStringBuffer) return (UniChar)(buf->directCStringBuffer[idx + buf->rangeToBuffer.location]);
    if (idx >= buf->bufferedRangeEnd || idx < buf->bufferedRangeStart) {
        if ((buf->bufferedRangeStart = idx - __kCFStringInlineBufferLength + 1) < 0) buf->bufferedRangeStart = 0;
        buf->bufferedRangeEnd = buf->bufferedRangeStart + __kCFStringInlineBufferLength;
        if (buf->bufferedRangeEnd > buf->rangeToBuffer.length) buf->bufferedRangeEnd = buf->rangeToBuffer.length;
        CFStringGetCharacters(buf->theString, CFRangeMake(buf->rangeToBuffer.location + buf->bufferedRangeStart, buf->bufferedRangeEnd - buf->bufferedRangeStart), buf->buffer);
    }
    return buf->buffer[idx - buf->bufferedRangeStart];
}

/* UniCharInitInlineBuffer initializes an in-line buffer to use an array of UniChar with CFStringGetCharacterFromInlineBuffer (and related SPI that use CFStringInlineBuffer).
 */
CF_INLINE void UniCharInitInlineBuffer(const UniChar *uchars, CFIndex ucharsLength, CFStringInlineBuffer *buf) {
    buf->theString = NULL;
    buf->rangeToBuffer = CFRangeMake(0, ucharsLength);
    buf->directUniCharBuffer = uchars;
    buf->directCStringBuffer = NULL;
    buf->bufferedRangeStart = buf->bufferedRangeEnd = 0;
}

// __IsInvalidExtensionCharacter returns true if C is a space, a path separator, or a unicode directional control character
CF_INLINE Boolean __IsInvalidExtensionCharacter(UniChar c)
{
    // Unicode directional control characters
    enum {
        UNICHAR_ARABIC_LETTER_MARK          = 0x061c,
        UNICHAR_LEFT_TO_RIGHT_MARK          = 0x200e,
        UNICHAR_RIGHT_TO_LEFT_MARK          = 0x200f,
        UNICHAR_LEFT_TO_RIGHT_EMBEDDING     = 0x202a,
        UNICHAR_RIGHT_TO_LEFT_EMBEDDING     = 0x202b,
        UNICHAR_POP_DIRECTIONAL_FORMATTING  = 0x202c,
        UNICHAR_LEFT_TO_RIGHT_OVERRIDE      = 0x202d,
        UNICHAR_RIGHT_TO_LEFT_OVERRIDE      = 0x202e,
        UNICHAR_LEFT_TO_RIGHT_ISOLATE       = 0x2066,
        UNICHAR_RIGHT_TO_LEFT_ISOLATE       = 0x2067,
        UNICHAR_FIRST_STRONG_ISOLATE        = 0x2068,
        UNICHAR_POP_DIRECTIONAL_ISOLATE     = 0x2069,
    };
    
    Boolean result;
    switch ( c ) {
        case (UniChar)' ':
        case (UniChar)'/':
        case UNICHAR_ARABIC_LETTER_MARK:
        case UNICHAR_LEFT_TO_RIGHT_MARK:
        case UNICHAR_RIGHT_TO_LEFT_MARK:
        case UNICHAR_LEFT_TO_RIGHT_EMBEDDING:
        case UNICHAR_RIGHT_TO_LEFT_EMBEDDING:
        case UNICHAR_POP_DIRECTIONAL_FORMATTING:
        case UNICHAR_LEFT_TO_RIGHT_OVERRIDE:
        case UNICHAR_RIGHT_TO_LEFT_OVERRIDE:
        case UNICHAR_LEFT_TO_RIGHT_ISOLATE:
        case UNICHAR_RIGHT_TO_LEFT_ISOLATE:
        case UNICHAR_FIRST_STRONG_ISOLATE:
        case UNICHAR_POP_DIRECTIONAL_ISOLATE:
            result = true;
            break;
            
        default:
            result = false;
            break;
    }
    return ( result );
}

/*  __GetPathExtensionRangesFromPathComponentBuffer finds the range of the primary path extension (if any) for the given path component. If requested, __GetPathExtensionRangesFromPathComponentBuffer also finds the secondary path extension (if any) for the given path component.
 */
CF_INLINE void __GetPathExtensionRangesFromPathComponentBuffer(CFStringInlineBuffer *buffer, CFRange *outPrimaryExtRange, CFRange *outSecondaryExtRange) {
    CFRange primaryExtRange = CFRangeMake(kCFNotFound, 0);
    CFRange secondaryExtRange = CFRangeMake(kCFNotFound, 0);
    
    if ( buffer && (outPrimaryExtRange || outSecondaryExtRange) ) {
        //
        // Look backwards for a period. The period may not be the first or last character of the path component,
        // but otherwise the extension may be of any length and contain any characters
        // except space, a path separator, or any unicode directional control character.
        //
        // The secondary extension is the "apparent" extension the user would see if
        // the primary extension is hidden. Since this is a security concern, the
        // rules for the secondary extension are looser. Leading and trailing spaces
        // are ignored, since the user might think "foo.txt .app" is a text file if
        // ".app" is hidden.
        
        CFIndex nameLen = buffer->rangeToBuffer.length;
        
        for ( CFIndex i = nameLen - 1; i > 0; i-- ) {
            UniChar c = __CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i);
            if ( (UniChar)'.' == c ) {
                // Found a dot
                CFIndex e2i = 0;
                if ( i == (nameLen - 1) ) {
                    // Last char disqualified
                    break;
                }
                
                primaryExtRange = CFRangeMake(i + 1, nameLen - (i + 1));
                
                if ( outSecondaryExtRange == NULL ) {
                    break;
                }
                
                // Look for an "apparent" secondary extension, using modified rules.
                CFIndex e2len = 0;
                
                // Skip trailing space (one-liner):
                for ( --i; (i > 0) && (__CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i) == (UniChar)' '); i-- );
                
                // Scan apparent extension text
                for ( ; i > 0; i-- ) {
                    c = __CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i);
                    if ( ((UniChar)'.' == c) || ((UniChar)' ' == c) ) {
                        break;
                    }
                    e2len++;
                }
                
                e2i = i + 1;
                
                // Skip leading space (one-liner):
                for ( ; (i > 0) && (__CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i) == (UniChar)' '); i-- );
                
                if ( (i > 0) && (__CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i) == (UniChar)'.') && (e2len > 0) ) {
                    secondaryExtRange = CFRangeMake( e2i, e2len );
                }
                
                break;
            }
            else if ( __IsInvalidExtensionCharacter(c) ) {
                // Found a space, a path separator, or a unicode directional control character -- terminate the loop
                break;
            }
        }
    }
    
    if ( outPrimaryExtRange ) {
        *outPrimaryExtRange = primaryExtRange;
    }
    
    if ( outSecondaryExtRange ) {
        *outSecondaryExtRange = secondaryExtRange;
    }
}

/*  __ExtensionIsValidToAppend determines if the characters in extension are valid to be appended as an extension.
 */
CF_INLINE Boolean __ExtensionIsValidToAppend(CFStringInlineBuffer *buffer) {
    Boolean result;
    
    if ( buffer && buffer->rangeToBuffer.length ) {
        //
        // Look backwards for a period. If a period is found, it must not be the
        // last character and any characters after the period must be valid in an
        // extension, and all characters before the period are considered valid
        // except for the path separator character '/' (this allow strings like
        // "tar.gz" to be appended as an extension). The path separator
        // character '/' isn't allowed anywhere in the extension. If there's no
        // period, the entire string must be characters valid in an extension.
        // The extension may be of any length.
        
        CFIndex nameLen = buffer->rangeToBuffer.length;
        if ( nameLen ) {
            result = true;  // assume the best
            
            for ( CFIndex i = nameLen - 1; i >= 0; i-- ) {
                UniChar c = __CFStringGetCharacterFromInlineBufferQuickReverse(buffer, i);
                if ( (UniChar)'.' == c ) {
                    // Found a period. If it's  the last character, then return false; otherwise true
                    result = i < nameLen - 1;
                    if ( result ) {
                        // check the rest of the string before the period for '/'
                        for ( CFIndex j = i - 1; j >= 0; j-- ) {
                            c = __CFStringGetCharacterFromInlineBufferQuickReverse(buffer, j);
                            if ( c == (UniChar)'/'){
                                result = false;
                                break;
                            }
                        }
                    }
                    break;
                }
                else if ( __IsInvalidExtensionCharacter(c) ) {
                    // Found an invalid extension character. Return false.
                    result = false;
                    break;
                }
            }
        }
        else {
            // NOTE: The implementation before we switched to _CFGetPathExtensionRangesFromPathComponent
            // allowed an empty string to be appended as an extension. That is not a valid extension.
            result = false;
        }
    }
    else {
        // no buffer or no extension
        result = false;
    }
    return ( result );
}

/*  _CFGetExtensionInfoFromPathComponent/_CFGetPathExtensionRangesFromPathComponentUniChars finds the range of the primary path extension (if any) for the given path component. If requested, _CFGetExtensionInfoFromPathComponent/_CFGetPathExtensionRangesFromPathComponentUniChars also finds the secondary path extension (if any) for the given path component.
 */
CF_EXPORT void _CFGetPathExtensionRangesFromPathComponentUniChars(const UniChar *uchars, CFIndex ucharsLength, CFRange *outPrimaryExtRange, CFRange *outSecondaryExtRange) {
    CFStringInlineBuffer buffer;
    UniCharInitInlineBuffer(uchars, ucharsLength, &buffer);
    __GetPathExtensionRangesFromPathComponentBuffer(&buffer, outPrimaryExtRange, outSecondaryExtRange);
}

CF_EXPORT void _CFGetPathExtensionRangesFromPathComponent(CFStringRef inName, CFRange *outPrimaryExtRange, CFRange *outSecondaryExtRange) {
    CFStringInlineBuffer buffer;
    CFStringInitInlineBuffer(inName, &buffer, CFRangeMake(0, CFStringGetLength(inName)));
    __GetPathExtensionRangesFromPathComponentBuffer(&buffer, outPrimaryExtRange, outSecondaryExtRange);
}

/*  _CFPathExtensionIsValid/_CFExtensionUniCharsIsValidToAppend determines if the characters in extension are valid to be appended as an extension.
 */
CF_EXPORT Boolean _CFExtensionUniCharsIsValidToAppend(const UniChar *uchars, CFIndex ucharsLength) {
    CFStringInlineBuffer buffer;
    UniCharInitInlineBuffer(uchars, ucharsLength, &buffer);
    return ( __ExtensionIsValidToAppend(&buffer) );
}

CF_EXPORT Boolean _CFExtensionIsValidToAppend(CFStringRef extension) {
    CFStringInlineBuffer buffer;
    CFStringInitInlineBuffer(extension, &buffer, CFRangeMake(0, CFStringGetLength(extension)));
    return ( __ExtensionIsValidToAppend(&buffer) );
}


#if DEPLOYMENT_RUNTIME_SWIFT && !TARGET_OS_WASI

CFDictionaryRef __CFGetEnvironment() {
    static dispatch_once_t once = 0L;
    static CFMutableDictionaryRef envDict = NULL;
    dispatch_once(&once, ^{
#if TARGET_OS_MAC
        extern char ***_NSGetEnviron(void);
        char **envp = *_NSGetEnviron();
#elif TARGET_OS_BSD || TARGET_OS_CYGWIN || TARGET_OS_WASI
        extern char **environ;
        char **envp = environ;
#elif TARGET_OS_LINUX
#if !defined(environ) && !TARGET_OS_ANDROID
#define environ __environ
#endif
        char **envp = environ;
#elif TARGET_OS_WIN32
        char **envp = _environ;
#endif
        envDict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        for (; *envp; ++envp) {
            char *eqp = *envp;	// '=' pointer
            while (*eqp && *eqp != '=') ++eqp;
            
            char *endp = eqp;
            while (*endp) ++endp;
            
            if (endp == eqp) {	// oops, badly formed value
                continue;
            }
            CFStringRef key = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const uint8_t *)*envp, (eqp - *envp), kCFStringEncodingUTF8, false);
            if (key == NULL) {
                key = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const uint8_t *)*envp, (eqp - *envp), CFStringGetSystemEncoding(), false);
            }
            CFStringRef value = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const uint8_t *)(eqp + 1), (endp - (eqp + 1)), kCFStringEncodingUTF8, false);
            if (key == NULL) {
                key = CFStringCreateWithBytes(kCFAllocatorSystemDefault, (const uint8_t *)(eqp + 1), (endp - (eqp + 1)), CFStringGetSystemEncoding(), false);
            }
            if (NULL == key || NULL == value) {
                if (key != NULL) CFRelease(key);
                if (value != NULL) CFRelease(value);
                continue;
            }
            CFStringRef existingValue = CFDictionaryGetValue(envDict, key);
            if (existingValue == NULL) {
                CFDictionarySetValue(envDict, key, value);
            } else if (CFEqual(existingValue, value)){
                CFLog(kCFLogLevelWarning, CFSTR("Warning: duplicate definition for key '%@' found in environment -- subsequent definitions are ignored.  The first definition was '%@', the ignored definition is '%@'."), key, existingValue, value);
            }
            CFRelease(key);
            CFRelease(value);
        }
    });
    return envDict;
}

int32_t __CFGetPid() {
    return getpid();
}

#endif
