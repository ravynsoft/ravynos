/* macro.c - macro support for gas
   Copyright (C) 1994-2023 Free Software Foundation, Inc.

   Written by Steve and Judy Chamberlain of Cygnus Support,
      sac@cygnus.com

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "sb.h"
#include "macro.h"

/* The routines in this file handle macro definition and expansion.
   They are called by gas.  */

#define ISWHITE(x) ((x) == ' ' || (x) == '\t')

#define ISSEP(x) \
 ((x) == ' ' || (x) == '\t' || (x) == ',' || (x) == '"' || (x) == ';' \
  || (x) == ')' || (x) == '(' \
  || ((flag_macro_alternate || flag_mri) && ((x) == '<' || (x) == '>')))

#define ISBASE(x) \
  ((x) == 'b' || (x) == 'B' \
   || (x) == 'q' || (x) == 'Q' \
   || (x) == 'h' || (x) == 'H' \
   || (x) == 'd' || (x) == 'D')

/* The macro hash table.  */

htab_t macro_hash;

/* Whether any macros have been defined.  */

int macro_defined;

/* Whether we should strip '@' characters.  */

#define macro_strip_at false

/* Number of macro expansions that have been done.  */

static int macro_number;

static void free_macro (macro_entry *);

static void
macro_del_f (void *ent)
{
  string_tuple_t *tuple = ent;
  free_macro ((macro_entry *) tuple->value);
}

/* Initialize macro processing.  */

void
macro_init (void)
{
  macro_hash = htab_create_alloc (16, hash_string_tuple, eq_string_tuple,
				  macro_del_f, notes_calloc, NULL);
  macro_defined = 0;
}

void
macro_end (void)
{
  htab_delete (macro_hash);
}

/* Read input lines till we get to a TO string.
   Increase nesting depth if we get a FROM string.
   Put the results into sb at PTR.
   FROM may be NULL (or will be ignored) if TO is "ENDR".
   Add a new input line to an sb using GET_LINE.
   Return 1 on success, 0 on unexpected EOF.  */

int
buffer_and_nest (const char *from, const char *to, sb *ptr,
		 size_t (*get_line) (sb *))
{
  size_t from_len;
  size_t to_len = strlen (to);
  int depth = 1;
  size_t line_start, more;

  if (to_len == 4 && strcasecmp (to, "ENDR") == 0)
    {
      from = NULL;
      from_len = 0;
    }
  else
    from_len = strlen (from);

  /* Record the present source position, such that diagnostics and debug info
     can be properly associated with the respective original lines, rather
     than with the line of the ending directive (TO).  */
  {
    unsigned int line;
    char *linefile;

    as_where_top (&line);
    if (!flag_m68k_mri)
      linefile = xasprintf ("\t.linefile %u .", line + 1);
    else
      linefile = xasprintf ("\tlinefile %u .", line + 1);
    sb_add_string (ptr, linefile);
    xfree (linefile);
  }

  line_start = ptr->len;
  more = get_line (ptr);
  while (more)
    {
      /* Try to find the first pseudo op on the line.  */
      size_t i = line_start;
      bool had_colon = false;

      /* With normal syntax we can suck what we want till we get
	 to the dot.  With the alternate, labels have to start in
	 the first column, since we can't tell what's a label and
	 what's a pseudoop.  */

      if (! LABELS_WITHOUT_COLONS)
	{
	  /* Skip leading whitespace.  */
	  while (i < ptr->len && ISWHITE (ptr->ptr[i]))
	    i++;
	}

      for (;;)
	{
	  /* Skip over a label, if any.  */
	  if (i >= ptr->len || ! is_name_beginner (ptr->ptr[i]))
	    break;
	  i++;
	  while (i < ptr->len && is_part_of_name (ptr->ptr[i]))
	    i++;
	  if (i < ptr->len && is_name_ender (ptr->ptr[i]))
	    i++;
	  /* Skip whitespace.  */
	  while (i < ptr->len && ISWHITE (ptr->ptr[i]))
	    i++;
	  /* Check for the colon.  */
	  if (i >= ptr->len || ptr->ptr[i] != ':')
	    {
	      /* LABELS_WITHOUT_COLONS doesn't mean we cannot have a
		 colon after a label.  If we do have a colon on the
		 first label then handle more than one label on the
		 line, assuming that each label has a colon.  */
	      if (LABELS_WITHOUT_COLONS && !had_colon)
		break;
	      i = line_start;
	      break;
	    }
	  i++;
	  line_start = i;
	  had_colon = true;
	}

      /* Skip trailing whitespace.  */
      while (i < ptr->len && ISWHITE (ptr->ptr[i]))
	i++;

      if (i < ptr->len && (ptr->ptr[i] == '.'
			   || NO_PSEUDO_DOT
			   || flag_mri))
	{
	  if (! flag_m68k_mri && ptr->ptr[i] == '.')
	    i++;
	  size_t len = ptr->len - i;
	  if (from == NULL)
	    {
	      if (len >= 5 && strncasecmp (ptr->ptr + i, "IREPC", 5) == 0)
		from_len = 5;
	      else if (len >= 4 && strncasecmp (ptr->ptr + i, "IREP", 4) == 0)
		from_len = 4;
	      else if (len >= 4 && strncasecmp (ptr->ptr + i, "IRPC", 4) == 0)
		from_len = 4;
	      else if (len >= 4 && strncasecmp (ptr->ptr + i, "REPT", 4) == 0)
		from_len = 4;
	      else if (len >= 3 && strncasecmp (ptr->ptr + i, "IRP", 3) == 0)
		from_len = 3;
	      else if (len >= 3 && strncasecmp (ptr->ptr + i, "REP", 3) == 0)
		from_len = 3;
	      else
		from_len = 0;
	    }
	  if ((from != NULL
	       ? (len >= from_len
		  && strncasecmp (ptr->ptr + i, from, from_len) == 0)
	       : from_len > 0)
	      && (len == from_len
		  || ! (is_part_of_name (ptr->ptr[i + from_len])
			|| is_name_ender (ptr->ptr[i + from_len]))))
	    depth++;
	  if (len >= to_len
	      && strncasecmp (ptr->ptr + i, to, to_len) == 0
	      && (len == to_len
		  || ! (is_part_of_name (ptr->ptr[i + to_len])
			|| is_name_ender (ptr->ptr[i + to_len]))))
	    {
	      depth--;
	      if (depth == 0)
		{
		  /* Reset the string to not include the ending rune.  */
		  ptr->len = line_start;
		  break;
		}
	    }

	  /* PR gas/16908
	     Apply .linefile directives that appear within the macro, alongside
	     keeping them for later expansion of the macro.  */
	  if (from != NULL && strcasecmp (from, "MACRO") == 0
	      && len >= 8 && strncasecmp (ptr->ptr + i, "linefile", 8) == 0)
	    {
	      sb_add_char (ptr, more);
	      temp_ilp (sb_terminate (ptr) + i + 8);
	      s_linefile (0);
	      restore_ilp ();
	      line_start = ptr->len;
	      more = get_line (ptr);
	      continue;
	    }
	}

      /* Add the original end-of-line char to the end and keep running.  */
      sb_add_char (ptr, more);
      line_start = ptr->len;
      more = get_line (ptr);
    }

  /* Return 1 on success, 0 on unexpected EOF.  */
  return depth == 0;
}

