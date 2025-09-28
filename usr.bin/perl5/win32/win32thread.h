#ifndef _WIN32THREAD_H
#define _WIN32THREAD_H

#include "win32.h"

typedef struct win32_cond { LONG waiters; HANDLE sem; } perl_cond;
typedef DWORD perl_key;
typedef HANDLE perl_os_thread;

#ifndef DONT_USE_CRITICAL_SECTION

/* Critical Sections used instead of mutexes: lightweight,
 * but can't be communicated to child processes, and can't get
 * HANDLE to it for use elsewhere.
 */
typedef CRITICAL_SECTION perl_mutex;
#define MUTEX_INIT(m) InitializeCriticalSection(m)
#define MUTEX_LOCK(m) EnterCriticalSection(m)
#define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)

#else

typedef HANDLE perl_mutex;
#  define MUTEX_INIT(m) \
    STMT_START {						\
        if ((*(m) = CreateMutex(NULL,FALSE,NULL)) == NULL)	\
            Perl_croak_nocontext("panic: MUTEX_INIT");		\
    } STMT_END

#  define MUTEX_LOCK(m) \
    STMT_START {						\
        if (WaitForSingleObject(*(m),INFINITE) == WAIT_FAILED)	\
            Perl_croak_nocontext("panic: MUTEX_LOCK");		\
    } STMT_END

#  define MUTEX_UNLOCK(m) \
    STMT_START {						\
        if (ReleaseMutex(*(m)) == 0)				\
            Perl_croak_nocontext("panic: MUTEX_UNLOCK");	\
    } STMT_END

#  define MUTEX_DESTROY(m) \
    STMT_START {						\
        if (CloseHandle(*(m)) == 0)				\
            Perl_croak_nocontext("panic: MUTEX_DESTROY");	\
    } STMT_END

#endif

/* These macros assume that the mutex associated with the condition
 * will always be held before COND_{SIGNAL,BROADCAST,WAIT,DESTROY},
 * so there's no separate mutex protecting access to (c)->waiters
 */
#define COND_INIT(c) \
    STMT_START {						\
        (c)->waiters = 0;					\
        (c)->sem = Win_CreateSemaphore(NULL,0,LONG_MAX,NULL);	\
        if ((c)->sem == NULL)					\
            Perl_croak_nocontext("panic: COND_INIT (%ld)",GetLastError());	\
    } STMT_END

#define COND_SIGNAL(c) \
    STMT_START {						\
        if ((c)->waiters > 0 &&					\
            ReleaseSemaphore((c)->sem,1,NULL) == 0)		\
            Perl_croak_nocontext("panic: COND_SIGNAL (%ld)",GetLastError());	\
    } STMT_END

#define COND_BROADCAST(c) \
    STMT_START {						\
        if ((c)->waiters > 0 &&					\
            ReleaseSemaphore((c)->sem,(c)->waiters,NULL) == 0)	\
            Perl_croak_nocontext("panic: COND_BROADCAST (%ld)",GetLastError());\
    } STMT_END

#define COND_WAIT(c, m) \
    STMT_START {						\
        (c)->waiters++;						\
        MUTEX_UNLOCK(m);					\
        /* Note that there's no race here, since a		\
         * COND_BROADCAST() on another thread will have seen the\
         * right number of waiters (i.e. including this one) */	\
        if (WaitForSingleObject((c)->sem,INFINITE)==WAIT_FAILED)\
            Perl_croak_nocontext("panic: COND_WAIT (%ld)",GetLastError());	\
        /* XXX there may be an inconsequential race here */	\
        MUTEX_LOCK(m);						\
        (c)->waiters--;						\
    } STMT_END

#define COND_DESTROY(c) \
    STMT_START {						\
        (c)->waiters = 0;					\
        if (CloseHandle((c)->sem) == 0)				\
            Perl_croak_nocontext("panic: COND_DESTROY (%ld)",GetLastError());	\
    } STMT_END

#define DETACH(t) \
    STMT_START {						\
        if (CloseHandle((t)->self) == 0) {			\
            MUTEX_UNLOCK(&(t)->mutex);				\
            Perl_croak_nocontext("panic: DETACH");		\
        }							\
    } STMT_END


/* XXX Docs mention that the RTL versions of thread creation routines
 * should be used, but that advice only seems applicable when the RTL
 * is not in a DLL.  RTL DLLs seem to do all of the init/deinit required
 * upon DLL_THREAD_ATTACH/DETACH.  So we seem to be completely safe using
 * straight Win32 API calls, rather than the much braindamaged RTL calls.
 *
 * _beginthread() in the RTLs call CloseHandle() just after the thread
 * function returns, which means: 1) we have a race on our hands
 * 2) it is impossible to implement join() semantics.
 *
 * IOW, do *NOT* turn on USE_RTL_THREAD_API!  It is here
 * for experimental purposes only. GSAR 98-01-02
 */
#ifdef USE_RTL_THREAD_API
#  include <process.h>
#  if defined (_MSC_VER)
#    define THREAD_RET_TYPE	unsigned __stdcall
#  else
     /* CRTDLL.DLL doesn't allow a return value from thread function! */
#    define THREAD_RET_TYPE	void __cdecl
#  endif
#else	/* !USE_RTL_THREAD_API */
#  define THREAD_RET_TYPE	DWORD WINAPI
#endif	/* !USE_RTL_THREAD_API */

typedef THREAD_RET_TYPE thread_func_t(void *);


START_EXTERN_C

#if defined(PERLDLL) && defined(USE_DECLSPEC_THREAD)
extern __declspec(thread) void *PL_current_context;
#define PERL_SET_CONTEXT(t)   		(PL_current_context = t)
#define PERL_GET_CONTEXT		PL_current_context
#else
#define PERL_GET_CONTEXT		Perl_get_context()
#define PERL_SET_CONTEXT(t)		Perl_set_context(t)
#endif

END_EXTERN_C

#define INIT_THREADS		NOOP
#define ALLOC_THREAD_KEY \
    STMT_START {							\
        if ((PL_thr_key = TlsAlloc()) == TLS_OUT_OF_INDEXES) {		\
            PerlIO_printf(PerlIO_stderr(),"panic: TlsAlloc");				\
            exit(1);							\
        }								\
    } STMT_END

#define FREE_THREAD_KEY \
    STMT_START {							\
        TlsFree(PL_thr_key);						\
    } STMT_END

#define PTHREAD_ATFORK(prepare,parent,child)	NOOP

#if defined(USE_RTL_THREAD_API) && !defined(_MSC_VER)
#define JOIN(t, avp)							\
    STMT_START {							\
        if ((WaitForSingleObject((t)->self,INFINITE) == WAIT_FAILED)	\
             || (GetExitCodeThread((t)->self,(LPDWORD)(avp)) == 0)	\
             || (CloseHandle((t)->self) == 0))				\
            Perl_croak_nocontext("panic: JOIN");			\
        *avp = (AV *)((t)->i.retv);					\
    } STMT_END
#else	/* !USE_RTL_THREAD_API || _MSC_VER */
#define JOIN(t, avp)							\
    STMT_START {							\
        if ((WaitForSingleObject((t)->self,INFINITE) == WAIT_FAILED)	\
             || (GetExitCodeThread((t)->self,(LPDWORD)(avp)) == 0)	\
             || (CloseHandle((t)->self) == 0))				\
            Perl_croak_nocontext("panic: JOIN");			\
    } STMT_END
#endif	/* !USE_RTL_THREAD_API || _MSC_VER */

#define YIELD			Sleep(0)

#endif /* _WIN32THREAD_H */

