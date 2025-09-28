/* seq - print sequence of numbers to standard output.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.

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

/* Written as bash builtin by Chet Ramey. Portions from seq.c by Ulrich Drepper. */

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "bashansi.h"
#include "loadables.h"
#include "bashintl.h"

#ifndef errno
extern int errno;
#endif

#if defined (PRI_MACROS_BROKEN)
#  undef PRIdMAX
#endif

#if !defined (PRIdMAX)
#  if HAVE_LONG_LONG
#    define PRIdMAX     "lld"
#  else
#    define PRIdMAX     "ld"
#  endif
#endif

#if defined (HAVE_LONG_DOUBLE) && HAVE_DECL_STRTOLD && !defined(STRTOLD_BROKEN)
typedef long double floatmax_t;
#  define FLOATMAX_CONV "L"
#  define strtofltmax   strtold
#  define FLOATMAX_FMT "%Lg"
#  define FLOATMAX_WFMT "%0.Lf"
#  define USE_LONG_DOUBLE
#else
typedef double floatmax_t;
#  define FLOATMAX_CONV ""
#  define strtofltmax   strtod
#  define FLOATMAX_FMT "%g"
#  define FLOATMAX_WFMT "%0.f"
#endif
static floatmax_t getfloatmax PARAMS((const char *));
static char *genformat PARAMS((floatmax_t, floatmax_t, floatmax_t));

#define MAX(a, b) (((a) < (b))? (b) : (a))

static int conversion_error = 0;

/* If true print all number with equal width.  */
static int equal_width;

/* The string used to separate two numbers.  */
static char const *separator;

/* The string output after all numbers have been output. */
static char const terminator[] = "\n";

static char decimal_point;

/* Pretty much the same as the version in builtins/printf.def */
static floatmax_t
getfloatmax (arg)
     const char *arg;
{
  floatmax_t ret;
  char *ep;

  errno = 0;
  ret = strtofltmax (arg, &ep);

  if (*ep)
    {
      sh_invalidnum ((char *)arg);
      conversion_error = 1;
    }
  else if (errno == ERANGE)
    {
      builtin_error ("warning: %s: %s", arg, strerror(ERANGE));
      conversion_error = 1;
    }

  if (ret == -0.0)
    ret = 0.0;

  return (ret);
}

/* If FORMAT is a valid printf format for a double argument, return
   its long double equivalent, allocated from dynamic storage. This
   was written by Ulrich Drepper, taken from coreutils:seq.c */
static char *
long_double_format (char const *fmt)
{
  size_t i;
  size_t length_modifier_offset;
  int has_L;

  for (i = 0; ! (fmt[i] == '%' && fmt[i + 1] != '%'); i += (fmt[i] == '%') + 1)
    {
      if (!fmt[i])
	{
	  builtin_error ("format %s has no %% directive", fmt);
	  return 0;
	}
    }

  i++;
  i += strspn (fmt + i, "-+#0 '");	/* zero or more flags */
  i += strspn (fmt + i, "0123456789");	/* optional minimum field width */
  if (fmt[i] == '.')			/* optional precision */
    {
      i++;
      i += strspn (fmt + i, "0123456789");
    }

  length_modifier_offset = i;		/* optional length modifier */
  /* we could ignore an 'l' length modifier here */
  has_L = (fmt[i] == 'L');
  i += has_L;
  switch (fmt[i])
    {
    case '\0':
      builtin_error ("format %s ends in %%", fmt);
      return 0;
    case 'A':
    case 'a':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G':
      break;
    default:
      builtin_error ("format %s has unknown `%%%c' directive", fmt, fmt[i]);
      return 0;
    }
  for (i++; ; i += (fmt[i] == '%') + 1)
    if (fmt[i] == '%' && fmt[i + 1] != '%')
      {
        builtin_error ("format %s has too many %% directives", fmt);
        return 0;
      }
    else if (fmt[i] == 0)
      {
        size_t format_size = i + 1;
        char *ldfmt = xmalloc (format_size + 1);
        memcpy (ldfmt, fmt, length_modifier_offset);
#ifdef USE_LONG_DOUBLE
        ldfmt[length_modifier_offset] = 'L';
        strcpy (ldfmt + length_modifier_offset + 1,
                fmt + length_modifier_offset + has_L);
#else
        strcpy (ldfmt + length_modifier_offset, fmt + length_modifier_offset);
#endif
        return ldfmt;
      }
}

/* Return the number of digits following the decimal point in NUMBUF */
static int
getprec (numbuf)
     const char *numbuf;
{
  int p;
  char *dp;

  if (dp = strchr (numbuf, decimal_point))
    dp++;		/* skip over decimal point */
  for (p = 0; dp && *dp && ISDIGIT (*dp); dp++)
    p++;
  return p;
}

