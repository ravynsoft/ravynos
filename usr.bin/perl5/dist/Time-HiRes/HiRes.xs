/*
 *
 * Copyright (c) 1996-2002 Douglas E. Wegscheid.  All rights reserved.
 *
 * Copyright (c) 2002-2010 Jarkko Hietaniemi.
 * All rights reserved.
 *
 * Copyright (C) 2011, 2012, 2013 Andrew Main (Zefram) <zefram@fysh.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the same terms as Perl itself.
 */

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "reentr.h"
#if !defined(IS_SAFE_PATHNAME) && defined(TIME_HIRES_UTIME) && defined(HAS_UTIMENSAT)
#define NEED_ck_warner
#endif
#include "ppport.h"
#if defined(__CYGWIN__) && defined(HAS_W32API_WINDOWS_H)
#  include <w32api/windows.h>
#  define CYGWIN_WITH_W32API
#endif
#ifdef WIN32
#  include <time.h>
#else
#  include <sys/time.h>
#endif
#ifdef HAS_SELECT
#  ifdef I_SYS_SELECT
#    include <sys/select.h>
#  endif
#endif
#if defined(TIME_HIRES_CLOCK_GETTIME_SYSCALL) || defined(TIME_HIRES_CLOCK_GETRES_SYSCALL)
#  include <syscall.h>
#endif

#ifndef GCC_DIAG_IGNORE
#  define GCC_DIAG_IGNORE(x)
#  define GCC_DIAG_RESTORE
#endif
#ifndef GCC_DIAG_IGNORE_STMT
#  define GCC_DIAG_IGNORE_STMT(x) GCC_DIAG_IGNORE(x) NOOP
#  define GCC_DIAG_RESTORE_STMT GCC_DIAG_RESTORE NOOP
#endif

#if PERL_VERSION_GE(5,7,3) && !PERL_VERSION_GE(5,10,1)
#  undef SAVEOP
#  define SAVEOP() SAVEVPTR(PL_op)
#endif

#define IV_1E6 1000000
#define IV_1E7 10000000
#define IV_1E9 1000000000

#define NV_1E6 1000000.0
#define NV_1E7 10000000.0
#define NV_1E9 1000000000.0

#ifndef PerlProc_pause
#  define PerlProc_pause() Pause()
#endif

#ifdef HAS_PAUSE
#  define Pause   pause
#else
#  undef Pause /* In case perl.h did it already. */
#  define Pause() sleep(~0) /* Zzz for a long time. */
#endif

/* Though the cpp define ITIMER_VIRTUAL is available the functionality
 * is not supported in Cygwin as of August 2004, ditto for Win32.
 * Neither are ITIMER_PROF or ITIMER_REALPROF implemented.  --jhi
 */
#if defined(__CYGWIN__) || defined(WIN32)
#  undef ITIMER_VIRTUAL
#  undef ITIMER_PROF
#  undef ITIMER_REALPROF
#endif

#ifndef TIME_HIRES_CLOCKID_T
typedef int clockid_t;
#endif

#if defined(TIME_HIRES_CLOCK_GETTIME) && defined(_STRUCT_ITIMERSPEC)

/* HP-UX has CLOCK_XXX values but as enums, not as defines.
 * The only way to detect these would be to test compile for each. */
#  ifdef __hpux
/* However, it seems that at least in HP-UX 11.31 ia64 there *are*
 * defines for these, so let's try detecting them. */
#    ifndef CLOCK_REALTIME
#      define CLOCK_REALTIME CLOCK_REALTIME
#      define CLOCK_VIRTUAL  CLOCK_VIRTUAL
#      define CLOCK_PROFILE  CLOCK_PROFILE
#    endif
#  endif /* # ifdef __hpux */

#endif /* #if defined(TIME_HIRES_CLOCK_GETTIME) && defined(_STRUCT_ITIMERSPEC) */

#if defined(WIN32) || defined(CYGWIN_WITH_W32API)

#  ifndef HAS_GETTIMEOFDAY
#    define HAS_GETTIMEOFDAY
#  endif

/* shows up in winsock.h?
struct timeval {
    long tv_sec;
    long tv_usec;
}
*/

typedef union {
    unsigned __int64    ft_i64;
    FILETIME            ft_val;
} FT_t;

#  define MY_CXT_KEY "Time::HiRes_" XS_VERSION

typedef struct {
    unsigned long run_count;
    unsigned __int64 base_ticks;
    unsigned __int64 tick_frequency;
    FT_t base_systime_as_filetime;
    unsigned __int64 reset_time;
} my_cxt_t;

/* Visual C++ 2013 and older don't have the timespec structure.
 * Neither do mingw.org compilers with MinGW runtimes older than 3.22. */
#  if((defined(_MSC_VER) && _MSC_VER < 1900) || \
      (defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR) && \
      defined(__MINGW32_MAJOR_VERSION) && (__MINGW32_MAJOR_VERSION < 3 || \
      (__MINGW32_MAJOR_VERSION == 3 && __MINGW32_MINOR_VERSION < 22))))
struct timespec {
    time_t tv_sec;
    long   tv_nsec;
};
#  endif

START_MY_CXT

/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#  ifdef __GNUC__
#    define Const64(x) x##LL
#  else
#    define Const64(x) x##i64
#  endif
#  define EPOCH_BIAS  Const64(116444736000000000)

#  ifdef Const64
#    ifdef __GNUC__
#      define IV_1E6LL  1000000LL /* Needed because of Const64() ##-appends LL (or i64). */
#      define IV_1E7LL  10000000LL
#      define IV_1E9LL  1000000000LL
#    else
#      define IV_1E6i64 1000000i64
#      define IV_1E7i64 10000000i64
#      define IV_1E9i64 1000000000i64
#    endif
#  endif

/* NOTE: This does not compute the timezone info (doing so can be expensive,
 * and appears to be unsupported even by glibc) */

/* dMY_CXT needs a Perl context and we don't want to call PERL_GET_CONTEXT
   for performance reasons */

#  undef gettimeofday
#  define gettimeofday(tp, not_used) _gettimeofday(aTHX_ tp, not_used)

#  undef GetSystemTimePreciseAsFileTime
#  define GetSystemTimePreciseAsFileTime(out) _GetSystemTimePreciseAsFileTime(aTHX_ out)

#  undef clock_gettime
#  define clock_gettime(clock_id, tp) _clock_gettime(aTHX_ clock_id, tp)

#  undef clock_getres
#  define clock_getres(clock_id, tp) _clock_getres(clock_id, tp)

#  ifndef CLOCK_REALTIME
#    define CLOCK_REALTIME  1
#    define CLOCK_MONOTONIC 2
#  endif

