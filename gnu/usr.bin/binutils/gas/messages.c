/* messages.c - error reporter -
   Copyright (C) 1987-2023 Free Software Foundation, Inc.
   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include <limits.h>
#include <signal.h>

/* If the system doesn't provide strsignal, we get it defined in
   libiberty but no declaration is supplied.  Because, reasons. */
#if !defined (HAVE_STRSIGNAL) && !defined (strsignal)
extern const char *strsignal (int);
#endif

static void identify (const char *);
static void as_show_where (void);
static void as_warn_internal (const char *, unsigned int, char *);
static void as_bad_internal (const char *, unsigned int, char *);
static void signal_crash (int) ATTRIBUTE_NORETURN;

/* Despite the rest of the comments in this file, (FIXME-SOON),
   here is the current scheme for error messages etc:

   as_fatal() is used when gas is quite confused and
   continuing the assembly is pointless.  In this case we
   exit immediately with error status.

   as_bad() is used to mark errors that result in what we
   presume to be a useless object file.  Say, we ignored
   something that might have been vital.  If we see any of
   these, assembly will continue to the end of the source,
   no object file will be produced, and we will terminate
   with error status.  The new option, -Z, tells us to
   produce an object file anyway but we still exit with
   error status.  The assumption here is that you don't want
   this object file but we could be wrong.

   as_warn() is used when we have an error from which we
   have a plausible error recovery.  eg, masking the top
   bits of a constant that is longer than will fit in the
   destination.  In this case we will continue to assemble
   the source, although we may have made a bad assumption,
   and we will produce an object file and return normal exit
   status (ie, no error).  The new option -X tells us to
   treat all as_warn() errors as as_bad() errors.  That is,
   no object file will be produced and we will exit with
   error status.  The idea here is that we don't kill an
   entire make because of an error that we knew how to
   correct.  On the other hand, sometimes you might want to
   stop the make at these points.

   as_tsktsk() is used when we see a minor error for which
   our error recovery action is almost certainly correct.
   In this case, we print a message and then assembly
   continues as though no error occurred.

   as_abort () is used for logic failure (assert or abort, signal).
*/

static void
identify (const char *file)
{
  static int identified;

  if (identified)
    return;
  identified++;

  if (!file)
    {
      unsigned int x;
      file = as_where (&x);
    }

  if (file)
    fprintf (stderr, "%s: ", file);
  fprintf (stderr, _("Assembler messages:\n"));
}

/* The number of warnings issued.  */
static int warning_count;

int
had_warnings (void)
{
  return warning_count;
}

/* Nonzero if we've hit a 'bad error', and should not write an obj file,
   and exit with a nonzero error code.  */

static int error_count;

int
had_errors (void)
{
  return error_count;
}

/* Print the current location to stderr.  */

static void
as_show_where (void)
{
  const char *file;
  unsigned int line;

  file = as_where_top (&line);
  identify (file);
  if (file)
    {
      if (line != 0)
	fprintf (stderr, "%s:%u: ", file, line);
      else
	fprintf (stderr, "%s: ", file);
    }
}

/* Send to stderr a string as information, with location data passed in.
   Note that for now this is not intended for general use.  */

void
as_info_where (const char *file, unsigned int line, unsigned int indent,
	       const char *format, ...)
{
  va_list args;
  char buffer[2000];

  va_start (args, format);
  vsnprintf (buffer, sizeof (buffer), format, args);
  va_end (args);
  fprintf (stderr, "%s:%u: %*s%s%s\n",
	   file, line, (int)indent, "", _("Info: "), buffer);
}

/* Send to stderr a string as a warning, and locate warning
   in input file(s).
   Please only use this for when we have some recovery action.
   Please explain in string (which may have '\n's) what recovery was
   done.  */

void
as_tsktsk (const char *format, ...)
{
  va_list args;

  as_show_where ();
  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
  (void) putc ('\n', stderr);
  as_report_context ();
}

/* The common portion of as_warn and as_warn_where.  */

