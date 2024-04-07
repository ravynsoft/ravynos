#define INCL_DOS
#define INCL_NOPM
#define INCL_DOSFILEMGR
#define INCL_DOSMEMMGR
#define INCL_DOSERRORS
#define INCL_WINERRORS
#define INCL_WINSYS
/* These 3 are needed for compile if os2.h includes os2tk.h, not os2emx.h */
#define INCL_DOSPROCESS
#define SPU_DISABLESUPPRESSION          0
#define SPU_ENABLESUPPRESSION           1
#include <os2.h>
#include "dlfcn.h"
#include <emx/syscalls.h>
#include <sys/emxload.h>

#include <sys/uflags.h>

/*
 * Various Unix compatibility functions for OS/2
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <process.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#define PERLIO_NOT_STDIO 0

#include "EXTERN.h"
#include "perl.h"

enum module_name_how { mod_name_handle, mod_name_shortname, mod_name_full,
  mod_name_C_function = 0x100, mod_name_HMODULE = 0x200};

/* Find module name to which *this* subroutine is compiled */
#define module_name(how)	module_name_at(&module_name_at, how)

static SV* module_name_at(void *pp, enum module_name_how how);

void
croak_with_os2error(char *s)
{
    Perl_croak_nocontext("%s: %s", s, os2error(Perl_rc));
}

struct PMWIN_entries_t PMWIN_entries;

/*****************************************************************************/
/* 2.1 would not resolve symbols on demand, and has no ExtLIBPATH. */

struct dll_handle_t {
    const char *modname;
    HMODULE handle;
    int requires_pm;
};

static struct dll_handle_t dll_handles[] = {
    {"doscalls", 0, 0},
    {"tcp32dll", 0, 0},
    {"pmwin", 0, 1},
    {"rexx", 0, 0},
    {"rexxapi", 0, 0},
    {"sesmgr", 0, 0},
    {"pmshapi", 0, 1},
    {"pmwp", 0, 1},
    {"pmgpi", 0, 1},
    {NULL, 0},
};

enum dll_handle_e {
    dll_handle_doscalls,
    dll_handle_tcp32dll,
    dll_handle_pmwin,
    dll_handle_rexx,
    dll_handle_rexxapi,
    dll_handle_sesmgr,
    dll_handle_pmshapi,
    dll_handle_pmwp,
    dll_handle_pmgpi,
    dll_handle_LAST,
};

#define doscalls_handle		(dll_handles[dll_handle_doscalls])
#define tcp_handle		(dll_handles[dll_handle_tcp32dll])
#define pmwin_handle		(dll_handles[dll_handle_pmwin])
#define rexx_handle		(dll_handles[dll_handle_rexx])
#define rexxapi_handle		(dll_handles[dll_handle_rexxapi])
#define sesmgr_handle		(dll_handles[dll_handle_sesmgr])
#define pmshapi_handle		(dll_handles[dll_handle_pmshapi])
#define pmwp_handle		(dll_handles[dll_handle_pmwp])
#define pmgpi_handle		(dll_handles[dll_handle_pmgpi])

/*  The following local-scope data is not yet included:
       fargs.140			// const => OK
       ino.165				// locked - and the access is almost cosmetic
       layout_table.260			// startup only, locked
       osv_res.257			// startup only, locked
       old_esp.254			// startup only, locked
       priors				// const ==> OK
       use_my_flock.283			// locked
       emx_init_done.268		// locked
       dll_handles			// locked
       hmtx_emx_init.267		// THIS is the lock for startup
       perlos2_state_mutex		// THIS is the lock for all the rest
BAD:
       perlos2_state			// see below
*/
/*  The following global-scope data is not yet included:
       OS2_Perl_data
       pthreads_states			// const now?
       start_thread_mutex
       thread_join_count		// protected
       thread_join_data			// protected
       tmppath

       pDosVerifyPidTid

       Perl_OS2_init3() - should it be protected?
*/
OS2_Perl_data_t OS2_Perl_data;

static struct perlos2_state_t {
  int po2__my_pwent;				/* = -1; */
  int po2_DOS_harderr_state;			/* = -1;    */
  signed char po2_DOS_suppression_state;	/* = -1;    */

  PFN po2_ExtFCN[ORD_NENTRIES];	/* Labeled by ord ORD_*. */
/*  struct PMWIN_entries_t po2_PMWIN_entries; */

  int po2_emx_wasnt_initialized;

  char po2_fname[9];
  int po2_rmq_cnt;

  int po2_grent_cnt;

  char *po2_newp;
  char *po2_oldp;
  int po2_newl;
  int po2_oldl;
  int po2_notfound;
  char po2_mangle_ret[STATIC_FILE_LENGTH+1];
  ULONG po2_os2_dll_fake;
  ULONG po2_os2_mytype;
  ULONG po2_os2_mytype_ini;
  int po2_pidtid_lookup;
  struct passwd po2_pw;

  int po2_pwent_cnt;
  char po2_pthreads_state_buf[80];
  char po2_os2error_buf[300];
/* There is no big sense to make it thread-specific, since signals 
   are delivered to thread 1 only.  XXXX Maybe make it into an array? */
  int po2_spawn_pid;
  int po2_spawn_killed;

  jmp_buf po2_at_exit_buf;
  int po2_longjmp_at_exit;
  int po2_emx_runtime_init;		/* If 1, we need to manually init it */
  int po2_emx_exception_init;		/* If 1, we need to manually set it */
  int po2_emx_runtime_secondary;
  char* (*po2_perllib_mangle_installed)(char *s, unsigned int l);
  char* po2_perl_sh_installed;
  PGINFOSEG po2_gTable;
  PLINFOSEG po2_lTable;
} perlos2_state = {
    -1,					/* po2__my_pwent */
    -1,					/* po2_DOS_harderr_state */
    -1,					/* po2_DOS_suppression_state */
};

#define Perl_po2()		(&perlos2_state)

#define ExtFCN			(Perl_po2()->po2_ExtFCN)
/* #define PMWIN_entries		(Perl_po2()->po2_PMWIN_entries) */
#define emx_wasnt_initialized	(Perl_po2()->po2_emx_wasnt_initialized)
#define fname			(Perl_po2()->po2_fname)
#define rmq_cnt			(Perl_po2()->po2_rmq_cnt)
#define grent_cnt		(Perl_po2()->po2_grent_cnt)
#define newp			(Perl_po2()->po2_newp)
#define oldp			(Perl_po2()->po2_oldp)
#define newl			(Perl_po2()->po2_newl)
#define oldl			(Perl_po2()->po2_oldl)
#define notfound		(Perl_po2()->po2_notfound)
#define mangle_ret		(Perl_po2()->po2_mangle_ret)
#define os2_dll_fake		(Perl_po2()->po2_os2_dll_fake)
#define os2_mytype		(Perl_po2()->po2_os2_mytype)
#define os2_mytype_ini		(Perl_po2()->po2_os2_mytype_ini)
#define pidtid_lookup		(Perl_po2()->po2_pidtid_lookup)
#define pw			(Perl_po2()->po2_pw)
#define pwent_cnt		(Perl_po2()->po2_pwent_cnt)
#define _my_pwent		(Perl_po2()->po2__my_pwent)
#define pthreads_state_buf	(Perl_po2()->po2_pthreads_state_buf)
#define os2error_buf		(Perl_po2()->po2_os2error_buf)
/* There is no big sense to make it thread-specific, since signals 
   are delivered to thread 1 only.  XXXX Maybe make it into an array? */
#define spawn_pid		(Perl_po2()->po2_spawn_pid)
#define spawn_killed		(Perl_po2()->po2_spawn_killed)
#define DOS_harderr_state	(Perl_po2()->po2_DOS_harderr_state)
#define DOS_suppression_state		(Perl_po2()->po2_DOS_suppression_state)

#define at_exit_buf		(Perl_po2()->po2_at_exit_buf)
#define longjmp_at_exit		(Perl_po2()->po2_longjmp_at_exit)
#define emx_runtime_init	(Perl_po2()->po2_emx_runtime_init)
#define emx_exception_init	(Perl_po2()->po2_emx_exception_init)
#define emx_runtime_secondary	(Perl_po2()->po2_emx_runtime_secondary)
#define perllib_mangle_installed	(Perl_po2()->po2_perllib_mangle_installed)
#define perl_sh_installed	(Perl_po2()->po2_perl_sh_installed)
#define gTable			(Perl_po2()->po2_gTable)
#define lTable			(Perl_po2()->po2_lTable)

const Perl_PFN * const pExtFCN = (Perl_po2()->po2_ExtFCN);

#if defined(USE_ITHREADS)

typedef void (*emx_startroutine)(void *);
typedef void* (*pthreads_startroutine)(void *);

enum pthreads_state {
    pthreads_st_none = 0, 
    pthreads_st_run,
    pthreads_st_exited, 
    pthreads_st_detached, 
    pthreads_st_waited,
    pthreads_st_norun,
    pthreads_st_exited_waited,
};
const char * const pthreads_states[] = {
    "uninit",
    "running",
    "exited",
    "detached",
    "waited for",
    "could not start",
    "exited, then waited on",
};

enum pthread_exists { pthread_not_existant = -0xff };

static const char*
pthreads_state_string(enum pthreads_state state)
{
  if (state < 0 || state >= sizeof(pthreads_states)/sizeof(*pthreads_states)) {
    snprintf(pthreads_state_buf, sizeof(pthreads_state_buf),
             "unknown thread state %d", (int)state);
    return pthreads_state_buf;
  }
  return pthreads_states[state];
}

typedef struct {
    void *status;
    perl_cond cond;
    enum pthreads_state state;
} thread_join_t;

thread_join_t *thread_join_data;
int thread_join_count;
perl_mutex start_thread_mutex;
static perl_mutex perlos2_state_mutex;


int
pthread_join(perl_os_thread tid, void **status)
{
    MUTEX_LOCK(&start_thread_mutex);
    if (tid < 1 || tid >= thread_join_count) {
        MUTEX_UNLOCK(&start_thread_mutex);
        if (tid != pthread_not_existant)
            Perl_croak_nocontext("panic: join with a thread with strange ordinal %d", (int)tid);
        Perl_warn_nocontext("panic: join with a thread which could not start");
        *status = 0;
        return 0;
    }
    switch (thread_join_data[tid].state) {
    case pthreads_st_exited:
        thread_join_data[tid].state = pthreads_st_exited_waited;
        *status = thread_join_data[tid].status;
        MUTEX_UNLOCK(&start_thread_mutex);
        COND_SIGNAL(&thread_join_data[tid].cond);    
        break;
    case pthreads_st_waited:
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("join with a thread with a waiter");
        break;
    case pthreads_st_norun:
    {
        int state = (int)thread_join_data[tid].status;

        thread_join_data[tid].state = pthreads_st_none;
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("panic: join with a thread which could not run"
                             " due to attempt of tid reuse (state='%s')",
                             pthreads_state_string(state));
        break;
    }
    case pthreads_st_run:
    {
        perl_cond cond;

        thread_join_data[tid].state = pthreads_st_waited;
        thread_join_data[tid].status = (void *)status;
        COND_INIT(&thread_join_data[tid].cond);
        cond = thread_join_data[tid].cond;
        COND_WAIT(&thread_join_data[tid].cond, &start_thread_mutex);
        COND_DESTROY(&cond);
        MUTEX_UNLOCK(&start_thread_mutex);
        break;
    }
    default:
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("panic: join with thread in unknown thread state: '%s'", 
              pthreads_state_string(thread_join_data[tid].state));
        break;
    }
    return 0;
}

typedef struct {
  pthreads_startroutine sub;
  void *arg;
  void *ctx;
} pthr_startit;

/* The lock is used:
        a) Since we temporarily usurp the caller interp, so malloc() may
           use it to decide on debugging the call;
        b) Since *args is on the caller's stack.
 */
void
pthread_startit(void *arg1)
{
    /* Thread is already started, we need to transfer control only */
    pthr_startit args = *(pthr_startit *)arg1;
    int tid = pthread_self();
    void *rc;
    int state;

    if (tid <= 1) {
        /* Can't croak, the setjmp() is not in scope... */
        char buf[80];

        snprintf(buf, sizeof(buf),
                 "panic: thread with strange ordinal %d created\n\r", tid);
        write(2,buf,strlen(buf));
        MUTEX_UNLOCK(&start_thread_mutex);
        return;
    }
    /* Until args.sub resets it, makes debugging Perl_malloc() work: */
    PERL_SET_CONTEXT(0);
    if (tid >= thread_join_count) {
        int oc = thread_join_count;
        
        thread_join_count = tid + 5 + tid/5;
        if (thread_join_data) {
            Renew(thread_join_data, thread_join_count, thread_join_t);
            Zero(thread_join_data + oc, thread_join_count - oc, thread_join_t);
        } else {
            Newxz(thread_join_data, thread_join_count, thread_join_t);
        }
    }
    if (thread_join_data[tid].state != pthreads_st_none) {
        /* Can't croak, the setjmp() is not in scope... */
        char buf[80];

        snprintf(buf, sizeof(buf),
                 "panic: attempt to reuse thread id %d (state='%s')\n\r",
                 tid, pthreads_state_string(thread_join_data[tid].state));
        write(2,buf,strlen(buf));
        thread_join_data[tid].status = (void*)thread_join_data[tid].state;
        thread_join_data[tid].state = pthreads_st_norun;
        MUTEX_UNLOCK(&start_thread_mutex);
        return;
    }
    thread_join_data[tid].state = pthreads_st_run;
    /* Now that we copied/updated the guys, we may release the caller... */
    MUTEX_UNLOCK(&start_thread_mutex);
    rc = (*args.sub)(args.arg);
    MUTEX_LOCK(&start_thread_mutex);
    switch (thread_join_data[tid].state) {
    case pthreads_st_waited:
        COND_SIGNAL(&thread_join_data[tid].cond);
        thread_join_data[tid].state = pthreads_st_none;
        *((void**)thread_join_data[tid].status) = rc;
        break;
    case pthreads_st_detached:
        thread_join_data[tid].state = pthreads_st_none;
        break;
    case pthreads_st_run:
        /* Somebody can wait on us; cannot exit, since OS can reuse the tid
           and our waiter will get somebody else's status. */
        thread_join_data[tid].state = pthreads_st_exited;
        thread_join_data[tid].status = rc;
        COND_INIT(&thread_join_data[tid].cond);
        COND_WAIT(&thread_join_data[tid].cond, &start_thread_mutex);
        COND_DESTROY(&thread_join_data[tid].cond);
        thread_join_data[tid].state = pthreads_st_none;	/* Ready to reuse */
        break;
    default:
        state = thread_join_data[tid].state;
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("panic: unexpected thread state on exit: '%s'",
                             pthreads_state_string(state));
    }
    MUTEX_UNLOCK(&start_thread_mutex);
}

int
pthread_create(perl_os_thread *tidp, const pthread_attr_t *attr, 
               void *(*start_routine)(void*), void *arg)
{
    dTHX;
    pthr_startit args;

    args.sub = (void*)start_routine;
    args.arg = arg;
    args.ctx = PERL_GET_CONTEXT;

    MUTEX_LOCK(&start_thread_mutex);
    /* Test suite creates 31 extra threads;
       on machine without shared-memory-hogs this stack sizeis OK with 31: */
    *tidp = _beginthread(pthread_startit, /*stack*/ NULL, 
                         /*stacksize*/ 4*1024*1024, (void*)&args);
    if (*tidp == -1) {
        *tidp = pthread_not_existant;
        MUTEX_UNLOCK(&start_thread_mutex);
        return EINVAL;
    }
    MUTEX_LOCK(&start_thread_mutex);		/* Wait for init to proceed */
    MUTEX_UNLOCK(&start_thread_mutex);
    return 0;
}

int 
pthread_detach(perl_os_thread tid)
{
    MUTEX_LOCK(&start_thread_mutex);
    if (tid < 1 || tid >= thread_join_count) {
        MUTEX_UNLOCK(&start_thread_mutex);
        if (tid != pthread_not_existant)
            Perl_croak_nocontext("panic: detach of a thread with strange ordinal %d", (int)tid);
        Perl_warn_nocontext("detach of a thread which could not start");
        return 0;
    }
    switch (thread_join_data[tid].state) {
    case pthreads_st_waited:
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("detach on a thread with a waiter");
        break;
    case pthreads_st_run:
        thread_join_data[tid].state = pthreads_st_detached;
        MUTEX_UNLOCK(&start_thread_mutex);
        break;
    case pthreads_st_exited:
        MUTEX_UNLOCK(&start_thread_mutex);
        COND_SIGNAL(&thread_join_data[tid].cond);    
        break;
    case pthreads_st_detached:
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_warn_nocontext("detach on an already detached thread");
        break;
    case pthreads_st_norun:
    {
        int state = (int)thread_join_data[tid].status;

        thread_join_data[tid].state = pthreads_st_none;
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("panic: detaching thread which could not run"
                             " due to attempt of tid reuse (state='%s')",
                             pthreads_state_string(state));
        break;
    }
    default:
        MUTEX_UNLOCK(&start_thread_mutex);
        Perl_croak_nocontext("panic: detach of a thread with unknown thread state: '%s'", 
              pthreads_state_string(thread_join_data[tid].state));
        break;
    }
    return 0;
}

/* This is a very bastardized version; may be OK due to edge trigger of Wait */
int
os2_cond_wait(perl_cond *c, perl_mutex *m)
{						
    int rc;
    STRLEN n_a;
    if ((rc = DosResetEventSem(*c,&n_a)) && (rc != ERROR_ALREADY_RESET))
        Perl_rc = CheckOSError(rc), croak_with_os2error("panic: COND_WAIT-reset");
    if (m) MUTEX_UNLOCK(m);					
    if (CheckOSError(DosWaitEventSem(*c,SEM_INDEFINITE_WAIT))
        && (rc != ERROR_INTERRUPT))
        croak_with_os2error("panic: COND_WAIT");		
    if (rc == ERROR_INTERRUPT)
        errno = EINTR;
    if (m) MUTEX_LOCK(m);
    return 0;
} 
#endif

static int exe_is_aout(void);

/* This should match enum entries_ordinals defined in os2ish.h. */
static const struct {
    struct dll_handle_t *dll;
    const char *entryname;
    int entrypoint;
} loadOrdinals[] = {
  {&doscalls_handle, NULL, 874},	/* DosQueryExtLibpath */
  {&doscalls_handle, NULL, 873},	/* DosSetExtLibpath */
  {&doscalls_handle, NULL, 460},	/* DosVerifyPidTid */
  {&tcp_handle, "SETHOSTENT", 0},
  {&tcp_handle, "SETNETENT" , 0},
  {&tcp_handle, "SETPROTOENT", 0},
  {&tcp_handle, "SETSERVENT", 0},
  {&tcp_handle, "GETHOSTENT", 0},
  {&tcp_handle, "GETNETENT" , 0},
  {&tcp_handle, "GETPROTOENT", 0},
  {&tcp_handle, "GETSERVENT", 0},
  {&tcp_handle, "ENDHOSTENT", 0},
  {&tcp_handle, "ENDNETENT", 0},
  {&tcp_handle, "ENDPROTOENT", 0},
  {&tcp_handle, "ENDSERVENT", 0},
  {&pmwin_handle, NULL, 763},		/* WinInitialize */
  {&pmwin_handle, NULL, 716},		/* WinCreateMsgQueue */
  {&pmwin_handle, NULL, 726},		/* WinDestroyMsgQueue */
  {&pmwin_handle, NULL, 918},		/* WinPeekMsg */
  {&pmwin_handle, NULL, 915},		/* WinGetMsg */
  {&pmwin_handle, NULL, 912},		/* WinDispatchMsg */
  {&pmwin_handle, NULL, 753},		/* WinGetLastError */
  {&pmwin_handle, NULL, 705},		/* WinCancelShutdown */
        /* These are needed in extensions.
           How to protect PMSHAPI: it comes through EMX functions? */
  {&rexx_handle,    "RexxStart", 0},
  {&rexx_handle,    "RexxVariablePool", 0},
  {&rexxapi_handle, "RexxRegisterFunctionExe", 0},
  {&rexxapi_handle, "RexxDeregisterFunction", 0},
  {&sesmgr_handle,  "DOSSMSETTITLE", 0}, /* Would not work runtime-loaded */
  {&pmshapi_handle, "PRF32QUERYPROFILESIZE", 0},
  {&pmshapi_handle, "PRF32OPENPROFILE", 0},
  {&pmshapi_handle, "PRF32CLOSEPROFILE", 0},
  {&pmshapi_handle, "PRF32QUERYPROFILE", 0},
  {&pmshapi_handle, "PRF32RESET", 0},
  {&pmshapi_handle, "PRF32QUERYPROFILEDATA", 0},
  {&pmshapi_handle, "PRF32WRITEPROFILEDATA", 0},

  /* At least some of these do not work by name, since they need
        WIN32 instead of WIN... */
#if 0
  These were generated with
    nm I:\emx\lib\os2.a  | fgrep -f API-list | grep = > API-list-entries
    perl -wnle "next unless /^0+\s+E\s+_(\w+)=(\w+).(\d+)/; print qq(    ORD_$1,)" API-list-entries > API-list-ORD_
    perl -wnle "next unless /^0+\s+E\s+_(\w+)=(\w+).(\d+)/; print qq(  {${2}_handle, NULL, $3},\t\t/* $1 */)" WinSwitch-API-list-entries  >API-list-entry
#endif
  {&pmshapi_handle, NULL, 123},		/* WinChangeSwitchEntry */
  {&pmshapi_handle, NULL, 124},		/* WinQuerySwitchEntry */
  {&pmshapi_handle, NULL, 125},		/* WinQuerySwitchHandle */
  {&pmshapi_handle, NULL, 126},		/* WinQuerySwitchList */
  {&pmshapi_handle, NULL, 131},		/* WinSwitchToProgram */
  {&pmwin_handle, NULL, 702},		/* WinBeginEnumWindows */
  {&pmwin_handle, NULL, 737},		/* WinEndEnumWindows */
  {&pmwin_handle, NULL, 740},		/* WinEnumDlgItem */
  {&pmwin_handle, NULL, 756},		/* WinGetNextWindow */
  {&pmwin_handle, NULL, 768},		/* WinIsChild */
  {&pmwin_handle, NULL, 799},		/* WinQueryActiveWindow */
  {&pmwin_handle, NULL, 805},		/* WinQueryClassName */
  {&pmwin_handle, NULL, 817},		/* WinQueryFocus */
  {&pmwin_handle, NULL, 834},		/* WinQueryWindow */
  {&pmwin_handle, NULL, 837},		/* WinQueryWindowPos */
  {&pmwin_handle, NULL, 838},		/* WinQueryWindowProcess */
  {&pmwin_handle, NULL, 841},		/* WinQueryWindowText */
  {&pmwin_handle, NULL, 842},		/* WinQueryWindowTextLength */
  {&pmwin_handle, NULL, 860},		/* WinSetFocus */
  {&pmwin_handle, NULL, 875},		/* WinSetWindowPos */
  {&pmwin_handle, NULL, 877},		/* WinSetWindowText */
  {&pmwin_handle, NULL, 883},		/* WinShowWindow */
  {&pmwin_handle, NULL, 772},		/* WinIsWindow */
  {&pmwin_handle, NULL, 899},		/* WinWindowFromId */
  {&pmwin_handle, NULL, 900},		/* WinWindowFromPoint */
  {&pmwin_handle, NULL, 919},		/* WinPostMsg */
  {&pmwin_handle, NULL, 735},		/* WinEnableWindow */
  {&pmwin_handle, NULL, 736},		/* WinEnableWindowUpdate */
  {&pmwin_handle, NULL, 773},		/* WinIsWindowEnabled */
  {&pmwin_handle, NULL, 774},		/* WinIsWindowShowing */
  {&pmwin_handle, NULL, 775},		/* WinIsWindowVisible */
  {&pmwin_handle, NULL, 839},		/* WinQueryWindowPtr */
  {&pmwin_handle, NULL, 843},		/* WinQueryWindowULong */
  {&pmwin_handle, NULL, 844},		/* WinQueryWindowUShort */
  {&pmwin_handle, NULL, 874},		/* WinSetWindowBits */
  {&pmwin_handle, NULL, 876},		/* WinSetWindowPtr */
  {&pmwin_handle, NULL, 878},		/* WinSetWindowULong */
  {&pmwin_handle, NULL, 879},		/* WinSetWindowUShort */
  {&pmwin_handle, NULL, 813},		/* WinQueryDesktopWindow */
  {&pmwin_handle, NULL, 851},		/* WinSetActiveWindow */
  {&doscalls_handle, NULL, 360},	/* DosQueryModFromEIP */
  {&doscalls_handle, NULL, 582},	/* Dos32QueryHeaderInfo */
  {&doscalls_handle, NULL, 362},	/* DosTmrQueryFreq */
  {&doscalls_handle, NULL, 363},	/* DosTmrQueryTime */
  {&pmwp_handle, NULL, 262},		/* WinQueryActiveDesktopPathname */
  {&pmwin_handle, NULL, 765},		/* WinInvalidateRect */
  {&pmwin_handle, NULL, 906},		/* WinCreateFrameControl */
  {&pmwin_handle, NULL, 807},		/* WinQueryClipbrdFmtInfo */
  {&pmwin_handle, NULL, 808},		/* WinQueryClipbrdOwner */
  {&pmwin_handle, NULL, 809},		/* WinQueryClipbrdViewer */
  {&pmwin_handle, NULL, 806},		/* WinQueryClipbrdData */
  {&pmwin_handle, NULL, 793},		/* WinOpenClipbrd */
  {&pmwin_handle, NULL, 707},		/* WinCloseClipbrd */
  {&pmwin_handle, NULL, 854},		/* WinSetClipbrdData */
  {&pmwin_handle, NULL, 855},		/* WinSetClipbrdOwner */
  {&pmwin_handle, NULL, 856},		/* WinSetClipbrdViewer */
  {&pmwin_handle, NULL, 739},		/* WinEnumClipbrdFmts  */
  {&pmwin_handle, NULL, 733},		/* WinEmptyClipbrd */
  {&pmwin_handle, NULL, 700},		/* WinAddAtom */
  {&pmwin_handle, NULL, 744},		/* WinFindAtom */
  {&pmwin_handle, NULL, 721},		/* WinDeleteAtom */
  {&pmwin_handle, NULL, 803},		/* WinQueryAtomUsage */
  {&pmwin_handle, NULL, 802},		/* WinQueryAtomName */
  {&pmwin_handle, NULL, 801},		/* WinQueryAtomLength */
  {&pmwin_handle, NULL, 830},		/* WinQuerySystemAtomTable */
  {&pmwin_handle, NULL, 714},		/* WinCreateAtomTable */
  {&pmwin_handle, NULL, 724},		/* WinDestroyAtomTable */
  {&pmwin_handle, NULL, 794},		/* WinOpenWindowDC */
  {&pmgpi_handle, NULL, 610},		/* DevOpenDC */
  {&pmgpi_handle, NULL, 606},		/* DevQueryCaps */
  {&pmgpi_handle, NULL, 604},		/* DevCloseDC */
  {&pmwin_handle, NULL, 789},		/* WinMessageBox */
  {&pmwin_handle, NULL, 1015},		/* WinMessageBox2 */
  {&pmwin_handle, NULL, 829},		/* WinQuerySysValue */
  {&pmwin_handle, NULL, 873},		/* WinSetSysValue */
  {&pmwin_handle, NULL, 701},		/* WinAlarm */
  {&pmwin_handle, NULL, 745},		/* WinFlashWindow */
  {&pmwin_handle, NULL, 780},		/* WinLoadPointer */
  {&pmwin_handle, NULL, 828},		/* WinQuerySysPointer */
  {&doscalls_handle, NULL, 417},	/* DosReplaceModule */
  {&doscalls_handle, NULL, 976},	/* DosPerfSysCall */
  {&rexxapi_handle, "RexxRegisterSubcomExe", 0},
};

