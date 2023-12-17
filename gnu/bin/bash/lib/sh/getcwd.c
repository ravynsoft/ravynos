/* getcwd.c -- get pathname of current directory */

/* Copyright (C) 1991 Free Software Foundation, Inc.

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

#if !defined (HAVE_GETCWD)

#if !defined (__GNUC__) && !defined (HAVE_ALLOCA_H) && defined (_AIX)
  #pragma alloca
#endif /* _AIX && RISC6000 && !__GNUC__ */

#if defined (__QNX__)
#  undef HAVE_LSTAT
#endif

#include <bashtypes.h>
#include <errno.h>

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <posixdir.h>
#include <posixstat.h>
#include <maxpath.h>
#include <memalloc.h>

#include <bashansi.h>

#if !defined (D_FILENO_AVAILABLE)
#  include "command.h"
#  include "general.h"
#  include "externs.h"
#endif

#include <xmalloc.h>

#if !defined (errno)
extern int errno;
#endif /* !errno */

#if !defined (HAVE_LSTAT)
#  define lstat stat
#endif

#if !defined (NULL)
#  define NULL 0
#endif

/* If the d_fileno member of a struct dirent doesn't return anything useful,
   we need to check inode number equivalence the hard way.  Return 1 if
   the inode corresponding to PATH/DIR is identical to THISINO. */
#if !defined (D_FILENO_AVAILABLE)
static int
_path_checkino (dotp, name, thisino)
     char *dotp;
     char *name;
     ino_t thisino;
{
  char *fullpath;
  int r, e;
  struct stat st;

  e = errno;
  fullpath = sh_makepath (dotp, name, MP_RMDOT);
  if (stat (fullpath, &st) < 0)
    {
      errno = e;
      return 0;
    }
  free (fullpath);
  errno = e;
  return (st.st_ino == thisino);
}
#endif
    
/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE <= 0, in which case it is as
   big as necessary.  */
#if defined (__STDC__)
char *
getcwd (char *buf, size_t size)
#else /* !__STDC__ */
char *
getcwd (buf, size)
     char *buf;
     size_t size;
