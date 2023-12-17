/* snprintf - formatted output to strings, with bounds checking and allocation */

/*
 build a test version with
   gcc -g -DDRIVER -I../.. -I../../include -o test-snprintf snprintf.c fmtu*long.o
*/
 
/*
   Unix snprintf implementation.
   derived from inetutils/libinetutils/snprintf.c Version 1.1

   Copyright (C) 2001-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
   
   Original (pre-bash) Revision History:

   1.1:
      *  added changes from Miles Bader
      *  corrected a bug with %f
      *  added support for %#g
      *  added more comments :-)
   1.0:
      *  supporting must ANSI syntaxic_sugars
   0.0:
      *  support %s %c %d

 THANKS(for the patches and ideas):
     Miles Bader
     Cyrille Rustom
     Jacek Slabocewiz
     Mike Parker(mouse)

*/

/*
 * Currently doesn't handle (and bash/readline doesn't use):
 *	* *M$ width, precision specifications
 *	* %N$ numbered argument conversions
 *	* support for `F' is imperfect with ldfallback(), since underlying
 *	  printf may not handle it -- should ideally have another autoconf test
 */

#define FLOATING_POINT

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* GCC 4.2 on Snow Leopard doesn't like the snprintf prototype */
#if defined(DEBUG) && !defined (MACOSX)
#  undef HAVE_SNPRINTF
#  undef HAVE_ASPRINTF

#  define HAVE_SNPRINTF 0
#  define HAVE_ASPRINTF 0
#endif

#if defined(DRIVER) && !defined(HAVE_CONFIG_H)
#define HAVE_LONG_LONG_INT
#define HAVE_LONG_DOUBLE
#ifdef __linux__
#define HAVE_PRINTF_A_FORMAT
#endif
#define HAVE_ISINF_IN_LIBC
#define HAVE_ISNAN_IN_LIBC
#define PREFER_STDARG
#define HAVE_STRINGIZE
#define HAVE_LIMITS_H
#define HAVE_STDDEF_H
#define HAVE_LOCALE_H
#define intmax_t long
#endif

#if !HAVE_SNPRINTF || !HAVE_ASPRINTF

#include <bashtypes.h>

#if defined(PREFER_STDARG)
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif
#include <bashansi.h>
#ifdef HAVE_STDDEF_H
#  include <stddef.h>
#endif
#include <chartypes.h>

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif

#ifdef FLOATING_POINT
#  include <float.h>	/* for manifest constants */
#  include <stdio.h>	/* for sprintf */
#endif

#include <typemax.h>

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

#include "stdc.h"
#include <shmbutil.h>

#ifndef DRIVER
#  include "shell.h"
#else
#  define FL_PREFIX     0x01    /* add 0x, 0X, or 0 prefix as appropriate */
#  define FL_ADDBASE    0x02    /* add base# prefix to converted value */
#  define FL_HEXUPPER   0x04    /* use uppercase when converting to hex */
#  define FL_UNSIGNED   0x08    /* don't add any sign */
extern char *fmtulong PARAMS((unsigned long int, int, char *, size_t, int));
extern char *fmtullong PARAMS((unsigned long long int, int, char *, size_t, int));
#endif

#ifndef FREE
#  define FREE(x)	if (x) free (x)
#endif

/* Bound on length of the string representing an integer value of type T.
   Subtract one for the sign bit if T is signed;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if t is signed.  */
#ifndef INT_STRLEN_BOUND
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - TYPE_SIGNED (t)) * 302 / 1000 \
     + 1 + TYPE_SIGNED (t))
#endif

/* conversion flags */
#define PF_ALTFORM	0x00001		/* # */
#define PF_HEXPREFIX	0x00002		/* 0[Xx] */
#define PF_LADJUST	0x00004		/* - */
#define PF_ZEROPAD	0x00008		/* 0 */
#define PF_PLUS		0x00010		/* + */
#define PF_SPACE	0x00020		/* ' ' */
#define PF_THOUSANDS	0x00040		/* ' */

#define PF_DOT		0x00080		/* `.precision' */
#define PF_STAR_P	0x00100		/* `*' after precision */
#define PF_STAR_W	0x00200		/* `*' before or without precision */

/* length modifiers */
#define PF_SIGNEDCHAR	0x00400		/* hh */
#define PF_SHORTINT	0x00800		/* h */
#define PF_LONGINT	0x01000		/* l */
#define PF_LONGLONG	0x02000		/* ll */
#define PF_LONGDBL	0x04000		/* L */
#define PF_INTMAX_T	0x08000		/* j */
#define PF_SIZE_T	0x10000		/* z */
#define PF_PTRDIFF_T	0x20000		/* t */

#define PF_ALLOCBUF	0x40000		/* for asprintf, vasprintf */

#define PFM_SN		0x01		/* snprintf, vsnprintf */
#define PFM_AS		0x02		/* asprintf, vasprintf */

#define ASBUFSIZE	128

#define x_digs	"0123456789abcdef"
#define X_digs	"0123456789ABCDEF"

static char intbuf[INT_STRLEN_BOUND(unsigned long) + 1];

static int decpoint;
static int thoussep;
static char *grouping;

/* 
 * For the FLOATING POINT FORMAT :
 *  the challenge was finding a way to
 *  manipulate the Real numbers without having
 *  to resort to mathematical function(it
 *  would require to link with -lm) and not
 *  going down to the bit pattern(not portable)
 *
 *  so a number, a real is:

      real = integral + fraction

      integral = ... + a(2)*10^2 + a(1)*10^1 + a(0)*10^0
      fraction = b(1)*10^-1 + b(2)*10^-2 + ...

      where:
       0 <= a(i) => 9 
       0 <= b(i) => 9 
 
    from then it was simple math
 */

/*
 * size of the buffer for the integral part
 * and the fraction part 
 */
#define MAX_INT  99 + 1 /* 1 for the null */
#define MAX_FRACT 307 + 1

/* 
 * These functions use static buffers to store the results,
 * and so are not reentrant
 */
#define itoa(n) fmtulong(n, 10, intbuf, sizeof(intbuf), 0);
#define dtoa(n, p, f) numtoa(n, 10, p, f)

#define SWAP_INT(a,b) {int t; t = (a); (a) = (b); (b) = t;}

#define GETARG(type)	(va_arg(args, type))

/* Macros that do proper sign extension and handle length modifiers.  Used
   for the integer conversion specifiers. */
#define GETSIGNED(p) \
  (((p)->flags & PF_LONGINT) \
	? GETARG (long) \
  	: (((p)->flags & PF_SHORTINT) ? (long)(short)GETARG (int) \
				      : (long)GETARG (int)))

#define GETUNSIGNED(p) \
  (((p)->flags & PF_LONGINT) \
	? GETARG (unsigned long) \
	: (((p)->flags & PF_SHORTINT) ? (unsigned long)(unsigned short)GETARG (int) \
				      : (unsigned long)GETARG (unsigned int)))


#ifdef HAVE_LONG_DOUBLE
#define GETLDOUBLE(p) GETARG (long double)
#endif
#define GETDOUBLE(p) GETARG (double)

#define SET_SIZE_FLAGS(p, type) \
  if (sizeof (type) > sizeof (int)) \
    (p)->flags |= PF_LONGINT; \
  if (sizeof (type) > sizeof (long)) \
    (p)->flags |= PF_LONGLONG;

/* this struct holds everything we need */
struct DATA
{
  int length;
  char *base;		/* needed for [v]asprintf */
  char *holder;
  int counter;
  const char *pf;

/* FLAGS */
  int flags;
  int justify;
  int width, precision;
  char pad;
};

/* the floating point stuff */
#ifdef FLOATING_POINT
static double pow_10 PARAMS((int));
static int log_10 PARAMS((double));
static double integral PARAMS((double, double *));
static char *numtoa PARAMS((double, int, int, char **));
#endif

static void init_data PARAMS((struct DATA *, char *, size_t, const char *, int));
static void init_conv_flag PARAMS((struct DATA *));