HMODULE
loadModule(const char *modname, int fail)
{
    HMODULE h = (HMODULE)dlopen(modname, 0);

    if (!h && fail)
        Perl_croak_nocontext("Error loading module '%s': %s", 
                             modname, dlerror());
    return h;
}

/* const char* const ptypes[] = { "FS", "DOS", "VIO", "PM", "DETACH" }; */

static int
my_type()
{
    int rc;
    TIB *tib;
    PIB *pib;
    
    if (!(_emx_env & 0x200)) return 1; /* not OS/2. */
    if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) 
        return -1; 
    
    return (pib->pib_ultype);
}

static void
my_type_set(int type)
{
    int rc;
    TIB *tib;
    PIB *pib;
    
    if (!(_emx_env & 0x200))
        Perl_croak_nocontext("Can't set type on DOS"); /* not OS/2. */
    if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) 
        croak_with_os2error("Error getting info blocks");
    pib->pib_ultype = type;
}

PFN
loadByOrdinal(enum entries_ordinals ord, int fail)
{
    if (sizeof(loadOrdinals)/sizeof(loadOrdinals[0]) != ORD_NENTRIES)
            Perl_croak_nocontext(
                 "Wrong size of loadOrdinals array: expected %d, actual %d", 
                 sizeof(loadOrdinals)/sizeof(loadOrdinals[0]), ORD_NENTRIES);
    if (ExtFCN[ord] == NULL) {
        PFN fcn = (PFN)-1;
        APIRET rc;

        if (!loadOrdinals[ord].dll->handle) {
            if (loadOrdinals[ord].dll->requires_pm && my_type() < 2) { /* FS */
                char *s = PerlEnv_getenv("PERL_ASIF_PM");

                if (!s || !atoi(s)) {
                    /* The module will not function well without PM.
                       The usual way to detect PM is the existence of the mutex
                       \SEM32\PMDRAG.SEM. */
                    HMTX hMtx = 0;

                    if (CheckOSError(DosOpenMutexSem("\\SEM32\\PMDRAG.SEM",
                                                     &hMtx)))
                        Perl_croak_nocontext("Looks like we have no PM; will not load DLL %s without $ENV{PERL_ASIF_PM}",
                                             loadOrdinals[ord].dll->modname);
                    DosCloseMutexSem(hMtx);
                }
            }
            MUTEX_LOCK(&perlos2_state_mutex);
            loadOrdinals[ord].dll->handle
                = loadModule(loadOrdinals[ord].dll->modname, fail);
            MUTEX_UNLOCK(&perlos2_state_mutex);
        }
        if (!loadOrdinals[ord].dll->handle)
            return 0;			/* Possible with FAIL==0 only */
        if (CheckOSError(DosQueryProcAddr(loadOrdinals[ord].dll->handle,
                                          loadOrdinals[ord].entrypoint,
                                          loadOrdinals[ord].entryname,&fcn))) {
            char buf[20], *s = (char*)loadOrdinals[ord].entryname;

            if (!fail)
                return 0;
            if (!s)
                sprintf(s = buf, "%d", loadOrdinals[ord].entrypoint);
            Perl_croak_nocontext(
                 "This version of OS/2 does not support %s.%s", 
                 loadOrdinals[ord].dll->modname, s);
        }
        ExtFCN[ord] = fcn;
    } 
    if ((long)ExtFCN[ord] == -1)
        Perl_croak_nocontext("panic queryaddr");
    return ExtFCN[ord];
}

void 
init_PMWIN_entries(void)
{
    int i;

    for (i = ORD_WinInitialize; i <= ORD_WinCancelShutdown; i++)
        ((PFN*)&PMWIN_entries)[i - ORD_WinInitialize] = loadByOrdinal(i, 1);
}

/*****************************************************/
/* socket forwarders without linking with tcpip DLLs */

DeclFuncByORD(struct hostent *,  gethostent,  ORD_GETHOSTENT,  (void), ())
DeclFuncByORD(struct netent  *,  getnetent,   ORD_GETNETENT,   (void), ())
DeclFuncByORD(struct protoent *, getprotoent, ORD_GETPROTOENT, (void), ())
DeclFuncByORD(struct servent *,  getservent,  ORD_GETSERVENT,  (void), ())

DeclVoidFuncByORD(sethostent,  ORD_SETHOSTENT,  (int x), (x))
DeclVoidFuncByORD(setnetent,   ORD_SETNETENT,   (int x), (x))
DeclVoidFuncByORD(setprotoent, ORD_SETPROTOENT, (int x), (x))
DeclVoidFuncByORD(setservent,  ORD_SETSERVENT,  (int x), (x))

DeclVoidFuncByORD(endhostent,  ORD_ENDHOSTENT,  (void), ())
DeclVoidFuncByORD(endnetent,   ORD_ENDNETENT,   (void), ())
DeclVoidFuncByORD(endprotoent, ORD_ENDPROTOENT, (void), ())
DeclVoidFuncByORD(endservent,  ORD_ENDSERVENT,  (void), ())

/* priorities */
static const signed char priors[] = {0, 1, 3, 2}; /* Last two interchanged,
                                                     self inverse. */
#define QSS_INI_BUFFER 1024

ULONG (*pDosVerifyPidTid) (PID pid, TID tid);

PQTOPLEVEL
get_sysinfo(ULONG pid, ULONG flags)
{
    char *pbuffer;
    ULONG rc, buf_len = QSS_INI_BUFFER;
    PQTOPLEVEL psi;

    if (pid) {
        if (!pidtid_lookup) {
            pidtid_lookup = 1;
            *(PFN*)&pDosVerifyPidTid = loadByOrdinal(ORD_DosVerifyPidTid, 0);
        }
        if (pDosVerifyPidTid) {	/* Warp3 or later */
            /* Up to some fixpak QuerySysState() kills the system if a non-existent
               pid is used. */
            if (CheckOSError(pDosVerifyPidTid(pid, 1)))
                return 0;
        }
    }
    Newx(pbuffer, buf_len, char);
    /* QSS_PROCESS | QSS_MODULE | QSS_SEMAPHORES | QSS_SHARED */
    rc = QuerySysState(flags, pid, pbuffer, buf_len);
    while (rc == ERROR_BUFFER_OVERFLOW) {
        Renew(pbuffer, buf_len *= 2, char);
        rc = QuerySysState(flags, pid, pbuffer, buf_len);
    }
    if (rc) {
        FillOSError(rc);
        Safefree(pbuffer);
        return 0;
    }
    psi = (PQTOPLEVEL)pbuffer;
    if (psi && pid && psi->procdata && pid != psi->procdata->pid) {
      Safefree(psi);
      Perl_croak_nocontext("panic: wrong pid in sysinfo");
    }
    return psi;
}

#define PRIO_ERR 0x1111

static ULONG
sys_prio(pid)
{
  ULONG prio;
  PQTOPLEVEL psi;

  if (!pid)
      return PRIO_ERR;
  psi = get_sysinfo(pid, QSS_PROCESS);
  if (!psi)
      return PRIO_ERR;
  prio = psi->procdata->threads->priority;
  Safefree(psi);
  return prio;
}

int 
setpriority(int which, int pid, int val)
{
  ULONG rc, prio = sys_prio(pid);

  if (!(_emx_env & 0x200)) return 0; /* Nop if not OS/2. */
  if (priors[(32 - val) >> 5] + 1 == (prio >> 8)) {
      /* Do not change class. */
      return CheckOSError(DosSetPriority((pid < 0) 
                                         ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
                                         0, 
                                         (32 - val) % 32 - (prio & 0xFF), 
                                         abs(pid)))
      ? -1 : 0;
  } else /* if ((32 - val) % 32 == (prio & 0xFF)) */ {
      /* Documentation claims one can change both class and basevalue,
       * but I find it wrong. */
      /* Change class, but since delta == 0 denotes absolute 0, correct. */
      if (CheckOSError(DosSetPriority((pid < 0) 
                                      ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
                                      priors[(32 - val) >> 5] + 1, 
                                      0, 
                                      abs(pid)))) 
          return -1;
      if ( ((32 - val) % 32) == 0 ) return 0;
      return CheckOSError(DosSetPriority((pid < 0) 
                                         ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
                                         0, 
                                         (32 - val) % 32, 
                                         abs(pid)))
          ? -1 : 0;
  } 
}

int 
getpriority(int which /* ignored */, int pid)
{
  ULONG ret;

  if (!(_emx_env & 0x200)) return 0; /* Nop if not OS/2. */
  ret = sys_prio(pid);
  if (ret == PRIO_ERR) {
      return -1;
  }
  return (1 - priors[((ret >> 8) - 1)])*32 - (ret & 0xFF);
}

/*****************************************************************************/
/* spawn */



static Signal_t
spawn_sighandler(int sig)
{
    /* Some programs do not arrange for the keyboard signals to be
       delivered to them.  We need to deliver the signal manually. */
    /* We may get a signal only if 
       a) kid does not receive keyboard signal: deliver it;
       b) kid already died, and we get a signal.  We may only hope
          that the pid number was not reused.
     */
    
    if (spawn_killed) 
        sig = SIGKILL;			/* Try harder. */
    kill(spawn_pid, sig);
    spawn_killed = 1;
}

static int
result(pTHX_ int flag, int pid)
{
        int r, status;
        Signal_t (*ihand)();     /* place to save signal during system() */
        Signal_t (*qhand)();     /* place to save signal during system() */
#ifndef __EMX__
        RESULTCODES res;
        int rpid;
#endif

        if (pid < 0 || flag != 0)
                return pid;

#ifdef __EMX__
        spawn_pid = pid;
        spawn_killed = 0;
        ihand = rsignal(SIGINT, &spawn_sighandler);
        qhand = rsignal(SIGQUIT, &spawn_sighandler);
        do {
            r = wait4pid(pid, &status, 0);
        } while (r == -1 && errno == EINTR);
        rsignal(SIGINT, ihand);
        rsignal(SIGQUIT, qhand);

        PL_statusvalue = (U16)status;
        if (r < 0)
                return -1;
        return status & 0xFFFF;
#else
        ihand = rsignal(SIGINT, SIG_IGN);
        r = DosWaitChild(DCWA_PROCESS, DCWW_WAIT, &res, &rpid, pid);
        rsignal(SIGINT, ihand);
        PL_statusvalue = res.codeResult << 8 | res.codeTerminate;
        if (r)
                return -1;
        return PL_statusvalue;
#endif
}

enum execf_t {
  EXECF_SPAWN,
  EXECF_EXEC,
  EXECF_TRUEEXEC,
  EXECF_SPAWN_NOWAIT,
  EXECF_SPAWN_BYFLAG,
  EXECF_SYNC
};

static ULONG
file_type(char *path)
{
    int rc;
    ULONG apptype;
    
    if (!(_emx_env & 0x200)) 
        Perl_croak_nocontext("file_type not implemented on DOS"); /* not OS/2. */
    if (CheckOSError(DosQueryAppType(path, &apptype))) {
        switch (rc) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return -1;
        case ERROR_ACCESS_DENIED:	/* Directory with this name found? */
            return -3;
        default:			/* Found, but not an
                                           executable, or some other
                                           read error. */
            return -2;
        }
    }    
    return apptype;
}

/* Spawn/exec a program, revert to shell if needed. */

extern ULONG _emx_exception (	EXCEPTIONREPORTRECORD *,
                                EXCEPTIONREGISTRATIONRECORD *,
                                CONTEXTRECORD *,
                                void *);

int
do_spawn_ve(pTHX_ SV *really, const char **argv, U32 flag, U32 execf, char *inicmd, U32 addflag)
{
        int trueflag = flag;
        int rc, pass = 1;
        char *real_name = NULL;			/* Shut down the warning */
        char const * args[4];
        static const char * const fargs[4] 
            = { "/bin/sh", "-c", "\"$@\"", "spawn-via-shell", };
        const char * const *argsp = fargs;
        int nargs = 4;
        int force_shell;
        int new_stderr = -1, nostderr = 0;
        int fl_stderr = 0;
        STRLEN n_a;
        char *buf;
        PerlIO *file;
        
        if (flag == P_WAIT)
                flag = P_NOWAIT;
        if (really) {
            real_name = SvPV(really, n_a);
            real_name = savepv(real_name);
            SAVEFREEPV(real_name);
            if (!*real_name)
                really = NULL;
        }

      retry:
        if (strEQ(argv[0],"/bin/sh")) 
            argv[0] = PL_sh_path;

        /* We should check PERL_SH* and PERLLIB_* as well? */
        if (!really || pass >= 2)
            real_name = argv[0];
        if (real_name[0] != '/' && real_name[0] != '\\'
            && !(real_name[0] && real_name[1] == ':' 
                 && (real_name[2] == '/' || real_name[2] != '\\'))
            ) /* will spawnvp use PATH? */
            TAINT_ENV();	/* testing IFS here is overkill, probably */

      reread:
        force_shell = 0;
        if (_emx_env & 0x200) { /* OS/2. */ 
            int type = file_type(real_name);
          type_again:
            if (type == -1) {		/* Not found */
                errno = ENOENT;
                rc = -1;
                goto do_script;
            }
            else if (type == -2) {		/* Not an EXE */
                errno = ENOEXEC;
                rc = -1;
                goto do_script;
            }
            else if (type == -3) {		/* Is a directory? */
                /* Special-case this */
                char tbuf[512];
                int l = strlen(real_name);

                if (l + 5 <= sizeof tbuf) {
                    strcpy(tbuf, real_name);
                    strcpy(tbuf + l, ".exe");
                    type = file_type(tbuf);
                    if (type >= -3)
                        goto type_again;
                }
                
                errno = ENOEXEC;
                rc = -1;
                goto do_script;
            }
            switch (type & 7) {
                /* Ignore WINDOWCOMPAT and FAPI, start them the same type we are. */
            case FAPPTYP_WINDOWAPI: 
            {	/* Apparently, kids are started basing on startup type, not the morphed type */
                if (os2_mytype != 3) {	/* not PM */
                    if (flag == P_NOWAIT)
                        flag = P_PM;
                    else if ((flag & 7) != P_PM && (flag & 7) != P_SESSION && ckWARN(WARN_EXEC))
                        Perl_warner(aTHX_ packWARN(WARN_EXEC), "Starting PM process with flag=%d, mytype=%d",
                             flag, os2_mytype);
                }
            }
            break;
            case FAPPTYP_NOTWINDOWCOMPAT: 
            {
                if (os2_mytype != 0) {	/* not full screen */
                    if (flag == P_NOWAIT)
                        flag = P_SESSION;
                    else if ((flag & 7) != P_SESSION && ckWARN(WARN_EXEC))
                        Perl_warner(aTHX_ packWARN(WARN_EXEC), "Starting Full Screen process with flag=%d, mytype=%d",
                             flag, os2_mytype);
                }
            }
            break;
            case FAPPTYP_NOTSPEC: 
                /* Let the shell handle this... */
                force_shell = 1;
                buf = "";		/* Pacify a warning */
                file = 0;		/* Pacify a warning */
                goto doshell_args;
                break;
            }
        }

        if (addflag) {
            addflag = 0;
            new_stderr = dup(2);		/* Preserve stderr */
            if (new_stderr == -1) {
                if (errno == EBADF)
                    nostderr = 1;
                else {
                    rc = -1;
                    goto finish;
                }
            } else
                fl_stderr = fcntl(2, F_GETFD);
            rc = dup2(1,2);
            if (rc == -1)
                goto finish;
            fcntl(new_stderr, F_SETFD, FD_CLOEXEC);
        }

#if 0
        rc = result(aTHX_ trueflag, spawnvp(flag,real_name,argv));
#else
        if (execf == EXECF_TRUEEXEC)
            rc = execvp(real_name,argv);
        else if (execf == EXECF_EXEC)
            rc = spawnvp(trueflag | P_OVERLAY,real_name,argv);
        else if (execf == EXECF_SPAWN_NOWAIT)
            rc = spawnvp(flag,real_name,argv);
        else if (execf == EXECF_SYNC)
            rc = spawnvp(trueflag,real_name,argv);
        else				/* EXECF_SPAWN, EXECF_SPAWN_BYFLAG */
            rc = result(aTHX_ trueflag, 
                        spawnvp(flag,real_name,argv));
#endif 
        if (rc < 0 && pass == 1) {
              do_script:
          if (real_name == argv[0]) {
            int err = errno;

            if (err == ENOENT || err == ENOEXEC) {
                /* No such file, or is a script. */
                /* Try adding script extensions to the file name, and
                   search on PATH. */
                char *scr = find_script(argv[0], TRUE, NULL, 0);

                if (scr) {
                    char *s = 0, *s1;
                    SV *scrsv = sv_2mortal(newSVpv(scr, 0));
                    SV *bufsv = sv_newmortal();

                    Safefree(scr);
                    scr = SvPV(scrsv, n_a); /* free()ed later */

                    file = PerlIO_open(scr, "r");
                    argv[0] = scr;
                    if (!file)
                        goto panic_file;

                    buf = sv_gets(bufsv, file, 0 /* No append */);
                    if (!buf)
                        buf = "";	/* XXX Needed? */
                    if (!buf[0]) {	/* Empty... */
                        struct stat statbuf;
                        PerlIO_close(file);
                        /* Special case: maybe from -Zexe build, so
                           there is an executable around (contrary to
                           documentation, DosQueryAppType sometimes (?)
                           does not append ".exe", so we could have
                           reached this place). */
                        sv_catpvs(scrsv, ".exe");
                        argv[0] = scr = SvPV(scrsv, n_a);	/* Reload */
                        if (PerlLIO_stat(scr,&statbuf) >= 0
                            && !S_ISDIR(statbuf.st_mode)) {	/* Found */
                                real_name = scr;
                                pass++;
                                goto reread;
                        } else {		/* Restore */
                                SvCUR_set(scrsv, SvCUR(scrsv) - 4);
                                *SvEND(scrsv) = 0;
                        }
                    }
                    if (PerlIO_close(file) != 0) { /* Failure */
                      panic_file:
                        if (ckWARN(WARN_EXEC))
                           Perl_warner(aTHX_ packWARN(WARN_EXEC), "Error reading \"%s\": %s", 
                             scr, Strerror(errno));
                        buf = "";	/* Not #! */
                        goto doshell_args;
                    }
                    if (buf[0] == '#') {
                        if (buf[1] == '!')
                            s = buf + 2;
                    } else if (buf[0] == 'e') {
                        if (strBEGINs(buf, "extproc")
                            && isSPACE(buf[7]))
                            s = buf + 8;
                    } else if (buf[0] == 'E') {
                        if (strBEGINs(buf, "EXTPROC")
                            && isSPACE(buf[7]))
                            s = buf + 8;
                    }
                    if (!s) {
                        buf = "";	/* Not #! */
                        goto doshell_args;
                    }
                    
                    s1 = s;
                    nargs = 0;
                    argsp = args;
                    while (1) {
                        /* Do better than pdksh: allow a few args,
                           strip trailing whitespace.  */
                        while (isSPACE(*s))
                            s++;
                        if (*s == 0) 
                            break;
                        if (nargs == 4) {
                            nargs = -1;
                            break;
                        }
                        args[nargs++] = s;
                        while (*s && !isSPACE(*s))
                            s++;
                        if (*s == 0) 
                            break;
                        *s++ = 0;
                    }
                    if (nargs == -1) {
                        Perl_warner(aTHX_ packWARN(WARN_EXEC), "Too many args on %.*s line of \"%s\"",
                             s1 - buf, buf, scr);
                        nargs = 4;
                        argsp = fargs;
                    }
                    /* Can jump from far, buf/file invalid if force_shell: */
                  doshell_args:
                    {
                        char **a = argv;
                        const char *exec_args[2];

                        if (force_shell 
                            || (!buf[0] && file)) { /* File without magic */
                            /* In fact we tried all what pdksh would
                               try.  There is no point in calling
                               pdksh, we may just emulate its logic. */
                            char *shell = PerlEnv_getenv("EXECSHELL");
                            char *shell_opt = NULL;
                            if (!shell) {
                                char *s;

                                shell_opt = "/c";
                                shell = PerlEnv_getenv("OS2_SHELL");
                                if (inicmd) { /* No spaces at start! */
                                    s = inicmd;
                                    while (*s && !isSPACE(*s)) {
                                        if (*s++ == '/') {
                                            inicmd = NULL; /* Cannot use */
                                            break;
                                        }
                                    }
                                }
                                if (!inicmd) {
                                    s = argv[0];
                                    while (*s) { 
                                        /* Dosish shells will choke on slashes
                                           in paths, fortunately, this is
                                           important for zeroth arg only. */
                                        if (*s == '/') 
                                            *s = '\\';
                                        s++;
                                    }
                                }
                            }
                            /* If EXECSHELL is set, we do not set */
                            
                            if (!shell)
                                shell = ((_emx_env & 0x200)
                                         ? "c:/os2/cmd.exe"
                                         : "c:/command.com");
                            nargs = shell_opt ? 2 : 1;	/* shell file args */
                            exec_args[0] = shell;
                            exec_args[1] = shell_opt;
                            argsp = exec_args;
                            if (nargs == 2 && inicmd) {
                                /* Use the original cmd line */
                                /* XXXX This is good only until we refuse
                                        quoted arguments... */
                                argv[0] = inicmd;
                                argv[1] = NULL;
                            }
                        } else if (!buf[0] && inicmd) { /* No file */
                            /* Start with the original cmdline. */
                            /* XXXX This is good only until we refuse
                                    quoted arguments... */

                            argv[0] = inicmd;
                            argv[1] = NULL;
                            nargs = 2;	/* shell -c */
                        } 

                        while (a[1])		/* Get to the end */
                            a++;
                        a++;			/* Copy finil NULL too */
                        while (a >= argv) {
                            *(a + nargs) = *a;	/* argv was preallocated to be
                                                   long enough. */
                            a--;
                        }
                        while (--nargs >= 0) /* XXXX Discard const... */
                            argv[nargs] = (char*)argsp[nargs];
                        /* Enable pathless exec if #! (as pdksh). */
                        pass = (buf[0] == '#' ? 2 : 3);
                        goto retry;
                    }
                }
                /* Not found: restore errno */
                errno = err;
            }
          } else if (errno == ENOEXEC) { /* Cannot transfer `real_name' via shell. */
                if (rc < 0 && ckWARN(WARN_EXEC))
                    Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't %s script `%s' with ARGV[0] being `%s'", 
                         ((execf != EXECF_EXEC && execf != EXECF_TRUEEXEC) 
                          ? "spawn" : "exec"),
                         real_name, argv[0]);
                goto warned;
          } else if (errno == ENOENT) { /* Cannot transfer `real_name' via shell. */
                if (rc < 0 && ckWARN(WARN_EXEC))
                    Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't %s `%s' with ARGV[0] being `%s' (looking for executables only, not found)", 
                         ((execf != EXECF_EXEC && execf != EXECF_TRUEEXEC) 
                          ? "spawn" : "exec"),
                         real_name, argv[0]);
                goto warned;
          }
        } else if (rc < 0 && pass == 2 && errno == ENOENT) { /* File not found */
            char *no_dir = strrchr(argv[0], '/');

            /* Do as pdksh port does: if not found with /, try without
               path. */
            if (no_dir) {
                argv[0] = no_dir + 1;
                pass++;
                goto retry;
            }
        }
        if (rc < 0 && ckWARN(WARN_EXEC))
            Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't %s \"%s\": %s\n", 
                 ((execf != EXECF_EXEC && execf != EXECF_TRUEEXEC) 
                  ? "spawn" : "exec"),
                 real_name, Strerror(errno));
      warned:
        if (rc < 0 && (execf != EXECF_SPAWN_NOWAIT) 
            && ((trueflag & 0xFF) == P_WAIT)) 
            rc = -1;

  finish:
    if (new_stderr != -1) {	/* How can we use error codes? */
        dup2(new_stderr, 2);
        close(new_stderr);
        fcntl(2, F_SETFD, fl_stderr);
    } else if (nostderr)
       close(2);
    return rc;
}

/* Try converting 1-arg form to (usually shell-less) multi-arg form. */
int
do_spawn3(pTHX_ char *cmd, int execf, int flag)
{
    char **argv, **a;
    char *s;
    char *shell, *copt, *news = NULL;
    int rc, seenspace = 0, mergestderr = 0;

    ENTER;
#ifdef TRYSHELL
    if ((shell = PerlEnv_getenv("EMXSHELL")) != NULL)
        copt = "-c";
    else if ((shell = PerlEnv_getenv("SHELL")) != NULL)
        copt = "-c";
    else if ((shell = PerlEnv_getenv("COMSPEC")) != NULL)
        copt = "/C";
    else
        shell = "cmd.exe";
#else
    /* Consensus on perl5-porters is that it is _very_ important to
       have a shell which will not change between computers with the
       same architecture, to avoid "action on a distance". 
       And to have simple build, this shell should be sh. */
    shell = PL_sh_path;
    copt = "-c";
#endif 

    while (*cmd && isSPACE(*cmd))
        cmd++;

    if (strBEGINs(cmd,"/bin/sh") && isSPACE(cmd[7])) {
        STRLEN l = strlen(PL_sh_path);
        
        Newx(news, strlen(cmd) - 7 + l + 1, char);
        strcpy(news, PL_sh_path);
        strcpy(news + l, cmd + 7);
        cmd = news;
    }

    /* save an extra exec if possible */
    /* see if there are shell metacharacters in it */

    if (*cmd == '.' && isSPACE(cmd[1]))
        goto doshell;

    if (strBEGINs(cmd,"exec") && isSPACE(cmd[4]))
        goto doshell;

    for (s = cmd; *s && isALPHA(*s); s++) ;	/* catch VAR=val gizmo */
    if (*s == '=')
        goto doshell;

    for (s = cmd; *s; s++) {
        if (*s != ' ' && !isALPHA(*s) && memCHRs("$&*(){}[]'\";\\|?<>~`\n",*s)) {
            if (*s == '\n' && s[1] == '\0') {
                *s = '\0';
                break;
            } else if (*s == '\\' && !seenspace) {
                continue;		/* Allow backslashes in names */
            } else if (*s == '>' && s >= cmd + 3
                        && s[-1] == '2' && s[1] == '&' && s[2] == '1'
                        && isSPACE(s[-2]) ) {
                char *t = s + 3;

                while (*t && isSPACE(*t))
                    t++;
                if (!*t) {
                    s[-2] = '\0';
                    mergestderr = 1;
                    break;		/* Allow 2>&1 as the last thing */
                }
            }
            /* We do not convert this to do_spawn_ve since shell
               should be smart enough to start itself gloriously. */
          doshell:
            if (execf == EXECF_TRUEEXEC)
                rc = execl(shell,shell,copt,cmd,(char*)0);
            else if (execf == EXECF_EXEC)
                rc = spawnl(P_OVERLAY,shell,shell,copt,cmd,(char*)0);
            else if (execf == EXECF_SPAWN_NOWAIT)
                rc = spawnl(P_NOWAIT,shell,shell,copt,cmd,(char*)0);
            else if (execf == EXECF_SPAWN_BYFLAG)
                rc = spawnl(flag,shell,shell,copt,cmd,(char*)0);
            else {
                /* In the ak code internal P_NOWAIT is P_WAIT ??? */
                if (execf == EXECF_SYNC)
                   rc = spawnl(P_WAIT,shell,shell,copt,cmd,(char*)0);
                else
                   rc = result(aTHX_ P_WAIT,
                               spawnl(P_NOWAIT,shell,shell,copt,cmd,(char*)0));
                if (rc < 0 && ckWARN(WARN_EXEC))
                    Perl_warner(aTHX_ packWARN(WARN_EXEC), "Can't %s \"%s\": %s", 
                         (execf == EXECF_SPAWN ? "spawn" : "exec"),
                         shell, Strerror(errno));
                if (rc < 0)
                    rc = -1;
            }
            if (news)
                Safefree(news);
            goto leave;
        } else if (*s == ' ' || *s == '\t') {
            seenspace = 1;
        }
    }

    /* cmd="a" may lead to "sh", "-c", "\"$@\"", "a", "a.cmd", NULL */
    Newx(argv, (s - cmd + 11) / 2, char*);
    SAVEFREEPV(argv);
    cmd = savepvn(cmd, s-cmd);
    SAVEFREEPV(cmd);
    a = argv;
    for (s = cmd; *s;) {
        while (*s && isSPACE(*s)) s++;
        if (*s)
            *(a++) = s;
        while (*s && !isSPACE(*s)) s++;
        if (*s)
            *s++ = '\0';
    }
    *a = NULL;
    if (argv[0])
        rc = do_spawn_ve(aTHX_ NULL, argv, flag, execf, cmd, mergestderr);
    else
        rc = -1;
    if (news)
        Safefree(news);
leave:
    LEAVE;
    return rc;
}

