/* strtol - convert string representation of a number into a long integer value. */

/* Copyright (C) 1991,92,94,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.

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
*/

#include <config.h>

#if !HAVE_STRTOL

#include <chartypes.h>
#include <errno.h>

#ifndef errno
extern int errno;
#endif

#ifndef __set_errno
#  define __set_errno(Val) errno = (Val)
#endif

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#include <typemax.h>

#include <stdc.h>
#include <bashansi.h>

#ifndef NULL
#  define NULL 0
#endif

/* Nonzero if we are defining `strtoul' or `strtoull', operating on
   unsigned integers.  */
#ifndef UNSIGNED
#  define UNSIGNED 0
#  define INT LONG int
#else
#  define INT unsigned LONG int
#endif

#if UNSIGNED
#  ifdef QUAD
#    define strtol strtoull
#  else
#    define strtol strtoul
#  endif
#else
#  ifdef QUAD
#    define strtol strtoll
#  endif
#endif

/* If QUAD is defined, we are defining `strtoll' or `strtoull',
   operating on `long long ints.  */

#ifdef QUAD
#  define LONG long long
#  define STRTOL_LONG_MIN LLONG_MIN
#  define STRTOL_LONG_MAX LLONG_MAX
#  define STRTOL_ULONG_MAX ULLONG_MAX
#else	/* !QUAD */
#  define LONG long
#  define STRTOL_LONG_MIN LONG_MIN
#  define STRTOL_LONG_MAX LONG_MAX
#  define STRTOL_ULONG_MAX ULONG_MAX
#endif

/* Convert NPTR to an `unsigned long int' or `long int' in base BASE.
   If BASE is 0 the base is determined by the presence of a leading
   zero, indicating octal or a leading "0x" or "0X", indicating hexadecimal.
   If BASE is < 2 or > 36, it is no longer reset to 10; EINVAL is returned.
   If ENDPTR is not NULL, a pointer to the character after the last
   one converted is stored in *ENDPTR.  */

INT
strtol (nptr, endptr, base)
     const char *nptr;
     char **endptr;
     int base;
{
  int negative;
  register unsigned LONG int cutoff;
  register unsigned int cutlim;
  register unsigned LONG int i;
  register const char *s;
  register unsigned char c;
  const char *save, *end;
  int overflow;

  if (base < 0 || base == 1 || base > 36)
    {
      __set_errno (EINVAL);
      return 0;
    }

  save = s = nptr;

  /* Skip white space.  */
  while (ISSPACE ((unsigned char)*s))
    ++s;
  if (*s == '\0')
    goto noconv;

  /* Check for a sign.  */
  if (*s == '-' || *s == '+')
    {
      negative = (*s == '-');
      ++s;
    }
  else
    negative = 0;

  /* Recognize number prefix and if BASE is zero, figure it out ourselves.  */
  if (*s == '0')
    {
      if ((base == 0 || base == 16) && TOUPPER ((unsigned char) s[1]) == 'X')
	{
	  s += 2;
	  base = 16;
	}
      else if (base == 0)
	base = 8;
    }
  else if (base == 0)
    base = 10;

  /* Save the pointer so we can check later if anything happened.  */
  save = s;

  end = NULL;

  cutoff = STRTOL_ULONG_MAX / (unsigned LONG int) base;
  cutlim = STRTOL_ULONG_MAX % (unsigned LONG int) base;

  overflow = 0;
  i = 0;
  c = *s;
  if (sizeof (long int) != sizeof (LONG int))
    {
      unsigned long int j = 0;
      unsigned long int jmax = ULONG_MAX / base;

      for (;c != '\0'; c = *++s)
	{
	  if (s == end)
	    break;
	  if (DIGIT (c))
	    c -= '0';
	  else if (ISALPHA (c))
	    c = TOUPPER (c) - 'A' + 10;
	  else
	    break;

	  if ((int) c >= base)
	    break;
	  /* Note that we never can have an overflow.  */
	  else if (j >= jmax)
	    {
	      /* We have an overflow.  Now use the long representation.  */
	      i = (unsigned LONG int) j;
	      goto use_long;
	    }
	  else
	    j = j * (unsigned long int) base + c;
	}

      i = (unsigned LONG int) j;
    }
  else
    for (;c != '\0'; c = *++s)
      {
	if (s == end)
	  break;
	if (DIGIT (c))
	  c -= '0';
	else if (ISALPHA (c))
	  c = TOUPPER (c) - 'A' + 10;
	else
	  break;
	if ((int) c >= base)
	  break;
	/* Check for overflow.  */
	if (i > cutoff || (i == cutoff && c > cutlim))
	  overflow = 1;
	else
	  {
	  use_long:
	    i *= (unsigned LONG int) base;
	    i += c;
	  }
      }

  /* Check if anything actually happened.  */
  if (s == save)
    goto noconv;

  /* Store in ENDPTR the address of one character
     past the last character we converted.  */
  if (endptr != NULL)
    *endptr = (char *) s;

#if !UNSIGNED
  /* Check for a value that is within the range of
     `unsigned LONG int', but outside the range of `LONG int'.  */
  if (overflow == 0
      && i > (negative
	      ? -((unsigned LONG int) (STRTOL_LONG_MIN + 1)) + 1
	      : (unsigned LONG int) STRTOL_LONG_MAX))
    overflow = 1;
#endif

  if (overflow)
    {
      __set_errno (ERANGE);
#if UNSIGNED
      return STRTOL_ULONG_MAX;
#else
      return negative ? STRTOL_LONG_MIN : STRTOL_LONG_MAX;
#endif
    }

  /* Return the result of the appropriate sign.  */
  return negative ? -i : i;

noconv:
  /* We must handle a special case here: the base is 0 or 16 and the
     first two characters are '0' and 'x', but the rest are no
     hexadecimal digits.  This is no error case.  We return 0 and
     ENDPTR points to the `x`.  */
  if (endptr != NULL)
    {
      if (save - nptr >= 2 && TOUPPER ((unsigned char) save[-1]) == 'X' && save[-2] == '0')
	*endptr = (char *) &save[-1];
      else
	/*  There was no number to convert.  */
	*endptr = (char *) nptr;
    }

  return 0L;
}

#endif /* !HAVE_STRTOL */
