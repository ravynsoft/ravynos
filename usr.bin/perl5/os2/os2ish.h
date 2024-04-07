#include <signal.h>
#include <io.h>
/* #include <sys/select.h> */

/* HAS_IOCTL:
 *	This symbol, if defined, indicates that the ioctl() routine is
 *	available to set I/O characteristics
 */
#define HAS_IOCTL               /**/
 
/* HAS_UTIME:
 *	This symbol, if defined, indicates that the routine utime() is
 *	available to update the access and modification times of files.
 */
#define HAS_UTIME		/**/

/* BIG_TIME:
 *	This symbol is defined if Time_t is an unsigned type on this system.
 */
#define BIG_TIME

#define HAS_KILL
#define HAS_WAIT
#define HAS_DLERROR
#define HAS_WAITPID_RUNTIME (_emx_env & 0x200)

/* HAS_PASSWD
 *	This symbol, if defined, indicates that the getpwnam() and
 *	getpwuid() routines are available to get password entries.
 *	The getpwent() has a separate definition, HAS_GETPWENT.
 */
#define HAS_PASSWD

/* HAS_GROUP
 *	This symbol, if defined, indicates that the getgrnam() and
 *	getgrgid() routines are available to get group entries.
 *	The getgrent() has a separate definition, HAS_GETGRENT.
 */
#define HAS_GROUP
#define HAS_GETGRENT			/* fake */
#define HAS_SETGRENT			/* fake */
#define HAS_ENDGRENT			/* fake */

/* USEMYBINMODE
 *	This symbol, if defined, indicates that the program should
 *	use the routine my_binmode(FILE *fp, char iotype, int mode) to insure
 *	that a file is in "binary" mode -- that is, that no translation
 *	of bytes occurs on read or write operations.
 */
#undef USEMYBINMODE

#define SOCKET_OPEN_MODE	"b"

/* Stat_t:
 *	This symbol holds the type used to declare buffers for information
 *	returned by stat().  It's usually just struct stat.  It may be necessary
 *	to include <sys/stat.h> and <sys/types.h> to get any typedef'ed
 *	information.
 */
#define Stat_t struct stat

/* USE_STAT_RDEV:
 *	This symbol is defined if this system has a stat structure declaring
 *	st_rdev
 */
#define USE_STAT_RDEV 	/**/

/* ACME_MESS:
 *	This symbol, if defined, indicates that error messages should be 
 *	should be generated in a format that allows the use of the Acme
 *	GUI/editor's autofind feature.
 */
#undef ACME_MESS	/**/

/* ALTERNATE_SHEBANG:
 *	This symbol, if defined, contains a "magic" string which may be used
 *	as the first line of a Perl program designed to be executed directly
 *	by name, instead of the standard Unix #!.  If ALTERNATE_SHEBANG
 *	begins with a character other then #, then Perl will only treat
 *	it as a command line if it finds the string "perl" in the first
 *	word; otherwise it's treated as the first line of code in the script.
 *	(IOW, Perl won't hand off to another interpreter via an alternate
 *	shebang sequence that might be legal Perl code.)
 */
#define ALTERNATE_SHEBANG "extproc "

#ifndef SIGABRT
#    define SIGABRT SIGILL
#endif
#ifndef SIGILL
#    define SIGILL 6         /* blech */
#endif
#define ABORT() kill(PerlProc_getpid(),SIGABRT);

#define BIT_BUCKET "/dev/nul"  /* Will this work? */

/* Apparently TCPIPV4 defines may be included even with only IAK present */

#if !defined(NO_TCPIPV4) && !defined(TCPIPV4)
#  define TCPIPV4
#  define TCPIPV4_FORCED		/* Just in case */
#endif

#if defined(I_SYS_UN) && !defined(TCPIPV4)
/* It is not working without TCPIPV4 defined. */
# undef I_SYS_UN
#endif 

#ifdef USE_ITHREADS

#define do_spawn(a)      os2_do_spawn(aTHX_ (a))
#define do_aspawn(a,b,c) os2_do_aspawn(aTHX_ (a),(b),(c))

#define OS2_ERROR_ALREADY_POSTED 299	/* Avoid os2.h */

extern int rc;

#define MUTEX_INIT(m) \
    STMT_START {						\
        int rc;							\
        if ((rc = _rmutex_create(m,0)))				\
            Perl_croak_nocontext("panic: MUTEX_INIT: rc=%i", rc);	\
    } STMT_END
#define MUTEX_LOCK(m) \
    STMT_START {						\
        int rc;							\
        if ((rc = _rmutex_request(m,_FMR_IGNINT)))		\
            Perl_croak_nocontext("panic: MUTEX_LOCK: rc=%i", rc);	\
    } STMT_END
#define MUTEX_UNLOCK(m) \
    STMT_START {						\
        int rc;							\
        if ((rc = _rmutex_release(m)))				\
            Perl_croak_nocontext("panic: MUTEX_UNLOCK: rc=%i", rc);	\
    } STMT_END
#define MUTEX_DESTROY(m) \
    STMT_START {						\
        int rc;							\
        if ((rc = _rmutex_close(m)))				\
            Perl_croak_nocontext("panic: MUTEX_DESTROY: rc=%i", rc);	\
    } STMT_END

#define COND_INIT(c) \
    STMT_START {						\
        int rc;							\
        if ((rc = DosCreateEventSem(NULL,c,0,0)))		\
            Perl_croak_nocontext("panic: COND_INIT: rc=%i", rc);	\
    } STMT_END
#define COND_SIGNAL(c) \
    STMT_START {						\
        int rc;							\
        if ((rc = DosPostEventSem(*(c))) && rc != OS2_ERROR_ALREADY_POSTED)\
            Perl_croak_nocontext("panic: COND_SIGNAL, rc=%ld", rc);	\
    } STMT_END
#define COND_BROADCAST(c) \
    STMT_START {						\
        int rc;							\
        if ((rc = DosPostEventSem(*(c))) && rc != OS2_ERROR_ALREADY_POSTED)\
            Perl_croak_nocontext("panic: COND_BROADCAST, rc=%i", rc);	\
    } STMT_END
/* #define COND_WAIT(c, m) \
    STMT_START {						\
        if (WaitForSingleObject(*(c),INFINITE) == WAIT_FAILED)	\
            Perl_croak_nocontext("panic: COND_WAIT");		\
    } STMT_END
*/
#define COND_WAIT(c, m) os2_cond_wait(c,m)

