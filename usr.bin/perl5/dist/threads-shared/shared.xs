/*    shared.xs
 *
 *    Copyright (c) 2001-2002, 2006 Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * "Hand any two wizards a piece of rope and they would instinctively pull in
 * opposite directions."
 *                         --Sourcery
 *
 * Contributed by Artur Bergman <sky AT crucially DOT net>
 * Pulled in the (an)other direction by Nick Ing-Simmons
 *      <nick AT ing-simmons DOT net>
 * CPAN version produced by Jerry D. Hedden <jdhedden AT cpan DOT org>
 */

/*
 * Shared variables are implemented by a scheme similar to tieing.
 * Each thread has a proxy SV with attached magic -- "private SVs" --
 * which all point to a single SV in a separate shared interpreter
 * (PL_sharedsv_space) -- "shared SVs".
 *
 * The shared SV holds the variable's true values, and its state is
 * copied between the shared and private SVs with the usual
 * mg_get()/mg_set() arrangement.
 *
 * Aggregates (AVs and HVs) are implemented using tie magic, except that
 * the vtable used is one defined in this file rather than the standard one.
 * This means that where a tie function like FETCH is normally invoked by
 * the tie magic's mg_get() function, we completely bypass the calling of a
 * perl-level function, and directly call C-level code to handle it. On
 * the other hand, calls to functions like PUSH are done directly by code
 * in av.c, etc., which we can't bypass. So the best we can do is to provide
 * XS versions of these functions. We also have to attach a tie object,
 * blessed into the class threads::shared::tie, to keep the method-calling
 * code happy.
 *
 * Access to aggregate elements is done the usual tied way by returning a
 * proxy PVLV element with attached element magic.
 *
 * Pointers to the shared SV are squirrelled away in the mg->mg_ptr field
 * of magic (with mg_len == 0), and in the INT2PTR(SvIV(sv)) field of tied
 * object SVs. These pointers have to be hidden like this because they
 * cross interpreter boundaries, and we don't want sv_clear() and friends
 * following them.
 *
 * The three basic shared types look like the following:
 *
 * -----------------
 *
 * Shared scalar (my $s : shared):
 *
 *  SV = PVMG(0x7ba238) at 0x7387a8
 *   FLAGS = (PADMY,GMG,SMG)
 *   MAGIC = 0x824d88
 *     MG_TYPE = PERL_MAGIC_shared_scalar(n)
 *     MG_PTR = 0x810358                <<<< pointer to the shared SV
 *
 * -----------------
 *
 * Shared aggregate (my @a : shared;  my %h : shared):
 *
 * SV = PVAV(0x7175d0) at 0x738708
 *   FLAGS = (PADMY,RMG)
 *   MAGIC = 0x824e48
 *     MG_TYPE = PERL_MAGIC_tied(P)
 *     MG_OBJ = 0x7136e0                <<<< ref to the tied object
 *     SV = RV(0x7136f0) at 0x7136e0
 *       RV = 0x738640
 *       SV = PVMG(0x7ba238) at 0x738640 <<<< the tied object
 *         FLAGS = (OBJECT,IOK,pIOK)
 *         IV = 8455000                 <<<< pointer to the shared AV
 *         STASH = 0x80abf0 "threads::shared::tie"
 *     MG_PTR = 0x810358 ""             <<<< another pointer to the shared AV
 *   ARRAY = 0x0
 *
 * -----------------
 *
 * Aggregate element (my @a : shared; $a[0])
 *
 * SV = PVLV(0x77f628) at 0x713550
 *   FLAGS = (GMG,SMG,RMG,pIOK)
 *   MAGIC = 0x72bd58
 *     MG_TYPE = PERL_MAGIC_shared_scalar(n)
 *     MG_PTR = 0x8103c0 ""             <<<< pointer to the shared element
 *   MAGIC = 0x72bd18
 *     MG_TYPE = PERL_MAGIC_tiedelem(p)
 *     MG_OBJ = 0x7136e0                <<<< ref to the tied object
 *     SV = RV(0x7136f0) at 0x7136e0
 *       RV = 0x738660
 *       SV = PVMG(0x7ba278) at 0x738660 <<<< the tied object
 *         FLAGS = (OBJECT,IOK,pIOK)
 *         IV = 8455064                 <<<< pointer to the shared AV
 *         STASH = 0x80ac30 "threads::shared::tie"
 *   TYPE = t
 *
 * Note that PERL_MAGIC_tiedelem(p) magic doesn't have a pointer to a
 * shared SV in mg_ptr; instead this is used to store the hash key,
 * if any, like normal tied elements. Note also that element SVs may have
 * pointers to both the shared aggregate and the shared element.
 *
 *
 * Userland locks:
 *
 * If a shared variable is used as a perl-level lock or condition
 * variable, then PERL_MAGIC_ext magic is attached to the associated
 * *shared* SV, whose mg_ptr field points to a malloc'ed structure
 * containing the necessary mutexes and condition variables.
 *
 * Nomenclature:
 *
 * In this file, any variable name prefixed with 's' (e.g., ssv, stmp or sobj)
 * usually represents a shared SV which corresponds to a private SV named
 * without the prefix (e.g., sv, tmp or obj).
 */

/* this is lower overhead than warn() and less likely to interfere
   with other parts of perl (like with the debugger.)
*/
#ifdef SHARED_TRACE_LOCKS
#  define TRACE_LOCK(x) DEBUG_U(x)
#  define TRACE_LOCKv(x) DEBUG_Uv(x)
#else
#  define TRACE_LOCK(x)
#  define TRACE_LOCKv(x)
#endif

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#define NEED_sv_2pv_flags
#define NEED_vnewSVpvf
#define NEED_warner
#define NEED_newSVpvn_flags
#include "ppport.h"

#ifndef CLANG_DIAG_IGNORE
# define CLANG_DIAG_IGNORE(x)
# define CLANG_DIAG_RESTORE
#endif
#ifndef CLANG_DIAG_IGNORE_STMT
# define CLANG_DIAG_IGNORE_STMT(x) CLANG_DIAG_IGNORE(x) NOOP
# define CLANG_DIAG_RESTORE_STMT CLANG_DIAG_RESTORE NOOP
#endif

#ifdef USE_ITHREADS

/* Magic signature(s) for mg_private to make PERL_MAGIC_ext magic safer */
#define UL_MAGIC_SIG 0x554C  /* UL = user lock */

/*
 * The shared things need an interpreter to live in ...
 */
static PerlInterpreter *PL_sharedsv_space;             /* The shared sv space */
/* To access shared space we fake aTHX in this scope and thread's context */

/* Bug #24255: We include ENTER+SAVETMPS/FREETMPS+LEAVE with
 * SHARED_CONTEXT/CALLER_CONTEXT macros, so that any mortals, etc. created
 * while in the shared interpreter context don't languish */

#define SHARED_CONTEXT                                  \
    STMT_START {                                        \
        PERL_SET_CONTEXT((aTHX = PL_sharedsv_space));   \
        ENTER;                                          \
        SAVETMPS;                                       \
    } STMT_END

/* So we need a way to switch back to the caller's context... */
/* So we declare _another_ copy of the aTHX variable ... */
#define dTHXc PerlInterpreter *caller_perl = aTHX

/* ... and use it to switch back */
#define CALLER_CONTEXT                                  \
    STMT_START {                                        \
        FREETMPS;                                       \
        LEAVE;                                          \
        PERL_SET_CONTEXT((aTHX = caller_perl));         \
    } STMT_END

/*
 * Only one thread at a time is allowed to mess with shared space.
 */

typedef struct {
    perl_mutex          mutex;
    PerlInterpreter    *owner;
    I32                 locks;
    perl_cond           cond;
#ifdef DEBUG_LOCKS
    const char *        file;
    int                 line;
#endif
} recursive_lock_t;

