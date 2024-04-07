/* WIN32.C
 *
 * (c) 1995 Microsoft Corporation. All rights reserved.
 * 		Developed by hip communications inc.
 * Portions (c) 1993 Intergraph Corporation. All rights reserved.
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */
#define PERLIO_NOT_STDIO 0
#define WIN32_LEAN_AND_MEAN
#define WIN32IO_IS_STDIO
/* for CreateSymbolicLinkA() etc */
#define _WIN32_WINNT 0x0601
#include <tchar.h>

#ifdef __GNUC__
#  define Win32_Winsock
#endif

#include <windows.h>

#ifndef HWND_MESSAGE
#  define HWND_MESSAGE ((HWND)-3)
#endif

#ifndef PROCESSOR_ARCHITECTURE_AMD64
#  define PROCESSOR_ARCHITECTURE_AMD64 9
#endif

#ifndef WC_NO_BEST_FIT_CHARS
#  define WC_NO_BEST_FIT_CHARS 0x00000400
#endif

#include <winnt.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <io.h>
#include <signal.h>
#include <winioctl.h>
#include <winternl.h>

/* #include "config.h" */


#define PerlIO FILE

#include <sys/stat.h>
#include "EXTERN.h"
#include "perl.h"

#define NO_XSLOCKS
#define PERL_NO_GET_CONTEXT
#include "XSUB.h"

#include <fcntl.h>
#ifndef __GNUC__
/* assert.h conflicts with #define of assert in perl.h */
#  include <assert.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <time.h>
#include <sys/utime.h>
#include <wchar.h>

#ifdef __GNUC__
/* Mingw32 defaults to globing command line
 * So we turn it off like this:
 */
int _CRT_glob = 0;
#endif

#if defined(__MINGW32__) && (__MINGW32_MAJOR_VERSION==1)	
/* Mingw32-1.1 is missing some prototypes */
START_EXTERN_C
FILE * _wfopen(LPCWSTR wszFileName, LPCWSTR wszMode);
FILE * _wfdopen(int nFd, LPCWSTR wszMode);
FILE * _freopen(LPCWSTR wszFileName, LPCWSTR wszMode, FILE * pOldStream);
int _flushall();
int _fcloseall();
END_EXTERN_C
#endif

#define EXECF_EXEC 1
#define EXECF_SPAWN 2
#define EXECF_SPAWN_NOWAIT 3

#if defined(PERL_IMPLICIT_SYS)
#  undef getlogin
#  define getlogin g_getlogin
#endif

#ifdef _MSC_VER
#  define SET_INVALID_PARAMETER_HANDLER
#endif

#ifdef SET_INVALID_PARAMETER_HANDLER
static BOOL	set_silent_invalid_parameter_handler(BOOL newvalue);
static void	my_invalid_parameter_handler(const wchar_t* expression,
                        const wchar_t* function, const wchar_t* file,
                        unsigned int line, uintptr_t pReserved);
#endif

#ifndef WIN32_NO_REGISTRY
static char*	get_regstr_from(HKEY hkey, const char *valuename, SV **svp);
static char*	get_regstr(const char *valuename, SV **svp);
#endif

static char*	get_emd_part(SV **prev_pathp, STRLEN *const len,
                        const char *trailing, ...);
static char*	win32_get_xlib(const char *pl,
                        WIN32_NO_REGISTRY_M_(const char *xlib)
                        const char *libname, STRLEN *const len);

static BOOL	has_shell_metachars(const char *ptr);
static long	tokenize(const char *str, char **dest, char ***destv);
static int	get_shell(void);
static char*	find_next_space(const char *s);
static int	do_spawn2(pTHX_ const char *cmd, int exectype);
static int	do_spawn2_handles(pTHX_ const char *cmd, int exectype,
                        const int *handles);
static int	do_spawnvp_handles(int mode, const char *cmdname,
                        const char * const *argv, const int *handles);
static PerlIO * do_popen(const char *mode, const char *command, IV narg,
                         SV **args);
static long	find_pid(pTHX_ int pid);
static void	remove_dead_process(long child);
static int	terminate_process(DWORD pid, HANDLE process_handle, int sig);
static int	my_killpg(int pid, int sig);
static int	my_kill(int pid, int sig);
static void	out_of_memory(void);
static char*	wstr_to_str(const wchar_t* wstr);
static long	filetime_to_clock(PFILETIME ft);
static BOOL	filetime_from_time(PFILETIME ft, time_t t);
static char*	create_command_line(char *cname, STRLEN clen,
                                    const char * const *args);
static char*	qualified_path(const char *cmd, bool other_exts);
static void	ansify_path(void);
static LRESULT	win32_process_message(HWND hwnd, UINT msg,
                        WPARAM wParam, LPARAM lParam);

#ifdef USE_ITHREADS
static long	find_pseudo_pid(pTHX_ int pid);
static void	remove_dead_pseudo_process(long child);
static HWND	get_hwnd_delay(pTHX, long child, DWORD tries);
#endif

#ifdef HAVE_INTERP_INTERN
static void	win32_csighandler(int sig);
#endif

static void translate_to_errno(void);

START_EXTERN_C
HANDLE	w32_perldll_handle = INVALID_HANDLE_VALUE;
char	w32_module_name[MAX_PATH+1];
END_EXTERN_C

static OSVERSIONINFO g_osver = {0, 0, 0, 0, 0, ""};

#ifndef WIN32_NO_REGISTRY
/* initialized by Perl_win32_init/PERL_SYS_INIT */
static HKEY HKCU_Perl_hnd;
static HKEY HKLM_Perl_hnd;
#endif

/* the time_t epoch start time as a filetime expressed as a large integer */
static ULARGE_INTEGER time_t_epoch_base_filetime;

static const SYSTEMTIME time_t_epoch_base_systemtime = {
    1970,    /* wYear         */
    1,       /* wMonth        */
    0,       /* wDayOfWeek    */
    1,       /* wDay          */
    0,       /* wHour         */
    0,       /* wMinute       */
    0,       /* wSecond       */
    0        /* wMilliseconds */
};

#define FILETIME_CHUNKS_PER_SECOND (10000000UL)

#ifdef USE_ITHREADS
static perl_mutex win32_read_console_mutex;
#endif

#ifdef SET_INVALID_PARAMETER_HANDLER
static BOOL silent_invalid_parameter_handler = FALSE;

static BOOL
set_silent_invalid_parameter_handler(BOOL newvalue)
{
    BOOL oldvalue = silent_invalid_parameter_handler;
#  ifdef _DEBUG
    silent_invalid_parameter_handler = newvalue;
#  endif
    return oldvalue;
}

static void
my_invalid_parameter_handler(const wchar_t* expression,
    const wchar_t* function, 
    const wchar_t* file, 
    unsigned int line, 
    uintptr_t pReserved)
{
#  ifdef _DEBUG
    char* ansi_expression;
    char* ansi_function;
    char* ansi_file;
    if (silent_invalid_parameter_handler)
        return;
    ansi_expression = wstr_to_str(expression);
    ansi_function = wstr_to_str(function);
    ansi_file = wstr_to_str(file);
    fprintf(stderr, "Invalid parameter detected in function %s. "
                    "File: %s, line: %d\n", ansi_function, ansi_file, line);
    fprintf(stderr, "Expression: %s\n", ansi_expression);
    free(ansi_expression);
    free(ansi_function);
    free(ansi_file);
#  endif
}
#endif

EXTERN_C void
set_w32_module_name(void)
{
    /* this function may be called at DLL_PROCESS_ATTACH time */
    char* ptr;
    HMODULE module = (HMODULE)((w32_perldll_handle == INVALID_HANDLE_VALUE)
                               ? GetModuleHandle(NULL)
                               : w32_perldll_handle);

    WCHAR modulename[MAX_PATH];
    WCHAR fullname[MAX_PATH];
    char *ansi;

    DWORD (__stdcall *pfnGetLongPathNameW)(LPCWSTR, LPWSTR, DWORD) =
        (DWORD (__stdcall *)(LPCWSTR, LPWSTR, DWORD))
        GetProcAddress(GetModuleHandle("kernel32.dll"), "GetLongPathNameW");

    GetModuleFileNameW(module, modulename, sizeof(modulename)/sizeof(WCHAR));

    /* Make sure we get an absolute pathname in case the module was loaded
     * explicitly by LoadLibrary() with a relative path. */
    GetFullPathNameW(modulename, sizeof(fullname)/sizeof(WCHAR), fullname, NULL);

    /* Make sure we start with the long path name of the module because we
     * later scan for pathname components to match "5.xx" to locate
     * compatible sitelib directories, and the short pathname might mangle
     * this path segment (e.g. by removing the dot on NTFS to something
     * like "5xx~1.yy") */
    if (pfnGetLongPathNameW)
        pfnGetLongPathNameW(fullname, fullname, sizeof(fullname)/sizeof(WCHAR));

    /* remove \\?\ prefix */
    if (memcmp(fullname, L"\\\\?\\", 4*sizeof(WCHAR)) == 0)
        memmove(fullname, fullname+4, (wcslen(fullname+4)+1)*sizeof(WCHAR));

    ansi = win32_ansipath(fullname);
    my_strlcpy(w32_module_name, ansi, sizeof(w32_module_name));
    win32_free(ansi);

    /* normalize to forward slashes */
    ptr = w32_module_name;
    while (*ptr) {
        if (*ptr == '\\')
            *ptr = '/';
        ++ptr;
    }
}

#ifndef WIN32_NO_REGISTRY
/* *svp (if non-NULL) is expected to be POK (valid allocated SvPVX(*svp)) */
static char*
get_regstr_from(HKEY handle, const char *valuename, SV **svp)
{
    /* Retrieve a REG_SZ or REG_EXPAND_SZ from the registry */
    DWORD type;
    char *str = NULL;
    long retval;
    DWORD datalen;

    retval = RegQueryValueEx(handle, valuename, 0, &type, NULL, &datalen);
    if (retval == ERROR_SUCCESS
        && (type == REG_SZ || type == REG_EXPAND_SZ))
    {
        dTHX;
        if (!*svp)
            *svp = sv_2mortal(newSVpvs(""));
        SvGROW(*svp, datalen);
        retval = RegQueryValueEx(handle, valuename, 0, NULL,
                                 (PBYTE)SvPVX(*svp), &datalen);
        if (retval == ERROR_SUCCESS) {
            str = SvPVX(*svp);
            SvCUR_set(*svp,datalen-1);
        }
    }
    return str;
}

/* *svp (if non-NULL) is expected to be POK (valid allocated SvPVX(*svp)) */
static char*
get_regstr(const char *valuename, SV **svp)
{
    char *str;
    if (HKCU_Perl_hnd) {
        str = get_regstr_from(HKCU_Perl_hnd, valuename, svp);
        if (!str)
            goto try_HKLM;
    }
    else {
        try_HKLM:
        if (HKLM_Perl_hnd)
            str = get_regstr_from(HKLM_Perl_hnd, valuename, svp);
        else
            str = NULL;
    }
    return str;
}
#endif /* ifndef WIN32_NO_REGISTRY */

/* *prev_pathp (if non-NULL) is expected to be POK (valid allocated SvPVX(sv)) */
static char *
get_emd_part(SV **prev_pathp, STRLEN *const len, const char *trailing_path, ...)
{
    char base[10];
    va_list ap;
    char mod_name[MAX_PATH+1];
    char *ptr;
    char *optr;
    char *strip;
    STRLEN baselen;

    va_start(ap, trailing_path);
    strip = va_arg(ap, char *);

    sprintf(base, "%d.%d", (int)PERL_REVISION, (int)PERL_VERSION);
    baselen = strlen(base);

    if (!*w32_module_name) {
        set_w32_module_name();
    }
    strcpy(mod_name, w32_module_name);
    ptr = strrchr(mod_name, '/');
    while (ptr && strip) {
        /* look for directories to skip back */
        optr = ptr;
        *ptr = '\0';
        ptr = strrchr(mod_name, '/');
        /* avoid stripping component if there is no slash,
         * or it doesn't match ... */
        if (!ptr || stricmp(ptr+1, strip) != 0) {
            /* ... but not if component matches m|5\.$patchlevel.*| */
            if (!ptr || !(*strip == '5' && *(ptr+1) == '5'
                          && strnEQ(strip, base, baselen)
                          && strnEQ(ptr+1, base, baselen)))
            {
                *optr = '/';
                ptr = optr;
            }
        }
        strip = va_arg(ap, char *);
    }
    if (!ptr) {
        ptr = mod_name;
        *ptr++ = '.';
        *ptr = '/';
    }
    va_end(ap);
    strcpy(++ptr, trailing_path);

    /* only add directory if it exists */
    if (GetFileAttributes(mod_name) != (DWORD) -1) {
        /* directory exists */
        dTHX;
        if (!*prev_pathp)
            *prev_pathp = sv_2mortal(newSVpvs(""));
        else if (SvPVX(*prev_pathp))
            sv_catpvs(*prev_pathp, ";");
        sv_catpv(*prev_pathp, mod_name);
        if(len)
            *len = SvCUR(*prev_pathp);
        return SvPVX(*prev_pathp);
    }

    return NULL;
}

EXTERN_C char *
win32_get_privlib(WIN32_NO_REGISTRY_M_(const char *pl) STRLEN *const len)
{
    const char *stdlib = "lib";
    SV *sv = NULL;
#ifndef WIN32_NO_REGISTRY
    char buffer[MAX_PATH+1];

    /* $stdlib = $HKCU{"lib-$]"} || $HKLM{"lib-$]"} || $HKCU{"lib"} || $HKLM{"lib"} || "";  */
    sprintf(buffer, "%s-%s", stdlib, pl);
    if (!get_regstr(buffer, &sv))
        (void)get_regstr(stdlib, &sv);
#endif

    /* $stdlib .= ";$EMD/../../lib" */
    return get_emd_part(&sv, len, stdlib, ARCHNAME, "bin", NULL);
}

static char *
win32_get_xlib(const char *pl, WIN32_NO_REGISTRY_M_(const char *xlib)
               const char *libname, STRLEN *const len)
{
#ifndef WIN32_NO_REGISTRY
    char regstr[40];
#endif
    char pathstr[MAX_PATH+1];
    SV *sv1 = NULL;
    SV *sv2 = NULL;

#ifndef WIN32_NO_REGISTRY
    /* $HKCU{"$xlib-$]"} || $HKLM{"$xlib-$]"} . ---; */
    sprintf(regstr, "%s-%s", xlib, pl);
    (void)get_regstr(regstr, &sv1);
#endif

    /* $xlib .=
     * ";$EMD/" . ((-d $EMD/../../../$]) ? "../../.." : "../.."). "/$libname/$]/lib";  */
    sprintf(pathstr, "%s/%s/lib", libname, pl);
    (void)get_emd_part(&sv1, NULL, pathstr, ARCHNAME, "bin", pl, NULL);

#ifndef WIN32_NO_REGISTRY
    /* $HKCU{$xlib} || $HKLM{$xlib} . ---; */
    (void)get_regstr(xlib, &sv2);
#endif

    /* $xlib .=
     * ";$EMD/" . ((-d $EMD/../../../$]) ? "../../.." : "../.."). "/$libname/lib";  */
    sprintf(pathstr, "%s/lib", libname);
    (void)get_emd_part(&sv2, NULL, pathstr, ARCHNAME, "bin", pl, NULL);

    if (!sv1 && !sv2)
        return NULL;
    if (!sv1) {
        sv1 = sv2;
    } else if (sv2) {
        dTHX;
        sv_catpvs(sv1, ";");
        sv_catsv(sv1, sv2);
    }

    if (len)
        *len = SvCUR(sv1);
    return SvPVX(sv1);
}

EXTERN_C char *
win32_get_sitelib(const char *pl, STRLEN *const len)
{
    return win32_get_xlib(pl, WIN32_NO_REGISTRY_M_("sitelib") "site", len);
}

#ifndef PERL_VENDORLIB_NAME
#  define PERL_VENDORLIB_NAME	"vendor"
#endif

EXTERN_C char *
win32_get_vendorlib(const char *pl, STRLEN *const len)
{
    return win32_get_xlib(pl, WIN32_NO_REGISTRY_M_("vendorlib") PERL_VENDORLIB_NAME, len);
}

static BOOL
has_shell_metachars(const char *ptr)
{
    int inquote = 0;
    char quote = '\0';

    /*
     * Scan string looking for redirection (< or >) or pipe
     * characters (|) that are not in a quoted string.
     * Shell variable interpolation (%VAR%) can also happen inside strings.
     */
    while (*ptr) {
        switch(*ptr) {
        case '%':
            return TRUE;
        case '\'':
        case '\"':
            if (inquote) {
                if (quote == *ptr) {
                    inquote = 0;
                    quote = '\0';
                }
            }
            else {
                quote = *ptr;
                inquote++;
            }
            break;
        case '>':
        case '<':
        case '|':
            if (!inquote)
                return TRUE;
        default:
            break;
        }
        ++ptr;
    }
    return FALSE;
}

#if !defined(PERL_IMPLICIT_SYS)
/* since the current process environment is being updated in util.c
 * the library functions will get the correct environment
 */
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    PERL_FLUSHALL_FOR_CHILD;
    return win32_popen(cmd, mode);
}

long
Perl_my_pclose(pTHX_ PerlIO *fp)
{
    return win32_pclose(fp);
}
#endif

DllExport unsigned long
win32_os_id(void)
{
    return (unsigned long)g_osver.dwPlatformId;
}

DllExport int
win32_getpid(void)
{
#ifdef USE_ITHREADS
    dTHX;
    if (w32_pseudo_id)
        return -((int)w32_pseudo_id);
#endif
    return _getpid();
}

/* Tokenize a string.  Words are null-separated, and the list
 * ends with a doubled null.  Any character (except null and
 * including backslash) may be escaped by preceding it with a
 * backslash (the backslash will be stripped).
 * Returns number of words in result buffer.
 */
static long
tokenize(const char *str, char **dest, char ***destv)
{
    char *retstart = NULL;
    char **retvstart = 0;
    int items = -1;
    if (str) {
        int slen = strlen(str);
        char *ret;
        char **retv;
        Newx(ret, slen+2, char);
        Newx(retv, (slen+3)/2, char*);

        retstart = ret;
        retvstart = retv;
        *retv = ret;
        items = 0;
        while (*str) {
            *ret = *str++;
            if (*ret == '\\' && *str)
                *ret = *str++;
            else if (*ret == ' ') {
                while (*str == ' ')
                    str++;
                if (ret == retstart)
                    ret--;
                else {
                    *ret = '\0';
                    ++items;
                    if (*str)
                        *++retv = ret+1;
                }
            }
            else if (!*str)
                ++items;
            ret++;
        }
        retvstart[items] = NULL;
        *ret++ = '\0';
        *ret = '\0';
    }
    *dest = retstart;
    *destv = retvstart;
    return items;
}

static const char
cmd_opts[] = "/x/d/c";

static const char
shell_cmd[] = "cmd.exe";

static int
get_shell(void)
{
    dTHX;
    if (!w32_perlshell_tokens) {
        /* we don't use COMSPEC here for two reasons:
         *  1. the same reason perl on UNIX doesn't use SHELL--rampant and
         *     uncontrolled unportability of the ensuing scripts.
         *  2. PERL5SHELL could be set to a shell that may not be fit for
         *     interactive use (which is what most programs look in COMSPEC
         *     for).
         */
        const char *shell = PerlEnv_getenv("PERL5SHELL");
        if (shell) {
            w32_perlshell_items = tokenize(shell,
                                           &w32_perlshell_tokens,
                                           &w32_perlshell_vec);
        }
        else {
            /* tokenize does some Unix-ish like things like
               \\ escaping that don't work well here
            */
            char shellbuf[MAX_PATH];
            UINT len = GetSystemDirectoryA(shellbuf, sizeof(shellbuf));
            if (len == 0) {
                translate_to_errno();
                return -1;
            }
            else if (len >= MAX_PATH) {
                /* buffer too small */
                errno = E2BIG;
                return -1;
            }
            if (shellbuf[len-1] != '\\') {
                my_strlcat(shellbuf, "\\", sizeof(shellbuf));
                ++len;
            }
            if (len + sizeof(shell_cmd) > sizeof(shellbuf)) {
                errno = E2BIG;
                return -1;
            }
            my_strlcat(shellbuf, shell_cmd, sizeof(shellbuf));
            len += sizeof(shell_cmd)-1;

            Newx(w32_perlshell_vec, 3, char *);
            Newx(w32_perlshell_tokens, len + 1 + sizeof(cmd_opts), char);

            my_strlcpy(w32_perlshell_tokens, shellbuf, len+1);
            my_strlcpy(w32_perlshell_tokens + len +1, cmd_opts,
                       sizeof(cmd_opts));

            w32_perlshell_vec[0] = w32_perlshell_tokens;
            w32_perlshell_vec[1] = w32_perlshell_tokens + len + 1;
            w32_perlshell_vec[2] = NULL;

            w32_perlshell_items = 2;
        }
    }
    return 0;
}

