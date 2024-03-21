/* Loadable builtin to get and set file descriptor flags. */

/* See Makefile for compilation details. */

/*
   Copyright (C) 2017-2022 Free Software Foundation, Inc.

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
#include <fcntl.h>
#include <errno.h>
#include "bashansi.h"
#include <stdio.h>

#include "loadables.h"

#ifndef FD_CLOEXEC
#  define FD_CLOEXEC 1
#endif

static const struct
{
  const char *name;
  int value;
} file_flags[] =
{
#ifdef O_APPEND
  { "append",	O_APPEND 	},
#else
#  define O_APPEND 0
#endif
#ifdef O_ASYNC
  { "async",	O_ASYNC		},
#else
#  define O_ASYNC 0
#endif
#ifdef O_SYNC
  { "sync",	O_SYNC		},
#else
#  define O_SYNC 0
#endif
#ifdef O_NONBLOCK
  { "nonblock",	O_NONBLOCK	},
#else
#  define O_NONBLOCK 0
#endif
#ifdef O_FSYNC
  { "fsync",	O_FSYNC		},
#else
#  define O_FSYNC 0
#endif
#ifdef O_DSYNC
  { "dsync",	O_DSYNC		},
#else
#  define O_DSYNC 0
#endif
#ifdef O_RSYNC
  { "rsync",	O_RSYNC		},
#else
#  define O_RSYNC 0
#endif
#ifdef O_ALT_IO
  { "altio",	O_ALT_IO	},
#else
#  define O_ALT_IO 0
#endif
#ifdef O_DIRECT
  { "direct",	O_DIRECT	},
#else
#  define O_DIRECT 0
#endif
#ifdef O_NOATIME
  { "noatime",	O_NOATIME	},
#else
#  define O_NOATIME 0
#endif
#ifdef O_NOSIGPIPE
  { "nosigpipe",	O_NOSIGPIPE	},
#else
#  define O_NOSIGPIPE 0
#endif

#ifndef O_CLOEXEC
#  define ALLFLAGS (O_APPEND|O_ASYNC|O_SYNC|O_NONBLOCK|O_FSYNC|O_DSYNC|\
	O_RSYNC|O_ALT_IO|O_DIRECT|O_NOATIME|O_NOSIGPIPE)

/* An unused bit in the file status flags word we can use to pass around the
   state of close-on-exec. */
# define O_CLOEXEC      ((~ALLFLAGS) ^ ((~ALLFLAGS) & ((~ALLFLAGS) - 1)))
#endif

#ifdef O_CLOEXEC
  { "cloexec",	O_CLOEXEC	},
#endif
};

#define N_FLAGS		(sizeof (file_flags) / sizeof (file_flags[0]))

#ifndef errno
extern int errno;
#endif

/* FIX THIS */
static int
getallflags ()
{
  int i, allflags;
  
  for (i = allflags = 0; i < N_FLAGS; i++)
    allflags |= file_flags[i].value;
  return allflags;
}

static int
getflags(int fd, int p)
{
  int c, f;
  int allflags;

  if ((c = fcntl(fd, F_GETFD)) == -1)
    {
      if (p)
	builtin_error("can't get status for fd %d: %s", fd, strerror(errno));
      return -1;
    }
    
  if ((f = fcntl(fd, F_GETFL)) == -1)
    {
      if (p)
	builtin_error("Can't get flags for fd %d: %s", fd, strerror(errno));
      return -1;
    }

  if (c)
    f |= O_CLOEXEC;

  return f & getallflags();
}

static void
printone(int fd, int p, int verbose)
{
  int f;
  size_t i;

  if ((f = getflags(fd, p)) == -1)
    return;

  printf ("%d:", fd);

  for (i = 0; i < N_FLAGS; i++)
    {
      if (f & file_flags[i].value)
	{
	  printf ("%s%s", verbose ? "+" : "", file_flags[i].name);
	  f &= ~file_flags[i].value;
	}
      else if (verbose)
	printf ( "-%s", file_flags[i].name);
      else
	continue;

      if (f || (verbose && i != N_FLAGS - 1))
	putchar (',');
    }
  printf ("\n");
}