static recursive_lock_t PL_sharedsv_lock;   /* Mutex protecting the shared sv space */

static void
recursive_lock_init(pTHX_ recursive_lock_t *lock)
{
    Zero(lock,1,recursive_lock_t);
    MUTEX_INIT(&lock->mutex);
    COND_INIT(&lock->cond);
}

static void
recursive_lock_destroy(pTHX_ recursive_lock_t *lock)
{
    MUTEX_DESTROY(&lock->mutex);
    COND_DESTROY(&lock->cond);
}

static void
recursive_lock_release(pTHX_ recursive_lock_t *lock)
{
    MUTEX_LOCK(&lock->mutex);
    if (lock->owner == aTHX) {
        if (--lock->locks == 0) {
            lock->owner = NULL;
            COND_SIGNAL(&lock->cond);
            TRACE_LOCK(
                    PerlIO_printf(Perl_debug_log, "shared lock released %p for %p at %s:%d\n",
                                  lock, aTHX, CopFILE(PL_curcop), CopLINE(PL_curcop))
                    );
        }
        else {
            TRACE_LOCKv(
                    PerlIO_printf(Perl_debug_log, "shared lock unbump %p for %p at %s:%d\n",
                                  lock, aTHX, CopFILE(PL_curcop), CopLINE(PL_curcop))
                    );
        }
    }
    else {
        TRACE_LOCK(
                PerlIO_printf(Perl_debug_log, "bad shared lock release %p for %p (owned by %p) at %s:%d\n",
                               lock, aTHX, lock->owner, CopFILE(PL_curcop), CopLINE(PL_curcop))
                 );
    }
    MUTEX_UNLOCK(&lock->mutex);
}

static void
recursive_lock_acquire(pTHX_ recursive_lock_t *lock, const char *file, int line)
{
    PERL_UNUSED_ARG(file);
    PERL_UNUSED_ARG(line);
    assert(aTHX);
    MUTEX_LOCK(&lock->mutex);
    if (lock->owner == aTHX) {
        TRACE_LOCKv(
                 PerlIO_printf(Perl_debug_log, "shared lock bump %p (%p) at %s:%d\n",
                               lock, lock->owner, CopFILE(PL_curcop), CopLINE(PL_curcop))
                 );
        lock->locks++;
    } else {
        TRACE_LOCK(
                 PerlIO_printf(Perl_debug_log, "shared lock try %p for %p (owned by %p) at %s:%d\n",
                               lock, aTHX, lock->owner, CopFILE(PL_curcop), CopLINE(PL_curcop))
                 );
        while (lock->owner) {
#ifdef DEBUG_LOCKS
            Perl_warn(aTHX_ " %p waiting - owned by %p %s:%d\n",
                      aTHX, lock->owner, lock->file, lock->line);
#endif
            COND_WAIT(&lock->cond,&lock->mutex);
        }
        TRACE_LOCK(
                 PerlIO_printf(Perl_debug_log, "shared lock got %p at %s:%d\n",
                               lock, CopFILE(PL_curcop), CopLINE(PL_curcop))
                 );
        lock->locks = 1;
        lock->owner = aTHX;
#ifdef DEBUG_LOCKS
        lock->file  = file;
        lock->line  = line;
#endif
    }
    MUTEX_UNLOCK(&lock->mutex);
    SAVEDESTRUCTOR_X(recursive_lock_release,lock);
}

#define ENTER_LOCK                                                          \
    STMT_START {                                                            \
        ENTER;                                                              \
        recursive_lock_acquire(aTHX_ &PL_sharedsv_lock, __FILE__, __LINE__);\
    } STMT_END

/* The unlocking is done automatically at scope exit */
#define LEAVE_LOCK      LEAVE


/* A common idiom is to acquire access and switch in ... */
#define SHARED_EDIT     \
    STMT_START {        \
        ENTER_LOCK;     \
        SHARED_CONTEXT; \
    } STMT_END

/* ... then switch out and release access. */
#define SHARED_RELEASE  \
    STMT_START {        \
        CALLER_CONTEXT; \
        LEAVE_LOCK;     \
    } STMT_END


/* User-level locks:
   This structure is attached (using ext magic) to any shared SV that
   is used by user-level locking or condition code
*/

typedef struct {
    recursive_lock_t    lock;           /* For user-levl locks */
    perl_cond           user_cond;      /* For user-level conditions */
} user_lock;

/* Magic used for attaching user_lock structs to shared SVs

   The vtable used has just one entry - when the SV goes away
   we free the memory for the above.
 */

static int
sharedsv_userlock_free(pTHX_ SV *sv, MAGIC *mg)
{
    user_lock *ul = (user_lock *) mg->mg_ptr;
    PERL_UNUSED_ARG(sv);
    assert(aTHX == PL_sharedsv_space);
    if (ul) {
        recursive_lock_destroy(aTHX_ &ul->lock);
        COND_DESTROY(&ul->user_cond);
        PerlMemShared_free(ul);
        mg->mg_ptr = NULL;
    }
    return (0);
}

static const MGVTBL sharedsv_userlock_vtbl = {
    0,                          /* get */
    0,                          /* set */
    0,                          /* len */
    0,                          /* clear */
    sharedsv_userlock_free,     /* free */
    0,                          /* copy */
    0,                          /* dup */
#ifdef MGf_LOCAL
    0,                          /* local */
#endif
};


/* Support for dual-valued variables */
#ifdef SVf_IVisUV
#  define DUALVAR_FLAGS(sv)                             \
    ((SvPOK(sv))                                        \
        ? ((SvNOK(sv) || SvNOKp(sv)) ? SVf_NOK          \
        : ((SvIsUV(sv)) ? (SVf_IOK | SVf_IVisUV)        \
        : ((SvIOK(sv) || SvIOKp(sv)) ? SVf_IOK : 0)))   \
        : 0)
#else
#  define DUALVAR_FLAGS(sv)                             \
    ((SvPOK(sv))                                        \
        ? ((SvNOK(sv) || SvNOKp(sv)) ? SVf_NOK          \
        : ((SvIOK(sv) || SvIOKp(sv)) ? SVf_IOK : 0))    \
        : 0)
#endif


/*
 * Access to shared things is heavily based on MAGIC
 *      - in mg.h/mg.c/sv.c sense
 */

/* In any thread that has access to a shared thing there is a "proxy"
   for it in its own space which has 'MAGIC' associated which accesses
   the shared thing.
 */

extern const MGVTBL sharedsv_scalar_vtbl;   /* Scalars have this vtable */
extern const MGVTBL sharedsv_array_vtbl;     /* Hashes and arrays have this
                                            - like 'tie' */
extern const MGVTBL sharedsv_elem_vtbl;      /* Elements of hashes and arrays have
                                          this _AS WELL AS_ the scalar magic:
   The sharedsv_elem_vtbl associates the element with the array/hash and
   the sharedsv_scalar_vtbl associates it with the value
 */


/* Get shared aggregate SV pointed to by threads::shared::tie magic object */

#define SHAREDSV_FROM_OBJ(sv) ((SvROK(sv)) ? INT2PTR(SV *, SvIV(SvRV(sv))) : NULL)


/* Return the user_lock structure (if any) associated with a shared SV.
 * If create is true, create one if it doesn't exist
 */
