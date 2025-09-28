#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#include <wchar.h>
#include <wctype.h>
#include <windows.h>
#include <shlobj.h>
#include <wchar.h>
#include <userenv.h>
#include <lm.h>
#if !defined(__GNUC__) || (((100000 * __GNUC__) + (1000 * __GNUC_MINOR__)) >= 408000)
#  include <winhttp.h>
#endif

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef countof
#  define countof(array) (sizeof (array) / sizeof (*(array)))
#endif

#define SE_SHUTDOWN_NAMEA   "SeShutdownPrivilege"

#ifndef WC_NO_BEST_FIT_CHARS
#  define WC_NO_BEST_FIT_CHARS 0x00000400
#endif

#define GETPROC(fn) pfn##fn = (PFN##fn)GetProcAddress(module, #fn)

typedef int (__stdcall *PFNDllRegisterServer)(void);
typedef int (__stdcall *PFNDllUnregisterServer)(void);
typedef BOOL (__stdcall *PFNIsUserAnAdmin)(void);
typedef BOOL (WINAPI *PFNGetProductInfo)(DWORD, DWORD, DWORD, DWORD, DWORD*);
typedef void (WINAPI *PFNGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
typedef LONG (WINAPI *PFNRegGetValueA)(HKEY, LPCSTR, LPCSTR, DWORD, LPDWORD, PVOID, LPDWORD);

#ifndef CSIDL_MYMUSIC
#   define CSIDL_MYMUSIC              0x000D
#endif
#ifndef CSIDL_MYVIDEO
#   define CSIDL_MYVIDEO              0x000E
#endif
#ifndef CSIDL_LOCAL_APPDATA
#   define CSIDL_LOCAL_APPDATA        0x001C
#endif
#ifndef CSIDL_COMMON_FAVORITES
#   define CSIDL_COMMON_FAVORITES     0x001F
#endif
#ifndef CSIDL_INTERNET_CACHE
#   define CSIDL_INTERNET_CACHE       0x0020
#endif
#ifndef CSIDL_COOKIES
#   define CSIDL_COOKIES              0x0021
#endif
#ifndef CSIDL_HISTORY
#   define CSIDL_HISTORY              0x0022
#endif
#ifndef CSIDL_COMMON_APPDATA
#   define CSIDL_COMMON_APPDATA       0x0023
#endif
#ifndef CSIDL_WINDOWS
#   define CSIDL_WINDOWS              0x0024
#endif
#ifndef CSIDL_PROGRAM_FILES
#   define CSIDL_PROGRAM_FILES        0x0026
#endif
#ifndef CSIDL_MYPICTURES
#   define CSIDL_MYPICTURES           0x0027
#endif
#ifndef CSIDL_PROFILE
#   define CSIDL_PROFILE              0x0028
#endif
#ifndef CSIDL_PROGRAM_FILES_COMMON
#   define CSIDL_PROGRAM_FILES_COMMON 0x002B
#endif
#ifndef CSIDL_COMMON_TEMPLATES
#   define CSIDL_COMMON_TEMPLATES     0x002D
#endif
#ifndef CSIDL_COMMON_DOCUMENTS
#   define CSIDL_COMMON_DOCUMENTS     0x002E
#endif
#ifndef CSIDL_COMMON_ADMINTOOLS
#   define CSIDL_COMMON_ADMINTOOLS    0x002F
#endif
#ifndef CSIDL_ADMINTOOLS
#   define CSIDL_ADMINTOOLS           0x0030
#endif
#ifndef CSIDL_COMMON_MUSIC
#   define CSIDL_COMMON_MUSIC         0x0035
#endif
#ifndef CSIDL_COMMON_PICTURES
#   define CSIDL_COMMON_PICTURES      0x0036
#endif
#ifndef CSIDL_COMMON_VIDEO
#   define CSIDL_COMMON_VIDEO         0x0037
#endif
#ifndef CSIDL_CDBURN_AREA
#   define CSIDL_CDBURN_AREA          0x003B
#endif
#ifndef CSIDL_FLAG_CREATE
#   define CSIDL_FLAG_CREATE          0x8000
#endif

/* Use explicit struct definition because wSuiteMask and
 * wProductType are not defined in the VC++ 6.0 headers.
 * WORD type has been replaced by unsigned short because
 * WORD is already used by Perl itself.
 */
struct g_osver_t {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
    unsigned short wServicePackMajor;
    unsigned short wServicePackMinor;
    unsigned short wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} g_osver = {0, 0, 0, 0, 0, "", 0, 0, 0, 0, 0};
BOOL g_osver_ex = TRUE;

#define ONE_K_BUFSIZE	1024

/* Convert SV to wide character string.  The return value must be
 * freed using Safefree().
 */
WCHAR*
sv_to_wstr(pTHX_ SV *sv)
{
    DWORD wlen;
    WCHAR *wstr;
    STRLEN len;
    char *str = SvPV(sv, len);
    UINT cp = SvUTF8(sv) ? CP_UTF8 : CP_ACP;

    wlen = MultiByteToWideChar(cp, 0, str, (int)(len+1), NULL, 0);
    New(0, wstr, wlen, WCHAR);
    MultiByteToWideChar(cp, 0, str, (int)(len+1), wstr, wlen);

    return wstr;
}

/* Convert wide character string to mortal SV.  Use UTF8 encoding
 * if the string cannot be represented in the system codepage.
 */
SV *
wstr_to_sv(pTHX_ WCHAR *wstr)
{
    int wlen = (int)wcslen(wstr)+1;
    BOOL use_default = FALSE;
    int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr, wlen, NULL, 0, NULL, NULL);
    SV *sv = sv_2mortal(newSV(len));

    len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr, wlen, SvPVX(sv), len, NULL, &use_default);
    if (use_default) {
        len = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, NULL, 0, NULL, NULL);
        sv_grow(sv, len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, SvPVX(sv), len, NULL, NULL);
        SvUTF8_on(sv);
    }
    /* Shouldn't really ever fail since we ask for the required length first, but who knows... */
    if (len) {
        SvPOK_on(sv);
        SvCUR_set(sv, len-1);
    }
    return sv;
}

/* Retrieve a variable from the Unicode environment in a mortal SV.
 *
 * Recreates the Unicode environment because a bug in earlier Perl versions
 * overwrites it with the ANSI version, which contains replacement
 * characters for the characters not in the ANSI codepage.
 */
SV*
get_unicode_env(pTHX_ const WCHAR *name)
{
    SV *sv = NULL;
    void *env;
    HANDLE token;

    /* Get security token for the current process owner */
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &token))
    {
        return NULL;
    }

    /* Create a Unicode environment block for this process */
    if (CreateEnvironmentBlock(&env, token, FALSE))
    {
        size_t name_len = wcslen(name);
        WCHAR *entry = (WCHAR *)env;
        while (*entry) {
            size_t i;
            size_t entry_len = wcslen(entry);
            BOOL equal = (entry_len > name_len) && (entry[name_len] == '=');

            for (i=0; equal && i < name_len; ++i)
                equal = (towupper(entry[i]) == towupper(name[i]));

            if (equal) {
                sv = wstr_to_sv(aTHX_ entry+name_len+1);
                break;
            }
            entry += entry_len+1;
        }
        DestroyEnvironmentBlock(env);
    }
    CloseHandle(token);
    return sv;
}

#define CHAR_T            WCHAR
#define WIN32_FIND_DATA_T WIN32_FIND_DATAW
#define FN_FINDFIRSTFILE  FindFirstFileW
#define FN_STRLEN         wcslen
#define FN_STRCPY         wcscpy
#define LONGPATH          my_longpathW
#include "longpath.inc"

/* The my_ansipath() function takes a Unicode filename and converts it
 * into the current Windows codepage. If some characters cannot be mapped,
 * then it will convert the short name instead.
 *
 * The buffer to the ansi pathname must be freed with Safefree() when it
 * it no longer needed.
 *
 * The argument to my_ansipath() must exist before this function is
 * called; otherwise there is no way to determine the short path name.
 *
 * Ideas for future refinement:
 * - Only convert those segments of the path that are not in the current
 *   codepage, but leave the other segments in their long form.
 * - If the resulting name is longer than MAX_PATH, start converting
 *   additional path segments into short names until the full name
 *   is shorter than MAX_PATH.  Shorten the filename part last!
 */

/* This is a modified version of core Perl win32/win32.c(win32_ansipath).
 * It uses New() etc. instead of win32_malloc().
 */

