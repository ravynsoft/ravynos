/* ldmisc.c
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support.

   This file is part of the GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libiberty.h"
#include "ctf-api.h"
#include "safe-ctype.h"
#include "filenames.h"
#include "demangle.h"
#include <stdarg.h>
#include "ld.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include <ldgram.h>
#include "ldlex.h"
#include "ldmain.h"
#include "ldfile.h"

/*
 %% literal %
 %C clever filename:linenumber with function
 %D like %C, but no function name
 %E current bfd error or errno
 %F error is fatal
 %G like %D, but only function name
 %H like %C but in addition emit section+offset
 %P print program name
 %V hex bfd_vma
 %W hex bfd_vma with 0x with no leading zeros taking up 10 spaces
 %X no object output, fail return
 %d integer, like printf
 %ld long, like printf
 %lu unsigned long, like printf
 %lx unsigned long, like printf
 %p native (host) void* pointer, like printf
 %pA section name from a section
 %pB filename from a bfd
 %pI filename from a lang_input_statement_type
 %pR info about a relent
 %pS print script file and linenumber from etree_type.
 %pT symbol name
 %pU print script file without linenumber from etree_type.
 %s arbitrary string, like printf
 %u integer, like printf
 %v hex bfd_vma, no leading zeros
 %x integer, like printf
*/

