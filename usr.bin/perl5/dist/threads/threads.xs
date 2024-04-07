#define PERL_NO_GET_CONTEXT
/* Workaround for mingw 32-bit compiler by mingw-w64.sf.net - has to come before any #include.
 * It also defines USE_NO_MINGW_SETJMP_TWO_ARGS for the mingw.org 32-bit compilers ... but
 * that's ok as that compiler makes no use of that symbol anyway */
#if defined(WIN32) && defined(__MINGW32__) && !defined(__MINGW64__)
#  define USE_NO_MINGW_SETJMP_TWO_ARGS 1
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
/* Workaround for XSUB.h bug under WIN32 */
#ifdef WIN32
#  undef setjmp
#  if defined(USE_NO_MINGW_SETJMP_TWO_ARGS) || (!defined(__BORLANDC__) && !defined(__MINGW64__))
#    define setjmp(x) _setjmp(x)
#  endif
#  if defined(__MINGW64__)
#    include <intrin.h>
#    define setjmp(x) _setjmpex((x), mingw_getsp())
#  endif
#endif
#define NEED_PL_signals
#define NEED_sv_2pv_flags
#include "ppport.h"
#include "threads.h"
#ifndef sv_dup_inc
#  define sv_dup_inc(s,t) SvREFCNT_inc(sv_dup(s,t))
#endif
#ifndef SvREFCNT_dec_NN
#  define SvREFCNT_dec_NN(x)  SvREFCNT_dec(x)
#endif
#ifndef PERL_UNUSED_RESULT
#  if defined(__GNUC__) && defined(HASATTRIBUTE_WARN_UNUSED_RESULT)
#    define PERL_UNUSED_RESULT(v) STMT_START { __typeof__(v) z = (v); (void)sizeof(z); } STMT_END
#  else
#    define PERL_UNUSED_RESULT(v) ((void)(v))
#  endif
#endif

#ifndef CLANG_DIAG_IGNORE
# define CLANG_DIAG_IGNORE(x)
# define CLANG_DIAG_RESTORE
#endif
#ifndef CLANG_DIAG_IGNORE_STMT
# define CLANG_DIAG_IGNORE_STMT(x) CLANG_DIAG_IGNORE(x) NOOP
# define CLANG_DIAG_RESTORE_STMT CLANG_DIAG_RESTORE NOOP
# define CLANG_DIAG_IGNORE_DECL(x) CLANG_DIAG_IGNORE(x) dNOOP
# define CLANG_DIAG_RESTORE_DECL CLANG_DIAG_RESTORE dNOOP
#endif

#ifdef USE_ITHREADS

#ifdef __amigaos4__
#  undef YIELD
#  define YIELD sleep(0)
#endif
#ifdef WIN32
#  include <windows.h>
   /* Supposed to be in Winbase.h */
#  ifndef STACK_SIZE_PARAM_IS_A_RESERVATION
#    define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000
#  endif
#  include <win32thread.h>
#else
#  ifdef OS2
typedef perl_os_thread pthread_t;
#  else
#    include <pthread.h>
#  endif
#  include <thread.h>
#  define PERL_THREAD_SETSPECIFIC(k,v) pthread_setspecific(k,v)
#  ifdef OLD_PTHREADS_API
#    define PERL_THREAD_DETACH(t) pthread_detach(&(t))
#  else
#    define PERL_THREAD_DETACH(t) pthread_detach((t))
#  endif
#endif
#if !defined(HAS_GETPAGESIZE) && defined(I_SYS_PARAM)
#  include <sys/param.h>
#endif

/* Values for 'state' member */
#define PERL_ITHR_DETACHED           1 /* Thread has been detached */
#define PERL_ITHR_JOINED             2 /* Thread is being / has been joined */
#define PERL_ITHR_FINISHED           4 /* Thread has finished execution */
#define PERL_ITHR_THREAD_EXIT_ONLY   8 /* exit() only exits current thread */
#define PERL_ITHR_NONVIABLE         16 /* Thread creation failed */
#define PERL_ITHR_DIED              32 /* Thread finished by dying */

#define PERL_ITHR_UNCALLABLE  (PERL_ITHR_DETACHED|PERL_ITHR_JOINED)


typedef struct _ithread {
    struct _ithread *next;      /* Next thread in the list */
    struct _ithread *prev;      /* Prev thread in the list */
    PerlInterpreter *interp;    /* The thread's interpreter */
    UV tid;                     /* Thread's module's thread id */
    perl_mutex mutex;           /* Mutex for updating things in this struct */
    int count;                  /* Reference count. See S_ithread_create. */
    int state;                  /* Detached, joined, finished, etc. */
    int gimme;                  /* Context of create */
    SV *init_function;          /* Code to run */
    AV *params;                 /* Args to pass function */
#ifdef WIN32
    DWORD  thr;                 /* OS's idea if thread id */
    HANDLE handle;              /* OS's waitable handle */
#else
    pthread_t thr;              /* OS's handle for the thread */
#endif
    IV stack_size;
    SV *err;                    /* Error from abnormally terminated thread */
    char *err_class;            /* Error object's classname if applicable */
#ifndef WIN32
    sigset_t initial_sigmask;   /* Thread wakes up with signals blocked */
#endif
} ithread;


#define MY_CXT_KEY "threads::_cxt" XS_VERSION

typedef struct {
    /* Used by Perl interpreter for thread context switching */
    ithread *context;
} my_cxt_t;

START_MY_CXT


#define MY_POOL_KEY "threads::_pool" XS_VERSION

typedef struct {
    /* Structure for 'main' thread
     * Also forms the 'base' for the doubly-linked list of threads */
    ithread main_thread;

    /* Protects the creation and destruction of threads*/
    perl_mutex create_destruct_mutex;

    UV tid_counter;
    IV joinable_threads;
    IV running_threads;
    IV detached_threads;
    IV total_threads;
    IV default_stack_size;
    IV page_size;
} my_pool_t;

#define dMY_POOL \
    SV *my_pool_sv = *hv_fetch(PL_modglobal, MY_POOL_KEY,               \
                               sizeof(MY_POOL_KEY)-1, TRUE);            \
    my_pool_t *my_poolp = INT2PTR(my_pool_t*, SvUV(my_pool_sv))

#define MY_POOL (*my_poolp)

#if defined(WIN32) || (defined(__amigaos4__) && defined(__NEWLIB__))
#  undef THREAD_SIGNAL_BLOCKING
#else
#  define THREAD_SIGNAL_BLOCKING
#endif

#ifdef THREAD_SIGNAL_BLOCKING

/* Block most signals for calling thread, setting the old signal mask to
 * oldmask, if it is not NULL */
STATIC int
S_block_most_signals(sigset_t *oldmask)
{
    sigset_t newmask;

    sigfillset(&newmask);
    /* Don't block certain "important" signals (stolen from mg.c) */
#ifdef SIGILL
    sigdelset(&newmask, SIGILL);
#endif
#ifdef SIGBUS
    sigdelset(&newmask, SIGBUS);
#endif
#ifdef SIGSEGV
    sigdelset(&newmask, SIGSEGV);
#endif

#if defined(VMS)
    /* no per-thread blocking available */
    return sigprocmask(SIG_BLOCK, &newmask, oldmask);
#else
    return pthread_sigmask(SIG_BLOCK, &newmask, oldmask);
#endif /* VMS */
}

/* Set the signal mask for this thread to newmask */
STATIC int
S_set_sigmask(sigset_t *newmask)
{
#if defined(VMS)
    return sigprocmask(SIG_SETMASK, newmask, NULL);
#else
    return pthread_sigmask(SIG_SETMASK, newmask, NULL);
#endif /* VMS */
}
#endif /* WIN32 */

/* Used by Perl interpreter for thread context switching */
STATIC void
S_ithread_set(pTHX_ ithread *thread)
{
    dMY_CXT;
    MY_CXT.context = thread;
#ifdef PERL_SET_NON_tTHX_CONTEXT
    PERL_SET_NON_tTHX_CONTEXT(thread->interp);
#endif
}