#define ASPAWN_WAIT	0
#define ASPAWN_EXEC	1
#define ASPAWN_NOWAIT	2

/* Array spawn/exec.  */
int
os2_aspawn_4(pTHX_ SV *really, SV **args, I32 cnt, int execing)
{
    SV **argp = (SV **)args;
    SV **last = argp + cnt;
    char **argv, **a;
    int rc;
    int flag = P_WAIT, flag_set = 0;
    STRLEN n_a;

    ENTER;
    if (cnt) {
        Newx(argv, cnt + 3, char*); /* 3 extra to expand #! */
        SAVEFREEPV(argv);
        a = argv;

        if (cnt > 1 && SvNIOKp(*argp) && !SvPOKp(*argp)) {
            flag = SvIVx(*argp);
            flag_set = 1;
        } else
            --argp;

        while (++argp < last) {
            if (*argp) {
                char *arg = SvPVx(*argp, n_a);
                arg = savepv(arg);
                SAVEFREEPV(arg);
                *a++ = arg;
            } else
                *a++ = "";
        }
        *a = NULL;

        if ( flag_set && (a == argv + 1)
             && !really && execing == ASPAWN_WAIT ) { 		/* One arg? */
            rc = do_spawn3(aTHX_ a[-1], EXECF_SPAWN_BYFLAG, flag);
        } else {
            const int execf[3] = {EXECF_SPAWN, EXECF_EXEC, EXECF_SPAWN_NOWAIT};
            
            rc = do_spawn_ve(aTHX_ really, argv, flag, execf[execing], NULL, 0);
        }
    } else
        rc = -1;
    LEAVE;
    return rc;
}

/* Array spawn.  */
int
os2_do_aspawn(pTHX_ SV *really, SV **vmark, SV **vsp)
{
    return os2_aspawn_4(aTHX_ really, vmark + 1, vsp - vmark, ASPAWN_WAIT);
}

/* Array exec.  */
bool
Perl_do_aexec(pTHX_ SV* really, SV** vmark, SV** vsp)
{
    return os2_aspawn_4(aTHX_ really, vmark + 1, vsp - vmark, ASPAWN_EXEC);
}

int
os2_do_spawn(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_SPAWN, 0);
}

int
do_spawn_nowait(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_SPAWN_NOWAIT,0);
}

bool
Perl_do_exec(pTHX_ const char *cmd)
{
    do_spawn3(aTHX_ cmd, EXECF_EXEC, 0);
    return FALSE;
}

bool
os2exec(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_TRUEEXEC, 0);
}

PerlIO *
my_syspopen4(pTHX_ char *cmd, char *mode, I32 cnt, SV** args)
{
#ifndef USE_POPEN
    int p[2];
    I32 this, that, newfd;
    I32 pid;
    SV *sv;
    int fh_fl = 0;			/* Pacify the warning */
    
    /* `this' is what we use in the parent, `that' in the child. */
    this = (*mode == 'w');
    that = !this;
    if (TAINTING_get) {
        taint_env();
        taint_proper("Insecure %s%s", "EXEC");
    }
    if (pipe(p) < 0)
        return NULL;
    /* Now we need to spawn the child. */
    if (p[this] == (*mode == 'r')) {	/* if fh 0/1 was initially closed. */
        int new = dup(p[this]);

        if (new == -1)
            goto closepipes;
        close(p[this]);
        p[this] = new;
    }
    newfd = dup(*mode == 'r');		/* Preserve std* */
    if (newfd == -1) {		
        /* This cannot happen due to fh being bad after pipe(), since
           pipe() should have created fh 0 and 1 even if they were
           initially closed.  But we closed p[this] before.  */
        if (errno != EBADF) {
          closepipes:
            close(p[0]);
            close(p[1]);
            return NULL;
        }
    } else
        fh_fl = fcntl(*mode == 'r', F_GETFD);
    if (p[that] != (*mode == 'r')) {	/* if fh 0/1 was initially closed. */
        dup2(p[that], *mode == 'r');
        close(p[that]);
    }
    /* Where is `this' and newfd now? */
    fcntl(p[this], F_SETFD, FD_CLOEXEC);
    if (newfd != -1)
        fcntl(newfd, F_SETFD, FD_CLOEXEC);
    if (cnt) {	/* Args: "Real cmd", before first arg, the last, execing */
        pid = os2_aspawn_4(aTHX_ NULL, args, cnt, ASPAWN_NOWAIT);
    } else
        pid = do_spawn_nowait(aTHX_ cmd);
    if (newfd == -1)
        close(*mode == 'r');		/* It was closed initially */
    else if (newfd != (*mode == 'r')) {	/* Probably this check is not needed */
        dup2(newfd, *mode == 'r');	/* Return std* back. */
        close(newfd);
        fcntl(*mode == 'r', F_SETFD, fh_fl);
    } else
        fcntl(*mode == 'r', F_SETFD, fh_fl);
    if (p[that] == (*mode == 'r'))
        close(p[that]);
    if (pid == -1) {
        close(p[this]);
        return NULL;
    }
    if (p[that] < p[this]) {		/* Make fh as small as possible */
        dup2(p[this], p[that]);
        close(p[this]);
        p[this] = p[that];
    }
    sv = *av_fetch(PL_fdpid,p[this],TRUE);
    (void)SvUPGRADE(sv,SVt_IV);
    SvIVX(sv) = pid;
    PL_forkprocess = pid;
    return PerlIO_fdopen(p[this], mode);

#else  /* USE_POPEN */

    PerlIO *res;
    SV *sv;

    if (cnt)
        Perl_croak(aTHX_ "List form of piped open not implemented");

#  ifdef TRYSHELL
    res = popen(cmd, mode);
#  else

    char *shell = PerlEnv_getenv("EMXSHELL");

    my_setenv("EMXSHELL", PL_sh_path);
    res = popen(cmd, mode);
    my_setenv("EMXSHELL", shell);
#  endif 
    sv = *av_fetch(PL_fdpid, PerlIO_fileno(res), TRUE);
    (void)SvUPGRADE(sv,SVt_IV);
    SvIVX(sv) = -2;                     /* A cooky. */
    return res;

#endif /* USE_POPEN */

}

PerlIO *
my_syspopen(pTHX_ char *cmd, char *mode)
{
    return my_syspopen4(aTHX_ cmd, mode, 0, NULL);
}

/******************************************************************/

#ifndef HAS_FORK
int
fork(void)
{
    Perl_croak_nocontext(PL_no_func, "Unsupported function fork");
    errno = EINVAL;
    return -1;
}
#endif

/*******************************************************************/
/* not implemented in EMX 0.9d */

char *	ctermid(char *s)	{ return 0; }

#ifdef MYTTYNAME /* was not in emx0.9a */
void *	ttyname(x)	{ return 0; }
#endif

/*****************************************************************************/
/* not implemented in C Set++ */

#ifndef __EMX__
int	setuid(x)	{ errno = EINVAL; return -1; }
int	setgid(x)	{ errno = EINVAL; return -1; }
#endif

/*****************************************************************************/
/* stat() hack for char/block device */

#if OS2_STAT_HACK

enum os2_stat_extra {	/* EMX 0.9d fix 4 defines up to 0100000 */
  os2_stat_archived	= 0x1000000,	/* 0100000000 */
  os2_stat_hidden	= 0x2000000,	/* 0200000000 */
  os2_stat_system	= 0x4000000,	/* 0400000000 */
  os2_stat_force	= 0x8000000,	/* Do not ignore flags on chmod */
};

#define OS2_STAT_SPECIAL (os2_stat_system | os2_stat_archived | os2_stat_hidden)

static void
massage_os2_attr(struct stat *st)
{
    if ( ((st->st_mode & S_IFMT) != S_IFREG
          && (st->st_mode & S_IFMT) != S_IFDIR)
         || !(st->st_attr & (FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM)))
        return;

    if ( st->st_attr & FILE_ARCHIVED )
        st->st_mode |= (os2_stat_archived | os2_stat_force);
    if ( st->st_attr & FILE_HIDDEN )
        st->st_mode |= (os2_stat_hidden | os2_stat_force);
    if ( st->st_attr & FILE_SYSTEM )
        st->st_mode |= (os2_stat_system | os2_stat_force);
}

    /* First attempt used DosQueryFSAttach which crashed the system when
       used with 5.001. Now just look for /dev/. */
int
os2_stat(const char *name, struct stat *st)
{
    static int ino = SHRT_MAX;
    STRLEN l = strlen(name);

    if ( ( l < 8 || l > 9) || strnicmp(name, "/dev/", 5) != 0
         || (    stricmp(name + 5, "con") != 0
              && stricmp(name + 5, "tty") != 0
              && stricmp(name + 5, "nul") != 0
              && stricmp(name + 5, "null") != 0) ) {
        int s = stat(name, st);

        if (s)
            return s;
        massage_os2_attr(st);
        return 0;
    }

    memset(st, 0, sizeof *st);
    st->st_mode = S_IFCHR|0666;
    MUTEX_LOCK(&perlos2_state_mutex);
    st->st_ino = (ino-- & 0x7FFF);
    MUTEX_UNLOCK(&perlos2_state_mutex);
    st->st_nlink = 1;
    return 0;
}

int
os2_fstat(int handle, struct stat *st)
{
    int s = fstat(handle, st);

    if (s)
        return s;
    massage_os2_attr(st);
    return 0;
}

#undef chmod
int
os2_chmod (const char *name, int pmode)	/* Modelled after EMX src/lib/io/chmod.c */
{
    int attr, rc;

    if (!(pmode & os2_stat_force))
        return chmod(name, pmode);

    attr = __chmod (name, 0, 0);           /* Get attributes */
    if (attr < 0)
        return -1;
    if (pmode & S_IWRITE)
        attr &= ~FILE_READONLY;
    else
        attr |= FILE_READONLY;
    /* New logic */
    attr &= ~(FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM);

    if ( pmode & os2_stat_archived )
        attr |= FILE_ARCHIVED;
    if ( pmode & os2_stat_hidden )
        attr |= FILE_HIDDEN;
    if ( pmode & os2_stat_system )
        attr |= FILE_SYSTEM;

    rc = __chmod (name, 1, attr);
    if (rc >= 0) rc = 0;
    return rc;
}

#endif

#ifdef USE_PERL_SBRK

/* SBRK() emulation, mostly moved to malloc.c. */

void *
sys_alloc(int size) {
    void *got;
    APIRET rc = DosAllocMem(&got, size, PAG_COMMIT | PAG_WRITE);

    if (rc == ERROR_NOT_ENOUGH_MEMORY) {
        return (void *) -1;
    } else if ( rc ) 
        Perl_croak_nocontext("Got an error from DosAllocMem: %li", (long)rc);
    return got;
}

#endif /* USE_PERL_SBRK */

/* tmp path */

const char *tmppath = TMPPATH1;

void
settmppath()
{
    char *p = PerlEnv_getenv("TMP"), *tpath;
    int len;

    if (!p) p = PerlEnv_getenv("TEMP");
    if (!p) p = PerlEnv_getenv("TMPDIR");
    if (!p) return;
    len = strlen(p);
    tpath = (char *)malloc(len + strlen(TMPPATH1) + 2);
    if (tpath) {
        strcpy(tpath, p);
        tpath[len] = '/';
        strcpy(tpath + len + 1, TMPPATH1);
        tmppath = tpath;
    }
}

#include "XSUB.h"

