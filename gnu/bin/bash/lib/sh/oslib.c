/* oslib.c - functions present only in some unix versions. */

/* Copyright (C) 1995,2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include <bashtypes.h>
#if defined (HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#endif

#include <posixstat.h>
#include <filecntl.h>
#include <bashansi.h>

#if !defined (HAVE_KILLPG)
#  include <signal.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <chartypes.h>

#include <shell.h>

#if !defined (errno)
extern int errno;
#endif /* !errno */

/* Make the functions strchr and strrchr if they do not exist. */
#if !defined (HAVE_STRCHR)
char *
strchr (string, c)
     char *string;
     int c;
{
  register char *s;

  for (s = string; s && *s; s++)
    if (*s == c)
      return (s);

  return ((char *) NULL);
}

char *
strrchr (string, c)
     char *string;
     int c;
{
  register char *s, *t;

  for (s = string, t = (char *)NULL; s && *s; s++)
    if (*s == c)
      t = s;
  return (t);
}
#endif /* !HAVE_STRCHR */

#if !defined (HAVE_DUP2) || defined (DUP2_BROKEN)
/* Replacement for dup2 (), for those systems which either don't have it,
   or supply one with broken behaviour. */
int
dup2 (fd1, fd2)
     int fd1, fd2;
{
  int saved_errno, r;

  /* If FD1 is not a valid file descriptor, then return immediately with
     an error. */
  if (fcntl (fd1, F_GETFL, 0) == -1)
    return (-1);

  if (fd2 < 0 || fd2 >= getdtablesize ())
    {
      errno = EBADF;
      return (-1);
    }

  if (fd1 == fd2)
    return (0);

  saved_errno = errno;

  (void) close (fd2);
  r = fcntl (fd1, F_DUPFD, fd2);

  if (r >= 0)
    errno = saved_errno;
  else
    if (errno == EINVAL)
      errno = EBADF;

  /* Force the new file descriptor to remain open across exec () calls. */
  SET_OPEN_ON_EXEC (fd2);
  return (r);
}
#endif /* !HAVE_DUP2 */

/*
 * Return the total number of available file descriptors.
 *
 * On some systems, like 4.2BSD and its descendants, there is a system call
 * that returns the size of the descriptor table: getdtablesize().  There are
 * lots of ways to emulate this on non-BSD systems.
 *
 * On System V.3, this can be obtained via a call to ulimit:
 *	return (ulimit(4, 0L));
 *
 * On other System V systems, NOFILE is defined in /usr/include/sys/param.h
 * (this is what we assume below), so we can simply use it:
 *	return (NOFILE);
 *
 * On POSIX systems, there are specific functions for retrieving various
 * configuration parameters:
 *	return (sysconf(_SC_OPEN_MAX));
 *
 */

#if !defined (HAVE_GETDTABLESIZE)
int
getdtablesize ()
{
#  if defined (_POSIX_VERSION) && defined (HAVE_SYSCONF) && defined (_SC_OPEN_MAX)
  return (sysconf(_SC_OPEN_MAX));	/* Posix systems use sysconf */
#  else /* ! (_POSIX_VERSION && HAVE_SYSCONF && _SC_OPEN_MAX) */
#    if defined (ULIMIT_MAXFDS)
  return (ulimit (4, 0L));	/* System V.3 systems use ulimit(4, 0L) */
#    else /* !ULIMIT_MAXFDS */
#      if defined (NOFILE)	/* Other systems use NOFILE */
  return (NOFILE);
#      else /* !NOFILE */
  return (20);			/* XXX - traditional value is 20 */
#      endif /* !NOFILE */
#    endif /* !ULIMIT_MAXFDS */
#  endif /* ! (_POSIX_VERSION && _SC_OPEN_MAX) */
}
#endif /* !HAVE_GETDTABLESIZE */

#if !defined (HAVE_BCOPY)
#  if defined (bcopy)
#    undef bcopy
#  endif
void
bcopy (s,d,n)
     void *d, *s;
     size_t n;
{
  FASTCOPY (s, d, n);
}
#endif /* !HAVE_BCOPY */

#if !defined (HAVE_BZERO)
#  if defined (bzero)
#    undef bzero
#  endif
void
bzero (s, n)
     void *s; 
     size_t n;
{
  register int i;
  register char *r;

  for (i = 0, r = s; i < n; i++)
    *r++ = '\0';
}
#endif

#if !defined (HAVE_GETHOSTNAME)
#  if defined (HAVE_UNAME)
#    include <sys/utsname.h>
int
gethostname (name, namelen)
     char *name;
     size_t namelen;
{
  int i;
  struct utsname ut;

  --namelen;

  uname (&ut);
  i = strlen (ut.nodename) + 1;
  strncpy (name, ut.nodename, i < namelen ? i : namelen);
  name[namelen] = '\0';
  return (0);
}
#  else /* !HAVE_UNAME */
int
gethostname (name, namelen)
     char *name;
     size_t namelen;
{
  strncpy (name, "unknown", namelen);
  name[namelen] = '\0';
  return 0;
}
#  endif /* !HAVE_UNAME */
#endif /* !HAVE_GETHOSTNAME */

#if !defined (HAVE_KILLPG)
int
killpg (pgrp, sig)
     pid_t pgrp;
     int sig;
{
  return (kill (-pgrp, sig));
}
#endif /* !HAVE_KILLPG */

#if !defined (HAVE_MKFIFO) && defined (PROCESS_SUBSTITUTION)
int
mkfifo (path, mode)
     char *path;
     mode_t mode;
{
#if defined (S_IFIFO)
  return (mknod (path, (mode | S_IFIFO), 0));
#else /* !S_IFIFO */
  return (-1);
#endif /* !S_IFIFO */
}
#endif /* !HAVE_MKFIFO && PROCESS_SUBSTITUTION */

#define DEFAULT_MAXGROUPS 64

int
getmaxgroups ()
{
  static int maxgroups = -1;

  if (maxgroups > 0)
    return maxgroups;

#if defined (HAVE_SYSCONF) && defined (_SC_NGROUPS_MAX)
  maxgroups = sysconf (_SC_NGROUPS_MAX);
#else
#  if defined (NGROUPS_MAX)
  maxgroups = NGROUPS_MAX;
#  else /* !NGROUPS_MAX */
#    if defined (NGROUPS)
  maxgroups = NGROUPS;
#    else /* !NGROUPS */
  maxgroups = DEFAULT_MAXGROUPS;
#    endif /* !NGROUPS */
#  endif /* !NGROUPS_MAX */  
#endif /* !HAVE_SYSCONF || !SC_NGROUPS_MAX */

  if (maxgroups <= 0)
    maxgroups = DEFAULT_MAXGROUPS;

  return maxgroups;
}

long
getmaxchild ()
{
  static long maxchild = -1L;

  if (maxchild > 0)
    return maxchild;

#if defined (HAVE_SYSCONF) && defined (_SC_CHILD_MAX)
  maxchild = sysconf (_SC_CHILD_MAX);
#else
#  if defined (CHILD_MAX)
  maxchild = CHILD_MAX;
#  else
#    if defined (MAXUPRC)
  maxchild = MAXUPRC;
#    endif /* MAXUPRC */
#  endif /* CHILD_MAX */
#endif /* !HAVE_SYSCONF || !_SC_CHILD_MAX */

  return (maxchild);
}