/* Pick up a token.  */

static size_t
get_token (size_t idx, sb *in, sb *name)
{
  if (idx < in->len
      && is_name_beginner (in->ptr[idx]))
    {
      sb_add_char (name, in->ptr[idx++]);
      while (idx < in->len
	     && is_part_of_name (in->ptr[idx]))
	{
	  sb_add_char (name, in->ptr[idx++]);
	}
      if (idx < in->len
	     && is_name_ender (in->ptr[idx]))
	{
	  sb_add_char (name, in->ptr[idx++]);
	}
    }
  /* Ignore trailing &.  */
  if (flag_macro_alternate && idx < in->len && in->ptr[idx] == '&')
    idx++;
  return idx;
}

/* Pick up a string.  */

static size_t
getstring (size_t idx, sb *in, sb *acc)
{
  while (idx < in->len
	 && (in->ptr[idx] == '"'
	     || (in->ptr[idx] == '<' && (flag_macro_alternate || flag_mri))
	     || (in->ptr[idx] == '\'' && flag_macro_alternate)))
    {
      if (in->ptr[idx] == '<')
	{
	  int nest = 0;
	  idx++;
	  while (idx < in->len
		 && (in->ptr[idx] != '>' || nest))
	    {
	      if (in->ptr[idx] == '!')
		{
		  idx++;
		  sb_add_char (acc, in->ptr[idx++]);
		}
	      else
		{
		  if (in->ptr[idx] == '>')
		    nest--;
		  if (in->ptr[idx] == '<')
		    nest++;
		  sb_add_char (acc, in->ptr[idx++]);
		}
	    }
	  idx++;
	}
      else if (in->ptr[idx] == '"' || in->ptr[idx] == '\'')
	{
	  char tchar = in->ptr[idx];
	  int escaped = 0;

	  idx++;

	  while (idx < in->len)
	    {
	      if (in->ptr[idx - 1] == '\\')
		escaped ^= 1;
	      else
		escaped = 0;

	      if (flag_macro_alternate && in->ptr[idx] == '!')
		{
		  idx ++;

		  sb_add_char (acc, in->ptr[idx]);

		  idx ++;
		}
	      else if (escaped && in->ptr[idx] == tchar)
		{
		  sb_add_char (acc, tchar);
		  idx ++;
		}
	      else
		{
		  if (in->ptr[idx] == tchar)
		    {
		      idx ++;

		      if (idx >= in->len || in->ptr[idx] != tchar)
			break;
		    }

		  sb_add_char (acc, in->ptr[idx]);
		  idx ++;
		}
	    }
	}
    }

  return idx;
}

