/*    pp_sys.c
 *
 *    Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
 *    2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * But only a short way ahead its floor and the walls on either side were
 * cloven by a great fissure, out of which the red glare came, now leaping
 * up, now dying down into darkness; and all the while far below there was
 * a rumour and a trouble as of great engines throbbing and labouring.
 *
 *     [p.945 of _The Lord of the Rings_, VI/iii: "Mount Doom"]
 */

/* This file contains system pp ("push/pop") functions that
 * execute the opcodes that make up a perl program. A typical pp function
 * expects to find its arguments on the stack, and usually pushes its
 * results onto the stack, hence the 'pp' terminology. Each OP structure
 * contains a pointer to the relevant pp_foo() function.
 *
 * By 'system', we mean ops which interact with the OS, such as pp_open().
 */

#include "EXTERN.h"
#define PERL_IN_PP_SYS_C
#include "perl.h"
#include "time64.h"

#ifdef I_SHADOW
/* Shadow password support for solaris - pdo@cs.umd.edu
 * Not just Solaris: at least HP-UX, IRIX, Linux.
 * The API is from SysV.
 *
 * There are at least two more shadow interfaces,
 * see the comments in pp_gpwent().
 *
 * --jhi */
#   ifdef __hpux__
/* There is a MAXINT coming from <shadow.h> <- <hpsecurity.h> <- <values.h>
 * and another MAXINT from "perl.h" <- <sys/param.h>. */
#       undef MAXINT
#   endif
#   include <shadow.h>
#endif

#ifdef I_SYS_RESOURCE
# include <sys/resource.h>
#endif

#ifdef HAS_SELECT
# ifdef I_SYS_SELECT
#  include <sys/select.h>
# endif
#endif

#ifdef I_SYS_SYSCALL
# include <sys/syscall.h>
#endif

/* XXX Configure test needed.
   h_errno might not be a simple 'int', especially for multi-threaded
   applications, see "extern int errno in perl.h".  Creating such
   a test requires taking into account the differences between
   compiling multithreaded and singlethreaded ($ccflags et al).
   HOST_NOT_FOUND is typically defined in <netdb.h>.
*/
#if defined(HOST_NOT_FOUND) && !defined(h_errno) && !defined(__CYGWIN__)
extern int h_errno;
#endif

#ifdef HAS_PASSWD
# ifdef I_PWD
#  include <pwd.h>
# elif !defined(VMS)
    struct passwd *getpwnam (char *);
    struct passwd *getpwuid (Uid_t);
# endif
# ifdef HAS_GETPWENT
#  ifndef getpwent
  struct passwd *getpwent (void);
#  elif defined (VMS) && defined (my_getpwent)
  struct passwd *Perl_my_getpwent (pTHX);
#  endif
# endif
#endif

#ifdef HAS_GROUP
# ifdef I_GRP
#  include <grp.h>
# else
    struct group *getgrnam (char *);
    struct group *getgrgid (Gid_t);
# endif
# ifdef HAS_GETGRENT
#  ifndef getgrent
    struct group *getgrent (void);
#  endif
# endif
#endif

#ifdef I_UTIME
#  if defined(_MSC_VER) || defined(__MINGW32__)
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#endif

#ifdef HAS_CHSIZE
# ifdef my_chsize  /* Probably #defined to Perl_my_chsize in embed.h */
#   undef my_chsize
# endif
# define my_chsize PerlLIO_chsize
#elif defined(HAS_TRUNCATE)
# define my_chsize PerlLIO_chsize
#else
I32 my_chsize(int fd, Off_t length);
#endif

#ifdef HAS_FLOCK
#  define FLOCK flock
#else /* no flock() */

   /* fcntl.h might not have been included, even if it exists, because
      the current Configure only sets I_FCNTL if it's needed to pick up
      the *_OK constants.  Make sure it has been included before testing
      the fcntl() locking constants. */
#  if defined(HAS_FCNTL) && !defined(I_FCNTL)
#    include <fcntl.h>
#  endif

#  if defined(HAS_FCNTL) && defined(FCNTL_CAN_LOCK)
#    define FLOCK fcntl_emulate_flock
#    define FCNTL_EMULATE_FLOCK
#  elif defined(HAS_LOCKF)
#    define FLOCK lockf_emulate_flock
#    define LOCKF_EMULATE_FLOCK
#  endif

#  ifdef FLOCK
     static int FLOCK (int, int);

    /*
     * These are the flock() constants.  Since this sytems doesn't have
     * flock(), the values of the constants are probably not available.
     */
#    ifndef LOCK_SH
#      define LOCK_SH 1
#    endif
#    ifndef LOCK_EX
#      define LOCK_EX 2
#    endif
#    ifndef LOCK_NB
#      define LOCK_NB 4
#    endif
#    ifndef LOCK_UN
#      define LOCK_UN 8
#    endif
#  endif /* emulating flock() */

#endif /* no flock() */

#define ZBTLEN 10
static const char zero_but_true[ZBTLEN + 1] = "0 but true";

#if defined(I_SYS_ACCESS) && !defined(R_OK)
#  include <sys/access.h>
#endif

#include "reentr.h"

#ifdef __Lynx__
/* Missing protos on LynxOS */
void sethostent(int);
void endhostent(void);
void setnetent(int);
void endnetent(void);
void setprotoent(int);
void endprotoent(void);
void setservent(int);
void endservent(void);
#endif

#ifdef __amigaos4__
#  include "amigaos4/amigaio.h"
#endif

#undef PERL_EFF_ACCESS	/* EFFective uid/gid ACCESS */

/* F_OK unused: if stat() cannot find it... */

#if !defined(PERL_EFF_ACCESS) && defined(HAS_ACCESS) && defined(EFF_ONLY_OK) && !defined(NO_EFF_ONLY_OK)
    /* Digital UNIX (when the EFF_ONLY_OK gets fixed), UnixWare */
#   define PERL_EFF_ACCESS(p,f) (access((p), (f) | EFF_ONLY_OK))
#endif

#if !defined(PERL_EFF_ACCESS) && defined(HAS_EACCESS)
#   ifdef I_SYS_SECURITY
#       include <sys/security.h>
#   endif
#   ifdef ACC_SELF
        /* HP SecureWare */
#       define PERL_EFF_ACCESS(p,f) (eaccess((p), (f), ACC_SELF))
#   else
        /* SCO */
#       define PERL_EFF_ACCESS(p,f) (eaccess((p), (f)))
#   endif
#endif

#if !defined(PERL_EFF_ACCESS) && defined(HAS_ACCESSX) && defined(ACC_SELF)
    /* AIX's accessx() doesn't declare its argument const, unlike every other platform */
#   define PERL_EFF_ACCESS(p,f) (accessx((char*)(p), (f), ACC_SELF))
#endif


#if !defined(PERL_EFF_ACCESS) && defined(HAS_ACCESS)	\
    && (defined(HAS_SETREUID) || defined(HAS_SETRESUID)		\
        || defined(HAS_SETREGID) || defined(HAS_SETRESGID))
/* The Hard Way. */
STATIC int
S_emulate_eaccess(pTHX_ const char* path, Mode_t mode)
{
    const Uid_t ruid = getuid();
    const Uid_t euid = geteuid();
    const Gid_t rgid = getgid();
    const Gid_t egid = getegid();
    int res;

#if !defined(HAS_SETREUID) && !defined(HAS_SETRESUID)
    Perl_croak(aTHX_ "switching effective uid is not implemented");
#else
#  ifdef HAS_SETREUID
    if (setreuid(euid, ruid))
#  elif defined(HAS_SETRESUID)
    if (setresuid(euid, ruid, (Uid_t)-1))
#  endif
        /* diag_listed_as: entering effective %s failed */
        Perl_croak(aTHX_ "entering effective uid failed");
#endif

#if !defined(HAS_SETREGID) && !defined(HAS_SETRESGID)
    Perl_croak(aTHX_ "switching effective gid is not implemented");
#else
#  ifdef HAS_SETREGID
    if (setregid(egid, rgid))
#  elif defined(HAS_SETRESGID)
    if (setresgid(egid, rgid, (Gid_t)-1))
#  endif
        /* diag_listed_as: entering effective %s failed */
        Perl_croak(aTHX_ "entering effective gid failed");
#endif

    res = access(path, mode);

#ifdef HAS_SETREUID
    if (setreuid(ruid, euid))
#elif defined(HAS_SETRESUID)
    if (setresuid(ruid, euid, (Uid_t)-1))
#endif
        /* diag_listed_as: leaving effective %s failed */
        Perl_croak(aTHX_ "leaving effective uid failed");

#ifdef HAS_SETREGID
    if (setregid(rgid, egid))
#elif defined(HAS_SETRESGID)
    if (setresgid(rgid, egid, (Gid_t)-1))
#endif
        /* diag_listed_as: leaving effective %s failed */
        Perl_croak(aTHX_ "leaving effective gid failed");

    return res;
}
#   define PERL_EFF_ACCESS(p,f) (S_emulate_eaccess(aTHX_ (p), (f)))
#endif

PP(pp_backtick)
{
    dSP; dTARGET;
    PerlIO *fp;
    const char * const tmps = POPpconstx;
    const U8 gimme = GIMME_V;
    const char *mode = "r";

    TAINT_PROPER("``");
    if (PL_op->op_private & OPpOPEN_IN_RAW)
        mode = "rb";
    else if (PL_op->op_private & OPpOPEN_IN_CRLF)
        mode = "rt";
    fp = PerlProc_popen(tmps, mode);
    if (fp) {
        const char * const type = Perl_PerlIO_context_layers(aTHX_ NULL);
        if (type && *type)
            PerlIO_apply_layers(aTHX_ fp,mode,type);

        if (gimme == G_VOID) {
            char tmpbuf[256];
            while (PerlIO_read(fp, tmpbuf, sizeof tmpbuf) > 0)
                NOOP;
        }
        else if (gimme == G_SCALAR) {
            ENTER_with_name("backtick");
            SAVESPTR(PL_rs);
            PL_rs = &PL_sv_undef;
            SvPVCLEAR(TARG);        /* note that this preserves previous buffer */
            while (sv_gets(TARG, fp, SvCUR(TARG)) != NULL)
                NOOP;
            LEAVE_with_name("backtick");
            XPUSHs(TARG);
            SvTAINTED_on(TARG);
        }
        else {
            for (;;) {
                SV * const sv = newSV(79);
                if (sv_gets(sv, fp, 0) == NULL) {
                    SvREFCNT_dec(sv);
                    break;
                }
                mXPUSHs(sv);
                if (SvLEN(sv) - SvCUR(sv) > 20) {
                    SvPV_shrink_to_cur(sv);
                }
                SvTAINTED_on(sv);
            }
        }
        STATUS_NATIVE_CHILD_SET(PerlProc_pclose(fp));
        TAINT;		/* "I believe that this is not gratuitous!" */
    }
    else {
        STATUS_NATIVE_CHILD_SET(-1);
        if (gimme == G_SCALAR)
            RETPUSHUNDEF;
    }

    RETURN;
}

PP(pp_glob)
{
    OP *result;
    dSP;
    GV * const gv = (PL_op->op_flags & OPf_SPECIAL) ? NULL : (GV *)POPs;

    PUTBACK;

    /* make a copy of the pattern if it is gmagical, to ensure that magic
     * is called once and only once */
    if (SvGMAGICAL(TOPs)) TOPs = sv_2mortal(newSVsv(TOPs));

    tryAMAGICunTARGETlist(iter_amg, (PL_op->op_flags & OPf_SPECIAL));

    if (PL_op->op_flags & OPf_SPECIAL) {
        /* call Perl-level glob function instead. Stack args are:
         * MARK, wildcard
         * and following OPs should be: gv(CORE::GLOBAL::glob), entersub
         * */
        return NORMAL;
    }
    if (PL_globhook) {
        PL_globhook(aTHX);
        return NORMAL;
    }

    /* Note that we only ever get here if File::Glob fails to load
     * without at the same time croaking, for some reason, or if
     * perl was built with PERL_EXTERNAL_GLOB */

    ENTER_with_name("glob");

#ifndef VMS
    if (TAINTING_get) {
        /*
         * The external globbing program may use things we can't control,
         * so for security reasons we must assume the worst.
         */
        TAINT;
        taint_proper(PL_no_security, "glob");
    }
#endif /* !VMS */

    SAVESPTR(PL_last_in_gv);	/* We don't want this to be permanent. */
    PL_last_in_gv = gv;

    SAVESPTR(PL_rs);		/* This is not permanent, either. */
    PL_rs = newSVpvs_flags("\000", SVs_TEMP);
#ifndef DOSISH
#ifndef CSH
    *SvPVX(PL_rs) = '\n';
#endif	/* !CSH */
#endif	/* !DOSISH */

    result = do_readline();
    LEAVE_with_name("glob");
    return result;
}

PP(pp_rcatline)
{
    PL_last_in_gv = cGVOP_gv;
    return do_readline();
}

PP(pp_warn)
{
    dSP; dMARK;
    SV *exsv;
    STRLEN len;
    if (SP - MARK > 1) {
        dTARGET;
        do_join(TARG, &PL_sv_no, MARK, SP);
        exsv = TARG;
        SP = MARK + 1;
    }
    else if (SP == MARK) {
        exsv = &PL_sv_no;
        MEXTEND(SP, 1);
        SP = MARK + 1;
    }
    else {
        exsv = TOPs;
        if (SvGMAGICAL(exsv)) exsv = sv_mortalcopy(exsv);
    }

    if (SvROK(exsv) || (SvPV_const(exsv, len), len)) {
        /* well-formed exception supplied */
    }
    else {
      SV * const errsv = ERRSV;
      SvGETMAGIC(errsv);
      if (SvROK(errsv)) {
        if (SvGMAGICAL(errsv)) {
            exsv = sv_newmortal();
            sv_setsv_nomg(exsv, errsv);
        }
        else exsv = errsv;
      }
      else if (SvPOKp(errsv) ? SvCUR(errsv) : SvNIOKp(errsv)) {
        exsv = sv_newmortal();
        sv_setsv_nomg(exsv, errsv);
        sv_catpvs(exsv, "\t...caught");
      }
      else {
        exsv = newSVpvs_flags("Warning: something's wrong", SVs_TEMP);
      }
    }
    if (SvROK(exsv) && !PL_warnhook)
         Perl_warn(aTHX_ "%" SVf, SVfARG(exsv));
    else warn_sv(exsv);
    RETSETYES;
}

PP(pp_die)
{
    dSP; dMARK;
    SV *exsv;
    STRLEN len;
#ifdef VMS
    VMSISH_HUSHED  =
        VMSISH_HUSHED || (PL_curcop->op_private & OPpHUSH_VMSISH);
#endif
    if (SP - MARK != 1) {
        dTARGET;
        do_join(TARG, &PL_sv_no, MARK, SP);
        exsv = TARG;
        SP = MARK + 1;
    }
    else {
        exsv = TOPs;
    }

    if (SvROK(exsv) || (SvPV_const(exsv, len), len)) {
        /* well-formed exception supplied */
    }
    else {
        SV * const errsv = ERRSV;
        SvGETMAGIC(errsv);
        if (SvROK(errsv)) {
            exsv = errsv;
            if (sv_isobject(exsv)) {
                HV * const stash = SvSTASH(SvRV(exsv));
                GV * const gv = gv_fetchmethod(stash, "PROPAGATE");
                if (gv) {
                    SV * const file = sv_2mortal(newSVpv(CopFILE(PL_curcop),0));
                    SV * const line = sv_2mortal(newSVuv(CopLINE(PL_curcop)));
                    EXTEND(SP, 3);
                    PUSHMARK(SP);
                    PUSHs(exsv);
                    PUSHs(file);
                    PUSHs(line);
                    PUTBACK;
                    call_sv(MUTABLE_SV(GvCV(gv)),
                            G_SCALAR|G_EVAL|G_KEEPERR);
                    exsv = sv_mortalcopy(*PL_stack_sp--);
                }
            }
        }
        else if (SvOK(errsv) && (SvPV_nomg(errsv,len), len)) {
            exsv = sv_mortalcopy(errsv);
            sv_catpvs(exsv, "\t...propagated");
        }
        else {
            exsv = newSVpvs_flags("Died", SVs_TEMP);
        }
    }
    die_sv(exsv);
    NOT_REACHED; /* NOTREACHED */
    return NULL; /* avoid missing return from non-void function warning */
}

/* I/O. */

OP *
Perl_tied_method(pTHX_ SV *methname, SV **sp, SV *const sv,
                 const MAGIC *const mg, const U32 flags, U32 argc, ...)
{
    SV **orig_sp = sp;
    I32 ret_args;
    SSize_t extend_size;

    PERL_ARGS_ASSERT_TIED_METHOD;

    /* Ensure that our flag bits do not overlap.  */
    STATIC_ASSERT_STMT((TIED_METHOD_MORTALIZE_NOT_NEEDED & G_WANT) == 0);
    STATIC_ASSERT_STMT((TIED_METHOD_ARGUMENTS_ON_STACK & G_WANT) == 0);
    STATIC_ASSERT_STMT((TIED_METHOD_SAY & G_WANT) == 0);

    PUTBACK; /* sp is at *foot* of args, so this pops args from old stack */
    PUSHSTACKi(PERLSI_MAGIC);
    /* extend for object + args. If argc might wrap/truncate when cast
     * to SSize_t and incremented, set to -1, which will trigger a panic in
     * EXTEND().
     * The weird way this is written is because g++ is dumb enough to
     * warn "comparison is always false" on something like:
     *
     * sizeof(a) >= sizeof(b) && a >= B_t_MAX -1
     *
     * (where the LH condition is false)
     */
    extend_size =
        (argc > (sizeof(argc) >= sizeof(SSize_t) ? SSize_t_MAX - 1 : argc))
            ? -1 : (SSize_t)argc + 1;
    EXTEND(SP, extend_size);
    PUSHMARK(sp);
    PUSHs(SvTIED_obj(sv, mg));
    if (flags & TIED_METHOD_ARGUMENTS_ON_STACK) {
        Copy(orig_sp + 2, sp + 1, argc, SV*); /* copy args to new stack */
        sp += argc;
    }
    else if (argc) {
        const U32 mortalize_not_needed
            = flags & TIED_METHOD_MORTALIZE_NOT_NEEDED;
        va_list args;
        va_start(args, argc);
        do {
            SV *const arg = va_arg(args, SV *);
            if(mortalize_not_needed)
                PUSHs(arg);
            else
                mPUSHs(arg);
        } while (--argc);
        va_end(args);
    }

    PUTBACK;
    ENTER_with_name("call_tied_method");
    if (flags & TIED_METHOD_SAY) {
        /* local $\ = "\n" */
        SAVEGENERICSV(PL_ors_sv);
        PL_ors_sv = newSVpvs("\n");
    }
    ret_args = call_sv(methname, (flags & G_WANT)|G_METHOD_NAMED);
    SPAGAIN;
    orig_sp = sp;
    POPSTACK;
    SPAGAIN;
    if (ret_args) { /* copy results back to original stack */
        EXTEND(sp, ret_args);
        Copy(orig_sp - ret_args + 1, sp + 1, ret_args, SV*);
        sp += ret_args;
        PUTBACK;
    }
    LEAVE_with_name("call_tied_method");
    return NORMAL;
}

#define tied_method0(a,b,c,d)		\
    Perl_tied_method(aTHX_ a,b,c,d,G_SCALAR,0)
#define tied_method1(a,b,c,d,e)		\
    Perl_tied_method(aTHX_ a,b,c,d,G_SCALAR,1,e)
#define tied_method2(a,b,c,d,e,f)	\
    Perl_tied_method(aTHX_ a,b,c,d,G_SCALAR,2,e,f)

PP(pp_open)
{
    dSP;
    dMARK; dORIGMARK;
    dTARGET;
    SV *sv;
    IO *io;
    const char *tmps;
    STRLEN len;
    bool  ok;

    GV * const gv = MUTABLE_GV(*++MARK);

    if (!isGV(gv) && !(SvTYPE(gv) == SVt_PVLV && isGV_with_GP(gv)))
        DIE(aTHX_ PL_no_usym, "filehandle");

    if ((io = GvIOp(gv))) {
        const MAGIC *mg;
        IoFLAGS(GvIOp(gv)) &= ~IOf_UNTAINT;

        if (IoDIRP(io))
            Perl_croak(aTHX_ "Cannot open %" HEKf " as a filehandle: it is already open as a dirhandle",
                             HEKfARG(GvENAME_HEK(gv)));

        mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            /* Method's args are same as ours ... */
            /* ... except handle is replaced by the object */
            return Perl_tied_method(aTHX_ SV_CONST(OPEN), mark - 1, MUTABLE_SV(io), mg,
                                    G_SCALAR | TIED_METHOD_ARGUMENTS_ON_STACK,
                                    sp - mark);
        }
    }

    if (MARK < SP) {
        sv = *++MARK;
    }
    else {
        sv = GvSVn(gv);
    }

    tmps = SvPV_const(sv, len);
    ok = do_open6(gv, tmps, len, NULL, MARK+1, (SP-MARK));
    SP = ORIGMARK;
    if (ok)
        PUSHi( (I32)PL_forkprocess );
    else if (PL_forkprocess == 0)		/* we are a new child */
        PUSHs(&PL_sv_zero);
    else
        RETPUSHUNDEF;
    RETURN;
}

PP(pp_close)
{
    dSP;
    /* pp_coreargs pushes a NULL to indicate no args passed to
     * CORE::close() */
    GV * const gv =
        MAXARG == 0 || (!TOPs && !POPs) ? PL_defoutgv : MUTABLE_GV(POPs);

    if (MAXARG == 0)
        EXTEND(SP, 1);

    if (gv) {
        IO * const io = GvIO(gv);
        if (io) {
            const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
            if (mg) {
                return tied_method0(SV_CONST(CLOSE), SP, MUTABLE_SV(io), mg);
            }
        }
    }
    PUSHs(boolSV(do_close(gv, TRUE)));
    RETURN;
}

PP(pp_pipe_op)
{
#ifdef HAS_PIPE
    dSP;
    IO *rstio;
    IO *wstio;
    int fd[2];

    GV * const wgv = MUTABLE_GV(POPs);
    GV * const rgv = MUTABLE_GV(POPs);

    rstio = GvIOn(rgv);
    if (IoIFP(rstio))
        do_close(rgv, FALSE);

    wstio = GvIOn(wgv);
    if (IoIFP(wstio))
        do_close(wgv, FALSE);

    if (PerlProc_pipe_cloexec(fd) < 0)
        goto badexit;

    IoIFP(rstio) = PerlIO_fdopen(fd[0], "r" PIPE_OPEN_MODE);
    IoOFP(wstio) = PerlIO_fdopen(fd[1], "w" PIPE_OPEN_MODE);
    IoOFP(rstio) = IoIFP(rstio);
    IoIFP(wstio) = IoOFP(wstio);
    IoTYPE(rstio) = IoTYPE_RDONLY;
    IoTYPE(wstio) = IoTYPE_WRONLY;

    if (!IoIFP(rstio) || !IoOFP(wstio)) {
        if (IoIFP(rstio))
            PerlIO_close(IoIFP(rstio));
        else
            PerlLIO_close(fd[0]);
        if (IoOFP(wstio))
            PerlIO_close(IoOFP(wstio));
        else
            PerlLIO_close(fd[1]);
        goto badexit;
    }
    RETPUSHYES;

  badexit:
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_func, "pipe");
#endif
}