int
Perl_do_aspawn(pTHX_ SV *really, SV **mark, SV **sp)
{
    const char **argv;
    char *str;
    int status;
    int flag = P_WAIT;
    int index = 0;
    int eno;

    PERL_ARGS_ASSERT_DO_ASPAWN;

    if (sp <= mark)
        return -1;

    if (get_shell() < 0)
        return -1;

    Newx(argv, (sp - mark) + w32_perlshell_items + 2, const char*);

    if (SvNIOKp(*(mark+1)) && !SvPOKp(*(mark+1))) {
        ++mark;
        flag = SvIVx(*mark);
    }

    while (++mark <= sp) {
        if (*mark && (str = SvPV_nolen(*mark)))
            argv[index++] = str;
        else
            argv[index++] = "";
    }
    argv[index++] = 0;

    status = win32_spawnvp(flag,
                           (const char*)(really ? SvPV_nolen(really) : argv[0]),
                           (const char* const*)argv);

    if (status < 0 && (eno = errno, (eno == ENOEXEC || eno == ENOENT))) {
        /* possible shell-builtin, invoke with shell */
        int sh_items;
        sh_items = w32_perlshell_items;
        while (--index >= 0)
            argv[index+sh_items] = argv[index];
        while (--sh_items >= 0)
            argv[sh_items] = w32_perlshell_vec[sh_items];

        status = win32_spawnvp(flag,
                               (const char*)(really ? SvPV_nolen(really) : argv[0]),
                               (const char* const*)argv);
    }

    if (flag == P_NOWAIT) {
        PL_statusvalue = -1;	/* >16bits hint for pp_system() */
    }
    else {
        if (status < 0) {
            if (ckWARN(WARN_EXEC))
                Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't spawn \"%s\": %s", argv[0], strerror(errno));
            status = 255 * 256;
        }
        else
            status *= 256;
        PL_statusvalue = status;
    }
    Safefree(argv);
    return (status);
}

/* returns pointer to the next unquoted space or the end of the string */
static char*
find_next_space(const char *s)
{
    bool in_quotes = FALSE;
    while (*s) {
        /* ignore doubled backslashes, or backslash+quote */
        if (*s == '\\' && (s[1] == '\\' || s[1] == '"')) {
            s += 2;
        }
        /* keep track of when we're within quotes */
        else if (*s == '"') {
            s++;
            in_quotes = !in_quotes;
        }
        /* break it up only at spaces that aren't in quotes */
        else if (!in_quotes && isSPACE(*s))
            return (char*)s;
        else
            s++;
    }
    return (char*)s;
}

static int
do_spawn2(pTHX_ const char *cmd, int exectype) {
    return do_spawn2_handles(aTHX_ cmd, exectype, NULL);
}

static int
do_spawn2_handles(pTHX_ const char *cmd, int exectype, const int *handles)
{
    char **a;
    char *s;
    char **argv;
    int status = -1;
    BOOL needToTry = TRUE;
    char *cmd2;

    /* Save an extra exec if possible. See if there are shell
     * metacharacters in it */
    if (!has_shell_metachars(cmd)) {
        Newx(argv, strlen(cmd) / 2 + 2, char*);
        Newx(cmd2, strlen(cmd) + 1, char);
        strcpy(cmd2, cmd);
        a = argv;
        for (s = cmd2; *s;) {
            while (*s && isSPACE(*s))
                s++;
            if (*s)
                *(a++) = s;
            s = find_next_space(s);
            if (*s)
                *s++ = '\0';
        }
        *a = NULL;
        if (argv[0]) {
            switch (exectype) {
            case EXECF_SPAWN:
                status = win32_spawnvp(P_WAIT, argv[0],
                                       (const char* const*)argv);
                break;
            case EXECF_SPAWN_NOWAIT:
                status = do_spawnvp_handles(P_NOWAIT, argv[0],
                                            (const char* const*)argv, handles);
                break;
            case EXECF_EXEC:
                status = win32_execvp(argv[0], (const char* const*)argv);
                break;
            }
            if (status != -1 || errno == 0)
                needToTry = FALSE;
        }
        Safefree(argv);
        Safefree(cmd2);
    }
    if (needToTry) {
        char **argv;
        int i = -1;
        if (get_shell() < 0)
            return -1;
        Newx(argv, w32_perlshell_items + 2, char*);
        while (++i < w32_perlshell_items)
            argv[i] = w32_perlshell_vec[i];
        argv[i++] = (char *)cmd;
        argv[i] = NULL;
        switch (exectype) {
        case EXECF_SPAWN:
            status = win32_spawnvp(P_WAIT, argv[0],
                                   (const char* const*)argv);
            break;
        case EXECF_SPAWN_NOWAIT:
            status = do_spawnvp_handles(P_NOWAIT, argv[0],
                                        (const char* const*)argv, handles);
            break;
        case EXECF_EXEC:
            status = win32_execvp(argv[0], (const char* const*)argv);
            break;
        }
        cmd = argv[0];
        Safefree(argv);
    }
    if (exectype == EXECF_SPAWN_NOWAIT) {
        PL_statusvalue = -1;	/* >16bits hint for pp_system() */
    }
    else {
        if (status < 0) {
            if (ckWARN(WARN_EXEC))
                Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't %s \"%s\": %s",
                     (exectype == EXECF_EXEC ? "exec" : "spawn"),
                     cmd, strerror(errno));
            status = 255 * 256;
        }
        else
            status *= 256;
        PL_statusvalue = status;
    }
    return (status);
}

int
Perl_do_spawn(pTHX_ char *cmd)
{
    PERL_ARGS_ASSERT_DO_SPAWN;

    return do_spawn2(aTHX_ cmd, EXECF_SPAWN);
}

int
Perl_do_spawn_nowait(pTHX_ char *cmd)
{
    PERL_ARGS_ASSERT_DO_SPAWN_NOWAIT;

    return do_spawn2(aTHX_ cmd, EXECF_SPAWN_NOWAIT);
}

bool
Perl_do_exec(pTHX_ const char *cmd)
{
    PERL_ARGS_ASSERT_DO_EXEC;

    do_spawn2(aTHX_ cmd, EXECF_EXEC);
    return FALSE;
}

/* The idea here is to read all the directory names into a string table
 * (separated by nulls) and when one of the other dir functions is called
 * return the pointer to the current file name.
 */
DllExport DIR *
win32_opendir(const char *filename)
{
    dTHXa(NULL);
    DIR			*dirp;
    long		len;
    long		idx;
    char		scanname[MAX_PATH+3];
    WCHAR		wscanname[sizeof(scanname)];
    WIN32_FIND_DATAW	wFindData;
    char		buffer[MAX_PATH*2];
    BOOL		use_default;

    len = strlen(filename);
    if (len == 0) {
        errno = ENOENT;
        return NULL;
    }
    if (len > MAX_PATH) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    /* Get us a DIR structure */
    Newxz(dirp, 1, DIR);

    /* Create the search pattern */
    strcpy(scanname, filename);

    /* bare drive name means look in cwd for drive */
    if (len == 2 && isALPHA(scanname[0]) && scanname[1] == ':') {
        scanname[len++] = '.';
        scanname[len++] = '/';
    }
    else if (scanname[len-1] != '/' && scanname[len-1] != '\\') {
        scanname[len++] = '/';
    }
    scanname[len++] = '*';
    scanname[len] = '\0';

    /* do the FindFirstFile call */
    MultiByteToWideChar(CP_ACP, 0, scanname, -1, wscanname, sizeof(wscanname)/sizeof(WCHAR));
    aTHXa(PERL_GET_THX);
    dirp->handle = FindFirstFileW(PerlDir_mapW(wscanname), &wFindData);

    if (dirp->handle == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        /* FindFirstFile() fails on empty drives! */
        switch (err) {
        case ERROR_FILE_NOT_FOUND:
            return dirp;
        case ERROR_NO_MORE_FILES:
        case ERROR_PATH_NOT_FOUND:
            errno = ENOENT;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            errno = ENOMEM;
            break;
        default:
            errno = EINVAL;
            break;
        }
        Safefree(dirp);
        return NULL;
    }

    use_default = FALSE;
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                        wFindData.cFileName, -1,
                        buffer, sizeof(buffer), NULL, &use_default);
    if (use_default && *wFindData.cAlternateFileName) {
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                            wFindData.cAlternateFileName, -1,
                            buffer, sizeof(buffer), NULL, NULL);
    }

    /* now allocate the first part of the string table for
     * the filenames that we find.
     */
    idx = strlen(buffer)+1;
    if (idx < 256)
        dirp->size = 256;
    else
        dirp->size = idx;
    Newx(dirp->start, dirp->size, char);
    strcpy(dirp->start, buffer);
    dirp->nfiles++;
    dirp->end = dirp->curr = dirp->start;
    dirp->end += idx;
    return dirp;
}


/* Readdir just returns the current string pointer and bumps the
 * string pointer to the nDllExport entry.
 */
DllExport struct direct *
win32_readdir(DIR *dirp)
{
    long         len;

    if (dirp->curr) {
        /* first set up the structure to return */
        len = strlen(dirp->curr);
        strcpy(dirp->dirstr.d_name, dirp->curr);
        dirp->dirstr.d_namlen = len;

        /* Fake an inode */
        dirp->dirstr.d_ino = dirp->curr - dirp->start;

        /* Now set up for the next call to readdir */
        dirp->curr += len + 1;
        if (dirp->curr >= dirp->end) {
            BOOL res;
            char buffer[MAX_PATH*2];

            if (dirp->handle == INVALID_HANDLE_VALUE) {
                res = 0;
            }
            /* finding the next file that matches the wildcard
             * (which should be all of them in this directory!).
             */
            else {
                WIN32_FIND_DATAW wFindData;
                res = FindNextFileW(dirp->handle, &wFindData);
                if (res) {
                    BOOL use_default = FALSE;
                    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                                        wFindData.cFileName, -1,
                                        buffer, sizeof(buffer), NULL, &use_default);
                    if (use_default && *wFindData.cAlternateFileName) {
                        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                                            wFindData.cAlternateFileName, -1,
                                            buffer, sizeof(buffer), NULL, NULL);
                    }
                }
            }
            if (res) {
                long endpos = dirp->end - dirp->start;
                long newsize = endpos + strlen(buffer) + 1;
                /* bump the string table size by enough for the
                 * new name and its null terminator */
                while (newsize > dirp->size) {
                    long curpos = dirp->curr - dirp->start;
                    Renew(dirp->start, dirp->size * 2, char);
                    dirp->size *= 2;
                    dirp->curr = dirp->start + curpos;
                }
                strcpy(dirp->start + endpos, buffer);
                dirp->end = dirp->start + newsize;
                dirp->nfiles++;
            }
            else {
                dirp->curr = NULL;
                if (dirp->handle != INVALID_HANDLE_VALUE) {
                    FindClose(dirp->handle);
                    dirp->handle = INVALID_HANDLE_VALUE;
                }
            }
        }
        return &(dirp->dirstr);
    }
    else
        return NULL;
}

/* Telldir returns the current string pointer position */
DllExport long
win32_telldir(DIR *dirp)
{
    return dirp->curr ? (dirp->curr - dirp->start) : -1;
}


/* Seekdir moves the string pointer to a previously saved position
 * (returned by telldir).
 */
DllExport void
win32_seekdir(DIR *dirp, long loc)
{
    /* Ensure dirp->curr remains within `dirp->start` buffer. */
    if (loc >= 0 && dirp->end - dirp->start > (ptrdiff_t) loc) {
        dirp->curr = dirp->start + loc;
    } else {
        dirp->curr = NULL;
    }
}

/* Rewinddir resets the string pointer to the start */
DllExport void
win32_rewinddir(DIR *dirp)
{
    dirp->curr = dirp->start;
}

/* free the memory allocated by opendir */
DllExport int
win32_closedir(DIR *dirp)
{
    if (dirp->handle != INVALID_HANDLE_VALUE)
        FindClose(dirp->handle);
    Safefree(dirp->start);
    Safefree(dirp);
    return 1;
}

/* duplicate a open DIR* for interpreter cloning */
DllExport DIR *
win32_dirp_dup(DIR *const dirp, CLONE_PARAMS *const param)
{
    PerlInterpreter *const from = param->proto_perl;
    PerlInterpreter *const to   = (PerlInterpreter *)PERL_GET_THX;

    long pos;
    DIR *dup;

    /* switch back to original interpreter because win32_readdir()
     * might Renew(dirp->start).
     */
    if (from != to) {
        PERL_SET_THX(from);
    }

    /* mark current position; read all remaining entries into the
     * cache, and then restore to current position.
     */
    pos = win32_telldir(dirp);
    while (win32_readdir(dirp)) {
        /* read all entries into cache */
    }
    win32_seekdir(dirp, pos);

    /* switch back to new interpreter to allocate new DIR structure */
    if (from != to) {
        PERL_SET_THX(to);
    }

    Newx(dup, 1, DIR);
    memcpy(dup, dirp, sizeof(DIR));

    Newx(dup->start, dirp->size, char);
    memcpy(dup->start, dirp->start, dirp->size);

    dup->end = dup->start + (dirp->end - dirp->start);
    if (dirp->curr)
        dup->curr = dup->start + (dirp->curr - dirp->start);

    return dup;
}

/*
 * various stubs
 */


/* Ownership
 *
 * Just pretend that everyone is a superuser. NT will let us know if
 * we don\'t really have permission to do something.
 */

#define ROOT_UID    ((uid_t)0)
#define ROOT_GID    ((gid_t)0)

uid_t
getuid(void)
{
    return ROOT_UID;
}

uid_t
geteuid(void)
{
    return ROOT_UID;
}

gid_t
getgid(void)
{
    return ROOT_GID;
}

gid_t
getegid(void)
{
    return ROOT_GID;
}

int
setuid(uid_t auid)
{
    return (auid == ROOT_UID ? 0 : -1);
}

int
setgid(gid_t agid)
{
    return (agid == ROOT_GID ? 0 : -1);
}

EXTERN_C char *
getlogin(void)
{
    dTHX;
    char *buf = w32_getlogin_buffer;
    DWORD size = sizeof(w32_getlogin_buffer);
    if (GetUserName(buf,&size))
        return buf;
    return (char*)NULL;
}

int
chown(const char *path, uid_t owner, gid_t group)
{
    /* XXX noop */
    return 0;
}

/*
 * XXX this needs strengthening  (for PerlIO)
 *   -- BKS, 11-11-200
*/
#if((!defined(__MINGW64_VERSION_MAJOR) || __MINGW64_VERSION_MAJOR < 4) && \
    (!defined(__MINGW32_MAJOR_VERSION) || __MINGW32_MAJOR_VERSION < 3 || \
     (__MINGW32_MAJOR_VERSION == 3 && __MINGW32_MINOR_VERSION < 21)))
int mkstemp(const char *path)
{
    dTHX;
    char buf[MAX_PATH+1];
    int i = 0, fd = -1;

retry:
    if (i++ > 10) { /* give up */
        errno = ENOENT;
        return -1;
    }
    if (!GetTempFileNameA((LPCSTR)path, "plr", 1, buf)) {
        errno = ENOENT;
        return -1;
    }
    fd = PerlLIO_open3(buf, O_CREAT|O_RDWR|O_EXCL, 0600);
    if (fd == -1)
        goto retry;
    return fd;
}
#endif

static long
find_pid(pTHX_ int pid)
{
    long child = w32_num_children;
    while (--child >= 0) {
        if ((int)w32_child_pids[child] == pid)
            return child;
    }
    return -1;
}

static void
remove_dead_process(long child)
{
    if (child >= 0) {
        dTHX;
        CloseHandle(w32_child_handles[child]);
        Move(&w32_child_handles[child+1], &w32_child_handles[child],
             (w32_num_children-child-1), HANDLE);
        Move(&w32_child_pids[child+1], &w32_child_pids[child],
             (w32_num_children-child-1), DWORD);
        w32_num_children--;
    }
}

#ifdef USE_ITHREADS
static long
find_pseudo_pid(pTHX_ int pid)
{
    long child = w32_num_pseudo_children;
    while (--child >= 0) {
        if ((int)w32_pseudo_child_pids[child] == pid)
            return child;
    }
    return -1;
}

static void
remove_dead_pseudo_process(long child)
{
    if (child >= 0) {
        dTHX;
        CloseHandle(w32_pseudo_child_handles[child]);
        Move(&w32_pseudo_child_handles[child+1], &w32_pseudo_child_handles[child],
             (w32_num_pseudo_children-child-1), HANDLE);
        Move(&w32_pseudo_child_pids[child+1], &w32_pseudo_child_pids[child],
             (w32_num_pseudo_children-child-1), DWORD);
        Move(&w32_pseudo_child_message_hwnds[child+1], &w32_pseudo_child_message_hwnds[child],
             (w32_num_pseudo_children-child-1), HWND);
        Move(&w32_pseudo_child_sigterm[child+1], &w32_pseudo_child_sigterm[child],
             (w32_num_pseudo_children-child-1), char);
        w32_num_pseudo_children--;
    }
}

void
win32_wait_for_children(pTHX)
{
    if (w32_pseudo_children && w32_num_pseudo_children) {
        long child = 0;
        long count = 0;
        HANDLE handles[MAXIMUM_WAIT_OBJECTS];

        for (child = 0; child < w32_num_pseudo_children; ++child) {
            if (!w32_pseudo_child_sigterm[child])
                handles[count++] = w32_pseudo_child_handles[child];
        }
        /* XXX should use MsgWaitForMultipleObjects() to continue
         * XXX processing messages while we wait.
         */
        WaitForMultipleObjects(count, handles, TRUE, INFINITE);

        while (w32_num_pseudo_children)
            CloseHandle(w32_pseudo_child_handles[--w32_num_pseudo_children]);
    }
}
#endif

static int
terminate_process(DWORD pid, HANDLE process_handle, int sig)
{
    switch(sig) {
    case 0:
        /* "Does process exist?" use of kill */
        return 1;
    case 2:
        if (GenerateConsoleCtrlEvent(CTRL_C_EVENT, pid))
            return 1;
        break;
    case SIGBREAK:
    case SIGTERM:
        if (GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid))
            return 1;
        break;
    default: /* For now be backwards compatible with perl 5.6 */
    case 9:
        /* Note that we will only be able to kill processes owned by the
         * current process owner, even when we are running as an administrator.
         * To kill processes of other owners we would need to set the
         * 'SeDebugPrivilege' privilege before obtaining the process handle.
         */
        if (TerminateProcess(process_handle, sig))
            return 1;
        break;
    }
    return 0;
}

/* returns number of processes killed */
static int
my_killpg(int pid, int sig)
{
    HANDLE process_handle;
    HANDLE snapshot_handle;
    int killed = 0;

    process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (process_handle == NULL)
        return 0;

    killed += terminate_process(pid, process_handle, sig);

    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot_handle != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry;

        entry.dwSize = sizeof(entry);
        if (Process32First(snapshot_handle, &entry)) {
            do {
                if (entry.th32ParentProcessID == (DWORD)pid)
                    killed += my_killpg(entry.th32ProcessID, sig);
                entry.dwSize = sizeof(entry);
            }
            while (Process32Next(snapshot_handle, &entry));
        }
        CloseHandle(snapshot_handle);
    }
    CloseHandle(process_handle);
    return killed;
}

/* returns number of processes killed */
static int
my_kill(int pid, int sig)
{
    int retval = 0;
    HANDLE process_handle;

    if (sig < 0)
        return my_killpg(pid, -sig);

    process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    /* OpenProcess() returns NULL on error, *not* INVALID_HANDLE_VALUE */
    if (process_handle != NULL) {
        retval = terminate_process(pid, process_handle, sig);
        CloseHandle(process_handle);
    }
    return retval;
}

#ifdef USE_ITHREADS
/* Get a child pseudo-process HWND, with retrying and delaying/yielding.
 * The "tries" parameter is the number of retries to make, with a Sleep(1)
 * (waiting and yielding the time slot) between each try. Specifying 0 causes
 * only Sleep(0) (no waiting and potentially no yielding) to be used, so is not
 * recommended
 * Returns an hwnd != INVALID_HANDLE_VALUE (so be aware that NULL can be
 * returned) or croaks if the child pseudo-process doesn't schedule and deliver
 * a HWND in the time period allowed.
 */
