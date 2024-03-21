/* dsv - process a line of delimiter-separated data and populate an indexed
	 array with the fields */

/*
   Copyright (C) 2022 Free Software Foundation, Inc.

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

#define DSV_ARRAY_DEFAULT	"DSV"

#define NQUOTE	0
#define DQUOTE	1
#define SQUOTE	2

#define F_SHELLQUOTE	0x01
#define F_GREEDY	0x02
#define F_PRESERVE	0x04

/* Split LINE into delimiter-separated fields, storing each field into a
   separate element of array variable DSV, starting at index 0. The format
   of LINE is delimiter-separated values. By default, this splits lines of
   CSV data as described in RFC 4180. If *DSTRING is any other value than
   ',', this uses that character as a field delimiter. Pass F_SHELLQUOTE in
   FLAGS to understand shell-like double-quoting and backslash-escaping in
   double quotes instead of the "" CSV behavior, and shell-like single quotes.
   Pass F_GREEDY in FLAGS to consume multiple leading and trailing instances
   of *DSTRING and consecutive instances of *DSTRING in LINE without creating
   null fields. If you want to preserve the quote characters in the generated
   fields, pass F_PRESERVE; by default, this removes them. */
static int
dsvsplit (dsv, line, dstring, flags)
     SHELL_VAR *dsv;
     char *line, *dstring;
     int flags;
{
  arrayind_t ind;
  char *field, *prev, *buf, *xbuf;
  int delim, qstate;
  int b, rval;

  xbuf = 0;
  ind = 0;
  field = prev = line;

  /* If we want a greedy split, consume leading instances of *DSTRING */
  if (flags & F_GREEDY)
    {
      while (*prev == *dstring)
	prev++;
      field = prev;
    }

  do
    {
      if (*prev == '"')
	{
	  if (xbuf == 0)
	    xbuf = xmalloc (strlen (prev) + 1);
	  buf = xbuf;
	  b = 0;
	  if (flags & F_PRESERVE)
	    buf[b++] = *prev;
	  qstate = DQUOTE;
	  for (field = ++prev; *field; field++)
	    {
	      if (qstate == DQUOTE && *field == '"' && field[1] == '"' && (flags & F_SHELLQUOTE) == 0)
		buf[b++] = *field++;	/* skip double quote */
	      else if (qstate == DQUOTE && (flags & F_SHELLQUOTE) && *field == '\\' && strchr (slashify_in_quotes, field[1]) != 0)
		buf[b++] = *++field;	/* backslash quoted double quote */
	      else if (qstate == DQUOTE && *field == '"')
		{
		  qstate = NQUOTE;
		  if (flags & F_PRESERVE)
		    buf[b++] = *field;
		}
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
      else if ((flags & F_SHELLQUOTE) && *prev == '\'')
	{
	  if (xbuf == 0)
	    xbuf = xmalloc (strlen (prev) + 1);
	  buf = xbuf;
	  b = 0;
	  if (flags & F_PRESERVE)
	    buf[b++] = *prev;
	  qstate = SQUOTE;
	  for (field = ++prev; *field; field++)
	    {
	      if (qstate == SQUOTE && *field == '\'')
		{
		  qstate = NQUOTE;
	  	  if (flags & F_PRESERVE)
		    buf[b++] = *field;
		}
	      else if (qstate == NQUOTE && *field == *dstring)
		break;
	      else
		/* This copies any text between a closing single quote and the
		   delimiter. If you want to change that, make sure to do the
		   copy only if qstate == SQUOTE. */
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

      if ((flags & F_GREEDY) == 0 || buf[0])
	{
	  bind_array_element (dsv, ind, buf, 0);
	  ind++;
	}

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
dsv_builtin (list)
     WORD_LIST *list;
{
  int opt, rval, flags;
  char *array_name, *dsvstring, *delims;
  SHELL_VAR *v;

  array_name = 0;
  rval = EXECUTION_SUCCESS;

  delims = ",";
  flags = 0;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "a:d:Sgp")) != -1)
    {
      switch (opt)
	{
	case 'a':
	  array_name = list_optarg;
	  break;
	case 'd':
	  delims = list_optarg;
	  break;
	case 'S':
	  flags |= F_SHELLQUOTE;
	  break;
	case 'g':
	  flags |= F_GREEDY;
	  break;
	case 'p':
	  flags |= F_PRESERVE;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (array_name == 0)
    array_name = DSV_ARRAY_DEFAULT;

  if (legal_identifier (array_name) == 0)
    {
      sh_invalidid (array_name);
      return (EXECUTION_FAILURE);
    }

  if (list == 0)
    {
      builtin_error ("dsv string argument required");
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

  dsvstring = list->word->word;

  if (dsvstring == 0 || *dsvstring == 0)
    return (EXECUTION_SUCCESS);

  opt = dsvsplit (v, dsvstring, delims, flags);
  /* Maybe do something with OPT here, it's the number of fields */

  return (rval);
}

/* Called when builtin is enabled and loaded from the shared object.  If this
   function returns 0, the load fails. */
int
dsv_builtin_load (name)
     char *name;
{
  return (1);
}

/* Called when builtin is disabled. */
void
dsv_builtin_unload (name)
     char *name;
{
}

char *dsv_doc[] = {
	"Read delimiter-separated fields from STRING.",
	"",	
	"Parse STRING, a line of delimiter-separated values, into individual",
	"fields, and store them into the indexed array ARRAYNAME starting at",
	"index 0. The parsing understands and skips over double-quoted strings. ",
	"If ARRAYNAME is not supplied, \"DSV\" is the default array name.",
	"If the delimiter is a comma, the default, this parses comma-",
	"separated values as specified in RFC 4180.",
	"",
	"The -d option specifies the delimiter. The delimiter is the first",
	"character of the DELIMS argument. Specifying a DELIMS argument that",
	"contains more than one character is not supported and will produce",
	"unexpected results. The -S option enables shell-like quoting: double-",
	"quoted strings can contain backslashes preceding special characters,",
	"and the backslash will be removed; and single-quoted strings are",
	"processed as the shell would process them. The -g option enables a",
	"greedy split: sequences of the delimiter are skipped at the beginning",
	"and end of STRING, and consecutive instances of the delimiter in STRING",
	"do not generate empty fields. If the -p option is supplied, dsv leaves",
	"quote characters as part of the generated field; otherwise they are",
	"removed.",
	"",
	"The return value is 0 unless an invalid option is supplied or the ARRAYNAME",
	"argument is invalid or readonly.",
	(char *)NULL
};

struct builtin dsv_struct = {
	"dsv",			/* builtin name */
	dsv_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	dsv_doc,		/* array of long documentation strings. */
	"dsv [-a ARRAYNAME] [-d DELIMS] [-Sgp] string",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
