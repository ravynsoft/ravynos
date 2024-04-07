/*    thread.h
 *
 *    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#if defined(USE_ITHREADS)

#if defined(VMS)
#include <builtins.h>
#endif

#ifdef WIN32
#  include <win32thread.h>
#else
#  ifdef OLD_PTHREADS_API /* Here be dragons. */
#    define DETACH(t) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_detach(&(t)->self))) {		\
            MUTEX_UNLOCK(&(t)->mutex);				\
            Perl_croak_nocontext("panic: DETACH (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
        }							\
    } STMT_END

#    define PERL_GET_CONTEXT	Perl_get_context()
#    define PERL_SET_CONTEXT(t)	Perl_set_context((void*)t)

#    define PTHREAD_GETSPECIFIC_INT
#    ifdef OEMVS
#      define pthread_addr_t void *
#      define pthread_create(t,a,s,d)        pthread_create(t,&(a),s,d)
#      define pthread_keycreate              pthread_key_create
#    endif
#    ifdef VMS
#      define pthread_attr_init(a) pthread_attr_create(a)
#      define PTHREAD_ATTR_SETDETACHSTATE(a,s) pthread_setdetach_np(a,s)
#      define PTHREAD_CREATE(t,a,s,d) pthread_create(t,a,s,d)
#      define pthread_key_create(k,d) pthread_keycreate(k,(pthread_destructor_t)(d))
#      define pthread_mutexattr_init(a) pthread_mutexattr_create(a)
#      define pthread_mutexattr_settype(a,t) pthread_mutexattr_setkind_np(a,t)
#    endif
#    if defined(__hpux) && defined(__ux_version) && __ux_version <= 1020
#      define pthread_attr_init(a) pthread_attr_create(a)
       /* XXX pthread_setdetach_np() missing in DCE threads on HP-UX 10.20 */
#      define PTHREAD_ATTR_SETDETACHSTATE(a,s)	(0)
#      define PTHREAD_CREATE(t,a,s,d) pthread_create(t,a,s,d)
#      define pthread_key_create(k,d) pthread_keycreate(k,(pthread_destructor_t)(d))
#      define pthread_mutexattr_init(a) pthread_mutexattr_create(a)
#      define pthread_mutexattr_settype(a,t) pthread_mutexattr_setkind_np(a,t)
#    endif
#    if defined(OEMVS)
#      define PTHREAD_ATTR_SETDETACHSTATE(a,s) pthread_attr_setdetachstate(a,&(s))
#      define YIELD pthread_yield(NULL)
#    endif
#  endif
#  if !defined(__hpux) || !defined(__ux_version) || __ux_version > 1020
#    define pthread_mutexattr_default NULL
#    define pthread_condattr_default  NULL
#  endif
#endif

#ifndef PTHREAD_CREATE
/* You are not supposed to pass NULL as the 2nd arg of PTHREAD_CREATE(). */
#  define PTHREAD_CREATE(t,a,s,d) pthread_create(t,&(a),s,d)
#endif

#ifndef PTHREAD_ATTR_SETDETACHSTATE
#  define PTHREAD_ATTR_SETDETACHSTATE(a,s) pthread_attr_setdetachstate(a,s)
#endif

#ifndef PTHREAD_CREATE_JOINABLE
#  ifdef OLD_PTHREAD_CREATE_JOINABLE
#    define PTHREAD_CREATE_JOINABLE OLD_PTHREAD_CREATE_JOINABLE
#  else
#    define PTHREAD_CREATE_JOINABLE 0 /* Panic?  No, guess. */
#  endif
#endif

#ifdef __VMS
  /* Default is 1024 on VAX, 8192 otherwise */
#  ifdef __ia64
#    define THREAD_CREATE_NEEDS_STACK (48*1024)
#  else
#    define THREAD_CREATE_NEEDS_STACK (32*1024)
#  endif
#endif

#ifdef I_MACH_CTHREADS

/* cthreads interface */

/* #include <mach/cthreads.h> is in perl.h #ifdef I_MACH_CTHREADS */

#define MUTEX_INIT(m) \
    STMT_START {						\
        *m = mutex_alloc();					\
        if (*m) {						\
            mutex_init(*m);					\
        } else {						\
            Perl_croak_nocontext("panic: MUTEX_INIT [%s:%d]",	\
                                 __FILE__, __LINE__);		\
        }							\
    } STMT_END