static HWND
get_hwnd_delay(pTHX, long child, DWORD tries)
{
    HWND hwnd = w32_pseudo_child_message_hwnds[child];
    if (hwnd != INVALID_HANDLE_VALUE) return hwnd;

    /* Pseudo-process has not yet properly initialized since hwnd isn't set.
     * Fast sleep: On some NT kernels/systems, a Sleep(0) won't deschedule a
     * thread 100% of the time since threads are attached to a CPU for NUMA and
     * caching reasons, and the child thread was attached to a different CPU
     * therefore there is no workload on that CPU and Sleep(0) returns control
     * without yielding the time slot.
     * https://github.com/Perl/perl5/issues/11267
     */
    Sleep(0);
    win32_async_check(aTHX);
    hwnd = w32_pseudo_child_message_hwnds[child];
    if (hwnd != INVALID_HANDLE_VALUE) return hwnd;

    {
        unsigned int count = 0;
        /* No Sleep(1) if tries==0, just fail instead if we get this far. */
        while (count++ < tries) {
            Sleep(1);
            win32_async_check(aTHX);
            hwnd = w32_pseudo_child_message_hwnds[child];
            if (hwnd != INVALID_HANDLE_VALUE) return hwnd;
        }
    }

    Perl_croak(aTHX_ "panic: child pseudo-process was never scheduled");
}
#endif

DllExport int
win32_kill(int pid, int sig)
{
    dTHX;
    long child;
#ifdef USE_ITHREADS
    if (pid < 0) {
        /* it is a pseudo-forked child */
        child = find_pseudo_pid(aTHX_ -pid);
        if (child >= 0) {
            HANDLE hProcess = w32_pseudo_child_handles[child];
            switch (sig) {
                case 0:
                    /* "Does process exist?" use of kill */
                    return 0;

                case 9: {
                    /* kill -9 style un-graceful exit */
                    /* Do a wait to make sure child starts and isn't in DLL
                     * Loader Lock */
                    HWND hwnd = get_hwnd_delay(aTHX, child, 5);
                    if (TerminateThread(hProcess, sig)) {
                        /* Allow the scheduler to finish cleaning up the other
                         * thread.
                         * Otherwise, if we ExitProcess() before another context
                         * switch happens we will end up with a process exit
                         * code of "sig" instead of our own exit status.
                         * https://rt.cpan.org/Ticket/Display.html?id=66016#txn-908976
                         */
                        Sleep(0);
                        remove_dead_pseudo_process(child);
                        return 0;
                    }
                    break;
                }

                default: {
                    HWND hwnd = get_hwnd_delay(aTHX, child, 5);
                    /* We fake signals to pseudo-processes using Win32
                     * message queue. */
                    if ((hwnd != NULL && PostMessage(hwnd, WM_USER_KILL, sig, 0)) ||
                        PostThreadMessage(-pid, WM_USER_KILL, sig, 0))
                    {
                        /* Don't wait for child process to terminate after we send a
                         * SIGTERM because the child may be blocked in a system call
                         * and never receive the signal.
                         */
                        if (sig == SIGTERM) {
                            Sleep(0);
                            w32_pseudo_child_sigterm[child] = 1;
                        }
                        /* It might be us ... */
                        PERL_ASYNC_CHECK();
                        return 0;
                    }
                    break;
                }
            } /* switch */
        }
    }
    else
#endif
    {
        child = find_pid(aTHX_ pid);
        if (child >= 0) {
            if (my_kill(pid, sig)) {
                DWORD exitcode = 0;
                if (GetExitCodeProcess(w32_child_handles[child], &exitcode) &&
                    exitcode != STILL_ACTIVE)
                {
                    remove_dead_process(child);
                }
                return 0;
            }
        }
        else {
            if (my_kill(pid, sig))
                return 0;
        }
    }
    errno = EINVAL;
    return -1;
}

PERL_STATIC_INLINE
time_t
translate_ft_to_time_t(FILETIME ft) {
    SYSTEMTIME st;
    struct tm pt;
    time_t retval;
    dTHX;

    if (!FileTimeToSystemTime(&ft, &st))
        return -1;

    Zero(&pt, 1, struct tm);
    pt.tm_year = st.wYear - 1900;
    pt.tm_mon = st.wMonth - 1;
    pt.tm_mday = st.wDay;
    pt.tm_hour = st.wHour;
    pt.tm_min = st.wMinute;
    pt.tm_sec = st.wSecond;

    MKTIME_LOCK;
    retval = _mkgmtime(&pt);
    MKTIME_UNLOCK;

    return retval;
}

typedef DWORD (__stdcall *pGetFinalPathNameByHandleA_t)(HANDLE, LPSTR, DWORD, DWORD);

/* Adapted from:

https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer

Renamed to avoid conflicts, apparently some SDKs define this
structure.

Hoisted the symlink and mount point data into a new type to allow us
to make a pointer to it, and to avoid C++ scoping issues.

*/

typedef struct {
    USHORT SubstituteNameOffset;
    USHORT SubstituteNameLength;
    USHORT PrintNameOffset;
    USHORT PrintNameLength;
    ULONG  Flags;
    WCHAR  PathBuffer[MAX_PATH*3];
} MY_SYMLINK_REPARSE_BUFFER, *PMY_SYMLINK_REPARSE_BUFFER;

typedef struct {
    USHORT SubstituteNameOffset;
    USHORT SubstituteNameLength;
    USHORT PrintNameOffset;
    USHORT PrintNameLength;
    WCHAR  PathBuffer[MAX_PATH*3];
} MY_MOUNT_POINT_REPARSE_BUFFER;

typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  union {
    MY_SYMLINK_REPARSE_BUFFER SymbolicLinkReparseBuffer;
    MY_MOUNT_POINT_REPARSE_BUFFER MountPointReparseBuffer;
    struct {
      UCHAR DataBuffer[1];
    } GenericReparseBuffer;
  } Data;
} MY_REPARSE_DATA_BUFFER, *PMY_REPARSE_DATA_BUFFER;

#ifndef IO_REPARSE_TAG_SYMLINK
#  define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)
#endif
#ifndef IO_REPARSE_TAG_AF_UNIX
#  define IO_REPARSE_TAG_AF_UNIX 0x80000023
#endif
#ifndef IO_REPARSE_TAG_LX_FIFO
#  define IO_REPARSE_TAG_LX_FIFO 0x80000024
#endif
#ifndef IO_REPARSE_TAG_LX_CHR
#  define IO_REPARSE_TAG_LX_CHR  0x80000025
#endif
#ifndef IO_REPARSE_TAG_LX_BLK
#  define IO_REPARSE_TAG_LX_BLK  0x80000026
#endif

static int
win32_stat_low(HANDLE handle, const char *path, STRLEN len, Stat_t *sbuf,
               DWORD reparse_type) {
    DWORD type = GetFileType(handle);
    BY_HANDLE_FILE_INFORMATION bhi;

    Zero(sbuf, 1, Stat_t);

    if (reparse_type) {
        /* Lie to get to the right place */
        type = FILE_TYPE_DISK;
    }

    type &= ~FILE_TYPE_REMOTE;

    switch (type) {
    case FILE_TYPE_DISK:
        if (GetFileInformationByHandle(handle, &bhi)) {
            sbuf->st_dev = bhi.dwVolumeSerialNumber;
            sbuf->st_ino = bhi.nFileIndexHigh;
            sbuf->st_ino <<= 32;
            sbuf->st_ino |= bhi.nFileIndexLow;
            sbuf->st_nlink = bhi.nNumberOfLinks;
            sbuf->st_uid = 0;
            sbuf->st_gid = 0;
            /* ucrt sets this to the drive letter for
               stat(), lets not reproduce that mistake */
            sbuf->st_rdev = 0;
            sbuf->st_size = bhi.nFileSizeHigh;
            sbuf->st_size <<= 32;
            sbuf->st_size |= bhi.nFileSizeLow;

            sbuf->st_atime = translate_ft_to_time_t(bhi.ftLastAccessTime);
            sbuf->st_mtime = translate_ft_to_time_t(bhi.ftLastWriteTime);
            sbuf->st_ctime = translate_ft_to_time_t(bhi.ftCreationTime);

            if (reparse_type) {
                /* https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/c8e77b37-3909-4fe6-a4ea-2b9d423b1ee4
                   describes all of these as WSL only, but the AF_UNIX tag
                   is known to be used for AF_UNIX sockets without WSL.
                */
                switch (reparse_type) {
                case IO_REPARSE_TAG_AF_UNIX:
                    sbuf->st_mode = _S_IFSOCK;
                    break;

                case IO_REPARSE_TAG_LX_FIFO:
                    sbuf->st_mode = _S_IFIFO;
                    break;

                case IO_REPARSE_TAG_LX_CHR:
                    sbuf->st_mode = _S_IFCHR;
                    break;

                case IO_REPARSE_TAG_LX_BLK:
                    sbuf->st_mode = _S_IFBLK;
                    break;

                default:
                    /* Is there anything else we can do here? */
                    errno = EINVAL;
                    return -1;
                }
            }
            else if (bhi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                sbuf->st_mode = _S_IFDIR | _S_IREAD | _S_IEXEC;
                /* duplicate the logic from the end of the old win32_stat() */
                if (!(bhi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
                    sbuf->st_mode |= S_IWRITE;
                }
            }
            else {
                char path_buf[MAX_PATH+1];
                sbuf->st_mode = _S_IFREG;

                if (!path) {
                    pGetFinalPathNameByHandleA_t pGetFinalPathNameByHandleA =
                        (pGetFinalPathNameByHandleA_t)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFinalPathNameByHandleA");
                    if (pGetFinalPathNameByHandleA) {
                        len = pGetFinalPathNameByHandleA(handle, path_buf, sizeof(path_buf), 0);
                    }
                    else {
                        len = 0;
                    }

                    /* < to ensure there's space for the \0 */
                    if (len && len < sizeof(path_buf)) {
                        path = path_buf;
                    }
                }

                if (path && len > 4 &&
                    (_stricmp(path + len - 4, ".exe") == 0 ||
                     _stricmp(path + len - 4, ".bat") == 0 ||
                     _stricmp(path + len - 4, ".cmd") == 0 ||
                     _stricmp(path + len - 4, ".com") == 0)) {
                    sbuf->st_mode |= _S_IEXEC;
                }
                if (!(bhi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
                    sbuf->st_mode |= _S_IWRITE;
                }
                sbuf->st_mode |= _S_IREAD;
            }
        }
        else {
            translate_to_errno();
            return -1;
        }
        break;

    case FILE_TYPE_CHAR:
    case FILE_TYPE_PIPE:
        sbuf->st_mode = (type == FILE_TYPE_CHAR) ? _S_IFCHR : _S_IFIFO;
        if (handle == GetStdHandle(STD_INPUT_HANDLE) ||
            handle == GetStdHandle(STD_OUTPUT_HANDLE) ||
            handle == GetStdHandle(STD_ERROR_HANDLE)) {
            sbuf->st_mode |= _S_IWRITE | _S_IREAD;
        }
        break;

    default:
        return -1;
    }

    /* owner == user == group */
    sbuf->st_mode |= (sbuf->st_mode & 0700) >> 3;
    sbuf->st_mode |= (sbuf->st_mode & 0700) >> 6;

    return 0;
}

/* https://docs.microsoft.com/en-us/windows/win32/fileio/reparse-points */
#define SYMLINK_FOLLOW_LIMIT 63

/*

Given a pathname, required to be a symlink, follow it until we find a
non-symlink path.

This should only be called when the symlink() chain doesn't lead to a
normal file, which should have been caught earlier.

On success, returns a HANDLE to the target and sets *reparse_type to
the ReparseTag of the target.

Returns INVALID_HANDLE_VALUE on error, which might be that the symlink
chain is broken, or requires too many links to resolve.

*/

static HANDLE
S_follow_symlinks_to(pTHX_ const char *pathname, DWORD *reparse_type) {
    char link_target[MAX_PATH];
    SV *work_path = newSVpvn(pathname, strlen(pathname));
    int link_count = 0;
    int link_len;
    HANDLE handle;

    *reparse_type = 0;

    while ((link_len = win32_readlink(SvPVX(work_path), link_target,
                                      sizeof(link_target))) > 0) {
        if (link_count++ >= SYMLINK_FOLLOW_LIMIT) {
            /* Windows doesn't appear to ever return ELOOP,
               let's do better ourselves
            */
            SvREFCNT_dec(work_path);
            errno = ELOOP;
            return INVALID_HANDLE_VALUE;
        }
        /* Adjust the linktarget based on the link source or current
           directory as needed.
        */
        if (link_target[0] == '\\'
            || link_target[0] == '/'
            || (link_len >=2 && link_target[1] == ':')) {
            /* link is absolute */
            sv_setpvn(work_path, link_target, link_len);
        }
        else {
            STRLEN work_len;
            const char *workp = SvPV(work_path, work_len);
            const char *final_bslash =
                (const char *)my_memrchr(workp, '\\', work_len);
            const char *final_slash =
                (const char *)my_memrchr(workp, '/', work_len);
            const char *path_sep = NULL;
            if (final_bslash && final_slash)
                path_sep = final_bslash > final_slash ? final_bslash : final_slash;
            else if (final_bslash)
                path_sep = final_bslash;
            else if (final_slash)
                path_sep = final_slash;

            if (path_sep) {
                SV *new_path = newSVpv(workp, path_sep - workp + 1);
                sv_catpvn(new_path, link_target, link_len);
                SvREFCNT_dec(work_path);
                work_path = new_path;
            }
            else {
                /* should only get here the first time around */
                assert(link_count == 1);
                char path_temp[MAX_PATH];
                DWORD path_len = GetCurrentDirectoryA(sizeof(path_temp), path_temp);
                if (!path_len || path_len > sizeof(path_temp)) {
                    SvREFCNT_dec(work_path);
                    errno = EINVAL;
                    return INVALID_HANDLE_VALUE;
                }

                SV *new_path = newSVpvn(path_temp, path_len);
                if (path_temp[path_len-1] != '\\') {
                    sv_catpvs(new_path, "\\");
                }
                sv_catpvn(new_path, link_target, link_len);
                SvREFCNT_dec(work_path);
                work_path = new_path;
            }
        }
    }

    handle =
        CreateFileA(SvPVX(work_path), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                    FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, 0);
    SvREFCNT_dec(work_path);
    if (handle != INVALID_HANDLE_VALUE) {
        MY_REPARSE_DATA_BUFFER linkdata;
        DWORD linkdata_returned;

        if (!DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, NULL, 0,
                             &linkdata, sizeof(linkdata),
                             &linkdata_returned, NULL)) {
            translate_to_errno();
            CloseHandle(handle);
            return INVALID_HANDLE_VALUE;
        }
        *reparse_type = linkdata.ReparseTag;
        return handle;
    }
    else {
        translate_to_errno();
    }

    return handle;
}

DllExport int
win32_stat(const char *path, Stat_t *sbuf)
{
    dTHX;
    BOOL        expect_dir = FALSE;
    int result;
    HANDLE handle;
    DWORD reparse_type = 0;

    path = PerlDir_mapA(path);

    handle =
        CreateFileA(path, FILE_READ_ATTRIBUTES,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        /* AF_UNIX sockets need to be opened as a reparse point, but
           that will also open symlinks rather than following them.

           There may be other reparse points that need similar
           treatment.
        */
        handle = S_follow_symlinks_to(aTHX_ path, &reparse_type);
        if (handle == INVALID_HANDLE_VALUE) {
            /* S_follow_symlinks_to() will set errno */
            return -1;
        }
    }
    if (handle != INVALID_HANDLE_VALUE) {
        result = win32_stat_low(handle, path, strlen(path), sbuf, reparse_type);
        CloseHandle(handle);
    }
    else {
        translate_to_errno();
        result = -1;
    }

    return result;
}

static void
translate_to_errno(void)
{
    /* This isn't perfect, eg. Win32 returns ERROR_ACCESS_DENIED for
       both permissions errors and if the source is a directory, while
       POSIX wants EACCES and EPERM respectively.
    */
    switch (GetLastError()) {
    case ERROR_BAD_NET_NAME:
    case ERROR_BAD_NETPATH:
    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_FILENAME_EXCED_RANGE:
    case ERROR_INVALID_DRIVE:
    case ERROR_PATH_NOT_FOUND:
      errno = ENOENT;
      break;
    case ERROR_ALREADY_EXISTS:
      errno = EEXIST;
      break;
    case ERROR_ACCESS_DENIED:
      errno = EACCES;
      break;
    case ERROR_PRIVILEGE_NOT_HELD:
      errno = EPERM;
      break;
    case ERROR_NOT_SAME_DEVICE:
      errno = EXDEV;
      break;
    case ERROR_DISK_FULL:
      errno = ENOSPC;
      break;
    case ERROR_NOT_ENOUGH_QUOTA:
      errno = EDQUOT;
      break;
    default:
      /* ERROR_INVALID_FUNCTION - eg. symlink on a FAT volume */
      errno = EINVAL;
      break;
    }
}

static BOOL
is_symlink(HANDLE h) {
    MY_REPARSE_DATA_BUFFER linkdata;
    const MY_SYMLINK_REPARSE_BUFFER * const sd =
        &linkdata.Data.SymbolicLinkReparseBuffer;
    DWORD linkdata_returned;

    if (!DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, &linkdata, sizeof(linkdata), &linkdata_returned, NULL)) {
        return FALSE;
    }

    if (linkdata_returned < offsetof(MY_REPARSE_DATA_BUFFER, Data.SymbolicLinkReparseBuffer.PathBuffer)
        || (linkdata.ReparseTag != IO_REPARSE_TAG_SYMLINK
            && linkdata.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)) {
        /* some other type of reparse point */
        return FALSE;
    }

    return TRUE;
}

static BOOL
is_symlink_name(const char *name) {
    HANDLE f = CreateFileA(name, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                           FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, 0);
    BOOL result;

    if (f == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    result = is_symlink(f);
    CloseHandle(f);

    return result;
}

static int
do_readlink_handle(HANDLE hlink, char *buf, size_t bufsiz, bool *is_symlink) {
    MY_REPARSE_DATA_BUFFER linkdata;
    DWORD linkdata_returned;

    if (is_symlink)
        *is_symlink = FALSE;

    if (!DeviceIoControl(hlink, FSCTL_GET_REPARSE_POINT, NULL, 0, &linkdata, sizeof(linkdata), &linkdata_returned, NULL)) {
        translate_to_errno();
        return -1;
    }

    int bytes_out;
    BOOL used_default;
    switch (linkdata.ReparseTag) {
    case IO_REPARSE_TAG_SYMLINK:
        {
            const MY_SYMLINK_REPARSE_BUFFER * const sd =
                &linkdata.Data.SymbolicLinkReparseBuffer;
            if (linkdata_returned < offsetof(MY_REPARSE_DATA_BUFFER, Data.SymbolicLinkReparseBuffer.PathBuffer)) {
                errno = EINVAL;
                return -1;
            }
            bytes_out =
                WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                                    sd->PathBuffer + sd->PrintNameOffset/2,
                                    sd->PrintNameLength/2,
                                    buf, (int)bufsiz, NULL, &used_default);
            if (is_symlink)
                *is_symlink = TRUE;
        }
        break;
    case IO_REPARSE_TAG_MOUNT_POINT:
        {
            const MY_MOUNT_POINT_REPARSE_BUFFER * const rd =
                &linkdata.Data.MountPointReparseBuffer;
            if (linkdata_returned < offsetof(MY_REPARSE_DATA_BUFFER, Data.MountPointReparseBuffer.PathBuffer)) {
                errno = EINVAL;
                return -1;
            }
            bytes_out =
                WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                                    rd->PathBuffer + rd->PrintNameOffset/2,
                                    rd->PrintNameLength/2,
                                    buf, (int)bufsiz, NULL, &used_default);
            if (is_symlink)
                *is_symlink = TRUE;
        }
        break;

    default:
        errno = EINVAL;
        return -1;
    }

    if (bytes_out == 0 || used_default) {
        /* failed conversion from unicode to ANSI or otherwise failed */
        errno = EINVAL;
        return -1;
    }

    return bytes_out;
}

DllExport int
win32_readlink(const char *pathname, char *buf, size_t bufsiz) {
    if (pathname == NULL || buf == NULL) {
        errno = EFAULT;
        return -1;
    }
    if (bufsiz <= 0) {
        errno = EINVAL;
        return -1;
    }

    DWORD fileattr = GetFileAttributes(pathname);
    if (fileattr == INVALID_FILE_ATTRIBUTES) {
        translate_to_errno();
        return -1;
    }

    if (!(fileattr & FILE_ATTRIBUTE_REPARSE_POINT)) {
        /* not a symbolic link */
        errno = EINVAL;
        return -1;
    }

    HANDLE hlink =
        CreateFileA(pathname, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                    FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (hlink == INVALID_HANDLE_VALUE) {
        translate_to_errno();
        return -1;
    }
    int bytes_out = do_readlink_handle(hlink, buf, bufsiz, NULL);
    CloseHandle(hlink);
    if (bytes_out < 0) {
        /* errno already set */
        return -1;
    }

    if ((size_t)bytes_out > bufsiz) {
        errno = EINVAL;
        return -1;
    }

    return bytes_out;
}

DllExport int
win32_lstat(const char *path, Stat_t *sbuf)
{
    HANDLE f;
    int result;
    DWORD attr = GetFileAttributes(path); /* doesn't follow symlinks */

    if (attr == INVALID_FILE_ATTRIBUTES) {
        translate_to_errno();
        return -1;
    }

    if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
        return win32_stat(path, sbuf);
    }

    f = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                           FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (f == INVALID_HANDLE_VALUE) {
        translate_to_errno();
        return -1;
    }
    bool is_symlink;
    int size = do_readlink_handle(f, NULL, 0, &is_symlink);
    if (!is_symlink) {
        /* it isn't a symlink, fallback to normal stat */
        CloseHandle(f);
        return win32_stat(path, sbuf);
    }
    else if (size < 0) {
        /* some other error, errno already set */
        CloseHandle(f);
        return -1;
    }
    result = win32_stat_low(f, NULL, 0, sbuf, 0);

    if (result != -1){
        sbuf->st_mode = (sbuf->st_mode & ~_S_IFMT) | _S_IFLNK;
        sbuf->st_size = size;
    }
    CloseHandle(f);

    return result;
}