PP(pp_fileno)
{
    dSP; dTARGET;
    GV *gv;
    IO *io;
    PerlIO *fp;
    const MAGIC *mg;

    if (MAXARG < 1)
        RETPUSHUNDEF;
    gv = MUTABLE_GV(POPs);
    io = GvIO(gv);

    if (io
        && (mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar)))
    {
        return tied_method0(SV_CONST(FILENO), SP, MUTABLE_SV(io), mg);
    }

    if (io && IoDIRP(io)) {
#if defined(HAS_DIRFD) || defined(HAS_DIR_DD_FD)
        PUSHi(my_dirfd(IoDIRP(io)));
        RETURN;
#elif defined(ENOTSUP)
        errno = ENOTSUP;        /* Operation not supported */
        RETPUSHUNDEF;
#elif defined(EOPNOTSUPP)
        errno = EOPNOTSUPP;     /* Operation not supported on socket */
        RETPUSHUNDEF;
#else
        errno = EINVAL;         /* Invalid argument */
        RETPUSHUNDEF;
#endif
    }

    if (!io || !(fp = IoIFP(io))) {
        /* Can't do this because people seem to do things like
           defined(fileno($foo)) to check whether $foo is a valid fh.

           report_evil_fh(gv);
            */
        RETPUSHUNDEF;
    }

    PUSHi(PerlIO_fileno(fp));
    RETURN;
}

PP(pp_umask)
{
    dSP;
#ifdef HAS_UMASK
    dTARGET;
    Mode_t anum;

    if (MAXARG < 1 || (!TOPs && !POPs)) {
        anum = PerlLIO_umask(022);
        /* setting it to 022 between the two calls to umask avoids
         * to have a window where the umask is set to 0 -- meaning
         * that another thread could create world-writeable files. */
        if (anum != 022)
            (void)PerlLIO_umask(anum);
    }
    else
        anum = PerlLIO_umask(POPi);
    TAINT_PROPER("umask");
    XPUSHi(anum);
#else
    /* Only DIE if trying to restrict permissions on "user" (self).
     * Otherwise it's harmless and more useful to just return undef
     * since 'group' and 'other' concepts probably don't exist here. */
    if (MAXARG >= 1 && (TOPs||POPs) && (POPi & 0700))
        DIE(aTHX_ "umask not implemented");
    XPUSHs(&PL_sv_undef);
#endif
    RETURN;
}

PP(pp_binmode)
{
    dSP;
    GV *gv;
    IO *io;
    PerlIO *fp;
    SV *discp = NULL;

    if (MAXARG < 1)
        RETPUSHUNDEF;
    if (MAXARG > 1) {
        discp = POPs;
    }

    gv = MUTABLE_GV(POPs);
    io = GvIO(gv);

    if (io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            /* This takes advantage of the implementation of the varargs
               function, which I don't think that the optimiser will be able to
               figure out. Although, as it's a static function, in theory it
               could.  */
            return Perl_tied_method(aTHX_ SV_CONST(BINMODE), SP, MUTABLE_SV(io), mg,
                                    G_SCALAR|TIED_METHOD_MORTALIZE_NOT_NEEDED,
                                    discp ? 1 : 0, discp);
        }
    }

    if (!io || !(fp = IoIFP(io))) {
        report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);
        RETPUSHUNDEF;
    }

    PUTBACK;
    {
        STRLEN len = 0;
        const char *d = NULL;
        int mode;
        if (discp)
            d = SvPV_const(discp, len);
        mode = mode_from_discipline(d, len);
        if (PerlIO_binmode(aTHX_ fp, IoTYPE(io), mode, d)) {
            if (IoOFP(io) && IoOFP(io) != IoIFP(io)) {
                if (!PerlIO_binmode(aTHX_ IoOFP(io), IoTYPE(io), mode, d)) {
                    SPAGAIN;
                    RETPUSHUNDEF;
                }
            }
            SPAGAIN;
            RETPUSHYES;
        }
        else {
            SPAGAIN;
            RETPUSHUNDEF;
        }
    }
}

PP(pp_tie)
{
    dSP; dMARK;
    HV* stash;
    GV *gv = NULL;
    SV *sv;
    const I32 markoff = MARK - PL_stack_base;
    const char *methname;
    int how = PERL_MAGIC_tied;
    U32 items;
    SV *varsv = *++MARK;

    switch(SvTYPE(varsv)) {
        case SVt_PVHV:
        {
            HE *entry;
            methname = "TIEHASH";
            if (HvLAZYDEL(varsv) && (entry = HvEITER_get((HV *)varsv))) {
                HvLAZYDEL_off(varsv);
                hv_free_ent(NULL, entry);
            }
            HvEITER_set(MUTABLE_HV(varsv), 0);
            HvRITER_set(MUTABLE_HV(varsv), -1);
            break;
        }
        case SVt_PVAV:
            methname = "TIEARRAY";
            if (!AvREAL(varsv)) {
                if (!AvREIFY(varsv))
                    Perl_croak(aTHX_ "Cannot tie unreifiable array");
                av_clear((AV *)varsv);
                AvREIFY_off(varsv);
                AvREAL_on(varsv);
            }
            break;
        case SVt_PVGV:
        case SVt_PVLV:
            if (isGV_with_GP(varsv) && !SvFAKE(varsv)) {
                methname = "TIEHANDLE";
                how = PERL_MAGIC_tiedscalar;
                /* For tied filehandles, we apply tiedscalar magic to the IO
                   slot of the GP rather than the GV itself. AMS 20010812 */
                if (!GvIOp(varsv))
                    GvIOp(varsv) = newIO();
                varsv = MUTABLE_SV(GvIOp(varsv));
                break;
            }
            if (SvTYPE(varsv) == SVt_PVLV && LvTYPE(varsv) == 'y') {
                vivify_defelem(varsv);
                varsv = LvTARG(varsv);
            }
            /* FALLTHROUGH */
        default:
            methname = "TIESCALAR";
            how = PERL_MAGIC_tiedscalar;
            break;
    }
    items = SP - MARK++;
    if (sv_isobject(*MARK)) { /* Calls GET magic. */
        ENTER_with_name("call_TIE");
        PUSHSTACKi(PERLSI_MAGIC);
        PUSHMARK(SP);
        EXTEND(SP,(I32)items);
        while (items--)
            PUSHs(*MARK++);
        PUTBACK;
        call_method(methname, G_SCALAR);
    }
    else {
        /* Can't use call_method here, else this: fileno FOO; tie @a, "FOO"
         * will attempt to invoke IO::File::TIEARRAY, with (best case) the
         * wrong error message, and worse case, supreme action at a distance.
         * (Sorry obfuscation writers. You're not going to be given this one.)
         */
       stash = gv_stashsv(*MARK, 0);
       if (!stash) {
           if (SvROK(*MARK))
               DIE(aTHX_ "Can't locate object method %" PVf_QUOTEDPREFIX
                         " via package %" SVf_QUOTEDPREFIX,
                   methname, SVfARG(*MARK));
           else if (isGV(*MARK)) {
               /* If the glob doesn't name an existing package, using
                * SVfARG(*MARK) would yield "*Foo::Bar" or *main::Foo. So
                * generate the name for the error message explicitly. */
               SV *stashname = sv_newmortal();
               gv_fullname4(stashname, (GV *) *MARK, NULL, FALSE);
               DIE(aTHX_ "Can't locate object method %" PVf_QUOTEDPREFIX
                         " via package %" SVf_QUOTEDPREFIX,
                   methname, SVfARG(stashname));
           }
           else {
               SV *stashname = !SvPOK(*MARK) ? &PL_sv_no
                             : SvCUR(*MARK)  ? *MARK
                             :                 newSVpvs_flags("main", SVs_TEMP);
               DIE(aTHX_ "Can't locate object method %" PVf_QUOTEDPREFIX
                         " via package %" SVf_QUOTEDPREFIX
                   " (perhaps you forgot to load %" SVf_QUOTEDPREFIX "?)",
                   methname, SVfARG(stashname), SVfARG(stashname));
           }
       }
       else if (!(gv = gv_fetchmethod(stash, methname))) {
           /* The effective name can only be NULL for stashes that have
            * been deleted from the symbol table, which this one can't
            * be, since we just looked it up by name.
            */
           DIE(aTHX_ "Can't locate object method %" PVf_QUOTEDPREFIX
                     " via package %" HEKf_QUOTEDPREFIX ,
               methname, HvENAME_HEK_NN(stash));
       }
        ENTER_with_name("call_TIE");
        PUSHSTACKi(PERLSI_MAGIC);
        PUSHMARK(SP);
        EXTEND(SP,(I32)items);
        while (items--)
            PUSHs(*MARK++);
        PUTBACK;
        call_sv(MUTABLE_SV(GvCV(gv)), G_SCALAR);
    }
    SPAGAIN;

    sv = TOPs;
    POPSTACK;
    if (sv_isobject(sv)) {
        sv_unmagic(varsv, how);
        /* Croak if a self-tie on an aggregate is attempted. */
        if (varsv == SvRV(sv) &&
            (SvTYPE(varsv) == SVt_PVAV ||
             SvTYPE(varsv) == SVt_PVHV))
            Perl_croak(aTHX_
                       "Self-ties of arrays and hashes are not supported");
        sv_magic(varsv, (SvRV(sv) == varsv ? NULL : sv), how, NULL, 0);
    }
    LEAVE_with_name("call_TIE");
    SP = PL_stack_base + markoff;
    PUSHs(sv);
    RETURN;
}


/* also used for: pp_dbmclose() */

PP(pp_untie)
{
    dSP;
    MAGIC *mg;
    SV *sv = POPs;
    const char how = (SvTYPE(sv) == SVt_PVHV || SvTYPE(sv) == SVt_PVAV)
                ? PERL_MAGIC_tied : PERL_MAGIC_tiedscalar;

    if (isGV_with_GP(sv) && !SvFAKE(sv) && !(sv = MUTABLE_SV(GvIOp(sv))))
        RETPUSHYES;

    if (SvTYPE(sv) == SVt_PVLV && LvTYPE(sv) == 'y' &&
        !(sv = defelem_target(sv, NULL))) RETPUSHUNDEF;

    if ((mg = SvTIED_mg(sv, how))) {
        SV * const obj = SvRV(SvTIED_obj(sv, mg));
        if (obj && SvSTASH(obj)) {
            GV * const gv = gv_fetchmethod_autoload(SvSTASH(obj), "UNTIE", FALSE);
            CV *cv;
            if (gv && isGV(gv) && (cv = GvCV(gv))) {
               PUSHMARK(SP);
               PUSHs(SvTIED_obj(MUTABLE_SV(gv), mg));
               mXPUSHi(SvREFCNT(obj) - 1);
               PUTBACK;
               ENTER_with_name("call_UNTIE");
               call_sv(MUTABLE_SV(cv), G_VOID);
               LEAVE_with_name("call_UNTIE");
               SPAGAIN;
            }
            else if (mg && SvREFCNT(obj) > 1) {
                Perl_ck_warner(aTHX_ packWARN(WARN_UNTIE),
                               "untie attempted while %" UVuf " inner references still exist",
                               (UV)SvREFCNT(obj) - 1 ) ;
            }
        }
    }
    sv_unmagic(sv, how) ;

    if (SvTYPE(sv) == SVt_PVHV) {
        /* If the tied hash was partway through iteration, free the iterator and
         * any key that it is pointing to. */
        HE *entry;
        if (HvLAZYDEL(sv) && (entry = HvEITER_get((HV *)sv))) {
            HvLAZYDEL_off(sv);
            hv_free_ent(NULL, entry);
            HvEITER_set(MUTABLE_HV(sv), 0);
        }
    }

    RETPUSHYES;
}

PP(pp_tied)
{
    dSP;
    const MAGIC *mg;
    dTOPss;
    const char how = (SvTYPE(sv) == SVt_PVHV || SvTYPE(sv) == SVt_PVAV)
                ? PERL_MAGIC_tied : PERL_MAGIC_tiedscalar;

    if (isGV_with_GP(sv) && !SvFAKE(sv) && !(sv = MUTABLE_SV(GvIOp(sv))))
        goto ret_undef;

    if (SvTYPE(sv) == SVt_PVLV && LvTYPE(sv) == 'y' &&
        !(sv = defelem_target(sv, NULL))) goto ret_undef;

    if ((mg = SvTIED_mg(sv, how))) {
        SETs(SvTIED_obj(sv, mg));
        return NORMAL; /* PUTBACK not needed, pp_tied never moves SP */
    }
    ret_undef:
    SETs(&PL_sv_undef);
    return NORMAL;
}

PP(pp_dbmopen)
{
    dSP;
    dPOPPOPssrl;
    HV* stash;
    GV *gv = NULL;

    HV * const hv = MUTABLE_HV(POPs);
    SV * const sv = newSVpvs_flags("AnyDBM_File", SVs_TEMP);
    stash = gv_stashsv(sv, 0);
    if (!stash || !(gv = gv_fetchmethod(stash, "TIEHASH"))) {
        PUTBACK;
        require_pv("AnyDBM_File.pm");
        SPAGAIN;
        if (!stash || !(gv = gv_fetchmethod(stash, "TIEHASH")))
            DIE(aTHX_ "No dbm on this machine");
    }

    ENTER;
    PUSHMARK(SP);

    EXTEND(SP, 5);
    PUSHs(sv);
    PUSHs(left);
    if (SvIV(right))
        mPUSHu(O_RDWR|O_CREAT);
    else
    {
        mPUSHu(O_RDWR);
        if (!SvOK(right)) right = &PL_sv_no;
    }
    PUSHs(right);
    PUTBACK;
    call_sv(MUTABLE_SV(GvCV(gv)), G_SCALAR);
    SPAGAIN;

    if (!sv_isobject(TOPs)) {
        SP--;
        PUSHMARK(SP);
        PUSHs(sv);
        PUSHs(left);
        mPUSHu(O_RDONLY);
        PUSHs(right);
        PUTBACK;
        call_sv(MUTABLE_SV(GvCV(gv)), G_SCALAR);
        SPAGAIN;
        if (sv_isobject(TOPs))
            goto retie;
    }
    else {
        retie:
        sv_unmagic(MUTABLE_SV(hv), PERL_MAGIC_tied);
        sv_magic(MUTABLE_SV(hv), TOPs, PERL_MAGIC_tied, NULL, 0);
    }
    LEAVE;
    RETURN;
}

PP(pp_sselect)
{
#ifdef HAS_SELECT
    dSP; dTARGET;
    I32 i;
    I32 j;
    char *s;
    SV *sv;
    NV value;
    I32 maxlen = 0;
    I32 nfound;
    struct timeval timebuf;
    struct timeval *tbuf = &timebuf;
    I32 growsize;
    char *fd_sets[4];
    SV *svs[4];
#if BYTEORDER != 0x1234 && BYTEORDER != 0x12345678
        I32 masksize;
        I32 offset;
        I32 k;

#   if BYTEORDER & 0xf0000
#	define ORDERBYTE (0x88888888 - BYTEORDER)
#   else
#	define ORDERBYTE (0x4444 - BYTEORDER)
#   endif

#endif

    SP -= 4;
    for (i = 1; i <= 3; i++) {
        SV * const sv = svs[i] = SP[i];
        SvGETMAGIC(sv);
        if (!SvOK(sv))
            continue;
        if (SvREADONLY(sv)) {
            if (!(SvPOK(sv) && SvCUR(sv) == 0))
                Perl_croak_no_modify();
        }
        else if (SvIsCOW(sv)) sv_force_normal_flags(sv, 0);
        if (SvPOK(sv)) {
            if (SvUTF8(sv)) sv_utf8_downgrade(sv, FALSE);
        }
        else {
            if (!SvPOKp(sv))
                Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
                                    "Non-string passed as bitmask");
            if (SvGAMAGIC(sv)) {
                svs[i] = sv_newmortal();
                sv_copypv_nomg(svs[i], sv);
            }
            else
                SvPV_force_nomg_nolen(sv); /* force string conversion */
        }
        j = SvCUR(svs[i]);
        if (maxlen < j)
            maxlen = j;
    }

/* little endians can use vecs directly */
#if BYTEORDER != 0x1234 && BYTEORDER != 0x12345678
#  ifdef NFDBITS

#    ifndef NBBY
#     define NBBY 8
#    endif

    masksize = NFDBITS / NBBY;
#  else
    masksize = sizeof(long);	/* documented int, everyone seems to use long */
#  endif
    Zero(&fd_sets[0], 4, char*);
#endif

#  if SELECT_MIN_BITS == 1
    growsize = sizeof(fd_set);
#  else
#   if defined(__GLIBC__) && defined(__FD_SETSIZE)
#      undef SELECT_MIN_BITS
#      define SELECT_MIN_BITS __FD_SETSIZE
#   endif
    /* If SELECT_MIN_BITS is greater than one we most probably will want
     * to align the sizes with SELECT_MIN_BITS/8 because for example
     * in many little-endian (Intel, Alpha) systems (Linux, OS/2, Digital
     * UNIX, Solaris, Darwin) the smallest quantum select() operates
     * on (sets/tests/clears bits) is 32 bits.  */
    growsize = maxlen + (SELECT_MIN_BITS/8 - (maxlen % (SELECT_MIN_BITS/8)));
#  endif

    sv = SP[4];
    SvGETMAGIC(sv);
    if (SvOK(sv)) {
        value = SvNV_nomg(sv);
        if (value < 0.0)
            value = 0.0;
        timebuf.tv_sec = (long)value;
        value -= (NV)timebuf.tv_sec;
        timebuf.tv_usec = (long)(value * 1000000.0);
    }
    else
        tbuf = NULL;

    for (i = 1; i <= 3; i++) {
        sv = svs[i];
        if (!SvOK(sv) || SvCUR(sv) == 0) {
            fd_sets[i] = 0;
            continue;
        }
        assert(SvPOK(sv));
        j = SvLEN(sv);
        if (j < growsize) {
            Sv_Grow(sv, growsize);
        }
        j = SvCUR(sv);
        s = SvPVX(sv) + j;
        while (++j <= growsize) {
            *s++ = '\0';
        }

#if BYTEORDER != 0x1234 && BYTEORDER != 0x12345678
        s = SvPVX(sv);
        Newx(fd_sets[i], growsize, char);
        for (offset = 0; offset < growsize; offset += masksize) {
            for (j = 0, k=ORDERBYTE; j < masksize; j++, (k >>= 4))
                fd_sets[i][j+offset] = s[(k % masksize) + offset];
        }
#else
        fd_sets[i] = SvPVX(sv);
#endif
    }

#ifdef PERL_IRIX5_SELECT_TIMEVAL_VOID_CAST
    /* Can't make just the (void*) conditional because that would be
     * cpp #if within cpp macro, and not all compilers like that. */
    nfound = PerlSock_select(
        maxlen * 8,
        (Select_fd_set_t) fd_sets[1],
        (Select_fd_set_t) fd_sets[2],
        (Select_fd_set_t) fd_sets[3],
        (void*) tbuf); /* Workaround for compiler bug. */
#else
    nfound = PerlSock_select(
        maxlen * 8,
        (Select_fd_set_t) fd_sets[1],
        (Select_fd_set_t) fd_sets[2],
        (Select_fd_set_t) fd_sets[3],
        tbuf);
#endif
    for (i = 1; i <= 3; i++) {
        if (fd_sets[i]) {
            sv = svs[i];
#if BYTEORDER != 0x1234 && BYTEORDER != 0x12345678
            s = SvPVX(sv);
            for (offset = 0; offset < growsize; offset += masksize) {
                for (j = 0, k=ORDERBYTE; j < masksize; j++, (k >>= 4))
                    s[(k % masksize) + offset] = fd_sets[i][j+offset];
            }
            Safefree(fd_sets[i]);
#endif
            if (sv != SP[i])
                SvSetMagicSV(SP[i], sv);
            else
                SvSETMAGIC(sv);
        }
    }

    PUSHi(nfound);
    if (GIMME_V == G_LIST && tbuf) {
        value = (NV)(timebuf.tv_sec) +
                (NV)(timebuf.tv_usec) / 1000000.0;
        mPUSHn(value);
    }
    RETURN;
#else
    DIE(aTHX_ "select not implemented");
#endif
}

/*

=for apidoc_section $GV

=for apidoc setdefout

Sets C<PL_defoutgv>, the default file handle for output, to the passed in
typeglob.  As C<PL_defoutgv> "owns" a reference on its typeglob, the reference
count of the passed in typeglob is increased by one, and the reference count
of the typeglob that C<PL_defoutgv> points to is decreased by one.

=for apidoc AmnU||PL_defoutgv

See C<L</setdefout>>.

=cut
*/

void
Perl_setdefout(pTHX_ GV *gv)
{
    GV *oldgv = PL_defoutgv;

    PERL_ARGS_ASSERT_SETDEFOUT;

    SvREFCNT_inc_simple_void_NN(gv);
    PL_defoutgv = gv;
    SvREFCNT_dec(oldgv);
}

PP(pp_select)
{
    dSP; dTARGET;
    HV *hv;
    GV * const newdefout = (PL_op->op_private > 0) ? (MUTABLE_GV(POPs)) : NULL;
    GV * egv = GvEGVx(PL_defoutgv);
    GV * const *gvp;

    if (!egv)
        egv = PL_defoutgv;
    hv = isGV_with_GP(egv) ? GvSTASH(egv) : NULL;
    gvp = hv && HvHasENAME(hv)
                ? (GV**)hv_fetch(hv, GvNAME(egv), HEK_UTF8(GvNAME_HEK(egv)) ? -GvNAMELEN(egv) : GvNAMELEN(egv), FALSE)
                : NULL;
    if (gvp && *gvp == egv) {
            gv_efullname4(TARG, PL_defoutgv, NULL, TRUE);
            XPUSHTARG;
    }
    else {
            mXPUSHs(newRV(MUTABLE_SV(egv)));
    }

    if (newdefout) {
        if (!GvIO(newdefout))
            gv_IOadd(newdefout);
        setdefout(newdefout);
    }

    RETURN;
}

PP(pp_getc)
{
    dSP; dTARGET;
    /* pp_coreargs pushes a NULL to indicate no args passed to
     * CORE::getc() */
    GV * const gv =
        MAXARG==0 || (!TOPs && !POPs) ? PL_stdingv : MUTABLE_GV(POPs);
    IO *const io = GvIO(gv);

    if (MAXARG == 0)
        EXTEND(SP, 1);

    if (io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            const U8 gimme = GIMME_V;
            Perl_tied_method(aTHX_ SV_CONST(GETC), SP, MUTABLE_SV(io), mg, gimme, 0);
            if (gimme == G_SCALAR) {
                SPAGAIN;
                SvSetMagicSV_nosteal(TARG, TOPs);
            }
            return NORMAL;
        }
    }
    if (!gv || do_eof(gv)) { /* make sure we have fp with something */
        if (!io || (!IoIFP(io) && IoTYPE(io) != IoTYPE_WRONLY))
            report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);
        RETPUSHUNDEF;
    }
    TAINT;
    sv_setpvs(TARG, " ");
    *SvPVX(TARG) = PerlIO_getc(IoIFP(GvIOp(gv))); /* should never be EOF */
    if (PerlIO_isutf8(IoIFP(GvIOp(gv)))) {
        /* Find out how many bytes the char needs */
        Size_t len = UTF8SKIP(SvPVX_const(TARG));
        if (len > 1) {
            SvGROW(TARG,len+1);
            len = PerlIO_read(IoIFP(GvIOp(gv)),SvPVX(TARG)+1,len-1);
            SvCUR_set(TARG,1+len);
        }
        SvUTF8_on(TARG);
    }
    else SvUTF8_off(TARG);
    PUSHTARG;
    RETURN;
}