/* If the performance counter delta drifts more than 0.5 seconds from the
 * system time then we recalibrate to the system time.  This means we may
 * move *backwards* in time! */
#  define MAX_PERF_COUNTER_SKEW Const64(5000000) /* 0.5 seconds */

/* Reset reading from the performance counter every five minutes.
 * Many PC clocks just seem to be so bad. */
#  define MAX_PERF_COUNTER_TICKS Const64(300000000) /* 300 seconds */

/*
 * Windows 8 introduced GetSystemTimePreciseAsFileTime(), but currently we have
 * to support older systems, so for now we provide our own implementation.
 * In the future we will switch to the real deal.
 */
static void
_GetSystemTimePreciseAsFileTime(pTHX_ FILETIME *out)
{
    dMY_CXT;
    FT_t ft;

    if (MY_CXT.run_count++ == 0 ||
        MY_CXT.base_systime_as_filetime.ft_i64 > MY_CXT.reset_time) {

        QueryPerformanceFrequency((LARGE_INTEGER*)&MY_CXT.tick_frequency);
        QueryPerformanceCounter((LARGE_INTEGER*)&MY_CXT.base_ticks);
        GetSystemTimeAsFileTime(&MY_CXT.base_systime_as_filetime.ft_val);
        ft.ft_i64 = MY_CXT.base_systime_as_filetime.ft_i64;
        MY_CXT.reset_time = ft.ft_i64 + MAX_PERF_COUNTER_TICKS;
    }
    else {
        __int64 diff;
        unsigned __int64 ticks;
        QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
        ticks -= MY_CXT.base_ticks;
        ft.ft_i64 = MY_CXT.base_systime_as_filetime.ft_i64
                    + Const64(IV_1E7) * (ticks / MY_CXT.tick_frequency)
                    +(Const64(IV_1E7) * (ticks % MY_CXT.tick_frequency)) / MY_CXT.tick_frequency;
        diff = ft.ft_i64 - MY_CXT.base_systime_as_filetime.ft_i64;
        if (diff < -MAX_PERF_COUNTER_SKEW || diff > MAX_PERF_COUNTER_SKEW) {
            MY_CXT.base_ticks += ticks;
            GetSystemTimeAsFileTime(&MY_CXT.base_systime_as_filetime.ft_val);
            ft.ft_i64 = MY_CXT.base_systime_as_filetime.ft_i64;
        }
    }

    *out = ft.ft_val;

    return;
}

static int
_gettimeofday(pTHX_ struct timeval *tp, void *not_used)
{
    FT_t ft;

    PERL_UNUSED_ARG(not_used);

    GetSystemTimePreciseAsFileTime(&ft.ft_val);

    /* seconds since epoch */
    tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / Const64(IV_1E7));

    /* microseconds remaining */
    tp->tv_usec = (long)((ft.ft_i64 / Const64(10)) % Const64(IV_1E6));

    return 0;
}

static int
_clock_gettime(pTHX_ clockid_t clock_id, struct timespec *tp)
{
    switch (clock_id) {
    case CLOCK_REALTIME: {
        FT_t ft;

        GetSystemTimePreciseAsFileTime(&ft.ft_val);
        tp->tv_sec = (time_t)((ft.ft_i64 - EPOCH_BIAS) / IV_1E7);
        tp->tv_nsec = (long)((ft.ft_i64 % IV_1E7) * 100);
        break;
    }
    case CLOCK_MONOTONIC: {
        unsigned __int64 freq, ticks;

        QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
        QueryPerformanceCounter((LARGE_INTEGER*)&ticks);

        tp->tv_sec = (time_t)(ticks / freq);
        tp->tv_nsec = (long)((IV_1E9 * (ticks % freq)) / freq);
        break;
    }
    default:
        errno = EINVAL;
        return 1;
    }

    return 0;
}

static int
_clock_getres(clockid_t clock_id, struct timespec *tp)
{
    unsigned __int64 freq, qpc_res_ns;

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    qpc_res_ns = IV_1E9 > freq ? IV_1E9 / freq : 1;

    switch (clock_id) {
    case CLOCK_REALTIME:
        tp->tv_sec = 0;
        /* the resolution can't be smaller than 100ns because our implementation
         * of CLOCK_REALTIME is using FILETIME internally */
        tp->tv_nsec = (long)(qpc_res_ns > 100 ? qpc_res_ns : 100);
        break;

    case CLOCK_MONOTONIC:
        tp->tv_sec = 0;
        tp->tv_nsec = (long)qpc_res_ns;
        break;

    default:
        errno = EINVAL;
        return 1;
    }

    return 0;
}

#endif /* #if defined(WIN32) || defined(CYGWIN_WITH_W32API) */

 /* Do not use H A S _ N A N O S L E E P
  * so that Perl Configure doesn't scan for it (and pull in -lrt and
  * the like which are not usually good ideas for the default Perl).
  * (We are part of the core perl now.)
  * The TIME_HIRES_NANOSLEEP is set by Makefile.PL. */
#if !defined(HAS_USLEEP) && defined(TIME_HIRES_NANOSLEEP)
#  define HAS_USLEEP
#  define usleep hrt_usleep  /* could conflict with ncurses for static build */

static void
hrt_usleep(unsigned long usec) /* This is used to emulate usleep. */
{
    struct timespec res;
    res.tv_sec = usec / IV_1E6;
    res.tv_nsec = ( usec - res.tv_sec * IV_1E6 ) * 1000;
    nanosleep(&res, NULL);
}

#endif /* #if !defined(HAS_USLEEP) && defined(TIME_HIRES_NANOSLEEP) */

#if !defined(HAS_USLEEP) && defined(HAS_SELECT)
#  ifndef SELECT_IS_BROKEN
#    define HAS_USLEEP
#    define usleep hrt_usleep  /* could conflict with ncurses for static build */

static void
hrt_usleep(unsigned long usec)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = usec;
    select(0, (Select_fd_set_t)NULL, (Select_fd_set_t)NULL,
        (Select_fd_set_t)NULL, &tv);
}
#  endif
#endif /* #if !defined(HAS_USLEEP) && defined(HAS_SELECT) */

#if !defined(HAS_USLEEP) && defined(WIN32)
#  define HAS_USLEEP
#  define usleep hrt_usleep  /* could conflict with ncurses for static build */

static void
hrt_usleep(unsigned long usec)
{
    long msec;
    msec = usec / 1000;
    Sleep (msec);
}
#endif /* #if !defined(HAS_USLEEP) && defined(WIN32) */

#if !defined(HAS_USLEEP) && defined(HAS_POLL)
#  define HAS_USLEEP
#  define usleep hrt_usleep  /* could conflict with ncurses for static build */

