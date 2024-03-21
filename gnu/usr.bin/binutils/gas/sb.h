/* sb.h - header file for string buffer manipulation routines
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

#ifndef SB_H

#define SB_H

/* String blocks

   I had a couple of choices when deciding upon this data structure.
   gas uses null terminated strings for all its internal work.  This
   often means that parts of the program that want to examine
   substrings have to manipulate the data in the string to do the
   right thing (a common operation is to single out a bit of text by
   saving away the character after it, nulling it out, operating on
   the substring and then replacing the character which was under the
   null).  This is a pain and I remember a load of problems that I had with
   code in gas which almost got this right.  Also, it's harder to grow and
   allocate null terminated strings efficiently.

   Obstacks provide all the functionality needed, but are too
   complicated, hence the sb.

   An sb is allocated by the caller.  */

typedef struct sb
{
  char *ptr;			/* Points to the current block.  */
  size_t len;			/* How much is used.  */
  size_t max;			/* The maximum length.  */
}
sb;

extern void sb_new (sb *);
extern void sb_build (sb *, size_t);
extern void sb_kill (sb *);
extern void sb_add_sb (sb *, sb *);
extern void sb_scrub_and_add_sb (sb *, sb *);
extern void sb_reset (sb *);
extern void sb_add_char (sb *, size_t);
extern void sb_add_string (sb *, const char *);
extern void sb_add_buffer (sb *, const char *, size_t);
extern char *sb_terminate (sb *);
extern size_t sb_skip_white (size_t, sb *);
extern size_t sb_skip_comma (size_t, sb *);

/* Actually in input-scrub.c.  */
enum expansion {
  expanding_none,
  expanding_repeat,
  expanding_macro,
};
extern void input_scrub_include_sb (sb *, char *, enum expansion);

#endif /* SB_H */
