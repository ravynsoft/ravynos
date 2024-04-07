/*
 * Cygwin extras
 */

#define PERLIO_NOT_STDIO 0
#include "EXTERN.h"
#include "perl.h"
#undef USE_DYNAMIC_LOADING
#include "XSUB.h"

#include <unistd.h>
#include <process.h>
#include <sys/cygwin.h>
#include <cygwin/version.h>
#include <mntent.h>
#include <alloca.h>
#include <dlfcn.h>
#if (CYGWIN_VERSION_API_MINOR >= 181)
#include <wchar.h>
#endif

#define PATH_LEN_GUESS (260 + 1001)

/*
 * pp_system() implemented via spawn()
 * - more efficient and useful when embedding Perl in non-Cygwin apps
 * - code mostly borrowed from djgpp.c
 */
static int
do_spawnvp (const char *path, const char * const *argv)
{
    dTHX;
    Sigsave_t ihand,qhand;
    int childpid, result, status;

    rsignal_save(SIGINT, (Sighandler_t) SIG_IGN, &ihand);
    rsignal_save(SIGQUIT, (Sighandler_t) SIG_IGN, &qhand);
    childpid = spawnvp(_P_NOWAIT,path,argv);
    if (childpid < 0) {
        status = -1;
        if(ckWARN(WARN_EXEC))
            Perl_warner(aTHX_ packWARN(WARN_EXEC),"Can't spawn \"%s\": %s",
                    path,Strerror (errno));
    } else {
        do {
            result = wait4pid(childpid, &status, 0);
        } while (result == -1 && errno == EINTR);
        if(result < 0)
            status = -1;
    }
    (void)rsignal_restore(SIGINT, &ihand);
    (void)rsignal_restore(SIGQUIT, &qhand);
    return status;
}

int
do_aspawn (SV *really, void **mark, void **sp)
{
    dTHX;
    int  rc;
    char const **a;
    char *tmps,**argv;
    STRLEN n_a;

    if (sp<=mark)
        return -1;
    argv=(char**) alloca ((sp-mark+3)*sizeof (char*));
    a=(char const **)argv;

    while (++mark <= sp)
        if (*mark)
            *a++ = SvPVx((SV *)*mark, n_a);
        else
            *a++ = "";
    *a = (char*)NULL;

    if (argv[0][0] != '/' && argv[0][0] != '\\'
        && !(argv[0][0] && argv[0][1] == ':'
        && (argv[0][2] == '/' || argv[0][2] != '\\'))
     ) /* will swawnvp use PATH? */
         TAINT_ENV();	/* testing IFS here is overkill, probably */

    if (really && *(tmps = SvPV(really, n_a)))
        rc=do_spawnvp (tmps,(const char * const *)argv);
    else
        rc=do_spawnvp (argv[0],(const char *const *)argv);

    return rc;
}

int
do_spawn (char *cmd)
{
    dTHX;
    char const **argv, **a;
    char *s;
    char const *metachars = "$&*(){}[]'\";\\?>|<~`\n";
    const char *command[4];
    int result;

    ENTER;
    while (*cmd && isSPACE(*cmd))
        cmd++;

    if (strBEGINs (cmd,"/bin/sh") && isSPACE (cmd[7]))
        cmd+=5;

    /* save an extra exec if possible */
    /* see if there are shell metacharacters in it */
    if (strstr (cmd,"..."))
        goto doshell;
    if (*cmd=='.' && isSPACE (cmd[1]))
        goto doshell;
    if (strBEGINs (cmd,"exec") && isSPACE (cmd[4]))
        goto doshell;
    for (s=cmd; *s && isALPHA (*s); s++) ;	/* catch VAR=val gizmo */
    if (*s=='=')
        goto doshell;

    for (s=cmd; *s; s++)
        if (strchr (metachars,*s))
        {
            if (*s=='\n' && s[1]=='\0')
            {
                *s='\0';
                break;
            }
        doshell:
            command[0] = "sh";
            command[1] = "-c";
            command[2] = cmd;
            command[3] = NULL;

            result = do_spawnvp("sh",command);
            goto leave;
        }

    Newx (argv, (s-cmd)/2+2, const char*);
    SAVEFREEPV(argv);
    cmd=savepvn (cmd,s-cmd);
    SAVEFREEPV(cmd);
    a=argv;
    for (s=cmd; *s;) {
        while (*s && isSPACE (*s)) s++;
        if (*s)
            *(a++)=s;
        while (*s && !isSPACE (*s)) s++;
        if (*s)
            *s++='\0';
    }
    *a = (char*)NULL;
    if (!argv[0])
        result = -1;
    else
        result = do_spawnvp(argv[0],(const char * const *)argv);
leave:
    LEAVE;
    return result;
}