/* Return the default format given FIRST, INCR, and LAST.  */
static char *
genformat (first, incr, last)
       floatmax_t first, incr, last;
{
  static char buf[6 + 2 * INT_STRLEN_BOUND (int)];
  int wfirst, wlast, width;
  int iprec, fprec, lprec, prec;

  if (equal_width == 0)
    return (FLOATMAX_FMT);

  /* OK, we have to figure out the largest number of decimal places. This is
     a little more expensive than using the original strings. */
  snprintf (buf, sizeof (buf), FLOATMAX_FMT, incr);
  iprec = getprec (buf);

  wfirst = snprintf (buf, sizeof (buf), FLOATMAX_FMT, first);
  fprec = getprec (buf);
    
  prec = MAX (fprec, iprec);

  wlast = snprintf (buf, sizeof (buf), FLOATMAX_FMT, last);
  lprec = getprec (buf);

  /* increase first width by any increased precision in increment */
  wfirst += (prec - fprec);

  /* adjust last width to use precision from first/incr */
  wlast += (prec - lprec);

  if (lprec && prec == 0)
    wlast--;		/* no decimal point */
  if (lprec == 0 && prec)
    wlast++;		/* include decimal point */
  if (fprec == 0 && prec)
    wfirst++;		/* include decimal point */

  width = MAX (wfirst, wlast);
  if (width)
    sprintf (buf, "%%0%d.%d%sf", width, prec, FLOATMAX_CONV);
  else
    sprintf (buf, "%%.%d%sf", prec, FLOATMAX_CONV);

  return buf;
}

int
print_fltseq (fmt, first, last, incr)
     const char *fmt;
     floatmax_t first, last, incr;
{
  int n;
  floatmax_t next;
  const char *s;

  n = 0;		/* iteration counter */
  s = "";
  for (next = first; incr >= 0 ? (next <= last) : (next >= last); next = first + n * incr)
    {
      QUIT;
      if (*s && fputs (s, stdout) == EOF)
	return (sh_chkwrite (EXECUTION_FAILURE));
      if (printf (fmt, next) < 0)
	return (sh_chkwrite (EXECUTION_FAILURE));
      s = separator;
      n++;
    }

  if (n > 0 && fputs (terminator, stdout) == EOF)
    return (sh_chkwrite (EXECUTION_FAILURE));
  return (sh_chkwrite (EXECUTION_SUCCESS));
}

/* must be <= INT_STRLEN_BOUND(intmax_t) */
int
width_needed (num)
     intmax_t num;
{
  int ret;

  ret = num < 0;		/* sign */
  if (ret)
    num = -num;
  do
    ret++;
  while (num /= 10);
  return ret;
}

int
print_intseq (ifirst, ilast, iincr)
     intmax_t ifirst, ilast, iincr;
{
  char intwfmt[6 + INT_STRLEN_BOUND(int) + sizeof (PRIdMAX)];
  const char *s;
  intmax_t i, next;

  /* compute integer format string */
  if (equal_width)	/* -w supplied */
    {
      int wfirst, wlast, width;

      wfirst = width_needed (ifirst);
      wlast = width_needed (ilast);
      width = MAX(wfirst, wlast);

      /* The leading %s is for the separator */
      snprintf (intwfmt, sizeof (intwfmt), "%%s%%0%u" PRIdMAX, width);
    }

  /* We could use braces.c:mkseq here but that allocates lots of memory */
  s = "";
  for (i = ifirst; (ifirst <= ilast) ? (i <= ilast) : (i >= ilast); i = next)
    {
      QUIT;
      /* The leading %s is for the separator */
      if (printf (equal_width ?  intwfmt : "%s%" PRIdMAX, s, i) < 0)
	return (sh_chkwrite (EXECUTION_FAILURE));
      s = separator;
      next = i + iincr;
    }

  if (fputs (terminator, stdout) == EOF)
    return (sh_chkwrite (EXECUTION_FAILURE));
  return (sh_chkwrite (EXECUTION_SUCCESS));
}