/* for the format */
#ifdef FLOATING_POINT
static void floating PARAMS((struct DATA *, double));
static void exponent PARAMS((struct DATA *, double));
#endif
static void number PARAMS((struct DATA *, unsigned long, int));
#ifdef HAVE_LONG_LONG_INT
static void lnumber PARAMS((struct DATA *, unsigned long long, int));
#endif
static void pointer PARAMS((struct DATA *, unsigned long));
static void strings PARAMS((struct DATA *, char *));

#ifdef FLOATING_POINT
#  define FALLBACK_FMTSIZE	32
#  define FALLBACK_BASE		4096
#  define LFALLBACK_BASE	5120
#  ifdef HAVE_LONG_DOUBLE
static void ldfallback PARAMS((struct DATA *, const char *, const char *, long double));
#  endif
static void dfallback PARAMS((struct DATA *, const char *, const char *, double));
#endif

static char *groupnum PARAMS((char *));

#if defined (HAVE_LONG_DOUBLE)
#  define LONGDOUBLE long double
#else
#  define LONGDOUBLE double
#endif

#ifndef isnan
  static inline int isnan_f  (float       x) { return x != x; }
  static inline int isnan_d  (double      x) { return x != x; }
  static inline int isnan_ld (LONGDOUBLE  x) { return x != x; }
  # define isnan(x) \
      (sizeof (x) == sizeof (LONGDOUBLE) ? isnan_ld (x) \
       : sizeof (x) == sizeof (double) ? isnan_d (x) \
       : isnan_f (x))
#endif
  
#ifndef isinf
  static inline int isinf_f  (float       x) { return !isnan (x) && isnan (x - x); }
  static inline int isinf_d  (double      x) { return !isnan (x) && isnan (x - x); }
  static inline int isinf_ld (LONGDOUBLE  x) { return !isnan (x) && isnan (x - x); }
  # define isinf(x) \
      (sizeof (x) == sizeof (LONGDOUBLE) ? isinf_ld (x) \
       : sizeof (x) == sizeof (double) ? isinf_d (x) \
       : isinf_f (x))
#endif

#ifdef DRIVER
static void memory_error_and_abort ();
static void *xmalloc PARAMS((size_t));
static void *xrealloc PARAMS((void *, size_t));
static void xfree PARAMS((void *));
#else
#  include <xmalloc.h>
#endif

/* those are defines specific to snprintf to hopefully
 * make the code clearer :-)
 */
#define RIGHT 1
#define LEFT  0
#define NOT_FOUND -1
#define FOUND 1
#define MAX_FIELD 15

/* round off to the precision */
#define ROUND(d, p) \
	    (d < 0.) ? \
	     d - pow_10(-(p)->precision) * 0.5 : \
	     d + pow_10(-(p)->precision) * 0.5

/* set default precision */
#define DEF_PREC(p) \
	    if ((p)->precision == NOT_FOUND) \
	      (p)->precision = 6

/* put a char.  increment the number of chars written even if we've exceeded
   the vsnprintf/snprintf buffer size (for the return value) */
#define PUT_CHAR(c, p) \
	do \
	  { \
	    if (((p)->flags & PF_ALLOCBUF) && ((p)->counter >= (p)->length - 1)) \
	      { \
		(p)->length += ASBUFSIZE; \
		(p)->base = (char *)xrealloc((p)->base, (p)->length); \
		(p)->holder = (p)->base + (p)->counter; /* in case reallocated */ \
	      } \
	    if ((p)->counter < (p)->length) \
	      *(p)->holder++ = (c); \
	    (p)->counter++; \
	  } \
	while (0)

/* Output a string.  P->WIDTH has already been adjusted for padding. */
#define PUT_STRING(string, len, p) \
	do \
	  { \
	    PAD_RIGHT (p); \
	    while ((len)-- > 0) \
	      { \
		PUT_CHAR (*(string), (p)); \
		(string)++; \
	      } \
	    PAD_LEFT (p); \
	  } \
	while (0)

#define PUT_PLUS(d, p, zero) \
	    if (((p)->flags & PF_PLUS) && (d) > zero) \
	      PUT_CHAR('+', p)

#define PUT_SPACE(d, p, zero) \
	    if (((p)->flags & PF_SPACE) && (d) > zero) \
	      PUT_CHAR(' ', p)

/* pad right */ 
#define PAD_RIGHT(p) \
	    if ((p)->width > 0 && (p)->justify != LEFT) \
	      for (; (p)->width > 0; (p)->width--) \
		 PUT_CHAR((p)->pad, p)

/* pad left */
#define PAD_LEFT(p) \
	    if ((p)->width > 0 && (p)->justify == LEFT) \
	      for (; (p)->width > 0; (p)->width--) \
		 PUT_CHAR((p)->pad, p)

/* pad with zeros from decimal precision */
#define PAD_ZERO(p) \
	if ((p)->precision > 0) \
	  for (; (p)->precision > 0; (p)->precision--) \
	    PUT_CHAR('0', p)

/* if width and prec. in the args */
#define STAR_ARGS(p) \
	do { \
	    if ((p)->flags & PF_STAR_W) \
	      { \
		(p)->width = GETARG (int); \
		if ((p)->width < 0) \
		  { \
		    (p)->flags |= PF_LADJUST; \
		    (p)->justify = LEFT; \
		    (p)->width = -(p)->width; \
		  } \
	      } \
	    if ((p)->flags & PF_STAR_P) \
	      { \
		(p)->precision = GETARG (int); \
		if ((p)->precision < 0) \
		  { \
		    (p)->flags &= ~PF_STAR_P; \
		    (p)->precision = NOT_FOUND; \
		  } \
	      } \
	} while (0)

#if defined (HAVE_LOCALE_H) && defined (HAVE_LOCALECONV)
#  define GETLOCALEDATA(d, t, g) \
      do \
	{ \
	  struct lconv *lv; \
	  if ((d) == 0) { \
	  (d) = '.'; (t) = -1; (g) = 0; /* defaults */ \
	  lv = localeconv(); \
	  if (lv) \
	    { \
	      if (lv->decimal_point && lv->decimal_point[0]) \
		(d) = lv->decimal_point[0]; \
	      if (lv->thousands_sep && lv->thousands_sep[0]) \
		(t) = lv->thousands_sep[0]; \
	      (g) = lv->grouping ? lv->grouping : ""; \
	      if (*(g) == '\0' || *(g) == CHAR_MAX || (t) == -1) (g) = 0; \
	    } \
	  } \
	} \
      while (0);
#else
#  define GETLOCALEDATA(d, t, g) \
      ( (d) = '.', (t) = ',', g = "\003" )
#endif

#ifdef FLOATING_POINT
/*
 * Find the nth power of 10
 */
static double
pow_10(n)
     int n;
{ 
  double P;

  /* handle common cases with fast switch statement. */
  switch (n)
    {
    case -3:	return .001;
    case -2:	return .01;
    case -1:	return .1;
    case 0:	return 1.;
    case 1:	return 10.;
    case 2:	return 100.;
    case 3:	return 1000.;
    }

  if (n < 0)
    {
      P = .0001;
      for (n += 4; n < 0; n++)
	P /= 10.;
    }
  else
    {
      P = 10000.;
      for (n -= 4; n > 0; n--)
	P *= 10.;
    }

  return P;
}

/*
 * Find the integral part of the log in base 10 
 * Note: this not a real log10()
	 I just need and approximation(integerpart) of x in:
	  10^x ~= r
 * log_10(200) = 2;
 * log_10(250) = 2;
 *
 * NOTE: do not call this with r == 0 -- an infinite loop results.
 */
static int
log_10(r)
     double r;
{ 
  int i = 0;
  double result = 1.;

  if (r < 0.)
    r = -r;

  if (r < 1.)
    {
      while (result >= r)
	{
	  result /= 10.;
	  i++;
	}
      return (-i);
    }
  else
    {
      while (result <= r)
	{
	  result *= 10.;
	  i++;
	}
      return (i - 1);
    }
}

/*
 * This function return the fraction part of a double
 * and set in ip the integral part.
 * In many ways it resemble the modf() found on most Un*x
 */