STATIC user_lock *
S_get_userlock(pTHX_ SV* ssv, bool create)
{
    MAGIC *mg;
    user_lock *ul = NULL;

    assert(ssv);
    /* XXX Redesign the storage of user locks so we don't need a global
     * lock to access them ???? DAPM */
    ENTER_LOCK;

    /* Version of mg_find that also checks the private signature */
    for (mg = SvMAGIC(ssv); mg; mg = mg->mg_moremagic) {
        if ((mg->mg_type == PERL_MAGIC_ext) &&
            (mg->mg_private == UL_MAGIC_SIG))
        {
            break;
        }
    }

    if (mg) {
        ul = (user_lock*)(mg->mg_ptr);
    } else if (create) {
        dTHXc;
        SHARED_CONTEXT;
        ul = (user_lock *) PerlMemShared_malloc(sizeof(user_lock));
        Zero(ul, 1, user_lock);
        /* Attach to shared SV using ext magic */
        mg = sv_magicext(ssv, NULL, PERL_MAGIC_ext, &sharedsv_userlock_vtbl,
                            (char *)ul, 0);
        mg->mg_private = UL_MAGIC_SIG;  /* Set private signature */
        recursive_lock_init(aTHX_ &ul->lock);
        COND_INIT(&ul->user_cond);
        CALLER_CONTEXT;
    }
    LEAVE_LOCK;
    return (ul);
}


/* Given a private side SV tries to find if the SV has a shared backend,
 * by looking for the magic.
 */
static SV *
Perl_sharedsv_find(pTHX_ SV *sv)
{
    MAGIC *mg;
    if (SvTYPE(sv) >= SVt_PVMG) {
        switch(SvTYPE(sv)) {
        case SVt_PVAV:
        case SVt_PVHV:
            if ((mg = mg_find(sv, PERL_MAGIC_tied))
                && mg->mg_virtual == &sharedsv_array_vtbl) {
                return ((SV *)mg->mg_ptr);
            }
            break;
        default:
            /* This should work for elements as well as they
             * have scalar magic as well as their element magic
             */
            if ((mg = mg_find(sv, PERL_MAGIC_shared_scalar))
                && mg->mg_virtual == &sharedsv_scalar_vtbl) {
                return ((SV *)mg->mg_ptr);
            }
            break;
        }
    }
    /* Just for tidyness of API also handle tie objects */
    if (SvROK(sv) && sv_derived_from(sv, "threads::shared::tie")) {
        return (SHAREDSV_FROM_OBJ(sv));
    }
    return (NULL);
}


/* Associate a private SV  with a shared SV by pointing the appropriate
 * magics at it.
 * Assumes lock is held.
 */
static void
Perl_sharedsv_associate(pTHX_ SV *sv, SV *ssv)
{
    MAGIC *mg = 0;

    /* If we are asked for any private ops we need a thread */
    assert ( aTHX !=  PL_sharedsv_space );

    /* To avoid need for recursive locks require caller to hold lock */
    assert ( PL_sharedsv_lock.owner == aTHX );

    switch(SvTYPE(sv)) {
    case SVt_PVAV:
    case SVt_PVHV:
        if (!(mg = mg_find(sv, PERL_MAGIC_tied))
            || mg->mg_virtual != &sharedsv_array_vtbl
            || (SV*) mg->mg_ptr != ssv)
        {
            SV *obj = newSV(0);
            sv_setref_iv(obj, "threads::shared::tie", PTR2IV(ssv));
            if (mg) {
                sv_unmagic(sv, PERL_MAGIC_tied);
            }
            mg = sv_magicext(sv, obj, PERL_MAGIC_tied, &sharedsv_array_vtbl,
                            (char *)ssv, 0);
            mg->mg_flags |= (MGf_COPY|MGf_DUP);
            SvREFCNT_inc_void(ssv);
            SvREFCNT_dec(obj);
        }
        break;

    default:
        if ((SvTYPE(sv) < SVt_PVMG)
            || !(mg = mg_find(sv, PERL_MAGIC_shared_scalar))
            || mg->mg_virtual != &sharedsv_scalar_vtbl
            || (SV*) mg->mg_ptr != ssv)
        {
            if (mg) {
                sv_unmagic(sv, PERL_MAGIC_shared_scalar);
            }
            mg = sv_magicext(sv, Nullsv, PERL_MAGIC_shared_scalar,
                            &sharedsv_scalar_vtbl, (char *)ssv, 0);
            mg->mg_flags |= (MGf_DUP
#ifdef MGf_LOCAL
                                    |MGf_LOCAL
#endif
                            );
            SvREFCNT_inc_void(ssv);
        }
        break;
    }

    assert ( Perl_sharedsv_find(aTHX_ sv) == ssv );
}


/* Given a private SV, create and return an associated shared SV.
 * Assumes lock is held.
 */
STATIC SV *
S_sharedsv_new_shared(pTHX_ SV *sv)
{
    dTHXc;
    SV *ssv;

    assert(PL_sharedsv_lock.owner == aTHX);
    assert(aTHX !=  PL_sharedsv_space);

    SHARED_CONTEXT;
    ssv = newSV(0);
    SvREFCNT(ssv) = 0; /* Will be upped to 1 by Perl_sharedsv_associate */
    sv_upgrade(ssv, SvTYPE(sv));
    CALLER_CONTEXT;
    Perl_sharedsv_associate(aTHX_ sv, ssv);
    return (ssv);
}


/* Given a shared SV, create and return an associated private SV.
 * Assumes lock is held.
 */
STATIC SV *
S_sharedsv_new_private(pTHX_ SV *ssv)
{
    SV *sv;

    assert(PL_sharedsv_lock.owner == aTHX);
    assert(aTHX !=  PL_sharedsv_space);

    sv = newSV(0);
    sv_upgrade(sv, SvTYPE(ssv));
    Perl_sharedsv_associate(aTHX_ sv, ssv);
    return (sv);
}


/* A threadsafe version of SvREFCNT_dec(ssv) */

STATIC void
S_sharedsv_dec(pTHX_ SV* ssv)
{
    if (! ssv)
        return;
    ENTER_LOCK;
    if (SvREFCNT(ssv) > 1) {
        /* No side effects, so can do it lightweight */
        SvREFCNT_dec(ssv);
    } else {
        dTHXc;
        SHARED_CONTEXT;
        SvREFCNT_dec(ssv);
        CALLER_CONTEXT;
    }
    LEAVE_LOCK;
}


/* Implements Perl-level share() and :shared */

static void
Perl_sharedsv_share(pTHX_ SV *sv)
{
    switch(SvTYPE(sv)) {
    case SVt_PVGV:
        Perl_croak(aTHX_ "Cannot share globs yet");
        break;

    case SVt_PVCV:
        Perl_croak(aTHX_ "Cannot share subs yet");
        break;

    default:
        ENTER_LOCK;
        (void) S_sharedsv_new_shared(aTHX_ sv);
        LEAVE_LOCK;
        SvSETMAGIC(sv);
        break;
    }
}


#ifdef WIN32
/* Number of milliseconds from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS      11644473600000.

/* Returns relative time in milliseconds.  (Adapted from Time::HiRes.) */
STATIC DWORD
S_abs_2_rel_milli(double abs)
{
    double rel;

    /* Get current time (in units of 100 nanoseconds since 1/1/1601) */
    union {
        FILETIME ft;
        __int64  i64;   /* 'signed' to keep compilers happy */
    } now;

    GetSystemTimeAsFileTime(&now.ft);

    /* Relative time in milliseconds */
    rel = (abs * 1000.) - (((double)now.i64 / 10000.) - EPOCH_BIAS);
    if (rel <= 0.0) {
        return (0);
    }
    return (DWORD)rel;
}

#else
# if defined(OS2)
#  define ABS2RELMILLI(abs)             \
    do {                                \
        abs -= (double)time(NULL);      \
        if (abs > 0) { abs *= 1000; }   \
        else         { abs  = 0;    }   \
    } while (0)
# endif /* OS2 */
#endif /* WIN32 */

/* Do OS-specific condition timed wait */