#define isSLASH(c) ((c) == '/' || (c) == '\\')
#define SKIP_SLASHES(s) \
    STMT_START {				\
        while (*(s) && isSLASH(*(s)))		\
            ++(s);				\
    } STMT_END
#define COPY_NONSLASHES(d,s) \
    STMT_START {				\
        while (*(s) && !isSLASH(*(s)))		\
            *(d)++ = *(s)++;			\
    } STMT_END

/* Find the longname of a given path.  path is destructively modified.
 * It should have space for at least MAX_PATH characters. */
DllExport char *
win32_longpath(char *path)
{
    WIN32_FIND_DATA fdata;
    HANDLE fhand;
    char tmpbuf[MAX_PATH+1];
    char *tmpstart = tmpbuf;
    char *start = path;
    char sep;
    if (!path)
        return NULL;

    /* drive prefix */
    if (isALPHA(path[0]) && path[1] == ':') {
        start = path + 2;
        *tmpstart++ = path[0];
        *tmpstart++ = ':';
    }
    /* UNC prefix */
    else if (isSLASH(path[0]) && isSLASH(path[1])) {
        start = path + 2;
        *tmpstart++ = path[0];
        *tmpstart++ = path[1];
        SKIP_SLASHES(start);
        COPY_NONSLASHES(tmpstart,start);	/* copy machine name */
        if (*start) {
            *tmpstart++ = *start++;
            SKIP_SLASHES(start);
            COPY_NONSLASHES(tmpstart,start);	/* copy share name */
        }
    }
    *tmpstart = '\0';
    while (*start) {
        /* copy initial slash, if any */
        if (isSLASH(*start)) {
            *tmpstart++ = *start++;
            *tmpstart = '\0';
            SKIP_SLASHES(start);
        }

        /* FindFirstFile() expands "." and "..", so we need to pass
         * those through unmolested */
        if (*start == '.'
            && (!start[1] || isSLASH(start[1])
                || (start[1] == '.' && (!start[2] || isSLASH(start[2])))))
        {
            COPY_NONSLASHES(tmpstart,start);	/* copy "." or ".." */
            *tmpstart = '\0';
            continue;
        }

        /* if this is the end, bust outta here */
        if (!*start)
            break;

        /* now we're at a non-slash; walk up to next slash */
        while (*start && !isSLASH(*start))
            ++start;

        /* stop and find full name of component */
        sep = *start;
        *start = '\0';
        fhand = FindFirstFile(path,&fdata);
        *start = sep;
        if (fhand != INVALID_HANDLE_VALUE) {
            STRLEN len = strlen(fdata.cFileName);
            if ((STRLEN)(tmpbuf + sizeof(tmpbuf) - tmpstart) > len) {
                strcpy(tmpstart, fdata.cFileName);
                tmpstart += len;
                FindClose(fhand);
            }
            else {
                FindClose(fhand);
                errno = ERANGE;
                return NULL;
            }
        }
        else {
            /* failed a step, just return without side effects */
            errno = EINVAL;
            return NULL;
        }
    }
    strcpy(path,tmpbuf);
    return path;
}

static void
out_of_memory(void)
{

    if (PL_curinterp)
        croak_no_mem();
    exit(1);
}

void
win32_croak_not_implemented(const char * fname)
{
    PERL_ARGS_ASSERT_WIN32_CROAK_NOT_IMPLEMENTED;

    Perl_croak_nocontext("%s not implemented!\n", fname);
}

/* Converts a wide character (UTF-16) string to the Windows ANSI code page,
 * potentially using the system's default replacement character for any
 * unrepresentable characters. The caller must free() the returned string. */
static char*
wstr_to_str(const wchar_t* wstr)
{
    BOOL used_default = FALSE;
    size_t wlen = wcslen(wstr) + 1;
    int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr, wlen,
                                   NULL, 0, NULL, NULL);
    char* str = (char*)malloc(len);
    if (!str)
        out_of_memory();
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr, wlen,
                        str, len, NULL, &used_default);
    return str;
}

/* The win32_ansipath() function takes a Unicode filename and converts it
 * into the current Windows codepage. If some characters cannot be mapped,
 * then it will convert the short name instead.
 *
 * The buffer to the ansi pathname must be freed with win32_free() when it
 * is no longer needed.
 *
 * The argument to win32_ansipath() must exist before this function is
 * called; otherwise there is no way to determine the short path name.
 *
 * Ideas for future refinement:
 * - Only convert those segments of the path that are not in the current
 *   codepage, but leave the other segments in their long form.
 * - If the resulting name is longer than MAX_PATH, start converting
 *   additional path segments into short names until the full name
 *   is shorter than MAX_PATH.  Shorten the filename part last!
 */
DllExport char *
win32_ansipath(const WCHAR *widename)
{
    char *name;
    BOOL use_default = FALSE;
    size_t widelen = wcslen(widename)+1;
    int len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, widename, widelen,
                                  NULL, 0, NULL, NULL);
    name = (char*)win32_malloc(len);
    if (!name)
        out_of_memory();

    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, widename, widelen,
                        name, len, NULL, &use_default);
    if (use_default) {
        DWORD shortlen = GetShortPathNameW(widename, NULL, 0);
        if (shortlen) {
            WCHAR *shortname = (WCHAR*)win32_malloc(shortlen*sizeof(WCHAR));
            if (!shortname)
                out_of_memory();
            shortlen = GetShortPathNameW(widename, shortname, shortlen)+1;

            len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shortname, shortlen,
                                      NULL, 0, NULL, NULL);
            name = (char*)win32_realloc(name, len);
            if (!name)
                out_of_memory();
            WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shortname, shortlen,
                                name, len, NULL, NULL);
            win32_free(shortname);
        }
    }
    return name;
}

/* the returned string must be freed with win32_freeenvironmentstrings which is
 * implemented as a macro
 * void win32_freeenvironmentstrings(void* block)
 */
DllExport char *
win32_getenvironmentstrings(void)
{
    LPWSTR lpWStr, lpWTmp;
    LPSTR lpStr, lpTmp;
    DWORD env_len, wenvstrings_len = 0, aenvstrings_len = 0;

    /* Get the process environment strings */
    lpWTmp = lpWStr = (LPWSTR) GetEnvironmentStringsW();
    for (wenvstrings_len = 1; *lpWTmp != '\0'; lpWTmp += env_len + 1) {
        env_len = wcslen(lpWTmp);
        /* calculate the size of the environment strings */
        wenvstrings_len += env_len + 1;
    }

    /* Get the number of bytes required to store the ACP encoded string */
    aenvstrings_len = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, 
                                          lpWStr, wenvstrings_len, NULL, 0, NULL, NULL);
    lpTmp = lpStr = (char *)win32_calloc(aenvstrings_len, sizeof(char));
    if(!lpTmp)
        out_of_memory();

    /* Convert the string from UTF-16 encoding to ACP encoding */
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, lpWStr, wenvstrings_len, lpStr, 
                        aenvstrings_len, NULL, NULL);

    FreeEnvironmentStringsW(lpWStr);

    return(lpStr);
}

DllExport char *
win32_getenv(const char *name)
{
    dTHX;
    DWORD needlen;
    SV *curitem = NULL;
    DWORD last_err;

    needlen = GetEnvironmentVariableA(name,NULL,0);
    if (needlen != 0) {
        curitem = sv_2mortal(newSVpvs(""));
        do {
            SvGROW(curitem, needlen+1);
            needlen = GetEnvironmentVariableA(name,SvPVX(curitem),
                                              needlen);
        } while (needlen >= SvLEN(curitem));
        SvCUR_set(curitem, needlen);
    }
    else {
        last_err = GetLastError();
        if (last_err == ERROR_NOT_ENOUGH_MEMORY) {
            /* It appears the variable is in the env, but the Win32 API
               doesn't have a canned way of getting it.  So we fall back to
               grabbing the whole env and pulling this value out if possible */
            char *envv = GetEnvironmentStrings();
            char *cur = envv;
            STRLEN len;
            while (*cur) {
                char *end = strchr(cur,'=');
                if (end && end != cur) {
                    *end = '\0';
                    if (strEQ(cur,name)) {
                        curitem = sv_2mortal(newSVpv(end+1,0));
                        *end = '=';
                        break;
                    }
                    *end = '=';
                    cur = end + strlen(end+1)+2;
                }
                else if ((len = strlen(cur)))
                    cur += len+1;
            }
            FreeEnvironmentStrings(envv);
        }
#ifndef WIN32_NO_REGISTRY
        else {
            /* last ditch: allow any environment variables that begin with 'PERL'
               to be obtained from the registry, if found there */
            if (strBEGINs(name, "PERL"))
                (void)get_regstr(name, &curitem);
        }
#endif
    }
    if (curitem && SvCUR(curitem))
        return SvPVX(curitem);

    return NULL;
}

DllExport int
win32_putenv(const char *name)
{
    char* curitem;
    char* val;
    int relval = -1;

    if (name) {
        curitem = (char *) win32_malloc(strlen(name)+1);
        strcpy(curitem, name);
        val = strchr(curitem, '=');
        if (val) {
            /* The sane way to deal with the environment.
             * Has these advantages over putenv() & co.:
             *  * enables us to store a truly empty value in the
             *    environment (like in UNIX).
             *  * we don't have to deal with RTL globals, bugs and leaks
             *    (specifically, see http://support.microsoft.com/kb/235601).
             *  * Much faster.
             * Why you may want to use the RTL environment handling
             * (previously enabled by USE_WIN32_RTL_ENV):
             *  * environ[] and RTL functions will not reflect changes,
             *    which might be an issue if extensions want to access
             *    the env. via RTL.  This cuts both ways, since RTL will
             *    not see changes made by extensions that call the Win32
             *    functions directly, either.
             * GSAR 97-06-07
             */
            *val++ = '\0';
            if (SetEnvironmentVariableA(curitem, *val ? val : NULL))
                relval = 0;
        }
        win32_free(curitem);
    }
    return relval;
}

static long
filetime_to_clock(PFILETIME ft)
{
    __int64 qw = ft->dwHighDateTime;
    qw <<= 32;
    qw |= ft->dwLowDateTime;
    qw /= 10000;  /* File time ticks at 0.1uS, clock at 1mS */
    return (long) qw;
}

DllExport int
win32_times(struct tms *timebuf)
{
    FILETIME user;
    FILETIME kernel;
    FILETIME dummy;
    clock_t process_time_so_far = clock();
    if (GetProcessTimes(GetCurrentProcess(), &dummy, &dummy,
                        &kernel,&user)) {
        timebuf->tms_utime = filetime_to_clock(&user);
        timebuf->tms_stime = filetime_to_clock(&kernel);
        timebuf->tms_cutime = 0;
        timebuf->tms_cstime = 0;
    } else {
        /* That failed - e.g. Win95 fallback to clock() */
        timebuf->tms_utime = process_time_so_far;
        timebuf->tms_stime = 0;
        timebuf->tms_cutime = 0;
        timebuf->tms_cstime = 0;
    }
    return process_time_so_far;
}

static BOOL
filetime_from_time(PFILETIME pFileTime, time_t Time)
{
    struct tm *pt;
    SYSTEMTIME st;
    dTHX;

    GMTIME_LOCK;
    pt = gmtime(&Time);
    if (!pt) {
        GMTIME_UNLOCK;
        pFileTime->dwLowDateTime = 0;
        pFileTime->dwHighDateTime = 0;
        return FALSE;
    }

    st.wYear = pt->tm_year + 1900;
    st.wMonth = pt->tm_mon + 1;
    st.wDay = pt->tm_mday;
    st.wHour = pt->tm_hour;
    st.wMinute = pt->tm_min;
    st.wSecond = pt->tm_sec;
    st.wMilliseconds = 0;

    GMTIME_UNLOCK;

    if (!SystemTimeToFileTime(&st, pFileTime)) {
        pFileTime->dwLowDateTime = 0;
        pFileTime->dwHighDateTime = 0;
        return FALSE;
    }

    return TRUE;
}

DllExport int
win32_unlink(const char *filename)
{
    dTHX;
    int ret;
    DWORD attrs;

    filename = PerlDir_mapA(filename);
    attrs = GetFileAttributesA(filename);
    if (attrs == 0xFFFFFFFF) {
        errno = ENOENT;
        return -1;
    }
    if (attrs & FILE_ATTRIBUTE_READONLY) {
        (void)SetFileAttributesA(filename, attrs & ~FILE_ATTRIBUTE_READONLY);
        ret = unlink(filename);
        if (ret == -1)
            (void)SetFileAttributesA(filename, attrs);
    }
    else if ((attrs & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY))
        == (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY)
             && is_symlink_name(filename)) {
        ret = rmdir(filename);
    }
    else {
        ret = unlink(filename);
    }
    return ret;
}

DllExport int
win32_utime(const char *filename, struct utimbuf *times)
{
    dTHX;
    HANDLE handle;
    FILETIME ftAccess;
    FILETIME ftWrite;
    struct utimbuf TimeBuffer;
    int rc = -1;

    filename = PerlDir_mapA(filename);
    /* This will (and should) still fail on readonly files */
    handle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                         OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        translate_to_errno();
        return -1;
    }

    if (times == NULL) {
        times = &TimeBuffer;
        time(&times->actime);
        times->modtime = times->actime;
    }

    if (filetime_from_time(&ftAccess, times->actime) &&
        filetime_from_time(&ftWrite, times->modtime)) {
        if (SetFileTime(handle, NULL, &ftAccess, &ftWrite)) {
            rc = 0;
        }
        else {
            translate_to_errno();
        }
    }
    else {
        errno = EINVAL; /* bad time? */
    }

    CloseHandle(handle);
    return rc;
}

typedef union {
    unsigned __int64	ft_i64;
    FILETIME		ft_val;
} FT_t;

#ifdef __GNUC__
#define Const64(x) x##LL
#else
#define Const64(x) x##i64
#endif
/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS  Const64(116444736000000000)

/* NOTE: This does not compute the timezone info (doing so can be expensive,
 * and appears to be unsupported even by glibc) */
DllExport int
win32_gettimeofday(struct timeval *tp, void *not_used)
{
    FT_t ft;

    /* this returns time in 100-nanosecond units  (i.e. tens of usecs) */
    GetSystemTimeAsFileTime(&ft.ft_val);

    /* seconds since epoch */
    tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / Const64(10000000));

    /* microseconds remaining */
    tp->tv_usec = (long)((ft.ft_i64 / Const64(10)) % Const64(1000000));

    return 0;
}

DllExport int
win32_uname(struct utsname *name)
{
    struct hostent *hep;
    STRLEN nodemax = sizeof(name->nodename)-1;

    /* sysname */
    switch (g_osver.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS:
        strcpy(name->sysname, "Windows");
        break;
    case VER_PLATFORM_WIN32_NT:
        strcpy(name->sysname, "Windows NT");
        break;
    case VER_PLATFORM_WIN32s:
        strcpy(name->sysname, "Win32s");
        break;
    default:
        strcpy(name->sysname, "Win32 Unknown");
        break;
    }

    /* release */
    sprintf(name->release, "%d.%d",
            g_osver.dwMajorVersion, g_osver.dwMinorVersion);

    /* version */
    sprintf(name->version, "Build %d",
            g_osver.dwPlatformId == VER_PLATFORM_WIN32_NT
            ? g_osver.dwBuildNumber : (g_osver.dwBuildNumber & 0xffff));
    if (g_osver.szCSDVersion[0]) {
        char *buf = name->version + strlen(name->version);
        sprintf(buf, " (%s)", g_osver.szCSDVersion);
    }

    /* nodename */
    hep = win32_gethostbyname("localhost");
    if (hep) {
        STRLEN len = strlen(hep->h_name);
        if (len <= nodemax) {
            strcpy(name->nodename, hep->h_name);
        }
        else {
            strncpy(name->nodename, hep->h_name, nodemax);
            name->nodename[nodemax] = '\0';
        }
    }
    else {
        DWORD sz = nodemax;
        if (!GetComputerName(name->nodename, &sz))
            *name->nodename = '\0';
    }

    /* machine (architecture) */
    {
        SYSTEM_INFO info;
        DWORD procarch;
        const char *arch;
        GetSystemInfo(&info);

#if (defined(__MINGW32__) && !defined(_ANONYMOUS_UNION) && !defined(__MINGW_EXTENSION))
        procarch = info.u.s.wProcessorArchitecture;
#else
        procarch = info.wProcessorArchitecture;
#endif
        switch (procarch) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            arch = "x86"; break;
        case PROCESSOR_ARCHITECTURE_IA64:
            arch = "ia64"; break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            arch = "amd64"; break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            arch = "unknown"; break;
        default:
            sprintf(name->machine, "unknown(0x%x)", procarch);
            arch = name->machine;
            break;
        }
        if (name->machine != arch)
            strcpy(name->machine, arch);
    }
    return 0;
}

/* Timing related stuff */

int
do_raise(pTHX_ int sig) 
{
    if (sig < SIG_SIZE) {
        Sighandler_t handler = w32_sighandler[sig];
        if (handler == SIG_IGN) {
            return 0;
        }
        else if (handler != SIG_DFL) {
            (*handler)(sig);
            return 0;
        }
        else {
            /* Choose correct default behaviour */
            switch (sig) {
#ifdef SIGCLD
                case SIGCLD:
#endif
#ifdef SIGCHLD
                case SIGCHLD:
#endif
                case 0:
                    return 0;
                case SIGTERM:
                default:
                    break;
            }
        }
    }
    /* Tell caller to exit thread/process as appropriate */
    return 1;
}

void
sig_terminate(pTHX_ int sig)
{
    Perl_warn(aTHX_ "Terminating on signal SIG%s(%d)\n",PL_sig_name[sig], sig);
    /* exit() seems to be safe, my_exit() or die() is a problem in ^C 
       thread 
     */
    exit(sig);
}