static double
integral(real, ip)
     double real;
     double *ip;
{ 
  int j;
  double i, s, p;
  double real_integral = 0.;

  /* take care of the obvious */
  /* equal to zero ? */
  if (real == 0.)
    {
      *ip = 0.;
      return (0.);
    }

  /* negative number ? */
  if (real < 0.)
    real = -real;

  /* a fraction ? */
  if ( real < 1.)
    {
      *ip = 0.;
      return real;
    }

  /* the real work :-) */
  for (j = log_10(real); j >= 0; j--)
    {
      p = pow_10(j);
      s = (real - real_integral)/p;
      i = 0.;
      while (i + 1. <= s)
	i++;
      real_integral += i*p;
    }
  *ip = real_integral;
  return (real - real_integral);
}

#define PRECISION 1.e-6
/* 
 * return an ascii representation of the integral part of the number
 * and set fract to be an ascii representation of the fraction part
 * the container for the fraction and the integral part or statically
 * declare with fix size 
 */
static char *
numtoa(number, base, precision, fract)
     double number;
     int base, precision;
     char **fract;
{
  register int i, j;
  double ip, fp; /* integer and fraction part */
  double fraction;
  int digits, sign;
  static char integral_part[MAX_INT];
  static char fraction_part[MAX_FRACT];
  int ch;

  /* taking care of the obvious case: 0.0 */
  if (number == 0.)
    { 
      integral_part[0] = '0';
      integral_part[1] = '\0';
      /* The fractional part has to take the precision into account */
      for (ch = 0; ch < precision-1; ch++)
 	fraction_part[ch] = '0';
      fraction_part[ch] = '0';
      fraction_part[ch+1] = '\0';
      if (fract)
	*fract = fraction_part;
      return integral_part;
    }

  /* -0 is tricky */
  sign = (number == -0.) ? '-' : ((number < 0.) ? '-' : '+');
  digits = MAX_INT - 1;

  /* for negative numbers */
  if (sign == '-')
    {
      number = -number;
      digits--; /* sign consume one digit */
    }

  fraction = integral(number, &ip);
  number = ip;

  /* do the integral part */
  if (ip == 0.)
    {
      integral_part[0] = '0';
      i = 1;
    }
  else
    {
      for ( i = 0; i < digits && number != 0.; ++i)
	{
	  number /= base;
	  fp = integral(number, &ip);
	  ch = (int)((fp + PRECISION)*base); /* force to round */
	  integral_part[i] = (ch <= 9) ? ch + '0' : ch + 'a' - 10;
	  if (! ISXDIGIT((unsigned char)integral_part[i]))
	    break;	/* bail out overflow !! */
	  number = ip;
	 }
    }
     
  /* Oh No !! out of bound, ho well fill it up ! */
  if (number != 0.)
    for (i = 0; i < digits; ++i)
      integral_part[i] = '9';

  /* put the sign ? */
  if (sign == '-')
    integral_part[i++] = '-';

  integral_part[i] = '\0';

  /* reverse every thing */
  for ( i--, j = 0; j < i; j++, i--)
    SWAP_INT(integral_part[i], integral_part[j]);  

  /* the fractional part */
  for (i=0, fp=fraction; precision > 0 && i < MAX_FRACT ; i++, precision--)
    {
      fraction_part[i] = (int)((fp + PRECISION)*10. + '0');
      if (! DIGIT(fraction_part[i])) /* underflow ? */
	break;
      fp = (fp*10.0) - (double)(long)((fp + PRECISION)*10.);
    }
  fraction_part[i] = '\0';

  if (fract != (char **)0)
    *fract = fraction_part;

  return integral_part;
}
#endif

/* for %d and friends, it puts in holder
 * the representation with the right padding
 */
static void
number(p, d, base)
     struct DATA *p;
     unsigned long d;
     int base;
{
  char *tmp, *t;
  long sd;
  int flags;

  /* An explicit precision turns off the zero-padding flag and sets the
     pad character back to space. */
  if ((p->flags & PF_ZEROPAD) && p->precision >= 0 && (p->flags & PF_DOT))
    {
      p->flags &= ~PF_ZEROPAD;
      p->pad = ' ';
    }

  sd = d;	/* signed for ' ' padding in base 10 */
  flags = 0;
  flags = (*p->pf == 'x' || *p->pf == 'X' || *p->pf == 'o' || *p->pf == 'u' || *p->pf == 'U') ? FL_UNSIGNED : 0;
  if (*p->pf == 'X')
    flags |= FL_HEXUPPER;

  tmp = fmtulong (d, base, intbuf, sizeof(intbuf), flags);
  t = 0;
  if ((p->flags & PF_THOUSANDS))
    {
      GETLOCALEDATA(decpoint, thoussep, grouping);
      if (grouping && (t = groupnum (tmp)))
	tmp = t;
    }

  /* need to add one for any `+', but we only add one in base 10 */
  p->width -= strlen(tmp) + (base == 10 && d > 0 && (p->flags & PF_PLUS));
  PAD_RIGHT(p);

  if ((p->flags & PF_DOT) && p->precision > 0)
    {
      p->precision -= strlen(tmp);
      PAD_ZERO(p);
    }

  switch (base)
    {
    case 10:
      PUT_PLUS(sd, p, 0);
      PUT_SPACE(sd, p, 0);
      break;
    case 8:
      if (p->flags & PF_ALTFORM)
	PUT_CHAR('0', p);
      break;
    case 16:
      if (p->flags & PF_ALTFORM)
	{
	  PUT_CHAR('0', p);
	  PUT_CHAR(*p->pf, p);
	}
      break;
    }

  while (*tmp)
    {
      PUT_CHAR(*tmp, p);
      tmp++;
    }

  PAD_LEFT(p);
  FREE (t);
}

#ifdef HAVE_LONG_LONG_INT
/*
 * identical to number() but works for `long long'
 */
static void
lnumber(p, d, base)
     struct DATA *p;
     unsigned long long d;
     int base;
{
  char *tmp, *t;
  long long sd;
  int flags;

  /* An explicit precision turns off the zero-padding flag and sets the
     pad character back to space. */
  if ((p->flags & PF_ZEROPAD) && p->precision >= 0 && (p->flags & PF_DOT))
    {
      p->flags &= ~PF_ZEROPAD;
      p->pad = ' ';
    }

  sd = d;	/* signed for ' ' padding in base 10 */
  flags = (*p->pf == 'x' || *p->pf == 'X' || *p->pf == 'o' || *p->pf == 'u' || *p->pf == 'U') ? FL_UNSIGNED : 0;
  if (*p->pf == 'X')
    flags |= FL_HEXUPPER;

  tmp = fmtullong (d, base, intbuf, sizeof(intbuf), flags);
  t = 0;
  if ((p->flags & PF_THOUSANDS))
    {
      GETLOCALEDATA(decpoint, thoussep, grouping);
      if (grouping && (t = groupnum (tmp)))
	tmp = t;
    }

  /* need to add one for any `+', but we only add one in base 10 */
  p->width -= strlen(tmp) + (base == 10 && d > 0 && (p->flags & PF_PLUS));
  PAD_RIGHT(p);

  if ((p->flags & PF_DOT) && p->precision > 0)
    {
      p->precision -= strlen(tmp);
      PAD_ZERO(p);
    }

  switch (base)
    {
    case 10:
      PUT_PLUS(sd, p, 0);
      PUT_SPACE(sd, p, 0);
      break;
    case 8:
      if (p->flags & PF_ALTFORM)
	PUT_CHAR('0', p);
      break;
    case 16:
      if (p->flags & PF_ALTFORM)
	{
	  PUT_CHAR('0', p);
	  PUT_CHAR(*p->pf, p);
	}
      break;
    }

  while (*tmp)
    {
      PUT_CHAR(*tmp, p);
      tmp++;
    }

  PAD_LEFT(p);
  FREE (t);
}
#endif

static void
pointer(p, d)
     struct DATA *p;
     unsigned long d;
{
  char *tmp;

  tmp = fmtulong(d, 16, intbuf, sizeof(intbuf), 0);
  p->width -= strlen(tmp);
  PAD_RIGHT(p);

  /* prefix '0x' for pointers */
  PUT_CHAR('0', p);
  PUT_CHAR('x', p);

  while (*tmp)
    {
      PUT_CHAR(*tmp, p);
      tmp++;
    }

  PAD_LEFT(p);
}