STATIC OP *
S_doform(pTHX_ CV *cv, GV *gv, OP *retop)
{
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;

    PERL_ARGS_ASSERT_DOFORM;

    if (CvCLONE(cv))
        cv = MUTABLE_CV(sv_2mortal(MUTABLE_SV(cv_clone(cv))));

    cx = cx_pushblock(CXt_FORMAT, gimme, PL_stack_sp, PL_savestack_ix);
    cx_pushformat(cx, cv, retop, gv);
    if (CvDEPTH(cv) >= 2)
        pad_push(CvPADLIST(cv), CvDEPTH(cv));
    PAD_SET_CUR_NOSAVE(CvPADLIST(cv), CvDEPTH(cv));

    setdefout(gv);	    /* locally select filehandle so $% et al work */
    return CvSTART(cv);
}

PP(pp_enterwrite)
{
    dSP;
    GV *gv;
    IO *io;
    GV *fgv;
    CV *cv = NULL;

    if (MAXARG == 0) {
        EXTEND(SP, 1);
        gv = PL_defoutgv;
    }
    else {
        gv = MUTABLE_GV(POPs);
        if (!gv)
            gv = PL_defoutgv;
    }
    io = GvIO(gv);
    if (!io) {
        RETPUSHNO;
    }
    if (IoFMT_GV(io))
        fgv = IoFMT_GV(io);
    else
        fgv = gv;

    assert(fgv);

    cv = GvFORM(fgv);
    if (!cv) {
        SV * const tmpsv = sv_newmortal();
        gv_efullname4(tmpsv, fgv, NULL, FALSE);
        DIE(aTHX_ "Undefined format \"%" SVf "\" called", SVfARG(tmpsv));
    }
    IoFLAGS(io) &= ~IOf_DIDTOP;
    RETURNOP(doform(cv,gv,PL_op->op_next));
}

PP(pp_leavewrite)
{
    dSP;
    GV * const gv = CX_CUR()->blk_format.gv;
    IO * const io = GvIOp(gv);
    PerlIO *ofp;
    PerlIO *fp;
    PERL_CONTEXT *cx;
    OP *retop;
    bool is_return = cBOOL(PL_op->op_type == OP_RETURN);

    if (is_return || !io || !(ofp = IoOFP(io)))
        goto forget_top;

    DEBUG_f(PerlIO_printf(Perl_debug_log, "left=%ld, todo=%ld\n",
          (long)IoLINES_LEFT(io), (long)FmLINES(PL_formtarget)));

    if (IoLINES_LEFT(io) < FmLINES(PL_formtarget) &&
        PL_formtarget != PL_toptarget)
    {
        GV *fgv;
        CV *cv;
        if (!IoTOP_GV(io)) {
            GV *topgv;

            if (!IoTOP_NAME(io)) {
                SV *topname;
                if (!IoFMT_NAME(io))
                    IoFMT_NAME(io) = savepv(GvNAME(gv));
                topname = sv_2mortal(Perl_newSVpvf(aTHX_ "%" HEKf "_TOP",
                                        HEKfARG(GvNAME_HEK(gv))));
                topgv = gv_fetchsv(topname, 0, SVt_PVFM);
                if ((topgv && GvFORM(topgv)) ||
                  !gv_fetchpvs("top", GV_NOTQUAL, SVt_PVFM))
                    IoTOP_NAME(io) = savesvpv(topname);
                else
                    IoTOP_NAME(io) = savepvs("top");
            }
            topgv = gv_fetchpv(IoTOP_NAME(io), 0, SVt_PVFM);
            if (!topgv || !GvFORM(topgv)) {
                IoLINES_LEFT(io) = IoPAGE_LEN(io);
                goto forget_top;
            }
            IoTOP_GV(io) = topgv;
        }
        if (IoFLAGS(io) & IOf_DIDTOP) {	/* Oh dear.  It still doesn't fit. */
            I32 lines = IoLINES_LEFT(io);
            const char *s = SvPVX_const(PL_formtarget);
            const char *e = SvEND(PL_formtarget);
            if (lines <= 0)		/* Yow, header didn't even fit!!! */
                goto forget_top;
            while (lines-- > 0) {
                s = (char *) memchr(s, '\n', e - s);
                if (!s)
                    break;
                s++;
            }
            if (s) {
                const STRLEN save = SvCUR(PL_formtarget);
                SvCUR_set(PL_formtarget, s - SvPVX_const(PL_formtarget));
                do_print(PL_formtarget, ofp);
                SvCUR_set(PL_formtarget, save);
                sv_chop(PL_formtarget, s);
                FmLINES(PL_formtarget) -= IoLINES_LEFT(io);
            }
        }
        if (IoLINES_LEFT(io) >= 0 && IoPAGE(io) > 0)
            do_print(GvSV(gv_fetchpvs("\f", GV_ADD, SVt_PV)), ofp);
        IoLINES_LEFT(io) = IoPAGE_LEN(io);
        IoPAGE(io)++;
        PL_formtarget = PL_toptarget;
        IoFLAGS(io) |= IOf_DIDTOP;
        fgv = IoTOP_GV(io);
        assert(fgv); /* IoTOP_GV(io) should have been set above */
        cv = GvFORM(fgv);
        if (!cv) {
            SV * const sv = sv_newmortal();
            gv_efullname4(sv, fgv, NULL, FALSE);
            DIE(aTHX_ "Undefined top format \"%" SVf "\" called", SVfARG(sv));
        }
        return doform(cv, gv, PL_op);
    }

  forget_top:
    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_FORMAT);
    SP = PL_stack_base + cx->blk_oldsp; /* ignore retval of formline */
    CX_LEAVE_SCOPE(cx);
    cx_popformat(cx);
    cx_popblock(cx);
    retop = cx->blk_sub.retop;
    CX_POP(cx);

    EXTEND(SP, 1);

    if (is_return)
        /* XXX the semantics of doing 'return' in a format aren't documented.
         * Currently we ignore any args to 'return' and just return
         * a single undef in both scalar and list contexts
         */
        PUSHs(&PL_sv_undef);
    else if (!io || !(fp = IoOFP(io))) {
        if (io && IoIFP(io))
            report_wrongway_fh(gv, '<');
        else
            report_evil_fh(gv);
        PUSHs(&PL_sv_no);
    }
    else {
        if ((IoLINES_LEFT(io) -= FmLINES(PL_formtarget)) < 0) {
            Perl_ck_warner(aTHX_ packWARN(WARN_IO), "page overflow");
        }
        if (!do_print(PL_formtarget, fp))
            PUSHs(&PL_sv_no);
        else {
            FmLINES(PL_formtarget) = 0;
            SvCUR_set(PL_formtarget, 0);
            *SvEND(PL_formtarget) = '\0';
            if (IoFLAGS(io) & IOf_FLUSH)
                (void)PerlIO_flush(fp);
            PUSHs(&PL_sv_yes);
        }
    }
    PL_formtarget = PL_bodytarget;
    RETURNOP(retop);
}

PP(pp_prtf)
{
    dSP; dMARK; dORIGMARK;
    PerlIO *fp;

    GV * const gv
        = (PL_op->op_flags & OPf_STACKED) ? MUTABLE_GV(*++MARK) : PL_defoutgv;
    IO *const io = GvIO(gv);

    /* Treat empty list as "" */
    if (MARK == SP) XPUSHs(&PL_sv_no);

    if (io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            if (MARK == ORIGMARK) {
                MEXTEND(SP, 1);
                ++MARK;
                Move(MARK, MARK + 1, (SP - MARK) + 1, SV*);
                ++SP;
            }
            return Perl_tied_method(aTHX_ SV_CONST(PRINTF), mark - 1, MUTABLE_SV(io),
                                    mg,
                                    G_SCALAR | TIED_METHOD_ARGUMENTS_ON_STACK,
                                    sp - mark);
        }
    }

    if (!io) {
        report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);
        goto just_say_no;
    }
    else if (!(fp = IoOFP(io))) {
        if (IoIFP(io))
            report_wrongway_fh(gv, '<');
        else if (ckWARN(WARN_CLOSED))
            report_evil_fh(gv);
        SETERRNO(EBADF,IoIFP(io)?RMS_FAC:RMS_IFI);
        goto just_say_no;
    }
    else {
        SV *sv = sv_newmortal();
        do_sprintf(sv, SP - MARK, MARK + 1);
        if (!do_print(sv, fp))
            goto just_say_no;

        if (IoFLAGS(io) & IOf_FLUSH)
            if (PerlIO_flush(fp) == EOF)
                goto just_say_no;
    }
    SP = ORIGMARK;
    PUSHs(&PL_sv_yes);
    RETURN;

  just_say_no:
    SP = ORIGMARK;
    PUSHs(&PL_sv_undef);
    RETURN;
}

PP(pp_sysopen)
{
    dSP;
    const int perm = (MAXARG > 3 && (TOPs || POPs)) ? POPi : 0666;
    const int mode = POPi;
    SV * const sv = POPs;
    GV * const gv = MUTABLE_GV(POPs);
    STRLEN len;

    /* Need TIEHANDLE method ? */
    const char * const tmps = SvPV_const(sv, len);
    if (do_open_raw(gv, tmps, len, mode, perm, NULL)) {
        IoLINES(GvIOp(gv)) = 0;
        PUSHs(&PL_sv_yes);
    }
    else {
        PUSHs(&PL_sv_undef);
    }
    RETURN;
}


/* also used for: pp_read() and pp_recv() (where supported) */

PP(pp_sysread)
{
    dSP; dMARK; dORIGMARK; dTARGET;
    SSize_t offset;
    IO *io;
    char *buffer;
    STRLEN orig_size;
    SSize_t length;
    SSize_t count;
    SV *bufsv;
    STRLEN blen;
    int fp_utf8;
    int buffer_utf8;
    SV *read_target;
    Size_t got = 0;
    Size_t wanted;
    bool charstart = FALSE;
    STRLEN charskip = 0;
    STRLEN skip = 0;
    GV * const gv = MUTABLE_GV(*++MARK);
    int fd;

    if ((PL_op->op_type == OP_READ || PL_op->op_type == OP_SYSREAD)
        && gv && (io = GvIO(gv)) )
    {
        const MAGIC *const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            return Perl_tied_method(aTHX_ SV_CONST(READ), mark - 1, MUTABLE_SV(io), mg,
                                    G_SCALAR | TIED_METHOD_ARGUMENTS_ON_STACK,
                                    sp - mark);
        }
    }

    if (!gv)
        goto say_undef;
    bufsv = *++MARK;
    if (! SvOK(bufsv))
        SvPVCLEAR(bufsv);
    length = SvIVx(*++MARK);
    if (length < 0)
        DIE(aTHX_ "Negative length");
    SETERRNO(0,0);
    if (MARK < SP)
        offset = SvIVx(*++MARK);
    else
        offset = 0;
    io = GvIO(gv);
    if (!io || !IoIFP(io)) {
        report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);
        goto say_undef;
    }

    /* Note that fd can here validly be -1, don't check it yet. */
    fd = PerlIO_fileno(IoIFP(io));

    if ((fp_utf8 = PerlIO_isutf8(IoIFP(io))) && !IN_BYTES) {
        if (PL_op->op_type == OP_SYSREAD || PL_op->op_type == OP_RECV) {
            Perl_croak(aTHX_
                       "%s() isn't allowed on :utf8 handles",
                       OP_DESC(PL_op));
        }
        buffer = SvPVutf8_force(bufsv, blen);
        /* UTF-8 may not have been set if they are all low bytes */
        SvUTF8_on(bufsv);
        buffer_utf8 = 0;
    }
    else {
        buffer = SvPV_force(bufsv, blen);
        buffer_utf8 = DO_UTF8(bufsv);
    }
    if (DO_UTF8(bufsv)) {
        blen = sv_len_utf8_nomg(bufsv);
    }

    charstart = TRUE;
    charskip  = 0;
    skip = 0;
    wanted = length;

#ifdef HAS_SOCKET
    if (PL_op->op_type == OP_RECV) {
        Sock_size_t bufsize;
        char namebuf[MAXPATHLEN];
        if (fd < 0) {
            SETERRNO(EBADF,SS_IVCHAN);
            goto say_undef;
        }
#if (defined(VMS_DO_SOCKETS) && defined(DECCRTL_SOCKETS)) || defined(__QNXNTO__)
        bufsize = sizeof (struct sockaddr_in);
#else
        bufsize = sizeof namebuf;
#endif
#ifdef OS2	/* At least Warp3+IAK: only the first byte of bufsize set */
        if (bufsize >= 256)
            bufsize = 255;
#endif
        buffer = SvGROW(bufsv, (STRLEN)(length+1));
        /* 'offset' means 'flags' here */
        count = PerlSock_recvfrom(fd, buffer, length, offset,
                                  (struct sockaddr *)namebuf, &bufsize);
        if (count < 0)
            goto say_undef;
        /* MSG_TRUNC can give oversized count; quietly lose it */
        if (count > length)
            count = length;
        SvCUR_set(bufsv, count);
        *SvEND(bufsv) = '\0';
        (void)SvPOK_only(bufsv);
        if (fp_utf8)
            SvUTF8_on(bufsv);
        SvSETMAGIC(bufsv);
        /* This should not be marked tainted if the fp is marked clean */
        if (!(IoFLAGS(io) & IOf_UNTAINT))
            SvTAINTED_on(bufsv);
        SP = ORIGMARK;
#if defined(__CYGWIN__)
        /* recvfrom() on cygwin doesn't set bufsize at all for
           connected sockets, leaving us with trash in the returned
           name, so use the same test as the Win32 code to check if it
           wasn't set, and set it [perl #118843] */
        if (bufsize == sizeof namebuf)
            bufsize = 0;
#endif
        sv_setpvn(TARG, namebuf, bufsize);
        PUSHs(TARG);
        RETURN;
    }
#endif
    if (offset < 0) {
        if (-offset > (SSize_t)blen)
            DIE(aTHX_ "Offset outside string");
        offset += blen;
    }
    if (DO_UTF8(bufsv)) {
        /* convert offset-as-chars to offset-as-bytes */
        if (offset >= (SSize_t)blen)
            offset += SvCUR(bufsv) - blen;
        else
            offset = utf8_hop((U8 *)buffer,offset) - (U8 *) buffer;
    }

 more_bytes:
    /* Reestablish the fd in case it shifted from underneath us. */
    fd = PerlIO_fileno(IoIFP(io));

    orig_size = SvCUR(bufsv);
    /* Allocating length + offset + 1 isn't perfect in the case of reading
       bytes from a byte file handle into a UTF8 buffer, but it won't harm us
       unduly.
       (should be 2 * length + offset + 1, or possibly something longer if
       IN_ENCODING Is true) */
    buffer  = SvGROW(bufsv, (STRLEN)(length+offset+1));
    if (offset > 0 && offset > (SSize_t)orig_size) { /* Zero any newly allocated space */
        Zero(buffer+orig_size, offset-orig_size, char);
    }
    buffer = buffer + offset;
    if (!buffer_utf8) {
        read_target = bufsv;
    } else {
        /* Best to read the bytes into a new SV, upgrade that to UTF8, then
           concatenate it to the current buffer.  */

        /* Truncate the existing buffer to the start of where we will be
           reading to:  */
        SvCUR_set(bufsv, offset);

        read_target = newSV_type_mortal(SVt_PV);
        buffer = sv_grow_fresh(read_target, (STRLEN)(length + 1));
    }

    if (PL_op->op_type == OP_SYSREAD) {
#ifdef PERL_SOCK_SYSREAD_IS_RECV
        if (IoTYPE(io) == IoTYPE_SOCKET) {
            if (fd < 0) {
                SETERRNO(EBADF,SS_IVCHAN);
                count = -1;
            }
            else
                count = PerlSock_recv(fd, buffer, length, 0);
        }
        else
#endif
        {
            if (fd < 0) {
                SETERRNO(EBADF,RMS_IFI);
                count = -1;
            }
            else
                count = PerlLIO_read(fd, buffer, length);
        }
    }
    else
    {
        count = PerlIO_read(IoIFP(io), buffer, length);
        /* PerlIO_read() - like fread() returns 0 on both error and EOF */
        if (count == 0 && PerlIO_error(IoIFP(io)))
            count = -1;
    }
    if (count < 0) {
        if (IoTYPE(io) == IoTYPE_WRONLY)
            report_wrongway_fh(gv, '>');
        goto say_undef;
    }
    SvCUR_set(read_target, count+(buffer - SvPVX_const(read_target)));
    *SvEND(read_target) = '\0';
    (void)SvPOK_only(read_target);
    if (fp_utf8 && !IN_BYTES) {
        /* Look at utf8 we got back and count the characters */
        const char *bend = buffer + count;
        while (buffer < bend) {
            if (charstart) {
                skip = UTF8SKIP(buffer);
                charskip = 0;
            }
            if (buffer - charskip + skip > bend) {
                /* partial character - try for rest of it */
                length = skip - (bend-buffer);
                offset = bend - SvPVX_const(bufsv);
                charstart = FALSE;
                charskip += count;
                goto more_bytes;
            }
            else {
                got++;
                buffer += skip;
                charstart = TRUE;
                charskip  = 0;
            }
        }
        /* If we have not 'got' the number of _characters_ we 'wanted' get some more
           provided amount read (count) was what was requested (length)
         */
        if (got < wanted && count == length) {
            length = wanted - got;
            offset = bend - SvPVX_const(bufsv);
            goto more_bytes;
        }
        /* return value is character count */
        count = got;
        SvUTF8_on(bufsv);
    }
    else if (buffer_utf8) {
        /* Let svcatsv upgrade the bytes we read in to utf8.
           The buffer is a mortal so will be freed soon.  */
        sv_catsv_nomg(bufsv, read_target);
    }
    SvSETMAGIC(bufsv);
    /* This should not be marked tainted if the fp is marked clean */
    if (!(IoFLAGS(io) & IOf_UNTAINT))
        SvTAINTED_on(bufsv);
    SP = ORIGMARK;
    PUSHi(count);
    RETURN;

  say_undef:
    SP = ORIGMARK;
    RETPUSHUNDEF;
}


/* also used for: pp_send() where defined */

PP(pp_syswrite)
{
    dSP; dMARK; dORIGMARK; dTARGET;
    SV *bufsv;
    const char *buffer;
    SSize_t retval;
    STRLEN blen;
    const int op_type = PL_op->op_type;
    bool doing_utf8;
    U8 *tmpbuf = NULL;
    GV *const gv = MUTABLE_GV(*++MARK);
    IO *const io = GvIO(gv);
    int fd;

    if (op_type == OP_SYSWRITE && io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            if (MARK == SP - 1) {
                SV *sv = *SP;
                mXPUSHi(sv_len(sv));
                PUTBACK;
            }

            return Perl_tied_method(aTHX_ SV_CONST(WRITE), mark - 1, MUTABLE_SV(io), mg,
                                    G_SCALAR | TIED_METHOD_ARGUMENTS_ON_STACK,
                                    sp - mark);
        }
    }
    if (!gv)
        goto say_undef;

    bufsv = *++MARK;

    SETERRNO(0,0);
    if (!io || !IoIFP(io) || IoTYPE(io) == IoTYPE_RDONLY) {
        retval = -1;
        if (io && IoIFP(io))
            report_wrongway_fh(gv, '<');
        else
            report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);
        goto say_undef;
    }
    fd = PerlIO_fileno(IoIFP(io));
    if (fd < 0) {
        SETERRNO(EBADF,SS_IVCHAN);
        retval = -1;
        goto say_undef;
    }

    /* Do this first to trigger any overloading.  */
    buffer = SvPV_const(bufsv, blen);
    doing_utf8 = DO_UTF8(bufsv);

    if (PerlIO_isutf8(IoIFP(io))) {
        Perl_croak(aTHX_
                   "%s() isn't allowed on :utf8 handles",
                   OP_DESC(PL_op));
    }
    else if (doing_utf8) {
        STRLEN tmplen = blen;
        U8 * const result = bytes_from_utf8((const U8*) buffer, &tmplen, &doing_utf8);
        if (!doing_utf8) {
            tmpbuf = result;
            buffer = (char *) tmpbuf;
            blen = tmplen;
        }
        else {
            assert((char *)result == buffer);
            Perl_croak(aTHX_ "Wide character in %s", OP_DESC(PL_op));
        }
    }

#ifdef HAS_SOCKET
    if (op_type == OP_SEND) {
        const int flags = SvIVx(*++MARK);
        if (SP > MARK) {
            STRLEN mlen;
            char * const sockbuf = SvPVx(*++MARK, mlen);
            retval = PerlSock_sendto(fd, buffer, blen,
                                     flags, (struct sockaddr *)sockbuf, mlen);
        }
        else {
            retval = PerlSock_send(fd, buffer, blen, flags);
        }
    }
    else
#endif
    {
        Size_t length = 0; /* This length is in characters.  */
        IV offset;

        if (MARK >= SP) {
            length = blen;
        } else {
#if Size_t_size > IVSIZE
            length = (Size_t)SvNVx(*++MARK);
#else
            length = (Size_t)SvIVx(*++MARK);
#endif
            if ((SSize_t)length < 0) {
                Safefree(tmpbuf);
                DIE(aTHX_ "Negative length");
            }
        }

        if (MARK < SP) {
            offset = SvIVx(*++MARK);
            if (offset < 0) {
                if (-offset > (IV)blen) {
                    Safefree(tmpbuf);
                    DIE(aTHX_ "Offset outside string");
                }
                offset += blen;
            } else if (offset > (IV)blen) {
                Safefree(tmpbuf);
                DIE(aTHX_ "Offset outside string");
            }
        } else
            offset = 0;
        if (length > blen - offset)
            length = blen - offset;
        buffer = buffer+offset;

#ifdef PERL_SOCK_SYSWRITE_IS_SEND
        if (IoTYPE(io) == IoTYPE_SOCKET) {
            retval = PerlSock_send(fd, buffer, length, 0);
        }
        else
#endif
        {
            /* See the note at doio.c:do_print about filesize limits. --jhi */
            retval = PerlLIO_write(fd, buffer, length);
        }
    }

    if (retval < 0)
        goto say_undef;
    SP = ORIGMARK;

    Safefree(tmpbuf);
#if Size_t_size > IVSIZE
    PUSHn(retval);
#else
    PUSHi(retval);
#endif
    RETURN;

  say_undef:
    Safefree(tmpbuf);
    SP = ORIGMARK;
    RETPUSHUNDEF;
}

