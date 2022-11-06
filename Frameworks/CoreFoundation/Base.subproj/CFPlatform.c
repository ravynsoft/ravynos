/*	CFPlatform.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Tony Parker
*/


#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>
#if TARGET_OS_MAC
    #include <stdlib.h>
    #include <sys/stat.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <pwd.h>
    #include <crt_externs.h>
    #include <mach-o/dyld.h>
#endif

#define _CFEmitInternalDiagnostics 0


#if TARGET_OS_WIN32
#include <shellapi.h>
#include <shlobj.h>
#include <WinIoCtl.h>
#include <direct.h>
#include <process.h>
#include <processthreadsapi.h>
#define SECURITY_WIN32
#include <Security.h>

#define getcwd _NS_getcwd
#define open _NS_open

#endif

#if TARGET_OS_ANDROID
#include <sys/prctl.h>
#endif

#if TARGET_OS_MAC || TARGET_OS_WIN32
#define kCFPlatformInterfaceStringEncoding	kCFStringEncodingUTF8
#else
#define kCFPlatformInterfaceStringEncoding	CFStringGetSystemEncoding()
#endif

extern void __CFGetUGIDs(uid_t *euid, gid_t *egid);

#if TARGET_OS_MAC
// CoreGraphics and LaunchServices are only projects (1 Dec 2006) that use these
char **_CFArgv(void) { return *_NSGetArgv(); }
int _CFArgc(void) { return *_NSGetArgc(); }
#endif


#if !TARGET_OS_WASI
CF_PRIVATE Boolean _CFGetCurrentDirectory(char *path, int maxlen) {
    return getcwd(path, maxlen) != NULL;
}
#endif

#if TARGET_OS_WIN32
// Returns the path to the CF DLL, which we can then use to find resources like char sets
bool bDllPathCached = false;
CF_PRIVATE const wchar_t *_CFDLLPath(void) {
    static wchar_t cachedPath[MAX_PATH+1];

    if (!bDllPathCached) {
#ifdef _DEBUG
        // might be nice to get this from the project file at some point
        wchar_t *DLLFileName = L"CoreFoundation_debug.dll";
#else
        wchar_t *DLLFileName = L"CoreFoundation.dll";
#endif
        HMODULE ourModule = GetModuleHandleW(DLLFileName);
        
        CFAssert(ourModule, __kCFLogAssertion, "GetModuleHandle failed");

        DWORD wResult = GetModuleFileNameW(ourModule, cachedPath, MAX_PATH+1);
        CFAssert1(wResult > 0, __kCFLogAssertion, "GetModuleFileName failed: %d", GetLastError());
        CFAssert1(wResult < MAX_PATH+1, __kCFLogAssertion, "GetModuleFileName result truncated: %s", cachedPath);

        // strip off last component, the DLL name
        CFIndex idx;
        for (idx = wResult - 1; idx; idx--) {
            if ('\\' == cachedPath[idx]) {
                cachedPath[idx] = '\0';
                break;
            }
        }
        bDllPathCached = true;
    }
    return cachedPath;
}
#endif // TARGET_OS_WIN32

#if !TARGET_OS_WASI
static const char *__CFProcessPath = NULL;
static const char *__CFprogname = NULL;

const char **_CFGetProgname(void) {
    if (!__CFprogname)
        _CFProcessPath();		// sets up __CFprogname as a side-effect
    return &__CFprogname;
}

const char **_CFGetProcessPath(void) {
    if (!__CFProcessPath)
        _CFProcessPath();		// sets up __CFProcessPath as a side-effect
    return &__CFProcessPath;
}

static inline void _CFSetProgramNameFromPath(const char *path) {
    __CFProcessPath = strdup(path);
    __CFprogname = strrchr(__CFProcessPath, PATH_SEP);
    __CFprogname = (__CFprogname ? __CFprogname + 1 : __CFProcessPath);
}

#if TARGET_OS_BSD && defined(__OpenBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/exec.h>
#endif

const char *_CFProcessPath(void) {
    if (__CFProcessPath) return __CFProcessPath;

#if TARGET_OS_WIN32
    wchar_t buf[CFMaxPathSize] = {0};
    DWORD rlen = GetModuleFileNameW(NULL, buf, sizeof(buf) / sizeof(buf[0]));
    if (0 < rlen) {
	char asciiBuf[CFMaxPathSize] = {0};
	int res = WideCharToMultiByte(CP_UTF8, 0, buf, rlen, asciiBuf, sizeof(asciiBuf) / sizeof(asciiBuf[0]), NULL, NULL);
	if (0 < res) {
            _CFSetProgramNameFromPath(asciiBuf);
	}
    }
    if (!__CFProcessPath) {
	__CFProcessPath = "";
        __CFprogname = __CFProcessPath;
    }
    return __CFProcessPath;
#elif TARGET_OS_MAC
#if TARGET_OS_OSX
    if (!__CFProcessIsRestricted()) {
	const char *path = (char *)__CFgetenv("CFProcessPath");
	if (path) {
            _CFSetProgramNameFromPath(path);
	    return __CFProcessPath;
	}
    }
#endif

    {
        uint32_t size = CFMaxPathSize;
        char buffer[size];
        if (0 == _NSGetExecutablePath(buffer, &size)) {
            _CFSetProgramNameFromPath(buffer);
        }
    }

    if (!__CFProcessPath) {
	__CFProcessPath = "";
        __CFprogname = __CFProcessPath;
    }
    return __CFProcessPath;
#elif TARGET_OS_LINUX
    char buf[CFMaxPathSize + 1];

    ssize_t res = readlink("/proc/self/exe", buf, CFMaxPathSize);
    if (res > 0) {
        // null terminate, readlink does not
        buf[res] = 0;
        _CFSetProgramNameFromPath(buf);
    } else {
        __CFProcessPath = "";
        __CFprogname = __CFProcessPath;
    }
    return __CFProcessPath;
#else // TARGET_OS_BSD
    char *argv0 = NULL;

    // Get argv[0].
#if defined(__OpenBSD__)
    int mib[2] = {CTL_VM, VM_PSSTRINGS};
    struct _ps_strings _ps;
    size_t len = sizeof(_ps);

    if (sysctl(mib, 2, &_ps, &len, NULL, 0) != -1) {
        struct ps_strings *ps = _ps.val;
        char *res = realpath(ps->ps_argvstr[0], NULL);
        argv0 = res? res: strdup(ps->ps_argvstr[0]);
    }
#endif

    if (!__CFProcessIsRestricted() && argv0 && argv0[0] == '/') {
        _CFSetProgramNameFromPath(argv0);
        free(argv0);
        return __CFProcessPath;
    }

    // Search PATH.
    if (argv0) {
        char *paths = getenv("PATH");
        char *p = NULL;
        while ((p = strsep(&paths, ":")) != NULL) {
            char pp[PATH_MAX];
            int l = snprintf(pp, PATH_MAX, "%s/%s", p, argv0);
            if (l >= PATH_MAX) {
                continue;
            }
            char *res = realpath(pp, NULL);
            if (!res) {
                continue;
            }
            if (!__CFProcessIsRestricted() && access(res, X_OK) == 0) {
                _CFSetProgramNameFromPath(res);
                free(argv0);
                free(res);
                return __CFProcessPath;
            }
            free(res);
        }
        free(argv0);
    }

    // See if the shell will help.
    if (!__CFProcessIsRestricted()) {
        char *path = getenv("_");
        if (path != NULL) {
            _CFSetProgramNameFromPath(path);
            return __CFProcessPath;
        }
    }

    // We don't yet have anything left to try.
    __CFProcessPath = "";
    __CFprogname = __CFProcessPath;
    return __CFProcessPath;
#endif
}
#endif // TARGET_OS_WASI

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_BSD
CF_CROSS_PLATFORM_EXPORT Boolean _CFIsMainThread(void) {
    return pthread_main_np() == 1;
}
#endif

#if TARGET_OS_LINUX
#include <unistd.h>
#if __has_include(<syscall.h>)
#include <syscall.h>
#else
#include <sys/syscall.h>
#endif // __has_include(<syscall.h>)

Boolean _CFIsMainThread(void) {
    return syscall(SYS_gettid) == getpid();
}
#endif // TARGET_OS_LINUX

#if !TARGET_OS_WASI
CF_PRIVATE CFStringRef _CFProcessNameString(void) {
    static CFStringRef __CFProcessNameString = NULL;
    if (!__CFProcessNameString) {
        const char *processName = *_CFGetProgname();
        if (!processName) processName = "";
        CFStringRef newStr = CFStringCreateWithCString(kCFAllocatorSystemDefault, processName, kCFPlatformInterfaceStringEncoding);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, (void *) newStr, (void * volatile *)& __CFProcessNameString)) {
#pragma GCC diagnostic pop
            CFRelease(newStr);    // someone else made the assignment, so just release the extra string.
        }
    }
    return __CFProcessNameString;
}
#endif // !TARGET_OS_WASI

#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD

#include <pwd.h>
#include <sys/param.h>