void
vfinfo (FILE *fp, const char *fmt, va_list ap, bool is_warning)
{
  bool fatal = false;
  const char *scan;
  int arg_type;
  unsigned int arg_count = 0;
  unsigned int arg_no;
  union vfinfo_args
  {
    int i;
    long l;
    void *p;
    bfd_vma v;
    struct {
      bfd *abfd;
      asection *sec;
      bfd_vma off;
    } reladdr;
    enum
      {
	Bad,
	Int,
	Long,
	Ptr,
	Vma,
	RelAddr
      } type;
  } args[9];

  if (is_warning && config.no_warnings)
    return;
  
  for (arg_no = 0; arg_no < sizeof (args) / sizeof (args[0]); arg_no++)
    args[arg_no].type = Bad;

  arg_count = 0;
  scan = fmt;
  while (*scan != '\0')
    {
      while (*scan != '%' && *scan != '\0')
	scan++;

      if (*scan == '%')
	{
	  scan++;

	  arg_no = arg_count;
	  if (*scan != '0' && ISDIGIT (*scan) && scan[1] == '$')
	    {
	      arg_no = *scan - '1';
	      scan += 2;
	    }

	  arg_type = Bad;
	  switch (*scan++)
	    {
	    case '\0':
	      --scan;
	      break;

	    case 'V':
	    case 'v':
	    case 'W':
	      arg_type = Vma;
	      break;

	    case 's':
	      arg_type = Ptr;
	      break;

	    case 'p':
	      if (*scan == 'A' || *scan == 'B' || *scan == 'I'
		  || *scan == 'R' || *scan == 'S' || *scan ==  'T')
		scan++;
	      arg_type = Ptr;
	      break;

	    case 'C':
	    case 'D':
	    case 'G':
	    case 'H':
	      arg_type = RelAddr;
	      break;

	    case 'd':
	    case 'u':
	    case 'x':
	      arg_type = Int;
	      break;

	    case 'l':
	      if (*scan == 'd' || *scan == 'u' || *scan == 'x')
		{
		  ++scan;
		  arg_type = Long;
		}
	      break;

	    default:
	      break;
	    }
	  if (arg_type != Bad)
	    {
	      if (arg_no >= sizeof (args) / sizeof (args[0]))
		abort ();
	      args[arg_no].type = arg_type;
	      ++arg_count;
	    }
	}
    }

  for (arg_no = 0; arg_no < arg_count; arg_no++)
    {
      switch (args[arg_no].type)
	{
	case Int:
	  args[arg_no].i = va_arg (ap, int);
	  break;
	case Long:
	  args[arg_no].l = va_arg (ap, long);
	  break;
	case Ptr:
	  args[arg_no].p = va_arg (ap, void *);
	  break;
	case Vma:
	  args[arg_no].v = va_arg (ap, bfd_vma);
	  break;
	case RelAddr:
	  args[arg_no].reladdr.abfd = va_arg (ap, bfd *);
	  args[arg_no].reladdr.sec = va_arg (ap, asection *);
	  args[arg_no].reladdr.off = va_arg (ap, bfd_vma);
	  break;
	default:
	  abort ();
	}
    }

  arg_count = 0;
  while (*fmt != '\0')
    {
      const char *str = fmt;
      while (*fmt != '%' && *fmt != '\0')
	fmt++;
      if (fmt != str)
	if (fwrite (str, 1, fmt - str, fp))
	  {
	    /* Ignore.  */
	  }

      if (*fmt == '%')
	{
	  fmt++;

	  arg_no = arg_count;
	  if (*fmt != '0' && ISDIGIT (*fmt) && fmt[1] == '$')
	    {
	      arg_no = *fmt - '1';
	      fmt += 2;
	    }

	  switch (*fmt++)
	    {
	    case '\0':
	      --fmt;
	      /* Fall through.  */

	    case '%':
	      /* literal % */
	      putc ('%', fp);
	      break;

	    case 'X':
	      /* no object output, fail return */
	      config.make_executable = false;
	      break;

	    case 'V':
	      /* hex bfd_vma */
	      {
		char buf[32];
		bfd_vma value;

		value = args[arg_no].v;
		++arg_count;
		bfd_sprintf_vma (link_info.output_bfd, buf, value);
		fprintf (fp, "%s", buf);
	      }
	      break;

	    case 'v':
	      /* hex bfd_vma, no leading zeros */
	      {
		uint64_t value = args[arg_no].v;
		++arg_count;
		fprintf (fp, "%" PRIx64, value);
	      }
	      break;

	    case 'W':
	      /* hex bfd_vma with 0x with no leading zeroes taking up
		 10 spaces (including the 0x).  */
	      {
		char buf[32];
		uint64_t value;

		value = args[arg_no].v;
		++arg_count;
		sprintf (buf, "0x%" PRIx64, value);
		fprintf (fp, "%10s", buf);
	      }
	      break;

	    case 'F':
	      /* Error is fatal.  */
	      fatal = true;
	      break;

	    case 'P':
	      /* Print program name.  */
	      fprintf (fp, "%s", program_name);
	      break;

	    case 'E':
	      /* current bfd error or errno */
	      fprintf (fp, "%s", bfd_errmsg (bfd_get_error ()));
	      break;

	    case 'C':
	    case 'D':
	    case 'G':
	    case 'H':
	      /* Clever filename:linenumber with function name if possible.
		 The arguments are a BFD, a section, and an offset.  */
	      {
		static bfd *last_bfd;
		static char *last_file;
		static char *last_function;
		bfd *abfd;
		asection *section;
		bfd_vma offset;
		asymbol **asymbols = NULL;
		const char *filename;
		const char *functionname;
		unsigned int linenumber;
		bool discard_last;
		bool done;
		bfd_error_type last_bfd_error = bfd_get_error ();

		abfd = args[arg_no].reladdr.abfd;
		section = args[arg_no].reladdr.sec;
		offset = args[arg_no].reladdr.off;
		++arg_count;

		if (abfd != NULL)
		  {
		    if (!bfd_generic_link_read_symbols (abfd))
		      einfo (_("%F%P: %pB: could not read symbols: %E\n"), abfd);

		    asymbols = bfd_get_outsymbols (abfd);
		  }

		/* The GNU Coding Standard requires that error messages
		   be of the form:

		     source-file-name:lineno: message

		   We do not always have a line number available so if
		   we cannot find them we print out the section name and
		   offset instead.  */
		discard_last = true;
		if (abfd != NULL
		    && bfd_find_nearest_line (abfd, section, asymbols, offset,
					      &filename, &functionname,
					      &linenumber))
		  {
		    if (functionname != NULL
			&& (fmt[-1] == 'C' || fmt[-1] == 'H'))
		      {
			/* Detect the case where we are printing out a
			   message for the same function as the last
			   call to vinfo ("%C").  In this situation do
			   not print out the ABFD filename or the
			   function name again.  Note - we do still
			   print out the source filename, as this will
			   allow programs that parse the linker's output
			   (eg emacs) to correctly locate multiple
			   errors in the same source file.  */
			if (last_bfd == NULL
			    || last_function == NULL
			    || last_bfd != abfd
			    || (last_file == NULL) != (filename == NULL)
			    || (filename != NULL
				&& filename_cmp (last_file, filename) != 0)
			    || strcmp (last_function, functionname) != 0)
			  {
			    lfinfo (fp, _("%pB: in function `%pT':\n"),
				    abfd, functionname);

			    last_bfd = abfd;
			    free (last_file);
			    last_file = NULL;
			    if (filename)
			      last_file = xstrdup (filename);
			    free (last_function);
			    last_function = xstrdup (functionname);
			  }
			discard_last = false;
		      }
		    else
		      lfinfo (fp, "%pB:", abfd);

		    if (filename != NULL)
		      fprintf (fp, "%s:", filename);

		    done = fmt[-1] != 'H';
		    if (functionname != NULL && fmt[-1] == 'G')
		      lfinfo (fp, "%pT", functionname);
		    else if (filename != NULL && linenumber != 0)
		      fprintf (fp, "%u%s", linenumber, done ? "" : ":");
		    else
		      done = false;
		  }
		else
		  {
		    lfinfo (fp, "%pB:", abfd);
		    done = false;
		  }
		if (!done)
		  lfinfo (fp, "(%pA+0x%v)", section, offset);
		bfd_set_error (last_bfd_error);

		if (discard_last)
		  {
		    last_bfd = NULL;
		    free (last_file);
		    last_file = NULL;
		    free (last_function);
		    last_function = NULL;
		  }
	      }
	      break;

	    case 'p':
	      if (*fmt == 'A')
		{
		  /* section name from a section */
		  asection *sec;
		  bfd *abfd;

		  fmt++;
		  sec = (asection *) args[arg_no].p;
		  ++arg_count;
		  fprintf (fp, "%s", sec->name);
		  abfd = sec->owner;
		  if (abfd != NULL)
		    {
		      const char *group = bfd_group_name (abfd, sec);
		      if (group != NULL)
			fprintf (fp, "[%s]", group);
		    }
		}
	      else if (*fmt == 'B')
		{
		  /* filename from a bfd */
		  bfd *abfd = (bfd *) args[arg_no].p;

		  fmt++;
		  ++arg_count;
		  if (abfd == NULL)
		    fprintf (fp, "%s generated", program_name);
		  else if (abfd->my_archive != NULL
			   && !bfd_is_thin_archive (abfd->my_archive))
		    fprintf (fp, "%s(%s)",
			     bfd_get_filename (abfd->my_archive),
			     bfd_get_filename (abfd));
		  else
		    fprintf (fp, "%s", bfd_get_filename (abfd));
		}
	      else if (*fmt == 'I')
		{
		  /* filename from a lang_input_statement_type */
		  lang_input_statement_type *i;

		  fmt++;
		  i = (lang_input_statement_type *) args[arg_no].p;
		  ++arg_count;
		  if (i->the_bfd != NULL
		      && i->the_bfd->my_archive != NULL
		      && !bfd_is_thin_archive (i->the_bfd->my_archive))
		    fprintf (fp, "(%s)%s",
			     bfd_get_filename (i->the_bfd->my_archive),
			     i->local_sym_name);
		  else
		    fprintf (fp, "%s", i->filename);
		}
	      else if (*fmt == 'R')
		{
		  /* Print all that's interesting about a relent.  */
		  arelent *relent = (arelent *) args[arg_no].p;

		  fmt++;
		  ++arg_count;
		  lfinfo (fp, "%s+0x%v (type %s)",
			  (*(relent->sym_ptr_ptr))->name,
			  relent->addend,
			  relent->howto->name);
		}
	      else if (*fmt == 'S' || *fmt == 'U')
		{
		  /* Print script file and perhaps the associated linenumber.  */
		  etree_type node;
		  etree_type *tp = (etree_type *) args[arg_no].p;

		  fmt++;
		  ++arg_count;
		  if (tp == NULL)
		    {
		      tp = &node;
		      tp->type.filename = ldlex_filename ();
		      tp->type.lineno = lineno;
		    }
		  if (tp->type.filename != NULL && fmt[-1] == 'S')
		    fprintf (fp, "%s:%u", tp->type.filename, tp->type.lineno);
		  else if (tp->type.filename != NULL && fmt[-1] == 'U')
		    fprintf (fp, "%s", tp->type.filename);
		}
	      else if (*fmt == 'T')
		{
		  /* Symbol name.  */
		  const char *name = (const char *) args[arg_no].p;

		  fmt++;
		  ++arg_count;
		  if (name == NULL || *name == 0)
		    {
		      fprintf (fp, _("no symbol"));
		      break;
		    }
		  else if (demangling)
		    {
		      char *demangled;

		      demangled = bfd_demangle (link_info.output_bfd, name,
						DMGL_ANSI | DMGL_PARAMS);
		      if (demangled != NULL)
			{
			  fprintf (fp, "%s", demangled);
			  free (demangled);
			  break;
			}
		    }
		  fprintf (fp, "%s", name);
		}
	      else
		{
		  /* native (host) void* pointer, like printf */
		  fprintf (fp, "%p", args[arg_no].p);
		  ++arg_count;
		}
	      break;

	    case 's':
	      /* arbitrary string, like printf */
	      fprintf (fp, "%s", (char *) args[arg_no].p);
	      ++arg_count;
	      break;

	    case 'd':
	      /* integer, like printf */
	      fprintf (fp, "%d", args[arg_no].i);
	      ++arg_count;
	      break;

	    case 'u':
	      /* unsigned integer, like printf */
	      fprintf (fp, "%u", args[arg_no].i);
	      ++arg_count;
	      break;

	    case 'x':
	      /* unsigned integer, like printf */
	      fprintf (fp, "%x", args[arg_no].i);
	      ++arg_count;
	      break;

	    case 'l':
	      if (*fmt == 'd')
		{
		  fprintf (fp, "%ld", args[arg_no].l);
		  ++arg_count;
		  ++fmt;
		  break;
		}
	      else if (*fmt == 'u')
		{
		  fprintf (fp, "%lu", args[arg_no].l);
		  ++arg_count;
		  ++fmt;
		  break;
		}
	      else if (*fmt == 'x')
		{
		  fprintf (fp, "%lx", args[arg_no].l);
		  ++arg_count;
		  ++fmt;
		  break;
		}
	      /* Fallthru */

	    default:
	      fprintf (fp, "%%%c", fmt[-1]);
	      break;
	    }
	}
    }

  if (is_warning && config.fatal_warnings)
    config.make_executable = false;

  if (fatal)
    xexit (1);
}

