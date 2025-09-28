/* bucomm.c -- Bin Utils COMmon code.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* We might put this in a library someday so it could be dynamically
   loaded, but for now it's not necessary.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "filenames.h"
#include <time.h>
#include <assert.h>
#include "bucomm.h"

/* Error reporting.  */

char *program_name;

void
bfd_nonfatal (const char *string)
{
  const char *errmsg;
  enum bfd_error err = bfd_get_error ();

  if (err == bfd_error_no_error)
    errmsg = _("cause of error unknown");
  else
    errmsg = bfd_errmsg (err);
  fflush (stdout);
  if (string)
    fprintf (stderr, "%s: %s: %s\n", program_name, string, errmsg);
  else
    fprintf (stderr, "%s: %s\n", program_name, errmsg);
}

/* Issue a non fatal error message.  FILENAME, or if NULL then BFD,
   are used to indicate the problematic file.  SECTION, if non NULL,
   is used to provide a section name.  If FORMAT is non-null, then it
   is used to print additional information via vfprintf.  Finally the
   bfd error message is printed.  In summary, error messages are of
   one of the following forms:

   PROGRAM: file: bfd-error-message
   PROGRAM: file[section]: bfd-error-message
   PROGRAM: file: printf-message: bfd-error-message
   PROGRAM: file[section]: printf-message: bfd-error-message.  */

void
bfd_nonfatal_message (const char *filename,
		      const bfd *abfd,
		      const asection *section,
		      const char *format, ...)
{
  const char *errmsg;
  const char *section_name;
  enum bfd_error err = bfd_get_error ();

  if (err == bfd_error_no_error)
    errmsg = _("cause of error unknown");
  else
    errmsg = bfd_errmsg (err);
  fflush (stdout);
  section_name = NULL;
  fprintf (stderr, "%s", program_name);

  if (abfd)
    {
      if (!filename)
	filename = bfd_get_archive_filename (abfd);
      if (section)
	section_name = bfd_section_name (section);
    }
  if (section_name)
    fprintf (stderr, ": %s[%s]", filename, section_name);
  else
    fprintf (stderr, ": %s", filename);

  if (format)
    {
      va_list args;
      va_start (args, format);
      fprintf (stderr, ": ");
      vfprintf (stderr, format, args);
      va_end (args);
    }
  fprintf (stderr, ": %s\n", errmsg);
}

void
bfd_fatal (const char *string)
{
  bfd_nonfatal (string);
  xexit (1);
}

void
report (const char * format, va_list args)
{
  fflush (stdout);
  fprintf (stderr, "%s: ", program_name);
  vfprintf (stderr, format, args);
  putc ('\n', stderr);
}

void
fatal (const char *format, ...)
{
  va_list args;

  va_start (args, format);

  report (format, args);
  va_end (args);
  xexit (1);
}

void
non_fatal (const char *format, ...)
{
  va_list args;

  va_start (args, format);

  report (format, args);
  va_end (args);
}

/* Like xmalloc except that ABFD's objalloc memory is returned.
   Use objalloc_free_block to free this memory and all more recently
   allocated, or more usually, leave it to bfd_close to free.  */

void *
bfd_xalloc (bfd *abfd, size_t size)
{
  void *ret = bfd_alloc (abfd, size);
  if (ret == NULL)
    bfd_fatal (NULL);
  return ret;
}

/* Set the default BFD target based on the configured target.  Doing
   this permits the binutils to be configured for a particular target,
   and linked against a shared BFD library which was configured for a
   different target.  */

void
set_default_bfd_target (void)
{
  /* The macro TARGET is defined by Makefile.  */
  const char *target = TARGET;

  if (! bfd_set_default_target (target))
    fatal (_("can't set BFD default target to `%s': %s"),
	   target, bfd_errmsg (bfd_get_error ()));
}

/* After a FALSE return from bfd_check_format_matches with
   bfd_get_error () == bfd_error_file_ambiguously_recognized, print
   the possible matching targets and free the list of targets.  */

void
list_matching_formats (char **matching)
{
  fflush (stdout);
  fprintf (stderr, _("%s: Matching formats:"), program_name);
  char **p = matching;
  while (*p)
    fprintf (stderr, " %s", *p++);
  free (matching);
  fputc ('\n', stderr);
}

/* List the supported targets.  */

void
list_supported_targets (const char *name, FILE *f)
{
  int t;
  const char **targ_names;

  if (name == NULL)
    fprintf (f, _("Supported targets:"));
  else
    fprintf (f, _("%s: supported targets:"), name);

  targ_names = bfd_target_list ();
  for (t = 0; targ_names[t] != NULL; t++)
    fprintf (f, " %s", targ_names[t]);
  fprintf (f, "\n");
  free (targ_names);
}