char *
my_ansipath(const WCHAR *widename)
{
    char *name;
    BOOL use_default = FALSE;
    int widelen = (int)wcslen(widename)+1;
    int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, widename, widelen,
                                  NULL, 0, NULL, NULL);
    New(0, name, len, char);
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, widename, widelen,
                        name, len, NULL, &use_default);
    if (use_default) {
        DWORD shortlen = GetShortPathNameW(widename, NULL, 0);
        if (shortlen) {
            WCHAR *shortname;
            New(0, shortname, shortlen, WCHAR);
            shortlen = GetShortPathNameW(widename, shortname, shortlen)+1;

            len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shortname, shortlen,
                                      NULL, 0, NULL, NULL);
            Renew(name, len, char);
            WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shortname, shortlen,
                                name, len, NULL, NULL);
            Safefree(shortname);
        }
    }
    return name;
}

/* Convert wide character path to ANSI path and return as mortal SV. */
SV*
wstr_to_ansipath(pTHX_ WCHAR *wstr)
{
    char *ansi = my_ansipath(wstr);
    SV *sv = sv_2mortal(newSVpvn(ansi, strlen(ansi)));
    Safefree(ansi);
    return sv;
}

#ifdef __CYGWIN__

char*
get_childdir(void)
{
    dTHX;
    WCHAR filename[MAX_PATH+1];

    GetCurrentDirectoryW(MAX_PATH+1, filename);
    return my_ansipath(filename);
}

void
free_childdir(char *d)
{
    dTHX;
    Safefree(d);
}

void*
get_childenv(void)
{
    return NULL;
}

void
free_childenv(void *d)
{
  PERL_UNUSED_ARG(d);
}

#  define PerlDir_mapA(dir) (dir)

#endif

XS(w32_ExpandEnvironmentStrings)
{
    dXSARGS;
    WCHAR value[31*1024];
    WCHAR *source;

    if (items != 1)
	croak("usage: Win32::ExpandEnvironmentStrings($String)");

    source = sv_to_wstr(aTHX_ ST(0));
    ExpandEnvironmentStringsW(source, value, countof(value)-1);
    ST(0) = wstr_to_sv(aTHX_ value);
    Safefree(source);
    XSRETURN(1);
}

XS(w32_IsAdminUser)
{
    dXSARGS;
    HMODULE                     module;
    PFNIsUserAnAdmin            pfnIsUserAnAdmin;
    HANDLE                      hTok;
    DWORD                       dwTokInfoLen;
    TOKEN_GROUPS                *lpTokInfo;
    SID_IDENTIFIER_AUTHORITY    NtAuth = SECURITY_NT_AUTHORITY;
    PSID                        pAdminSid;
    int                         iRetVal;
    unsigned int                i;

    if (items)
        croak("usage: Win32::IsAdminUser()");

    /* Use IsUserAnAdmin() when available.  On Vista this will only return TRUE
     * if the process is running with elevated privileges and not just when the
     * process owner is a member of the "Administrators" group.
     */
    module = GetModuleHandleA("shell32.dll");
    GETPROC(IsUserAnAdmin);
    if (pfnIsUserAnAdmin) {
        EXTEND(SP, 1);
        ST(0) = sv_2mortal(newSViv(pfnIsUserAnAdmin() ? 1 : 0));
        XSRETURN(1);
    }

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hTok)) {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTok)) {
            warn("Cannot open thread token or process token");
            XSRETURN_UNDEF;
        }
    }

    GetTokenInformation(hTok, TokenGroups, NULL, 0, &dwTokInfoLen);
    if (!New(1, lpTokInfo, dwTokInfoLen, TOKEN_GROUPS)) {
        warn("Cannot allocate token information structure");
        CloseHandle(hTok);
        XSRETURN_UNDEF;
    }

    if (!GetTokenInformation(hTok, TokenGroups, lpTokInfo, dwTokInfoLen,
            &dwTokInfoLen))
    {
        warn("Cannot get token information");
        Safefree(lpTokInfo);
        CloseHandle(hTok);
        XSRETURN_UNDEF;
    }

    if (!AllocateAndInitializeSid(&NtAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSid))
    {
        warn("Cannot allocate administrators' SID");
        Safefree(lpTokInfo);
        CloseHandle(hTok);
        XSRETURN_UNDEF;
    }

    iRetVal = 0;
    for (i = 0; i < lpTokInfo->GroupCount; ++i) {
        if (EqualSid(lpTokInfo->Groups[i].Sid, pAdminSid)) {
            iRetVal = 1;
            break;
        }
    }

    FreeSid(pAdminSid);
    Safefree(lpTokInfo);
    CloseHandle(hTok);

    EXTEND(SP, 1);
    ST(0) = sv_2mortal(newSViv(iRetVal));
    XSRETURN(1);
}

XS(w32_LookupAccountName)
{
    dXSARGS;
    char SID[400];
    DWORD SIDLen;
    SID_NAME_USE snu;
    char Domain[256];
    DWORD DomLen;
    BOOL bResult;

    if (items != 5)
	croak("usage: Win32::LookupAccountName($system, $account, $domain, "
	      "$sid, $sidtype)");

    SIDLen = sizeof(SID);
    DomLen = sizeof(Domain);

    bResult = LookupAccountNameA(SvPV_nolen(ST(0)),	/* System */
                                 SvPV_nolen(ST(1)),	/* Account name */
                                 &SID,			/* SID structure */
                                 &SIDLen,		/* Size of SID buffer */
                                 Domain,		/* Domain buffer */
                                 &DomLen,		/* Domain buffer size */
                                 &snu);			/* SID name type */
    if (bResult) {
	sv_setpv(ST(2), Domain);
	sv_setpvn(ST(3), SID, SIDLen);
	sv_setiv(ST(4), snu);
	XSRETURN_YES;
    }
    XSRETURN_NO;
}


XS(w32_LookupAccountSID)
{
    dXSARGS;
    PSID sid;
    char Account[256];
    DWORD AcctLen = sizeof(Account);
    char Domain[256];
    DWORD DomLen = sizeof(Domain);
    SID_NAME_USE snu;
    BOOL bResult;

    if (items != 5)
	croak("usage: Win32::LookupAccountSID($system, $sid, $account, $domain, $sidtype)");

    sid = SvPV_nolen(ST(1));
    if (IsValidSid(sid)) {
        bResult = LookupAccountSidA(SvPV_nolen(ST(0)),	/* System */
                                    sid,		/* SID structure */
                                    Account,		/* Account name buffer */
                                    &AcctLen,		/* name buffer length */
                                    Domain,		/* Domain buffer */
                                    &DomLen,		/* Domain buffer length */
                                    &snu);		/* SID name type */
	if (bResult) {
	    sv_setpv(ST(2), Account);
	    sv_setpv(ST(3), Domain);
	    sv_setiv(ST(4), (IV)snu);
	    XSRETURN_YES;
	}
    }
    XSRETURN_NO;
}

XS(w32_InitiateSystemShutdown)
{
    dXSARGS;
    HANDLE hToken;              /* handle to process token   */
    TOKEN_PRIVILEGES tkp;       /* pointer to token structure  */
    BOOL bRet;
    char *machineName, *message;

    if (items != 5)
	croak("usage: Win32::InitiateSystemShutdown($machineName, $message, "
	      "$timeOut, $forceClose, $reboot)");

    machineName = SvPV_nolen(ST(0));

    if (OpenProcessToken(GetCurrentProcess(),
			 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
			 &hToken))
    {
        LookupPrivilegeValueA(machineName,
                              SE_SHUTDOWN_NAMEA,
                              &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1; /* only setting one */
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	/* Get shutdown privilege for this process. */
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
			      (PTOKEN_PRIVILEGES)NULL, 0);
    }

    message = SvPV_nolen(ST(1));
    bRet = InitiateSystemShutdownA(machineName, message, (DWORD)SvIV(ST(2)),
                                   (BOOL)SvIV(ST(3)), (BOOL)SvIV(ST(4)));

    /* Disable shutdown privilege. */
    tkp.Privileges[0].Attributes = 0; 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
			  (PTOKEN_PRIVILEGES)NULL, 0); 
    CloseHandle(hToken);
    XSRETURN_IV(bRet);
}

