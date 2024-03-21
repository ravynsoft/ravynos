/* stat - load up an associative array with stat information about a file */

/* See Makefile for compilation details. */

/*
   Copyright (C) 2016,2022 Free Software Foundation, Inc.

   This file is part of GNU Bash.
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

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>

#include <sys/types.h>
#include "posixstat.h"
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include "posixtime.h"

#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "common.h"
#include "bashgetopt.h"

#ifndef errno
extern int	errno;
#endif

#define ST_NAME		0
#define ST_DEV		1
#define ST_INO		2
#define ST_MODE		3
#define ST_NLINK	4
#define ST_UID		5
#define ST_GID		6
#define ST_RDEV		7
#define ST_SIZE		8
#define ST_ATIME	9
#define ST_MTIME	10
#define ST_CTIME	11
#define ST_BLKSIZE	12
#define ST_BLOCKS	13
#define ST_CHASELINK	14
#define ST_PERMS	15

#define ST_END		16

static char *arraysubs[] =
  {
    "name", "device", "inode", "type", "nlink", "uid", "gid", "rdev",
    "size", "atime", "mtime", "ctime", "blksize", "blocks", "link", "perms",
    0
  };

#define DEFTIMEFMT	"%a %b %e %k:%M:%S %Z %Y"
#ifndef TIMELEN_MAX
#  define TIMELEN_MAX 128
#endif

static char *stattime (time_t, const char *);

static int
getstat (fname, flags, sp)
     const char *fname;
     int flags;
     struct stat *sp;
{
  intmax_t lfd;
  int fd, r;

  if (strncmp (fname, "/dev/fd/", 8) == 0)
    {
      if ((legal_number(fname + 8, &lfd) == 0) || (int)lfd != lfd)
	{
	  errno = EINVAL;
	  return -1;
	}
      fd = lfd;
      r = fstat(fd, sp);
    }
#ifdef HAVE_LSTAT
  else if (flags & 1)
    r = lstat(fname, sp);
#endif
  else
    r = stat(fname, sp);

  return r;
}

static char *
statlink (fname, sp)
     char *fname;
     struct stat *sp;
{
#if defined (HAVE_READLINK)
  char linkbuf[PATH_MAX];
  int n;

  if (fname && S_ISLNK (sp->st_mode) && (n = readlink (fname, linkbuf, PATH_MAX)) > 0)
    {
      linkbuf[n] = '\0';
      return (savestring (linkbuf));
    }
  else
#endif
    return (savestring (fname));
}

static char *
octalperms (m)
     int m;
{
  int operms;
  char *ret;

  operms = 0;

  if (m & S_IRUSR)
    operms |= 0400;
  if (m & S_IWUSR)
    operms |= 0200;
  if (m & S_IXUSR)
    operms |= 0100;

  if (m & S_IRGRP)
    operms |= 0040;
  if (m & S_IWGRP)
    operms |= 0020;
  if (m & S_IXGRP)
    operms |= 0010;

  if (m & S_IROTH)
    operms |= 0004;
  if (m & S_IWOTH)
    operms |= 0002;
  if (m & S_IXOTH)
    operms |= 0001;

  if (m & S_ISUID)
    operms |= 04000;
  if (m & S_ISGID)
    operms |= 02000;
  if (m & S_ISVTX)
    operms |= 01000;

  ret = (char *)xmalloc (16);
  snprintf (ret, 16, "%04o", operms);
  return ret;
}

static char *
statperms (m)
     int m;
{
  char ubits[4], gbits[4], obits[4];	/* u=rwx,g=rwx,o=rwx */
  int i;
  char *ret;

  i = 0;
  if (m & S_IRUSR)
    ubits[i++] = 'r';
  if (m & S_IWUSR)
    ubits[i++] = 'w';
  if (m & S_IXUSR)
    ubits[i++] = 'x';
  ubits[i] = '\0';

  i = 0;
  if (m & S_IRGRP)
    gbits[i++] = 'r';
  if (m & S_IWGRP)
    gbits[i++] = 'w';
  if (m & S_IXGRP)
    gbits[i++] = 'x';
  gbits[i] = '\0';

  i = 0;
  if (m & S_IROTH)
    obits[i++] = 'r';
  if (m & S_IWOTH)
    obits[i++] = 'w';
  if (m & S_IXOTH)
    obits[i++] = 'x';
  obits[i] = '\0';

  if (m & S_ISUID)
    ubits[2] = (m & S_IXUSR) ? 's' : 'S';
  if (m & S_ISGID)
    gbits[2] = (m & S_IXGRP) ? 's' : 'S';
  if (m & S_ISVTX)
    obits[2] = (m & S_IXOTH) ? 't' : 'T';

  ret = (char *)xmalloc (32);
  snprintf (ret, 32, "u=%s,g=%s,o=%s", ubits, gbits, obits);
  return ret;
}

static char *
statmode(mode)
     int mode;
{
  char *modestr, *m;

  modestr = m = (char *)xmalloc (8);
  if (S_ISBLK (mode))
    *m++ = 'b';
  if (S_ISCHR (mode))
    *m++ = 'c';
  if (S_ISDIR (mode))
    *m++ = 'd';
  if (S_ISREG(mode))
    *m++ = '-';
  if (S_ISFIFO(mode))
    *m++ = 'p';
  if (S_ISLNK(mode))
    *m++ = 'l';
  if (S_ISSOCK(mode))
    *m++ = 's';

#ifdef S_ISDOOR
  if (S_ISDOOR (mode))
    *m++ = 'D';
#endif
#ifdef S_ISWHT
  if (S_ISWHT(mode))
    *m++ = 'W';
#endif
#ifdef S_ISNWK
  if (S_ISNWK(mode))
    *m++ = 'n';
#endif
#ifdef S_ISMPC
  if (S_ISMPC (mode))
    *m++ = 'm';
#endif

  *m = '\0';
  return (modestr);
}