/* List the supported architectures.  */

void
list_supported_architectures (const char *name, FILE *f)
{
  const char ** arch;
  const char ** arches;

  if (name == NULL)
    fprintf (f, _("Supported architectures:"));
  else
    fprintf (f, _("%s: supported architectures:"), name);

  for (arch = arches = bfd_arch_list (); *arch; arch++)
    fprintf (f, " %s", *arch);
  fprintf (f, "\n");
  free (arches);
}

static const char *
endian_string (enum bfd_endian endian)
{
  switch (endian)
    {
    case BFD_ENDIAN_BIG: return _("big endian");
    case BFD_ENDIAN_LITTLE: return _("little endian");
    default: return _("endianness unknown");
    }
}

/* Data passed to do_display_target and other target iterators.  */

struct display_target {
  /* Temp file.  */
  char *filename;
  /* Return status.  */
  int error;
  /* Number of targets.  */
  int count;
  /* Size of info in bytes.  */
  size_t alloc;
  /* Per-target info.  */
  struct {
    /* Target name.  */
    const char *name;
    /* Non-zero if target/arch combination supported.  */
    unsigned char arch[bfd_arch_last - bfd_arch_obscure - 1];
  } *info;
};

/* List the targets that BFD is configured to support, each followed
   by its endianness and the architectures it supports.  Also build
   info about target/archs.  */

static int
do_display_target (const bfd_target *targ, void *data)
{
  struct display_target *param = (struct display_target *) data;
  bfd *abfd;
  size_t amt;

  param->count += 1;
  amt = param->count * sizeof (*param->info);
  if (param->alloc < amt)
    {
      size_t size = ((param->count < 64 ? 64 : param->count)
		     * sizeof (*param->info) * 2);
      param->info = xrealloc (param->info, size);
      memset ((char *) param->info + param->alloc, 0, size - param->alloc);
      param->alloc = size;
    }
  param->info[param->count - 1].name = targ->name;

  printf (_("%s\n (header %s, data %s)\n"), targ->name,
	  endian_string (targ->header_byteorder),
	  endian_string (targ->byteorder));

  abfd = bfd_openw (param->filename, targ->name);
  if (abfd == NULL)
    {
      bfd_nonfatal (param->filename);
      param->error = 1;
    }
  else if (!bfd_set_format (abfd, bfd_object))
    {
      if (bfd_get_error () != bfd_error_invalid_operation)
	{
	  bfd_nonfatal (targ->name);
	  param->error = 1;
	}
    }
  else
    {
      enum bfd_architecture a;

      for (a = bfd_arch_obscure + 1; a < bfd_arch_last; a++)
	if (bfd_set_arch_mach (abfd, a, 0))
	  {
	    printf ("  %s\n", bfd_printable_arch_mach (a, 0));
	    param->info[param->count - 1].arch[a - bfd_arch_obscure - 1] = 1;
	  }
    }
  if (abfd != NULL)
    bfd_close_all_done (abfd);

  return param->error;
}

static void
display_target_list (struct display_target *arg)
{
  arg->filename = make_temp_file (NULL);
  arg->error = 0;
  arg->count = 0;
  arg->alloc = 0;
  arg->info = NULL;

  bfd_iterate_over_targets (do_display_target, arg);

  unlink (arg->filename);
  free (arg->filename);
}

/* Calculate how many targets we can print across the page.  */

static int
do_info_size (int targ, int width, const struct display_target *arg)
{
  while (targ < arg->count)
    {
      width -= strlen (arg->info[targ].name) + 1;
      if (width < 0)
	return targ;
      ++targ;
    }
  return targ;
}

/* Print header of target names.  */

static void
do_info_header (int targ, int stop_targ, const struct display_target *arg)
{
  while (targ != stop_targ)
    printf ("%s ", arg->info[targ++].name);
}

/* Print a table row.  */

static void
do_info_row (int targ, int stop_targ, enum bfd_architecture a,
	     const struct display_target *arg)
{
  while (targ != stop_targ)
    {
      if (arg->info[targ].arch[a - bfd_arch_obscure - 1])
	fputs (arg->info[targ].name, stdout);
      else
	{
	  int l = strlen (arg->info[targ].name);
	  while (l--)
	    putchar ('-');
	}
      ++targ;
      if (targ != stop_targ)
	putchar (' ');
    }
}

/* Print tables of all the target-architecture combinations that
   BFD has been configured to support.  */

