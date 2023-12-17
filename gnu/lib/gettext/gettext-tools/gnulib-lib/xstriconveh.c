/* Charset conversion with out-of-memory checking.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "xstriconveh.h"

#include <errno.h>

#include "striconveh.h"
#include "xalloc.h"


#if HAVE_ICONV

int
xmem_cd_iconveh (const char *src, size_t srclen,
                 const iconveh_t *cd,
                 enum iconv_ilseq_handler handler,
                 size_t *offsets,
                 char **resultp, size_t *lengthp)
{
  int retval =
    mem_cd_iconveh (src, srclen, cd, handler, offsets, resultp, lengthp);

  if (retval < 0 && errno == ENOMEM)
    xalloc_die ();
  return retval;
}

char *
xstr_cd_iconveh (const char *src,
                 const iconveh_t *cd,
                 enum iconv_ilseq_handler handler)
{
  char *result = str_cd_iconveh (src, cd, handler);

  if (result == NULL && errno == ENOMEM)
    xalloc_die ();
  return result;
}

#endif

int
xmem_iconveh (const char *src, size_t srclen,
              const char *from_codeset, const char *to_codeset,
              enum iconv_ilseq_handler handler,
              size_t *offsets,
              char **resultp, size_t *lengthp)
{
  int retval =
    mem_iconveh (src, srclen, from_codeset, to_codeset, handler, offsets,
                 resultp, lengthp);

  if (retval < 0 && errno == ENOMEM)
    xalloc_die ();
  return retval;
}

char *
xstr_iconveh (const char *src,
              const char *from_codeset, const char *to_codeset,
              enum iconv_ilseq_handler handler)
{
  char *result = str_iconveh (src, from_codeset, to_codeset, handler);

  if (result == NULL && errno == ENOMEM)
    xalloc_die ();
  return result;
}