XS(w32_AbortSystemShutdown)
{
    dXSARGS;
    HANDLE hToken;              /* handle to process token   */
    TOKEN_PRIVILEGES tkp;       /* pointer to token structure  */
    BOOL bRet;
    char *machineName;

    if (items != 1)
	croak("usage: Win32::AbortSystemShutdown($machineName)");

    machineName = SvPV_nolen(ST(0));

    if (OpenProcessToken(GetCurrentProcess(),
			 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
			 &hToken))
    {
        LookupPrivilegeValueA(machineName,
                              SE_SHUTDOWN_NAMEA,
                              &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1; /* only setting one */
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	/* Get shutdown privilege for this process. */
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
			      (PTOKEN_PRIVILEGES)NULL, 0);
    }

    bRet = AbortSystemShutdownA(machineName);

    /* Disable shutdown privilege. */
    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
			  (PTOKEN_PRIVILEGES)NULL, 0);
    CloseHandle(hToken);
    XSRETURN_IV(bRet);
}


XS(w32_MsgBox)
{
    dXSARGS;
    DWORD flags = MB_ICONEXCLAMATION;
    I32 result;
    WCHAR *title = NULL, *msg;

    if (items < 1 || items > 3)
	croak("usage: Win32::MsgBox($message [, $flags [, $title]])");

    msg = sv_to_wstr(aTHX_ ST(0));
    if (items > 1)
        flags = (DWORD)SvIV(ST(1));
    if (items > 2)
        title = sv_to_wstr(aTHX_ ST(2));

    result = MessageBoxW(GetActiveWindow(), msg, title ? title : L"Perl", flags);

    Safefree(msg);
    if (title)
        Safefree(title);

    XSRETURN_IV(result);
}

XS(w32_LoadLibrary)
{
    dXSARGS;
    HANDLE hHandle;

    if (items != 1)
	croak("usage: Win32::LoadLibrary($libname)\n");
    hHandle = LoadLibraryA(SvPV_nolen(ST(0)));
#ifdef _WIN64
    XSRETURN_IV((DWORD_PTR)hHandle);
#else
    XSRETURN_IV((DWORD)hHandle);
#endif
}

XS(w32_FreeLibrary)
{
    dXSARGS;

    if (items != 1)
	croak("usage: Win32::FreeLibrary($handle)\n");
    if (FreeLibrary(INT2PTR(HINSTANCE, SvIV(ST(0))))) {
	XSRETURN_YES;
    }
    XSRETURN_NO;
}

XS(w32_GetProcAddress)
{
    dXSARGS;

    if (items != 2)
	croak("usage: Win32::GetProcAddress($hinstance, $procname)\n");
    XSRETURN_IV(PTR2IV(GetProcAddress(INT2PTR(HINSTANCE, SvIV(ST(0))), SvPV_nolen(ST(1)))));
}

XS(w32_RegisterServer)
{
    dXSARGS;
    BOOL result = FALSE;
    HMODULE module;

    if (items != 1)
	croak("usage: Win32::RegisterServer($libname)\n");

    module = LoadLibraryA(SvPV_nolen(ST(0)));
    if (module) {
	PFNDllRegisterServer pfnDllRegisterServer;
        GETPROC(DllRegisterServer);
	if (pfnDllRegisterServer && pfnDllRegisterServer() == 0)
	    result = TRUE;
	FreeLibrary(module);
    }
    ST(0) = boolSV(result);
    XSRETURN(1);
}

XS(w32_UnregisterServer)
{
    dXSARGS;
    BOOL result = FALSE;
    HINSTANCE module;

    if (items != 1)
	croak("usage: Win32::UnregisterServer($libname)\n");

    module = LoadLibraryA(SvPV_nolen(ST(0)));
    if (module) {
	PFNDllUnregisterServer pfnDllUnregisterServer;
        GETPROC(DllUnregisterServer);
	if (pfnDllUnregisterServer && pfnDllUnregisterServer() == 0)
	    result = TRUE;
	FreeLibrary(module);
    }
    ST(0) = boolSV(result);
    XSRETURN(1);
}

/* XXX rather bogus */
XS(w32_GetArchName)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetArchName()");
    XSRETURN_PV(getenv("PROCESSOR_ARCHITECTURE"));
}

XS(w32_GetChipArch)
{
    dXSARGS;
    SYSTEM_INFO sysinfo;
    HMODULE module;
    PFNGetNativeSystemInfo pfnGetNativeSystemInfo;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetChipArch()");

    Zero(&sysinfo,1,SYSTEM_INFO);
    module = GetModuleHandle("kernel32.dll");
    GETPROC(GetNativeSystemInfo);
    if (pfnGetNativeSystemInfo)
        pfnGetNativeSystemInfo(&sysinfo);
    else
        GetSystemInfo(&sysinfo);

    XSRETURN_IV(sysinfo.wProcessorArchitecture);
}

XS(w32_GetChipName)
{
    dXSARGS;
    SYSTEM_INFO sysinfo;
    HMODULE module;
    PFNGetNativeSystemInfo pfnGetNativeSystemInfo;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetChipName()");

    Zero(&sysinfo,1,SYSTEM_INFO);
    module = GetModuleHandle("kernel32.dll");
    GETPROC(GetNativeSystemInfo);
    if (pfnGetNativeSystemInfo)
        pfnGetNativeSystemInfo(&sysinfo);
    else
        GetSystemInfo(&sysinfo);

    /* XXX docs say dwProcessorType is deprecated on NT */
    XSRETURN_IV(sysinfo.dwProcessorType);
}