static void
hrt_usleep(unsigned long usec)
{
    int msec = usec / 1000;
    poll(0, 0, msec);
}

#endif /* #if !defined(HAS_USLEEP) && defined(HAS_POLL) */

#if defined(HAS_SETITIMER) && defined(ITIMER_REAL)

static int
hrt_ualarm_itimero(struct itimerval *oitv, int usec, int uinterval)
{
    struct itimerval itv;
    itv.it_value.tv_sec = usec / IV_1E6;
    itv.it_value.tv_usec = usec % IV_1E6;
    itv.it_interval.tv_sec = uinterval / IV_1E6;
    itv.it_interval.tv_usec = uinterval % IV_1E6;
    return setitimer(ITIMER_REAL, &itv, oitv);
}

#endif /* #if !defined(HAS_UALARM) && defined(HAS_SETITIMER) */

#if !defined(HAS_UALARM) && defined(HAS_SETITIMER)
#  define HAS_UALARM
#  define ualarm hrt_ualarm_itimer  /* could conflict with ncurses for static build */
#endif

#if !defined(HAS_UALARM) && defined(VMS)
#  define HAS_UALARM
#  define ualarm vms_ualarm

#  include <lib$routines.h>
#  include <ssdef.h>
#  include <starlet.h>
#  include <descrip.h>
#  include <signal.h>
#  include <jpidef.h>
#  include <psldef.h>

#  define VMSERR(s)   (!((s)&1))

static void
us_to_VMS(useconds_t mseconds, unsigned long v[])
{
    int iss;
    unsigned long qq[2];

    qq[0] = mseconds;
    qq[1] = 0;
    v[0] = v[1] = 0;

    iss = lib$addx(qq,qq,qq);
    if (VMSERR(iss)) lib$signal(iss);
    iss = lib$subx(v,qq,v);
    if (VMSERR(iss)) lib$signal(iss);
    iss = lib$addx(qq,qq,qq);
    if (VMSERR(iss)) lib$signal(iss);
    iss = lib$subx(v,qq,v);
    if (VMSERR(iss)) lib$signal(iss);
    iss = lib$subx(v,qq,v);
    if (VMSERR(iss)) lib$signal(iss);
}

static int
VMS_to_us(unsigned long v[])
{
    int iss;
    unsigned long div=10,quot, rem;

    iss = lib$ediv(&div,v,&quot,&rem);
    if (VMSERR(iss)) lib$signal(iss);

    return quot;
}

typedef unsigned short word;
typedef struct _ualarm {
    int function;
    int repeat;
    unsigned long delay[2];
    unsigned long interval[2];
    unsigned long remain[2];
} Alarm;


static int alarm_ef;
static Alarm *a0, alarm_base;
#  define UAL_NULL   0
#  define UAL_SET    1
#  define UAL_CLEAR  2
#  define UAL_ACTIVE 4
static void ualarm_AST(Alarm *a);

static int
vms_ualarm(int mseconds, int interval)
{
    Alarm *a, abase;
    struct item_list3 {
        word length;
        word code;
        void *bufaddr;
        void *retlenaddr;
    } ;
    static struct item_list3 itmlst[2];
    static int first = 1;
    unsigned long asten;
    int iss, enabled;

    if (first) {
        first = 0;
        itmlst[0].code       = JPI$_ASTEN;
        itmlst[0].length     = sizeof(asten);
        itmlst[0].retlenaddr = NULL;
        itmlst[1].code       = 0;
        itmlst[1].length     = 0;
        itmlst[1].bufaddr    = NULL;
        itmlst[1].retlenaddr = NULL;

        iss = lib$get_ef(&alarm_ef);
        if (VMSERR(iss)) lib$signal(iss);

        a0 = &alarm_base;
        a0->function = UAL_NULL;
    }
    itmlst[0].bufaddr    = &asten;

    iss = sys$getjpiw(0,0,0,itmlst,0,0,0);
    if (VMSERR(iss)) lib$signal(iss);
    if (!(asten&0x08)) return -1;

    a = &abase;
    if (mseconds) {
        a->function = UAL_SET;
    } else {
        a->function = UAL_CLEAR;
    }

    us_to_VMS(mseconds, a->delay);
    if (interval) {
        us_to_VMS(interval, a->interval);
        a->repeat = 1;
    } else
        a->repeat = 0;

    iss = sys$clref(alarm_ef);
    if (VMSERR(iss)) lib$signal(iss);

    iss = sys$dclast(ualarm_AST,a,0);
    if (VMSERR(iss)) lib$signal(iss);

    iss = sys$waitfr(alarm_ef);
    if (VMSERR(iss)) lib$signal(iss);

    if (a->function == UAL_ACTIVE)
        return VMS_to_us(a->remain);
    else
        return 0;
}



static void
ualarm_AST(Alarm *a)
{
    int iss;
    unsigned long now[2];

    iss = sys$gettim(now);
    if (VMSERR(iss)) lib$signal(iss);

    if (a->function == UAL_SET || a->function == UAL_CLEAR) {
        if (a0->function == UAL_ACTIVE) {
            iss = sys$cantim(a0,PSL$C_USER);
            if (VMSERR(iss)) lib$signal(iss);

            iss = lib$subx(a0->remain, now, a->remain);
            if (VMSERR(iss)) lib$signal(iss);

            if (a->remain[1] & 0x80000000)
                a->remain[0] = a->remain[1] = 0;
        }

        if (a->function == UAL_SET) {
            a->function = a0->function;
            a0->function = UAL_ACTIVE;
            a0->repeat = a->repeat;
            if (a0->repeat) {
                a0->interval[0] = a->interval[0];
                a0->interval[1] = a->interval[1];
            }
            a0->delay[0] = a->delay[0];
            a0->delay[1] = a->delay[1];

            iss = lib$subx(now, a0->delay, a0->remain);
            if (VMSERR(iss)) lib$signal(iss);

            iss = sys$setimr(0,a0->delay,ualarm_AST,a0);
            if (VMSERR(iss)) lib$signal(iss);
        } else {
            a->function = a0->function;
            a0->function = UAL_NULL;
        }
        iss = sys$setef(alarm_ef);
        if (VMSERR(iss)) lib$signal(iss);
    } else if (a->function == UAL_ACTIVE) {
        if (a->repeat) {
            iss = lib$subx(now, a->interval, a->remain);
            if (VMSERR(iss)) lib$signal(iss);

            iss = sys$setimr(0,a->interval,ualarm_AST,a);
            if (VMSERR(iss)) lib$signal(iss);
        } else {
            a->function = UAL_NULL;
        }
        iss = sys$wake(0,0);
        if (VMSERR(iss)) lib$signal(iss);
        lib$signal(SS$_ASTFLT);
    } else {
        lib$signal(SS$_BADPARAM);
    }
}