STATIC ithread *
S_ithread_get(pTHX)
{
    dMY_CXT;
    return (MY_CXT.context);
}


/* Free any data (such as the Perl interpreter) attached to an ithread
 * structure.  This is a bit like undef on SVs, where the SV isn't freed,
 * but the PVX is.  Must be called with thread->mutex already locked.  Also,
 * must be called with MY_POOL.create_destruct_mutex unlocked as destruction
 * of the interpreter can lead to recursive destruction calls that could
 * lead to a deadlock on that mutex.
 */
STATIC void
S_ithread_clear(pTHX_ ithread *thread)
{
    PerlInterpreter *interp;
#ifndef WIN32
    sigset_t origmask;
#endif

    assert(((thread->state & PERL_ITHR_FINISHED) &&
            (thread->state & PERL_ITHR_UNCALLABLE))
                ||
           (thread->state & PERL_ITHR_NONVIABLE));

#ifdef THREAD_SIGNAL_BLOCKING
    /* We temporarily set the interpreter context to the interpreter being
     * destroyed.  It's in no condition to handle signals while it's being
     * taken apart.
     */
    S_block_most_signals(&origmask);
#endif

#if PERL_VERSION_GE(5, 37, 5)
    int save_veto = PL_veto_switch_non_tTHX_context;
#endif

    interp = thread->interp;
    if (interp) {
        dTHXa(interp);

        /* We will pretend to be a thread that we are not by switching tTHX,
         * which doesn't work with things that don't rely on tTHX during
         * tear-down, as they will tend to rely on a mapping from the tTHX
         * structure, and that structure is being destroyed. */
#if PERL_VERSION_GE(5, 37, 5)
        PL_veto_switch_non_tTHX_context = true;
#endif

        PERL_SET_CONTEXT(interp);

        S_ithread_set(aTHX_ thread);

        SvREFCNT_dec(thread->params);
        thread->params = NULL;

        if (thread->err) {
            SvREFCNT_dec_NN(thread->err);
            thread->err = Nullsv;
        }

        perl_destruct(interp);
        perl_free(interp);
        thread->interp = NULL;
    }

    PERL_SET_CONTEXT(aTHX);
#if PERL_VERSION_GE(5, 37, 5)
    PL_veto_switch_non_tTHX_context = save_veto;
#endif

#ifdef THREAD_SIGNAL_BLOCKING
    S_set_sigmask(&origmask);
#endif
}


/* Decrement the refcount of an ithread, and if it reaches zero, free it.
 * Must be called with the mutex held.
 * On return, mutex is released (or destroyed).
 */
STATIC void
S_ithread_free(pTHX_ ithread *thread)
  PERL_TSA_RELEASE(thread->mutex)
{
#ifdef WIN32
    HANDLE handle;
#endif
    dMY_POOL;

    if (! (thread->state & PERL_ITHR_NONVIABLE)) {
        assert(thread->count > 0);
        if (--thread->count > 0) {
            MUTEX_UNLOCK(&thread->mutex);
            return;
        }
        assert((thread->state & PERL_ITHR_FINISHED) &&
               (thread->state & PERL_ITHR_UNCALLABLE));
    }
    MUTEX_UNLOCK(&thread->mutex);

    /* Main thread (0) is immortal and should never get here */
    assert(thread->tid != 0);

    /* Remove from circular list of threads */
    MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
    assert(thread->prev && thread->next);
    thread->next->prev = thread->prev;
    thread->prev->next = thread->next;
    thread->next = NULL;
    thread->prev = NULL;
    MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);

    /* Thread is now disowned */
    MUTEX_LOCK(&thread->mutex);
    S_ithread_clear(aTHX_ thread);

#ifdef WIN32
    handle = thread->handle;
    thread->handle = NULL;
#endif
    MUTEX_UNLOCK(&thread->mutex);
    MUTEX_DESTROY(&thread->mutex);

#ifdef WIN32
    if (handle) {
        CloseHandle(handle);
    }
#endif

    PerlMemShared_free(thread);

    /* total_threads >= 1 is used to veto cleanup by the main thread,
     * should it happen to exit while other threads still exist.
     * Decrement this as the very last thing in the thread's existence.
     * Otherwise, MY_POOL and global state such as PL_op_mutex may get
     * freed while we're still using it.
     */
    MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
    MY_POOL.total_threads--;
    MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);
}


static void
S_ithread_count_inc(pTHX_ ithread *thread)
  PERL_TSA_EXCLUDES(thread->mutex)
{
    MUTEX_LOCK(&thread->mutex);
    thread->count++;
    MUTEX_UNLOCK(&thread->mutex);
}


/* Warn if exiting with any unjoined threads */
STATIC int
S_exit_warning(pTHX)
{
    int veto_cleanup, warn;
    dMY_POOL;

    MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
    veto_cleanup = (MY_POOL.total_threads > 0);
    warn         = (MY_POOL.running_threads || MY_POOL.joinable_threads);
    MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);

    if (warn) {
        if (ckWARN_d(WARN_THREADS)) {
            Perl_warn(aTHX_ "Perl exited with active threads:\n\t%"
                            IVdf " running and unjoined\n\t%"
                            IVdf " finished and unjoined\n\t%"
                            IVdf " running and detached\n",
                            MY_POOL.running_threads,
                            MY_POOL.joinable_threads,
                            MY_POOL.detached_threads);
        }
    }

    return (veto_cleanup);
}


/* Called from perl_destruct() in each thread.  If it's the main thread,
 * stop it from freeing everything if there are other threads still running.
 */
STATIC int
Perl_ithread_hook(pTHX)
{
    dMY_POOL;
    return ((aTHX == MY_POOL.main_thread.interp) ? S_exit_warning(aTHX) : 0);
}


/* MAGIC (in mg.h sense) hooks */

STATIC int
ithread_mg_get(pTHX_ SV *sv, MAGIC *mg)
{
    ithread *thread = (ithread *)mg->mg_ptr;
    SvIV_set(sv, PTR2IV(thread));
    SvIOK_on(sv);
    return (0);
}

STATIC int
ithread_mg_free(pTHX_ SV *sv, MAGIC *mg)
{
    ithread *thread = (ithread *)mg->mg_ptr;
    PERL_UNUSED_ARG(sv);
    MUTEX_LOCK(&thread->mutex);
    S_ithread_free(aTHX_ thread);   /* Releases MUTEX */
    return (0);
}

STATIC int
ithread_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *param)
{
    PERL_UNUSED_ARG(param);
    S_ithread_count_inc(aTHX_ (ithread *)mg->mg_ptr);
    return (0);
}

STATIC const MGVTBL ithread_vtbl = {
    ithread_mg_get,     /* get */
    0,                  /* set */
    0,                  /* len */
    0,                  /* clear */
    ithread_mg_free,    /* free */
    0,                  /* copy */
    ithread_mg_dup,     /* dup */
#if PERL_VERSION_GT(5,8,8)
    0                   /* local */
#endif
};