XS(w32_GuidGen)
{
    dXSARGS;
    GUID guid;
    char szGUID[50] = {'\0'};
    HRESULT  hr;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GuidGen()");

    hr     = CoCreateGuid(&guid);
    if (SUCCEEDED(hr)) {
	LPOLESTR pStr = NULL;
#ifdef __cplusplus
	if (SUCCEEDED(StringFromCLSID(guid, &pStr))) {
#else
	if (SUCCEEDED(StringFromCLSID(&guid, &pStr))) {
#endif
            WideCharToMultiByte(CP_ACP, 0, pStr, (int)wcslen(pStr), szGUID,
                                sizeof(szGUID), NULL, NULL);
            CoTaskMemFree(pStr);
            XSRETURN_PV(szGUID);
        }
    }
    XSRETURN_UNDEF;
}

XS(w32_GetFolderPath)
{
    dXSARGS;
    WCHAR wpath[MAX_PATH+1];
    int folder;
    int create = 0;

    if (items != 1 && items != 2)
	croak("usage: Win32::GetFolderPath($csidl [, $create])\n");

    folder = (int)SvIV(ST(0));
    if (items == 2)
        create = SvTRUE(ST(1)) ? CSIDL_FLAG_CREATE : 0;

    if (SUCCEEDED(SHGetFolderPathW(NULL, folder|create, NULL, 0, wpath))) {
        ST(0) = wstr_to_ansipath(aTHX_ wpath);
        XSRETURN(1);
    }

    if (SHGetSpecialFolderPathW(NULL, wpath, folder, !!create)) {
        ST(0) = wstr_to_ansipath(aTHX_ wpath);
        XSRETURN(1);
    }

    /* SHGetFolderPathW() and SHGetSpecialFolderPathW() may fail on older
     * Perl versions that have replaced the Unicode environment with an
     * ANSI version.  Let's go spelunking in the registry now...
     */
    {
        SV *sv;
        HKEY hkey;
        HKEY root = HKEY_CURRENT_USER;
        const WCHAR *name = NULL;

        switch (folder) {
        case CSIDL_ADMINTOOLS:                  name = L"Administrative Tools";        break;
        case CSIDL_APPDATA:                     name = L"AppData";                     break;
        case CSIDL_CDBURN_AREA:                 name = L"CD Burning";                  break;
        case CSIDL_COOKIES:                     name = L"Cookies";                     break;
        case CSIDL_DESKTOP:
        case CSIDL_DESKTOPDIRECTORY:            name = L"Desktop";                     break;
        case CSIDL_FAVORITES:                   name = L"Favorites";                   break;
        case CSIDL_FONTS:                       name = L"Fonts";                       break;
        case CSIDL_HISTORY:                     name = L"History";                     break;
        case CSIDL_INTERNET_CACHE:              name = L"Cache";                       break;
        case CSIDL_LOCAL_APPDATA:               name = L"Local AppData";               break;
        case CSIDL_MYMUSIC:                     name = L"My Music";                    break;
        case CSIDL_MYPICTURES:                  name = L"My Pictures";                 break;
        case CSIDL_MYVIDEO:                     name = L"My Video";                    break;
        case CSIDL_NETHOOD:                     name = L"NetHood";                     break;
        case CSIDL_PERSONAL:                    name = L"Personal";                    break;
        case CSIDL_PRINTHOOD:                   name = L"PrintHood";                   break;
        case CSIDL_PROGRAMS:                    name = L"Programs";                    break;
        case CSIDL_RECENT:                      name = L"Recent";                      break;
        case CSIDL_SENDTO:                      name = L"SendTo";                      break;
        case CSIDL_STARTMENU:                   name = L"Start Menu";                  break;
        case CSIDL_STARTUP:                     name = L"Startup";                     break;
        case CSIDL_TEMPLATES:                   name = L"Templates";                   break;
            /* XXX L"Local Settings" */
        }

        if (!name) {
            root = HKEY_LOCAL_MACHINE;
            switch (folder) {
            case CSIDL_COMMON_ADMINTOOLS:       name = L"Common Administrative Tools"; break;
            case CSIDL_COMMON_APPDATA:          name = L"Common AppData";              break;
            case CSIDL_COMMON_DESKTOPDIRECTORY: name = L"Common Desktop";              break;
            case CSIDL_COMMON_DOCUMENTS:        name = L"Common Documents";            break;
            case CSIDL_COMMON_FAVORITES:        name = L"Common Favorites";            break;
            case CSIDL_COMMON_PROGRAMS:         name = L"Common Programs";             break;
            case CSIDL_COMMON_STARTMENU:        name = L"Common Start Menu";           break;
            case CSIDL_COMMON_STARTUP:          name = L"Common Startup";              break;
            case CSIDL_COMMON_TEMPLATES:        name = L"Common Templates";            break;
            case CSIDL_COMMON_MUSIC:            name = L"CommonMusic";                 break;
            case CSIDL_COMMON_PICTURES:         name = L"CommonPictures";              break;
            case CSIDL_COMMON_VIDEO:            name = L"CommonVideo";                 break;
            }
        }
        /* XXX todo
         * case CSIDL_SYSTEM               # GetSystemDirectory()
         * case CSIDL_RESOURCES            # %windir%\Resources\, For theme and other windows resources.
         * case CSIDL_RESOURCES_LOCALIZED  # %windir%\Resources\<LangID>, for theme and other windows specific resources.
         */

#define SHELL_FOLDERS "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"

        if (name && RegOpenKeyEx(root, SHELL_FOLDERS, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS) {
            WCHAR data[MAX_PATH+1];
            DWORD cb = sizeof(data)-sizeof(WCHAR);
            DWORD type = REG_NONE;
            long rc = RegQueryValueExW(hkey, name, NULL, &type, (BYTE*)&data, &cb);
            RegCloseKey(hkey);
            if (rc == ERROR_SUCCESS && type == REG_SZ && cb > sizeof(WCHAR) && data[0]) {
                /* Make sure the string is properly terminated */
                data[cb/sizeof(WCHAR)] = '\0';
                ST(0) = wstr_to_ansipath(aTHX_ data);
                XSRETURN(1);
            }
        }

#undef SHELL_FOLDERS

        /* Unders some circumstances the registry entries seem to have a null string
         * as their value even when the directory already exists.  The environment
         * variables do get set though, so try re-create a Unicode environment and
         * check if they are there.
         */
        sv = NULL;
        switch (folder) {
        case CSIDL_APPDATA:              sv = get_unicode_env(aTHX_ L"APPDATA");            break;
        case CSIDL_PROFILE:              sv = get_unicode_env(aTHX_ L"USERPROFILE");        break;
        case CSIDL_PROGRAM_FILES:        sv = get_unicode_env(aTHX_ L"ProgramFiles");       break;
        case CSIDL_PROGRAM_FILES_COMMON: sv = get_unicode_env(aTHX_ L"CommonProgramFiles"); break;
        case CSIDL_WINDOWS:              sv = get_unicode_env(aTHX_ L"SystemRoot");         break;
        }
        if (sv) {
            ST(0) = sv;
            XSRETURN(1);
        }
    }

    XSRETURN_UNDEF;
}

XS(w32_GetFileVersion)
{
    dXSARGS;
    DWORD size;
    DWORD handle;
    char *filename;
    char *data;

    if (items != 1)
	croak("usage: Win32::GetFileVersion($filename)");

    filename = SvPV_nolen(ST(0));
    size = GetFileVersionInfoSize(filename, &handle);
    if (!size)
        XSRETURN_UNDEF;

    New(0, data, size, char);
    if (!data)
        XSRETURN_UNDEF;

    if (GetFileVersionInfo(filename, handle, size, data)) {
        VS_FIXEDFILEINFO *info;
        UINT len;
        if (VerQueryValue(data, "\\", (void**)&info, &len)) {
            int dwValueMS1 = (info->dwFileVersionMS>>16);
            int dwValueMS2 = (info->dwFileVersionMS&0xffff);
            int dwValueLS1 = (info->dwFileVersionLS>>16);
            int dwValueLS2 = (info->dwFileVersionLS&0xffff);

            if (GIMME_V == G_ARRAY) {
                EXTEND(SP, 4);
                XST_mIV(0, dwValueMS1);
                XST_mIV(1, dwValueMS2);
                XST_mIV(2, dwValueLS1);
                XST_mIV(3, dwValueLS2);
                items = 4;
            }
            else {
                char version[50];
                sprintf(version, "%d.%d.%d.%d", dwValueMS1, dwValueMS2, dwValueLS1, dwValueLS2);
                XST_mPV(0, version);
            }
        }
    }
    else
        items = 0;

    Safefree(data);
    XSRETURN(items);
}

#ifdef __CYGWIN__
XS(w32_SetChildShowWindow)
{
    /* This function doesn't do anything useful for cygwin.  In the
     * MSWin32 case it modifies w32_showwindow, which is used by
     * win32_spawnvp().  Since w32_showwindow is an internal variable
     * inside the thread_intern structure, the MSWin32 implementation
     * lives in win32/win32.c in the core Perl distribution.
     */
    dSP;
    I32 ax = POPMARK;
    EXTEND(SP,1);
    XSRETURN_UNDEF;
}
#endif

XS(w32_GetCwd)
{
    dXSARGS;
    char* ptr;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetCwd()");

    /* Make the host for current directory */
    ptr = PerlEnv_get_childdir();
    /*
     * If ptr != Nullch
     *   then it worked, set PV valid,
     *   else return 'undef'
     */
    if (ptr) {
	SV *sv = sv_newmortal();
	sv_setpv(sv, ptr);
	PerlEnv_free_childdir(ptr);

#ifndef INCOMPLETE_TAINTS
	SvTAINTED_on(sv);
#endif

	EXTEND(SP,1);
	ST(0) = sv;
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

XS(w32_SetCwd)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetCwd($cwd)");

    if (SvUTF8(ST(0))) {
        WCHAR *wide = sv_to_wstr(aTHX_ ST(0));
        char *ansi = my_ansipath(wide);
        int rc = PerlDir_chdir(ansi);
        Safefree(wide);
        Safefree(ansi);
        if (!rc)
            XSRETURN_YES;
    }
    else {
        if (!PerlDir_chdir(SvPV_nolen(ST(0))))
            XSRETURN_YES;
    }

    XSRETURN_NO;
}

XS(w32_GetNextAvailDrive)
{
    dXSARGS;
    char ix = 'C';
    char root[] = "_:\\";

    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetNextAvailDrive()");
    EXTEND(SP,1);
    while (ix <= 'Z') {
	root[0] = ix++;
	if (GetDriveType(root) == 1) {
	    root[2] = '\0';
	    XSRETURN_PV(root);
	}
    }
    XSRETURN_UNDEF;
}

XS(w32_GetLastError)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetLastError()");
    EXTEND(SP,1);
    XSRETURN_IV(GetLastError());
}

XS(w32_SetLastError)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetLastError($error)");
    SetLastError((DWORD)SvIV(ST(0)));
    XSRETURN_EMPTY;
}

XS(w32_LoginName)
{
    dXSARGS;
    WCHAR name[128];
    DWORD size = countof(name);

    if (items)
	Perl_croak(aTHX_ "usage: Win32::LoginName()");

    EXTEND(SP,1);

    if (GetUserNameW(name, &size)) {
        ST(0) = wstr_to_sv(aTHX_ name);
        XSRETURN(1);
    }

    XSRETURN_UNDEF;
}

XS(w32_NodeName)
{
    dXSARGS;
    char name[MAX_COMPUTERNAME_LENGTH+1];
    DWORD size = sizeof(name);
    if (items)
	Perl_croak(aTHX_ "usage: Win32::NodeName()");
    EXTEND(SP,1);
    if (GetComputerName(name,&size)) {
	/* size does NOT include NULL :-( */
	ST(0) = sv_2mortal(newSVpvn(name,size));
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}


XS(w32_DomainName)
{
    dXSARGS;
    char dname[256];
    DWORD dnamelen = sizeof(dname);
    WKSTA_INFO_100 *pwi;
    DWORD retval;

    if (items)
	Perl_croak(aTHX_ "usage: Win32::DomainName()");

    EXTEND(SP,1);

    retval = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pwi);
    /* NERR_Success *is* 0*/
    if (retval == 0) {
        if (pwi->wki100_langroup && *(pwi->wki100_langroup)) {
            WideCharToMultiByte(CP_ACP, 0, pwi->wki100_langroup,
                                -1, (LPSTR)dname, dnamelen, NULL, NULL);
        }
        else {
            WideCharToMultiByte(CP_ACP, 0, pwi->wki100_computername,
                                -1, (LPSTR)dname, dnamelen, NULL, NULL);
        }
        NetApiBufferFree(pwi);
        XSRETURN_PV(dname);
    }
    SetLastError(retval);
    XSRETURN_UNDEF;
}

XS(w32_FsType)
{
    dXSARGS;
    char fsname[256];
    DWORD flags, filecomplen;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::FsType()");
    if (GetVolumeInformation(NULL, NULL, 0, NULL, &filecomplen,
                             &flags, fsname, sizeof(fsname))) {
	if (GIMME_V == G_ARRAY) {
	    XPUSHs(sv_2mortal(newSVpvn(fsname,strlen(fsname))));
	    XPUSHs(sv_2mortal(newSViv(flags)));
	    XPUSHs(sv_2mortal(newSViv(filecomplen)));
	    PUTBACK;
	    return;
	}
	EXTEND(SP,1);
	XSRETURN_PV(fsname);
    }
    XSRETURN_EMPTY;
}

XS(w32_GetOSVersion)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetOSVersion()");

    if (GIMME_V == G_SCALAR) {
        XSRETURN_IV(g_osver.dwPlatformId);
    }
    XPUSHs(sv_2mortal(newSVpvn(g_osver.szCSDVersion, strlen(g_osver.szCSDVersion))));

    XPUSHs(sv_2mortal(newSViv(g_osver.dwMajorVersion)));
    XPUSHs(sv_2mortal(newSViv(g_osver.dwMinorVersion)));
    XPUSHs(sv_2mortal(newSViv(g_osver.dwBuildNumber)));
    XPUSHs(sv_2mortal(newSViv(g_osver.dwPlatformId)));
    if (g_osver_ex) {
        XPUSHs(sv_2mortal(newSViv(g_osver.wServicePackMajor)));
        XPUSHs(sv_2mortal(newSViv(g_osver.wServicePackMinor)));
        XPUSHs(sv_2mortal(newSViv(g_osver.wSuiteMask)));
        XPUSHs(sv_2mortal(newSViv(g_osver.wProductType)));
    }
    PUTBACK;
}