/* %s strings */
static void
strings(p, tmp)
     struct DATA *p;
     char *tmp;
{
  size_t len;

  len = strlen(tmp);
  if (p->precision != NOT_FOUND) /* the smallest number */
    len = (len < p->precision ? len : p->precision);
  p->width -= len;

  PUT_STRING (tmp, len, p);
}

#if HANDLE_MULTIBYTE
/* %ls wide-character strings */
static void
wstrings(p, tmp)
     struct DATA *p;
     wchar_t *tmp;
{
  size_t len;
  mbstate_t mbs;
  char *os;
  const wchar_t *ws;

  memset (&mbs, '\0', sizeof (mbstate_t));
  ws = (const wchar_t *)tmp;

  os = (char *)NULL;
  if (p->precision != NOT_FOUND)
    {
      os = (char *)xmalloc (p->precision + 1);
      len = wcsrtombs (os, &ws, p->precision, &mbs);
    }
  else
    {
      len = wcsrtombs (NULL, &ws, 0, &mbs);
      if (len != (size_t)-1)
	{
	  memset (&mbs, '\0', sizeof (mbstate_t));
	  os = (char *)xmalloc (len + 1);
	  (void)wcsrtombs (os, &ws, len + 1, &mbs);
	}
    }
  if (len == (size_t)-1)
    {
      /* invalid multibyte sequence; bail now. */
      FREE (os);      
      return;
    }

  p->width -= len;
  PUT_STRING (os, len, p);
  free (os);
}

static void
wchars (p, wc)
     struct DATA *p;
     wint_t wc;
{
  char *lbuf, *l;
  mbstate_t mbs;
  size_t len;

  lbuf = (char *)malloc (MB_CUR_MAX+1);
  if (lbuf == 0)
    return;
  memset (&mbs, '\0', sizeof (mbstate_t));
  len = wcrtomb (lbuf, wc, &mbs);
  if (len == (size_t)-1)
    /* conversion failed; bail now. */
    return;
  p->width -= len;
  l = lbuf;
  PUT_STRING (l, len, p);
  free (lbuf);
}
#endif /* HANDLE_MULTIBYTE */

#ifdef FLOATING_POINT

/* Check for [+-]infinity and NaN.  If MODE == 1, we check for Infinity, else
   (mode == 2) we check for NaN.  This does the necessary printing.  Returns
   1 if Inf or Nan, 0 if not. */
static int
chkinfnan(p, d, mode)
     struct DATA *p;
     double d;
     int mode;		/* == 1 for inf, == 2 for nan */
{
  int i;
  char *tmp;
  char *big, *small;

  i = (mode == 1) ? isinf(d) : isnan(d);
  if (i == 0)
    return 0;
  big = (mode == 1) ? "INF" : "NAN";
  small = (mode == 1) ? "inf" : "nan";

  tmp = (*p->pf == 'F' || *p->pf == 'G' || *p->pf == 'E') ? big : small;

  if (i < 0)
    PUT_CHAR('-', p);

  while (*tmp)
    {
      PUT_CHAR (*tmp, p);
      tmp++;
    }

  return 1;
}

/* %f %F %g %G floating point representation */
static void
floating(p, d)
     struct DATA *p;
     double d;
{
  char *tmp, *tmp2, *t;
  int i;

  if (d != 0 && (chkinfnan(p, d, 1) || chkinfnan(p, d, 2)))
    return;	/* already printed nan or inf */

  GETLOCALEDATA(decpoint, thoussep, grouping);
  DEF_PREC(p);
  d = ROUND(d, p);
  tmp = dtoa(d, p->precision, &tmp2);
  t = 0;
  if ((p->flags & PF_THOUSANDS) && grouping && (t = groupnum (tmp)))
    tmp = t;

  if ((*p->pf == 'g' || *p->pf == 'G') && (p->flags & PF_ALTFORM) == 0)
    {
      /* smash the trailing zeros unless altform */
      for (i = strlen(tmp2) - 1; i >= 0 && tmp2[i] == '0'; i--)
	tmp2[i] = '\0'; 
      if (tmp2[0] == '\0')
	p->precision = 0;
    }

  /* calculate the padding. 1 for the dot */
  p->width = p->width -
  	    /* XXX - should this be d>0. && (p->flags & PF_PLUS) ? */
#if 0
	    ((d > 0. && p->justify == RIGHT) ? 1:0) -
#else
	    ((d > 0. && (p->flags & PF_PLUS)) ? 1:0) -
#endif
	    ((p->flags & PF_SPACE) ? 1:0) -
	    strlen(tmp) - p->precision -
	    ((p->precision != 0 || (p->flags & PF_ALTFORM)) ? 1 : 0);	/* radix char */

  if (p->pad == ' ')
    {
      PAD_RIGHT(p);
      PUT_PLUS(d, p, 0.);
    }
  else
    {
      if (*tmp == '-')
	PUT_CHAR(*tmp++, p);
      PUT_PLUS(d, p, 0.);
      PAD_RIGHT(p);
    }
  PUT_SPACE(d, p, 0.);

  while (*tmp)
    {
      PUT_CHAR(*tmp, p);	/* the integral */
      tmp++;
    }
  FREE (t);

  if (p->precision != 0 || (p->flags & PF_ALTFORM))
    PUT_CHAR(decpoint, p);  /* put the '.' */

  for (; *tmp2; tmp2++)
    PUT_CHAR(*tmp2, p); /* the fraction */
  
  PAD_LEFT(p);
} 

/* %e %E %g %G exponent representation */
static void
exponent(p, d)
     struct DATA *p;
     double d;
{
  char *tmp, *tmp2;
  int j, i;

  if (d != 0 && (chkinfnan(p, d, 1) || chkinfnan(p, d, 2)))
    return;	/* already printed nan or inf */

  GETLOCALEDATA(decpoint, thoussep, grouping);
  DEF_PREC(p);
  if (d == 0.)
    j = 0;
  else
    {
      j = log_10(d);
      d = d / pow_10(j);  /* get the Mantissa */
      d = ROUND(d, p);		  
    }
  tmp = dtoa(d, p->precision, &tmp2);

  /* 1 for unit, 1 for the '.', 1 for 'e|E',
   * 1 for '+|-', 2 for 'exp'  (but no `.' if precision == 0 */
  /* calculate how much padding need */
  p->width = p->width - 
  	    /* XXX - should this be d>0. && (p->flags & PF_PLUS) ? */
#if 0
	     ((d > 0. && p->justify == RIGHT) ? 1:0) -
#else
	     ((d > 0. && (p->flags & PF_PLUS)) ? 1:0) -
#endif
	     (p->precision != 0 || (p->flags & PF_ALTFORM)) -
	     ((p->flags & PF_SPACE) ? 1:0) - p->precision - 5;

  if (p->pad == ' ')
    {
      PAD_RIGHT(p);
      PUT_PLUS(d, p, 0.);
    }
  else
    {
      if (*tmp == '-')
	PUT_CHAR(*tmp++, p);
      PUT_PLUS(d, p, 0.);
      PAD_RIGHT(p);
    }
  PUT_SPACE(d, p, 0.);

  while (*tmp)
    {
      PUT_CHAR(*tmp, p);
      tmp++;
    }

  if (p->precision != 0 || (p->flags & PF_ALTFORM))
      PUT_CHAR(decpoint, p);  /* the '.' */

  if ((*p->pf == 'g' || *p->pf == 'G') && (p->flags & PF_ALTFORM) == 0)
    /* smash the trailing zeros unless altform */
    for (i = strlen(tmp2) - 1; i >= 0 && tmp2[i] == '0'; i--)
      tmp2[i] = '\0'; 

  for (; *tmp2; tmp2++)
    PUT_CHAR(*tmp2, p); /* the fraction */

  /* the exponent put the 'e|E' */
  if (*p->pf == 'g' || *p->pf == 'e')
    PUT_CHAR('e', p);
  else
    PUT_CHAR('E', p);

  /* the sign of the exp */
  if (j >= 0)
    PUT_CHAR('+', p);
  else
    {
      PUT_CHAR('-', p);
      j = -j;
    }

   tmp = itoa(j);
   /* pad out to at least two spaces.  pad with `0' if the exponent is a
      single digit. */
   if (j <= 9)
     PUT_CHAR('0', p);

   /* the exponent */
   while (*tmp)
     {
       PUT_CHAR(*tmp, p);
       tmp++;
     }

   PAD_LEFT(p);
}
#endif