// Set the fallBackToHome parameter to true if we should fall back to the HOME environment variable if all else fails. Otherwise return NULL.
static CFURLRef _CFCopyHomeDirURLForUser(const char *username, bool fallBackToHome) {
    const char *fixedHomePath = issetugid() ? NULL : __CFgetenv("CFFIXED_USER_HOME");
    
    __block CFMutableStringRef errorMessage = NULL;
    void (^prepareErrorMessage)(void) = ^{
        if (!errorMessage) {
            errorMessage = CFStringCreateMutable(NULL, 0);
        } else {
            CFStringAppend(errorMessage, CFSTR("\n"));
        }
    };
    
    // Calculate the home directory we will use
    // First try CFFIXED_USER_HOME (only if not setugid), then fall back to the upwd, then fall back to HOME environment variable
    CFURLRef home = NULL;
    if (!issetugid() && fixedHomePath) {
        home = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)fixedHomePath, strlen(fixedHomePath), true);
        if (!home) {
            prepareErrorMessage();
            if (_CFEmitInternalDiagnostics) {
                CFStringAppendFormat(errorMessage, NULL, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for CFFIXED_USER_HOME value: %s"), fixedHomePath);
            } else {
                CFStringAppend(errorMessage, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for CFFIXED_USER_HOME value"));
            }
        }
    }
    if (!home) {
        struct passwd *upwd = NULL;
        if (username) {
            errno = 0;
            upwd = getpwnam(username);
        } else {
            uid_t euid;
            __CFGetUGIDs(&euid, NULL);
            
            errno = 0;
            upwd = getpwuid(euid ?: getuid());
        }
        if (upwd) {
            if (upwd->pw_dir) {
                home = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)upwd->pw_dir, strlen(upwd->pw_dir), true);
            }
            if (!home && !username) {
                prepareErrorMessage();
                if (!upwd->pw_dir) {
                    CFStringAppend(errorMessage, CFSTR("upwd->pw_dir is NULL"));
                } else if (_CFEmitInternalDiagnostics) {
                    CFStringAppendFormat(errorMessage, NULL, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for upwd->pw_dir value: %s"), upwd->pw_dir);
                } else {
                    CFStringAppend(errorMessage, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for upwd->pw_dir value"));
                }
            }
        } else if (!username) {
            int const savederrno = errno;
            prepareErrorMessage();
            CFStringAppendFormat(errorMessage, NULL, CFSTR("getpwuid failed with code: %d"), savederrno);
        }
    }
    if (fallBackToHome && !home) {
        const char *homePath = __CFgetenv("HOME");
        if (homePath) {
            home = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)homePath, strlen(homePath), true);
            if (!home) {
                prepareErrorMessage();
                if (_CFEmitInternalDiagnostics) {
                    CFStringAppendFormat(errorMessage, NULL, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for HOME value: %s"), homePath);
                } else {
                    CFStringAppend(errorMessage, CFSTR("CFURLCreateFromFileSystemRepresentation failed to create URL for HOME value"));
                }
            }
        }
    }
    
    if (errorMessage) {
        if (!home) {
            os_log_error(_CFOSLog(), "_CFCopyHomeDirURLForUser failed to create a proper home directory. Falling back to /var/empty. Error(s):\n%{public}@", errorMessage);
            const char *_var_empty = "/var/empty";
            home = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (const UInt8 *)_var_empty, strlen(_var_empty), true);
        }
        CFRelease(errorMessage);
    }
    
    return home;
}

#endif

#if !TARGET_OS_WASI
#define CFMaxHostNameLength	256
#define CFMaxHostNameSize	(CFMaxHostNameLength+1)

CF_PRIVATE CFStringRef _CFStringCreateHostName(void) {
    char myName[CFMaxHostNameSize];

    // return @"" instead of nil a la CFUserName() and Ali Ozer
    if (0 != gethostname(myName, CFMaxHostNameSize)) myName[0] = '\0';
    return CFStringCreateWithCString(kCFAllocatorSystemDefault, myName, kCFPlatformInterfaceStringEncoding);
}

/* These are sanitized versions of the above functions. We might want to eliminate the above ones someday.
   These can return NULL.
*/
CF_EXPORT CFStringRef CFGetUserName(void) CF_RETURNS_RETAINED {
    return CFCopyUserName();
}

CF_EXPORT CFStringRef CFCopyUserName(void) {
    CFStringRef result = NULL;
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
    uid_t euid;
    __CFGetUGIDs(&euid, NULL);
    struct passwd *upwd = getpwuid(euid ? euid : getuid());
    if (upwd && upwd->pw_name) {
        result = CFStringCreateWithCString(kCFAllocatorSystemDefault, upwd->pw_name, kCFPlatformInterfaceStringEncoding);
    } else {
        const char *cuser = __CFgetenv("USER");
        if (cuser) {
            result = CFStringCreateWithCString(kCFAllocatorSystemDefault, cuser, kCFPlatformInterfaceStringEncoding);
        }
    }
#elif TARGET_OS_WIN32
	wchar_t username[1040];
	DWORD size = 1040;
	username[0] = 0;
	if (GetUserNameW(username, &size)) {
	    // discount the extra NULL by decrementing the size
	    result = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)username, size - 1);
	} else {
	    const char *cname = __CFgetenv("USERNAME");
	    if (cname) {
                result = CFStringCreateWithCString(kCFAllocatorSystemDefault, cname, kCFPlatformInterfaceStringEncoding);
            }
	}
#else
#error "Please add an implementation for CFCopyUserName() that copies the account username"
#endif
    if (!result)
        result = (CFStringRef)CFRetain(CFSTR(""));
    return result;
}

#if TARGET_OS_ANDROID
#define pw_gecos pw_name
#endif

CF_CROSS_PLATFORM_EXPORT CFStringRef CFCopyFullUserName(void) {
    CFStringRef result = NULL;
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
    uid_t euid;
    __CFGetUGIDs(&euid, NULL);
    struct passwd *upwd = getpwuid(euid ? euid : getuid());
    if (upwd && upwd->pw_gecos) {
        result = CFStringCreateWithCString(kCFAllocatorSystemDefault, upwd->pw_gecos, kCFPlatformInterfaceStringEncoding);
    }
#elif TARGET_OS_WIN32
    ULONG ulLength = 0;
    GetUserNameExW(NameDisplay, NULL, &ulLength);

    WCHAR *wszBuffer[ulLength + 1];
    GetUserNameExW(NameDisplay, (LPWSTR)wszBuffer, &ulLength);

    result = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (UniChar *)wszBuffer, ulLength);
#else
#error "Please add an implementation for CFCopyFullUserName() that copies the full (display) user name"
#endif
    if (!result) {
        result = (CFStringRef)CFRetain(CFSTR(""));
    }
    
    return result;
}

#if TARGET_OS_ANDROID
#undef pw_gecos
#endif


CFURLRef CFCopyHomeDirectoryURL(void) {
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
    return _CFCopyHomeDirURLForUser(NULL, true);
#elif TARGET_OS_WIN32
    CFURLRef retVal = NULL;
    CFIndex len = 0;
    CFStringRef str = NULL;
   
    UniChar pathChars[MAX_PATH];
    if (S_OK == SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, (wchar_t *)pathChars)) {
        len = (CFIndex)wcslen((wchar_t *)pathChars);
        str = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, pathChars, len);
        retVal = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str, kCFURLWindowsPathStyle, true);
        CFRelease(str);
    }

    if (!retVal) {
        // Fall back to environment variable, but this will not be unicode compatible
        const char *cpath = __CFgetenv("HOMEPATH");
        const char *cdrive = __CFgetenv("HOMEDRIVE");
        if (cdrive && cpath) {
            char fullPath[CFMaxPathSize];
            strlcpy(fullPath, cdrive, sizeof(fullPath));
            strlcat(fullPath, cpath, sizeof(fullPath));
            str = CFStringCreateWithCString(kCFAllocatorSystemDefault, fullPath, kCFPlatformInterfaceStringEncoding);
            retVal = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str, kCFURLWindowsPathStyle, true);
            CFRelease(str);
        }
    }

    if (!retVal) {
        // Last resort: We have to get "some" directory location, so fall-back to the processes current directory.
        UniChar currDir[MAX_PATH];
        DWORD dwChars = GetCurrentDirectoryW(MAX_PATH + 1, (wchar_t *)currDir);
        if (dwChars > 0) {
            len = (CFIndex)wcslen((wchar_t *)currDir);
            str = CFStringCreateWithCharacters(kCFAllocatorDefault, currDir, len);
            retVal = CFURLCreateWithFileSystemPath(NULL, str, kCFURLWindowsPathStyle, true);
            CFRelease(str);
        }
    }

    // We could do more here (as in KB Article Q101507). If that article is to be believed, we should only run into this case on Win95, or through user error.
    CFStringRef testPath = CFURLCopyFileSystemPath(retVal, kCFURLWindowsPathStyle);
    if (CFStringGetLength(testPath) == 0) {
        CFRelease(retVal);
        retVal = NULL;
    }
    if (testPath) CFRelease(testPath);

    return retVal;
#else
#error Dont know how to compute users home directories on this platform
#endif
}

CF_EXPORT CFURLRef CFCopyHomeDirectoryURLForUser(CFStringRef uName) {
#if TARGET_IPHONE_SIMULATOR
    if (!uName) { // TODO: Handle other cases here? See <rdar://problem/18504645> SIM: CFCopyHomeDirectoryURLForUser should not call getpwuid
        static CFURLRef home;
        static dispatch_once_t once;
        
        dispatch_once(&once, ^{
            const char *env = getenv("CFFIXED_USER_HOME");
            if (!env) {
                env = getenv("HOME");
            }
            if (env) {
                CFStringRef str = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, env);
                if (str) {
                    home = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str, kCFURLPOSIXPathStyle, true);
                    CFRelease(str);
                }
            }
        });
        
        if (home) {
            return CFRetain(home);
        }
    }
