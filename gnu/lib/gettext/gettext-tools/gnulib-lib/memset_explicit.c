/* Erase sensitive data from memory.
   Copyright 2022-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* memset_s need this define */
#if HAVE_MEMSET_S
# define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <string.h>

/* Set S's bytes to C, where S has LEN bytes.  The compiler will not
   optimize effects away, even if S is dead after the call.  */
void *
memset_explicit (void *s, int c, size_t len)
{
#if HAVE_EXPLICIT_MEMSET
  return explicit_memset (s, c, len);
#elif HAVE_MEMSET_S
  (void) memset_s (s, len, c, len);
  return s;
#elif defined __GNUC__ && !defined __clang__
  memset (s, c, len);
  /* Compiler barrier.  */
  __asm__ volatile ("" ::: "memory");
  return s;
#elif defined __clang__
  memset (s, c, len);
  /* Compiler barrier.  */
  /* With asm ("" ::: "memory") LLVM analyzes uses of 's' and finds that the
     whole thing is dead and eliminates it.  Use 'g' to work around this
     problem.  See <https://bugs.llvm.org/show_bug.cgi?id=15495#c11>.  */
  __asm__ volatile ("" : : "g"(s) : "memory");
  return s;
#else
  /* Invoke memset through a volatile function pointer.  This defeats compiler
     optimizations.  */
  void * (* const volatile volatile_memset) (void *, int, size_t) = memset;
  return volatile_memset (s, c, len);
#endif
}