#define COND_WAIT_win32(c, m) \
    STMT_START {						\
        int rc;							\
        if ((rc = SignalObjectAndWait(*(m),*(c),INFINITE,FALSE)))	\
            Perl_croak_nocontext("panic: COND_WAIT");			\
        else							\
            MUTEX_LOCK(m);					\
    } STMT_END
#define COND_DESTROY(c) \
    STMT_START {						\
        int rc;							\
        if ((rc = DosCloseEventSem(*(c))))			\
            Perl_croak_nocontext("panic: COND_DESTROY, rc=%i", rc);	\
    } STMT_END
/*#define THR ((struct thread *) TlsGetValue(PL_thr_key))
*/

#ifdef USE_SLOW_THREAD_SPECIFIC
#  define pthread_getspecific(k)	(*_threadstore())
#  define pthread_setspecific(k,v)	(*_threadstore()=v,0)
#  define pthread_key_create(keyp,flag)	(*keyp=_gettid(),0)
#else /* USE_SLOW_THREAD_SPECIFIC */
#  define pthread_getspecific(k)	(*(k))
#  define pthread_setspecific(k,v)	(*(k)=(v),0)
#  define pthread_key_create(keyp,flag)			\
        ( DosAllocThreadLocalMemory(1,(unsigned long**)keyp)	\
          ? Perl_croak_nocontext("Out of memory!"), 1        \
          : 0						\
        )
#endif /* USE_SLOW_THREAD_SPECIFIC */
#define pthread_key_delete(keyp)
#define pthread_self()			_gettid()
#define YIELD				DosSleep(0)

#ifdef PTHREADS_INCLUDED		/* For ./x2p stuff. */
int pthread_join(pthread_t tid, void **status);
int pthread_detach(pthread_t tid);
int pthread_create(pthread_t *tid, const pthread_attr_t *attr,
                   void *(*start_routine)(void*), void *arg);
#endif /* PTHREAD_INCLUDED */

#define THREADS_ELSEWHERE

#else /* USE_ITHREADS */

#define do_spawn(a)      os2_do_spawn(a)
#define do_aspawn(a,b,c) os2_do_aspawn((a),(b),(c))
 
void Perl_OS2_init(char **);
void Perl_OS2_init3(char **envp, void **excH, int flags);
void Perl_OS2_term(void **excH, int exitstatus, int flags);

/* The code without INIT3 hideously puts env inside: */

/* These ones should be in the same block as PERL_SYS_TERM() */
#ifdef PERL_CORE

