/* Copyright (C) 1991-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

/* Modified by Chet Ramey <chet.ramey@case.edu> for inclusion in bash. */

#include <config.h>

#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "typemax.h"
#include "loadables.h"
#include "getconf.h"

#ifndef errno
extern int errno;
#endif

/* Hack to `encode' values wider than long into a conf_variable */
#define VAL_LLONG_MIN           -1000
#define VAL_LLONG_MAX           -1001
#define VAL_ULLONG_MAX          -1002
#define VAL_ULONG_MAX		-1003
#define VAL_SSIZE_MAX		-1004
#define VAL_SIZE_MAX		-1005

struct conf
  {
    const char *name;
    const long call_name;	/* or value for CONSTANT */
    const enum { SYSCONF, CONFSTR, PATHCONF, CONSTANT, UNDEFINED } call;
  };

static const struct conf vars[] =
  {
#ifdef _PC_LINK_MAX
    { "LINK_MAX", _PC_LINK_MAX, PATHCONF },
    { "_POSIX_LINK_MAX", _PC_LINK_MAX, PATHCONF },
#else
    { "LINK_MAX", _POSIX_LINK_MAX, CONSTANT },
    { "_POSIX_LINK_MAX", _POSIX_LINK_MAX, CONSTANT },
#endif
#ifdef _PC_MAX_CANON
    { "MAX_CANON", _PC_MAX_CANON, PATHCONF },
    { "_POSIX_MAX_CANON", _PC_MAX_CANON, PATHCONF },
#else
    { "MAX_CANON", _POSIX_MAX_CANON, CONSTANT },
    { "_POSIX_MAX_CANON", _POSIX_MAX_CANON, CONSTANT },
#endif
#ifdef _PC_MAX_INPUT
    { "MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
    { "_POSIX_MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
#else
    { "MAX_INPUT", _POSIX_MAX_INPUT, CONSTANT },
    { "_POSIX_MAX_INPUT", _POSIX_MAX_INPUT, CONSTANT },
#endif
#ifdef _PC_NAME_MAX
    { "NAME_MAX", _PC_NAME_MAX, PATHCONF },
    { "_POSIX_NAME_MAX", _PC_NAME_MAX, PATHCONF },
#else
    { "NAME_MAX", _POSIX_NAME_MAX, CONSTANT },
    { "_POSIX_NAME_MAX", _POSIX_NAME_MAX, CONSTANT },
#endif
#ifdef _PC_PATH_MAX
    { "PATH_MAX", _PC_PATH_MAX, PATHCONF },
    { "_POSIX_PATH_MAX", _PC_PATH_MAX, PATHCONF },
#else
    { "PATH_MAX", _POSIX_PATH_MAX, CONSTANT },
    { "_POSIX_PATH_MAX", _POSIX_PATH_MAX, CONSTANT },
#endif
#ifdef _PC_PIPE_BUF
    { "PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
    { "_POSIX_PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
#else
    { "PIPE_BUF", _POSIX_PIPE_BUF, CONSTANT },
    { "_POSIX_PIPE_BUF", _POSIX_PIPE_BUF, CONSTANT },
#endif
#ifdef _PC_SOCK_MAXBUF
    { "SOCK_MAXBUF", _PC_SOCK_MAXBUF, PATHCONF },
#endif
#ifdef _PC_ASYNC_IO
    { "_POSIX_ASYNC_IO", _PC_ASYNC_IO, PATHCONF },
#endif
    { "_POSIX_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED, PATHCONF },
    { "_POSIX_NO_TRUNC", _PC_NO_TRUNC, PATHCONF },
#ifdef _PC_PRIO_IO
    { "_POSIX_PRIO_IO", _PC_PRIO_IO, PATHCONF },
#endif
#ifdef _PC_SYNC_IO
    { "_POSIX_SYNC_IO", _PC_SYNC_IO, PATHCONF },
#endif
    { "_POSIX_VDISABLE", _PC_VDISABLE, PATHCONF },

    { "ARG_MAX", _SC_ARG_MAX, SYSCONF },
    { "ATEXIT_MAX", _SC_ATEXIT_MAX, SYSCONF },
#ifdef _SC_CHAR_BIT
    { "CHAR_BIT", _SC_CHAR_BIT, SYSCONF },
#else
    { "CHAR_BIT", CHAR_BIT, CONSTANT },
#endif
#ifdef _SC_CHAR_MAX
    { "CHAR_MAX", _SC_CHAR_MAX, SYSCONF },
#else
    { "CHAR_BIT", CHAR_MAX, CONSTANT },
#endif
#ifdef _SC_CHAR_MIN
    { "CHAR_MIN", _SC_CHAR_MIN, SYSCONF },
#else
    { "CHAR_MIN", CHAR_MIN, CONSTANT },
#endif
    { "CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
    { "CLK_TCK", _SC_CLK_TCK, SYSCONF },
#ifdef _SC_INT_BIT
    { "INT_BIT", _SC_INT_BIT, SYSCONF },
#else
    { "INT_BIT", INT_BIT, CONSTANT },
#endif
#ifdef _SC_INT_MAX
    { "INT_MAX", _SC_INT_MAX, SYSCONF },
#else
    { "INT_MAX", INT_MAX, CONSTANT },
#endif
#ifdef _SC_INT_MIN
    { "INT_MIN", _SC_INT_MIN, SYSCONF },
#else
    { "INT_MIN", INT_MIN, CONSTANT },
#endif
#ifdef _SC_UIO_MAXIOV
    { "IOV_MAX", _SC_UIO_MAXIOV, SYSCONF },
#endif
    { "LOGNAME_MAX", _SC_LOGIN_NAME_MAX, SYSCONF },
#ifdef _SC_LONG_BIT
    { "LONG_BIT", _SC_LONG_BIT, SYSCONF },
#else
    { "LONG_BIT", LONG_BIT, CONSTANT },
#endif
    { "LONG_MIN", LONG_MIN, CONSTANT },
    { "LONG_MAX", LONG_MAX, CONSTANT },
#if HAVE_LONG_LONG_INT
    { "LLONG_MIN", VAL_LLONG_MIN, CONSTANT },
    { "LLONG_MAX", VAL_LLONG_MAX, CONSTANT },
#else
    { "LLONG_MIN", LLONG_MIN, CONSTANT },
    { "LLONG_MAX", LLONG_MAX, CONSTANT },
#endif
#ifdef _SC_MB_LEN_MAX
    { "MB_LEN_MAX", _SC_MB_LEN_MAX, SYSCONF },
#else
    { "MB_LEN_MAX", MB_LEN_MAX, CONSTANT },
#endif
    { "NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
#ifdef _SC_NL_ARGMAX
    { "NL_ARGMAX", _SC_NL_ARGMAX, SYSCONF },
#endif
#ifdef _SC_NL_LANGMAX
    { "NL_LANGMAX", _SC_NL_LANGMAX, SYSCONF },
#endif
#ifdef _SC_NL_MSGMAX
    { "NL_MSGMAX", _SC_NL_MSGMAX, SYSCONF },
#endif
#ifdef _SC_NL_NMAX
    { "NL_NMAX", _SC_NL_NMAX, SYSCONF },
#endif
#ifdef _SC_NL_SETMAX
    { "NL_SETMAX", _SC_NL_SETMAX, SYSCONF },
#endif
#ifdef _SC_NL_TEXTMAX
    { "NL_TEXTMAX", _SC_NL_TEXTMAX, SYSCONF },
#endif
    { "NSS_BUFLEN_GROUP", _SC_GETGR_R_SIZE_MAX, SYSCONF },
    { "NSS_BUFLEN_PASSWD", _SC_GETPW_R_SIZE_MAX, SYSCONF },
#ifdef _SC_NZERO
    { "NZERO", _SC_NZERO, SYSCONF },
#endif
    { "OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
    { "PAGESIZE", _SC_PAGESIZE, SYSCONF },
    { "PAGE_SIZE", _SC_PAGESIZE, SYSCONF },
#ifdef _SC_PASS_MAX
    { "PASS_MAX", _SC_PASS_MAX, SYSCONF },
#endif
    { "PTHREAD_DESTRUCTOR_ITERATIONS", _SC_THREAD_DESTRUCTOR_ITERATIONS, SYSCONF },
    { "PTHREAD_KEYS_MAX", _SC_THREAD_KEYS_MAX, SYSCONF },
    { "PTHREAD_STACK_MIN", _SC_THREAD_STACK_MIN, SYSCONF },
    { "PTHREAD_THREADS_MAX", _SC_THREAD_THREADS_MAX, SYSCONF },
#ifdef _SC_SCHAR_MAX
    { "SCHAR_MAX", _SC_SCHAR_MAX, SYSCONF },
#else
    { "SCHAR_MAX", SCHAR_MAX, CONSTANT },
#endif
#ifdef _SC_SCHAR_MIN
    { "SCHAR_MIN", _SC_SCHAR_MIN, SYSCONF },
#else
    { "SCHAR_MIN", SCHAR_MIN, CONSTANT },
#endif
#ifdef _SC_SHRT_MAX
    { "SHRT_MAX", _SC_SHRT_MAX, SYSCONF },
#else
    { "SHRT_MAX", SHRT_MAX, CONSTANT },
#endif
#ifdef _SC_SHRT_MIN
    { "SHRT_MIN", _SC_SHRT_MIN, SYSCONF },
#else
    { "SHRT_MIN", SHRT_MIN, CONSTANT },
#endif
#ifdef _SC_SIZE_MAX
    { "SIZE_MAX", _SC_SIZE_MAX, SYSCONF },
#else
    { "SIZE_MAX", VAL_SIZE_MAX, CONSTANT },
#endif
#ifdef _SC_SSIZE_MAX
    { "SSIZE_MAX", _SC_SSIZE_MAX, SYSCONF },
#elif SIZEOF_SIZE_MAX == 8
    { "SSIZE_MAX", VAL_SSIZE_MAX, CONSTANT },
#else
    { "SSIZE_MAX", VAL_SSIZE_MAX, CONSTANT },
#endif
    { "TTY_NAME_MAX", _SC_TTY_NAME_MAX, SYSCONF },
    { "TZNAME_MAX", _SC_TZNAME_MAX, SYSCONF },
#ifdef _SC_UCHAR_MAX
    { "UCHAR_MAX", _SC_UCHAR_MAX, SYSCONF },
#else
    { "UCHAR_MAX", UCHAR_MAX, CONSTANT },
#endif
#ifdef _SC_UINT_MAX
    { "UINT_MAX", _SC_UINT_MAX, SYSCONF },
#else
    { "UINT_MAX", UINT_MAX, CONSTANT },
#endif
#ifdef _SC_UIO_MAXIOV
    { "UIO_MAXIOV", _SC_UIO_MAXIOV, SYSCONF },
#endif
#ifdef _SC_ULONG_MAX
    { "ULONG_MAX", _SC_ULONG_MAX, SYSCONF },
#else
    { "ULONG_MAX", VAL_ULONG_MAX, CONSTANT },
#endif
#ifdef HAVE_LONG_LONG_INT
    { "ULLONG_MAX", VAL_ULLONG_MAX, CONSTANT },
#endif
#ifdef _SC_USHRT_MAX
    { "USHRT_MAX", _SC_USHRT_MAX, SYSCONF },
#else
    { "USHRT_MAX", USHRT_MAX, CONSTANT },
#endif
#ifdef _SC_WORD_BIT
    { "WORD_BIT", _SC_WORD_BIT, SYSCONF },
#else
    { "WORD_BIT", WORD_BIT, CONSTANT },
#endif
#ifdef _SC_AVPHYS_PAGES
    { "_AVPHYS_PAGES", _SC_AVPHYS_PAGES, SYSCONF },
#endif
    { "_NPROCESSORS_CONF", _SC_NPROCESSORS_CONF, SYSCONF },
    { "_NPROCESSORS_ONLN", _SC_NPROCESSORS_ONLN, SYSCONF },
    { "_PHYS_PAGES", _SC_PHYS_PAGES, SYSCONF },
#ifdef _SC_ARG_MAX
    { "_POSIX_ARG_MAX", _SC_ARG_MAX, SYSCONF },
#else
    { "_POSIX_ARG_MAX", _POSIX_ARG_MAX, CONSTANT },
#endif
    { "_POSIX_ASYNCHRONOUS_IO", _SC_ASYNCHRONOUS_IO, SYSCONF },
#ifdef _SC_CHILD_MAX
    { "_POSIX_CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
#else
    { "_POSIX_CHILD_MAX", _POSIX_CHILD_MAX, CONSTANT },
#endif
    { "_POSIX_FSYNC", _SC_FSYNC, SYSCONF },
    { "_POSIX_JOB_CONTROL", _SC_JOB_CONTROL, SYSCONF },
    { "_POSIX_MAPPED_FILES", _SC_MAPPED_FILES, SYSCONF },
    { "_POSIX_MEMLOCK", _SC_MEMLOCK, SYSCONF },
    { "_POSIX_MEMLOCK_RANGE", _SC_MEMLOCK_RANGE, SYSCONF },
    { "_POSIX_MEMORY_PROTECTION", _SC_MEMORY_PROTECTION, SYSCONF },
    { "_POSIX_MESSAGE_PASSING", _SC_MESSAGE_PASSING, SYSCONF },
    { "_POSIX_NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
    { "_POSIX_OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
#ifdef _SC_PII
    { "_POSIX_PII", _SC_PII, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET
    { "_POSIX_PII_INTERNET", _SC_PII_INTERNET, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET_DGRAM
    { "_POSIX_PII_INTERNET_DGRAM", _SC_PII_INTERNET_DGRAM, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET_STREAM
    { "_POSIX_PII_INTERNET_STREAM", _SC_PII_INTERNET_STREAM, SYSCONF },
#endif
#ifdef _SC_PII_OSI
    { "_POSIX_PII_OSI", _SC_PII_OSI, SYSCONF },
#endif
#ifdef _SC_PII_OSI_CLTS
    { "_POSIX_PII_OSI_CLTS", _SC_PII_OSI_CLTS, SYSCONF },
#endif
#ifdef _SC_PII_OSI_COTS
    { "_POSIX_PII_OSI_COTS", _SC_PII_OSI_COTS, SYSCONF },
#endif
#ifdef _SC_PII_OSI_M
    { "_POSIX_PII_OSI_M", _SC_PII_OSI_M, SYSCONF },
#endif
#ifdef _SC_PII_SOCKET
    { "_POSIX_PII_SOCKET", _SC_PII_SOCKET, SYSCONF },
#endif
#ifdef _SC_PII_XTI
    { "_POSIX_PII_XTI", _SC_PII_XTI, SYSCONF },
#endif
#ifdef _SC_POLL
    { "_POSIX_POLL", _SC_POLL, SYSCONF },
#endif
#ifdef _SC_PRIORITIZED_IO
    { "_POSIX_PRIORITIZED_IO", _SC_PRIORITIZED_IO, SYSCONF },
#endif
    { "_POSIX_PRIORITY_SCHEDULING", _SC_PRIORITY_SCHEDULING, SYSCONF },
    { "_POSIX_REALTIME_SIGNALS", _SC_REALTIME_SIGNALS, SYSCONF },
    { "_POSIX_SAVED_IDS", _SC_SAVED_IDS, SYSCONF },
#ifdef _SC_SELECT
    { "_POSIX_SELECT", _SC_SELECT, SYSCONF },
#endif
    { "_POSIX_SEMAPHORES", _SC_SEMAPHORES, SYSCONF },
    { "_POSIX_SHARED_MEMORY_OBJECTS", _SC_SHARED_MEMORY_OBJECTS, SYSCONF },
#ifdef _SC_SSIZE_MAX
    { "_POSIX_SSIZE_MAX", _SC_SSIZE_MAX, SYSCONF },
#elif SIZEOF_SIZE_T == 8
    { "_POSIX_SSIZE_MAX", VAL_SSIZE_MAX, CONSTANT },
#else
    { "_POSIX_SSIZE_MAX", VAL_SSIZE_MAX, CONSTANT },
#endif
#ifdef _SC_STREAM_MAX
    { "_POSIX_STREAM_MAX", _SC_STREAM_MAX, SYSCONF },
#else
    { "_POSIX_STREAM_MAX", _POSIX_STREAM_MAX, CONSTANT },
#endif
    { "_POSIX_SYNCHRONIZED_IO", _SC_SYNCHRONIZED_IO, SYSCONF },
    { "_POSIX_THREADS", _SC_THREADS, SYSCONF },
    { "_POSIX_THREAD_ATTR_STACKADDR", _SC_THREAD_ATTR_STACKADDR, SYSCONF },
    { "_POSIX_THREAD_ATTR_STACKSIZE", _SC_THREAD_ATTR_STACKSIZE, SYSCONF },
    { "_POSIX_THREAD_PRIORITY_SCHEDULING", _SC_THREAD_PRIORITY_SCHEDULING, SYSCONF },
    { "_POSIX_THREAD_PRIO_INHERIT", _SC_THREAD_PRIO_INHERIT, SYSCONF },
    { "_POSIX_THREAD_PRIO_PROTECT", _SC_THREAD_PRIO_PROTECT, SYSCONF },
#ifdef _SC_THREAD_ROBUST_PRIO_INHERIT
    { "_POSIX_THREAD_ROBUST_PRIO_INHERIT", _SC_THREAD_ROBUST_PRIO_INHERIT,
      SYSCONF },
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_PROTECT
    { "_POSIX_THREAD_ROBUST_PRIO_PROTECT", _SC_THREAD_ROBUST_PRIO_PROTECT,
      SYSCONF },
#endif
    { "_POSIX_THREAD_PROCESS_SHARED", _SC_THREAD_PROCESS_SHARED, SYSCONF },
    { "_POSIX_THREAD_SAFE_FUNCTIONS", _SC_THREAD_SAFE_FUNCTIONS, SYSCONF },
    { "_POSIX_TIMERS", _SC_TIMERS, SYSCONF },
    { "TIMER_MAX", _SC_TIMER_MAX, SYSCONF },
#ifdef _POSIX_TZNAME_MAX
    { "_POSIX_TZNAME_MAX", _SC_TZNAME_MAX, SYSCONF },
#else
    { "_POSIX_TZNAME_MAX", _POSIX_TZNAME_MAX, CONSTANT },
#endif
    { "_POSIX_VERSION", _SC_VERSION, SYSCONF },
#ifdef _SC_T_IOV_MAX
    { "_T_IOV_MAX", _SC_T_IOV_MAX, SYSCONF },
#endif
#ifdef _SC_XOPEN_CRYPT
    { "_XOPEN_CRYPT", _SC_XOPEN_CRYPT, SYSCONF },
#endif
#ifdef _SC_XOPEN_ENH_I18N
    { "_XOPEN_ENH_I18N", _SC_XOPEN_ENH_I18N, SYSCONF },
#endif
#ifdef _SC_XOPEN_LEGACY
    { "_XOPEN_LEGACY", _SC_XOPEN_LEGACY, SYSCONF },
#endif
#ifdef _SC_XOPEN_REALTIME
    { "_XOPEN_REALTIME", _SC_XOPEN_REALTIME, SYSCONF },
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
    { "_XOPEN_REALTIME_THREADS", _SC_XOPEN_REALTIME_THREADS, SYSCONF },
#endif
#ifdef _SC_XOPEN_SHM
    { "_XOPEN_SHM", _SC_XOPEN_SHM, SYSCONF },
#endif
#ifdef _SC_XOPEN_UNIX
    { "_XOPEN_UNIX", _SC_XOPEN_UNIX, SYSCONF },
#endif
#ifdef _SC_XOPEN_VERSION
    { "_XOPEN_VERSION", _SC_XOPEN_VERSION, SYSCONF },
#endif
#ifdef _SC_XOPEN_XCU_VERSION
    { "_XOPEN_XCU_VERSION", _SC_XOPEN_XCU_VERSION, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG2
    { "_XOPEN_XPG2", _SC_XOPEN_XPG2, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG3
    { "_XOPEN_XPG3", _SC_XOPEN_XPG3, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG4
    { "_XOPEN_XPG4", _SC_XOPEN_XPG4, SYSCONF },
#endif
    /* POSIX.2  */
#ifdef _SC_BC_BASE_MAX
    { "BC_BASE_MAX", _SC_BC_BASE_MAX, SYSCONF },
    { "POSIX2_BC_BASE_MAX", _SC_BC_BASE_MAX, SYSCONF },
#else
    { "BC_BASE_MAX", _POSIX2_BC_BASE_MAX, CONSTANT },
#endif
#ifdef _SC_BC_BASE_MAX
    { "BC_DIM_MAX", _SC_BC_DIM_MAX, SYSCONF },
    { "POSIX2_BC_DIM_MAX", _SC_BC_DIM_MAX, SYSCONF },
#else
    { "BC_DIM_MAX", _POSIX2_BC_DIM_MAX, CONSTANT },
#endif
#ifdef _SC_BC_SCALE_MAX
    { "BC_SCALE_MAX", _SC_BC_SCALE_MAX, SYSCONF },
    { "POSIX2_BC_SCALE_MAX", _SC_BC_SCALE_MAX, SYSCONF },
#else
    { "BC_SCALE_MAX", _POSIX2_BC_SCALE_MAX, CONSTANT },
#endif
#ifdef _SC_BC_STRING_MAX
    { "BC_STRING_MAX", _SC_BC_STRING_MAX, SYSCONF },
    { "POSIX2_BC_STRING_MAX", _SC_BC_STRING_MAX, SYSCONF },
#else
    { "BC_STRING_MAX", _POSIX2_BC_STRING_MAX, CONSTANT },
    { "POSIX2_BC_STRING_MAX", _POSIX2_BC_STRING_MAX, CONSTANT },
#endif
#ifdef _SC_CHARCLASS_NAME_MAX
    { "CHARCLASS_NAME_MAX", _SC_CHARCLASS_NAME_MAX, SYSCONF },
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    { "COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX, SYSCONF },
    { "POSIX2_COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX, SYSCONF },
#else
    { "COLL_WEIGHTS_MAX", _POSIX2_COLL_WEIGHTS_MAX, CONSTANT },
    { "POSIX2_COLL_WEIGHTS_MAX", _POSIX2_COLL_WEIGHTS_MAX, CONSTANT },
#endif
#ifdef _SC_EQUIV_CLASS_MAX
    { "EQUIV_CLASS_MAX", _SC_EQUIV_CLASS_MAX, SYSCONF },
#else
    { "EQUIV_CLASS_MAX", _POSIX2_EQUIV_CLASS_MAX, CONSTANT },
#endif
#ifdef _SC_EXPR_NEST_MAX
    { "EXPR_NEST_MAX", _SC_EXPR_NEST_MAX, SYSCONF },
    { "POSIX2_EXPR_NEST_MAX", _SC_EXPR_NEST_MAX, SYSCONF },
#else
    { "EXPR_NEST_MAX", _POSIX2_EXPR_NEST_MAX, CONSTANT },
    { "POSIX2_EXPR_NEST_MAX", _POSIX2_EXPR_NEST_MAX, CONSTANT },
#endif
#ifdef _SC_LINE_MAX
    { "LINE_MAX", _SC_LINE_MAX, SYSCONF },
    { "_POSIX2_LINE_MAX", _SC_LINE_MAX, SYSCONF },
    { "POSIX2_LINE_MAX", _SC_LINE_MAX, SYSCONF },
#else
    { "LINE_MAX", _POSIX2_LINE_MAX, CONSTANT },
    { "_POSIX2_LINE_MAX", _POSIX2_LINE_MAX, CONSTANT },
    { "POSIX2_LINE_MAX", _POSIX2_LINE_MAX, CONSTANT },
#endif
#ifdef _SC_RE_DUP_MAX
    { "POSIX2_RE_DUP_MAX", _SC_RE_DUP_MAX, SYSCONF },
    { "RE_DUP_MAX", _SC_RE_DUP_MAX, SYSCONF },
#else
    { "POSIX2_RE_DUP_MAX", _POSIX2_RE_DUP_MAX, CONSTANT },
    { "RE_DUP_MAX", _POSIX2_RE_DUP_MAX, CONSTANT },
#endif
    { "POSIX2_CHAR_TERM", _SC_2_CHAR_TERM, SYSCONF },
    { "POSIX2_C_BIND", _SC_2_C_BIND, SYSCONF },
    { "POSIX2_C_DEV", _SC_2_C_DEV, SYSCONF },
#ifdef _SC_2_C_VERSION
    { "POSIX2_C_VERSION", _SC_2_C_VERSION, SYSCONF },
#endif
    { "POSIX2_FORT_DEV", _SC_2_FORT_DEV, SYSCONF },
    { "POSIX2_FORT_RUN", _SC_2_FORT_RUN, SYSCONF },
    { "POSIX2_LOCALEDEF", _SC_2_LOCALEDEF, SYSCONF },
    { "POSIX2_SW_DEV", _SC_2_SW_DEV, SYSCONF },
    { "POSIX2_UPE", _SC_2_UPE, SYSCONF },
    { "POSIX2_VERSION", _SC_2_VERSION, SYSCONF },

    { "PATH", _CS_PATH, CONFSTR },
    { "CS_PATH", _CS_PATH, CONFSTR },

    /* LFS */
#ifdef _CS_LFS_CFLAGS
    { "LFS_CFLAGS", _CS_LFS_CFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS_LDFLAGS
    { "LFS_LDFLAGS", _CS_LFS_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS_LIBS
    { "LFS_LIBS", _CS_LFS_LIBS, CONFSTR },
#endif
#ifdef _CS_LFS_LINTFLAGS
    { "LFS_LINTFLAGS", _CS_LFS_LINTFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_CFLAGS
    { "LFS64_CFLAGS", _CS_LFS64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_LDFLAGS
    { "LFS64_LDFLAGS", _CS_LFS64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_LIBS
    { "LFS64_LIBS", _CS_LFS64_LIBS, CONFSTR },
#endif

#ifdef _CS_LFS64_LINTFLAGS
    { "LFS64_LINTFLAGS", _CS_LFS64_LINTFLAGS, CONFSTR },
#endif

    /* Programming environments.  */
#ifdef _CS_V5_WIDTH_RESTRICTED_ENVS
    { "_XBS5_WIDTH_RESTRICTED_ENVS", _CS_V5_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "XBS5_WIDTH_RESTRICTED_ENVS", _CS_V5_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_XBS5_ILP32_OFF32
    { "_XBS5_ILP32_OFF32", _SC_XBS5_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
    { "XBS5_ILP32_OFF32_CFLAGS", _CS_XBS5_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
    { "XBS5_ILP32_OFF32_LDFLAGS", _CS_XBS5_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
    { "XBS5_ILP32_OFF32_LIBS", _CS_XBS5_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
    { "XBS5_ILP32_OFF32_LINTFLAGS", _CS_XBS5_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_ILP32_OFFBIG
    { "_XBS5_ILP32_OFFBIG", _SC_XBS5_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
    { "XBS5_ILP32_OFFBIG_CFLAGS", _CS_XBS5_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
    { "XBS5_ILP32_OFFBIG_LDFLAGS", _CS_XBS5_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
    { "XBS5_ILP32_OFFBIG_LIBS", _CS_XBS5_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
    { "XBS5_ILP32_OFFBIG_LINTFLAGS", _CS_XBS5_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_LP64_OFF64
    { "_XBS5_LP64_OFF64", _SC_XBS5_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
    { "XBS5_LP64_OFF64_CFLAGS", _CS_XBS5_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
    { "XBS5_LP64_OFF64_LDFLAGS", _CS_XBS5_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
    { "XBS5_LP64_OFF64_LIBS", _CS_XBS5_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
    { "XBS5_LP64_OFF64_LINTFLAGS", _CS_XBS5_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_LPBIG_OFFBIG
    { "_XBS5_LPBIG_OFFBIG", _SC_XBS5_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
    { "XBS5_LPBIG_OFFBIG_CFLAGS", _CS_XBS5_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
    { "XBS5_LPBIG_OFFBIG_LDFLAGS", _CS_XBS5_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
    { "XBS5_LPBIG_OFFBIG_LIBS", _CS_XBS5_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
    { "XBS5_LPBIG_OFFBIG_LINTFLAGS", _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_ILP32_OFF32
    { "_POSIX_V6_ILP32_OFF32", _SC_V6_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_CFLAGS
    { "POSIX_V6_ILP32_OFF32_CFLAGS", _CS_POSIX_V6_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LDFLAGS
    { "POSIX_V6_ILP32_OFF32_LDFLAGS", _CS_POSIX_V6_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LIBS
    { "POSIX_V6_ILP32_OFF32_LIBS", _CS_POSIX_V6_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS
    { "POSIX_V6_ILP32_OFF32_LINTFLAGS", _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _CS_V6_WIDTH_RESTRICTED_ENVS
    { "_POSIX_V6_WIDTH_RESTRICTED_ENVS", _CS_V6_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "POSIX_V6_WIDTH_RESTRICTED_ENVS", _CS_V6_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_V6_ILP32_OFFBIG
    { "_POSIX_V6_ILP32_OFFBIG", _SC_V6_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS
    { "POSIX_V6_ILP32_OFFBIG_CFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LDFLAGS
    { "POSIX_V6_ILP32_OFFBIG_LDFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LIBS
    { "POSIX_V6_ILP32_OFFBIG_LIBS", _CS_POSIX_V6_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS
    { "POSIX_V6_ILP32_OFFBIG_LINTFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_LP64_OFF64
    { "_POSIX_V6_LP64_OFF64", _SC_V6_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_CFLAGS
    { "POSIX_V6_LP64_OFF64_CFLAGS", _CS_POSIX_V6_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LDFLAGS
    { "POSIX_V6_LP64_OFF64_LDFLAGS", _CS_POSIX_V6_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LIBS
    { "POSIX_V6_LP64_OFF64_LIBS", _CS_POSIX_V6_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LINTFLAGS
    { "POSIX_V6_LP64_OFF64_LINTFLAGS", _CS_POSIX_V6_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_LPBIG_OFFBIG
    { "_POSIX_V6_LPBIG_OFFBIG", _SC_V6_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_CFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_LDFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LIBS
    { "POSIX_V6_LPBIG_OFFBIG_LIBS", _CS_POSIX_V6_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_LINTFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_ILP32_OFF32
    { "_POSIX_V7_ILP32_OFF32", _SC_V7_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_CFLAGS
    { "POSIX_V7_ILP32_OFF32_CFLAGS", _CS_POSIX_V7_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LDFLAGS
    { "POSIX_V7_ILP32_OFF32_LDFLAGS", _CS_POSIX_V7_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LIBS
    { "POSIX_V7_ILP32_OFF32_LIBS", _CS_POSIX_V7_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS
    { "POSIX_V7_ILP32_OFF32_LINTFLAGS", _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _CS_V7_WIDTH_RESTRICTED_ENVS
    { "_POSIX_V7_WIDTH_RESTRICTED_ENVS", _CS_V7_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "POSIX_V7_WIDTH_RESTRICTED_ENVS", _CS_V7_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_V7_ILP32_OFFBIG
    { "_POSIX_V7_ILP32_OFFBIG", _SC_V7_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS
    { "POSIX_V7_ILP32_OFFBIG_CFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS
    { "POSIX_V7_ILP32_OFFBIG_LDFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LIBS
    { "POSIX_V7_ILP32_OFFBIG_LIBS", _CS_POSIX_V7_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS
    { "POSIX_V7_ILP32_OFFBIG_LINTFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_LP64_OFF64
    { "_POSIX_V7_LP64_OFF64", _SC_V7_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_CFLAGS
    { "POSIX_V7_LP64_OFF64_CFLAGS", _CS_POSIX_V7_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LDFLAGS
    { "POSIX_V7_LP64_OFF64_LDFLAGS", _CS_POSIX_V7_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LIBS
    { "POSIX_V7_LP64_OFF64_LIBS", _CS_POSIX_V7_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LINTFLAGS
    { "POSIX_V7_LP64_OFF64_LINTFLAGS", _CS_POSIX_V7_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_LPBIG_OFFBIG
    { "_POSIX_V7_LPBIG_OFFBIG", _SC_V7_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_CFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_LDFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LIBS
    { "POSIX_V7_LPBIG_OFFBIG_LIBS", _CS_POSIX_V7_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_LINTFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_ADVISORY_INFO
    { "_POSIX_ADVISORY_INFO", _SC_ADVISORY_INFO, SYSCONF },
#endif
#ifdef _SC_BARRIERS
    { "_POSIX_BARRIERS", _SC_BARRIERS, SYSCONF },
#endif
#ifdef _SC_BASE
    { "_POSIX_BASE", _SC_BASE, SYSCONF },
#endif
#ifdef _SC_C_LANG_SUPPORT
    { "_POSIX_C_LANG_SUPPORT", _SC_C_LANG_SUPPORT, SYSCONF },
#endif
#ifdef _SC_C_LANG_SUPPORT_R
    { "_POSIX_C_LANG_SUPPORT_R", _SC_C_LANG_SUPPORT_R, SYSCONF },
#endif
    { "_POSIX_CLOCK_SELECTION", _SC_CLOCK_SELECTION, SYSCONF },
    { "_POSIX_CPUTIME", _SC_CPUTIME, SYSCONF },
    { "_POSIX_THREAD_CPUTIME", _SC_THREAD_CPUTIME, SYSCONF },
#ifdef _SC_DEVICE_SPECIFIC
    { "_POSIX_DEVICE_SPECIFIC", _SC_DEVICE_SPECIFIC, SYSCONF },
#endif
#ifdef _SC_DEVICE_SPECIFIC_R
    { "_POSIX_DEVICE_SPECIFIC_R", _SC_DEVICE_SPECIFIC_R, SYSCONF },
#endif
#ifdef _SC_FD_MGMT
    { "_POSIX_FD_MGMT", _SC_FD_MGMT, SYSCONF },
#endif
#ifdef _SC_FIFO
    { "_POSIX_FIFO", _SC_FIFO, SYSCONF },
#endif
#ifdef _SC_PIPE
    { "_POSIX_PIPE", _SC_PIPE, SYSCONF },
#endif
#ifdef _SC_FILE_ATTRIBUTES
    { "_POSIX_FILE_ATTRIBUTES", _SC_FILE_ATTRIBUTES, SYSCONF },
#endif
#ifdef _SC_FILE_LOCKING
    { "_POSIX_FILE_LOCKING", _SC_FILE_LOCKING, SYSCONF },
#endif
#ifdef _SC_FILE_SYSTEM
    { "_POSIX_FILE_SYSTEM", _SC_FILE_SYSTEM, SYSCONF },
#endif
    { "_POSIX_MONOTONIC_CLOCK", _SC_MONOTONIC_CLOCK, SYSCONF },
#ifdef _SC_MULTI_PROCESS
    { "_POSIX_MULTI_PROCESS", _SC_MULTI_PROCESS, SYSCONF },
#endif
#ifdef _SC_SINGLE_PROCESS
    { "_POSIX_SINGLE_PROCESS", _SC_SINGLE_PROCESS, SYSCONF },
#endif
#ifdef _SC_NETWORKING
    { "_POSIX_NETWORKING", _SC_NETWORKING, SYSCONF },
#endif
    { "_POSIX_READER_WRITER_LOCKS", _SC_READER_WRITER_LOCKS, SYSCONF },
    { "_POSIX_SPIN_LOCKS", _SC_SPIN_LOCKS, SYSCONF },
    { "_POSIX_REGEXP", _SC_REGEXP, SYSCONF },
#ifdef _SC_REGEX_VERSION
    { "_REGEX_VERSION", _SC_REGEX_VERSION, SYSCONF },
#endif
    { "_POSIX_SHELL", _SC_SHELL, SYSCONF },
#ifdef _SC_SIGNALS
    { "_POSIX_SIGNALS", _SC_SIGNALS, SYSCONF },
#endif
    { "_POSIX_SPAWN", _SC_SPAWN, SYSCONF },
#ifdef _SC_SPORADIC_SERVER
    { "_POSIX_SPORADIC_SERVER", _SC_SPORADIC_SERVER, SYSCONF },
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
    { "_POSIX_THREAD_SPORADIC_SERVER", _SC_THREAD_SPORADIC_SERVER, SYSCONF },
#endif
#ifdef _SC_SYSTEM_DATABASE
    { "_POSIX_SYSTEM_DATABASE", _SC_SYSTEM_DATABASE, SYSCONF },
#endif
#ifdef _SC_SYSTEM_DATABASE_R
    { "_POSIX_SYSTEM_DATABASE_R", _SC_SYSTEM_DATABASE_R, SYSCONF },
#endif
#ifdef _SC_TIMEOUTS
    { "_POSIX_TIMEOUTS", _SC_TIMEOUTS, SYSCONF },
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
    { "_POSIX_TYPED_MEMORY_OBJECTS", _SC_TYPED_MEMORY_OBJECTS, SYSCONF },
#endif
#ifdef _SC_USER_GROUPS
    { "_POSIX_USER_GROUPS", _SC_USER_GROUPS, SYSCONF },
#endif
#ifdef _SC_USER_GROUPS_R
    { "_POSIX_USER_GROUPS_R", _SC_USER_GROUPS_R, SYSCONF },
#endif
    { "POSIX2_PBS", _SC_2_PBS, SYSCONF },
    { "POSIX2_PBS_ACCOUNTING", _SC_2_PBS_ACCOUNTING, SYSCONF },
    { "POSIX2_PBS_LOCATE", _SC_2_PBS_LOCATE, SYSCONF },
    { "POSIX2_PBS_TRACK", _SC_2_PBS_TRACK, SYSCONF },
    { "POSIX2_PBS_MESSAGE", _SC_2_PBS_MESSAGE, SYSCONF },
    { "SYMLOOP_MAX", _SC_SYMLOOP_MAX, SYSCONF },
    { "STREAM_MAX", _SC_STREAM_MAX, SYSCONF },
    { "AIO_LISTIO_MAX", _SC_AIO_LISTIO_MAX, SYSCONF },
    { "AIO_MAX", _SC_AIO_MAX, SYSCONF },
#ifdef _SC_AIO_PRIO_DELTA_MAX
    { "AIO_PRIO_DELTA_MAX", _SC_AIO_PRIO_DELTA_MAX, SYSCONF },
#endif
    { "DELAYTIMER_MAX", _SC_DELAYTIMER_MAX, SYSCONF },
    { "HOST_NAME_MAX", _SC_HOST_NAME_MAX, SYSCONF },
    { "LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX, SYSCONF },
    { "MQ_OPEN_MAX", _SC_MQ_OPEN_MAX, SYSCONF },
    { "MQ_PRIO_MAX", _SC_MQ_PRIO_MAX, SYSCONF },
#ifdef _SC_DEVICE_IO
    { "_POSIX_DEVICE_IO", _SC_DEVICE_IO, SYSCONF },
#endif
#ifdef _SC_TRACE
    { "_POSIX_TRACE", _SC_TRACE, SYSCONF },
#endif
#ifdef _SC_TRACE_EVENT_FILTER
    { "_POSIX_TRACE_EVENT_FILTER", _SC_TRACE_EVENT_FILTER, SYSCONF },
#endif
#ifdef _SC_TRACE_INHERIT
    { "_POSIX_TRACE_INHERIT", _SC_TRACE_INHERIT, SYSCONF },
#endif
#ifdef _SC_TRACE_LOG
    { "_POSIX_TRACE_LOG", _SC_TRACE_LOG, SYSCONF },
#endif
    { "RTSIG_MAX", _SC_RTSIG_MAX, SYSCONF },
#ifdef _SC_SEM_NSEMS_MAX
    { "SEM_NSEMS_MAX", _SC_SEM_NSEMS_MAX, SYSCONF },
#endif
#ifdef _SC_SEM_VALUE_MAX
    { "SEM_VALUE_MAX", _SC_SEM_VALUE_MAX, SYSCONF },
#endif
    { "SIGQUEUE_MAX", _SC_SIGQUEUE_MAX, SYSCONF },
    { "FILESIZEBITS", _PC_FILESIZEBITS, PATHCONF },
#ifdef _PC_ALLOC_SIZE_MIN
    { "POSIX_ALLOC_SIZE_MIN", _PC_ALLOC_SIZE_MIN, PATHCONF },
#endif
#ifdef _PC_REC_INCR_XFER_SIZE
    { "POSIX_REC_INCR_XFER_SIZE", _PC_REC_INCR_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_MAX_XFER_SIZE
    { "POSIX_REC_MAX_XFER_SIZE", _PC_REC_MAX_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_MIN_XFER_SIZE
    { "POSIX_REC_MIN_XFER_SIZE", _PC_REC_MIN_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_XFER_ALIGN
    { "POSIX_REC_XFER_ALIGN", _PC_REC_XFER_ALIGN, PATHCONF },
#endif
    { "SYMLINK_MAX", _PC_SYMLINK_MAX, PATHCONF },
#ifdef _PC_2_SYMLINKS
    { "POSIX2_SYMLINKS", _PC_2_SYMLINKS, PATHCONF },
#endif

#ifdef _SC_LEVEL1_ICACHE_SIZE
    { "LEVEL1_ICACHE_SIZE", _SC_LEVEL1_ICACHE_SIZE, SYSCONF },
    { "LEVEL1_ICACHE_ASSOC", _SC_LEVEL1_ICACHE_ASSOC, SYSCONF },
    { "LEVEL1_ICACHE_LINESIZE", _SC_LEVEL1_ICACHE_LINESIZE, SYSCONF },
    { "LEVEL1_DCACHE_SIZE", _SC_LEVEL1_DCACHE_SIZE, SYSCONF },
    { "LEVEL1_DCACHE_ASSOC", _SC_LEVEL1_DCACHE_ASSOC, SYSCONF },
    { "LEVEL1_DCACHE_LINESIZE", _SC_LEVEL1_DCACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
    { "LEVEL2_CACHE_SIZE", _SC_LEVEL2_CACHE_SIZE, SYSCONF },
    { "LEVEL2_CACHE_ASSOC", _SC_LEVEL2_CACHE_ASSOC, SYSCONF },
    { "LEVEL2_CACHE_LINESIZE", _SC_LEVEL2_CACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL3_CACHE_SIZE
    { "LEVEL3_CACHE_SIZE", _SC_LEVEL3_CACHE_SIZE, SYSCONF },
    { "LEVEL3_CACHE_ASSOC", _SC_LEVEL3_CACHE_ASSOC, SYSCONF },
    { "LEVEL3_CACHE_LINESIZE", _SC_LEVEL3_CACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL4_CACHE_SIZE
    { "LEVEL4_CACHE_SIZE", _SC_LEVEL4_CACHE_SIZE, SYSCONF },
    { "LEVEL4_CACHE_ASSOC", _SC_LEVEL4_CACHE_ASSOC, SYSCONF },
    { "LEVEL4_CACHE_LINESIZE", _SC_LEVEL4_CACHE_LINESIZE, SYSCONF },
#endif

#ifdef _SC_IPV6
    { "IPV6", _SC_IPV6, SYSCONF },
    { "_POSIX_IPV6", _SC_IPV6, SYSCONF },
#endif
#ifdef _SC_RAW_SOCKETS
    { "RAW_SOCKETS", _SC_RAW_SOCKETS, SYSCONF },
    { "_POSIX_RAW_SOCKETS", _SC_RAW_SOCKETS, SYSCONF },
#endif

    { NULL, 0, SYSCONF }
  };

static int getconf_print (const struct conf *, const char *, int);
static int getconf_all (void);
static int getconf_one (WORD_LIST *);
static int getconf_internal (const struct conf *, int);

static int
getconf_internal (const struct conf *c, int all)
{
  long l, val;
  intmax_t v;
  uintmax_t uv;
  unsigned long ul;
#if HAVE_LONG_LONG_INT
  unsigned long long ull;
  long long ll;
#endif
  int r;

  val = c->call_name;
  r = EXECUTION_SUCCESS;
  if (val == VAL_ULONG_MAX)
    {
      ul = ULONG_MAX;
      printf ("%lu", ul);
    }
#if HAVE_LONG_LONG_INT
  else if (val == VAL_ULLONG_MAX)
    {
      ull = ULLONG_MAX;
      printf ("%llu", ull);
    }
  else if (val == VAL_LLONG_MIN || val == VAL_LLONG_MAX)
    {
      ll = (val == VAL_LLONG_MIN) ? LLONG_MIN : LLONG_MAX;
      printf ("%lld", ll);
    }
#endif
#if HAVE_LONG_LONG_INT
  else if (val == VAL_SSIZE_MAX)
    {
      ll = SSIZE_MAX;
      printf ("%lld", ll);
    }
  else if (val == VAL_SIZE_MAX)
    {
      ul = SIZE_MAX;
      printf ("%lu", ul);
    }
#else
  else if (val == VAL_SSIZE_MAX)
    {
      v = SSIZE_MAX;
      printf ("%jd", v);
    }
  else if (val == VAL_SIZE_MAX)
    {
      uv = SIZE_MAX;
      printf ("%ju", uv);
    }
#endif
  else
    printf ("%ld", val);

  if (r == EXECUTION_SUCCESS)
    printf ("\n");

  return (r);
}
    
static int
getconf_all (void)
{
  const struct conf *c;
  char *path;
  int r;

  r = EXECUTION_SUCCESS;
  for (c = vars; c->name != NULL; ++c)
    {
      printf("%-35s", c->name);
      path = "/";		/* XXX for now */
      if (getconf_print (c, path, 1) == EXECUTION_FAILURE)
        r = EXECUTION_FAILURE;
    }
  return (r);
}

static int
getconf_one (WORD_LIST *list)
{
  const struct conf *c;
  char *vname, *vpath;

  vname = list->word->word;
  vpath = (list->next && list->next->word) ? list->next->word->word : 0;

  for (c = vars; c->name != NULL; ++c)
    {
      if (strcmp (c->name, vname) == 0 || (strncmp (c->name, "_POSIX_", 7) == 0 &&
					  strcmp (c->name + 7, vname) == 0))
 	break;
    }
  if (c->name == NULL)
    {
      builtin_error ("%s: unknown variable", vname);
      return (EXECUTION_FAILURE);
    }

  if (c->call_name == PATHCONF && list->next == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }
  else if (c->call_name != PATHCONF && list->next)
    {
      builtin_usage ();
      return (EX_USAGE);
    }
     
  return (getconf_print (c, vpath, 0));
}

static int
getconf_print (const struct conf *c, const char *vpath, int all)
{
  long value;
  size_t clen;
  char *cvalue;
  int cn;

  cn = c->call_name;
  switch (c->call)
    {
    case PATHCONF:
      errno = 0;
      value = pathconf (vpath, cn);
      if (value == -1L)
	{
	  if (errno)
	    builtin_error ("pathconf %s: %s: %s", c->name, vpath, strerror (errno));
	  printf ("undefined\n");
	  return (EXECUTION_FAILURE);
	}
      else
        printf ("%ld\n", value);
      return (EXECUTION_SUCCESS);

    case SYSCONF:
      errno = 0;
      value = sysconf (cn);
      if (value == -1L)
	{
	  if (errno)
	    builtin_error ("%s: %s", c->name, strerror (errno));
	  printf ("undefined\n");
	  return (EXECUTION_FAILURE);
	}
      else
	printf ("%ld\n", value);
      return (EXECUTION_SUCCESS);

    case CONFSTR:
      errno = 0;
      clen = confstr (cn, (char *) NULL, 0);
      cvalue = (char *) malloc (clen);
      if (cvalue == NULL)
        {
          builtin_error ("memory allocation failure");
          return (EXECUTION_FAILURE);
        }
      if (confstr (c->call_name, cvalue, clen) != clen)
	{
	  if (errno != 0)
	    builtin_error ("%s: confstr: %s", c->name, strerror (errno));
	  printf ("undefined\n");
	  return (EXECUTION_FAILURE);
	}
      else
	printf ("%.*s\n", (int) clen, cvalue);
      free (cvalue);
      return (EXECUTION_SUCCESS);

    case CONSTANT:
      return (getconf_internal (c, all));

    case UNDEFINED:
      builtin_error ("%s: undefined", c->name);
      return (EXECUTION_FAILURE);
    }   

  /* NOTREACHED */
  return (EX_USAGE);
}

int
getconf_builtin (WORD_LIST *list)
{
  const struct conf *c;
  int r, opt, aflag, vflag;
  char *varg;

  aflag = vflag = 0;
  varg = 0;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "ahv:")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  aflag = 1;
	  break;
	CASE_HELPOPT;
	case 'h':
	  builtin_help ();
	  return (EX_USAGE);
	case 'v':
	  return (EX_DISKFALLBACK);
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }

  list = loptend;
  if ((aflag == 0 && list == 0) || (aflag && list) || list_length(list) > 2)
    {
      builtin_usage();
      return (EX_USAGE);
    }

  r = aflag ? getconf_all () : getconf_one (list);
  return r;
}

static char *getconf_doc[] = {
	"Display values of system limits and options.",
	"",
	"getconf writes the current value of a configurable system limit or",
	"option variable to the standard output.",
	(char *)NULL
};

struct builtin getconf_struct = {
	"getconf",
	getconf_builtin,
	BUILTIN_ENABLED,
	getconf_doc,
	"getconf -[ah] or getconf [-v spec] sysvar or getconf [-v spec] pathvar pathname",
	0
};
