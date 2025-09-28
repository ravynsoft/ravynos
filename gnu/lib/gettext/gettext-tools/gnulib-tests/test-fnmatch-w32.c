/* Test of fnmatch string matching function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#include <fnmatch.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "localcharset.h"
#include "macros.h"

#if defined _WIN32 && !defined __CYGWIN__

static int
test_one_locale (const char *name, int codepage)
{
# if 1
  /* Portable code to set the locale.  */
  {
    char name_with_codepage[1024];

    sprintf (name_with_codepage, "%s.%d", name, codepage);

    /* Set the locale.  */
    if (setlocale (LC_ALL, name_with_codepage) == NULL)
      return 77;
  }
# else
  /* Hacky way to set a locale.codepage combination that setlocale() refuses
     to set.  */
  {
    /* Codepage of the current locale, set with setlocale().
       Not necessarily the same as GetACP().  */
    extern __declspec(dllimport) unsigned int __lc_codepage;

    /* Set the locale.  */
    if (setlocale (LC_ALL, name) == NULL)
      return 77;

    /* Clobber the codepage and MB_CUR_MAX, both set by setlocale().  */
    __lc_codepage = codepage;
    switch (codepage)
      {
      case 1252:
      case 1256:
        MB_CUR_MAX = 1;
        break;
      case 932:
      case 950:
      case 936:
        MB_CUR_MAX = 2;
        break;
      case 54936:
      case 65001:
        MB_CUR_MAX = 4;
        break;
      }

    /* Test whether the codepage is really available.  */
    mbstate_t state;
    wchar_t wc;
    memset (&state, '\0', sizeof (mbstate_t));
    if (mbrtowc (&wc, " ", 1, &state) == (size_t)(-1))
      return 77;
  }