#endif /* #if !defined(HAS_UALARM) && defined(VMS) */

#ifdef HAS_GETTIMEOFDAY

static int
myU2time(pTHX_ UV *ret)
{
    struct timeval Tp;
    int status;
    status = gettimeofday (&Tp, NULL);
    ret[0] = Tp.tv_sec;
    ret[1] = Tp.tv_usec;
    return status;
}

static NV
myNVtime()
{
#  ifdef WIN32
    dTHX;
#  endif
    struct timeval Tp;
    int status;
    status = gettimeofday (&Tp, NULL);
    return status == 0 ? Tp.tv_sec + (Tp.tv_usec / NV_1E6) : -1.0;
}

#endif /* #ifdef HAS_GETTIMEOFDAY */

static void
hrstatns(UV *atime_nsec, UV *mtime_nsec, UV *ctime_nsec)
{
    dTHX;
#if TIME_HIRES_STAT == 1
    *atime_nsec = PL_statcache.st_atimespec.tv_nsec;
    *mtime_nsec = PL_statcache.st_mtimespec.tv_nsec;
    *ctime_nsec = PL_statcache.st_ctimespec.tv_nsec;
#elif TIME_HIRES_STAT == 2
    *atime_nsec = PL_statcache.st_atimensec;
    *mtime_nsec = PL_statcache.st_mtimensec;
    *ctime_nsec = PL_statcache.st_ctimensec;
#elif TIME_HIRES_STAT == 3
    *atime_nsec = PL_statcache.st_atime_n;
    *mtime_nsec = PL_statcache.st_mtime_n;
    *ctime_nsec = PL_statcache.st_ctime_n;
#elif TIME_HIRES_STAT == 4
    *atime_nsec = PL_statcache.st_atim.tv_nsec;
    *mtime_nsec = PL_statcache.st_mtim.tv_nsec;
    *ctime_nsec = PL_statcache.st_ctim.tv_nsec;
#elif TIME_HIRES_STAT == 5
    *atime_nsec = PL_statcache.st_uatime * 1000;
    *mtime_nsec = PL_statcache.st_umtime * 1000;
    *ctime_nsec = PL_statcache.st_uctime * 1000;
#else /* !TIME_HIRES_STAT */
    *atime_nsec = 0;
    *mtime_nsec = 0;
    *ctime_nsec = 0;
#endif /* !TIME_HIRES_STAT */
}

/* Until Apple implements clock_gettime()
 * (ditto clock_getres() and clock_nanosleep())
 * we will emulate them using the Mach kernel interfaces. */
#if defined(PERL_DARWIN) && \
  (defined(TIME_HIRES_CLOCK_GETTIME_EMULATION)   || \
   defined(TIME_HIRES_CLOCK_GETRES_EMULATION)    || \
   defined(TIME_HIRES_CLOCK_NANOSLEEP_EMULATION))

#  ifndef CLOCK_REALTIME
#    define CLOCK_REALTIME  0x01
#    define CLOCK_MONOTONIC 0x02
#  endif

#  ifndef TIMER_ABSTIME
#    define TIMER_ABSTIME   0x01
#  endif

#  ifdef USE_ITHREADS
#    define PERL_DARWIN_MUTEX
#  endif

#  ifdef PERL_DARWIN_MUTEX
STATIC perl_mutex darwin_time_mutex;
#  endif

#  include <mach/mach_time.h>

static uint64_t absolute_time_init;
static mach_timebase_info_data_t timebase_info;
static struct timespec timespec_init;

static int darwin_time_init() {
    struct timeval tv;
    int success = 1;
#  ifdef PERL_DARWIN_MUTEX
    MUTEX_LOCK(&darwin_time_mutex);
#  endif
    if (absolute_time_init == 0) {
        /* mach_absolute_time() cannot fail */
        absolute_time_init = mach_absolute_time();
        success = mach_timebase_info(&timebase_info) == KERN_SUCCESS;
        if (success) {
            success = gettimeofday(&tv, NULL) == 0;
            if (success) {
                timespec_init.tv_sec  = tv.tv_sec;
                timespec_init.tv_nsec = tv.tv_usec * 1000;
            }
        }
    }
#  ifdef PERL_DARWIN_MUTEX
    MUTEX_UNLOCK(&darwin_time_mutex);
#  endif
    return success;
}

#  ifdef TIME_HIRES_CLOCK_GETTIME_EMULATION
static int th_clock_gettime(clockid_t clock_id, struct timespec *ts) {
    if (darwin_time_init() && timebase_info.denom) {
        switch (clock_id) {
        case CLOCK_REALTIME:
            {
                uint64_t nanos =
                    ((mach_absolute_time() - absolute_time_init) *
                    (uint64_t)timebase_info.numer) / (uint64_t)timebase_info.denom;
                ts->tv_sec  = timespec_init.tv_sec  + nanos / IV_1E9;
                ts->tv_nsec = timespec_init.tv_nsec + nanos % IV_1E9;
                return 0;
            }

        case CLOCK_MONOTONIC:
            {
                uint64_t nanos =
                    (mach_absolute_time() *
                    (uint64_t)timebase_info.numer) / (uint64_t)timebase_info.denom;
                ts->tv_sec  = nanos / IV_1E9;
                ts->tv_nsec = nanos - ts->tv_sec * IV_1E9;
                return 0;
            }

        default:
            break;
        }
    }

    SETERRNO(EINVAL, LIB_INVARG);
    return -1;
}

#    define clock_gettime(clock_id, ts) th_clock_gettime((clock_id), (ts))

#  endif /* TIME_HIRES_CLOCK_GETTIME_EMULATION */

#  ifdef TIME_HIRES_CLOCK_GETRES_EMULATION
static int th_clock_getres(clockid_t clock_id, struct timespec *ts) {
    if (darwin_time_init() && timebase_info.denom) {
        switch (clock_id) {
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC:
            ts->tv_sec  = 0;
            /* In newer kernels both the numer and denom are one,
             * resulting in conversion factor of one, which is of
             * course unrealistic. */
            ts->tv_nsec = timebase_info.numer / timebase_info.denom;
            return 0;
        default:
            break;
        }
    }

    SETERRNO(EINVAL, LIB_INVARG);
    return -1;
}

#    define clock_getres(clock_id, ts) th_clock_getres((clock_id), (ts))
#  endif /* TIME_HIRES_CLOCK_GETRES_EMULATION */