#endif
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
    if (!uName) {
        return _CFCopyHomeDirURLForUser(NULL, true);
    } else {
        char buf[128], *user;
        SInt32 len = CFStringGetLength(uName), size = CFStringGetMaximumSizeForEncoding(len, kCFPlatformInterfaceStringEncoding);
        CFIndex usedSize;
        if (size < 127) {
            user = buf;
        } else {
            user = CFAllocatorAllocate(kCFAllocatorSystemDefault, size+1, 0);
        }
        CFURLRef result = NULL;
        if (CFStringGetBytes(uName, CFRangeMake(0, len), kCFPlatformInterfaceStringEncoding, 0, true, (uint8_t *)user, size, &usedSize) == len) {
            user[usedSize] = '\0';
            result = _CFCopyHomeDirURLForUser(user, false);
        } else {
            result = _CFCopyHomeDirURLForUser(NULL, false);
        }
        if (buf != user) {
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, user);
        }
        return result;
    }
#elif TARGET_OS_WIN32
    // This code can only get the directory for the current user
    CFStringRef userName = uName ? CFCopyUserName() : NULL;
    if (uName && !CFEqual(uName, userName)) {
        CFLog(kCFLogLevelError, CFSTR("CFCopyHomeDirectoryURLForUser(): Unable to get home directory for other user"));
        if (userName) CFRelease(userName);
        return NULL;
    }
    if (userName) CFRelease(userName);
    return CFCopyHomeDirectoryURL();
#else
#error Dont know how to compute users home directories on this platform
#endif
}


#undef CFMaxHostNameLength
#undef CFMaxHostNameSize
#endif // !TARGET_OS_WASI

#if TARGET_OS_WIN32
CF_INLINE CFIndex strlen_UniChar(const UniChar* p) {
	CFIndex result = 0;
	while ((*p++) != 0)
		++result;
	return result;
}

//#include <shfolder.h>
/*
 * _CFCreateApplicationRepositoryPath returns the path to the application's
 * repository in a CFMutableStringRef. The path returned will be:
 *     <nFolder_path>\Apple Computer\<bundle_name>\
 * or if the bundle name cannot be obtained:
 *     <nFolder_path>\Apple Computer\
 * where nFolder_path is obtained by calling SHGetFolderPath with nFolder
 * (for example, with CSIDL_APPDATA or CSIDL_LOCAL_APPDATA).
 *
 * The CFMutableStringRef result must be released by the caller.
 *
 * If anything fails along the way, the result will be NULL.  
 */
CF_EXPORT CFMutableStringRef _CFCreateApplicationRepositoryPath(CFAllocatorRef alloc, int nFolder) {
    CFMutableStringRef result = NULL;
    UniChar szPath[MAX_PATH];
    
    // get the current path to the data repository: CSIDL_APPDATA (roaming) or CSIDL_LOCAL_APPDATA (nonroaming)
    if (S_OK == SHGetFolderPathW(NULL, nFolder, NULL, 0, (wchar_t *) szPath)) {
	CFStringRef directoryPath;
	
	// make it a CFString
	directoryPath = CFStringCreateWithCharacters(alloc, szPath, strlen_UniChar(szPath));
	if (directoryPath) {
	    CFBundleRef bundle;
	    CFStringRef bundleName;
	    CFStringRef completePath;
	    
	    // attempt to get the bundle name
	    bundle = CFBundleGetMainBundle();
	    if (bundle) {
		bundleName = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleNameKey);
	    }
	    else {
		bundleName = NULL;
	    }
	    
	    if (bundleName) {
		// the path will be "<directoryPath>\Apple Computer\<bundleName>\" if there is a bundle name
		completePath = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@\\Apple Computer\\%@\\"), directoryPath, bundleName);
	    }
	    else {
		// or "<directoryPath>\Apple Computer\" if there is no bundle name.
		completePath = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@\\Apple Computer\\"), directoryPath);
	    }

	    CFRelease(directoryPath);

	    // make a mutable copy to return
	    if (completePath) {
		result = CFStringCreateMutableCopy(alloc, 0, completePath);
		CFRelease(completePath);
	    }
	}
    }

    return ( result );
}
#endif



#pragma mark -
#pragma mark Thread Functions

#if TARGET_OS_WIN32

CF_EXPORT void _NS_pthread_setname_np(const char *name) {
  _CFThreadSetName(GetCurrentThread(), name);
}

static _CFThreadRef __initialPthread = INVALID_HANDLE_VALUE;

CF_EXPORT int _NS_pthread_main_np() {
    if (__initialPthread == INVALID_HANDLE_VALUE)
      DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
                      GetCurrentProcess(), &__initialPthread, 0, FALSE,
                      DUPLICATE_SAME_ACCESS);
    return CompareObjectHandles(__initialPthread, GetCurrentThread());
}

CF_EXPORT bool _NS_pthread_equal(_CFThreadRef t1, _CFThreadRef t2) {
  return CompareObjectHandles(t1, t2) == TRUE;
}

#endif

#pragma mark -
#pragma mark Thread Local Data

// If slot >= CF_TSD_MAX_SLOTS, the SPI functions will crash at NULL + slot address.
// If thread data has been torn down, these functions should crash on CF_TSD_BAD_PTR + slot address.
#define CF_TSD_MAX_SLOTS 70


// Windows and Linux, not sure how many times the destructor could get called; CF_TSD_MAX_DESTRUCTOR_CALLS could be 1

#define CF_TSD_BAD_PTR ((void *)0x1000)

typedef void (*tsdDestructor)(void *);

// Data structure to hold TSD data, cleanup functions for each
typedef struct __CFTSDTable {
    uint32_t destructorCount;
    uintptr_t data[CF_TSD_MAX_SLOTS];
    tsdDestructor destructors[CF_TSD_MAX_SLOTS];
} __CFTSDTable;

static void __CFTSDFinalize(void *arg);

#if TARGET_OS_WIN32

static DWORD __CFTSDIndexKey = 0xFFFFFFFF;

// Called from CFRuntime's startup code, on Windows only
CF_PRIVATE void __CFTSDWindowsInitialize() {
    __CFTSDIndexKey = FlsAlloc(__CFTSDFinalize);
}

// Called from CFRuntime's cleanup code, on Windows only
CF_PRIVATE void __CFTSDWindowsCleanup() {
    FlsFree(__CFTSDIndexKey);
}

#else

static _CFThreadSpecificKey __CFTSDIndexKey;

#if TARGET_OS_WASI
static void *__CFThreadSpecificData;
#endif

CF_PRIVATE void __CFTSDInitialize() {
#if !TARGET_OS_WASI
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        (void)pthread_key_create(&__CFTSDIndexKey, __CFTSDFinalize);
    });
#endif
}

#endif

static void __CFTSDSetSpecific(void *arg) {
#if TARGET_OS_MAC
    pthread_setspecific(__CFTSDIndexKey, arg);
#elif TARGET_OS_LINUX || TARGET_OS_BSD
    pthread_setspecific(__CFTSDIndexKey, arg);
#elif TARGET_OS_WIN32
    FlsSetValue(__CFTSDIndexKey, arg);
#elif TARGET_OS_WASI
    __CFThreadSpecificData = arg;
#endif
}

static void *__CFTSDGetSpecific() {
#if TARGET_OS_MAC
    return pthread_getspecific(__CFTSDIndexKey);
#elif TARGET_OS_LINUX || TARGET_OS_BSD
    return pthread_getspecific(__CFTSDIndexKey);
#elif TARGET_OS_WIN32
    return FlsGetValue(__CFTSDIndexKey);
#elif TARGET_OS_WASI
    return __CFThreadSpecificData;
#endif
}

_Atomic(bool) __CFMainThreadHasExited = false;

static void __CFTSDFinalize(void *arg) {
#if TARGET_OS_WASI
    __CFMainThreadHasExited = true;
#else
    if (_CFIsMainThread()) {
        // Important: we need to be sure that the only time we set this flag to true is when we actually can guarentee we ARE the main thread. 
        __CFMainThreadHasExited = true;
    }
#endif
    
    // Set our TSD so we're called again by pthreads. It will call the destructor PTHREAD_DESTRUCTOR_ITERATIONS times as long as a value is set in the thread specific data. We handle each case below.
    __CFTSDSetSpecific(arg);

    if (!arg || arg == CF_TSD_BAD_PTR) {
        // We've already been destroyed. The call above set the bad pointer again. Now we just return.
        return;
    }
    
    __CFTSDTable *table = (__CFTSDTable *)arg;
    table->destructorCount++;
        
    // On first calls invoke destructor. Later we destroy the data.
    // Note that invocation of the destructor may cause a value to be set again in the per-thread data slots. The destructor count and destructors are preserved.  
    // This logic is basically the same as what pthreads does. We just skip the 'created' flag.
    for (int32_t i = 0; i < CF_TSD_MAX_SLOTS; i++) {
        if (table->data[i] && table->destructors[i]) {
            uintptr_t old = table->data[i];
            table->data[i] = (uintptr_t)NULL;
            table->destructors[i]((void *)(old));
        }
    }

#if _POSIX_THREADS && !TARGET_OS_WASI
    if (table->destructorCount == PTHREAD_DESTRUCTOR_ITERATIONS - 1) {    // On PTHREAD_DESTRUCTOR_ITERATIONS-1 call, destroy our data
        free(table);
        
        // Now if the destructor is called again we will take the shortcut at the beginning of this function.
        __CFTSDSetSpecific(CF_TSD_BAD_PTR);
        return;
    }
#else
    free(table);
    __CFTSDSetSpecific(CF_TSD_BAD_PTR);
#endif
}

#if TARGET_OS_MAC
extern int pthread_key_init_np(int, void (*)(void *));
#endif

