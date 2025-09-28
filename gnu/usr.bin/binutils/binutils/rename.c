/* rename.c -- rename a file, preserving symlinks.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bucomm.h"

#if defined HAVE_UTIMES
#include <sys/time.h>
#elif defined HAVE_GOOD_UTIME_H
#include <utime.h>
#endif

/* The number of bytes to copy at once.  */
#define COPY_BUF 8192

/* Copy file FROMFD to file TO, performing no translations.
   Return 0 if ok, -1 if error.  */

static int
simple_copy (int fromfd, const char *to,
	     struct stat *target_stat ATTRIBUTE_UNUSED)
{
  int tofd, nread;
  int saved;
  char buf[COPY_BUF];

  if (fromfd < 0
      || lseek (fromfd, 0, SEEK_SET) != 0)
    return -1;

  tofd = open (to, O_WRONLY | O_TRUNC | O_BINARY);
  if (tofd < 0)
    {
      saved = errno;
      close (fromfd);
      errno = saved;
      return -1;
    }

  while ((nread = read (fromfd, buf, sizeof buf)) > 0)
    {
      if (write (tofd, buf, nread) != nread)
	{
	  saved = errno;
	  close (fromfd);
	  close (tofd);
	  errno = saved;
	  return -1;
	}
    }

  saved = errno;

#if !defined (_WIN32) || defined (__CYGWIN32__)
  /* Writing to a setuid/setgid file may clear S_ISUID and S_ISGID.
     Try to restore them, ignoring failure.  */
  if (target_stat != NULL)
    fchmod (tofd, target_stat->st_mode);
#endif

  close (fromfd);
  close (tofd);
  if (nread < 0)
    {
      errno = saved;
      return -1;
    }
  return 0;
}

/* The following defines and inline functions are copied from gnulib.
   FIXME: Use a gnulib import and stat-time.h instead.  */
#if defined HAVE_STRUCT_STAT_ST_ATIM_TV_NSEC
# if defined TYPEOF_STRUCT_STAT_ST_ATIM_IS_STRUCT_TIMESPEC
#  define STAT_TIMESPEC(st, st_xtim) ((st)->st_xtim)
# else
#  define STAT_TIMESPEC_NS(st, st_xtim) ((st)->st_xtim.tv_nsec)
# endif
#elif defined HAVE_STRUCT_STAT_ST_ATIMESPEC_TV_NSEC
# define STAT_TIMESPEC(st, st_xtim) ((st)->st_xtim##espec)
#elif defined HAVE_STRUCT_STAT_ST_ATIMENSEC
# define STAT_TIMESPEC_NS(st, st_xtim) ((st)->st_xtim##ensec)
#elif defined HAVE_STRUCT_STAT_ST_ATIM_ST__TIM_TV_NSEC
# define STAT_TIMESPEC_NS(st, st_xtim) ((st)->st_xtim.st__tim.tv_nsec)
#endif

static inline long int get_stat_atime_ns (struct stat const *) ATTRIBUTE_UNUSED;
static inline long int get_stat_mtime_ns (struct stat const *) ATTRIBUTE_UNUSED;

/* Return the nanosecond component of *ST's access time.  */
static inline long int
get_stat_atime_ns (struct stat const *st ATTRIBUTE_UNUSED)
{
# if defined STAT_TIMESPEC
  return STAT_TIMESPEC (st, st_atim).tv_nsec;
# elif defined STAT_TIMESPEC_NS
  return STAT_TIMESPEC_NS (st, st_atim);
# else
  return 0;
# endif
}

/* Return the nanosecond component of *ST's data modification time.  */
static inline long int
get_stat_mtime_ns (struct stat const *st ATTRIBUTE_UNUSED)
{
# if defined STAT_TIMESPEC
  return STAT_TIMESPEC (st, st_mtim).tv_nsec;
# elif defined STAT_TIMESPEC_NS
  return STAT_TIMESPEC_NS (st, st_mtim);
# else
  return 0;
# endif
}

#if defined HAVE_UTIMENSAT
/* Return *ST's access time.  */
static inline struct timespec
get_stat_atime (struct stat const *st)
{
#ifdef STAT_TIMESPEC
  return STAT_TIMESPEC (st, st_atim);
#else
  struct timespec t;
  t.tv_sec = st->st_atime;
  t.tv_nsec = get_stat_atime_ns (st);
  return t;
#endif
}

/* Return *ST's data modification time.  */
static inline struct timespec
get_stat_mtime (struct stat const *st)
{
#ifdef STAT_TIMESPEC
  return STAT_TIMESPEC (st, st_mtim);
#else
  struct timespec t;
  t.tv_sec = st->st_mtime;
  t.tv_nsec = get_stat_mtime_ns (st);
  return t;
#endif
}
#endif
/* End FIXME.  */

/* Set the times of the file DESTINATION to be the same as those in
   STATBUF.  */

void
set_times (const char *destination, const struct stat *statbuf)
{
  int result;
#if defined HAVE_UTIMENSAT
  struct timespec times[2];
  times[0] = get_stat_atime (statbuf);
  times[1] = get_stat_mtime (statbuf);
  result = utimensat (AT_FDCWD, destination, times, 0);
#elif defined HAVE_UTIMES
  struct timeval tv[2];

  tv[0].tv_sec = statbuf->st_atime;
  tv[0].tv_usec = get_stat_atime_ns (statbuf) / 1000;
  tv[1].tv_sec = statbuf->st_mtime;
  tv[1].tv_usec = get_stat_mtime_ns (statbuf) / 1000;
  result = utimes (destination, tv);
#elif defined HAVE_GOOD_UTIME_H
  struct utimbuf tb;

  tb.actime = statbuf->st_atime;
  tb.modtime = statbuf->st_mtime;
  result = utime (destination, &tb);
#else
  long tb[2];

  tb[0] = statbuf->st_atime;
  tb[1] = statbuf->st_mtime;
  result = utime (destination, tb);
#endif

  if (result != 0)
    non_fatal (_("%s: cannot set time: %s"), destination, strerror (errno));
}

/* Copy FROM to TO.  TARGET_STAT has the file status that, if non-NULL,
   is used to fix up timestamps.  Return 0 if ok, -1 if error.
   At one time this function renamed files, but file permissions are
   tricky to update given the number of different schemes used by
   various systems.  So now we just copy.  */

int
smart_rename (const char *from, const char *to, int fromfd,
	      struct stat *target_stat, bool preserve_dates)
{
  int ret = 0;

  if (to != from)
    {
      ret = simple_copy (fromfd, to, target_stat);
      if (ret != 0)
	non_fatal (_("unable to copy file '%s'; reason: %s"),
		   to, strerror (errno));
      unlink (from);
    }

  if (preserve_dates)
    set_times (to, target_stat);

  return ret;
}