XS(XS_File__Copy_syscopy)
{
    dXSARGS;
    if (items < 2 || items > 3)
        Perl_croak_nocontext("Usage: File::Copy::syscopy(src,dst,flag=0)");
    {
        STRLEN n_a;
        char *	src = (char *)SvPV(ST(0),n_a);
        char *	dst = (char *)SvPV(ST(1),n_a);
        U32	flag;
        int	RETVAL, rc;
        dXSTARG;

        if (items < 3)
            flag = 0;
        else {
            flag = (unsigned long)SvIV(ST(2));
        }

        RETVAL = !CheckOSError(DosCopy(src, dst, flag));
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

/* APIRET APIENTRY DosReplaceModule (PCSZ pszOld, PCSZ pszNew, PCSZ pszBackup); */

DeclOSFuncByORD(ULONG,replaceModule,ORD_DosReplaceModule,
                (char *old, char *new, char *backup), (old, new, backup))

XS(XS_OS2_replaceModule); /* prototype to pass -Wmissing-prototypes */
XS(XS_OS2_replaceModule)
{
    dXSARGS;
    if (items < 1 || items > 3)
        Perl_croak(aTHX_ "Usage: OS2::replaceModule(target [, source [, backup]])");
    {
        char *	target = (char *)SvPV_nolen(ST(0));
        char *	source = (items < 2) ? NULL : (char *)SvPV_nolen(ST(1));
        char *	backup = (items < 3) ? NULL : (char *)SvPV_nolen(ST(2));

        if (!replaceModule(target, source, backup))
            croak_with_os2error("replaceModule() error");
    }
    XSRETURN_YES;
}

/* APIRET APIENTRY DosPerfSysCall(ULONG ulCommand, ULONG ulParm1,
                                  ULONG ulParm2, ULONG ulParm3); */

DeclOSFuncByORD(ULONG,perfSysCall,ORD_DosPerfSysCall,
                (ULONG ulCommand, ULONG ulParm1, ULONG ulParm2, ULONG ulParm3),
                (ulCommand, ulParm1, ulParm2, ulParm3))

#ifndef CMD_KI_RDCNT
#  define CMD_KI_RDCNT	0x63
#endif
#ifndef CMD_KI_GETQTY
#  define CMD_KI_GETQTY 0x41
#endif
#ifndef QSV_NUMPROCESSORS
#  define QSV_NUMPROCESSORS         26
#endif

typedef unsigned long long myCPUUTIL[4];	/* time/idle/busy/intr */

/*
NO_OUTPUT ULONG
perfSysCall(ULONG ulCommand, ULONG ulParm1, ULONG ulParm2, ULONG ulParm3)
    PREINIT:
        ULONG rc;
    POSTCALL:
        if (!RETVAL)
            croak_with_os2error("perfSysCall() error");
 */

static int
numprocessors(void)
{
    ULONG res;

    if (DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, (PVOID)&res, sizeof(res)))
        return 1;			/* Old system? */
    return res;
}

XS(XS_OS2_perfSysCall); /* prototype to pass -Wmissing-prototypes */
XS(XS_OS2_perfSysCall)
{
    dXSARGS;
    if (items < 0 || items > 4)
        Perl_croak(aTHX_ "Usage: OS2::perfSysCall(ulCommand = CMD_KI_RDCNT, ulParm1= 0, ulParm2= 0, ulParm3= 0)");
    SP -= items;
    {
        dXSTARG;
        ULONG RETVAL, ulCommand, ulParm1, ulParm2, ulParm3, res;
        myCPUUTIL u[64];
        int total = 0, tot2 = 0;

        if (items < 1)
            ulCommand = CMD_KI_RDCNT;
        else {
            ulCommand = (ULONG)SvUV(ST(0));
        }

        if (items < 2) {
            total = (ulCommand == CMD_KI_RDCNT ? numprocessors() : 0);
            ulParm1 = (total ? (ULONG)u : 0);

            if (total > C_ARRAY_LENGTH(u))
                croak("Unexpected number of processors: %d", total);
        } else {
            ulParm1 = (ULONG)SvUV(ST(1));
        }

        if (items < 3) {
            tot2 = (ulCommand == CMD_KI_GETQTY);
            ulParm2 = (tot2 ? (ULONG)&res : 0);
        } else {
            ulParm2 = (ULONG)SvUV(ST(2));
        }

        if (items < 4)
            ulParm3 = 0;
        else {
            ulParm3 = (ULONG)SvUV(ST(3));
        }

        RETVAL = perfSysCall(ulCommand, ulParm1, ulParm2, ulParm3);
        if (!RETVAL)
            croak_with_os2error("perfSysCall() error");
        XSprePUSH;
        if (total) {
            int i,j;

            if (GIMME_V != G_LIST) {
                PUSHn(u[0][0]);		/* Total ticks on the first processor */
                XSRETURN(1);
            }
            EXTEND(SP, 4*total);
            for (i=0; i < total; i++)
                for (j=0; j < 4; j++)
                    PUSHs(sv_2mortal(newSVnv(u[i][j])));
            XSRETURN(4*total);
        }
        if (tot2) {
            PUSHu(res);
            XSRETURN(1);
        }
    }
    XSRETURN_EMPTY;
}

#define PERL_PATCHLEVEL_H_IMPLICIT	/* Do not init local_patches. */
#include "patchlevel.h"
#undef PERL_PATCHLEVEL_H_IMPLICIT

char *
mod2fname(pTHX_ SV *sv)
{
    int pos = 6, len, avlen;
    unsigned int sum = 0;
    char *s;
    STRLEN n_a;

    if (!SvROK(sv)) Perl_croak_nocontext("Not a reference given to mod2fname");
    sv = SvRV(sv);
    if (SvTYPE(sv) != SVt_PVAV) 
      Perl_croak_nocontext("Not array reference given to mod2fname");

    avlen = av_count((AV*)sv);
    if (avlen == 0)
      Perl_croak_nocontext("Empty array reference given to mod2fname");

    s = SvPV(*av_fetch((AV*)sv, avlen, FALSE), n_a);
    strncpy(fname, s, 8);
    len = strlen(s);
    if (len < 6) pos = len;
    while (*s) {
        sum = 33 * sum + *(s++);	/* Checksumming first chars to
                                         * get the capitalization into c.s. */
    }
    while (avlen > 0) {
        s = SvPV(*av_fetch((AV*)sv, avlen, FALSE), n_a);
        while (*s) {
            sum = 33 * sum + *(s++);	/* 7 is primitive mod 13. */
        }
        avlen --;
    }
   /* We always load modules as *specific* DLLs, and with the full name.
      When loading a specific DLL by its full name, one cannot get a
      different DLL, even if a DLL with the same basename is loaded already.
      Thus there is no need to include the version into the mangling scheme. */
#if 0
    sum += PERL_VERSION * 200 + PERL_SUBVERSION * 2;  /* Up to 5.6.1 */
#else
#  ifndef COMPATIBLE_VERSION_SUM  /* Binary compatibility with the 5.00553 binary */
#    define COMPATIBLE_VERSION_SUM (5 * 200 + 53 * 2)
#  endif
    sum += COMPATIBLE_VERSION_SUM;
#endif
    fname[pos] = 'A' + (sum % 26);
    fname[pos + 1] = 'A' + (sum / 26 % 26);
    fname[pos + 2] = '\0';
    return (char *)fname;
}

XS(XS_DynaLoader_mod2fname)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: DynaLoader::mod2fname(sv)");
    {
        SV *	sv = ST(0);
        char *	RETVAL;
        dXSTARG;

        RETVAL = mod2fname(aTHX_ sv);
        sv_setpv(TARG, RETVAL);
        XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}

char *
os2error(int rc)
{
        dTHX;
        ULONG len;
        char *s;
        int number = SvTRUE(get_sv("OS2::nsyserror", GV_ADD));

        if (!(_emx_env & 0x200)) return ""; /* Nop if not OS/2. */
        if (rc == 0)
                return "";
        if (number) {
            sprintf(os2error_buf, "SYS%04d=%#x: ", rc, rc);
            s = os2error_buf + strlen(os2error_buf);
        } else
            s = os2error_buf;
        if (DosGetMessage(NULL, 0, s, sizeof(os2error_buf) - 1 - (s-os2error_buf), 
                          rc, "OSO001.MSG", &len)) {
            char *name = "";

            if (!number) {
                sprintf(os2error_buf, "SYS%04d=%#x: ", rc, rc);
                s = os2error_buf + strlen(os2error_buf);
            }
            switch (rc) {
            case PMERR_INVALID_HWND:
                name = "PMERR_INVALID_HWND";
                break;
            case PMERR_INVALID_HMQ:
                name = "PMERR_INVALID_HMQ";
                break;
            case PMERR_CALL_FROM_WRONG_THREAD:
                name = "PMERR_CALL_FROM_WRONG_THREAD";
                break;
            case PMERR_NO_MSG_QUEUE:
                name = "PMERR_NO_MSG_QUEUE";
                break;
            case PMERR_NOT_IN_A_PM_SESSION:
                name = "PMERR_NOT_IN_A_PM_SESSION";
                break;
            case PMERR_INVALID_ATOM:
                name = "PMERR_INVALID_ATOM";
                break;
            case PMERR_INVALID_HATOMTBL:
                name = "PMERR_INVALID_HATOMTMB";
                break;
            case PMERR_INVALID_INTEGER_ATOM:
                name = "PMERR_INVALID_INTEGER_ATOM";
                break;
            case PMERR_INVALID_ATOM_NAME:
                name = "PMERR_INVALID_ATOM_NAME";
                break;
            case PMERR_ATOM_NAME_NOT_FOUND:
                name = "PMERR_ATOM_NAME_NOT_FOUND";
                break;
            }
            sprintf(s, "%s%s[No description found in OSO001.MSG]", 
                    name, (*name ? "=" : ""));
        } else {
                s[len] = '\0';
                if (len && s[len - 1] == '\n')
                        s[--len] = 0;
                if (len && s[len - 1] == '\r')
                        s[--len] = 0;
                if (len && s[len - 1] == '.')
                        s[--len] = 0;
                if (len >= 10 && number && strnEQ(s, os2error_buf, 7)
                    && s[7] == ':' && s[8] == ' ')
                    /* Some messages start with SYSdddd:, some not */
                    Move(s + 9, s, (len -= 9) + 1, char);
        }
        return os2error_buf;
}

void
ResetWinError(void)
{
  WinError_2_Perl_rc;
}

void
CroakWinError(int die, char *name)
{
  FillWinError;
  if (die && Perl_rc)
    croak_with_os2error(name ? name : "Win* API call");
}

static char *
dllname2buffer(pTHX_ char *buf, STRLEN l)
{
    char *o;
    STRLEN ll;
    SV *dll = NULL;

    dll = module_name(mod_name_full);
    o = SvPV(dll, ll);
    if (ll < l)
       memcpy(buf,o,ll);
    SvREFCNT_dec(dll);
    return (ll >= l ? "???" : buf);
}

static char *
execname2buffer(char *buf, STRLEN l, char *oname)
{
  char *p, *orig = oname, ok = oname != NULL;

  if (_execname(buf, l) != 0) {
    if (!oname || strlen(oname) >= l)
      return oname;
    strcpy(buf, oname);
    ok = 0;
  }
  p = buf;
  while (*p) {
    if (*p == '\\')
        *p = '/';
    if (*p == '/') {
        if (ok && *oname != '/' && *oname != '\\')
            ok = 0;
    } else if (ok && tolower(*oname) != tolower(*p))
        ok = 0;	
    p++;
    oname++;
  }
  if (ok) { /* orig matches the real name.  Use orig: */
     strcpy(buf, orig);		/* _execname() is always uppercased */
     p = buf;
     while (*p) {
       if (*p == '\\')
           *p = '/';
       p++;
     }     
  }
  return buf;
}

char *
os2_execname(pTHX)
{
  char buf[300], *p = execname2buffer(buf, sizeof buf, PL_origargv[0]);

  p = savepv(p);
  SAVEFREEPV(p);
  return p;
}

int
Perl_OS2_handler_install(void *handler, enum Perlos2_handler how)
{
    char *s, b[300];

    switch (how) {
      case Perlos2_handler_mangle:
        perllib_mangle_installed = (char *(*)(char *s, unsigned int l))handler;
        return 1;
      case Perlos2_handler_perl_sh:
        s = (char *)handler;
        s = dir_subst(s, strlen(s), b, sizeof b, 0, "handler_perl_sh");
        perl_sh_installed = savepv(s);
        return 1;
      case Perlos2_handler_perllib_from:
        s = (char *)handler;
        s = dir_subst(s, strlen(s), b, sizeof b, 0, "handler_perllib_from");
        oldl = strlen(s);
        oldp = savepv(s);
        return 1;
      case Perlos2_handler_perllib_to:
        s = (char *)handler;
        s = dir_subst(s, strlen(s), b, sizeof b, 0, "handler_perllib_to");
        newl = strlen(s);
        newp = savepv(s);
        strcpy(mangle_ret, newp);
        s = mangle_ret - 1;
        while (*++s)
            if (*s == '\\')
                *s = '/';
        return 1;
      default:
        return 0;
    }
}

/* Returns a malloc()ed copy */
char *
dir_subst(char *s, unsigned int l, char *b, unsigned int bl, enum dir_subst_e flags, char *msg)
{
    char *from, *to = b, *e = b; /* `to' assignment: shut down the warning */
    STRLEN froml = 0, tol = 0, rest = 0;	/* froml: likewise */

    if (l >= 2 && s[0] == '~') {
        switch (s[1]) {
          case 'i': case 'I':
            from = "installprefix";	break;
          case 'd': case 'D':
            from = "dll";		break;
          case 'e': case 'E':
            from = "exe";		break;
          default:
            from = NULL;
            froml = l + 1;			/* Will not match */
            break;
        }
        if (from)
            froml = strlen(from) + 1;
        if (l >= froml && strnicmp(s + 2, from + 1, froml - 2) == 0) {
            int strip = 1;

            switch (s[1]) {
              case 'i': case 'I':
                strip = 0;
                tol = strlen(INSTALL_PREFIX);
                if (tol >= bl) {
                    if (flags & dir_subst_fatal)
                        Perl_croak_nocontext("INSTALL_PREFIX too long: `%s'", INSTALL_PREFIX);
                    else
                        return NULL;
                }
                memcpy(b, INSTALL_PREFIX, tol + 1);
                to = b;
                e = b + tol;
                break;
              case 'd': case 'D':
                if (flags & dir_subst_fatal) {
                    dTHX;

                    to = dllname2buffer(aTHX_ b, bl);
                } else {				/* No Perl present yet */
                    HMODULE self = find_myself();
                    APIRET rc = DosQueryModuleName(self, bl, b);

                    if (rc)
                        return 0;
                    to = b - 1;
                    while (*++to)
                        if (*to == '\\')
                            *to = '/';
                    to = b;
                }
                break;
              case 'e': case 'E':
                if (flags & dir_subst_fatal) {
                    dTHX;

                    to = execname2buffer(b, bl, PL_origargv[0]);
                } else
                    to = execname2buffer(b, bl, NULL);
                break;
            }
            if (!to)
                return NULL;
            if (strip) {
                e = strrchr(to, '/');
                if (!e && (flags & dir_subst_fatal))
                    Perl_croak_nocontext("%s: Can't parse EXE/DLL name: '%s'", msg, to);
                else if (!e)
                    return NULL;
                *e = 0;
            }
            s += froml; l -= froml;
            if (!l)
                return to;
            if (!tol)
                tol = strlen(to);

            while (l >= 3 && (s[0] == '/' || s[0] == '\\')
                   && s[1] == '.' && s[2] == '.'
                   && (l == 3 || s[3] == '/' || s[3] == '\\' || s[3] == ';')) {
                e = strrchr(b, '/');
                if (!e && (flags & dir_subst_fatal))
                        Perl_croak_nocontext("%s: Error stripping dirs from EXE/DLL/INSTALLDIR name", msg);
                else if (!e)
                        return NULL;
                *e = 0;
                l -= 3; s += 3;
            }
            if (l && s[0] != '/' && s[0] != '\\' && s[0] != ';')
                *e++ = '/';
        }
    }						/* Else: copy as is */
    if (l && (flags & dir_subst_pathlike)) {
        STRLEN i = 0;

        while ( i < l - 2 && s[i] != ';')	/* May have ~char after `;' */
            i++;
        if (i < l - 2) {			/* Found */
            rest = l - i - 1;
            l = i + 1;
        }
    }
    if (e + l >= b + bl) {
        if (flags & dir_subst_fatal)
            Perl_croak_nocontext("%s: name `%s%s' too long", msg, b, s);
        else
            return NULL;
    }
    memcpy(e, s, l);
    if (rest) {
        e = dir_subst(s + l, rest, e + l, bl - (e + l - b), flags, msg);
        return e ? b : e;
    }
    e[l] = 0;
    return b;
}

char *
perllib_mangle_with(char *s, unsigned int l, char *from, unsigned int froml, char *to, unsigned int tol)
{
    if (!to)
        return s;
    if (l == 0)
        l = strlen(s);
    if (l < froml || strnicmp(from, s, froml) != 0)
        return s;
    if (l + tol - froml > STATIC_FILE_LENGTH || tol > STATIC_FILE_LENGTH)
        Perl_croak_nocontext("Malformed PERLLIB_PREFIX");
    if (to && to != mangle_ret)
        memcpy(mangle_ret, to, tol);
    strcpy(mangle_ret + tol, s + froml);
    return mangle_ret;
}

char *
perllib_mangle(char *s, unsigned int l)
{
    char *name;

    if (perllib_mangle_installed && (name = perllib_mangle_installed(s,l)))
        return name;
    if (!newp && !notfound) {
        newp = PerlEnv_getenv(name = "PERLLIB_" STRINGIFY(PERL_REVISION)
                      STRINGIFY(PERL_VERSION) STRINGIFY(PERL_SUBVERSION)
                      "_PREFIX");
        if (!newp)
            newp = PerlEnv_getenv(name = "PERLLIB_" STRINGIFY(PERL_REVISION)
                          STRINGIFY(PERL_VERSION) "_PREFIX");
        if (!newp)
            newp = PerlEnv_getenv(name = "PERLLIB_" STRINGIFY(PERL_REVISION) "_PREFIX");
        if (!newp)
            newp = PerlEnv_getenv(name = "PERLLIB_PREFIX");
        if (newp) {
            char *s, b[300];
            
            oldp = newp;
            while (*newp && !isSPACE(*newp) && *newp != ';')
                newp++;			/* Skip old name. */
            oldl = newp - oldp;
            s = dir_subst(oldp, oldl, b, sizeof b, dir_subst_fatal, name);
            oldp = savepv(s);
            oldl = strlen(s);
            while (*newp && (isSPACE(*newp) || *newp == ';'))
                newp++;			/* Skip whitespace. */
            Perl_OS2_handler_install((void *)newp, Perlos2_handler_perllib_to);
            if (newl == 0 || oldl == 0)
                Perl_croak_nocontext("Malformed %s", name);
        } else
            notfound = 1;
    }
    if (!newp)
        return s;
    if (l == 0)
        l = strlen(s);
    if (l < oldl || strnicmp(oldp, s, oldl) != 0)
        return s;
    if (l + newl - oldl > STATIC_FILE_LENGTH || newl > STATIC_FILE_LENGTH)
        Perl_croak_nocontext("Malformed PERLLIB_PREFIX");
    strcpy(mangle_ret + newl, s + oldl);
    return mangle_ret;
}

unsigned long 
Perl_hab_GET()			/* Needed if perl.h cannot be included */
{
    return perl_hab_GET();
}

static void
Create_HMQ(int serve, char *message)	/* Assumes morphing */
{
    unsigned fpflag = _control87(0,0);

    init_PMWIN_entries();
    /* 64 messages if before OS/2 3.0, ignored otherwise */
    Perl_hmq = (*PMWIN_entries.CreateMsgQueue)(perl_hab_GET(), 64);
    if (!Perl_hmq) {
        dTHX;

        SAVEINT(rmq_cnt);		/* Allow catch()ing. */
        if (rmq_cnt++)
            _exit(188);		/* Panic can try to create a window. */
        CroakWinError(1, message ? message : "Cannot create a message queue");
    }
    if (serve != -1)
        (*PMWIN_entries.CancelShutdown)(Perl_hmq, !serve);
    /* We may have loaded some modules */
    _control87(fpflag, MCW_EM); /* Some modules reset FP flags on (un)load */
}

#define REGISTERMQ_WILL_SERVE		1
#define REGISTERMQ_IMEDIATE_UNMORPH	2

HMQ
Perl_Register_MQ(int serve)
{
  if (Perl_hmq_refcnt <= 0) {
    PPIB pib;
    PTIB tib;

    Perl_hmq_refcnt = 0;		/* Be extra safe */
    DosGetInfoBlocks(&tib, &pib);
    if (!Perl_morph_refcnt) {    
        Perl_os2_initial_mode = pib->pib_ultype;
        /* Try morphing into a PM application. */
        if (pib->pib_ultype != 3)		/* 2 is VIO */
            pib->pib_ultype = 3;		/* 3 is PM */	
    }
    Create_HMQ(-1,			/* We do CancelShutdown ourselves */
               "Cannot create a message queue, or morph to a PM application");
    if ((serve & REGISTERMQ_IMEDIATE_UNMORPH)) {
        if (!Perl_morph_refcnt && Perl_os2_initial_mode != 3)
            pib->pib_ultype = Perl_os2_initial_mode;
    }
  }
    if (serve & REGISTERMQ_WILL_SERVE) {
        if ( Perl_hmq_servers <= 0	/* Safe to inform us on shutdown, */
             && Perl_hmq_refcnt > 0 )	/* this was switched off before... */
            (*PMWIN_entries.CancelShutdown)(Perl_hmq, 0);
        Perl_hmq_servers++;
    } else if (!Perl_hmq_servers)	/* Do not inform us on shutdown */
        (*PMWIN_entries.CancelShutdown)(Perl_hmq, 1);
    Perl_hmq_refcnt++;
    if (!(serve & REGISTERMQ_IMEDIATE_UNMORPH))
        Perl_morph_refcnt++;
    return Perl_hmq;
}

int
Perl_Serve_Messages(int force)
{
    int cnt = 0;
    QMSG msg;

    if (Perl_hmq_servers > 0 && !force)
        return 0;
    if (Perl_hmq_refcnt <= 0)
        Perl_croak_nocontext("No message queue");
    while ((*PMWIN_entries.PeekMsg)(Perl_hab, &msg, NULLHANDLE, 0, 0, PM_REMOVE)) {
        cnt++;
        if (msg.msg == WM_QUIT)
            Perl_croak_nocontext("QUITing...");
        (*PMWIN_entries.DispatchMsg)(Perl_hab, &msg);
    }
    return cnt;
}

int
Perl_Process_Messages(int force, I32 *cntp)
{
    QMSG msg;

    if (Perl_hmq_servers > 0 && !force)
        return 0;
    if (Perl_hmq_refcnt <= 0)
        Perl_croak_nocontext("No message queue");
    while ((*PMWIN_entries.GetMsg)(Perl_hab, &msg, NULLHANDLE, 0, 0)) {
        if (cntp)
            (*cntp)++;
        (*PMWIN_entries.DispatchMsg)(Perl_hab, &msg);
        if (msg.msg == WM_DESTROY)
            return -1;
        if (msg.msg == WM_CREATE)
            return +1;
    }
    Perl_croak_nocontext("QUITing...");
}

void
Perl_Deregister_MQ(int serve)
{
    if (serve & REGISTERMQ_WILL_SERVE)
        Perl_hmq_servers--;

    if (--Perl_hmq_refcnt <= 0) {
        unsigned fpflag = _control87(0,0);

        init_PMWIN_entries();			/* To be extra safe */
        (*PMWIN_entries.DestroyMsgQueue)(Perl_hmq);
        Perl_hmq = 0;
        /* We may have (un)loaded some modules */
        _control87(fpflag, MCW_EM); /* Some modules reset FP flags on (un)load */
    } else if ((serve & REGISTERMQ_WILL_SERVE) && Perl_hmq_servers <= 0)
        (*PMWIN_entries.CancelShutdown)(Perl_hmq, 1); /* Last server exited */
    if (!(serve & REGISTERMQ_IMEDIATE_UNMORPH) && (--Perl_morph_refcnt <= 0)) {
        /* Try morphing back from a PM application. */
        PPIB pib;
        PTIB tib;

        DosGetInfoBlocks(&tib, &pib);
        if (pib->pib_ultype == 3)		/* 3 is PM */
            pib->pib_ultype = Perl_os2_initial_mode;
        else
            Perl_warn_nocontext("Unexpected program mode %d when morphing back from PM",
                                pib->pib_ultype);
    }
}

#define sys_is_absolute(path) ( isALPHA((path)[0]) && (path)[1] == ':' \
                                && ((path)[2] == '/' || (path)[2] == '\\'))
#define sys_is_rooted _fnisabs
#define sys_is_relative _fnisrel
#define current_drive _getdrive

#undef chdir				/* Was _chdir2. */
#define sys_chdir(p) (chdir(p) == 0)
#define change_drive(d) (_chdrive(d), (current_drive() == toupper(d)))

XS(XS_OS2_Error)
{
    dXSARGS;
    if (items != 2)
        Perl_croak_nocontext("Usage: OS2::Error(harderr, exception)");
    {
        int	arg1 = SvIV(ST(0));
        int	arg2 = SvIV(ST(1));
        int	a = ((arg1 ? FERR_ENABLEHARDERR : FERR_DISABLEHARDERR)
                     | (arg2 ? FERR_ENABLEEXCEPTION : FERR_DISABLEEXCEPTION));
        int	RETVAL = ((arg1 ? 1 : 0) | (arg2 ? 2 : 0));
        unsigned long rc;

        if (CheckOSError(DosError(a)))
            Perl_croak_nocontext("DosError(%d) failed: %s", a, os2error(Perl_rc));
        ST(0) = sv_newmortal();
        if (DOS_harderr_state >= 0)
            sv_setiv(ST(0), DOS_harderr_state);
        DOS_harderr_state = RETVAL;
    }
    XSRETURN(1);
}

XS(XS_OS2_Errors2Drive)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: OS2::Errors2Drive(drive)");
    {
        STRLEN n_a;
        SV  *sv = ST(0);
        int	suppress = SvOK(sv);
        char	*s = suppress ? SvPV(sv, n_a) : NULL;
        char	drive = (s ? *s : 0);
        unsigned long rc;

        if (suppress && !isALPHA(drive))
            Perl_croak_nocontext("Non-char argument '%c' to OS2::Errors2Drive()", drive);
        if (CheckOSError(DosSuppressPopUps((suppress
                                            ? SPU_ENABLESUPPRESSION 
                                            : SPU_DISABLESUPPRESSION),
                                           drive)))
            Perl_croak_nocontext("DosSuppressPopUps(%c) failed: %s", drive,
                                 os2error(Perl_rc));
        ST(0) = sv_newmortal();
        if (DOS_suppression_state > 0)
            sv_setpvn(ST(0), &DOS_suppression_state, 1);
        else if (DOS_suppression_state == 0)
            SvPVCLEAR(ST(0));
        DOS_suppression_state = drive;
    }
    XSRETURN(1);
}