// Get or initialize a thread local storage. It is created on demand.
static __CFTSDTable *__CFTSDGetTable(const Boolean create) {
    __CFTSDTable *table = (__CFTSDTable *)__CFTSDGetSpecific();
    // Make sure we're not setting data again after destruction.
    if (table == CF_TSD_BAD_PTR) {
        return NULL;
    }
    // Create table on demand
    if (!table && create) {
        // This memory is freed in the finalize function
        table = (__CFTSDTable *)calloc(1, sizeof(__CFTSDTable));
        // Windows and Linux have created the table already, we need to initialize it here for other platforms. On Windows, the cleanup function is called by DllMain when a thread exits. On Linux the destructor is set at init time.
#if !TARGET_OS_WIN32
        __CFTSDInitialize();
#endif
        __CFTSDSetSpecific(table);
    }
    
    return table;
}


// For the use of CF and Foundation only
CF_EXPORT void *_CFGetTSDCreateIfNeeded(const uint32_t slot, const Boolean create) CF_RETURNS_NOT_RETAINED {
    if (slot >= CF_TSD_MAX_SLOTS) {
        _CFLogSimple(kCFLogLevelError, "Error: TSD slot %d out of range (get)", slot);
        HALT;
    }
    void * result = NULL;
    __CFTSDTable *table = __CFTSDGetTable(create);
    if (table) {
        uintptr_t *slots = (uintptr_t *)(table->data);
        result = (void *)slots[slot];
    }
    else if (create) {
        // Someone is getting TSD during thread destruction. The table is gone, so we can't get any data anymore.
        _CFLogSimple(kCFLogLevelWarning, "Warning: TSD slot %d retrieved but the thread data has already been torn down.", slot);
        return NULL;
    }
    return result;
}

// For the use of CF and Foundation only
CF_EXPORT void *_CFGetTSD(uint32_t slot) {
    return _CFGetTSDCreateIfNeeded(slot, true);
}

// For the use of CF and Foundation only
CF_EXPORT void *_CFSetTSD(uint32_t slot, void *newVal, tsdDestructor destructor) {
    if (slot >= CF_TSD_MAX_SLOTS) {
        _CFLogSimple(kCFLogLevelError, "Error: TSD slot %d out of range (set)", slot);
        HALT;
    }
    __CFTSDTable *table = __CFTSDGetTable(true);
    if (!table) {
        // Someone is setting TSD during thread destruction. The table is gone, so we can't get any data anymore.
        _CFLogSimple(kCFLogLevelWarning, "Warning: TSD slot %d set but the thread data has already been torn down.", slot);
        return NULL;
    }

    void *oldVal = (void *)table->data[slot];
    
    table->data[slot] = (uintptr_t)newVal;
    table->destructors[slot] = destructor;
    
    return oldVal;
}


#pragma mark -
#pragma mark Windows Wide to UTF8 and UTF8 to Wide

#if TARGET_OS_WIN32
/* On Windows, we want to use UTF-16LE for path names to get full unicode support. Internally, however, everything remains in UTF-8 representation. These helper functions stand between CF and the Microsoft CRT to ensure that we are using the right representation on both sides. */

#include <sys/stat.h>
#include <share.h>

// Creates a buffer of wchar_t to hold a UTF16LE version of the UTF8 str passed in. Caller must free the buffer when done. If resultLen is non-NULL, it is filled out with the number of characters in the string.
static wchar_t *createWideFileSystemRepresentation(const char *str, CFIndex *resultLen) {
    // Get the real length of the string in UTF16 characters
    CFStringRef cfStr = CFStringCreateWithCString(kCFAllocatorSystemDefault, str, kCFStringEncodingUTF8);
    CFIndex strLen = CFStringGetLength(cfStr);
    
    // Allocate a wide buffer to hold the converted string, including space for a NULL terminator
    wchar_t *wideBuf = (wchar_t *)malloc((strLen + 1) * sizeof(wchar_t));
    
    // Copy the string into the buffer and terminate
    CFStringGetCharacters(cfStr, CFRangeMake(0, strLen), (UniChar *)wideBuf);
    wideBuf[strLen] = 0;
    
    CFRelease(cfStr);
    if (resultLen) *resultLen = strLen;
    return wideBuf;
}

// Copies a UTF16 buffer into a supplied UTF8 buffer. 
static void copyToNarrowFileSystemRepresentation(const wchar_t *wide, CFIndex dstBufSize, char *dstbuf) {
    // Get the real length of the wide string in UTF8 characters
    CFStringRef cfStr = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)wide, wcslen(wide));
    CFIndex strLen = CFStringGetLength(cfStr);
    CFIndex bytesUsed;
    
    // Copy the wide string into the buffer and terminate
    CFStringGetBytes(cfStr, CFRangeMake(0, strLen), kCFStringEncodingUTF8, 0, false, (uint8_t *)dstbuf, dstBufSize, &bytesUsed);
    dstbuf[bytesUsed] = 0;
    
    CFRelease(cfStr);
}

CF_EXPORT int _NS_stat(const char *name, struct _stat *st) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    int res = _wstat(wide, st);
    free(wide);
    return res;
}

CF_EXPORT int _NS_mkdir(const char *name) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    int res = _wmkdir(wide);
    free(wide);
    return res;
}

CF_EXPORT int _NS_rmdir(const char *name) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    int res = _wrmdir(wide);
    free(wide);
    return res;
}

CF_EXPORT int _NS_chmod(const char *name, int mode) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    
    // Convert mode
    int newMode = 0;
    if (mode | 0400) newMode |= _S_IREAD;
    if (mode | 0200) newMode |= _S_IWRITE;
    if (mode | 0100) newMode |= _S_IEXEC;
    
    int res = _wchmod(wide, newMode);
    free(wide);
    return res;
}

CF_EXPORT int _NS_unlink(const char *name) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    int res = _wunlink(wide);
    free(wide);
    return res;
}

// Warning: this doesn't support dstbuf as null even though 'getcwd' does
CF_EXPORT char *_NS_getcwd(char *dstbuf, size_t size) {
    if (!dstbuf) {
	CFLog(kCFLogLevelWarning, CFSTR("CFPlatform: getcwd called with null buffer"));
	return 0;
    }
    
    wchar_t *buf = _wgetcwd(NULL, 0);
    if (!buf) {
        return NULL;
    }
        
    // Convert result to UTF8
    copyToNarrowFileSystemRepresentation(buf, (CFIndex)size, dstbuf);
    free(buf);
    return dstbuf;
}

CF_EXPORT char *_NS_getenv(const char *name) {
    // todo: wide getenv
    // We have to be careful what happens here, because getenv is called during cf initialization, and things like cfstring may not be working yet
    return getenv(name);
}

CF_EXPORT int _NS_rename(const char *oldName, const char *newName) {
    wchar_t *oldWide = createWideFileSystemRepresentation(oldName, NULL);
    wchar_t *newWide = createWideFileSystemRepresentation(newName, NULL);
    // _wrename on Windows does not behave exactly as rename() on Mac OS -- if the file exists, the Windows one will fail whereas the Mac OS version will replace
    // To simulate the Mac OS behavior, we use the Win32 API then fill out errno if something goes wrong
    BOOL winRes = MoveFileExW(oldWide, newWide, MOVEFILE_REPLACE_EXISTING);
    DWORD error = GetLastError();
    if (!winRes) {
	    switch (error) {
            case ERROR_SUCCESS:
                errno = 0;
                break;
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_OPEN_FAILED:
                errno = ENOENT;
                break;
            case ERROR_ACCESS_DENIED:
                errno = EACCES;
                break;
            default:
                errno = error;
        }
    }
    free(oldWide);
    free(newWide);
    return (winRes ? 0 : -1);
}

CF_EXPORT int _NS_open(const char *name, int oflag, int pmode) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);

    DWORD dwDesiredAccess = 0;
    switch (oflag & (O_RDONLY | O_WRONLY | O_RDWR)) {
      case _O_RDONLY:
        dwDesiredAccess = GENERIC_READ;
        break;
      case _O_WRONLY:
        dwDesiredAccess = GENERIC_WRITE;
        break;
      case _O_RDWR:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    }

    DWORD dwCreationDisposition;
    switch (oflag & (O_CREAT | O_EXCL | O_TRUNC)) {
      case O_CREAT | O_EXCL | O_TRUNC:
      case O_CREAT | O_EXCL:
        dwCreationDisposition = CREATE_NEW;
        break;
      case O_CREAT | O_TRUNC:
        dwCreationDisposition = CREATE_ALWAYS;
        break;
      case O_CREAT:
        dwCreationDisposition = OPEN_ALWAYS;
        break;
      case O_TRUNC:
        dwCreationDisposition = TRUNCATE_EXISTING;
        break;
      default:
        dwCreationDisposition = OPEN_EXISTING;
    }

    // Backup semantics are required to receive a handle to a directory
    DWORD dwFlagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS;
    if (!(pmode & _S_IWRITE)) {
      dwFlagsAndAttributes |= FILE_ATTRIBUTE_READONLY;
    }

    HANDLE handle = CreateFileW(wide, dwDesiredAccess, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        /* lpSecurityAttributes= */NULL, dwCreationDisposition, dwFlagsAndAttributes,
        /* hTemplatefile= */ NULL);
    free(wide);
    if (handle == INVALID_HANDLE_VALUE) {
      DWORD error = GetLastError();
      switch (error) {
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
          errno = EACCES;
          break;
        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS:
          errno = EEXIST;
          break;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
          errno = ENOENT;
          break;
        default:
          errno = EIO;
          break;
      }
      return -1;
    }

    // _open_osfhandle handles _O_APPEND and _O_RDONLY
    int fd = _open_osfhandle((intptr_t)handle, oflag);
    if (fd == -1) {
      CloseHandle(handle);
      return -1;
    }
    return fd;
}

CF_EXPORT int _NS_chdir(const char *name) {
    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    int res = _wchdir(wide);
    free(wide);
    return res;
}

