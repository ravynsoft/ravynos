/* mksignames.c -- Create and write `signames.h', which contains an array of
   signal names. */

/* Copyright (C) 1992-2020 Free Software Foundation, Inc.

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

#include <sys/types.h>
#include <signal.h>

#include <stdio.h>
#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* HAVE_STDLIB_H */

/* Duplicated from signames.c */
#if !defined (NSIG)
#  define NSIG 64
#endif

#define LASTSIG NSIG+2

/* Imported from signames.c */
extern void initialize_signames ();
extern char *signal_names[];

char *progname;

void
write_signames (stream)
     FILE *stream;
{
  register int i;

  fprintf (stream, "/* This file was automatically created by %s.\n",
	   progname);
  fprintf (stream, "   Do not edit.  Edit support/mksignames.c instead. */\n\n");
  fprintf (stream,
	   "/* A translation list so we can be polite to our users. */\n");
#if defined (CROSS_COMPILING)
  fprintf (stream, "extern char *signal_names[];\n\n");
  fprintf (stream, "extern void initialize_signames PARAMS((void));\n\n");
#else
  fprintf (stream, "char *signal_names[NSIG + 4] = {\n");

  for (i = 0; i <= LASTSIG; i++)
    fprintf (stream, "    \"%s\",\n", signal_names[i]);

  fprintf (stream, "    (char *)0x0\n");
  fprintf (stream, "};\n\n");
  fprintf (stream, "#define initialize_signames()\n\n");
#endif
}

int
main (argc, argv)
     int argc;
     char **argv;
{
  char *stream_name;
  FILE *stream;

  progname = argv[0];

  if (argc == 1)
    {
      stream_name = "stdout";
      stream = stdout;
    }
  else if (argc == 2)
    {
      stream_name = argv[1];
      stream = fopen (stream_name, "w");
    }
  else
    {
      fprintf (stderr, "Usage: %s [output-file]\n", progname);
      exit (1);
    }

  if (!stream)
    {
      fprintf (stderr, "%s: %s: cannot open for writing\n",
	       progname, stream_name);
      exit (2);
    }

#if !defined (CROSS_COMPILING)
  initialize_signames ();
#endif
  write_signames (stream);
  exit (0);
}