#if (CYGWIN_VERSION_API_MINOR >= 181)
char*
wide_to_utf8(const wchar_t *wsrc)
{
    dTHX;
    const Size_t wlen = (wcslen(wsrc) + 1) * sizeof(wchar_t);

    /* Max expansion factor is 3/2 */
    Size_t blen = wlen * 3 / 2;

    char *buf = (char *) safemalloc(blen);

    utf16_to_utf8((U8 *) wsrc, buf, wlen, &blen);

    return buf;
}

wchar_t*
utf8_to_wide_extra_len(const char *buf, Size_t *extra_len)
{
    /* Return the conversion to UTF-16 of the UTF-8 string 'buf'
     * (terminated by a NUL), making sure to have space for at least *extra_len
     * extra (wide) characters in the result.  The result must be freed by the
     * caller when no longer needed */

    dTHX;
    Size_t len = strlen(buf) + extra_len + 1;

    /* Max expansion factor is sizeof(wchar_t) */
    Size_t wlen = sizeof(wchar_t) * len;

    wchar_t* wsrc = (wchar_t *) safemalloc(wlen);

    utf8_to_utf16(buf, (U8 *) wsrc, len, &wlen);

    return wsrc;
}

wchar_t*
utf8_to_wide(const char *buf)
{
    Size_t extra_len = 0;

    return utf8_to_wide_extra_len(buf, &extra_len);
}

#endif /* cygwin 1.7 */