DllExport int
win32_async_check(pTHX)
{
    MSG msg;
    HWND hwnd = w32_message_hwnd;

    /* Reset w32_poll_count before doing anything else, in case we dispatch
     * messages that end up calling back into perl */
    w32_poll_count = 0;

    if (hwnd != INVALID_HANDLE_VALUE) {
        /* Passing PeekMessage -1 as HWND (2nd arg) only gets PostThreadMessage() messages
        * and ignores window messages - should co-exist better with windows apps e.g. Tk
        */
        if (hwnd == NULL)
            hwnd = (HWND)-1;

        while (PeekMessage(&msg, hwnd, WM_TIMER,    WM_TIMER,    PM_REMOVE|PM_NOYIELD) ||
               PeekMessage(&msg, hwnd, WM_USER_MIN, WM_USER_MAX, PM_REMOVE|PM_NOYIELD))
        {
            /* re-post a WM_QUIT message (we'll mark it as read later) */
            if(msg.message == WM_QUIT) {
                PostQuitMessage((int)msg.wParam);
                break;
            }

            if(!CallMsgFilter(&msg, MSGF_USER))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    /* Call PeekMessage() to mark all pending messages in the queue as "old".
     * This is necessary when we are being called by win32_msgwait() to
     * make sure MsgWaitForMultipleObjects() stops reporting the same waiting
     * message over and over.  An example how this can happen is when
     * Perl is calling win32_waitpid() inside a GUI application and the GUI
     * is generating messages before the process terminated.
     */
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE|PM_NOYIELD);

    /* Above or other stuff may have set a signal flag */
    if (PL_sig_pending)
        despatch_signals();
    
    return 1;
}

/* This function will not return until the timeout has elapsed, or until
 * one of the handles is ready. */
DllExport DWORD
win32_msgwait(pTHX_ DWORD count, LPHANDLE handles, DWORD timeout, LPDWORD resultp)
{
    /* We may need several goes at this - so compute when we stop */
    FT_t ticks = {0};
    unsigned __int64 endtime = timeout;
    if (timeout != INFINITE) {
        GetSystemTimeAsFileTime(&ticks.ft_val);
        ticks.ft_i64 /= 10000;
        endtime += ticks.ft_i64;
    }
    /* This was a race condition. Do not let a non INFINITE timeout to
     * MsgWaitForMultipleObjects roll under 0 creating a near
     * infinity/~(UINT32)0 timeout which will appear as a deadlock to the
     * user who did a CORE perl function with a non infinity timeout,
     * sleep for example.  This is 64 to 32 truncation minefield.
     *
     * This scenario can only be created if the timespan from the return of
     * MsgWaitForMultipleObjects to GetSystemTimeAsFileTime exceeds 1 ms. To
     * generate the scenario, manual breakpoints in a C debugger are required,
     * or a context switch occurred in win32_async_check in PeekMessage, or random
     * messages are delivered to the *thread* message queue of the Perl thread
     * from another process (msctf.dll doing IPC among its instances, VS debugger
     * causes msctf.dll to be loaded into Perl by kernel), see [perl #33096].
     */
    while (ticks.ft_i64 <= endtime) {
        /* if timeout's type is lengthened, remember to split 64b timeout
         * into multiple non-infinity runs of MWFMO */
        DWORD result = MsgWaitForMultipleObjects(count, handles, FALSE,
                                                (DWORD)(endtime - ticks.ft_i64),
                                                QS_POSTMESSAGE|QS_TIMER|QS_SENDMESSAGE);
        if (resultp)
           *resultp = result;
        if (result == WAIT_TIMEOUT) {
            /* Ran out of time - explicit return of zero to avoid -ve if we
               have scheduling issues
             */
            return 0;
        }
        if (timeout != INFINITE) {
            GetSystemTimeAsFileTime(&ticks.ft_val);
            ticks.ft_i64 /= 10000;
        }
        if (result == WAIT_OBJECT_0 + count) {
            /* Message has arrived - check it */
            (void)win32_async_check(aTHX);

            /* retry */
            if (ticks.ft_i64 > endtime)
                endtime = ticks.ft_i64;

            continue;
        }
        else {
           /* Not timeout or message - one of handles is ready */
           break;
        }
    }
    /* If we are past the end say zero */
    if (!ticks.ft_i64 || ticks.ft_i64 > endtime)
        return 0;
    /* compute time left to wait */
    ticks.ft_i64 = endtime - ticks.ft_i64;
    /* if more ms than DWORD, then return max DWORD */
    return ticks.ft_i64 <= UINT_MAX ? (DWORD)ticks.ft_i64 : UINT_MAX;
}

int
win32_internal_wait(pTHX_ int *status, DWORD timeout)
{
    /* XXX this wait emulation only knows about processes
     * spawned via win32_spawnvp(P_NOWAIT, ...).
     */
    int i, retval;
    DWORD exitcode, waitcode;

#ifdef USE_ITHREADS
    if (w32_num_pseudo_children) {
        win32_msgwait(aTHX_ w32_num_pseudo_children, w32_pseudo_child_handles,
                      timeout, &waitcode);
        /* Time out here if there are no other children to wait for. */
        if (waitcode == WAIT_TIMEOUT) {
            if (!w32_num_children) {
                return 0;
            }
        }
        else if (waitcode != WAIT_FAILED) {
            if (waitcode >= WAIT_ABANDONED_0
                && waitcode < WAIT_ABANDONED_0 + w32_num_pseudo_children)
                i = waitcode - WAIT_ABANDONED_0;
            else
                i = waitcode - WAIT_OBJECT_0;
            if (GetExitCodeThread(w32_pseudo_child_handles[i], &exitcode)) {
                *status = (int)(((U8) exitcode) << 8);
                retval = (int)w32_pseudo_child_pids[i];
                remove_dead_pseudo_process(i);
                return -retval;
            }
        }
    }
#endif

    if (!w32_num_children) {
        errno = ECHILD;
        return -1;
    }

    /* if a child exists, wait for it to die */
    win32_msgwait(aTHX_ w32_num_children, w32_child_handles, timeout, &waitcode);
    if (waitcode == WAIT_TIMEOUT) {
        return 0;
    }
    if (waitcode != WAIT_FAILED) {
        if (waitcode >= WAIT_ABANDONED_0
            && waitcode < WAIT_ABANDONED_0 + w32_num_children)
            i = waitcode - WAIT_ABANDONED_0;
        else
            i = waitcode - WAIT_OBJECT_0;
        if (GetExitCodeProcess(w32_child_handles[i], &exitcode) ) {
            *status = (int)(((U8) exitcode) << 8);
            retval = (int)w32_child_pids[i];
            remove_dead_process(i);
            return retval;
        }
    }

    errno = GetLastError();
    return -1;
}

DllExport int
win32_waitpid(int pid, int *status, int flags)
{
    dTHX;
    DWORD timeout = (flags & WNOHANG) ? 0 : INFINITE;
    int retval = -1;
    long child;
    if (pid == -1)				/* XXX threadid == 1 ? */
        return win32_internal_wait(aTHX_ status, timeout);
#ifdef USE_ITHREADS
    else if (pid < 0) {
        child = find_pseudo_pid(aTHX_ -pid);
        if (child >= 0) {
            HANDLE hThread = w32_pseudo_child_handles[child];
            DWORD waitcode;
            win32_msgwait(aTHX_ 1, &hThread, timeout, &waitcode);
            if (waitcode == WAIT_TIMEOUT) {
                return 0;
            }
            else if (waitcode == WAIT_OBJECT_0) {
                if (GetExitCodeThread(hThread, &waitcode)) {
                    *status = (int)(((U8) waitcode) << 8);
                    retval = (int)w32_pseudo_child_pids[child];
                    remove_dead_pseudo_process(child);
                    return -retval;
                }
            }
            else
                errno = ECHILD;
        }
    }
#endif
    else {
        HANDLE hProcess;
        DWORD waitcode;
        child = find_pid(aTHX_ pid);
        if (child >= 0) {
            hProcess = w32_child_handles[child];
            win32_msgwait(aTHX_ 1, &hProcess, timeout, &waitcode);
            if (waitcode == WAIT_TIMEOUT) {
                return 0;
            }
            else if (waitcode == WAIT_OBJECT_0) {
                if (GetExitCodeProcess(hProcess, &waitcode)) {
                    *status = (int)(((U8) waitcode) << 8);
                    retval = (int)w32_child_pids[child];
                    remove_dead_process(child);
                    return retval;
                }
            }
            else
                errno = ECHILD;
        }
        else {
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
            if (hProcess) {
                win32_msgwait(aTHX_ 1, &hProcess, timeout, &waitcode);
                if (waitcode == WAIT_TIMEOUT) {
                    CloseHandle(hProcess);
                    return 0;
                }
                else if (waitcode == WAIT_OBJECT_0) {
                    if (GetExitCodeProcess(hProcess, &waitcode)) {
                        *status = (int)(((U8) waitcode) << 8);
                        CloseHandle(hProcess);
                        return pid;
                    }
                }
                CloseHandle(hProcess);
            }
            else
                errno = ECHILD;
        }
    }
    return retval >= 0 ? pid : retval;
}

DllExport int
win32_wait(int *status)
{
    dTHX;
    return win32_internal_wait(aTHX_ status, INFINITE);
}

DllExport unsigned int
win32_sleep(unsigned int t)
{
    dTHX;
    /* Win32 times are in ms so *1000 in and /1000 out */
    if (t > UINT_MAX / 1000) {
        Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW),
                        "sleep(%lu) too large", t);
    }
    return win32_msgwait(aTHX_ 0, NULL, t * 1000, NULL) / 1000;
}

DllExport int
win32_pause(void)
{
    dTHX;
    win32_msgwait(aTHX_ 0, NULL, INFINITE, NULL);
    return -1;
}

DllExport unsigned int
win32_alarm(unsigned int sec)
{
    /*
     * the 'obvious' implementation is SetTimer() with a callback
     * which does whatever receiving SIGALRM would do
     * we cannot use SIGALRM even via raise() as it is not
     * one of the supported codes in <signal.h>
     */
    dTHX;

    if (w32_message_hwnd == INVALID_HANDLE_VALUE)
        w32_message_hwnd = win32_create_message_window();

    if (sec) {
        if (w32_message_hwnd == NULL)
            w32_timerid = SetTimer(NULL, w32_timerid, sec*1000, NULL);
        else {
            w32_timerid = 1;
            SetTimer(w32_message_hwnd, w32_timerid, sec*1000, NULL);
        }
    }
    else {
        if (w32_timerid) {
            KillTimer(w32_message_hwnd, w32_timerid);
            w32_timerid = 0;
        }
    }
    return 0;
}

extern char *	des_fcrypt(const char *txt, const char *salt, char *cbuf);

DllExport char *
win32_crypt(const char *txt, const char *salt)
{
    dTHX;
    return des_fcrypt(txt, salt, w32_crypt_buffer);
}

/* simulate flock by locking a range on the file */

#define LK_LEN		0xffff0000

DllExport int
win32_flock(int fd, int oper)
{
    OVERLAPPED o;
    int i = -1;
    HANDLE fh;

    fh = (HANDLE)_get_osfhandle(fd);
    if (fh == (HANDLE)-1)  /* _get_osfhandle() already sets errno to EBADF */
        return -1;

    memset(&o, 0, sizeof(o));

    switch(oper) {
    case LOCK_SH:		/* shared lock */
        if (LockFileEx(fh, 0, 0, LK_LEN, 0, &o))
            i = 0;
        break;
    case LOCK_EX:		/* exclusive lock */
        if (LockFileEx(fh, LOCKFILE_EXCLUSIVE_LOCK, 0, LK_LEN, 0, &o))
            i = 0;
        break;
    case LOCK_SH|LOCK_NB:	/* non-blocking shared lock */
        if (LockFileEx(fh, LOCKFILE_FAIL_IMMEDIATELY, 0, LK_LEN, 0, &o))
            i = 0;
        break;
    case LOCK_EX|LOCK_NB:	/* non-blocking exclusive lock */
        if (LockFileEx(fh, LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,
                       0, LK_LEN, 0, &o))
            i = 0;
        break;
    case LOCK_UN:		/* unlock lock */
        if (UnlockFileEx(fh, 0, LK_LEN, 0, &o))
            i = 0;
        break;
    default:			/* unknown */
        errno = EINVAL;
        return -1;
    }
    if (i == -1) {
        if (GetLastError() == ERROR_LOCK_VIOLATION)
            errno = EWOULDBLOCK;
        else
            errno = EINVAL;
    }
    return i;
}

#undef LK_LEN

extern int convert_wsa_error_to_errno(int wsaerr); /* in win32sck.c */

/* Get the errno value corresponding to the given err. This function is not
 * intended to handle conversion of general GetLastError() codes. It only exists
 * to translate Windows sockets error codes from WSAGetLastError(). Such codes
 * used to be assigned to errno/$! in earlier versions of perl; this function is
 * used to catch any old Perl code which is still trying to assign such values
 * to $! and convert them to errno values instead.
 */
int
win32_get_errno(int err)
{
    return convert_wsa_error_to_errno(err);
}

/*
 *  redirected io subsystem for all XS modules
 *
 */

DllExport int *
win32_errno(void)
{
    return (&errno);
}

DllExport char ***
win32_environ(void)
{
    return (&(_environ));
}

/* the rest are the remapped stdio routines */
DllExport FILE *
win32_stderr(void)
{
    return (stderr);
}

DllExport FILE *
win32_stdin(void)
{
    return (stdin);
}

DllExport FILE *
win32_stdout(void)
{
    return (stdout);
}

DllExport int
win32_ferror(FILE *fp)
{
    return (ferror(fp));
}


DllExport int
win32_feof(FILE *fp)
{
    return (feof(fp));
}

#ifdef ERRNO_HAS_POSIX_SUPPLEMENT
extern int convert_errno_to_wsa_error(int err); /* in win32sck.c */
#endif

/*
 * Since the errors returned by the socket error function
 * WSAGetLastError() are not known by the library routine strerror
 * we have to roll our own to cover the case of socket errors
 * that could not be converted to regular errno values by
 * get_last_socket_error() in win32/win32sck.c.
 */

DllExport char *
win32_strerror(int e)
{
#if !defined __MINGW32__      /* compiler intolerance */
    extern int sys_nerr;
#endif

    if (e < 0 || e > sys_nerr) {
        dTHXa(NULL);
        if (e < 0)
            e = GetLastError();
#ifdef ERRNO_HAS_POSIX_SUPPLEMENT
        /* VC10+ and some MinGW/gcc-4.8+ define a "POSIX supplement" of errno
         * values ranging from EADDRINUSE (100) to EWOULDBLOCK (140), but
         * sys_nerr is still 43 and strerror() returns "Unknown error" for them.
         * We must therefore still roll our own messages for these codes, and
         * additionally map them to corresponding Windows (sockets) error codes
         * first to avoid getting the wrong system message.
         */
        else if (inRANGE(e, EADDRINUSE, EWOULDBLOCK)) {
            e = convert_errno_to_wsa_error(e);
        }
#endif

        aTHXa(PERL_GET_THX);
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
                         |FORMAT_MESSAGE_IGNORE_INSERTS, NULL, e, 0,
                          w32_strerror_buffer, sizeof(w32_strerror_buffer),
                          NULL) == 0)
        {
            strcpy(w32_strerror_buffer, "Unknown Error");
        }
        return w32_strerror_buffer;
    }
#undef strerror
    return strerror(e);
#define strerror win32_strerror
}

DllExport void
win32_str_os_error(void *sv, DWORD dwErr)
{
    DWORD dwLen;
    char *sMsg;
    dwLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
                          |FORMAT_MESSAGE_IGNORE_INSERTS
                          |FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                           dwErr, 0, (char *)&sMsg, 1, NULL);
    /* strip trailing whitespace and period */
    if (0 < dwLen) {
        do {
            --dwLen;	/* dwLen doesn't include trailing null */
        } while (0 < dwLen && isSPACE(sMsg[dwLen]));
        if ('.' != sMsg[dwLen])
            dwLen++;
        sMsg[dwLen] = '\0';
    }
    if (0 == dwLen) {
        sMsg = (char*)LocalAlloc(0, 64/**sizeof(TCHAR)*/);
        if (sMsg)
            dwLen = sprintf(sMsg,
                            "Unknown error #0x%lX (lookup 0x%lX)",
                            dwErr, GetLastError());
    }
    if (sMsg) {
        dTHX;
        sv_setpvn((SV*)sv, sMsg, dwLen);
        LocalFree(sMsg);
    }
}

DllExport int
win32_fprintf(FILE *fp, const char *format, ...)
{
    va_list marker;
    va_start(marker, format);     /* Initialize variable arguments. */

    return (vfprintf(fp, format, marker));
}

DllExport int
win32_printf(const char *format, ...)
{
    va_list marker;
    va_start(marker, format);     /* Initialize variable arguments. */

    return (vprintf(format, marker));
}

DllExport int
win32_vfprintf(FILE *fp, const char *format, va_list args)
{
    return (vfprintf(fp, format, args));
}

DllExport int
win32_vprintf(const char *format, va_list args)
{
    return (vprintf(format, args));
}

DllExport size_t
win32_fread(void *buf, size_t size, size_t count, FILE *fp)
{
    return fread(buf, size, count, fp);
}

DllExport size_t
win32_fwrite(const void *buf, size_t size, size_t count, FILE *fp)
{
    return fwrite(buf, size, count, fp);
}

#define MODE_SIZE 10

DllExport FILE *
win32_fopen(const char *filename, const char *mode)
{
    dTHXa(NULL);
    FILE *f;

    if (!*filename)
        return NULL;

    if (stricmp(filename, "/dev/null")==0)
        filename = "NUL";

    aTHXa(PERL_GET_THX);
    f = fopen(PerlDir_mapA(filename), mode);
    /* avoid buffering headaches for child processes */
    if (f && *mode == 'a')
        win32_fseek(f, 0, SEEK_END);
    return f;
}

DllExport FILE *
win32_fdopen(int handle, const char *mode)
{
    FILE *f;
    f = fdopen(handle, (char *) mode);
    /* avoid buffering headaches for child processes */
    if (f && *mode == 'a')
        win32_fseek(f, 0, SEEK_END);
    return f;
}

DllExport FILE *
win32_freopen(const char *path, const char *mode, FILE *stream)
{
    dTHXa(NULL);
    if (stricmp(path, "/dev/null")==0)
        path = "NUL";

    aTHXa(PERL_GET_THX);
    return freopen(PerlDir_mapA(path), mode, stream);
}

DllExport int
win32_fclose(FILE *pf)
{
    return fclose(pf);
}

DllExport int
win32_fputs(const char *s,FILE *pf)
{
    return fputs(s, pf);
}

DllExport int
win32_fputc(int c,FILE *pf)
{
    return fputc(c,pf);
}

DllExport int
win32_ungetc(int c,FILE *pf)
{
    return ungetc(c,pf);
}

DllExport int
win32_getc(FILE *pf)
{
    return getc(pf);
}

DllExport int
win32_fileno(FILE *pf)
{
    return fileno(pf);
}

DllExport void
win32_clearerr(FILE *pf)
{
    clearerr(pf);
    return;
}

DllExport int
win32_fflush(FILE *pf)
{
    return fflush(pf);
}

DllExport Off_t
win32_ftell(FILE *pf)
{
    fpos_t pos;
    if (fgetpos(pf, &pos))
        return -1;
    return (Off_t)pos;
}