/* Fetch string from the input stream,
   rules:
    'Bxyx<whitespace>  	-> return 'Bxyza
    %<expr>		-> return string of decimal value of <expr>
    "string"		-> return string
    (string)		-> return (string-including-whitespaces)
    xyx<whitespace>     -> return xyz.  */

static size_t
get_any_string (size_t idx, sb *in, sb *out)
{
  sb_reset (out);
  idx = sb_skip_white (idx, in);

  if (idx < in->len)
    {
      if (in->len > idx + 2 && in->ptr[idx + 1] == '\'' && ISBASE (in->ptr[idx]))
	{
	  while (idx < in->len && !ISSEP (in->ptr[idx]))
	    sb_add_char (out, in->ptr[idx++]);
	}
      else if (in->ptr[idx] == '%' && flag_macro_alternate)
	{
	  /* Turn the following expression into a string.  */
	  expressionS ex;
	  char buf[64];

	  sb_terminate (in);

	  temp_ilp (in->ptr + idx + 1);
	  expression_and_evaluate (&ex);
	  idx = input_line_pointer - in->ptr;
	  restore_ilp ();

	  if (ex.X_op != O_constant)
	    as_bad (_("%% operator needs absolute expression"));

	  sprintf (buf, "%" PRId64, (int64_t) ex.X_add_number);
	  sb_add_string (out, buf);
	}
      else if (in->ptr[idx] == '"'
	       || (in->ptr[idx] == '<' && (flag_macro_alternate || flag_mri))
	       || (flag_macro_alternate && in->ptr[idx] == '\''))
	{
	  if (flag_macro_alternate && ! macro_strip_at && in->ptr[idx] != '<')
	    {
	      /* Keep the quotes.  */
	      sb_add_char (out, '"');
	      idx = getstring (idx, in, out);
	      sb_add_char (out, '"');
	    }
	  else
	    {
	      idx = getstring (idx, in, out);
	    }
	}
      else
	{
	  char *br_buf = XNEWVEC (char, 1);
	  char *in_br = br_buf;

	  *in_br = '\0';
	  while (idx < in->len
		 && (*in_br
		     || (in->ptr[idx] != ' '
			 && in->ptr[idx] != '\t'))
		 && in->ptr[idx] != ','
		 && (in->ptr[idx] != '<'
		     || (! flag_macro_alternate && ! flag_mri)))
	    {
	      char tchar = in->ptr[idx];

	      switch (tchar)
		{
		case '"':
		case '\'':
		  sb_add_char (out, in->ptr[idx++]);
		  while (idx < in->len
			 && in->ptr[idx] != tchar)
		    sb_add_char (out, in->ptr[idx++]);
		  if (idx == in->len)
		    {
		      free (br_buf);
		      return idx;
		    }
		  break;
		case '(':
		case '[':
		  if (in_br > br_buf)
		    --in_br;
		  else
		    {
		      br_buf = XNEWVEC (char, strlen (in_br) + 2);
		      strcpy (br_buf + 1, in_br);
		      free (in_br);
		      in_br = br_buf;
		    }
		  *in_br = tchar;
		  break;
		case ')':
		  if (*in_br == '(')
		    ++in_br;
		  break;
		case ']':
		  if (*in_br == '[')
		    ++in_br;
		  break;
		}
	      sb_add_char (out, tchar);
	      ++idx;
	    }
	  free (br_buf);
	}
    }

  return idx;
}

/* Allocate a new formal.  */

static formal_entry *
new_formal (void)
{
  formal_entry *formal;

  formal = XNEW (formal_entry);

  sb_new (&formal->name);
  sb_new (&formal->def);
  sb_new (&formal->actual);
  formal->next = NULL;
  formal->type = FORMAL_OPTIONAL;
  return formal;
}

/* Free a formal.  */

static void
del_formal (formal_entry *formal)
{
  sb_kill (&formal->actual);
  sb_kill (&formal->def);
  sb_kill (&formal->name);
  free (formal);
}

/* Pick up the formal parameters of a macro definition.  */