#define MUTEX_LOCK(m)			mutex_lock(*m)
#define MUTEX_UNLOCK(m)			mutex_unlock(*m)
#define MUTEX_DESTROY(m) \
    STMT_START {						\
        mutex_free(*m);						\
        *m = 0;							\
    } STMT_END

#define COND_INIT(c) \
    STMT_START {						\
        *c = condition_alloc();					\
        if (*c) {						\
            condition_init(*c);					\
        }							\
        else {							\
            Perl_croak_nocontext("panic: COND_INIT [%s:%d]",	\
                                 __FILE__, __LINE__);		\
        }							\
    } STMT_END

#define COND_SIGNAL(c)		condition_signal(*c)
#define COND_BROADCAST(c)	condition_broadcast(*c)
#define COND_WAIT(c, m)		condition_wait(*c, *m)
#define COND_DESTROY(c) \
    STMT_START {						\
        condition_free(*c);					\
        *c = 0;							\
    } STMT_END

#define THREAD_RET_TYPE		any_t

#define DETACH(t)		cthread_detach(t->self)
#define JOIN(t, avp)		(*(avp) = MUTABLE_AV(cthread_join(t->self)))

#define PERL_SET_CONTEXT(t)	cthread_set_data(cthread_self(), t)
#define PERL_GET_CONTEXT	cthread_data(cthread_self())

#define INIT_THREADS		cthread_init()
#define YIELD			cthread_yield()
#define ALLOC_THREAD_KEY	NOOP
#define FREE_THREAD_KEY		NOOP
#define SET_THREAD_SELF(thr)	(thr->self = cthread_self())

#endif /* I_MACH_CTHREADS */

#ifndef YIELD
#  ifdef SCHED_YIELD
#    define YIELD SCHED_YIELD
#  elif defined(HAS_SCHED_YIELD)
#    define YIELD sched_yield()
#  elif defined(HAS_PTHREAD_YIELD)
    /* pthread_yield(NULL) platforms are expected
     * to have #defined YIELD for themselves. */
#    define YIELD pthread_yield()
#  endif
#endif

#ifdef __hpux
#  define MUTEX_INIT_NEEDS_MUTEX_ZEROED
#endif

#ifndef MUTEX_INIT

#  ifdef MUTEX_INIT_NEEDS_MUTEX_ZEROED
    /* Temporary workaround, true bug is deeper. --jhi 1999-02-25 */
#    define MUTEX_INIT(m) \
    STMT_START {                                                    \
        int _eC_;                                                   \
        Zero((m), 1, perl_mutex);                                   \
        if ((_eC_ = pthread_mutex_init((m), pthread_mutexattr_default)))\
            Perl_croak_nocontext("panic: MUTEX_INIT (%d) [%s:%d]",  \
                                 _eC_, __FILE__, __LINE__);         \
    } STMT_END
#  else
#    define MUTEX_INIT(m) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_mutex_init((m), pthread_mutexattr_default))) \
            Perl_croak_nocontext("panic: MUTEX_INIT (%d) [%s:%d]", \
                                 _eC_, __FILE__, __LINE__);     \
    } STMT_END
#  endif

#  ifdef PERL_TSA_ACTIVE
#    define perl_pthread_mutex_lock(m) perl_tsa_mutex_lock(m)
#    define perl_pthread_mutex_unlock(m) perl_tsa_mutex_unlock(m)
#  else
#    define perl_pthread_mutex_lock(m) pthread_mutex_lock(m)
#    define perl_pthread_mutex_unlock(m) pthread_mutex_unlock(m)
#  endif

#  define MUTEX_LOCK(m)                                         \
    STMT_START {						\
        dSAVE_ERRNO;                                            \
        int _eC_;						\
        if ((_eC_ = perl_pthread_mutex_lock((m))))		\
            Perl_croak_nocontext("panic: MUTEX_LOCK (%d) [%s:%d]",\
                                 _eC_, __FILE__, __LINE__);	\
        RESTORE_ERRNO;                                          \
    } STMT_END