PP(pp_eof)
{
    dSP;
    GV *gv;
    IO *io;
    const MAGIC *mg;
    /*
     * in Perl 5.12 and later, the additional parameter is a bitmask:
     * 0 = eof
     * 1 = eof(FH)
     * 2 = eof()  <- ARGV magic
     *
     * I'll rely on the compiler's trace flow analysis to decide whether to
     * actually assign this out here, or punt it into the only block where it is
     * used. Doing it out here is DRY on the condition logic.
     */
    unsigned int which;

    if (MAXARG) {
        gv = PL_last_in_gv = MUTABLE_GV(POPs);	/* eof(FH) */
        which = 1;
    }
    else {
        EXTEND(SP, 1);

        if (PL_op->op_flags & OPf_SPECIAL) {
            gv = PL_last_in_gv = GvEGVx(PL_argvgv);	/* eof() - ARGV magic */
            which = 2;
        }
        else {
            gv = PL_last_in_gv;			/* eof */
            which = 0;
        }
    }

    if (!gv)
        RETPUSHYES;

    if ((io = GvIO(gv)) && (mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar))) {
        return tied_method1(SV_CONST(EOF), SP, MUTABLE_SV(io), mg, newSVuv(which));
    }

    if (!MAXARG && (PL_op->op_flags & OPf_SPECIAL)) {	/* eof() */
        if (io && !IoIFP(io)) {
            if ((IoFLAGS(io) & IOf_START) && av_count(GvAVn(gv)) == 0) {
                SV ** svp;
                IoLINES(io) = 0;
                IoFLAGS(io) &= ~IOf_START;
                do_open6(gv, "-", 1, NULL, NULL, 0);
                svp = &GvSV(gv);
                if (*svp) {
                    SV * sv = *svp;
                    sv_setpvs(sv, "-");
                    SvSETMAGIC(sv);
                }
                else
                    *svp = newSVpvs("-");
            }
            else if (!nextargv(gv, FALSE))
                RETPUSHYES;
        }
    }

    PUSHs(boolSV(do_eof(gv)));
    RETURN;
}

PP(pp_tell)
{
    dSP; dTARGET;
    GV *gv;
    IO *io;

    if (MAXARG != 0 && (TOPs || POPs))
        PL_last_in_gv = MUTABLE_GV(POPs);
    else
        EXTEND(SP, 1);
    gv = PL_last_in_gv;

    io = GvIO(gv);
    if (io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
            return tied_method0(SV_CONST(TELL), SP, MUTABLE_SV(io), mg);
        }
    }
    else if (!gv) {
        if (!errno)
            SETERRNO(EBADF,RMS_IFI);
        PUSHi(-1);
        RETURN;
    }

#if LSEEKSIZE > IVSIZE
    PUSHn( (NV)do_tell(gv) );
#else
    PUSHi( (IV)do_tell(gv) );
#endif
    RETURN;
}


/* also used for: pp_seek() */

PP(pp_sysseek)
{
    dSP;
    const int whence = POPi;
#if LSEEKSIZE > IVSIZE
    const Off_t offset = (Off_t)SvNVx(POPs);
#else
    const Off_t offset = (Off_t)SvIVx(POPs);
#endif

    GV * const gv = PL_last_in_gv = MUTABLE_GV(POPs);
    IO *const io = GvIO(gv);

    if (io) {
        const MAGIC * const mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar);
        if (mg) {
#if LSEEKSIZE > IVSIZE
            SV *const offset_sv = newSVnv((NV) offset);
#else
            SV *const offset_sv = newSViv(offset);
#endif

            return tied_method2(SV_CONST(SEEK), SP, MUTABLE_SV(io), mg, offset_sv,
                                newSViv(whence));
        }
    }

    if (PL_op->op_type == OP_SEEK)
        PUSHs(boolSV(do_seek(gv, offset, whence)));
    else {
        const Off_t sought = do_sysseek(gv, offset, whence);
        if (sought < 0)
            PUSHs(&PL_sv_undef);
        else {
            SV* const sv = sought ?
#if LSEEKSIZE > IVSIZE
                newSVnv((NV)sought)
#else
                newSViv(sought)
#endif
                : newSVpvn(zero_but_true, ZBTLEN);
            mPUSHs(sv);
        }
    }
    RETURN;
}

PP(pp_truncate)
{
    dSP;
    /* There seems to be no consensus on the length type of truncate()
     * and ftruncate(), both off_t and size_t have supporters. In
     * general one would think that when using large files, off_t is
     * at least as wide as size_t, so using an off_t should be okay. */
    /* XXX Configure probe for the length type of *truncate() needed XXX */
    Off_t len;

#if Off_t_size > IVSIZE
    len = (Off_t)POPn;
#else
    len = (Off_t)POPi;
#endif
    /* Checking for length < 0 is problematic as the type might or
     * might not be signed: if it is not, clever compilers will moan. */
    /* XXX Configure probe for the signedness of the length type of *truncate() needed? XXX */
    SETERRNO(0,0);
    {
        SV * const sv = POPs;
        int result = 1;
        GV *tmpgv;
        IO *io;

        if (PL_op->op_flags & OPf_SPECIAL
                       ? (tmpgv = gv_fetchsv(sv, 0, SVt_PVIO), 1)
                       : cBOOL(tmpgv = MAYBE_DEREF_GV(sv)) )
        {
            io = GvIO(tmpgv);
            if (!io)
                result = 0;
            else {
                PerlIO *fp;
            do_ftruncate_io:
                TAINT_PROPER("truncate");
                if (!(fp = IoIFP(io))) {
                    result = 0;
                }
                else {
                    int fd = PerlIO_fileno(fp);
                    if (fd < 0) {
                        SETERRNO(EBADF,RMS_IFI);
                        result = 0;
                    } else {
                        if (len < 0) {
                            SETERRNO(EINVAL, LIB_INVARG);
                            result = 0;
                        } else {
                           PerlIO_flush(fp);
#ifdef HAS_TRUNCATE
                           if (ftruncate(fd, len) < 0)
#else
                           if (my_chsize(fd, len) < 0)
#endif
                               result = 0;
                        }
                    }
                }
            }
        }
        else if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVIO) {
                io = MUTABLE_IO(SvRV(sv)); /* *main::FRED{IO} for example */
                goto do_ftruncate_io;
        }
        else {
            const char * const name = SvPV_nomg_const_nolen(sv);
            TAINT_PROPER("truncate");
#ifdef HAS_TRUNCATE
            if (truncate(name, len) < 0)
                result = 0;
#else
            {
                int mode = O_RDWR;
                int tmpfd;

#if defined(USE_64_BIT_RAWIO) && defined(O_LARGEFILE)
                mode |= O_LARGEFILE;	/* Transparently largefiley. */
#endif
#ifdef O_BINARY
                /* On open(), the Win32 CRT tries to seek around text
                 * files using 32-bit offsets, which causes the open()
                 * to fail on large files, so open in binary mode.
                 */
                mode |= O_BINARY;
#endif
                tmpfd = PerlLIO_open_cloexec(name, mode);

                if (tmpfd < 0) {
                    result = 0;
                } else {
                    if (my_chsize(tmpfd, len) < 0)
                        result = 0;
                    PerlLIO_close(tmpfd);
                }
            }
#endif
        }

        if (result)
            RETPUSHYES;
        if (!errno)
            SETERRNO(EBADF,RMS_IFI);
        RETPUSHUNDEF;
    }
}


/* also used for: pp_fcntl() */

PP(pp_ioctl)
{
    dSP; dTARGET;
    SV * const argsv = POPs;
    const unsigned int func = POPu;
    int optype;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);
    char *s;
    IV retval;

    if (!IoIFP(io)) {
        report_evil_fh(gv);
        SETERRNO(EBADF,RMS_IFI);	/* well, sort of... */
        RETPUSHUNDEF;
    }

    if (SvPOK(argsv) || !SvNIOK(argsv)) {
        STRLEN len;
        STRLEN need;
        s = SvPV_force(argsv, len);
        need = IOCPARM_LEN(func);
        if (len < need) {
            s = Sv_Grow(argsv, need + 1);
            SvCUR_set(argsv, need);
        }

        s[SvCUR(argsv)] = 17;	/* a little sanity check here */
    }
    else {
        retval = SvIV(argsv);
        s = INT2PTR(char*,retval);		/* ouch */
    }

    optype = PL_op->op_type;
    TAINT_PROPER(PL_op_desc[optype]);

    if (optype == OP_IOCTL)
#ifdef HAS_IOCTL
        retval = PerlLIO_ioctl(PerlIO_fileno(IoIFP(io)), func, s);
#else
        DIE(aTHX_ "ioctl is not implemented");
#endif
    else
#ifndef HAS_FCNTL
      DIE(aTHX_ "fcntl is not implemented");
#elif defined(OS2) && defined(__EMX__)
        retval = fcntl(PerlIO_fileno(IoIFP(io)), func, (int)s);
#else
        retval = fcntl(PerlIO_fileno(IoIFP(io)), func, s);
#endif

#if defined(HAS_IOCTL) || defined(HAS_FCNTL)
    if (SvPOK(argsv)) {
        if (s[SvCUR(argsv)] != 17)
            DIE(aTHX_ "Possible memory corruption: %s overflowed 3rd argument",
                OP_NAME(PL_op));
        s[SvCUR(argsv)] = 0;		/* put our null back */
        SvSETMAGIC(argsv);		/* Assume it has changed */
    }

    if (retval == -1)
        RETPUSHUNDEF;
    if (retval != 0) {
        PUSHi(retval);
    }
    else {
        PUSHp(zero_but_true, ZBTLEN);
    }
#endif
    RETURN;
}

PP(pp_flock)
{
#ifdef FLOCK
    dSP; dTARGET;
    I32 value;
    const int argtype = POPi;
    GV * const gv = MUTABLE_GV(POPs);
    IO *const io = GvIO(gv);
    PerlIO *const fp = io ? IoIFP(io) : NULL;

    /* XXX Looks to me like io is always NULL at this point */
    if (fp) {
        (void)PerlIO_flush(fp);
        value = (I32)(PerlLIO_flock(PerlIO_fileno(fp), argtype) >= 0);
    }
    else {
        report_evil_fh(gv);
        value = 0;
        SETERRNO(EBADF,RMS_IFI);
    }
    PUSHi(value);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "flock");
#endif
}

/* Sockets. */

#ifdef HAS_SOCKET

PP(pp_socket)
{
    dSP;
    const int protocol = POPi;
    const int type = POPi;
    const int domain = POPi;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);
    int fd;

    if (IoIFP(io))
        do_close(gv, FALSE);

    TAINT_PROPER("socket");
    fd = PerlSock_socket_cloexec(domain, type, protocol);
    if (fd < 0) {
        RETPUSHUNDEF;
    }
    IoIFP(io) = PerlIO_fdopen(fd, "r" SOCKET_OPEN_MODE); /* stdio gets confused about sockets */
    IoOFP(io) = PerlIO_fdopen(fd, "w" SOCKET_OPEN_MODE);
    IoTYPE(io) = IoTYPE_SOCKET;
    if (!IoIFP(io) || !IoOFP(io)) {
        if (IoIFP(io)) PerlIO_close(IoIFP(io));
        if (IoOFP(io)) PerlIO_close(IoOFP(io));
        if (!IoIFP(io) && !IoOFP(io)) PerlLIO_close(fd);
        RETPUSHUNDEF;
    }

    RETPUSHYES;
}
#endif

PP(pp_sockpair)
{
#if defined (HAS_SOCKETPAIR) || (defined (HAS_SOCKET) && defined(SOCK_DGRAM) && defined(AF_INET) && defined(PF_INET))
    dSP;
    int fd[2];
    const int protocol = POPi;
    const int type = POPi;
    const int domain = POPi;

    GV * const gv2 = MUTABLE_GV(POPs);
    IO * const io2 = GvIOn(gv2);
    GV * const gv1 = MUTABLE_GV(POPs);
    IO * const io1 = GvIOn(gv1);

    if (IoIFP(io1))
        do_close(gv1, FALSE);
    if (IoIFP(io2))
        do_close(gv2, FALSE);

    TAINT_PROPER("socketpair");
    if (PerlSock_socketpair_cloexec(domain, type, protocol, fd) < 0)
        RETPUSHUNDEF;
    IoIFP(io1) = PerlIO_fdopen(fd[0], "r" SOCKET_OPEN_MODE);
    IoOFP(io1) = PerlIO_fdopen(fd[0], "w" SOCKET_OPEN_MODE);
    IoTYPE(io1) = IoTYPE_SOCKET;
    IoIFP(io2) = PerlIO_fdopen(fd[1], "r" SOCKET_OPEN_MODE);
    IoOFP(io2) = PerlIO_fdopen(fd[1], "w" SOCKET_OPEN_MODE);
    IoTYPE(io2) = IoTYPE_SOCKET;
    if (!IoIFP(io1) || !IoOFP(io1) || !IoIFP(io2) || !IoOFP(io2)) {
        if (IoIFP(io1)) PerlIO_close(IoIFP(io1));
        if (IoOFP(io1)) PerlIO_close(IoOFP(io1));
        if (!IoIFP(io1) && !IoOFP(io1)) PerlLIO_close(fd[0]);
        if (IoIFP(io2)) PerlIO_close(IoIFP(io2));
        if (IoOFP(io2)) PerlIO_close(IoOFP(io2));
        if (!IoIFP(io2) && !IoOFP(io2)) PerlLIO_close(fd[1]);
        RETPUSHUNDEF;
    }

    RETPUSHYES;
#else
    DIE(aTHX_ PL_no_sock_func, "socketpair");
#endif
}

#ifdef HAS_SOCKET

/* also used for: pp_connect() */

PP(pp_bind)
{
    dSP;
    SV * const addrsv = POPs;
    /* OK, so on what platform does bind modify addr?  */
    const char *addr;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);
    STRLEN len;
    int op_type;
    int fd;

    if (!IoIFP(io))
        goto nuts;
    fd = PerlIO_fileno(IoIFP(io));
    if (fd < 0)
        goto nuts;

    addr = SvPV_const(addrsv, len);
    op_type = PL_op->op_type;
    TAINT_PROPER(PL_op_desc[op_type]);
    if ((op_type == OP_BIND
         ? PerlSock_bind(fd, (struct sockaddr *)addr, len)
         : PerlSock_connect(fd, (struct sockaddr *)addr, len))
        >= 0)
        RETPUSHYES;
    else
        RETPUSHUNDEF;

  nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,SS_IVCHAN);
    RETPUSHUNDEF;
}

PP(pp_listen)
{
    dSP;
    const int backlog = POPi;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoIFP(io))
        goto nuts;

    if (PerlSock_listen(PerlIO_fileno(IoIFP(io)), backlog) >= 0)
        RETPUSHYES;
    else
        RETPUSHUNDEF;

  nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,SS_IVCHAN);
    RETPUSHUNDEF;
}

PP(pp_accept)
{
    dSP; dTARGET;
    IO *nstio;
    char namebuf[MAXPATHLEN];
#if (defined(VMS_DO_SOCKETS) && defined(DECCRTL_SOCKETS)) || defined(__QNXNTO__)
    Sock_size_t len = sizeof (struct sockaddr_in);
#else
    Sock_size_t len = sizeof namebuf;
#endif
    GV * const ggv = MUTABLE_GV(POPs);
    GV * const ngv = MUTABLE_GV(POPs);
    int fd;

    IO * const gstio = GvIO(ggv);
    if (!gstio || !IoIFP(gstio))
        goto nuts;

    nstio = GvIOn(ngv);
    fd = PerlSock_accept_cloexec(PerlIO_fileno(IoIFP(gstio)), (struct sockaddr *) namebuf, &len);
#if defined(OEMVS)
    if (len == 0) {
        /* Some platforms indicate zero length when an AF_UNIX client is
         * not bound. Simulate a non-zero-length sockaddr structure in
         * this case. */
        namebuf[0] = 0;        /* sun_len */
        namebuf[1] = AF_UNIX;  /* sun_family */
        len = 2;
    }
#endif

    if (fd < 0)
        goto badexit;
    if (IoIFP(nstio))
        do_close(ngv, FALSE);
    IoIFP(nstio) = PerlIO_fdopen(fd, "r" SOCKET_OPEN_MODE);
    IoOFP(nstio) = PerlIO_fdopen(fd, "w" SOCKET_OPEN_MODE);
    IoTYPE(nstio) = IoTYPE_SOCKET;
    if (!IoIFP(nstio) || !IoOFP(nstio)) {
        if (IoIFP(nstio)) PerlIO_close(IoIFP(nstio));
        if (IoOFP(nstio)) PerlIO_close(IoOFP(nstio));
        if (!IoIFP(nstio) && !IoOFP(nstio)) PerlLIO_close(fd);
        goto badexit;
    }

#ifdef __SCO_VERSION__
    len = sizeof (struct sockaddr_in); /* OpenUNIX 8 somehow truncates info */
#endif

    PUSHp(namebuf, len);
    RETURN;

  nuts:
    report_evil_fh(ggv);
    SETERRNO(EBADF,SS_IVCHAN);

  badexit:
    RETPUSHUNDEF;

}

PP(pp_shutdown)
{
    dSP; dTARGET;
    const int how = POPi;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoIFP(io))
        goto nuts;

    PUSHi( PerlSock_shutdown(PerlIO_fileno(IoIFP(io)), how) >= 0 );
    RETURN;

  nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,SS_IVCHAN);
    RETPUSHUNDEF;
}

#ifndef PERL_GETSOCKOPT_SIZE
#define PERL_GETSOCKOPT_SIZE 1024
#endif

/* also used for: pp_gsockopt() */

PP(pp_ssockopt)
{
    dSP;
    const int optype = PL_op->op_type;
    SV * const sv = (optype == OP_GSOCKOPT) ? sv_2mortal(newSV(PERL_GETSOCKOPT_SIZE+1)) : POPs;
    const unsigned int optname = (unsigned int) POPi;
    const unsigned int lvl = (unsigned int) POPi;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);
    int fd;
    Sock_size_t len;

    if (!IoIFP(io))
        goto nuts;

    fd = PerlIO_fileno(IoIFP(io));
    if (fd < 0)
        goto nuts;
    switch (optype) {
    case OP_GSOCKOPT:
        /* Note: there used to be an explicit SvGROW(sv,257) here, but
         * this is redundant given the sv initialization ternary above */
        (void)SvPOK_only(sv);
        SvCUR_set(sv, PERL_GETSOCKOPT_SIZE);
        *SvEND(sv) ='\0';
        len = SvCUR(sv);
        if (PerlSock_getsockopt(fd, lvl, optname, SvPVX(sv), &len) < 0)
            goto nuts2;
#if defined(_AIX)
        /* XXX Configure test: does getsockopt set the length properly? */
        if (len == PERL_GETSOCKOPT_SIZE)
            len = sizeof(int);
#endif
        SvCUR_set(sv, len);
        *SvEND(sv) ='\0';
        PUSHs(sv);
        break;
    case OP_SSOCKOPT: {
            const char *buf;
            int aint;
            SvGETMAGIC(sv);
            if (SvPOK(sv) && !SvIsBOOL(sv)) { /* sv is originally a string */
                STRLEN l;
                buf = SvPVbyte_nomg(sv, l);
                len = l;
            }
            else {
                aint = (int)SvIV_nomg(sv);
                buf = (const char *) &aint;
                len = sizeof(int);
            }
            if (PerlSock_setsockopt(fd, lvl, optname, buf, len) < 0)
                goto nuts2;
            PUSHs(&PL_sv_yes);
        }
        break;
    }
    RETURN;

  nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,SS_IVCHAN);
  nuts2:
    RETPUSHUNDEF;

}


/* also used for: pp_getsockname() */

PP(pp_getpeername)
{
    dSP;
    const int optype = PL_op->op_type;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);
    Sock_size_t len;
    SV *sv;
    int fd;

    if (!IoIFP(io))
        goto nuts;

#ifdef HAS_SOCKADDR_STORAGE
    len = sizeof(struct sockaddr_storage);
#else
    len = 256;
#endif
    sv = sv_2mortal(newSV(len+1));
    (void)SvPOK_only(sv);
    SvCUR_set(sv, len);
    *SvEND(sv) ='\0';
    fd = PerlIO_fileno(IoIFP(io));
    if (fd < 0)
        goto nuts;
    switch (optype) {
    case OP_GETSOCKNAME:
        if (PerlSock_getsockname(fd, (struct sockaddr *)SvPVX(sv), &len) < 0)
            goto nuts2;
        break;
    case OP_GETPEERNAME:
        if (PerlSock_getpeername(fd, (struct sockaddr *)SvPVX(sv), &len) < 0)
            goto nuts2;
#if defined(VMS_DO_SOCKETS) && defined (DECCRTL_SOCKETS)
        {
            static const char nowhere[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
            /* If the call succeeded, make sure we don't have a zeroed port/addr */
            if (((struct sockaddr *)SvPVX_const(sv))->sa_family == AF_INET &&
                !memcmp(SvPVX_const(sv) + sizeof(u_short), nowhere,
                        sizeof(u_short) + sizeof(struct in_addr))) {
                goto nuts2;	
            }
        }
#endif
        break;
    }
#ifdef BOGUS_GETNAME_RETURN
    /* Interactive Unix, getpeername() and getsockname()
      does not return valid namelen */
    if (len == BOGUS_GETNAME_RETURN)
        len = sizeof(struct sockaddr);
#endif
    SvCUR_set(sv, len);
    *SvEND(sv) ='\0';
    PUSHs(sv);
    RETURN;

  nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,SS_IVCHAN);
  nuts2:
    RETPUSHUNDEF;
}

#endif

/* Stat calls. */

/* also used for: pp_lstat() */