static int
parseflags(char *s, int *p, int *n)
{
  int f, *v;
  size_t i;

  f = 0;
  *p = *n = 0;

  for (s = strtok(s, ","); s; s = strtok(NULL, ","))
    {
      switch (*s)
        {
	case '+':
	  v = p;
	  s++;
	  break;
	case '-':
	  v = n;
	  s++;
	  break;
	default:
	  v = &f;
	  break;
	}
			
      for (i = 0; i < N_FLAGS; i++)
	if (strcmp(s, file_flags[i].name) == 0)
	  {
	    *v |= file_flags[i].value;
	    break;
	  }
      if (i == N_FLAGS)
	builtin_error("invalid flag `%s'", s);
    }

  return f;
}

static void
setone(int fd, char *v, int verbose)
{
  int f, n, pos, neg, cloexec;

  f = getflags(fd, 1);
  if (f == -1)
    return;

  parseflags(v, &pos, &neg);

  cloexec = -1;

  if ((pos & O_CLOEXEC) && (f & O_CLOEXEC) == 0)
    cloexec = FD_CLOEXEC;
  if ((neg & O_CLOEXEC) && (f & O_CLOEXEC))
    cloexec = 0;

  if (cloexec != -1 && fcntl(fd, F_SETFD, cloexec) == -1)
    builtin_error("can't set status for fd %d: %s", fd, strerror(errno));

  pos &= ~O_CLOEXEC;
  neg &= ~O_CLOEXEC;
  f &= ~O_CLOEXEC;

  n = f;
  n |= pos;
  n &= ~neg;

  if (n != f && fcntl(fd, F_SETFL, n) == -1)
    builtin_error("can't set flags for fd %d: %s", fd, strerror(errno));
}

static int
getmaxfd ()
{
  int maxfd, ignore;

#ifdef F_MAXFD
  maxfd = fcntl (0, F_MAXFD);
  if (maxfd > 0)
    return maxfd;
#endif

  maxfd = getdtablesize ();
  if (maxfd <= 0)
    maxfd = HIGH_FD_MAX;
  for (maxfd--; maxfd > 0; maxfd--)
    if (fcntl (maxfd, F_GETFD, &ignore) != -1)
      break;

  return maxfd;
}

int
fdflags_builtin (WORD_LIST *list)
{
  int opt, maxfd, i, num, verbose, setflag;
  char *setspec;
  WORD_LIST *l;
  intmax_t inum;

  setflag = verbose = 0;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "s:v")) != -1)
    {
      switch (opt)
	{
	case 's':
	  setflag = 1;
	  setspec = list_optarg;
	  break;
	case 'v':
	  verbose = 1;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
	
    }
  list = loptend;

  /* Maybe we could provide some default here, but we don't yet. */
  if (list == 0 && setflag)
    return (EXECUTION_SUCCESS);

  if (list == 0)
    {
      maxfd = getmaxfd ();
      if (maxfd < 0)
	{
	  builtin_error ("can't get max fd: %s", strerror (errno));
	  return (EXECUTION_FAILURE);
	}
      for (i = 0; i < maxfd; i++)
	printone (i, 0, verbose);
      return (EXECUTION_SUCCESS);
    }

  opt = EXECUTION_SUCCESS;
  for (l = list; l; l = l->next)
    {
      if (legal_number (l->word->word, &inum) == 0 || inum < 0)
	{
	  builtin_error ("%s: invalid file descriptor", l->word->word);
	  opt = EXECUTION_FAILURE;
	  continue;
	}
      num = inum;		/* truncate to int */
      if (setflag)
	setone (num, setspec, verbose);
      else
	printone (num, 1, verbose);
    }

  return (opt);
}

char *fdflags_doc[] =
{
  "Display and modify file descriptor flags.",
  "",
  "Display or, if the -s option is supplied, set flags for each file",
  "descriptor supplied as an argument.  If the -v option is supplied,",
  "the display is verbose, including each settable option name in the",
  "form of a string such as that accepted by the -s option.",
  "",
  "The -s option accepts a string with a list of flag names, each preceded",
  "by a `+' (set) or `-' (unset).  Those changes are applied to each file",
  "descriptor supplied as an argument.",
  "",
  "If no file descriptor arguments are supplied, the displayed information",
  "consists of the status of flags for each of the shell's open files.",
  (char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures.  The flags must include BUILTIN_ENABLED so the
   builtin can be used. */
struct builtin fdflags_struct = {
	"fdflags",		/* builtin name */
	fdflags_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	fdflags_doc,		/* array of long documentation strings. */
	"fdflags [-v] [-s flags_string] [fd ...]",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
