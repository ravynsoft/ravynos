/* Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Tell glibc's <string.h> to provide a prototype for stpcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#if defined _LIBC
# include <argz.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>
#if defined _WIN32 && !defined __CYGWIN__
# include <wchar.h>
#endif

#include "loadinfo.h"

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

/* @@ end of prolog @@ */

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# ifndef stpcpy
#  define stpcpy(dest, src) __stpcpy(dest, src)
# endif
#else
# ifndef HAVE_STPCPY
static char *stpcpy (char *dest, const char *src);
# endif
#endif

#ifdef _LIBC
# define IS_ABSOLUTE_FILE_NAME(P) ((P)[0] == '/')
# define IS_RELATIVE_FILE_NAME(P) (! IS_ABSOLUTE_FILE_NAME (P))
#else
# include "filename.h"
#endif

/* Return number of bits set in X.  */
#ifndef ARCH_POP
static inline int
pop (int x)
{
  /* We assume that no more than 16 bits are used.  */
  x = ((x & ~0x5555) >> 1) + (x & 0x5555);
  x = ((x & ~0x3333) >> 2) + (x & 0x3333);
  x = ((x >> 4) + x) & 0x0f0f;
  x = ((x >> 8) + x) & 0xff;

  return x;
}
#endif


struct loaded_l10nfile *
_nl_make_l10nflist (struct loaded_l10nfile **l10nfile_list,
		    const char *dirlist, size_t dirlist_len,
#if defined _WIN32 && !defined __CYGWIN__
		    const wchar_t *wdirlist, size_t wdirlist_len,
#endif
		    int mask, const char *language, const char *territory,
		    const char *codeset, const char *normalized_codeset,
		    const char *modifier,
		    const char *filename, int do_allocate)
{
  char *abs_filename;
#if defined _WIN32 && !defined __CYGWIN__
  wchar_t *abs_wfilename;
#endif
  struct loaded_l10nfile **lastp;
  struct loaded_l10nfile *retval;
  size_t dirlist_count;
  size_t entries;
  int cnt;

  /* If LANGUAGE contains an absolute directory specification, we ignore
     DIRLIST and WDIRLIST.  */
  if (!IS_RELATIVE_FILE_NAME (language))
    {
      dirlist_len = 0;
#if defined _WIN32 && !defined __CYGWIN__
      wdirlist_len = 0;
#endif
    }

  /* Allocate room for the full file name.  */
  abs_filename = (char *) malloc (dirlist_len
				  + strlen (language)
				  + ((mask & XPG_TERRITORY) != 0
				     ? strlen (territory) + 1 : 0)
				  + ((mask & XPG_CODESET) != 0
				     ? strlen (codeset) + 1 : 0)
				  + ((mask & XPG_NORM_CODESET) != 0
				     ? strlen (normalized_codeset) + 1 : 0)
				  + ((mask & XPG_MODIFIER) != 0
				     ? strlen (modifier) + 1 : 0)
				  + 1 + strlen (filename) + 1);

  if (abs_filename == NULL)
    return NULL;

  /* Construct file name.  */
  {
    char *cp;

    cp = abs_filename;
    if (dirlist_len > 0)
      {
	memcpy (cp, dirlist, dirlist_len);
#ifdef _LIBC
	__argz_stringify (cp, dirlist_len, PATH_SEPARATOR);
#endif
	cp += dirlist_len;
	cp[-1] = '/';
      }

    cp = stpcpy (cp, language);

    if ((mask & XPG_TERRITORY) != 0)
      {
	*cp++ = '_';
	cp = stpcpy (cp, territory);
      }
    if ((mask & XPG_CODESET) != 0)
      {
	*cp++ = '.';
	cp = stpcpy (cp, codeset);
      }
    if ((mask & XPG_NORM_CODESET) != 0)
      {
	*cp++ = '.';
	cp = stpcpy (cp, normalized_codeset);
      }
    if ((mask & XPG_MODIFIER) != 0)
      {
	*cp++ = '@';
	cp = stpcpy (cp, modifier);
      }

    *cp++ = '/';
    stpcpy (cp, filename);
  }

#if defined _WIN32 && !defined __CYGWIN__
  /* Construct wide-char file name.  */
  if (wdirlist_len > 0)
    {
      /* Since dirlist_len == 0, just concatenate wdirlist and abs_filename.  */
      /* An upper bound for wcslen (mbstowcs (abs_filename)).  */
      size_t abs_filename_bound = mbstowcs (NULL, abs_filename, 0);
      if (abs_filename_bound == (size_t)-1)
	{
	  free (abs_filename);
	  return NULL;
	}

      /* Allocate and fill abs_wfilename.  */
      abs_wfilename =
	(wchar_t *)
	malloc ((wdirlist_len + abs_filename_bound + 1) * sizeof (wchar_t));
      if (abs_wfilename == NULL)
	{
	  free (abs_filename);
	  return NULL;
	}
      wmemcpy (abs_wfilename, wdirlist, wdirlist_len - 1);
      abs_wfilename[wdirlist_len - 1] = L'/';
      if (mbstowcs (abs_wfilename + wdirlist_len, abs_filename,
		    abs_filename_bound + 1)
	  > abs_filename_bound)
	{
	  free (abs_filename);
	  free (abs_wfilename);
	  return NULL;
	}

      free (abs_filename);
      abs_filename = NULL;
    }
  else
    abs_wfilename = NULL;
#endif

  /* Look in list of already loaded domains whether it is already
     available.  */
  lastp = l10nfile_list;
#if defined _WIN32 && !defined __CYGWIN__
  if (abs_wfilename != NULL)
    {
      for (retval = *l10nfile_list; retval != NULL; retval = retval->next)
	{
	  if (retval->wfilename != NULL)
	    {
	      int compare = wcscmp (retval->wfilename, abs_wfilename);
	      if (compare == 0)
		/* We found it!  */
		break;
	      if (compare < 0)
		{
		  /* It's not in the list, and we have found the place where it
		     needs to be inserted: at *LASTP.  */
		  retval = NULL;
		  break;
		}
	    }
	  lastp = &retval->next;
	}
    }
  else
#endif
    for (retval = *l10nfile_list; retval != NULL; retval = retval->next)
      {
#if defined _WIN32 && !defined __CYGWIN__
	if (retval->filename != NULL)
#endif
	  {
	    int compare = strcmp (retval->filename, abs_filename);
	    if (compare == 0)
	      /* We found it!  */
	      break;
	    if (compare < 0)
	      {
		/* It's not in the list, and we have found the place where it
		   needs to be inserted: at *LASTP.  */
		retval = NULL;
	        break;
	      }
	  }
	lastp = &retval->next;
      }

  if (retval != NULL || do_allocate == 0)
    {
      free (abs_filename);
#if defined _WIN32 && !defined __CYGWIN__
      free (abs_wfilename);
#endif
      return retval;
    }

#ifdef _LIBC
  dirlist_count = (dirlist_len > 0 ? __argz_count (dirlist, dirlist_len) : 1);
#else
  dirlist_count = 1;
#endif

  /* Allocate a new loaded_l10nfile.  */
  retval =
    (struct loaded_l10nfile *)
    malloc (sizeof (*retval)
	    + (((dirlist_count << pop (mask)) + (dirlist_count > 1 ? 1 : 0))
	       * sizeof (struct loaded_l10nfile *)));
  if (retval == NULL)
    {
      free (abs_filename);
#if defined _WIN32 && !defined __CYGWIN__
      free (abs_wfilename);
#endif
      return NULL;
    }

  retval->filename = abs_filename;
#if defined _WIN32 && !defined __CYGWIN__
  retval->wfilename = abs_wfilename;
#endif

  /* We set retval->data to NULL here; it is filled in later.
     Setting retval->decided to 1 here means that retval does not
     correspond to a real file (dirlist_count > 1) or is not worth
     looking up (if an unnormalized codeset was specified).  */
  retval->decided = (dirlist_count > 1
		     || ((mask & XPG_CODESET) != 0
			 && (mask & XPG_NORM_CODESET) != 0));
  retval->data = NULL;

  retval->next = *lastp;
  *lastp = retval;

  entries = 0;
  /* Recurse to fill the inheritance list of RETVAL.
     If the DIRLIST is a real list (i.e. DIRLIST_COUNT > 1), the RETVAL
     entry does not correspond to a real file; retval->filename contains
     colons.  In this case we loop across all elements of DIRLIST and
     across all bit patterns dominated by MASK.
     If the DIRLIST is a single directory or entirely redundant (i.e.
     DIRLIST_COUNT == 1), we loop across all bit patterns dominated by
     MASK, excluding MASK itself.
     In either case, we loop down from MASK to 0.  This has the effect
     that the extra bits in the locale name are dropped in this order:
     first the modifier, then the territory, then the codeset, then the
     normalized_codeset.  */
  for (cnt = dirlist_count > 1 ? mask : mask - 1; cnt >= 0; --cnt)
    if ((cnt & ~mask) == 0
	&& !((cnt & XPG_CODESET) != 0 && (cnt & XPG_NORM_CODESET) != 0))
      {
#ifdef _LIBC
	if (dirlist_count > 1)
	  {
	    /* Iterate over all elements of the DIRLIST.  */
	    char *dir = NULL;

	    while ((dir = __argz_next ((char *) dirlist, dirlist_len, dir))
		   != NULL)
	      retval->successor[entries++]
		= _nl_make_l10nflist (l10nfile_list, dir, strlen (dir) + 1,
				      cnt, language, territory, codeset,
				      normalized_codeset, modifier, filename,
				      1);
	  }
	else
#endif
	  retval->successor[entries++]
	    = _nl_make_l10nflist (l10nfile_list,
				  dirlist, dirlist_len,
#if defined _WIN32 && !defined __CYGWIN__
				  wdirlist, wdirlist_len,
#endif
				  cnt, language, territory, codeset,
				  normalized_codeset, modifier, filename, 1);
      }
  retval->successor[entries] = NULL;

  return retval;
}