static size_t
do_formals (macro_entry *macro, size_t idx, sb *in)
{
  formal_entry **p = &macro->formals;
  const char *name;

  idx = sb_skip_white (idx, in);
  while (idx < in->len)
    {
      formal_entry *formal = new_formal ();
      size_t cidx;

      idx = get_token (idx, in, &formal->name);
      if (formal->name.len == 0)
	{
	  if (macro->formal_count)
	    --idx;
	  del_formal (formal);	/* 'formal' goes out of scope.  */
	  break;
	}
      idx = sb_skip_white (idx, in);
      /* This is a formal.  */
      name = sb_terminate (&formal->name);
      if (! flag_mri
	  && idx < in->len
	  && in->ptr[idx] == ':'
	  && (! is_name_beginner (':')
	      || idx + 1 >= in->len
	      || ! is_part_of_name (in->ptr[idx + 1])))
	{
	  /* Got a qualifier.  */
	  sb qual;

	  sb_new (&qual);
	  idx = get_token (sb_skip_white (idx + 1, in), in, &qual);
	  sb_terminate (&qual);
	  if (qual.len == 0)
	    as_bad_where (macro->file,
			  macro->line,
			  _("Missing parameter qualifier for `%s' in macro `%s'"),
			  name,
			  macro->name);
	  else if (strcmp (qual.ptr, "req") == 0)
	    formal->type = FORMAL_REQUIRED;
	  else if (strcmp (qual.ptr, "vararg") == 0)
	    formal->type = FORMAL_VARARG;
	  else
	    as_bad_where (macro->file,
			  macro->line,
			  _("`%s' is not a valid parameter qualifier for `%s' in macro `%s'"),
			  qual.ptr,
			  name,
			  macro->name);
	  sb_kill (&qual);
	  idx = sb_skip_white (idx, in);
	}
      if (idx < in->len && in->ptr[idx] == '=')
	{
	  /* Got a default.  */
	  idx = get_any_string (idx + 1, in, &formal->def);
	  idx = sb_skip_white (idx, in);
	  if (formal->type == FORMAL_REQUIRED)
	    {
	      sb_reset (&formal->def);
	      as_warn_where (macro->file,
			    macro->line,
			    _("Pointless default value for required parameter `%s' in macro `%s'"),
			    name,
			    macro->name);
	    }
	}

      /* Add to macro's hash table.  */
      if (str_hash_insert (macro->formal_hash, name, formal, 0) != NULL)
	{
	  as_bad_where (macro->file, macro->line,
			_("A parameter named `%s' "
			  "already exists for macro `%s'"),
			name, macro->name);
	}

      formal->index = macro->formal_count++;
      *p = formal;
      p = &formal->next;
      if (formal->type == FORMAL_VARARG)
	break;
      cidx = idx;
      idx = sb_skip_comma (idx, in);
      if (idx != cidx && idx >= in->len)
	{
	  idx = cidx;
	  break;
	}
    }

  if (flag_mri)
    {
      formal_entry *formal = new_formal ();

      /* Add a special NARG formal, which macro_expand will set to the
	 number of arguments.  */
      /* The same MRI assemblers which treat '@' characters also use
	 the name $NARG.  At least until we find an exception.  */
      if (macro_strip_at)
	name = "$NARG";
      else
	name = "NARG";

      sb_add_string (&formal->name, name);

      /* Add to macro's hash table.  */
      if (str_hash_insert (macro->formal_hash, name, formal, 0) != NULL)
	{
	  as_bad_where (macro->file, macro->line,
			_("Reserved word `%s' used as parameter in macro `%s'"),
			name, macro->name);
	}

      formal->index = NARG_INDEX;
      *p = formal;
    }

  return idx;
}

/* Free the memory allocated to a macro.  */

static void
free_macro (macro_entry *macro)
{
  formal_entry *formal;

  for (formal = macro->formals; formal; )
    {
      formal_entry *f;

      f = formal;
      formal = formal->next;
      del_formal (f);
    }
  htab_delete (macro->formal_hash);
  sb_kill (&macro->sub);
  free ((char *) macro->name);
  free (macro);
}

/* Define a new macro.  */

