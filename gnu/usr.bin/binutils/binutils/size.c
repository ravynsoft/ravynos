/* size.c -- report size of various sections of an executable file.
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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Extensions/incompatibilities:
   o - BSD output has filenames at the end.
   o - BSD output can appear in different radicies.
   o - SysV output has less redundant whitespace.  Filename comes at end.
   o - SysV output doesn't show VMA which is always the same as the PMA.
   o - We also handle core files.
   o - We also handle archives.
   If you write shell scripts which manipulate this info then you may be
   out of luck; there's no --compatibility or --pedantic option.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "getopt.h"
#include "bucomm.h"

#ifndef BSD_DEFAULT
#define BSD_DEFAULT 1
#endif

/* Program options.  */

static enum
  {
    decimal, octal, hex
  }
radix = decimal;

/* Select the desired output format.  */
enum output_format
  {
   FORMAT_BERKLEY,
   FORMAT_SYSV,
   FORMAT_GNU
  };
static enum output_format selected_output_format =
#if BSD_DEFAULT
  FORMAT_BERKLEY
#else
  FORMAT_SYSV
#endif
  ;

static int show_version = 0;
static int show_help = 0;
static int show_totals = 0;
static int show_common = 0;

static bfd_size_type common_size;
static bfd_size_type total_bsssize;
static bfd_size_type total_datasize;
static bfd_size_type total_textsize;

/* Program exit status.  */
static int return_code = 0;

static char *target = NULL;

/* Forward declarations.  */

static void display_file (char *);
static void rprint_number (int, bfd_size_type);
static void print_sizes (bfd * file);

