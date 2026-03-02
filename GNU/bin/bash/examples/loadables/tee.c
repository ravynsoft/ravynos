/* tee - duplicate standard input */

/* See Makefile for compilation details. */

/*
   Copyright (C) 1999-2021 Free Software Foundation, Inc.

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
#include "posixstat.h"
#include "filecntl.h"

#include <signal.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

typedef struct flist {
  struct flist *next;
  int fd;
  char *fname;
} FLIST;

static FLIST *tee_flist;

#define TEE_BUFSIZE	8192

extern int interrupt_immediately;

extern char *strerror ();

int
tee_builtin (list)
     WORD_LIST *list;
{
  int opt, append, nointr, rval, fd, fflags;
  int n, nr, nw;
  FLIST *fl;
  char *buf, *bp;

  char *t;

  reset_internal_getopt ();
  append = nointr = 0;
  tee_flist = (FLIST *)NULL;
  while ((opt = internal_getopt (list, "ai")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  append = 1;
	  break;
	case 'i':
	  nointr = 1;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (nointr == 0)
    interrupt_immediately++;

  buf = xmalloc (TEE_BUFSIZE);

  /* Initialize output file list. */
  fl = tee_flist = (FLIST *)xmalloc (sizeof(FLIST));
  tee_flist->fd = 1;
  tee_flist->fname = "stdout";
  tee_flist->next = (FLIST *)NULL;

  /* Add file arguments to list of output files. */
  fflags = append ? O_WRONLY|O_CREAT|O_APPEND : O_WRONLY|O_CREAT|O_TRUNC;
  for (rval = EXECUTION_SUCCESS; list; list = list->next)
    {
      fd = open (list->word->word, fflags, 0666);
      if (fd < 0)
        {
          builtin_error ("%s: cannot open: %s", list->word->word, strerror (errno));
          rval = EXECUTION_FAILURE;
        }
      else
        {
          fl->next = (FLIST *)xmalloc (sizeof(FLIST));
          fl->next->fd = fd;
          fl->next->fname = list->word->word;
          fl = fl->next;
          fl->next = (FLIST *)NULL;
        }
      QUIT;
    }

  while ((nr = read(0, buf, TEE_BUFSIZE)) > 0)
    for (fl = tee_flist; fl; fl = fl->next)
      {
	n = nr;
	bp = buf;
	do
	  {
	    if ((nw = write (fl->fd, bp, n)) == -1)
	      {
		builtin_error ("%s: write error: %s", fl->fname, strerror (errno));
		rval = EXECUTION_FAILURE;
		break;
	      }
            bp += nw;
            QUIT;
	  }
	while (n -= nw);
      }
  if (nr < 0)
    builtin_error ("read error: %s", strerror (errno));

  /* Deallocate resources -- this is a builtin command. */
  tee_flist = tee_flist->next;		/* skip bogus close of stdout */
  while (tee_flist)
    {
      fl = tee_flist;
      if (close (fl->fd) < 0)
	{
	  builtin_error ("%s: close_error: %s", fl->fname, strerror (errno));
	  rval = EXECUTION_FAILURE;
	}
      tee_flist = tee_flist->next;
      free (fl);
    }

  QUIT;  
  return (rval);
}

char *tee_doc[] = {
	"Duplicate standard output.",
	"",
	"Copy standard input to standard output, making a copy in each",
	"filename argument.  If the `-a' option is given, the specified",
	"files are appended to, otherwise they are overwritten.  If the",
	"`-i' option is supplied, tee ignores interrupts.",
	(char *)NULL
};

struct builtin tee_struct = {
	"tee",			/* builtin name */
	tee_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	tee_doc,		/* array of long documentation strings. */
	"tee [-ai] [file ...]",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