static void
as_warn_internal (const char *file, unsigned int line, char *buffer)
{
  bool context = false;

  ++warning_count;

  if (file == NULL)
    {
      file = as_where_top (&line);
      context = true;
    }

  identify (file);
  if (file)
    {
      if (line != 0)
	fprintf (stderr, "%s:%u: %s%s\n", file, line, _("Warning: "), buffer);
      else
	fprintf (stderr, "%s: %s%s\n", file, _("Warning: "), buffer);
    }
  else
    fprintf (stderr, "%s%s\n", _("Warning: "), buffer);

  if (context)
    as_report_context ();

#ifndef NO_LISTING
  listing_warning (buffer);
#endif
}

/* Send to stderr a string as a warning, and locate warning
   in input file(s).
   Please only use this for when we have some recovery action.
   Please explain in string (which may have '\n's) what recovery was
   done.  */

void
as_warn (const char *format, ...)
{
  va_list args;
  char buffer[2000];

  if (!flag_no_warnings)
    {
      va_start (args, format);
      vsnprintf (buffer, sizeof (buffer), format, args);
      va_end (args);
      as_warn_internal ((char *) NULL, 0, buffer);
    }
}

/* Like as_warn but the file name and line number are passed in.
   Unfortunately, we have to repeat the function in order to handle
   the varargs correctly and portably.  */

void
as_warn_where (const char *file, unsigned int line, const char *format, ...)
{
  va_list args;
  char buffer[2000];

  if (!flag_no_warnings)
    {
      va_start (args, format);
      vsnprintf (buffer, sizeof (buffer), format, args);
      va_end (args);
      as_warn_internal (file, line, buffer);
    }
}

/* The common portion of as_bad and as_bad_where.  */

static void
as_bad_internal (const char *file, unsigned int line, char *buffer)
{
  bool context = false;

  ++error_count;

  if (file == NULL)
    {
      file = as_where_top (&line);
      context = true;
    }

  identify (file);
  if (file)
    {
      if (line != 0)
	fprintf (stderr, "%s:%u: %s%s\n", file, line, _("Error: "), buffer);
      else
	fprintf (stderr, "%s: %s%s\n", file, _("Error: "), buffer);
    }
  else
    fprintf (stderr, "%s%s\n", _("Error: "), buffer);

  if (context)
    as_report_context ();

#ifndef NO_LISTING
  listing_error (buffer);
#endif
}

/* Send to stderr a string as a warning, and locate warning in input
   file(s).  Please use when there is no recovery, but we want to
   continue processing but not produce an object file.
   Please explain in string (which may have '\n's) what recovery was
   done.  */

void
as_bad (const char *format, ...)
{
  va_list args;
  char buffer[2000];

  va_start (args, format);
  vsnprintf (buffer, sizeof (buffer), format, args);
  va_end (args);

  as_bad_internal ((char *) NULL, 0, buffer);
}

/* Like as_bad but the file name and line number are passed in.
   Unfortunately, we have to repeat the function in order to handle
   the varargs correctly and portably.  */

void
as_bad_where (const char *file, unsigned int line, const char *format, ...)
{
  va_list args;
  char buffer[2000];

  va_start (args, format);
  vsnprintf (buffer, sizeof (buffer), format, args);
  va_end (args);

  as_bad_internal (file, line, buffer);
}

/* Send to stderr a string as a fatal message, and print location of
   error in input file(s).
   Please only use this for when we DON'T have some recovery action.
   It xexit()s with a warning status.  */

void
as_fatal (const char *format, ...)
{
  va_list args;

  as_show_where ();
  va_start (args, format);
  fprintf (stderr, _("Fatal error: "));
  vfprintf (stderr, format, args);
  (void) putc ('\n', stderr);
  va_end (args);
  as_report_context ();
  /* Delete the output file, if it exists.  This will prevent make from
     thinking that a file was created and hence does not need rebuilding.  */
  if (out_file_name != NULL)
    unlink_if_ordinary (out_file_name);
  xexit (EXIT_FAILURE);
}

/* Indicate internal constency error.
   Arguments: Filename, line number, optional function name.
   FILENAME may be NULL, which we use for crash-via-signal.  */