macro_entry *
define_macro (sb *in, sb *label, size_t (*get_line) (sb *))
{
  macro_entry *macro;
  sb name;
  size_t idx;
  const char *error = NULL;

  macro = XNEW (macro_entry);
  sb_new (&macro->sub);
  sb_new (&name);
  macro->file = as_where (&macro->line);

  macro->formal_count = 0;
  macro->formals = 0;
  macro->formal_hash = str_htab_create ();

  idx = sb_skip_white (0, in);
  if (! buffer_and_nest ("MACRO", "ENDM", &macro->sub, get_line))
    error = _("unexpected end of file in macro `%s' definition");
  if (label != NULL && label->len != 0)
    {
      sb_add_sb (&name, label);
      macro->name = sb_terminate (&name);
      if (idx < in->len && in->ptr[idx] == '(')
	{
	  /* It's the label: MACRO (formals,...)  sort  */
	  idx = do_formals (macro, idx + 1, in);
	  if (idx < in->len && in->ptr[idx] == ')')
	    idx = sb_skip_white (idx + 1, in);
	  else if (!error)
	    error = _("missing `)' after formals in macro definition `%s'");
	}
      else
	{
	  /* It's the label: MACRO formals,...  sort  */
	  idx = do_formals (macro, idx, in);
	}
    }
  else
    {
      size_t cidx;

      idx = get_token (idx, in, &name);
      macro->name = sb_terminate (&name);
      if (name.len == 0)
	error = _("Missing macro name");
      cidx = sb_skip_white (idx, in);
      idx = sb_skip_comma (cidx, in);
      if (idx == cidx || idx < in->len)
	idx = do_formals (macro, idx, in);
      else
	idx = cidx;
    }
  if (!error && idx < in->len)
    error = _("Bad parameter list for macro `%s'");

  /* And stick it in the macro hash table.  */
  for (idx = 0; idx < name.len; idx++)
    name.ptr[idx] = TOLOWER (name.ptr[idx]);
  if (!error)
    {
      if (str_hash_insert (macro_hash, macro->name, macro, 0) != NULL)
	error = _("Macro `%s' was already defined");
    }

  if (!error)
    macro_defined = 1;
  else
    {
      as_bad_where (macro->file, macro->line, error, macro->name);
      free_macro (macro);
      macro = NULL;
    }

  return macro;
}

/* Scan a token, and then skip KIND.  */

static size_t
get_apost_token (size_t idx, sb *in, sb *name, int kind)
{
  idx = get_token (idx, in, name);
  if (idx < in->len
      && in->ptr[idx] == kind
      && (! flag_mri || macro_strip_at)
      && (! macro_strip_at || kind == '@'))
    idx++;
  return idx;
}

/* Substitute the actual value for a formal parameter.  */

static size_t
sub_actual (size_t start, sb *in, sb *t, struct htab *formal_hash,
	    int kind, sb *out, int copyifnotthere)
{
  size_t src;
  formal_entry *ptr;

  src = get_apost_token (start, in, t, kind);
  /* See if it's in the macro's hash table, unless this is
     macro_strip_at and kind is '@' and the token did not end in '@'.  */
  if (macro_strip_at
      && kind == '@'
      && (src == start || in->ptr[src - 1] != '@'))
    ptr = NULL;
  else
    ptr = str_hash_find (formal_hash, sb_terminate (t));
  if (ptr)
    {
      if (ptr->actual.len)
	{
	  sb_add_sb (out, &ptr->actual);
	}
      else
	{
	  sb_add_sb (out, &ptr->def);
	}
    }
  else if (kind == '&')
    {
      /* Doing this permits people to use & in macro bodies.  */
      sb_add_char (out, '&');
      sb_add_sb (out, t);
      if (src != start && in->ptr[src - 1] == '&')
	sb_add_char (out, '&');
    }
  else if (copyifnotthere)
    {
      sb_add_sb (out, t);
    }
  else
    {
      sb_add_char (out, '\\');
      sb_add_sb (out, t);
    }
  return src;
}

/* Expand the body of a macro.  */

static const char *
macro_expand_body (sb *in, sb *out, formal_entry *formals,
		   struct htab *formal_hash, const macro_entry *macro)
{
  sb t;
  size_t src = 0;
  int inquote = 0, macro_line = 0;
  formal_entry *loclist = NULL;
  const char *err = NULL;

  sb_new (&t);