/* Normalize codeset name.  There is no standard for the codeset
   names.  Normalization allows the user to use any of the common
   names.  The return value is dynamically allocated and has to be
   freed by the caller.  */
const char *
_nl_normalize_codeset (const char *codeset, size_t name_len)
{
  size_t len = 0;
  int only_digit = 1;
  char *retval;
  char *wp;
  size_t cnt;

  for (cnt = 0; cnt < name_len; ++cnt)
    if (isalnum ((unsigned char) codeset[cnt]))
      {
	++len;

	if (isalpha ((unsigned char) codeset[cnt]))
	  only_digit = 0;
      }

  retval = (char *) malloc ((only_digit ? 3 : 0) + len + 1);

  if (retval != NULL)
    {
      if (only_digit)
	wp = stpcpy (retval, "iso");
      else
	wp = retval;

      for (cnt = 0; cnt < name_len; ++cnt)
	if (isalpha ((unsigned char) codeset[cnt]))
	  *wp++ = tolower ((unsigned char) codeset[cnt]);
	else if (isdigit ((unsigned char) codeset[cnt]))
	  *wp++ = codeset[cnt];

      *wp = '\0';
    }

  return (const char *) retval;
}


/* @@ begin of epilog @@ */

/* We don't want libintl.a to depend on any other library.  So we
   avoid the non-standard function stpcpy.  In GNU C Library this
   function is available, though.  Also allow the symbol HAVE_STPCPY
   to be defined.  */
#if !_LIBC && !HAVE_STPCPY
static char *
stpcpy (char *dest, const char *src)
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif
