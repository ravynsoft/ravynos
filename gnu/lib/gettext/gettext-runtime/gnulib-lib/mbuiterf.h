/* Iterating through multibyte strings, faster: macros for multi-byte encodings.
   Copyright (C) 2001, 2005, 2007, 2009-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>,
   with insights from Paul Eggert.  */

/* The macros in this file implement forward iteration through a
   multi-byte string, without knowing its length a-priori.

   With these macros, an iteration loop that looks like

      char *iter;
      for (iter = buf; *iter != '\0'; iter++)
        {
          do_something (*iter);
        }

   becomes

      mbuif_state_t state;
      [const] char *iter;
      for (mbuif_init (state), iter = buf; mbuif_avail (state, iter); )
        {
          mbchar_t cur = mbuif_next (state, iter);
          // Note: Here always mb_ptr (cur) == iter.
          do_something (iter, mb_len (cur));
          iter += mb_len (cur);
        }

   The benefit of these macros over plain use of mbrtowc or mbrtoc32 is:
   - Handling of invalid multibyte sequences is possible without
     making the code more complicated, while still preserving the
     invalid multibyte sequences.

   Compared to mbiterf.h, the macros here don't need to know the string's
   length a-priori.  The downside is that at each step, the look-ahead
   that guards against overrunning the terminating '\0' is more expensive.
   The mbuif_* macros are therefore suitable when there is a high probability
   that only the first few multibyte characters need to be inspected.
   Whereas the mbif_* macros are better if usually the iteration runs
   through the entire string.

   The benefit of these macros over those from mbuiter.h is that it
   produces faster code with today's optimizing compilers (because mbuif_next
   returns its result by value).

   mbuif_state_t
     is a type usable for variable declarations.

   mbuif_init (state)
     initializes the state.

   mbuif_avail (state, iter)
     returns true if another loop round is needed.

   mbuif_next (state, iter)
     returns the next multibyte character.
     It asssumes that the state is initialized and that *iter != '\0'.

   Here are the function prototypes of the macros.

   extern void      mbuif_init (mbuif_state_t state);
   extern bool      mbuif_avail (mbuif_state_t state, const char *iter);
   extern mbchar_t  mbuif_next (mbuif_state_t state, const char *iter);
 */

#ifndef _MBUITERF_H
#define _MBUITERF_H 1

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE,
   _GL_ATTRIBUTE_ALWAYS_INLINE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

#include "mbchar.h"
#include "strnlen1.h"

_GL_INLINE_HEADER_BEGIN
#ifndef MBUITERF_INLINE
# define MBUITERF_INLINE _GL_INLINE _GL_ATTRIBUTE_ALWAYS_INLINE
#endif

struct mbuif_state
{
  #if !GNULIB_MBRTOC32_REGULAR
  bool in_shift;        /* true if next byte may not be interpreted as ASCII */
                        /* If GNULIB_MBRTOC32_REGULAR, it is always false,
                           so optimize it away.  */
  #endif
  mbstate_t state;      /* if in_shift: current shift state */
                        /* If GNULIB_MBRTOC32_REGULAR, it is in an initial state
                           before and after every mbuiterf_next invocation.
                         */
  unsigned int cur_max; /* A cache of MB_CUR_MAX.  */
};

MBUITERF_INLINE mbchar_t
mbuiterf_next (struct mbuif_state *ps, const char *iter)
{
  #if !GNULIB_MBRTOC32_REGULAR
  if (ps->in_shift)
    goto with_shift;
  #endif
  /* Handle most ASCII characters quickly, without calling mbrtowc().  */
  if (is_basic (*iter))
    {
      /* These characters are part of the POSIX portable character set.
         For most of them, namely those in the ISO C basic character set,
         ISO C 99 guarantees that their wide character code is identical to
         their char code.  For the few other ones, this is the case as well,
         in all locale encodings that are in use.  The 32-bit wide character
         code is the same as well.  */
      return (mbchar_t) { .ptr = iter, .bytes = 1, .wc_valid = true, .wc = *iter };
    }
  else
    {
      assert (mbsinit (&ps->state));
      #if !GNULIB_MBRTOC32_REGULAR
      ps->in_shift = true;
    with_shift:;
      #endif
      size_t bytes;
      char32_t wc;
      bytes = mbrtoc32 (&wc, iter, strnlen1 (iter, ps->cur_max), &ps->state);
      if (bytes == (size_t) -1)
        {
          /* An invalid multibyte sequence was encountered.  */
          /* Allow the next invocation to continue from a sane state.  */
          #if !GNULIB_MBRTOC32_REGULAR
          ps->in_shift = false;
          #endif
          mbszero (&ps->state);
          return (mbchar_t) { .ptr = iter, .bytes = 1, .wc_valid = false };
        }
      else if (bytes == (size_t) -2)
        {
          /* An incomplete multibyte character at the end.  */
          /* Whether to set ps->in_shift = false and reset ps->state or not is
             not important; the string end is reached anyway.  */
          return (mbchar_t) { .ptr = iter, .bytes = strlen (iter), .wc_valid = false };
        }
      else
        {
          if (bytes == 0)
            {
              /* A null wide character was encountered.  */
              bytes = 1;
              assert (*iter == '\0');
              assert (wc == 0);
            }
          #if !GNULIB_MBRTOC32_REGULAR
          else if (bytes == (size_t) -3)
            /* The previous multibyte sequence produced an additional 32-bit
               wide character.  */
            bytes = 0;
          #endif

          /* When in an initial state, we can go back treating ASCII
             characters more quickly.  */
          #if !GNULIB_MBRTOC32_REGULAR
          if (mbsinit (&ps->state))
            ps->in_shift = false;
          #endif
          return (mbchar_t) { .ptr = iter, .bytes = bytes, .wc_valid = true, .wc = wc };
        }
    }
}

/* Iteration macros.  */
typedef struct mbuif_state mbuif_state_t;
#if !GNULIB_MBRTOC32_REGULAR
#define mbuif_init(st) \
  ((st).in_shift = false, mbszero (&(st).state), \
   (st).cur_max = MB_CUR_MAX)
#else
/* Optimized: no in_shift.  */
#define mbuif_init(st) \
  (mbszero (&(st).state), \
   (st).cur_max = MB_CUR_MAX)
#endif
#if !GNULIB_MBRTOC32_REGULAR
#define mbuif_avail(st, iter) ((st).in_shift || (*(iter) != '\0'))
#else
/* Optimized: no in_shift.  */
#define mbuif_avail(st, iter) (*(iter) != '\0')
#endif
#define mbuif_next(st, iter) \
  mbuiterf_next (&(st), (iter))

_GL_INLINE_HEADER_END

#endif /* _MBUITERF_H */