XS(w32_IsWinNT)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::IsWinNT()");
    EXTEND(SP,1);
    XSRETURN_IV(g_osver.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

XS(w32_IsWin95)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::IsWin95()");
    EXTEND(SP,1);
    XSRETURN_IV(g_osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}

XS(w32_FormatMessage)
{
    dXSARGS;
    DWORD source = 0;
    char msgbuf[ONE_K_BUFSIZE];

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::FormatMessage($errno)");

    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                       &source, (DWORD)SvIV(ST(0)), 0,
                       msgbuf, sizeof(msgbuf)-1, NULL))
    {
        XSRETURN_PV(msgbuf);
    }

    XSRETURN_UNDEF;
}

XS(w32_Spawn)
{
    dXSARGS;
    char *cmd, *args;
    void *env;
    char *dir;
    PROCESS_INFORMATION stProcInfo;
    STARTUPINFO stStartInfo;
    BOOL bSuccess = FALSE;

    if (items != 3)
	Perl_croak(aTHX_ "usage: Win32::Spawn($cmdName, $args, $PID)");

    cmd = SvPV_nolen(ST(0));
    args = SvPV_nolen(ST(1));

    env = PerlEnv_get_childenv();
    dir = PerlEnv_get_childdir();

    memset(&stStartInfo, 0, sizeof(stStartInfo));   /* Clear the block */
    stStartInfo.cb = sizeof(stStartInfo);	    /* Set the structure size */
    stStartInfo.dwFlags = STARTF_USESHOWWINDOW;	    /* Enable wShowWindow control */
    stStartInfo.wShowWindow = SW_SHOWMINNOACTIVE;   /* Start min (normal) */

    if (CreateProcess(
		cmd,			/* Image path */
		args,	 		/* Arguments for command line */
		NULL,			/* Default process security */
		NULL,			/* Default thread security */
		FALSE,			/* Must be TRUE to use std handles */
		NORMAL_PRIORITY_CLASS,	/* No special scheduling */
		env,			/* Inherit our environment block */
		dir,			/* Inherit our currrent directory */
		&stStartInfo,		/* -> Startup info */
		&stProcInfo))		/* <- Process info (if OK) */
    {
	int pid = (int)stProcInfo.dwProcessId;
	sv_setiv(ST(2), pid);
	CloseHandle(stProcInfo.hThread);/* library source code does this. */
	bSuccess = TRUE;
    }
    PerlEnv_free_childenv(env);
    PerlEnv_free_childdir(dir);
    XSRETURN_IV(bSuccess);
}

XS(w32_GetTickCount)
{
    dXSARGS;
    DWORD msec = GetTickCount();
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetTickCount()");
    EXTEND(SP,1);
    if ((IV)msec > 0)
	XSRETURN_IV(msec);
    XSRETURN_NV(msec);
}

XS(w32_GetShortPathName)
{
    dXSARGS;
    DWORD len;
    WCHAR wshort[MAX_PATH+1], *wlong;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetShortPathName($longPathName)");

    wlong = sv_to_wstr(aTHX_ ST(0));
    len = GetShortPathNameW(wlong, wshort, countof(wshort));
    Safefree(wlong);

    if (len && len < sizeof(wshort)) {
        ST(0) = wstr_to_sv(aTHX_ wshort);
        XSRETURN(1);
    }

    XSRETURN_UNDEF;
}

XS(w32_GetFullPathName)
{
    dXSARGS;
    char *fullname;
    char *ansi = NULL;

/* The code below relies on the fact that PerlDir_mapX() returns an
 * absolute path, which is only true under PERL_IMPLICIT_SYS when
 * we use the virtualization code from win32/vdir.h.
 * Without it PerlDir_mapX() is a no-op and we need to use the same
 * code as we use for Cygwin.
 */
#if __CYGWIN__ || !defined(PERL_IMPLICIT_SYS)
    char buffer[2*MAX_PATH];
#endif

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetFullPathName($filename)");

#if __CYGWIN__ || !defined(PERL_IMPLICIT_SYS)
    {
        WCHAR *filename = sv_to_wstr(aTHX_ ST(0));
        WCHAR full[2*MAX_PATH];
        DWORD len = GetFullPathNameW(filename, countof(full), full, NULL);
        Safefree(filename);
        if (len == 0 || len >= countof(full))
            XSRETURN_EMPTY;
        ansi = fullname = my_ansipath(full);
    }
#else
    /* Don't use my_ansipath() unless the $filename argument is in Unicode.
     * If the relative path doesn't exist, GetShortPathName() will fail and
     * my_ansipath() will use the long name with replacement characters.
     * In that case we will be better off using PerlDir_mapA(), which
     * already uses the ANSI name of the current directory.
     *
     * XXX The one missing case is where we could downgrade $filename
     * XXX from UTF8 into the current codepage.
     */
    if (SvUTF8(ST(0))) {
        WCHAR *filename = sv_to_wstr(aTHX_ ST(0));
        WCHAR *mappedname = PerlDir_mapW(filename);
        Safefree(filename);
        ansi = fullname = my_ansipath(mappedname);
    }
    else {
        fullname = PerlDir_mapA(SvPV_nolen(ST(0)));
    }
#  if PERL_VERSION < 8
    {
        /* PerlDir_mapX() in Perl 5.6 used to return forward slashes */
        char *str = fullname;
        while (*str) {
            if (*str == '/')
                *str = '\\';
            ++str;
        }
    }
#  endif
#endif

    /* GetFullPathName() on Windows NT drops trailing backslash */
    if (g_osver.dwMajorVersion == 4 && *fullname) {
        STRLEN len;
        char *pv = SvPV(ST(0), len);
        char *lastchar = fullname + strlen(fullname) - 1;
        /* If ST(0) ends with a slash, but fullname doesn't ... */
        if (len && (pv[len-1] == '/' || pv[len-1] == '\\') && *lastchar != '\\') {
            /* fullname is the MAX_PATH+1 sized buffer returned from PerlDir_mapA()
             * or the 2*MAX_PATH sized local buffer in the __CYGWIN__ case.
             */
            if (lastchar - fullname < MAX_PATH - 1)
                strcpy(lastchar+1, "\\");
        }
    }

    if (GIMME_V == G_ARRAY) {
        char *filepart = strrchr(fullname, '\\');

        EXTEND(SP,1);
        if (filepart) {
            XST_mPV(1, ++filepart);
            *filepart = '\0';
        }
        else {
            XST_mPVN(1, "", 0);
        }
        items = 2;
    }
    XST_mPV(0, fullname);

    if (ansi)
        Safefree(ansi);
    XSRETURN(items);
}