#  define PERL_SYS_INIT3_BODY(argcp, argvp, envp)	\
  { void *xreg[2];				\
    MALLOC_CHECK_TAINT(*argcp, *argvp, *envp)	\
    _response(argcp, argvp);			\
    _wildcard(argcp, argvp);			\
    Perl_OS2_init3(*envp, xreg, 0);		\
    PERLIO_INIT

#  define PERL_SYS_INIT_BODY(argcp, argvp)  {	\
  { void *xreg[2];				\
    _response(argcp, argvp);			\
    _wildcard(argcp, argvp);			\
    Perl_OS2_init3(NULL, xreg, 0);		\
    PERLIO_INIT

#else  /* Compiling embedded Perl or Perl extension */

#  define PERL_SYS_INIT3_BODY(argcp, argvp, envp)	\
  { void *xreg[2];				\
    Perl_OS2_init3(*envp, xreg, 0);		\
    PERLIO_INIT
#  define PERL_SYS_INIT_BODY(argcp, argvp)	{	\
  { void *xreg[2];				\
    Perl_OS2_init3(NULL, xreg, 0);		\
    PERLIO_INIT
#endif

#define FORCE_EMX_DEINIT_EXIT		1
#define FORCE_EMX_DEINIT_CRT_TERM	2
#define FORCE_EMX_DEINIT_RUN_ATEXIT	4

#define PERL_SYS_TERM2(xreg,flags)					\
  Perl_OS2_term(xreg, 0, flags);					\
  PERLIO_TERM;								\
  MALLOC_TERM

#define PERL_SYS_TERM1(xreg)						\
     Perl_OS2_term(xreg, 0, FORCE_EMX_DEINIT_RUN_ATEXIT)

/* This one should come in pair with PERL_SYS_INIT_BODY() and in the same block */
#define PERL_SYS_TERM_BODY()							\
     PERL_SYS_TERM1(xreg);						\
  }

#ifndef __EMX__
#  define PERL_CALLCONV _System
#endif

/* #define PERL_SYS_TERM_BODY() STMT_START {	\
    if (Perl_HAB_set) WinTerminate(Perl_hab);	} STMT_END */

#define dXSUB_SYS int fake = OS2_XS_init() PERL_UNUSED_DECL

#ifdef PERL_IS_AOUT
/* #  define HAS_FORK */
/* #  define HIDEMYMALLOC */
/* #  define PERL_SBRK_VIA_MALLOC */ /* gets off-page sbrk... */
#else /* !PERL_IS_AOUT */
#  ifndef PERL_FOR_X2P
#    ifdef EMX_BAD_SBRK
#      define USE_PERL_SBRK
#    endif 
#  else
#    define PerlIO FILE
#  endif 
#  define SYSTEM_ALLOC(a) sys_alloc(a)

void *sys_alloc(int size);

#endif /* !PERL_IS_AOUT */
#if !defined(PERL_CORE) && !defined(PerlIO) /* a2p */
#  define PerlIO FILE
#endif 

/* os2ish is used from a2p/a2p.h without pTHX/pTHX_ first being
 * defined.  Hack around this to get us to compile.
*/
#ifdef PTHX_UNUSED
# ifndef pTHX
#  define pTHX
# endif
# ifndef pTHX_
#  define pTHX_
# endif
#endif

#define TMPPATH1 "plXXXXXX"
extern const char *tmppath;
PerlIO *my_syspopen(pTHX_ char *cmd, char *mode);
#ifdef PERL_CORE
/* Cannot prototype with I32, SV at this point (used in x2p too). */
PerlIO *my_syspopen4(pTHX_ char *cmd, char *mode, I32 cnt, SV** args);
#endif
int my_syspclose(PerlIO *f);
FILE *my_tmpfile (void);
char *my_tmpnam (char *);
int my_mkdir (__const__ char *, long);
int my_rmdir (__const__ char *);
struct passwd *my_getpwent (void);
void my_setpwent (void);
void my_endpwent (void);
char *gcvt_os2(double value, int digits, char *buffer);

extern int async_mssleep(unsigned long ms, int switch_priority);
extern unsigned long msCounter(void);
extern unsigned long InfoTable(int local);
extern unsigned long find_myself(void);

#define MAX_SLEEP	(((1<30) / (1000/4))-1)	/* 1<32 msec */

static __inline__ unsigned
my_sleep(unsigned sec)
{
  int remain;
  while (sec > MAX_SLEEP) {
    sec -= MAX_SLEEP;
    remain = sleep(MAX_SLEEP);
    if (remain)
      return remain + sec;
  }
  return sleep(sec);
}

#define sleep		my_sleep

#ifndef INCL_DOS
unsigned long DosSleep(unsigned long);
unsigned long DosAllocThreadLocalMemory (unsigned long cb, unsigned long **p);
#endif

struct group *getgrent (void);
void setgrent (void);
void endgrent (void);

struct passwd *my_getpwuid (uid_t);
struct passwd *my_getpwnam (__const__ char *);

#undef L_tmpnam
#define L_tmpnam MAXPATHLEN

#define tmpfile	my_tmpfile
#define tmpnam	my_tmpnam
#define isatty	_isterm
#define rand	random
#define srand	srandom
#define strtoll	_strtoll
#define strtoull	_strtoull

#define usleep(usec)	((void)async_mssleep(((usec)+500)/1000, 500))


/*
 * fwrite1() should be a routine with the same calling sequence as fwrite(),
 * but which outputs all of the bytes requested as a single stream (unlike
 * fwrite() itself, which on some systems outputs several distinct records
 * if the number_of_items parameter is >1).
 */
#define fwrite1 fwrite

#define my_getenv(var) getenv(var)
#define flock	my_flock
#define rmdir	my_rmdir
#define mkdir	my_mkdir
#define setpwent	my_setpwent
#define getpwent	my_getpwent
#define endpwent	my_endpwent
#define getpwuid	my_getpwuid
#define getpwnam	my_getpwnam

void *emx_calloc (size_t, size_t);
void emx_free (void *);
void *emx_malloc (size_t);
void *emx_realloc (void *, size_t);

/*****************************************************************************/

#include <stdlib.h>	/* before the following definitions */
#include <unistd.h>	/* before the following definitions */
#include <fcntl.h>
#include <sys/stat.h>

#define chdir	_chdir2
#define getcwd	_getcwd2

/* This guy is needed for quick stdstd  */

#if defined(USE_STDIO_PTR) && defined(STDIO_PTR_LVALUE) && defined(STDIO_CNT_LVALUE)
        /* Perl uses ungetc only with successful return */
#  define ungetc(c,fp) \
        (FILE_ptr(fp) > FILE_base(fp) && c == (int)*(FILE_ptr(fp) - 1) \
         ? (--FILE_ptr(fp), ++FILE_cnt(fp), (int)c) : ungetc(c,fp))
#endif

#define PERLIO_IS_BINMODE_FD(fd) _PERLIO_IS_BINMODE_FD(fd)

#include <emx/io.h> /* for _fd_flags() prototype */

static inline bool
_PERLIO_IS_BINMODE_FD(int fd)
{
    int *pflags = _fd_flags(fd);

    return pflags && (*pflags) & O_BINARY;
}

/* ctermid is missing from emx0.9d */
char *ctermid(char *s);

#define OP_BINARY O_BINARY

#define OS2_STAT_HACK 1
#if OS2_STAT_HACK

#define Stat(fname,bufptr) os2_stat((fname),(bufptr))
#define Fstat(fd,bufptr)   os2_fstat((fd),(bufptr))
#define Fflush(fp)         fflush(fp)
#define Mkdir(path,mode)   mkdir((path),(mode))
#define chmod(path,mode)   os2_chmod((path),(mode))

#undef S_IFBLK
#undef S_ISBLK
#define S_IFBLK		0120000		/* Hacks to make things compile... */
#define S_ISBLK(mode)	(((mode) & S_IFMT) == S_IFBLK)

int os2_chmod(const char *name, int pmode);
int os2_fstat(int handle, struct stat *st);

#else

#define Stat(fname,bufptr) stat((fname),(bufptr))
#define Fstat(fd,bufptr)   fstat((fd),(bufptr))
#define Fflush(fp)         fflush(fp)
#define Mkdir(path,mode)   mkdir((path),(mode))

#endif

/* With SD386 it is impossible to debug register variables. */
#if !defined(PERL_IS_AOUT) && defined(DEBUGGING) && !defined(register)
#  define register
#endif

/* Our private OS/2 specific data. */

typedef struct OS2_Perl_data {
  unsigned long flags;
  unsigned long phab;
  int (*xs_init)();
  unsigned long rc;
  unsigned long severity;
  unsigned long	phmq;			/* Handle to message queue */
  unsigned long	phmq_refcnt;
  unsigned long	phmq_servers;
  unsigned long	initial_mode;		/* VIO etc. mode we were started in */
  unsigned long	morph_refcnt;
} OS2_Perl_data_t;

extern OS2_Perl_data_t OS2_Perl_data;

#define Perl_hab		((HAB)OS2_Perl_data.phab)
#define Perl_rc			(OS2_Perl_data.rc)
#define Perl_severity		(OS2_Perl_data.severity)
#define errno_isOS2		12345678
#define errno_isOS2_set		12345679
#define OS2_Perl_flags	(OS2_Perl_data.flags)
#define Perl_HAB_set_f	1
#define Perl_HAB_set	(OS2_Perl_flags & Perl_HAB_set_f)
#define set_Perl_HAB_f	(OS2_Perl_flags |= Perl_HAB_set_f)
#define set_Perl_HAB(h) (set_Perl_HAB_f, Perl_hab = h)
#define _obtain_Perl_HAB (init_PMWIN_entries(),				\
                          Perl_hab = (*PMWIN_entries.Initialize)(0),	\
                          set_Perl_HAB_f, Perl_hab)
#define perl_hab_GET()	(Perl_HAB_set ? Perl_hab : _obtain_Perl_HAB)
#define Acquire_hab()	perl_hab_GET()
#define Perl_hmq	((HMQ)OS2_Perl_data.phmq)
#define Perl_hmq_refcnt	(OS2_Perl_data.phmq_refcnt)
#define Perl_hmq_servers	(OS2_Perl_data.phmq_servers)
#define Perl_os2_initial_mode	(OS2_Perl_data.initial_mode)
#define Perl_morph_refcnt	(OS2_Perl_data.morph_refcnt)

unsigned long Perl_hab_GET();
unsigned long Perl_Register_MQ(int serve);
void	Perl_Deregister_MQ(int serve);
int	Perl_Serve_Messages(int force);
/* Cannot prototype with I32 at this point. */
int	Perl_Process_Messages(int force, long *cntp);
char	*os2_execname(pTHX);

struct _QMSG;
struct PMWIN_entries_t {
    unsigned long (*Initialize)( unsigned long fsOptions );
    unsigned long (*CreateMsgQueue)(unsigned long hab, long cmsg);
    int (*DestroyMsgQueue)(unsigned long hmq);
    int (*PeekMsg)(unsigned long hab, struct _QMSG *pqmsg,
                   unsigned long hwndFilter, unsigned long msgFilterFirst,
                   unsigned long msgFilterLast, unsigned long fl);
    int (*GetMsg)(unsigned long hab, struct _QMSG *pqmsg,
                  unsigned long hwndFilter, unsigned long msgFilterFirst,
                  unsigned long msgFilterLast);
    void * (*DispatchMsg)(unsigned long hab, struct _QMSG *pqmsg);
    unsigned long (*GetLastError)(unsigned long hab);
    unsigned long (*CancelShutdown)(unsigned long hmq, unsigned long fCancelAlways);
};
extern struct PMWIN_entries_t PMWIN_entries;
void init_PMWIN_entries(void);

#define perl_hmq_GET(serve)	Perl_Register_MQ(serve)
#define perl_hmq_UNSET(serve)	Perl_Deregister_MQ(serve)

#define OS2_XS_init() (*OS2_Perl_data.xs_init)(aTHX)

#if _EMX_CRT_REV_ >= 60
# define os2_setsyserrno(rc)	(Perl_rc = rc, errno = errno_isOS2_set, \
                                _setsyserrno(rc))
#else
# define os2_setsyserrno(rc)	(Perl_rc = rc, errno = errno_isOS2)
#endif

/* The expressions below return true on error. */
/* INCL_DOSERRORS needed. rc should be declared outside. */
#define CheckOSError(expr) ((rc = (expr)) ? (FillOSError(rc), rc) : 0)
/* INCL_WINERRORS needed. */
#define CheckWinError(expr) ((expr) ? 0: (FillWinError, 1))

/* This form propagates the return value, setting $^E if needed */
#define SaveWinError(expr) ((expr) ? : (FillWinError, 0))

/* This form propagates the return value, dieing with $^E if needed */
#define SaveCroakWinError(expr,die,name1,name2)		\
  ((expr) ? : (CroakWinError(die,name1 name2), 0))

#define FillOSError(rc) (os2_setsyserrno(rc),				\
                        Perl_severity = SEVERITY_ERROR) 

