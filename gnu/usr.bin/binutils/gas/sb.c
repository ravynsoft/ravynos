/* sb.c - string buffer manipulation routines
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
#include "sb.h"

#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* These routines are about manipulating strings.

   They are managed in things called `sb's which is an abbreviation
   for string buffers.  An sb has to be created, things can be glued
   on to it, and at the end of it's life it should be freed.  The
   contents should never be pointed at whilst it is still growing,
   since it could be moved at any time

   eg:
   sb_new (&foo);
   sb_grow... (&foo,...);
   use foo->ptr[*];
   sb_kill (&foo);  */

/* Buffers start at INIT_ALLOC size, and roughly double each time we
   go over the current allocation.  MALLOC_OVERHEAD is a guess at the
   system malloc overhead.  We aim to not waste any memory in the
   underlying page/chunk allocated by the system malloc.  */
#define MALLOC_OVERHEAD (2 * sizeof (size_t))
#define INIT_ALLOC (64 - MALLOC_OVERHEAD - 1)

static void sb_check (sb *, size_t);

/* Initializes an sb.  */

void
sb_build (sb *ptr, size_t size)
{
  ptr->ptr = XNEWVEC (char, size + 1);
  ptr->max = size;
  ptr->len = 0;
}

void
sb_new (sb *ptr)
{
  sb_build (ptr, INIT_ALLOC);
}

/* Deallocate the sb at ptr.  */

void
sb_kill (sb *ptr)
{
  free (ptr->ptr);
}

/* Add the sb at s to the end of the sb at ptr.  */

void
sb_add_sb (sb *ptr, sb *s)
{
  sb_check (ptr, s->len);
  memcpy (ptr->ptr + ptr->len, s->ptr, s->len);
  ptr->len += s->len;
}

/* Helper for sb_scrub_and_add_sb.  */

static sb *sb_to_scrub;
static char *scrub_position;
static size_t
scrub_from_sb (char *buf, size_t buflen)
{
  size_t copy;
  copy = sb_to_scrub->len - (scrub_position - sb_to_scrub->ptr);
  if (copy > buflen)
    copy = buflen;
  memcpy (buf, scrub_position, copy);
  scrub_position += copy;
  return copy;
}

/* Run the sb at s through do_scrub_chars and add the result to the sb
   at ptr.  */

void
sb_scrub_and_add_sb (sb *ptr, sb *s)
{
  sb_to_scrub = s;
  scrub_position = s->ptr;

  /* do_scrub_chars can expand text, for example when replacing
     # 123 "filename"
     with
     \t.linefile 123 "filename"
     or when replacing a 'c with the decimal ascii number for c.
     So we loop until the input S is consumed.  */
  while (1)
    {
      size_t copy = s->len - (scrub_position - s->ptr) + do_scrub_pending ();
      if (copy == 0)
	break;
      sb_check (ptr, copy);
      ptr->len += do_scrub_chars (scrub_from_sb, ptr->ptr + ptr->len,
				  ptr->max - ptr->len);
    }

  sb_to_scrub = 0;
  scrub_position = 0;
}

/* Make sure that the sb at ptr has room for another len characters,
   and grow it if it doesn't.  */

static void
sb_check (sb *ptr, size_t len)
{
  size_t want = ptr->len + len;

  if (want > ptr->max)
    {
      size_t max;

      want += MALLOC_OVERHEAD + 1;
      if ((ssize_t) want < 0)
	as_fatal ("string buffer overflow");
#if GCC_VERSION >= 3004
      max = (size_t) 1 << (CHAR_BIT * sizeof (want)
			   - (sizeof (want) <= sizeof (long)
			      ? __builtin_clzl ((long) want)
			      : __builtin_clzll ((long long) want)));
#else
      max = 128;
      while (want > max)
	max <<= 1;
#endif
      max -= MALLOC_OVERHEAD + 1;
      ptr->max = max;
      ptr->ptr = XRESIZEVEC (char, ptr->ptr, max + 1);
    }
}

/* Make the sb at ptr point back to the beginning.  */

void
sb_reset (sb *ptr)
{
  ptr->len = 0;
}

/* Add character c to the end of the sb at ptr.  */

void
sb_add_char (sb *ptr, size_t c)
{
  sb_check (ptr, 1);
  ptr->ptr[ptr->len++] = c;
}

/* Add null terminated string s to the end of sb at ptr.  */

void
sb_add_string (sb *ptr, const char *s)
{
  size_t len = strlen (s);
  sb_check (ptr, len);
  memcpy (ptr->ptr + ptr->len, s, len);
  ptr->len += len;
}

/* Add string at s of length len to sb at ptr */

void
sb_add_buffer (sb *ptr, const char *s, size_t len)
{
  sb_check (ptr, len);
  memcpy (ptr->ptr + ptr->len, s, len);
  ptr->len += len;
}

/* Write terminating NUL and return string.  */

char *
sb_terminate (sb *in)
{
  in->ptr[in->len] = 0;
  return in->ptr;
}

/* Start at the index idx into the string in sb at ptr and skip
   whitespace. return the index of the first non whitespace character.  */

size_t
sb_skip_white (size_t idx, sb *ptr)
{
  while (idx < ptr->len
	 && (ptr->ptr[idx] == ' '
	     || ptr->ptr[idx] == '\t'))
    idx++;
  return idx;
}

/* Start at the index idx into the sb at ptr. skips whitespace,
   a comma and any following whitespace. returns the index of the
   next character.  */

size_t
sb_skip_comma (size_t idx, sb *ptr)
{
  while (idx < ptr->len
	 && (ptr->ptr[idx] == ' '
	     || ptr->ptr[idx] == '\t'))
    idx++;

  if (idx < ptr->len
      && ptr->ptr[idx] == ',')
    idx++;

  while (idx < ptr->len
	 && (ptr->ptr[idx] == ' '
	     || ptr->ptr[idx] == '\t'))
    idx++;

  return idx;
}