#  define MUTEX_UNLOCK(m)                                       \
    STMT_START {						\
        dSAVE_ERRNO; /* Shouldn't be necessary as panics if fails */\
        int _eC_;						\
        if ((_eC_ = perl_pthread_mutex_unlock((m)))) {          \
            Perl_croak_nocontext(                               \
                            "panic: MUTEX_UNLOCK (%d) [%s:%d]", \
                                 _eC_, __FILE__, __LINE__);	\
        }                                                       \
        RESTORE_ERRNO;                                          \
    } STMT_END

#  define MUTEX_DESTROY(m)                                                  \
    STMT_START {						            \
        int _eC_;						            \
        if ((_eC_ = pthread_mutex_destroy((m)))) {                          \
            dTHX;                                                           \
            if (PL_phase != PERL_PHASE_DESTRUCT) {                          \
                Perl_croak_nocontext("panic: MUTEX_DESTROY (%d) [%s:%d]",   \
                                    _eC_, __FILE__, __LINE__);	            \
            }                                                               \
        }                                                                   \
    } STMT_END
#endif /* MUTEX_INIT */

#ifndef COND_INIT
#  define COND_INIT(c) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_cond_init((c), pthread_condattr_default)))	\
            Perl_croak_nocontext("panic: COND_INIT (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
    } STMT_END

#  define COND_SIGNAL(c) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_cond_signal((c))))			\
            Perl_croak_nocontext("panic: COND_SIGNAL (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
    } STMT_END

#  define COND_BROADCAST(c) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_cond_broadcast((c))))		\
            Perl_croak_nocontext("panic: COND_BROADCAST (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
    } STMT_END

#  define COND_WAIT(c, m) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_cond_wait((c), (m))))		\
            Perl_croak_nocontext("panic: COND_WAIT (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
    } STMT_END

#  define COND_DESTROY(c) \
    STMT_START {						            \
        int _eC_;						            \
        if ((_eC_ = pthread_cond_destroy((c)))) {                           \
            dTHX;                                                           \
            if (PL_phase != PERL_PHASE_DESTRUCT) {                          \
                Perl_croak_nocontext("panic: COND_DESTROY (%d) [%s:%d]",    \
                                    _eC_, __FILE__, __LINE__);	            \
            }                                                               \
        }                                                                   \
    } STMT_END
#endif /* COND_INIT */

#if defined(MUTEX_LOCK) && defined(MUTEX_UNLOCK)                \
 && defined(COND_SIGNAL) && defined(COND_WAIT)

/* These emulate native many-reader/1-writer locks.
 * Basically a locking reader just locks the semaphore long enough to increment
 * a counter; and similarly decrements it when when through.  Any writer will
 * run only when the count of readers is 0.  That is because it blocks on that
 * semaphore (doing a COND_WAIT) until it gets control of it, which won't
 * happen unless the count becomes 0.  ALL readers and other writers are then
 * blocked until it releases the semaphore.  The reader whose unlocking causes
 * the count to become 0 signals any waiting writers, and the system guarantees
 * that only one gets control at a time */

#  define PERL_READ_LOCK(mutex)                                     \
    STMT_START {                                                    \
        MUTEX_LOCK(&(mutex)->lock);                                 \
        (mutex)->readers_count++;                                   \
        MUTEX_UNLOCK(&(mutex)->lock);                               \
    } STMT_END

#  define PERL_READ_UNLOCK(mutex)                                   \
    STMT_START {                                                    \
        MUTEX_LOCK(&(mutex)->lock);                                 \
        (mutex)->readers_count--;                                   \
        if ((mutex)->readers_count <= 0) {                          \
            assert((mutex)->readers_count == 0);                    \
            COND_SIGNAL(&(mutex)->wakeup);                          \
            (mutex)->readers_count = 0;                             \
        }                                                           \
        MUTEX_UNLOCK(&(mutex)->lock);                               \
    } STMT_END

#  define PERL_WRITE_LOCK(mutex)                                    \
    STMT_START {                                                    \
        MUTEX_LOCK(&(mutex)->lock);                                 \
        do {                                                        \
            if ((mutex)->readers_count <= 0) {                      \
                assert((mutex)->readers_count == 0);                \
                (mutex)->readers_count = 0;                         \
                break;                                              \
            }                                                       \
            COND_WAIT(&(mutex)->wakeup, &(mutex)->lock);            \
        }                                                           \
        while (1);                                                  \
                                                                    \
        /* Here, the mutex is locked, with no readers */            \
    } STMT_END

#  define PERL_WRITE_UNLOCK(mutex)                                  \
    STMT_START {                                                    \
        COND_SIGNAL(&(mutex)->wakeup);                              \
        MUTEX_UNLOCK(&(mutex)->lock);                               \
    } STMT_END

#  define PERL_RW_MUTEX_INIT(mutex)                                 \
    STMT_START {                                                    \
        MUTEX_INIT(&(mutex)->lock);                                 \
        COND_INIT(&(mutex)->wakeup);                                \
        (mutex)->readers_count = 0;                                 \
    } STMT_END

#  define PERL_RW_MUTEX_DESTROY(mutex)                              \
    STMT_START {                                                    \
        COND_DESTROY(&(mutex)->wakeup);                             \
        MUTEX_DESTROY(&(mutex)->lock);                              \
    } STMT_END

#endif

/* DETACH(t) must only be called while holding t->mutex */
#ifndef DETACH
#  define DETACH(t) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_detach((t)->self))) {		\
            MUTEX_UNLOCK(&(t)->mutex);				\
            Perl_croak_nocontext("panic: DETACH (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
        }							\
    } STMT_END
#endif /* DETACH */

#ifndef JOIN
#  define JOIN(t, avp) \
    STMT_START {						\
        int _eC_;						\
        if ((_eC_ = pthread_join((t)->self, (void**)(avp))))	\
            Perl_croak_nocontext("panic: pthread_join (%d) [%s:%d]",	\
                                 _eC_, __FILE__, __LINE__);	\
    } STMT_END
#endif /* JOIN */

/* Use an unchecked fetch of thread-specific data instead of a checked one.
 * It would fail if the key were bogus, but if the key were bogus then
 * Really Bad Things would be happening anyway. --dan */
#if (defined(__ALPHA) && (__VMS_VER >= 70000000)) || \
    (defined(__alpha) && defined(__osf__) && !defined(__GNUC__)) /* Available only on >= 4.0 */
#  define HAS_PTHREAD_UNCHECKED_GETSPECIFIC_NP /* Configure test needed */
#endif

#ifdef HAS_PTHREAD_UNCHECKED_GETSPECIFIC_NP
#  define PTHREAD_GETSPECIFIC(key) pthread_unchecked_getspecific_np(key)
#else
#    define PTHREAD_GETSPECIFIC(key) pthread_getspecific(key)
#endif

#if defined(PERL_THREAD_LOCAL) && !defined(PERL_GET_CONTEXT) && !defined(PERL_SET_CONTEXT) && !defined(__cplusplus)
/* Use C11 thread-local storage, where possible.
 * Frustratingly we can't use it for C++ extensions, C++ and C disagree on the
 * syntax used for thread local storage, meaning that the working token that
 * Configure probed for C turns out to be a compiler error on C++. Great.
 * (Well, unless one or both is supporting non-standard syntax as an extension)
 * As Configure doesn't have a way to probe for C++ dialects, we just take the
 * safe option and do the same as 5.34.0 and earlier - use pthreads on C++.
 * Of course, if C++ XS extensions really want to avoid *all* this overhead,
 * they should #define PERL_NO_GET_CONTEXT and pass aTHX/aTHX_ explicitly) */
#  define PERL_USE_THREAD_LOCAL
extern PERL_THREAD_LOCAL void *PL_current_context;

#  define PERL_GET_CONTEXT        PL_current_context

/* We must also call pthread_setspecific() always, as C++ code has to read it
 * with pthreads (the #else side just below) */

#  define PERL_SET_CONTEXT(t)                                               \
    STMT_START {                                                            \
        int _eC_;                                                           \
        if ((_eC_ = pthread_setspecific(PL_thr_key,                         \
                                        PL_current_context = (void *)(t)))) \
            Perl_croak_nocontext("panic: pthread_setspecific (%d) [%s:%d]", \
                                 _eC_, __FILE__, __LINE__);                 \
        PERL_SET_NON_tTHX_CONTEXT(t);                                       \
    } STMT_END

#else
/* else fall back to pthreads */