static void
display_target_tables (const struct display_target *arg)
{
  const char *columns;
  int width, start_targ, stop_targ;
  enum bfd_architecture arch;
  int longest_arch = 0;

  for (arch = bfd_arch_obscure + 1; arch < bfd_arch_last; arch++)
    {
      const char *s = bfd_printable_arch_mach (arch, 0);
      int len = strlen (s);
      if (len > longest_arch)
	longest_arch = len;
    }

  width = 0;
  columns = getenv ("COLUMNS");
  if (columns != NULL)
    width = atoi (columns);
  if (width == 0)
    width = 80;

  for (start_targ = 0; start_targ < arg->count; start_targ = stop_targ)
    {
      stop_targ = do_info_size (start_targ, width - longest_arch - 1, arg);

      printf ("\n%*s", longest_arch + 1, " ");
      do_info_header (start_targ, stop_targ, arg);
      putchar ('\n');

      for (arch = bfd_arch_obscure + 1; arch < bfd_arch_last; arch++)
	{
	  if (strcmp (bfd_printable_arch_mach (arch, 0), "UNKNOWN!") != 0)
	    {
	      printf ("%*s ", longest_arch,
		      bfd_printable_arch_mach (arch, 0));

	      do_info_row (start_targ, stop_targ, arch, arg);
	      putchar ('\n');
	    }
	}
    }
}

int
display_info (void)
{
  struct display_target arg;

  printf (_("BFD header file version %s\n"), BFD_VERSION_STRING);

  display_target_list (&arg);
  if (!arg.error)
    display_target_tables (&arg);

  return arg.error;
}

/* Display the archive header for an element as if it were an ls -l listing:

   Mode       User\tGroup\tSize\tDate               Name */

void
print_arelt_descr (FILE *file, bfd *abfd, bool verbose, bool offsets)
{
  struct stat buf;

  if (verbose)
    {
      if (bfd_stat_arch_elt (abfd, &buf) == 0)
	{
	  char modebuf[11];
	  char timebuf[40];
	  time_t when = buf.st_mtime;
	  const char *ctime_result = (const char *) ctime (&when);

	  /* PR binutils/17605: Check for corrupt time values.  */
	  if (ctime_result == NULL)
	    sprintf (timebuf, _("<time data corrupt>"));
	  else
	    /* POSIX format:  skip weekday and seconds from ctime output.  */
	    sprintf (timebuf, "%.12s %.4s", ctime_result + 4, ctime_result + 20);

	  mode_string (buf.st_mode, modebuf);
	  modebuf[10] = '\0';
	  /* POSIX 1003.2/D11 says to skip first character (entry type).  */
	  fprintf (file, "%s %ld/%ld %6" PRIu64 " %s ", modebuf + 1,
		   (long) buf.st_uid, (long) buf.st_gid,
		   (uint64_t) buf.st_size, timebuf);
	}
    }

  fprintf (file, "%s", bfd_get_filename (abfd));

  if (offsets)
    {
      if (bfd_is_thin_archive (abfd) && abfd->proxy_origin)
        fprintf (file, " 0x%lx", (unsigned long) abfd->proxy_origin);
      else if (!bfd_is_thin_archive (abfd) && abfd->origin)
        fprintf (file, " 0x%lx", (unsigned long) abfd->origin);
    }

  fprintf (file, "\n");
}

/* Return a path for a new temporary file in the same directory
   as file PATH.  */

static char *
template_in_dir (const char *path)
{
#define template "stXXXXXX"
  const char *slash = strrchr (path, '/');
  char *tmpname;
  size_t len;

#ifdef HAVE_DOS_BASED_FILE_SYSTEM
  {
    /* We could have foo/bar\\baz, or foo\\bar, or d:bar.  */
    char *bslash = strrchr (path, '\\');

    if (slash == NULL || (bslash != NULL && bslash > slash))
      slash = bslash;
    if (slash == NULL && path[0] != '\0' && path[1] == ':')
      slash = path + 1;
  }
#endif

  if (slash != (char *) NULL)
    {
      len = slash - path;
      tmpname = (char *) xmalloc (len + sizeof (template) + 2);
      memcpy (tmpname, path, len);

#ifdef HAVE_DOS_BASED_FILE_SYSTEM
      /* If tmpname is "X:", appending a slash will make it a root
	 directory on drive X, which is NOT the same as the current
	 directory on drive X.  */
      if (len == 2 && tmpname[1] == ':')
	tmpname[len++] = '.';
#endif
      tmpname[len++] = '/';
    }
  else
    {
      tmpname = (char *) xmalloc (sizeof (template));
      len = 0;
    }

  memcpy (tmpname + len, template, sizeof (template));
  return tmpname;
#undef template
}

/* Return the name of a created temporary file in the same directory
   as FILENAME.  */