DllExport int
win32_fseek(FILE *pf, Off_t offset,int origin)
{
    fpos_t pos;
    switch (origin) {
    case SEEK_CUR:
        if (fgetpos(pf, &pos))
            return -1;
        offset += pos;
        break;
    case SEEK_END:
        fseek(pf, 0, SEEK_END);
        pos = _telli64(fileno(pf));
        offset += pos;
        break;
    case SEEK_SET:
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    return fsetpos(pf, &offset);
}

DllExport int
win32_fgetpos(FILE *pf,fpos_t *p)
{
    return fgetpos(pf, p);
}

DllExport int
win32_fsetpos(FILE *pf,const fpos_t *p)
{
    return fsetpos(pf, p);
}

DllExport void
win32_rewind(FILE *pf)
{
    rewind(pf);
    return;
}

DllExport int
win32_tmpfd(void)
{
    return win32_tmpfd_mode(0);
}

DllExport int
win32_tmpfd_mode(int mode)
{
    char prefix[MAX_PATH+1];
    char filename[MAX_PATH+1];
    DWORD len = GetTempPath(MAX_PATH, prefix);
    mode &= ~( O_ACCMODE | O_CREAT | O_EXCL );
    mode |= O_RDWR;
    if (len && len < MAX_PATH) {
        if (GetTempFileName(prefix, "plx", 0, filename)) {
            HANDLE fh = CreateFile(filename,
                                   DELETE | GENERIC_READ | GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL
                                   | FILE_FLAG_DELETE_ON_CLOSE,
                                   NULL);
            if (fh != INVALID_HANDLE_VALUE) {
                int fd = win32_open_osfhandle((intptr_t)fh, mode);
                if (fd >= 0) {
                    PERL_DEB(dTHX;)
                    DEBUG_p(PerlIO_printf(Perl_debug_log,
                                          "Created tmpfile=%s\n",filename));
                    return fd;
                }
            }
        }
    }
    return -1;
}

DllExport FILE*
win32_tmpfile(void)
{
    int fd = win32_tmpfd();
    if (fd >= 0)
        return win32_fdopen(fd, "w+b");
    return NULL;
}

DllExport void
win32_abort(void)
{
    abort();
    return;
}

DllExport int
win32_fstat(int fd, Stat_t *sbufptr)
{
    HANDLE handle = (HANDLE)win32_get_osfhandle(fd);

    return win32_stat_low(handle, NULL, 0, sbufptr, 0);
}

DllExport int
win32_pipe(int *pfd, unsigned int size, int mode)
{
    return _pipe(pfd, size, mode);
}

DllExport PerlIO*
win32_popenlist(const char *mode, IV narg, SV **args)
{
    if (get_shell() < 0)
        return NULL;

    return do_popen(mode, NULL, narg, args);
}

STATIC PerlIO*
do_popen(const char *mode, const char *command, IV narg, SV **args) {
    int p[2];
    int handles[3];
    int parent, child;
    int stdfd;
    int ourmode;
    int childpid;
    DWORD nhandle;
    int lock_held = 0;
    const char **args_pvs = NULL;

    /* establish which ends read and write */
    if (strchr(mode,'w')) {
        stdfd = 0;		/* stdin */
        parent = 1;
        child = 0;
        nhandle = STD_INPUT_HANDLE;
    }
    else if (strchr(mode,'r')) {
        stdfd = 1;		/* stdout */
        parent = 0;
        child = 1;
        nhandle = STD_OUTPUT_HANDLE;
    }
    else
        return NULL;

    /* set the correct mode */
    if (strchr(mode,'b'))
        ourmode = O_BINARY;
    else if (strchr(mode,'t'))
        ourmode = O_TEXT;
    else
        ourmode = _fmode & (O_TEXT | O_BINARY);

    /* the child doesn't inherit handles */
    ourmode |= O_NOINHERIT;

    if (win32_pipe(p, 512, ourmode) == -1)
        return NULL;

    /* Previously this code redirected stdin/out temporarily so the
       child process inherited those handles, this caused race
       conditions when another thread was writing/reading those
       handles.

       To avoid that we just feed the handles to CreateProcess() so
       the handles are redirected only in the child.
     */
    handles[child] = p[child];
    handles[parent] = -1;
    handles[2] = -1;

    /* CreateProcess() requires inheritable handles */
    if (!SetHandleInformation((HANDLE)_get_osfhandle(p[child]), HANDLE_FLAG_INHERIT,
                              HANDLE_FLAG_INHERIT)) {
        goto cleanup;
    }

    /* start the child */
    {
        dTHX;

        if (command) {
            if ((childpid = do_spawn2_handles(aTHX_ command, EXECF_SPAWN_NOWAIT, handles)) == -1)
                goto cleanup;

        }
        else {
            int i;
            const char *exe_name;

            Newx(args_pvs, narg + 1 + w32_perlshell_items, const char *);
            SAVEFREEPV(args_pvs);
            for (i = 0; i < narg; ++i)
                args_pvs[i] = SvPV_nolen(args[i]);
            args_pvs[i] = NULL;
            exe_name = qualified_path(args_pvs[0], TRUE);
            if (!exe_name)
                /* let CreateProcess() try to find it instead */
                exe_name = args_pvs[0];

            if ((childpid = do_spawnvp_handles(P_NOWAIT, exe_name, args_pvs, handles)) == -1) {
                goto cleanup;
            }
        }

        win32_close(p[child]);

        sv_setiv(*av_fetch(w32_fdpid, p[parent], TRUE), childpid);

        /* set process id so that it can be returned by perl's open() */
        PL_forkprocess = childpid;
    }

    /* we have an fd, return a file stream */
    return (PerlIO_fdopen(p[parent], (char *)mode));

cleanup:
    /* we don't need to check for errors here */
    win32_close(p[0]);
    win32_close(p[1]);

    return (NULL);
}

/*
 * a popen() clone that respects PERL5SHELL
 *
 * changed to return PerlIO* rather than FILE * by BKS, 11-11-2000
 */

DllExport PerlIO*
win32_popen(const char *command, const char *mode)
{
#ifdef USE_RTL_POPEN
    return _popen(command, mode);
#else
    return do_popen(mode, command, 0, NULL);
#endif /* USE_RTL_POPEN */
}

/*
 * pclose() clone
 */

DllExport int
win32_pclose(PerlIO *pf)
{
#ifdef USE_RTL_POPEN
    return _pclose(pf);
#else
    dTHX;
    int childpid, status;
    SV *sv;

    sv = *av_fetch(w32_fdpid, PerlIO_fileno(pf), TRUE);

    if (SvIOK(sv))
        childpid = SvIVX(sv);
    else
        childpid = 0;

    if (!childpid) {
        errno = EBADF;
        return -1;
    }

#ifdef USE_PERLIO
    PerlIO_close(pf);
#else
    fclose(pf);
#endif
    SvIVX(sv) = 0;

    if (win32_waitpid(childpid, &status, 0) == -1)
        return -1;

    return status;

#endif /* USE_RTL_POPEN */
}

DllExport int
win32_link(const char *oldname, const char *newname)
{
    dTHXa(NULL);
    WCHAR wOldName[MAX_PATH+1];
    WCHAR wNewName[MAX_PATH+1];

    if (MultiByteToWideChar(CP_ACP, 0, oldname, -1, wOldName, MAX_PATH+1) &&
        MultiByteToWideChar(CP_ACP, 0, newname, -1, wNewName, MAX_PATH+1) &&
        ((aTHXa(PERL_GET_THX)), wcscpy(wOldName, PerlDir_mapW(wOldName)),
        CreateHardLinkW(PerlDir_mapW(wNewName), wOldName, NULL)))
    {
        return 0;
    }
    translate_to_errno();
    return -1;
}

typedef BOOLEAN (__stdcall *pCreateSymbolicLinkA_t)(LPCSTR, LPCSTR, DWORD);

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#  define SYMBOLIC_LINK_FLAG_DIRECTORY 0x1
#endif

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#  define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif

DllExport int
win32_symlink(const char *oldfile, const char *newfile)
{
    dTHX;
    size_t oldfile_len = strlen(oldfile);
    pCreateSymbolicLinkA_t pCreateSymbolicLinkA =
        (pCreateSymbolicLinkA_t)GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateSymbolicLinkA");
    DWORD create_flags = 0;

    /* this flag can be used only on Windows 10 1703 or newer */
    if (g_osver.dwMajorVersion > 10 ||
        (g_osver.dwMajorVersion == 10 &&
         (g_osver.dwMinorVersion > 0 || g_osver.dwBuildNumber > 15063)))
    {
        create_flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    }

    if (!pCreateSymbolicLinkA) {
        errno = ENOSYS;
        return -1;
    }

    /* oldfile might be relative and we don't want to change that,
       so don't map that.
    */
    newfile = PerlDir_mapA(newfile);

    if (strchr(oldfile, '/')) {
        /* Win32 (or perhaps NTFS) won't follow symlinks containing
           /, so replace any with \\
        */
        char *temp = savepv(oldfile);
        SAVEFREEPV(temp);
        char *p = temp;
        while (*p) {
            if (*p == '/') {
                *p = '\\';
            }
            ++p;
        }
        *p = 0;
        oldfile = temp;
        oldfile_len = p - temp;
    }

    /* are we linking to a directory?
       CreateSymlinkA() needs to know if the target is a directory,
       If it looks like a directory name:
        - ends in slash
        - is just . or ..
        - ends in /. or /.. (with either slash)
        - is a simple drive letter
       assume it's a directory.

       Otherwise if the oldfile is relative we need to make a relative path
       based on the newfile to check if the target is a directory.
    */
    if ((oldfile_len >= 1 && isSLASH(oldfile[oldfile_len-1])) ||
        strEQ(oldfile, "..") ||
        strEQ(oldfile, ".") ||
        (isSLASH(oldfile[oldfile_len-2]) && oldfile[oldfile_len-1] == '.') ||
        strEQ(oldfile+oldfile_len-3, "\\..") ||
        (oldfile_len == 2 && oldfile[1] == ':')) {
        create_flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
    }
    else {
        DWORD dest_attr;
        const char *dest_path = oldfile;
        char szTargetName[MAX_PATH+1];

        if (oldfile_len >= 3 && oldfile[1] == ':') {
            /* relative to current directory on a drive, or absolute */
            /* dest_path = oldfile; already done */
        }
        else if (oldfile[0] != '\\') {
            size_t newfile_len = strlen(newfile);
            const char *last_slash = strrchr(newfile, '/');
            const char *last_bslash = strrchr(newfile, '\\');
            const char *end_dir = last_slash && last_bslash
                ? ( last_slash > last_bslash ? last_slash : last_bslash)
                : last_slash ? last_slash : last_bslash ? last_bslash : NULL;

            if (end_dir) {
                if ((end_dir - newfile + 1) + oldfile_len > MAX_PATH) {
                    /* too long */
                    errno = EINVAL;
                    return -1;
                }

                memcpy(szTargetName, newfile, end_dir - newfile + 1);
                strcpy(szTargetName + (end_dir - newfile + 1), oldfile);
                dest_path = szTargetName;
            }
            else {
                /* newpath is just a filename */
                /* dest_path = oldfile; */
            }
        }

        dest_attr = GetFileAttributes(dest_path);
        if (dest_attr != (DWORD)-1 && (dest_attr & FILE_ATTRIBUTE_DIRECTORY)) {
            create_flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
        }
    }

    if (!pCreateSymbolicLinkA(newfile, oldfile, create_flags)) {
        translate_to_errno();
        return -1;
    }

    return 0;
}

DllExport int
win32_rename(const char *oname, const char *newname)
{
    char szOldName[MAX_PATH+1];
    BOOL bResult;
    DWORD dwFlags = MOVEFILE_COPY_ALLOWED;
    dTHX;

    if (stricmp(newname, oname))
        dwFlags |= MOVEFILE_REPLACE_EXISTING;
    strcpy(szOldName, PerlDir_mapA(oname));

    bResult = MoveFileExA(szOldName,PerlDir_mapA(newname), dwFlags);
    if (!bResult) {
        DWORD err = GetLastError();
        switch (err) {
        case ERROR_BAD_NET_NAME:
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_PATHNAME:
        case ERROR_FILE_NOT_FOUND:
        case ERROR_FILENAME_EXCED_RANGE:
        case ERROR_INVALID_DRIVE:
        case ERROR_NO_MORE_FILES:
        case ERROR_PATH_NOT_FOUND:
            errno = ENOENT;
            break;
        case ERROR_DISK_FULL:
            errno = ENOSPC;
            break;
        case ERROR_NOT_ENOUGH_QUOTA:
            errno = EDQUOT;
            break;
        default:
            errno = EACCES;
            break;
        }
        return -1;
    }
    return 0;
}

DllExport int
win32_setmode(int fd, int mode)
{
    return setmode(fd, mode);
}

DllExport int
win32_chsize(int fd, Off_t size)
{
    int retval = 0;
    Off_t cur, end, extend;

    cur = win32_tell(fd);
    if (cur < 0)
        return -1;
    end = win32_lseek(fd, 0, SEEK_END);
    if (end < 0)
        return -1;
    extend = size - end;
    if (extend == 0) {
        /* do nothing */
    }
    else if (extend > 0) {
        /* must grow the file, padding with nulls */
        char b[4096];
        int oldmode = win32_setmode(fd, O_BINARY);
        size_t count;
        memset(b, '\0', sizeof(b));
        do {
            count = extend >= sizeof(b) ? sizeof(b) : (size_t)extend;
            count = win32_write(fd, b, count);
            if ((int)count < 0) {
                retval = -1;
                break;
            }
        } while ((extend -= count) > 0);
        win32_setmode(fd, oldmode);
    }
    else {
        /* shrink the file */
        win32_lseek(fd, size, SEEK_SET);
        if (!SetEndOfFile((HANDLE)_get_osfhandle(fd))) {
            errno = EACCES;
            retval = -1;
        }
    }
    win32_lseek(fd, cur, SEEK_SET);
    return retval;
}

DllExport Off_t
win32_lseek(int fd, Off_t offset, int origin)
{
    return _lseeki64(fd, offset, origin);
}

DllExport Off_t
win32_tell(int fd)
{
    return _telli64(fd);
}

DllExport int
win32_open(const char *path, int flag, ...)
{
    dTHXa(NULL);
    va_list ap;
    int pmode;

    va_start(ap, flag);
    pmode = va_arg(ap, int);
    va_end(ap);

    if (stricmp(path, "/dev/null")==0)
        path = "NUL";

    aTHXa(PERL_GET_THX);
    return open(PerlDir_mapA(path), flag, pmode);
}

DllExport int
win32_close(int fd)
{
    return _close(fd);
}

DllExport int
win32_eof(int fd)
{
    return eof(fd);
}

DllExport int
win32_isatty(int fd)
{
    /* The Microsoft isatty() function returns true for *all*
     * character mode devices, including "nul".  Our implementation
     * should only return true if the handle has a console buffer.
     */
    DWORD mode;
    HANDLE fh = (HANDLE)_get_osfhandle(fd);
    if (fh == (HANDLE)-1) {
        /* errno is already set to EBADF */
        return 0;
    }

    if (GetConsoleMode(fh, &mode))
        return 1;

    errno = ENOTTY;
    return 0;
}

DllExport int
win32_dup(int fd)
{
    return dup(fd);
}

DllExport int
win32_dup2(int fd1,int fd2)
{
    return dup2(fd1,fd2);
}

static int
win32_read_console(int fd, U8 *buf, unsigned int cnt)
{
    /* This function is a workaround for a bug in Windows:
     * https://github.com/microsoft/terminal/issues/4551
     * tl;dr: ReadFile() and ReadConsoleA() return garbage when reading
     * non-ASCII characters from the console with the 65001 codepage.
     */
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    size_t left_to_read = cnt;
    DWORD mode;

    if (h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }

    if (!GetConsoleMode(h, &mode)) {
        translate_to_errno();
        return -1;
    }

    while (left_to_read) {
        /* The purpose of converted_buf is to preserve partial UTF-8 (or of any
         * other multibyte encoding) code points between read() calls. Since
         * there's only one console, the buffer is global. It's needed because
         * ReadConsoleW() returns a string of UTF-16 code units and its result,
         * after conversion to the current console codepage, may not fit in the
         * return buffer.
         *
         * The buffer's size is 8 because it will contain at most two UTF-8 code
         * points.
         */
        static char converted_buf[8];
        static size_t converted_buf_len = 0;
        WCHAR wbuf[2];
        DWORD wbuf_len = 0, chars_read;

        if (converted_buf_len) {
            bool newline = 0;
            size_t to_write = MIN(converted_buf_len, left_to_read);

            /* Don't read anything if the *first* character is ^Z and
             * ENABLE_PROCESSED_INPUT is enabled. On some versions of Windows,
             * ReadFile() ignores ENABLE_PROCESSED_INPUT, but apparently it's a
             * bug: https://github.com/microsoft/terminal/issues/4958
             */
            if (left_to_read == cnt && (mode & ENABLE_PROCESSED_INPUT) &&
                converted_buf[0] == 0x1a)
                 break;

            /* Are we returning a newline? */
            if (memchr(converted_buf, '\n', to_write))
                newline = 1;

            memcpy(buf, converted_buf, to_write);
            buf += to_write;

            /* If there's anything left in converted_buf, move it to the
             * beginning of the buffer. */
            converted_buf_len -= to_write;
            if (converted_buf_len)
                memmove(
                    converted_buf, converted_buf + to_write, converted_buf_len
                );

            left_to_read -= to_write;

            /* With ENABLE_LINE_INPUT enabled, we stop reading after the first
             * newline, otherwise we stop reading after the first character. */
            if (!left_to_read || newline || (mode & ENABLE_LINE_INPUT) == 0)
                break;
        }

        /* Reading one code unit at a time is inefficient, but since this code
         * is used only for the interactive console, that shouldn't matter. */
        if (!ReadConsoleW(h, wbuf, 1, &chars_read, 0)) {
            translate_to_errno();
            return -1;
        }
        if (!chars_read)
            break;

        ++wbuf_len;

        if (wbuf[0] >= 0xD800 && wbuf[0] <= 0xDBFF) {
            /* High surrogate, read one more code unit. */
            if (!ReadConsoleW(h, wbuf + 1, 1, &chars_read, 0)) {
                translate_to_errno();
                return -1;
            }
            if (chars_read)
                ++wbuf_len;
        }

        converted_buf_len = WideCharToMultiByte(
            GetConsoleCP(), 0, wbuf, wbuf_len, converted_buf,
            sizeof(converted_buf), NULL, NULL
        );
        if (!converted_buf_len) {
            translate_to_errno();
            return -1;
        }
    }

    return cnt - left_to_read;
}


DllExport int
win32_read(int fd, void *buf, unsigned int cnt)
{
    int ret;
    if (UNLIKELY(win32_isatty(fd) && GetConsoleCP() == 65001)) {
        MUTEX_LOCK(&win32_read_console_mutex);
        ret = win32_read_console(fd, (U8 *)buf, cnt);
        MUTEX_UNLOCK(&win32_read_console_mutex);
    }
    else
        ret = read(fd, buf, cnt);

    return ret;
}

DllExport int
win32_write(int fd, const void *buf, unsigned int cnt)
{
    return write(fd, buf, cnt);
}

DllExport int
win32_mkdir(const char *dir, int mode)
{
    dTHX;
    return mkdir(PerlDir_mapA(dir)); /* just ignore mode */
}

DllExport int
win32_rmdir(const char *dir)
{
    dTHX;
    return rmdir(PerlDir_mapA(dir));
}

DllExport int
win32_chdir(const char *dir)
{
    if (!dir || !*dir) {
        errno = ENOENT;
        return -1;
    }
    return chdir(dir);
}

DllExport  int
win32_access(const char *path, int mode)
{
    dTHX;
    return access(PerlDir_mapA(path), mode);
}

DllExport  int
win32_chmod(const char *path, int mode)
{
    dTHX;
    return chmod(PerlDir_mapA(path), mode);
}


static char *
create_command_line(char *cname, STRLEN clen, const char * const *args)
{
    PERL_DEB(dTHX;)
    int index, argc;
    char *cmd, *ptr;
    const char *arg;
    STRLEN len = 0;
    bool bat_file = FALSE;
    bool cmd_shell = FALSE;
    bool dumb_shell = FALSE;
    bool extra_quotes = FALSE;
    bool quote_next = FALSE;

    if (!cname)
        cname = (char*)args[0];

    /* The NT cmd.exe shell has the following peculiarity that needs to be
     * worked around.  It strips a leading and trailing dquote when any
     * of the following is true:
     *    1. the /S switch was used
     *    2. there are more than two dquotes
     *    3. there is a special character from this set: &<>()@^|
     *    4. no whitespace characters within the two dquotes
     *    5. string between two dquotes isn't an executable file
     * To work around this, we always add a leading and trailing dquote
     * to the string, if the first argument is either "cmd.exe" or "cmd",
     * and there were at least two or more arguments passed to cmd.exe
     * (not including switches).
     * XXX the above rules (from "cmd /?") don't seem to be applied
     * always, making for the convolutions below :-(
     */
    if (cname) {
        if (!clen)
            clen = strlen(cname);

        if (clen > 4
            && (stricmp(&cname[clen-4], ".bat") == 0
                || (stricmp(&cname[clen-4], ".cmd") == 0)))
        {
            bat_file = TRUE;
            len += 3;
        }
        else {
            char *exe = strrchr(cname, '/');
            char *exe2 = strrchr(cname, '\\');
            if (exe2 > exe)
                exe = exe2;
            if (exe)
                ++exe;
            else
                exe = cname;
            if (stricmp(exe, "cmd.exe") == 0 || stricmp(exe, "cmd") == 0) {
                cmd_shell = TRUE;
                len += 3;
            }
            else if (stricmp(exe, "command.com") == 0
                     || stricmp(exe, "command") == 0)
            {
                dumb_shell = TRUE;
            }
        }
    }

    DEBUG_p(PerlIO_printf(Perl_debug_log, "Args "));
    for (index = 0; (arg = (char*)args[index]) != NULL; ++index) {
        STRLEN curlen = strlen(arg);
        if (!(arg[0] == '"' && arg[curlen-1] == '"'))
            len += 2;	/* assume quoting needed (worst case) */
        len += curlen + 1;
        DEBUG_p(PerlIO_printf(Perl_debug_log, "[%s]",arg));
    }
    DEBUG_p(PerlIO_printf(Perl_debug_log, "\n"));

    argc = index;
    Newx(cmd, len, char);
    ptr = cmd;

    if (bat_file) {
        *ptr++ = '"';
        extra_quotes = TRUE;
    }

    for (index = 0; (arg = (char*)args[index]) != NULL; ++index) {
        bool do_quote = 0;
        STRLEN curlen = strlen(arg);

        /* we want to protect empty arguments and ones with spaces with
         * dquotes, but only if they aren't already there */
        if (!dumb_shell) {
            if (!curlen) {
                do_quote = 1;
            }
            else if (quote_next) {
                /* see if it really is multiple arguments pretending to
                 * be one and force a set of quotes around it */
                if (*find_next_space(arg))
                    do_quote = 1;
            }
            else if (!(arg[0] == '"' && curlen > 1 && arg[curlen-1] == '"')) {
                STRLEN i = 0;
                while (i < curlen) {
                    if (isSPACE(arg[i])) {
                        do_quote = 1;
                    }
                    else if (arg[i] == '"') {
                        do_quote = 0;
                        break;
                    }
                    i++;
                }
            }
        }

        if (do_quote)
            *ptr++ = '"';

        strcpy(ptr, arg);
        ptr += curlen;

        if (do_quote)
            *ptr++ = '"';

        if (args[index+1])
            *ptr++ = ' ';

        if (!extra_quotes
            && cmd_shell
            && curlen >= 2
            && *arg  == '/'     /* see if arg is "/c", "/x/c", "/x/d/c" etc. */
            && stricmp(arg+curlen-2, "/c") == 0)
        {
            /* is there a next argument? */
            if (args[index+1]) {
                /* are there two or more next arguments? */
                if (args[index+2]) {
                    *ptr++ = '"';
                    extra_quotes = TRUE;
                }
                else {
                    /* single argument, force quoting if it has spaces */
                    quote_next = TRUE;
                }
            }
        }
    }

    if (extra_quotes)
        *ptr++ = '"';

    *ptr = '\0';

    return cmd;
}

static const char *exe_extensions[] =
  {
    ".exe", /* this must be first */
    ".cmd",
    ".bat"
  };

static char *
qualified_path(const char *cmd, bool other_exts)
{
    char *pathstr;
    char *fullcmd, *curfullcmd;
    STRLEN cmdlen = 0;
    int has_slash = 0;

    if (!cmd)
        return NULL;
    fullcmd = (char*)cmd;
    while (*fullcmd) {
        if (*fullcmd == '/' || *fullcmd == '\\')
            has_slash++;
        fullcmd++;
        cmdlen++;
    }

    /* look in PATH */
    {
        dTHX;
        pathstr = PerlEnv_getenv("PATH");
    }
    /* worst case: PATH is a single directory; we need additional space
     * to append "/", ".exe" and trailing "\0" */
    Newx(fullcmd, (pathstr ? strlen(pathstr) : 0) + cmdlen + 6, char);
    curfullcmd = fullcmd;

    while (1) {
        DWORD res;

        /* start by appending the name to the current prefix */
        strcpy(curfullcmd, cmd);
        curfullcmd += cmdlen;

        /* if it doesn't end with '.', or has no extension, try adding
         * a trailing .exe first */
        if (cmd[cmdlen-1] != '.'
            && (cmdlen < 4 || cmd[cmdlen-4] != '.'))
        {
            int i;
            /* first extension is .exe */
            int ext_limit = other_exts ? C_ARRAY_LENGTH(exe_extensions) : 1;
            for (i = 0; i < ext_limit; ++i) {
                strcpy(curfullcmd, exe_extensions[i]);
                res = GetFileAttributes(fullcmd);
                if (res != 0xFFFFFFFF && !(res & FILE_ATTRIBUTE_DIRECTORY))
                    return fullcmd;
            }

            *curfullcmd = '\0';
        }

        /* that failed, try the bare name */
        res = GetFileAttributes(fullcmd);
        if (res != 0xFFFFFFFF && !(res & FILE_ATTRIBUTE_DIRECTORY))
            return fullcmd;

        /* quit if no other path exists, or if cmd already has path */
        if (!pathstr || !*pathstr || has_slash)
            break;

        /* skip leading semis */
        while (*pathstr == ';')
            pathstr++;

        /* build a new prefix from scratch */
        curfullcmd = fullcmd;
        while (*pathstr && *pathstr != ';') {
            if (*pathstr == '"') {	/* foo;"baz;etc";bar */
                pathstr++;		/* skip initial '"' */
                while (*pathstr && *pathstr != '"') {
                    *curfullcmd++ = *pathstr++;
                }
                if (*pathstr)
                    pathstr++;		/* skip trailing '"' */
            }
            else {
                *curfullcmd++ = *pathstr++;
            }
        }
        if (*pathstr)
            pathstr++;			/* skip trailing semi */
        if (curfullcmd > fullcmd	/* append a dir separator */
            && curfullcmd[-1] != '/' && curfullcmd[-1] != '\\')
        {
            *curfullcmd++ = '\\';
        }
    }

    Safefree(fullcmd);
    return NULL;
}

/* The following are just place holders.
 * Some hosts may provide and environment that the OS is
 * not tracking, therefore, these host must provide that
 * environment and the current directory to CreateProcess
 */

DllExport void*
win32_get_childenv(void)
{
    return NULL;
}

DllExport void
win32_free_childenv(void* d)
{
}

DllExport void
win32_clearenv(void)
{
    char *envv = GetEnvironmentStrings();
    char *cur = envv;
    STRLEN len;
    while (*cur) {
        char *end = strchr(cur,'=');
        if (end && end != cur) {
            *end = '\0';
            SetEnvironmentVariable(cur, NULL);
            *end = '=';
            cur = end + strlen(end+1)+2;
        }
        else if ((len = strlen(cur)))
            cur += len+1;
    }
    FreeEnvironmentStrings(envv);
}

DllExport char*
win32_get_childdir(void)
{
    char* ptr;
    char szfilename[MAX_PATH+1];

    GetCurrentDirectoryA(MAX_PATH+1, szfilename);
    Newx(ptr, strlen(szfilename)+1, char);
    strcpy(ptr, szfilename);
    return ptr;
}

DllExport void
win32_free_childdir(char* d)
{
    Safefree(d);
}


/* XXX this needs to be made more compatible with the spawnvp()
 * provided by the various RTLs.  In particular, searching for
 * *.{com,bat,cmd} files (as done by the RTLs) is unimplemented.
 * This doesn't significantly affect perl itself, because we
 * always invoke things using PERL5SHELL if a direct attempt to
 * spawn the executable fails.
 *
 * XXX splitting and rejoining the commandline between do_aspawn()
 * and win32_spawnvp() could also be avoided.
 */

DllExport int
win32_spawnvp(int mode, const char *cmdname, const char *const *argv)
{
#ifdef USE_RTL_SPAWNVP
    return _spawnvp(mode, cmdname, (char * const *)argv);
#else
    return do_spawnvp_handles(mode, cmdname, argv, NULL);
#endif
}

static int
do_spawnvp_handles(int mode, const char *cmdname, const char *const *argv,
                const int *handles) {
    dTHXa(NULL);
    int ret;
    void* env;
    char* dir;
    child_IO_table tbl;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    DWORD create = 0;
    char *cmd;
    char *fullcmd = NULL;
    char *cname = (char *)cmdname;
    STRLEN clen = 0;

    if (cname) {
        clen = strlen(cname);
        /* if command name contains dquotes, must remove them */
        if (strchr(cname, '"')) {
            cmd = cname;
            Newx(cname,clen+1,char);
            clen = 0;
            while (*cmd) {
                if (*cmd != '"') {
                    cname[clen] = *cmd;
                    ++clen;
                }
                ++cmd;
            }
            cname[clen] = '\0';
        }
    }

    cmd = create_command_line(cname, clen, argv);

    aTHXa(PERL_GET_THX);
    env = PerlEnv_get_childenv();
    dir = PerlEnv_get_childdir();

    switch(mode) {
    case P_NOWAIT:	/* asynch + remember result */
        if (w32_num_children >= MAXIMUM_WAIT_OBJECTS) {
            errno = EAGAIN;
            ret = -1;
            goto RETVAL;
        }
        /* Create a new process group so we can use GenerateConsoleCtrlEvent()
         * in win32_kill()
         */
        create |= CREATE_NEW_PROCESS_GROUP;
        /* FALL THROUGH */

    case P_WAIT:	/* synchronous execution */
        break;
    default:		/* invalid mode */
        errno = EINVAL;
        ret = -1;
        goto RETVAL;
    }

    memset(&StartupInfo,0,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    memset(&tbl,0,sizeof(tbl));
    PerlEnv_get_child_IO(&tbl);
    StartupInfo.dwFlags		= tbl.dwFlags;
    StartupInfo.dwX		= tbl.dwX;
    StartupInfo.dwY		= tbl.dwY;
    StartupInfo.dwXSize		= tbl.dwXSize;
    StartupInfo.dwYSize		= tbl.dwYSize;
    StartupInfo.dwXCountChars	= tbl.dwXCountChars;
    StartupInfo.dwYCountChars	= tbl.dwYCountChars;
    StartupInfo.dwFillAttribute	= tbl.dwFillAttribute;
    StartupInfo.wShowWindow	= tbl.wShowWindow;
    StartupInfo.hStdInput	= handles && handles[0] != -1 ?
            (HANDLE)_get_osfhandle(handles[0]) : tbl.childStdIn;
    StartupInfo.hStdOutput	= handles && handles[1] != -1 ?
            (HANDLE)_get_osfhandle(handles[1]) : tbl.childStdOut;
    StartupInfo.hStdError	= handles && handles[2] != -1 ?
            (HANDLE)_get_osfhandle(handles[2]) : tbl.childStdErr;
    if (StartupInfo.hStdInput == INVALID_HANDLE_VALUE &&
        StartupInfo.hStdOutput == INVALID_HANDLE_VALUE &&
        StartupInfo.hStdError == INVALID_HANDLE_VALUE)
    {
        create |= CREATE_NEW_CONSOLE;
    }
    else {
        StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    }
    if (w32_use_showwindow) {
        StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow = w32_showwindow;
    }

    DEBUG_p(PerlIO_printf(Perl_debug_log, "Spawning [%s] with [%s]\n",
                          cname,cmd));
RETRY:
    if (!CreateProcess(cname,		/* search PATH to find executable */
                       cmd,		/* executable, and its arguments */
                       NULL,		/* process attributes */
                       NULL,		/* thread attributes */
                       TRUE,		/* inherit handles */
                       create,		/* creation flags */
                       (LPVOID)env,	/* inherit environment */
                       dir,		/* inherit cwd */
                       &StartupInfo,
                       &ProcessInformation))
    {
        /* initial NULL argument to CreateProcess() does a PATH
         * search, but it always first looks in the directory
         * where the current process was started, which behavior
         * is undesirable for backward compatibility.  So we
         * jump through our own hoops by picking out the path
         * we really want it to use. */
        if (!fullcmd) {
            fullcmd = qualified_path(cname, FALSE);
            if (fullcmd) {
                if (cname != cmdname)
                    Safefree(cname);
                cname = fullcmd;
                DEBUG_p(PerlIO_printf(Perl_debug_log,
                                      "Retrying [%s] with same args\n",
                                      cname));
                goto RETRY;
            }
        }
        errno = ENOENT;
        ret = -1;
        goto RETVAL;
    }

    if (mode == P_NOWAIT) {
        /* asynchronous spawn -- store handle, return PID */
        ret = (int)ProcessInformation.dwProcessId;

        w32_child_handles[w32_num_children] = ProcessInformation.hProcess;
        w32_child_pids[w32_num_children] = (DWORD)ret;
        ++w32_num_children;
    }
    else {
        DWORD status;
        win32_msgwait(aTHX_ 1, &ProcessInformation.hProcess, INFINITE, NULL);
        /* FIXME: if msgwait returned due to message perhaps forward the
           "signal" to the process
         */
        GetExitCodeProcess(ProcessInformation.hProcess, &status);
        ret = (int)status;
        CloseHandle(ProcessInformation.hProcess);
    }

    CloseHandle(ProcessInformation.hThread);

RETVAL:
    PerlEnv_free_childenv(env);
    PerlEnv_free_childdir(dir);
    Safefree(cmd);
    if (cname != cmdname)
        Safefree(cname);
    return ret;
}

DllExport int
win32_execv(const char *cmdname, const char *const *argv)
{
#ifdef USE_ITHREADS
    dTHX;
    /* if this is a pseudo-forked child, we just want to spawn
     * the new program, and return */
    if (w32_pseudo_id)
        return _spawnv(P_WAIT, cmdname, argv);
#endif
    return _execv(cmdname, argv);
}

DllExport int
win32_execvp(const char *cmdname, const char *const *argv)
{
#ifdef USE_ITHREADS
    dTHX;
    /* if this is a pseudo-forked child, we just want to spawn
     * the new program, and return */
    if (w32_pseudo_id) {
        int status = win32_spawnvp(P_WAIT, cmdname, (const char *const *)argv);
        if (status != -1) {
            my_exit(status);
            return 0;
        }
        else
            return status;
    }
#endif
    return _execvp(cmdname, argv);
}

DllExport void
win32_perror(const char *str)
{
    perror(str);
}

DllExport void
win32_setbuf(FILE *pf, char *buf)
{
    setbuf(pf, buf);
}

DllExport int
win32_setvbuf(FILE *pf, char *buf, int type, size_t size)
{
    return setvbuf(pf, buf, type, size);
}

DllExport int
win32_flushall(void)
{
    return flushall();
}

DllExport int
win32_fcloseall(void)
{
    return fcloseall();
}

DllExport char*
win32_fgets(char *s, int n, FILE *pf)
{
    return fgets(s, n, pf);
}

DllExport char*
win32_gets(char *s)
{
    return gets(s);
}

DllExport int
win32_fgetc(FILE *pf)
{
    return fgetc(pf);
}

DllExport int
win32_putc(int c, FILE *pf)
{
    return putc(c,pf);
}

DllExport int
win32_puts(const char *s)
{
    return puts(s);
}

DllExport int
win32_getchar(void)
{
    return getchar();
}

DllExport int
win32_putchar(int c)
{
    return putchar(c);
}

#ifdef MYMALLOC

#ifndef USE_PERL_SBRK

static char *committed = NULL;		/* XXX threadead */
static char *base      = NULL;		/* XXX threadead */
static char *reserved  = NULL;		/* XXX threadead */
static char *brk       = NULL;		/* XXX threadead */
static DWORD pagesize  = 0;		/* XXX threadead */

void *
sbrk(ptrdiff_t need)
{
 void *result;
 if (!pagesize)
  {SYSTEM_INFO info;
   GetSystemInfo(&info);
   /* Pretend page size is larger so we don't perpetually
    * call the OS to commit just one page ...
    */
   pagesize = info.dwPageSize << 3;
  }
 if (brk+need >= reserved)
  {
   DWORD size = brk+need-reserved;
   char *addr;
   char *prev_committed = NULL;
   if (committed && reserved && committed < reserved)
    {
     /* Commit last of previous chunk cannot span allocations */
     addr = (char *) VirtualAlloc(committed,reserved-committed,MEM_COMMIT,PAGE_READWRITE);
     if (addr)
      {
      /* Remember where we committed from in case we want to decommit later */
      prev_committed = committed;
      committed = reserved;
      }
    }
   /* Reserve some (more) space
    * Contiguous blocks give us greater efficiency, so reserve big blocks -
    * this is only address space not memory...
    * Note this is a little sneaky, 1st call passes NULL as reserved
    * so lets system choose where we start, subsequent calls pass
    * the old end address so ask for a contiguous block
    */
sbrk_reserve:
   if (size < 64*1024*1024)
    size = 64*1024*1024;
   size = ((size + pagesize - 1) / pagesize) * pagesize;
   addr  = (char *) VirtualAlloc(reserved,size,MEM_RESERVE,PAGE_NOACCESS);
   if (addr)
    {
     reserved = addr+size;
     if (!base)
      base = addr;
     if (!committed)
      committed = base;
     if (!brk)
      brk = committed;
    }
   else if (reserved)
    {
      /* The existing block could not be extended far enough, so decommit
       * anything that was just committed above and start anew */
      if (prev_committed)
       {
       if (!VirtualFree(prev_committed,reserved-prev_committed,MEM_DECOMMIT))
        return (void *) -1;
       }
      reserved = base = committed = brk = NULL;
      size = need;
      goto sbrk_reserve;
    }
   else
    {
     return (void *) -1;
    }
  }
 result = brk;
 brk += need;
 if (brk > committed)
  {
   DWORD size = ((brk-committed + pagesize -1)/pagesize) * pagesize;
   char *addr;
   if (committed+size > reserved)
    size = reserved-committed;
   addr = (char *) VirtualAlloc(committed,size,MEM_COMMIT,PAGE_READWRITE);
   if (addr)
    committed += size;
   else
    return (void *) -1;
  }
 return result;
}

#endif
#endif

DllExport void*
win32_malloc(size_t size)
{
    return malloc(size);
}

DllExport void*
win32_calloc(size_t numitems, size_t size)
{
    return calloc(numitems,size);
}

DllExport void*
win32_realloc(void *block, size_t size)
{
    return realloc(block,size);
}

DllExport void
win32_free(void *block)
{
    free(block);
}


DllExport int
win32_open_osfhandle(intptr_t handle, int flags)
{
    return _open_osfhandle(handle, flags);
}

DllExport intptr_t
win32_get_osfhandle(int fd)
{
    return (intptr_t)_get_osfhandle(fd);
}

DllExport FILE *
win32_fdupopen(FILE *pf)
{
    FILE* pfdup;
    fpos_t pos;
    char mode[3];
    int fileno = win32_dup(win32_fileno(pf));

    /* open the file in the same mode */
    if (PERLIO_FILE_flag(pf) & PERLIO_FILE_flag_RD) {
        mode[0] = 'r';
        mode[1] = 0;
    }
    else if (PERLIO_FILE_flag(pf) & PERLIO_FILE_flag_WR) {
        mode[0] = 'a';
        mode[1] = 0;
    }
    else if (PERLIO_FILE_flag(pf) & PERLIO_FILE_flag_RW) {
        mode[0] = 'r';
        mode[1] = '+';
        mode[2] = 0;
    }

    /* it appears that the binmode is attached to the
     * file descriptor so binmode files will be handled
     * correctly
     */
    pfdup = win32_fdopen(fileno, mode);

    /* move the file pointer to the same position */
    if (!fgetpos(pf, &pos)) {
        fsetpos(pfdup, &pos);
    }
    return pfdup;
}

DllExport void*
win32_dynaload(const char* filename)
{
    dTHXa(NULL);
    char buf[MAX_PATH+1];
    const char *first;

    /* LoadLibrary() doesn't recognize forward slashes correctly,
     * so turn 'em back. */
    first = strchr(filename, '/');
    if (first) {
        STRLEN len = strlen(filename);
        if (len <= MAX_PATH) {
            strcpy(buf, filename);
            filename = &buf[first - filename];
            while (*filename) {
                if (*filename == '/')
                    *(char*)filename = '\\';
                ++filename;
            }
            filename = buf;
        }
    }
    aTHXa(PERL_GET_THX);
    return LoadLibraryExA(PerlDir_mapA(filename), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

XS(w32_SetChildShowWindow)
{
    dXSARGS;
    BOOL use_showwindow = w32_use_showwindow;
    /* use "unsigned short" because Perl has redefined "WORD" */
    unsigned short showwindow = w32_showwindow;

    if (items > 1)
        croak_xs_usage(cv, "[showwindow]");

    if (items == 0 || !SvOK(ST(0)))
        w32_use_showwindow = FALSE;
    else {
        w32_use_showwindow = TRUE;
        w32_showwindow = (unsigned short)SvIV(ST(0));
    }

    EXTEND(SP, 1);
    if (use_showwindow)
        ST(0) = sv_2mortal(newSViv(showwindow));
    else
        ST(0) = &PL_sv_undef;
    XSRETURN(1);
}


#ifdef PERL_IS_MINIPERL
/* shelling out is much slower, full perl uses Win32.pm */
XS(w32_GetCwd)
{
    dXSARGS;
    /* Make the host for current directory */
    char* ptr = PerlEnv_get_childdir();
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

        ST(0) = sv;
        XSRETURN(1);
    }
    XSRETURN_UNDEF;
}
#endif

void
Perl_init_os_extras(void)
{
    dTHXa(NULL);
    const char *file = __FILE__;

    /* Initialize Win32CORE if it has been statically linked. */
#ifndef PERL_IS_MINIPERL
    void (*pfn_init)(pTHX);
    HMODULE module = (HMODULE)((w32_perldll_handle == INVALID_HANDLE_VALUE)
                               ? GetModuleHandle(NULL)
                               : w32_perldll_handle);
    pfn_init = (void (*)(pTHX))GetProcAddress(module, "init_Win32CORE");
    aTHXa(PERL_GET_THX);
    if (pfn_init)
        pfn_init(aTHX);
#else
    aTHXa(PERL_GET_THX);
#endif

    newXS("Win32::SetChildShowWindow", w32_SetChildShowWindow, file);
#ifdef PERL_IS_MINIPERL
    newXS("Win32::GetCwd", w32_GetCwd, file);
#endif
}

void *
win32_signal_context(void)
{
    dTHX;
#ifdef MULTIPLICITY
    if (!my_perl) {
        my_perl = PL_curinterp;
        PERL_SET_THX(my_perl);
    }
    return my_perl;
#else
    return PL_curinterp;
#endif
}


BOOL WINAPI
win32_ctrlhandler(DWORD dwCtrlType)
{
#ifdef MULTIPLICITY
    dTHXa(PERL_GET_SIG_CONTEXT);

    if (!my_perl)
        return FALSE;
#endif

    switch(dwCtrlType) {
    case CTRL_CLOSE_EVENT:
     /*  A signal that the system sends to all processes attached to a console when
         the user closes the console (either by choosing the Close command from the
         console window's System menu, or by choosing the End Task command from the
         Task List
      */
        if (do_raise(aTHX_ 1))	      /* SIGHUP */
            sig_terminate(aTHX_ 1);
        return TRUE;

    case CTRL_C_EVENT:
        /*  A CTRL+c signal was received */
        if (do_raise(aTHX_ SIGINT))
            sig_terminate(aTHX_ SIGINT);
        return TRUE;

    case CTRL_BREAK_EVENT:
        /*  A CTRL+BREAK signal was received */
        if (do_raise(aTHX_ SIGBREAK))
            sig_terminate(aTHX_ SIGBREAK);
        return TRUE;

    case CTRL_LOGOFF_EVENT:
      /*  A signal that the system sends to all console processes when a user is logging
          off. This signal does not indicate which user is logging off, so no
          assumptions can be made.
       */
        break;
    case CTRL_SHUTDOWN_EVENT:
      /*  A signal that the system sends to all console processes when the system is
          shutting down.
       */
        if (do_raise(aTHX_ SIGTERM))
            sig_terminate(aTHX_ SIGTERM);
        return TRUE;
    default:
        break;
    }
    return FALSE;
}


#ifdef SET_INVALID_PARAMETER_HANDLER
#  include <crtdbg.h>
#endif

static void
ansify_path(void)
{
    size_t len;
    char *ansi_path;
    WCHAR *wide_path;
    WCHAR *wide_dir;

    /* fetch Unicode version of PATH */
    len = 2000;
    wide_path = (WCHAR*)win32_malloc(len*sizeof(WCHAR));
    while (wide_path) {
        size_t newlen = GetEnvironmentVariableW(L"PATH", wide_path, len);
        if (newlen == 0) {
            win32_free(wide_path);
            return;
        }
        if (newlen < len)
            break;
        len = newlen;
        wide_path = (WCHAR*)win32_realloc(wide_path, len*sizeof(WCHAR));
    }
    if (!wide_path)
        return;

    /* convert to ANSI pathnames */
    wide_dir = wide_path;
    ansi_path = NULL;
    while (wide_dir) {
        WCHAR *sep = wcschr(wide_dir, ';');
        char *ansi_dir;
        size_t ansi_len;
        size_t wide_len;

        if (sep)
            *sep++ = '\0';

        /* remove quotes around pathname */
        if (*wide_dir == '"')
            ++wide_dir;
        wide_len = wcslen(wide_dir);
        if (wide_len && wide_dir[wide_len-1] == '"')
            wide_dir[wide_len-1] = '\0';

        /* append ansi_dir to ansi_path */
        ansi_dir = win32_ansipath(wide_dir);
        ansi_len = strlen(ansi_dir);
        if (ansi_path) {
            size_t newlen = len + 1 + ansi_len;
            ansi_path = (char*)win32_realloc(ansi_path, newlen+1);
            if (!ansi_path)
                break;
            ansi_path[len] = ';';
            memcpy(ansi_path+len+1, ansi_dir, ansi_len+1);
            len = newlen;
        }
        else {
            len = ansi_len;
            ansi_path = (char*)win32_malloc(5+len+1);
            if (!ansi_path)
                break;
            memcpy(ansi_path, "PATH=", 5);
            memcpy(ansi_path+5, ansi_dir, len+1);
            len += 5;
        }
        win32_free(ansi_dir);
        wide_dir = sep;
    }

    if (ansi_path) {
        /* Update C RTL environ array.  This will only have full effect if
         * perl_parse() is later called with `environ` as the `env` argument.
         * Otherwise S_init_postdump_symbols() will overwrite PATH again.
         *
         * We do have to ansify() the PATH before Perl has been fully
         * initialized because S_find_script() uses the PATH when perl
         * is being invoked with the -S option.  This happens before %ENV
         * is initialized in S_init_postdump_symbols().
         *
         * XXX Is this a bug? Should S_find_script() use the environment
         * XXX passed in the `env` arg to parse_perl()?
         */
        putenv(ansi_path);
        /* Keep system environment in sync because S_init_postdump_symbols()
         * will not call mg_set() if it initializes %ENV from `environ`.
         */
        SetEnvironmentVariableA("PATH", ansi_path+5);
        win32_free(ansi_path);
    }
    win32_free(wide_path);
}

/* This hooks a function that is imported by the specified module. The hook is
 * local to that module. */
static bool
win32_hook_imported_function_in_module(
    HMODULE module, LPCSTR fun_name, FARPROC hook_ptr
)
{
    ULONG_PTR image_base = (ULONG_PTR)module;
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)image_base;
    PIMAGE_NT_HEADERS nt_headers
        = (PIMAGE_NT_HEADERS)(image_base + dos_header->e_lfanew);
    PIMAGE_OPTIONAL_HEADER opt_header = &nt_headers->OptionalHeader;

    PIMAGE_DATA_DIRECTORY data_dir = opt_header->DataDirectory;
    DWORD data_dir_len = opt_header->NumberOfRvaAndSizes;

    BOOL is_idt_present = data_dir_len > IMAGE_DIRECTORY_ENTRY_IMPORT
        && data_dir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress != 0;

    if (!is_idt_present)
        return FALSE;

    BOOL found = FALSE;

    /* Import Directory Table */
    PIMAGE_IMPORT_DESCRIPTOR idt = (PIMAGE_IMPORT_DESCRIPTOR)(
        image_base + data_dir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
    );

    for (; idt->Name != 0; ++idt) {
        /* Import Lookup Table */
        PIMAGE_THUNK_DATA ilt
            = (PIMAGE_THUNK_DATA)(image_base + idt->OriginalFirstThunk);
        /* Import Address Table */
        PIMAGE_THUNK_DATA iat
            = (PIMAGE_THUNK_DATA)(image_base + idt->FirstThunk);

        ULONG_PTR address_of_data;
        for (; address_of_data = ilt->u1.AddressOfData; ++ilt, ++iat) {
            /* Ordinal imports are quite rare, so skipping them will most likely
             * not cause any problems. */
            BOOL is_ordinal
                = address_of_data >> ((sizeof(address_of_data) * 8) - 1);

            if (is_ordinal)
                continue;

            LPCSTR name = (
                (PIMAGE_IMPORT_BY_NAME)(image_base + address_of_data)
            )->Name;

            if (strEQ(name, fun_name)) {
                DWORD old_protect = 0;
                BOOL succ = VirtualProtect(
                    &iat->u1.Function, sizeof(iat->u1.Function), PAGE_READWRITE,
                    &old_protect
                );
                if (!succ)
                    return FALSE;

                iat->u1.Function = (ULONG_PTR)hook_ptr;
                found = TRUE;

                VirtualProtect(
                    &iat->u1.Function, sizeof(iat->u1.Function), old_protect,
                    &old_protect
                );
                break;
            }
        }
    }

    return found;
}

typedef NTSTATUS (NTAPI *pNtQueryInformationFile_t)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, ULONG);
pNtQueryInformationFile_t pNtQueryInformationFile = NULL;

typedef BOOL (WINAPI *pCloseHandle)(HANDLE h);
static pCloseHandle CloseHandle_orig;

/* CloseHandle() that supports sockets. CRT uses mutexes during file operations,
 * so the lack of thread safety in this function isn't a problem. */
static BOOL WINAPI
my_CloseHandle(HANDLE h)
{
    /* In theory, passing a non-socket handle to closesocket() is fine. It
     * should return a WSAENOTSOCK error, which is easy to recover from.
     * However, we should avoid doing that because it's not that simple in
     * practice. For instance, it can deadlock on a handle to a stuck pipe (see:
     * https://github.com/Perl/perl5/issues/19963).
     *
     * There's no foolproof way to tell if a handle is a socket (mostly because
     * of the non-IFS sockets), but in some cases we can tell if a handle
     * is definitely *not* a socket.
     */

    /* GetFileType() always returns FILE_TYPE_PIPE for sockets. */
    BOOL maybe_socket = (GetFileType(h) == FILE_TYPE_PIPE);

    if (maybe_socket && pNtQueryInformationFile) {
        IO_STATUS_BLOCK isb;
        struct {
            ULONG name_len;
            WCHAR name[100];
        } volume = {0};

        /* There are many ways to tell a named pipe from a socket, but almost
         * all of them can deadlock on a handle to a stuck pipe (like in the
         * bug ticket mentioned above). According to my tests,
         * FileVolumeNameInfomation is the only relevant function that doesn't
         * suffer from this problem.
         *
         * It's undocumented and it requires Windows 10, so on older systems
         * we always pass pipes to closesocket().
         */
        NTSTATUS s = pNtQueryInformationFile(
            h, &isb, &volume, sizeof(volume), 58 /* FileVolumeNameInformation */
        );
        if (NT_SUCCESS(s)) {
            maybe_socket = (_wcsnicmp(
                volume.name, L"\\Device\\NamedPipe", C_ARRAY_LENGTH(volume.name)
            ) != 0);
        }
    }

    if (maybe_socket)
        if (closesocket((SOCKET)h) == 0)
            return TRUE;
        else if (WSAGetLastError() != WSAENOTSOCK)
            return FALSE;

    return CloseHandle_orig(h);
}

/* Hook CloseHandle() inside CRT so its functions like _close() or
 * _dup2() can close sockets properly. */
static void
win32_hook_closehandle_in_crt()
{
    /* Get the handle to the CRT module basing on the address of _close()
     * function. */
    HMODULE crt_handle;
    BOOL succ = GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)_close,
        &crt_handle
    );
    if (!succ)
        return;

    CloseHandle_orig = (pCloseHandle)GetProcAddress(
        GetModuleHandleA("kernel32.dll"), "CloseHandle"
    );
    if (!CloseHandle_orig)
        return;

    win32_hook_imported_function_in_module(
        crt_handle, "CloseHandle", (FARPROC)my_CloseHandle
    );

    pNtQueryInformationFile = (pNtQueryInformationFile_t)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "NtQueryInformationFile"
    );
}