/* Format info message and print on stdout.  */

/* (You would think this should be called just "info", but then you
   would be hosed by LynxOS, which defines that name in its libc.)  */

void
info_msg (const char *fmt, ...)
{
  va_list arg;

  va_start (arg, fmt);
  vfinfo (stdout, fmt, arg, false);
  va_end (arg);
}

/* ('e' for error.) Format info message and print on stderr.  */

void
einfo (const char *fmt, ...)
{
  va_list arg;

  fflush (stdout);
  va_start (arg, fmt);
  vfinfo (stderr, fmt, arg, true);
  va_end (arg);
  fflush (stderr);
}

void
info_assert (const char *file, unsigned int line)
{
  einfo (_("%F%P: internal error %s %d\n"), file, line);
}

/* ('m' for map) Format info message and print on map.  */

void
minfo (const char *fmt, ...)
{
  if (config.map_file != NULL)
    {
      va_list arg;

      va_start (arg, fmt);
      if (fmt[0] == '%' && fmt[1] == '!' && fmt[2] == 0)
	{
	  /* Stash info about --as-needed shared libraries.  Print
	     later so they don't appear intermingled with archive
	     library info.  */
	  struct asneeded_minfo *m = xmalloc (sizeof *m);

	  m->next = NULL;
	  m->soname = va_arg (arg, const char *);
	  m->ref = va_arg (arg, bfd *);
	  m->name = va_arg (arg, const char *);
	  *asneeded_list_tail = m;
	  asneeded_list_tail = &m->next;
	}
      else
	vfinfo (config.map_file, fmt, arg, false);
      va_end (arg);
    }
}

void
lfinfo (FILE *file, const char *fmt, ...)
{
  va_list arg;

  va_start (arg, fmt);
  vfinfo (file, fmt, arg, false);
  va_end (arg);
}

/* Functions to print the link map.  */

void
print_spaces (int count)
{
  fprintf (config.map_file, "%*s", count, "");
}

void
print_nl (void)
{
  fprintf (config.map_file, "\n");
}

/* A more or less friendly abort message.  In ld.h abort is defined to
   call this function.  */

void
ld_abort (const char *file, int line, const char *fn)
{
  if (fn != NULL)
    einfo (_("%P: internal error: aborting at %s:%d in %s\n"),
	   file, line, fn);
  else
    einfo (_("%P: internal error: aborting at %s:%d\n"),
	   file, line);
  einfo (_("%F%P: please report this bug\n"));
  xexit (1);
}
