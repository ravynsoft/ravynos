/* strerror_r.c --- POSIX compatible system error routine

   Copyright (C) 2010-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2010.  */

#include <config.h>

/* Enable declaration of sys_nerr and sys_errlist in <errno.h> on NetBSD.  */
#define _NETBSD_SOURCE 1

/* Specification.  */
#include <string.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#if !HAVE_SNPRINTF
# include <stdarg.h>
#endif

#include "strerror-override.h"

#if STRERROR_R_CHAR_P

# if HAVE___XPG_STRERROR_R
_GL_EXTERN_C int __xpg_strerror_r (int errnum, char *buf, size_t buflen);
# endif

#elif HAVE_DECL_STRERROR_R

/* The system's strerror_r function's API is OK, except that its third argument
   is 'int', not 'size_t', or its return type is wrong.  */

# include <limits.h>

#else

/* Use the system's strerror().  Exclude glibc and cygwin because the
   system strerror_r has the wrong return type, and cygwin 1.7.9
   strerror_r clobbers strerror.  */
# undef strerror

# if defined __NetBSD__ || defined __hpux || (defined _WIN32 && !defined __CYGWIN__) || defined __sgi || (defined __sun && !defined _LP64) || defined __CYGWIN__

/* No locking needed.  */

/* Get catgets internationalization functions.  */
#  if HAVE_CATGETS
#   include <nl_types.h>
#  endif

#ifdef __cplusplus
extern "C" {
#endif

/* Get sys_nerr, sys_errlist on HP-UX (otherwise only declared in C++ mode).
   Get sys_nerr, sys_errlist on IRIX (otherwise only declared with _SGIAPI).  */
#  if defined __hpux || defined __sgi
extern int sys_nerr;
extern char *sys_errlist[];
#  endif

/* Get sys_nerr on Solaris.  */
#  if defined __sun && !defined _LP64
extern int sys_nerr;
#  endif

#ifdef __cplusplus
}
#endif

# else

#  include "glthread/lock.h"

/* This lock protects the buffer returned by strerror().  We assume that
   no other uses of strerror() exist in the program.  */
gl_lock_define_initialized(static, strerror_lock)

# endif

#endif

/* On MSVC, there is no snprintf() function, just a _snprintf().
   It is of lower quality, but sufficient for the simple use here.
   We only have to make sure to NUL terminate the result (_snprintf
   does not NUL terminate, like strncpy).  */
#if !HAVE_SNPRINTF
static int
local_snprintf (char *buf, size_t buflen, const char *format, ...)
{
  va_list args;
  int result;

  va_start (args, format);
  result = _vsnprintf (buf, buflen, format, args);
  va_end (args);
  if (buflen > 0 && (result < 0 || result >= buflen))
    buf[buflen - 1] = '\0';
  return result;
}
# undef snprintf
# define snprintf local_snprintf
#endif

/* Copy as much of MSG into BUF as possible, without corrupting errno.
   Return 0 if MSG fit in BUFLEN, otherwise return ERANGE.  */
static int
safe_copy (char *buf, size_t buflen, const char *msg)
{
  size_t len = strlen (msg);
  size_t moved = len < buflen ? len : buflen - 1;

  /* Although POSIX lets memmove corrupt errno, we don't
     know of any implementation where this is a real problem.  */
  memmove (buf, msg, moved);
  buf[moved] = '\0';
  return len < buflen ? 0 : ERANGE;
}