CF_EXPORT int _NS_access(const char *name, int amode) {
    // execute is always true
    if (amode == 1) return 0;

    wchar_t *wide = createWideFileSystemRepresentation(name, NULL);
    // we only care about the read-only (04) and write-only (02) bits, so mask octal 06
    int res = _waccess(wide, amode & 06);
    free(wide);
    return res;
}

// This is a bit different than the standard 'mkstemp', because the size parameter is needed so we know the size of the UTF8 buffer
// Also, we don't avoid the race between creating a temporary file name and opening it on Windows like we do on Mac
CF_EXPORT int _NS_mkstemp(char *name, int bufSize) {
    CFIndex nameLen;
    wchar_t *wide = createWideFileSystemRepresentation(name, &nameLen);
    
    // First check to see if the directory that this new temporary file will be created in exists. If not, set errno to ENOTDIR. This mimics the behavior of mkstemp on MacOS more closely.
    // Look for the last '\' in the path
    wchar_t *lastSlash = wcsrchr(wide, '\\');
    if (!lastSlash) {
	free(wide);
	return -1;
    }
    
    // Set the last slash to NULL temporarily and use it for _wstat
    *lastSlash = 0;
    struct _stat dirInfo;
    int res = _wstat(wide, &dirInfo);
    if (res < 0) {
	if (errno == ENOENT) {
	    errno = ENOTDIR;
	}
	free(wide);
	return -1;
    }
    // Restore the last slash
    *lastSlash = '\\';
    
    errno_t err = _wmktemp_s(wide, nameLen + 1);
    if (err != 0) {
        free(wide);
        return 0;
    }
    
    int fd;
    _wsopen_s(&fd, wide, _O_RDWR | _O_CREAT | CF_OPENFLGS, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    
    // Convert the wide name back into the UTF8 buffer the caller supplied
    copyToNarrowFileSystemRepresentation(wide, bufSize, name);
    free(wide);
    return fd;    
}


// Utilities to convert from a volume name to a drive letter

Boolean _isAFloppy(char driveLetter)
{
    HANDLE h;
    TCHAR tsz[8];
    Boolean retval = false;
    int iDrive;
    
    if (driveLetter >= 'a' && driveLetter <= 'z') {
        driveLetter = driveLetter - 'a' + 'A';
    }
    
    if ((driveLetter < 'A') || (driveLetter > 'Z')) {
        // invalid driveLetter; I guess it's not a floppy...
        return false;
    }
    
    iDrive = driveLetter - 'A' + 1;
    
    // On Windows NT, use the technique described in the Knowledge Base article Q115828 and in the "FLOPPY" SDK sample.
    wsprintf(tsz, TEXT("\\\\.\\%c:"), TEXT('@') + iDrive);
    h = CreateFile(tsz, 0, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (h != INVALID_HANDLE_VALUE)
    {
        DISK_GEOMETRY Geom[20];
        DWORD cb;
        
        if (DeviceIoControl (h, IOCTL_DISK_GET_MEDIA_TYPES, 0, 0,
                             Geom, sizeof(Geom), &cb, 0)
            && cb > 0)
        {
            switch (Geom[0].MediaType)
            {
                case F5_1Pt2_512: // 5.25 1.2MB floppy
                case F5_360_512:  // 5.25 360K  floppy
                case F5_320_512:  // 5.25 320K  floppy
                case F5_320_1024: // 5.25 320K  floppy
                case F5_180_512:  // 5.25 180K  floppy
                case F5_160_512:  // 5.25 160K  floppy
                case F3_1Pt44_512: // 3.5 1.44MB floppy
                case F3_2Pt88_512: // 3.5 2.88MB floppy
                case F3_20Pt8_512: // 3.5 20.8MB floppy
                case F3_720_512:   // 3.5 720K   floppy
                    retval = true;
                    break;
            }
        }
        
        CloseHandle(h);
    }

    return retval;
}


extern CFStringRef CFCreateWindowsDrivePathFromVolumeName(CFStringRef volNameStr) {
    if (!volNameStr) return NULL;
    
    // This code is designed to match as closely as possible code from QuickTime's library
    CFIndex strLen = CFStringGetLength(volNameStr);
    if (strLen == 0) {
	return NULL;
    }
    
    // Get drive names
    long length, result;
    wchar_t *driveNames = NULL;
    
    // Get the size of the buffer to store the list of drives
    length = GetLogicalDriveStringsW(0, 0);
    if (!length) {
        return NULL;
    }
    
    driveNames = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
    result = GetLogicalDriveStringsW(length, driveNames);
    
    if (!result || result > length) {
        free(driveNames);
        return NULL;
    }
    
    // Get the volume name string into a wide buffer
    wchar_t *theVolumeName = (wchar_t *)malloc((strLen + 1) * sizeof(wchar_t));
    CFStringGetCharacters(volNameStr, CFRangeMake(0, strLen), (UniChar *)theVolumeName);
    theVolumeName[strLen] = 0;
    
    // lowercase volume name
    _wcslwr(theVolumeName);
    
    // Iterate through the drive names, looking for something that matches
    wchar_t *drivePtr = driveNames;
    CFStringRef drivePathResult = NULL;

    while (*drivePtr) {
        _wcslwr(drivePtr);
        
        if (!_isAFloppy((char)*drivePtr)) {
            UINT                oldErrorMode;
            DWORD               whoCares1, whoCares2;
            BOOL                getVolInfoSucceeded;
            UniChar             thisVolumeName[MAX_PATH];
            
            // Convert this drive string into a volume name
            oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
            getVolInfoSucceeded = GetVolumeInformationW(drivePtr, (LPWSTR)thisVolumeName, sizeof(thisVolumeName), NULL, &whoCares1, &whoCares2, NULL, 0);
            SetErrorMode(oldErrorMode);
            
            if (getVolInfoSucceeded) {
                _wcslwr((wchar_t *)thisVolumeName);
                
                // If the volume corresponding to this drive matches the input volume
                // then this drive is the winner.
                if (!wcscmp((const wchar_t *)thisVolumeName, theVolumeName) || 
                    (*thisVolumeName == 0x00 && (CFStringCompare(volNameStr, CFSTR("NONAME"), 0) == kCFCompareEqualTo))) {
                    drivePathResult = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, (const UniChar *)drivePtr, wcslen(drivePtr));
                    break;
                }
            }
        }
        
        drivePtr += wcslen(drivePtr) + 1;
    }
    
    
    free(driveNames);
    free(theVolumeName);
    return drivePathResult;
}

struct timezone {
    int	tz_minuteswest;	/* minutes west of Greenwich */
    int	tz_dsttime;	/* type of dst correction */
};

CF_PRIVATE int _NS_gettimeofday(struct timeval *tv, struct timezone *tz) {
    if (tv) {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        unsigned __int64 t = 0;
        t |= ft.dwHighDateTime;
        t <<= 32;
        t |= ft.dwLowDateTime;
        
        // Convert to microseconds
        t /= 10;
        
        // Difference between 1/1/1970 and 1/1/1601
        t -= 11644473600000000Ui64;
        
        // Convert microseconds to seconds
        tv->tv_sec = (long)(t / 1000000UL);
        tv->tv_usec = (long)(t % 1000000UL);
    }
    
    // We don't support tz
    return 0;
}

#endif // TARGET_OS_WIN32

#pragma mark -
#pragma mark Linux, BSD, and WASI OSAtomic

#if TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI

bool OSAtomicCompareAndSwapPtr(void *oldp, void *newp, void *volatile *dst) 
{ 
    return __sync_bool_compare_and_swap(dst, oldp, newp);
}

bool OSAtomicCompareAndSwapLong(long oldl, long newl, long volatile *dst) 
{ 
    return __sync_val_compare_and_swap(dst, oldl, newl);
}

bool OSAtomicCompareAndSwapPtrBarrier(void *oldp, void *newp, void *volatile *dst) 
{ 
    return __sync_bool_compare_and_swap(dst, oldp, newp);
}

int32_t OSAtomicAdd32Barrier( int32_t theAmount, volatile int32_t *theValue ) {
    return __sync_fetch_and_add(theValue, theAmount) + theAmount;
}

int64_t OSAtomicAdd64( int64_t theAmount, volatile int64_t *theValue ) {
    return __sync_fetch_and_add(theValue, theAmount) + theAmount;
}

bool OSAtomicCompareAndSwap32Barrier(int32_t oldValue, int32_t newValue, volatile int32_t *theValue) {
    return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

bool OSAtomicCompareAndSwap64Barrier(int64_t oldValue, int64_t newValue, volatile int64_t *theValue) {
    return __sync_bool_compare_and_swap(theValue, oldValue, newValue);
}

int32_t OSAtomicDecrement32Barrier(volatile int32_t *dst)
{
    return OSAtomicAdd32Barrier(-1, dst);
}

int32_t OSAtomicIncrement32Barrier(volatile int32_t *dst)
{
    return OSAtomicAdd32Barrier(1, dst);
}

int32_t OSAtomicAdd32( int32_t theAmount, volatile int32_t *theValue ) {
    return OSAtomicAdd32Barrier(theAmount, theValue);
}

int32_t OSAtomicIncrement32(volatile int32_t *theValue) {
    return OSAtomicIncrement32Barrier(theValue);
}

int32_t OSAtomicDecrement32(volatile int32_t *theValue) {
    return OSAtomicDecrement32Barrier(theValue);
}

void OSMemoryBarrier() {
    __sync_synchronize();
}

#endif // TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI

#pragma mark -
#pragma mark Dispatch Replacements

#if !__HAS_DISPATCH__

#include <semaphore.h>

typedef struct _CF_sema_s {
    sem_t sema;
} * _CF_sema_t;

CF_INLINE void _CF_sem_signal(_CF_sema_t s) {
#if !TARGET_OS_WASI
    sem_post(&s->sema);
#endif
}

CF_INLINE void _CF_sem_wait(_CF_sema_t s) {
#if !TARGET_OS_WASI
    sem_wait(&s->sema);
#endif
}

static void _CF_sem_destroy(_CF_sema_t s) {
    free(s);
}

CF_INLINE _CFThreadSpecificKey _CF_thread_sem_key() {
    static _CFThreadSpecificKey key = 0;
#if !TARGET_OS_WASI
    static OSSpinLock lock = OS_SPINLOCK_INIT;
    if (key == 0) {
        OSSpinLockLock(&lock);
        if (key == 0) {
            pthread_key_create(&key, (void (*)(void *))&_CF_sem_destroy);
        }
        OSSpinLockUnlock(&lock);
    }
#endif
    return key;
}

#if !TARGET_OS_WASI
CF_INLINE _CF_sema_t _CF_get_thread_semaphore() {
    _CFThreadSpecificKey key = _CF_thread_sem_key();
    _CF_sema_t s = (_CF_sema_t)pthread_getspecific(key);
    if (s == NULL) {
        s = malloc(sizeof(struct _CF_sema_s));
        pthread_setspecific(key, s);
    }
    return s;
    
}

CF_INLINE void _CF_put_thread_semaphore(_CF_sema_t s) {
    pthread_setspecific(_CF_thread_sem_key(), s);
}
#endif

#define CF_DISPATCH_ONCE_DONE ((_CF_dispatch_once_waiter_t)~0l)

typedef struct _CF_dispatch_once_waiter_s {
    volatile struct _CF_dispatch_once_waiter_s *volatile dow_next;
    _CF_sema_t dow_sema;
    _CFThreadRef dow_thread;
} *_CF_dispatch_once_waiter_t;

#if defined(__x86_64__) || defined(__i386__)
#define _CF_hardware_pause() __asm__("pause")
#elif (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) || \
defined(__arm64__)
#define _CF_hardware_pause() __asm__("yield")
#else
#define _CF_hardware_pause() __asm__("")
#endif

void _CF_dispatch_once(dispatch_once_t *predicate, void (^block)(void)) {
#if TARGET_OS_WASI
    if (!*predicate) {
        block();
        *predicate = 1;
    }
#else
    _CF_dispatch_once_waiter_t volatile *vval = (_CF_dispatch_once_waiter_t*)predicate;
    struct _CF_dispatch_once_waiter_s dow = { NULL };
    _CF_dispatch_once_waiter_t tail = &dow, next, tmp;
    _CF_sema_t sema;
    if (__sync_bool_compare_and_swap(vval, NULL, tail)) {
        dow.dow_thread = pthread_self();
        block();
        __sync_synchronize();
        next = (_CF_dispatch_once_waiter_t)__sync_swap((vval), (CF_DISPATCH_ONCE_DONE));
        while (next != tail) {
            while (!(tmp = (_CF_dispatch_once_waiter_t)next->dow_next)) {
                _CF_hardware_pause();
            }
            sema = next->dow_sema;
            next = tmp;
            _CF_sem_signal(sema);
        }
    } else {
        dow.dow_sema = _CF_get_thread_semaphore();
        next = *vval;
        while (next != CF_DISPATCH_ONCE_DONE) {
            if (__sync_bool_compare_and_swap(vval, next, tail, &next)) {
                dow.dow_thread = next->dow_thread;
                dow.dow_next = next;
                _CF_sem_wait(dow.dow_sema);
                break;
            }
        }
        _CF_put_thread_semaphore(dow.dow_sema);
    }
#endif // TARGET_OS_WASI
}

#endif

#pragma mark -
#pragma mark Windows and Linux Helpers

#if TARGET_OS_WIN32 || (TARGET_OS_LINUX && !defined(_GNU_SOURCE))

#include <stdio.h>

CF_PRIVATE int asprintf(char **ret, const char *format, ...) {
    va_list args;
    size_t sz = 1024;
    *ret = (char *) malloc(sz * sizeof(char));
    if (!*ret) return -1;
    va_start(args, format);
    int cnt = vsnprintf(*ret, sz, format, args);
    va_end(args);
    if (cnt < sz - 1) return cnt;
    sz = cnt + 8;
    char *oldret = *ret;
    *ret = __CFSafelyReallocate(*ret, sz * sizeof(char), NULL);
    if (!*ret && oldret) free(oldret);
    if (!*ret) return -1;
    va_start(args, format);
    cnt = vsnprintf(*ret, sz, format, args);
    va_end(args);
    if (cnt < sz - 1) return cnt;
    free(*ret);
    *ret = NULL;
    return -1;
}

#endif

#if DEPLOYMENT_RUNTIME_SWIFT
#include <fcntl.h>

extern void swift_retain(void *);
extern void swift_release(void *);

#if TARGET_OS_WIN32
typedef struct _CFThreadSpecificData {
    CFTypeRef value;
    _CFThreadSpecificKey key;
} _CFThreadSpecificData;
#endif

static void _CFThreadSpecificDestructor(void *ctx) {
#if TARGET_OS_WIN32
    _CFThreadSpecificData *data = (_CFThreadSpecificData *)ctx;
    FlsSetValue(data->key, NULL);
    swift_release(data->value);
    free(data);
#else
    swift_release(ctx);
#endif
}

_CFThreadSpecificKey _CFThreadSpecificKeyCreate() {
    _CFThreadSpecificKey key;
#if TARGET_OS_WIN32
    key = FlsAlloc(_CFThreadSpecificDestructor);
#else
    pthread_key_create(&key, &_CFThreadSpecificDestructor);
#endif
    return key;
}

CFTypeRef _Nullable _CFThreadSpecificGet(_CFThreadSpecificKey key) {
#if TARGET_OS_WIN32
    _CFThreadSpecificData *data = (_CFThreadSpecificData *)FlsGetValue(key);
    if (data == NULL) {
        return NULL;
    }
    return data->value;
#else
    return (CFTypeRef)pthread_getspecific(key);
#endif
}

void _CFThreadSpecificSet(_CFThreadSpecificKey key, CFTypeRef _Nullable value) {
    // Intentionally not calling `swift_release` for previous value.
    // OperationQueue uses these API (through NSThreadSpecific), and balances
    // retain count manually.
#if TARGET_OS_WIN32
    free(FlsGetValue(key));

    _CFThreadSpecificData *data = NULL;
    if (value != NULL) {
        data = malloc(sizeof(_CFThreadSpecificData));
        if (!data) {
            HALT_MSG("Out of memory");
        }
        data->value = value;
        data->key = key;

        swift_retain((void *)value);
    }

    FlsSetValue(key, data);
#else
    if (value != NULL) {
        swift_retain((void *)value);
        pthread_setspecific(key, value);
    } else {
        pthread_setspecific(key, NULL);
    }
#endif
}

_CFThreadRef _CFThreadCreate(const _CFThreadAttributes attrs, void *_Nullable (* _Nonnull startfn)(void *_Nullable), void *_CF_RESTRICT _Nullable context) {
#if TARGET_OS_WIN32
    DWORD dwCreationFlags = 0;
    DWORD dwStackSize = 0;
    if (attrs.dwSizeOfAttributes >=
            offsetof(struct _CFThreadAttributes,
                     dwThreadStackReservation) + sizeof(dwStackSize)) {
      dwStackSize = attrs.dwThreadStackReservation;
      if (dwStackSize) {
        dwCreationFlags |= STACK_SIZE_PARAM_IS_A_RESERVATION;
      }
    }

    return (_CFThreadRef)_beginthreadex(NULL, dwStackSize,
                                        (_beginthreadex_proc_type)startfn,
                                        context, dwCreationFlags, NULL);
#else
    _CFThreadRef thread;
    pthread_create(&thread, &attrs, startfn, context);
    return thread;
#endif
}

CF_CROSS_PLATFORM_EXPORT int _CFThreadSetName(_CFThreadRef thread, const char *_Nonnull name) {
#if TARGET_OS_MAC
    if (pthread_equal(pthread_self(), thread)) {
        return pthread_setname_np(name);
    }
    return EINVAL;
#elif TARGET_OS_WIN32
    // Convert To UTF-16
    int szLength =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, NULL, 0);
    if (szLength == 0) {
        return EINVAL;
    }

    WCHAR *pszThreadDescription = calloc(szLength + 1, sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1,
                        pszThreadDescription, szLength);

    // Set Thread Name
    SetThreadDescription(thread, pszThreadDescription);

    // Free Conversion
    free(pszThreadDescription);

    return 0;
#elif TARGET_OS_LINUX
    return pthread_setname_np(thread, name);
#elif TARGET_OS_BSD
    pthread_set_name_np(thread, name);
    return 0;
#endif
}

CF_CROSS_PLATFORM_EXPORT int _CFThreadGetName(char *buf, int length) {
#if TARGET_OS_MAC
    return pthread_getname_np(pthread_self(), buf, length);
#elif TARGET_OS_ANDROID
    // Android did not get pthread_getname_np until API 26, but prctl seems to
    // return at most 15 chars of the name + null terminator.
    char *buffer[16] = {0};
    if (prctl(PR_GET_NAME, buffer, 0, 0, 0) != 0) {
        return -1;
    }
    size_t sz = MIN(strnlen(buffer, 15), length - 1);
    memcpy(buf, buffer, sz);
    buf[sz] = 0;
    return 0;
#elif TARGET_OS_LINUX
    return pthread_getname_np(pthread_self(), buf, length);
#elif TARGET_OS_BSD
    pthread_get_name_np(pthread_self(), buf, length);
    return 0;
#elif TARGET_OS_WIN32
    *buf = '\0';

    // Get Thread Name
    PWSTR pszThreadDescription = NULL;
    HRESULT hr = GetThreadDescription(GetCurrentThread(), &pszThreadDescription);
    if (FAILED(hr)) {
        return -1;
    }

    // Convert to UTF-8
    int szLength =
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pszThreadDescription,
                            -1, NULL, 0, NULL, NULL);
    if (szLength) {
        char *buffer = calloc(szLength + 1, sizeof(char));
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pszThreadDescription,
                            -1, buffer, szLength, NULL, NULL);
        memcpy(buf, buffer, MIN(szLength, length - 1));
        buf[MIN(szLength, length - 1)] = '\0';
        free(buffer);
    }

    // Free Result
    LocalFree(pszThreadDescription);

    return 0;