#  ifdef TIME_HIRES_CLOCK_NANOSLEEP_EMULATION
static int th_clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *rqtp,
                           struct timespec *rmtp) {
    if (darwin_time_init()) {
        switch (clock_id) {
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC:
            {
                uint64_t nanos = rqtp->tv_sec * IV_1E9 + rqtp->tv_nsec;
                int success;
                if ((flags & TIMER_ABSTIME)) {
                    uint64_t back =
                        timespec_init.tv_sec * IV_1E9 + timespec_init.tv_nsec;
                    nanos = nanos > back ? nanos - back : 0;
                }
                success =
                    mach_wait_until(mach_absolute_time() + nanos) == KERN_SUCCESS;

                /* In the relative sleep, the rmtp should be filled in with
                 * the 'unused' part of the rqtp in case the sleep gets
                 * interrupted by a signal.  But it is unknown how signals
                 * interact with mach_wait_until().  In the absolute sleep,
                 * the rmtp should stay untouched. */
                rmtp->tv_sec  = 0;
                rmtp->tv_nsec = 0;

                return success;
            }

        default:
            break;
        }
    }

    SETERRNO(EINVAL, LIB_INVARG);
    return -1;
}

#    define clock_nanosleep(clock_id, flags, rqtp, rmtp) \
  th_clock_nanosleep((clock_id), (flags), (rqtp), (rmtp))

#  endif /* TIME_HIRES_CLOCK_NANOSLEEP_EMULATION */

#endif /* PERL_DARWIN */

/* The macOS headers warn about using certain interfaces in
 * OS-release-ignorant manner, for example:
 *
 * warning: 'futimens' is only available on macOS 10.13 or newer
 *       [-Wunguarded-availability-new]
 *
 * (ditto for utimensat)
 *
 * There is clang __builtin_available() *runtime* check for this.
 * The gotchas are that neither __builtin_available() nor __has_builtin()
 * are always available.
 */
#ifndef __has_builtin
#  define __has_builtin(x) 0 /* non-clang */
#endif
#ifdef HAS_FUTIMENS
#  if defined(PERL_DARWIN) && __has_builtin(__builtin_available)
#    define FUTIMENS_AVAILABLE __builtin_available(macOS 10.13, *)
#  else
#    define FUTIMENS_AVAILABLE 1
#  endif
#else
#  define FUTIMENS_AVAILABLE 0
#endif
#ifdef HAS_UTIMENSAT
#  if defined(PERL_DARWIN) && __has_builtin(__builtin_available)
#    define UTIMENSAT_AVAILABLE __builtin_available(macOS 10.13, *)
#  else
#    define UTIMENSAT_AVAILABLE 1
#  endif
#else
#  define UTIMENSAT_AVAILABLE 0
#endif

#include "const-c.inc"

#if (defined(TIME_HIRES_NANOSLEEP)) || \
    (defined(TIME_HIRES_CLOCK_NANOSLEEP) && defined(TIMER_ABSTIME))

static void
nanosleep_init(NV nsec,
                    struct timespec *sleepfor,
                    struct timespec *unslept) {
  sleepfor->tv_sec = (Time_t)(nsec / NV_1E9);
  sleepfor->tv_nsec = (long)(nsec - ((NV)sleepfor->tv_sec) * NV_1E9);
  unslept->tv_sec = 0;
  unslept->tv_nsec = 0;
}

static NV
nsec_without_unslept(struct timespec *sleepfor,
                     const struct timespec *unslept) {
    if (sleepfor->tv_sec >= unslept->tv_sec) {
        sleepfor->tv_sec -= unslept->tv_sec;
        if (sleepfor->tv_nsec >= unslept->tv_nsec) {
            sleepfor->tv_nsec -= unslept->tv_nsec;
        } else if (sleepfor->tv_sec > 0) {
            sleepfor->tv_sec--;
            sleepfor->tv_nsec += IV_1E9;
            sleepfor->tv_nsec -= unslept->tv_nsec;
        } else {
            sleepfor->tv_sec = 0;
            sleepfor->tv_nsec = 0;
        }
    } else {
        sleepfor->tv_sec = 0;
        sleepfor->tv_nsec = 0;
    }
    return ((NV)sleepfor->tv_sec) * NV_1E9 + ((NV)sleepfor->tv_nsec);
}

#endif

/* In case Perl and/or Devel::PPPort are too old, minimally emulate
 * IS_SAFE_PATHNAME() (which looks for zero bytes in the pathname). */
#ifndef IS_SAFE_PATHNAME
#  if PERL_VERSION_GE(5,12,0) /* Perl_ck_warner is 5.10.0 -> */
#    ifdef WARN_SYSCALLS
#      define WARNEMUCAT WARN_SYSCALLS /* 5.22.0 -> */
#    else
#      define WARNEMUCAT WARN_MISC
#    endif
#    define WARNEMU(opname) Perl_ck_warner(aTHX_ packWARN(WARNEMUCAT), "Invalid \\0 character in pathname for %s",opname)
#  else
#    define WARNEMU(opname) Perl_warn(aTHX_ "Invalid \\0 character in pathname for %s",opname)
#  endif
#  define IS_SAFE_PATHNAME(pv, len, opname) (((len)>1)&&memchr((pv), 0, (len)-1)?(SETERRNO(ENOENT, LIB_INVARG),WARNEMU(opname),FALSE):(TRUE))
#endif

MODULE = Time::HiRes            PACKAGE = Time::HiRes

PROTOTYPES: ENABLE

BOOT:
    {
#ifdef MY_CXT_KEY
        MY_CXT_INIT;
#endif
#ifdef HAS_GETTIMEOFDAY
        {
            (void) hv_store(PL_modglobal, "Time::NVtime", 12,
                            newSViv(PTR2IV(myNVtime)), 0);
            (void) hv_store(PL_modglobal, "Time::U2time", 12,
                            newSViv(PTR2IV(myU2time)), 0);
        }
#endif
#if defined(PERL_DARWIN)
#  if defined(USE_ITHREADS) && defined(PERL_DARWIN_MUTEX)
        MUTEX_INIT(&darwin_time_mutex);
#  endif
#endif
    }

#if defined(USE_ITHREADS) && defined(MY_CXT_KEY)

void
CLONE(...)
    CODE:
        MY_CXT_CLONE;

#endif

INCLUDE: const-xs.inc

#if defined(HAS_USLEEP) && defined(HAS_GETTIMEOFDAY)