#  ifndef PERL_GET_CONTEXT
#    define PERL_GET_CONTEXT	PTHREAD_GETSPECIFIC(PL_thr_key)
#  endif

/* For C++ extensions built on a system where the C compiler provides thread
 * local storage that call PERL_SET_CONTEXT() also need to set
 * PL_current_context, so need to call into C code to do this.
 * To avoid exploding code complexity, do this also on C platforms that don't
 * support thread local storage. PERL_SET_CONTEXT is not called that often. */

#  ifndef PERL_SET_CONTEXT
#    define PERL_SET_CONTEXT(t) Perl_set_context((void*)t)
#  endif /* PERL_SET_CONTEXT */
#endif /* PERL_THREAD_LOCAL */

#ifndef INIT_THREADS
#  ifdef NEED_PTHREAD_INIT
#    define INIT_THREADS pthread_init()
#  endif
#endif

#ifndef ALLOC_THREAD_KEY
#  define ALLOC_THREAD_KEY \
    STMT_START {						\
        if (pthread_key_create(&PL_thr_key, 0)) {		\
            PERL_UNUSED_RESULT(write(2, STR_WITH_LEN("panic: pthread_key_create failed\n"))); \
            exit(1);						\
        }							\
    } STMT_END
#endif

#ifndef FREE_THREAD_KEY
#  define FREE_THREAD_KEY \
    STMT_START {						\
        pthread_key_delete(PL_thr_key);				\
    } STMT_END
#endif

#ifndef PTHREAD_ATFORK
#  ifdef HAS_PTHREAD_ATFORK
#    define PTHREAD_ATFORK(prepare,parent,child)		\
        pthread_atfork(prepare,parent,child)
#  else
#    define PTHREAD_ATFORK(prepare,parent,child)		\
        NOOP
#  endif
#endif

#ifndef THREAD_RET_TYPE
#  define THREAD_RET_TYPE	void *
#endif /* THREAD_RET */

#  define LOCK_DOLLARZERO_MUTEX		MUTEX_LOCK(&PL_dollarzero_mutex)
#  define UNLOCK_DOLLARZERO_MUTEX	MUTEX_UNLOCK(&PL_dollarzero_mutex)

#endif /* USE_ITHREADS */

#ifndef MUTEX_LOCK
#  define MUTEX_LOCK(m)           NOOP
#endif

#ifndef MUTEX_UNLOCK
#  define MUTEX_UNLOCK(m)         NOOP
#endif

#ifndef MUTEX_INIT
#  define MUTEX_INIT(m)           NOOP
#endif

#ifndef MUTEX_DESTROY
#  define MUTEX_DESTROY(m)        NOOP
#endif

#ifndef COND_INIT
#  define COND_INIT(c)            NOOP
#endif

#ifndef COND_SIGNAL
#  define COND_SIGNAL(c)          NOOP
#endif

#ifndef COND_BROADCAST
#  define COND_BROADCAST(c)       NOOP
#endif

#ifndef COND_WAIT
#  define COND_WAIT(c, m)         NOOP
#endif

#ifndef COND_DESTROY
#  define COND_DESTROY(c)         NOOP
#endif

#ifndef PERL_READ_LOCK
#  define PERL_READ_LOCK          NOOP
#  define PERL_READ_UNLOCK        NOOP
#  define PERL_WRITE_LOCK         NOOP
#  define PERL_WRITE_UNLOCK       NOOP
#  define PERL_RW_MUTEX_INIT      NOOP
#  define PERL_RW_MUTEX_DESTROY   NOOP
#endif

#ifndef LOCK_DOLLARZERO_MUTEX
#  define LOCK_DOLLARZERO_MUTEX   NOOP
#endif

#ifndef UNLOCK_DOLLARZERO_MUTEX
#  define UNLOCK_DOLLARZERO_MUTEX NOOP
#endif

/* THR, SET_THR, and dTHR are there for compatibility with old versions */
#ifndef THR
#  define THR		PERL_GET_THX
#endif

#ifndef SET_THR
#  define SET_THR(t)	PERL_SET_THX(t)
#endif

#ifndef dTHR
#  define dTHR dNOOP
#endif

#ifndef INIT_THREADS
#  define INIT_THREADS NOOP
#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
