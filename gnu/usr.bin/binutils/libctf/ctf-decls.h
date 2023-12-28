/* Declarations for missing functions.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef _CTF_DECLS_H
#define _CTF_DECLS_H

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include "libiberty.h"

#if HAVE_QSORT_R_ARG_LAST
static inline void
ctf_qsort_r (void *base, size_t nmemb, size_t size,
	     int (*compar)(const void *, const void *, void *),
	     void *arg)
{
  qsort_r (base, nmemb, size, compar, arg);
}
#elif HAVE_QSORT_R_COMPAR_LAST
struct ctf_qsort_arg
{
  int (*compar) (const void *, const void *, void *);
  void *arg;
};

static int
ctf_qsort_compar_thunk (void *arg, const void *a, const void *b)
{
  struct ctf_qsort_arg *qsort_arg = (struct ctf_qsort_arg *) arg;

  return qsort_arg->compar (a, b, qsort_arg->arg);
}

static inline void
ctf_qsort_r (void *base, size_t nmemb, size_t size,
	     int (*compar)(const void *, const void *, void *),
	     void *arg)
{
  struct ctf_qsort_arg thunk = { compar, arg };
  qsort_r (base, nmemb, size, &thunk, ctf_qsort_compar_thunk);
}
#else
void ctf_qsort_r (void *base, size_t nmemb, size_t size,
	      int (*compar)(const void *, const void *, void *),
	      void *arg);
#endif

#ifndef HAVE_O_CLOEXEC
# define O_CLOEXEC 0
#endif

#undef MAX
#undef MIN
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if !HAVE_DECL_STPCPY
extern char *stpcpy (char *, const char *);
#endif

#endif /* _CTF_DECLS_H */