static bool
Perl_sharedsv_cond_timedwait(perl_cond *cond, perl_mutex *mut, double abs)
{
#if defined(NETWARE) || defined(I_MACH_CTHREADS)
    Perl_croak_nocontext("cond_timedwait not supported on this platform");
#else
#  ifdef WIN32
    int got_it = 0;

    cond->waiters++;
    MUTEX_UNLOCK(mut);
    /* See comments in win32/win32thread.h COND_WAIT vis-a-vis race */
    switch (WaitForSingleObject(cond->sem, S_abs_2_rel_milli(abs))) {
        case WAIT_OBJECT_0:   got_it = 1; break;
        case WAIT_TIMEOUT:                break;
        default:
            /* WAIT_FAILED? WAIT_ABANDONED? others? */
            Perl_croak_nocontext("panic: cond_timedwait (%ld)",GetLastError());
            break;
    }
    MUTEX_LOCK(mut);
    cond->waiters--;
    return (got_it);
#  else
#    ifdef OS2
    int rc, got_it = 0;
    STRLEN n_a;

    ABS2RELMILLI(abs);

    if ((rc = DosResetEventSem(*cond,&n_a)) && (rc != ERROR_ALREADY_RESET))
        Perl_rc = rc, croak_with_os2error("panic: cond_timedwait-reset");
    MUTEX_UNLOCK(mut);
    if (CheckOSError(DosWaitEventSem(*cond,abs))
        && (rc != ERROR_INTERRUPT))
        croak_with_os2error("panic: cond_timedwait");
    if (rc == ERROR_INTERRUPT) errno = EINTR;
    MUTEX_LOCK(mut);
    return (got_it);
#    else         /* Hope you're I_PTHREAD! */
    struct timespec ts;
    int got_it = 0;

    ts.tv_sec = (long)abs;
    abs -= (NV)ts.tv_sec;
    ts.tv_nsec = (long)(abs * 1000000000.0);

    CLANG_DIAG_IGNORE(-Wthread-safety)
    /* warning: calling function 'pthread_cond_timedwait' requires holding mutex 'mut' exclusively [-Wthread-safety-analysis] */
    switch (pthread_cond_timedwait(cond, mut, &ts)) {
	CLANG_DIAG_RESTORE

        case 0:         got_it = 1; break;
        case ETIMEDOUT:             break;
#ifdef OEMVS
        case -1:
            if (errno == ETIMEDOUT || errno == EAGAIN)
                break;
#endif
        default:
            Perl_croak_nocontext("panic: cond_timedwait");
            break;
    }
    return (got_it);
#    endif /* OS2 */
#  endif /* WIN32 */
#endif /* NETWARE || I_MACH_CTHREADS */
}


/* Given a thingy referenced by a shared RV, copy it's value to a private
 * RV, also copying the object status of the referent.
 * If the private side is already an appropriate RV->SV combination, keep
 * it if possible.
 */
STATIC void
S_get_RV(pTHX_ SV *sv, SV *sobj) {
    SV *obj;
    if (! (SvROK(sv) &&
           ((obj = SvRV(sv))) &&
           (Perl_sharedsv_find(aTHX_ obj) == sobj) &&
           (SvTYPE(obj) == SvTYPE(sobj))))
    {
        /* Can't reuse obj */
        if (SvROK(sv)) {
            SvREFCNT_dec(SvRV(sv));
        } else {
            assert(SvTYPE(sv) >= SVt_RV);
            sv_setsv_nomg(sv, &PL_sv_undef);
            SvROK_on(sv);
        }
        obj = S_sharedsv_new_private(aTHX_ sobj);
        SvRV_set(sv, obj);
    }

    if (SvOBJECT(obj)) {
        /* Remove any old blessing */
        SvREFCNT_dec(SvSTASH(obj));
        SvOBJECT_off(obj);
    }
    if (SvOBJECT(sobj)) {
        /* Add any new old blessing */
        STRLEN len;
        char* stash_ptr = SvPV((SV*) SvSTASH(sobj), len);
        HV* stash = gv_stashpvn(stash_ptr, len, TRUE);
        SvOBJECT_on(obj);
        SvSTASH_set(obj, (HV*)SvREFCNT_inc(stash));
    }
}

/* Every caller of S_get_RV needs this incantation (which cannot go inside
   S_get_RV itself, as we do not want recursion beyond one level): */
#define get_RV(sv, sobj)                     \
        S_get_RV(aTHX_ sv, sobj);             \
        /* Look ahead for refs of refs */      \
        if (SvROK(sobj)) {                      \
            SvROK_on(SvRV(sv));                  \
            S_get_RV(aTHX_ SvRV(sv), SvRV(sobj)); \
        }


/* ------------ PERL_MAGIC_shared_scalar(n) functions -------------- */

/* Get magic for PERL_MAGIC_shared_scalar(n) */

static int
sharedsv_scalar_mg_get(pTHX_ SV *sv, MAGIC *mg)
{
    SV *ssv = (SV *) mg->mg_ptr;
    assert(ssv);

    ENTER_LOCK;
    if (SvROK(ssv)) {
        get_RV(sv, SvRV(ssv));
    } else {
        sv_setsv_nomg(sv, ssv);
    }
    LEAVE_LOCK;
    return (0);
}

/* Copy the contents of a private SV to a shared SV.
 * Used by various mg_set()-type functions.
 * Assumes lock is held.
 */
static void
sharedsv_scalar_store(pTHX_ SV *sv, SV *ssv)
{
    dTHXc;
    bool allowed = TRUE;

    assert(PL_sharedsv_lock.owner == aTHX);
    if (!PL_dirty && SvROK(ssv) && SvREFCNT(SvRV(ssv)) == 1) {
        SV *sv = sv_newmortal();
        sv_upgrade(sv, SVt_RV);
        get_RV(sv, SvRV(ssv));
    }
    if (SvROK(sv)) {
        SV *obj = SvRV(sv);
        SV *sobj = Perl_sharedsv_find(aTHX_ obj);
        if (sobj) {
            SV* tmpref;
            SHARED_CONTEXT;
            /* Creating a tmp ref to sobj then assigning it to ssv ensures
             * that any previous contents of ssv are correctly freed
             * by sv_setsv(). Not sure if there is a better, API-legal way
             * to achieve this */
            tmpref = newRV_inc(sobj);
            sv_setsv_nomg(ssv, tmpref);
            SvREFCNT_dec_NN(tmpref);

            if (SvOBJECT(sobj)) {
                /* Remove any old blessing */
                SvREFCNT_dec(SvSTASH(sobj));
                SvOBJECT_off(sobj);
            }
            if (SvOBJECT(obj)) {
              SV* fake_stash = newSVpv(HvNAME_get(SvSTASH(obj)),0);
              SvOBJECT_on(sobj);
              SvSTASH_set(sobj, (HV*)fake_stash);
            }
            CALLER_CONTEXT;
        } else {
            allowed = FALSE;
        }
    } else {
        SvTEMP_off(sv);
        SHARED_CONTEXT;
        sv_setsv_nomg(ssv, sv);
        if (SvOBJECT(ssv)) {
            /* Remove any old blessing */
            SvREFCNT_dec(SvSTASH(ssv));
            SvOBJECT_off(ssv);
        }
        if (SvOBJECT(sv)) {
          SV* fake_stash = newSVpv(HvNAME_get(SvSTASH(sv)),0);
          SvOBJECT_on(ssv);
          SvSTASH_set(ssv, (HV*)fake_stash);
        }
        CALLER_CONTEXT;
    }
    if (!allowed) {
        Perl_croak(aTHX_ "Invalid value for shared scalar");
    }
}

/* Set magic for PERL_MAGIC_shared_scalar(n) */