XS(w32_GetLongPathName)
{
    dXSARGS;
    WCHAR *wstr, *long_path, wide_path[MAX_PATH+1];

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetLongPathName($pathname)");

    wstr = sv_to_wstr(aTHX_ ST(0));

    if (wcslen(wstr) < (size_t)countof(wide_path)) {
        wcscpy(wide_path, wstr);
        long_path = my_longpathW(wide_path);
        if (long_path) {
            Safefree(wstr);
            ST(0) = wstr_to_sv(aTHX_ long_path);
            XSRETURN(1);
        }
    }
    Safefree(wstr);
    XSRETURN_EMPTY;
}

XS(w32_GetANSIPathName)
{
    dXSARGS;
    WCHAR *wide_path;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetANSIPathName($pathname)");

    wide_path = sv_to_wstr(aTHX_ ST(0));
    ST(0) = wstr_to_ansipath(aTHX_ wide_path);
    Safefree(wide_path);
    XSRETURN(1);
}

XS(w32_Sleep)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::Sleep($milliseconds)");
    Sleep((DWORD)SvIV(ST(0)));
    XSRETURN_YES;
}

XS(w32_CopyFile)
{
    dXSARGS;
    BOOL bResult;
    char *pszSourceFile;
    char szSourceFile[MAX_PATH+1];

    if (items != 3)
	Perl_croak(aTHX_ "usage: Win32::CopyFile($from, $to, $overwrite)");

    pszSourceFile = PerlDir_mapA(SvPV_nolen(ST(0)));
    if (strlen(pszSourceFile) < sizeof(szSourceFile)) {
        strcpy(szSourceFile, pszSourceFile);
        bResult = CopyFileA(szSourceFile, PerlDir_mapA(SvPV_nolen(ST(1))), !SvTRUE(ST(2)));
        if (bResult)
            XSRETURN_YES;
    }
    XSRETURN_NO;
}

XS(w32_OutputDebugString)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::OutputDebugString($string)");

    if (SvUTF8(ST(0))) {
        WCHAR *str = sv_to_wstr(aTHX_ ST(0));
        OutputDebugStringW(str);
        Safefree(str);
    }
    else
        OutputDebugStringA(SvPV_nolen(ST(0)));

    XSRETURN_EMPTY;
}

XS(w32_GetCurrentProcessId)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetCurrentProcessId()");
    EXTEND(SP,1);
    XSRETURN_IV(GetCurrentProcessId());
}

XS(w32_GetCurrentThreadId)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetCurrentThreadId()");
    EXTEND(SP,1);
    XSRETURN_IV(GetCurrentThreadId());
}

XS(w32_CreateDirectory)
{
    dXSARGS;
    BOOL result;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::CreateDirectory($dir)");

    if (SvUTF8(ST(0))) {
        WCHAR *dir = sv_to_wstr(aTHX_ ST(0));
        result = CreateDirectoryW(dir, NULL);
        Safefree(dir);
    }
    else {
        result = CreateDirectoryA(SvPV_nolen(ST(0)), NULL);
    }

    ST(0) = boolSV(result);
    XSRETURN(1);
}

XS(w32_CreateFile)
{
    dXSARGS;
    HANDLE handle;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::CreateFile($file)");

    if (SvUTF8(ST(0))) {
        WCHAR *file = sv_to_wstr(aTHX_ ST(0));
        handle = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_WRITE,
                             NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        Safefree(file);
    }
    else {
        handle = CreateFileA(SvPV_nolen(ST(0)), GENERIC_WRITE, FILE_SHARE_WRITE,
                             NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (handle != INVALID_HANDLE_VALUE)
        CloseHandle(handle);

    ST(0) = boolSV(handle != INVALID_HANDLE_VALUE);
    XSRETURN(1);
}

XS(w32_GetSystemMetrics)
{
    dXSARGS;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetSystemMetrics($index)");

    XSRETURN_IV(GetSystemMetrics((int)SvIV(ST(0))));
}

XS(w32_GetProductInfo)
{
    dXSARGS;
    DWORD type;
    HMODULE module;
    PFNGetProductInfo pfnGetProductInfo;

    if (items != 4)
	Perl_croak(aTHX_ "usage: Win32::GetProductInfo($major,$minor,$spmajor,$spminor)");

    module = GetModuleHandle("kernel32.dll");
    GETPROC(GetProductInfo);
    if (pfnGetProductInfo &&
        pfnGetProductInfo((DWORD)SvIV(ST(0)), (DWORD)SvIV(ST(1)),
                          (DWORD)SvIV(ST(2)), (DWORD)SvIV(ST(3)), &type))
    {
        XSRETURN_IV(type);
    }

    /* PRODUCT_UNDEFINED */
    XSRETURN_IV(0);
}

XS(w32_GetACP)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetACP()");
    EXTEND(SP,1);
    XSRETURN_IV(GetACP());
}

XS(w32_GetConsoleCP)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetConsoleCP()");
    EXTEND(SP,1);
    XSRETURN_IV(GetConsoleCP());
}

XS(w32_GetConsoleOutputCP)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetConsoleOutputCP()");
    EXTEND(SP,1);
    XSRETURN_IV(GetConsoleOutputCP());
}

XS(w32_GetOEMCP)
{
    dXSARGS;
    if (items)
	Perl_croak(aTHX_ "usage: Win32::GetOEMCP()");
    EXTEND(SP,1);
    XSRETURN_IV(GetOEMCP());
}

XS(w32_SetConsoleCP)
{
    dXSARGS;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetConsoleCP($id)");

    XSRETURN_IV(SetConsoleCP((int)SvIV(ST(0))));
}

XS(w32_SetConsoleOutputCP)
{
    dXSARGS;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetConsoleOutputCP($id)");

    XSRETURN_IV(SetConsoleOutputCP((int)SvIV(ST(0))));
}

