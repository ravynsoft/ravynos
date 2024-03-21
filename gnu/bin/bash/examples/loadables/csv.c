/* csv - process a line of csv data and populate an indexed array with the
	 fields */

/*
   Copyright (C) 2020 Free Software Foundation, Inc.

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

/* See Makefile for compilation details. */

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include "bashansi.h"
#include <stdio.h>

#include "loadables.h"

#define CSV_ARRAY_DEFAULT	"CSV"

#define NQUOTE	0
#define DQUOTE	1

/* Split LINE into comma-separated fields, storing each field into a separate
   element of array variable CSV, starting at index 0. The format of LINE is
   as described in RFC 4180. */
static int
csvsplit (csv, line, dstring)
     SHELL_VAR *csv;
     char *line, *dstring;
{
  arrayind_t ind;
  char *field, *prev, *buf, *xbuf;
  int delim, qstate;
  int b, rval;

  xbuf = 0;
  ind = 0;
  field = prev = line;

  do
    {
      if (*prev == '"')
	{
	  if (xbuf == 0)
	    xbuf = xmalloc (strlen (prev) + 1);
	  buf = xbuf;
	  b = 0;
	  qstate = DQUOTE;
	  for (field = ++prev; *field; field++)
	    {
	      if (qstate == DQUOTE && *field == '"' && field[1] == '"')
		buf[b++] = *field++;	/* skip double quote */
	      else if (qstate == DQUOTE && *field == '"')
	        qstate = NQUOTE;
	      else if (qstate == NQUOTE && *field == *dstring)
		break;
	      else
		/* This copies any text between a closing double quote and the
		   delimiter. If you want to change that, make sure to do the
		   copy only if qstate == DQUOTE. */
		buf[b++] = *field;
	    }
	  buf[b] = '\0';
	}
      else
	{
	  buf = prev;
	  field = prev + strcspn (prev, dstring);
	}

      delim = *field;
      *field = '\0';

      bind_array_element (csv, ind, buf, 0);
      ind++;

      *field = delim;

      if (delim == *dstring)
	prev = field + 1;
    }
  while (delim == *dstring);

  if (xbuf)
    free (xbuf);

  return (rval = ind);				/* number of fields */
}

int
csv_builtin (list)
     WORD_LIST *list;
{
  int opt, rval;
  char *array_name, *csvstring;
  SHELL_VAR *v;

  array_name = 0;
  rval = EXECUTION_SUCCESS;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "a:")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  array_name = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (array_name == 0)
    array_name = CSV_ARRAY_DEFAULT;

  if (legal_identifier (array_name) == 0)
    {
      sh_invalidid (array_name);
      return (EXECUTION_FAILURE);
    }

  if (list == 0)
    {
      builtin_error ("csv string argument required");
      return (EX_USAGE);
    }

  v = find_or_make_array_variable (array_name, 1);
  if (v == 0 || readonly_p (v) || noassign_p (v))
    {
      if (v && readonly_p (v))
	err_readonly (array_name);
      return (EXECUTION_FAILURE);
    }
  else if (array_p (v) == 0)
    {
      builtin_error ("%s: not an indexed array", array_name);
      return (EXECUTION_FAILURE);
    }
  if (invisible_p (v))
    VUNSETATTR (v, att_invisible);
  array_flush (array_cell (v));

  csvstring = list->word->word;

  if (csvstring == 0 || *csvstring == 0)
    return (EXECUTION_SUCCESS);

  opt = csvsplit (v, csvstring, ",");
  /* Maybe do something with OPT here, it's the number of fields */

  return (rval);
}

/* Called when builtin is enabled and loaded from the shared object.  If this
   function returns 0, the load fails. */
int
csv_builtin_load (name)
     char *name;
{
  return (1);
}

/* Called when builtin is disabled. */
void
csv_builtin_unload (name)
     char *name;
{
}

char *csv_doc[] = {
	"Read comma-separated fields from a string.",
	"",
	"Parse STRING, a line of comma-separated values, into individual fields,",
	"and store them into the indexed array ARRAYNAME starting at index 0.",
	"If ARRAYNAME is not supplied, \"CSV\" is the default array name.",
	(char *)NULL
};

struct builtin csv_struct = {
	"csv",			/* builtin name */
	csv_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	csv_doc,		/* array of long documentation strings. */
	"csv [-a ARRAY] string",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