PP(pp_stat)
{
    dSP;
    GV *gv = NULL;
    IO *io = NULL;
    U8 gimme;
    I32 max = 13;
    SV* sv;

    if (PL_op->op_flags & OPf_REF ? (gv = cGVOP_gv, 1)
                                  : cBOOL((sv=POPs, gv = MAYBE_DEREF_GV(sv))))
    {
        if (PL_op->op_type == OP_LSTAT) {
            if (gv != PL_defgv) {
            do_fstat_warning_check:
                Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                               "lstat() on filehandle%s%" SVf,
                                gv ? " " : "",
                                SVfARG(gv
                                        ? newSVhek_mortal(GvENAME_HEK(gv))
                                        : &PL_sv_no));
            } else if (PL_laststype != OP_LSTAT)
                /* diag_listed_as: The stat preceding %s wasn't an lstat */
                Perl_croak(aTHX_ "The stat preceding lstat() wasn't an lstat");
        }

        if (gv == PL_defgv) {
            if (PL_laststatval < 0)
                SETERRNO(EBADF,RMS_IFI);
        } else {
          do_fstat_have_io:
            PL_laststype = OP_STAT;
            PL_statgv = gv ? gv : (GV *)io;
            SvPVCLEAR(PL_statname);
            if(gv) {
                io = GvIO(gv);
            }
            if (io) {
                    if (IoIFP(io)) {
                        int fd = PerlIO_fileno(IoIFP(io));
                        if (fd < 0) {
                            report_evil_fh(gv);
                            PL_laststatval = -1;
                            SETERRNO(EBADF,RMS_IFI);
                        } else {
                            PL_laststatval = PerlLIO_fstat(fd, &PL_statcache);
                        }
                    } else if (IoDIRP(io)) {
                        PL_laststatval =
                            PerlLIO_fstat(my_dirfd(IoDIRP(io)), &PL_statcache);
                    } else {
                        report_evil_fh(gv);
                        PL_laststatval = -1;
                        SETERRNO(EBADF,RMS_IFI);
                    }
            } else {
                report_evil_fh(gv);
                PL_laststatval = -1;
                SETERRNO(EBADF,RMS_IFI);
            }
        }

        if (PL_laststatval < 0) {
            max = 0;
        }
    }
    else {
        const char *file;
        const char *temp;
        STRLEN len;
        if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVIO) { 
            io = MUTABLE_IO(SvRV(sv));
            if (PL_op->op_type == OP_LSTAT)
                goto do_fstat_warning_check;
            goto do_fstat_have_io; 
        }
        SvTAINTED_off(PL_statname); /* previous tainting irrelevant */
        temp = SvPV_nomg_const(sv, len);
        sv_setpv(PL_statname, temp);
        PL_statgv = NULL;
        PL_laststype = PL_op->op_type;
        file = SvPV_nolen_const(PL_statname);
        if (!IS_SAFE_PATHNAME(temp, len, OP_NAME(PL_op))) {
            PL_laststatval = -1;
        }
        else if (PL_op->op_type == OP_LSTAT)
            PL_laststatval = PerlLIO_lstat(file, &PL_statcache);
        else
            PL_laststatval = PerlLIO_stat(file, &PL_statcache);
        if (PL_laststatval < 0) {
            if (ckWARN(WARN_NEWLINE) && should_warn_nl(file)) {
                /* PL_warn_nl is constant */
                GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
                Perl_warner(aTHX_ packWARN(WARN_NEWLINE), PL_warn_nl, "stat");
                GCC_DIAG_RESTORE_STMT;
            }
            max = 0;
        }
    }

    gimme = GIMME_V;
    if (gimme != G_LIST) {
        if (gimme != G_VOID)
            XPUSHs(boolSV(max));
        RETURN;
    }
    if (max) {
        EXTEND(SP, max);
        EXTEND_MORTAL(max);
#if ST_DEV_SIZE < IVSIZE || (ST_DEV_SIZE == IVSIZE && ST_DEV_SIGN < 0)
        mPUSHi(PL_statcache.st_dev);
#elif ST_DEV_SIZE == IVSIZE
        mPUSHu(PL_statcache.st_dev);
#else
#  if ST_DEV_SIGN < 0
        if (LIKELY((IV)PL_statcache.st_dev == PL_statcache.st_dev)) {
            mPUSHi((IV)PL_statcache.st_dev);
        }
#  else
        if (LIKELY((UV)PL_statcache.st_dev == PL_statcache.st_dev)) {
            mPUSHu((UV)PL_statcache.st_dev);
        }
#  endif
        else {
            char buf[sizeof(PL_statcache.st_dev)*3+1];
            /* sv_catpvf() casts 'j' size values down to IV, so it
               isn't suitable for use here.
            */
#    if defined(I_INTTYPES) && defined(HAS_SNPRINTF)
#      if ST_DEV_SIGN < 0
            int size = snprintf(buf, sizeof(buf), "%" PRIdMAX, (intmax_t)PL_statcache.st_dev);
#      else
            int size = snprintf(buf, sizeof(buf), "%" PRIuMAX, (uintmax_t)PL_statcache.st_dev);
#      endif
            STATIC_ASSERT_STMT(sizeof(intmax_t) >= sizeof(PL_statcache.st_dev));
            mPUSHp(buf, size);
#    else
#      error extraordinarily large st_dev but no inttypes.h or no snprintf
#    endif
        }
#endif
        {
            /*
             * We try to represent st_ino as a native IV or UV where
             * possible, but fall back to a decimal string where
             * necessary.  The code to generate these decimal strings
             * is quite obtuse, because (a) we're portable to non-POSIX
             * platforms where st_ino might be signed; (b) we didn't
             * necessarily detect at Configure time whether st_ino is
             * signed; (c) we're portable to non-POSIX platforms where
             * ino_t isn't defined, so have no name for the type of
             * st_ino; and (d) sprintf() doesn't necessarily support
             * integers as large as st_ino.
             */
            bool neg;
            Stat_t s;
            CLANG_DIAG_IGNORE_STMT(-Wtautological-compare);
            GCC_DIAG_IGNORE_STMT(-Wtype-limits);
#if defined(__HP_cc) || defined(__HP_aCC)
#pragma diag_suppress 2186
#endif
            neg = PL_statcache.st_ino < 0;
#if defined(__HP_cc) || defined(__HP_aCC)
#pragma diag_default 2186
#endif
            GCC_DIAG_RESTORE_STMT;
            CLANG_DIAG_RESTORE_STMT;
            if (neg) {
                s.st_ino = (IV)PL_statcache.st_ino;
                if (LIKELY(s.st_ino == PL_statcache.st_ino)) {
                    mPUSHi(s.st_ino);
                } else {
                    char buf[sizeof(s.st_ino)*3+1], *p;
                    s.st_ino = PL_statcache.st_ino;
                    for (p = buf + sizeof(buf); p != buf+1; ) {
                        Stat_t t;
                        t.st_ino = s.st_ino / 10;
                        *--p = '0' + (int)(t.st_ino*10 - s.st_ino);
                        s.st_ino = t.st_ino;
                    }
                    while (*p == '0')
                        p++;
                    *--p = '-';
                    mPUSHp(p, buf+sizeof(buf) - p);
                }
            } else {
                s.st_ino = (UV)PL_statcache.st_ino;
                if (LIKELY(s.st_ino == PL_statcache.st_ino)) {
                    mPUSHu(s.st_ino);
                } else {
                    char buf[sizeof(s.st_ino)*3], *p;
                    s.st_ino = PL_statcache.st_ino;
                    for (p = buf + sizeof(buf); p != buf; ) {
                        Stat_t t;
                        t.st_ino = s.st_ino / 10;
                        *--p = '0' + (int)(s.st_ino - t.st_ino*10);
                        s.st_ino = t.st_ino;
                    }
                    while (*p == '0')
                        p++;
                    mPUSHp(p, buf+sizeof(buf) - p);
                }
            }
        }
        mPUSHu(PL_statcache.st_mode);
        mPUSHu(PL_statcache.st_nlink);
        
        sv_setuid(PUSHmortal, PL_statcache.st_uid);
        sv_setgid(PUSHmortal, PL_statcache.st_gid);

#ifdef USE_STAT_RDEV
        mPUSHi(PL_statcache.st_rdev);
#else
        PUSHs(newSVpvs_flags("", SVs_TEMP));
#endif
#if Off_t_size > IVSIZE
        mPUSHn(PL_statcache.st_size);
#else
        mPUSHi(PL_statcache.st_size);
#endif
#ifdef BIG_TIME
        mPUSHn(PL_statcache.st_atime);
        mPUSHn(PL_statcache.st_mtime);
        mPUSHn(PL_statcache.st_ctime);
#else
        mPUSHi(PL_statcache.st_atime);
        mPUSHi(PL_statcache.st_mtime);
        mPUSHi(PL_statcache.st_ctime);
#endif
#ifdef USE_STAT_BLOCKS
        mPUSHu(PL_statcache.st_blksize);
        mPUSHu(PL_statcache.st_blocks);
#else
        PUSHs(newSVpvs_flags("", SVs_TEMP));
        PUSHs(newSVpvs_flags("", SVs_TEMP));
#endif
    }
    RETURN;
}

/* All filetest ops avoid manipulating the perl stack pointer in their main
   bodies (since commit d2c4d2d1e22d3125), and return using either
   S_ft_return_false() or S_ft_return_true().  These two helper functions are
   the only two which manipulate the perl stack.  To ensure that no stack
   manipulation macros are used, the filetest ops avoid defining a local copy
   of the stack pointer with dSP.  */

/* If the next filetest is stacked up with this one
   (PL_op->op_private & OPpFT_STACKING), we leave
   the original argument on the stack for success,
   and skip the stacked operators on failure.
   The next few macros/functions take care of this.
*/

static OP *
S_ft_return_false(pTHX_ SV *ret) {
    OP *next = NORMAL;
    dSP;

    if (PL_op->op_flags & OPf_REF) XPUSHs(ret);
    else			   SETs(ret);
    PUTBACK;

    if (PL_op->op_private & OPpFT_STACKING) {
        while (next && OP_IS_FILETEST(next->op_type)
               && next->op_private & OPpFT_STACKED)
            next = next->op_next;
    }
    return next;
}

PERL_STATIC_INLINE OP *
S_ft_return_true(pTHX_ SV *ret) {
    dSP;
    if (PL_op->op_flags & OPf_REF)
        XPUSHs(PL_op->op_private & OPpFT_STACKING ? (SV *)cGVOP_gv : (ret));
    else if (!(PL_op->op_private & OPpFT_STACKING))
        SETs(ret);
    PUTBACK;
    return NORMAL;
}

#define FT_RETURNNO	return S_ft_return_false(aTHX_ &PL_sv_no)
#define FT_RETURNUNDEF	return S_ft_return_false(aTHX_ &PL_sv_undef)
#define FT_RETURNYES	return S_ft_return_true(aTHX_ &PL_sv_yes)

/* NB: OPf_REF implies '-X _' and thus no arg on the stack */
#define tryAMAGICftest_MG(chr) STMT_START { \
        if (   !(PL_op->op_flags & OPf_REF)                   \
            && (SvFLAGS(*PL_stack_sp) & (SVf_ROK|SVs_GMG)))   \
        {                                                     \
            OP *next = S_try_amagic_ftest(aTHX_ chr);	\
            if (next) return next;			  \
        }						   \
    } STMT_END

STATIC OP *
S_try_amagic_ftest(pTHX_ char chr) {
    SV *const arg = *PL_stack_sp;

    assert(chr != '?');
    if (!(PL_op->op_private & OPpFT_STACKED)) SvGETMAGIC(arg);

    if (SvAMAGIC(arg))
    {
        const char tmpchr = chr;
        SV * const tmpsv = amagic_call(arg,
                                newSVpvn_flags(&tmpchr, 1, SVs_TEMP),
                                ftest_amg, AMGf_unary);

        if (!tmpsv)
            return NULL;

        return SvTRUE(tmpsv)
            ? S_ft_return_true(aTHX_ tmpsv) : S_ft_return_false(aTHX_ tmpsv);
    }
    return NULL;
}


/* also used for: pp_fteexec() pp_fteread() pp_ftewrite() pp_ftrexec()
 *                pp_ftrwrite() */

PP(pp_ftrread)
{
    I32 result;
    /* Not const, because things tweak this below. Not bool, because there's
       no guarantee that OPpFT_ACCESS is <= CHAR_MAX  */
#if defined(HAS_ACCESS) || defined (PERL_EFF_ACCESS)
    I32 use_access = PL_op->op_private & OPpFT_ACCESS;
    /* Giving some sort of initial value silences compilers.  */
#  ifdef R_OK
    int access_mode = R_OK;
#  else
    int access_mode = 0;
#  endif
#else
    /* access_mode is never used, but leaving use_access in makes the
       conditional compiling below much clearer.  */
    I32 use_access = 0;
#endif
    Mode_t stat_mode = S_IRUSR;

    bool effective = FALSE;
    char opchar = '?';

    switch (PL_op->op_type) {
    case OP_FTRREAD:	opchar = 'R'; break;
    case OP_FTRWRITE:	opchar = 'W'; break;
    case OP_FTREXEC:	opchar = 'X'; break;
    case OP_FTEREAD:	opchar = 'r'; break;
    case OP_FTEWRITE:	opchar = 'w'; break;
    case OP_FTEEXEC:	opchar = 'x'; break;
    }
    tryAMAGICftest_MG(opchar);

    switch (PL_op->op_type) {
    case OP_FTRREAD:
#if !(defined(HAS_ACCESS) && defined(R_OK))
        use_access = 0;
#endif
        break;

    case OP_FTRWRITE:
#if defined(HAS_ACCESS) && defined(W_OK)
        access_mode = W_OK;
#else
        use_access = 0;
#endif
        stat_mode = S_IWUSR;
        break;

    case OP_FTREXEC:
#if defined(HAS_ACCESS) && defined(X_OK)
        access_mode = X_OK;
#else
        use_access = 0;
#endif
        stat_mode = S_IXUSR;
        break;

    case OP_FTEWRITE:
#ifdef PERL_EFF_ACCESS
        access_mode = W_OK;
#endif
        stat_mode = S_IWUSR;
        /* FALLTHROUGH */

    case OP_FTEREAD:
#ifndef PERL_EFF_ACCESS
        use_access = 0;
#endif
        effective = TRUE;
        break;

    case OP_FTEEXEC:
#ifdef PERL_EFF_ACCESS
        access_mode = X_OK;
#else
        use_access = 0;
#endif
        stat_mode = S_IXUSR;
        effective = TRUE;
        break;
    }

    if (use_access) {
#if defined(HAS_ACCESS) || defined (PERL_EFF_ACCESS)
        STRLEN len;
        const char *name = SvPV(*PL_stack_sp, len);
        if (!IS_SAFE_PATHNAME(name, len, OP_NAME(PL_op))) {
            result = -1;
        }
        else if (effective) {
#  ifdef PERL_EFF_ACCESS
            result = PERL_EFF_ACCESS(name, access_mode);
#  else
            DIE(aTHX_ "panic: attempt to call PERL_EFF_ACCESS in %s",
                OP_NAME(PL_op));
#  endif
        }
        else {
#  ifdef HAS_ACCESS
            result = access(name, access_mode);
#  else
            DIE(aTHX_ "panic: attempt to call access() in %s", OP_NAME(PL_op));
#  endif
        }
        if (result == 0)
            FT_RETURNYES;
        if (result < 0)
            FT_RETURNUNDEF;
        FT_RETURNNO;
#endif
    }

    result = my_stat_flags(0);
    if (result < 0)
        FT_RETURNUNDEF;
    if (cando(stat_mode, effective, &PL_statcache))
        FT_RETURNYES;
    FT_RETURNNO;
}


/* also used for: pp_ftatime() pp_ftctime() pp_ftmtime() pp_ftsize() */

PP(pp_ftis)
{
    I32 result;
    const int op_type = PL_op->op_type;
    char opchar = '?';

    switch (op_type) {
    case OP_FTIS:	opchar = 'e'; break;
    case OP_FTSIZE:	opchar = 's'; break;
    case OP_FTMTIME:	opchar = 'M'; break;
    case OP_FTCTIME:	opchar = 'C'; break;
    case OP_FTATIME:	opchar = 'A'; break;
    }
    tryAMAGICftest_MG(opchar);

    result = my_stat_flags(0);
    if (result < 0)
        FT_RETURNUNDEF;
    if (op_type == OP_FTIS)
        FT_RETURNYES;
    {
        /* You can't dTARGET inside OP_FTIS, because you'll get
           "panic: pad_sv po" - the op is not flagged to have a target.  */
        dTARGET;
        switch (op_type) {
        case OP_FTSIZE:
#if Off_t_size > IVSIZE
            sv_setnv(TARG, (NV)PL_statcache.st_size);
#else
            sv_setiv(TARG, (IV)PL_statcache.st_size);
#endif
            break;
        case OP_FTMTIME:
            sv_setnv(TARG,
                    ((NV)PL_basetime - PL_statcache.st_mtime) / 86400.0 );
            break;
        case OP_FTATIME:
            sv_setnv(TARG,
                    ((NV)PL_basetime - PL_statcache.st_atime) / 86400.0 );
            break;
        case OP_FTCTIME:
            sv_setnv(TARG,
                    ((NV)PL_basetime - PL_statcache.st_ctime) / 86400.0 );
            break;
        }
        SvSETMAGIC(TARG);
        return SvTRUE_nomg_NN(TARG)
            ? S_ft_return_true(aTHX_ TARG) : S_ft_return_false(aTHX_ TARG);
    }
}


/* also used for: pp_ftblk() pp_ftchr() pp_ftdir() pp_fteowned()
 *                pp_ftfile() pp_ftpipe() pp_ftsgid() pp_ftsock()
 *                pp_ftsuid() pp_ftsvtx() pp_ftzero() */