XS(w32_GetProcessPrivileges)
{
    dXSARGS;
    BOOL ret;
    HV *priv_hv;
    HANDLE proc_handle, token;
    char *priv_name = NULL;
    TOKEN_PRIVILEGES *privs = NULL;
    DWORD i, pid, priv_name_len = 100, privs_len = 300;

    if (items > 1)
        Perl_croak(aTHX_ "usage: Win32::GetProcessPrivileges([$pid])");

    if (items == 0) {
        EXTEND(SP, 1);
        pid = GetCurrentProcessId();
    }
    else {
        pid = (DWORD)SvUV(ST(0));
    }

    proc_handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (!proc_handle)
        XSRETURN_NO;

    ret = OpenProcessToken(proc_handle, TOKEN_QUERY, &token);
    CloseHandle(proc_handle);

    if (!ret)
        XSRETURN_NO;

    do {
        Renewc(privs, privs_len, char, TOKEN_PRIVILEGES);
        ret = GetTokenInformation(
            token, TokenPrivileges, privs, privs_len, &privs_len
        );
    } while (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    CloseHandle(token);

    if (!ret) {
        Safefree(privs);
        XSRETURN_NO;
    }

    priv_hv = newHV();
    New(0, priv_name, priv_name_len, char);

    for (i = 0; i < privs->PrivilegeCount; ++i) {
        DWORD ret_len = 0;
        LUID_AND_ATTRIBUTES *priv = &privs->Privileges[i];
        BOOL is_enabled = !!(priv->Attributes & SE_PRIVILEGE_ENABLED);

        if (priv->Attributes & SE_PRIVILEGE_REMOVED)
            continue;

        do {
            ret_len = priv_name_len;
            ret = LookupPrivilegeNameA(
                NULL, &priv->Luid, priv_name, &ret_len
            );

            if (ret_len > priv_name_len) {
                priv_name_len = ret_len + 1;
                Renew(priv_name, priv_name_len, char);
            }
        } while (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        if (!ret) {
            SvREFCNT_dec((SV*)priv_hv);
            Safefree(privs);
            Safefree(priv_name);
            XSRETURN_NO;
        }

        hv_store(priv_hv, priv_name, ret_len, newSViv(is_enabled), 0);
    }

    Safefree(privs);
    Safefree(priv_name);

    ST(0) = sv_2mortal(newRV_noinc((SV*)priv_hv));
    XSRETURN(1);
}

XS(w32_IsDeveloperModeEnabled)
{
    dXSARGS;
    LONG status;
    DWORD val, val_size = sizeof(val);
    PFNRegGetValueA pfnRegGetValueA;
    HMODULE module;

    if (items)
        Perl_croak(aTHX_ "usage: Win32::IsDeveloperModeEnabled()");

    EXTEND(SP, 1);

    /* developer mode was introduced in Windows 10 */
    if (g_osver.dwMajorVersion < 10)
        XSRETURN_NO;

    module = GetModuleHandleA("advapi32.dll");
    GETPROC(RegGetValueA);
    if (!pfnRegGetValueA)
        XSRETURN_NO;

    status = pfnRegGetValueA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock",
        "AllowDevelopmentWithoutDevLicense",
        RRF_RT_REG_DWORD | KEY_WOW64_64KEY,
        NULL,
        &val,
        &val_size
    );

    if (status == ERROR_SUCCESS && val == 1)
        XSRETURN_YES;

    XSRETURN_NO;
}

#ifdef WINHTTPAPI

XS(w32_HttpGetFile)
{
    dXSARGS;
    WCHAR *url = NULL, *file = NULL, *hostName = NULL, *urlPath = NULL;
    bool bIgnoreCertErrors = FALSE;
    WCHAR msgbuf[ONE_K_BUFSIZE];
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL,
               hConnect = NULL,
               hRequest = NULL;
    HANDLE hOut = INVALID_HANDLE_VALUE;
    BOOL   bParsed = FALSE,
           bAborted = FALSE,
           bFileError = FALSE,
           bHttpError = FALSE;
    DWORD error = 0;
    URL_COMPONENTS urlComp;
    LPCWSTR acceptTypes[] = { L"*/*", NULL };
    DWORD dwHttpStatusCode = 0, dwQuerySize = 0;

    if (items < 2 || items > 3)
        croak("usage: Win32::HttpGetFile($url, $file[, $ignore_cert_errors])");

    url = sv_to_wstr(aTHX_ ST(0));
    file = sv_to_wstr(aTHX_ ST(1));

    if (items == 3)
        bIgnoreCertErrors = (BOOL)SvIV(ST(2));

    /* Initialize the URL_COMPONENTS structure, setting the required
     * component lengths to non-zero so that they get populated.
     */
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength    = (DWORD)-1;
    urlComp.dwHostNameLength  = (DWORD)-1;
    urlComp.dwUrlPathLength   = (DWORD)-1;
    urlComp.dwExtraInfoLength = (DWORD)-1;

    /* Parse the URL. */
    bParsed = WinHttpCrackUrl(url, (DWORD)wcslen(url), 0, &urlComp);

    /* Only support http and htts, not ftp, gopher, etc. */
    if (bParsed
        && !(urlComp.nScheme == INTERNET_SCHEME_HTTPS
             || urlComp.nScheme == INTERNET_SCHEME_HTTP)) {
        SetLastError(12006); /* not a recognized protocol */
        bParsed = FALSE;
    }

    if (bParsed) {
        New(0, hostName,  urlComp.dwHostNameLength + 1, WCHAR);
        wcsncpy(hostName, urlComp.lpszHostName, urlComp.dwHostNameLength);
        hostName[urlComp.dwHostNameLength] = 0;

        New(0, urlPath,  urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength + 1, WCHAR);
        wcsncpy(urlPath, urlComp.lpszUrlPath, urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength);
        urlPath[urlComp.dwUrlPathLength + urlComp.dwExtraInfoLength] = 0;

        /* Use WinHttpOpen to obtain a session handle. */
        hSession = WinHttpOpen(L"Perl",
                               WINHTTP_ACCESS_TYPE_NO_PROXY,
                               WINHTTP_NO_PROXY_NAME,
                               WINHTTP_NO_PROXY_BYPASS,
                               0);
    }

    /* Specify an HTTP server. */
    if (hSession)
        hConnect = WinHttpConnect(hSession,
                                  hostName,
                                  urlComp.nPort,
                                  0);

    /* Create an HTTP request handle. */
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect,
                                      L"GET",
                                      urlPath,
                                      NULL,
                                      WINHTTP_NO_REFERER,
                                      acceptTypes,
                                      urlComp.nScheme == INTERNET_SCHEME_HTTPS
                                                      ? WINHTTP_FLAG_SECURE
                                                      : 0);

    /* If specified, disable certificate-related errors for https connections. */
    if (hRequest
        && bIgnoreCertErrors
        && urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
        DWORD secFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID
                         | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
                         | SECURITY_FLAG_IGNORE_UNKNOWN_CA
                         | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        if(!WinHttpSetOption(hRequest,
                             WINHTTP_OPTION_SECURITY_FLAGS,
                             &secFlags,
                             sizeof(secFlags))) {
            bAborted = TRUE;
        }
    }

    /* Call WinHttpGetProxyForUrl with our target URL. If auto-proxy succeeds,
     * then set the proxy info on the request handle. If auto-proxy fails,
     * ignore the error and attempt to send the HTTP request directly to the
     * target server (using the default WINHTTP_ACCESS_TYPE_NO_PROXY
     * configuration, which the request handle will inherit from the session).
     */
    if (hRequest && !bAborted) {
        WINHTTP_AUTOPROXY_OPTIONS  AutoProxyOptions;
        WINHTTP_PROXY_INFO         ProxyInfo;
        DWORD                      cbProxyInfoSize = sizeof(ProxyInfo);

        ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
        ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));
        AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
        AutoProxyOptions.dwAutoDetectFlags =
                                    WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                    WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

        if(WinHttpGetProxyForUrl(hSession,
                                url,
                                &AutoProxyOptions,
                                &ProxyInfo)) {
            if(!WinHttpSetOption(hRequest,
                                WINHTTP_OPTION_PROXY,
                                &ProxyInfo,
                                cbProxyInfoSize)) {
                bAborted = TRUE;
                Perl_warn(aTHX_ "Win32::HttpGetFile: setting proxy options failed");
            }
            Safefree(ProxyInfo.lpszProxy);
            Safefree(ProxyInfo.lpszProxyBypass);
        }
    }

    /* Send a request. */
    if (hRequest && !bAborted)
        bResults = WinHttpSendRequest(hRequest,
                                      WINHTTP_NO_ADDITIONAL_HEADERS,
                                      0,
                                      WINHTTP_NO_REQUEST_DATA,
                                      0,
                                      0,
                                      0);

    /* End the request. */
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);

    /* Retrieve HTTP status code. */
    if (bResults) {
        dwQuerySize = sizeof(dwHttpStatusCode);
        bResults = WinHttpQueryHeaders(hRequest,
                                       WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                       WINHTTP_HEADER_NAME_BY_INDEX,
                                       &dwHttpStatusCode,
                                       &dwQuerySize,
                                       WINHTTP_NO_HEADER_INDEX);
    }

    /* Retrieve HTTP status text. Note this may be a success message. */
    if (bResults) {
        dwQuerySize = ONE_K_BUFSIZE * 2 - 2;
        ZeroMemory(&msgbuf, ONE_K_BUFSIZE * 2);
        bResults = WinHttpQueryHeaders(hRequest,
                                       WINHTTP_QUERY_STATUS_TEXT,
                                       WINHTTP_HEADER_NAME_BY_INDEX,
                                       msgbuf,
                                       &dwQuerySize,
                                       WINHTTP_NO_HEADER_INDEX);
    }

    /* There is no point in successfully downloading an error page from
     * the server, so consider HTTP errors to be failures.
     */
    if (bResults) {
        if (dwHttpStatusCode < 200 || dwHttpStatusCode > 299) {
            bResults = FALSE;
            bHttpError = TRUE;
        }
    }

    /* Create output file for download. */
    if (bResults) {
        hOut = CreateFileW(file,
                           GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hOut == INVALID_HANDLE_VALUE)
            bFileError = TRUE;
    }

    if (!bFileError && bResults) {
        DWORD dwDownloaded = 0;
        DWORD dwBytesWritten = 0;
        DWORD dwSize = 65536;
        char *pszOutBuffer;

        New(0, pszOutBuffer, dwSize, char);

        /* Keep checking for data until there is nothing left. */
        while (1) {
            if (!WinHttpReadData(hRequest,
                                 (LPVOID)pszOutBuffer,
                                 dwSize,
                                 &dwDownloaded)) {
                bAborted = TRUE;
                break;
            }
            if (!dwDownloaded)
                break;

            /* Write what we just read to the output file */
            if (!WriteFile(hOut,
                           pszOutBuffer,
                           dwDownloaded,
                           &dwBytesWritten,
                           NULL)) {
                bAborted = TRUE;
                bFileError = TRUE;
                break;
            }

        }

        Safefree(pszOutBuffer);
    }
    else {
        bAborted = TRUE;
    }

    /* Clean-up may lose this. */
    if (bAborted)
        error = GetLastError();

    /* If we successfully opened the output file but failed later, mark
     * the file for deletion.
     */
    if (bAborted && hOut != INVALID_HANDLE_VALUE)
        (void) DeleteFileW(file);

    /* Close any open handles. */
    if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    Safefree(url);
    Safefree(file);
    Safefree(hostName);
    Safefree(urlPath);

    /* Retrieve system and WinHttp error messages, or compose a user-defined
     * error code if we got a failed HTTP status text above.  Conveniently, adding
     * 1e9 to the HTTP status sets bit 29, denoting a user-defined error code,
     * and also makes it easy to lop off the upper part and just get HTTP status.
     */
    if (bAborted) {
        if (bHttpError) {
            SetLastError(dwHttpStatusCode + 1000000000);
        }
        else {
            DWORD msgFlags = bFileError
                            ? FORMAT_MESSAGE_FROM_SYSTEM
                            : FORMAT_MESSAGE_FROM_HMODULE;
            msgFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;

            ZeroMemory(&msgbuf, ONE_K_BUFSIZE * 2);
            if (!FormatMessageW(msgFlags,
                                GetModuleHandleW(L"winhttp.dll"),
                                error,
                                0,
                                msgbuf,
                                ONE_K_BUFSIZE - 1, /* TCHARs, not bytes */
                                NULL)) {
                wcsncpy(msgbuf, L"unable to format error message", ONE_K_BUFSIZE - 1);
            }
            SetLastError(error);
        }
    }

    if (GIMME_V == G_SCALAR) {
        EXTEND(SP, 1);
        ST(0) = !bAborted ? &PL_sv_yes : &PL_sv_no;
        XSRETURN(1);
    }
    else if (GIMME_V == G_ARRAY) {
        EXTEND(SP, 2);
        ST(0) = !bAborted ? &PL_sv_yes : &PL_sv_no;
        ST(1) = wstr_to_sv(aTHX_ msgbuf);
        XSRETURN(2);
    }
    else {
        XSRETURN_EMPTY;
    }
}