#define WinError_2_Perl_rc	\
 (	init_PMWIN_entries(),	\
        Perl_rc=(*PMWIN_entries.GetLastError)(perl_hab_GET()) )

/* Calling WinGetLastError() resets the error code of the current thread.
   Since for some Win* API return value 0 is normal, one needs to call
   this before calling them to distinguish normal and anomalous returns.  */
/*#define ResetWinError()	WinError_2_Perl_rc */

/* At this moment init_PMWIN_entries() should be a nop (WinInitialize should
   be called already, right?), so we do not risk stepping over our own error */
#define FillWinError (	WinError_2_Perl_rc,				\
                        Perl_severity = ERRORIDSEV(Perl_rc),		\
                        Perl_rc = ERRORIDERROR(Perl_rc),		\
                        os2_setsyserrno(Perl_rc))

#define STATIC_FILE_LENGTH 127

    /* This should match loadOrdinals[] array in os2.c */
enum entries_ordinals {
    ORD_DosQueryExtLibpath,
    ORD_DosSetExtLibpath,
    ORD_DosVerifyPidTid,
    ORD_SETHOSTENT,
    ORD_SETNETENT, 
    ORD_SETPROTOENT,
    ORD_SETSERVENT,
    ORD_GETHOSTENT,
    ORD_GETNETENT, 
    ORD_GETPROTOENT,
    ORD_GETSERVENT,
    ORD_ENDHOSTENT,
    ORD_ENDNETENT,
    ORD_ENDPROTOENT,
    ORD_ENDSERVENT,
    ORD_WinInitialize,
    ORD_WinCreateMsgQueue,
    ORD_WinDestroyMsgQueue,
    ORD_WinPeekMsg,
    ORD_WinGetMsg,
    ORD_WinDispatchMsg,
    ORD_WinGetLastError,
    ORD_WinCancelShutdown,
    ORD_RexxStart,
    ORD_RexxVariablePool,
    ORD_RexxRegisterFunctionExe,
    ORD_RexxDeregisterFunction,
    ORD_DOSSMSETTITLE,
    ORD_PRF32QUERYPROFILESIZE,
    ORD_PRF32OPENPROFILE,
    ORD_PRF32CLOSEPROFILE,
    ORD_PRF32QUERYPROFILE,
    ORD_PRF32RESET,
    ORD_PRF32QUERYPROFILEDATA,
    ORD_PRF32WRITEPROFILEDATA,