PP(pp_ftrowned)
{
    I32 result;
    char opchar = '?';

    switch (PL_op->op_type) {
    case OP_FTROWNED:	opchar = 'O'; break;
    case OP_FTEOWNED:	opchar = 'o'; break;
    case OP_FTZERO:	opchar = 'z'; break;
    case OP_FTSOCK:	opchar = 'S'; break;
    case OP_FTCHR:	opchar = 'c'; break;
    case OP_FTBLK:	opchar = 'b'; break;
    case OP_FTFILE:	opchar = 'f'; break;
    case OP_FTDIR:	opchar = 'd'; break;
    case OP_FTPIPE:	opchar = 'p'; break;
    case OP_FTSUID:	opchar = 'u'; break;
    case OP_FTSGID:	opchar = 'g'; break;
    case OP_FTSVTX:	opchar = 'k'; break;
    }
    tryAMAGICftest_MG(opchar);

    result = my_stat_flags(0);
    if (result < 0)
        FT_RETURNUNDEF;
    switch (PL_op->op_type) {
    case OP_FTROWNED:
        if (PL_statcache.st_uid == PerlProc_getuid())
            FT_RETURNYES;
        break;
    case OP_FTEOWNED:
        if (PL_statcache.st_uid == PerlProc_geteuid())
            FT_RETURNYES;
        break;
    case OP_FTZERO:
        if (PL_statcache.st_size == 0)
            FT_RETURNYES;
        break;
    case OP_FTSOCK:
        if (S_ISSOCK(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
    case OP_FTCHR:
        if (S_ISCHR(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
    case OP_FTBLK:
        if (S_ISBLK(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
    case OP_FTFILE:
        if (S_ISREG(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
    case OP_FTDIR:
        if (S_ISDIR(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
    case OP_FTPIPE:
        if (S_ISFIFO(PL_statcache.st_mode))
            FT_RETURNYES;
        break;
#ifdef S_ISUID
    case OP_FTSUID:
        if (PL_statcache.st_mode & S_ISUID)
            FT_RETURNYES;
        break;
#endif
#ifdef S_ISGID
    case OP_FTSGID:
        if (PL_statcache.st_mode & S_ISGID)
            FT_RETURNYES;
        break;
#endif
#ifdef S_ISVTX
    case OP_FTSVTX:
        if (PL_statcache.st_mode & S_ISVTX)
            FT_RETURNYES;
        break;
#endif
    }
    FT_RETURNNO;
}

PP(pp_ftlink)
{
    I32 result;

    tryAMAGICftest_MG('l');
    result = my_lstat_flags(0);

    if (result < 0)
        FT_RETURNUNDEF;
    if (S_ISLNK(PL_statcache.st_mode))
        FT_RETURNYES;
    FT_RETURNNO;
}

PP(pp_fttty)
{
    int fd;
    GV *gv;
    char *name = NULL;
    STRLEN namelen;
    UV uv;

    tryAMAGICftest_MG('t');

    if (PL_op->op_flags & OPf_REF)
        gv = cGVOP_gv;
    else {
      SV *tmpsv = *PL_stack_sp;
      if (!(gv = MAYBE_DEREF_GV_nomg(tmpsv))) {
        name = SvPV_nomg(tmpsv, namelen);
        gv = gv_fetchpvn_flags(name, namelen, SvUTF8(tmpsv), SVt_PVIO);
      }
    }

    if (GvIO(gv) && IoIFP(GvIOp(gv)))
        fd = PerlIO_fileno(IoIFP(GvIOp(gv)));
    else if (name && isDIGIT(*name) && grok_atoUV(name, &uv, NULL) && uv <= PERL_INT_MAX)
        fd = (int)uv;
    else
        fd = -1;
    if (fd < 0) {
        SETERRNO(EBADF,RMS_IFI);
        FT_RETURNUNDEF;
    }
    if (PerlLIO_isatty(fd))
        FT_RETURNYES;
    FT_RETURNNO;
}


/* also used for: pp_ftbinary() */

PP(pp_fttext)
{
    I32 i;
    SSize_t len;
    I32 odd = 0;
    STDCHAR tbuf[512];
    STDCHAR *s;
    IO *io;
    SV *sv = NULL;
    GV *gv;
    PerlIO *fp;
    const U8 * first_variant;

    tryAMAGICftest_MG(PL_op->op_type == OP_FTTEXT ? 'T' : 'B');

    if (PL_op->op_flags & OPf_REF)
        gv = cGVOP_gv;
    else if ((PL_op->op_private & (OPpFT_STACKED|OPpFT_AFTER_t))
             == OPpFT_STACKED)
        gv = PL_defgv;
    else {
        sv = *PL_stack_sp;
        gv = MAYBE_DEREF_GV_nomg(sv);
    }

    if (gv) {
        if (gv == PL_defgv) {
            if (PL_statgv)
                io = SvTYPE(PL_statgv) == SVt_PVIO
                    ? (IO *)PL_statgv
                    : GvIO(PL_statgv);
            else {
                goto really_filename;
            }
        }
        else {
            PL_statgv = gv;
            SvPVCLEAR(PL_statname);
            io = GvIO(PL_statgv);
        }
        PL_laststatval = -1;
        PL_laststype = OP_STAT;
        if (io && IoIFP(io)) {
            int fd;
            if (! PerlIO_has_base(IoIFP(io)))
                DIE(aTHX_ "-T and -B not implemented on filehandles");
            fd = PerlIO_fileno(IoIFP(io));
            if (fd < 0) {
                SETERRNO(EBADF,RMS_IFI);
                FT_RETURNUNDEF;
            }
            PL_laststatval = PerlLIO_fstat(fd, &PL_statcache);
            if (PL_laststatval < 0)
                FT_RETURNUNDEF;
            if (S_ISDIR(PL_statcache.st_mode)) { /* handle NFS glitch */
                if (PL_op->op_type == OP_FTTEXT)
                    FT_RETURNNO;
                else
                    FT_RETURNYES;
            }
            if (PerlIO_get_cnt(IoIFP(io)) <= 0) {
                i = PerlIO_getc(IoIFP(io));
                if (i != EOF)
                    (void)PerlIO_ungetc(IoIFP(io),i);
                else
                    /* null file is anything */
                    FT_RETURNYES;
            }
            len = PerlIO_get_bufsiz(IoIFP(io));
            s = (STDCHAR *) PerlIO_get_base(IoIFP(io));
            /* sfio can have large buffers - limit to 512 */
            if (len > 512)
                len = 512;
        }
        else {
            SETERRNO(EBADF,RMS_IFI);
            report_evil_fh(gv);
            SETERRNO(EBADF,RMS_IFI);
            FT_RETURNUNDEF;
        }
    }
    else {
        const char *file;
        const char *temp;
        STRLEN temp_len;
        int fd; 

        assert(sv);
        temp = SvPV_nomg_const(sv, temp_len);
        sv_setpv(PL_statname, temp);
        if (!IS_SAFE_PATHNAME(temp, temp_len, OP_NAME(PL_op))) {
            PL_laststatval = -1;
            PL_laststype = OP_STAT;
            FT_RETURNUNDEF;
        }
      really_filename:
        file = SvPVX_const(PL_statname);
        PL_statgv = NULL;
        if (!(fp = PerlIO_open(file, "r"))) {
            if (!gv) {
                PL_laststatval = -1;
                PL_laststype = OP_STAT;
            }
            if (ckWARN(WARN_NEWLINE) && should_warn_nl(file)) {
                /* PL_warn_nl is constant */
                GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
                Perl_warner(aTHX_ packWARN(WARN_NEWLINE), PL_warn_nl, "open");
                GCC_DIAG_RESTORE_STMT;
            }
            FT_RETURNUNDEF;
        }
        PL_laststype = OP_STAT;
        fd = PerlIO_fileno(fp);
        if (fd < 0) {
            (void)PerlIO_close(fp);
            SETERRNO(EBADF,RMS_IFI);
            FT_RETURNUNDEF;
        }
        PL_laststatval = PerlLIO_fstat(fd, &PL_statcache);
        if (PL_laststatval < 0)	{
            dSAVE_ERRNO;
            (void)PerlIO_close(fp);
            RESTORE_ERRNO;
            FT_RETURNUNDEF;
        }
        PerlIO_binmode(aTHX_ fp, '<', O_BINARY, NULL);
        len = PerlIO_read(fp, tbuf, sizeof(tbuf));
        (void)PerlIO_close(fp);
        if (len <= 0) {
            if (S_ISDIR(PL_statcache.st_mode) && PL_op->op_type == OP_FTTEXT)
                FT_RETURNNO;		/* special case NFS directories */
            FT_RETURNYES;		/* null file is anything */
        }
        s = tbuf;
    }

    /* now scan s to look for textiness */

#if defined(DOSISH) || defined(USEMYBINMODE)
    /* ignore trailing ^Z on short files */
    if (len && len < (I32)sizeof(tbuf) && tbuf[len-1] == 26)
        --len;
#endif

    assert(len);
    if (! is_utf8_invariant_string_loc((U8 *) s, len, &first_variant)) {

        /* Here contains a variant under UTF-8 .  See if the entire string is
         * UTF-8. */
        if (is_utf8_fixed_width_buf_flags(first_variant,
                                          len - ((char *) first_variant - (char *) s),
                                          0))
        {
            if (PL_op->op_type == OP_FTTEXT) {
                FT_RETURNYES;
            }
            else {
                FT_RETURNNO;
            }
        }
    }

    /* Here, is not UTF-8 or is entirely ASCII.  Look through the buffer for
     * things that wouldn't be in ASCII text or rich ASCII text.  Count these
     * in 'odd' */
    for (i = 0; i < len; i++, s++) {
        if (!*s) {			/* null never allowed in text */
            odd += len;
            break;
        }
#ifdef USE_LOCALE_CTYPE
        if (IN_LC_RUNTIME(LC_CTYPE)) {
            if ( isPRINT_LC(*s) || isSPACE_LC(*s)) {
                continue;
            }
        }
        else
#endif
             if (  isPRINT_A(*s)
                    /* VT occurs so rarely in text, that we consider it odd */
                 || (isSPACE_A(*s) && *s != VT_NATIVE)

                    /* But there is a fair amount of backspaces and escapes in
                     * some text */
                 || *s == '\b'
                 || *s == ESC_NATIVE)
        {
            continue;
        }
        odd++;
    }

    if ((odd * 3 > len) == (PL_op->op_type == OP_FTTEXT)) /* allow 1/3 odd */
        FT_RETURNNO;
    else
        FT_RETURNYES;
}

/* File calls. */

PP(pp_chdir)
{
    dSP; dTARGET;
    const char *tmps = NULL;
    GV *gv = NULL;

    if( MAXARG == 1 ) {
        SV * const sv = POPs;
        if (PL_op->op_flags & OPf_SPECIAL) {
            gv = gv_fetchsv(sv, 0, SVt_PVIO);
            if (!gv) {
                if (ckWARN(WARN_UNOPENED)) {
                    Perl_warner(aTHX_ packWARN(WARN_UNOPENED),
                                "chdir() on unopened filehandle %" SVf, sv);
                }
                SETERRNO(EBADF,RMS_IFI);
                PUSHs(&PL_sv_zero);
                TAINT_PROPER("chdir");
                RETURN;
            }
        }
        else if (!(gv = MAYBE_DEREF_GV(sv)))
                tmps = SvPV_nomg_const_nolen(sv);
    }
    else {
        HV * const table = GvHVn(PL_envgv);
        SV **svp;

        EXTEND(SP, 1);
        if (    (svp = hv_fetchs(table, "HOME", FALSE))
             || (svp = hv_fetchs(table, "LOGDIR", FALSE))
#ifdef VMS
             || (svp = hv_fetchs(table, "SYS$LOGIN", FALSE))
#endif
           )
        {
            tmps = SvPV_nolen_const(*svp);
        }
        else {
            PUSHs(&PL_sv_zero);
            SETERRNO(EINVAL, LIB_INVARG);
            TAINT_PROPER("chdir");
            RETURN;
        }
    }

    TAINT_PROPER("chdir");
    if (gv) {
#ifdef HAS_FCHDIR
        IO* const io = GvIO(gv);
        if (io) {
            if (IoDIRP(io)) {
                PUSHi(fchdir(my_dirfd(IoDIRP(io))) >= 0);
            } else if (IoIFP(io)) {
                int fd = PerlIO_fileno(IoIFP(io));
                if (fd < 0) {
                    goto nuts;
                }
                PUSHi(fchdir(fd) >= 0);
            }
            else {
                goto nuts;
            }
        } else {
            goto nuts;
        }

#else
        DIE(aTHX_ PL_no_func, "fchdir");
#endif
    }
    else 
        PUSHi( PerlDir_chdir(tmps) >= 0 );
#ifdef VMS
    /* Clear the DEFAULT element of ENV so we'll get the new value
     * in the future. */
    hv_delete(GvHVn(PL_envgv),"DEFAULT",7,G_DISCARD);
#endif
    RETURN;

#ifdef HAS_FCHDIR
 nuts:
    report_evil_fh(gv);
    SETERRNO(EBADF,RMS_IFI);
    PUSHs(&PL_sv_zero);
    RETURN;
#endif
}


/* also used for: pp_chmod() pp_kill() pp_unlink() pp_utime() */

PP(pp_chown)
{
    dSP; dMARK; dTARGET;
    const I32 value = (I32)apply(PL_op->op_type, MARK, SP);

    SP = MARK;
    XPUSHi(value);
    RETURN;
}

PP(pp_chroot)
{
#ifdef HAS_CHROOT
    dSP; dTARGET;
    char * const tmps = POPpx;
    TAINT_PROPER("chroot");
    PUSHi( chroot(tmps) >= 0 );
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "chroot");
#endif
}

PP(pp_rename)
{
    dSP; dTARGET;
    int anum;
#ifndef HAS_RENAME
    Stat_t statbuf;
#endif
    const char * const tmps2 = POPpconstx;
    const char * const tmps = SvPV_nolen_const(TOPs);
    TAINT_PROPER("rename");
#ifdef HAS_RENAME
    anum = PerlLIO_rename(tmps, tmps2);
#else
    if (!(anum = PerlLIO_stat(tmps, &statbuf))) {
        if (same_dirent(tmps2, tmps))	/* can always rename to same name */
            anum = 1;
        else {
            if (PerlProc_geteuid() || PerlLIO_stat(tmps2, &statbuf) < 0 || !S_ISDIR(statbuf.st_mode))
                (void)UNLINK(tmps2);
            if (!(anum = link(tmps, tmps2)))
                anum = UNLINK(tmps);
        }
    }
#endif
    SETi( anum >= 0 );
    RETURN;
}


/* also used for: pp_symlink() */

#if defined(HAS_LINK) || defined(HAS_SYMLINK)
PP(pp_link)
{
    dSP; dTARGET;
    const int op_type = PL_op->op_type;
    int result;

#  ifndef HAS_LINK
    if (op_type == OP_LINK)
        DIE(aTHX_ PL_no_func, "link");
#  endif
#  ifndef HAS_SYMLINK
    if (op_type == OP_SYMLINK)
        DIE(aTHX_ PL_no_func, "symlink");
#  endif

    {
        const char * const tmps2 = POPpconstx;
        const char * const tmps = SvPV_nolen_const(TOPs);
        TAINT_PROPER(PL_op_desc[op_type]);
        result =
#  if defined(HAS_LINK) && defined(HAS_SYMLINK)
            /* Both present - need to choose which.  */
            (op_type == OP_LINK) ?
            PerlLIO_link(tmps, tmps2) : PerlLIO_symlink(tmps, tmps2);
#  elif defined(HAS_LINK)
    /* Only have link, so calls to pp_symlink will have DIE()d above.  */
        PerlLIO_link(tmps, tmps2);
#  elif defined(HAS_SYMLINK)
    /* Only have symlink, so calls to pp_link will have DIE()d above.  */
        PerlLIO_symlink(tmps, tmps2);
#  endif
    }

    SETi( result >= 0 );
    RETURN;
}
#else

/* also used for: pp_symlink() */

PP(pp_link)
{
    /* Have neither.  */
    DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
}
#endif

PP(pp_readlink)
{
    dSP;
#ifdef HAS_SYMLINK
    dTARGET;
    const char *tmps;
    char buf[MAXPATHLEN];
    SSize_t len;

    TAINT;
    tmps = POPpconstx;
    /* NOTE: if the length returned by readlink() is sizeof(buf) - 1,
     * it is impossible to know whether the result was truncated. */
    len = PerlLIO_readlink(tmps, buf, sizeof(buf) - 1);
    if (len < 0)
        RETPUSHUNDEF;
    buf[len] = '\0';
    PUSHp(buf, len);
    RETURN;
#else
    EXTEND(SP, 1);
    RETSETUNDEF;		/* just pretend it's a normal file */
#endif
}

#if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
STATIC int
S_dooneliner(pTHX_ const char *cmd, const char *filename)
{
    char * const save_filename = filename;
    char *cmdline;
    char *s;
    PerlIO *myfp;
    int anum = 1;
    Size_t size = strlen(cmd) + (strlen(filename) * 2) + 10;

    PERL_ARGS_ASSERT_DOONELINER;

    Newx(cmdline, size, char);
    my_strlcpy(cmdline, cmd, size);
    my_strlcat(cmdline, " ", size);
    for (s = cmdline + strlen(cmdline); *filename; ) {
        *s++ = '\\';
        *s++ = *filename++;
    }
    if (s - cmdline < size)
        my_strlcpy(s, " 2>&1", size - (s - cmdline));
    myfp = PerlProc_popen(cmdline, "r");
    Safefree(cmdline);

    if (myfp) {
        SV * const tmpsv = sv_newmortal();
        /* Need to save/restore 'PL_rs' ?? */
        s = sv_gets(tmpsv, myfp, 0);
        (void)PerlProc_pclose(myfp);
        if (s != NULL) {
            int e;
            for (e = 1;
#ifdef HAS_SYS_ERRLIST
                 e <= sys_nerr
#endif
                 ; e++)
            {
                /* you don't see this */
                const char * const errmsg = Strerror(e) ;
                if (!errmsg)
                    break;
                if (instr(s, errmsg)) {
                    SETERRNO(e,0);
                    return 0;
                }
            }
            SETERRNO(0,0);
#ifndef EACCES
#define EACCES EPERM
#endif
            if (instr(s, "cannot make"))
                SETERRNO(EEXIST,RMS_FEX);
            else if (instr(s, "existing file"))
                SETERRNO(EEXIST,RMS_FEX);
            else if (instr(s, "ile exists"))
                SETERRNO(EEXIST,RMS_FEX);
            else if (instr(s, "non-exist"))
                SETERRNO(ENOENT,RMS_FNF);
            else if (instr(s, "does not exist"))
                SETERRNO(ENOENT,RMS_FNF);
            else if (instr(s, "not empty"))
                SETERRNO(EBUSY,SS_DEVOFFLINE);
            else if (instr(s, "cannot access"))
                SETERRNO(EACCES,RMS_PRV);
            else
                SETERRNO(EPERM,RMS_PRV);
            return 0;
        }
        else {	/* some mkdirs return no failure indication */
            Stat_t statbuf;
            anum = (PerlLIO_stat(save_filename, &statbuf) >= 0);
            if (PL_op->op_type == OP_RMDIR)
                anum = !anum;
            if (anum)
                SETERRNO(0,0);
            else
                SETERRNO(EACCES,RMS_PRV);	/* a guess */
        }
        return anum;
    }
    else
        return 0;
}
#endif

/* This macro removes trailing slashes from a directory name.
 * Different operating and file systems take differently to
 * trailing slashes.  According to POSIX 1003.1 1996 Edition
 * any number of trailing slashes should be allowed.
 * Thusly we snip them away so that even non-conforming
 * systems are happy.
 * We should probably do this "filtering" for all
 * the functions that expect (potentially) directory names:
 * -d, chdir(), chmod(), chown(), chroot(), fcntl()?,
 * (mkdir()), opendir(), rename(), rmdir(), stat(). --jhi */

#define TRIMSLASHES(tmps,len,copy) (tmps) = SvPV_const(TOPs, (len)); \
    if ((len) > 1 && (tmps)[(len)-1] == '/') { \
        do { \
            (len)--; \
        } while ((len) > 1 && (tmps)[(len)-1] == '/'); \
        (tmps) = savepvn((tmps), (len)); \
        (copy) = TRUE; \
    }

PP(pp_mkdir)
{
    dSP; dTARGET;
    STRLEN len;
    const char *tmps;
    bool copy = FALSE;
    const unsigned int mode = (MAXARG > 1 && (TOPs||((void)POPs,0))) ? POPu : 0777;

    TRIMSLASHES(tmps,len,copy);

    TAINT_PROPER("mkdir");
#ifdef HAS_MKDIR
    SETi( PerlDir_mkdir(tmps, mode) >= 0 );
#else
    {
    int oldumask;
    SETi( dooneliner("mkdir", tmps) );
    oldumask = PerlLIO_umask(0);
    PerlLIO_umask(oldumask);
    PerlLIO_chmod(tmps, (mode & ~oldumask) & 0777);
    }
#endif
    if (copy)
        Safefree(tmps);
    RETURN;
}

PP(pp_rmdir)
{
    dSP; dTARGET;
    STRLEN len;
    const char *tmps;
    bool copy = FALSE;

    TRIMSLASHES(tmps,len,copy);
    TAINT_PROPER("rmdir");
#ifdef HAS_RMDIR
    SETi( PerlDir_rmdir(tmps) >= 0 );
#else
    SETi( dooneliner("rmdir", tmps) );
#endif
    if (copy)
        Safefree(tmps);
    RETURN;
}

/* Directory calls. */

PP(pp_open_dir)
{
#if defined(Direntry_t) && defined(HAS_READDIR)
    dSP;
    const char * const dirname = POPpconstx;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if ((IoIFP(io) || IoOFP(io)))
        Perl_croak(aTHX_ "Cannot open %" HEKf " as a dirhandle: it is already open as a filehandle",
                         HEKfARG(GvENAME_HEK(gv)));
    if (IoDIRP(io))
        PerlDir_close(IoDIRP(io));
    if (!(IoDIRP(io) = PerlDir_open(dirname)))
        goto nope;

    RETPUSHYES;
  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_DIR);
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_dir_func, "opendir");
#endif
}

PP(pp_readdir)
{
#if !defined(Direntry_t) || !defined(HAS_READDIR)
    DIE(aTHX_ PL_no_dir_func, "readdir");
#else
#if !defined(I_DIRENT) && !defined(VMS)
    Direntry_t *readdir (DIR *);
#endif
    dSP;

    SV *sv;
    const U8 gimme = GIMME_V;
    GV * const gv = MUTABLE_GV(POPs);
    const Direntry_t *dp;
    IO * const io = GvIOn(gv);

    if (!IoDIRP(io)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                       "readdir() attempted on invalid dirhandle %" HEKf,
                            HEKfARG(GvENAME_HEK(gv)));
        goto nope;
    }

    do {
        dp = (Direntry_t *)PerlDir_read(IoDIRP(io));
        if (!dp)
            break;
#ifdef DIRNAMLEN
        sv = newSVpvn(dp->d_name, dp->d_namlen);
#else
        sv = newSVpv(dp->d_name, 0);
#endif
        if (!(IoFLAGS(io) & IOf_UNTAINT))
            SvTAINTED_on(sv);
        mXPUSHs(sv);
    } while (gimme == G_LIST);

    if (!dp && gimme != G_LIST)
        RETPUSHUNDEF;

    RETURN;

  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_ISI);
    if (gimme == G_LIST)
        RETURN;
    else
        RETPUSHUNDEF;
#endif
}

PP(pp_telldir)
{
#if defined(HAS_TELLDIR) || defined(telldir)
    dSP; dTARGET;
 /* XXX does _anyone_ need this? --AD 2/20/1998 */
 /* XXX netbsd still seemed to.
    XXX HAS_TELLDIR_PROTO is new style, NEED_TELLDIR_PROTO is old style.
    --JHI 1999-Feb-02 */
# if !defined(HAS_TELLDIR_PROTO) || defined(NEED_TELLDIR_PROTO)
    long telldir (DIR *);
# endif
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoDIRP(io)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                       "telldir() attempted on invalid dirhandle %" HEKf,
                            HEKfARG(GvENAME_HEK(gv)));
        goto nope;
    }

    PUSHi( PerlDir_tell(IoDIRP(io)) );
    RETURN;
  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_ISI);
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_dir_func, "telldir");
#endif
}

PP(pp_seekdir)
{
#if defined(HAS_SEEKDIR) || defined(seekdir)
    dSP;
    const long along = POPl;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoDIRP(io)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                       "seekdir() attempted on invalid dirhandle %" HEKf,
                                HEKfARG(GvENAME_HEK(gv)));
        goto nope;
    }
    (void)PerlDir_seek(IoDIRP(io), along);

    RETPUSHYES;
  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_ISI);
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_dir_func, "seekdir");
#endif
}

PP(pp_rewinddir)
{
#if defined(HAS_REWINDDIR) || defined(rewinddir)
    dSP;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoDIRP(io)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                       "rewinddir() attempted on invalid dirhandle %" HEKf,
                                HEKfARG(GvENAME_HEK(gv)));
        goto nope;
    }
    (void)PerlDir_rewind(IoDIRP(io));
    RETPUSHYES;
  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_ISI);
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_dir_func, "rewinddir");
#endif
}

PP(pp_closedir)
{
#if defined(Direntry_t) && defined(HAS_READDIR)
    dSP;
    GV * const gv = MUTABLE_GV(POPs);
    IO * const io = GvIOn(gv);

    if (!IoDIRP(io)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_IO),
                       "closedir() attempted on invalid dirhandle %" HEKf,
                                HEKfARG(GvENAME_HEK(gv)));
        goto nope;
    }
#ifdef VOID_CLOSEDIR
    PerlDir_close(IoDIRP(io));
#else
    if (PerlDir_close(IoDIRP(io)) < 0) {
        IoDIRP(io) = 0; /* Don't try to close again--coredumps on SysV */
        goto nope;
    }
#endif
    IoDIRP(io) = 0;

    RETPUSHYES;
  nope:
    if (!errno)
        SETERRNO(EBADF,RMS_IFI);
    RETPUSHUNDEF;
#else
    DIE(aTHX_ PL_no_dir_func, "closedir");
#endif
}

/* Process control. */

PP(pp_fork)
{
#ifdef HAS_FORK
    dSP; dTARGET;
    Pid_t childpid;
#ifdef HAS_SIGPROCMASK
    sigset_t oldmask, newmask;
#endif


    EXTEND(SP, 1);
    PERL_FLUSHALL_FOR_CHILD;
#ifdef HAS_SIGPROCMASK
    sigfillset(&newmask);
    sigprocmask(SIG_SETMASK, &newmask, &oldmask);
#endif
    childpid = PerlProc_fork();
    if (childpid == 0) {
        int sig;
        PL_sig_pending = 0;
        if (PL_psig_pend)
            for (sig = 1; sig < SIG_SIZE; sig++)
                PL_psig_pend[sig] = 0;
    }
#ifdef HAS_SIGPROCMASK
    {
        dSAVE_ERRNO;
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
        RESTORE_ERRNO;
    }
#endif
    if (childpid < 0)
        RETPUSHUNDEF;
    if (!childpid) {
#ifdef PERL_USES_PL_PIDSTATUS
        hv_clear(PL_pidstatus);	/* no kids, so don't wait for 'em */
#endif
        PERL_SRAND_OVERRIDE_NEXT_CHILD();
    } else {
        PERL_SRAND_OVERRIDE_NEXT_PARENT();
    }
    PUSHi(childpid);
    RETURN;
#elif (defined(USE_ITHREADS) && defined(PERL_IMPLICIT_SYS)) || defined(__amigaos4__)
    dSP; dTARGET;
    Pid_t childpid;

    EXTEND(SP, 1);
    PERL_FLUSHALL_FOR_CHILD;
    childpid = PerlProc_fork();
    if (childpid == -1)
        RETPUSHUNDEF;
    else if (childpid) {
        /* we are in the parent */
        PERL_SRAND_OVERRIDE_NEXT_PARENT();
    }
    else {
        /* This is part of the logic supporting the env var
         * PERL_RAND_SEED which causes use of rand() without an
         * explicit srand() to use a deterministic seed. This logic is
         * intended to give most forked children of a process a
         * deterministic but different srand seed.
         */
        PERL_SRAND_OVERRIDE_NEXT_CHILD();
    }
    PUSHi(childpid);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "fork");
#endif
}

PP(pp_wait)
{
#if (!defined(DOSISH) || defined(OS2) || defined(WIN32)) && !defined(__LIBCATAMOUNT__)
    dSP; dTARGET;
    Pid_t childpid;
    int argflags;

    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        childpid = wait4pid(-1, &argflags, 0);
    else {
        while ((childpid = wait4pid(-1, &argflags, 0)) == -1 &&
               errno == EINTR) {
          PERL_ASYNC_CHECK();
        }
    }
#  if defined(USE_ITHREADS) && defined(PERL_IMPLICIT_SYS)
    /* 0 and -1 are both error returns (the former applies to WNOHANG case) */
    STATUS_NATIVE_CHILD_SET((childpid && childpid != -1) ? argflags : -1);
#  else
    STATUS_NATIVE_CHILD_SET((childpid > 0) ? argflags : -1);
#  endif
    XPUSHi(childpid);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "wait");
#endif
}

PP(pp_waitpid)
{
#if (!defined(DOSISH) || defined(OS2) || defined(WIN32)) && !defined(__LIBCATAMOUNT__)
    dSP; dTARGET;
    const int optype = POPi;
    const Pid_t pid = TOPi;
    Pid_t result;
#ifdef __amigaos4__
    int argflags = 0;
    result = amigaos_waitpid(aTHX_ optype, pid, &argflags);
    STATUS_NATIVE_CHILD_SET((result >= 0) ? argflags : -1);
    result = result == 0 ? pid : -1;
#else
    int argflags;

    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        result = wait4pid(pid, &argflags, optype);
    else {
        while ((result = wait4pid(pid, &argflags, optype)) == -1 &&
               errno == EINTR) {
          PERL_ASYNC_CHECK();
        }
    }
#  if defined(USE_ITHREADS) && defined(PERL_IMPLICIT_SYS)
    /* 0 and -1 are both error returns (the former applies to WNOHANG case) */
    STATUS_NATIVE_CHILD_SET((result && result != -1) ? argflags : -1);
#  else
    STATUS_NATIVE_CHILD_SET((result > 0) ? argflags : -1);
#  endif
# endif /* __amigaos4__ */
    SETi(result);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "waitpid");
#endif
}