int
seq_builtin (list)
     WORD_LIST *list;
{
  floatmax_t first, last, incr;
  intmax_t ifirst, ilast, iincr;
  WORD_LIST *l;
  int opt, nargs, intseq, freefmt;
  char *first_str, *incr_str, *last_str;
  char const *fmtstr;		/* The printf(3) format used for output.  */

  equal_width = 0;
  separator = "\n";
  fmtstr = NULL;

  first = 1.0;
  last = 0.0;
  incr = 0.0;		/* set later */
  ifirst = ilast = iincr = 0;
  first_str = incr_str = last_str = 0;

  intseq = freefmt = 0;
  opt = 0;

  reset_internal_getopt ();
  while (opt != -1)
    {
      l = lcurrent ? lcurrent : list;
      if (l && l->word && l->word->word && l->word->word[0] == '-' &&
	   (l->word->word[1] == '.' || DIGIT (l->word->word[1])))
	{
	  loptend = l;
	  break;	/* negative number */
	}
      if ((opt = internal_getopt (list, "f:s:w")) == -1)
        break;

      switch (opt)
	{
	case 'f':
	  fmtstr = list_optarg;
	  break;
	case 's':
	  separator = list_optarg;
	  break;
	case 'w':
	  equal_width = 1;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (list == 0)
    {
      builtin_usage ();
      return (EXECUTION_FAILURE);
    }

  for (nargs = 1, l = list; l->next; l = l->next)
    nargs++;
  if (nargs > 3)
    {
      builtin_usage ();
      return (EXECUTION_FAILURE);
    }

  /* LAST */
  conversion_error = 0;
  last = getfloatmax (last_str = l->word->word);
  if (conversion_error)
    return (EXECUTION_FAILURE);

  /* FIRST LAST */
  if (nargs > 1)
    {
      conversion_error = 0;
      first = getfloatmax (first_str = list->word->word);
      if (conversion_error)
	return (EXECUTION_FAILURE);
    }

  /* FIRST INCR LAST */
  if (nargs > 2)
    {
      conversion_error = 0;
      incr = getfloatmax (incr_str = list->next->word->word);
      if (conversion_error)
	return (EXECUTION_FAILURE);
      if (incr == 0.0)
	{
	  builtin_error ("zero %screment", (first < last) ?  "in" : "de");
	  return (EXECUTION_FAILURE);
	}
    }

  /* Sanitize arguments */
  if (incr == 0.0)
    incr = (first <= last) ?  1.0 : -1.0;
  if ((incr < 0.0 && first < last) || (incr > 0 && first > last))
    {
      builtin_error ("incorrect %screment", (first < last) ?  "in" : "de");
      return (EXECUTION_FAILURE);
    }

  /* validate format here */
  if (fmtstr)
    {
      fmtstr = long_double_format (fmtstr);
      freefmt = 1;
      if (fmtstr == 0)
	return (EXECUTION_FAILURE);
    }
      
  if (fmtstr != NULL && equal_width)
    {
      builtin_warning ("-w ignored when the format string is specified");
      equal_width = 0;
    }

  /* Placeholder for later additional conditions */
  if (last_str && all_digits (last_str) &&
      (first_str == 0 || all_digits (first_str)) &&
      (incr_str == 0 || all_digits (incr_str)) &&
      fmtstr == NULL)
    intseq = 1;

  if (intseq)
    {
      ifirst = (intmax_t)first; /* truncation */
      ilast = (intmax_t)last;
      iincr = (intmax_t)incr;

      return (print_intseq (ifirst, ilast, iincr));
    }

  decimal_point = locale_decpoint ();
  if (fmtstr == NULL)
    fmtstr = genformat (first, incr, last);

  print_fltseq (fmtstr, first, last, incr);

  if (freefmt)
    free ((void *)fmtstr);
  return sh_chkwrite (EXECUTION_SUCCESS);
}

/* Taken largely from GNU seq. */
char *seq_doc[] = {
	"Print numbers from FIRST to LAST, in steps of INCREMENT.",
	"",
	"-f FORMAT    use printf style floating-point FORMAT",
	"-s STRING    use STRING to separate numbers (default: \\n)",
	"-w           equalize width by padding with leading zeroes",
	"",
	"If FIRST or INCREMENT is omitted, it defaults to 1.  However, an",
	"omitted INCREMENT defaults to -1 when LAST is smaller than FIRST.",
	"The sequence of numbers ends when the sum of the current number and",
	"INCREMENT would become greater than LAST.",
	"FIRST, INCREMENT, and LAST are interpreted as floating point values.",
	"",
	"FORMAT must be suitable for printing one argument of type 'double';",
	"it defaults to %.PRECf if FIRST, INCREMENT, and LAST are all fixed point",
	"decimal numbers with maximum precision PREC, and to %g otherwise.",
	(char *)NULL
};	

struct builtin seq_struct = {
	"seq",
	seq_builtin,
	BUILTIN_ENABLED,
	seq_doc,
	"seq [-f format] [-s separator] [-w] [FIRST [INCR]] LAST",
	0
};