char *
make_tempname (const char *filename, int *ofd)
{
  char *tmpname = template_in_dir (filename);
  int fd;

#ifdef HAVE_MKSTEMP
  fd = mkstemp (tmpname);
#else
  tmpname = mktemp (tmpname);
  if (tmpname == NULL)
    fd = -1;
  else
    fd = open (tmpname, O_RDWR | O_CREAT | O_EXCL, 0600);
#endif
  if (fd == -1)
    {
      free (tmpname);
      bfd_set_error (bfd_error_system_call);
      return NULL;
    }
  *ofd = fd;
  return tmpname;
}

/* Return the name of a created temporary directory inside the
   directory containing FILENAME.  */

char *
make_tempdir (const char *filename)
{
  char *tmpname = template_in_dir (filename);
  char *ret;

#ifdef HAVE_MKDTEMP
  ret = mkdtemp (tmpname);
#else
  ret = mktemp (tmpname);
#if defined (_WIN32) && !defined (__CYGWIN32__)
  if (mkdir (tmpname) != 0)
    ret = NULL;
#else
  if (mkdir (tmpname, 0700) != 0)
    ret = NULL;
#endif
#endif
  if (ret == NULL)
    {
      free (tmpname);
      bfd_set_error (bfd_error_system_call);
    }
  return ret;
}

/* Parse a string into a VMA, with a fatal error if it can't be
   parsed.  */

bfd_vma
parse_vma (const char *s, const char *arg)
{
  bfd_vma ret;
  const char *end;

  ret = bfd_scan_vma (s, &end, 0);

  if (*end != '\0')
    fatal (_("%s: bad number: %s"), arg, s);

  return ret;
}

/* Returns the size of the named file.  If the file does not
   exist, or if it is not a real file, then a suitable non-fatal
   error message is printed and (off_t) -1 is returned.  */

off_t
get_file_size (const char * file_name)
{
  struct stat statbuf;

  if (file_name == NULL)
    return (off_t) -1;

  if (stat (file_name, &statbuf) < 0)
    {
      if (errno == ENOENT)
	non_fatal (_("'%s': No such file"), file_name);
      else
	non_fatal (_("Warning: could not locate '%s'.  reason: %s"),
		   file_name, strerror (errno));
    }
  else if (S_ISDIR (statbuf.st_mode))
    non_fatal (_("Warning: '%s' is a directory"), file_name);
  else if (! S_ISREG (statbuf.st_mode))
    non_fatal (_("Warning: '%s' is not an ordinary file"), file_name);
  else if (statbuf.st_size < 0)
    non_fatal (_("Warning: '%s' has negative size, probably it is too large"),
               file_name);
#if defined (_WIN32) && !defined (__CYGWIN__)
  else if (statbuf.st_size == 0)
    {
      /* MS-Windows 'stat' reports the null device as a regular file;
	 fix that.  */
      int fd = open (file_name, O_RDONLY | O_BINARY);
      if (isatty (fd))
	{
	  close (fd);
	  non_fatal (_("Warning: '%s' is not an ordinary file"),
		     /* libtool wants to see /dev/null in the output.  */
		     strcasecmp (file_name, "nul") ? file_name : "/dev/null");
	}
    }
#endif
  else
    return statbuf.st_size;

  return (off_t) -1;
}

/* Return the filename in a static buffer.  */

const char *
bfd_get_archive_filename (const bfd *abfd)
{
  static size_t curr = 0;
  static char *buf;
  size_t needed;

  assert (abfd != NULL);

  if (abfd->my_archive == NULL
      || bfd_is_thin_archive (abfd->my_archive))
    return bfd_get_filename (abfd);

  needed = (strlen (bfd_get_filename (abfd->my_archive))
	    + strlen (bfd_get_filename (abfd)) + 3);
  if (needed > curr)
    {
      if (curr)
	free (buf);
      curr = needed + (needed >> 1);
      buf = (char *) xmalloc (curr);
    }
  sprintf (buf, "%s(%s)", bfd_get_filename (abfd->my_archive),
	   bfd_get_filename (abfd));
  return buf;
}

/* Returns TRUE iff PATHNAME, a filename of an archive member,
   is valid for writing.  For security reasons absolute paths
   and paths containing /../ are not allowed.  See PR 17533.  */

bool
is_valid_archive_path (char const * pathname)
{
  const char * n = pathname;

  if (IS_ABSOLUTE_PATH (n))
    return false;

  while (*n)
    {
      if (*n == '.' && *++n == '.' && ( ! *++n || IS_DIR_SEPARATOR (*n)))
	return false;

      while (*n && ! IS_DIR_SEPARATOR (*n))
	n++;
      while (IS_DIR_SEPARATOR (*n))
	n++;
    }

  return true;
}
