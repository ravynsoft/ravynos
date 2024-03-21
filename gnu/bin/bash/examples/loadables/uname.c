/*
 * uname - print system information
 *
 * usage: uname [-amnrsv]
 *
 */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

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
#include <stdio.h>

#include "bashtypes.h"

#if defined (HAVE_UNAME)
#  include <sys/utsname.h>
#else
struct utsname {
	char	sysname[32];
	char	nodename[32];
	char	release[32];
	char	version[32];
	char	machine[32];
};
#endif

#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

#define FLAG_SYSNAME	0x01	/* -s */
#define FLAG_NODENAME	0x02	/* -n */
#define FLAG_RELEASE	0x04	/* -r */
#define FLAG_VERSION	0x08	/* -v */
#define FLAG_MACHINE	0x10	/* -m, -p */

#define FLAG_ALL	0x1f

#ifndef errno
extern int errno;
#endif

static void uprint();

static int uname_flags;

int
uname_builtin (list)
     WORD_LIST *list;
{
  int opt, r;
  struct utsname uninfo;

  uname_flags = 0;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "amnprsv")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  uname_flags |= FLAG_ALL;
	  break;
	case 'm':
	case 'p':
	  uname_flags |= FLAG_MACHINE;
	  break;
	case 'n':
	  uname_flags |= FLAG_NODENAME;
	  break;
	case 'r':
	  uname_flags |= FLAG_RELEASE;
	  break;
	case 's':
	  uname_flags |= FLAG_SYSNAME;
	  break;
	case 'v':
	  uname_flags |= FLAG_VERSION;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (list)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (uname_flags == 0)
    uname_flags = FLAG_SYSNAME;

  /* Only ancient systems will not have uname(2). */
#ifdef HAVE_UNAME
  if (uname (&uninfo) < 0)
    {
      builtin_error ("cannot get system name: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }
#else
  builtin_error ("cannot get system information: uname(2) not available");
  return (EXECUTION_FAILURE);
#endif

  uprint (FLAG_SYSNAME, uninfo.sysname);
  uprint (FLAG_NODENAME, uninfo.nodename);
  uprint (FLAG_RELEASE, uninfo.release);
  uprint (FLAG_VERSION, uninfo.version);
  uprint (FLAG_MACHINE, uninfo.machine);

  return (EXECUTION_SUCCESS);
}

static void
uprint (flag, info)
     int flag;
     char *info;
{
  if (uname_flags & flag)
    {
      uname_flags &= ~flag;
      printf ("%s%c", info, uname_flags ? ' ' : '\n');
    }
}

char *uname_doc[] = {
	"Display system information.",
	"",
	"Display information about the system hardware and OS.",
	(char *)NULL
};

struct builtin uname_struct = {
	"uname",
	uname_builtin,
	BUILTIN_ENABLED,
	uname_doc,
	"uname [-amnrsv]",
	0
};