void
as_abort (const char *file, int line, const char *fn)
{
  as_show_where ();

  if (!file)
    fprintf (stderr, _("Internal error (%s).\n"), fn ? fn : "unknown");
  else if (fn)
    fprintf (stderr, _("Internal error in %s at %s:%d.\n"), fn, file, line);
  else
    fprintf (stderr, _("Internal error at %s:%d.\n"), file, line);
  as_report_context ();

  fprintf (stderr, _("Please report this bug.\n"));

  xexit (EXIT_FAILURE);
}

/* Handler for fatal signals, such as SIGSEGV. */

static void
signal_crash (int signo)
{
  /* Reset, to prevent unbounded recursion.  */
  signal (signo, SIG_DFL);

  as_abort (NULL, 0, strsignal (signo));
}

/* Register signal handlers, for less abrubt crashes.  */

void
signal_init (void)
{
#ifdef SIGSEGV
  signal (SIGSEGV, signal_crash);
#endif
#ifdef SIGILL
  signal (SIGILL, signal_crash);
#endif
#ifdef SIGBUS
  signal (SIGBUS, signal_crash);
#endif
#ifdef SIGABRT
  signal (SIGABRT, signal_crash);
#endif
#if defined SIGIOT && (!defined SIGABRT || SIGABRT != SIGIOT)
  signal (SIGIOT, signal_crash);
#endif
#ifdef SIGFPE
  signal (SIGFPE, signal_crash);
#endif
}

/* Support routines.  */

#define HEX_MAX_THRESHOLD	1024
#define HEX_MIN_THRESHOLD	-(HEX_MAX_THRESHOLD)

static void
as_internal_value_out_of_range (const char *prefix,
				offsetT val,
				offsetT min,
				offsetT max,
				const char *file,
				unsigned line,
				bool bad)
{
  const char * err;

  if (prefix == NULL)
    prefix = "";

  if (val >= min && val <= max)
    {
      addressT right = max & -max;

      if (max <= 1)
	abort ();

      /* xgettext:c-format  */
      err = _("%s out of domain (%" PRId64
	      " is not a multiple of %" PRId64 ")");

      if (bad)
	as_bad_where (file, line, err, prefix, (int64_t) val, (int64_t) right);
      else
	as_warn_where (file, line, err, prefix, (int64_t) val, (int64_t) right);
    }
  else if (   val < HEX_MAX_THRESHOLD
	   && min < HEX_MAX_THRESHOLD
	   && max < HEX_MAX_THRESHOLD
	   && val > HEX_MIN_THRESHOLD
	   && min > HEX_MIN_THRESHOLD
	   && max > HEX_MIN_THRESHOLD)
    {
      /* xgettext:c-format.  */
      err = _("%s out of range (%" PRId64
	      " is not between %" PRId64 " and %" PRId64 ")");

      if (bad)
	as_bad_where (file, line, err, prefix,
		      (int64_t) val, (int64_t) min, (int64_t) max);
      else
	as_warn_where (file, line, err, prefix,
		       (int64_t) val, (int64_t) min, (int64_t) max);
    }
  else
    {
      /* xgettext:c-format.  */
      err = _("%s out of range (0x%" PRIx64
	      " is not between 0x%" PRIx64 " and 0x%" PRIx64 ")");

      if (bad)
	as_bad_where (file, line, err, prefix,
		      (int64_t) val, (int64_t) min, (int64_t) max);
      else
	as_warn_where (file, line, err, prefix,
		       (int64_t) val, (int64_t) min, (int64_t) max);
    }
}

void
as_warn_value_out_of_range (const char *prefix,
			   offsetT value,
			   offsetT min,
			   offsetT max,
			   const char *file,
			   unsigned line)
{
  as_internal_value_out_of_range (prefix, value, min, max, file, line, false);
}

void
as_bad_value_out_of_range (const char *prefix,
			   offsetT value,
			   offsetT min,
			   offsetT max,
			   const char *file,
			   unsigned line)
{
  as_internal_value_out_of_range (prefix, value, min, max, file, line, true);
}