#endif

MODULE = Win32            PACKAGE = Win32

PROTOTYPES: DISABLE

BOOT:
{
    const char *file = __FILE__;

    if (g_osver.dwOSVersionInfoSize == 0) {
        g_osver.dwOSVersionInfoSize = sizeof(g_osver);
        if (!GetVersionExA((OSVERSIONINFOA*)&g_osver)) {
            g_osver_ex = FALSE;
            g_osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
            GetVersionExA((OSVERSIONINFOA*)&g_osver);
        }
    }

    newXS("Win32::LookupAccountName", w32_LookupAccountName, file);
    newXS("Win32::LookupAccountSID", w32_LookupAccountSID, file);
    newXS("Win32::InitiateSystemShutdown", w32_InitiateSystemShutdown, file);
    newXS("Win32::AbortSystemShutdown", w32_AbortSystemShutdown, file);
    newXS("Win32::ExpandEnvironmentStrings", w32_ExpandEnvironmentStrings, file);
    newXS("Win32::MsgBox", w32_MsgBox, file);
    newXS("Win32::LoadLibrary", w32_LoadLibrary, file);
    newXS("Win32::FreeLibrary", w32_FreeLibrary, file);
    newXS("Win32::GetProcAddress", w32_GetProcAddress, file);
    newXS("Win32::RegisterServer", w32_RegisterServer, file);
    newXS("Win32::UnregisterServer", w32_UnregisterServer, file);
    newXS("Win32::GetArchName", w32_GetArchName, file);
    newXS("Win32::GetChipArch", w32_GetChipArch, file);
    newXS("Win32::GetChipName", w32_GetChipName, file);
    newXS("Win32::GuidGen", w32_GuidGen, file);
    newXS("Win32::GetFolderPath", w32_GetFolderPath, file);
    newXS("Win32::IsAdminUser", w32_IsAdminUser, file);
    newXS("Win32::GetFileVersion", w32_GetFileVersion, file);

    newXS("Win32::GetCwd", w32_GetCwd, file);
    newXS("Win32::SetCwd", w32_SetCwd, file);
    newXS("Win32::GetNextAvailDrive", w32_GetNextAvailDrive, file);
    newXS("Win32::GetLastError", w32_GetLastError, file);
    newXS("Win32::SetLastError", w32_SetLastError, file);
    newXS("Win32::LoginName", w32_LoginName, file);
    newXS("Win32::NodeName", w32_NodeName, file);
    newXS("Win32::DomainName", w32_DomainName, file);
    newXS("Win32::FsType", w32_FsType, file);
    newXS("Win32::GetOSVersion", w32_GetOSVersion, file);
    newXS("Win32::IsWinNT", w32_IsWinNT, file);
    newXS("Win32::IsWin95", w32_IsWin95, file);
    newXS("Win32::FormatMessage", w32_FormatMessage, file);
    newXS("Win32::Spawn", w32_Spawn, file);
    newXS("Win32::GetTickCount", w32_GetTickCount, file);
    newXS("Win32::GetShortPathName", w32_GetShortPathName, file);
    newXS("Win32::GetFullPathName", w32_GetFullPathName, file);
    newXS("Win32::GetLongPathName", w32_GetLongPathName, file);
    newXS("Win32::GetANSIPathName", w32_GetANSIPathName, file);
    newXS("Win32::CopyFile", w32_CopyFile, file);
    newXS("Win32::Sleep", w32_Sleep, file);
    newXS("Win32::OutputDebugString", w32_OutputDebugString, file);
    newXS("Win32::GetCurrentProcessId", w32_GetCurrentProcessId, file);
    newXS("Win32::GetCurrentThreadId", w32_GetCurrentThreadId, file);
    newXS("Win32::CreateDirectory", w32_CreateDirectory, file);
    newXS("Win32::CreateFile", w32_CreateFile, file);
    newXS("Win32::GetSystemMetrics", w32_GetSystemMetrics, file);
    newXS("Win32::GetProductInfo", w32_GetProductInfo, file);
    newXS("Win32::GetACP", w32_GetACP, file);
    newXS("Win32::GetConsoleCP", w32_GetConsoleCP, file);
    newXS("Win32::GetConsoleOutputCP", w32_GetConsoleOutputCP, file);
    newXS("Win32::GetOEMCP", w32_GetOEMCP, file);
    newXS("Win32::SetConsoleCP", w32_SetConsoleCP, file);
    newXS("Win32::SetConsoleOutputCP", w32_SetConsoleOutputCP, file);
    newXS("Win32::GetProcessPrivileges", w32_GetProcessPrivileges, file);
    newXS("Win32::IsDeveloperModeEnabled", w32_IsDeveloperModeEnabled, file);
#ifdef __CYGWIN__
    newXS("Win32::SetChildShowWindow", w32_SetChildShowWindow, file);
#endif
#ifdef WINHTTPAPI
    newXS("Win32::HttpGetFile", w32_HttpGetFile, file);
#endif
    XSRETURN_YES;
}