/* Return a new string with the digits in S grouped according to the locale's
   grouping info and thousands separator.  If no grouping should be performed,
   this returns NULL; the caller needs to check for it. */
static char *
groupnum (s)
     char *s;
{
  char *se, *ret, *re, *g;
  int len, slen;

  if (grouping == 0 || *grouping <= 0 || *grouping == CHAR_MAX)
    return ((char *)NULL);

  /* find min grouping to size returned string */
  for (len = *grouping, g = grouping; *g; g++)
      if (*g > 0 && *g < len)
	len = *g;

  slen = strlen (s);
  len = slen / len + 1;
  ret = (char *)xmalloc (slen + len + 1);
  re = ret + slen + len;
  *re = '\0';

  g = grouping;
  se = s + slen;
  len = *g;

  while (se > s)
    {
      *--re = *--se;

      /* handle `-' inserted by numtoa() and the fmtu* family here. */
      if (se > s && se[-1] == '-')
	continue;

      /* begin new group. */
      if (--len == 0 && se > s)
	{
	  *--re = thoussep;
	  len = *++g;		/* was g++, but that uses first char twice (glibc bug, too) */
	  if (*g == '\0')
	    len = *--g;		/* use previous grouping */
	  else if (*g == CHAR_MAX)
	    {
	      do
		*--re = *--se;
	      while (se > s);
	      break;
	    }
	}
    }

  if (re > ret)
#ifdef HAVE_MEMMOVE
    memmove (ret, re, strlen (re) + 1);
#else
    strcpy (ret, re);
#endif
   
  return ret;
}

/* initialize the conversion specifiers */
static void
init_conv_flag (p)
     struct DATA *p;
{
  p->flags &= PF_ALLOCBUF;		/* preserve PF_ALLOCBUF flag */
  p->precision = p->width = NOT_FOUND;
  p->justify = NOT_FOUND;
  p->pad = ' ';
}

static void
init_data (p, string, length, format, mode)
     struct DATA *p;
     char *string;
     size_t length;
     const char *format;
     int mode;
{
  p->length = length - 1; /* leave room for '\0' */
  p->holder = p->base = string;
  p->pf = format;
  p->counter = 0;
  p->flags = (mode == PFM_AS) ? PF_ALLOCBUF : 0;
}

static int
#if defined (__STDC__)
vsnprintf_internal(struct DATA *data, char *string, size_t length, const char *format, va_list args)
#else
vsnprintf_internal(data, string, length, format, args)
     struct DATA *data;
     char *string;
     size_t length;
     const char *format;
     va_list args;
#endif
{
  double d; /* temporary holder */
#ifdef HAVE_LONG_DOUBLE
  long double ld;	/* for later */
#endif
  unsigned long ul;
#ifdef HAVE_UNSIGNED_LONG_LONG_INT
  unsigned long long ull;
#endif
  int state, i, c, n;
  char *s;
#if HANDLE_MULTIBYTE
  wchar_t *ws;
  wint_t wc;
#endif
  const char *convstart;
  int negprec;

  /* Sanity check, the string length must be >= 0.  C99 actually says that
     LENGTH can be zero here, in the case of snprintf/vsnprintf (it's never
     0 in the case of asprintf/vasprintf), and the return value is the number
     of characters that would have been written. */
  if (length < 0)
    return -1;

  if (format == 0)
    return 0;

  /* Reset these for each call because the locale might have changed. */
  decpoint = thoussep = 0;
  grouping = 0;

  negprec = 0;
  for (; c = *(data->pf); data->pf++)
    {
      if (c != '%')
	{
	  PUT_CHAR (c, data);
	  continue;
	}

      convstart = data->pf;
      init_conv_flag (data); /* initialise format flags */

      state = 1;
      for (state = 1; state && *data->pf; )
	{
	  c = *(++data->pf);
	      /* fmtend = data->pf */
#if defined (FLOATING_POINT) && defined (HAVE_LONG_DOUBLE)
	  if (data->flags & PF_LONGDBL)
	    {
	      switch (c)
		{
		case 'f': case 'F':
		case 'e': case 'E':
		case 'g': case 'G':
#  ifdef HAVE_PRINTF_A_FORMAT
		case 'a': case 'A':
#  endif
		  STAR_ARGS (data);
		  ld = GETLDOUBLE (data);
		  ldfallback (data, convstart, data->pf, ld);
		  goto conv_break;
		}
	    }
#endif /* FLOATING_POINT && HAVE_LONG_DOUBLE */

	  switch (c)
	    {
	      /* Parse format flags */
	      case '\0': /* a NULL here ? ? bail out */
		*data->holder = '\0';
		return data->counter;
		break;
	      case '#':
		data->flags |= PF_ALTFORM;
		continue;
	      case '*':
		if (data->flags & PF_DOT)
		  data->flags |= PF_STAR_P;
		else
		  data->flags |= PF_STAR_W;
		continue;
	      case '-':
		if ((data->flags & PF_DOT) == 0)
		  {
		    data->flags |= PF_LADJUST;
		    data->justify = LEFT;
		  }
		else
		  negprec = 1;
		continue;
	      case ' ':
		if ((data->flags & PF_PLUS) == 0)
		  data->flags |= PF_SPACE;
		continue;
	      case '+':
		if ((data->flags & PF_DOT) == 0)
		  {
		    data->flags |= PF_PLUS;
		    if ((data->flags & PF_LADJUST) == 0)
		      data->justify = RIGHT;
		  }
		continue;
	      case '\'':
		data->flags |= PF_THOUSANDS;
		continue;

	      case '0':
		/* If we're not specifying precision (in which case we've seen
		   a `.') and we're not performing left-adjustment (in which
		   case the `0' is ignored), a `0' is taken as the zero-padding
		   flag. */
	        if ((data->flags & (PF_DOT|PF_LADJUST)) == 0)
		  {
		    data->flags |= PF_ZEROPAD;
		    data->pad = '0';
		    continue;
		  }
	      case '1': case '2': case '3':
	      case '4': case '5': case '6':
	      case '7': case '8': case '9':
		n = 0;
		do
		  {
		    n = n * 10 + TODIGIT(c);
		    c = *(++data->pf);
		  }
		while (DIGIT(c));
		data->pf--;		/* went too far */
		if (n < 0)
		  n = 0;
		if (data->flags & PF_DOT)
		  data->precision = negprec ? NOT_FOUND : n;
		else
		  data->width = n;
		continue;

	      /* optional precision */
	      case '.':
		data->flags |= PF_DOT;
		data->precision = 0;
		continue;

	      /* length modifiers */
	      case 'h':
		data->flags |= (data->flags & PF_SHORTINT) ? PF_SIGNEDCHAR : PF_SHORTINT;
		continue;
	      case 'l':
		data->flags |= (data->flags & PF_LONGINT) ? PF_LONGLONG : PF_LONGINT;
		continue;
	      case 'L':
		data->flags |= PF_LONGDBL;
		continue;
	      case 'q':
		data->flags |= PF_LONGLONG;
		continue;
	      case 'j':
		data->flags |= PF_INTMAX_T;
		SET_SIZE_FLAGS(data, intmax_t);
		continue;
	      case 'z':
		data->flags |= PF_SIZE_T;
		SET_SIZE_FLAGS(data, size_t);
		continue;
	      case 't':
		data->flags |= PF_PTRDIFF_T;
		SET_SIZE_FLAGS(data, ptrdiff_t);
		continue;
		
	      /* Conversion specifiers */
#ifdef FLOATING_POINT
	      case 'f':  /* float, double */
	      case 'F':
		STAR_ARGS(data);
		d = GETDOUBLE(data);
		floating(data, d);
conv_break:		
		state = 0;
		break;
	      case 'g': 
	      case 'G':
		STAR_ARGS(data);
		DEF_PREC(data);
		d = GETDOUBLE(data);
		i = (d != 0.) ? log_10(d) : -1;
		/*
		 * for '%g|%G' ANSI: use f if exponent
		 * is in the range or [-4,p] exclusively
		 * else use %e|%E
		 */
		if (-4 < i && i < data->precision)
		  {
		    /* reset precision */
		    data->precision -= i + 1;
		    floating(data, d);
		  }
		else
		  {
		    /* reduce precision by 1 because of leading digit before
		       decimal point in e format, unless specified as 0. */
		    if (data->precision > 0)
		      data->precision--;
		    exponent(data, d);
		  }
		state = 0;
		break;
	      case 'e':
	      case 'E':  /* Exponent double */
		STAR_ARGS(data);
		d = GETDOUBLE(data);
		exponent(data, d);
		state = 0;
		break;
#  ifdef HAVE_PRINTF_A_FORMAT
	      case 'a':
	      case 'A':
		STAR_ARGS(data);
		d = GETDOUBLE(data);
		dfallback(data, convstart, data->pf, d);
		state = 0;
		break;
#  endif /* HAVE_PRINTF_A_FORMAT */
#endif /* FLOATING_POINT */
	      case 'U':
		data->flags |= PF_LONGINT;
		/* FALLTHROUGH */
	      case 'u':
		STAR_ARGS(data);
#ifdef HAVE_LONG_LONG_INT
		if (data->flags & PF_LONGLONG)
		  {
		    ull = GETARG (unsigned long long);
		    lnumber(data, ull, 10);
		  }
		else
#endif
		  {
		    ul = GETUNSIGNED(data);
		    number(data, ul, 10);
		  }
		state = 0;
		break;
	      case 'D':
		data->flags |= PF_LONGINT;
		/* FALLTHROUGH */
	      case 'd':  /* decimal */
	      case 'i':
		STAR_ARGS(data);
#ifdef HAVE_LONG_LONG_INT
		if (data->flags & PF_LONGLONG)
		  {
		    ull = GETARG (long long);
		    lnumber(data, ull, 10);
		  }
		else
#endif
		  {
		    ul = GETSIGNED(data);
		    number(data, ul, 10);
		  }
		state = 0;
		break;
	      case 'o':  /* octal */
		STAR_ARGS(data);
#ifdef HAVE_LONG_LONG_INT
		if (data->flags & PF_LONGLONG)
		  {
		    ull = GETARG (unsigned long long);
		    lnumber(data, ull, 8);
		  }
		else
#endif
		  {
		    ul = GETUNSIGNED(data);
		    number(data, ul, 8);
		  }
		state = 0;
		break;
	      case 'x': 
	      case 'X':  /* hexadecimal */
		STAR_ARGS(data);
#ifdef HAVE_LONG_LONG_INT
		if (data->flags & PF_LONGLONG)
		  {
		    ull = GETARG (unsigned long long);
		    lnumber(data, ull, 16);
		  }
		else
#endif
		  {
		    ul = GETUNSIGNED(data);
		    number(data, ul, 16);
		  }
		state = 0;
		break;
	      case 'p':
		STAR_ARGS(data);
		ul = (unsigned long)GETARG (void *);
		pointer(data, ul);
		state = 0;
		break;
#if HANDLE_MULTIBYTE
	      case 'C':
		data->flags |= PF_LONGINT;
		/* FALLTHROUGH */
#endif
	      case 'c': /* character */
		STAR_ARGS(data);
#if HANDLE_MULTIBYTE
		if (data->flags & PF_LONGINT)
		  {
		    wc = GETARG (wint_t);
		    wchars (data, wc);
		  }
		else
#endif
		  {		
		    ul = GETARG (int);
		    PUT_CHAR(ul, data);
		  }
		state = 0;
		break;
#if HANDLE_MULTIBYTE
	      case 'S':
		data->flags |= PF_LONGINT;
		/* FALLTHROUGH */
#endif
	      case 's':  /* string */
		STAR_ARGS(data);
#if HANDLE_MULTIBYTE
		if (data->flags & PF_LONGINT)
		  {
		    ws = GETARG (wchar_t *);
		    wstrings (data, ws);
		  }
		else
#endif
		  {
		    s = GETARG (char *);
		    strings(data, s);
		  }
		state = 0;
		break;
	      case 'n':
#ifdef HAVE_LONG_LONG_INT
		if (data->flags & PF_LONGLONG)
		  *(GETARG (long long *)) = data->counter;
		else
#endif
		if (data->flags & PF_LONGINT)
		  *(GETARG (long *)) = data->counter;
		else if (data->flags & PF_SHORTINT)
		  *(GETARG (short *)) = data->counter;
		else
		  *(GETARG (int *)) = data->counter;
		state = 0;
		break;
	      case '%':  /* nothing just % */
		PUT_CHAR('%', data);
		state = 0;
		break;
  	      default:
		/* is this an error ? maybe bail out */
		state = 0;
		break;
	} /* end switch */
      } /* end of `%' for loop */
    } /* end of format string for loop */

  if (data->length >= 0)
    *data->holder = '\0'; /* the end ye ! */

  return data->counter;
}

