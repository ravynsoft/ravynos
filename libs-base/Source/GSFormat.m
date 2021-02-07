/** Implementation of GNUSTEP printf-style formatting
   Copyright (C) 1994-2000, 2001-2013 Free Software Foundation, Inc.

   Hacked together by Kai Henningsen <kai@cats.ms>
   from the glibc 2.2.1 sources
	_i18n_number.h
	_itowa.h
	itowa-digits.c
	outdigits.h
	outdigitswc.h
	printf-parse.h
	printf.h
	vfprintf.c
   which were contributed by Ulrich Drepper <drepper@gnu.org>, 2000,
   Date: January 2001
   FIXME: I wasn't brave enough to include floating point formatting -
   glibc has CPU dependent routines and also uses gmp. So floating point
   formats (AaFfGgAa) simply use sprintf.
   FIXME: This needs to use length, not '\0', when dealing with format
   and %@
   No register_printf_functions in this thing.

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#include "common.h"

#if	!defined(LLONG_MAX)
#  if	defined(__LONG_LONG_MAX__)
#    define LLONG_MAX __LONG_LONG_MAX__
#    define LLONG_MIN	(-LLONG_MAX-1)
#    define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#  else
#    error Neither LLONG_MAX nor __LONG_LONG_MAX__ found
#  endif
#endif

#ifdef HAVE_MALLOC_H
#if !defined(__OpenBSD__)
#include <malloc.h>
#endif
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <stdio.h>

#import "Foundation/NSArray.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSException.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSPortCoder.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSRange.h"
#import "Foundation/NSException.h"
#import "Foundation/NSData.h"
#import "Foundation/NSBundle.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSLock.h"
#import "GNUstepBase/GSLocale.h"

#import "GSPrivate.h"

#include <string.h>		// for strstr()
#include <sys/stat.h>
#include <sys/types.h>

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#include <stdio.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#else
typedef uint32_t wint_t;
#endif
#ifdef HAVE_SYS_INTTYPES_H
#include <sys/inttypes.h>
#endif
#ifndef HAVE_UINTMAX_T
typedef unsigned long long uintmax_t;
#endif


#import "GNUstepBase/Unicode.h"

struct printf_info
{
  int prec;			/* Precision.  */
  int width;			/* Width.  */
  unichar spec;			/* Format letter.  */
  unsigned int is_long_double:1;/* L flag.  */
  unsigned int is_short:1;	/* h flag.  */
  unsigned int is_long:1;	/* l flag.  */
  unsigned int alt:1;		/* # flag.  */
  unsigned int space:1;		/* Space flag.  */
  unsigned int left:1;		/* - flag.  */
  unsigned int showsign:1;	/* + flag.  */
  unsigned int group:1;		/* ' flag.  */
  unsigned int extra:1;		/* For special use.  */
  unsigned int is_char:1;	/* hh flag.  */
  unsigned int wide:1;		/* Nonzero for wide character streams.  */
  unsigned int i18n:1;		/* I flag.  */
  unichar pad;			/* Padding character.  */
};

/* Type of a printf specifier-handler function.
   STREAM is the GSStr on which to write output.
   INFO gives information about the format specification.
   ARGS is a vector of pointers to the argument data;
   the number of pointers will be the number returned
   by the associated arginfo function for the same INFO.

   The function should return the number of characters written,
   or -1 for errors.  */


/* Type of a printf specifier-arginfo function.
   INFO gives information about the format specification.
   N, ARGTYPES, and return value are as for printf_parse_format.  */



/* Register FUNC to be called to format SPEC specifiers; ARGINFO must be
   specified to determine how many arguments a SPEC conversion requires and
   what their types are.  */



/* Parse FMT, and fill in N elements of ARGTYPES with the
   types needed for the conversions FMT specifies.  Returns
   the number of arguments required by FMT.

   The ARGINFO function registered with a user-defined format is passed a
   `struct printf_info' describing the format spec being parsed.  A width
   or precision of INT_MIN means a `*' was used to indicate that the
   width/precision will come from an arg.  The function should fill in the
   array it is passed with the types of the arguments it wants, and return
   the number of arguments it wants.  */



/* Codes returned by `parse_printf_format' for basic types.

   These values cover all the standard format specifications.
   Users can add new values after PA_LAST for their own types.  */

enum
{				/* C type: */
  PA_INT,			/* int */
  PA_CHAR,			/* int, cast to char */
  PA_WCHAR,			/* wide char */
  PA_STRING,			/* const char *, a '\0'-terminated string */
  PA_WSTRING,			/* const wchar_t *, wide character string */
  PA_POINTER,			/* void * */
  PA_FLOAT,			/* float */
  PA_DOUBLE,			/* double */
  PA_OBJECT,			/* id */
  PA_LAST
};

/* Flag bits that can be set in a type returned by `parse_printf_format'.  */
#define	PA_FLAG_MASK		0xff00
#define	PA_FLAG_LONG_LONG	(1 << 8)
#define	PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG
#define	PA_FLAG_LONG		(1 << 9)
#define	PA_FLAG_SHORT		(1 << 10)
#define	PA_FLAG_PTR		(1 << 11)



/* Function which can be registered as `printf'-handlers.  */

/* Print floating point value using using abbreviations for the orders
   of magnitude used for numbers ('k' for kilo, 'm' for mega etc).  If
   the format specifier is a uppercase character powers of 1000 are
   used.  Otherwise powers of 1024.  */

/* This is the appropriate argument information function for `printf_size'.  */

/* Digits.  */

/* Lower-case digits.  */
static const char _itowa_lower_digits[36]
	= "0123456789abcdefghijklmnopqrstuvwxyz";
/* Upper-case digits.  */
static const char _itowa_upper_digits[36]
	= "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


/* This code is shared between the standard stdio implementation found
   in GNU C library and the libio implementation originally found in
   GNU libg++.

   Beside this it is also shared between the normal and wide character
   implementation as defined in ISO/IEC 9899:1990/Amendment 1:1995.  */


#  define ISDIGIT(Ch)	((unsigned int) ((Ch) - '0') < 10)

/* Internal function for converting integers to ASCII.  */


/* Convert VALUE into ASCII in base BASE (2..36).
   Write backwards starting the character just before BUFLIM.
   Return the address of the first (left-to-right) character in the number.
   Use upper case letters iff UPPER_CASE is nonzero.  */

static unichar *_itowa (unsigned long long int value, unichar *buflim,
			unsigned int base, int upper_case)
{
  const char *digits = (upper_case
			   ? _itowa_upper_digits : _itowa_lower_digits);
  unichar *bp = buflim;

  switch (base)
    {
#define SPECIAL(Base)							      \
    case Base:								      \
      do								      \
	*--bp = digits[value % Base];					      \
      while ((value /= Base) != 0);					      \
      break

      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
	*--bp = digits[value % base];
      while ((value /= base) != 0);
      break;
    }
  return bp;
}

static inline unichar *
_itowa_word (unsigned long value, unichar *buflim,
	     unsigned int base, int upper_case)
{
  const char *digits = (upper_case
			   ? _itowa_upper_digits : _itowa_lower_digits);
  unichar *bp = buflim;

  switch (base)
    {
#define SPECIAL(Base)							      \
    case Base:								      \
      do								      \
	*--bp = digits[value % Base];					      \
      while ((value /= Base) != 0);					      \
      break

      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
	*--bp = digits[value % base];
      while ((value /= base) != 0);
      break;
    }
  return bp;
}