/* Provided default, minimum and rational stack sizes */
STATIC IV
S_good_stack_size(pTHX_ IV stack_size)
{
    dMY_POOL;

    /* Use default stack size if no stack size specified */
    if (! stack_size) {
        return (MY_POOL.default_stack_size);
    }

#ifdef PTHREAD_STACK_MIN
    /* Can't use less than minimum */
    if (stack_size < PTHREAD_STACK_MIN) {
        if (ckWARN(WARN_THREADS)) {
            Perl_warn(aTHX_ "Using minimum thread stack size of %" IVdf, (IV)PTHREAD_STACK_MIN);
        }
        return (PTHREAD_STACK_MIN);
    }
#endif

    /* Round up to page size boundary */
    if (MY_POOL.page_size <= 0) {
#if defined(HAS_SYSCONF) && (defined(_SC_PAGESIZE) || defined(_SC_MMAP_PAGE_SIZE))
        SETERRNO(0, SS_NORMAL);
#  ifdef _SC_PAGESIZE
        MY_POOL.page_size = sysconf(_SC_PAGESIZE);
#  else
        MY_POOL.page_size = sysconf(_SC_MMAP_PAGE_SIZE);
#  endif
        if ((long)MY_POOL.page_size < 0) {
            if (errno) {
                SV * const error = get_sv("@", 0);
                (void)SvUPGRADE(error, SVt_PV);
                Perl_croak(aTHX_ "PANIC: sysconf: %s", SvPV_nolen(error));
            } else {
                Perl_croak(aTHX_ "PANIC: sysconf: pagesize unknown");
            }
        }
#else
#  ifdef HAS_GETPAGESIZE
        MY_POOL.page_size = getpagesize();
#  else
#    if defined(I_SYS_PARAM) && defined(PAGESIZE)
        MY_POOL.page_size = PAGESIZE;
#    else
        MY_POOL.page_size = 8192;   /* A conservative default */
#    endif
#  endif
        if (MY_POOL.page_size <= 0) {
            Perl_croak(aTHX_ "PANIC: bad pagesize %" IVdf, (IV)MY_POOL.page_size);
        }
#endif
    }
    stack_size = ((stack_size + (MY_POOL.page_size - 1)) / MY_POOL.page_size) * MY_POOL.page_size;

    return (stack_size);
}


/* Run code within a JMPENV environment.
 * Using a separate function avoids
 *   "variable 'foo' might be clobbered by 'longjmp'"
 * warnings.
 * The three _p vars return values to the caller
 */
static int
S_jmpenv_run(pTHX_ int action, ithread *thread,
             int *len_p, int *exit_app_p, int *exit_code_p)
{
    dJMPENV;
    volatile I32 oldscope = PL_scopestack_ix;
    int jmp_rc = 0;

    JMPENV_PUSH(jmp_rc);
    if (jmp_rc == 0) {
        if (action == 0) {
            /* Run the specified function */
            *len_p = (int)call_sv(thread->init_function, thread->gimme|G_EVAL);
        } else if (action == 1) {
            /* Warn that thread died */
            Perl_warn(aTHX_ "Thread %" UVuf " terminated abnormally: %" SVf, thread->tid, ERRSV);
        } else {
            /* Warn if there are unjoined threads */
            S_exit_warning(aTHX);
        }
    } else if (jmp_rc == 2) {
        /* Thread exited */
        *exit_app_p = 1;
        *exit_code_p = STATUS_CURRENT;
        while (PL_scopestack_ix > oldscope) {
            LEAVE;
        }
    }
    JMPENV_POP;
    return jmp_rc;
}


/* Starts executing the thread.
 * Passed as the C level function to run in the new thread.
 */
#ifdef WIN32
STATIC THREAD_RET_TYPE
S_ithread_run(LPVOID arg)
#else
STATIC void *
S_ithread_run(void * arg)
#endif
{
    ithread *thread = (ithread *)arg;
    int exit_app = 0;   /* Thread terminated using 'exit' */
    int exit_code = 0;
    int died = 0;       /* Thread terminated abnormally */


    dTHXa(thread->interp);

    dMY_POOL;

    /* The following mutex lock + mutex unlock pair explained.
     *
     * parent:
     * - calls ithread_create (and S_ithread_create), which:
     *   - creates the new thread
     *   - does MUTEX_LOCK(&thread->mutex)
     *   - calls pthread_create(..., S_ithread_run,...)
     * child:
     * - starts the S_ithread_run (where we are now), which:
     *   - tries to MUTEX_LOCK(&thread->mutex)
     *   - blocks
     * parent:
     *   - continues doing more createy stuff
     *   - does MUTEX_UNLOCK(&thread->mutex)
     *   - continues
     * child:
     *   - finishes MUTEX_LOCK(&thread->mutex)
     *   - does MUTEX_UNLOCK(&thread->mutex)
     *   - continues
     */
    MUTEX_LOCK(&thread->mutex);
    MUTEX_UNLOCK(&thread->mutex);

    PERL_SET_CONTEXT(thread->interp);
    S_ithread_set(aTHX_ thread);

#ifdef THREAD_SIGNAL_BLOCKING
    /* Thread starts with most signals blocked - restore the signal mask from
     * the ithread struct.
     */
    S_set_sigmask(&thread->initial_sigmask);
#endif

    thread_locale_init();

    PL_perl_destruct_level = 2;

    {
        AV *params = thread->params;
        int len = (int)av_len(params)+1;
        int ii;
        int jmp_rc;

        dSP;
        ENTER;
        SAVETMPS;

        /* Put args on the stack */
        PUSHMARK(SP);
        for (ii=0; ii < len; ii++) {
            XPUSHs(av_shift(params));
        }
        PUTBACK;

        jmp_rc = S_jmpenv_run(aTHX_ 0, thread, &len, &exit_app, &exit_code);

#ifdef THREAD_SIGNAL_BLOCKING
        /* The interpreter is finished, so this thread can stop receiving
         * signals.  This way, our signal handler doesn't get called in the
         * middle of our parent thread calling perl_destruct()...
         */
        S_block_most_signals(NULL);
#endif

        /* Remove args from stack and put back in params array */
        SPAGAIN;
        for (ii=len-1; ii >= 0; ii--) {
            SV *sv = POPs;
            if (jmp_rc == 0 && (thread->gimme & G_WANT) != G_VOID) {
                av_store(params, ii, SvREFCNT_inc(sv));
            }
        }

        FREETMPS;
        LEAVE;

        /* Check for abnormal termination */
        if (SvTRUE(ERRSV)) {
            died = PERL_ITHR_DIED;
            thread->err = newSVsv(ERRSV);
            /* If ERRSV is an object, remember the classname and then
             * rebless into 'main' so it will survive 'cloning'
             */
            if (sv_isobject(thread->err)) {
                thread->err_class = HvNAME(SvSTASH(SvRV(thread->err)));
                sv_bless(thread->err, gv_stashpv("main", 0));
            }

            if (ckWARN_d(WARN_THREADS)) {
                (void)S_jmpenv_run(aTHX_ 1, thread, NULL,
                                            &exit_app, &exit_code);
            }
        }

        /* Release function ref */
        SvREFCNT_dec(thread->init_function);
        thread->init_function = Nullsv;
    }

    PerlIO_flush((PerlIO *)NULL);

    MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
    MUTEX_LOCK(&thread->mutex);
    /* Mark as finished */
    thread->state |= (PERL_ITHR_FINISHED | died);
    /* Clear exit flag if required */
    if (thread->state & PERL_ITHR_THREAD_EXIT_ONLY) {
        exit_app = 0;
    }

    /* Adjust thread status counts */
    if (thread->state & PERL_ITHR_DETACHED) {
        MY_POOL.detached_threads--;
    } else {
        MY_POOL.running_threads--;
        MY_POOL.joinable_threads++;
    }
    MUTEX_UNLOCK(&thread->mutex);
    MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);

    thread_locale_term();

    /* Exit application if required */
    if (exit_app) {
        (void)S_jmpenv_run(aTHX_ 2, thread, NULL, &exit_app, &exit_code);
        my_exit(exit_code);
    }

    /* At this point, the interpreter may have been freed, so call
     * free in the context of the 'main' interpreter which
     * can't have been freed due to the veto_cleanup mechanism.
     */
    aTHX = MY_POOL.main_thread.interp;

    MUTEX_LOCK(&thread->mutex);
    S_ithread_free(aTHX_ thread);   /* Releases MUTEX */

#ifdef WIN32
    return ((DWORD)0);
#else
    return (0);
#endif
}


/* Type conversion helper functions */

