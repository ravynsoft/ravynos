/*
   Copyright (C) 2021 Free Software Foundation, Inc.
   
   This file is part of GNU Bash.
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
/* getconf.h -- replacement definitions for ones the system doesn't provide
   and don't appear in <typemax.h> */

#ifndef _GETCONF_H
#define _GETCONF_H

/* Some systems do not define these; use POSIX.2 minimum recommended values. */
#ifndef _POSIX2_COLL_WEIGHTS_MAX
#  define _POSIX2_COLL_WEIGHTS_MAX 2
#endif

/* If we're on a posix system, but the system doesn't define the necessary
   constants, use posix.1 minimum values. */
#if defined (_POSIX_VERSION)

#ifndef _POSIX_ARG_MAX
#  define _POSIX_ARG_MAX	4096
#endif
#ifndef _POSIX_CHILD_MAX
#  define _POSIX_CHILD_MAX	6
#endif
#ifndef _POSIX_LINK_MAX
#  define _POSIX_LINK_MAX	8
#endif
#ifndef _POSIX_MAX_CANON
#  define _POSIX_MAX_CANON	255
#endif
#ifndef _POSIX_MAX_INPUT
#  define _POSIX_MAX_INPUT	255
#endif
#ifndef _POSIX_NAME_MAX
#  define _POSIX_NAME_MAX	14
#endif
#ifndef _POSIX_NGROUPS_MAX
#  define _POSIX_NGROUPS_MAX	0
#endif
#ifndef _POSIX_OPEN_MAX
#  define _POSIX_OPEN_MAX	16
#endif
#ifndef _POSIX_PATH_MAX
#  define _POSIX_PATH_MAX	255
#endif
#ifndef _POSIX_PIPE_BUF
#  define _POSIX_PIPE_BUF	512
#endif
#ifndef _POSIX_SSIZE_MAX
#  define _POSIX_SSIZE_MAX	32767
#endif
#ifndef _POSIX_STREAM_MAX
#  define _POSIX_STREAM_MAX	8
#endif
#ifndef _POSIX_TZNAME_MAX
#  define _POSIX_TZNAME_MAX	3
#endif

#ifndef _POSIX2_BC_BASE_MAX
#  define _POSIX2_BC_BASE_MAX     99
#endif
#ifndef _POSIX2_BC_DIM_MAX
#  define _POSIX2_BC_DIM_MAX      2048
#endif
#ifndef _POSIX2_BC_SCALE_MAX
#  define _POSIX2_BC_SCALE_MAX    99
#endif
#ifndef _POSIX2_BC_STRING_MAX
#  define _POSIX2_BC_STRING_MAX   1000
#endif
#ifndef _POSIX2_EQUIV_CLASS_MAX
#  define _POSIX2_EQUIV_CLASS_MAX 2
#endif
#ifndef _POSIX2_EXPR_NEST_MAX
#  define _POSIX2_EXPR_NEST_MAX   32
#endif
#ifndef _POSIX2_LINE_MAX
#  define _POSIX2_LINE_MAX        2048
#endif
#ifndef _POSIX2_RE_DUP_MAX
#  define _POSIX2_RE_DUP_MAX      255
#endif

#endif /* _POSIX_VERSION */

/* ANSI/ISO C, POSIX.1-200x, XPG 4.2, and C language type limits.
   Defined only if the system include files and <typemax.h> don't. */

#ifndef CHAR_MAX
#  define CHAR_MAX	127
#endif
#ifndef CHAR_MIN
#  define CHAR_MIN	-128
#endif
#ifndef SCHAR_MAX
#  define SCHAR_MAX	127
#endif
#ifndef SCHAR_MIN
#  define SCHAR_MIN	-128
#endif

#ifndef INT_BIT
#  define INT_BIT	(sizeof (int) * CHAR_BIT)
#endif

#ifndef LONG_BIT
#  define LONG_BIT	(sizeof (long int) * CHAR_BIT)
#endif

#ifndef WORD_BIT
#  define WORD_BIT	(sizeof (int) * CHAR_BIT)
#endif

#if !defined (PRIdMAX)
#  if HAVE_LONG_LONG
#    define PRIdMAX     "lld"
#  else
#    define PRIdMAX     "ld"
#  endif
#endif

#endif /* _GETCONF_H */
