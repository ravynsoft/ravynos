/* rddbg.c -- Read debugging information into a generic form.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */


/* This file reads debugging information into a generic form.  This
   file knows how to dig the debugging information out of an object
   file.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "bucomm.h"
#include "debug.h"
#include "budbg.h"

static bool read_section_stabs_debugging_info
  (bfd *, asymbol **, long, void *, bool *);
static bool read_symbol_stabs_debugging_info
  (bfd *, asymbol **, long, void *, bool *);
static void save_stab (int, int, bfd_vma, const char *);
static void stab_context (void);
static void free_saved_stabs (void);

/* Read debugging information from a BFD.  Returns a generic debugging
   pointer.  */

void *
read_debugging_info (bfd *abfd, asymbol **syms, long symcount,
		     bool no_messages)
{
  void *dhandle;
  bool found;

  dhandle = debug_init (abfd);
  if (dhandle == NULL)
    return NULL;

  if (!debug_set_filename (dhandle, bfd_get_filename (abfd)))
    return NULL;

  if (! read_section_stabs_debugging_info (abfd, syms, symcount, dhandle,
					   &found))
    return NULL;

  if (bfd_get_flavour (abfd) == bfd_target_aout_flavour)
    {
      if (! read_symbol_stabs_debugging_info (abfd, syms, symcount, dhandle,
					      &found))
	return NULL;
    }

  /* Try reading the COFF symbols if we didn't find any stabs in COFF
     sections.  */
  if (! found
      && bfd_get_flavour (abfd) == bfd_target_coff_flavour
      && symcount > 0)
    {
      if (! parse_coff (abfd, syms, symcount, dhandle))
	return NULL;
      found = true;
    }

  if (! found)
    {
      if (! no_messages)
	non_fatal (_("%s: no recognized debugging information"),
		   bfd_get_filename (abfd));
      return NULL;
    }

  return dhandle;
}

/* Read stabs in sections debugging information from a BFD.  */

static bool
read_section_stabs_debugging_info (bfd *abfd, asymbol **syms, long symcount,
				   void *dhandle, bool *pfound)
{
  static struct
    {
      const char *secname;
      const char *strsecname;
    }
  names[] =
    {
      { ".stab", ".stabstr" },
      { "LC_SYMTAB.stabs", "LC_SYMTAB.stabstr" },
      { "$GDB_SYMBOLS$", "$GDB_STRINGS$" }
    };
  unsigned int i;
  void *shandle;
  bool ret = false;

  *pfound = false;
  shandle = NULL;

  for (i = 0; i < sizeof names / sizeof names[0]; i++)
    {
      asection *sec, *strsec;

      sec = bfd_get_section_by_name (abfd, names[i].secname);
      strsec = bfd_get_section_by_name (abfd, names[i].strsecname);
      if (sec != NULL
	  && (bfd_section_flags (sec) & SEC_HAS_CONTENTS) != 0
	  && bfd_section_size (sec) >= 12
	  && strsec != NULL
	  && (bfd_section_flags (strsec) & SEC_HAS_CONTENTS) != 0)
	{
	  bfd_size_type stabsize, strsize;
	  bfd_byte *stabs, *strings;
	  bfd_byte *stab;
	  bfd_size_type stroff, next_stroff;

	  if (!bfd_malloc_and_get_section (abfd, sec, &stabs))
	    {
	      fprintf (stderr, "%s: %s: %s\n",
		       bfd_get_filename (abfd), names[i].secname,
		       bfd_errmsg (bfd_get_error ()));
	      goto out;
	    }

	  if (!bfd_malloc_and_get_section (abfd, strsec, &strings))
	    {
	      fprintf (stderr, "%s: %s: %s\n",
		       bfd_get_filename (abfd), names[i].strsecname,
		       bfd_errmsg (bfd_get_error ()));
	      free (stabs);
	      goto out;
	    }
	  /* Zero terminate the strings table, just in case.  */
	  strsize = bfd_section_size (strsec);
	  if (strsize != 0)
	    strings [strsize - 1] = 0;
	  if (shandle == NULL)
	    {
	      shandle = start_stab (dhandle, abfd, true, syms, symcount);
	      if (shandle == NULL)
		{
		  free (strings);
		  free (stabs);
		  goto out;
		}
	    }

	  *pfound = true;

	  stroff = 0;
	  next_stroff = 0;
	  stabsize = bfd_section_size (sec);
	  /* PR 17512: file: 078-60391-0.001:0.1.  */
	  for (stab = stabs; stab <= (stabs + stabsize) - 12; stab += 12)
	    {
	      unsigned int strx;
	      int type;
	      int other ATTRIBUTE_UNUSED;
	      int desc;
	      bfd_vma value;

	      /* This code presumes 32 bit values.  */

	      strx = bfd_get_32 (abfd, stab);
	      type = bfd_get_8 (abfd, stab + 4);
	      other = bfd_get_8 (abfd, stab + 5);
	      desc = bfd_get_16 (abfd, stab + 6);
	      value = bfd_get_32 (abfd, stab + 8);

	      if (type == 0)
		{
		  /* Special type 0 stabs indicate the offset to the
		     next string table.  */
		  stroff = next_stroff;
		  next_stroff += value;
		}
	      else
		{
		  size_t len;
		  char *f, *s;

		  if (stroff + strx >= strsize)
		    {
		      fprintf (stderr, _("%s: %s: stab entry %ld is corrupt, strx = 0x%x, type = %d\n"),
			       bfd_get_filename (abfd), names[i].secname,
			       (long) (stab - stabs) / 12, strx, type);
		      continue;
		    }

		  s = (char *) strings + stroff + strx;
		  f = NULL;

		  /* PR 17512: file: 002-87578-0.001:0.1.
		     It is possible to craft a file where, without the 'strlen (s) > 0',
		     an attempt to read the byte before 'strings' would occur.  */
		  while ((len = strlen (s)) > 0
			 && s[len  - 1] == '\\'
			 && stab + 16 <= stabs + stabsize)
		    {
		      char *p;

		      stab += 12;
		      p = s + len - 1;
		      *p = '\0';
		      strx = stroff + bfd_get_32 (abfd, stab);
		      if (strx >= strsize)
			{
			  fprintf (stderr, _("%s: %s: stab entry %ld is corrupt\n"),
				   bfd_get_filename (abfd), names[i].secname,
				   (long) (stab - stabs) / 12);
			  break;
			}

		      s = concat (s, (char *) strings + strx,
				  (const char *) NULL);

		      /* We have to restore the backslash, because, if
			 the linker is hashing stabs strings, we may
			 see the same string more than once.  */
		      *p = '\\';

		      free (f);
		      f = s;
		    }

		  save_stab (type, desc, value, s);

		  if (!parse_stab (dhandle, shandle, type, desc, value, s))
		    {
		      stab_context ();
		      free_saved_stabs ();
		      free (f);
		      free (stabs);
		      free (strings);
		      goto out;
		    }

		  free (f);
		}
	    }

	  free_saved_stabs ();
	  free (stabs);
	  free (strings);
	}
    }
  ret = true;

 out:
  if (shandle != NULL)
    {
      if (! finish_stab (dhandle, shandle, ret))
	return false;
    }

  return ret;
}