  while (src < in->len && !err)
    {
      if (in->ptr[src] == '&')
	{
	  sb_reset (&t);
	  if (flag_mri)
	    {
	      if (src + 1 < in->len && in->ptr[src + 1] == '&')
		src = sub_actual (src + 2, in, &t, formal_hash, '\'', out, 1);
	      else
		sb_add_char (out, in->ptr[src++]);
	    }
	  else
	    {
	      /* Permit macro parameter substitution delineated with
		 an '&' prefix and optional '&' suffix.  */
	      src = sub_actual (src + 1, in, &t, formal_hash, '&', out, 0);
	    }
	}
      else if (in->ptr[src] == '\\')
	{
	  src++;
	  if (src < in->len && in->ptr[src] == '(')
	    {
	      /* Sub in till the next ')' literally.  */
	      src++;
	      while (src < in->len && in->ptr[src] != ')')
		{
		  sb_add_char (out, in->ptr[src++]);
		}
	      if (src < in->len)
		src++;
	      else if (!macro)
		err = _("missing `)'");
	      else
		as_bad_where (macro->file, macro->line + macro_line, _("missing `)'"));
	    }
	  else if (src < in->len && in->ptr[src] == '@')
	    {
	      /* Sub in the macro invocation number.  */

	      char buffer[12];
	      src++;
	      sprintf (buffer, "%d", macro_number);
	      sb_add_string (out, buffer);
	    }
	  else if (src < in->len && in->ptr[src] == '&')
	    {
	      /* This is a preprocessor variable name, we don't do them
		 here.  */
	      sb_add_char (out, '\\');
	      sb_add_char (out, '&');
	      src++;
	    }
	  else if (flag_mri && src < in->len && ISALNUM (in->ptr[src]))
	    {
	      int ind;
	      formal_entry *f;

	      if (ISDIGIT (in->ptr[src]))
		ind = in->ptr[src] - '0';
	      else if (ISUPPER (in->ptr[src]))
		ind = in->ptr[src] - 'A' + 10;
	      else
		ind = in->ptr[src] - 'a' + 10;
	      ++src;
	      for (f = formals; f != NULL; f = f->next)
		{
		  if (f->index == ind - 1)
		    {
		      if (f->actual.len != 0)
			sb_add_sb (out, &f->actual);
		      else
			sb_add_sb (out, &f->def);
		      break;
		    }
		}
	    }
	  else
	    {
	      sb_reset (&t);
	      src = sub_actual (src, in, &t, formal_hash, '\'', out, 0);
	    }
	}
      else if ((flag_macro_alternate || flag_mri)
	       && is_name_beginner (in->ptr[src])
	       && (! inquote
		   || ! macro_strip_at
		   || (src > 0 && in->ptr[src - 1] == '@')))
	{
	  if (! macro
	      || src + 5 >= in->len
	      || strncasecmp (in->ptr + src, "LOCAL", 5) != 0
	      || ! ISWHITE (in->ptr[src + 5])
	      /* PR 11507: Skip keyword LOCAL if it is found inside a quoted string.  */
	      || inquote)
	    {
	      sb_reset (&t);
	      src = sub_actual (src, in, &t, formal_hash,
				(macro_strip_at && inquote) ? '@' : '\'',
				out, 1);
	    }
	  else
	    {
	      src = sb_skip_white (src + 5, in);
	      while (in->ptr[src] != '\n')
		{
		  const char *name;
		  formal_entry *f = new_formal ();

		  src = get_token (src, in, &f->name);
		  name = sb_terminate (&f->name);
		  if (str_hash_insert (formal_hash, name, f, 0) != NULL)
		    {
		      as_bad_where (macro->file, macro->line + macro_line,
				    _("`%s' was already used as parameter "
				      "(or another local) name"), name);
		      del_formal (f);
		    }
		  else
		    {
		      static int loccnt;
		      char buf[20];

		      f->index = LOCAL_INDEX;
		      f->next = loclist;
		      loclist = f;

		      sprintf (buf, IS_ELF ? ".LL%04x" : "LL%04x", ++loccnt);
		      sb_add_string (&f->actual, buf);
		    }

		  src = sb_skip_comma (src, in);
		}
	    }
	}
      else if (in->ptr[src] == '"'
	       || (flag_mri && in->ptr[src] == '\''))
	{
	  inquote = !inquote;
	  sb_add_char (out, in->ptr[src++]);
	}
      else if (in->ptr[src] == '@' && macro_strip_at)
	{
	  ++src;
	  if (src < in->len
	      && in->ptr[src] == '@')
	    {
	      sb_add_char (out, '@');
	      ++src;
	    }
	}
      else if (flag_mri
	       && in->ptr[src] == '='
	       && src + 1 < in->len
	       && in->ptr[src + 1] == '=')
	{
	  formal_entry *ptr;

	  sb_reset (&t);
	  src = get_token (src + 2, in, &t);
	  ptr = str_hash_find (formal_hash, sb_terminate (&t));
	  if (ptr == NULL)
	    {
	      /* FIXME: We should really return a warning string here,
		 but we can't, because the == might be in the MRI
		 comment field, and, since the nature of the MRI
		 comment field depends upon the exact instruction
		 being used, we don't have enough information here to
		 figure out whether it is or not.  Instead, we leave
		 the == in place, which should cause a syntax error if
		 it is not in a comment.  */
	      sb_add_char (out, '=');
	      sb_add_char (out, '=');
	      sb_add_sb (out, &t);
	    }
	  else
	    {
	      if (ptr->actual.len)
		{
		  sb_add_string (out, "-1");
		}
	      else
		{
		  sb_add_char (out, '0');
		}
	    }
	}
      else
	{
	  if (in->ptr[src] == '\n')
	    ++macro_line;
	  sb_add_char (out, in->ptr[src++]);
	}
    }

  sb_kill (&t);

  while (loclist != NULL)
    {
      formal_entry *f;
      const char *name;

      f = loclist->next;
      name = sb_terminate (&loclist->name);
      str_hash_delete (formal_hash, name);
      del_formal (loclist);
      loclist = f;
    }

  if (!err && (out->len == 0 || out->ptr[out->len - 1] != '\n'))
    sb_add_char (out, '\n');
  return err;
}

/* Assign values to the formal parameters of a macro, and expand the
   body.  */