#endif
    return -1;
}

CF_EXPORT char **_CFEnviron(void) {
#if TARGET_OS_MAC
    return *_NSGetEnviron();
#elif TARGET_OS_WIN32
    return _environ;
#else
#if TARGET_OS_BSD || TARGET_OS_WASI
    extern char **environ;
#endif
    return environ;
#endif
}

#if TARGET_OS_WIN32
CF_CROSS_PLATFORM_EXPORT int _CFOpenFileWithMode(const unsigned short *path, int opts, mode_t mode) {
    return _wopen(path, opts, mode);
}
#else
CF_CROSS_PLATFORM_EXPORT int _CFOpenFileWithMode(const char *path, int opts, mode_t mode) {
    return open(path, opts, mode);
}
#endif

int _CFOpenFile(const char *path, int opts) {
    return open(path, opts, 0);
}

CF_CROSS_PLATFORM_EXPORT void *_CFReallocf(void *ptr, size_t size) {
#if TARGET_OS_WIN32 || TARGET_OS_LINUX || TARGET_OS_WASI || defined(__OpenBSD__)
    void *mem = realloc(ptr, size);
    if (mem == NULL && ptr != NULL && size != 0) {
        free(ptr);
    }
    return mem;
#else
    return reallocf(ptr, size);
#endif
}