#endif /* !__STDC__ */
{
  static const char dots[]
    = "../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../..";
  const char *dotp, *dotlist;
  size_t dotsize;
  dev_t rootdev, thisdev;
  ino_t rootino, thisino;
  char path[PATH_MAX + 1];
  register char *pathp;
  char *pathbuf;
  size_t pathsize;
  struct stat st;
  int saved_errno;

  if (buf != NULL && size == 0)
    {
      errno = EINVAL;
      return ((char *)NULL);
    }

  pathsize = sizeof (path);
  pathp = &path[pathsize];
  *--pathp = '\0';
  pathbuf = path;

  if (stat (".", &st) < 0)
    return ((char *)NULL);
  thisdev = st.st_dev;
  thisino = st.st_ino;

  if (stat ("/", &st) < 0)
    return ((char *)NULL);
  rootdev = st.st_dev;
  rootino = st.st_ino;

  saved_errno = 0;

  dotsize = sizeof (dots) - 1;
  dotp = &dots[sizeof (dots)];
  dotlist = dots;
  while (!(thisdev == rootdev && thisino == rootino))
    {
      register DIR *dirstream;
      register struct dirent *d;
      dev_t dotdev;
      ino_t dotino;
      char mount_point;
      int namlen;

      /* Look at the parent directory.  */
      if (dotp == dotlist)
	{
	  /* My, what a deep directory tree you have, Grandma.  */
	  char *new;
	  if (dotlist == dots)
	    {
	      new = (char *)malloc (dotsize * 2 + 1);
	      if (new == NULL)
		goto lose;
	      memcpy (new, dots, dotsize);
	    }
	  else
	    {
	      new = (char *)realloc ((PTR_T) dotlist, dotsize * 2 + 1);
	      if (new == NULL)
		goto lose;
	    }
	  memcpy (&new[dotsize], new, dotsize);
	  dotp = &new[dotsize];
	  dotsize *= 2;
	  new[dotsize] = '\0';
	  dotlist = new;
	}

      dotp -= 3;

      /* Figure out if this directory is a mount point.  */
      if (stat (dotp, &st) < 0)
	goto lose;
      dotdev = st.st_dev;
      dotino = st.st_ino;
      mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      dirstream = opendir (dotp);
      if (dirstream == NULL)
	goto lose;
      while ((d = readdir (dirstream)) != NULL)
	{
	  if (d->d_name[0] == '.' &&
	      (d->d_name[1] == '\0' ||
		(d->d_name[1] == '.' && d->d_name[2] == '\0')))
	    continue;
#if defined (D_FILENO_AVAILABLE)
	  if (mount_point || d->d_fileno == thisino)
#else
	  if (mount_point || _path_checkino (dotp, d->d_name, thisino))
#endif
	    {
	      char *name;

	      namlen = D_NAMLEN(d);
	      name = (char *)
		alloca (dotlist + dotsize - dotp + 1 + namlen + 1);
	      memcpy (name, dotp, dotlist + dotsize - dotp);
	      name[dotlist + dotsize - dotp] = '/';
	      memcpy (&name[dotlist + dotsize - dotp + 1],
		      d->d_name, namlen + 1);
	      if (lstat (name, &st) < 0)
		{
#if 0
		  int save = errno;
		  (void) closedir (dirstream);
		  errno = save;
		  goto lose;
#else
		  saved_errno = errno;
#endif
		}
	      if (st.st_dev == thisdev && st.st_ino == thisino)
		break;
	    }
	}
      if (d == NULL)
	{
#if 0
	  int save = errno;
#else
	  int save = errno ? errno : saved_errno;
#endif
	  (void) closedir (dirstream);
	  errno = save;
	  goto lose;
	}
      else
	{
	  size_t space;

	  while ((space = pathp - pathbuf) <= namlen)
	    {
	      char *new;

	      if (pathbuf == path)
		{
		  new = (char *)malloc (pathsize * 2);
		  if (!new)
		    goto lose;
		}
	      else
		{
		  new = (char *)realloc ((PTR_T) pathbuf, (pathsize * 2));
		  if (!new)
		    goto lose;
		  pathp = new + space;
		}
	      (void) memcpy (new + pathsize + space, pathp, pathsize - space);
	      pathp = new + pathsize + space;
	      pathbuf = new;
	      pathsize *= 2;
	    }

	  pathp -= namlen;
	  (void) memcpy (pathp, d->d_name, namlen);
	  *--pathp = '/';
	  (void) closedir (dirstream);
	}

      thisdev = dotdev;
      thisino = dotino;
    }

  if (pathp == &path[sizeof(path) - 1])
    *--pathp = '/';

  if (dotlist != dots)
    free ((PTR_T) dotlist);

  {
    size_t len = pathbuf + pathsize - pathp;
    if (buf == NULL && size <= 0)
      size = len;

    if ((size_t) size < len)
      {
	errno = ERANGE;
	goto lose2;
      }
    if (buf == NULL)
      {
	buf = (char *) malloc (size);
	if (buf == NULL)
	  goto lose2;
      }

    (void) memcpy((PTR_T) buf, (PTR_T) pathp, len);
  }

  if (pathbuf != path)
    free (pathbuf);

  return (buf);

 lose:
  if ((dotlist != dots) && dotlist)
    {
      int e = errno;
      free ((PTR_T) dotlist);
      errno = e;
    }

 lose2:
  if ((pathbuf != path) && pathbuf)
    {
      int e = errno;
      free ((PTR_T) pathbuf);
      errno = e;
    }
  return ((char *)NULL);
}

#if defined (TEST)
#  include <stdio.h>
main (argc, argv)
     int argc;
     char **argv;
{
  char b[PATH_MAX];

  if (getcwd(b, sizeof(b)))
    {
      printf ("%s\n", b);
      exit (0);
    }
  else
    {
      perror ("cwd: getcwd");
      exit (1);
    }
}
#endif /* TEST */
#endif /* !HAVE_GETCWD */
