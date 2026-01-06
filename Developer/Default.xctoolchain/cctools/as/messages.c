/* messages.c - error reporter -
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/mach_init.h>
#if defined(__OPENSTEP__) || defined(__GONZO_BUNSEN_BEAKER__)
#include <servers/netname.h>
#else
#include <servers/bootstrap.h>
#endif
#include "as.h"
#include "input-scrub.h"
#include "messages.h"

/*
		ERRORS

	We print the error message 1st, beginning in column 1.
	All ancillary info starts in column 2 on lines after the
	key error text.
	We try to print a location in logical and physical file
	just after the main error text.
	Caller then prints any appendices after that, begining all
	lines with at least 1 space.

	Optionally, we may die.
	There is no need for a trailing '\n' in your error text format
	because we supply one.

as_warn(fmt,args)  Like fprintf(stderr,fmt,args) but also call errwhere().

as_fatal(fmt,args) Like as_warn() but exit with a fatal status.

*/


/*
 * Nonzero if we've hit a 'bad error', and should not write an obj file,
 * and exit with a nonzero error code.
 */
int bad_error = 0;

/*
 * If set to non-zero in main() -arch_multiple as been specified so if any
 * error messages are printed print a single line first to start which errors
 * the architectures are for.
 */
int arch_multiple = 0;

/*
 * architecture_banner() returns the string to say what target we are assembling
 * for.
 */
static
const char *
architecture_banner(void)
{
#ifdef M68K
	return("as: for architecture m68k\n");
#endif
#ifdef M88K
	return("as: for architecture m88k\n");
#endif
#ifdef PPC
	return("as: for architecture ppc\n");
#endif
#ifdef I860
	return("as: for architecture i860\n");
#endif
#ifdef I386
	return("as: for architecture i386\n");
#endif
#ifdef HPPA
	return("as: for architecture hppa\n");
#endif
#ifdef SPARC
	return("as: for architecture sparc\n");
#endif
#ifdef ARM
	return("as: for architecture arm\n");
#endif
}

/*
 * print_architecture_banner() prints what architecture we are assembling for
 * if it has not previously been printed.
 */
static
void
print_architecture_banner(void)
{
    static int printed = 0;

	if(arch_multiple && !printed){
	    printf("%s", architecture_banner());
	    printed = 1;
	}
}

/*
 *			a s _ w a r n ( )
 *
 * Send to stderr a string as a warning, and locate warning in input file(s).
 * Please only use this for when we have some recovery action.
 * Please explain in string (which may have '\n's) what recovery was done.
 */
void
as_warn(
const char *format,
...)
{
    va_list ap;

	if(!flagseen['W']){
	    print_architecture_banner();
	    as_where();
	    va_start(ap, format);
	    vfprintf(stderr, format, ap);
	    fprintf(stderr, "\n");
	    va_end(ap);
	}
}

/* Like as_bad but the file name and line number are passed in.  */
void
as_warn_where (char *file, unsigned int line, const char *format, ...)
{
  va_list args;

  if (!flagseen['W'])
    {
	    print_architecture_banner();
		fprintf(stderr,"%s:%u:", file, line);
	    va_start (args, format);
	    vfprintf(stderr, format, args);
	    va_end (args);
    }
}

/*
 * Like as_warn_where but the file name and optional line number and column
 * are passed in.
 */
void
as_warn_where_with_column (char *file, unsigned int line, unsigned int column, const char *format, ...)
{
  va_list args;

  if (!flagseen['W'])
    {
	    print_architecture_banner();
	    fprintf(stderr, "%s:", file);
	    if (line)
	      {
		fprintf(stderr, "%u:", line);
		if (column)
		    fprintf(stderr, "%u:", column);
	      }
	    va_start (args, format);
	    vfprintf(stderr, format, args);
	    fprintf(stderr, "\n");
	    va_end (args);
    }
}

/*
 *			a s _ b a d ( )
 *
 * Send to stderr a string as a warning, * and locate warning in input file(s).
 * Please us when there is no recovery, but we want to continue processing
 * but not produce an object file.
 * Please explain in string (which may have '\n's) what recovery was done.
 */
void
as_bad(
const char *format,
...)
{
    va_list ap;

	print_architecture_banner();
	bad_error = 1;
	as_where();
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

/*
 *			a s _ f a t a l ( )
 *
 * Send to stderr a string (with bell) (JF: Bell is obnoxious!) as a fatal
 * message, and locate stdsource in input file(s).
 * Please only use this for when we DON'T have some recovery action.
 * It exit()s with a warning status.
 */
void
as_fatal(
const char *format,
...)
{
    va_list ap;

	print_architecture_banner();
	bad_error = 1;
	as_where();
	va_start(ap, format);
	fprintf (stderr, "FATAL:");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void
sprint_value (char *buf, signed_expr_t val)
{
    /* cctools-port: Replaced 'qd' with 'lld' in format string */
    sprintf (buf, "%lld", val);
}
