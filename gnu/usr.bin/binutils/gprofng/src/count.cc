/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <i18n.h>
#include <Elf.h>
#include <collctrl.h>
#include <StringBuilder.h>
#include "collect.h"

/* get_count_data -- format exec of bit to do the real work */
void
collect::get_count_data ()
{
  char command[8192];
  char *s;
  struct stat statbuf;

  // reserve space for original args, plus 30 arguments to bit
  nargs = origargc + 30;
  char **narglist = (char **) calloc (nargs, sizeof (char *));
  arglist = narglist;

  // construct the command for bit
  snprintf (command, sizeof (command), NTXT ("%s"), run_dir);
  s = strstr_r (command, NTXT ("/bin"));
  if (s != NULL)
    {
      // build command line for launching it
      snprintf (s, sizeof (command) - (s - command), NTXT ("/lib/compilers/bit"));
      if (stat (command, &statbuf) == -1)
	{
	  // if bit command does not exist there
	  char *first_look = strdup (command);
	  snprintf (command, sizeof (command), NTXT ("%s"), run_dir);
	  s = strstr (command, NTXT ("/bin"));
	  snprintf (s, sizeof (command) - (s - command), NTXT ("/prod/bin/bit"));
	  if (stat (command, &statbuf) == -1)
	    {
	      // if bit command does not exist
	      dbe_write (2, GTXT ("bit is not installed as `%s' or `%s'\nNo experiment is possible\n"), first_look, command);
	      exit (2);
	    }
	  free (first_look);
	}
      *arglist++ = strdup (command);
    }
  else
    {
      dbe_write (2, GTXT ("collect can't find install bin directory\n"));
      exit (1);
    }

  // Tell it to collect data
  *arglist++ = NTXT ("collect");

  // add the flag for real-data vs. static data
  switch (cc->get_count ())
    {
    case -1:
      *arglist++ = NTXT ("-i");
      *arglist++ = NTXT ("static");
      *arglist++ = NTXT ("-M");
      break;
    case 1:
      *arglist++ = NTXT ("-M");
      *arglist++ = NTXT ("-u");
      break;
    default:
      abort ();
    }

  // tell bit to produce an experiment
  *arglist++ = NTXT ("-e");

  // now copy an edited list of collect options to the arglist
  char **oargv = origargv;

  // skip the "collect"
  oargv++;
  int argc = 1;
  while (argc != targ_index)
    {
      char *p = *oargv;
      switch (p[1])
	{
	  // pass these arguments along, with parameter
	case 'o':
	case 'd':
	case 'g':
	case 'A':
	case 'C':
	case 'O':
	case 'N':
	  *arglist++ = *oargv++;
	  *arglist++ = *oargv++;
	  argc = argc + 2;
	  break;
	case 'I':
	  *arglist++ = *oargv++; // set the -I flag
	  *arglist++ = *oargv; // and the directory name
	  *arglist++ = NTXT ("-d"); // and the -d flag
	  *arglist++ = *oargv++; // to the same directory name
	  argc = argc + 2;
	  break;
	case 'n':
	case 'v':
	  // pass these arguments along as is
	  *arglist++ = *oargv++;
	  argc = argc + 1;
	  break;
	case 'x':
	  // skip one argument
	  oargv++;
	  argc++;
	  break;
	case 'c':
	case 'L':
	case 'y':
	case 'l':
	case 'F':
	case 'j':
	case 'J':
	case 'p':
	case 's':
	case 'h':
	case 'S':
	case 'm':
	case 'M':
	case 'H':
	case 'r':
	case 'i':
	  // skip two arguments
	  oargv++;
	  oargv++;
	  argc = argc + 2;
	  break;
	case 'R':
	case 'Z':
	default:
	  // these should never get this far
	  dbe_write (2, GTXT ("unexpected argument %s\n"), p);
	  abort ();
	}
    }

  // now copy the target and its arguments
  if (access (prog_name, X_OK) != 0)    // not found
    *arglist++ = *oargv++;
  else
    {
      oargv++;
      *arglist++ = prog_name;
    }
  while (*oargv != NULL)
    *arglist++ = *oargv++;

  /* now we have the full argument list composed; if verbose, print it */
  if ((verbose == 1) || (disabled))
    {
      /* describe the experiment */
      char *ccret = cc->show (0);
      if (ccret != NULL)
	{
	  writeStr (2, ccret);
	  free (ccret);
	}
      ccret = cc->show_expt ();
      if (ccret != NULL)
	{
	  /* write this to stdout */
	  writeStr (1, ccret);
	  free (ccret);
	}
      /* print the arguments to bit */
      arglist = narglist;
      StringBuilder sb;
      sb.append (NTXT ("Exec argv[] = "));
      for (int ret = 0; ret < nargs; ret++)
	{
	  if (narglist[ret] == NULL)
	    break;
	  if (ret > 0)
	    sb.append (NTXT (" "));
	  sb.append (narglist[ret]);
	}
      sb.append (NTXT ("\n\n"));
      write (2, sb.toString (), sb.length ());
    }

  /* check for dry run */
  if (disabled)
    exit (0);

  /* ensure original outputs restored for target */
  reset_output ();

  /* now exec the bit to instrument and run the target ... */
  // (void) execve( *narglist, narglist, origenvp);
  (void) execvp (*narglist, narglist);

  /* exec failed; no experiment to delete */
  /* restore output for collector */
  set_output ();
  char *em = strerror (errno);
  if (em == NULL)
    dbe_write (2, GTXT ("execve of %s failed: errno = %d\n"), narglist[0], errno);
  else
    dbe_write (2, GTXT ("execve of %s failed: %s\n"), narglist[0], em);
  exit (1);
}