#define PAD(Padchar) \
  { \
    int w = width; \
    while (w-- > 0) outchar(Padchar); \
  }


/* Look up the value of the next multibyte character and return its numerical
   value if it is one of the digits known in the locale.  If *DECIDED is
   -1 this means it is not yet decided which form it is and we have to
   search through all available digits.  Otherwise we know which script
   the digits are from.  */

static unichar *
_i18n_number_rewrite (unichar *w, unichar *rear_ptr, NSString *locale_digits)
{
  unichar	*src;
  unichar	*s;
  unichar	buf[10];
  unichar	*ld = 0;

  /* Copy existing string so that nothing gets overwritten.  */
  src = (unichar *) alloca ((rear_ptr - w) * sizeof (unichar));
  s = (unichar *) memcpy (src, w, (rear_ptr - w) * sizeof (unichar));
  w = rear_ptr;

  /* Process all characters in the string.  */
  while (--s >= src)
    {
      if (*s >= '0' && *s <= '9')
	{
	  if (ld == 0)
	    {
	      if (!locale_digits || [locale_digits length] != 10)
		{
		  locale_digits = @"0123456789";
		}
	      [locale_digits getCharacters: buf];
	      ld = buf;
	    }
	  *--w = ld[*s - '0'];
	}
      else
	{
	  *--w = *s;
	}
    }

  return w;
}

/* Include the shared code for parsing the format string.  */
/* Internal header for parsing printf format strings.  */




struct printf_spec
  {
    /* Information parsed from the format spec.  */
    struct printf_info info;

    /* Pointers into the format string for the end of this format
       spec and the next (or to the end of the string if no more).  */
    const unichar *end_of_fmt, *next_fmt;

    /* Position of arguments for precision and width, or -1 if `info' has
       the constant value.  */
    int prec_arg, width_arg;

    int data_arg;		/* Position of data argument.  */
    int data_arg_type;		/* Type of first argument.  */
    /* Number of arguments consumed by this format specifier.  */
    size_t ndata_args;
  };


/* The various kinds off arguments that can be passed to printf.  */
union printf_arg
  {
    unsigned char pa_char;
    wchar_t pa_wchar;
    short int pa_short_int;
    int pa_int;
    long int pa_long_int;
    long long int pa_long_long_int;
    unsigned short int pa_u_short_int;
    unsigned int pa_u_int;
    unsigned long int pa_u_long_int;
    unsigned long long int pa_u_long_long_int;
    float pa_float;
    double pa_double;
    long double pa_long_double;
    const char *pa_string;
    const wchar_t *pa_wstring;
    id pa_object;
    void *pa_pointer;
  };


/* Read a simple integer from a string and update the string pointer.
   It is assumed that the first character is a digit.  */
static inline unsigned int
read_int (const unichar * *pstr)
{
  unsigned int retval = **pstr - '0';

  while (ISDIGIT (*++(*pstr)))
    {
      retval *= 10;
      retval += **pstr - '0';
    }

  return retval;
}



/* Find the next spec in FORMAT, or the end of the string.  Returns
   a pointer into FORMAT, to a '%' or a '\0'.  */
static inline const unichar *
find_spec (const unichar *format)
{
	while (*format && *format != '%') format++;
  return format;
}


/* These are defined in reg-printf.c.  */


/* FORMAT must point to a '%' at the beginning of a spec.  Fills in *SPEC
   with the parsed details.  POSN is the number of arguments already
   consumed.  At most MAXTYPES - POSN types are filled in TYPES.  Return
   the number of args consumed by this spec; *MAX_REF_ARG is updated so it
   remains the highest argument index used.  */