#if TARGET_OS_ANDROID

#include <dlfcn.h>
#include <spawn.h>

// Android doesn't provide posix_spawn APIs until recent API level, so we cannot
// depend on them, but we can imitate the API, and perform the same work.

static pthread_once_t posixSpawnOnce = PTHREAD_ONCE_INIT;
static _CFPosixSpawnFileActionsRef (*_CFPosixSpawnFileActionsAllocImpl)(void);
static int (*_CFPosixSpawnFileActionsInitImpl)(_CFPosixSpawnFileActionsRef);
static int (*_CFPosixSpawnFileActionsDestroyImpl)(_CFPosixSpawnFileActionsRef);
static void (*_CFPosixSpawnFileActionsDeallocImpl)(_CFPosixSpawnFileActionsRef);
static int (*_CFPosixSpawnFileActionsAddDup2Impl)(_CFPosixSpawnFileActionsRef, int, int);
static int (*_CFPosixSpawnFileActionsAddCloseImpl)(_CFPosixSpawnFileActionsRef, int);
static int (*_CFPosixSpawnImpl)(pid_t *_CF_RESTRICT, const char *_CF_RESTRICT, _CFPosixSpawnFileActionsRef, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT, char *_Nullable const[_Nullable _CF_RESTRICT], char *_Nullable const[_Nullable _CF_RESTRICT]);

static _CFPosixSpawnFileActionsRef _CFPosixSpawnFileActionsAllocImplPost28() {
    _CFPosixSpawnFileActionsRef actions = malloc(sizeof(posix_spawn_file_actions_t));
    CFAssert(actions != NULL, __kCFLogAssertion, "malloc failed");
    return actions;
}

static void _CFPosixSpawnFileActionsDeallocImplBoth(_CFPosixSpawnFileActionsRef file_actions) {
    free(file_actions);
}

enum _CFPosixSpawnFileActionTypePre28 {
    _CFPosixSpawnFileActionDup2Pre28,
    _CFPosixSpawnFileActionClosePre28,
};

struct _CFPosixSpawnFileActionPre28 {
    enum _CFPosixSpawnFileActionTypePre28 type;
    union {
        struct {
            int filedes;
            int newfiledes;
        } dup2Action;
        struct {
            int filedes;
        } closeAction;
    };
};

struct _CFPosixSpawnFileActionsPre28 {
    struct _CFPosixSpawnFileActionPre28 *actions;
    size_t actionsCount;
    size_t actionsCapacity;
    int32_t isValid;
};

static const int32_t _CFPosixSpawnFileActionsPre28Valid = 0x600D600D;

static _CFPosixSpawnFileActionsRef _CFPosixSpawnFileActionsAllocImplPre28() {
    _CFPosixSpawnFileActionsRef actions = calloc(1, sizeof(struct _CFPosixSpawnFileActionsPre28));
    CFAssert(actions != NULL, __kCFLogAssertion, "malloc failed");
    return actions;
}

static int _CFPosixSpawnFileActionsInitImplPre28(_CFPosixSpawnFileActionsRef file_actions) {
    if (file_actions == NULL) {
        return EINVAL;
    }

    struct _CFPosixSpawnFileActionsPre28 *actions = (struct _CFPosixSpawnFileActionsPre28 *)file_actions;

    actions->actions = malloc(8 * sizeof(struct _CFPosixSpawnFileActionPre28));
    if (actions->actions == NULL) {
        return ENOMEM;
    }
    actions->actionsCount = 0;
    actions->actionsCapacity = 8;

    actions->isValid = _CFPosixSpawnFileActionsPre28Valid;

    return 0;
}

static int _CFPosixSpawnFileActionsDestroyImplPre28(_CFPosixSpawnFileActionsRef file_actions) {
    if (file_actions == NULL) {
        return EINVAL;
    }

    struct _CFPosixSpawnFileActionsPre28 *actions = (struct _CFPosixSpawnFileActionsPre28 *)file_actions;
    if (actions->isValid != _CFPosixSpawnFileActionsPre28Valid) {
        return EINVAL;
    }

    free(actions->actions);
    actions->actionsCount = 0;
    actions->actionsCapacity = 0;

    actions->isValid = 0;

    return 0;
}

static int _CFPosixSpawnFileActionsAddDup2ImplPre28(_CFPosixSpawnFileActionsRef file_actions, int filedes, int newfiledes) {
    if (file_actions == NULL) {
        return EINVAL;
    }

    if (filedes < 0 || newfiledes < 0) {
        return EBADF;
    }

    struct _CFPosixSpawnFileActionsPre28 *actions = (struct _CFPosixSpawnFileActionsPre28 *)file_actions;
    if (actions->isValid != _CFPosixSpawnFileActionsPre28Valid) {
        return EINVAL;
    }

    if (actions->actionsCount == actions->actionsCapacity) {
        struct _CFPosixSpawnFileActionPre28 *newActions = realloc(actions->actions, actions->actionsCapacity * 2);
        if (newActions == NULL) {
            return ENOMEM;
        }
        actions->actions = newActions;
        actions->actionsCapacity *= 2;
    }

    actions->actions[actions->actionsCount++] = (struct _CFPosixSpawnFileActionPre28) {
        .type = _CFPosixSpawnFileActionDup2Pre28,
        .dup2Action = {
            .filedes = filedes,
            .newfiledes = newfiledes
        }
    };

    return 0;
}

static int _CFPosixSpawnFileActionsAddCloseImplPre28(_CFPosixSpawnFileActionsRef file_actions, int filedes) {
    if (file_actions == NULL) {
        return EINVAL;
    }

    if (filedes < 0) {
        return EBADF;
    }

    struct _CFPosixSpawnFileActionsPre28 *actions = (struct _CFPosixSpawnFileActionsPre28 *)file_actions;
    if (actions->isValid != _CFPosixSpawnFileActionsPre28Valid) {
        return EINVAL;
    }

    if (actions->actionsCount == actions->actionsCapacity) {
        struct _CFPosixSpawnFileActionPre28 *newActions = realloc(actions->actions, actions->actionsCapacity * 2);
        if (newActions == NULL) {
            return ENOMEM;
        }
        actions->actions = newActions;
        actions->actionsCapacity *= 2;
    }

    actions->actions[actions->actionsCount++] = (struct _CFPosixSpawnFileActionPre28) {
        .type = _CFPosixSpawnFileActionClosePre28,
        .closeAction = {
            .filedes = filedes
        }
    };

    return 0;
}

