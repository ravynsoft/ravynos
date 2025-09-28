/* gen-helpfiles - create files containing builtin help text */

/* Copyright (C) 2012-2021 Free Software Foundation, Inc.

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

/* This links with a specially-generated version of builtins.c and takes
   the long_doc members of each struct builtin element and writes those to
   the file named by the `handle' member of the struct builtin element. */

#if !defined (CROSS_COMPILING) 
#  include <config.h>
#else	/* CROSS_COMPILING */
/* A conservative set of defines based on POSIX/SUS3/XPG6 */
#  define HAVE_UNISTD_H
#  define HAVE_STRING_H
#  define HAVE_STDLIB_H

#  define HAVE_RENAME
#endif /* CROSS_COMPILING */

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#ifndef _MINIX
#  include "../bashtypes.h"
#  if defined (HAVE_SYS_FILE_H)
#    include <sys/file.h>
#  endif
#endif

#include "posixstat.h"
#include "filecntl.h"

#include "../bashansi.h"
#include <stdio.h>
#include <errno.h>

#include "stdc.h"

#include "../builtins.h"
#include "tmpbuiltins.h"

#if defined (USING_BASH_MALLOC)
#undef xmalloc
#undef xrealloc
#undef xfree

#undef malloc
#undef free		/* defined in xmalloc.h */
#endif

#ifndef errno
extern int errno;
#endif

#if !defined (__STDC__) && !defined (strcpy)
extern char *strcpy ();
#endif /* !__STDC__ && !strcpy */

#define whitespace(c) (((c) == ' ') || ((c) == '\t'))

/* Flag values that builtins can have. */
#define BUILTIN_FLAG_SPECIAL	0x01
#define BUILTIN_FLAG_ASSIGNMENT 0x02
#define BUILTIN_FLAG_POSIX_BUILTIN 0x04

#define BASE_INDENT	4

/* Non-zero means to produce separate help files for each builtin, named by
   the builtin name, in `./helpfiles'. */
int separate_helpfiles = 0;

/* Non-zero means to create single C strings for each `longdoc', with
   embedded newlines, for ease of translation. */
int single_longdoc_strings = 1;

/* The name of a directory into which the separate external help files will
   eventually be installed. */
char *helpfile_directory;

/* Forward declarations. */

int write_helpfiles PARAMS((struct builtin *));

/* For each file mentioned on the command line, process it and
   write the information to STRUCTFILE and EXTERNFILE, while
   creating the production file if necessary. */
int
main (argc, argv)
     int argc;
     char **argv;
{
  int arg_index = 1;

  while (arg_index < argc && argv[arg_index][0] == '-')
    {
      char *arg = argv[arg_index++];

      if (strcmp (arg, "-noproduction") == 0)
	;
      else if (strcmp (arg, "-H") == 0)
	helpfile_directory = argv[arg_index++];
      else if (strcmp (arg, "-S") == 0)
	single_longdoc_strings = 0;
      else
	{
	  fprintf (stderr, "%s: Unknown flag %s.\n", argv[0], arg);
	  exit (2);
	}
    }

  write_helpfiles(shell_builtins);

  exit (0);
}

/* Write DOCUMENTATION to STREAM, perhaps surrounding it with double-quotes
   and quoting special characters in the string.  Handle special things for
   internationalization (gettext) and the single-string vs. multiple-strings
   issues. */
void
write_documentation (stream, documentation, indentation)
     FILE *stream;
     char *documentation;
     int indentation;
{
  if (stream == 0)
    return;

  if (documentation)
    fprintf (stream, "%*s%s\n", indentation, " ", documentation);
}

int
write_helpfiles (builtins)
     struct builtin *builtins;
{
  char *helpfile, *bname, *fname;
  FILE *helpfp;
  int i, hdlen;
  struct builtin b;

  i = mkdir ("helpfiles", 0777);
  if (i < 0 && errno != EEXIST)
    {
      fprintf (stderr, "write_helpfiles: helpfiles: cannot create directory\n");
      return -1;
    }

  hdlen = strlen ("helpfiles/");
  for (i = 0; i < num_shell_builtins; i++)
    {
      b = builtins[i];

      fname = (char *)b.handle;
      helpfile = (char *)malloc (hdlen + strlen (fname) + 1);
      if (helpfile == 0)
	{
	  fprintf (stderr, "gen-helpfiles: cannot allocate memory\n");
	  exit (1);
	}
      sprintf (helpfile, "helpfiles/%s", fname);

      helpfp = fopen (helpfile, "w");
      if (helpfp == 0)
	{
	  fprintf (stderr, "write_helpfiles: cannot open %s\n", helpfile);
	  free (helpfile);
	  continue;
	}

      write_documentation (helpfp, b.long_doc[0], 4);

      fflush (helpfp);
      fclose (helpfp);
      free (helpfile);
    }
  return 0;
}
