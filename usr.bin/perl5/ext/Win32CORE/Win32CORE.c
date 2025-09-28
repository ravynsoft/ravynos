/*    Win32CORE.c
 *
 *    Copyright (C) 2007 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(__CYGWIN__) && !defined(USEIMPORTLIB)
  #undef WIN32
#endif
#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#if defined(__CYGWIN__) && !defined(USEIMPORTLIB)
  #define EXTCONST extern const
#endif
#include "perl.h"
#include "XSUB.h"


XS(w32_CORE_all){
    /* I'd use dSAVE_ERRNO() here, but it doesn't save the Win32 error code
     * under cygwin, if that changes this code should change to use that.
     */
    int saved_errno = errno;
    DWORD err = GetLastError();
    /* capture the XSANY value before Perl_load_module, the CV's any member will
     * be overwritten by Perl_load_module and subsequent newXSes or pure perl
     * subs
     */
    const char *function  = (const char *) XSANY.any_ptr;
    Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT, newSVpvs("Win32"), newSVnv(0.27));
    SetLastError(err);
    errno = saved_errno;
    /* mark and SP from caller are passed through unchanged */
    call_pv(function, GIMME_V);
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Win32CORE)
{
    /* This function only exists because writemain.SH, lib/ExtUtils/Embed.pm
     * and win32/buildext.pl will all generate references to it.  The function
     * should never be called though, as Win32CORE.pm doesn't use DynaLoader.
     */
    PERL_UNUSED_ARG(cv);
}

EXTERN_C
#if !defined(__CYGWIN__) || defined(USEIMPORTLIB)
__declspec(dllexport)
#endif
void
init_Win32CORE(pTHX)
{
    /* This function is called from init_os_extras().  The Perl interpreter
     * is not yet fully initialized, so don't do anything fancy in here.
     */

    static const struct {
        char Win32__GetCwd [sizeof("Win32::GetCwd")];
        char Win32__SetCwd [sizeof("Win32::SetCwd")];
        char Win32__GetNextAvailDrive [sizeof("Win32::GetNextAvailDrive")];
        char Win32__GetLastError [sizeof("Win32::GetLastError")];
        char Win32__SetLastError [sizeof("Win32::SetLastError")];
        char Win32__LoginName [sizeof("Win32::LoginName")];
        char Win32__NodeName [sizeof("Win32::NodeName")];
        char Win32__DomainName [sizeof("Win32::DomainName")];
        char Win32__FsType [sizeof("Win32::FsType")];
        char Win32__GetOSVersion [sizeof("Win32::GetOSVersion")];
        char Win32__IsWinNT [sizeof("Win32::IsWinNT")];
        char Win32__IsWin95 [sizeof("Win32::IsWin95")];
        char Win32__FormatMessage [sizeof("Win32::FormatMessage")];
        char Win32__Spawn [sizeof("Win32::Spawn")];
        char Win32__GetTickCount [sizeof("Win32::GetTickCount")];
        char Win32__GetShortPathName [sizeof("Win32::GetShortPathName")];
        char Win32__GetFullPathName [sizeof("Win32::GetFullPathName")];
        char Win32__GetLongPathName [sizeof("Win32::GetLongPathName")];
        char Win32__CopyFile [sizeof("Win32::CopyFile")];
        char Win32__Sleep [sizeof("Win32::Sleep")];
    } fnname_table = {
        "Win32::GetCwd",
        "Win32::SetCwd",
        "Win32::GetNextAvailDrive",
        "Win32::GetLastError",
        "Win32::SetLastError",
        "Win32::LoginName",
        "Win32::NodeName",
        "Win32::DomainName",
        "Win32::FsType",
        "Win32::GetOSVersion",
        "Win32::IsWinNT",
        "Win32::IsWin95",
        "Win32::FormatMessage",
        "Win32::Spawn",
        "Win32::GetTickCount",
        "Win32::GetShortPathName",
        "Win32::GetFullPathName",
        "Win32::GetLongPathName",
        "Win32::CopyFile",
        "Win32::Sleep"
    };

    static const unsigned char fnname_lens [] = {
        sizeof("Win32::GetCwd"),
        sizeof("Win32::SetCwd"),
        sizeof("Win32::GetNextAvailDrive"),
        sizeof("Win32::GetLastError"),
        sizeof("Win32::SetLastError"),
        sizeof("Win32::LoginName"),
        sizeof("Win32::NodeName"),
        sizeof("Win32::DomainName"),
        sizeof("Win32::FsType"),
        sizeof("Win32::GetOSVersion"),
        sizeof("Win32::IsWinNT"),
        sizeof("Win32::IsWin95"),
        sizeof("Win32::FormatMessage"),
        sizeof("Win32::Spawn"),
        sizeof("Win32::GetTickCount"),
        sizeof("Win32::GetShortPathName"),
        sizeof("Win32::GetFullPathName"),
        sizeof("Win32::GetLongPathName"),
        sizeof("Win32::CopyFile"),
        sizeof("Win32::Sleep")
    };
    const unsigned char * len = (const unsigned char *)&fnname_lens;
    const char * function = (char *)&fnname_table;
    while (function < (char *)&fnname_table + sizeof(fnname_table)) {
        const char * const file = __FILE__;
        CV * const cv = newXS(function, w32_CORE_all, file);
        XSANY.any_ptr = (void *)function;
        function += *len++;
    }


    /* Don't forward Win32::SetChildShowWindow().  It accesses the internal variable
     * w32_showwindow in thread_intern and is therefore not implemented in Win32.xs.
     */
    /* newXS("Win32::SetChildShowWindow", w32_SetChildShowWindow, file); */
}