static int
sharedsv_scalar_mg_set(pTHX_ SV *sv, MAGIC *mg)
{
    SV *ssv = (SV*)(mg->mg_ptr);
    assert(ssv);
    ENTER_LOCK;
    if (SvTYPE(ssv) < SvTYPE(sv)) {
        dTHXc;
        SHARED_CONTEXT;
        sv_upgrade(ssv, SvTYPE(sv));
        CALLER_CONTEXT;
    }
    sharedsv_scalar_store(aTHX_ sv, ssv);
    LEAVE_LOCK;
    return (0);
}

/* Free magic for PERL_MAGIC_shared_scalar(n) */

static int
sharedsv_scalar_mg_free(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_UNUSED_ARG(sv);
    ENTER_LOCK;
    if (!PL_dirty
     && SvROK((SV *)mg->mg_ptr) && SvREFCNT(SvRV((SV *)mg->mg_ptr)) == 1) {
        SV *sv = sv_newmortal();
        sv_upgrade(sv, SVt_RV);
        get_RV(sv, SvRV((SV *)mg->mg_ptr));
    }
    S_sharedsv_dec(aTHX_ (SV*)mg->mg_ptr);
    LEAVE_LOCK;
    return (0);
}

/*
 * Called during cloning of PERL_MAGIC_shared_scalar(n) magic in new thread
 */
static int
sharedsv_scalar_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *param)
{
    PERL_UNUSED_ARG(param);
    SvREFCNT_inc_void(mg->mg_ptr);
    return (0);
}

#ifdef MGf_LOCAL
/*
 * Called during local $shared
 */
static int
sharedsv_scalar_mg_local(pTHX_ SV* nsv, MAGIC *mg)
{
    MAGIC *nmg;
    SV *ssv = (SV *) mg->mg_ptr;
    if (ssv) {
        ENTER_LOCK;
        SvREFCNT_inc_void(ssv);
        LEAVE_LOCK;
    }
    nmg = sv_magicext(nsv, mg->mg_obj, mg->mg_type, mg->mg_virtual,
                           mg->mg_ptr, mg->mg_len);
    nmg->mg_flags   = mg->mg_flags;
    nmg->mg_private = mg->mg_private;

    return (0);
}
#endif

const MGVTBL sharedsv_scalar_vtbl = {
    sharedsv_scalar_mg_get,     /* get */
    sharedsv_scalar_mg_set,     /* set */
    0,                          /* len */
    0,                          /* clear */
    sharedsv_scalar_mg_free,    /* free */
    0,                          /* copy */
    sharedsv_scalar_mg_dup,     /* dup */
#ifdef MGf_LOCAL
    sharedsv_scalar_mg_local,   /* local */
#endif
};

/* ------------ PERL_MAGIC_tiedelem(p) functions -------------- */

/* Get magic for PERL_MAGIC_tiedelem(p) */

static int
sharedsv_elem_mg_FETCH(pTHX_ SV *sv, MAGIC *mg)
{
    dTHXc;
    SV *saggregate = SHAREDSV_FROM_OBJ(mg->mg_obj);
    SV** svp = NULL;

    ENTER_LOCK;
    if (saggregate) {  /* During global destruction, underlying
                          aggregate may no longer exist */
        if (SvTYPE(saggregate) == SVt_PVAV) {
            assert ( mg->mg_ptr == 0 );
            SHARED_CONTEXT;
            svp = av_fetch((AV*) saggregate, mg->mg_len, 0);
        } else {
            char *key = mg->mg_ptr;
            I32 len = mg->mg_len;
            assert ( mg->mg_ptr != 0 );
            if (mg->mg_len == HEf_SVKEY) {
                STRLEN slen;
                key = SvPV((SV *)mg->mg_ptr, slen);
                len = slen;
                if (SvUTF8((SV *)mg->mg_ptr)) {
                    len = -len;
                }
            }
            SHARED_CONTEXT;
            svp = hv_fetch((HV*) saggregate, key, len, 0);
        }
        CALLER_CONTEXT;
    }
    if (svp) {
        /* Exists in the array */
        if (SvROK(*svp)) {
            get_RV(sv, SvRV(*svp));
        } else {
            /* $ary->[elem] or $ary->{elem} is a scalar */
            Perl_sharedsv_associate(aTHX_ sv, *svp);
            sv_setsv(sv, *svp);
        }
    } else {
        /* Not in the array */
        sv_setsv(sv, &PL_sv_undef);
    }
    LEAVE_LOCK;
    return (0);
}

/* Set magic for PERL_MAGIC_tiedelem(p) */

static int
sharedsv_elem_mg_STORE(pTHX_ SV *sv, MAGIC *mg)
{
    dTHXc;
    SV *saggregate = SHAREDSV_FROM_OBJ(mg->mg_obj);
    SV **svp;
    U32 dualvar_flags = DUALVAR_FLAGS(sv);

    /* Theory - SV itself is magically shared - and we have ordered the
       magic such that by the time we get here it has been stored
       to its shared counterpart
     */
    ENTER_LOCK;
    assert(saggregate);
    if (SvTYPE(saggregate) == SVt_PVAV) {
        assert ( mg->mg_ptr == 0 );
        SHARED_CONTEXT;
        svp = av_fetch((AV*) saggregate, mg->mg_len, 1);
    } else {
        char *key = mg->mg_ptr;
        I32 len = mg->mg_len;
        assert ( mg->mg_ptr != 0 );
        if (mg->mg_len == HEf_SVKEY) {
            STRLEN slen;
            key = SvPV((SV *)mg->mg_ptr, slen);
            len = slen;
            if (SvUTF8((SV *)mg->mg_ptr)) {
                len = -len;
            }
        }
        SHARED_CONTEXT;
        svp = hv_fetch((HV*) saggregate, key, len, 1);
    }
    CALLER_CONTEXT;
    Perl_sharedsv_associate(aTHX_ sv, *svp);
    sharedsv_scalar_store(aTHX_ sv, *svp);
    SvFLAGS(*svp) |= dualvar_flags;
    LEAVE_LOCK;
    return (0);
}

/* Clear magic for PERL_MAGIC_tiedelem(p) */

static int
sharedsv_elem_mg_DELETE(pTHX_ SV *sv, MAGIC *mg)
{
    dTHXc;
    MAGIC *shmg;
    SV *saggregate = SHAREDSV_FROM_OBJ(mg->mg_obj);

    /* Object may not exist during global destruction */
    if (! saggregate) {
        return (0);
    }

    ENTER_LOCK;
    sharedsv_elem_mg_FETCH(aTHX_ sv, mg);
    if ((shmg = mg_find(sv, PERL_MAGIC_shared_scalar)))
        sharedsv_scalar_mg_get(aTHX_ sv, shmg);
    if (SvTYPE(saggregate) == SVt_PVAV) {
        SHARED_CONTEXT;
        av_delete((AV*) saggregate, mg->mg_len, G_DISCARD);
    } else {
        char *key = mg->mg_ptr;
        I32 len = mg->mg_len;
        assert ( mg->mg_ptr != 0 );
        if (mg->mg_len == HEf_SVKEY) {
            STRLEN slen;
            key = SvPV((SV *)mg->mg_ptr, slen);
            len = slen;
            if (SvUTF8((SV *)mg->mg_ptr)) {
                len = -len;
            }
        }
        SHARED_CONTEXT;
        (void) hv_delete((HV*) saggregate, key, len, G_DISCARD);
    }
    CALLER_CONTEXT;
    LEAVE_LOCK;
    return (0);
}

/* Called during cloning of PERL_MAGIC_tiedelem(p) magic in new
 * thread */

static int
sharedsv_elem_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *param)
{
    PERL_UNUSED_ARG(param);
    SvREFCNT_inc_void(SHAREDSV_FROM_OBJ(mg->mg_obj));
    assert(mg->mg_flags & MGf_DUP);
    return (0);
}

