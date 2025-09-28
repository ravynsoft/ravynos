/* Functions (currently) for use by the shell to do malloc debugging and
   tracking. */
/* Copyright (C) 2001-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne-Again SHell.

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

#ifndef _SH_MALLOC_H
#define _SH_MALLOC_H

#ifndef PARAMS
#  if defined (__STDC__) || defined (__GNUC__) || defined (__cplusplus)
#    define PARAMS(protos) protos
#  else
#    define PARAMS(protos) ()
#  endif
#endif

/* Generic pointer type. */
#ifndef PTR_T

#if defined (__STDC__)
#  define PTR_T void *
#else
#  define PTR_T char *
#endif

#endif /* PTR_T */


extern PTR_T sh_malloc PARAMS((size_t, const char *, int));
extern PTR_T sh_realloc PARAMS((PTR_T, size_t, const char *, int));
extern void sh_free PARAMS((PTR_T, const char *, int));

extern PTR_T sh_memalign PARAMS((size_t, size_t, const char *, int));

extern PTR_T sh_calloc PARAMS((size_t, size_t, const char *, int));
extern void sh_cfree PARAMS((PTR_T, const char *, int));

extern PTR_T sh_valloc PARAMS((size_t, const char *, int));

/* trace.c */
extern int malloc_set_trace PARAMS((int));
extern void malloc_set_tracefp ();	/* full prototype requires stdio.h */
extern void malloc_set_tracefn PARAMS((char *, char *));

/* table.c */
extern void mregister_dump_table PARAMS((void));
extern void mregister_table_init PARAMS((void));
extern int malloc_set_register PARAMS((int));

/* stats.c */
extern void print_malloc_stats PARAMS((char *));
extern void fprint_malloc_stats ();	/* full prototype requires stdio.h */
extern void trace_malloc_stats PARAMS((char *, char *));

#endif