static inline size_t
parse_one_spec (const unichar *format, size_t posn, struct printf_spec *spec,
		size_t *max_ref_arg)
{
  unsigned int n;
  size_t nargs = 0;

  /* Skip the '%'.  */
  ++format;

  /* Clear information structure.  */
  spec->data_arg = -1;
  spec->info.alt = 0;
  spec->info.space = 0;
  spec->info.left = 0;
  spec->info.showsign = 0;
  spec->info.group = 0;
  spec->info.i18n = 0;
  spec->info.pad = ' ';
  spec->info.wide = sizeof (unichar) > 1;

  /* Test for positional argument.  */
  if (ISDIGIT (*format))
    {
      const unichar *begin = format;

      n = read_int (&format);

      if (n > 0 && *format == '$')
	/* Is positional parameter.  */
	{
	  ++format;		/* Skip the '$'.  */
	  spec->data_arg = n - 1;
	  *max_ref_arg = MAX (*max_ref_arg, n);
	}
      else
	/* Oops; that was actually the width and/or 0 padding flag.
	   Step back and read it again.  */
	format = begin;
    }

  /* Check for spec modifiers.  */
  do
    {
      switch (*format)
	{
	case ' ':
	  /* Output a space in place of a sign, when there is no sign.  */
	  spec->info.space = 1;
	  continue;
	case '+':
	  /* Always output + or - for numbers.  */
	  spec->info.showsign = 1;
	  continue;
	case '-':
	  /* Left-justify things.  */
	  spec->info.left = 1;
	  continue;
	case '#':
	  /* Use the "alternate form":
	     Hex has 0x or 0X, FP always has a decimal point.  */
	  spec->info.alt = 1;
	  continue;
	case '0':
	  /* Pad with 0s.  */
	  spec->info.pad = '0';
	  continue;
	case '\'':
	  /* Show grouping in numbers if the locale information
	     indicates any.  */
	  spec->info.group = 1;
	  continue;
	case 'I':
	  /* Use the internationalized form of the output.  Currently
	     means to use the `outdigits' of the current locale.  */
	  spec->info.i18n = 1;
	  continue;
	default:
	  break;
	}
      break;
    }
  while (*++format);

  if (spec->info.left)
    spec->info.pad = ' ';

  /* Get the field width.  */
  spec->width_arg = -1;
  spec->info.width = 0;
  if (*format == '*')
    {
      /* The field width is given in an argument.
	 A negative field width indicates left justification.  */
      const unichar *begin = ++format;

      if (ISDIGIT (*format))
	{
	  /* The width argument might be found in a positional parameter.  */
	  n = read_int (&format);

	  if (n > 0 && *format == '$')
	    {
	      spec->width_arg = n - 1;
	      *max_ref_arg = MAX (*max_ref_arg, n);
	      ++format;		/* Skip '$'.  */
	    }
	}

      if (spec->width_arg < 0)
	{
	  /* Not in a positional parameter.  Consume one argument.  */
	  spec->width_arg = posn++;
	  ++nargs;
	  format = begin;	/* Step back and reread.  */
	}
    }
  else if (ISDIGIT (*format))
    /* Constant width specification.  */
    spec->info.width = read_int (&format);

  /* Get the precision.  */
  spec->prec_arg = -1;
  /* -1 means none given; 0 means explicit 0.  */
  spec->info.prec = -1;
  if (*format == '.')
    {
      ++format;
      if (*format == '*')
	{
	  /* The precision is given in an argument.  */
	  const unichar *begin = ++format;

	  if (ISDIGIT (*format))
	    {
	      n = read_int (&format);

	      if (n > 0 && *format == '$')
		{
		  spec->prec_arg = n - 1;
		  *max_ref_arg = MAX (*max_ref_arg, n);
		  ++format;
		}
	    }

	  if (spec->prec_arg < 0)
	    {
	      /* Not in a positional parameter.  */
	      spec->prec_arg = posn++;
	      ++nargs;
	      format = begin;
	    }
	}
      else if (ISDIGIT (*format))
	spec->info.prec = read_int (&format);
      else
	/* "%.?" is treated like "%.0?".  */
	spec->info.prec = 0;
    }

  /* Check for type modifiers.  */
  spec->info.is_long_double = 0;
  spec->info.is_short = 0;
  spec->info.is_long = 0;
  spec->info.is_char = 0;

  switch (*format++)
    {
    case 'h':
      /* ints are short ints or chars.  */
      if (*format != 'h')
	spec->info.is_short = 1;
      else
	{
	  ++format;
	  spec->info.is_char = 1;
	}
      break;
    case 'l':
      /* ints are long ints.  */
      spec->info.is_long = 1;
      if (*format != 'l')
	break;
      ++format;
      /* FALLTHROUGH */
    case 'L':
      /* doubles are long doubles, and ints are long long ints.  */
    case 'q':
      /* 4.4 uses this for long long.  */
      spec->info.is_long_double = 1;
      break;
    case 'z':
    case 'Z':
      /* ints are size_ts.  */
      NSCParameterAssert (sizeof (size_t) <= sizeof (unsigned long long int));
#if defined(LLONG_MAX)
#if LONG_MAX != LLONG_MAX
      spec->info.is_long_double = sizeof (size_t) > sizeof (unsigned long int);
#endif
#endif
      spec->info.is_long = sizeof (size_t) > sizeof (unsigned int);
      break;
    case 't':
      NSCParameterAssert (sizeof (ptrdiff_t) <= sizeof (long long int));
#if defined(LLONG_MAX)
#if LONG_MAX != LLONG_MAX
      spec->info.is_long_double = (sizeof (ptrdiff_t) > sizeof (long int));
#endif
#endif
      spec->info.is_long = sizeof (ptrdiff_t) > sizeof (int);
      break;
    case 'j':
      NSCParameterAssert (sizeof (uintmax_t) <= sizeof (unsigned long long int));
#if defined(LLONG_MAX)
#if LONG_MAX != LLONG_MAX
      spec->info.is_long_double = (sizeof (uintmax_t)
				   > sizeof (unsigned long int));
#endif
#endif
      spec->info.is_long = sizeof (uintmax_t) > sizeof (unsigned int);
      break;
    default:
      /* Not a recognized modifier.  Backup.  */
      --format;
      break;
    }

  /* Get the format specification.  */
  spec->info.spec = (unichar) *format++;
    {
      /* Find the data argument types of a built-in spec.  */
      spec->ndata_args = 1;

      switch (spec->info.spec)
	{
	case 'i':
	case 'd':
	case 'u':
	case 'o':
	case 'X':
	case 'x':
#if defined(LLONG_MAX)
#if LONG_MAX != LLONG_MAX
	  if (spec->info.is_long_double)
	    spec->data_arg_type = PA_INT|PA_FLAG_LONG_LONG;
	  else
#endif
#endif
	    if (spec->info.is_long)
	      spec->data_arg_type = PA_INT|PA_FLAG_LONG;
	    else if (spec->info.is_short)
	      spec->data_arg_type = PA_INT|PA_FLAG_SHORT;
	    else if (spec->info.is_char)
	      spec->data_arg_type = PA_CHAR;
	    else
	      spec->data_arg_type = PA_INT;
	  break;
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
	case 'a':
	case 'A':
	  if (spec->info.is_long_double)
	    spec->data_arg_type = PA_DOUBLE|PA_FLAG_LONG_DOUBLE;
	  else
	    spec->data_arg_type = PA_DOUBLE;
	  break;
	case 'c':
	  spec->data_arg_type = PA_CHAR;
	  break;
	case 'C':
	  spec->data_arg_type = PA_WCHAR;
	  break;
	case 's':
	  spec->data_arg_type = PA_STRING;
	  break;
	case 'S':
	  spec->data_arg_type = PA_WSTRING;
	  break;
	case '@':
	  spec->data_arg_type = PA_OBJECT;
	  break;
	case 'p':
	  spec->data_arg_type = PA_POINTER;
	  break;
	case 'n':
	  spec->data_arg_type = PA_INT|PA_FLAG_PTR;
	  break;

	case 'm':
	default:
	  /* An unknown spec will consume no args.  */
	  spec->ndata_args = 0;
	  break;
	}
    }

  if (spec->data_arg == -1 && spec->ndata_args > 0)
    {
      /* There are args consumed, but no positional spec.  Use the
	 next sequential arg position.  */
      spec->data_arg = posn;
      nargs += spec->ndata_args;
    }

  if (spec->info.spec == '\0')
    /* Format ended before this spec was complete.  */
    spec->end_of_fmt = spec->next_fmt = format - 1;
  else
    {
      /* Find the next format spec.  */
      spec->end_of_fmt = format;
      spec->next_fmt = find_spec (format);
    }

  return nargs;
}

static inline void GSStrAppendUnichar(GSStr s, unichar u)
{
  GSPrivateStrAppendUnichars(s, &u, 1);
}

#define	outchar(Ch)		GSStrAppendUnichar(s, Ch)
#define outstring(String, Len)	GSPrivateStrAppendUnichars(s, String, Len)

/* For handling long_double and longlong we use the same flag.  If
   `long' and `long long' are effectively the same type define it to
   zero.  */
#if defined(LLONG_MAX)
#if LONG_MAX == LLONG_MAX
# define is_longlong 0
#else
# define is_longlong is_long_double
#endif
#else
# define is_longlong 0
#endif

/* If `long' and `int' is effectively the same type we don't have to
   handle `long separately.  */
#if INT_MAX == LONG_MAX
# define is_long_num	0
#else
# define is_long_num	is_long
#endif

static NSString	*locale_sep()
{
  static NSString	*sep = nil;

  if (sep == nil)
    {
      char	buf[32];
      char	*from = buf;
      char	*to;

      snprintf(buf, sizeof(buf), "%g", 1.2);
      if (*from == '1') from++;
      to = from;
      while (*to != '\0' && *to != '2')
	to++;
      *to = '\0';
      sep = [[NSString alloc] initWithCString: from];
    }
  return sep;
}
 

/* Global variables.  */
static const unichar null[] = {'(','n','u','l','l',')','\0'};


/* Handle unknown format specifier.  */
static int printf_unknown (GSStr, const struct printf_info *,
				const void *const *);

/* Group digits of number string.  */
static unichar *group_number (unichar *, unichar *, const char *, NSString *);