static int _CFPosixSpawnImplPre28(pid_t *_CF_RESTRICT pid, const char *_CF_RESTRICT path, _CFPosixSpawnFileActionsRef file_actions, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT attrp, char *_Nullable const argv[_Nullable _CF_RESTRICT], char *_Nullable const envp[_Nullable _CF_RESTRICT]) {
    // TODO: We completely ignore attrp, because at the moment, the only
    // invocation doesn't pass a value.
    if (attrp != NULL) {
        return EINVAL;
    }

    struct _CFPosixSpawnFileActionsPre28 *actions = (struct _CFPosixSpawnFileActionsPre28 *)file_actions;
    if (actions != NULL && actions->isValid != _CFPosixSpawnFileActionsPre28Valid) {
        return EINVAL;
    }

    // Block signals during this fork/execv dance.
    sigset_t signalSet;
    sigfillset(&signalSet);
    sigset_t oldMask;
    if (sigprocmask(SIG_BLOCK, &signalSet, &oldMask) != 0) {
        CFAssert1(FALSE, __kCFLogAssertion, "sigprocmask() failed: %d", errno);
    }

    pid_t forkPid = fork();
    if (forkPid != 0) {
        // This is the parent. fork call might have been successful or not.

        // Unblock signals.
        if (sigprocmask(SIG_SETMASK, &oldMask, NULL) != 0) {
            CFAssert1(FALSE, __kCFLogAssertion, "sigprocmask() failed: %d", errno);
        }

        if (forkPid < 0) {
            return forkPid;
        }

        if (pid != NULL) {
            *pid = forkPid;
        }

        return 0;
    }

    // This is the child.

    // Clean up the parent signal handlers
    for (int idx = 1; idx < NSIG; idx++) {
        // Seems that SIGKILL/SIGSTOP are sometimes silently ignored, and
        // sometimes return EINVAL. Since one cannot change the handlers anyway,
        // skip them.
        if (idx == SIGKILL || idx == SIGSTOP) {
            continue;
        }

        struct sigaction sigAction;
        if (sigaction(idx, NULL, &sigAction) != 0) {
            exit(127);
        }

        if (sigAction.sa_handler != SIG_IGN) {
            sigAction.sa_handler = SIG_DFL;
            if (sigaction(idx, &sigAction, NULL) != 0) {
                exit(127);
            }
        }
    }

    // Perform the actions
    if (actions != NULL) {
        for (size_t idx = 0; idx < actions->actionsCount; idx++) {
            struct _CFPosixSpawnFileActionPre28 *action = &(actions->actions[idx]);
            if (action->type == _CFPosixSpawnFileActionDup2Pre28) {
                if (dup2(action->dup2Action.filedes, action->dup2Action.newfiledes) < 0) {
                    exit(127);
                }
            } else if (actions->actions[idx].type == _CFPosixSpawnFileActionClosePre28) {
                if (close(action->closeAction.filedes) != 0) {
                    exit(127);
                }
            }
        }
    }

    // Unblock the signals
    if (sigprocmask(SIG_SETMASK, &oldMask, NULL) != 0) {
        CFAssert1(FALSE, __kCFLogAssertion, "sigprocmask() failed: %d", errno);
    }

    // If execv fails, we will simply exit 127 as the standard says.
    execve(path, argv, envp ?: environ);
    exit(127);
    // no need for return here
}

static void _CFPosixSpawnInitializeCallback() {
    // Let's check if the posix_spawn is present.
    (void)dlerror(); // Clean up the error.
    _CFPosixSpawnImpl = (int (*)(pid_t *_CF_RESTRICT, const char *_CF_RESTRICT, void *, void *_CF_RESTRICT, char *const *_CF_RESTRICT, char *const *_CF_RESTRICT))dlsym(RTLD_DEFAULT, "posix_spawn");
    char *dlsymError = dlerror();
    CFAssert1(dlsymError == NULL, __kCFLogAssertion, "dlsym failed: %s", dlsymError);
    if (_CFPosixSpawnImpl != NULL) {
        // posix_spawn_fn is available, so use it
        _CFPosixSpawnFileActionsAllocImpl = _CFPosixSpawnFileActionsAllocImplPost28;
        _CFPosixSpawnFileActionsDeallocImpl = _CFPosixSpawnFileActionsDeallocImplBoth;

        _CFPosixSpawnFileActionsInitImpl = (int (*)(void *))dlsym(RTLD_DEFAULT, "posix_spawn_file_actions_init");
        dlsymError = dlerror();
        CFAssert1(_CFPosixSpawnFileActionsInitImpl != NULL, __kCFLogAssertion, "loading posix_spawn_file_actions_init failed: %s", dlsymError);

        _CFPosixSpawnFileActionsDestroyImpl = (int (*)(void *))dlsym(RTLD_DEFAULT, "posix_spawn_file_actions_destroy");
        dlsymError = dlerror();
        CFAssert1(_CFPosixSpawnFileActionsDestroyImpl != NULL, __kCFLogAssertion, "loading posix_spawn_file_actions_destroy failed: %s", dlsymError);

        _CFPosixSpawnFileActionsAddDup2Impl = (int (*)(void *, int, int))dlsym(RTLD_DEFAULT, "posix_spawn_file_actions_adddup2");
        dlsymError = dlerror();
        CFAssert1(_CFPosixSpawnFileActionsAddDup2Impl != NULL, __kCFLogAssertion, "loading posix_spawn_file_actions_adddup2 failed: %s", dlsymError);

        _CFPosixSpawnFileActionsAddCloseImpl = (int (*)(void *, int))dlsym(RTLD_DEFAULT, "posix_spawn_file_actions_addclose");
        dlsymError = dlerror();
        CFAssert1(_CFPosixSpawnFileActionsAddCloseImpl != NULL, __kCFLogAssertion, "loading posix_spawn_file_actions_addclose failed: %s", dlsymError);
    } else {
        // posix_spawn_fn is not available, setup our workaround
        _CFPosixSpawnFileActionsAllocImpl = _CFPosixSpawnFileActionsAllocImplPre28;
        _CFPosixSpawnFileActionsDeallocImpl = _CFPosixSpawnFileActionsDeallocImplBoth;
        _CFPosixSpawnFileActionsInitImpl = _CFPosixSpawnFileActionsInitImplPre28;
        _CFPosixSpawnFileActionsDestroyImpl = _CFPosixSpawnFileActionsDestroyImplPre28;
        _CFPosixSpawnFileActionsAddDup2Impl = _CFPosixSpawnFileActionsAddDup2ImplPre28;
        _CFPosixSpawnFileActionsAddCloseImpl = _CFPosixSpawnFileActionsAddCloseImplPre28;
        _CFPosixSpawnImpl = _CFPosixSpawnImplPre28;
    }
}

static void _CFPosixSpawnInitialize() {
    int r = pthread_once(&posixSpawnOnce, _CFPosixSpawnInitializeCallback);
    CFAssert(r == 0, __kCFLogAssertion, "pthread_once failed");
}

CF_EXPORT _CFPosixSpawnFileActionsRef _CFPosixSpawnFileActionsAlloc() {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnFileActionsAllocImpl();
}

CF_EXPORT int _CFPosixSpawnFileActionsInit(_CFPosixSpawnFileActionsRef file_actions) {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnFileActionsInitImpl(file_actions);
}

CF_EXPORT int _CFPosixSpawnFileActionsDestroy(_CFPosixSpawnFileActionsRef file_actions) {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnFileActionsDestroyImpl(file_actions);
}

CF_EXPORT void _CFPosixSpawnFileActionsDealloc(_CFPosixSpawnFileActionsRef file_actions) {
    _CFPosixSpawnInitialize();
    _CFPosixSpawnFileActionsDeallocImpl(file_actions);
}

CF_EXPORT int _CFPosixSpawnFileActionsAddDup2(_CFPosixSpawnFileActionsRef file_actions, int filedes, int newfiledes) {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnFileActionsAddDup2Impl(file_actions, filedes, newfiledes);
}

CF_EXPORT int _CFPosixSpawnFileActionsAddClose(_CFPosixSpawnFileActionsRef file_actions, int filedes) {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnFileActionsAddCloseImpl(file_actions, filedes);
}

CF_EXPORT int _CFPosixSpawn(pid_t *_CF_RESTRICT pid, const char *_CF_RESTRICT path, _CFPosixSpawnFileActionsRef file_actions, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT attrp, char *_Nullable const argv[_Nullable _CF_RESTRICT], char *_Nullable const envp[_Nullable _CF_RESTRICT]) {
    _CFPosixSpawnInitialize();
    return _CFPosixSpawnImpl(pid, path, file_actions, attrp, argv, envp);
}

#elif !TARGET_OS_WIN32 && !TARGET_OS_WASI

#include <spawn.h>

CF_EXPORT _CFPosixSpawnFileActionsRef _CFPosixSpawnFileActionsAlloc() {
  _CFPosixSpawnFileActionsRef actions = malloc(sizeof(posix_spawn_file_actions_t));
  CFAssert(actions != NULL, __kCFLogAssertion, "malloc failed");
  return actions;
}

CF_EXPORT int _CFPosixSpawnFileActionsInit(_CFPosixSpawnFileActionsRef file_actions) {
  return posix_spawn_file_actions_init((posix_spawn_file_actions_t *)file_actions);
}

CF_EXPORT int _CFPosixSpawnFileActionsDestroy(_CFPosixSpawnFileActionsRef file_actions) {
  return posix_spawn_file_actions_destroy((posix_spawn_file_actions_t *)file_actions);
}

CF_EXPORT void _CFPosixSpawnFileActionsDealloc(_CFPosixSpawnFileActionsRef file_actions) {
  free(file_actions);
}

CF_EXPORT int _CFPosixSpawnFileActionsAddDup2(_CFPosixSpawnFileActionsRef file_actions, int filedes, int newfiledes) {
  return posix_spawn_file_actions_adddup2((posix_spawn_file_actions_t *)file_actions, filedes, newfiledes);
}

CF_EXPORT int _CFPosixSpawnFileActionsAddClose(_CFPosixSpawnFileActionsRef file_actions, int filedes) {
  return posix_spawn_file_actions_addclose((posix_spawn_file_actions_t *)file_actions, filedes);
}

CF_EXPORT int _CFPosixSpawn(pid_t *_CF_RESTRICT pid, const char *_CF_RESTRICT path, _CFPosixSpawnFileActionsRef file_actions, _CFPosixSpawnAttrRef _Nullable _CF_RESTRICT attrp, char *_Nullable const argv[_Nullable _CF_RESTRICT], char *_Nullable const envp[_Nullable _CF_RESTRICT]) {
  return posix_spawn(pid, path, (posix_spawn_file_actions_t *)file_actions, (posix_spawnattr_t *)attrp, argv, envp);
}

#endif

#endif