    ORD_WinChangeSwitchEntry,
    ORD_WinQuerySwitchEntry,
    ORD_WinQuerySwitchHandle,
    ORD_WinQuerySwitchList,
    ORD_WinSwitchToProgram,
    ORD_WinBeginEnumWindows,
    ORD_WinEndEnumWindows,
    ORD_WinEnumDlgItem,
    ORD_WinGetNextWindow,
    ORD_WinIsChild,
    ORD_WinQueryActiveWindow,
    ORD_WinQueryClassName,
    ORD_WinQueryFocus,
    ORD_WinQueryWindow,
    ORD_WinQueryWindowPos,
    ORD_WinQueryWindowProcess,
    ORD_WinQueryWindowText,
    ORD_WinQueryWindowTextLength,
    ORD_WinSetFocus,
    ORD_WinSetWindowPos,
    ORD_WinSetWindowText,
    ORD_WinShowWindow,
    ORD_WinIsWindow,
    ORD_WinWindowFromId,
    ORD_WinWindowFromPoint,
    ORD_WinPostMsg,
    ORD_WinEnableWindow,
    ORD_WinEnableWindowUpdate,
    ORD_WinIsWindowEnabled,
    ORD_WinIsWindowShowing,
    ORD_WinIsWindowVisible,
    ORD_WinQueryWindowPtr,
    ORD_WinQueryWindowULong,
    ORD_WinQueryWindowUShort,
    ORD_WinSetWindowBits,
    ORD_WinSetWindowPtr,
    ORD_WinSetWindowULong,
    ORD_WinSetWindowUShort,
    ORD_WinQueryDesktopWindow,
    ORD_WinSetActiveWindow,
    ORD_DosQueryModFromEIP,
    ORD_Dos32QueryHeaderInfo,
    ORD_DosTmrQueryFreq,
    ORD_DosTmrQueryTime,
    ORD_WinQueryActiveDesktopPathname,
    ORD_WinInvalidateRect,
    ORD_WinCreateFrameControls,
    ORD_WinQueryClipbrdFmtInfo,
    ORD_WinQueryClipbrdOwner,
    ORD_WinQueryClipbrdViewer,
    ORD_WinQueryClipbrdData,
    ORD_WinOpenClipbrd,
    ORD_WinCloseClipbrd,
    ORD_WinSetClipbrdData,
    ORD_WinSetClipbrdOwner,
    ORD_WinSetClipbrdViewer,
    ORD_WinEnumClipbrdFmts, 
    ORD_WinEmptyClipbrd,
    ORD_WinAddAtom,
    ORD_WinFindAtom,
    ORD_WinDeleteAtom,
    ORD_WinQueryAtomUsage,
    ORD_WinQueryAtomName,
    ORD_WinQueryAtomLength,
    ORD_WinQuerySystemAtomTable,
    ORD_WinCreateAtomTable,
    ORD_WinDestroyAtomTable,
    ORD_WinOpenWindowDC,
    ORD_DevOpenDC,
    ORD_DevQueryCaps,
    ORD_DevCloseDC,
    ORD_WinMessageBox,
    ORD_WinMessageBox2,
    ORD_WinQuerySysValue,
    ORD_WinSetSysValue,
    ORD_WinAlarm,
    ORD_WinFlashWindow,
    ORD_WinLoadPointer,
    ORD_WinQuerySysPointer,
    ORD_DosReplaceModule,
    ORD_DosPerfSysCall,
    ORD_RexxRegisterSubcomExe,
    ORD_NENTRIES
};

/* RET: return type, AT: argument signature in (), ARGS: should be in () */
#define CallORD(ret,o,at,args)	(((ret (*)at) loadByOrdinal(o, 1))args)
#define DeclFuncByORD(ret,name,o,at,args)	\
  ret name at { return CallORD(ret,o,at,args); }
#define DeclVoidFuncByORD(name,o,at,args)	\
  void name at { CallORD(void,o,at,args); }

/* This function returns error code on error, and saves the error info in $^E and Perl_rc */
#define DeclOSFuncByORD_native(ret,name,o,at,args)	\
  ret name at { unsigned long rc; return CheckOSError(CallORD(ret,o,at,args)); }

/* These functions return false on error, and save the error info in $^E and Perl_rc */
#define DeclOSFuncByORD(ret,name,o,at,args)	\
  ret name at { unsigned long rc; return !CheckOSError(CallORD(ret,o,at,args)); }
#define DeclWinFuncByORD(ret,name,o,at,args)	\
  ret name at { return SaveWinError(CallORD(ret,o,at,args)); }

#define AssignFuncPByORD(p,o)	(*(Perl_PFN*)&(p) = (loadByOrdinal(o, 1)))

/* This flavor caches the procedure pointer (named as p__Win#name) locally */
#define DeclWinFuncByORD_CACHE(ret,name,o,at,args)	\
        DeclWinFuncByORD_CACHE_r(ret,name,o,at,args,0,1)

/* This flavor may reset the last error before the call (if ret=0 may be OK) */
#define DeclWinFuncByORD_CACHE_resetError(ret,name,o,at,args)	\
        DeclWinFuncByORD_CACHE_r(ret,name,o,at,args,1,1)

/* Two flavors below do the same as above, but do not auto-croak */
/* This flavor caches the procedure pointer (named as p__Win#name) locally */
#define DeclWinFuncByORD_CACHE_survive(ret,name,o,at,args)	\
        DeclWinFuncByORD_CACHE_r(ret,name,o,at,args,0,0)

/* This flavor may reset the last error before the call (if ret=0 may be OK) */
#define DeclWinFuncByORD_CACHE_resetError_survive(ret,name,o,at,args)	\
        DeclWinFuncByORD_CACHE_r(ret,name,o,at,args,1,0)

#define DeclWinFuncByORD_CACHE_r(ret,name,o,at,args,r,die)	\
  static ret (*CAT2(p__Win,name)) at;				\
  static ret name at {						\
        if (!CAT2(p__Win,name))					\
            AssignFuncPByORD(CAT2(p__Win,name), o);		\
        if (r) ResetWinError();					\
        return SaveCroakWinError(CAT2(p__Win,name) args, die, "[Win]", STRINGIFY(name)); }

/* These flavors additionally assume ORD is name with prepended ORD_Win  */
#define DeclWinFunc_CACHE(ret,name,at,args)	\
        DeclWinFuncByORD_CACHE(ret,name,CAT2(ORD_Win,name),at,args)
#define DeclWinFunc_CACHE_resetError(ret,name,at,args)	\
        DeclWinFuncByORD_CACHE_resetError(ret,name,CAT2(ORD_Win,name),at,args)
#define DeclWinFunc_CACHE_survive(ret,name,at,args)	\
        DeclWinFuncByORD_CACHE_survive(ret,name,CAT2(ORD_Win,name),at,args)
#define DeclWinFunc_CACHE_resetError_survive(ret,name,at,args)	\
        DeclWinFuncByORD_CACHE_resetError_survive(ret,name,CAT2(ORD_Win,name),at,args)

void ResetWinError(void);
void CroakWinError(int die, char *name);

