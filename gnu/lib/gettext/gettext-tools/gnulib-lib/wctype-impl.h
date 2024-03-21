/* Get descriptor for a wide character property.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2011.

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

wctype_t
wctype (const char* name)
{
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
                return (wctype_t) iswalnum;
              break;
            case 'p':
              if (strcmp (name + 3, "ha") == 0)
                return (wctype_t) iswalpha;
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
        return (wctype_t) iswblank;
      break;
    case 'c':
      if (strcmp (name + 1, "ntrl") == 0)
        return (wctype_t) iswcntrl;
      break;
    case 'd':
      if (strcmp (name + 1, "igit") == 0)
        return (wctype_t) iswdigit;
      break;
    case 'g':
      if (strcmp (name + 1, "raph") == 0)
        return (wctype_t) iswgraph;
      break;
    case 'l':
      if (strcmp (name + 1, "ower") == 0)
        return (wctype_t) iswlower;
      break;
    case 'p':
      switch (name[1])
        {
        case 'r':
          if (strcmp (name + 2, "int") == 0)
            return (wctype_t) iswprint;
          break;
        case 'u':
          if (strcmp (name + 2, "nct") == 0)
            return (wctype_t) iswpunct;
          break;
        default:
          break;
        }
      break;
    case 's':
      if (strcmp (name + 1, "pace") == 0)
        return (wctype_t) iswspace;
      break;
    case 'u':
      if (strcmp (name + 1, "pper") == 0)
        return (wctype_t) iswupper;
      break;
    case 'x':
      if (strcmp (name + 1, "digit") == 0)
        return (wctype_t) iswxdigit;
      break;
    default:
      break;
    }
  return NULL;
}