static void
usage (FILE *stream, int status)
{
  fprintf (stream, _("Usage: %s [option(s)] [file(s)]\n"), program_name);
  fprintf (stream, _(" Displays the sizes of sections inside binary files\n"));
  fprintf (stream, _(" If no input file(s) are specified, a.out is assumed\n"));
  fprintf (stream, _(" The options are:\n\
  -A|-B|-G  --format={sysv|berkeley|gnu}  Select output style (default is %s)\n\
  -o|-d|-x  --radix={8|10|16}         Display numbers in octal, decimal or hex\n\
  -t        --totals                  Display the total sizes (Berkeley only)\n\
  -f                                  Ignored.\n\
            --common                  Display total size for *COM* syms\n\
            --target=<bfdname>        Set the binary file format\n\
            @<file>                   Read options from <file>\n\
  -h|-H|-?  --help                    Display this information\n\
  -v|-V     --version                 Display the program's version\n\
\n"),
#if BSD_DEFAULT
  "berkeley"
#else
  "sysv"
#endif
);
  list_supported_targets (program_name, stream);
  if (REPORT_BUGS_TO[0] && status == 0)
    fprintf (stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
  exit (status);
}

#define OPTION_FORMAT (200)
#define OPTION_RADIX (OPTION_FORMAT + 1)
#define OPTION_TARGET (OPTION_RADIX + 1)

static struct option long_options[] =
{
  {"common", no_argument, &show_common, 1},
  {"format", required_argument, 0, OPTION_FORMAT},
  {"radix", required_argument, 0, OPTION_RADIX},
  {"target", required_argument, 0, OPTION_TARGET},
  {"totals", no_argument, &show_totals, 1},
  {"version", no_argument, &show_version, 1},
  {"help", no_argument, &show_help, 1},
  {0, no_argument, 0, 0}
};

int main (int, char **);

int
main (int argc, char **argv)
{
  int temp;
  int c;

#ifdef HAVE_LC_MESSAGES
  setlocale (LC_MESSAGES, "");
#endif
  setlocale (LC_CTYPE, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = *argv;
  xmalloc_set_program_name (program_name);
  bfd_set_error_program_name (program_name);

  expandargv (&argc, &argv);

  if (bfd_init () != BFD_INIT_MAGIC)
    fatal (_("fatal error: libbfd ABI mismatch"));
  set_default_bfd_target ();

  while ((c = getopt_long (argc, argv, "ABGHhVvdfotx", long_options,
			   (int *) 0)) != EOF)
    switch (c)
      {
      case OPTION_FORMAT:
	switch (*optarg)
	  {
	  case 'B':
	  case 'b':
	    selected_output_format = FORMAT_BERKLEY;
	    break;
	  case 'S':
	  case 's':
	    selected_output_format = FORMAT_SYSV;
	    break;
	  case 'G':
	  case 'g':
	    selected_output_format = FORMAT_GNU;
	    break;
	  default:
	    non_fatal (_("invalid argument to --format: %s"), optarg);
	    usage (stderr, 1);
	  }
	break;

      case OPTION_TARGET:
	target = optarg;
	break;

      case OPTION_RADIX:
#ifdef ANSI_LIBRARIES
	temp = strtol (optarg, NULL, 10);
#else
	temp = atol (optarg);
#endif
	switch (temp)
	  {
	  case 10:
	    radix = decimal;
	    break;
	  case 8:
	    radix = octal;
	    break;
	  case 16:
	    radix = hex;
	    break;
	  default:
	    non_fatal (_("Invalid radix: %s\n"), optarg);
	    usage (stderr, 1);
	  }
	break;

      case 'A':
	selected_output_format = FORMAT_SYSV;
	break;
      case 'B':
	selected_output_format = FORMAT_BERKLEY;
	break;
      case 'G':
	selected_output_format = FORMAT_GNU;
	break;
      case 'v':
      case 'V':
	show_version = 1;
	break;
      case 'd':
	radix = decimal;
	break;
      case 'x':
	radix = hex;
	break;
      case 'o':
	radix = octal;
	break;
      case 't':
	show_totals = 1;
	break;
      case 'f': /* FIXME : For sysv68, `-f' means `full format', i.e.
		   `[fname:] M(.text) + N(.data) + O(.bss) + P(.comment) = Q'
		   where `fname: ' appears only if there are >= 2 input files,
		   and M, N, O, P, Q are expressed in decimal by default,
		   hexa or octal if requested by `-x' or `-o'.
		   Just to make things interesting, Solaris also accepts -f,
		   which prints out the size of each allocatable section, the
		   name of the section, and the total of the section sizes.  */
		/* For the moment, accept `-f' silently, and ignore it.  */
	break;
      case 0:
	break;
      case 'h':
      case 'H':
      case '?':
	usage (stderr, 1);
      }

  if (show_version)
    print_version ("size");
  if (show_help)
    usage (stdout, 0);

  if (optind == argc)
    display_file ("a.out");
  else
    for (; optind < argc;)
      display_file (argv[optind++]);

  if (show_totals && (selected_output_format == FORMAT_BERKLEY
		      || selected_output_format == FORMAT_GNU))
    {
      bfd_size_type total = total_textsize + total_datasize + total_bsssize;
      int col_width = (selected_output_format == FORMAT_BERKLEY) ? 7 : 10;
      char sep_char = (selected_output_format == FORMAT_BERKLEY) ? '\t' : ' ';

      rprint_number (col_width, total_textsize);
      putchar(sep_char);
      rprint_number (col_width, total_datasize);
      putchar(sep_char);
      rprint_number (col_width, total_bsssize);
      putchar(sep_char);
      if (selected_output_format == FORMAT_BERKLEY)
	printf (((radix == octal) ? "%7lo\t%7lx" : "%7lu\t%7lx"),
		(unsigned long) total, (unsigned long) total);
      else
	rprint_number (col_width, total);
      putchar(sep_char);
      fputs ("(TOTALS)\n", stdout);
    }

  return return_code;
}

/* Total size required for common symbols in ABFD.  */

static void
calculate_common_size (bfd *abfd)
{
  asymbol **syms = NULL;
  long storage, symcount;

  common_size = 0;
  if ((bfd_get_file_flags (abfd) & (EXEC_P | DYNAMIC | HAS_SYMS)) != HAS_SYMS)
    return;

  storage = bfd_get_symtab_upper_bound (abfd);
  if (storage < 0)
    bfd_fatal (bfd_get_filename (abfd));
  if (storage)
    syms = (asymbol **) xmalloc (storage);

  symcount = bfd_canonicalize_symtab (abfd, syms);
  if (symcount < 0)
    bfd_fatal (bfd_get_filename (abfd));

  while (--symcount >= 0)
    {
      asymbol *sym = syms[symcount];

      if (bfd_is_com_section (sym->section)
	  && (sym->flags & BSF_SECTION_SYM) == 0)
	common_size += sym->value;
    }
  free (syms);
}

/* Display stats on file or archive member ABFD.  */

static void
display_bfd (bfd *abfd)
{
  char **matching;

  if (bfd_check_format (abfd, bfd_archive))
    /* An archive within an archive.  */
    return;

  if (bfd_check_format_matches (abfd, bfd_object, &matching))
    {
      print_sizes (abfd);
      printf ("\n");
      return;
    }

  if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
    {
      bfd_nonfatal (bfd_get_filename (abfd));
      list_matching_formats (matching);
      return_code = 3;
      return;
    }

  if (bfd_check_format_matches (abfd, bfd_core, &matching))
    {
      const char *core_cmd;

      print_sizes (abfd);
      fputs (" (core file", stdout);

      core_cmd = bfd_core_file_failing_command (abfd);
      if (core_cmd)
	printf (" invoked as %s", core_cmd);

      puts (")\n");
      return;
    }

  bfd_nonfatal (bfd_get_filename (abfd));

  if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
    list_matching_formats (matching);

  return_code = 3;
}

static void
display_archive (bfd *file)
{
  bfd *arfile = (bfd *) NULL;
  bfd *last_arfile = (bfd *) NULL;

  for (;;)
    {
      bfd_set_error (bfd_error_no_error);

      arfile = bfd_openr_next_archived_file (file, arfile);
      if (arfile == NULL)
	{
	  if (bfd_get_error () != bfd_error_no_more_archived_files)
	    {
	      bfd_nonfatal (bfd_get_filename (file));
	      return_code = 2;
	    }
	  break;
	}

      display_bfd (arfile);

      if (last_arfile != NULL)
	{
	  bfd_close (last_arfile);

	  /* PR 17512: file: a244edbc.  */
	  if (last_arfile == arfile)
	    return;
	}

      last_arfile = arfile;
    }

  if (last_arfile != NULL)
    bfd_close (last_arfile);
}

static void
display_file (char *filename)
{
  bfd *file;

  if (get_file_size (filename) < 1)
    {
      return_code = 1;
      return;
    }

  file = bfd_openr (filename, target);
  if (file == NULL)
    {
      bfd_nonfatal (filename);
      return_code = 1;
      return;
    }

  if (bfd_check_format (file, bfd_archive))
    display_archive (file);
  else
    display_bfd (file);

  if (!bfd_close (file))
    {
      bfd_nonfatal (filename);
      return_code = 1;
      return;
    }
}

static int
size_number (bfd_size_type num)
{
  char buffer[40];

  sprintf (buffer, (radix == decimal ? "%" PRIu64
		    : radix == octal ? "0%" PRIo64 : "0x%" PRIx64),
	   (uint64_t) num);

  return strlen (buffer);
}

static void
rprint_number (int width, bfd_size_type num)
{
  char buffer[40];

  sprintf (buffer, (radix == decimal ? "%" PRIu64
		    : radix == octal ? "0%" PRIo64 : "0x%" PRIx64),
	   (uint64_t) num);

  printf ("%*s", width, buffer);
}

static bfd_size_type bsssize;
static bfd_size_type datasize;
static bfd_size_type textsize;

static void
berkeley_or_gnu_sum (bfd *abfd ATTRIBUTE_UNUSED, sec_ptr sec,
		     void *ignore ATTRIBUTE_UNUSED)
{
  flagword flags;
  bfd_size_type size;

  flags = bfd_section_flags (sec);
  if ((flags & SEC_ALLOC) == 0)
    return;

  size = bfd_section_size (sec);
  if ((flags & SEC_CODE) != 0
      || (selected_output_format == FORMAT_BERKLEY
	  && (flags & SEC_READONLY) != 0))
    textsize += size;
  else if ((flags & SEC_HAS_CONTENTS) != 0)
    datasize += size;
  else
    bsssize += size;
}

static void
print_berkeley_or_gnu_format (bfd *abfd)
{
  static int files_seen = 0;
  bfd_size_type total;
  int col_width = (selected_output_format == FORMAT_BERKLEY) ? 7 : 10;
  char sep_char = (selected_output_format == FORMAT_BERKLEY) ? '\t' : ' ';

  bsssize = 0;
  datasize = 0;
  textsize = 0;

  bfd_map_over_sections (abfd, berkeley_or_gnu_sum, NULL);

  bsssize += common_size;
  if (files_seen++ == 0)
    {
      if (selected_output_format == FORMAT_BERKLEY)
	puts ((radix == octal) ? "   text\t   data\t    bss\t    oct\t    hex\tfilename" :
	      "   text\t   data\t    bss\t    dec\t    hex\tfilename");
      else
	puts ("      text       data        bss      total filename");
    }

  total = textsize + datasize + bsssize;

  if (show_totals)
    {
      total_textsize += textsize;
      total_datasize += datasize;
      total_bsssize  += bsssize;
    }

  rprint_number (col_width, textsize);
  putchar (sep_char);
  rprint_number (col_width, datasize);
  putchar (sep_char);
  rprint_number (col_width, bsssize);
  putchar (sep_char);

  if (selected_output_format == FORMAT_BERKLEY)
    printf (((radix == octal) ? "%7lo\t%7lx" : "%7lu\t%7lx"),
	    (unsigned long) total, (unsigned long) total);
  else
    rprint_number (col_width, total);

  putchar (sep_char);
  fputs (bfd_get_filename (abfd), stdout);

  if (abfd->my_archive)
    printf (" (ex %s)", bfd_get_filename (abfd->my_archive));
}

/* I REALLY miss lexical functions! */
bfd_size_type svi_total = 0;
bfd_vma svi_maxvma = 0;
int svi_namelen = 0;
int svi_vmalen = 0;
int svi_sizelen = 0;

static void
sysv_internal_sizer (bfd *file ATTRIBUTE_UNUSED, sec_ptr sec,
		     void *ignore ATTRIBUTE_UNUSED)
{
  flagword flags = bfd_section_flags (sec);
  /* Exclude sections with no flags set.  This is to omit som spaces.  */
  if (flags == 0)
    return;

  if (   ! bfd_is_abs_section (sec)
      && ! bfd_is_com_section (sec)
      && ! bfd_is_und_section (sec))
    {
      bfd_size_type size = bfd_section_size (sec);
      int namelen = strlen (bfd_section_name (sec));

      if (namelen > svi_namelen)
	svi_namelen = namelen;

      svi_total += size;

      if (bfd_section_vma (sec) > svi_maxvma)
	svi_maxvma = bfd_section_vma (sec);
    }
}

static void
sysv_one_line (const char *name, bfd_size_type size, bfd_vma vma)
{
  printf ("%-*s   ", svi_namelen, name);
  rprint_number (svi_sizelen, size);
  printf ("   ");
  rprint_number (svi_vmalen, vma);
  printf ("\n");
}

static void
sysv_internal_printer (bfd *file ATTRIBUTE_UNUSED, sec_ptr sec,
		       void *ignore ATTRIBUTE_UNUSED)
{
  flagword flags = bfd_section_flags (sec);
  if (flags == 0)
    return;

  if (   ! bfd_is_abs_section (sec)
      && ! bfd_is_com_section (sec)
      && ! bfd_is_und_section (sec))
    {
      bfd_size_type size = bfd_section_size (sec);

      svi_total += size;

      sysv_one_line (bfd_section_name (sec),
		     size,
		     bfd_section_vma (sec));
    }
}

static void
print_sysv_format (bfd *file)
{
  /* Size all of the columns.  */
  svi_total = 0;
  svi_maxvma = 0;
  svi_namelen = 0;
  bfd_map_over_sections (file, sysv_internal_sizer, NULL);
  if (show_common)
    {
      if (svi_namelen < (int) sizeof ("*COM*") - 1)
	svi_namelen = sizeof ("*COM*") - 1;
      svi_total += common_size;
    }

  svi_vmalen = size_number ((bfd_size_type)svi_maxvma);

  if ((size_t) svi_vmalen < sizeof ("addr") - 1)
    svi_vmalen = sizeof ("addr")-1;

  svi_sizelen = size_number (svi_total);
  if ((size_t) svi_sizelen < sizeof ("size") - 1)
    svi_sizelen = sizeof ("size")-1;

  svi_total = 0;
  printf ("%s  ", bfd_get_filename (file));

  if (file->my_archive)
    printf (" (ex %s)", bfd_get_filename (file->my_archive));

  printf (":\n%-*s   %*s   %*s\n", svi_namelen, "section",
	  svi_sizelen, "size", svi_vmalen, "addr");

  bfd_map_over_sections (file, sysv_internal_printer, NULL);
  if (show_common)
    {
      svi_total += common_size;
      sysv_one_line ("*COM*", common_size, 0);
    }

  printf ("%-*s   ", svi_namelen, "Total");
  rprint_number (svi_sizelen, svi_total);
  printf ("\n\n");
}

static void
print_sizes (bfd *file)
{
  if (show_common)
    calculate_common_size (file);
  if (selected_output_format == FORMAT_SYSV)
    print_sysv_format (file);
  else
    print_berkeley_or_gnu_format (file);
}
