/* pselect - synchronous I/O multiplexing

   Copyright 2011-2023 Free Software Foundation, Inc.

   This file is part of gnulib.

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

/* written by Paul Eggert */

#include <config.h>

#include <sys/select.h>

#include <errno.h>
#include <signal.h>

/* Examine the size-NFDS file descriptor sets in RFDS, WFDS, and XFDS
   to see whether some of their descriptors are ready for reading,
   ready for writing, or have exceptions pending.  Wait for at most
   TIMEOUT seconds, and use signal mask SIGMASK while waiting.  A null
   pointer parameter stands for no descriptors, an infinite timeout,
   or an unaffected signal mask.  */

#if !HAVE_PSELECT

int
pselect (int nfds, fd_set *restrict rfds,
         fd_set *restrict wfds, fd_set *restrict xfds,
         struct timespec const *restrict timeout,
         sigset_t const *restrict sigmask)
{
  int select_result;
  sigset_t origmask;
  struct timeval tv, *tvp;

  if (nfds < 0 || nfds > FD_SETSIZE)
    {
      errno = EINVAL;
      return -1;
    }

  if (timeout)
    {
      if (! (0 <= timeout->tv_nsec && timeout->tv_nsec < 1000000000))
        {
          errno = EINVAL;
          return -1;
        }

      tv = (struct timeval) {
        .tv_sec = timeout->tv_sec,
        .tv_usec = (timeout->tv_nsec + 999) / 1000
      };
      tvp = &tv;
    }
  else
    tvp = NULL;

  /* Signal mask munging should be atomic, but this is the best we can
     do in this emulation.  */
  if (sigmask)
    pthread_sigmask (SIG_SETMASK, sigmask, &origmask);

  select_result = select (nfds, rfds, wfds, xfds, tvp);

  if (sigmask)
    {
      int select_errno = errno;
      pthread_sigmask (SIG_SETMASK, &origmask, NULL);
      errno = select_errno;
    }

  return select_result;
}

#else /* HAVE_PSELECT */
# include <unistd.h>
# undef pselect

int
rpl_pselect (int nfds, fd_set *restrict rfds,
             fd_set *restrict wfds, fd_set *restrict xfds,
             struct timespec const *restrict timeout,
             sigset_t const *restrict sigmask)
{
  int i;

  /* FreeBSD 8.2 has a bug: it does not always detect invalid fds.  */
  if (nfds < 0 || nfds > FD_SETSIZE)
    {
      errno = EINVAL;
      return -1;
    }
  for (i = 0; i < nfds; i++)
    {
      if (((rfds && FD_ISSET (i, rfds))
           || (wfds && FD_ISSET (i, wfds))
           || (xfds && FD_ISSET (i, xfds)))
          && dup2 (i, i) != i)
        return -1;
    }

  return pselect (nfds, rfds, wfds, xfds, timeout, sigmask);
}

#endif