NV
usleep(useconds)
    NV useconds
    PREINIT:
        struct timeval Ta, Tb;
    CODE:
        gettimeofday(&Ta, NULL);
        if (items > 0) {
            if (useconds >= NV_1E6) {
                IV seconds = (IV) (useconds / NV_1E6);
                /* If usleep() has been implemented using setitimer()
                 * then this contortion is unnecessary-- but usleep()
                 * may be implemented in some other way, so let's contort. */
                if (seconds) {
                    sleep(seconds);
                    useconds -= NV_1E6 * seconds;
                }
            } else if (useconds < 0.0)
                croak("Time::HiRes::usleep(%" NVgf
                      "): negative time not invented yet", useconds);

            usleep((U32)useconds);
        } else
            PerlProc_pause();

        gettimeofday(&Tb, NULL);
#  if 0
        printf("[%ld %ld] [%ld %ld]\n", Tb.tv_sec, Tb.tv_usec, Ta.tv_sec, Ta.tv_usec);
#  endif
        RETVAL = NV_1E6*(Tb.tv_sec-Ta.tv_sec)+(NV)((IV)Tb.tv_usec-(IV)Ta.tv_usec);

    OUTPUT:
        RETVAL

#  if defined(TIME_HIRES_NANOSLEEP)

NV
nanosleep(nsec)
    NV nsec
    PREINIT:
        struct timespec sleepfor, unslept;
    CODE:
        if (nsec < 0.0)
            croak("Time::HiRes::nanosleep(%" NVgf
                  "): negative time not invented yet", nsec);
        nanosleep_init(nsec, &sleepfor, &unslept);
        if (nanosleep(&sleepfor, &unslept) == 0) {
            RETVAL = nsec;
        } else {
            RETVAL = nsec_without_unslept(&sleepfor, &unslept);
        }
    OUTPUT:
        RETVAL

#  else  /* #if defined(TIME_HIRES_NANOSLEEP) */

