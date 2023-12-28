/* ldmisc.h -
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef LDMISC_H
#define LDMISC_H

extern void vfinfo (FILE *fp, const char *fmt, va_list arg, bool is_warning);
extern void einfo (const char *, ...);
extern void minfo (const char *, ...);
extern void info_msg (const char *, ...);
extern void lfinfo (FILE *, const char *, ...);
extern void info_assert (const char *, unsigned int);
extern void yyerror (const char *);
extern void *xmalloc (size_t);
extern void *xrealloc (void *, size_t);
extern void xexit (int);

#define ASSERT(x) \
do { if (!(x)) info_assert(__FILE__,__LINE__); } while (0)

#define FAIL() \
do { info_assert(__FILE__,__LINE__); } while (0)

extern void print_spaces (int);
#define print_space() print_spaces (1)
extern void print_nl (void);

#endif