/* Read stabs in the symbol table.  */

static bool
read_symbol_stabs_debugging_info (bfd *abfd, asymbol **syms, long symcount,
				  void *dhandle, bool *pfound)
{
  void *shandle;
  asymbol **ps, **symend;

  shandle = NULL;
  symend = syms + symcount;
  for (ps = syms; ps < symend; ps++)
    {
      symbol_info i;

      bfd_get_symbol_info (abfd, *ps, &i);

      if (i.type == '-')
	{
	  const char *s;
	  char *f;

	  if (shandle == NULL)
	    {
	      shandle = start_stab (dhandle, abfd, false, syms, symcount);
	      if (shandle == NULL)
		return false;
	    }

	  *pfound = true;

	  s = i.name;
	  if (s == NULL || strlen (s) < 1)
	    break;
	  f = NULL;

	  while (strlen (s) > 0
		 && s[strlen (s) - 1] == '\\'
		 && ps + 1 < symend)
	    {
	      char *sc, *n;

	      ++ps;
	      sc = xstrdup (s);
	      sc[strlen (sc) - 1] = '\0';
	      n = concat (sc, bfd_asymbol_name (*ps), (const char *) NULL);
	      free (sc);
	      free (f);
	      f = n;
	      s = n;
	    }

	  save_stab (i.stab_type, i.stab_desc, i.value, s);

	  if (!parse_stab (dhandle, shandle, i.stab_type, i.stab_desc,
			   i.value, s))
	    {
	      stab_context ();
	      free (f);
	      break;
	    }

	  free (f);
	}
    }
  bool ret = ps >= symend;

  free_saved_stabs ();

  if (shandle != NULL)
    {
      if (! finish_stab (dhandle, shandle, ret))
	return false;
    }

  return ret;
}

/* Record stabs strings, so that we can give some context for errors.  */

#define SAVE_STABS_COUNT (16)

struct saved_stab
{
  int type;
  int desc;
  bfd_vma value;
  char *string;
};

static struct saved_stab saved_stabs[SAVE_STABS_COUNT];
static int saved_stabs_index;

/* Save a stabs string.  */

static void
save_stab (int type, int desc, bfd_vma value, const char *string)
{
  free (saved_stabs[saved_stabs_index].string);
  saved_stabs[saved_stabs_index].type = type;
  saved_stabs[saved_stabs_index].desc = desc;
  saved_stabs[saved_stabs_index].value = value;
  saved_stabs[saved_stabs_index].string = xstrdup (string);
  saved_stabs_index = (saved_stabs_index + 1) % SAVE_STABS_COUNT;
}

/* Provide context for an error.  */

static void
stab_context (void)
{
  int i;

  fprintf (stderr, _("Last stabs entries before error:\n"));
  fprintf (stderr, "n_type n_desc n_value  string\n");

  i = saved_stabs_index;
  do
    {
      struct saved_stab *stabp;

      stabp = saved_stabs + i;
      if (stabp->string != NULL)
	{
	  const char *s;

	  s = bfd_get_stab_name (stabp->type);
	  if (s != NULL)
	    fprintf (stderr, "%-6s", s);
	  else if (stabp->type == 0)
	    fprintf (stderr, "HdrSym");
	  else
	    fprintf (stderr, "%-6d", stabp->type);
	  fprintf (stderr, " %-6d ", stabp->desc);
	  fprintf (stderr, "%08" PRIx64, (uint64_t) stabp->value);
	  if (stabp->type != 0)
	    fprintf (stderr, " %s", stabp->string);
	  fprintf (stderr, "\n");
	}
      i = (i + 1) % SAVE_STABS_COUNT;
    }
  while (i != saved_stabs_index);
}

/* Free the saved stab strings.  */

static void
free_saved_stabs (void)
{
  int i;

  for (i = 0; i < SAVE_STABS_COUNT; i++)
    {
      free (saved_stabs[i].string);
      saved_stabs[i].string = NULL;
    }

  saved_stabs_index = 0;
}
