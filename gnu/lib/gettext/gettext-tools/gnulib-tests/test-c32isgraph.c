/* Test of c32isgraph() function.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

#include <uchar.h>

#include "signature.h"
SIGNATURE_CHECK (c32isgraph, int, (wint_t));

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "macros.h"

/* Returns the value of c32isgraph for the multibyte character s[0..n-1].  */
static int
for_character (const char *s, size_t n)
{
  mbstate_t state;
  char32_t wc;
  size_t ret;

  memset (&state, '\0', sizeof (mbstate_t));
  wc = (char32_t) 0xBADFACE;
  ret = mbrtoc32 (&wc, s, n, &state);
  ASSERT (ret == n);

  return c32isgraph (wc);
}

int
main (int argc, char *argv[])
{
  int is;
  char buf[4];

  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  /* Test WEOF.  */
  is = c32isgraph (WEOF);
  ASSERT (is == 0);

  /* Test single-byte characters.
     POSIX specifies in
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap07.html>
     no explicit list of graphic characters.  */
  {
    int c;

    for (c = 0; c < 0x100; c++)
      switch (c)
        {
        case '\t': case '\v': case '\f':
        case ' ': case '!': case '"': case '#': case '%':
        case '&': case '\'': case '(': case ')': case '*':
        case '+': case ',': case '-': case '.': case '/':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case ':': case ';': case '<': case '=': case '>':
        case '?':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z':
        case '[': case '\\': case ']': case '^': case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z': case '{': case '|': case '}': case '~':
          /* c is in the ISO C "basic character set".  */
          buf[0] = (unsigned char) c;
          is = for_character (buf, 1);
          switch (c)
            {
            case '\t': case '\v': case '\f':
            case ' ':
              ASSERT (is == 0);
              break;
            default:
              ASSERT (is != 0);
              break;
            }
          break;
        }
  }

  if (argc > 1)
    switch (argv[1][0])
      {
      case '0':
        /* C locale; tested above.  */
        return 0;

      case '1':
        /* Locale encoding is ISO-8859-1 or ISO-8859-15.  */
        {
          /* U+007F <control> */
          is = for_character ("\177", 1);
          ASSERT (is == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sgi || defined __sun || defined __CYGWIN__ || (defined _WIN32 && !defined __CYGWIN__))
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\240", 1);
          ASSERT (is != 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
          /* U+00B8 CEDILLA */
          is = for_character ("\270", 1);
          ASSERT (is != 0);
        #endif
        }
        return 0;

      case '2':
        /* Locale encoding is EUC-JP.  */
        {
          /* U+007F <control> */
          is = for_character ("\177", 1);
          ASSERT (is == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__)
          /* U+00B8 CEDILLA */
          is = for_character ("\217\242\261", 3);
          ASSERT (is != 0);
        #endif
          /* U+3000 IDEOGRAPHIC SPACE */
          is = for_character ("\241\241", 2);
          ASSERT (is == 0);
        }
        return 0;

      case '3':
        /* Locale encoding is UTF-8.  */
        {
          /* U+007F <control> */
          is = for_character ("\177", 1);
          ASSERT (is == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun || defined __CYGWIN__ || (defined _WIN32 && !defined __CYGWIN__))
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\302\240", 2);
          ASSERT (is != 0);
        #endif
          /* U+00B8 CEDILLA */
          is = for_character ("\302\270", 2);
          ASSERT (is != 0);
          /* U+2002 EN SPACE */
          is = for_character ("\342\200\202", 3);
          ASSERT (is == 0);
        #if !(defined __GLIBC__ || defined MUSL_LIBC || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __NetBSD__ || defined _AIX || defined __sun || defined __CYGWIN__)
          /* U+202E RIGHT-TO-LEFT OVERRIDE */
          is = for_character ("\342\200\256", 3);
          ASSERT (is == 0);
        #endif
          /* U+3000 IDEOGRAPHIC SPACE */
          is = for_character ("\343\200\200", 3);
          ASSERT (is == 0);
        #if !(defined __GLIBC__ || defined MUSL_LIBC || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __NetBSD__ || defined _AIX || defined __sun || defined __CYGWIN__)
          /* U+FEFF ZERO WIDTH NO-BREAK SPACE */
          is = for_character ("\357\273\277", 3);
          ASSERT (is == 0);
        #endif
        #if !defined __sun
          /* U+20000 <CJK Ideograph> */
          is = for_character ("\360\240\200\200", 4);
          ASSERT (is != 0);
        #endif
        #if !(defined __GLIBC__ || defined MUSL_LIBC || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __NetBSD__ || defined _AIX || defined __sun || defined __CYGWIN__ || (defined _WIN32 && !defined __CYGWIN__))
          /* U+E0001 LANGUAGE TAG */
          is = for_character ("\363\240\200\201", 4);
          ASSERT (is == 0);
        #endif
        }
        return 0;

      case '4':
        /* Locale encoding is GB18030.  */
        #if (defined __GLIBC__ && __GLIBC__ == 2 && __GLIBC_MINOR__ >= 13 && __GLIBC_MINOR__ <= 15) || (GL_CHAR32_T_IS_UNICODE && (defined __NetBSD__ || defined __sun))
        fputs ("Skipping test: The GB18030 converter in this system's iconv is broken.\n", stderr);
        return 77;
        #endif
        {
          /* U+007F <control> */
          is = for_character ("\177", 1);
          ASSERT (is == 0);
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
          /* U+00A0 NO-BREAK SPACE */
          is = for_character ("\201\060\204\062", 4);
          ASSERT (is != 0);
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
          /* U+00B8 CEDILLA */
          is = for_character ("\201\060\206\060", 4);
          ASSERT (is != 0);
        #endif
          /* U+2002 EN SPACE */
          is = for_character ("\201\066\243\070", 4);
          ASSERT (is == 0);
        #if !(defined __GLIBC__ || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__)
          /* U+202E RIGHT-TO-LEFT OVERRIDE */
          is = for_character ("\201\066\247\061", 4);
          ASSERT (is == 0);
        #endif
          /* U+3000 IDEOGRAPHIC SPACE */
          is = for_character ("\241\241", 2);
          ASSERT (is == 0);
        #if !(defined __GLIBC__ || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__)
          /* U+FEFF ZERO WIDTH NO-BREAK SPACE */
          is = for_character ("\204\061\225\063", 4);
          ASSERT (is == 0);
        #endif
        #if !((defined __APPLE__ && defined __MACH__) || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __sun)
          /* U+20000 <CJK Ideograph> */
          is = for_character ("\225\062\202\066", 4);
          ASSERT (is != 0);
        #endif
        #if !(defined __GLIBC__ || (defined __APPLE__ && defined __MACH__) || defined __FreeBSD__)
          /* U+E0001 LANGUAGE TAG */
          is = for_character ("\323\066\225\071", 4);
          ASSERT (is == 0);
        #endif
        }
        return 0;

      }

  return 1;
}