STATIC SV *
S_ithread_to_SV(pTHX_ SV *obj, ithread *thread, char *classname, bool inc)
{
    SV *sv;
    MAGIC *mg;

    if (inc)
        S_ithread_count_inc(aTHX_ thread);

    if (! obj) {
        obj = newSV(0);
    }

    sv = newSVrv(obj, classname);
    sv_setiv(sv, PTR2IV(thread));
    mg = sv_magicext(sv, Nullsv, PERL_MAGIC_shared_scalar, &ithread_vtbl, (char *)thread, 0);
    mg->mg_flags |= MGf_DUP;
    SvREADONLY_on(sv);

    return (obj);
}

STATIC ithread *
S_SV_to_ithread(pTHX_ SV *sv)
{
    /* Argument is a thread */
    if (SvROK(sv)) {
      return (INT2PTR(ithread *, SvIV(SvRV(sv))));
    }
    /* Argument is classname, therefore return current thread */
    return (S_ithread_get(aTHX));
}


/* threads->create()
 * Called in context of parent thread.
 * Called with my_pool->create_destruct_mutex locked.
 * (Unlocked both on error and on success.)
 */
STATIC ithread *
S_ithread_create(
        PerlInterpreter *parent_perl,
        my_pool_t *my_pool,
        SV       *init_function,
        IV        stack_size,
        int       gimme,
        int       exit_opt,
        int       params_start,
        int       num_params)
  PERL_TSA_RELEASE(my_pool->create_destruct_mutex)
{
    dTHXa(parent_perl);
    ithread     *thread;
    ithread     *current_thread = S_ithread_get(aTHX);
    AV          *params;
    SV          **array;

#if PERL_VERSION_LE(5,8,7)
    SV         **tmps_tmp = PL_tmps_stack;
    IV           tmps_ix  = PL_tmps_ix;
#endif
#ifndef WIN32
    int          rc_stack_size = 0;
    int          rc_thread_create = 0;
#endif

    /* Allocate thread structure in context of the main thread's interpreter */
    {
        PERL_SET_CONTEXT(my_pool->main_thread.interp);
        thread = (ithread *)PerlMemShared_malloc(sizeof(ithread));
    }
    PERL_SET_CONTEXT(aTHX);
    if (!thread) {
        /* This lock was acquired in ithread_create()
         * prior to calling S_ithread_create(). */
        MUTEX_UNLOCK(&my_pool->create_destruct_mutex);
        {
          int fd = PerlIO_fileno(Perl_error_log);
          if (fd >= 0) {
            /* If there's no error_log, we cannot scream about it missing. */
            PERL_UNUSED_RESULT(PerlLIO_write(fd, PL_no_mem, strlen(PL_no_mem)));
          }
        }
        my_exit(1);
    }
    Zero(thread, 1, ithread);

    /* Add to threads list */
    thread->next = &my_pool->main_thread;
    thread->prev = my_pool->main_thread.prev;
    my_pool->main_thread.prev = thread;
    thread->prev->next = thread;
    my_pool->total_threads++;

    /* 1 ref to be held by the local var 'thread' in S_ithread_run().
     * 1 ref to be held by the threads object that we assume we will
     *      be embedded in upon our return.
     * 1 ref to be the responsibility of join/detach, so we don't get
     *      freed until join/detach, even if no thread objects remain.
     *      This allows the following to work:
     *          { threads->create(sub{...}); } threads->object(1)->join;
     */
    thread->count = 3;

    /* Block new thread until ->create() call finishes */
    MUTEX_INIT(&thread->mutex);
    MUTEX_LOCK(&thread->mutex); /* See S_ithread_run() for more detail. */

    thread->tid = my_pool->tid_counter++;
    thread->stack_size = S_good_stack_size(aTHX_ stack_size);
    thread->gimme = gimme;
    thread->state = exit_opt;


    /* "Clone" our interpreter into the thread's interpreter.
     * This gives thread access to "static data" and code.
     */
    PerlIO_flush((PerlIO *)NULL);
    S_ithread_set(aTHX_ thread);

    SAVEBOOL(PL_srand_called); /* Save this so it becomes the correct value */
    PL_srand_called = FALSE;   /* Set it to false so we can detect if it gets
                                  set during the clone */

#ifdef THREAD_SIGNAL_BLOCKING
    /* perl_clone() will leave us the new interpreter's context.  This poses
     * two problems for our signal handler.  First, it sets the new context
     * before the new interpreter struct is fully initialized, so our signal
     * handler might find bogus data in the interpreter struct it gets.
     * Second, even if the interpreter is initialized before a signal comes in,
     * we would like to avoid that interpreter receiving notifications for
     * signals (especially when they ought to be for the one running in this
     * thread), until it is running in its own thread.  Another problem is that
     * the new thread will not have set the context until some time after it
     * has started, so it won't be safe for our signal handler to run until
     * that time.
     *
     * So we block most signals here, so the new thread will inherit the signal
     * mask, and unblock them right after the thread creation.  The original
     * mask is saved in the thread struct so that the new thread can restore
     * the original mask.
     */
    S_block_most_signals(&thread->initial_sigmask);
#endif

#ifdef WIN32
    thread->interp = perl_clone(aTHX, CLONEf_KEEP_PTR_TABLE | CLONEf_CLONE_HOST);
#else
    thread->interp = perl_clone(aTHX, CLONEf_KEEP_PTR_TABLE);
#endif

    /* perl_clone() leaves us in new interpreter's context.  As it is tricky
     * to spot an implicit aTHX, create a new scope with aTHX matching the
     * context for the duration of our work for new interpreter.
     */
    {
#if PERL_VERSION_GE(5,13,2)
        CLONE_PARAMS *clone_param = Perl_clone_params_new(aTHX, thread->interp);
#else
        CLONE_PARAMS clone_param_s;
        CLONE_PARAMS *clone_param = &clone_param_s;
#endif
        dTHXa(thread->interp);

        MY_CXT_CLONE;

#if PERL_VERSION_LT(5,13,2)
        clone_param->flags = 0;
#endif

        /* Here we remove END blocks since they should only run in the thread
         * they are created
         */
        SvREFCNT_dec(PL_endav);
        PL_endav = NULL;

        if (SvPOK(init_function)) {
            thread->init_function = newSV(0);
            sv_copypv(thread->init_function, init_function);
        } else {
            thread->init_function = sv_dup_inc(init_function, clone_param);
        }

        thread->params = params = newAV();
        av_extend(params, num_params - 1);
        AvFILLp(params) = num_params - 1;
        array = AvARRAY(params);

        /* params_start is an offset onto the Perl stack. This can be
           reallocated (and hence move) as a side effect of calls to
           perl_clone() and sv_dup_inc(). Hence copy the parameters
           somewhere under our control first, before duplicating.  */
        if (num_params) {
#if PERL_VERSION_GE(5,9,0)
            Copy(parent_perl->Istack_base + params_start, array, num_params, SV *);
#else
            Copy(parent_perl->Tstack_base + params_start, array, num_params, SV *);
#endif
            while (num_params--) {
                *array = sv_dup_inc(*array, clone_param);
                ++array;
            }
        }

#if PERL_VERSION_GE(5,13,2)
        Perl_clone_params_del(clone_param);
#endif

#if PERL_VERSION_LT(5,8,8)
        /* The code below checks that anything living on the tmps stack and
         * has been cloned (so it lives in the ptr_table) has a refcount
         * higher than 0.
         *
         * If the refcount is 0 it means that a something on the stack/context
         * was holding a reference to it and since we init_stacks() in
         * perl_clone that won't get cleaned and we will get a leaked scalar.
         * The reason it was cloned was that it lived on the @_ stack.
         *
         * Example of this can be found in bugreport 15837 where calls in the
         * parameter list end up as a temp.
         *
         * As of 5.8.8 this is done in perl_clone.
         */
        while (tmps_ix > 0) {
            SV* sv = (SV*)ptr_table_fetch(PL_ptr_table, tmps_tmp[tmps_ix]);
            tmps_ix--;
            if (sv && SvREFCNT(sv) == 0) {
                SvREFCNT_inc_void(sv);
                SvREFCNT_dec(sv);
            }
        }
#endif

        SvTEMP_off(thread->init_function);
        ptr_table_free(PL_ptr_table);
        PL_ptr_table = NULL;
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    }
    S_ithread_set(aTHX_ current_thread);
    PERL_SET_CONTEXT(aTHX);

    /* Create/start the thread */
#ifdef WIN32
    thread->handle = CreateThread(NULL,
                                  (DWORD)thread->stack_size,
                                  S_ithread_run,
                                  (LPVOID)thread,
                                  STACK_SIZE_PARAM_IS_A_RESERVATION,
                                  &thread->thr);
#else
    {
        STATIC pthread_attr_t attr;
        STATIC int attr_inited = 0;
        STATIC int attr_joinable = PTHREAD_CREATE_JOINABLE;
        if (! attr_inited) {
            pthread_attr_init(&attr);
            attr_inited = 1;
        }

#  ifdef PTHREAD_ATTR_SETDETACHSTATE
        /* Threads start out joinable */
        PTHREAD_ATTR_SETDETACHSTATE(&attr, attr_joinable);
#  endif

#  ifdef _POSIX_THREAD_ATTR_STACKSIZE
        /* Set thread's stack size */
        if (thread->stack_size > 0) {
            rc_stack_size = pthread_attr_setstacksize(&attr, (size_t)thread->stack_size);
        }
#  endif

        /* Create the thread */
        if (! rc_stack_size) {
#  ifdef OLD_PTHREADS_API
            rc_thread_create = pthread_create(&thread->thr,
                                              attr,
                                              S_ithread_run,
                                              (void *)thread);
#  else
#    if defined(HAS_PTHREAD_ATTR_SETSCOPE) && defined(PTHREAD_SCOPE_SYSTEM)
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#    endif
            rc_thread_create = pthread_create(&thread->thr,
                                              &attr,
                                              S_ithread_run,
                                              (void *)thread);
#  endif
        }

#ifdef THREAD_SIGNAL_BLOCKING
    /* Now it's safe to accept signals, since we're in our own interpreter's
     * context and we have created the thread.
     */
    S_set_sigmask(&thread->initial_sigmask);
#endif

#  ifdef _POSIX_THREAD_ATTR_STACKSIZE
        /* Try to get thread's actual stack size */
        {
            size_t stacksize;
#ifdef HPUX1020
            stacksize = pthread_attr_getstacksize(attr);
#else
            if (! pthread_attr_getstacksize(&attr, &stacksize))
#endif
                if (stacksize > 0) {
                    thread->stack_size = (IV)stacksize;
                }
        }
#  endif
    }
#endif

    /* Check for errors */
#ifdef WIN32
    if (thread->handle == NULL) {
#else
    if (rc_stack_size || rc_thread_create) {
#endif
        /* Must unlock mutex for destruct call */
        /* This lock was acquired in ithread_create()
         * prior to calling S_ithread_create(). */
        MUTEX_UNLOCK(&my_pool->create_destruct_mutex);
        thread->state |= PERL_ITHR_NONVIABLE;
        S_ithread_free(aTHX_ thread);   /* Releases MUTEX */
#ifndef WIN32
        if (ckWARN_d(WARN_THREADS)) {
            if (rc_stack_size) {
                Perl_warn(aTHX_ "Thread creation failed: pthread_attr_setstacksize(%" IVdf ") returned %d", thread->stack_size, rc_stack_size);
            } else {
                Perl_warn(aTHX_ "Thread creation failed: pthread_create returned %d", rc_thread_create);
            }
        }
#endif
        return (NULL);
    }

    my_pool->running_threads++;
    MUTEX_UNLOCK(&my_pool->create_destruct_mutex);
    return (thread);

    CLANG_DIAG_IGNORE(-Wthread-safety)
    /* warning: mutex 'thread->mutex' is not held on every path through here [-Wthread-safety-analysis] */
}
CLANG_DIAG_RESTORE

#endif /* USE_ITHREADS */


MODULE = threads    PACKAGE = threads    PREFIX = ithread_
PROTOTYPES: DISABLE

#ifdef USE_ITHREADS

void
ithread_create(...)
    PREINIT:
        char *classname;
        ithread *thread;
        SV *function_to_call;
        HV *specs;
        IV stack_size;
        int context;
        int exit_opt;
        SV *thread_exit_only;
        char *str;
        int idx;
        dMY_POOL;
    CODE:
        if ((items >= 2) && SvROK(ST(1)) && SvTYPE(SvRV(ST(1)))==SVt_PVHV) {
            if (--items < 2) {
                Perl_croak(aTHX_ "Usage: threads->create(\\%%specs, function, ...)");
            }
            specs = (HV*)SvRV(ST(1));
            idx = 1;
        } else {
            if (items < 2) {
                Perl_croak(aTHX_ "Usage: threads->create(function, ...)");
            }
            specs = NULL;
            idx = 0;
        }

        if (sv_isobject(ST(0))) {
            /* $thr->create() */
            classname = HvNAME(SvSTASH(SvRV(ST(0))));
            thread = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
            MUTEX_LOCK(&thread->mutex);
            stack_size = thread->stack_size;
            exit_opt = thread->state & PERL_ITHR_THREAD_EXIT_ONLY;
            MUTEX_UNLOCK(&thread->mutex);
        } else {
            /* threads->create() */
            classname = (char *)SvPV_nolen(ST(0));
            stack_size = MY_POOL.default_stack_size;
            thread_exit_only = get_sv("threads::thread_exit_only", GV_ADD);
            exit_opt = (SvTRUE(thread_exit_only))
                                    ? PERL_ITHR_THREAD_EXIT_ONLY : 0;
        }

        function_to_call = ST(idx+1);

        context = -1;
        if (specs) {
            SV **svp;
            /* stack_size */
            if ((svp = hv_fetchs(specs, "stack", 0))) {
                stack_size = SvIV(*svp);
            } else if ((svp = hv_fetchs(specs, "stacksize", 0))) {
                stack_size = SvIV(*svp);
            } else if ((svp = hv_fetchs(specs, "stack_size", 0))) {
                stack_size = SvIV(*svp);
            }

            /* context */
            if ((svp = hv_fetchs(specs, "context", 0))) {
                str = (char *)SvPV_nolen(*svp);
                switch (*str) {
                    case 'a':
                    case 'A':
                    case 'l':
                    case 'L':
                        context = G_LIST;
                        break;
                    case 's':
                    case 'S':
                        context = G_SCALAR;
                        break;
                    case 'v':
                    case 'V':
                        context = G_VOID;
                        break;
                    default:
                        Perl_croak(aTHX_ "Invalid context: %s", str);
                }
            } else if ((svp = hv_fetchs(specs, "array", 0))) {
                if (SvTRUE(*svp)) {
                    context = G_LIST;
                }
            } else if ((svp = hv_fetchs(specs, "list", 0))) {
                if (SvTRUE(*svp)) {
                    context = G_LIST;
                }
            } else if ((svp = hv_fetchs(specs, "scalar", 0))) {
                if (SvTRUE(*svp)) {
                    context = G_SCALAR;
                }
            } else if ((svp = hv_fetchs(specs, "void", 0))) {
                if (SvTRUE(*svp)) {
                    context = G_VOID;
                }
            }

            /* exit => thread_only */
            if ((svp = hv_fetchs(specs, "exit", 0))) {
                str = (char *)SvPV_nolen(*svp);
                exit_opt = (*str == 't' || *str == 'T')
                                    ? PERL_ITHR_THREAD_EXIT_ONLY : 0;
            }
        }
        if (context == -1) {
            context = GIMME_V;  /* Implicit context */
        } else {
            context |= (GIMME_V & (~(G_LIST|G_SCALAR|G_VOID)));
        }

        /* Create thread */
        MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
        thread = S_ithread_create(aTHX_ &MY_POOL,
                                        function_to_call,
                                        stack_size,
                                        context,
                                        exit_opt,
                                        ax + idx + 2,
                                        items > 2 ? items - 2 : 0);
        if (! thread) {
            XSRETURN_UNDEF;     /* Mutex already unlocked */
        }
        PERL_SRAND_OVERRIDE_NEXT_PARENT();
        ST(0) = sv_2mortal(S_ithread_to_SV(aTHX_ Nullsv, thread, classname, FALSE));

        /* Let thread run. */
        /* See S_ithread_run() for more detail. */
        CLANG_DIAG_IGNORE_STMT(-Wthread-safety);
        /* warning: releasing mutex 'thread->mutex' that was not held [-Wthread-safety-analysis] */
        MUTEX_UNLOCK(&thread->mutex);
        CLANG_DIAG_RESTORE_STMT;
        /* XSRETURN(1); - implied */


void
ithread_list(...)
    PREINIT:
        char *classname;
        ithread *thread;
        int list_context;
        IV count = 0;
        int want_running = 0;
        int state;
        dMY_POOL;
    PPCODE:
        /* Class method only */
        if (SvROK(ST(0))) {
            Perl_croak(aTHX_ "Usage: threads->list(...)");
        }
        classname = (char *)SvPV_nolen(ST(0));

        /* Calling context */
        list_context = (GIMME_V == G_LIST);

        /* Running or joinable parameter */
        if (items > 1) {
            want_running = SvTRUE(ST(1));
        }

        /* Walk through threads list */
        MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
        for (thread = MY_POOL.main_thread.next;
             thread != &MY_POOL.main_thread;
             thread = thread->next)
        {
            MUTEX_LOCK(&thread->mutex);
            state = thread->state;
            MUTEX_UNLOCK(&thread->mutex);

            /* Ignore detached or joined threads */
            if (state & PERL_ITHR_UNCALLABLE) {
                continue;
            }

            /* Filter per parameter */
            if (items > 1) {
                if (want_running) {
                    if (state & PERL_ITHR_FINISHED) {
                        continue;   /* Not running */
                    }
                } else {
                    if (! (state & PERL_ITHR_FINISHED)) {
                        continue;   /* Still running - not joinable yet */
                    }
                }
            }

            /* Push object on stack if list context */
            if (list_context) {
                XPUSHs(sv_2mortal(S_ithread_to_SV(aTHX_ Nullsv, thread, classname, TRUE)));
            }
            count++;
        }
        MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);
        /* If scalar context, send back count */
        if (! list_context) {
            XSRETURN_IV(count);
        }