PP(pp_system)
{
    dSP; dMARK; dORIGMARK; dTARGET;
#if defined(__LIBCATAMOUNT__)
    PL_statusvalue = -1;
    SP = ORIGMARK;
    XPUSHi(-1);
#else
    I32 value;
# ifdef __amigaos4__
    void * result;
# else
    int result;
# endif

    while (++MARK <= SP) {
        SV *origsv = *MARK, *copysv;
        STRLEN len;
        char *pv;
        SvGETMAGIC(origsv);
#if defined(WIN32) || defined(__VMS)
        /*
         * Because of a nasty platform-specific variation on the meaning
         * of arguments to this op, we must preserve numeric arguments
         * as numeric, not just retain the string value.
         */
        if (SvNIOK(origsv) || SvNIOKp(origsv)) {
            copysv = newSV_type(SVt_PVNV);
            sv_2mortal(copysv);
            if (SvPOK(origsv) || SvPOKp(origsv)) {
                pv = SvPV_nomg(origsv, len);
                sv_setpvn_fresh(copysv, pv, len);
                SvPOK_off(copysv);
            }
            if (SvIOK(origsv) || SvIOKp(origsv))
                SvIV_set(copysv, SvIVX(origsv));
            if (SvNOK(origsv) || SvNOKp(origsv))
                SvNV_set(copysv, SvNVX(origsv));
            SvFLAGS(copysv) |= SvFLAGS(origsv) &
                (SVf_IOK|SVf_NOK|SVf_POK|SVp_IOK|SVp_NOK|SVp_POK|
                    SVf_UTF8|SVf_IVisUV);
        } else
#endif
        {
            pv = SvPV_nomg(origsv, len);
            copysv = newSVpvn_flags(pv, len,
                        (SvFLAGS(origsv) & SVf_UTF8) | SVs_TEMP);
        }
        *MARK = copysv;
    }
    MARK = ORIGMARK;

    if (TAINTING_get) {
        TAINT_ENV();
        TAINT_PROPER("system");
    }
    PERL_FLUSHALL_FOR_CHILD;
#if (defined(HAS_FORK) || defined(__amigaos4__)) && !defined(VMS) && !defined(OS2) || defined(PERL_MICRO)
    {
#ifdef __amigaos4__
        struct UserData userdata;
        pthread_t proc;
#else
        Pid_t childpid;
#endif
        int pp[2];
        I32 did_pipes = 0;
        bool child_success = FALSE;
#ifdef HAS_SIGPROCMASK
        sigset_t newset, oldset;
#endif

        if (PerlProc_pipe_cloexec(pp) >= 0)
            did_pipes = 1;
#ifdef __amigaos4__
        amigaos_fork_set_userdata(aTHX_
                                  &userdata,
                                  did_pipes,
                                  pp[1],
                                  SP,
                                  mark);
        pthread_create(&proc,NULL,amigaos_system_child,(void *)&userdata);
        child_success = proc > 0;
#else
#ifdef HAS_SIGPROCMASK
        sigemptyset(&newset);
        sigaddset(&newset, SIGCHLD);
        sigprocmask(SIG_BLOCK, &newset, &oldset);
#endif
        while ((childpid = PerlProc_fork()) == -1) {
            if (errno != EAGAIN) {
                value = -1;
                SP = ORIGMARK;
                XPUSHi(value);
                if (did_pipes) {
                    PerlLIO_close(pp[0]);
                    PerlLIO_close(pp[1]);
                }
#ifdef HAS_SIGPROCMASK
                sigprocmask(SIG_SETMASK, &oldset, NULL);
#endif
                RETURN;
            }
            sleep(5);
        }
        child_success = childpid > 0;
#endif
        if (child_success) {
            Sigsave_t ihand,qhand; /* place to save signals during system() */
            int status;

#ifndef __amigaos4__
            if (did_pipes)
                PerlLIO_close(pp[1]);
#endif
#ifndef PERL_MICRO
            rsignal_save(SIGINT,  (Sighandler_t) SIG_IGN, &ihand);
            rsignal_save(SIGQUIT, (Sighandler_t) SIG_IGN, &qhand);
#endif
#ifdef __amigaos4__
            result = pthread_join(proc, (void **)&status);
#else
            do {
                result = wait4pid(childpid, &status, 0);
            } while (result == -1 && errno == EINTR);
#endif
#ifndef PERL_MICRO
#ifdef HAS_SIGPROCMASK
            sigprocmask(SIG_SETMASK, &oldset, NULL);
#endif
            (void)rsignal_restore(SIGINT, &ihand);
            (void)rsignal_restore(SIGQUIT, &qhand);
#endif
            STATUS_NATIVE_CHILD_SET(result == -1 ? -1 : status);
            SP = ORIGMARK;
            if (did_pipes) {
                int errkid;
                unsigned n = 0;

                while (n < sizeof(int)) {
                    const SSize_t n1 = PerlLIO_read(pp[0],
                                      (void*)(((char*)&errkid)+n),
                                      (sizeof(int)) - n);
                    if (n1 <= 0)
                        break;
                    n += n1;
                }
                PerlLIO_close(pp[0]);
                if (n) {			/* Error */
                    if (n != sizeof(int))
                        DIE(aTHX_ "panic: kid popen errno read, n=%u", n);
                    errno = errkid;		/* Propagate errno from kid */
#ifdef __amigaos4__
                    /* The pipe always has something in it
                     * so n alone is not enough. */
                    if (errno > 0)
#endif
                    {
                        STATUS_NATIVE_CHILD_SET(-1);
                    }
                }
            }
            XPUSHi(STATUS_CURRENT);
            RETURN;
        }
#ifndef __amigaos4__
#ifdef HAS_SIGPROCMASK
        sigprocmask(SIG_SETMASK, &oldset, NULL);
#endif
        if (did_pipes)
            PerlLIO_close(pp[0]);
        if (PL_op->op_flags & OPf_STACKED) {
            SV * const really = *++MARK;
            value = (I32)do_aexec5(really, MARK, SP, pp[1], did_pipes);
        }
        else if (SP - MARK != 1)
            value = (I32)do_aexec5(NULL, MARK, SP, pp[1], did_pipes);
        else {
            value = (I32)do_exec3(SvPVx_nolen(sv_mortalcopy(*SP)), pp[1], did_pipes);
        }
#endif /* __amigaos4__ */
        PerlProc__exit(-1);
    }
#else /* ! FORK or VMS or OS/2 */
    PL_statusvalue = 0;
    result = 0;
    if (PL_op->op_flags & OPf_STACKED) {
        SV * const really = *++MARK;
#  if defined(WIN32) || defined(OS2) || defined(__VMS)
        value = (I32)do_aspawn(really, MARK, SP);
#  else
        value = (I32)do_aspawn(really, (void **)MARK, (void **)SP);
#  endif
    }
    else if (SP - MARK != 1) {
#  if defined(WIN32) || defined(OS2) || defined(__VMS)
        value = (I32)do_aspawn(NULL, MARK, SP);
#  else
        value = (I32)do_aspawn(NULL, (void **)MARK, (void **)SP);
#  endif
    }
    else {
        value = (I32)do_spawn(SvPVx_nolen(sv_mortalcopy(*SP)));
    }
    if (PL_statusvalue == -1)	/* hint that value must be returned as is */
        result = 1;
    STATUS_NATIVE_CHILD_SET(value);
    SP = ORIGMARK;
    XPUSHi(result ? value : STATUS_CURRENT);
#endif /* !FORK or VMS or OS/2 */
#endif
    RETURN;
}

PP(pp_exec)
{
    dSP; dMARK; dORIGMARK; dTARGET;
    I32 value;

    if (TAINTING_get) {
        TAINT_ENV();
        while (++MARK <= SP) {
            (void)SvPV_nolen_const(*MARK);      /* stringify for taint check */
            if (TAINT_get)
                break;
        }
        MARK = ORIGMARK;
        TAINT_PROPER("exec");
    }

    PERL_FLUSHALL_FOR_CHILD;
    if (PL_op->op_flags & OPf_STACKED) {
        SV * const really = *++MARK;
        value = (I32)do_aexec(really, MARK, SP);
    }
    else if (SP - MARK != 1)
#ifdef VMS
        value = (I32)vms_do_aexec(NULL, MARK, SP);
#else
        value = (I32)do_aexec(NULL, MARK, SP);
#endif
    else {
#ifdef VMS
        value = (I32)vms_do_exec(SvPVx_nolen(sv_mortalcopy(*SP)));
#else
        value = (I32)do_exec(SvPVx_nolen(sv_mortalcopy(*SP)));
#endif
    }
    SP = ORIGMARK;
    XPUSHi(value);
    RETURN;
}

PP(pp_getppid)
{
#ifdef HAS_GETPPID
    dSP; dTARGET;
    XPUSHi( getppid() );
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "getppid");
#endif
}

PP(pp_getpgrp)
{
#ifdef HAS_GETPGRP
    dSP; dTARGET;
    Pid_t pgrp;
    const Pid_t pid =
        (MAXARG < 1) ? 0 : TOPs ? SvIVx(POPs) : ((void)POPs, 0);

#ifdef BSD_GETPGRP
    pgrp = (I32)BSD_GETPGRP(pid);
#else
    if (pid != 0 && pid != PerlProc_getpid())
        DIE(aTHX_ "POSIX getpgrp can't take an argument");
    pgrp = getpgrp();
#endif
    XPUSHi(pgrp);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "getpgrp");
#endif
}

PP(pp_setpgrp)
{
#ifdef HAS_SETPGRP
    dSP; dTARGET;
    Pid_t pgrp;
    Pid_t pid;
    pgrp = MAXARG == 2 && (TOPs||POPs) ? POPi : 0;
    if (MAXARG > 0) pid = TOPs ? TOPi : 0;
    else {
        pid = 0;
        EXTEND(SP,1);
        SP++;
    }

    TAINT_PROPER("setpgrp");
#ifdef BSD_SETPGRP
    SETi( BSD_SETPGRP(pid, pgrp) >= 0 );
#else
    if ((pgrp != 0 && pgrp != PerlProc_getpid())
        || (pid != 0 && pid != PerlProc_getpid()))
    {
        DIE(aTHX_ "setpgrp can't take arguments");
    }
    SETi( setpgrp() >= 0 );
#endif /* USE_BSDPGRP */
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "setpgrp");
#endif
}

/*
 * The glibc headers typedef __priority_which_t to an enum under C, but
 * under C++, it keeps it as int. -Wc++-compat doesn't know this, so we
 * need to explicitly cast it to shut up the warning.
 */
#if defined(__GLIBC__) && ((__GLIBC__ == 2 && __GLIBC_MINOR__ >= 3) || (__GLIBC__ > 2))
#  define PRIORITY_WHICH_T(which) (__priority_which_t)which
#else
#  define PRIORITY_WHICH_T(which) which
#endif

PP(pp_getpriority)
{
#ifdef HAS_GETPRIORITY
    dSP; dTARGET;
    const int who = POPi;
    const int which = TOPi;
    SETi( getpriority(PRIORITY_WHICH_T(which), who) );
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "getpriority");
#endif
}

PP(pp_setpriority)
{
#ifdef HAS_SETPRIORITY
    dSP; dTARGET;
    const int niceval = POPi;
    const int who = POPi;
    const int which = TOPi;
    TAINT_PROPER("setpriority");
    SETi( setpriority(PRIORITY_WHICH_T(which), who, niceval) >= 0 );
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "setpriority");
#endif
}

#undef PRIORITY_WHICH_T

/* Time calls. */

PP(pp_time)
{
    dSP; dTARGET;
#ifdef BIG_TIME
    XPUSHn( (NV)time(NULL) );
#else
    XPUSHu( (UV)time(NULL) );
#endif
    RETURN;
}

PP(pp_tms)
{
#ifdef HAS_TIMES
    dSP;
    struct tms timesbuf;

    EXTEND(SP, 4);
    (void)PerlProc_times(&timesbuf);

    mPUSHn(((NV)timesbuf.tms_utime)/(NV)PL_clocktick);
    if (GIMME_V == G_LIST) {
        mPUSHn(((NV)timesbuf.tms_stime)/(NV)PL_clocktick);
        mPUSHn(((NV)timesbuf.tms_cutime)/(NV)PL_clocktick);
        mPUSHn(((NV)timesbuf.tms_cstime)/(NV)PL_clocktick);
    }
    RETURN;
#elif defined(PERL_MICRO)
    dSP;
    mPUSHn(0.0);
    EXTEND(SP, 4);
    if (GIMME_V == G_LIST) {
         mPUSHn(0.0);
         mPUSHn(0.0);
         mPUSHn(0.0);
    }
    RETURN;
#else
    DIE(aTHX_ "times not implemented");
#endif /* HAS_TIMES */
}

/* The 32 bit int year limits the times we can represent to these
   boundaries with a few days wiggle room to account for time zone
   offsets
*/
/* Sat Jan  3 00:00:00 -2147481748 */
#define TIME_LOWER_BOUND -67768100567755200.0
/* Sun Dec 29 12:00:00  2147483647 */
#define TIME_UPPER_BOUND  67767976233316800.0


/* also used for: pp_localtime() */

PP(pp_gmtime)
{
    dSP;
    Time64_T when;
    struct TM tmbuf;
    struct TM *err;
    const char *opname = PL_op->op_type == OP_LOCALTIME ? "localtime" : "gmtime";
    static const char * const dayname[] =
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char * const monname[] =
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if (MAXARG < 1 || (!TOPs && ((void)POPs, 1))) {
        time_t now;
        (void)time(&now);
        when = (Time64_T)now;
    }
    else {
        NV input = Perl_floor(POPn);
        const bool pl_isnan = Perl_isnan(input);
        when = (Time64_T)input;
        if (UNLIKELY(pl_isnan || when != input)) {
            /* diag_listed_as: gmtime(%f) too large */
            Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW),
                           "%s(%.0" NVff ") too large", opname, input);
            if (pl_isnan) {
                err = NULL;
                goto failed;
            }
        }
    }

    if ( TIME_LOWER_BOUND > when ) {
        /* diag_listed_as: gmtime(%f) too small */
        Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW),
                       "%s(%.0" NVff ") too small", opname, when);
        err = NULL;
    }
    else if( when > TIME_UPPER_BOUND ) {
        /* diag_listed_as: gmtime(%f) too small */
        Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW),
                       "%s(%.0" NVff ") too large", opname, when);
        err = NULL;
    }
    else {
        if (PL_op->op_type == OP_LOCALTIME)
            err = Perl_localtime64_r(&when, &tmbuf);
        else
            err = Perl_gmtime64_r(&when, &tmbuf);
    }

    if (err == NULL) {
        /* diag_listed_as: gmtime(%f) failed */
        /* XXX %lld broken for quads */
      failed:
        Perl_ck_warner(aTHX_ packWARN(WARN_OVERFLOW),
                       "%s(%.0" NVff ") failed", opname, when);
    }

    if (GIMME_V != G_LIST) {	/* scalar context */
        EXTEND(SP, 1);
        if (err == NULL)
            RETPUSHUNDEF;
       else {
           dTARGET;
           PUSHs(TARG);
           Perl_sv_setpvf_mg(aTHX_ TARG, "%s %s %2d %02d:%02d:%02d %" IVdf,
                                dayname[tmbuf.tm_wday],
                                monname[tmbuf.tm_mon],
                                tmbuf.tm_mday,
                                tmbuf.tm_hour,
                                tmbuf.tm_min,
                                tmbuf.tm_sec,
                                (IV)tmbuf.tm_year + 1900);
        }
    }
    else {			/* list context */
        if ( err == NULL )
            RETURN;

        EXTEND(SP, 9);
        EXTEND_MORTAL(9);
        mPUSHi(tmbuf.tm_sec);
        mPUSHi(tmbuf.tm_min);
        mPUSHi(tmbuf.tm_hour);
        mPUSHi(tmbuf.tm_mday);
        mPUSHi(tmbuf.tm_mon);
        mPUSHn(tmbuf.tm_year);
        mPUSHi(tmbuf.tm_wday);
        mPUSHi(tmbuf.tm_yday);
        mPUSHi(tmbuf.tm_isdst);
    }
    RETURN;
}

PP(pp_alarm)
{
#ifdef HAS_ALARM
    dSP; dTARGET;
    /* alarm() takes an unsigned int number of seconds, and return the
     * unsigned int number of seconds remaining in the previous alarm
     * (alarms don't stack).  Therefore negative return values are not
     * possible. */
    int anum = POPi;
    if (anum < 0) {
        /* Note that while the C library function alarm() as such has
         * no errors defined (or in other words, properly behaving client
         * code shouldn't expect any), alarm() being obsoleted by
         * setitimer() and often being implemented in terms of
         * setitimer(), can fail. */
        /* diag_listed_as: %s() with negative argument */
        Perl_ck_warner_d(aTHX_ packWARN(WARN_MISC),
                         "alarm() with negative argument");
        SETERRNO(EINVAL, LIB_INVARG);
        RETPUSHUNDEF;
    }
    else {
        unsigned int retval = alarm(anum);
        if ((int)retval < 0) /* Strictly speaking "cannot happen". */
            RETPUSHUNDEF;
        PUSHu(retval);
        RETURN;
    }
#else
    DIE(aTHX_ PL_no_func, "alarm");
#endif
}

PP(pp_sleep)
{
    dSP; dTARGET;
    Time_t lasttime;
    Time_t when;

    (void)time(&lasttime);
    if (MAXARG < 1 || (!TOPs && !POPs))
        PerlProc_pause();
    else {
        const I32 duration = POPi;
        if (duration < 0) {
          /* diag_listed_as: %s() with negative argument */
          Perl_ck_warner_d(aTHX_ packWARN(WARN_MISC),
                           "sleep() with negative argument");
          SETERRNO(EINVAL, LIB_INVARG);
          XPUSHs(&PL_sv_zero);
          RETURN;
        } else {
          PerlProc_sleep((unsigned int)duration);
        }
    }
    (void)time(&when);
    XPUSHu((UV)(when - lasttime));
    RETURN;
}

/* Shared memory. */
/* Merged with some message passing. */

/* also used for: pp_msgrcv() pp_msgsnd() pp_semop() pp_shmread() */

PP(pp_shmwrite)
{
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
    dSP; dMARK; dTARGET;
    const int op_type = PL_op->op_type;
    I32 value;

    switch (op_type) {
    case OP_MSGSND:
        value = (I32)(do_msgsnd(MARK, SP) >= 0);
        break;
    case OP_MSGRCV:
        value = (I32)(do_msgrcv(MARK, SP) >= 0);
        break;
    case OP_SEMOP:
        value = (I32)(do_semop(MARK, SP) >= 0);
        break;
    default:
        value = (I32)(do_shmio(op_type, MARK, SP) >= 0);
        break;
    }

    SP = MARK;
    PUSHi(value);
    RETURN;
#else
    return Perl_pp_semget(aTHX);
#endif
}

/* Semaphores. */

/* also used for: pp_msgget() pp_shmget() */

PP(pp_semget)
{
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
    dSP; dMARK; dTARGET;
    const int anum = do_ipcget(PL_op->op_type, MARK, SP);
    SP = MARK;
    if (anum == -1)
        RETPUSHUNDEF;
    PUSHi(anum);
    RETURN;
#else
    DIE(aTHX_ "System V IPC is not implemented on this machine");
#endif
}

/* also used for: pp_msgctl() pp_shmctl() */

PP(pp_semctl)
{
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
    dSP; dMARK; dTARGET;
    const int anum = do_ipcctl(PL_op->op_type, MARK, SP);
    SP = MARK;
    if (anum == -1)
        RETPUSHUNDEF;
    if (anum != 0) {
        PUSHi(anum);
    }
    else {
        PUSHp(zero_but_true, ZBTLEN);
    }
    RETURN;
#else
    return Perl_pp_semget(aTHX);
#endif
}

/* I can't const this further without getting warnings about the types of
   various arrays passed in from structures.  */
static SV *
S_space_join_names_mortal(pTHX_ char *const *array)
{
    SV *target;

    if (array && *array) {
        target = newSVpvs_flags("", SVs_TEMP);
        while (1) {
            sv_catpv(target, *array);
            if (!*++array)
                break;
            sv_catpvs(target, " ");
        }
    } else {
        target = sv_mortalcopy(&PL_sv_no);
    }
    return target;
}

/* Get system info. */

/* also used for: pp_ghbyaddr() pp_ghbyname() */

PP(pp_ghostent)
{
#if defined(HAS_GETHOSTBYNAME) || defined(HAS_GETHOSTBYADDR) || defined(HAS_GETHOSTENT)
    dSP;
    I32 which = PL_op->op_type;
    char **elem;
    SV *sv;
#ifndef HAS_GETHOST_PROTOS /* XXX Do we need individual probes? */
    struct hostent *gethostbyaddr(Netdb_host_t, Netdb_hlen_t, int);
    struct hostent *gethostbyname(Netdb_name_t);
    struct hostent *gethostent(void);
#endif
    struct hostent *hent = NULL;
    unsigned long len;

    EXTEND(SP, 10);
    if (which == OP_GHBYNAME) {
#ifdef HAS_GETHOSTBYNAME
        const char* const name = POPpbytex;
        hent = PerlSock_gethostbyname(name);
#else
        DIE(aTHX_ PL_no_sock_func, "gethostbyname");
#endif
    }
    else if (which == OP_GHBYADDR) {
#ifdef HAS_GETHOSTBYADDR
        const int addrtype = POPi;
        SV * const addrsv = POPs;
        STRLEN addrlen;
        const char *addr = (char *)SvPVbyte(addrsv, addrlen);

        hent = PerlSock_gethostbyaddr(addr, (Netdb_hlen_t) addrlen, addrtype);
#else
        DIE(aTHX_ PL_no_sock_func, "gethostbyaddr");
#endif
    }
    else
#ifdef HAS_GETHOSTENT
        hent = PerlSock_gethostent();
#else
        DIE(aTHX_ PL_no_sock_func, "gethostent");
#endif

#ifdef HOST_NOT_FOUND
        if (!hent) {
#ifdef USE_REENTRANT_API
#   ifdef USE_GETHOSTENT_ERRNO
            h_errno = PL_reentrant_buffer->_gethostent_errno;
#   endif
#endif
            STATUS_UNIX_SET(h_errno);
        }
#endif

    if (GIMME_V != G_LIST) {
        PUSHs(sv = sv_newmortal());
        if (hent) {
            if (which == OP_GHBYNAME) {
                if (hent->h_addr) {
                    sv_upgrade(sv, SVt_PV);
                    sv_setpvn_fresh(sv, hent->h_addr, hent->h_length);
                }
            }
            else
                sv_setpv(sv, (char*)hent->h_name);
        }
        RETURN;
    }

    if (hent) {
        mPUSHs(newSVpv((char*)hent->h_name, 0));
        PUSHs(space_join_names_mortal(hent->h_aliases));
        mPUSHi(hent->h_addrtype);
        len = hent->h_length;
        mPUSHi(len);
#ifdef h_addr
        for (elem = hent->h_addr_list; elem && *elem; elem++) {
            mXPUSHp(*elem, len);
        }
#else
        if (hent->h_addr)
            mPUSHp(hent->h_addr, len);
        else
            PUSHs(sv_mortalcopy(&PL_sv_no));
#endif /* h_addr */
    }
    RETURN;
#else
    DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
}

/* also used for: pp_gnbyaddr() pp_gnbyname() */