const MGVTBL sharedsv_elem_vtbl = {
    sharedsv_elem_mg_FETCH,     /* get */
    sharedsv_elem_mg_STORE,     /* set */
    0,                          /* len */
    sharedsv_elem_mg_DELETE,    /* clear */
    0,                          /* free */
    0,                          /* copy */
    sharedsv_elem_mg_dup,       /* dup */
#ifdef MGf_LOCAL
    0,                          /* local */
#endif
};

/* ------------ PERL_MAGIC_tied(P) functions -------------- */

/* Len magic for PERL_MAGIC_tied(P) */

static U32
sharedsv_array_mg_FETCHSIZE(pTHX_ SV *sv, MAGIC *mg)
{
    dTHXc;
    SV *ssv = (SV *) mg->mg_ptr;
    U32 val;
    PERL_UNUSED_ARG(sv);
    SHARED_EDIT;
    if (SvTYPE(ssv) == SVt_PVAV) {
        val = av_len((AV*) ssv);
    } else {
        /* Not actually defined by tie API but ... */
        val = HvUSEDKEYS((HV*) ssv);
    }
    SHARED_RELEASE;
    return (val);
}

/* Clear magic for PERL_MAGIC_tied(P) */

static int
sharedsv_array_mg_CLEAR(pTHX_ SV *sv, MAGIC *mg)
{
    dTHXc;
    SV *ssv = (SV *) mg->mg_ptr;
    const bool isav = SvTYPE(ssv) == SVt_PVAV;
    PERL_UNUSED_ARG(sv);
    SHARED_EDIT;
    if (!PL_dirty) {
            SV **svp = isav ? AvARRAY((AV *)ssv) : NULL;
            I32 items = isav ? AvFILLp((AV *)ssv) + 1 : 0;
            HE *iter;
            if (!isav) hv_iterinit((HV *)ssv);
            while (isav ? items-- : cBOOL(iter = hv_iternext((HV *)ssv))) {
                SV *sv = isav ? *svp++ : HeVAL(iter);
                if (!sv) continue;
                if ( (SvOBJECT(sv) || (SvROK(sv) && (sv = SvRV(sv))))
                  && SvREFCNT(sv) == 1 ) {
                    SV *tmp;
                    PERL_SET_CONTEXT((aTHX = caller_perl));
                    tmp = sv_newmortal();
                    sv_upgrade(tmp, SVt_RV);
                    get_RV(tmp, sv);
                    PERL_SET_CONTEXT((aTHX = PL_sharedsv_space));
                }
            }
    }
    if (isav) av_clear((AV*) ssv);
    else      hv_clear((HV*) ssv);
    SHARED_RELEASE;
    return (0);
}

/* Free magic for PERL_MAGIC_tied(P) */

static int
sharedsv_array_mg_free(pTHX_ SV *sv, MAGIC *mg)
{
    PERL_UNUSED_ARG(sv);
    S_sharedsv_dec(aTHX_ (SV*)mg->mg_ptr);
    return (0);
}

/*
 * Copy magic for PERL_MAGIC_tied(P)
 * This is called when perl is about to access an element of
 * the array -
 */
#if PERL_VERSION_GE(5,11,0)
static int
sharedsv_array_mg_copy(pTHX_ SV *sv, MAGIC* mg,
                       SV *nsv, const char *name, I32 namlen)
#else
static int
sharedsv_array_mg_copy(pTHX_ SV *sv, MAGIC* mg,
                       SV *nsv, const char *name, int namlen)
#endif
{
    MAGIC *nmg = sv_magicext(nsv,mg->mg_obj,
                            toLOWER(mg->mg_type),&sharedsv_elem_vtbl,
                            name, namlen);
    PERL_UNUSED_ARG(sv);
    nmg->mg_flags |= MGf_DUP;
    return (1);
}

/* Called during cloning of PERL_MAGIC_tied(P) magic in new thread */

static int
sharedsv_array_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *param)
{
    PERL_UNUSED_ARG(param);
    SvREFCNT_inc_void((SV*)mg->mg_ptr);
    assert(mg->mg_flags & MGf_DUP);
    return (0);
}

const MGVTBL sharedsv_array_vtbl = {
    0,                          /* get */
    0,                          /* set */
    sharedsv_array_mg_FETCHSIZE,/* len */
    sharedsv_array_mg_CLEAR,    /* clear */
    sharedsv_array_mg_free,     /* free */
    sharedsv_array_mg_copy,     /* copy */
    sharedsv_array_mg_dup,      /* dup */
#ifdef MGf_LOCAL
    0,                          /* local */
#endif
};


/* Recursive locks on a sharedsv.
 * Locks are dynamically scoped at the level of the first lock.
 */
static void
Perl_sharedsv_lock(pTHX_ SV *ssv)
{
    user_lock *ul;
    if (! ssv)
        return;
    ul = S_get_userlock(aTHX_ ssv, 1);
    recursive_lock_acquire(aTHX_ &ul->lock, __FILE__, __LINE__);
}

/* Handles calls from lock() builtin via PL_lockhook */

static void
Perl_sharedsv_locksv(pTHX_ SV *sv)
{
    SV *ssv;

    if (SvROK(sv))
        sv = SvRV(sv);
    ssv = Perl_sharedsv_find(aTHX_ sv);
    if (!ssv)
        croak("lock can only be used on shared values");
    Perl_sharedsv_lock(aTHX_ ssv);
}


/* Can a shared object be destroyed?
 * True if not a shared,
 * or if destroying last proxy on a shared object
 */
#ifdef PL_destroyhook
static bool
Perl_shared_object_destroy(pTHX_ SV *sv)
{
    SV *ssv;

    if (SvROK(sv))
        sv = SvRV(sv);
    ssv = Perl_sharedsv_find(aTHX_ sv);
    return (!ssv || (SvREFCNT(ssv) <= 1));
}
#endif

/* veto signal dispatch if we have the lock */

#ifdef PL_signalhook

STATIC despatch_signals_proc_t prev_signal_hook = NULL;

STATIC void
S_shared_signal_hook(pTHX) {
    int us;
    MUTEX_LOCK(&PL_sharedsv_lock.mutex);
    us = (PL_sharedsv_lock.owner == aTHX);
    MUTEX_UNLOCK(&PL_sharedsv_lock.mutex);
    if (us)
        return; /* try again later */
    prev_signal_hook(aTHX);
}
#endif

/* Saves a space for keeping SVs wider than an interpreter. */

static void
Perl_sharedsv_init(pTHX)
{
    dTHXc;
    if (!PL_sharedsv_space) {
        PL_sharedsv_space = perl_alloc();
        perl_construct(PL_sharedsv_space);
        /* The pair above leaves us in shared context (what dTHX would get),
         * but aTHX still points to caller context */
        aTHX = PL_sharedsv_space;
        LEAVE; /* This balances the ENTER at the end of perl_construct.  */
        PERL_SET_CONTEXT((aTHX = caller_perl));
        recursive_lock_init(aTHX_ &PL_sharedsv_lock);
    }
    PL_lockhook = &Perl_sharedsv_locksv;
    PL_sharehook = &Perl_sharedsv_share;
#ifdef PL_destroyhook
    PL_destroyhook = &Perl_shared_object_destroy;
#endif
#ifdef PL_signalhook
    if (!prev_signal_hook) {
        prev_signal_hook = PL_signalhook;
        PL_signalhook = &S_shared_signal_hook;
    }
#endif
}

#endif /* USE_ITHREADS */

MODULE = threads::shared        PACKAGE = threads::shared::tie

PROTOTYPES: DISABLE

#ifdef USE_ITHREADS