#if defined (FLOATING_POINT) && defined (HAVE_LONG_DOUBLE)
/*
 * Printing floating point numbers accurately is an art.  I'm not good
 * at it.  Fall back to sprintf for long double formats.
 */
static void
ldfallback (data, fs, fe, ld)
     struct DATA *data;
     const char *fs, *fe;
     long double ld;
{
  register char *x;
  char fmtbuf[FALLBACK_FMTSIZE], *obuf;
  int fl;

  fl = LFALLBACK_BASE + (data->precision < 6 ? 6 : data->precision) + 2;
  obuf = (char *)xmalloc (fl);
  fl = fe - fs + 1;
  strncpy (fmtbuf, fs, fl);
  fmtbuf[fl] = '\0';

  if ((data->flags & PF_STAR_W) && (data->flags & PF_STAR_P))
    sprintf (obuf, fmtbuf, data->width, data->precision, ld);
  else if (data->flags & PF_STAR_W)
    sprintf (obuf, fmtbuf, data->width, ld);
  else if (data->flags & PF_STAR_P)
    sprintf (obuf, fmtbuf, data->precision, ld);
  else
    sprintf (obuf, fmtbuf, ld);

  for (x = obuf; *x; x++)
    PUT_CHAR (*x, data);    
  xfree (obuf);
}
#endif /* FLOATING_POINT && HAVE_LONG_DOUBLE */

#ifdef FLOATING_POINT
/* Used for %a, %A if the libc printf supports them. */
static void
dfallback (data, fs, fe, d)
     struct DATA *data;
     const char *fs, *fe;
     double d;
{
  register char *x;
  char fmtbuf[FALLBACK_FMTSIZE], obuf[FALLBACK_BASE];
  int fl;

  fl = fe - fs + 1;
  strncpy (fmtbuf, fs, fl);
  fmtbuf[fl] = '\0';

  if ((data->flags & PF_STAR_W) && (data->flags & PF_STAR_P))
    sprintf (obuf, fmtbuf, data->width, data->precision, d);
  else if (data->flags & PF_STAR_W)
    sprintf (obuf, fmtbuf, data->width, d);
  else if (data->flags & PF_STAR_P)
    sprintf (obuf, fmtbuf, data->precision, d);
  else
    sprintf (obuf, fmtbuf, d);

  for (x = obuf; *x; x++)
    PUT_CHAR (*x, data);    
}
#endif /* FLOATING_POINT */

#if !HAVE_SNPRINTF

int
#if defined (__STDC__)
vsnprintf(char *string, size_t length, const char *format, va_list args)
#else
vsnprintf(string, length, format, args)
     char *string;
     size_t length;
     const char *format;
     va_list args;
#endif
{
  struct DATA data;

  if (string == 0 && length != 0)
    return 0;
  init_data (&data, string, length, format, PFM_SN);
  return (vsnprintf_internal(&data, string, length, format, args));
}

int
#if defined(PREFER_STDARG)
snprintf(char *string, size_t length, const char * format, ...)
#else
snprintf(string, length, format, va_alist)
     char *string;
     size_t length;
     const char *format;
     va_dcl