int
async_mssleep(ULONG ms, int switch_priority) {
  /* This is similar to DosSleep(), but has 8ms granularity in time-critical
     threads even on Warp3. */
  HEV     hevEvent1     = 0;			/* Event semaphore handle    */
  HTIMER  htimerEvent1  = 0;			/* Timer handle              */
  APIRET  rc            = NO_ERROR;		/* Return code               */
  int ret = 1;
  ULONG priority = 0, nesting;			/* Shut down the warnings */
  PPIB pib;
  PTIB tib;
  char *e = NULL;
  APIRET badrc;

  if (!(_emx_env & 0x200))	/* DOS */
    return !_sleep2(ms);

  os2cp_croak(DosCreateEventSem(NULL,	     /* Unnamed */
                                &hevEvent1,  /* Handle of semaphore returned */
                                DC_SEM_SHARED, /* Shared needed for DosAsyncTimer */
                                FALSE),      /* Semaphore is in RESET state  */
              "DosCreateEventSem");

  if (ms >= switch_priority)
    switch_priority = 0;
  if (switch_priority) {
    if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) 
        switch_priority = 0;
    else {
        /* In Warp3, to switch scheduling to 8ms step, one needs to do 
           DosAsyncTimer() in time-critical thread.  On laters versions,
           more and more cases of wait-for-something are covered.

           It turns out that on Warp3fp42 it is the priority at the time
           of DosAsyncTimer() which matters.  Let's hope that this works
           with later versions too...		XXXX
         */
        priority = (tib->tib_ptib2->tib2_ulpri);
        if ((priority & 0xFF00) == 0x0300) /* already time-critical */
            switch_priority = 0;
        /* Make us time-critical.  Just modifying TIB is not enough... */
        /* tib->tib_ptib2->tib2_ulpri = 0x0300;*/
        /* We do not want to run at high priority if a signal causes us
           to longjmp() out of this section... */
        if (DosEnterMustComplete(&nesting))
            switch_priority = 0;
        else
            DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0);
    }
  }

  if ((badrc = DosAsyncTimer(ms,
                             (HSEM) hevEvent1,	/* Semaphore to post        */
                             &htimerEvent1)))	/* Timer handler (returned) */
     e = "DosAsyncTimer";

  if (switch_priority && tib->tib_ptib2->tib2_ulpri == 0x0300) {
        /* Nobody switched priority while we slept...  Ignore errors... */
        /* tib->tib_ptib2->tib2_ulpri = priority; */	/* Get back... */
        if (!(rc = DosSetPriority(PRTYS_THREAD, (priority>>8) & 0xFF, 0, 0)))
            rc = DosSetPriority(PRTYS_THREAD, 0, priority & 0xFF, 0);
  }
  if (switch_priority)
      rc = DosExitMustComplete(&nesting);	/* Ignore errors */

  /* The actual blocking call is made with "normal" priority.  This way we
     should not bother with DosSleep(0) etc. to compensate for us interrupting
     higher-priority threads.  The goal is to prohibit the system spending too
     much time halt()ing, not to run us "no matter what". */
  if (!e)					/* Wait for AsyncTimer event */
      badrc = DosWaitEventSem(hevEvent1, SEM_INDEFINITE_WAIT);

  if (e) ;				/* Do nothing */
  else if (badrc == ERROR_INTERRUPT)
     ret = 0;
  else if (badrc)
     e = "DosWaitEventSem";
  if ((rc = DosCloseEventSem(hevEvent1)) && !e) { /* Get rid of semaphore */
     e = "DosCloseEventSem";
     badrc = rc;
  }
  if (e)
     os2cp_croak(badrc, e);
  return ret;
}

XS(XS_OS2_ms_sleep)		/* for testing only... */
{
    dXSARGS;
    ULONG ms, lim;

    if (items > 2 || items < 1)
        Perl_croak_nocontext("Usage: OS2::ms_sleep(wait_ms [, high_priority_limit])");
    ms = SvUV(ST(0));
    lim = items > 1 ? SvUV(ST(1)) : ms + 1;
    async_mssleep(ms, lim);
    XSRETURN_YES;
}

ULONG (*pDosTmrQueryFreq) (PULONG);
ULONG (*pDosTmrQueryTime) (unsigned long long *);

XS(XS_OS2_Timer)
{
    dXSARGS;
    static ULONG freq;
    unsigned long long count;
    ULONG rc;

    if (items != 0)
        Perl_croak_nocontext("Usage: OS2::Timer()");
    if (!freq) {
        *(PFN*)&pDosTmrQueryFreq = loadByOrdinal(ORD_DosTmrQueryFreq, 0);
        *(PFN*)&pDosTmrQueryTime = loadByOrdinal(ORD_DosTmrQueryTime, 0);
        MUTEX_LOCK(&perlos2_state_mutex);
        if (!freq)
            if (CheckOSError(pDosTmrQueryFreq(&freq)))
                croak_with_os2error("DosTmrQueryFreq");
        MUTEX_UNLOCK(&perlos2_state_mutex);
    }
    if (CheckOSError(pDosTmrQueryTime(&count)))
        croak_with_os2error("DosTmrQueryTime");
    {    
        dXSTARG;

        XSprePUSH; PUSHn(((NV)count)/freq);
    }
    XSRETURN(1);
}

XS(XS_OS2_msCounter)
{
    dXSARGS;

    if (items != 0)
        Perl_croak_nocontext("Usage: OS2::msCounter()");
    {    
        dXSTARG;

        XSprePUSH; PUSHu(msCounter());
    }
    XSRETURN(1);
}

XS(XS_OS2__InfoTable)
{
    dXSARGS;
    int is_local = 0;

    if (items > 1)
        Perl_croak_nocontext("Usage: OS2::_infoTable([isLocal])");
    if (items == 1)
        is_local = (int)SvIV(ST(0));
    {    
        dXSTARG;

        XSprePUSH; PUSHu(InfoTable(is_local));
    }
    XSRETURN(1);
}

static const char * const dc_fields[] = {
  "FAMILY",
  "IO_CAPS",
  "TECHNOLOGY",
  "DRIVER_VERSION",
  "WIDTH",
  "HEIGHT",
  "WIDTH_IN_CHARS",
  "HEIGHT_IN_CHARS",
  "HORIZONTAL_RESOLUTION",
  "VERTICAL_RESOLUTION",
  "CHAR_WIDTH",
  "CHAR_HEIGHT",
  "SMALL_CHAR_WIDTH",
  "SMALL_CHAR_HEIGHT",
  "COLORS",
  "COLOR_PLANES",
  "COLOR_BITCOUNT",
  "COLOR_TABLE_SUPPORT",
  "MOUSE_BUTTONS",
  "FOREGROUND_MIX_SUPPORT",
  "BACKGROUND_MIX_SUPPORT",
  "VIO_LOADABLE_FONTS",
  "WINDOW_BYTE_ALIGNMENT",
  "BITMAP_FORMATS",
  "RASTER_CAPS",
  "MARKER_HEIGHT",
  "MARKER_WIDTH",
  "DEVICE_FONTS",
  "GRAPHICS_SUBSET",
  "GRAPHICS_VERSION",
  "GRAPHICS_VECTOR_SUBSET",
  "DEVICE_WINDOWING",
  "ADDITIONAL_GRAPHICS",
  "PHYS_COLORS",
  "COLOR_INDEX",
  "GRAPHICS_CHAR_WIDTH",
  "GRAPHICS_CHAR_HEIGHT",
  "HORIZONTAL_FONT_RES",
  "VERTICAL_FONT_RES",
  "DEVICE_FONT_SIM",
  "LINEWIDTH_THICK",
  "DEVICE_POLYSET_POINTS",
};

enum {
    DevCap_dc, DevCap_hwnd
};

HDC (*pWinOpenWindowDC) (HWND hwnd);
HMF (*pDevCloseDC) (HDC hdc);
HDC (*pDevOpenDC) (HAB hab, LONG lType, PCSZ pszToken, LONG lCount,
    PDEVOPENDATA pdopData, HDC hdcComp);
BOOL (*pDevQueryCaps) (HDC hdc, LONG lStart, LONG lCount, PLONG alArray);


XS(XS_OS2_DevCap)
{
    dXSARGS;
    if (items > 2)
        Perl_croak_nocontext("Usage: OS2::DevCap()");
    {
        /* Device Capabilities Data Buffer (10 extra w.r.t. Warp 4.5) */
        LONG   si[CAPS_DEVICE_POLYSET_POINTS - CAPS_FAMILY + 1];
        int i = 0, j = 0, how = DevCap_dc;
        HDC hScreenDC;
        DEVOPENSTRUC doStruc= {0L, (PSZ)"DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};
        ULONG rc1 = NO_ERROR;
        HWND hwnd;
        static volatile int devcap_loaded;

        if (!devcap_loaded) {
            *(PFN*)&pWinOpenWindowDC = loadByOrdinal(ORD_WinOpenWindowDC, 0);
            *(PFN*)&pDevOpenDC = loadByOrdinal(ORD_DevOpenDC, 0);
            *(PFN*)&pDevCloseDC = loadByOrdinal(ORD_DevCloseDC, 0);
            *(PFN*)&pDevQueryCaps = loadByOrdinal(ORD_DevQueryCaps, 0);
            devcap_loaded = 1;
        }

        if (items >= 2)
            how = SvIV(ST(1));
        if (!items) {			/* Get device contents from PM */
            hScreenDC = pDevOpenDC(perl_hab_GET(), OD_MEMORY, (PSZ)"*", 0,
                                  (PDEVOPENDATA)&doStruc, NULLHANDLE);
            if (CheckWinError(hScreenDC))
                croak_with_os2error("DevOpenDC() failed");
        } else if (how == DevCap_dc)
            hScreenDC = (HDC)SvIV(ST(0));
        else {				/* DevCap_hwnd */
            if (!Perl_hmq)
                Perl_croak(aTHX_ "Getting a window's device context without a message queue would lock PM");
            hwnd = (HWND)SvIV(ST(0));
            hScreenDC = pWinOpenWindowDC(hwnd); /* No need to DevCloseDC() */
            if (CheckWinError(hScreenDC))
                croak_with_os2error("WinOpenWindowDC() failed");
        }
        if (CheckWinError(pDevQueryCaps(hScreenDC,
                                        CAPS_FAMILY, /* W3 documented caps */
                                        CAPS_DEVICE_POLYSET_POINTS
                                          - CAPS_FAMILY + 1,
                                        si)))
            rc1 = Perl_rc;
        else {
            EXTEND(SP,2*(CAPS_DEVICE_POLYSET_POINTS - CAPS_FAMILY + 1));
            while (i < CAPS_DEVICE_POLYSET_POINTS - CAPS_FAMILY + 1) {
                ST(j) = sv_newmortal();
                sv_setpv(ST(j++), dc_fields[i]);
                ST(j) = sv_newmortal();
                sv_setiv(ST(j++), si[i]);
                i++;
            }
            i = CAPS_DEVICE_POLYSET_POINTS + 1;
            while (i < CAPS_DEVICE_POLYSET_POINTS + 11) { /* Just in case... */
                LONG l;

                if (CheckWinError(pDevQueryCaps(hScreenDC, i, 1, &l)))
                    break;
                EXTEND(SP, j + 2);
                ST(j) = sv_newmortal();
                sv_setiv(ST(j++), i);
                ST(j) = sv_newmortal();
                sv_setiv(ST(j++), l);
                i++;
            }	    
        }
        if (!items && CheckWinError(pDevCloseDC(hScreenDC)))
            Perl_warn_nocontext("DevCloseDC() failed: %s", os2error(Perl_rc));
        if (rc1)
            Perl_rc = rc1, croak_with_os2error("DevQueryCaps() failed");
        XSRETURN(j);
    }
}

LONG (*pWinQuerySysValue) (HWND hwndDesktop, LONG iSysValue);
BOOL (*pWinSetSysValue) (HWND hwndDesktop, LONG iSysValue, LONG lValue);

const char * const sv_keys[] = {
  "SWAPBUTTON",
  "DBLCLKTIME",
  "CXDBLCLK",
  "CYDBLCLK",
  "CXSIZEBORDER",
  "CYSIZEBORDER",
  "ALARM",
  "7",
  "8",
  "CURSORRATE",
  "FIRSTSCROLLRATE",
  "SCROLLRATE",
  "NUMBEREDLISTS",
  "WARNINGFREQ",
  "NOTEFREQ",
  "ERRORFREQ",
  "WARNINGDURATION",
  "NOTEDURATION",
  "ERRORDURATION",
  "19",
  "CXSCREEN",
  "CYSCREEN",
  "CXVSCROLL",
  "CYHSCROLL",
  "CYVSCROLLARROW",
  "CXHSCROLLARROW",
  "CXBORDER",
  "CYBORDER",
  "CXDLGFRAME",
  "CYDLGFRAME",
  "CYTITLEBAR",
  "CYVSLIDER",
  "CXHSLIDER",
  "CXMINMAXBUTTON",
  "CYMINMAXBUTTON",
  "CYMENU",
  "CXFULLSCREEN",
  "CYFULLSCREEN",
  "CXICON",
  "CYICON",
  "CXPOINTER",
  "CYPOINTER",
  "DEBUG",
  "CPOINTERBUTTONS",
  "POINTERLEVEL",
  "CURSORLEVEL",
  "TRACKRECTLEVEL",
  "CTIMERS",
  "MOUSEPRESENT",
  "CXALIGN",
  "CYALIGN",
  "DESKTOPWORKAREAYTOP",
  "DESKTOPWORKAREAYBOTTOM",
  "DESKTOPWORKAREAXRIGHT",
  "DESKTOPWORKAREAXLEFT",
  "55",
  "NOTRESERVED",
  "EXTRAKEYBEEP",
  "SETLIGHTS",
  "INSERTMODE",
  "60",
  "61",
  "62",
  "63",
  "MENUROLLDOWNDELAY",
  "MENUROLLUPDELAY",
  "ALTMNEMONIC",
  "TASKLISTMOUSEACCESS",
  "CXICONTEXTWIDTH",
  "CICONTEXTLINES",
  "CHORDTIME",
  "CXCHORD",
  "CYCHORD",
  "CXMOTIONSTART",
  "CYMOTIONSTART",
  "BEGINDRAG",
  "ENDDRAG",
  "SINGLESELECT",
  "OPEN",
  "CONTEXTMENU",
  "CONTEXTHELP",
  "TEXTEDIT",
  "BEGINSELECT",
  "ENDSELECT",
  "BEGINDRAGKB",
  "ENDDRAGKB",
  "SELECTKB",
  "OPENKB",
  "CONTEXTMENUKB",
  "CONTEXTHELPKB",
  "TEXTEDITKB",
  "BEGINSELECTKB",
  "ENDSELECTKB",
  "ANIMATION",
  "ANIMATIONSPEED",
  "MONOICONS",
  "KBDALTERED",
  "PRINTSCREEN",		/* 97, the last one on one of the DDK header */
  "LOCKSTARTINPUT",
  "DYNAMICDRAG",
  "100",
  "101",
  "102",
  "103",
  "104",
  "105",
  "106",
  "107",
/*  "CSYSVALUES",*/
                                        /* In recent DDK the limit is 108 */
};

XS(XS_OS2_SysValues)
{
    dXSARGS;
    if (items > 2)
        Perl_croak_nocontext("Usage: OS2::SysValues(which = -1, hwndDesktop = HWND_DESKTOP)");
    {
        int i = 0, j = 0, which = -1;
        HWND hwnd = HWND_DESKTOP;
        static volatile int sv_loaded;
        LONG RETVAL;

        if (!sv_loaded) {
            *(PFN*)&pWinQuerySysValue = loadByOrdinal(ORD_WinQuerySysValue, 0);
            sv_loaded = 1;
        }

        if (items == 2)
            hwnd = (HWND)SvIV(ST(1));
        if (items >= 1)
            which = (int)SvIV(ST(0));
        if (which == -1) {
            EXTEND(SP,2*C_ARRAY_LENGTH(sv_keys));
            while (i < C_ARRAY_LENGTH(sv_keys)) {
                ResetWinError();
                RETVAL = pWinQuerySysValue(hwnd, i);
                if ( !RETVAL
                     && !(sv_keys[i][0] >= '0' && sv_keys[i][0] <= '9'
                          && i <= SV_PRINTSCREEN) ) {
                    FillWinError;
                    if (Perl_rc) {
                        if (i > SV_PRINTSCREEN)
                            break; /* May be not present on older systems */
                        croak_with_os2error("SysValues():");
                    }
                    
                }
                ST(j) = sv_newmortal();
                sv_setpv(ST(j++), sv_keys[i]);
                ST(j) = sv_newmortal();
                sv_setiv(ST(j++), RETVAL);
                i++;
            }
            XSRETURN(2 * i);
        } else {
            dXSTARG;

            ResetWinError();
            RETVAL = pWinQuerySysValue(hwnd, which);
            if (!RETVAL) {
                FillWinError;
                if (Perl_rc)
                    croak_with_os2error("SysValues():");
            }
            XSprePUSH; PUSHi((IV)RETVAL);
        }
    }
}

XS(XS_OS2_SysValues_set)
{
    dXSARGS;
    if (items < 2 || items > 3)
        Perl_croak_nocontext("Usage: OS2::SysValues_set(which, val, hwndDesktop = HWND_DESKTOP)");
    {
        int which = (int)SvIV(ST(0));
        LONG val = (LONG)SvIV(ST(1));
        HWND hwnd = HWND_DESKTOP;
        static volatile int svs_loaded;

        if (!svs_loaded) {
            *(PFN*)&pWinSetSysValue = loadByOrdinal(ORD_WinSetSysValue, 0);
            svs_loaded = 1;
        }

        if (items == 3)
            hwnd = (HWND)SvIV(ST(2));
        if (CheckWinError(pWinSetSysValue(hwnd, which, val)))
            croak_with_os2error("SysValues_set()");
    }
    XSRETURN_YES;
}

#define QSV_MAX_WARP3				QSV_MAX_COMP_LENGTH

static const char * const si_fields[] = {
  "MAX_PATH_LENGTH",
  "MAX_TEXT_SESSIONS",
  "MAX_PM_SESSIONS",
  "MAX_VDM_SESSIONS",
  "BOOT_DRIVE",
  "DYN_PRI_VARIATION",
  "MAX_WAIT",
  "MIN_SLICE",
  "MAX_SLICE",
  "PAGE_SIZE",
  "VERSION_MAJOR",
  "VERSION_MINOR",
  "VERSION_REVISION",
  "MS_COUNT",
  "TIME_LOW",
  "TIME_HIGH",
  "TOTPHYSMEM",
  "TOTRESMEM",
  "TOTAVAILMEM",
  "MAXPRMEM",
  "MAXSHMEM",
  "TIMER_INTERVAL",
  "MAX_COMP_LENGTH",
  "FOREGROUND_FS_SESSION",
  "FOREGROUND_PROCESS",			/* Warp 3 toolkit defines up to this */
  "NUMPROCESSORS",
  "MAXHPRMEM",
  "MAXHSHMEM",
  "MAXPROCESSES",
  "VIRTUALADDRESSLIMIT",
  "INT10ENABLED",			/* From $TOOLKIT-ddk\DDK\video\rel\os2c\include\base\os2\bsedos.h */
};

XS(XS_OS2_SysInfo)
{
    dXSARGS;
    if (items != 0)
        Perl_croak_nocontext("Usage: OS2::SysInfo()");
    {
        /* System Information Data Buffer (10 extra w.r.t. Warp 4.5) */
        ULONG   si[C_ARRAY_LENGTH(si_fields) + 10];
        APIRET  rc	= NO_ERROR;	/* Return code            */
        int i = 0, j = 0, last = QSV_MAX_WARP3;

        if (CheckOSError(DosQuerySysInfo(1L, /* Request documented system */
                                         last, /* info for Warp 3 */
                                         (PVOID)si,
                                         sizeof(si))))
            croak_with_os2error("DosQuerySysInfo() failed");
        while (++last <= C_ARRAY_LENGTH(si)) {
            if (CheckOSError(DosQuerySysInfo(last, last, /* One entry only */
                                             (PVOID)(si+last-1),
                                             sizeof(*si)))) {
                if (Perl_rc != ERROR_INVALID_PARAMETER)
                    croak_with_os2error("DosQuerySysInfo() failed");
                break;
            }
        }
        last--;			/* Count of successfully processed offsets */
        EXTEND(SP,2*last);
        while (i < last) {
            ST(j) = sv_newmortal();
            if (i < C_ARRAY_LENGTH(si_fields))
                sv_setpv(ST(j++),  si_fields[i]);
            else
                sv_setiv(ST(j++),  i + 1);
            ST(j) = sv_newmortal();
            sv_setuv(ST(j++), si[i]);
            i++;
        }
        XSRETURN(2 * last);
    }
}

XS(XS_OS2_SysInfoFor)
{
    dXSARGS;
    int count = (items == 2 ? (int)SvIV(ST(1)) : 1);

    if (items < 1 || items > 2)
        Perl_croak_nocontext("Usage: OS2::SysInfoFor(id[,count])");
    {
        /* System Information Data Buffer (10 extra w.r.t. Warp 4.5) */
        ULONG   si[C_ARRAY_LENGTH(si_fields) + 10];
        APIRET  rc	= NO_ERROR;	/* Return code            */
        int i = 0;
        int start = (int)SvIV(ST(0));

        if (count > C_ARRAY_LENGTH(si) || count <= 0)
            Perl_croak(aTHX_ "unexpected count %d for OS2::SysInfoFor()", count);
        if (CheckOSError(DosQuerySysInfo(start,
                                         start + count - 1,
                                         (PVOID)si,
                                         sizeof(si))))
            croak_with_os2error("DosQuerySysInfo() failed");
        EXTEND(SP,count);
        while (i < count) {
            ST(i) = sv_newmortal();
            sv_setiv(ST(i), si[i]);
            i++;
        }
    }
    XSRETURN(count);
}

XS(XS_OS2_BootDrive)
{
    dXSARGS;
    if (items != 0)
        Perl_croak_nocontext("Usage: OS2::BootDrive()");
    {
        ULONG   si[1] = {0};	/* System Information Data Buffer */
        APIRET  rc    = NO_ERROR;	/* Return code            */
        char c;
        dXSTARG;
        
        if (CheckOSError(DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                                         (PVOID)si, sizeof(si))))
            croak_with_os2error("DosQuerySysInfo() failed");
        c = 'a' - 1 + si[0];
        sv_setpvn(TARG, &c, 1);
        XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}

XS(XS_OS2_Beep)
{
    dXSARGS;
    if (items > 2)			/* Defaults as for WinAlarm(ERROR) */
        Perl_croak_nocontext("Usage: OS2::Beep(freq = 440, ms = 100)");
    {
        ULONG freq	= (items > 0 ? (ULONG)SvUV(ST(0)) : 440);
        ULONG ms	= (items > 1 ? (ULONG)SvUV(ST(1)) : 100);
        ULONG rc;

        if (CheckOSError(DosBeep(freq, ms)))
            croak_with_os2error("SysValues_set()");
    }
    XSRETURN_YES;
}



XS(XS_OS2_MorphPM)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: OS2::MorphPM(serve)");
    {
        bool  serve = SvOK(ST(0));
        unsigned long   pmq = perl_hmq_GET(serve);
        dXSTARG;

        XSprePUSH; PUSHi((IV)pmq);
    }
    XSRETURN(1);
}

XS(XS_OS2_UnMorphPM)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: OS2::UnMorphPM(serve)");
    {
        bool  serve = SvOK(ST(0));

        perl_hmq_UNSET(serve);
    }
    XSRETURN(0);
}

XS(XS_OS2_Serve_Messages)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: OS2::Serve_Messages(force)");
    {
        bool  force = SvOK(ST(0));
        unsigned long   cnt = Perl_Serve_Messages(force);
        dXSTARG;

        XSprePUSH; PUSHi((IV)cnt);
    }
    XSRETURN(1);
}

XS(XS_OS2_Process_Messages)
{
    dXSARGS;
    if (items < 1 || items > 2)
        Perl_croak_nocontext("Usage: OS2::Process_Messages(force [, cnt])");
    {
        bool  force = SvOK(ST(0));
        unsigned long   cnt;
        dXSTARG;

        if (items == 2) {
            I32 cntr;
            SV *sv = ST(1);

            (void)SvIV(sv);		/* Force SvIVX */	    
            if (!SvIOK(sv))
                Perl_croak_nocontext("Can't upgrade count to IV");
            cntr = SvIVX(sv);
            cnt =  Perl_Process_Messages(force, &cntr);
            SvIVX(sv) = cntr;
        } else {
            cnt =  Perl_Process_Messages(force, NULL);
        }
        XSprePUSH; PUSHi((IV)cnt);
    }
    XSRETURN(1);
}