/* The function itself.  */
void
GSPrivateFormat (GSStr s, const unichar *format, va_list ap,
NSDictionary *locale)
{
  /* The character used as thousands separator.  */
  NSString *thousands_sep = @"";

  /* The string describing the size of groups of digits.  */
  const char *grouping;

  /* Place to accumulate the result.  */
  int done;

  /* Current character in format string.  */
  const unichar *f;

  /* End of leading constant string.  */
  const unichar *lead_str_end;

  /* Points to next format specifier.  */

  /* Buffer intermediate results.  */
  unichar work_buffer[1000];
  unichar *workend;
  int workend_malloced = 0;

  /* State for restartable multibyte character handling functions.  */

  /* We have to save the original argument pointer.  */
  va_list ap_save;

  /* Count number of specifiers we already processed.  */
  int nspecs_done;

  /* For the %m format we may need the current `errno' value.  */
  int save_errno = errno;


  /* This table maps a character into a number representing a
     class.  In each step there is a destination label for each
     class.  */
  static const int jump_table[] =
  {
    /* ' ' */  1,            0,            0, /* '#' */  4,
	       0, /* '%' */ 14,            0, /* '\''*/  6,
	       0,            0, /* '*' */  7, /* '+' */  2,
	       0, /* '-' */  3, /* '.' */  9,            0,
    /* '0' */  5, /* '1' */  8, /* '2' */  8, /* '3' */  8,
    /* '4' */  8, /* '5' */  8, /* '6' */  8, /* '7' */  8,
    /* '8' */  8, /* '9' */  8,            0,            0,
	       0,            0,            0,            0,
    /* '@' */ 30, /* 'A' */ 26,            0, /* 'C' */ 25,
	       0, /* 'E' */ 19, /* F */   19, /* 'G' */ 19,
	       0, /* 'I' */ 29,            0,            0,
    /* 'L' */ 12,            0,            0,            0,
	       0,            0,            0, /* 'S' */ 21,
	       0,            0,            0,            0,
    /* 'X' */ 18,            0, /* 'Z' */ 13,            0,
	       0,            0,            0,            0,
	       0, /* 'a' */ 26,            0, /* 'c' */ 20,
    /* 'd' */ 15, /* 'e' */ 19, /* 'f' */ 19, /* 'g' */ 19,
    /* 'h' */ 10, /* 'i' */ 15, /* 'j' */ 28,            0,
    /* 'l' */ 11, /* 'm' */ 24, /* 'n' */ 23, /* 'o' */ 17,
    /* 'p' */ 22, /* 'q' */ 12,            0, /* 's' */ 21,
    /* 't' */ 27, /* 'u' */ 16,            0,            0,
    /* 'x' */ 18,            0, /* 'z' */ 13
  };

#define NOT_IN_JUMP_RANGE(Ch) ((Ch) < ' ' || (Ch) > 'z')
#define CHAR_CLASS(Ch) (jump_table[(wint_t) (Ch) - ' '])
# define JUMP_TABLE_TYPE const void *const

  /* Initialize local variables.  */
  done = 0;
  grouping = (const char *) -1;
#ifdef __va_copy
  /* This macro will be available soon in gcc's <stdarg.h>.  We need it
     since on some systems `va_list' is not an integral type.  */
  __va_copy (ap_save, ap);
#else
  ap_save = ap;
#endif
  nspecs_done = 0;

  /* Find the first format specifier.  */
  f = lead_str_end = find_spec ((const unichar *) format);


  /* Write the literal text before the first format.  */
  outstring ((const unichar *) format,
	     lead_str_end - (const unichar *) format);

  /* If we only have to print a simple string, return now.  */
  if (*f == '\0')
    goto all_done;

  /* Process whole format string.  */

      workend = &work_buffer[sizeof (work_buffer) / sizeof (unichar)];

  /* Here starts the more complex loop to handle positional parameters.  */
  {
    /* Array with information about the needed arguments.  This has to
       be dynamically extensible.  */
    size_t nspecs = 0;
    size_t nspecs_max = 32;	/* A more or less arbitrary start value.  */
    struct printf_spec *specs
      = alloca (nspecs_max * sizeof (struct printf_spec));

    /* The number of arguments the format string requests.  This will
       determine the size of the array needed to store the argument
       attributes.  */
    size_t nargs = 0;
    int *args_type;
    union printf_arg *args_value = NULL;

    /* Positional parameters refer to arguments directly.  This could
       also determine the maximum number of arguments.  Track the
       maximum number.  */
    size_t max_ref_arg = 0;

    /* Just a counter.  */
    size_t cnt;


    if (grouping == (const char *) -1)
      {
	thousands_sep = [locale objectForKey: NSThousandsSeparator];
	if (thousands_sep == nil) thousands_sep = @",";

	grouping = ""; // FIXME: grouping info missing in locale?
	if (*grouping == '\0' || *grouping == CHAR_MAX)
	  grouping = NULL;
      }

    for (f = lead_str_end; *f != '\0'; f = specs[nspecs++].next_fmt)
      {
	if (nspecs >= nspecs_max)
	  {
	    /* Extend the array of format specifiers.  */
	    struct printf_spec *old = specs;

	    nspecs_max *= 2;
	    specs = alloca (nspecs_max * sizeof (struct printf_spec));

	    if (specs == &old[nspecs])
	      /* Stack grows up, OLD was the last thing allocated;
		 extend it.  */
	      nspecs_max += nspecs_max / 2;
	    else
	      {
		/* Copy the old array's elements to the new space.  */
		memcpy (specs, old, nspecs * sizeof (struct printf_spec));
		if (old == &specs[nspecs])
		  /* Stack grows down, OLD was just below the new
		     SPECS.  We can use that space when the new space
		     runs out.  */
		  nspecs_max += nspecs_max / 2;
	      }
	  }

	/* Parse the format specifier.  */
	nargs += parse_one_spec (f, nargs, &specs[nspecs], &max_ref_arg);
      }

    /* Determine the number of arguments the format string consumes.  */
    nargs = MAX (nargs, max_ref_arg);

    /* Allocate memory for the argument descriptions.  */
    args_type = alloca (nargs * sizeof (int));
    memset (args_type, 0, nargs * sizeof (int));
    args_value = alloca (nargs * sizeof (union printf_arg));

    /* XXX Could do sanity check here: If any element in ARGS_TYPE is
       still zero after this loop, format is invalid.  For now we
       simply use 0 as the value.  */

    /* Fill in the types of all the arguments.  */
    for (cnt = 0; cnt < nspecs; ++cnt)
      {
	/* If the width is determined by an argument this is an int.  */
	if (specs[cnt].width_arg != -1)
	  args_type[specs[cnt].width_arg] = PA_INT;

	/* If the precision is determined by an argument this is an int.  */
	if (specs[cnt].prec_arg != -1)
	  args_type[specs[cnt].prec_arg] = PA_INT;

	switch (specs[cnt].ndata_args)
	  {
	  case 0:		/* No arguments.  */
	    break;
	  case 1:		/* One argument; we already have the type.  */
	    args_type[specs[cnt].data_arg] = specs[cnt].data_arg_type;
	    break;
	  default:
	    /* ??? */
	    break;
	  }
      }

    /* Now we know all the types and the order.  Fill in the argument
       values.  */
    for (cnt = 0; cnt < nargs; ++cnt)
      switch (args_type[cnt])
	{
#define T(tag, mem, type)						      \
	case tag:							      \
	  args_value[cnt].mem = va_arg (ap_save, type);			      \
	  break

	T (PA_CHAR, pa_char, int); /* Promoted.  */
	T (PA_WCHAR, pa_wchar, int); /* Sometimes promoted.  */
	T (PA_INT|PA_FLAG_SHORT, pa_short_int, int); /* Promoted.  */
	T (PA_INT, pa_int, int);
	T (PA_INT|PA_FLAG_LONG, pa_long_int, long int);
	T (PA_INT|PA_FLAG_LONG_LONG, pa_long_long_int, long long int);
	T (PA_FLOAT, pa_float, double);	/* Promoted.  */
	T (PA_DOUBLE, pa_double, double);
	T (PA_DOUBLE|PA_FLAG_LONG_DOUBLE, pa_long_double, long double);
	T (PA_STRING, pa_string, const char *);
	T (PA_WSTRING, pa_wstring, const wchar_t *);
	T (PA_OBJECT, pa_object, id);
	T (PA_POINTER, pa_pointer, void *);
#undef T
	default:
	  if ((args_type[cnt] & PA_FLAG_PTR) != 0)
	    args_value[cnt].pa_pointer = va_arg (ap_save, void *);
	  else
	    args_value[cnt].pa_long_double = 0.0;
	  break;
	}

    /* Now walk through all format specifiers and process them.  */
    for (; (size_t) nspecs_done < nspecs; ++nspecs_done)
      {
# define REF(Name) &&do2_##Name
#define LABEL(Name) do2_##Name
    /* Step 4: processing format specifier.  */
    static JUMP_TABLE_TYPE step4_jumps[31] =
    {
      REF (form_unknown),
      REF (form_unknown),	/* for ' ' */
      REF (form_unknown),	/* for '+' */
      REF (form_unknown),	/* for '-' */
      REF (form_unknown),	/* for '<hash>' */
      REF (form_unknown),	/* for '0' */
      REF (form_unknown),	/* for '\'' */
      REF (form_unknown),	/* for '*' */
      REF (form_unknown),	/* for '1'...'9' */
      REF (form_unknown),	/* for '.' */
      REF (form_unknown),	/* for 'h' */
      REF (form_unknown),	/* for 'l' */
      REF (form_unknown),	/* for 'L', 'q' */
      REF (form_unknown),	/* for 'z', 'Z' */
      REF (form_percent),	/* for '%' */
      REF (form_integer),	/* for 'd', 'i' */
      REF (form_unsigned),	/* for 'u' */
      REF (form_octal),		/* for 'o' */
      REF (form_hexa),		/* for 'X', 'x' */
      REF (form_float),		/* for 'E', 'e', 'F', 'f', 'G', 'g' */
      REF (form_character),	/* for 'c' */
      REF (form_string),	/* for 's', 'S' */
      REF (form_pointer),	/* for 'p' */
      REF (form_number),	/* for 'n' */
      REF (form_strerror),	/* for 'm' */
      REF (form_wcharacter),	/* for 'C' */
      REF (form_floathex),	/* for 'A', 'a' */
      REF (form_unknown),       /* for 't' */
      REF (form_unknown),       /* for 'j' */
      REF (form_unknown),       /* for 'I' */
      REF (form_object)         /* for '@' */
    };

	int is_negative;
	union
	{
	  unsigned long long int longlong;
	  unsigned long int word;
	} number;
	int base;
	unichar *string;		/* Pointer to argument string.  */

	/* Fill variables from values in struct.  */
	int alt = specs[nspecs_done].info.alt;
	int space = specs[nspecs_done].info.space;
	int left = specs[nspecs_done].info.left;
	int showsign = specs[nspecs_done].info.showsign;
	int group = specs[nspecs_done].info.group;
#if defined(LLONG_MAX) && (LONG_MAX != LLONG_MAX)
	int is_long_double = specs[nspecs_done].info.is_long_double;
#endif
	int is_short = specs[nspecs_done].info.is_short;
	int is_char = specs[nspecs_done].info.is_char;
	int is_long = specs[nspecs_done].info.is_long;
	int width = specs[nspecs_done].info.width;
	int prec = specs[nspecs_done].info.prec;
	int use_outdigits = specs[nspecs_done].info.i18n;
	char pad = specs[nspecs_done].info.pad;
	unichar spec = specs[nspecs_done].info.spec;

	/* Fill in last information.  */
	if (specs[nspecs_done].width_arg != -1)
	  {
	    /* Extract the field width from an argument.  */
	    specs[nspecs_done].info.width =
	      args_value[specs[nspecs_done].width_arg].pa_int;

	    if (specs[nspecs_done].info.width < 0)
	      /* If the width value is negative left justification is
		 selected and the value is taken as being positive.  */
	      {
		specs[nspecs_done].info.width *= -1;
		left = specs[nspecs_done].info.left = 1;
	      }
	    width = specs[nspecs_done].info.width;
	  }

	if (specs[nspecs_done].prec_arg != -1)
	  {
	    /* Extract the precision from an argument.  */
	    specs[nspecs_done].info.prec =
	      args_value[specs[nspecs_done].prec_arg].pa_int;

	    if (specs[nspecs_done].info.prec < 0)
	      /* If the precision is negative the precision is
		 omitted.  */
	      specs[nspecs_done].info.prec = -1;

	    prec = specs[nspecs_done].info.prec;
	  }

	/* Maybe the buffer is too small.  */
	if ((unsigned)(MAX (prec, width) + 32)
	  > sizeof (work_buffer) / sizeof (unichar))
	  {
            size_t      want = ((MAX (prec, width) + 32)
	      * sizeof (unichar)) + (MAX (prec, width) + 32);

            if (want > 168384)
              {
                workend = (unichar *)malloc(want);
                workend_malloced = 1;
              }
            else
              {
                workend = (unichar *)alloca(want);
              }
	  }

	/* Process format specifiers.  */
	while (1)
	  {
	    int string_malloced;
      do
	{
	  void *ptr;
	  ptr = NOT_IN_JUMP_RANGE (spec) ? (void*)REF (form_unknown)
	    : (void*)step4_jumps[CHAR_CLASS (spec)];
	  goto *ptr;
	}
      while (0);

      /* Start real work.  We know about all flags and modifiers and
	 now process the wanted format specifier.  */
    LABEL (form_percent):
      /* Write a literal "%".  */
      outchar ('%');
      break;

    LABEL (form_integer):
      /* Signed decimal integer.  */
      base = 10;

      if (is_longlong)
	{
	  long long int signed_number;

	  signed_number
	    = args_value[specs[nspecs_done].data_arg].pa_long_long_int;

	  is_negative = signed_number < 0;
	  number.longlong = is_negative ? (- signed_number) : signed_number;

	  goto LABEL (longlong_number);
	}
      else
	{
	  long int signed_number;

	  if (is_char)
	    {
	      signed_number = args_value[specs[nspecs_done].data_arg].pa_char;
	    }
	  else if (is_short)
	    {
	      signed_number
		= args_value[specs[nspecs_done].data_arg].pa_short_int;
	    }
	  else if (is_long_num)
	    {
	      signed_number
		= args_value[specs[nspecs_done].data_arg].pa_long_int;
	    }
	  else
	    {
	      signed_number = args_value[specs[nspecs_done].data_arg].pa_int;
	    }

	  is_negative = signed_number < 0;
	  number.word = is_negative ? (- signed_number) : signed_number;

	  goto LABEL (number);
	}
      /* NOTREACHED */

    LABEL (form_unsigned):
      /* Unsigned decimal integer.  */
      base = 10;
      goto LABEL (unsigned_number);
      /* NOTREACHED */

    LABEL (form_octal):
      /* Unsigned octal integer.  */
      base = 8;
      goto LABEL (unsigned_number);
      /* NOTREACHED */

    LABEL (form_hexa):
      /* Unsigned hexadecimal integer.  */
      base = 16;

    LABEL (unsigned_number):	  /* Unsigned number of base BASE.  */

      /* ISO specifies the `+' and ` ' flags only for signed
	 conversions.  */
      is_negative = 0;
      showsign = 0;
      space = 0;

      if (is_longlong)
	{
	  number.longlong
	    = args_value[specs[nspecs_done].data_arg].pa_u_long_long_int;

	LABEL (longlong_number):
	  if (prec < 0)
	    /* Supply a default precision if none was given.  */
	    prec = 1;
	  else
	    /* We have to take care for the '0' flag.  If a precision
	       is given it must be ignored.  */
	    pad = ' ';

	  /* If the precision is 0 and the number is 0 nothing has to
	     be written for the number, except for the 'o' format in
	     alternate form.  */
	  if (prec == 0 && number.longlong == 0)
	    {
	      string = workend;
	      if (base == 8 && alt)
		*--string = '0';
	    }
	  else
	    {
	      /* Put the number in WORK.  */
	      string = _itowa (number.longlong, workend, base,
			      spec == 'X');
	      if (group && grouping)
		string = group_number (string, workend, grouping,
				       thousands_sep);

	      if (use_outdigits && base == 10)
		string = _i18n_number_rewrite
		  (string, workend, [locale objectForKey: NSDecimalDigits]);
	    }
	  /* Simplify further test for num != 0.  */
	  number.word = number.longlong != 0;
	}
      else
	{
	  if (is_long_num)
	    number.word = args_value[specs[nspecs_done].data_arg].pa_u_long_int;
	  else if (is_char)
	    number.word = (unsigned char)
	      args_value[specs[nspecs_done].data_arg].pa_char;
	  else if (!is_short)
	    number.word = args_value[specs[nspecs_done].data_arg].pa_u_int;
	  else
	    number.word = (unsigned short int)
	      args_value[specs[nspecs_done].data_arg].pa_u_short_int;

	LABEL (number):
	  if (prec < 0)
	    /* Supply a default precision if none was given.  */
	    prec = 1;
	  else
	    /* We have to take care for the '0' flag.  If a precision
	       is given it must be ignored.  */
	    pad = ' ';

	  /* If the precision is 0 and the number is 0 nothing has to
	     be written for the number, except for the 'o' format in
	     alternate form.  */
	  if (prec == 0 && number.word == 0)
	    {
	      string = workend;
	      if (base == 8 && alt)
		*--string = '0';
	    }
	  else
	    {
	      /* Put the number in WORK.  */
	      string = _itowa_word (number.word, workend, base,
				   spec == 'X');
	      if (group && grouping)
		string = group_number (string, workend, grouping,
				       thousands_sep);

	      if (use_outdigits && base == 10)
		string = _i18n_number_rewrite
		  (string, workend, [locale objectForKey: NSDecimalDigits]);
	    }
	}

      if (prec <= workend - string && number.word != 0 && alt && base == 8)
	/* Add octal marker.  */
	*--string = '0';

      prec = MAX (0, prec - (workend - string));

      if (!left)
	{
	  width -= workend - string + prec;

	  if (number.word != 0 && alt && base == 16)
	    /* Account for 0X hex marker.  */
	    width -= 2;

	  if (is_negative || showsign || space)
	    --width;

	  if (pad == ' ')
	    {
	      PAD (' ');
	      width = 0;
	    }

	  if (is_negative)
	    outchar ('-');
	  else if (showsign)
	    outchar ('+');
	  else if (space)
	    outchar (' ');

	  if (number.word != 0 && alt && base == 16)
	    {
	      outchar ('0');
	      outchar (spec);
	    }

	  width += prec;
	  PAD ('0');

	  outstring (string, workend - string);

	  break;
	}
      else
	{
	  if (is_negative)
	    {
	      outchar ('-');
	      --width;
	    }
	  else if (showsign)
	    {
	      outchar ('+');
	      --width;
	    }
	  else if (space)
	    {
	      outchar (' ');
	      --width;
	    }

	  if (number.word != 0 && alt && base == 16)
	    {
	      outchar ('0');
	      outchar (spec);
	      width -= 2;
	    }

	  width -= workend - string + prec;

	  if (prec > 0)
	    {
	      int temp = width;
	      width = prec;
	      PAD ('0');
	      width = temp;
	    }

	  outstring (string, workend - string);

	  PAD (' ');
	  break;
	}

    LABEL (form_float):
      {
	/* Floating-point number.  This is handled by the native sprintf.  */
	char buf1[32], *bp;
	char buf2[specs[nspecs_done].info.width
	  +specs[nspecs_done].info.prec+32];
	unichar work_buffer[MAX (specs[nspecs_done].info.width,
	  specs[nspecs_done].info.spec) + 32];
	unichar *const workend
	  = &work_buffer[sizeof (work_buffer) / sizeof (unichar)];
	register unichar *w;
	NSString	*decimal_sep;

	decimal_sep = [locale objectForKey: NSDecimalSeparator];
	if (decimal_sep == nil) decimal_sep = @".";

	bp = buf1;

	*bp++ = '%';

	if (specs[nspecs_done].info.alt)
	  *bp++ = '#';
	if (specs[nspecs_done].info.group)
	  *bp++ = '\'';
	if (specs[nspecs_done].info.showsign)
	  *bp++ = '+';
	else if (specs[nspecs_done].info.space)
	  *bp++ = ' ';
	if (specs[nspecs_done].info.left)
	  *bp++ = '-';
	if (specs[nspecs_done].info.pad == '0')
	  *bp++ = '0';
	if (specs[nspecs_done].info.i18n)
	  *bp++ = 'I';

	if (specs[nspecs_done].info.width != 0)
	  {
	    w = _itowa_word (specs[nspecs_done].info.width, workend, 10, 0);
	    while (w < workend)
	      *bp++ = *w++;
	  }

	if (specs[nspecs_done].info.prec != -1)
	  {
	    *bp++ = '.';
	    w = _itowa_word (specs[nspecs_done].info.prec, workend, 10, 0);
	    while (w < workend)
	      *bp++ = *w++;
	  }

	if (specs[nspecs_done].info.spec != '\0')
	  *bp++ = specs[nspecs_done].info.spec;

	*bp = '\0';

	if (specs[nspecs_done].info.is_long_double)
	  {
	    snprintf(buf2, sizeof(buf2), buf1,
	      args_value[specs[nspecs_done].data_arg].pa_long_double);
	  }
	else
	  {
	    snprintf(buf2, sizeof(buf2), buf1,
	      args_value[specs[nspecs_done].data_arg].pa_double);
	  }

	/*
	 * FIXME - hack to rewrite decimal separator into correct locale
	 * if necessary.
	 */
	if (decimal_sep != nil)
	  {
	    NSString	*sep = locale_sep();

	    if ([decimal_sep isEqual: sep] == NO && [sep length] == 1)
	      {
		unichar	m = [sep characterAtIndex: 0];
		char	*p = &buf2[strlen(buf2)];

		/*
		 * Assume that we won't be finding an escape in the string
		 * so we can use it as a marker.
		 */
		while (p-- > buf2)
		  {
		    if (*p == m)
		      {
			*p = '\033';
			break;
		      }
		  }
	      }
	  }

	bp = buf2;
	while (*bp)
	  {
	    if (*bp == '\033')
	      {
		int	i = 0;
		int	c = [decimal_sep length];
		unichar	b[c];

		[decimal_sep getCharacters: b];
		while (i < c)
		  {
		    outchar(b[i++]);
		  }
		bp++;
	      }
	    else
	      {
		outchar(*bp++);
	      }
	  }
      }
      break;

    LABEL (form_floathex):
      {
	/* Floating point number printed as hexadecimal number.  */
	char buf1[32], *bp;
	char buf2[specs[nspecs_done].info.width
	  +specs[nspecs_done].info.prec+32];
	unichar work_buffer[MAX (specs[nspecs_done].info.width,
	  specs[nspecs_done].info.spec) + 32];
	unichar *const workend
	  = &work_buffer[sizeof (work_buffer) / sizeof (unichar)];
	register unichar *w;
	NSString	*decimal_sep;

	decimal_sep = [locale objectForKey: NSDecimalSeparator];
	if (decimal_sep == nil) decimal_sep = @".";

	bp = buf1;

	*bp++ = '%';

	if (specs[nspecs_done].info.alt)
	  *bp++ = '#';
	if (specs[nspecs_done].info.group)
	  *bp++ = '\'';
	if (specs[nspecs_done].info.showsign)
	  *bp++ = '+';
	else if (specs[nspecs_done].info.space)
	  *bp++ = ' ';
	if (specs[nspecs_done].info.left)
	  *bp++ = '-';
	if (specs[nspecs_done].info.pad == '0')
	  *bp++ = '0';
	if (specs[nspecs_done].info.i18n)
	  *bp++ = 'I';

	if (specs[nspecs_done].info.width != 0)
	  {
	    w = _itowa_word (specs[nspecs_done].info.width, workend, 10, 0);
	    while (w < workend)
	      *bp++ = *w++;
	  }

	if (specs[nspecs_done].info.prec != -1)
	  {
	    *bp++ = '.';
	    w = _itowa_word (specs[nspecs_done].info.prec, workend, 10, 0);
	    while (w < workend)
	      *bp++ = *w++;
	  }

	if (specs[nspecs_done].info.spec != '\0')
	  *bp++ = specs[nspecs_done].info.spec;

	*bp = '\0';

	if (specs[nspecs_done].info.is_long_double)
	  {
	    snprintf(buf2, sizeof(buf2), buf1,
	      args_value[specs[nspecs_done].data_arg].pa_long_double);
	  }
	else
	  {
	    snprintf(buf2, sizeof(buf2), buf1,
	      args_value[specs[nspecs_done].data_arg].pa_double);
	  }

	/*
	 * FIXME - hack to rewrite decimal separator into correct locale
	 * if necessary.
	 */
	if (decimal_sep != nil)
	  {
	    NSString	*sep = locale_sep();

	    if ([decimal_sep isEqual: sep] == NO && [sep length] == 1)
	      {
		unichar	m = [sep characterAtIndex: 0];
		char	*p = &buf2[strlen(buf2)];

		/*
		 * Assume that we won't be finding an escape in the string
		 * so we can use it as a marker.
		 */
		while (p-- > buf2)
		  {
		    if (*p == m)
		      {
			*p = '\033';
			break;
		      }
		  }
	      }
	  }

	bp = buf2;
	while (*bp)
	  {
	    if (*bp == '\033')
	      {
		int	i = 0;
		int	c = [decimal_sep length];
		unichar	b[c];

		[decimal_sep getCharacters: b];
		while (i < c)
		  {
		    outchar(b[i++]);
		  }
		bp++;
	      }
	    else
	      {
		outchar(*bp++);
	      }
	  }
      }
      break;

    LABEL (form_pointer):
      /* Generic pointer.  */
      {
	const void *ptr;
	  ptr = args_value[specs[nspecs_done].data_arg].pa_pointer;
	if (ptr != NULL)
	  {
	    /* If the pointer is not NULL, write it as a %#x spec.  */
	    base = 16;
	    number.word = (size_t) ptr;
	    is_negative = 0;
	    alt = 1;
	    group = 0;
	    spec = 'x';
	    goto LABEL (number);
	  }
	else
	  {
	    string = NULL;
	    goto LABEL (print_string);
	  }
      }
      /* NOTREACHED */

    LABEL (form_number):
      /* Answer the count of characters written.  */
	if (is_longlong)
	  *(long long int *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
	else if (is_long_num)
	  *(long int *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
	else if (is_long_num)
	  *(long int *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
	else if (is_char)
	  *(char *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
	else if (!is_short)
	  *(int *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
	else
	  *(short int *) args_value[specs[nspecs_done].data_arg].pa_pointer = done;
      break;

    LABEL (form_strerror):
      /* Print description of error ERRNO.  */
      errno = save_errno;
      string = (unichar *)(void*)[[[NSError _last] localizedDescription]
	cStringUsingEncoding: NSUnicodeStringEncoding];
      is_long = 1;		/* This is a unicode string.  */
      goto LABEL (print_string);
    LABEL (form_character):
      /* Character.  */
      if (is_long)
	goto LABEL (form_wcharacter);
      --width;	/* Account for the character itself.  */
      if (!left)
	PAD (' ');
	outchar (((unsigned char)
			  args_value[specs[nspecs_done].data_arg].pa_char));
      if (left)
	PAD (' ');
      break;

    LABEL (form_wcharacter):
      {
	/* Wide character.  */
	--width;
	if (!left)
	  PAD (' ');
	  outchar (args_value[specs[nspecs_done].data_arg].pa_wchar);
	if (left)
	  PAD (' ');
      }
      break;

    LABEL (form_string):
      {
	size_t len = 0;

	/* The string argument could in fact be `char *' or `wchar_t *'.
	   But this should not make a difference here.  */
	string = (unichar *) args_value[specs[nspecs_done].data_arg].pa_wstring;

	/* Entry point for printing other strings.  */
      LABEL (print_string):

	string_malloced = 0;
	if (string == NULL)
	  {
	    /* Write "(null)" if there's space.  */
	    if (prec == -1
	      || prec >= (int) (sizeof (null) / sizeof (null[0])) - 1)
	      {
		string = (unichar *) null;
		len = (sizeof (null) / sizeof (null[0])) - 1;
	      }
	    else
	      {
                static unichar  empty = 0;
		string = &empty;
		len = 0;
	      }
	  }
	else if (!is_long && spec != 'S')
	  {
	    /* This is complicated.  We have to transform the multibyte
	       string into a unicode string.  */
	    const char			*str = (const char*)string;
	    unsigned			blen;
	    static NSStringEncoding	enc = GSUndefinedEncoding;
	    static BOOL			byteEncoding = NO;

	    if (enc == GSUndefinedEncoding)
	      {
	        enc = [NSString defaultCStringEncoding];
		byteEncoding = GSPrivateIsByteEncoding(enc);
	      }

	    if (-1 == prec)
              {
                len = strlen(str);	// Number of bytes to convert.
                blen = len;		// Size of unichar output buffer.
              }
	    else
	      {
		if (byteEncoding == YES)
		  {
		    /* Where the external encoding is one byte per character,
		     * we know we don't need to convert more bytes than the
		     * precision required for output.
		     */
                    len = 0;
                    while (len < prec && str[len] != 0)
                      {
                        len++;
		      }
                    blen = len;
		  }
		else
		  {
                    /* FIXME ... it looks like modern standards mean that
                     * the number of *bytes* in an input string may not
                     * exceed the precision ... but that's unintuitive for
                     * input strings with multibyte characters, so we need
                     * to check and emulate OSX behavior.
                     */
                    len = 0;
                    while (len < prec && str[len] != 0)
                      {
                        len++;
		      }
                    blen = len;
		  }
	      }

	    /* Allocate dynamically an array which definitely is long
	     * enough for the unichar version.
	     */
	    if (blen < 8192 || ((string = (unichar *)
	      NSZoneMalloc(s->_zone, (blen + 1) * sizeof(unichar))) == NULL))
	      string = (unichar *) alloca((blen + 1) * sizeof(unichar));
	    else
	      string_malloced = 1;

	    GSToUnicode(&string, &blen, (unsigned char*)str, len, enc, 0, 0);

	    /* get the number of unichars produced and truncate to the
	     * required output precision if necessary.
	     */
	    len = blen;
	    if (prec != -1 && prec < len)
	      {
	        len = prec;
	      }
	  }
	else
	  {
	    /* This is simple.  Wide string == unicode string.  */
	    int prc;
	    unichar *wsp;

	    len = 0;
	    prc = prec;
	    wsp = (unichar *)string;
	    while (prc-- && *wsp++) len++;
	  }

	if ((width -= len) <= 0)
	  {
	    outstring (string, len);
	  }
	else
	  {
	    if (!left)
	      PAD (' ');
	    outstring (string, len);
	    if (left)
	      PAD (' ');
	  }
	if (string_malloced)
	  NSZoneFree(s->_zone, string);
      }
      break;

    LABEL (form_object):
      {
	size_t len;
	id obj;
	NSString *dsc;

	obj = args_value[specs[nspecs_done].data_arg].pa_object;

	/* On OSX a nil object is reported as '(null)' so we do the same.
	 */
	if (!obj) dsc = @"(null)";
	else if ([obj respondsToSelector: @selector(descriptionWithLocale:)]) dsc = [obj descriptionWithLocale: locale];
	else dsc = [obj description];

	if (!dsc) dsc = @"(null)";

	len = [dsc length];

	string_malloced = 0;
	  {
	    /* This is complicated.  We have to transform the
	       NSString into a unicode string.  */
	    NSRange r;

	    if (prec >= 0 && prec < (int)len) len = prec;

	    /* Allocate dynamically an array which definitely is long
	       enough for the wide character version.  */
	    if (len < 8192
	      || ((string = (unichar *) NSZoneMalloc(s->_zone, len * sizeof (unichar)))
		    == NULL))
	      string = (unichar *) alloca (len * sizeof (unichar));
	    else
	      string_malloced = 1;

	    r.location = 0;
	    r.length = len;
	    [dsc getCharacters: string range: r];
	  }

	if ((width -= len) < 0)
	  {
	    outstring (string, len);
	  }
	else
	  {
	    if (!left)
	      PAD (' ');
	    outstring (string, len);
	    if (left)
	      PAD (' ');
	  }
	if (string_malloced)
	  NSZoneFree(s->_zone, string);
      }
      break;

	  LABEL (form_unknown):
	    {
	      int function_done;
	      unsigned int i;
	      const void **ptr;


	      ptr = alloca (specs[nspecs_done].ndata_args
			    * sizeof (const void *));

	      /* Fill in an array of pointers to the argument values.  */
	      for (i = 0; i < specs[nspecs_done].ndata_args; ++i)
		ptr[i] = &args_value[specs[nspecs_done].data_arg + i];

	      /* Call the function.  */
	      function_done = printf_unknown(s, &specs[nspecs_done].info, ptr);

	      /* If an error occurred we don't have information about #
		 of chars.  */

	      done += function_done;
	    }
	    break;
	  }

	/* Write the following constant string.  */
	outstring (specs[nspecs_done].end_of_fmt,
		   specs[nspecs_done].next_fmt
		   - specs[nspecs_done].end_of_fmt);
      }
  }

all_done:
  if (workend_malloced) free(workend);
  /* Unlock the stream.  */
#ifdef __va_copy
  va_end(ap_save);
#endif
  return;
}

/* Handle an unknown format specifier.  This prints out a canonicalized
   representation of the format spec itself.  */
static int
printf_unknown (GSStr s, const struct printf_info *info,
		const void *const *args)

{
  int done = 0;
  unichar work_buffer[MAX (info->width, info->spec) + 32];
  unichar *const workend
    = &work_buffer[sizeof (work_buffer) / sizeof (unichar)];
  register unichar *w;

  outchar ('%');

  if (info->alt)
    outchar ('#');
  if (info->group)
    outchar ('\'');
  if (info->showsign)
    outchar ('+');
  else if (info->space)
    outchar (' ');
  if (info->left)
    outchar ('-');
  if (info->pad == '0')
    outchar ('0');
  if (info->i18n)
    outchar ('I');

  if (info->width != 0)
    {
      w = _itowa_word (info->width, workend, 10, 0);
      while (w < workend)
	outchar (*w++);
    }

  if (info->prec != -1)
    {
      outchar ('.');
      w = _itowa_word (info->prec, workend, 10, 0);
      while (w < workend)
	outchar (*w++);
    }

  if (info->spec != '\0')
    outchar (info->spec);

  return done;
}

/* Group the digits according to the grouping rules of the current locale.
   The interpretation of GROUPING is as in `struct lconv' from <locale.h>.  */
static unichar *
group_number (unichar *w, unichar *rear_ptr, const char *grouping,
	      NSString *thousands_sep
	     )
{
  int len;
  unichar *src, *s;

  /* We treat all negative values like CHAR_MAX.  */

  if (*grouping == CHAR_MAX || *grouping <= 0)
    /* No grouping should be done.  */
    return w;

  len = *grouping;

  /* Copy existing string so that nothing gets overwritten.  */
  src = (unichar *) alloca ((rear_ptr - w) * sizeof (unichar));
  s = (unichar *) memcpy (src, w,
			    (rear_ptr - w) * sizeof (unichar));
  w = rear_ptr;

  /* Process all characters in the string.  */
  while (s > src)
    {
      *--w = *--s;

      if (--len == 0 && s > src)
	{
	  /* A new group begins.  */
	  *--w = [thousands_sep characterAtIndex: 0];

	  len = *grouping++;
	  if (*grouping == '\0')
	    /* The previous grouping repeats ad infinitum.  */
	    --grouping;
	  else if (*grouping == CHAR_MAX
#if CHAR_MIN < 0
		   || *grouping < 0
#endif
		  )
	    {
	      /* No further grouping to be done.
		 Copy the rest of the number.  */
	      do
		{
		  *--w = *--s;
		}
	      while (s > src);
	      break;
	    }
	}
    }
  return w;
}

