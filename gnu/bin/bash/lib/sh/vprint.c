/* vprint.c -- v[fs]printf() for 4.[23] BSD systems. */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#if defined (USE_VFPRINTF_EMULATION)

#include <stdio.h>

#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *)0)
#  else
#    define NULL 0x0
#  endif /* __STDC__ */
#endif /* !NULL */

/*
 * Beware!  Don't trust the value returned by either of these functions; it
 * seems that pre-4.3-tahoe implementations of _doprnt () return the first
 * argument, i.e. a char *.
 */
#include <varargs.h>

int
vfprintf (iop, fmt, ap)
     FILE *iop;
     char *fmt;
     va_list ap;
{
  int len;
  char localbuf[BUFSIZ];

  if (iop->_flag & _IONBF)
    {
      iop->_flag &= ~_IONBF;
      iop->_ptr = iop->_base = localbuf;
      len = _doprnt (fmt, ap, iop);
      (void) fflush (iop);
      iop->_flag |= _IONBF;
      iop->_base = NULL;
      iop->_bufsiz = 0;
      iop->_cnt = 0;
    }
  else
    len = _doprnt (fmt, ap, iop);
  return (ferror (iop) ? EOF : len);
}

/*
 * Ditto for vsprintf
 */
int
vsprintf (str, fmt, ap)
     char *str, *fmt;
     va_list ap;
{
  FILE f;
  int len;

  f._flag = _IOWRT|_IOSTRG;
  f._ptr = str;
  f._cnt = 32767;
  len = _doprnt (fmt, ap, &f);
  *f._ptr = 0;
  return (len);
}
#endif /* USE_VFPRINTF_EMULATION */