XS(XS_Cwd_current_drive)
{
    dXSARGS;
    if (items != 0)
        Perl_croak_nocontext("Usage: Cwd::current_drive()");
    {
        char	RETVAL;
        dXSTARG;

        RETVAL = current_drive();
        sv_setpvn(TARG, (char *)&RETVAL, 1);
        XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_chdir)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: Cwd::sys_chdir(path)");
    {
        STRLEN n_a;
        char *	path = (char *)SvPV(ST(0),n_a);
        bool	RETVAL;

        RETVAL = sys_chdir(path);
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_change_drive)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: Cwd::change_drive(d)");
    {
        STRLEN n_a;
        char	d = (char)*SvPV(ST(0),n_a);
        bool	RETVAL;

        RETVAL = change_drive(d);
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_absolute)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: Cwd::sys_is_absolute(path)");
    {
        STRLEN n_a;
        char *	path = (char *)SvPV(ST(0),n_a);
        bool	RETVAL;

        RETVAL = sys_is_absolute(path);
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_rooted)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: Cwd::sys_is_rooted(path)");
    {
        STRLEN n_a;
        char *	path = (char *)SvPV(ST(0),n_a);
        bool	RETVAL;

        RETVAL = sys_is_rooted(path);
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_relative)
{
    dXSARGS;
    if (items != 1)
        Perl_croak_nocontext("Usage: Cwd::sys_is_relative(path)");
    {
        STRLEN n_a;
        char *	path = (char *)SvPV(ST(0),n_a);
        bool	RETVAL;

        RETVAL = sys_is_relative(path);
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_cwd)
{
    dXSARGS;
    if (items != 0)
        Perl_croak_nocontext("Usage: Cwd::sys_cwd()");
    {
        char p[MAXPATHLEN];
        char *	RETVAL;

        /* Can't use TARG, since tainting behaves differently */
        RETVAL = _getcwd2(p, MAXPATHLEN);
        ST(0) = sv_newmortal();
        sv_setpv(ST(0), RETVAL);
        SvTAINTED_on(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_abspath)
{
    dXSARGS;
    if (items > 2)
        Perl_croak_nocontext("Usage: Cwd::sys_abspath(path = '.', dir = NULL)");
    {
        STRLEN n_a;
        char *	path = items ? (char *)SvPV(ST(0),n_a) : ".";
        char *	dir, *s, *t, *e;
        char p[MAXPATHLEN];
        char *	RETVAL;
        int l;
        SV *sv;

        if (items < 2)
            dir = NULL;
        else {
            dir = (char *)SvPV(ST(1),n_a);
        }
        if (path[0] == '.' && (path[1] == '/' || path[1] == '\\')) {
            path += 2;
        }
        if (dir == NULL) {
            if (_abspath(p, path, MAXPATHLEN) == 0) {
                RETVAL = p;
            } else {
                RETVAL = NULL;
            }
        } else {
            /* Absolute with drive: */
            if ( sys_is_absolute(path) ) {
                if (_abspath(p, path, MAXPATHLEN) == 0) {
                    RETVAL = p;
                } else {
                    RETVAL = NULL;
                }
            } else if (path[0] == '/' || path[0] == '\\') {
                /* Rooted, but maybe on different drive. */
                if (isALPHA(dir[0]) && dir[1] == ':' ) {
                    char p1[MAXPATHLEN];

                    /* Need to prepend the drive. */
                    p1[0] = dir[0];
                    p1[1] = dir[1];
                    Copy(path, p1 + 2, strlen(path) + 1, char);
                    RETVAL = p;
                    if (_abspath(p, p1, MAXPATHLEN) == 0) {
                        RETVAL = p;
                    } else {
                        RETVAL = NULL;
                    }
                } else if (_abspath(p, path, MAXPATHLEN) == 0) {
                    RETVAL = p;
                } else {
                    RETVAL = NULL;
                }
            } else {
                /* Either path is relative, or starts with a drive letter. */
                /* If the path starts with a drive letter, then dir is
                   relevant only if 
                   a/b)	it is absolute/x:relative on the same drive.  
                   c)	path is on current drive, and dir is rooted
                   In all the cases it is safe to drop the drive part
                   of the path. */
                if ( !sys_is_relative(path) ) {
                    if ( ( ( sys_is_absolute(dir)
                             || (isALPHA(dir[0]) && dir[1] == ':' 
                                 && strnicmp(dir, path,1) == 0)) 
                           && strnicmp(dir, path,1) == 0)
                         || ( !(isALPHA(dir[0]) && dir[1] == ':')
                              && toupper(path[0]) == current_drive())) {
                        path += 2;
                    } else if (_abspath(p, path, MAXPATHLEN) == 0) {
                        RETVAL = p; goto done;
                    } else {
                        RETVAL = NULL; goto done;
                    }
                }
                {
                    /* Need to prepend the absolute path of dir. */
                    char p1[MAXPATHLEN];

                    if (_abspath(p1, dir, MAXPATHLEN) == 0) {
                        int l = strlen(p1);

                        if (p1[ l - 1 ] != '/') {
                            p1[ l ] = '/';
                            l++;
                        }
                        Copy(path, p1 + l, strlen(path) + 1, char);
                        if (_abspath(p, p1, MAXPATHLEN) == 0) {
                            RETVAL = p;
                        } else {
                            RETVAL = NULL;
                        }
                    } else {
                        RETVAL = NULL;
                    }
                }
              done:
            }
        }
        if (!RETVAL)
            XSRETURN_EMPTY;
        /* Backslashes are already converted to slashes. */
        /* Remove trailing slashes */
        l = strlen(RETVAL);
        while (l > 0 && RETVAL[l-1] == '/')
            l--;
        ST(0) = sv_newmortal();
        sv_setpvn( sv = (SV*)ST(0), RETVAL, l);
        /* Remove duplicate slashes, skipping the first three, which
           may be parts of a server-based path */
        s = t = 3 + SvPV_force(sv, n_a);
        e = SvEND(sv);
        /* Do not worry about multibyte chars here, this would contradict the
           eventual UTFization, and currently most other places break too... */
        while (s < e) {
            if (s[0] == t[-1] && s[0] == '/')
                s++;				/* Skip duplicate / */
            else
                *t++ = *s++;
        }
        if (t < e) {
            *t = 0;
            SvCUR_set(sv, t - SvPVX(sv));
        }
        if (!items)
            SvTAINTED_on(ST(0));
    }
    XSRETURN(1);
}
typedef APIRET (*PELP)(PSZ path, ULONG type);

/* Kernels after 2000/09/15 understand this too: */
#ifndef LIBPATHSTRICT
#  define LIBPATHSTRICT 3
#endif

APIRET
ExtLIBPATH(ULONG ord, PSZ path, IV type, int fatal)
{
    ULONG what;
    PFN f = loadByOrdinal(ord, fatal);	/* if fatal: load or die! */

    if (!f)				/* Impossible with fatal */
        return Perl_rc;
    if (type > 0)
        what = END_LIBPATH;
    else if (type == 0)
        what = BEGIN_LIBPATH;
    else
        what = LIBPATHSTRICT;
    return (*(PELP)f)(path, what);
}

#define extLibpath(to,type, fatal) 					\
    (CheckOSError(ExtLIBPATH(ORD_DosQueryExtLibpath, (to), (type), fatal)) ? NULL : (to) )

#define extLibpath_set(p,type, fatal) 					\
    (!CheckOSError(ExtLIBPATH(ORD_DosSetExtLibpath, (p), (type), fatal)))

static void
early_error(char *msg1, char *msg2, char *msg3)
{	/* Buffer overflow detected; there is very little we can do... */
    ULONG rc;

    DosWrite(2, msg1, strlen(msg1), &rc);
    DosWrite(2, msg2, strlen(msg2), &rc);
    DosWrite(2, msg3, strlen(msg3), &rc);
    DosExit(EXIT_PROCESS, 2);
}

XS(XS_Cwd_extLibpath)
{
    dXSARGS;
    if (items < 0 || items > 1)
        Perl_croak_nocontext("Usage: OS2::extLibpath(type = 0)");
    {
        IV	type;
        char	to[1024];
        U32	rc;
        char *	RETVAL;
        dXSTARG;
        STRLEN l;

        if (items < 1)
            type = 0;
        else {
            type = SvIV(ST(0));
        }

        to[0] = 1; to[1] = 0;		/* Sometimes no error reported */
        RETVAL = extLibpath(to, type, 1);	/* Make errors fatal */
        if (RETVAL && RETVAL[0] == 1 && RETVAL[1] == 0)
            Perl_croak_nocontext("panic OS2::extLibpath parameter");
        l = strlen(to);
        if (l >= sizeof(to))
            early_error("Buffer overflow while getting BEGIN/ENDLIBPATH: `",
                        to, "'\r\n");		/* Will not return */
        sv_setpv(TARG, RETVAL);
        XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}

XS(XS_Cwd_extLibpath_set)
{
    dXSARGS;
    if (items < 1 || items > 2)
        Perl_croak_nocontext("Usage: OS2::extLibpath_set(s, type = 0)");
    {
        STRLEN n_a;
        char *	s = (char *)SvPV(ST(0),n_a);
        IV	type;
        U32	rc;
        bool	RETVAL;

        if (items < 2)
            type = 0;
        else {
            type = SvIV(ST(1));
        }

        RETVAL = extLibpath_set(s, type, 1);	/* Make errors fatal */
        ST(0) = boolSV(RETVAL);
        if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

ULONG
fill_extLibpath(int type, char *pre, char *post, int replace, char *msg)
{
    char buf[2048], *to = buf, buf1[300], *s;
    STRLEN l;
    ULONG rc;

    if (!pre && !post)
        return 0;
    if (pre) {
        pre = dir_subst(pre, strlen(pre), buf1, sizeof buf1, dir_subst_pathlike, msg);
        if (!pre)
            return ERROR_INVALID_PARAMETER;
        l = strlen(pre);
        if (l >= sizeof(buf)/2)
            return ERROR_BUFFER_OVERFLOW;
        s = pre - 1;
        while (*++s)
            if (*s == '/')
                *s = '\\';			/* Be extra cautious */
        memcpy(to, pre, l);
        if (!l || to[l-1] != ';')
            to[l++] = ';';
        to += l;
    }

    if (!replace) {
      to[0] = 1; to[1] = 0;		/* Sometimes no error reported */
      rc = ExtLIBPATH(ORD_DosQueryExtLibpath, to, type, 0);	/* Do not croak */
      if (rc)
        return rc;
      if (to[0] == 1 && to[1] == 0)
        return ERROR_INVALID_PARAMETER;
      to += strlen(to);
      if (buf + sizeof(buf) - 1 <= to)	/* Buffer overflow */
        early_error("Buffer overflow while getting BEGIN/ENDLIBPATH: `",
                    buf, "'\r\n");		/* Will not return */
      if (to > buf && to[-1] != ';')
        *to++ = ';';
    }
    if (post) {
        post = dir_subst(post, strlen(post), buf1, sizeof buf1, dir_subst_pathlike, msg);
        if (!post)
            return ERROR_INVALID_PARAMETER;
        l = strlen(post);
        if (l + to - buf >= sizeof(buf) - 1)
            return ERROR_BUFFER_OVERFLOW;
        s = post - 1;
        while (*++s)
            if (*s == '/')
                *s = '\\';			/* Be extra cautious */
        memcpy(to, post, l);
        if (!l || to[l-1] != ';')
            to[l++] = ';';
        to += l;
    }
    *to = 0;
    rc = ExtLIBPATH(ORD_DosSetExtLibpath, buf, type, 0); /* Do not croak */
    return rc;
}

/* Input: Address, BufLen
APIRET APIENTRY
DosQueryModFromEIP (HMODULE * hmod, ULONG * obj, ULONG BufLen, PCHAR Buf,
                    ULONG * Offset, ULONG Address);
*/

DeclOSFuncByORD(APIRET, _DosQueryModFromEIP,ORD_DosQueryModFromEIP,
                        (HMODULE * hmod, ULONG * obj, ULONG BufLen, PCHAR Buf,
                        ULONG * Offset, ULONG Address),
                        (hmod, obj, BufLen, Buf, Offset, Address))

static SV*
module_name_at(void *pp, enum module_name_how how)
{
    dTHX;
    char buf[MAXPATHLEN];
    char *p = buf;
    HMODULE mod;
    ULONG obj, offset, rc, addr = (ULONG)pp;

    if (how & mod_name_HMODULE) {
        if ((how & ~mod_name_HMODULE) == mod_name_shortname)
            Perl_croak(aTHX_ "Can't get short module name from a handle");
        mod = (HMODULE)pp;
        how &= ~mod_name_HMODULE;
    } else if (!_DosQueryModFromEIP(&mod, &obj, sizeof(buf), buf, &offset, addr))
        return &PL_sv_undef;
    if (how == mod_name_handle)
        return newSVuv(mod);
    /* Full name... */
    if ( how != mod_name_shortname
         && CheckOSError(DosQueryModuleName(mod, sizeof(buf), buf)) )
        return &PL_sv_undef;
    while (*p) {
        if (*p == '\\')
            *p = '/';
        p++;
    }
    return newSVpv(buf, 0);
}

static SV*
module_name_of_cv(SV *cv, enum module_name_how how)
{
    if (!cv || !SvROK(cv) || SvTYPE(SvRV(cv)) != SVt_PVCV || !CvXSUB(SvRV(cv))) {
        dTHX;

        if (how & mod_name_C_function)
            return module_name_at((void*)SvIV(cv), how & ~mod_name_C_function);
        else if (how & mod_name_HMODULE)
            return module_name_at((void*)SvIV(cv), how);
        Perl_croak(aTHX_ "Not an XSUB reference");
    }
    return module_name_at(CvXSUB(SvRV(cv)), how);
}

XS(XS_OS2_DLLname)
{
    dXSARGS;
    if (items > 2)
        Perl_croak(aTHX_ "Usage: OS2::DLLname( [ how, [\\&xsub] ] )");
    {
        SV *	RETVAL;
        int	how;

        if (items < 1)
            how = mod_name_full;
        else {
            how = (int)SvIV(ST(0));
        }
        if (items < 2)
            RETVAL = module_name(how);
        else
            RETVAL = module_name_of_cv(ST(1), how);
        ST(0) = RETVAL;
        sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

DeclOSFuncByORD(INT, _Dos32QueryHeaderInfo, ORD_Dos32QueryHeaderInfo,
                        (ULONG r1, ULONG r2, PVOID buf, ULONG szbuf, ULONG fnum),
                        (r1, r2, buf, szbuf, fnum))

XS(XS_OS2__headerInfo)
{
    dXSARGS;
    if (items > 4 || items < 2)
        Perl_croak(aTHX_ "Usage: OS2::_headerInfo(req,size[,handle,[offset]])");
    {
        ULONG	req = (ULONG)SvIV(ST(0));
        STRLEN	size = (STRLEN)SvIV(ST(1)), n_a;
        ULONG	handle = (items >= 3 ? (ULONG)SvIV(ST(2)) : 0);
        ULONG	offset = (items >= 4 ? (ULONG)SvIV(ST(3)) : 0);

        if (size <= 0)
            Perl_croak(aTHX_ "OS2::_headerInfo(): unexpected size: %d", (int)size);
        ST(0) = newSVpvs("");
        SvGROW(ST(0), size + 1);
        sv_2mortal(ST(0));

        if (!_Dos32QueryHeaderInfo(handle, offset, SvPV(ST(0), n_a), size, req)) 
            Perl_croak(aTHX_ "OS2::_headerInfo(%ld,%ld,%ld,%ld) error: %s",
                       req, size, handle, offset, os2error(Perl_rc));
        SvCUR_set(ST(0), size);
        *SvEND(ST(0)) = 0;
    }
    XSRETURN(1);
}

#define DQHI_QUERYLIBPATHSIZE      4
#define DQHI_QUERYLIBPATH          5

XS(XS_OS2_libPath)
{
    dXSARGS;
    if (items != 0)
        Perl_croak(aTHX_ "Usage: OS2::libPath()");
    {
        ULONG	size;
        STRLEN	n_a;

        if (!_Dos32QueryHeaderInfo(0, 0, &size, sizeof(size), 
                                   DQHI_QUERYLIBPATHSIZE)) 
            Perl_croak(aTHX_ "OS2::_headerInfo(%ld,%ld,%ld,%ld) error: %s",
                       DQHI_QUERYLIBPATHSIZE, sizeof(size), 0, 0,
                       os2error(Perl_rc));
        ST(0) = newSVpvs("");
        SvGROW(ST(0), size + 1);
        sv_2mortal(ST(0));

        /* We should be careful: apparently, this entry point does not
           pay attention to the size argument, so may overwrite
           unrelated data! */
        if (!_Dos32QueryHeaderInfo(0, 0, SvPV(ST(0), n_a), size,
                                   DQHI_QUERYLIBPATH)) 
            Perl_croak(aTHX_ "OS2::_headerInfo(%ld,%ld,%ld,%ld) error: %s",
                       DQHI_QUERYLIBPATH, size, 0, 0, os2error(Perl_rc));
        SvCUR_set(ST(0), size);
        *SvEND(ST(0)) = 0;
    }
    XSRETURN(1);
}

#define get_control87()		_control87(0,0)
#define set_control87		_control87

XS(XS_OS2__control87)
{
    dXSARGS;
    if (items != 2)
        Perl_croak(aTHX_ "Usage: OS2::_control87(new,mask)");
    {
        unsigned	new = (unsigned)SvIV(ST(0));
        unsigned	mask = (unsigned)SvIV(ST(1));
        unsigned	RETVAL;
        dXSTARG;

        RETVAL = _control87(new, mask);
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_OS2_mytype)
{
    dXSARGS;
    int which = 0;

    if (items < 0 || items > 1)
        Perl_croak(aTHX_ "Usage: OS2::mytype([which])");
    if (items == 1)
        which = (int)SvIV(ST(0));
    {
        unsigned	RETVAL;
        dXSTARG;

        switch (which) {
        case 0:
            RETVAL = os2_mytype;	/* Reset after fork */
            break;
        case 1:
            RETVAL = os2_mytype_ini;	/* Before any fork */
            break;
        case 2:
            RETVAL = Perl_os2_initial_mode;	/* Before first morphing */
            break;
        case 3:
            RETVAL = my_type();		/* Morphed type */
            break;
        default:
            Perl_croak(aTHX_ "OS2::mytype(which): unknown which=%d", which);
        }
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_OS2_mytype_set)
{
    dXSARGS;
    int type;

    if (items == 1)
        type = (int)SvIV(ST(0));
    else
        Perl_croak(aTHX_ "Usage: OS2::mytype_set(type)");
    my_type_set(type);
    XSRETURN_YES;
}


XS(XS_OS2_get_control87)
{
    dXSARGS;
    if (items != 0)
        Perl_croak(aTHX_ "Usage: OS2::get_control87()");
    {
        unsigned	RETVAL;
        dXSTARG;

        RETVAL = get_control87();
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_OS2_set_control87)
{
    dXSARGS;
    if (items < 0 || items > 2)
        Perl_croak(aTHX_ "Usage: OS2::set_control87(new=MCW_EM, mask=MCW_EM)");
    {
        unsigned	new;
        unsigned	mask;
        unsigned	RETVAL;
        dXSTARG;

        if (items < 1)
            new = MCW_EM;
        else {
            new = (unsigned)SvIV(ST(0));
        }

        if (items < 2)
            mask = MCW_EM;
        else {
            mask = (unsigned)SvIV(ST(1));
        }

        RETVAL = set_control87(new, mask);
        XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_OS2_incrMaxFHandles)		/* DosSetRelMaxFH */
{
    dXSARGS;
    if (items < 0 || items > 1)
        Perl_croak(aTHX_ "Usage: OS2::incrMaxFHandles(delta = 0)");
    {
        LONG	delta;
        ULONG	RETVAL, rc;
        dXSTARG;

        if (items < 1)
            delta = 0;
        else
            delta = (LONG)SvIV(ST(0));

        if (CheckOSError(DosSetRelMaxFH(&delta, &RETVAL)))
            croak_with_os2error("OS2::incrMaxFHandles(): DosSetRelMaxFH() error");
        XSprePUSH; PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}

/* wait>0: force wait, wait<0: force nowait;
   if restore, save/restore flags; otherwise flags are in oflags.

   Returns 1 if connected, 0 if not (due to nowait); croaks on error. */
static ULONG
connectNPipe(ULONG hpipe, int wait, ULONG restore, ULONG oflags)
{
    ULONG ret = ERROR_INTERRUPT, rc, flags;

    if (restore && wait)
        os2cp_croak(DosQueryNPHState(hpipe, &oflags), "DosQueryNPHState()");
    /* DosSetNPHState fails if more bits than NP_NOWAIT|NP_READMODE_MESSAGE */
    oflags &= (NP_NOWAIT | NP_READMODE_MESSAGE);
    flags = (oflags & ~NP_NOWAIT) | (wait > 0 ? NP_WAIT : NP_NOWAIT);
    /* We know (o)flags unless wait == 0 && restore */
    if (wait && (flags != oflags))
        os2cp_croak(DosSetNPHState(hpipe, flags), "DosSetNPHState()");
    while (ret == ERROR_INTERRUPT)
        ret = DosConnectNPipe(hpipe);
    (void)CheckOSError(ret);
    if (restore && wait && (flags != oflags))
        os2cp_croak(DosSetNPHState(hpipe, oflags), "DosSetNPHState() back");
    /* We know flags unless wait == 0 && restore */
    if ( ((wait || restore) ? (flags & NP_NOWAIT) : 1)
         && (ret == ERROR_PIPE_NOT_CONNECTED) )
        return 0;			/* normal return value */
    if (ret == NO_ERROR)
        return 1;
    croak_with_os2error("DosConnectNPipe()");
}

/* With a lot of manual editing:
NO_OUTPUT ULONG
DosCreateNPipe(PCSZ pszName, OUTLIST HPIPE hpipe, ULONG ulOpenMode, int connect = 1, int count = 1, ULONG ulInbufLength = 8192, ULONG ulOutbufLength = ulInbufLength, ULONG ulPipeMode = count | NP_NOWAIT | NP_TYPE_BYTE | NP_READMODE_BYTE, ULONG ulTimeout = 0)
   PREINIT:
        ULONG rc;
   C_ARGS:
        pszName, &hpipe, ulOpenMode, ulPipeMode, ulInbufLength, ulOutbufLength, ulTimeout
   POSTCALL:
        if (CheckOSError(RETVAL))
            croak_with_os2error("OS2::mkpipe() error");
*/
XS(XS_OS2_pipe); /* prototype to pass -Wmissing-prototypes */
XS(XS_OS2_pipe)
{
    dXSARGS;
    if (items < 2 || items > 8)
        Perl_croak(aTHX_ "Usage: OS2::pipe(pszName, ulOpenMode, connect= 1, count= 1, ulInbufLength= 8192, ulOutbufLength= ulInbufLength, ulPipeMode= count | NP_NOWAIT | NP_TYPE_BYTE | NP_READMODE_BYTE, ulTimeout= 0)");
    {
        ULONG	RETVAL;
        PCSZ	pszName = ( SvOK(ST(0)) ? (PCSZ)SvPV_nolen(ST(0)) : NULL );
        HPIPE	hpipe;
        SV	*OpenMode = ST(1);
        ULONG	ulOpenMode;
        int	connect = 0, count, message_r = 0, message = 0, b = 0;
        ULONG	ulInbufLength,	ulOutbufLength,	ulPipeMode, ulTimeout, rc;
        STRLEN	len;
        char	*s, buf[10], *s1, *perltype = NULL;
        PerlIO	*perlio;
        double	timeout;

        if (!pszName || !*pszName)
            Perl_croak(aTHX_ "OS2::pipe(): empty pipe name");
        s = SvPV(OpenMode, len);
        if (memEQs(s, len, "wait")) {	/* DosWaitNPipe() */
            ULONG ms = 0xFFFFFFFF, ret = ERROR_INTERRUPT; /* Indefinite */

            if (items == 3) {
                timeout = (double)SvNV(ST(2));
                ms = timeout * 1000;
                if (timeout < 0)
                    ms = 0xFFFFFFFF; /* Indefinite */
                else if (timeout && !ms)
                    ms = 1;
            } else if (items > 3)
                Perl_croak(aTHX_ "OS2::pipe(): too many arguments for wait-for-connect: %ld", (long)items);

            while (ret == ERROR_INTERRUPT)
                ret = DosWaitNPipe(pszName, ms);	/* XXXX Update ms? */
            os2cp_croak(ret, "DosWaitNPipe()");
            XSRETURN_YES;
        }
        if (memEQs(s, len, "call")) {	/* DosCallNPipe() */
            ULONG ms = 0xFFFFFFFF, got; /* Indefinite */
            STRLEN l;
            char *s;
            char buf[8192];
            STRLEN ll = sizeof(buf);
            char *b = buf;

            if (items < 3 || items > 5)
                Perl_croak(aTHX_ "usage: OS2::pipe(pszName, 'call', write [, timeout= 0xFFFFFFFF, buffsize = 8192])");
            s = SvPV(ST(2), l);
            if (items >= 4) {
                timeout = (double)SvNV(ST(3));
                ms = timeout * 1000;
                if (timeout < 0)
                    ms = 0xFFFFFFFF; /* Indefinite */
                else if (timeout && !ms)
                    ms = 1;
            }
            if (items >= 5) {
                STRLEN lll = SvUV(ST(4));
                SV *sv = NEWSV(914, lll);

                sv_2mortal(sv);
                ll = lll;
                b = SvPVX(sv);
            }	    

            os2cp_croak(DosCallNPipe(pszName, s, l, b, ll, &got, ms),
                        "DosCallNPipe()");
            XSRETURN_PVN(b, got);
        }
        s1 = buf;
        if (len && len <= 3 && !(*s >= '0' && *s <= '9')) {
            int r, w, R, W;

            r = strchr(s, 'r') != 0;
            w = strchr(s, 'w') != 0;
            R = strchr(s, 'R') != 0;
            W = strchr(s, 'W') != 0;
            b = strchr(s, 'b') != 0;
            if (r + w + R + W + b != len || (r && R) || (w && W))
                Perl_croak(aTHX_ "OS2::pipe(): unknown OpenMode argument: `%s'", s);
            if ((r || R) && (w || W))
                ulOpenMode = NP_INHERIT | NP_NOWRITEBEHIND | NP_ACCESS_DUPLEX;
            else if (r || R)
                ulOpenMode = NP_INHERIT | NP_NOWRITEBEHIND | NP_ACCESS_INBOUND;
            else
                ulOpenMode = NP_INHERIT | NP_NOWRITEBEHIND | NP_ACCESS_OUTBOUND;
            if (R)
                message = message_r = 1;
            if (W)
                message = 1;
            else if (w && R)
                Perl_croak(aTHX_ "OS2::pipe(): can't have message read mode for non-message pipes");
        } else
            ulOpenMode = (ULONG)SvUV(OpenMode);	/* ST(1) */

        if ( (ulOpenMode & 0x3) == NP_ACCESS_DUPLEX
             || (ulOpenMode & 0x3) == NP_ACCESS_INBOUND )
            *s1++ = 'r';
        if ( (ulOpenMode & 0x3) == NP_ACCESS_DUPLEX )
            *s1++ = '+';
        if ( (ulOpenMode & 0x3) == NP_ACCESS_OUTBOUND )
            *s1++ = 'w';
        if (b)
            *s1++ = 'b';
        *s1 = 0;
        if ( (ulOpenMode & 0x3) == NP_ACCESS_DUPLEX )
            perltype = "+<&";
        else if ( (ulOpenMode & 0x3) == NP_ACCESS_OUTBOUND )
            perltype = ">&";
        else
            perltype = "<&";

        if (items < 3)
            connect = -1;			/* no wait */
        else if (SvTRUE(ST(2))) {
            s = SvPV(ST(2), len);
            if (memEQs(s, len, "nowait"))
                connect = -1;			/* no wait */
            else if (memEQs(s, len, "wait"))
                connect = 1;			/* wait */
            else
                Perl_croak(aTHX_ "OS2::pipe(): unknown connect argument: `%s'", s);
        }

        if (items < 4)
            count = 1;
        else
            count = (int)SvIV(ST(3));

        if (items < 5)
            ulInbufLength = 8192;
        else
            ulInbufLength = (ULONG)SvUV(ST(4));

        if (items < 6)
            ulOutbufLength = ulInbufLength;
        else
            ulOutbufLength = (ULONG)SvUV(ST(5));

        if (count < -1 || count == 0 || count >= 255)
            Perl_croak(aTHX_ "OS2::pipe(): count should be -1 or between 1 and 254: %ld", (long)count);
        if (count < 0 )
            count = 255;		/* Unlimited */

        ulPipeMode = count;
        if (items < 7)
            ulPipeMode |= (NP_WAIT 
                           | (message ? NP_TYPE_MESSAGE : NP_TYPE_BYTE)
                           | (message_r ? NP_READMODE_MESSAGE : NP_READMODE_BYTE));
        else
            ulPipeMode |= (ULONG)SvUV(ST(6));

        if (items < 8)
            timeout = 0;
        else
            timeout = (double)SvNV(ST(7));
        ulTimeout = timeout * 1000;
        if (timeout < 0)
            ulTimeout = 0xFFFFFFFF; /* Indefinite */
        else if (timeout && !ulTimeout)
            ulTimeout = 1;

        RETVAL = DosCreateNPipe(pszName, &hpipe, ulOpenMode, ulPipeMode, ulInbufLength, ulOutbufLength, ulTimeout);
        if (CheckOSError(RETVAL))
            croak_with_os2error("OS2::pipe(): DosCreateNPipe() error");

        if (connect)
            connectNPipe(hpipe, connect, 1, 0);	/* XXXX wait, retval */
        hpipe = __imphandle(hpipe);

        perlio = PerlIO_fdopen(hpipe, buf);
        ST(0) = sv_newmortal();
        {
            GV *gv = (GV *)sv_newmortal();
            gv_init_pvn(gv, gv_stashpvs("OS2::pipe",1),"__ANONIO__",10,0);
            if ( do_open6(gv, perltype, strlen(perltype), perlio, NULL, 0) )
                sv_setsv(ST(0), sv_bless(newRV((SV*)gv), gv_stashpv("IO::Handle",1)));
            else
                ST(0) = &PL_sv_undef;
        }
    }
    XSRETURN(1);
}

XS(XS_OS2_pipeCntl); /* prototype to pass -Wmissing-prototypes */
XS(XS_OS2_pipeCntl)
{
    dXSARGS;
    if (items < 2 || items > 3)
        Perl_croak(aTHX_ "Usage: OS2::pipeCntl(pipe, op [, wait])");
    {
        ULONG	rc;
        PerlIO *perlio = IoIFP(sv_2io(ST(0)));
        IV	fn = PerlIO_fileno(perlio);
        HPIPE	hpipe = (HPIPE)fn;
        STRLEN	len;
        char	*s = SvPV(ST(1), len);
        int	wait = 0, disconnect = 0, connect = 0, message = -1, query = 0;
        int	peek = 0, state = 0, info = 0;

        if (fn < 0)
            Perl_croak(aTHX_ "OS2::pipeCntl(): not a pipe");	
        if (items == 3)
            wait = (SvTRUE(ST(2)) ? 1 : -1);

        switch (len) {
        case 4:
            if (strEQ(s, "byte"))
                message = 0;
            else if (strEQ(s, "peek"))
                peek = 1;
            else if (strEQ(s, "info"))
                info = 1;
            else
                goto unknown;
            break;
        case 5:
            if (strEQ(s, "reset"))
                disconnect = connect = 1;
            else if (strEQ(s, "state"))
                query = 1;
            else
                goto unknown;
            break;
        case 7:
            if (strEQ(s, "connect"))
                connect = 1;
            else if (strEQ(s, "message"))
                message = 1;
            else
                goto unknown;
            break;
        case 9:
            if (!strEQ(s, "readstate"))
                goto unknown;
            state = 1;
            break;
        case 10:
            if (!strEQ(s, "disconnect"))
                goto unknown;
            disconnect = 1;
            break;
        default:
          unknown:
            Perl_croak(aTHX_ "OS2::pipeCntl(): unknown argument: `%s'", s);
            break;
        }

        if (items == 3 && !connect)
            Perl_croak(aTHX_ "OS2::pipeCntl(): no wait argument for `%s'", s);

        XSprePUSH;		/* Do not need arguments any more */
        if (disconnect) {
            os2cp_croak(DosDisConnectNPipe(hpipe), "OS2::pipeCntl(): DosDisConnectNPipe()");
            PerlIO_clearerr(perlio);
        }
        if (connect) {
            if (!connectNPipe(hpipe, wait , 1, 0))
                XSRETURN_IV(-1);
        }
        if (query) {
            ULONG flags;

            os2cp_croak(DosQueryNPHState(hpipe, &flags), "DosQueryNPHState()");
            XSRETURN_UV(flags);
        }
        if (peek || state || info) {
            ULONG BytesRead, PipeState;
            AVAILDATA BytesAvail;

            os2cp_croak( DosPeekNPipe(hpipe, NULL, 0, &BytesRead, &BytesAvail,
                                      &PipeState), "DosPeekNPipe() for state");
            if (state) {
                EXTEND(SP, 3);
                mPUSHu(PipeState);
                /*   Bytes (available/in-message) */
                mPUSHi(BytesAvail.cbpipe);
                mPUSHi(BytesAvail.cbmessage);
                XSRETURN(3);
            } else if (info) {
                /* L S S C C C/Z*
                   ID of the (remote) computer
                   buffers (out/in)
                   instances (max/actual)
                 */
                struct pipe_info_t {
                    ULONG id;			/* char id[4]; */
                    PIPEINFO pInfo;
                    char buf[512];
                } b;
                int size;

                os2cp_croak( DosQueryNPipeInfo(hpipe, 1, &b.pInfo, sizeof(b) - STRUCT_OFFSET(struct pipe_info_t, pInfo)),
                             "DosQueryNPipeInfo(1)");
                os2cp_croak( DosQueryNPipeInfo(hpipe, 2, &b.id, sizeof(b.id)),
                             "DosQueryNPipeInfo(2)");
                size = b.pInfo.cbName;
                /* Trailing 0 is included in cbName - undocumented; so
                   one should always extract with Z* */
                if (size)		/* name length 254 or less */
                    size--;
                else
                    size = strlen(b.pInfo.szName);
                EXTEND(SP, 6);
                mPUSHp(b.pInfo.szName, size);
                mPUSHu(b.id);
                mPUSHi(b.pInfo.cbOut);
                mPUSHi(b.pInfo.cbIn);
                mPUSHi(b.pInfo.cbMaxInst);
                mPUSHi(b.pInfo.cbCurInst);
                XSRETURN(6);
            } else if (BytesAvail.cbpipe == 0) {
                XSRETURN_NO;
            } else {
                SV *tmp = NEWSV(914, BytesAvail.cbpipe);
                char *s = SvPVX(tmp);

                sv_2mortal(tmp);
                os2cp_croak( DosPeekNPipe(hpipe, s, BytesAvail.cbpipe, &BytesRead,
                                          &BytesAvail, &PipeState), "DosPeekNPipe()");
                SvCUR_set(tmp, BytesRead);
                *SvEND(tmp) = 0;
                SvPOK_on(tmp);
                XSprePUSH; PUSHs(tmp);
                XSRETURN(1);
            }
        }
        if (message > -1) {
            ULONG oflags, flags;

            os2cp_croak(DosQueryNPHState(hpipe, &oflags), "DosQueryNPHState()");
            /* DosSetNPHState fails if more bits than NP_NOWAIT|NP_READMODE_MESSAGE */
            oflags &= (NP_NOWAIT | NP_READMODE_MESSAGE);
            flags = (oflags & NP_NOWAIT)
                | (message ? NP_READMODE_MESSAGE : NP_READMODE_BYTE);
            if (flags != oflags)
                os2cp_croak(DosSetNPHState(hpipe, flags), "DosSetNPHState()");
        }
    }
    XSRETURN_YES;
}

/*
NO_OUTPUT ULONG
DosOpen(PCSZ pszFileName, OUTLIST HFILE hFile, OUTLIST ULONG ulAction, ULONG ulOpenFlags, ULONG ulOpenMode = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW, ULONG ulAttribute = FILE_NORMAL, ULONG ulFileSize = 0, PEAOP2 pEABuf = NULL);
   PREINIT:
        ULONG rc;
   C_ARGS:
        pszFileName, &hFile, &ulAction, ulFileSize, ulAttribute, ulOpenFlags, ulOpenMode, pEABuf
   POSTCALL:
        if (CheckOSError(RETVAL))
            croak_with_os2error("OS2::open() error");
*/
XS(XS_OS2_open); /* prototype to pass -Wmissing-prototypes */
XS(XS_OS2_open)
{
    dXSARGS;
    if (items < 2 || items > 6)
        Perl_croak(aTHX_ "Usage: OS2::open(pszFileName, ulOpenMode, ulOpenFlags= OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW, ulAttribute= FILE_NORMAL, ulFileSize= 0, pEABuf= NULL)");
    {
#line 39 "pipe.xs"
        ULONG rc;
#line 113 "pipe.c"
        ULONG	RETVAL;
        PCSZ	pszFileName = ( SvOK(ST(0)) ? (PCSZ)SvPV_nolen(ST(0)) : NULL );
        HFILE	hFile;
        ULONG	ulAction;
        ULONG	ulOpenMode = (ULONG)SvUV(ST(1));
        ULONG	ulOpenFlags;
        ULONG	ulAttribute;
        ULONG	ulFileSize;
        PEAOP2	pEABuf;

        if (items < 3)
            ulOpenFlags = OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW;
        else {
            ulOpenFlags = (ULONG)SvUV(ST(2));
        }

        if (items < 4)
            ulAttribute = FILE_NORMAL;
        else {
            ulAttribute = (ULONG)SvUV(ST(3));
        }

        if (items < 5)
            ulFileSize = 0;
        else {
            ulFileSize = (ULONG)SvUV(ST(4));
        }

        if (items < 6)
            pEABuf = NULL;
        else {
            pEABuf = (PEAOP2)SvUV(ST(5));
        }

        RETVAL = DosOpen(pszFileName, &hFile, &ulAction, ulFileSize, ulAttribute, ulOpenFlags, ulOpenMode, pEABuf);
        if (CheckOSError(RETVAL))
            croak_with_os2error("OS2::open() error");
        XSprePUSH;	EXTEND(SP,2);
        PUSHs(sv_newmortal());
        sv_setuv(ST(0), (UV)hFile);
        PUSHs(sv_newmortal());
        sv_setuv(ST(1), (UV)ulAction);
    }
    XSRETURN(2);
}

int
Xs_OS2_init(pTHX)
{
    char *file = __FILE__;
    {
        GV *gv;

        if (_emx_env & 0x200) {	/* OS/2 */
            newXS("File::Copy::syscopy", XS_File__Copy_syscopy, file);
            newXS("Cwd::extLibpath", XS_Cwd_extLibpath, file);
            newXS("Cwd::extLibpath_set", XS_Cwd_extLibpath_set, file);
            newXS("OS2::extLibpath", XS_Cwd_extLibpath, file);
            newXS("OS2::extLibpath_set", XS_Cwd_extLibpath_set, file);
        }
        newXS("OS2::Error", XS_OS2_Error, file);
        newXS("OS2::Errors2Drive", XS_OS2_Errors2Drive, file);
        newXS("OS2::SysInfo", XS_OS2_SysInfo, file);
        newXSproto("OS2::DevCap", XS_OS2_DevCap, file, ";$$");
        newXSproto("OS2::SysInfoFor", XS_OS2_SysInfoFor, file, "$;$");
        newXS("OS2::BootDrive", XS_OS2_BootDrive, file);
        newXS("OS2::MorphPM", XS_OS2_MorphPM, file);
        newXS("OS2::UnMorphPM", XS_OS2_UnMorphPM, file);
        newXS("OS2::Serve_Messages", XS_OS2_Serve_Messages, file);
        newXS("OS2::Process_Messages", XS_OS2_Process_Messages, file);
        newXS("DynaLoader::mod2fname", XS_DynaLoader_mod2fname, file);
        newXS("Cwd::current_drive", XS_Cwd_current_drive, file);
        newXS("Cwd::sys_chdir", XS_Cwd_sys_chdir, file);
        newXS("Cwd::change_drive", XS_Cwd_change_drive, file);
        newXS("Cwd::sys_is_absolute", XS_Cwd_sys_is_absolute, file);
        newXS("Cwd::sys_is_rooted", XS_Cwd_sys_is_rooted, file);
        newXS("Cwd::sys_is_relative", XS_Cwd_sys_is_relative, file);
        newXS("Cwd::sys_cwd", XS_Cwd_sys_cwd, file);
        newXS("Cwd::sys_abspath", XS_Cwd_sys_abspath, file);
        newXS("OS2::replaceModule", XS_OS2_replaceModule, file);
        newXS("OS2::perfSysCall", XS_OS2_perfSysCall, file);
        newXSproto("OS2::_control87", XS_OS2__control87, file, "$$");
        newXSproto("OS2::get_control87", XS_OS2_get_control87, file, "");
        newXSproto("OS2::set_control87", XS_OS2_set_control87, file, ";$$");
        newXSproto("OS2::DLLname", XS_OS2_DLLname, file, ";$$");
        newXSproto("OS2::mytype", XS_OS2_mytype, file, ";$");
        newXSproto("OS2::mytype_set", XS_OS2_mytype_set, file, "$");
        newXSproto("OS2::_headerInfo", XS_OS2__headerInfo, file, "$$;$$");
        newXSproto("OS2::libPath", XS_OS2_libPath, file, "");
        newXSproto("OS2::Timer", XS_OS2_Timer, file, "");
        newXSproto("OS2::msCounter", XS_OS2_msCounter, file, "");
        newXSproto("OS2::ms_sleep", XS_OS2_ms_sleep, file, "$;$");
        newXSproto("OS2::_InfoTable", XS_OS2__InfoTable, file, ";$");
        newXSproto("OS2::incrMaxFHandles", XS_OS2_incrMaxFHandles, file, ";$");
        newXSproto("OS2::SysValues", XS_OS2_SysValues, file, ";$$");
        newXSproto("OS2::SysValues_set", XS_OS2_SysValues_set, file, "$$;$");
        newXSproto("OS2::Beep", XS_OS2_Beep, file, ";$$");
        newXSproto("OS2::pipe", XS_OS2_pipe, file, "$$;$$$$$$");
        newXSproto("OS2::pipeCntl", XS_OS2_pipeCntl, file, "$$;$");
        newXSproto("OS2::open", XS_OS2_open, file, "$$;$$$$");
        gv = gv_fetchpv("OS2::is_aout", TRUE, SVt_PV);
        GvMULTI_on(gv);
#ifdef PERL_IS_AOUT
        sv_setiv(GvSV(gv), 1);
#endif
        gv = gv_fetchpv("OS2::is_static", TRUE, SVt_PV);
        GvMULTI_on(gv);
#ifdef PERL_IS_AOUT
        sv_setiv(GvSV(gv), 1);
#endif
        gv = gv_fetchpv("OS2::can_fork", TRUE, SVt_PV);
        GvMULTI_on(gv);
        sv_setiv(GvSV(gv), exe_is_aout());
        gv = gv_fetchpv("OS2::emx_rev", TRUE, SVt_PV);
        GvMULTI_on(gv);
        sv_setiv(GvSV(gv), _emx_rev);
        sv_setpv(GvSV(gv), _emx_vprt);
        SvIOK_on(GvSV(gv));
        gv = gv_fetchpv("OS2::emx_env", TRUE, SVt_PV);
        GvMULTI_on(gv);
        sv_setiv(GvSV(gv), _emx_env);
        gv = gv_fetchpv("OS2::os_ver", TRUE, SVt_PV);
        GvMULTI_on(gv);
        sv_setnv(GvSV(gv), _osmajor + 0.001 * _osminor);
        gv = gv_fetchpv("OS2::nsyserror", TRUE, SVt_PV);
        GvMULTI_on(gv);
        sv_setiv(GvSV(gv), 1);		/* DEFAULT: Show number on syserror */
    }
    return 0;
}

extern void _emx_init(void*);

static void jmp_out_of_atexit(void);

#define FORCE_EMX_INIT_CONTRACT_ARGV	1
#define FORCE_EMX_INIT_INSTALL_ATEXIT	2

static void
my_emx_init(void *layout) {
    static volatile void *old_esp = 0;	/* Cannot be on stack! */

    /* Can't just call emx_init(), since it moves the stack pointer */
    /* It also busts a lot of registers, so be extra careful */
    __asm__(	"pushf\n"
                "pusha\n"
                "movl %%esp, %1\n"
                "push %0\n"
                "call __emx_init\n"
                "movl %1, %%esp\n"
                "popa\n"
                "popf\n" : : "r" (layout), "m" (old_esp)	);
}

struct layout_table_t {
    ULONG text_base;
    ULONG text_end;
    ULONG data_base;
    ULONG data_end;
    ULONG bss_base;
    ULONG bss_end;
    ULONG heap_base;
    ULONG heap_end;
    ULONG heap_brk;
    ULONG heap_off;
    ULONG os2_dll;
    ULONG stack_base;
    ULONG stack_end;
    ULONG flags;
    ULONG reserved[2];
    char options[64];
};

static ULONG
my_os_version() {
    static ULONG osv_res;		/* Cannot be on stack! */

    /* Can't just call __os_version(), since it does not follow C
       calling convention: it busts a lot of registers, so be extra careful */
    __asm__(	"pushf\n"
                "pusha\n"
                "call ___os_version\n"
                "movl %%eax, %0\n"
                "popa\n"
                "popf\n" : "=m" (osv_res)	);

    return osv_res;
}

static void
force_init_emx_runtime(EXCEPTIONREGISTRATIONRECORD *preg, ULONG flags)
{
    /* Calling emx_init() will bust the top of stack: it installs an
       exception handler and puts argv data there. */
    char *oldarg, *oldenv;
    void *oldstackend, *oldstack;
    PPIB pib;
    PTIB tib;
    ULONG rc, error = 0, out;
    char buf[512];
    static struct layout_table_t layout_table;
    struct {
        char buf[48*1024]; /* _emx_init() requires 32K, cmd.exe has 64K only */
        double alignment1;
        EXCEPTIONREGISTRATIONRECORD xreg;
    } *newstack;
    char *s;

    layout_table.os2_dll = (ULONG)&os2_dll_fake;
    layout_table.flags   = 0x02000002;	/* flags: application, OMF */

    DosGetInfoBlocks(&tib, &pib);
    oldarg = pib->pib_pchcmd;
    oldenv = pib->pib_pchenv;
    oldstack = tib->tib_pstack;
    oldstackend = tib->tib_pstacklimit;

    if ( (char*)&s < (char*)oldstack + 4*1024 
         || (char *)oldstackend < (char*)oldstack + 52*1024 )
        early_error("It is a lunacy to try to run EMX Perl ",
                    "with less than 64K of stack;\r\n",
                    "  at least with non-EMX starter...\r\n");

    /* Minimize the damage to the stack via reducing the size of argv. */
    if (flags & FORCE_EMX_INIT_CONTRACT_ARGV) {
        pib->pib_pchcmd = "\0\0";	/* Need 3 concatenated strings */
        pib->pib_pchcmd = "\0";		/* Ended by an extra \0. */
    }

    newstack = alloca(sizeof(*newstack));
    /* Emulate the stack probe */
    s = ((char*)newstack) + sizeof(*newstack);
    while (s > (char*)newstack) {
        s[-1] = 0;
        s -= 4096;
    }

    /* Reassigning stack is documented to work */
    tib->tib_pstack = (void*)newstack;
    tib->tib_pstacklimit = (void*)((char*)newstack + sizeof(*newstack));

    /* Can't just call emx_init(), since it moves the stack pointer */
    my_emx_init((void*)&layout_table);

    /* Remove the exception handler, cannot use it - too low on the stack.
       Check whether it is inside the new stack.  */
    buf[0] = 0;
    if (tib->tib_pexchain >= tib->tib_pstacklimit
        || tib->tib_pexchain < tib->tib_pstack) {
        error = 1;
        sprintf(buf,
                "panic: ExceptionHandler misplaced: not %#lx <= %#lx < %#lx\n",
                (unsigned long)tib->tib_pstack,
                (unsigned long)tib->tib_pexchain,
                (unsigned long)tib->tib_pstacklimit);	
        goto finish;
    }
    if (tib->tib_pexchain != &(newstack->xreg)) {
        sprintf(buf, "ExceptionHandler misplaced: %#lx != %#lx\n",
                (unsigned long)tib->tib_pexchain,
                (unsigned long)&(newstack->xreg));	
    }
    rc = DosUnsetExceptionHandler((EXCEPTIONREGISTRATIONRECORD *)tib->tib_pexchain);
    if (rc)
        sprintf(buf + strlen(buf), 
                "warning: DosUnsetExceptionHandler rc=%#lx=%lu\n", rc, rc);

    if (preg) {
        /* ExceptionRecords should be on stack, in a correct order.  Sigh... */
        preg->prev_structure = 0;
        preg->ExceptionHandler = _emx_exception;
        rc = DosSetExceptionHandler(preg);
        if (rc) {
            sprintf(buf + strlen(buf),
                    "warning: DosSetExceptionHandler rc=%#lx=%lu\n", rc, rc);
            DosWrite(2, buf, strlen(buf), &out);
            emx_exception_init = 1;	/* Do it around spawn*() calls */
        }
    } else
        emx_exception_init = 1;		/* Do it around spawn*() calls */

  finish:
    /* Restore the damage */
    pib->pib_pchcmd = oldarg;
    pib->pib_pchcmd = oldenv;
    tib->tib_pstacklimit = oldstackend;
    tib->tib_pstack = oldstack;
    emx_runtime_init = 1;
    if (buf[0])
        DosWrite(2, buf, strlen(buf), &out);
    if (error)
        exit(56);
}

static void
jmp_out_of_atexit(void)
{
    if (longjmp_at_exit)
        longjmp(at_exit_buf, 1);
}

extern void _CRT_term(void);

void
Perl_OS2_term(void **p, int exitstatus, int flags)
{
    if (!emx_runtime_secondary)
        return;

    /* The principal executable is not running the same CRTL, so there
       is nobody to shutdown *this* CRTL except us... */
    if (flags & FORCE_EMX_DEINIT_EXIT) {
        if (p && !emx_exception_init)
            DosUnsetExceptionHandler((EXCEPTIONREGISTRATIONRECORD *)p);
        /* Do not run the executable's CRTL's termination routines */
        exit(exitstatus);		/* Run at-exit, flush buffers, etc */
    }
    /* Run at-exit list, and jump out at the end */
    if ((flags & FORCE_EMX_DEINIT_RUN_ATEXIT) && !setjmp(at_exit_buf)) {
        longjmp_at_exit = 1;
        exit(exitstatus);		/* The first pass through "if" */
    }

    /* Get here if we managed to jump out of exit(), or did not run atexit. */
    longjmp_at_exit = 0;		/* Maybe exit() is called again? */
#if 0 /* _atexit_n is not exported */
    if (flags & FORCE_EMX_DEINIT_RUN_ATEXIT)
        _atexit_n = 0;			/* Remove the atexit() handlers */
#endif
    /* Will segfault on program termination if we leave this dangling... */
    if (p && !emx_exception_init)
        DosUnsetExceptionHandler((EXCEPTIONREGISTRATIONRECORD *)p);
    /* Typically there is no need to do this, done from _DLL_InitTerm() */
    if (flags & FORCE_EMX_DEINIT_CRT_TERM)
        _CRT_term();			/* Flush buffers, etc. */
    /* Now it is a good time to call exit() in the caller's CRTL... */
}

#include <emx/startup.h>

extern ULONG __os_version();		/* See system.doc */

void
check_emx_runtime(char **env, EXCEPTIONREGISTRATIONRECORD *preg)
{
    ULONG v_crt, v_emx, count = 0, rc = NO_ERROR, rc1, maybe_inited = 0;
    static HMTX hmtx_emx_init = NULLHANDLE;
    static int emx_init_done = 0;

    /*  If _environ is not set, this code sits in a DLL which
        uses a CRT DLL which not compatible with the executable's
        CRT library.  Some parts of the DLL are not initialized.
     */
    if (_environ != NULL)
        return;				/* Properly initialized */

    /* It is not DOS, so we may use OS/2 API now */
    /* Some data we manipulate is static; protect ourselves from
       calling the same API from a different thread. */
    DosEnterMustComplete(&count);

    rc1 = DosEnterCritSec();
    if (!hmtx_emx_init)
        rc = DosCreateMutexSem(NULL, &hmtx_emx_init, 0, TRUE); /*Create owned*/
    else
        maybe_inited = 1;

    if (rc != NO_ERROR)
        hmtx_emx_init = NULLHANDLE;

    if (rc1 == NO_ERROR)
        DosExitCritSec();
    DosExitMustComplete(&count);

    while (maybe_inited) { /* Other thread did or is doing the same now */
        if (emx_init_done)
            return;
        rc = DosRequestMutexSem(hmtx_emx_init,
                                (ULONG) SEM_INDEFINITE_WAIT);  /* Timeout (none) */
        if (rc == ERROR_INTERRUPT)
            continue;
        if (rc != NO_ERROR) {
            char buf[80];
            ULONG out;

            sprintf(buf,
                    "panic: EMX backdoor init: DosRequestMutexSem error: %lu=%#lx\n", rc, rc);	    
            DosWrite(2, buf, strlen(buf), &out);
            return;
        }
        DosReleaseMutexSem(hmtx_emx_init);
        return;
    }

    /*  If the executable does not use EMX.DLL, EMX.DLL is not completely
        initialized either.  Uninitialized EMX.DLL returns 0 in the low
        nibble of __os_version().  */
    v_emx = my_os_version();

    /*	_osmajor and _osminor are normally set in _DLL_InitTerm of CRT DLL
        (=>_CRT_init=>_entry2) via a call to __os_version(), then
        reset when the EXE initialization code calls _text=>_init=>_entry2.
        The first time they are wrongly set to 0; the second time the
        EXE initialization code had already called emx_init=>initialize1
        which correctly set version_major, version_minor used by
        __os_version().  */
    v_crt = (_osmajor | _osminor);

    if ((_emx_env & 0x200) && !(v_emx & 0xFFFF)) {	/* OS/2, EMX uninit. */ 
        force_init_emx_runtime( preg,
                                FORCE_EMX_INIT_CONTRACT_ARGV 
                                | FORCE_EMX_INIT_INSTALL_ATEXIT );
        emx_wasnt_initialized = 1;
        /* Update CRTL data basing on now-valid EMX runtime data */
        if (!v_crt) {		/* The only wrong data are the versions. */
            v_emx = my_os_version();			/* *Now* it works */
            *(unsigned char *)&_osmajor = v_emx & 0xFF;	/* Cast out const */
            *(unsigned char *)&_osminor = (v_emx>>8) & 0xFF;
        }
    }
    emx_runtime_secondary = 1;
    /* if (flags & FORCE_EMX_INIT_INSTALL_ATEXIT) */
    atexit(jmp_out_of_atexit);		/* Allow run of atexit() w/o exit()  */

    if (env == NULL) {			/* Fetch from the process info block */
        int c = 0;
        PPIB pib;
        PTIB tib;
        char *e, **ep;

        DosGetInfoBlocks(&tib, &pib);
        e = pib->pib_pchenv;
        while (*e) {			/* Get count */
            c++;
            e = e + strlen(e) + 1;
        }
        Newx(env, c + 1, char*);
        ep = env;
        e = pib->pib_pchenv;
        while (c--) {
            *ep++ = e;
            e = e + strlen(e) + 1;
        }
        *ep = NULL;
    }
    _environ = _org_environ = env;
    emx_init_done = 1;
    if (hmtx_emx_init)
        DosReleaseMutexSem(hmtx_emx_init);
}

#define ENTRY_POINT 0x10000

static int
exe_is_aout(void)
{
    struct layout_table_t *layout;
    if (emx_wasnt_initialized)
        return 0;
    /* Now we know that the principal executable is an EMX application 
       - unless somebody did already play with delayed initialization... */
    /* With EMX applications to determine whether it is AOUT one needs
       to examine the start of the executable to find "layout" */
    if ( *(unsigned char*)ENTRY_POINT != 0x68		/* PUSH n */
         || *(unsigned char*)(ENTRY_POINT+5) != 0xe8	/* CALL */
         || *(unsigned char*)(ENTRY_POINT+10) != 0xeb	/* JMP */
         || *(unsigned char*)(ENTRY_POINT+12) != 0xe8)	/* CALL */
        return 0;					/* ! EMX executable */
    /* Fix alignment */
    Copy((char*)(ENTRY_POINT+1), &layout, 1, struct layout_table_t*);
    return !(layout->flags & 2);			
}

void
Perl_OS2_init(char **env)
{
    Perl_OS2_init3(env, 0, 0);
}

void
Perl_OS2_init3(char **env, void **preg, int flags)
{
    char *shell, *s;
    ULONG rc;

    _uflags (_UF_SBRK_MODEL, _UF_SBRK_ARBITRARY);
    MALLOC_INIT;

    check_emx_runtime(env, (EXCEPTIONREGISTRATIONRECORD *)preg);

    settmppath();
    OS2_Perl_data.xs_init = &Xs_OS2_init;
    if (perl_sh_installed) {
        int l = strlen(perl_sh_installed);

        Newx(PL_sh_path, l + 1, char);
        memcpy(PL_sh_path, perl_sh_installed, l + 1);
    } else if ( (shell = PerlEnv_getenv("PERL_SH_DRIVE")) ) {
        Newx(PL_sh_path, strlen(SH_PATH) + 1, char);
        strcpy(PL_sh_path, SH_PATH);
        PL_sh_path[0] = shell[0];
    } else if ( (shell = PerlEnv_getenv("PERL_SH_DIR")) ) {
        int l = strlen(shell), i;

        while (l && (shell[l-1] == '/' || shell[l-1] == '\\'))
            l--;
        Newx(PL_sh_path, l + 8, char);
        strncpy(PL_sh_path, shell, l);
        strcpy(PL_sh_path + l, "/sh.exe");
        for (i = 0; i < l; i++) {
            if (PL_sh_path[i] == '\\') PL_sh_path[i] = '/';
        }
    }
    MUTEX_INIT(&start_thread_mutex);
    MUTEX_INIT(&perlos2_state_mutex);
    os2_mytype = my_type();		/* Do it before morphing.  Needed? */
    os2_mytype_ini = os2_mytype;
    Perl_os2_initial_mode = -1;		/* Uninit */

    s = PerlEnv_getenv("PERL_BEGINLIBPATH");
    if (s)
      rc = fill_extLibpath(0, s, NULL, 1, "PERL_BEGINLIBPATH");
    else
      rc = fill_extLibpath(0, PerlEnv_getenv("PERL_PRE_BEGINLIBPATH"), PerlEnv_getenv("PERL_POST_BEGINLIBPATH"), 0, "PERL_(PRE/POST)_BEGINLIBPATH");
    if (!rc) {
        s = PerlEnv_getenv("PERL_ENDLIBPATH");
        if (s)
            rc = fill_extLibpath(1, s, NULL, 1, "PERL_ENDLIBPATH");
        else
            rc = fill_extLibpath(1, PerlEnv_getenv("PERL_PRE_ENDLIBPATH"), PerlEnv_getenv("PERL_POST_ENDLIBPATH"), 0, "PERL_(PRE/POST)_ENDLIBPATH");
    }
    if (rc) {
        char buf[1024];

        snprintf(buf, sizeof buf, "Error setting BEGIN/ENDLIBPATH: %s\n",
                 os2error(rc));
        DosWrite(2, buf, strlen(buf), &rc);
        exit(2);
    }

    _emxload_env("PERL_EMXLOAD_SECS");
    /* Some DLLs reset FP flags on load.  We may have been linked with them */
    _control87(MCW_EM, MCW_EM);
}

int
fd_ok(int fd)
{
    static ULONG max_fh = 0;

    if (!(_emx_env & 0x200)) return 1;		/* not OS/2. */
    if (fd >= max_fh) {				/* Renew */
        LONG delta = 0;

        if (DosSetRelMaxFH(&delta, &max_fh))	/* Assume it OK??? */
            return 1;
    }
    return fd < max_fh;
}

/* Kernels up to Oct 2003 trap on (invalid) dup(max_fh); [off-by-one + double fault].  */
int
dup2(int from, int to)
{
    if (fd_ok(from < to ? to : from))
        return _dup2(from, to);
    errno = EBADF;
    return -1;
}

int
dup(int from)
{
    if (fd_ok(from))
        return _dup(from);
    errno = EBADF;
    return -1;
}

#undef tmpnam
#undef tmpfile

char *
my_tmpnam (char *str)
{
    char *p = PerlEnv_getenv("TMP"), *tpath;

    if (!p) p = PerlEnv_getenv("TEMP");
    ENV_READ_LOCK;
    tpath = tempnam(p, "pltmp");
    if (str && tpath) {
        strcpy(str, tpath);
        ENV_READ_UNLOCK;
        return str;
    }
    ENV_READ_UNLOCK;
    return tpath;
}

FILE *
my_tmpfile ()
{
    struct stat s;

    stat(".", &s);
    if (s.st_mode & S_IWOTH) {
        return tmpfile();
    }
    return fopen(my_tmpnam(NULL), "w+b"); /* Race condition, but
                                             grants TMP. */
}

#undef rmdir

/* EMX flavors do not tolerate trailing slashes.  t/op/mkdir.t has many
   trailing slashes, so we need to support this as well. */

int
my_rmdir (__const__ char *s)
{
    char b[MAXPATHLEN];
    char *buf = b;
    STRLEN l = strlen(s);
    int rc;

    if (s[l-1] == '/' || s[l-1] == '\\') {	/* EMX mkdir fails... */
        if (l >= sizeof b)
            Newx(buf, l + 1, char);
        strcpy(buf,s);
        while (l > 1 && (s[l-1] == '/' || s[l-1] == '\\'))
            l--;
        buf[l] = 0;
        s = buf;
    }
    rc = rmdir(s);
    if (b != buf)
        Safefree(buf);
    return rc;
}

#undef mkdir

int
my_mkdir (__const__ char *s, long perm)
{
    char b[MAXPATHLEN];
    char *buf = b;
    STRLEN l = strlen(s);
    int rc;

    if (s[l-1] == '/' || s[l-1] == '\\') {	/* EMX mkdir fails... */
        if (l >= sizeof b)
            Newx(buf, l + 1, char);
        strcpy(buf,s);
        while (l > 1 && (s[l-1] == '/' || s[l-1] == '\\'))
            l--;
        buf[l] = 0;
        s = buf;
    }
    rc = mkdir(s, perm);
    if (b != buf)
        Safefree(buf);
    return rc;
}

#undef flock

/* This code was contributed by Rocco Caputo. */
int 
my_flock(int handle, int o)
{
  FILELOCK      rNull, rFull;
  ULONG         timeout, handle_type, flag_word;
  APIRET        rc;
  int           blocking, shared;
  static int	use_my_flock = -1;

  if (use_my_flock == -1) {
   MUTEX_LOCK(&perlos2_state_mutex);
   if (use_my_flock == -1) {
    char *s = PerlEnv_getenv("USE_PERL_FLOCK");
    if (s)
        use_my_flock = atoi(s);
    else 
        use_my_flock = 1;
   }
   MUTEX_UNLOCK(&perlos2_state_mutex);
  }
  if (!(_emx_env & 0x200) || !use_my_flock) 
    return flock(handle, o);	/* Delegate to EMX. */
  
                                        /* is this a file? */
  if ((DosQueryHType(handle, &handle_type, &flag_word) != 0) ||
      (handle_type & 0xFF))
  {
    errno = EBADF;
    return -1;
  }
                                        /* set lock/unlock ranges */
  rNull.lOffset = rNull.lRange = rFull.lOffset = 0;
  rFull.lRange = 0x7FFFFFFF;
                                        /* set timeout for blocking */
  timeout = ((blocking = !(o & LOCK_NB))) ? 100 : 1;
                                        /* shared or exclusive? */
  shared = (o & LOCK_SH) ? 1 : 0;
                                        /* do not block the unlock */
  if (o & (LOCK_UN | LOCK_SH | LOCK_EX)) {
    rc = DosSetFileLocks(handle, &rFull, &rNull, timeout, shared);
    switch (rc) {
      case 0:
        errno = 0;
        return 0;
      case ERROR_INVALID_HANDLE:
        errno = EBADF;
        return -1;
      case ERROR_SHARING_BUFFER_EXCEEDED:
        errno = ENOLCK;
        return -1;
      case ERROR_LOCK_VIOLATION:
        break;                          /* not an error */
      case ERROR_INVALID_PARAMETER:
      case ERROR_ATOMIC_LOCK_NOT_SUPPORTED:
      case ERROR_READ_LOCKS_NOT_SUPPORTED:
        errno = EINVAL;
        return -1;
      case ERROR_INTERRUPT:
        errno = EINTR;
        return -1;
      default:
        errno = EINVAL;
        return -1;
    }
  }
                                        /* lock may block */
  if (o & (LOCK_SH | LOCK_EX)) {
                                        /* for blocking operations */
    for (;;) {
      rc =
        DosSetFileLocks(
                handle,
                &rNull,
                &rFull,
                timeout,
                shared
        );
      switch (rc) {
        case 0:
          errno = 0;
          return 0;
        case ERROR_INVALID_HANDLE:
          errno = EBADF;
          return -1;
        case ERROR_SHARING_BUFFER_EXCEEDED:
          errno = ENOLCK;
          return -1;
        case ERROR_LOCK_VIOLATION:
          if (!blocking) {
            errno = EWOULDBLOCK;
            return -1;
          }
          break;
        case ERROR_INVALID_PARAMETER:
        case ERROR_ATOMIC_LOCK_NOT_SUPPORTED:
        case ERROR_READ_LOCKS_NOT_SUPPORTED:
          errno = EINVAL;
          return -1;
        case ERROR_INTERRUPT:
          errno = EINTR;
          return -1;
        default:
          errno = EINVAL;
          return -1;
      }
                                        /* give away timeslice */
      DosSleep(1);
    }
  }

  errno = 0;
  return 0;
}

static int
use_my_pwent(void)
{
  if (_my_pwent == -1) {
    char *s = PerlEnv_getenv("USE_PERL_PWENT");
    if (s)
        _my_pwent = atoi(s);
    else 
        _my_pwent = 1;
  }
  return _my_pwent;
}

#undef setpwent
#undef getpwent
#undef endpwent

void
my_setpwent(void)
{
  if (!use_my_pwent()) {
    setpwent();			/* Delegate to EMX. */
    return;
  }
  pwent_cnt = 0;
}

void
my_endpwent(void)
{
  if (!use_my_pwent()) {
    endpwent();			/* Delegate to EMX. */
    return;
  }
}

struct passwd *
my_getpwent (void)
{
  if (!use_my_pwent())
    return getpwent();			/* Delegate to EMX. */
  if (pwent_cnt++)
    return 0;				/* Return one entry only */
  return getpwuid(0);
}

void
setgrent(void)
{
  grent_cnt = 0;
}

void
endgrent(void)
{
}

struct group *
getgrent (void)
{
  if (grent_cnt++)
    return 0;				/* Return one entry only */
  return getgrgid(0);
}

#undef getpwuid
#undef getpwnam

/* Too long to be a crypt() of anything, so it is not-a-valid pw_passwd. */
static const char pw_p[] = "Jf0Wb/BzMFvk7K7lrzK";

static struct passwd *
passw_wrap(struct passwd *p)
{
    char *s;

    if (!p || (p->pw_passwd && *p->pw_passwd)) /* Not a dangerous password */
        return p;
    pw = *p;
    s = PerlEnv_getenv("PW_PASSWD");
    if (!s)
        s = (char*)pw_p;		/* Make match impossible */


    pw.pw_passwd = s;
    return &pw;    
}

struct passwd *
my_getpwuid (uid_t id)
{
    /* On Linux, only getpwuid_r is thread safe, and even then not if the
     * locale changes */

    return passw_wrap(getpwuid(id));
}

struct passwd *
my_getpwnam (__const__ char *n)
{
    /* On Linux, only getpwnam_r is thread safe, and even then not if the
     * locale changes */

    return passw_wrap(getpwnam(n));
}

char *
gcvt_os2 (double value, int digits, char *buffer)
{
  double absv = value > 0 ? value : -value;
  /* EMX implementation is lousy between 0.1 and 0.0001 (uses exponents below
     0.1), 1-digit stuff is ok below 0.001; multi-digit below 0.0001. */
  int buggy;

  absv *= 10000;
  buggy = (absv < 1000 && (absv >= 10 || (absv > 1 && floor(absv) != absv)));
  
  if (buggy) {
    char pat[12];

    sprintf(pat, "%%.%dg", digits);
    sprintf(buffer, pat, value);
    return buffer;
  }
  return gcvt (value, digits, buffer);
}

#undef fork
int fork_with_resources()
{
#if defined(USE_ITHREADS) && !defined(USE_SLOW_THREAD_SPECIFIC)
  dTHX;
  void *ctx = PERL_GET_CONTEXT;
#endif
  unsigned fpflag = _control87(0,0);
  int rc = fork();

  if (rc == 0) {			/* child */
#if defined(USE_ITHREADS) && !defined(USE_SLOW_THREAD_SPECIFIC)
    ALLOC_THREAD_KEY;			/* Acquire the thread-local memory */
    PERL_SET_CONTEXT(ctx);		/* Reinit the thread-local memory */
#endif
    
    {					/* Reload loaded-on-demand DLLs */
        struct dll_handle_t *dlls = dll_handles;

        while (dlls->modname) {
            char dllname[260], fail[260];
            ULONG rc;

            if (!dlls->handle) {	/* Was not loaded */
                dlls++;
                continue;
            }
            /* It was loaded in the parent.  We need to reload it. */

            rc = DosQueryModuleName(dlls->handle, sizeof(dllname), dllname);
            if (rc) {
                Perl_warn_nocontext("Can't find DLL name for the module `%s' by the handle %d, rc=%lu=%#lx",
                                    dlls->modname, (int)dlls->handle, rc, rc);
                dlls++;
                continue;
            }
            rc = DosLoadModule(fail, sizeof fail, dllname, &dlls->handle);
            if (rc)
                Perl_warn_nocontext("Can't load DLL `%s', possible problematic module `%s'",
                                    dllname, fail);
            dlls++;
        }
    }
    
    {					/* Support message queue etc. */
        os2_mytype = my_type();
        /* Apparently, subprocesses (in particular, fork()) do not
           inherit the morphed state, so os2_mytype is the same as
           os2_mytype_ini. */

        if (Perl_os2_initial_mode != -1
            && Perl_os2_initial_mode != os2_mytype) {
                                        /* XXXX ??? */
        }
    }
    if (Perl_HAB_set)
        (void)_obtain_Perl_HAB;
    if (Perl_hmq_refcnt) {
        if (my_type() != 3)
            my_type_set(3);
        Create_HMQ(Perl_hmq_servers != 0,
                   "Cannot create a message queue on fork");
    }

    /* We may have loaded some modules */
    _control87(fpflag, MCW_EM); /* Some modules reset FP flags on (un)load */
  }
  return rc;
}

/* APIRET  APIENTRY DosGetInfoSeg(PSEL pselGlobal, PSEL pselLocal); */

ULONG _THUNK_FUNCTION(Dos16GetInfoSeg)(USHORT *pGlobal, USHORT *pLocal);

APIRET  APIENTRY
myDosGetInfoSeg(PGINFOSEG *pGlobal, PLINFOSEG *pLocal)
{
    APIRET rc;
    USHORT gSel, lSel;		/* Will not cross 64K boundary */

    rc = ((USHORT)
          (_THUNK_PROLOG (4+4);
           _THUNK_FLAT (&gSel);
           _THUNK_FLAT (&lSel);
           _THUNK_CALL (Dos16GetInfoSeg)));
    if (rc)
        return rc;
    *pGlobal = MAKEPGINFOSEG(gSel);
    *pLocal  = MAKEPLINFOSEG(lSel);
    return rc;
}

static void
GetInfoTables(void)
{
    ULONG rc = 0;

    MUTEX_LOCK(&perlos2_state_mutex);
    if (!gTable)
      rc = myDosGetInfoSeg(&gTable, &lTable);
    MUTEX_UNLOCK(&perlos2_state_mutex);
    os2cp_croak(rc, "Dos16GetInfoSeg");
}

ULONG
msCounter(void)
{				/* XXXX Is not lTable thread-specific? */
  if (!gTable)
    GetInfoTables();
  return gTable->SIS_MsCount;
}

ULONG
InfoTable(int local)
{
  if (!gTable)
    GetInfoTables();
  return local ? (ULONG)lTable : (ULONG)gTable;
}