void
ithread_self(...)
    PREINIT:
        char *classname;
        ithread *thread;
    CODE:
        /* Class method only */
        if ((items != 1) || SvROK(ST(0))) {
            Perl_croak(aTHX_ "Usage: threads->self()");
        }
        classname = (char *)SvPV_nolen(ST(0));

        thread = S_ithread_get(aTHX);

        ST(0) = sv_2mortal(S_ithread_to_SV(aTHX_ Nullsv, thread, classname, TRUE));
        /* XSRETURN(1); - implied */


void
ithread_tid(...)
    PREINIT:
        ithread *thread;
    CODE:
        PERL_UNUSED_VAR(items);
        thread = S_SV_to_ithread(aTHX_ ST(0));
        XST_mUV(0, thread->tid);
        /* XSRETURN(1); - implied */


void
ithread_join(...)
    PREINIT:
        ithread *thread;
        ithread *current_thread;
        int join_err;
        AV *params = NULL;
        int len;
        int ii;
#ifndef WIN32
        int rc_join;
        void *retval;
#endif
        dMY_POOL;
    PPCODE:
        /* Object method only */
        if ((items != 1) || ! sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Usage: $thr->join()");
        }

        /* Check if the thread is joinable and not ourselves */
        thread = S_SV_to_ithread(aTHX_ ST(0));
        current_thread = S_ithread_get(aTHX);

        MUTEX_LOCK(&thread->mutex);
        if ((join_err = (thread->state & PERL_ITHR_UNCALLABLE))) {
            MUTEX_UNLOCK(&thread->mutex);
            Perl_croak(aTHX_ (join_err & PERL_ITHR_DETACHED)
                                ? "Cannot join a detached thread"
                                : "Thread already joined");
        } else if (thread->tid == current_thread->tid) {
            MUTEX_UNLOCK(&thread->mutex);
            Perl_croak(aTHX_ "Cannot join self");
        }

        /* Mark as joined */
        thread->state |= PERL_ITHR_JOINED;
        MUTEX_UNLOCK(&thread->mutex);

        MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
        MY_POOL.joinable_threads--;
        MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);

        /* Join the thread */
