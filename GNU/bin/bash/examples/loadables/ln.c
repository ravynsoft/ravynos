/* ln - make links */

/* See Makefile for compilation details. */

/*
   Copyright (C) 1999-2020 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "posixstat.h"

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

typedef int unix_link_syscall_t PARAMS((const char *, const char *));

#define LN_SYMLINK 0x01
#define LN_UNLINK  0x02
#define LN_NOFOLLOW 0x04

static unix_link_syscall_t *linkfn;
static int dolink ();

int
ln_builtin (list)
     WORD_LIST *list;
{
  int rval, opt, flags;
  WORD_LIST *l;
  char *sdir;
  struct stat sb;

  flags = 0;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "fs")) != -1)
    {
      switch (opt)
	{
	case 'f':
	  flags |= LN_UNLINK;
	  break;
	case 's':
	  flags |= LN_SYMLINK;
	  break;
	case 'h':
	case 'n':
	  flags |= LN_NOFOLLOW;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }
    
  linkfn = (flags & LN_SYMLINK) ? symlink : link;  

  if (list->next == 0)			/* ln target, equivalent to ln target . */
    return (dolink (list->word->word, ".", flags));

  if (list->next->next == 0)		/* ln target source */
    return (dolink (list->word->word, list->next->word->word, flags));

  /* ln target1 target2 ... directory */

  /* find last argument: target directory, and make sure it's an existing
     directory. */
  for (l = list; l->next; l = l->next)  
    ;
  sdir = l->word->word;

  if (stat(sdir, &sb) < 0)
    {
      builtin_error ("%s", sdir);
      return (EXECUTION_FAILURE);
    }

  if (S_ISDIR (sb.st_mode) == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  for (rval = EXECUTION_SUCCESS; list != l; list = list->next)
    rval += dolink (list->word->word, sdir, flags);
  
  return rval;
}

static char *
mkdirpath (dir, file)
     char *dir, *file;
{
  int dlen, flen;
  char *ret;

  dlen = strlen (dir);
  flen = strlen (file);

  ret = xmalloc (2 + dlen + flen);

  strcpy (ret, dir);
  if (ret[dlen - 1] != '/')
    ret[dlen++] = '/';
  strcpy (ret + dlen, file);
  return ret;
}

#if defined (HAVE_LSTAT)
#  define LSTAT lstat
#  define LSTAT_OR_STAT_IF(c, f, b)	((c) ? lstat((f), (b)) : stat((f), (b)))
#else
#  define LSTAT stat
#  define LSTAT_OR_STAT_IF(c, f, b)	(stat((f), (b)))
#endif

static int
dolink (src, dst, flags)
     char *src, *dst;
     int flags;
{
  struct stat ssb, dsb;
  int exists;
  char *dst_path, *p;

  /* If we're not doing symlinks, the source must exist and not be a 
     directory. */
  if ((flags & LN_SYMLINK) == 0)
    {
      if (stat (src, &ssb) != 0)
	{
	  builtin_error ("%s: %s", src, strerror (errno));
	  return (EXECUTION_FAILURE);
	}
      if (S_ISDIR (ssb.st_mode))
	{
	  errno = EISDIR;
	  builtin_error ("%s: %s", src, strerror (errno));
	  return (EXECUTION_FAILURE);
	}
    }

  /* If the destination is a directory, create the final filename by appending
     the basename of the source to the destination. */
  dst_path = 0;
  if ((LSTAT_OR_STAT_IF((flags & LN_NOFOLLOW), dst, &dsb) == 0) && S_ISDIR (dsb.st_mode))
    {
      if ((p = strrchr (src, '/')) == 0)
	p = src;
      else
	p++;

      dst_path = mkdirpath (dst, p);
      dst = dst_path;
    }

  exists = LSTAT (dst, &dsb) == 0;

  /* If -f was specified, and the destination exists, unlink it. */
  if ((flags & LN_UNLINK) && exists && unlink (dst) != 0)
    {
      builtin_error ("%s: cannot unlink: %s", dst, strerror (errno));
      FREE (dst_path);
      return (EXECUTION_FAILURE);
    }

  /* Perform the link. */
  if ((*linkfn) (src, dst) != 0)
    {
      builtin_error ("cannot link %s to %s: %s", dst, src, strerror (errno));
      FREE (dst_path);
      return (EXECUTION_FAILURE);
    }

  FREE (dst_path);
  return (EXECUTION_SUCCESS);
}

char *ln_doc[] = {
	"Link files.",
	"",
	"Create a new directory entry with the same modes as the original",
	"file.  The -f option means to unlink any existing file, permitting",
	"the link to occur.  The -s option means to create a symbolic link.",
	"By default, ln makes hard links.  Specifying -n or its synonym -h",
	"causes ln to not resolve symlinks in the target file or directory.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin ln_struct = {
	"ln",		/* builtin name */
	ln_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	ln_doc,		/* array of long documentation strings. */
	"ln [-fhns] file1 [file2] OR ln [-fhns] file ... directory",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