void
PUSH(SV *obj, ...)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        int ii;
        for (ii = 1; ii < items; ii++) {
            SV* tmp = newSVsv(ST(ii));
            SV *stmp;
            U32 dualvar_flags = DUALVAR_FLAGS(tmp);
            ENTER_LOCK;
            stmp = S_sharedsv_new_shared(aTHX_ tmp);
            sharedsv_scalar_store(aTHX_ tmp, stmp);
            SvFLAGS(stmp) |= dualvar_flags;
            SHARED_CONTEXT;
            av_push((AV*) sobj, stmp);
            SvREFCNT_inc_void(stmp);
            SHARED_RELEASE;
            SvREFCNT_dec(tmp);
        }


void
UNSHIFT(SV *obj, ...)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        int ii;
        ENTER_LOCK;
        SHARED_CONTEXT;
        av_unshift((AV*)sobj, items - 1);
        CALLER_CONTEXT;
        for (ii = 1; ii < items; ii++) {
            SV *tmp = newSVsv(ST(ii));
            U32 dualvar_flags = DUALVAR_FLAGS(tmp);
            SV *stmp = S_sharedsv_new_shared(aTHX_ tmp);
            sharedsv_scalar_store(aTHX_ tmp, stmp);
            SHARED_CONTEXT;
            SvFLAGS(stmp) |= dualvar_flags;
            av_store((AV*) sobj, ii - 1, stmp);
            SvREFCNT_inc_void(stmp);
            CALLER_CONTEXT;
            SvREFCNT_dec(tmp);
        }
        LEAVE_LOCK;


void
POP(SV *obj)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        SV* ssv;
        ENTER_LOCK;
        SHARED_CONTEXT;
        ssv = av_pop((AV*)sobj);
        CALLER_CONTEXT;
        ST(0) = sv_newmortal();
        Perl_sharedsv_associate(aTHX_ ST(0), ssv);
        SvREFCNT_dec(ssv);
        LEAVE_LOCK;
        /* XSRETURN(1); - implied */


void
SHIFT(SV *obj)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        SV* ssv;
        ENTER_LOCK;
        SHARED_CONTEXT;
        ssv = av_shift((AV*)sobj);
        CALLER_CONTEXT;
        ST(0) = sv_newmortal();
        Perl_sharedsv_associate(aTHX_ ST(0), ssv);
        SvREFCNT_dec(ssv);
        LEAVE_LOCK;
        /* XSRETURN(1); - implied */


void
EXTEND(SV *obj, IV count)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        SHARED_EDIT;
        av_extend((AV*)sobj, count);
        SHARED_RELEASE;


void
STORESIZE(SV *obj,IV count)
    CODE:
        dTHXc;
        SV *ssv = SHAREDSV_FROM_OBJ(obj);

        SHARED_EDIT;
        assert(SvTYPE(ssv) == SVt_PVAV);
        if (!PL_dirty) {
            SV **svp = AvARRAY((AV *)ssv);
            I32 ix = AvFILLp((AV *)ssv);
            for (;ix >= count; ix--) {
                SV *sv = svp[ix];
                if (!sv)
                    continue;
                if (   (SvOBJECT(sv) || (SvROK(sv) && (sv = SvRV(sv))))
                    && SvREFCNT(sv) == 1 )
                {
                    SV *tmp;
                    PERL_SET_CONTEXT((aTHX = caller_perl));
                    tmp = sv_newmortal();
                    sv_upgrade(tmp, SVt_RV);
                    get_RV(tmp, sv);
                    PERL_SET_CONTEXT((aTHX = PL_sharedsv_space));
                }
            }
        }
        av_fill((AV*) ssv, count - 1);
        SHARED_RELEASE;


void
EXISTS(SV *obj, SV *index)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        bool exists;
        if (SvTYPE(sobj) == SVt_PVAV) {
            SHARED_EDIT;
            exists = av_exists((AV*) sobj, SvIV(index));
        } else {
            I32 len;
            STRLEN slen;
            char *key = SvPVutf8(index, slen);
            len = slen;
            if (SvUTF8(index)) {
                len = -len;
            }
            SHARED_EDIT;
            exists = hv_exists((HV*) sobj, key, len);
        }
        SHARED_RELEASE;
        ST(0) = (exists) ? &PL_sv_yes : &PL_sv_no;
        /* XSRETURN(1); - implied */


void
FIRSTKEY(SV *obj)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        char* key = NULL;
        I32 len = 0;
        HE* entry;
        ENTER_LOCK;
        SHARED_CONTEXT;
        hv_iterinit((HV*) sobj);
        entry = hv_iternext((HV*) sobj);
        if (entry) {
            I32 utf8 = HeKUTF8(entry);
            key = hv_iterkey(entry,&len);
            CALLER_CONTEXT;
            ST(0) = newSVpvn_flags(key, len, SVs_TEMP | (utf8 ? SVf_UTF8 : 0));
        } else {
            CALLER_CONTEXT;
            ST(0) = &PL_sv_undef;
        }
        LEAVE_LOCK;
        /* XSRETURN(1); - implied */


void
NEXTKEY(SV *obj, SV *oldkey)
    CODE:
        dTHXc;
        SV *sobj = SHAREDSV_FROM_OBJ(obj);
        char* key = NULL;
        I32 len = 0;
        HE* entry;

        PERL_UNUSED_VAR(oldkey);

        ENTER_LOCK;
        SHARED_CONTEXT;
        entry = hv_iternext((HV*) sobj);
        if (entry) {
            I32 utf8 = HeKUTF8(entry);
            key = hv_iterkey(entry,&len);
            CALLER_CONTEXT;
            ST(0) = newSVpvn_flags(key, len, SVs_TEMP | (utf8 ? SVf_UTF8 : 0));
        } else {
            CALLER_CONTEXT;
            ST(0) = &PL_sv_undef;
        }
        LEAVE_LOCK;
        /* XSRETURN(1); - implied */


MODULE = threads::shared        PACKAGE = threads::shared

PROTOTYPES: ENABLE

void
_id(SV *myref)
    PROTOTYPE: \[$@%]
    PREINIT:
        SV *ssv;
    CODE:
        myref = SvRV(myref);
        if (SvMAGICAL(myref))
            mg_get(myref);
        if (SvROK(myref))
            myref = SvRV(myref);
        ssv = Perl_sharedsv_find(aTHX_ myref);
        if (! ssv)
            XSRETURN_UNDEF;
        ST(0) = sv_2mortal(newSVuv(PTR2UV(ssv)));
        /* XSRETURN(1); - implied */


void
_refcnt(SV *myref)
    PROTOTYPE: \[$@%]
    PREINIT:
        SV *ssv;
    CODE:
        myref = SvRV(myref);
        if (SvROK(myref))
            myref = SvRV(myref);
        ssv = Perl_sharedsv_find(aTHX_ myref);
        if (! ssv) {
            if (ckWARN(WARN_THREADS)) {
                Perl_warner(aTHX_ packWARN(WARN_THREADS),
                                "%" SVf " is not shared", ST(0));
            }
            XSRETURN_UNDEF;
        }
        ST(0) = sv_2mortal(newSViv(SvREFCNT(ssv)));
        /* XSRETURN(1); - implied */


void
share(SV *myref)
    PROTOTYPE: \[$@%]
    CODE:
        if (! SvROK(myref))
            Perl_croak(aTHX_ "Argument to share needs to be passed as ref");
        myref = SvRV(myref);
        if (SvROK(myref))
            myref = SvRV(myref);
        Perl_sharedsv_share(aTHX_ myref);
        ST(0) = sv_2mortal(newRV_inc(myref));
        /* XSRETURN(1); - implied */