#ifdef WIN32
        if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) {
            /* Timeout/abandonment unexpected here; check $^E */
            Perl_croak(aTHX_ "PANIC: underlying join failed");
        };
#else
        if ((rc_join = pthread_join(thread->thr, &retval)) != 0) {
            /* In progress/deadlock/unknown unexpected here; check $! */
            errno = rc_join;
            Perl_croak(aTHX_ "PANIC: underlying join failed");
        };
#endif

        MUTEX_LOCK(&thread->mutex);
        /* Get the return value from the call_sv */
        /* Objects do not survive this process - FIXME */
        if ((thread->gimme & G_WANT) != G_VOID) {
#if PERL_VERSION_LT(5,13,2)
            AV *params_copy;
            PerlInterpreter *other_perl;
            CLONE_PARAMS clone_params;

            params_copy = thread->params;
            other_perl = thread->interp;
            clone_params.stashes = newAV();
            clone_params.flags = CLONEf_JOIN_IN;
            PL_ptr_table = ptr_table_new();
            S_ithread_set(aTHX_ thread);
            /* Ensure 'meaningful' addresses retain their meaning */
            ptr_table_store(PL_ptr_table, &other_perl->Isv_undef, &PL_sv_undef);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_no, &PL_sv_no);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_yes, &PL_sv_yes);
            params = (AV *)sv_dup((SV*)params_copy, &clone_params);
            S_ithread_set(aTHX_ current_thread);
            SvREFCNT_dec(clone_params.stashes);
            SvREFCNT_inc_void(params);
            ptr_table_free(PL_ptr_table);
            PL_ptr_table = NULL;
#else
            AV *params_copy;
            PerlInterpreter *other_perl = thread->interp;
            CLONE_PARAMS *clone_params = Perl_clone_params_new(other_perl, aTHX);

            params_copy = thread->params;
            clone_params->flags |= CLONEf_JOIN_IN;
            PL_ptr_table = ptr_table_new();
            S_ithread_set(aTHX_ thread);
            /* Ensure 'meaningful' addresses retain their meaning */
            ptr_table_store(PL_ptr_table, &other_perl->Isv_undef, &PL_sv_undef);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_no, &PL_sv_no);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_yes, &PL_sv_yes);
#  ifdef PL_sv_zero
            ptr_table_store(PL_ptr_table, &other_perl->Isv_zero, &PL_sv_zero);
#  endif
            params = (AV *)sv_dup((SV*)params_copy, clone_params);
            S_ithread_set(aTHX_ current_thread);
            Perl_clone_params_del(clone_params);
            SvREFCNT_inc_void(params);
            ptr_table_free(PL_ptr_table);
            PL_ptr_table = NULL;
#endif
        }

        /* If thread didn't die, then we can free its interpreter */
        if (! (thread->state & PERL_ITHR_DIED)) {
            S_ithread_clear(aTHX_ thread);
        }
        S_ithread_free(aTHX_ thread);   /* Releases MUTEX */

        /* If no return values, then just return */
        if (! params) {
            XSRETURN_UNDEF;
        }

        /* Put return values on stack */
        len = (int)AvFILL(params);
        for (ii=0; ii <= len; ii++) {
            SV* param = av_shift(params);
            XPUSHs(sv_2mortal(param));
        }

        /* Free return value array */
        SvREFCNT_dec(params);