int
strerror_r (int errnum, char *buf, size_t buflen)
#undef strerror_r
{
  /* Filter this out now, so that rest of this replacement knows that
     there is room for a non-empty message and trailing NUL.  */
  if (buflen <= 1)
    {
      if (buflen)
        *buf = '\0';
      return ERANGE;
    }
  *buf = '\0';

  /* Check for gnulib overrides.  */
  {
    char const *msg = strerror_override (errnum);

    if (msg)
      return safe_copy (buf, buflen, msg);
  }

  {
    int ret;
    int saved_errno = errno;

#if STRERROR_R_CHAR_P

    {
      ret = 0;

# if HAVE___XPG_STRERROR_R
      ret = __xpg_strerror_r (errnum, buf, buflen);
      /* ret is 0 upon success, or EINVAL or ERANGE upon failure.  */
# endif

      if (!*buf)
        {
          /* glibc 2.13 ... 2.34 (at least) don't touch buf upon failure.
             Therefore we have to fall back to strerror_r which, for valid
             errnum, returns a thread-safe untruncated string.  For invalid
             errnum, though, it returns a truncated string, which does not
             allow us to determine whether to return ERANGE or 0.  Thus we
             need to pass a sufficiently large buffer.  */
          char stackbuf[80];
          char *errstring = strerror_r (errnum, stackbuf, sizeof stackbuf);
          ret = errstring ? safe_copy (buf, buflen, errstring) : errno;
        }
    }

#elif HAVE_DECL_STRERROR_R

    if (buflen > INT_MAX)
      buflen = INT_MAX;

# ifdef __hpux
    /* On HP-UX 11.31, strerror_r always fails when buflen < 80; it
       also fails to change buf on EINVAL.  */
    {
      char stackbuf[80];

      if (buflen < sizeof stackbuf)
        {
          ret = strerror_r (errnum, stackbuf, sizeof stackbuf);
          if (ret == 0)
            ret = safe_copy (buf, buflen, stackbuf);
        }
      else
        ret = strerror_r (errnum, buf, buflen);
    }
# else
    ret = strerror_r (errnum, buf, buflen);

    /* Some old implementations may return (-1, EINVAL) instead of EINVAL.
       But on Haiku, valid error numbers are negative.  */
#  if !defined __HAIKU__
    if (ret < 0)
      ret = errno;
#  endif
# endif

# if defined _AIX || defined __HAIKU__
    /* AIX and Haiku return 0 rather than ERANGE when truncating strings; try
       again until we are sure we got the entire string.  */
    if (!ret && strlen (buf) == buflen - 1)
      {
        char stackbuf[STACKBUF_LEN];
        size_t len;
        strerror_r (errnum, stackbuf, sizeof stackbuf);
        len = strlen (stackbuf);
        /* STACKBUF_LEN should have been large enough.  */
        if (len + 1 == sizeof stackbuf)
          abort ();
        if (buflen <= len)
          ret = ERANGE;
      }
# else
    /* Solaris 10 does not populate buf on ERANGE.  OpenBSD 4.7
       truncates early on ERANGE rather than return a partial integer.
       We prefer the maximal string.  We set buf[0] earlier, and we
       know of no implementation that modifies buf to be an
       unterminated string, so this strlen should be portable in
       practice (rather than pulling in a safer strnlen).  */
    if (ret == ERANGE && strlen (buf) < buflen - 1)
      {
        char stackbuf[STACKBUF_LEN];

        /* STACKBUF_LEN should have been large enough.  */
        if (strerror_r (errnum, stackbuf, sizeof stackbuf) == ERANGE)
          abort ();
        safe_copy (buf, buflen, stackbuf);
      }
# endif

#else /* strerror_r is not declared.  */

    /* Try to do what strerror (errnum) does, but without clobbering the
       buffer used by strerror().  */

# if defined __NetBSD__ || defined __hpux || (defined _WIN32 && !defined __CYGWIN__) || defined __CYGWIN__ /* NetBSD, HP-UX, native Windows, Cygwin */

    /* NetBSD:         sys_nerr, sys_errlist are declared through _NETBSD_SOURCE
                       and <errno.h> above.
       HP-UX:          sys_nerr, sys_errlist are declared explicitly above.
       native Windows: sys_nerr, sys_errlist are declared in <stdlib.h>.
       Cygwin:         sys_nerr, sys_errlist are declared in <errno.h>.  */
    if (errnum >= 0 && errnum < sys_nerr)
      {
#  if HAVE_CATGETS && (defined __NetBSD__ || defined __hpux)
#   if defined __NetBSD__
        nl_catd catd = catopen ("libc", NL_CAT_LOCALE);
        const char *errmsg =
          (catd != (nl_catd)-1
           ? catgets (catd, 1, errnum, sys_errlist[errnum])
           : sys_errlist[errnum]);
#   endif
#   if defined __hpux
        nl_catd catd = catopen ("perror", NL_CAT_LOCALE);
        const char *errmsg =
          (catd != (nl_catd)-1
           ? catgets (catd, 1, 1 + errnum, sys_errlist[errnum])
           : sys_errlist[errnum]);
#   endif
#  else
        const char *errmsg = sys_errlist[errnum];
#  endif
        if (errmsg == NULL || *errmsg == '\0')
          ret = EINVAL;
        else
          ret = safe_copy (buf, buflen, errmsg);
#  if HAVE_CATGETS && (defined __NetBSD__ || defined __hpux)
        if (catd != (nl_catd)-1)
          catclose (catd);
#  endif
      }
    else
      ret = EINVAL;

# elif defined __sgi || (defined __sun && !defined _LP64) /* IRIX, Solaris <= 9 32-bit */

    /* For a valid error number, the system's strerror() function returns
       a pointer to a not copied string, not to a buffer.  */
    if (errnum >= 0 && errnum < sys_nerr)
      {
        char *errmsg = strerror (errnum);

        if (errmsg == NULL || *errmsg == '\0')
          ret = EINVAL;
        else
          ret = safe_copy (buf, buflen, errmsg);
      }
    else
      ret = EINVAL;

# else

    gl_lock_lock (strerror_lock);

    {
      char *errmsg = strerror (errnum);

      /* For invalid error numbers, strerror() on
           - IRIX 6.5 returns NULL,
           - HP-UX 11 returns an empty string.  */
      if (errmsg == NULL || *errmsg == '\0')
        ret = EINVAL;
      else
        ret = safe_copy (buf, buflen, errmsg);
    }

    gl_lock_unlock (strerror_lock);

# endif

#endif

#if defined _WIN32 && !defined __CYGWIN__
    /* MSVC 14 defines names for many error codes in the range 100..140,
       but _sys_errlist contains strings only for the error codes
       < _sys_nerr = 43.  */
    if (ret == EINVAL)
      {
        const char *errmsg;

        switch (errnum)
          {
          case 100 /* EADDRINUSE */:
            errmsg = "Address already in use";
            break;
          case 101 /* EADDRNOTAVAIL */:
            errmsg = "Cannot assign requested address";
            break;
          case 102 /* EAFNOSUPPORT */:
            errmsg = "Address family not supported by protocol";
            break;
          case 103 /* EALREADY */:
            errmsg = "Operation already in progress";
            break;
          case 105 /* ECANCELED */:
            errmsg = "Operation canceled";
            break;
          case 106 /* ECONNABORTED */:
            errmsg = "Software caused connection abort";
            break;
          case 107 /* ECONNREFUSED */:
            errmsg = "Connection refused";
            break;
          case 108 /* ECONNRESET */:
            errmsg = "Connection reset by peer";
            break;
          case 109 /* EDESTADDRREQ */:
            errmsg = "Destination address required";
            break;
          case 110 /* EHOSTUNREACH */:
            errmsg = "No route to host";
            break;
          case 112 /* EINPROGRESS */:
            errmsg = "Operation now in progress";
            break;
          case 113 /* EISCONN */:
            errmsg = "Transport endpoint is already connected";
            break;
          case 114 /* ELOOP */:
            errmsg = "Too many levels of symbolic links";
            break;
          case 115 /* EMSGSIZE */:
            errmsg = "Message too long";
            break;
          case 116 /* ENETDOWN */:
            errmsg = "Network is down";
            break;
          case 117 /* ENETRESET */:
            errmsg = "Network dropped connection on reset";
            break;
          case 118 /* ENETUNREACH */:
            errmsg = "Network is unreachable";
            break;
          case 119 /* ENOBUFS */:
            errmsg = "No buffer space available";
            break;
          case 123 /* ENOPROTOOPT */:
            errmsg = "Protocol not available";
            break;
          case 126 /* ENOTCONN */:
            errmsg = "Transport endpoint is not connected";
            break;
          case 128 /* ENOTSOCK */:
            errmsg = "Socket operation on non-socket";
            break;
          case 129 /* ENOTSUP */:
            errmsg = "Not supported";
            break;
          case 130 /* EOPNOTSUPP */:
            errmsg = "Operation not supported";
            break;
          case 132 /* EOVERFLOW */:
            errmsg = "Value too large for defined data type";
            break;
          case 133 /* EOWNERDEAD */:
            errmsg = "Owner died";
            break;
          case 134 /* EPROTO */:
            errmsg = "Protocol error";
            break;
          case 135 /* EPROTONOSUPPORT */:
            errmsg = "Protocol not supported";
            break;
          case 136 /* EPROTOTYPE */:
            errmsg = "Protocol wrong type for socket";
            break;
          case 138 /* ETIMEDOUT */:
            errmsg = "Connection timed out";
            break;
          case 140 /* EWOULDBLOCK */:
            errmsg = "Operation would block";
            break;
          default:
            errmsg = NULL;
            break;
          }
        if (errmsg != NULL)
          ret = safe_copy (buf, buflen, errmsg);
      }
#endif

    if (ret == EINVAL && !*buf)
      {
#if defined __HAIKU__
        /* For consistency with perror().  */
        snprintf (buf, buflen, "Unknown Application Error (%d)", errnum);
#else
        snprintf (buf, buflen, "Unknown error %d", errnum);
#endif
      }

    errno = saved_errno;
    return ret;
  }
}