enum Perlos2_handler { 
  Perlos2_handler_mangle = 1,
  Perlos2_handler_perl_sh,
  Perlos2_handler_perllib_from,
  Perlos2_handler_perllib_to,
};
enum dir_subst_e {
    dir_subst_fatal = 1,
    dir_subst_pathlike = 2
};

extern int Perl_OS2_handler_install(void *handler, enum Perlos2_handler how);
extern char *dir_subst(char *s, unsigned int l, char *b, unsigned int bl, enum dir_subst_e flags, char *msg);
extern unsigned long fill_extLibpath(int type, char *pre, char *post, int replace, char *msg);

#define PERLLIB_MANGLE(s, n) perllib_mangle((s), (n))
char *perllib_mangle(char *, unsigned int);

#define fork	fork_with_resources

#ifdef EINTR				/* x2p do not include perl.h!!! */
static __inline__ int
my_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  if (nfds == 0 && timeout && (_emx_env & 0x200)) {
    if (async_mssleep(1000 * timeout->tv_sec + (timeout->tv_usec + 500)/1000, 500))
      return 0;
    errno = EINTR;
    return -1;
  }
  return select(nfds, readfds, writefds, exceptfds, timeout);
}

#define select		my_select
#endif


typedef int (*Perl_PFN)();
Perl_PFN loadByOrdinal(enum entries_ordinals ord, int fail);
extern const Perl_PFN * const pExtFCN;
char *os2error(int rc);
int os2_stat(const char *name, struct stat *st);
int fork_with_resources();
int setpriority(int which, int pid, int val);
int getpriority(int which /* ignored */, int pid);

void croak_with_os2error(char *s) __attribute__((noreturn));

/* void return value */
#define os2cp_croak(rc,msg)	(CheckOSError(rc) && (croak_with_os2error(msg),0))

/* propagates rc */
#define os2win_croak(rc,msg)						\
        SaveCroakWinError((expr), 1 /* die */, /* no prefix */, (msg))

/* propagates rc; use with functions which may return 0 on success */
#define os2win_croak_0OK(rc,msg)					\
        SaveCroakWinError((ResetWinError, (expr)),			\
                          1 /* die */, /* no prefix */, (msg))

#ifdef PERL_CORE
int os2_do_spawn(pTHX_ char *cmd);
int os2_do_aspawn(pTHX_ SV *really, SV **vmark, SV **vsp);
#endif

#ifndef LOG_DAEMON

/* Replacement for syslog.h */
#  define LOG_EMERG     0       /* system is unusable */
#  define LOG_ALERT     1       /* action must be taken immediately */
#  define LOG_CRIT      2       /* critical conditions */
#  define LOG_ERR       3       /* error conditions */
#  define LOG_WARNING   4       /* warning conditions */
#  define LOG_NOTICE    5       /* normal but significant condition */
#  define LOG_INFO      6       /* informational */
#  define LOG_DEBUG     7       /* debug-level messages */

#  define LOG_PRIMASK   0x007   /* mask to extract priority part (internal) */
                                /* extract priority */
#  define LOG_PRI(p)    ((p) & LOG_PRIMASK)
#  define LOG_MAKEPRI(fac, pri) (((fac) << 3) | (pri))

/* facility codes */
#  define LOG_KERN      (0<<3)  /* kernel messages */
#  define LOG_USER      (1<<3)  /* random user-level messages */
#  define LOG_MAIL      (2<<3)  /* mail system */
#  define LOG_DAEMON    (3<<3)  /* system daemons */
#  define LOG_AUTH      (4<<3)  /* security/authorization messages */
#  define LOG_SYSLOG    (5<<3)  /* messages generated internally by syslogd */
#  define LOG_LPR       (6<<3)  /* line printer subsystem */
#  define LOG_NEWS      (7<<3)  /* network news subsystem */
#  define LOG_UUCP      (8<<3)  /* UUCP subsystem */
#  define LOG_CRON      (15<<3) /* clock daemon */
        /* other codes through 15 reserved for system use */
#  define LOG_LOCAL0    (16<<3) /* reserved for local use */
#  define LOG_LOCAL1    (17<<3) /* reserved for local use */
#  define LOG_LOCAL2    (18<<3) /* reserved for local use */
#  define LOG_LOCAL3    (19<<3) /* reserved for local use */
#  define LOG_LOCAL4    (20<<3) /* reserved for local use */
#  define LOG_LOCAL5    (21<<3) /* reserved for local use */
#  define LOG_LOCAL6    (22<<3) /* reserved for local use */
#  define LOG_LOCAL7    (23<<3) /* reserved for local use */

#  define LOG_NFACILITIES       24      /* current number of facilities */
#  define LOG_FACMASK   0x03f8  /* mask to extract facility part */
                                /* facility of pri */
#  define LOG_FAC(p)    (((p) & LOG_FACMASK) >> 3)

/*
 * arguments to setlogmask.
 */
#  define LOG_MASK(pri) (1 << (pri))            /* mask for one priority */
#  define       LOG_UPTO(pri)   nBIT_MASK((pri)+1)      /* all priorities through pri */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#  define LOG_PID               0x01    /* log the pid with each message */
#  define LOG_CONS      0x02    /* log on the console if errors in sending */
#  define LOG_ODELAY    0x04    /* delay open until first syslog() (default) */
#  define LOG_NDELAY    0x08    /* don't delay open */
#  define LOG_NOWAIT    0x10    /* don't wait for console forks: DEPRECATED */
#  define LOG_PERROR    0x20    /* log to stderr as well */

#endif

/* ************************************************* */
#ifndef MAKEPLINFOSEG

/* From $DDK\base32\rel\os2c\include\base\os2\16bit\infoseg.h + typedefs */

/*
 * The structure below defines the content and organization of the system
 * information segment (InfoSeg).  The actual table is statically defined in
 * SDATA.ASM.  Ring 0, read/write access is obtained by the clock device
 * driver using the DevHlp GetDOSVar function.  (GetDOSVar returns a ring 0,
 * read-only selector to all other requestors.)
 *
 * In order to prevent an errant process from destroying the infoseg, two
 * identical global infosegs are maintained.  One is in the tiled shared
 * arena and is accessible in user mode (and therefore can potentially be
 * overwritten from ring 2), and the other is in the system arena and is
 * accessible only in kernel mode.  All kernel code (except the clock driver)
 * is responsible for updating BOTH copies of the infoseg.  The copy kept
 * in the system arena is addressable as DOSGROUP:SISData, and the copy
 * in the shared arena is addressable via a system arena alias.  16:16 and
 * 0:32 pointers to the alias are stored in _Sis2.
 */