void
ithread_yield(...)
    CODE:
        PERL_UNUSED_VAR(items);
        YIELD;


void
ithread_detach(...)
    PREINIT:
        ithread *thread;
        int detach_err;
        dMY_POOL;
    CODE:
        PERL_UNUSED_VAR(items);

        /* Detach the thread */
        thread = S_SV_to_ithread(aTHX_ ST(0));
        MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
        MUTEX_LOCK(&thread->mutex);
        if (! (detach_err = (thread->state & PERL_ITHR_UNCALLABLE))) {
            /* Thread is detachable */
            thread->state |= PERL_ITHR_DETACHED;
#ifdef WIN32
            /* Windows has no 'detach thread' function */
#else
            PERL_THREAD_DETACH(thread->thr);
#endif
            if (thread->state & PERL_ITHR_FINISHED) {
                MY_POOL.joinable_threads--;
            } else {
                MY_POOL.running_threads--;
                MY_POOL.detached_threads++;
            }
        }
        MUTEX_UNLOCK(&thread->mutex);
        MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);

        if (detach_err) {
            Perl_croak(aTHX_ (detach_err & PERL_ITHR_DETACHED)
                                ? "Thread already detached"
                                : "Cannot detach a joined thread");
        }

        /* If thread is finished and didn't die,
         * then we can free its interpreter */
        MUTEX_LOCK(&thread->mutex);
        if ((thread->state & PERL_ITHR_FINISHED) &&
            ! (thread->state & PERL_ITHR_DIED))
        {
            S_ithread_clear(aTHX_ thread);
        }
        S_ithread_free(aTHX_ thread);   /* Releases MUTEX */


void
ithread_kill(...)
    PREINIT:
        ithread *thread;
        char *sig_name;
        IV signal;
        int no_handler = 1;
    CODE:
        /* Must have safe signals */
        if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG) {
            Perl_croak(aTHX_ "Cannot signal threads without safe signals");
        }

        /* Object method only */
        if ((items != 2) || ! sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Usage: $thr->kill('SIG...')");
        }

        /* Get signal */
        sig_name = SvPV_nolen(ST(1));
        if (isALPHA(*sig_name)) {
            if (*sig_name == 'S' && sig_name[1] == 'I' && sig_name[2] == 'G') {
                sig_name += 3;
            }
            if ((signal = whichsig(sig_name)) < 0) {
                Perl_croak(aTHX_ "Unrecognized signal name: %s", sig_name);
            }
        } else {
            signal = SvIV(ST(1));
        }

        /* Set the signal for the thread */
        thread = S_SV_to_ithread(aTHX_ ST(0));
        MUTEX_LOCK(&thread->mutex);
        if (thread->interp && ! (thread->state & PERL_ITHR_FINISHED)) {
            dTHXa(thread->interp);
            if (PL_psig_pend && PL_psig_ptr[signal]) {
                PL_psig_pend[signal]++;
                PL_sig_pending = 1;
                no_handler = 0;
            }
        } else {
            /* Ignore signal to terminated/finished thread */
            no_handler = 0;
        }
        MUTEX_UNLOCK(&thread->mutex);

        if (no_handler) {
            Perl_croak(aTHX_ "Signal %s received in thread %" UVuf
                             ", but no signal handler set.",
                             sig_name, thread->tid);
        }

        /* Return the thread to allow for method chaining */
        ST(0) = ST(0);
        /* XSRETURN(1); - implied */


void
ithread_DESTROY(...)
    CODE:
        PERL_UNUSED_VAR(items);
        sv_unmagic(SvRV(ST(0)), PERL_MAGIC_shared_scalar);


void
ithread_equal(...)
    PREINIT:
        int are_equal = 0;
    CODE:
        PERL_UNUSED_VAR(items);

        /* Compares TIDs to determine thread equality */
        if (sv_isobject(ST(0)) && sv_isobject(ST(1))) {
            ithread *thr1 = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
            ithread *thr2 = INT2PTR(ithread *, SvIV(SvRV(ST(1))));
            are_equal = (thr1->tid == thr2->tid);
        }
        if (are_equal) {
            XST_mYES(0);
        } else {
            /* Return 0 on false for backward compatibility */
            XST_mIV(0, 0);
        }
        /* XSRETURN(1); - implied */


void
ithread_object(...)
    PREINIT:
        char *classname;
        SV *arg;
        UV tid;
        ithread *thread;
        int state;
        int have_obj = 0;
        dMY_POOL;
    CODE:
        /* Class method only */
        if (SvROK(ST(0))) {
            Perl_croak(aTHX_ "Usage: threads->object($tid)");
        }
        classname = (char *)SvPV_nolen(ST(0));

        /* Turn $tid from PVLV to SV if needed (bug #73330) */
        arg = ST(1);
        SvGETMAGIC(arg);

        if ((items < 2) || ! SvOK(arg)) {
            XSRETURN_UNDEF;
        }

        /* threads->object($tid) */
        tid = SvUV(arg);

        /* If current thread wants its own object, then behave the same as
           ->self() */
        thread = S_ithread_get(aTHX);
        if (thread->tid == tid) {
            ST(0) = sv_2mortal(S_ithread_to_SV(aTHX_ Nullsv, thread, classname, TRUE));
            have_obj = 1;

        } else {
            /* Walk through threads list */
            MUTEX_LOCK(&MY_POOL.create_destruct_mutex);
            for (thread = MY_POOL.main_thread.next;
                 thread != &MY_POOL.main_thread;
                 thread = thread->next)
            {
                /* Look for TID */
                if (thread->tid == tid) {
                    /* Ignore if detached or joined */
                    MUTEX_LOCK(&thread->mutex);
                    state = thread->state;
                    MUTEX_UNLOCK(&thread->mutex);
                    if (! (state & PERL_ITHR_UNCALLABLE)) {
                        /* Put object on stack */
                        ST(0) = sv_2mortal(S_ithread_to_SV(aTHX_ Nullsv, thread, classname, TRUE));
                        have_obj = 1;
                    }
                    break;
                }
            }
            MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);
        }

        if (! have_obj) {
            XSRETURN_UNDEF;
        }
        /* XSRETURN(1); - implied */


void
ithread__handle(...);
    PREINIT:
        ithread *thread;
    CODE:
        PERL_UNUSED_VAR(items);
        thread = S_SV_to_ithread(aTHX_ ST(0));
#ifdef WIN32
        XST_mUV(0, PTR2UV(&thread->handle));
#else
        XST_mUV(0, PTR2UV(&thread->thr));
#endif
        /* XSRETURN(1); - implied */


void
ithread_get_stack_size(...)
    PREINIT:
        IV stack_size;
        dMY_POOL;
    CODE:
        PERL_UNUSED_VAR(items);
        if (sv_isobject(ST(0))) {
            /* $thr->get_stack_size() */
            ithread *thread = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
            stack_size = thread->stack_size;
        } else {
            /* threads->get_stack_size() */
            stack_size = MY_POOL.default_stack_size;
        }
        XST_mIV(0, stack_size);
        /* XSRETURN(1); - implied */


void
ithread_set_stack_size(...)
    PREINIT:
        IV old_size;
        dMY_POOL;
    CODE:
        if (items != 2) {
            Perl_croak(aTHX_ "Usage: threads->set_stack_size($size)");
        }
        if (sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Cannot change stack size of an existing thread");
        }
        if (! looks_like_number(ST(1))) {
            Perl_croak(aTHX_ "Stack size must be numeric");
        }

        old_size = MY_POOL.default_stack_size;
        MY_POOL.default_stack_size = S_good_stack_size(aTHX_ SvIV(ST(1)));
        XST_mIV(0, old_size);
        /* XSRETURN(1); - implied */


