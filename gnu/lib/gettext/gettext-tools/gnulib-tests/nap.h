/* Assist in file system timestamp tests.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#ifndef GLTEST_NAP_H
# define GLTEST_NAP_H

# include <limits.h>

# include <stdckdint.h>

/* Avoid a conflict with a function called nap() on UnixWare.  */
# if defined _SCO_DS || (defined __SCO_VERSION__ || defined __sysv5__)  /* OpenServer, UnixWare */
#  include <unistd.h>
#  undef nap
#  define nap gl_nap
# endif

/* Name of the witness file.  */
#define TEMPFILE BASE "nap.tmp"

/* File descriptor used for the witness file.  */
static int nap_fd = -1;

/* Return A - B, in ns.
   Return 0 if the true result would be negative.
   Return INT_MAX if the true result would be greater than INT_MAX.  */
static int
diff_timespec (struct timespec a, struct timespec b)
{
  time_t as = a.tv_sec;
  time_t bs = b.tv_sec;
  int ans = a.tv_nsec;
  int bns = b.tv_nsec;
  int sdiff;

  ASSERT (0 <= ans && ans < 2000000000);
  ASSERT (0 <= bns && bns < 2000000000);

  if (! (bs < as || (bs == as && bns < ans)))
    return 0;

  if (ckd_sub (&sdiff, as, bs)
      || ckd_mul (&sdiff, sdiff, 1000000000)
      || ckd_add (&sdiff, sdiff, ans - bns))
    return INT_MAX;

  return sdiff;
}

/* If DO_WRITE, bump the modification time of the file designated by NAP_FD.
   Then fetch the new STAT information of NAP_FD.  */
static void
nap_get_stat (struct stat *st, int do_write)
{
  if (do_write)
    {
      ASSERT (write (nap_fd, "\n", 1) == 1);
#if defined _WIN32 || defined __CYGWIN__
      /* On Windows, the modification times are not changed until NAP_FD
         is closed. See
         <https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-writefile> */
      close (nap_fd);
      nap_fd = open (TEMPFILE, O_RDWR, 0600);
      ASSERT (nap_fd != -1);
      lseek (nap_fd, 0, SEEK_END);
#endif
    }
  ASSERT (fstat (nap_fd, st) == 0);
}

/* Given a file whose descriptor is FD, see whether delaying by DELAY
   nanoseconds causes a change in a file's mtime.
   OLD_ST is the file's status, recently gotten.  */
static bool
nap_works (int delay, struct stat old_st)
{
  struct stat st;
  struct timespec delay_spec;
  delay_spec.tv_sec = delay / 1000000000;
  delay_spec.tv_nsec = delay % 1000000000;
  ASSERT (nanosleep (&delay_spec, 0) == 0);
  nap_get_stat (&st, 1);

  if (diff_timespec (get_stat_mtime (&st), get_stat_mtime (&old_st)))
    return true;

  return false;
}

static void
clear_temp_file (void)
{
  if (0 <= nap_fd)
    {
      ASSERT (close (nap_fd) != -1);
      ASSERT (unlink (TEMPFILE) != -1);
    }
}

/* Sleep long enough to notice a timestamp difference on the file
   system in the current directory.  Use an adaptive approach, trying
   to find the smallest delay which works on the current file system
   to make the timestamp difference appear.  Assert a maximum delay of
   ~2 seconds, more precisely sum(2^n) from 0 to 30 = 2^31 - 1 = 2.1s.
   Assumes that BASE is defined, and requires that the test module
   depends on nanosleep.  */
static void
nap (void)
{
  struct stat old_st;
  static int delay = 1;

  if (-1 == nap_fd)
    {
      atexit (clear_temp_file);
      ASSERT ((nap_fd = creat (TEMPFILE, 0600)) != -1);
      nap_get_stat (&old_st, 0);
    }
  else
    {
      ASSERT (0 <= nap_fd);
      nap_get_stat (&old_st, 1);
    }

  if (1 < delay)
    delay = delay / 2;  /* Try half of the previous delay.  */
  ASSERT (0 < delay);

  for (;;)
    {
      if (nap_works (delay, old_st))
        return;
      if (delay <= (2147483647 - 1) / 2)
        {
          delay = delay * 2 + 1;
          continue;
        }
      else
        break;
    }

  /* Bummer: even the highest nap delay didn't work. */
  ASSERT (0);
}

#endif /* GLTEST_NAP_H */