NV
nanosleep(nsec)
    NV nsec
    CODE:
        PERL_UNUSED_ARG(nsec);
        croak("Time::HiRes::nanosleep(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#  endif /* #if defined(TIME_HIRES_NANOSLEEP) */

NV
sleep(...)
    PREINIT:
        struct timeval Ta, Tb;
    CODE:
        gettimeofday(&Ta, NULL);
        if (items > 0) {
            NV seconds  = SvNV(ST(0));
            if (seconds >= 0.0) {
                UV useconds = (UV)(1E6 * (seconds - (UV)seconds));
                if (seconds >= 1.0)
                    sleep((U32)seconds);
                if ((IV)useconds < 0) {
#  if defined(__sparc64__) && defined(__GNUC__)
                    /* Sparc64 gcc 2.95.3 (e.g. on NetBSD) has a bug
                     * where (0.5 - (UV)(0.5)) will under certain
                     * circumstances (if the double is cast to UV more
                     * than once?) evaluate to -0.5, instead of 0.5. */
                    useconds = -(IV)useconds;
#  endif /* #if defined(__sparc64__) && defined(__GNUC__) */
                    if ((IV)useconds < 0)
                        croak("Time::HiRes::sleep(%" NVgf
                              "): internal error: useconds < 0 (unsigned %" UVuf
                              " signed %" IVdf ")",
                              seconds, useconds, (IV)useconds);
                }
                usleep(useconds);
            } else
                croak("Time::HiRes::sleep(%" NVgf
                      "): negative time not invented yet", seconds);
        } else
            PerlProc_pause();

        gettimeofday(&Tb, NULL);
#  if 0
        printf("[%ld %ld] [%ld %ld]\n", Tb.tv_sec, Tb.tv_usec, Ta.tv_sec, Ta.tv_usec);
#  endif
        RETVAL = (NV)(Tb.tv_sec-Ta.tv_sec)+0.000001*(NV)(Tb.tv_usec-Ta.tv_usec);

    OUTPUT:
        RETVAL

#else  /* #if defined(HAS_USLEEP) && defined(HAS_GETTIMEOFDAY) */

NV
usleep(useconds)
    NV useconds
    CODE:
        PERL_UNUSED_ARG(useconds);
        croak("Time::HiRes::usleep(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /* #if defined(HAS_USLEEP) && defined(HAS_GETTIMEOFDAY) */

#ifdef HAS_UALARM

IV
ualarm(useconds,uinterval=0)
    int useconds
    int uinterval
    CODE:
        if (useconds < 0 || uinterval < 0)
            croak("Time::HiRes::ualarm(%d, %d): negative time not invented yet", useconds, uinterval);
#  if defined(HAS_SETITIMER) && defined(ITIMER_REAL)
        {
            struct itimerval itv;
            if (hrt_ualarm_itimero(&itv, useconds, uinterval)) {
                /* To conform to ualarm's interface, we're actually ignoring
                   an error here.  */
                RETVAL = 0;
            } else {
                RETVAL = itv.it_value.tv_sec * IV_1E6 + itv.it_value.tv_usec;
            }
        }
#  else
        if (useconds >= IV_1E6 || uinterval >= IV_1E6)
            croak("Time::HiRes::ualarm(%d, %d): useconds or uinterval"
                  " equal to or more than %" IVdf,
                  useconds, uinterval, IV_1E6);

        RETVAL = ualarm(useconds, uinterval);
#  endif

    OUTPUT:
        RETVAL

NV
alarm(seconds,interval=0)
    NV seconds
    NV interval
    CODE:
        if (seconds < 0.0 || interval < 0.0)
            croak("Time::HiRes::alarm(%" NVgf ", %" NVgf
                  "): negative time not invented yet", seconds, interval);

        {
            IV iseconds = (IV)seconds;
            IV iinterval = (IV)interval;
            NV fseconds = seconds - iseconds;
            NV finterval = interval - iinterval;
            IV useconds, uinterval;
            if (fseconds >= 1.0 || finterval >= 1.0)
                croak("Time::HiRes::alarm(%" NVgf ", %" NVgf
                      "): seconds or interval too large to split correctly",
                      seconds, interval);

            useconds = IV_1E6 * fseconds;
            uinterval = IV_1E6 * finterval;
#  if defined(HAS_SETITIMER) && defined(ITIMER_REAL)
            {
                struct itimerval nitv, oitv;
                nitv.it_value.tv_sec = iseconds;
                nitv.it_value.tv_usec = useconds;
                nitv.it_interval.tv_sec = iinterval;
                nitv.it_interval.tv_usec = uinterval;
                if (setitimer(ITIMER_REAL, &nitv, &oitv)) {
                    /* To conform to alarm's interface, we're actually ignoring
                       an error here.  */
                    RETVAL = 0;
                } else {
                    RETVAL = oitv.it_value.tv_sec + ((NV)oitv.it_value.tv_usec) / NV_1E6;
                }
            }
#  else
            if (iseconds || iinterval)
                croak("Time::HiRes::alarm(%" NVgf ", %" NVgf
                      "): seconds or interval equal to or more than 1.0 ",
                      seconds, interval);

            RETVAL = (NV)ualarm( useconds, uinterval ) / NV_1E6;
#  endif
        }

    OUTPUT:
        RETVAL

#else /* #ifdef HAS_UALARM */

int
ualarm(useconds,interval=0)
    int useconds
    int interval
    CODE:
        PERL_UNUSED_ARG(useconds);
        PERL_UNUSED_ARG(interval);
        croak("Time::HiRes::ualarm(): unimplemented in this platform");
        RETVAL = -1;
    OUTPUT:
        RETVAL

NV
alarm(seconds,interval=0)
    NV seconds
    NV interval
    CODE:
        PERL_UNUSED_ARG(seconds);
        PERL_UNUSED_ARG(interval);
        croak("Time::HiRes::alarm(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /* #ifdef HAS_UALARM */

#ifdef HAS_GETTIMEOFDAY

void
gettimeofday()
    PREINIT:
        struct timeval Tp;
    PPCODE:
        int status;
        status = gettimeofday (&Tp, NULL);
        if (status == 0) {
            if (GIMME_V == G_LIST) {
                EXTEND(sp, 2);
                PUSHs(sv_2mortal(newSViv(Tp.tv_sec)));
                PUSHs(sv_2mortal(newSViv(Tp.tv_usec)));
            } else {
                EXTEND(sp, 1);
                PUSHs(sv_2mortal(newSVnv(Tp.tv_sec + (Tp.tv_usec / NV_1E6))));
            }
        }

NV
time()
    PREINIT:
        struct timeval Tp;
    CODE:
        int status;
        status = gettimeofday (&Tp, NULL);
        if (status == 0) {
            RETVAL = Tp.tv_sec + (Tp.tv_usec / NV_1E6);
        } else {
            RETVAL = -1.0;
        }
    OUTPUT:
        RETVAL

#endif /* #ifdef HAS_GETTIMEOFDAY */

#if defined(HAS_GETITIMER) && defined(HAS_SETITIMER)

#  define TV2NV(tv) ((NV)((tv).tv_sec) + 0.000001 * (NV)((tv).tv_usec))

void
setitimer(which, seconds, interval = 0)
    int which
    NV seconds
    NV interval
    PREINIT:
        struct itimerval newit;
        struct itimerval oldit;
    PPCODE:
        if (seconds < 0.0 || interval < 0.0)
            croak("Time::HiRes::setitimer(%" IVdf ", %" NVgf ", %" NVgf
                  "): negative time not invented yet",
                  (IV)which, seconds, interval);
        newit.it_value.tv_sec  = (IV)seconds;
        newit.it_value.tv_usec =
          (IV)((seconds  - (NV)newit.it_value.tv_sec)    * NV_1E6);
        newit.it_interval.tv_sec  = (IV)interval;
        newit.it_interval.tv_usec =
          (IV)((interval - (NV)newit.it_interval.tv_sec) * NV_1E6);
        /* on some platforms the 1st arg to setitimer is an enum, which
         * causes -Wc++-compat to complain about passing an int instead
         */
        GCC_DIAG_IGNORE_STMT(-Wc++-compat);
        if (setitimer(which, &newit, &oldit) == 0) {
            EXTEND(sp, 1);
            PUSHs(sv_2mortal(newSVnv(TV2NV(oldit.it_value))));
            if (GIMME_V == G_LIST) {
                EXTEND(sp, 1);
                PUSHs(sv_2mortal(newSVnv(TV2NV(oldit.it_interval))));
            }
        }
        GCC_DIAG_RESTORE_STMT;

void
getitimer(which)
    int which
    PREINIT:
        struct itimerval nowit;
    PPCODE:
        /* on some platforms the 1st arg to getitimer is an enum, which
         * causes -Wc++-compat to complain about passing an int instead
         */
        GCC_DIAG_IGNORE_STMT(-Wc++-compat);
        if (getitimer(which, &nowit) == 0) {
            EXTEND(sp, 1);
            PUSHs(sv_2mortal(newSVnv(TV2NV(nowit.it_value))));
            if (GIMME_V == G_LIST) {
                EXTEND(sp, 1);
                PUSHs(sv_2mortal(newSVnv(TV2NV(nowit.it_interval))));
            }
        }
        GCC_DIAG_RESTORE_STMT;

#endif /* #if defined(HAS_GETITIMER) && defined(HAS_SETITIMER) */

#if defined(TIME_HIRES_UTIME)

I32
utime(accessed, modified, ...)
PROTOTYPE: $$@
    PREINIT:
        SV* accessed;
        SV* modified;
        SV* file;

        struct timespec utbuf[2];
        struct timespec *utbufp = utbuf;
        int tot;

    CODE:
        accessed = ST(0);
        modified = ST(1);
        items -= 2;
        tot = 0;

        if ( accessed == &PL_sv_undef && modified == &PL_sv_undef )
            utbufp = NULL;
        else {
            if (SvNV(accessed) < 0.0 || SvNV(modified) < 0.0)
                croak("Time::HiRes::utime(%" NVgf ", %" NVgf
                      "): negative time not invented yet",
                          SvNV(accessed), SvNV(modified));
            Zero(&utbuf, sizeof utbuf, char);

            utbuf[0].tv_sec = (Time_t)SvNV(accessed);  /* time accessed */
            utbuf[0].tv_nsec = (long)(
                (SvNV(accessed) - (NV)utbuf[0].tv_sec)
                * NV_1E9 + (NV)0.5);

            utbuf[1].tv_sec = (Time_t)SvNV(modified);  /* time modified */
            utbuf[1].tv_nsec = (long)(
                (SvNV(modified) - (NV)utbuf[1].tv_sec)
                * NV_1E9 + (NV)0.5);
        }

        while (items > 0) {
            file = POPs; items--;

            if (SvROK(file) && GvIO(SvRV(file)) && IoIFP(sv_2io(SvRV(file)))) {
	        int fd =  PerlIO_fileno(IoIFP(sv_2io(file)));
                if (fd < 0) {
                    SETERRNO(EBADF,RMS_IFI);
                } else {
#  ifdef HAS_FUTIMENS
                    if (FUTIMENS_AVAILABLE) {
                        if (futimens(fd, utbufp) == 0) {
                            tot++;
                        }
                    } else {
                        croak("futimens unimplemented in this platform");
                    }
#  else  /* HAS_FUTIMENS */
                    croak("futimens unimplemented in this platform");
#  endif /* HAS_FUTIMENS */
                }
            }
            else {
#  ifdef HAS_UTIMENSAT
                if (UTIMENSAT_AVAILABLE) {
                    STRLEN len;
                    char * name = SvPV(file, len);
                    if (IS_SAFE_PATHNAME(name, len, "utime") &&
                        utimensat(AT_FDCWD, name, utbufp, 0) == 0) {

                        tot++;
                    }
                } else {
                    croak("utimensat unimplemented in this platform");
                }
#  else  /* HAS_UTIMENSAT */
                croak("utimensat unimplemented in this platform");
#  endif /* HAS_UTIMENSAT */
            }
        } /* while items */
        RETVAL = tot;

    OUTPUT:
        RETVAL

#else  /* #if defined(TIME_HIRES_UTIME) */

I32
utime(accessed, modified, ...)
    CODE:
        croak("Time::HiRes::utime(): unimplemented in this platform");
        RETVAL = 0;
    OUTPUT:
        RETVAL

#endif /* #if defined(TIME_HIRES_UTIME) */

#if defined(TIME_HIRES_CLOCK_GETTIME)

NV
clock_gettime(clock_id = CLOCK_REALTIME)
    clockid_t clock_id
    PREINIT:
        struct timespec ts;
        int status = -1;
    CODE:
#  ifdef TIME_HIRES_CLOCK_GETTIME_SYSCALL
        status = syscall(SYS_clock_gettime, clock_id, &ts);
#  else
        status = clock_gettime(clock_id, &ts);
#  endif
        RETVAL = status == 0 ? ts.tv_sec + (NV) ts.tv_nsec / NV_1E9 : -1;

    OUTPUT:
        RETVAL

#else  /* if defined(TIME_HIRES_CLOCK_GETTIME) */

NV
clock_gettime(clock_id = 0)
    clockid_t clock_id
    CODE:
        PERL_UNUSED_ARG(clock_id);
        croak("Time::HiRes::clock_gettime(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /*  #if defined(TIME_HIRES_CLOCK_GETTIME) */

#if defined(TIME_HIRES_CLOCK_GETRES)

NV
clock_getres(clock_id = CLOCK_REALTIME)
    clockid_t clock_id
    PREINIT:
        int status = -1;
        struct timespec ts;
    CODE:
#  ifdef TIME_HIRES_CLOCK_GETRES_SYSCALL
        status = syscall(SYS_clock_getres, clock_id, &ts);
#  else
        status = clock_getres(clock_id, &ts);
#  endif
        RETVAL = status == 0 ? ts.tv_sec + (NV) ts.tv_nsec / NV_1E9 : -1;

    OUTPUT:
        RETVAL

#else  /* if defined(TIME_HIRES_CLOCK_GETRES) */

NV
clock_getres(clock_id = 0)
    clockid_t clock_id
    CODE:
        PERL_UNUSED_ARG(clock_id);
        croak("Time::HiRes::clock_getres(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /*  #if defined(TIME_HIRES_CLOCK_GETRES) */

#if defined(TIME_HIRES_CLOCK_NANOSLEEP) && defined(TIMER_ABSTIME)

NV
clock_nanosleep(clock_id, nsec, flags = 0)
    clockid_t clock_id
    NV  nsec
    int flags
    PREINIT:
        struct timespec sleepfor, unslept;
    CODE:
        if (nsec < 0.0)
            croak("Time::HiRes::clock_nanosleep(..., %" NVgf
                  "): negative time not invented yet", nsec);
        nanosleep_init(nsec, &sleepfor, &unslept);
        if (clock_nanosleep(clock_id, flags, &sleepfor, &unslept) == 0) {
            RETVAL = nsec;
        } else {
            RETVAL = nsec_without_unslept(&sleepfor, &unslept);
        }
    OUTPUT:
        RETVAL

#else  /* if defined(TIME_HIRES_CLOCK_NANOSLEEP) && defined(TIMER_ABSTIME) */

NV
clock_nanosleep(clock_id, nsec, flags = 0)
    clockid_t clock_id
    NV  nsec
    int flags
    CODE:
        PERL_UNUSED_ARG(clock_id);
        PERL_UNUSED_ARG(nsec);
        PERL_UNUSED_ARG(flags);
        croak("Time::HiRes::clock_nanosleep(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /*  #if defined(TIME_HIRES_CLOCK_NANOSLEEP) && defined(TIMER_ABSTIME) */

#if defined(TIME_HIRES_CLOCK) && defined(CLOCKS_PER_SEC)

NV
clock()
    PREINIT:
        clock_t clocks;
    CODE:
        clocks = clock();
        RETVAL = clocks == (clock_t) -1 ? (clock_t) -1 : (NV)clocks / (NV)CLOCKS_PER_SEC;

    OUTPUT:
        RETVAL

#else  /* if defined(TIME_HIRES_CLOCK) && defined(CLOCKS_PER_SEC) */

NV
clock()
    CODE:
        croak("Time::HiRes::clock(): unimplemented in this platform");
        RETVAL = 0.0;
    OUTPUT:
        RETVAL

#endif /*  #if defined(TIME_HIRES_CLOCK) && defined(CLOCKS_PER_SEC) */

void
stat(...)
PROTOTYPE: ;$
    PREINIT:
        OP fakeop;
        int nret;
    ALIAS:
        Time::HiRes::lstat = 1
    PPCODE:
        XPUSHs(sv_2mortal(newSVsv(items == 1 ? ST(0) : DEFSV)));
        PUTBACK;
        ENTER;
        PL_laststatval = -1;
        SAVEOP();
        Zero(&fakeop, 1, OP);
        fakeop.op_type = ix ? OP_LSTAT : OP_STAT;
        fakeop.op_ppaddr = PL_ppaddr[fakeop.op_type];
        fakeop.op_flags = GIMME_V == G_LIST ? OPf_WANT_LIST :
            GIMME_V == G_SCALAR ? OPf_WANT_SCALAR : OPf_WANT_VOID;
        PL_op = &fakeop;
        (void)fakeop.op_ppaddr(aTHX);
        SPAGAIN;
        LEAVE;
        nret = SP+1 - &ST(0);
        if (nret == 13) {
            UV atime = SvUV(ST( 8));
            UV mtime = SvUV(ST( 9));
            UV ctime = SvUV(ST(10));
            UV atime_nsec;
            UV mtime_nsec;
            UV ctime_nsec;
            hrstatns(&atime_nsec, &mtime_nsec, &ctime_nsec);
            if (atime_nsec)
                ST( 8) = sv_2mortal(newSVnv(atime + (NV) atime_nsec / NV_1E9));
            if (mtime_nsec)
                ST( 9) = sv_2mortal(newSVnv(mtime + (NV) mtime_nsec / NV_1E9));
            if (ctime_nsec)
                ST(10) = sv_2mortal(newSVnv(ctime + (NV) ctime_nsec / NV_1E9));
        }
        XSRETURN(nret);
