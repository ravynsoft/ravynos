/* Expand escape sequences in a string.
   Copyright (C) 1995-1997, 2000-2007, 2012, 2018-2020 Free Software
   Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, May 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Expand some escape sequences found in the argument string.
   If backslash_c_seen is != NULL, '\c' sequences are recognized and
   have the effect of setting *backslash_c_seen to true.
   Returns either the argument string or a freshly allocated string.  */
static const char *
expand_escapes (const char *str, bool *backslash_c_seen)
{
  const char *cp = str;

  /* Find the location of the first escape sequence.
     If the string contains no escape sequences, return it right away.  */
  for (;;)
    {
      while (cp[0] != '\0' && cp[0] != '\\')
        ++cp;
      if (cp[0] == '\0')
        /* The argument string contains no escape sequence.  */
        return str;
      /* Found a backslash.  */
      if (cp[1] == '\0')
        return str;
      if (strchr ("abcfnrtv\\01234567", cp[1]) != NULL)
        break;
      ++cp;
    }

  {
    char *retval = XNMALLOC (strlen (str), char);

    memcpy (retval, str, cp - str);
    {
      char *rp = retval + (cp - str);

      do
        {
          /* Here cp[0] == '\\'.  */
          switch (*++cp)
            {
            case 'a':               /* alert */
              *rp++ = '\a';
              ++cp;
              break;
            case 'b':               /* backspace */
              *rp++ = '\b';
              ++cp;
              break;
            case 'f':               /* form feed */
              *rp++ = '\f';
              ++cp;
              break;
            case 'n':               /* new line */
              *rp++ = '\n';
              ++cp;
              break;
            case 'r':               /* carriage return */
              *rp++ = '\r';
              ++cp;
              break;
            case 't':               /* horizontal tab */
              *rp++ = '\t';
              ++cp;
              break;
            case 'v':               /* vertical tab */
              *rp++ = '\v';
              ++cp;
              break;
            case '\\':
              *rp++ = '\\';
              ++cp;
              break;
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
              {
                int ch = *cp++ - '0';

                if (*cp >= '0' && *cp <= '7')
                  {
                    ch *= 8;
                    ch += *cp++ - '0';

                    if (*cp >= '0' && *cp <= '7')
                      {
                        ch *= 8;
                        ch += *cp++ - '0';
                      }
                  }
                *rp++ = ch;
              }
              break;
            case 'c':
              if (backslash_c_seen != NULL)
                {
                  *backslash_c_seen = true;
                  ++cp;
                  break;
                }
              FALLTHROUGH;
            default:
              *rp++ = '\\';
              break;
            }

          /* Find the next escape sequence.  */
          while (cp[0] != '\0' && cp[0] != '\\')
            *rp++ = *cp++;
        }
      while (cp[0] != '\0');

      /* Terminate the resulting string.  */
      *rp = '\0';
    }

    return retval;
  }
}