/* Remove the hook installed by win32_hook_closehandle_crt(). This is needed in
 * case the Perl DLL is unloaded, which would cause the hook become invalid.
 * This can happen in embedded Perls, for example in mod_perl. */
static void
win32_unhook_closehandle_in_crt()
{
    if (!CloseHandle_orig)
        return;

    HMODULE crt_handle;
    BOOL succ = GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)_close,
        &crt_handle
    );
    if (!succ)
        return;

    win32_hook_imported_function_in_module(
        crt_handle, "CloseHandle", (FARPROC)CloseHandle_orig
    );

    CloseHandle_orig = NULL;
}

void
Perl_win32_init(int *argcp, char ***argvp)
{
#ifdef SET_INVALID_PARAMETER_HANDLER
    _invalid_parameter_handler oldHandler, newHandler;
    newHandler = my_invalid_parameter_handler;
    oldHandler = _set_invalid_parameter_handler(newHandler);
    _CrtSetReportMode(_CRT_ASSERT, 0);
#endif
    /* Disable floating point errors, Perl will trap the ones we
     * care about.  VC++ RTL defaults to switching these off
     * already, but some RTLs don't.  Since we don't
     * want to be at the vendor's whim on the default, we set
     * it explicitly here.
     */
#if !defined(__GNUC__)
    _control87(MCW_EM, MCW_EM);
#endif
    MALLOC_INIT;

    /* When the manifest resource requests Common-Controls v6 then
     * user32.dll no longer registers all the Windows classes used for
     * standard controls but leaves some of them to be registered by
     * comctl32.dll.  InitCommonControls() doesn't do anything but calling
     * it makes sure comctl32.dll gets loaded into the process and registers
     * the standard control classes.  Without this even normal Windows APIs
     * like MessageBox() can fail under some versions of Windows XP.
     */
    InitCommonControls();

    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    g_osver.dwOSVersionInfoSize = sizeof(g_osver);
    GetVersionEx(&g_osver);

    win32_hook_closehandle_in_crt();

    ansify_path();

#ifndef WIN32_NO_REGISTRY
    {
        LONG retval;
        retval = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Perl", 0, KEY_READ, &HKCU_Perl_hnd);
        if (retval != ERROR_SUCCESS) {
            HKCU_Perl_hnd = NULL;
        }
        retval = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Perl", 0, KEY_READ, &HKLM_Perl_hnd);
        if (retval != ERROR_SUCCESS) {
            HKLM_Perl_hnd = NULL;
        }
    }
#endif

    {
        FILETIME ft;
        if (!SystemTimeToFileTime(&time_t_epoch_base_systemtime,
                                  &ft)) {
            fprintf(stderr, "panic: cannot convert base system time to filetime\n"); /* no interp */
            exit(1);
        }
        time_t_epoch_base_filetime.LowPart  = ft.dwLowDateTime;
        time_t_epoch_base_filetime.HighPart = ft.dwHighDateTime;
    }

    MUTEX_INIT(&win32_read_console_mutex);
}