#endif
{
  struct DATA data;
  int rval;
  va_list args;

  SH_VA_START(args, format);

  if (string == 0 && length != 0)
    return 0;
  init_data (&data, string, length, format, PFM_SN);
  rval = vsnprintf_internal (&data, string, length, format, args);

  va_end(args);

  return rval;
}

#endif /* HAVE_SNPRINTF */

#if !HAVE_ASPRINTF

int
#if defined (__STDC__)
vasprintf(char **stringp, const char *format, va_list args)
#else
vasprintf(stringp, format, args)
     char **stringp;
     const char *format;
     va_list args;
#endif
{
  struct DATA data;
  char *string;
  int r;

  string = (char *)xmalloc(ASBUFSIZE);
  init_data (&data, string, ASBUFSIZE, format, PFM_AS);
  r = vsnprintf_internal(&data, string, ASBUFSIZE, format, args);
  *stringp = data.base;		/* not string in case reallocated */
  return r;
}

int
#if defined(PREFER_STDARG)
asprintf(char **stringp, const char * format, ...)
#else
asprintf(stringp, format, va_alist)
     char **stringp;
     const char *format;
     va_dcl
#endif
{
  int rval;
  va_list args;

  SH_VA_START(args, format);

  rval = vasprintf (stringp, format, args);

  va_end(args);

  return rval;
}

#endif /* !HAVE_ASPRINTF */

#endif /* !HAVE_SNPRINTF || !HAVE_ASPRINTF */

#ifdef DRIVER

static void
memory_error_and_abort ()
{
  write (2, "out of virtual memory\n", 22);
  abort ();
}

static void *
xmalloc(bytes)
     size_t bytes;
{
  void *ret;

  ret = malloc(bytes);
  if (ret == 0)
    memory_error_and_abort ();
  return ret;
}

static void *
xrealloc (pointer, bytes)
     void *pointer;
     size_t bytes;
{
  void *ret;

  ret = pointer ? realloc(pointer, bytes) : malloc(bytes);
  if (ret == 0)
    memory_error_and_abort ();
  return ret;
}

static void
xfree(x)
     void *x;
{
  if (x)
    free (x);
}