static const char *
macro_expand (size_t idx, sb *in, macro_entry *m, sb *out)
{
  sb t;
  formal_entry *ptr;
  formal_entry *f;
  int is_keyword = 0;
  int narg = 0;
  const char *err = NULL;

  sb_new (&t);

  /* Reset any old value the actuals may have.  */
  for (f = m->formals; f; f = f->next)
    sb_reset (&f->actual);
  f = m->formals;
  while (f != NULL && f->index < 0)
    f = f->next;

  if (flag_mri)
    {
      /* The macro may be called with an optional qualifier, which may
	 be referred to in the macro body as \0.  */
      if (idx < in->len && in->ptr[idx] == '.')
	{
	  /* The Microtec assembler ignores this if followed by a white space.
	     (Macro invocation with empty extension) */
	  idx++;
	  if (    idx < in->len
		  && in->ptr[idx] != ' '
		  && in->ptr[idx] != '\t')
	    {
	      formal_entry *n = new_formal ();

	      n->index = QUAL_INDEX;

	      n->next = m->formals;
	      m->formals = n;

	      idx = get_any_string (idx, in, &n->actual);
	    }
	}
    }

  /* Peel off the actuals and store them away in the hash tables' actuals.  */
  idx = sb_skip_white (idx, in);
  while (idx < in->len)
    {
      size_t scan;

      /* Look and see if it's a positional or keyword arg.  */
      scan = idx;
      while (scan < in->len
	     && !ISSEP (in->ptr[scan])
	     && !(flag_mri && in->ptr[scan] == '\'')
	     && (!flag_macro_alternate && in->ptr[scan] != '='))
	scan++;
      if (scan < in->len && !flag_macro_alternate && in->ptr[scan] == '=')
	{
	  is_keyword = 1;

	  /* It's OK to go from positional to keyword.  */

	  /* This is a keyword arg, fetch the formal name and
	     then the actual stuff.  */
	  sb_reset (&t);
	  idx = get_token (idx, in, &t);
	  if (idx >= in->len || in->ptr[idx] != '=')
	    {
	      err = _("confusion in formal parameters");
	      break;
	    }

	  /* Lookup the formal in the macro's list.  */
	  ptr = str_hash_find (m->formal_hash, sb_terminate (&t));
	  if (!ptr)
	    {
	      as_bad (_("Parameter named `%s' does not exist for macro `%s'"),
		      t.ptr,
		      m->name);
	      sb_reset (&t);
	      idx = get_any_string (idx + 1, in, &t);
	    }
	  else
	    {
	      /* Insert this value into the right place.  */
	      if (ptr->actual.len)
		{
		  as_warn (_("Value for parameter `%s' of macro `%s' was already specified"),
			   ptr->name.ptr,
			   m->name);
		  sb_reset (&ptr->actual);
		}
	      idx = get_any_string (idx + 1, in, &ptr->actual);
	      if (ptr->actual.len > 0)
		++narg;
	    }
	}
      else
	{
	  if (is_keyword)
	    {
	      err = _("can't mix positional and keyword arguments");
	      break;
	    }

	  if (!f)
	    {
	      formal_entry **pf;
	      int c;

	      if (!flag_mri)
		{
		  err = _("too many positional arguments");
		  break;
		}

	      f = new_formal ();

	      c = -1;
	      for (pf = &m->formals; *pf != NULL; pf = &(*pf)->next)
		if ((*pf)->index >= c)
		  c = (*pf)->index + 1;
	      if (c == -1)
		c = 0;
	      *pf = f;
	      f->index = c;
	    }

	  if (f->type != FORMAL_VARARG)
	    idx = get_any_string (idx, in, &f->actual);
	  else if (idx < in->len)
	    {
	      sb_add_buffer (&f->actual, in->ptr + idx, in->len - idx);
	      idx = in->len;
	    }
	  if (f->actual.len > 0)
	    ++narg;
	  do
	    {
	      f = f->next;
	    }
	  while (f != NULL && f->index < 0);
	}

      if (! flag_mri)
	idx = sb_skip_comma (idx, in);
      else
	{
	  if (idx < in->len && in->ptr[idx] == ',')
	    ++idx;
	  if (idx < in->len && ISWHITE (in->ptr[idx]))
	    break;
	}
    }

  if (! err)
    {
      for (ptr = m->formals; ptr; ptr = ptr->next)
	{
	  if (ptr->type == FORMAL_REQUIRED && ptr->actual.len == 0)
	    as_bad (_("Missing value for required parameter `%s' of macro `%s'"),
		    ptr->name.ptr,
		    m->name);
	}

      if (flag_mri)
	{
	  ptr = str_hash_find (m->formal_hash,
			       macro_strip_at ? "$NARG" : "NARG");
	  if (ptr)
	    {
	      char buffer[20];
	      sprintf (buffer, "%d", narg);
	      sb_add_string (&ptr->actual, buffer);
	    }
	}

      err = macro_expand_body (&m->sub, out, m->formals, m->formal_hash, m);
    }

  /* Discard any unnamed formal arguments.  */
  if (flag_mri)
    {
      formal_entry **pf;

      pf = &m->formals;
      while (*pf != NULL)
	{
	  if ((*pf)->name.len != 0)
	    pf = &(*pf)->next;
	  else
	    {
	      f = (*pf)->next;
	      del_formal (*pf);
	      *pf = f;
	    }
	}
    }

  sb_kill (&t);
  if (!err)
    macro_number++;

  return err;
}

