/* Look at first character in UTF-8 string.
   Copyright (C) 1999-2002, 2006-2007, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001.

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

#if defined IN_LIBUNISTRING
/* Tell unistr.h to declare u8_mbtouc_unsafe as 'extern', not
   'static inline'.  */
# include "unistring-notinline.h"
#endif

/* Specification.  */
#include "unistr.h"

#if !HAVE_INLINE

int
u8_mbtouc_unsafe (ucs4_t *puc, const uint8_t *s, size_t n)
{
  uint8_t c = *s;

  if (c < 0x80)
    {
      *puc = c;
      return 1;
    }
  else if (c >= 0xc2)
    {
      if (c < 0xe0)
        {
          if (n >= 2)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  *puc = ((unsigned int) (c & 0x1f) << 6)
                         | (unsigned int) (s[1] ^ 0x80);
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              return 1;
            }
        }
      else if (c < 0xf0)
        {
          if (n >= 3)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  if ((s[2] ^ 0x80) < 0x40)
                    {
                      if ((c >= 0xe1 || s[1] >= 0xa0)
                          && (c != 0xed || s[1] < 0xa0))
                        {
                          *puc = ((unsigned int) (c & 0x0f) << 12)
                                 | ((unsigned int) (s[1] ^ 0x80) << 6)
                                 | (unsigned int) (s[2] ^ 0x80);
                          return 3;
                        }
                      /* invalid multibyte character */
                      *puc = 0xfffd;
                      return 3;
                    }
                  /* invalid multibyte character */
                  *puc = 0xfffd;
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              if (n == 1 || (s[1] ^ 0x80) >= 0x40)
                return 1;
              else
                return 2;
            }
        }
      else if (c < 0xf8)
        {
          if (n >= 4)
            {
              if ((s[1] ^ 0x80) < 0x40)
                {
                  if ((s[2] ^ 0x80) < 0x40)
                    {
                      if ((s[3] ^ 0x80) < 0x40)
                        {
                          if ((c >= 0xf1 || s[1] >= 0x90)
                              && (c < 0xf4 || (c == 0xf4 && s[1] < 0x90))
                             )
                            {
                              *puc = ((unsigned int) (c & 0x07) << 18)
                                     | ((unsigned int) (s[1] ^ 0x80) << 12)
                                     | ((unsigned int) (s[2] ^ 0x80) << 6)
                                     | (unsigned int) (s[3] ^ 0x80);
                              return 4;
                            }
                          /* invalid multibyte character */
                          *puc = 0xfffd;
                          return 4;
                        }
                      /* invalid multibyte character */
                      *puc = 0xfffd;
                      return 3;
                    }
                  /* invalid multibyte character */
                  *puc = 0xfffd;
                  return 2;
                }
              /* invalid multibyte character */
            }
          else
            {
              /* incomplete multibyte character */
              *puc = 0xfffd;
              if (n == 1 || (s[1] ^ 0x80) >= 0x40)
                return 1;
              else if (n == 2 || (s[2] ^ 0x80) >= 0x40)
                return 2;
              else
                return 3;
            }
        }
    }
  /* invalid multibyte character */
  *puc = 0xfffd;
  return 1;
}

#endif