void
Perl_win32_term(void)
{
    HINTS_REFCNT_TERM;
    OP_REFCNT_TERM;
    PERLIO_TERM;
    MALLOC_TERM;
    LOCALE_TERM;
    ENV_TERM;
#ifndef WIN32_NO_REGISTRY
    /* handles might be NULL, RegCloseKey then returns ERROR_INVALID_HANDLE
       but no point of checking and we can't die() at this point */
    RegCloseKey(HKLM_Perl_hnd);
    RegCloseKey(HKCU_Perl_hnd);
    /* the handles are in an undefined state until the next PERL_SYS_INIT3 */
#endif
    win32_unhook_closehandle_in_crt();
}

void
win32_get_child_IO(child_IO_table* ptbl)
{
    ptbl->childStdIn	= GetStdHandle(STD_INPUT_HANDLE);
    ptbl->childStdOut	= GetStdHandle(STD_OUTPUT_HANDLE);
    ptbl->childStdErr	= GetStdHandle(STD_ERROR_HANDLE);
}

Sighandler_t
win32_signal(int sig, Sighandler_t subcode)
{
    dTHXa(NULL);
    if (sig < SIG_SIZE) {
        int save_errno = errno;
        Sighandler_t result;
#ifdef SET_INVALID_PARAMETER_HANDLER
        /* Silence our invalid parameter handler since we expect to make some
         * calls with invalid signal numbers giving a SIG_ERR result. */
        BOOL oldvalue = set_silent_invalid_parameter_handler(TRUE);
#endif
        result = signal(sig, subcode);
#ifdef SET_INVALID_PARAMETER_HANDLER
        set_silent_invalid_parameter_handler(oldvalue);
#endif
        aTHXa(PERL_GET_THX);
        if (result == SIG_ERR) {
            result = w32_sighandler[sig];
            errno = save_errno;
        }
        w32_sighandler[sig] = subcode;
        return result;
    }
    else {
        errno = EINVAL;
        return SIG_ERR;
    }
}

/* The PerlMessageWindowClass's WindowProc */
LRESULT CALLBACK
win32_message_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return win32_process_message(hwnd, msg, wParam, lParam) ?
        0 : DefWindowProc(hwnd, msg, wParam, lParam);
}

/* The real message handler. Can be called with
 * hwnd == NULL to process our thread messages. Returns TRUE for any messages
 * that it processes */
static LRESULT
win32_process_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    /* BEWARE. The context retrieved using dTHX; is the context of the
     * 'parent' thread during the CreateWindow() phase - i.e. for all messages
     * up to and including WM_CREATE.  If it ever happens that you need the
     * 'child' context before this, then it needs to be passed into
     * win32_create_message_window(), and passed to the WM_NCCREATE handler
     * from the lparam of CreateWindow().  It could then be stored/retrieved
     * using [GS]etWindowLongPtr(... GWLP_USERDATA ...), possibly eliminating
     * the dTHX calls here. */
    /* XXX For now it is assumed that the overhead of the dTHX; for what
     * are relativley infrequent code-paths, is better than the added
     * complexity of getting the correct context passed into
     * win32_create_message_window() */
    dTHX;

    switch(msg) {

#ifdef USE_ITHREADS
        case WM_USER_MESSAGE: {
            long child = find_pseudo_pid(aTHX_ (int)wParam);
            if (child >= 0) {
                w32_pseudo_child_message_hwnds[child] = (HWND)lParam;
                return 1;
            }
            break;
        }
#endif

        case WM_USER_KILL: {
            /* We use WM_USER_KILL to fake kill() with other signals */
            int sig = (int)wParam;
            if (do_raise(aTHX_ sig))
                sig_terminate(aTHX_ sig);

            return 1;
        }

        case WM_TIMER: {
            /* alarm() is a one-shot but SetTimer() repeats so kill it */
            if (w32_timerid && w32_timerid==(UINT)wParam) {
                KillTimer(w32_message_hwnd, w32_timerid);
                w32_timerid=0;

                /* Now fake a call to signal handler */
                if (do_raise(aTHX_ 14))
                    sig_terminate(aTHX_ 14);

                return 1;
            }
            break;
        }

        default:
            break;

    } /* switch */

    /* Above or other stuff may have set a signal flag, and we may not have
     * been called from win32_async_check() (e.g. some other GUI's message
     * loop.  BUT DON'T dispatch signals here: If someone has set a SIGALRM
     * handler that die's, and the message loop that calls here is wrapped
     * in an eval, then you may well end up with orphaned windows - signals
     * are dispatched by win32_async_check() */

    return 0;
}

void
win32_create_message_window_class(void)
{
    /* create the window class for "message only" windows */
    WNDCLASS wc;

    Zero(&wc, 1, wc);
    wc.lpfnWndProc = win32_message_window_proc;
    wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wc.lpszClassName = "PerlMessageWindowClass";

    /* second and subsequent calls will fail, but class
     * will already be registered */
    RegisterClass(&wc);
}

HWND
win32_create_message_window(void)
{
    win32_create_message_window_class();
    return CreateWindow("PerlMessageWindowClass", "PerlMessageWindow",
                        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
}

#ifdef HAVE_INTERP_INTERN

static void
win32_csighandler(int sig)
{
#if 0
    dTHXa(PERL_GET_SIG_CONTEXT);
    Perl_warn(aTHX_ "Got signal %d",sig);
#endif
    /* Does nothing */
}

#if defined(__MINGW32__) && defined(__cplusplus)
#define CAST_HWND__(x) (HWND__*)(x)
#else
#define CAST_HWND__(x) x
#endif

void
Perl_sys_intern_init(pTHX)
{
    int i;

    w32_perlshell_tokens	= NULL;
    w32_perlshell_vec		= (char**)NULL;
    w32_perlshell_items		= 0;
    w32_fdpid			= newAV();
    Newx(w32_children, 1, child_tab);
    w32_num_children		= 0;
#  ifdef USE_ITHREADS
    w32_pseudo_id		= 0;
    Newx(w32_pseudo_children, 1, pseudo_child_tab);
    w32_num_pseudo_children	= 0;
#  endif
    w32_timerid                 = 0;
    w32_message_hwnd            = CAST_HWND__(INVALID_HANDLE_VALUE);
    w32_poll_count              = 0;
    for (i=0; i < SIG_SIZE; i++) {
        w32_sighandler[i] = SIG_DFL;
    }
#  ifdef MULTIPLICITY
    if (my_perl == PL_curinterp) {
#  else
    {
#  endif
        /* Force C runtime signal stuff to set its console handler */
        signal(SIGINT,win32_csighandler);
        signal(SIGBREAK,win32_csighandler);

        /* We spawn asynchronous processes with the CREATE_NEW_PROCESS_GROUP
         * flag.  This has the side-effect of disabling Ctrl-C events in all
         * processes in this group.
         * We re-enable Ctrl-C handling by calling SetConsoleCtrlHandler()
         * with a NULL handler.
         */
        SetConsoleCtrlHandler(NULL,FALSE);

        /* Push our handler on top */
        SetConsoleCtrlHandler(win32_ctrlhandler,TRUE);
    }
}

void
Perl_sys_intern_clear(pTHX)
{

    Safefree(w32_perlshell_tokens);
    Safefree(w32_perlshell_vec);
    /* NOTE: w32_fdpid is freed by sv_clean_all() */
    Safefree(w32_children);
    if (w32_timerid) {
        KillTimer(w32_message_hwnd, w32_timerid);
        w32_timerid = 0;
    }
    if (w32_message_hwnd != NULL && w32_message_hwnd != INVALID_HANDLE_VALUE)
        DestroyWindow(w32_message_hwnd);
#  ifdef MULTIPLICITY
    if (my_perl == PL_curinterp) {
#  else
    {
#  endif
        SetConsoleCtrlHandler(win32_ctrlhandler,FALSE);
    }
#  ifdef USE_ITHREADS
    Safefree(w32_pseudo_children);
#  endif
}

#  ifdef USE_ITHREADS

void
Perl_sys_intern_dup(pTHX_ struct interp_intern *src, struct interp_intern *dst)
{
    PERL_ARGS_ASSERT_SYS_INTERN_DUP;

    dst->perlshell_tokens	= NULL;
    dst->perlshell_vec		= (char**)NULL;
    dst->perlshell_items	= 0;
    dst->fdpid			= newAV();
    Newxz(dst->children, 1, child_tab);
    dst->pseudo_id		= 0;
    Newxz(dst->pseudo_children, 1, pseudo_child_tab);
    dst->timerid                = 0;
    dst->message_hwnd		= CAST_HWND__(INVALID_HANDLE_VALUE);
    dst->poll_count             = 0;
    Copy(src->sigtable,dst->sigtable,SIG_SIZE,Sighandler_t);
}
#  endif /* USE_ITHREADS */
#endif /* HAVE_INTERP_INTERN */