/* set of small tests for snprintf() */
main()
{
  char holder[100];
  char *h;
  int i, si, ai;

#ifdef HAVE_LOCALE_H
  setlocale(LC_ALL, "");
#endif

#if 1
  si = snprintf((char *)NULL, 0, "abcde\n");
  printf("snprintf returns %d with NULL first argument and size of 0\n", si);
  si = snprintf(holder, 0, "abcde\n");
  printf("snprintf returns %d with non-NULL first argument and size of 0\n", si);
  si = snprintf((char *)NULL, 16, "abcde\n");
  printf("snprintf returns %d with NULL first argument and non-zero size\n", si);
  
/*
  printf("Suite of test for snprintf:\n");
  printf("a_format\n");
  printf("printf() format\n");
  printf("snprintf() format\n\n");
*/
/* Checking the field widths */

  printf("/%%ld %%ld/, 336, 336\n");
  snprintf(holder, sizeof holder, "/%ld %ld/\n", 336, 336);
  asprintf(&h, "/%ld %ld/\n", 336, 336);
  printf("/%ld %ld/\n", 336, 336);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%d/, 336\n");
  snprintf(holder, sizeof holder, "/%d/\n", 336);
  asprintf(&h, "/%d/\n", 336);
  printf("/%d/\n", 336);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%2d/, 336\n");
  snprintf(holder, sizeof holder, "/%2d/\n", 336);
  asprintf(&h, "/%2d/\n", 336);
  printf("/%2d/\n", 336);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%10d/, 336\n");
  snprintf(holder, sizeof holder, "/%10d/\n", 336);
  asprintf(&h, "/%10d/\n", 336);
  printf("/%10d/\n", 336);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%-10d/, 336\n");
  snprintf(holder, sizeof holder, "/%-10d/\n", 336);
  asprintf(&h, "/%-10d/\n", 336);
  printf("/%-10d/\n", 336);
  printf("%s", holder);
  printf("%s\n", h);


/* floating points */

  printf("/%%f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%f/\n", 1234.56);
  asprintf(&h, "/%f/\n", 1234.56);
  printf("/%f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%e/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%e/\n", 1234.56);
  asprintf(&h, "/%e/\n", 1234.56);
  printf("/%e/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%4.2f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%4.2f/\n", 1234.56);
  asprintf(&h, "/%4.2f/\n", 1234.56);
  printf("/%4.2f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%3.1f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%3.1f/\n", 1234.56);
  asprintf(&h, "/%3.1f/\n", 1234.56);
  printf("/%3.1f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%10.3f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%10.3f/\n", 1234.56);
  asprintf(&h, "/%10.3f/\n", 1234.56);
  printf("/%10.3f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%10.3e/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%10.3e/\n", 1234.56);
  asprintf(&h, "/%10.3e/\n", 1234.56);
  printf("/%10.3e/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%+4.2f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%+4.2f/\n", 1234.56);
  asprintf(&h, "/%+4.2f/\n", 1234.56);
  printf("/%+4.2f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%010.2f/, 1234.56\n");
  snprintf(holder, sizeof holder, "/%010.2f/\n", 1234.56);
  asprintf(&h, "/%010.2f/\n", 1234.56);
  printf("/%010.2f/\n", 1234.56);
  printf("%s", holder);
  printf("%s\n", h);

#define BLURB "Outstanding acting !"
/* strings precisions */

  printf("/%%2s/, \"%s\"\n", BLURB);
  snprintf(holder, sizeof holder, "/%2s/\n", BLURB);
  asprintf(&h, "/%2s/\n", BLURB);
  printf("/%2s/\n", BLURB);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%22s/ %s\n", BLURB);
  snprintf(holder, sizeof holder, "/%22s/\n", BLURB);
  asprintf(&h, "/%22s/\n", BLURB);
  printf("/%22s/\n", BLURB);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%22.5s/ %s\n", BLURB);
  snprintf(holder, sizeof holder, "/%22.5s/\n", BLURB);
  asprintf(&h, "/%22.5s/\n", BLURB);
  printf("/%22.5s/\n", BLURB);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%-22.5s/ %s\n", BLURB);
  snprintf(holder, sizeof holder, "/%-22.5s/\n", BLURB);
  asprintf(&h, "/%-22.5s/\n", BLURB);
  printf("/%-22.5s/\n", BLURB);
  printf("%s", holder);
  printf("%s\n", h);

/* see some flags */

  printf("%%x %%X %%#x, 31, 31, 31\n");
  snprintf(holder, sizeof holder, "%x %X %#x\n", 31, 31, 31);
  asprintf(&h, "%x %X %#x\n", 31, 31, 31);
  printf("%x %X %#x\n", 31, 31, 31);
  printf("%s", holder);
  printf("%s\n", h);

  printf("**%%d**%% d**%% d**, 42, 42, -42\n");
  snprintf(holder, sizeof holder, "**%d**% d**% d**\n", 42, 42, -42);
  asprintf(&h, "**%d**% d**% d**\n", 42, 42, -42);
  printf("**%d**% d**% d**\n", 42, 42, -42);
  printf("%s", holder);
  printf("%s\n", h);

/* other flags */

  printf("/%%g/, 31.4\n");
  snprintf(holder, sizeof holder, "/%g/\n", 31.4);
  asprintf(&h, "/%g/\n", 31.4);
  printf("/%g/\n", 31.4);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%.6g/, 31.4\n");
  snprintf(holder, sizeof holder, "/%.6g/\n", 31.4);
  asprintf(&h, "/%.6g/\n", 31.4);
  printf("/%.6g/\n", 31.4);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%.1G/, 31.4\n");
  snprintf(holder, sizeof holder, "/%.1G/\n", 31.4);
  asprintf(&h, "/%.1G/\n", 31.4);
  printf("/%.1G/\n", 31.4);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%.1G/, 3100000000.4\n");
  snprintf(holder, sizeof holder, "/%.1G/\n", 3100000000.4);  
  asprintf(&h, "/%.1G/\n", 3100000000.4);  
  printf("/%.1G/\n", 3100000000.4); 
  printf("%s", holder);
  printf("%s\n", h);

  printf("abc%%n\n");
  printf("abc%n", &i); printf("%d\n", i);
  snprintf(holder, sizeof holder, "abc%n", &i);
  printf("%s", holder); printf("%d\n\n", i);
  asprintf(&h, "abc%n", &i);
  printf("%s", h); printf("%d\n\n", i);
  
  printf("%%*.*s --> 10.10\n");
  snprintf(holder, sizeof holder, "%*.*s\n", 10, 10, BLURB);
  asprintf(&h, "%*.*s\n", 10, 10, BLURB);
  printf("%*.*s\n", 10, 10, BLURB);
  printf("%s", holder);
  printf("%s\n", h);

  printf("%%%%%%%%\n");
  snprintf(holder, sizeof holder, "%%%%\n");
  asprintf(&h, "%%%%\n");
  printf("%%%%\n");
  printf("%s", holder);
  printf("%s\n", h);

#define BIG "Hello this is a too big string for the buffer"
/*  printf("A buffer to small of 10, trying to put this:\n");*/
  printf("<%%>, %s\n", BIG); 
  i = snprintf(holder, 10, "%s\n", BIG);
  i = asprintf(&h, "%s", BIG);
  printf("<%s>\n", BIG);
  printf("<%s>\n", holder);
  printf("<%s>\n\n", h);

  printf ("<%%p> vsnprintf\n");
  i = snprintf(holder, 100, "%p", vsnprintf);
  i = asprintf(&h, "%p", vsnprintf);
  printf("<%p>\n", vsnprintf);
  printf("<%s>\n", holder);  
  printf("<%s>\n\n", h);

  printf ("<%%lu> LONG_MAX+1\n");
  i = snprintf(holder, 100, "%lu", (unsigned long)(LONG_MAX)+1);
  i = asprintf(&h, "%lu", (unsigned long)(LONG_MAX)+1);
  printf("<%lu>\n", (unsigned long)(LONG_MAX)+1);
  printf("<%s>\n", holder);
  printf("<%s>\n\n", h);

#ifdef HAVE_LONG_LONG_INT
  printf ("<%%llu> LLONG_MAX+1\n");
  i = snprintf(holder, 100, "%llu", (unsigned long long)(LLONG_MAX)+1);
  i = asprintf(&h, "%llu", (unsigned long long)(LLONG_MAX)+1);
  printf("<%llu>\n", (unsigned long long)(LLONG_MAX)+1);
  printf("<%s>\n", holder);
  printf("<%s>\n\n", h);
#endif

#ifdef HAVE_LONG_DOUBLE
  printf ("<%%6.2LE> 42.42\n");
  i = snprintf(holder, 100, "%6.2LE", (long double)42.42);
  i = asprintf(&h, "%6.2LE", (long double)42.42);
  printf ("<%6.2LE>\n", (long double)42.42);
  printf ("<%s>\n", holder);
  printf ("<%s>\n\n", h);
#endif

#ifdef HAVE_PRINTF_A_FORMAT
  printf ("<%%6.2A> 42.42\n");
  i = snprintf(holder, 100, "%6.2A", 42.42);
  i = asprintf(&h, "%6.2A", 42.42);
  printf ("<%6.2A>\n", 42.42);
  printf ("<%s>\n", holder);
  printf ("<%s>\n\n", h);

  printf ("<%%6.2LA> 42.42\n");
  i = snprintf(holder, 100, "%6.2LA", (long double)42.42);
  i = asprintf(&h, "%6.2LA", (long double)42.42);
  printf ("<%6.2LA>\n", (long double)42.42);
  printf ("<%s>\n", holder);
  printf ("<%s>\n\n", h);
#endif

  printf ("<%%.10240f> DBL_MAX\n");
  si = snprintf(holder, 100, "%.10240f", DBL_MAX);
  ai = asprintf(&h, "%.10240f", DBL_MAX);
  printf ("<%.10240f>\n", DBL_MAX);
  printf ("<%d> <%s>\n", si, holder);
  printf ("<%d> <%s>\n\n", ai, h);

  printf ("<%%.10240Lf> LDBL_MAX\n");
  si = snprintf(holder, 100, "%.10240Lf", (long double)LDBL_MAX);
  ai = asprintf(&h, "%.10240Lf", (long double)LDBL_MAX);
  printf ("<%.10240Lf>\n", (long double)LDBL_MAX);
  printf ("<%d> <%s>\n", si, holder);
  printf ("<%d> <%s>\n\n", ai, h);

  /* huh? */
  printf("/%%g/, 421.2345\n");
  snprintf(holder, sizeof holder, "/%g/\n", 421.2345);
  asprintf(&h, "/%g/\n", 421.2345);
  printf("/%g/\n", 421.2345);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%g/, 4214.2345\n");
  snprintf(holder, sizeof holder, "/%g/\n", 4214.2345);
  asprintf(&h, "/%g/\n", 4214.2345);
  printf("/%g/\n", 4214.2345);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%.5g/, 4214.2345\n");
  snprintf(holder, sizeof holder, "/%.5g/\n", 4214.2345);
  asprintf(&h, "/%.5g/\n", 4214.2345);
  printf("/%.5g/\n", 4214.2345);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%.4g/, 4214.2345\n");
  snprintf(holder, sizeof holder, "/%.4g/\n", 4214.2345);
  asprintf(&h, "/%.4g/\n", 4214.2345);
  printf("/%.4g/\n", 4214.2345);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'ld %%'ld/, 12345, 1234567\n");
  snprintf(holder, sizeof holder, "/%'ld %'ld/\n", 12345, 1234567);
  asprintf(&h, "/%'ld %'ld/\n", 12345, 1234567);
  printf("/%'ld %'ld/\n", 12345, 1234567);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'ld %%'ld/, 336, 3336\n");
  snprintf(holder, sizeof holder, "/%'ld %'ld/\n", 336, 3336);
  asprintf(&h, "/%'ld %'ld/\n", 336, 3336);
  printf("/%'ld %'ld/\n", 336, 3336);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'ld %%'ld/, -42786, -142786\n");
  snprintf(holder, sizeof holder, "/%'ld %'ld/\n", -42786, -142786);
  asprintf(&h, "/%'ld %'ld/\n", -42786, -142786);
  printf("/%'ld %'ld/\n", -42786, -142786);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'f %%'f/, 421.2345, 421234.56789\n");
  snprintf(holder, sizeof holder, "/%'f %'f/\n", 421.2345, 421234.56789);
  asprintf(&h, "/%'f %'f/\n", 421.2345, 421234.56789);
  printf("/%'f %'f/\n", 421.2345, 421234.56789);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'f %%'f/, -421.2345, -421234.56789\n");
  snprintf(holder, sizeof holder, "/%'f %'f/\n", -421.2345, -421234.56789);
  asprintf(&h, "/%'f %'f/\n", -421.2345, -421234.56789);
  printf("/%'f %'f/\n", -421.2345, -421234.56789);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'g %%'g/, 421.2345, 421234.56789\n");
  snprintf(holder, sizeof holder, "/%'g %'g/\n", 421.2345, 421234.56789);
  asprintf(&h, "/%'g %'g/\n", 421.2345, 421234.56789);
  printf("/%'g %'g/\n", 421.2345, 421234.56789);
  printf("%s", holder);
  printf("%s\n", h);

  printf("/%%'g %%'g/, -421.2345, -421234.56789\n");
  snprintf(holder, sizeof holder, "/%'g %'g/\n", -421.2345, -421234.56789);
  asprintf(&h, "/%'g %'g/\n", -421.2345, -421234.56789);
  printf("/%'g %'g/\n", -421.2345, -421234.56789);
  printf("%s", holder);
  printf("%s\n", h);
#endif

  printf("/%%'g/, 4213455.8392\n");
  snprintf(holder, sizeof holder, "/%'g/\n", 4213455.8392);
  asprintf(&h, "/%'g/\n", 4213455.8392);
  printf("/%'g/\n", 4213455.8392);
  printf("%s", holder);
  printf("%s\n", h);

  exit (0);
}
#endif