typedef struct InfoSegGDT {

/* Time (offset 0x00) */

unsigned long   SIS_BigTime;    /* Time from 1-1-1970 in seconds */
unsigned long   SIS_MsCount;    /* Freerunning milliseconds counter */
unsigned char   SIS_HrsTime;    /* Hours */
unsigned char   SIS_MinTime;    /* Minutes */
unsigned char   SIS_SecTime;    /* Seconds */
unsigned char   SIS_HunTime;    /* Hundredths of seconds */
unsigned short  SIS_TimeZone;   /* Timezone in min from GMT (Set to EST) */
unsigned short  SIS_ClkIntrvl;  /* Timer interval (units=0.0001 secs) */

/* Date (offset 0x10) */

unsigned char   SIS_DayDate;    /* Day-of-month (1-31) */
unsigned char   SIS_MonDate;    /* Month (1-12) */
unsigned short  SIS_YrsDate;    /* Year (>= 1980) */
unsigned char   SIS_DOWDate;    /* Day-of-week (1-1-80 = Tues = 3) */

/* Version (offset 0x15) */

unsigned char   SIS_VerMajor;   /* Major version number */
unsigned char   SIS_VerMinor;   /* Minor version number */
unsigned char   SIS_RevLettr;   /* Revision letter */

/* System Status (offset 0x18) */

unsigned char   SIS_CurScrnGrp; /* Fgnd screen group # */
unsigned char   SIS_MaxScrnGrp; /* Maximum number of screen groups */
unsigned char   SIS_HugeShfCnt; /* Shift count for huge segments */
unsigned char   SIS_ProtMdOnly; /* Protect-mode-only indicator */
unsigned short  SIS_FgndPID;    /* Foreground process ID */

/* Scheduler Parms (offset 0x1E) */

unsigned char   SIS_Dynamic;    /* Dynamic variation flag (1=enabled) */
unsigned char   SIS_MaxWait;    /* Maxwait (seconds) */
unsigned short  SIS_MinSlice;   /* Minimum timeslice (milliseconds) */
unsigned short  SIS_MaxSlice;   /* Maximum timeslice (milliseconds) */

/* Boot Drive (offset 0x24) */

unsigned short  SIS_BootDrv;    /* Drive from which system was booted */

/* RAS Major Event Code Table (offset 0x26) */

unsigned char   SIS_mec_table[32]; /* Table of RAS Major Event Codes (MECs) */

/* Additional Session Data (offset 0x46) */

unsigned char   SIS_MaxVioWinSG;  /* Max. no. of VIO windowable SG's */
unsigned char   SIS_MaxPresMgrSG; /* Max. no. of Presentation Manager SG's */

/* Error logging Information (offset 0x48) */

unsigned short  SIS_SysLog;     /* Error Logging Status */

/* Additional RAS Information (offset 0x4A) */

unsigned short  SIS_MMIOBase;   /* Memory mapped I/O selector */
unsigned long   SIS_MMIOAddr;   /* Memory mapped I/O address  */

/* Additional 2.0 Data (offset 0x50) */

unsigned char   SIS_MaxVDMs;      /* Max. no. of Virtual DOS machines */
unsigned char   SIS_Reserved;

unsigned char   SIS_perf_mec_table[32]; /* varga 6/5/97 Table of Performance Major Event Codes (MECS) varga*/
} GINFOSEG, *PGINFOSEG;

#define SIS_LEN         sizeof(struct InfoSegGDT)

/*
 *      InfoSeg LDT Data Segment Structure
 *
 * The structure below defines the content and organization of the system
 * information in a special per-process segment to be accessible by the
 * process through the LDT (read-only).
 *
 * As in the global infoseg, two copies of the current processes local
 * infoseg exist, one accessible in both user and kernel mode, the other
 * only in kernel mode.  Kernel code is responsible for updating BOTH copies.
 * Pointers to the local infoseg copy are stored in _Lis2.
 *
 * Note that only the currently running process has an extra copy of the
 * local infoseg.  The copy is done at context switch time.
 */

typedef struct InfoSegLDT {
unsigned short  LIS_CurProcID;  /* Current process ID */
unsigned short  LIS_ParProcID;  /* Process ID of parent */
unsigned short  LIS_CurThrdPri; /* Current thread priority */
unsigned short  LIS_CurThrdID;  /* Current thread ID */
unsigned short  LIS_CurScrnGrp; /* Screengroup */
unsigned char   LIS_ProcStatus; /* Process status bits */
unsigned char   LIS_fillbyte1;  /* filler byte */
unsigned short  LIS_Fgnd;       /* Current process is in foreground */
unsigned char   LIS_ProcType;   /* Current process type */
unsigned char   LIS_fillbyte2;  /* filler byte */

unsigned short  LIS_AX;         /* @@V1 Environment selector */
unsigned short  LIS_BX;         /* @@V1 Offset of command line start */
unsigned short  LIS_CX;         /* @@V1 Length of Data Segment */
unsigned short  LIS_DX;         /* @@V1 STACKSIZE from the .EXE file */
unsigned short  LIS_SI;         /* @@V1 HEAPSIZE  from the .EXE file */
unsigned short  LIS_DI;         /* @@V1 Module handle of the application */
unsigned short  LIS_DS;         /* @@V1 Data Segment Handle of application */

unsigned short  LIS_PackSel;    /* First tiled selector in this EXE */
unsigned short  LIS_PackShrSel; /* First selector above shared arena */
unsigned short  LIS_PackPckSel; /* First selector above packed arena */
/* #ifdef SMP */
unsigned long   LIS_pTIB;       /* Pointer to TIB */
unsigned long   LIS_pPIB;       /* Pointer to PIB */
/* #endif */
} LINFOSEG, *PLINFOSEG;

#define LIS_LEN         sizeof(struct InfoSegLDT)


/*
 *      Process Type codes
 *
 *      These are the definitions for the codes stored
 *      in the LIS_ProcType field in the local infoseg.
 */

#define         LIS_PT_FULLSCRN 0       /* Full screen app. */
#define         LIS_PT_REALMODE 1       /* Real mode process */
#define         LIS_PT_VIOWIN   2       /* VIO windowable app. */
#define         LIS_PT_PRESMGR  3       /* Presentation Manager app. */
#define         LIS_PT_DETACHED 4       /* Detached app. */