# endif

  switch (codepage)
    {
    case 1252:
      /* Locale encoding is CP1252, an extension of ISO-8859-1.  */

      ASSERT (fnmatch ("x?y", "x\374y", 0) == 0); /* "xüy" */
      ASSERT (fnmatch ("x?y", "x\337y", 0) == 0); /* "xßy" */
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\330y", 0) == 0);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\330y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\270y", 0) == 0);
      /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\377y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:print:]]y", "x\270y", 0) == 0);
      /* U+00BF INVERTED QUESTION MARK */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\277y", 0) == 0);
      /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\311y", 0) == 0);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\327y", 0) == FNM_NOMATCH);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\327y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:blank:]]y", "x\330y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:cntrl:]]y", "x\330y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:digit:]]y", "x\330y", 0) == FNM_NOMATCH);
      /* U+00B2 SUPERSCRIPT TWO */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\262y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\330y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\330y", 0) == FNM_NOMATCH);
      /* U+00B2 SUPERSCRIPT TWO */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\262y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:xdigit:]]y", "x\330y", 0) == FNM_NOMATCH);

      #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
      /* "Höhle" */
      ASSERT (fnmatch ("H\366hle", "H\326hLe", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("H\326hLe", "H\366hle", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("H\326hle", "H\366hLe", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("H\366hLe", "H\326hle", FNM_CASEFOLD) == 0);
      #endif

      return 0;

    case 1256:
      /* Locale encoding is CP1256, not the same as ISO-8859-6.  */

      ASSERT (fnmatch ("x?y", "x\302y", 0) == 0); /* "xآy" */
      ASSERT (fnmatch ("x?y", "x\341y", 0) == 0); /* "xلy" */
      ASSERT (fnmatch ("x?y", "x\346y", 0) == 0); /* "xوy" */

      return 0;

    case 65001:
      /* Locale encoding is CP65001 = UTF-8.  */
      if (strcmp (locale_charset (), "UTF-8") != 0)
        return 77;

      ASSERT (fnmatch ("x?y", "x\303\274y", 0) == 0); /* "xüy" */
      ASSERT (fnmatch ("x?y", "x\303\237y", 0) == 0); /* "xßy" */
      ASSERT (fnmatch ("x?y", "x\360\237\230\213y", 0) == 0); /* "x😋y" */
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\305\201y", 0) == 0);
      /* U+10330 GOTHIC LETTER AHSA */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\360\220\214\260y", 0) == 0);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\305\201y", 0) == 0);
      /* U+10330 GOTHIC LETTER AHSA */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\360\220\214\260y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\302\270y", 0) == 0);
      /* U+20000 <CJK Ideograph> */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\360\240\200\200y", 0) == 0);
      /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\303\277y", 0) == 0);
      /* U+10441 DESERET SMALL LETTER EF */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\360\220\221\201y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:print:]]y", "x\302\270y", 0) == 0);
      /* U+20000 <CJK Ideograph> */
      ASSERT (fnmatch ("x[[:print:]]y", "x\360\240\200\200y", 0) == 0);
      /* U+00BF INVERTED QUESTION MARK */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\302\277y", 0) == 0);
      /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\360\235\204\200y", 0) == 0);
      /* U+3000 IDEOGRAPHIC SPACE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\343\200\200y", 0) == 0);
      /* U+0429 CYRILLIC CAPITAL LETTER SHCHA */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\320\251y", 0) == 0);
      /* U+10419 DESERET CAPITAL LETTER EF */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\360\220\220\231y", 0) == 0);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\303\227y", 0) == FNM_NOMATCH);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\303\227y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:blank:]]y", "x\305\201y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:cntrl:]]y", "x\305\201y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:digit:]]y", "x\305\201y", 0) == FNM_NOMATCH);
      /* U+2002 EN SPACE */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\342\200\202y", 0) == FNM_NOMATCH);
      /* U+00B2 SUPERSCRIPT TWO */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\302\262y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\305\201y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\305\201y", 0) == FNM_NOMATCH);
      /* U+00B2 SUPERSCRIPT TWO */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\302\262y", 0) == FNM_NOMATCH);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:xdigit:]]y", "x\305\201y", 0) == FNM_NOMATCH);

      #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
      /* "özgür" */
      {
        /* Some platforms, e.g. MSVC 14, lack the upper/lower mappings for
           these wide characters in the *.65001 locales.  */
        mbstate_t state;
        wchar_t wc;
        memset (&state, 0, sizeof (mbstate_t));
        if (mbrtowc (&wc, "\303\274", 2, &state) == 2
            && towupper (wc) != wc)
          {
            ASSERT (fnmatch ("\303\266zg\303\274r", "\303\226ZG\303\234R", FNM_CASEFOLD) == 0);
            ASSERT (fnmatch ("\303\226ZG\303\234R", "\303\266zg\303\274r", FNM_CASEFOLD) == 0);
            ASSERT (fnmatch ("\303\266Zg\303\234r", "\303\226zG\303\274R", FNM_CASEFOLD) == 0);
            ASSERT (fnmatch ("\303\226zG\303\274R", "\303\266Zg\303\234r", FNM_CASEFOLD) == 0);
          }
      }
      #endif

      return 0;

    case 932:
      /* Locale encoding is CP932, similar to Shift_JIS.  */

      ASSERT (fnmatch ("x?y", "x\223\372y", 0) == 0); /* "x日y" */
      ASSERT (fnmatch ("x?y", "x\226\173y", 0) == 0); /* "x本y" */
      ASSERT (fnmatch ("x?y", "x\214\352y", 0) == 0); /* "x語y" */

      return 0;

    case 950:
      /* Locale encoding is CP950, similar to Big5.  */

      ASSERT (fnmatch ("x?y", "x\244\351y", 0) == 0); /* "x日y" */
      ASSERT (fnmatch ("x?y", "x\245\273y", 0) == 0); /* "x本y" */
      ASSERT (fnmatch ("x?y", "x\273\171y", 0) == 0); /* "x語y" */

      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\242\365y", 0) == 0);
      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\242\365y", 0) == 0);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\241\102y", 0) == 0);
      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\242\365y", 0) == 0);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:print:]]y", "x\241\102y", 0) == 0);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\241\321y", 0) == 0);
      /* U+3000 IDEOGRAPHIC SPACE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\241\100y", 0) == 0);
      /* U+FF2D FULLWIDTH LATIN CAPITAL LETTER M */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\242\333y", 0) == 0);

      #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
      /* "α-ω" */
      ASSERT (fnmatch ("\243\134-\243\163", "\243\104-\243\133", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\243\104-\243\133", "\243\134-\243\163", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\243\134-\243\133", "\243\104-\243\163", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\243\104-\243\163", "\243\134-\243\133", FNM_CASEFOLD) == 0);
      #endif

      return 0;

    case 936:
      /* Locale encoding is CP936 = GBK, an extension of GB2312.  */

      ASSERT (fnmatch ("x?y", "x\310\325y", 0) == 0); /* "x日y" */
      ASSERT (fnmatch ("x?y", "x\261\276y", 0) == 0); /* "x本y" */
      ASSERT (fnmatch ("x?y", "x\325\132y", 0) == 0); /* "x語y" */

      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\243\355y", 0) == 0);
      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\243\355y", 0) == 0);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\241\242y", 0) == 0);
      /* U+FF4D FULLWIDTH LATIN SMALL LETTER M */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\243\355y", 0) == 0);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:print:]]y", "x\241\242y", 0) == 0);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\241\301y", 0) == 0);
      /* U+3000 IDEOGRAPHIC SPACE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\241\241y", 0) == 0);
      /* U+FF2D FULLWIDTH LATIN CAPITAL LETTER M */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\243\315y", 0) == 0);

      #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
      /* "α-ω" */
      ASSERT (fnmatch ("\246\301-\246\330", "\246\241-\246\270", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\246\241-\246\270", "\246\301-\246\330", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\246\301-\246\270", "\246\241-\246\330", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\246\241-\246\330", "\246\301-\246\270", FNM_CASEFOLD) == 0);
      #endif

      return 0;

    case 54936:
      /* Locale encoding is CP54936 = GB18030.  */
      if (strcmp (locale_charset (), "GB18030") != 0)
        return 77;

      ASSERT (fnmatch ("x?y", "x\250\271y", 0) == 0); /* "xüy" */
      ASSERT (fnmatch ("x?y", "x\201\060\211\070y", 0) == 0); /* "xßy" */
      ASSERT (fnmatch ("x?y", "x\224\071\375\067y", 0) == 0); /* "x😋y" */
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\201\060\221\071y", 0) == 0);
      /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\201\060\221\071y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\201\060\206\060y", 0) == 0);
      /* U+20000 <CJK Ideograph> */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\225\062\202\066y", 0) == 0);
      /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\201\060\213\067y", 0) == 0);
      /* U+10441 DESERET SMALL LETTER EF */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\220\060\355\071y", 0) == 0);
      /* U+00B8 CEDILLA */
      ASSERT (fnmatch ("x[[:print:]]y", "x\201\060\206\060y", 0) == 0);
      /* U+20000 <CJK Ideograph> */
      ASSERT (fnmatch ("x[[:print:]]y", "x\225\062\202\066y", 0) == 0);
      /* U+00D7 MULTIPLICATION SIGN */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\241\301y", 0) == 0);
      /* U+1D100 MUSICAL SYMBOL SINGLE BARLINE */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\224\062\273\064y", 0) == 0);
      /* U+3000 IDEOGRAPHIC SPACE */
      ASSERT (fnmatch ("x[[:space:]]y", "x\241\241y", 0) == 0);
      /* U+0429 CYRILLIC CAPITAL LETTER SHCHA */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\247\273y", 0) == 0);
      /* U+10419 DESERET CAPITAL LETTER EF */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\220\060\351\071y", 0) == 0);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:alnum:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:alpha:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:blank:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:cntrl:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:digit:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3000 IDEOGRAPHIC SPACE */
      ASSERT (fnmatch ("x[[:graph:]]y", "x\241\241y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:lower:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
      ASSERT (fnmatch ("x[[:punct:]]y", "x\201\060\211\061y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:space:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:upper:]]y", "x\241\242y", 0) == FNM_NOMATCH);
      /* U+3001 IDEOGRAPHIC COMMA */
      ASSERT (fnmatch ("x[[:xdigit:]]y", "x\241\242y", 0) == FNM_NOMATCH);

      #if GNULIB_FNMATCH_GNU && defined FNM_CASEFOLD
      /* "özgür" */
      ASSERT (fnmatch ("\201\060\213\062zg\250\271r", "\201\060\211\060ZG\201\060\211\065R", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\201\060\211\060ZG\201\060\211\065R", "\201\060\213\062zg\250\271r", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\201\060\213\062Zg\201\060\211\065r", "\201\060\211\060zG\250\271R", FNM_CASEFOLD) == 0);
      ASSERT (fnmatch ("\201\060\211\060zG\250\271R", "\201\060\213\062Zg\201\060\211\065r", FNM_CASEFOLD) == 0);
      #endif

      return 0;

    default:
      return 1;
    }
}

int
main (int argc, char *argv[])
{
  int codepage = atoi (argv[argc - 1]);
  int result;
  int i;

  result = 77;
  for (i = 1; i < argc - 1; i++)
    {
      int ret = test_one_locale (argv[i], codepage);

      if (ret != 77)
        result = ret;
    }

  if (result == 77)
    {
      fprintf (stderr, "Skipping test: found no locale with codepage %d\n",
               codepage);
    }
  return result;
}

#else

int
main (int argc, char *argv[])
{
  fputs ("Skipping test: not a native Windows system\n", stderr);
  return 77;
}

#endif