PP(pp_gnetent)
{
#if defined(HAS_GETNETBYNAME) || defined(HAS_GETNETBYADDR) || defined(HAS_GETNETENT)
    dSP;
    I32 which = PL_op->op_type;
    SV *sv;
#ifndef HAS_GETNET_PROTOS /* XXX Do we need individual probes? */
    struct netent *getnetbyaddr(Netdb_net_t, int);
    struct netent *getnetbyname(Netdb_name_t);
    struct netent *getnetent(void);
#endif
    struct netent *nent;

    if (which == OP_GNBYNAME){
#ifdef HAS_GETNETBYNAME
        const char * const name = POPpbytex;
        nent = PerlSock_getnetbyname(name);
#else
        DIE(aTHX_ PL_no_sock_func, "getnetbyname");
#endif
    }
    else if (which == OP_GNBYADDR) {
#ifdef HAS_GETNETBYADDR
        const int addrtype = POPi;
        const Netdb_net_t addr = (Netdb_net_t) (U32)POPu;
        nent = PerlSock_getnetbyaddr(addr, addrtype);
#else
        DIE(aTHX_ PL_no_sock_func, "getnetbyaddr");
#endif
    }
    else
#ifdef HAS_GETNETENT
        nent = PerlSock_getnetent();
#else
        DIE(aTHX_ PL_no_sock_func, "getnetent");
#endif

#ifdef HOST_NOT_FOUND
        if (!nent) {
#ifdef USE_REENTRANT_API
#   ifdef USE_GETNETENT_ERRNO
             h_errno = PL_reentrant_buffer->_getnetent_errno;
#   endif
#endif
            STATUS_UNIX_SET(h_errno);
        }
#endif

    EXTEND(SP, 4);
    if (GIMME_V != G_LIST) {
        PUSHs(sv = sv_newmortal());
        if (nent) {
            if (which == OP_GNBYNAME)
                sv_setiv(sv, (IV)nent->n_net);
            else
                sv_setpv(sv, nent->n_name);
        }
        RETURN;
    }

    if (nent) {
        mPUSHs(newSVpv(nent->n_name, 0));
        PUSHs(space_join_names_mortal(nent->n_aliases));
        mPUSHi(nent->n_addrtype);
        mPUSHi(nent->n_net);
    }

    RETURN;
#else
    DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
}


/* also used for: pp_gpbyname() pp_gpbynumber() */

PP(pp_gprotoent)
{
#if defined(HAS_GETPROTOBYNAME) || defined(HAS_GETPROTOBYNUMBER) || defined(HAS_GETPROTOENT)
    dSP;
    I32 which = PL_op->op_type;
    SV *sv;
#ifndef HAS_GETPROTO_PROTOS /* XXX Do we need individual probes? */
    struct protoent *getprotobyname(Netdb_name_t);
    struct protoent *getprotobynumber(int);
    struct protoent *getprotoent(void);
#endif
    struct protoent *pent;

    if (which == OP_GPBYNAME) {
#ifdef HAS_GETPROTOBYNAME
        const char* const name = POPpbytex;
        pent = PerlSock_getprotobyname(name);
#else
        DIE(aTHX_ PL_no_sock_func, "getprotobyname");
#endif
    }
    else if (which == OP_GPBYNUMBER) {
#ifdef HAS_GETPROTOBYNUMBER
        const int number = POPi;
        pent = PerlSock_getprotobynumber(number);
#else
        DIE(aTHX_ PL_no_sock_func, "getprotobynumber");
#endif
    }
    else
#ifdef HAS_GETPROTOENT
        pent = PerlSock_getprotoent();
#else
        DIE(aTHX_ PL_no_sock_func, "getprotoent");
#endif

    EXTEND(SP, 3);
    if (GIMME_V != G_LIST) {
        PUSHs(sv = sv_newmortal());
        if (pent) {
            if (which == OP_GPBYNAME)
                sv_setiv(sv, (IV)pent->p_proto);
            else
                sv_setpv(sv, pent->p_name);
        }
        RETURN;
    }

    if (pent) {
        mPUSHs(newSVpv(pent->p_name, 0));
        PUSHs(space_join_names_mortal(pent->p_aliases));
        mPUSHi(pent->p_proto);
    }

    RETURN;
#else
    DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
}


/* also used for: pp_gsbyname() pp_gsbyport() */

PP(pp_gservent)
{
#if defined(HAS_GETSERVBYNAME) || defined(HAS_GETSERVBYPORT) || defined(HAS_GETSERVENT)
    dSP;
    I32 which = PL_op->op_type;
    SV *sv;
#ifndef HAS_GETSERV_PROTOS /* XXX Do we need individual probes? */
    struct servent *getservbyname(Netdb_name_t, Netdb_name_t);
    struct servent *getservbyport(int, Netdb_name_t);
    struct servent *getservent(void);
#endif
    struct servent *sent;

    if (which == OP_GSBYNAME) {
#ifdef HAS_GETSERVBYNAME
        const char * const proto = POPpbytex;
        const char * const name = POPpbytex;
        sent = PerlSock_getservbyname(name, (proto && !*proto) ? NULL : proto);
#else
        DIE(aTHX_ PL_no_sock_func, "getservbyname");
#endif
    }
    else if (which == OP_GSBYPORT) {
#ifdef HAS_GETSERVBYPORT
        const char * const proto = POPpbytex;
        unsigned short port = (unsigned short)POPu;
        port = PerlSock_htons(port);
        sent = PerlSock_getservbyport(port, (proto && !*proto) ? NULL : proto);
#else
        DIE(aTHX_ PL_no_sock_func, "getservbyport");
#endif
    }
    else
#ifdef HAS_GETSERVENT
        sent = PerlSock_getservent();
#else
        DIE(aTHX_ PL_no_sock_func, "getservent");
#endif

    EXTEND(SP, 4);
    if (GIMME_V != G_LIST) {
        PUSHs(sv = sv_newmortal());
        if (sent) {
            if (which == OP_GSBYNAME) {
                sv_setiv(sv, (IV)PerlSock_ntohs(sent->s_port));
            }
            else
                sv_setpv(sv, sent->s_name);
        }
        RETURN;
    }

    if (sent) {
        mPUSHs(newSVpv(sent->s_name, 0));
        PUSHs(space_join_names_mortal(sent->s_aliases));
        mPUSHi(PerlSock_ntohs(sent->s_port));
        mPUSHs(newSVpv(sent->s_proto, 0));
    }

    RETURN;
#else
    DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
}


/* also used for: pp_snetent() pp_sprotoent() pp_sservent() */

PP(pp_shostent)
{
    dSP;
    const int stayopen = TOPi;
    switch(PL_op->op_type) {
    case OP_SHOSTENT:
#ifdef HAS_SETHOSTENT
        PerlSock_sethostent(stayopen);
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_SNETENT:
#ifdef HAS_SETNETENT
        PerlSock_setnetent(stayopen);
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_SPROTOENT:
#ifdef HAS_SETPROTOENT
        PerlSock_setprotoent(stayopen);
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_SSERVENT:
#ifdef HAS_SETSERVENT
        PerlSock_setservent(stayopen);
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    }
    RETSETYES;
}


/* also used for: pp_egrent() pp_enetent() pp_eprotoent() pp_epwent()
 *                pp_eservent() pp_sgrent() pp_spwent() */

PP(pp_ehostent)
{
    dSP;
    switch(PL_op->op_type) {
    case OP_EHOSTENT:
#ifdef HAS_ENDHOSTENT
        PerlSock_endhostent();
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_ENETENT:
#ifdef HAS_ENDNETENT
        PerlSock_endnetent();
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_EPROTOENT:
#ifdef HAS_ENDPROTOENT
        PerlSock_endprotoent();
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_ESERVENT:
#ifdef HAS_ENDSERVENT
        PerlSock_endservent();
#else
        DIE(aTHX_ PL_no_sock_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_SGRENT:
#if defined(HAS_GROUP) && defined(HAS_SETGRENT)
        setgrent();
#else
        DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_EGRENT:
#if defined(HAS_GROUP) && defined(HAS_ENDGRENT)
        endgrent();
#else
        DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_SPWENT:
#if defined(HAS_PASSWD) && defined(HAS_SETPWENT)
        setpwent();
#else
        DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    case OP_EPWENT:
#if defined(HAS_PASSWD) && defined(HAS_ENDPWENT)
        endpwent();
#else
        DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
        break;
    }
    EXTEND(SP,1);
    RETPUSHYES;
}


/* also used for: pp_gpwnam() pp_gpwuid() */

PP(pp_gpwent)
{
#ifdef HAS_PASSWD
    dSP;
    I32 which = PL_op->op_type;
    SV *sv;
    struct passwd *pwent  = NULL;
    /*
     * We currently support only the SysV getsp* shadow password interface.
     * The interface is declared in <shadow.h> and often one needs to link
     * with -lsecurity or some such.
     * This interface is used at least by Solaris, HP-UX, IRIX, and Linux.
     * (and SCO?)
     *
     * AIX getpwnam() is clever enough to return the encrypted password
     * only if the caller (euid?) is root.
     *
     * There are at least three other shadow password APIs.  Many platforms
     * seem to contain more than one interface for accessing the shadow
     * password databases, possibly for compatibility reasons.
     * The getsp*() is by far he simplest one, the other two interfaces
     * are much more complicated, but also very similar to each other.
     *
     * <sys/types.h>
     * <sys/security.h>
     * <prot.h>
     * struct pr_passwd *getprpw*();
     * The password is in
     * char getprpw*(...).ufld.fd_encrypt[]
     * Mention HAS_GETPRPWNAM here so that Configure probes for it.
     *
     * <sys/types.h>
     * <sys/security.h>
     * <prot.h>
     * struct es_passwd *getespw*();
     * The password is in
     * char *(getespw*(...).ufld.fd_encrypt)
     * Mention HAS_GETESPWNAM here so that Configure probes for it.
     *
     * <userpw.h> (AIX)
     * struct userpw *getuserpw();
     * The password is in
     * char *(getuserpw(...)).spw_upw_passwd
     * (but the de facto standard getpwnam() should work okay)
     *
     * Mention I_PROT here so that Configure probes for it.
     *
     * In HP-UX for getprpw*() the manual page claims that one should include
     * <hpsecurity.h> instead of <sys/security.h>, but that is not needed
     * if one includes <shadow.h> as that includes <hpsecurity.h>,
     * and pp_sys.c already includes <shadow.h> if there is such.
     *
     * Note that <sys/security.h> is already probed for, but currently
     * it is only included in special cases.
     *
     * In Digital UNIX/Tru64 if using the getespw*() (which seems to be
     * the preferred interface, even though also the getprpw*() interface
     * is available) one needs to link with -lsecurity -ldb -laud -lm.
     * One also needs to call set_auth_parameters() in main() before
     * doing anything else, whether one is using getespw*() or getprpw*().
     *
     * Note that accessing the shadow databases can be magnitudes
     * slower than accessing the standard databases.
     *
     * --jhi
     */

#   if defined(__CYGWIN__) && defined(USE_REENTRANT_API)
    /* Cygwin 1.5.3-1 has buggy getpwnam_r() and getpwuid_r():
     * the pw_comment is left uninitialized. */
    PL_reentrant_buffer->_pwent_struct.pw_comment = NULL;
#   endif

    switch (which) {
    case OP_GPWNAM:
      {
        const char* const name = POPpbytex;
        GETPWNAM_LOCK;
        pwent  = getpwnam(name);
        GETPWNAM_UNLOCK;
      }
      break;
    case OP_GPWUID:
      {
        Uid_t uid = POPi;
        GETPWUID_LOCK;
        pwent = getpwuid(uid);
        GETPWUID_UNLOCK;
      }
        break;
    case OP_GPWENT:
#   ifdef HAS_GETPWENT
        pwent  = getpwent();
#ifdef POSIX_BC   /* In some cases pw_passwd has invalid addresses */
        if (pwent) {
            GETPWNAM_LOCK;
            pwent = getpwnam(pwent->pw_name);
            GETPWNAM_UNLOCK;
        }
#endif
#   else
        DIE(aTHX_ PL_no_func, "getpwent");
#   endif
        break;
    }

    EXTEND(SP, 10);
    if (GIMME_V != G_LIST) {
        PUSHs(sv = sv_newmortal());
        if (pwent) {
            if (which == OP_GPWNAM)
                sv_setuid(sv, pwent->pw_uid);
            else
                sv_setpv(sv, pwent->pw_name);
        }
        RETURN;
    }

    if (pwent) {
        mPUSHs(newSVpv(pwent->pw_name, 0));

        sv = newSViv(0);
        mPUSHs(sv);
        /* If we have getspnam(), we try to dig up the shadow
         * password.  If we are underprivileged, the shadow
         * interface will set the errno to EACCES or similar,
         * and return a null pointer.  If this happens, we will
         * use the dummy password (usually "*" or "x") from the
         * standard password database.
         *
         * In theory we could skip the shadow call completely
         * if euid != 0 but in practice we cannot know which
         * security measures are guarding the shadow databases
         * on a random platform.
         *
         * Resist the urge to use additional shadow interfaces.
         * Divert the urge to writing an extension instead.
         *
         * --jhi */
        /* Some AIX setups falsely(?) detect some getspnam(), which
         * has a different API than the Solaris/IRIX one. */
#   if defined(HAS_GETSPNAM) && !defined(_AIX)
        {
            const struct spwd * spwent;
            dSAVE_ERRNO;
            GETSPNAM_LOCK;
            spwent = getspnam(pwent->pw_name);
                          /* Save and restore errno so that
                           * underprivileged attempts seem
                           * to have never made the unsuccessful
                           * attempt to retrieve the shadow password. */
            RESTORE_ERRNO;
            if (spwent && spwent->sp_pwdp)
                sv_setpv(sv, spwent->sp_pwdp);
            GETSPNAM_UNLOCK;
        }
#   endif
#   ifdef PWPASSWD
        if (!SvPOK(sv)) /* Use the standard password, then. */
            sv_setpv(sv, pwent->pw_passwd);
#   endif

        /* passwd is tainted because user himself can diddle with it.
         * admittedly not much and in a very limited way, but nevertheless. */
        SvTAINTED_on(sv);

        sv_setuid(PUSHmortal, pwent->pw_uid);
        sv_setgid(PUSHmortal, pwent->pw_gid);

        /* pw_change, pw_quota, and pw_age are mutually exclusive--
         * because of the poor interface of the Perl getpw*(),
         * not because there's some standard/convention saying so.
         * A better interface would have been to return a hash,
         * but we are accursed by our history, alas. --jhi.  */
#   ifdef PWCHANGE
        mPUSHi(pwent->pw_change);
#   elif defined(PWQUOTA)
        mPUSHi(pwent->pw_quota);
#   elif defined(PWAGE)
        mPUSHs(newSVpv(pwent->pw_age, 0));
#   else
        /* I think that you can never get this compiled, but just in case.  */
        PUSHs(sv_mortalcopy(&PL_sv_no));
#   endif

        /* pw_class and pw_comment are mutually exclusive--.
         * see the above note for pw_change, pw_quota, and pw_age. */
#   ifdef PWCLASS
        mPUSHs(newSVpv(pwent->pw_class, 0));
#   elif defined(PWCOMMENT)
        mPUSHs(newSVpv(pwent->pw_comment, 0));
#   else
        /* I think that you can never get this compiled, but just in case.  */
        PUSHs(sv_mortalcopy(&PL_sv_no));
#   endif

#   ifdef PWGECOS
        PUSHs(sv = newSVpvn_flags(pwent->pw_gecos,
            pwent->pw_gecos == NULL ? 0 : strlen(pwent->pw_gecos),
            SVs_TEMP));
#   else
        PUSHs(sv = sv_mortalcopy(&PL_sv_no));
#   endif
        /* pw_gecos is tainted because user himself can diddle with it. */
        SvTAINTED_on(sv);

        mPUSHs(newSVpv(pwent->pw_dir, 0));

        PUSHs(sv = newSVpvn_flags(pwent->pw_shell,
            pwent->pw_shell == NULL ? 0 : strlen(pwent->pw_shell),
            SVs_TEMP));
        /* pw_shell is tainted because user himself can diddle with it. */
        SvTAINTED_on(sv);

#   ifdef PWEXPIRE
        mPUSHi(pwent->pw_expire);
#   endif
    }
    RETURN;
#else
    DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
}


/* also used for: pp_ggrgid() pp_ggrnam() */

PP(pp_ggrent)
{
#ifdef HAS_GROUP
    dSP;
    const I32 which = PL_op->op_type;
    const struct group *grent;

    if (which == OP_GGRNAM) {
        const char* const name = POPpbytex;
        grent = (const struct group *)getgrnam(name);
    }
    else if (which == OP_GGRGID) {
#if Gid_t_sign == 1
        const Gid_t gid = POPu;
#elif Gid_t_sign == -1
        const Gid_t gid = POPi;
#else
#  error "Unexpected Gid_t_sign"
#endif
        grent = (const struct group *)getgrgid(gid);
    }
    else
#ifdef HAS_GETGRENT
        grent = (struct group *)getgrent();
#else
        DIE(aTHX_ PL_no_func, "getgrent");
#endif

    EXTEND(SP, 4);
    if (GIMME_V != G_LIST) {
        SV * const sv = sv_newmortal();

        PUSHs(sv);
        if (grent) {
            if (which == OP_GGRNAM)
                sv_setgid(sv, grent->gr_gid);
            else
                sv_setpv(sv, grent->gr_name);
        }
        RETURN;
    }

    if (grent) {
        mPUSHs(newSVpv(grent->gr_name, 0));

#ifdef GRPASSWD
        mPUSHs(newSVpv(grent->gr_passwd, 0));
#else
        PUSHs(sv_mortalcopy(&PL_sv_no));
#endif

        sv_setgid(PUSHmortal, grent->gr_gid);

#if !(defined(_CRAYMPP) && defined(USE_REENTRANT_API))
        /* In UNICOS/mk (_CRAYMPP) the multithreading
         * versions (getgrnam_r, getgrgid_r)
         * seem to return an illegal pointer
         * as the group members list, gr_mem.
         * getgrent() doesn't even have a _r version
         * but the gr_mem is poisonous anyway.
         * So yes, you cannot get the list of group
         * members if building multithreaded in UNICOS/mk. */
        PUSHs(space_join_names_mortal(grent->gr_mem));
#endif
    }

    RETURN;
#else
    DIE(aTHX_ PL_no_func, PL_op_desc[PL_op->op_type]);
#endif
}

PP(pp_getlogin)
{
#ifdef HAS_GETLOGIN
    dSP; dTARGET;
    char *tmps;
    EXTEND(SP, 1);
    if (!(tmps = PerlProc_getlogin()))
        RETPUSHUNDEF;
    sv_setpv_mg(TARG, tmps);
    PUSHs(TARG);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "getlogin");
#endif
}

/* Miscellaneous. */

PP(pp_syscall)
{
#ifdef HAS_SYSCALL
    dSP; dMARK; dORIGMARK; dTARGET;
    I32 items = SP - MARK;
    unsigned long a[20];
    I32 i = 0;
    IV retval = -1;

    if (TAINTING_get) {
        while (++MARK <= SP) {
            if (SvTAINTED(*MARK)) {
                TAINT;
                break;
            }
        }
        MARK = ORIGMARK;
        TAINT_PROPER("syscall");
    }

    /* This probably won't work on machines where sizeof(long) != sizeof(int)
     * or where sizeof(long) != sizeof(char*).  But such machines will
     * not likely have syscall implemented either, so who cares?
     */
    while (++MARK <= SP) {
        if (SvNIOK(*MARK) || !i)
            a[i++] = SvIV(*MARK);
        else if (*MARK == &PL_sv_undef)
            a[i++] = 0;
        else
            a[i++] = (unsigned long)SvPV_force_nolen(*MARK);
        if (i > 15)
            break;
    }
    switch (items) {
    default:
        DIE(aTHX_ "Too many args to syscall");
    case 0:
        DIE(aTHX_ "Too few args to syscall");
    case 1:
        retval = syscall(a[0]);
        break;
    case 2:
        retval = syscall(a[0],a[1]);
        break;
    case 3:
        retval = syscall(a[0],a[1],a[2]);
        break;
    case 4:
        retval = syscall(a[0],a[1],a[2],a[3]);
        break;
    case 5:
        retval = syscall(a[0],a[1],a[2],a[3],a[4]);
        break;
    case 6:
        retval = syscall(a[0],a[1],a[2],a[3],a[4],a[5]);
        break;
    case 7:
        retval = syscall(a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
        break;
    case 8:
        retval = syscall(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
        break;
    }
    SP = ORIGMARK;
    PUSHi(retval);
    RETURN;
#else
    DIE(aTHX_ PL_no_func, "syscall");
#endif
}

#ifdef FCNTL_EMULATE_FLOCK

/*  XXX Emulate flock() with fcntl().
    What's really needed is a good file locking module.
*/

static int
fcntl_emulate_flock(int fd, int operation)
{
    int res;
    struct flock flock;

    switch (operation & ~LOCK_NB) {
    case LOCK_SH:
        flock.l_type = F_RDLCK;
        break;
    case LOCK_EX:
        flock.l_type = F_WRLCK;
        break;
    case LOCK_UN:
        flock.l_type = F_UNLCK;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    flock.l_whence = SEEK_SET;
    flock.l_start = flock.l_len = (Off_t)0;

    res = fcntl(fd, (operation & LOCK_NB) ? F_SETLK : F_SETLKW, &flock);
    if (res == -1 && ((errno == EAGAIN) || (errno == EACCES)))
        errno = EWOULDBLOCK;
    return res;
}

#endif /* FCNTL_EMULATE_FLOCK */

#ifdef LOCKF_EMULATE_FLOCK

/*  XXX Emulate flock() with lockf().  This is just to increase
    portability of scripts.  The calls are not completely
    interchangeable.  What's really needed is a good file
    locking module.
*/

/*  The lockf() constants might have been defined in <unistd.h>.
    Unfortunately, <unistd.h> causes troubles on some mixed
    (BSD/POSIX) systems, such as SunOS 4.1.3.

   Further, the lockf() constants aren't POSIX, so they might not be
   visible if we're compiling with _POSIX_SOURCE defined.  Thus, we'll
   just stick in the SVID values and be done with it.  Sigh.
*/

# ifndef F_ULOCK
#  define F_ULOCK	0	/* Unlock a previously locked region */
# endif
# ifndef F_LOCK
#  define F_LOCK	1	/* Lock a region for exclusive use */
# endif
# ifndef F_TLOCK
#  define F_TLOCK	2	/* Test and lock a region for exclusive use */
# endif
# ifndef F_TEST
#  define F_TEST	3	/* Test a region for other processes locks */
# endif

static int
lockf_emulate_flock(int fd, int operation)
{
    int i;
    Off_t pos;
    dSAVE_ERRNO;

    /* flock locks entire file so for lockf we need to do the same	*/
    pos = PerlLIO_lseek(fd, (Off_t)0, SEEK_CUR);    /* get pos to restore later */
    if (pos > 0)	/* is seekable and needs to be repositioned	*/
        if (PerlLIO_lseek(fd, (Off_t)0, SEEK_SET) < 0)
            pos = -1;	/* seek failed, so don't seek back afterwards	*/
    RESTORE_ERRNO;

    switch (operation) {

        /* LOCK_SH - get a shared lock */
        case LOCK_SH:
        /* LOCK_EX - get an exclusive lock */
        case LOCK_EX:
            i = lockf (fd, F_LOCK, 0);
            break;

        /* LOCK_SH|LOCK_NB - get a non-blocking shared lock */
        case LOCK_SH|LOCK_NB:
        /* LOCK_EX|LOCK_NB - get a non-blocking exclusive lock */
        case LOCK_EX|LOCK_NB:
            i = lockf (fd, F_TLOCK, 0);
            if (i == -1)
                if ((errno == EAGAIN) || (errno == EACCES))
                    errno = EWOULDBLOCK;
            break;

        /* LOCK_UN - unlock (non-blocking is a no-op) */
        case LOCK_UN:
        case LOCK_UN|LOCK_NB:
            i = lockf (fd, F_ULOCK, 0);
            break;

        /* Default - can't decipher operation */
        default:
            i = -1;
            errno = EINVAL;
            break;
    }

    if (pos > 0)      /* need to restore position of the handle	*/
        PerlLIO_lseek(fd, pos, SEEK_SET);	/* ignore error here	*/

    return (i);
}

#endif /* LOCKF_EMULATE_FLOCK */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