/*
 *
 *      Process Status Bit Definitions
 *
 */

#define         LIS_PS_EXITLIST 0x01    /* In exitlist handler */


/*
 *      Flags equates for the Global Info Segment
 *      SIS_SysLog  WORD in Global Info Segment
 *
 *        xxxx xxxx xxxx xxx0         Error Logging Disabled
 *        xxxx xxxx xxxx xxx1         Error Logging Enabled
 *
 *        xxxx xxxx xxxx xx0x         Error Logging not available
 *        xxxx xxxx xxxx xx1x         Error Logging available
 */

#define LF_LOGENABLE    0x0001          /* Logging enabled */
#define LF_LOGAVAILABLE 0x0002          /* Logging available */

#define MAKEPGINFOSEG(sel)  ((PGINFOSEG)MAKEP(sel, 0))
#define MAKEPLINFOSEG(sel)  ((PLINFOSEG)MAKEP(sel, 0))

#endif	/* ndef(MAKEPLINFOSEG) */

/* ************************************************************ */
#define Dos32QuerySysState DosQuerySysState
#define QuerySysState(flags, pid, buf, bufsz) \
        Dos32QuerySysState(flags, 0,  pid, 0, buf, bufsz)

#define QSS_PROCESS	1
#define QSS_MODULE	4
#define QSS_SEMAPHORES	2
#define QSS_FILE	8		/* Buggy until fixpack18 */
#define QSS_SHARED	16

#ifdef _OS2_H

APIRET APIENTRY Dos32QuerySysState(ULONG func,ULONG arg1,ULONG pid,
                        ULONG _res_,PVOID buf,ULONG bufsz);
typedef struct {
        ULONG	threadcnt;
        ULONG	proccnt;
        ULONG	modulecnt;
} QGLOBAL, *PQGLOBAL;

typedef struct {
        ULONG	rectype;
        USHORT	threadid;
        USHORT	slotid;
        ULONG	sleepid;
        ULONG	priority;
        ULONG	systime;
        ULONG	usertime;
        UCHAR	state;
        UCHAR	_reserved1_;	/* padding to ULONG */
        USHORT	_reserved2_;	/* padding to ULONG */
} QTHREAD, *PQTHREAD;

typedef struct {
        USHORT	sfn;
        USHORT	refcnt;
        USHORT	flags1;
        USHORT	flags2;
        USHORT	accmode1;
        USHORT	accmode2;
        ULONG	filesize;
        USHORT  volhnd;
        USHORT	attrib;
        USHORT	_reserved_;
} QFDS, *PQFDS;

typedef struct qfile {
        ULONG		rectype;
        struct qfile	*next;
        ULONG		opencnt;
        PQFDS		filedata;
        char		name[1];
} QFILE, *PQFILE;

typedef struct {
        ULONG	rectype;
        PQTHREAD threads;
        USHORT	pid;
        USHORT	ppid;
        ULONG	type;
        ULONG	state;
        ULONG	sessid;
        USHORT	hndmod;
        USHORT	threadcnt;
        ULONG	privsem32cnt;
        ULONG	_reserved2_;
        USHORT	sem16cnt;
        USHORT	dllcnt;
        USHORT	shrmemcnt;
        USHORT	fdscnt;
        PUSHORT	sem16s;
        PUSHORT	dlls;
        PUSHORT	shrmems;
        PUSHORT	fds;
} QPROCESS, *PQPROCESS;

typedef struct sema {
        struct sema *next;
        USHORT	refcnt;
        UCHAR	sysflags;
        UCHAR	sysproccnt;
        ULONG	_reserved1_;
        USHORT	index;
        CHAR	name[1];
} QSEMA, *PQSEMA;

typedef struct {
        ULONG	rectype;
        ULONG	_reserved1_;
        USHORT	_reserved2_;
        USHORT	syssemidx;
        ULONG	index;
        QSEMA	sema;
} QSEMSTRUC, *PQSEMSTRUC;

typedef struct {
        USHORT	pid;
        USHORT	opencnt;
} QSEMOWNER32, *PQSEMOWNER32;

typedef struct {
        PQSEMOWNER32	own;
        PCHAR		name;
        PVOID		semrecs; /* array of associated sema's */
        USHORT		flags;
        USHORT		semreccnt;
        USHORT		waitcnt;
        USHORT		_reserved_;	/* padding to ULONG */
} QSEMSMUX32, *PQSEMSMUX32;

typedef struct {
        PQSEMOWNER32	own;
        PCHAR		name;
        PQSEMSMUX32	mux;
        USHORT		flags;
        USHORT		postcnt;
} QSEMEV32, *PQSEMEV32;

typedef struct {
        PQSEMOWNER32	own;
        PCHAR		name;
        PQSEMSMUX32	mux;
        USHORT		flags;
        USHORT		refcnt;
        USHORT		thrdnum;
        USHORT		_reserved_;	/* padding to ULONG */
} QSEMMUX32, *PQSEMMUX32;

typedef struct semstr32 {
        struct semstr *next;
        QSEMEV32 evsem;
        QSEMMUX32  muxsem;
        QSEMSMUX32 smuxsem;
} QSEMSTRUC32, *PQSEMSTRUC32;

typedef struct shrmem {
        struct shrmem *next;
        USHORT	hndshr;
        USHORT	selshr;
        USHORT	refcnt;
        CHAR	name[1];
} QSHRMEM, *PQSHRMEM;

typedef struct module {
        struct module *next;
        USHORT	hndmod;
        USHORT	type;
        ULONG	refcnt;
        ULONG	segcnt;
        PVOID	_reserved_;
        PCHAR	name;
        USHORT	modref[1];
} QMODULE, *PQMODULE;

typedef struct {
        PQGLOBAL	gbldata;
        PQPROCESS	procdata;
        PQSEMSTRUC	semadata;
        PQSEMSTRUC32	sem32data;
        PQSHRMEM	shrmemdata;
        PQMODULE	moddata;
        PVOID		_reserved2_;
        PQFILE		filedata;
} QTOPLEVEL, *PQTOPLEVEL;
/* ************************************************************ */

PQTOPLEVEL get_sysinfo(ULONG pid, ULONG flags);

#endif /* _OS2_H */