/* see also Cwd.pm */
XS(Cygwin_cwd)
{
    dXSARGS;
    char *cwd;

    /* See https://github.com/Perl/perl5/issues/8345
       There is Cwd->cwd() usage in the wild, and previous versions didn't die.
     */
    if(items > 1)
        Perl_croak(aTHX_ "Usage: Cwd::cwd()");
    if((cwd = getcwd(NULL, -1))) {
        ST(0) = sv_2mortal(newSVpv(cwd, 0));
        free(cwd);
        SvTAINTED_on(ST(0));
        XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

XS(XS_Cygwin_pid_to_winpid)
{
    dXSARGS;
    dXSTARG;
    pid_t pid, RETVAL;

    if (items != 1)
        Perl_croak(aTHX_ "Usage: Cygwin::pid_to_winpid(pid)");

    pid = (pid_t)SvIV(ST(0));

    if ((RETVAL = cygwin_internal(CW_CYGWIN_PID_TO_WINPID, pid)) > 0) {
        XSprePUSH; PUSHi((IV)RETVAL);
        XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

XS(XS_Cygwin_winpid_to_pid)
{
    dXSARGS;
    dXSTARG;
    pid_t pid, RETVAL;

    if (items != 1)
        Perl_croak(aTHX_ "Usage: Cygwin::winpid_to_pid(pid)");

    pid = (pid_t)SvIV(ST(0));

#if (CYGWIN_VERSION_API_MINOR >= 181)
    RETVAL = cygwin_winpid_to_pid(pid);
#else
    RETVAL = cygwin32_winpid_to_pid(pid);
#endif
    if (RETVAL > 0) {
        XSprePUSH; PUSHi((IV)RETVAL);
        XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

/* The conversion between Posix and Windows paths is essentially the same in
 * either direction, so a common function is used, with which direction passed
 * in.
 *
 * These numbers are chosen so can be or'd with absolute flag to get 0..3 */
typedef enum {
    to_posix = 0,
    to_win   = 2
} direction_t;

static void
S_convert_path_common(pTHX_ const direction_t direction)
{
    dXSARGS;
    bool absolute_flag = 0;
    STRLEN len;
    int err = 0;
    char *src_path;
    char *converted_path;
    int isutf8 = 0;

    if (items < 1 || items > 2) {
        char *name = (direction == to_posix)
                     ? "win::win_to_posix_path"
                     : "posix_to_win_path";
        Perl_croak(aTHX_ "Usage: Cygwin::%s(pathname, [absolute])", name);
    }

    src_path = SvPVx(ST(0), len);
    if (items == 2)
        absolute_flag = SvTRUE(ST(1));

    if (!len)
        Perl_croak(aTHX_ "can't convert empty path");
    isutf8 = SvUTF8(ST(0));

#if (CYGWIN_VERSION_API_MINOR >= 181)
    /* Check utf8 flag and use wide api then.
       Size calculation: On overflow let cygwin_conv_path calculate the final size.
     */
    if (isutf8) {
        int what =  ((absolute_flag) ? 0 : CCP_RELATIVE)
                  | ((direction == to_posix)
                     ? CCP_WIN_W_TO_POSIX
                     : CCP_POSIX_TO_WIN_W);
        STRLEN wlen;
        wchar_t *wsrc = NULL;       /* The source, as a wchar_t */
        wchar_t *wconverted = NULL; /* wsrc, converted to the destination */

        /* ptr to either wsrc, or under BYTES, the src_path so can have common
         * code below */
        wchar_t *which_src = (wchar_t *) src_path;

        if (LIKELY(! IN_BYTES)) {    /* Normal case, convert UTF-8 to UTF-16 */
            wlen = PATH_LEN_GUESS;
            wsrc = utf8_to_wide_extra_len(src_path, &wlen);
            which_src = wsrc;
        }
        else { /* use bytes; assume already UTF-16 encoded bytestream */
            wlen = sizeof(wchar_t) * (len + PATH_LEN_GUESS);
        }

        if (LIKELY(wlen > 0)) { /* Make sure didn't get an error */
            wconverted = (wchar_t *) safemalloc(wlen);
            err = cygwin_conv_path(what, which_src, wconverted, wlen);
        }

        if (err == ENOSPC) { /* our space assumption was wrong, not enough space */
            int newlen = cygwin_conv_path(what, which_src, wconverted, 0);
            wconverted = (wchar_t *) realloc(&wconverted, newlen);
            err = cygwin_conv_path(what, which_src, wconverted, newlen);
        }

        converted_path = wide_to_utf8(wconverted);

        safefree(wconverted);
        safefree(wsrc);
    } else {
        int what =  ((absolute_flag) ? 0 : CCP_RELATIVE)
                  | ((direction == to_posix)
                     ? CCP_WIN_A_TO_POSIX
                     : CCP_POSIX_TO_WIN_A);

        converted_path = (char *) safemalloc (len + PATH_LEN_GUESS);
        err = cygwin_conv_path(what, src_path, converted_path, len + PATH_LEN_GUESS);
        if (err == ENOSPC) { /* our space assumption was wrong, not enough space */
            int newlen = cygwin_conv_path(what, src_path, converted_path, 0);
            converted_path = (char *) realloc(&converted_path, newlen);
            err = cygwin_conv_path(what, src_path, converted_path, newlen);
        }
    }

#else
    converted_path = (char *) safemalloc (len + PATH_LEN_GUESS);

    switch (absolute_flag | direction) {
      case (1|to_posix):
        err = cygwin_conv_to_full_posix_path(src_path, converted_path);
        break;
      case (0|to_posix):
        err = cygwin_conv_to_posix_path(src_path, converted_path);
        break;
      case (1|to_win):
        err = cygwin_conv_to_full_win32_path(src_path, converted_path);
        break;
      case (0|to_win):
        err = cygwin_conv_to_win32_path(src_path, converted_path);
        break;
    }

#endif

    if (!err) {
        EXTEND(SP, 1);
        ST(0) = sv_2mortal(newSVpv(converted_path, 0));
        if (isutf8) { /* src was utf-8, so result should also */
            /* TODO: convert ANSI (local windows encoding) to utf-8 on cygwin-1.5 */
            SvUTF8_on(ST(0));
        }
        safefree(converted_path);
        XSRETURN(1);
    } else {
        safefree(converted_path);
        XSRETURN_UNDEF;
    }
}

XS(XS_Cygwin_win_to_posix_path)
{
    S_convert_path_common(aTHX_ to_posix);
}

XS(XS_Cygwin_posix_to_win_path)
{
    S_convert_path_common(aTHX_ to_win);
}

XS(XS_Cygwin_mount_table)
{
    dXSARGS;
    struct mntent *mnt;

    if (items != 0)
        Perl_croak(aTHX_ "Usage: Cygwin::mount_table");
    /* => array of [mnt_dir mnt_fsname mnt_type mnt_opts] */

    setmntent (0, 0);
    while ((mnt = getmntent (0))) {
        AV* av = newAV();
        av_push(av, newSVpvn(mnt->mnt_dir, strlen(mnt->mnt_dir)));
        av_push(av, newSVpvn(mnt->mnt_fsname, strlen(mnt->mnt_fsname)));
        av_push(av, newSVpvn(mnt->mnt_type, strlen(mnt->mnt_type)));
        av_push(av, newSVpvn(mnt->mnt_opts, strlen(mnt->mnt_opts)));
        XPUSHs(sv_2mortal(newRV_noinc((SV*)av)));
    }
    endmntent (0);
    PUTBACK;
}

XS(XS_Cygwin_mount_flags)
{
    dXSARGS;
    char *pathname;
    char flags[PATH_MAX];
    flags[0] = '\0';

    if (items != 1)
        Perl_croak(aTHX_ "Usage: Cygwin::mount_flags( mnt_dir | '/cygdrive' )");

    pathname = SvPV_nolen(ST(0));

    if (strEQ(pathname, "/cygdrive")) {
        char user[PATH_MAX];
        char system[PATH_MAX];
        char user_flags[PATH_MAX];
        char system_flags[PATH_MAX];

        cygwin_internal (CW_GET_CYGDRIVE_INFO, user, system,
                         user_flags, system_flags);

        if (strlen(user) > 0) {
            sprintf(flags, "%s,cygdrive,%s", user_flags, user);
        } else {
            sprintf(flags, "%s,cygdrive,%s", system_flags, system);
        }

        ST(0) = sv_2mortal(newSVpv(flags, 0));
        XSRETURN(1);

    } else {
        struct mntent *mnt;
        int found = 0;
        setmntent (0, 0);
        while ((mnt = getmntent (0))) {
            if (strEQ(pathname, mnt->mnt_dir)) {
                strcpy(flags, mnt->mnt_type);
                if (strlen(mnt->mnt_opts) > 0) {
                    strcat(flags, ",");
                    strcat(flags, mnt->mnt_opts);
                }
                found++;
                break;
            }
        }
        endmntent (0);

        /* Check if arg is the current volume moint point if not default,
         * and then use CW_GET_CYGDRIVE_INFO also.
         */
        if (!found) {
            char user[PATH_MAX];
            char system[PATH_MAX];
            char user_flags[PATH_MAX];
            char system_flags[PATH_MAX];

            cygwin_internal (CW_GET_CYGDRIVE_INFO, user, system,
                             user_flags, system_flags);

            if (strlen(user) > 0) {
                if (strNE(user,pathname)) {
                    sprintf(flags, "%s,cygdrive,%s", user_flags, user);
                    found++;
                }
            } else {
                if (strNE(user,pathname)) {
                    sprintf(flags, "%s,cygdrive,%s", system_flags, system);
                    found++;
                }
            }
        }
        if (found) {
            ST(0) = sv_2mortal(newSVpv(flags, 0));
            XSRETURN(1);
        } else {
            XSRETURN_UNDEF;
        }
    }
}

XS(XS_Cygwin_is_binmount)
{
    dXSARGS;
    char *pathname;

    if (items != 1)
        Perl_croak(aTHX_ "Usage: Cygwin::is_binmount(pathname)");

    pathname = SvPV_nolen(ST(0));

    ST(0) = boolSV(cygwin_internal(CW_GET_BINMODE, pathname));
    XSRETURN(1);
}

XS(XS_Cygwin_sync_winenv){ cygwin_internal(CW_SYNC_WINENV); }

void
init_os_extras(void)
{
    dTHX;
    char const *file = __FILE__;
    void *handle;

    newXS("Cwd::cwd", Cygwin_cwd, file);
    newXSproto("Cygwin::winpid_to_pid", XS_Cygwin_winpid_to_pid, file, "$");
    newXSproto("Cygwin::pid_to_winpid", XS_Cygwin_pid_to_winpid, file, "$");
    newXSproto("Cygwin::win_to_posix_path", XS_Cygwin_win_to_posix_path, file, "$;$");
    newXSproto("Cygwin::posix_to_win_path", XS_Cygwin_posix_to_win_path, file, "$;$");
    newXSproto("Cygwin::mount_table", XS_Cygwin_mount_table, file, "");
    newXSproto("Cygwin::mount_flags", XS_Cygwin_mount_flags, file, "$");
    newXSproto("Cygwin::is_binmount", XS_Cygwin_is_binmount, file, "$");
    newXS("Cygwin::sync_winenv", XS_Cygwin_sync_winenv, file);

    /* Initialize Win32CORE if it has been statically linked. */
    handle = dlopen(NULL, RTLD_LAZY);
    if (handle) {
        void (*pfn_init)(pTHX);
        pfn_init = (void (*)(pTHX))dlsym(handle, "init_Win32CORE");
        if (pfn_init)
            pfn_init(aTHX);
        dlclose(handle);
    }
}