static char *
stattime (t, timefmt)
     time_t t;
     const char *timefmt;
{
  char *tbuf, *ret;
  const char *fmt;
  size_t tlen;
  struct tm *tm;

  fmt = timefmt ? timefmt : DEFTIMEFMT;
  tm = localtime (&t);

  ret = xmalloc (TIMELEN_MAX);

  tlen = strftime (ret, TIMELEN_MAX, fmt, tm);
  if (tlen == 0)
    tlen = strftime (ret, TIMELEN_MAX, DEFTIMEFMT, tm);

  return ret;
}

static char *
statval (which, fname, flags, fmt, sp)
     int which;
     char *fname;
     int flags;
     char *fmt;
     struct stat *sp;
{
  int temp;

  switch (which)
    {
    case ST_NAME:
      return savestring (fname);
    case ST_DEV:
      return itos (sp->st_dev);
    case ST_INO:
      return itos (sp->st_ino);
    case ST_MODE:
      return (statmode (sp->st_mode));
    case ST_NLINK:
      return itos (sp->st_nlink);
    case ST_UID:
      return itos (sp->st_uid);
    case ST_GID:
      return itos (sp->st_gid);
    case ST_RDEV:
      return itos (sp->st_rdev);
    case ST_SIZE:
      return itos (sp->st_size);
    case ST_ATIME:
      return ((flags & 2) ? stattime (sp->st_atime, fmt) : itos (sp->st_atime));
    case ST_MTIME:
      return ((flags & 2) ? stattime (sp->st_mtime, fmt) : itos (sp->st_mtime));
    case ST_CTIME:
      return ((flags & 2) ? stattime (sp->st_ctime, fmt) : itos (sp->st_ctime));
    case ST_BLKSIZE:
      return itos (sp->st_blksize);
    case ST_BLOCKS:
      return itos (sp->st_blocks);
    case ST_CHASELINK:
      return (statlink (fname, sp));
    case ST_PERMS:
      temp = sp->st_mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID);
      return (flags & 2) ? statperms (temp) : octalperms (temp);
    default:
      return savestring ("42");
    }
}

static int
loadstat (vname, var, fname, flags, fmt, sp)
     char *vname;
     SHELL_VAR *var;
     char *fname;
     int flags;
     char *fmt;
     struct stat *sp;
{
  int i;
  char *key, *value;
  SHELL_VAR *v;

  for (i = 0; arraysubs[i]; i++)
    {
      key = savestring (arraysubs[i]);
      value = statval (i, fname, flags, fmt, sp);
      v = bind_assoc_variable (var, vname, key, value, ASS_FORCE);
    }
  return 0;
}

int
stat_builtin (list)
     WORD_LIST *list;
{
  int opt, flags;
  char *aname, *fname, *timefmt;
  struct stat st;
  SHELL_VAR *v;

  aname = "STAT";
  flags = 0;
  timefmt = 0;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "A:F:Ll")) != -1)
    {
      switch (opt)
	{
	case 'A':
	  aname = list_optarg;
	  break;
	case 'L':
	  flags |= 1;		/* operate on links rather than resolving them */
	  break;
	case 'l':
	  flags |= 2;
	  break;
	case 'F':
	  timefmt = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }

  if (legal_identifier (aname) == 0)
    {
      sh_invalidid (aname);
      return (EXECUTION_FAILURE);
    }

  list = loptend;
  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }


#if 0
  unbind_variable (aname);
#endif
  fname = list->word->word;

  if (getstat (fname, flags, &st) < 0)
    {
      builtin_error ("%s: cannot stat: %s", fname, strerror (errno));
      return (EXECUTION_FAILURE);
    }

  v = find_or_make_array_variable (aname, 3);
  if (v == 0)
    {
      builtin_error ("%s: cannot create variable", aname);
      return (EXECUTION_FAILURE);
    }
  if (loadstat (aname, v, fname, flags, timefmt, &st) < 0)
    {
      builtin_error ("%s: cannot assign file status information", aname);
      unbind_variable (aname);
      return (EXECUTION_FAILURE);
    }

  return (EXECUTION_SUCCESS);
}

/* An array of strings forming the `long' documentation for a builtin xxx,
   which is printed by `help xxx'.  It must end with a NULL.  By convention,
   the first line is a short description. */
char *stat_doc[] = {
	"Load an associative array with file status information.",
	"",
	"Take a filename and load the status information returned by a",
	"stat(2) call on that file into the associative array specified",
	"by the -A option.  The default array name is STAT.",
	"",
	"If the -L option is supplied, stat does not resolve symbolic links",
	"and reports information about the link itself.  The -l option results",
	"in longer-form listings for some of the fields. When -l is used,",
	"the -F option supplies a format string passed to strftime(3) to",
	"display the file time information.",
	"The exit status is 0 unless the stat fails or assigning the array",
	"is unsuccessful.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures.  The flags must include BUILTIN_ENABLED so the
   builtin can be used. */
struct builtin stat_struct = {
	"stat",			/* builtin name */
	stat_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	stat_doc,		/* array of long documentation strings. */
	"stat [-lL] [-A aname] file",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
