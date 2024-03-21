/* Conversion to UTF-8 from legacy encodings.
   Copyright (C) 2002, 2006-2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>.  */

#include <config.h>

/* Specification.  */
#include "uniconv.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c-strcaseeq.h"
#include "striconveha.h"
#include "unistr.h"

uint8_t *
u8_conv_from_encoding (const char *fromcode,
                       enum iconv_ilseq_handler handler,
                       const char *src, size_t srclen,
                       size_t *offsets,
                       uint8_t *resultbuf, size_t *lengthp)
{
  if (STRCASEEQ (fromcode, "UTF-8", 'U','T','F','-','8',0,0,0,0))
    {
      /* Conversion from UTF-8 to UTF-8.  No need to go through iconv().  */
      uint8_t *result;

      if (u8_check ((const uint8_t *) src, srclen))
        {
          errno = EILSEQ;
          return NULL;
        }

      if (offsets != NULL)
        {
          size_t i;

          for (i = 0; i < srclen; )
            {
              int count = u8_mblen ((const uint8_t *) src + i, srclen - i);
              /* We can rely on count > 0 because of the previous u8_check.  */
              if (count <= 0)
                abort ();
              offsets[i] = i;
              i++;
              while (--count > 0)
                offsets[i++] = (size_t)(-1);
            }
        }

      /* Memory allocation.  */
      if (resultbuf != NULL && *lengthp >= srclen)
        result = resultbuf;
      else
        {
          result = (uint8_t *) malloc (srclen > 0 ? srclen : 1);
          if (result == NULL)
            {
              errno = ENOMEM;
              return NULL;
            }
        }

      if (srclen > 0)
        memcpy ((char *) result, src, srclen);
      *lengthp = srclen;
      return result;
    }
  else
    {
      char *result = (char *) resultbuf;
      size_t length = *lengthp;

      if (mem_iconveha (src, srclen, fromcode, "UTF-8", true, handler,
                        offsets, &result, &length) < 0)
        return NULL;

      if (result == NULL) /* when (resultbuf == NULL && length == 0)  */
        {
          result = (char *) malloc (1);
          if (result == NULL)
            {
              errno = ENOMEM;
              return NULL;
            }
        }
      *lengthp = length;
      return (uint8_t *) result;
    }
}