void
ithread_is_running(...)
    PREINIT:
        ithread *thread;
    CODE:
        /* Object method only */
        if ((items != 1) || ! sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Usage: $thr->is_running()");
        }

        thread = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
        MUTEX_LOCK(&thread->mutex);
        ST(0) = (thread->state & PERL_ITHR_FINISHED) ? &PL_sv_no : &PL_sv_yes;
        MUTEX_UNLOCK(&thread->mutex);
        /* XSRETURN(1); - implied */


void
ithread_is_detached(...)
    PREINIT:
        ithread *thread;
    CODE:
        PERL_UNUSED_VAR(items);
        thread = S_SV_to_ithread(aTHX_ ST(0));
        MUTEX_LOCK(&thread->mutex);
        ST(0) = (thread->state & PERL_ITHR_DETACHED) ? &PL_sv_yes : &PL_sv_no;
        MUTEX_UNLOCK(&thread->mutex);
        /* XSRETURN(1); - implied */


void
ithread_is_joinable(...)
    PREINIT:
        ithread *thread;
    CODE:
        /* Object method only */
        if ((items != 1) || ! sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Usage: $thr->is_joinable()");
        }

        thread = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
        MUTEX_LOCK(&thread->mutex);
        ST(0) = ((thread->state & PERL_ITHR_FINISHED) &&
                 ! (thread->state & PERL_ITHR_UNCALLABLE))
            ? &PL_sv_yes : &PL_sv_no;
        MUTEX_UNLOCK(&thread->mutex);
        /* XSRETURN(1); - implied */


void
ithread_wantarray(...)
    PREINIT:
        ithread *thread;
    CODE:
        PERL_UNUSED_VAR(items);
        thread = S_SV_to_ithread(aTHX_ ST(0));
        ST(0) = ((thread->gimme & G_WANT) == G_LIST) ? &PL_sv_yes :
                ((thread->gimme & G_WANT) == G_VOID) ? &PL_sv_undef
                                      /* G_SCALAR */ : &PL_sv_no;
        /* XSRETURN(1); - implied */


void
ithread_set_thread_exit_only(...)
    PREINIT:
        ithread *thread;
    CODE:
        if (items != 2) {
            Perl_croak(aTHX_ "Usage: ->set_thread_exit_only(boolean)");
        }
        thread = S_SV_to_ithread(aTHX_ ST(0));
        MUTEX_LOCK(&thread->mutex);
        if (SvTRUE(ST(1))) {
            thread->state |= PERL_ITHR_THREAD_EXIT_ONLY;
        } else {
            thread->state &= ~PERL_ITHR_THREAD_EXIT_ONLY;
        }
        MUTEX_UNLOCK(&thread->mutex);


void
ithread_error(...)
    PREINIT:
        ithread *thread;
        SV *err = NULL;
    CODE:
        /* Object method only */
        if ((items != 1) || ! sv_isobject(ST(0))) {
            Perl_croak(aTHX_ "Usage: $thr->err()");
        }

        thread = INT2PTR(ithread *, SvIV(SvRV(ST(0))));
        MUTEX_LOCK(&thread->mutex);

        /* If thread died, then clone the error into the calling thread */
        if (thread->state & PERL_ITHR_DIED) {
#if PERL_VERSION_LT(5,13,2)
            PerlInterpreter *other_perl;
            CLONE_PARAMS clone_params;
            ithread *current_thread;

            other_perl = thread->interp;
            clone_params.stashes = newAV();
            clone_params.flags = CLONEf_JOIN_IN;
            PL_ptr_table = ptr_table_new();
            current_thread = S_ithread_get(aTHX);
            S_ithread_set(aTHX_ thread);
            /* Ensure 'meaningful' addresses retain their meaning */
            ptr_table_store(PL_ptr_table, &other_perl->Isv_undef, &PL_sv_undef);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_no, &PL_sv_no);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_yes, &PL_sv_yes);
            err = sv_dup(thread->err, &clone_params);
            S_ithread_set(aTHX_ current_thread);
            SvREFCNT_dec(clone_params.stashes);
            SvREFCNT_inc_void(err);
            /* If error was an object, bless it into the correct class */
            if (thread->err_class) {
                sv_bless(err, gv_stashpv(thread->err_class, 1));
            }
            ptr_table_free(PL_ptr_table);
            PL_ptr_table = NULL;
#else
            PerlInterpreter *other_perl = thread->interp;
            CLONE_PARAMS *clone_params = Perl_clone_params_new(other_perl, aTHX);
            ithread *current_thread;

            clone_params->flags |= CLONEf_JOIN_IN;
            PL_ptr_table = ptr_table_new();
            current_thread = S_ithread_get(aTHX);
            S_ithread_set(aTHX_ thread);
            /* Ensure 'meaningful' addresses retain their meaning */
            ptr_table_store(PL_ptr_table, &other_perl->Isv_undef, &PL_sv_undef);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_no, &PL_sv_no);
            ptr_table_store(PL_ptr_table, &other_perl->Isv_yes, &PL_sv_yes);
#  ifdef PL_sv_zero
            ptr_table_store(PL_ptr_table, &other_perl->Isv_zero, &PL_sv_zero);
#  endif
            err = sv_dup(thread->err, clone_params);
            S_ithread_set(aTHX_ current_thread);
            Perl_clone_params_del(clone_params);
            SvREFCNT_inc_void(err);
            /* If error was an object, bless it into the correct class */
            if (thread->err_class) {
                sv_bless(err, gv_stashpv(thread->err_class, 1));
            }
            ptr_table_free(PL_ptr_table);
            PL_ptr_table = NULL;
#endif
        }

        MUTEX_UNLOCK(&thread->mutex);

        if (! err) {
            XSRETURN_UNDEF;
        }

        ST(0) = sv_2mortal(err);
        /* XSRETURN(1); - implied */


#endif /* USE_ITHREADS */


BOOT:
{
#ifdef USE_ITHREADS
    SV *my_pool_sv = *hv_fetch(PL_modglobal, MY_POOL_KEY,
                               sizeof(MY_POOL_KEY)-1, TRUE);
    my_pool_t *my_poolp = (my_pool_t*)SvPVX(newSV(sizeof(my_pool_t)-1));

    MY_CXT_INIT;

    Zero(my_poolp, 1, my_pool_t);
    sv_setuv(my_pool_sv, PTR2UV(my_poolp));

    PL_perl_destruct_level = 2;
    MUTEX_INIT(&MY_POOL.create_destruct_mutex);
    MUTEX_LOCK(&MY_POOL.create_destruct_mutex);

    PL_threadhook = &Perl_ithread_hook;

    MY_POOL.tid_counter = 1;
#  ifdef THREAD_CREATE_NEEDS_STACK
    MY_POOL.default_stack_size = THREAD_CREATE_NEEDS_STACK;
#  endif

    /* The 'main' thread is thread 0.
     * It is detached (unjoinable) and immortal.
     */

    MUTEX_INIT(&MY_POOL.main_thread.mutex);

    /* Head of the threads list */
    MY_POOL.main_thread.next = &MY_POOL.main_thread;
    MY_POOL.main_thread.prev = &MY_POOL.main_thread;

    MY_POOL.main_thread.count = 1;                  /* Immortal */

    MY_POOL.main_thread.interp = aTHX;
    MY_POOL.main_thread.state = PERL_ITHR_DETACHED; /* Detached */
    MY_POOL.main_thread.stack_size = MY_POOL.default_stack_size;
#  ifdef WIN32
    MY_POOL.main_thread.thr = GetCurrentThreadId();
#  else
    MY_POOL.main_thread.thr = pthread_self();
#  endif

    S_ithread_set(aTHX_ &MY_POOL.main_thread);
    MUTEX_UNLOCK(&MY_POOL.create_destruct_mutex);
#endif /* USE_ITHREADS */
}