void
cond_wait(SV *ref_cond, SV *ref_lock = 0)
    PROTOTYPE: \[$@%];\[$@%]
    PREINIT:
        SV *ssv;
        perl_cond* user_condition;
        int locks;
        user_lock *ul;
    CODE:
        if (!SvROK(ref_cond))
            Perl_croak(aTHX_ "Argument to cond_wait needs to be passed as ref");
        ref_cond = SvRV(ref_cond);
        if (SvROK(ref_cond))
            ref_cond = SvRV(ref_cond);
        ssv = Perl_sharedsv_find(aTHX_ ref_cond);
        if (! ssv)
            Perl_croak(aTHX_ "cond_wait can only be used on shared values");
        ul = S_get_userlock(aTHX_ ssv, 1);

        user_condition = &ul->user_cond;
        if (ref_lock && (ref_cond != ref_lock)) {
            if (!SvROK(ref_lock))
                Perl_croak(aTHX_ "cond_wait lock needs to be passed as ref");
            ref_lock = SvRV(ref_lock);
            if (SvROK(ref_lock)) ref_lock = SvRV(ref_lock);
            ssv = Perl_sharedsv_find(aTHX_ ref_lock);
            if (! ssv)
                Perl_croak(aTHX_ "cond_wait lock must be a shared value");
            ul = S_get_userlock(aTHX_ ssv, 1);
        }
        if (ul->lock.owner != aTHX)
            croak("You need a lock before you can cond_wait");

        /* Stealing the members of the lock object worries me - NI-S */
        MUTEX_LOCK(&ul->lock.mutex);
        ul->lock.owner = NULL;
        locks = ul->lock.locks;
        ul->lock.locks = 0;

        /* Since we are releasing the lock here, we need to tell other
         * people that it is ok to go ahead and use it */
        COND_SIGNAL(&ul->lock.cond);
        COND_WAIT(user_condition, &ul->lock.mutex);
        while (ul->lock.owner != NULL) {
            /* OK -- must reacquire the lock */
            COND_WAIT(&ul->lock.cond, &ul->lock.mutex);
        }
        ul->lock.owner = aTHX;
        ul->lock.locks = locks;
        MUTEX_UNLOCK(&ul->lock.mutex);


int
cond_timedwait(SV *ref_cond, double abs, SV *ref_lock = 0)
    PROTOTYPE: \[$@%]$;\[$@%]
    PREINIT:
        SV *ssv;
        perl_cond* user_condition;
        int locks;
        user_lock *ul;
    CODE:
        if (! SvROK(ref_cond))
            Perl_croak(aTHX_ "Argument to cond_timedwait needs to be passed as ref");
        ref_cond = SvRV(ref_cond);
        if (SvROK(ref_cond))
            ref_cond = SvRV(ref_cond);
        ssv = Perl_sharedsv_find(aTHX_ ref_cond);
        if (! ssv)
            Perl_croak(aTHX_ "cond_timedwait can only be used on shared values");
        ul = S_get_userlock(aTHX_ ssv, 1);

        user_condition = &ul->user_cond;
        if (ref_lock && (ref_cond != ref_lock)) {
            if (! SvROK(ref_lock))
                Perl_croak(aTHX_ "cond_timedwait lock needs to be passed as ref");
            ref_lock = SvRV(ref_lock);
            if (SvROK(ref_lock)) ref_lock = SvRV(ref_lock);
            ssv = Perl_sharedsv_find(aTHX_ ref_lock);
            if (! ssv)
                Perl_croak(aTHX_ "cond_timedwait lock must be a shared value");
            ul = S_get_userlock(aTHX_ ssv, 1);
        }
        if (ul->lock.owner != aTHX)
            Perl_croak(aTHX_ "You need a lock before you can cond_wait");

        MUTEX_LOCK(&ul->lock.mutex);
        ul->lock.owner = NULL;
        locks = ul->lock.locks;
        ul->lock.locks = 0;
        /* Since we are releasing the lock here, we need to tell other
         * people that it is ok to go ahead and use it */
        COND_SIGNAL(&ul->lock.cond);
        RETVAL = Perl_sharedsv_cond_timedwait(user_condition, &ul->lock.mutex, abs);
        while (ul->lock.owner != NULL) {
            /* OK -- must reacquire the lock... */
            COND_WAIT(&ul->lock.cond, &ul->lock.mutex);
        }
        ul->lock.owner = aTHX;
        ul->lock.locks = locks;
        MUTEX_UNLOCK(&ul->lock.mutex);

        if (RETVAL == 0)
            XSRETURN_UNDEF;
    OUTPUT:
        RETVAL


void
cond_signal(SV *myref)
    PROTOTYPE: \[$@%]
    PREINIT:
        SV *ssv;
        user_lock *ul;
    CODE:
        if (! SvROK(myref))
            Perl_croak(aTHX_ "Argument to cond_signal needs to be passed as ref");
        myref = SvRV(myref);
        if (SvROK(myref))
            myref = SvRV(myref);
        ssv = Perl_sharedsv_find(aTHX_ myref);
        if (! ssv)
            Perl_croak(aTHX_ "cond_signal can only be used on shared values");
        ul = S_get_userlock(aTHX_ ssv, 1);
        if (ckWARN(WARN_THREADS) && ul->lock.owner != aTHX) {
            Perl_warner(aTHX_ packWARN(WARN_THREADS),
                            "cond_signal() called on unlocked variable");
        }
        COND_SIGNAL(&ul->user_cond);


void
cond_broadcast(SV *myref)
    PROTOTYPE: \[$@%]
    PREINIT:
        SV *ssv;
        user_lock *ul;
    CODE:
        if (! SvROK(myref))
            Perl_croak(aTHX_ "Argument to cond_broadcast needs to be passed as ref");
        myref = SvRV(myref);
        if (SvROK(myref))
            myref = SvRV(myref);
        ssv = Perl_sharedsv_find(aTHX_ myref);
        if (! ssv)
            Perl_croak(aTHX_ "cond_broadcast can only be used on shared values");
        ul = S_get_userlock(aTHX_ ssv, 1);
        if (ckWARN(WARN_THREADS) && ul->lock.owner != aTHX) {
            Perl_warner(aTHX_ packWARN(WARN_THREADS),
                            "cond_broadcast() called on unlocked variable");
        }
        COND_BROADCAST(&ul->user_cond);


void
bless(SV* myref, ...)
    PROTOTYPE: $;$
    PREINIT:
        HV* stash;
        SV *ssv;
    CODE:
        if (items == 1) {
            stash = CopSTASH(PL_curcop);
        } else {
            SV* classname = ST(1);
            STRLEN len;
            char *ptr;

            if (classname &&
                ! SvGMAGICAL(classname) &&
                ! SvAMAGIC(classname) &&
                SvROK(classname))
            {
                Perl_croak(aTHX_ "Attempt to bless into a reference");
            }
            ptr = SvPV(classname, len);
            if (ckWARN(WARN_MISC) && len == 0) {
                Perl_warner(aTHX_ packWARN(WARN_MISC),
                        "Explicit blessing to '' (assuming package main)");
            }
            stash = gv_stashpvn(ptr, len, TRUE);
        }
        SvREFCNT_inc_void(myref);
        (void)sv_bless(myref, stash);
        ST(0) = sv_2mortal(myref);
        ssv = Perl_sharedsv_find(aTHX_ myref);
        if (ssv) {
            dTHXc;
            ENTER_LOCK;
            SHARED_CONTEXT;
            {
                SV* fake_stash = newSVpv(HvNAME_get(stash), 0);
                (void)sv_bless(ssv, (HV*)fake_stash);
            }
            CALLER_CONTEXT;
            LEAVE_LOCK;
        }
        /* XSRETURN(1); - implied */

#endif /* USE_ITHREADS */

BOOT:
{
#ifdef USE_ITHREADS
     Perl_sharedsv_init(aTHX);
#endif /* USE_ITHREADS */
}
