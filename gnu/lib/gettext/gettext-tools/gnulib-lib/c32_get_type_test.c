/* Get descriptor for a 32-bit wide character property.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#define IN_C32_GET_TYPE_TEST
/* Specification.  */
#include <uchar.h>

#include <string.h>
#include <wctype.h>

#if _GL_WCHAR_T_IS_UCS4
_GL_EXTERN_INLINE
#endif
c32_type_test_t
c32_get_type_test (const char *name)
{
#if _GL_WCHAR_T_IS_UCS4
  return wctype (name);
#else
  switch (name[0])
    {
    case 'a':
      switch (name[1])
        {
        case 'l':
          switch (name[2])
            {
            case 'n':
              if (strcmp (name + 3, "um") == 0)
                return c32isalnum;
              break;
            case 'p':
              if (strcmp (name + 3, "ha") == 0)
                return c32isalpha;
              break;
            default:
              break;
            }
          break;
        default:
          break;
        }
      break;
    case 'b':
      if (strcmp (name + 1, "lank") == 0)
        return c32isblank;
      break;
    case 'c':
      if (strcmp (name + 1, "ntrl") == 0)
        return c32iscntrl;
      break;
    case 'd':
      if (strcmp (name + 1, "igit") == 0)
        return c32isdigit;
      break;
    case 'g':
      if (strcmp (name + 1, "raph") == 0)
        return c32isgraph;
      break;
    case 'l':
      if (strcmp (name + 1, "ower") == 0)
        return c32islower;
      break;
    case 'p':
      switch (name[1])
        {
        case 'r':
          if (strcmp (name + 2, "int") == 0)
            return c32isprint;
          break;
        case 'u':
          if (strcmp (name + 2, "nct") == 0)
            return c32ispunct;
          break;
        default:
          break;
        }
      break;
    case 's':
      if (strcmp (name + 1, "pace") == 0)
        return c32isspace;
      break;
    case 'u':
      if (strcmp (name + 1, "pper") == 0)
        return c32isupper;
      break;
    case 'x':
      if (strcmp (name + 1, "digit") == 0)
        return c32isxdigit;
      break;
    default:
      break;
    }
  return (c32_type_test_t) 0;
#endif
}