/* Check for a macro.  If one is found, put the expansion into
   *EXPAND.  Return 1 if a macro is found, 0 otherwise.  */

int
check_macro (const char *line, sb *expand,
	     const char **error, macro_entry **info)
{
  const char *s;
  char *copy, *cls;
  macro_entry *macro;
  sb line_sb;

  if (! is_name_beginner (*line)
      && (! flag_mri || *line != '.'))
    return 0;

  s = line + 1;
  while (is_part_of_name (*s))
    ++s;
  if (is_name_ender (*s))
    ++s;

  copy = xmemdup0 (line, s - line);
  for (cls = copy; *cls != '\0'; cls ++)
    *cls = TOLOWER (*cls);

  macro = str_hash_find (macro_hash, copy);
  free (copy);

  if (macro == NULL)
    return 0;

  /* Wrap the line up in an sb.  */
  sb_new (&line_sb);
  while (*s != '\0' && *s != '\n' && *s != '\r')
    sb_add_char (&line_sb, *s++);

  sb_new (expand);
  *error = macro_expand (0, &line_sb, macro, expand);

  sb_kill (&line_sb);

  /* Export the macro information if requested.  */
  if (info)
    *info = macro;

  return 1;
}

/* Delete a macro.  */

void
delete_macro (const char *name)
{
  char *copy;
  size_t i, len;
  macro_entry *macro;

  len = strlen (name);
  copy = XNEWVEC (char, len + 1);
  for (i = 0; i < len; ++i)
    copy[i] = TOLOWER (name[i]);
  copy[i] = '\0';

  macro = str_hash_find (macro_hash, copy);
  if (macro != NULL)
    str_hash_delete (macro_hash, copy);
  else
    as_warn (_("Attempt to purge non-existing macro `%s'"), copy);
  free (copy);
}

/* Handle the MRI IRP and IRPC pseudo-ops.  These are handled as a
   combined macro definition and execution.  This returns NULL on
   success, or an error message otherwise.  */

const char *
expand_irp (int irpc, size_t idx, sb *in, sb *out, size_t (*get_line) (sb *))
{
  sb sub;
  formal_entry f;
  struct htab *h;
  const char *err = NULL;

  idx = sb_skip_white (idx, in);

  sb_new (&sub);
  if (! buffer_and_nest (NULL, "ENDR", &sub, get_line))
    {
      err = _("unexpected end of file in irp or irpc");
      goto out2;
    }

  sb_new (&f.name);
  sb_new (&f.def);
  sb_new (&f.actual);

  idx = get_token (idx, in, &f.name);
  if (f.name.len == 0)
    {
      err = _("missing model parameter");
      goto out1;
    }

  h = str_htab_create ();

  str_hash_insert (h, sb_terminate (&f.name), &f, 0);

  f.index = 1;
  f.next = NULL;
  f.type = FORMAL_OPTIONAL;

  sb_reset (out);

  idx = sb_skip_comma (idx, in);
  if (idx >= in->len)
    {
      /* Expand once with a null string.  */
      err = macro_expand_body (&sub, out, &f, h, 0);
    }
  else
    {
      bool in_quotes = false;

      if (irpc && in->ptr[idx] == '"')
	{
	  in_quotes = true;
	  ++idx;
	}

      while (idx < in->len)
	{
	  if (!irpc)
	    idx = get_any_string (idx, in, &f.actual);
	  else
	    {
	      if (in->ptr[idx] == '"')
		{
		  size_t nxt;

		  if (irpc)
		    in_quotes = ! in_quotes;

		  nxt = sb_skip_white (idx + 1, in);
		  if (nxt >= in->len)
		    {
		      idx = nxt;
		      break;
		    }
		}
	      sb_reset (&f.actual);
	      sb_add_char (&f.actual, in->ptr[idx]);
	      ++idx;
	    }

	  err = macro_expand_body (&sub, out, &f, h, 0);
	  if (err != NULL)
	    break;
	  if (!irpc)
	    idx = sb_skip_comma (idx, in);
	  else if (! in_quotes)
	    idx = sb_skip_white (idx, in);
	}
    }

  htab_delete (h);
 out1:
  sb_kill (&f.actual);
  sb_kill (&f.def);
  sb_kill (&f.name);
 out2:
  sb_kill (&sub);

  return err;
}
