/* nm.c -- Describe symbol table of a rel file.
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

#include "sysdep.h"
#include "bfd.h"
#include "getopt.h"
#include "aout/stab_gnu.h"
#include "aout/ranlib.h"
#include "demangle.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "elf/common.h"
#define DO_NOT_DEFINE_AOUTHDR
#define DO_NOT_DEFINE_FILHDR
#define DO_NOT_DEFINE_LINENO
#define DO_NOT_DEFINE_SCNHDR
#include "coff/external.h"
#include "coff/internal.h"
#include "libcoff.h"
#include "bucomm.h"
#include "demanguse.h"
#include "plugin-api.h"
#include "plugin.h"
#include "safe-ctype.h"

#ifndef streq
#define streq(a,b) (strcmp ((a),(b)) == 0)
#endif

/* When sorting by size, we use this structure to hold the size and a
   pointer to the minisymbol.  */

struct size_sym
{
  const void *minisym;
  bfd_vma size;
};

/* line number related info cached in bfd usrdata.  */

struct lineno_cache
{
  asection **secs;
  arelent ***relocs;
  long *relcount;
  asymbol **syms;
  long symcount;
  unsigned int seccount;
};

struct extended_symbol_info
{
  symbol_info *sinfo;
  bfd_vma ssize;
  elf_symbol_type *elfinfo;
  coff_symbol_type *coffinfo;
  /* FIXME: We should add more fields for Type, Line, Section.  */
};
#define SYM_VALUE(sym)       (sym->sinfo->value)
#define SYM_TYPE(sym)        (sym->sinfo->type)
#define SYM_STAB_NAME(sym)   (sym->sinfo->stab_name)
#define SYM_STAB_DESC(sym)   (sym->sinfo->stab_desc)
#define SYM_STAB_OTHER(sym)  (sym->sinfo->stab_other)
#define SYM_SIZE(sym) \
  (sym->elfinfo ? sym->elfinfo->internal_elf_sym.st_size: sym->ssize)

/* The output formatting functions.  */
static void print_object_filename_bsd (const char *);
static void print_object_filename_sysv (const char *);
static void print_object_filename_posix (const char *);
static void do_not_print_object_filename (const char *);

static void print_archive_filename_bsd (const char *);
static void print_archive_filename_sysv (const char *);
static void print_archive_filename_posix (const char *);
static void do_not_print_archive_filename (const char *);

static void print_archive_member_bsd (const char *, const char *);
static void print_archive_member_sysv (const char *, const char *);
static void print_archive_member_posix (const char *, const char *);
static void do_not_print_archive_member (const char *, const char *);

static void print_symbol_filename_bsd (bfd *, bfd *);
static void print_symbol_filename_sysv (bfd *, bfd *);
static void print_symbol_filename_posix (bfd *, bfd *);
static void do_not_print_symbol_filename (bfd *, bfd *);

static void print_symbol_info_bsd (struct extended_symbol_info *, bfd *);
static void print_symbol_info_sysv (struct extended_symbol_info *, bfd *);
static void print_symbol_info_posix (struct extended_symbol_info *, bfd *);
static void just_print_symbol_name (struct extended_symbol_info *, bfd *);

static void print_value (bfd *, bfd_vma);

/* Support for different output formats.  */
struct output_fns
{
  /* Print the name of an object file given on the command line.  */
  void (*print_object_filename) (const char *);

  /* Print the name of an archive file given on the command line.  */
  void (*print_archive_filename) (const char *);

  /* Print the name of an archive member file.  */
  void (*print_archive_member) (const char *, const char *);

  /* Print the name of the file (and archive, if there is one)
     containing a symbol.  */
  void (*print_symbol_filename) (bfd *, bfd *);

  /* Print a line of information about a symbol.  */
  void (*print_symbol_info) (struct extended_symbol_info *, bfd *);
};

/* Indices in `formats'.  */
enum formats
{
  FORMAT_BSD = 0,
  FORMAT_SYSV,
  FORMAT_POSIX,
  FORMAT_JUST_SYMBOLS,
  FORMAT_MAX
};

#define FORMAT_DEFAULT FORMAT_BSD

static const struct output_fns formats[FORMAT_MAX] =
{
  {print_object_filename_bsd,
   print_archive_filename_bsd,
   print_archive_member_bsd,
   print_symbol_filename_bsd,
   print_symbol_info_bsd},
  {print_object_filename_sysv,
   print_archive_filename_sysv,
   print_archive_member_sysv,
   print_symbol_filename_sysv,
   print_symbol_info_sysv},
  {print_object_filename_posix,
   print_archive_filename_posix,
   print_archive_member_posix,
   print_symbol_filename_posix,
   print_symbol_info_posix},
  {do_not_print_object_filename,
   do_not_print_archive_filename,
   do_not_print_archive_member,
   do_not_print_symbol_filename,
   just_print_symbol_name}
};


/* The output format to use.  */
static const struct output_fns *format = &formats[FORMAT_DEFAULT];
static unsigned int print_format = FORMAT_DEFAULT;
static const char *print_format_string = NULL;

/* Command options.  */

static int do_demangle = 0;	/* Pretty print C++ symbol names.  */
static int external_only = 0;	/* Print external symbols only.  */
static int defined_only = 0;	/* Print defined symbols only.  */
static int non_weak = 0;	/* Ignore weak symbols.  */
static int no_sort = 0;		/* Don't sort; print syms in order found.  */
static int print_debug_syms = 0;/* Print debugger-only symbols too.  */
static int print_armap = 0;	/* Describe __.SYMDEF data in archive files.  */
static int print_size = 0;	/* Print size of defined symbols.  */
static int reverse_sort = 0;	/* Sort in downward(alpha or numeric) order.  */
static int sort_numerically = 0;/* Sort in numeric rather than alpha order.  */
static int sort_by_size = 0;	/* Sort by size of symbol.  */
static int undefined_only = 0;	/* Print undefined symbols only.  */
static int dynamic = 0;		/* Print dynamic symbols.  */
static int show_version = 0;	/* Show the version number.  */
static int show_synthetic = 0;	/* Display synthesized symbols too.  */
static int line_numbers = 0;	/* Print line numbers for symbols.  */
static int allow_special_symbols = 0;  /* Allow special symbols.  */
static int with_symbol_versions = -1; /* Output symbol version information.  */
static int quiet = 0;		/* Suppress "no symbols" diagnostic.  */

/* The characters to use for global and local ifunc symbols.  */
#if DEFAULT_F_FOR_IFUNC_SYMBOLS
static const char * ifunc_type_chars = "Ff";
#else
static const char * ifunc_type_chars = NULL;
#endif

static int demangle_flags = DMGL_ANSI | DMGL_PARAMS;

/* When to print the names of files.  Not mutually exclusive in SYSV format.  */
static int filename_per_file = 0;	/* Once per file, on its own line.  */
static int filename_per_symbol = 0;	/* Once per symbol, at start of line.  */

static int print_width = 0;
static int print_radix = 16;
/* Print formats for printing stab info.  */
static char other_format[] = "%02x";
static char desc_format[] = "%04x";

static char *target = NULL;
#if BFD_SUPPORTS_PLUGINS
static const char *plugin_target = "plugin";
#else
static const char *plugin_target = NULL;
#endif

typedef enum unicode_display_type
{
  unicode_default = 0,
  unicode_locale,
  unicode_escape,
  unicode_hex,
  unicode_highlight,
  unicode_invalid
} unicode_display_type;

static unicode_display_type unicode_display = unicode_default;

enum long_option_values
{
  OPTION_TARGET = 200,
  OPTION_PLUGIN,
  OPTION_SIZE_SORT,
  OPTION_RECURSE_LIMIT,
  OPTION_NO_RECURSE_LIMIT,
  OPTION_IFUNC_CHARS,
  OPTION_UNICODE,
  OPTION_QUIET
};

static struct option long_options[] =
{
  {"debug-syms", no_argument, &print_debug_syms, 1},
  {"demangle", optional_argument, 0, 'C'},
  {"dynamic", no_argument, &dynamic, 1},
  {"extern-only", no_argument, &external_only, 1},
  {"format", required_argument, 0, 'f'},
  {"help", no_argument, 0, 'h'},
  {"ifunc-chars", required_argument, 0, OPTION_IFUNC_CHARS},
  {"just-symbols", no_argument, 0, 'j'},
  {"line-numbers", no_argument, 0, 'l'},
  {"no-cplus", no_argument, &do_demangle, 0},  /* Linux compatibility.  */
  {"no-demangle", no_argument, &do_demangle, 0},
  {"no-recurse-limit", no_argument, NULL, OPTION_NO_RECURSE_LIMIT},
  {"no-recursion-limit", no_argument, NULL, OPTION_NO_RECURSE_LIMIT},
  {"no-sort", no_argument, 0, 'p'},
  {"numeric-sort", no_argument, 0, 'n'},
  {"plugin", required_argument, 0, OPTION_PLUGIN},
  {"portability", no_argument, 0, 'P'},
  {"print-armap", no_argument, &print_armap, 1},
  {"print-file-name", no_argument, 0, 'o'},
  {"print-size", no_argument, 0, 'S'},
  {"quiet", no_argument, 0, OPTION_QUIET},
  {"radix", required_argument, 0, 't'},
  {"recurse-limit", no_argument, NULL, OPTION_RECURSE_LIMIT},
  {"recursion-limit", no_argument, NULL, OPTION_RECURSE_LIMIT},
  {"reverse-sort", no_argument, &reverse_sort, 1},
  {"size-sort", no_argument, 0, OPTION_SIZE_SORT},
  {"special-syms", no_argument, &allow_special_symbols, 1},
  {"synthetic", no_argument, &show_synthetic, 1},
  {"target", required_argument, 0, OPTION_TARGET},
  {"defined-only", no_argument, 0, 'U'},
  {"undefined-only", no_argument, 0, 'u'},
  {"unicode", required_argument, NULL, OPTION_UNICODE},
  {"version", no_argument, &show_version, 1},
  {"no-weak", no_argument, 0, 'W'},
  {"with-symbol-versions", no_argument, &with_symbol_versions, 1},
  {"without-symbol-versions", no_argument, &with_symbol_versions, 0},
  {0, no_argument, 0, 0}
};

/* Some error-reporting functions.  */

ATTRIBUTE_NORETURN static void
usage (FILE *stream, int status)
{
  fprintf (stream, _("Usage: %s [option(s)] [file(s)]\n"), program_name);
  fprintf (stream, _(" List symbols in [file(s)] (a.out by default).\n"));
  fprintf (stream, _(" The options are:\n"));
  fprintf (stream, _("\
  -a, --debug-syms       Display debugger-only symbols\n"));
  fprintf (stream, _("\
  -A, --print-file-name  Print name of the input file before every symbol\n"));
  fprintf (stream, _("\
  -B                     Same as --format=bsd\n"));
  fprintf (stream, _("\
  -C, --demangle[=STYLE] Decode mangled/processed symbol names\n"));
  display_demangler_styles (stream, _("\
                           STYLE can be "));
  fprintf (stream, _("\
      --no-demangle      Do not demangle low-level symbol names\n"));
  fprintf (stream, _("\
      --recurse-limit    Enable a demangling recursion limit.  (default)\n"));
  fprintf (stream, _("\
      --no-recurse-limit Disable a demangling recursion limit.\n"));
  fprintf (stream, _("\
  -D, --dynamic          Display dynamic symbols instead of normal symbols\n"));
  fprintf (stream, _("\
  -e                     (ignored)\n"));
  fprintf (stream, _("\
  -f, --format=FORMAT    Use the output format FORMAT.  FORMAT can be `bsd',\n\
                           `sysv', `posix' or 'just-symbols'.\n\
                           The default is `bsd'\n"));
  fprintf (stream, _("\
  -g, --extern-only      Display only external symbols\n"));
  fprintf (stream, _("\
    --ifunc-chars=CHARS  Characters to use when displaying ifunc symbols\n"));
  fprintf (stream, _("\
  -j, --just-symbols     Same as --format=just-symbols\n"));
  fprintf (stream, _("\
  -l, --line-numbers     Use debugging information to find a filename and\n\
                           line number for each symbol\n"));
  fprintf (stream, _("\
  -n, --numeric-sort     Sort symbols numerically by address\n"));
  fprintf (stream, _("\
  -o                     Same as -A\n"));
  fprintf (stream, _("\
  -p, --no-sort          Do not sort the symbols\n"));
  fprintf (stream, _("\
  -P, --portability      Same as --format=posix\n"));
  fprintf (stream, _("\
  -r, --reverse-sort     Reverse the sense of the sort\n"));
#if BFD_SUPPORTS_PLUGINS
  fprintf (stream, _("\
      --plugin NAME      Load the specified plugin\n"));
#endif
  fprintf (stream, _("\
  -S, --print-size       Print size of defined symbols\n"));
  fprintf (stream, _("\
  -s, --print-armap      Include index for symbols from archive members\n"));
  fprintf (stream, _("\
      --quiet            Suppress \"no symbols\" diagnostic\n"));
  fprintf (stream, _("\
      --size-sort        Sort symbols by size\n"));
  fprintf (stream, _("\
      --special-syms     Include special symbols in the output\n"));
  fprintf (stream, _("\
      --synthetic        Display synthetic symbols as well\n"));
  fprintf (stream, _("\
  -t, --radix=RADIX      Use RADIX for printing symbol values\n"));
  fprintf (stream, _("\
      --target=BFDNAME   Specify the target object format as BFDNAME\n"));
  fprintf (stream, _("\
  -u, --undefined-only   Display only undefined symbols\n"));
  fprintf (stream, _("\
  -U, --defined-only     Display only defined symbols\n"));
  fprintf (stream, _("\
      --unicode={default|show|invalid|hex|escape|highlight}\n\
                         Specify how to treat UTF-8 encoded unicode characters\n"));
  fprintf (stream, _("\
  -W, --no-weak          Ignore weak symbols\n"));
  fprintf (stream, _("\
      --with-symbol-versions  Display version strings after symbol names\n"));
  fprintf (stream, _("\
  -X 32_64               (ignored)\n"));
  fprintf (stream, _("\
  @FILE                  Read options from FILE\n"));
  fprintf (stream, _("\
  -h, --help             Display this information\n"));
  fprintf (stream, _("\
  -V, --version          Display this program's version number\n"));

  list_supported_targets (program_name, stream);
  if (REPORT_BUGS_TO[0] && status == 0)
    fprintf (stream, _("Report bugs to %s.\n"), REPORT_BUGS_TO);
  exit (status);
}

/* Set the radix for the symbol value and size according to RADIX.  */

static void
set_print_radix (char *radix)
{
  switch (*radix)
    {
    case 'x': print_radix = 16; break;
    case 'd': print_radix = 10; break;
    case 'o': print_radix =  8; break;

    default:
      fatal (_("%s: invalid radix"), radix);
    }

  other_format[3] = desc_format[3] = *radix;
}

static void
set_output_format (char *f)
{
  int i;

  switch (*f)
    {
    case 'b':
    case 'B':
      i = FORMAT_BSD;
      break;
    case 'p':
    case 'P':
      i = FORMAT_POSIX;
      break;
    case 's':
    case 'S':
      i = FORMAT_SYSV;
      break;
    case 'j':
    case 'J':
      i = FORMAT_JUST_SYMBOLS;
      break;
    default:
      fatal (_("%s: invalid output format"), f);
    }
  format = &formats[i];
  print_format = i;
}

static const char *
get_elf_symbol_type (unsigned int type)
{
  static char *bufp;
  int n;

  switch (type)
    {
    case STT_NOTYPE:   return "NOTYPE";
    case STT_OBJECT:   return "OBJECT";
    case STT_FUNC:     return "FUNC";
    case STT_SECTION:  return "SECTION";
    case STT_FILE:     return "FILE";
    case STT_COMMON:   return "COMMON";
    case STT_TLS:      return "TLS";
    }

  free (bufp);
  if (type >= STT_LOPROC && type <= STT_HIPROC)
    n = asprintf (&bufp, _("<processor specific>: %d"), type);
  else if (type >= STT_LOOS && type <= STT_HIOS)
    n = asprintf (&bufp, _("<OS specific>: %d"), type);
  else
    n = asprintf (&bufp, _("<unknown>: %d"), type);
  if (n < 0)
    fatal ("%s", xstrerror (errno));
  return bufp;
}

static const char *
get_coff_symbol_type (const struct internal_syment *sym)
{
  static char *bufp;
  int n;

  switch (sym->n_sclass)
    {
    case C_BLOCK: return "Block";
    case C_FILE:  return "File";
    case C_LINE:  return "Line";
    }

  if (!sym->n_type)
    return "None";

  switch (DTYPE(sym->n_type))
    {
    case DT_FCN: return "Function";
    case DT_PTR: return "Pointer";
    case DT_ARY: return "Array";
    }

  free (bufp);
  n = asprintf (&bufp, _("<unknown>: %d/%d"), sym->n_sclass, sym->n_type);
  if (n < 0)
    fatal ("%s", xstrerror (errno));
  return bufp;
}

/* Convert a potential UTF-8 encoded sequence in IN into characters in OUT.
   The conversion format is controlled by the unicode_display variable.
   Returns the number of characters added to OUT.
   Returns the number of bytes consumed from IN in CONSUMED.
   Always consumes at least one byte and displays at least one character.  */
   
static unsigned int
display_utf8 (const unsigned char * in, char * out, unsigned int * consumed)
{
  char *        orig_out = out;
  unsigned int  nchars = 0;
  unsigned int j;

  if (unicode_display == unicode_default)
    goto invalid;

  if (in[0] < 0xc0)
    goto invalid;

  if ((in[1] & 0xc0) != 0x80)
    goto invalid;

  if ((in[0] & 0x20) == 0)
    {
      nchars = 2;
      goto valid;
    }

  if ((in[2] & 0xc0) != 0x80)
    goto invalid;

  if ((in[0] & 0x10) == 0)
    {
      nchars = 3;
      goto valid;
    }

  if ((in[3] & 0xc0) != 0x80)
    goto invalid;

  nchars = 4;

 valid:
  switch (unicode_display)
    {
    case unicode_locale:
      /* Copy the bytes into the output buffer as is.  */
      memcpy (out, in, nchars);
      out += nchars;
      break;

    case unicode_invalid:
    case unicode_hex:
      out += sprintf (out, "%c", unicode_display == unicode_hex ? '<' : '{');
      out += sprintf (out, "0x");
      for (j = 0; j < nchars; j++)
	out += sprintf (out, "%02x", in [j]);
      out += sprintf (out, "%c", unicode_display == unicode_hex ? '>' : '}');
      break;
      
    case unicode_highlight:
      if (isatty (1))
	out += sprintf (out, "\x1B[31;47m"); /* Red.  */
      /* Fall through.  */
    case unicode_escape:
      switch (nchars)
	{
	case 2:
	  out += sprintf (out, "\\u%02x%02x",
		  ((in[0] & 0x1c) >> 2), 
		  ((in[0] & 0x03) << 6) | (in[1] & 0x3f));
	  break;

	case 3:
	  out += sprintf (out, "\\u%02x%02x",
		  ((in[0] & 0x0f) << 4) | ((in[1] & 0x3c) >> 2),
		  ((in[1] & 0x03) << 6) | ((in[2] & 0x3f)));
	  break;

	case 4:
	  out += sprintf (out, "\\u%02x%02x%02x",
		  ((in[0] & 0x07) << 6) | ((in[1] & 0x3c) >> 2),
		  ((in[1] & 0x03) << 6) | ((in[2] & 0x3c) >> 2),
		  ((in[2] & 0x03) << 6) | ((in[3] & 0x3f)));
	  break;
	default:
	  /* URG.  */
	  break;
	}

      if (unicode_display == unicode_highlight && isatty (1))
	out += sprintf (out, "\033[0m"); /* Default colour.  */
      break;

    default:
      /* URG */
      break;
    }

  * consumed = nchars;
  return out - orig_out;

 invalid:
  /* Not a valid UTF-8 sequence.  */
  *out = *in;
  * consumed = 1;
  return 1;
}

/* Convert any UTF-8 encoded characters in NAME into the form specified by
   unicode_display.  Also converts control characters.  Returns a static
   buffer if conversion was necessary.
   Code stolen from objdump.c:sanitize_string().  */

static const char *
convert_utf8 (const char * in)
{
  static char *  buffer = NULL;
  static size_t  buffer_len = 0;
  const char *   original = in;
  char *         out;

  /* Paranoia.  */
  if (in == NULL)
    return "";

  /* See if any conversion is necessary.
     In the majority of cases it will not be needed.  */
  do
    {
      unsigned char c = *in++;

      if (c == 0)
	return original;

      if (ISCNTRL (c))
	break;

      if (unicode_display != unicode_default && c >= 0xc0)
	break;
    }
  while (1);

  /* Copy the input, translating as needed.  */
  in = original;
  if (buffer_len < (strlen (in) * 9))
    {
      free ((void *) buffer);
      buffer_len = strlen (in) * 9;
      buffer = xmalloc (buffer_len + 1);
    }

  out = buffer;
  do
    {
      unsigned char c = *in++;

      if (c == 0)
	break;

      if (ISCNTRL (c))
	{
	  *out++ = '^';
	  *out++ = c + 0x40;
	}
      else if (unicode_display != unicode_default && c >= 0xc0)
	{
	  unsigned int num_consumed;

	  out += display_utf8 ((const unsigned char *)(in - 1), out, & num_consumed);
	  in += num_consumed - 1;
	}
      else
	*out++ = c;
    }
  while (1);

  *out = 0;
  return buffer;
}

/* Print symbol name NAME, read from ABFD, with printf format FORM,
   demangling it if requested.  */

static void
print_symname (const char *form, struct extended_symbol_info *info,
	       const char *name, bfd *abfd)
{
  char *alloc = NULL;
  char *atver = NULL;

  if (name == NULL)
    name = info->sinfo->name;

  if (!with_symbol_versions
      && bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      atver = strchr (name, '@');
      if (atver)
	*atver = 0;
    }

  if (do_demangle && *name)
    {
      alloc = bfd_demangle (abfd, name, demangle_flags);
      if (alloc != NULL)
	name = alloc;
    }

  if (unicode_display != unicode_default)
    {
      name = convert_utf8 (name);
    }

  if (info != NULL && info->elfinfo && with_symbol_versions)
    {
      const char *version_string;
      bool hidden;

      version_string
	= bfd_get_symbol_version_string (abfd, &info->elfinfo->symbol,
					 false, &hidden);
      if (version_string && version_string[0])
	{
	  const char *at = "@@";
	  if (hidden || bfd_is_und_section (info->elfinfo->symbol.section))
	    at = "@";
	  alloc = reconcat (alloc, name, at, version_string, NULL);
	  if (alloc != NULL)
	    name = alloc;
	}
    }
  printf (form, name);
  if (atver)
    *atver = '@';
  free (alloc);
}

static void
print_symdef_entry (bfd *abfd)
{
  symindex idx = BFD_NO_MORE_SYMBOLS;
  carsym *thesym;
  bool everprinted = false;

  for (idx = bfd_get_next_mapent (abfd, idx, &thesym);
       idx != BFD_NO_MORE_SYMBOLS;
       idx = bfd_get_next_mapent (abfd, idx, &thesym))
    {
      if (!everprinted)
	{
	  printf (_("\nArchive index:\n"));
	  everprinted = true;
	}
      if (thesym->name != NULL)
	{
	  print_symname ("%s", NULL, thesym->name, abfd);
	  bfd *elt = bfd_get_elt_at_index (abfd, idx);
	  if (elt)
	    printf (" in %s\n", bfd_get_filename (elt));
	  else
	    printf ("\n");
	}
    }
}


/* True when we can report missing plugin error.  */
bool report_plugin_err = true;

/* Choose which symbol entries to print;
   compact them downward to get rid of the rest.
   Return the number of symbols to be printed.  */

static long
filter_symbols (bfd *abfd, bool is_dynamic, void *minisyms,
		long symcount, unsigned int size)
{
  bfd_byte *from, *fromend, *to;
  asymbol *store;

  store = bfd_make_empty_symbol (abfd);
  if (store == NULL)
    bfd_fatal (bfd_get_filename (abfd));

  from = (bfd_byte *) minisyms;
  fromend = from + symcount * size;
  to = (bfd_byte *) minisyms;

  for (; from < fromend; from += size)
    {
      int keep = 0;
      asymbol *sym;

      sym = bfd_minisymbol_to_symbol (abfd, is_dynamic, from, store);
      if (sym == NULL)
	continue;

      if (sym->name != NULL
	  && sym->name[0] == '_'
	  && sym->name[1] == '_'
	  && strcmp (sym->name + (sym->name[2] == '_'), "__gnu_lto_slim") == 0
	  && report_plugin_err)
	{
	  report_plugin_err = false;
	  non_fatal (_("%s: plugin needed to handle lto object"),
		     bfd_get_filename (abfd));
	}

      if (undefined_only)
	keep = bfd_is_und_section (sym->section);
      else if (external_only)
	/* PR binutls/12753: Unique symbols are global too.  */
	keep = ((sym->flags & (BSF_GLOBAL
			       | BSF_WEAK
			       | BSF_GNU_UNIQUE)) != 0
		|| bfd_is_und_section (sym->section)
		|| bfd_is_com_section (sym->section));
      else if (non_weak)
	keep = ((sym->flags & BSF_WEAK) == 0);
      else
	keep = 1;

      if (keep
	  && ! print_debug_syms
	  && (sym->flags & BSF_DEBUGGING) != 0)
	keep = 0;

      if (keep
	  && sort_by_size
	  && (bfd_is_abs_section (sym->section)
	      || bfd_is_und_section (sym->section)))
	keep = 0;

      if (keep
	  && defined_only)
	{
	  if (bfd_is_und_section (sym->section))
	    keep = 0;
	}

      if (keep
	  && bfd_is_target_special_symbol (abfd, sym)
	  && ! allow_special_symbols)
	keep = 0;

      if (keep)
	{
	  if (to != from)
	    memcpy (to, from, size);
	  to += size;
	}
    }

  return (to - (bfd_byte *) minisyms) / size;
}

/* These globals are used to pass information into the sorting
   routines.  */
static bfd *sort_bfd;
static bool sort_dynamic;
static asymbol *sort_x;
static asymbol *sort_y;

/* Symbol-sorting predicates */
#define valueof(x) ((x)->section->vma + (x)->value)

/* Numeric sorts.  Undefined symbols are always considered "less than"
   defined symbols with zero values.  Common symbols are not treated
   specially -- i.e., their sizes are used as their "values".  */

static int
non_numeric_forward (const void *P_x, const void *P_y)
{
  asymbol *x, *y;
  const char *xn, *yn;

  x = bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_x, sort_x);
  y = bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_y, sort_y);
  if (x == NULL || y == NULL)
    bfd_fatal (bfd_get_filename (sort_bfd));

  xn = bfd_asymbol_name (x);
  yn = bfd_asymbol_name (y);

  if (yn == NULL)
    return xn != NULL;
  if (xn == NULL)
    return -1;

  /* Solaris 2.5 has a bug in strcoll.
     strcoll returns invalid values when confronted with empty strings.  */
  if (*yn == '\0')
    return *xn != '\0';
  if (*xn == '\0')
    return -1;

  return strcoll (xn, yn);
}

static int
non_numeric_reverse (const void *x, const void *y)
{
  return - non_numeric_forward (x, y);
}

static int
numeric_forward (const void *P_x, const void *P_y)
{
  asymbol *x, *y;
  asection *xs, *ys;

  x = bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_x, sort_x);
  y =  bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_y, sort_y);
  if (x == NULL || y == NULL)
    bfd_fatal (bfd_get_filename (sort_bfd));

  xs = bfd_asymbol_section (x);
  ys = bfd_asymbol_section (y);

  if (bfd_is_und_section (xs))
    {
      if (! bfd_is_und_section (ys))
	return -1;
    }
  else if (bfd_is_und_section (ys))
    return 1;
  else if (valueof (x) != valueof (y))
    return valueof (x) < valueof (y) ? -1 : 1;

  return non_numeric_forward (P_x, P_y);
}

static int
numeric_reverse (const void *x, const void *y)
{
  return - numeric_forward (x, y);
}

static int (*(sorters[2][2])) (const void *, const void *) =
{
  { non_numeric_forward, non_numeric_reverse },
  { numeric_forward, numeric_reverse }
};

/* This sort routine is used by sort_symbols_by_size.  It is similar
   to numeric_forward, but when symbols have the same value it sorts
   by section VMA.  This simplifies the sort_symbols_by_size code
   which handles symbols at the end of sections.  Also, this routine
   tries to sort file names before other symbols with the same value.
   That will make the file name have a zero size, which will make
   sort_symbols_by_size choose the non file name symbol, leading to
   more meaningful output.  For similar reasons, this code sorts
   gnu_compiled_* and gcc2_compiled before other symbols with the same
   value.  */

static int
size_forward1 (const void *P_x, const void *P_y)
{
  asymbol *x, *y;
  asection *xs, *ys;
  const char *xn, *yn;
  size_t xnl, ynl;
  int xf, yf;

  x = bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_x, sort_x);
  y = bfd_minisymbol_to_symbol (sort_bfd, sort_dynamic, P_y, sort_y);
  if (x == NULL || y == NULL)
    bfd_fatal (bfd_get_filename (sort_bfd));

  xs = bfd_asymbol_section (x);
  ys = bfd_asymbol_section (y);

  if (bfd_is_und_section (xs))
    abort ();
  if (bfd_is_und_section (ys))
    abort ();

  if (valueof (x) != valueof (y))
    return valueof (x) < valueof (y) ? -1 : 1;

  if (xs->vma != ys->vma)
    return xs->vma < ys->vma ? -1 : 1;

  xn = bfd_asymbol_name (x);
  yn = bfd_asymbol_name (y);
  xnl = strlen (xn);
  ynl = strlen (yn);

  /* The symbols gnu_compiled and gcc2_compiled convey even less
     information than the file name, so sort them out first.  */

  xf = (strstr (xn, "gnu_compiled") != NULL
	|| strstr (xn, "gcc2_compiled") != NULL);
  yf = (strstr (yn, "gnu_compiled") != NULL
	|| strstr (yn, "gcc2_compiled") != NULL);

  if (xf && ! yf)
    return -1;
  if (! xf && yf)
    return 1;

  /* We use a heuristic for the file name.  It may not work on non
     Unix systems, but it doesn't really matter; the only difference
     is precisely which symbol names get printed.  */

#define file_symbol(s, sn, snl)			\
  (((s)->flags & BSF_FILE) != 0			\
   || ((snl) > 2				\
       && (sn)[(snl) - 2] == '.'		\
       && ((sn)[(snl) - 1] == 'o'		\
	   || (sn)[(snl) - 1] == 'a')))

  xf = file_symbol (x, xn, xnl);
  yf = file_symbol (y, yn, ynl);

  if (xf && ! yf)
    return -1;
  if (! xf && yf)
    return 1;

  return non_numeric_forward (P_x, P_y);
}

/* This sort routine is used by sort_symbols_by_size.  It is sorting
   an array of size_sym structures into size order.  */

static int
size_forward2 (const void *P_x, const void *P_y)
{
  const struct size_sym *x = (const struct size_sym *) P_x;
  const struct size_sym *y = (const struct size_sym *) P_y;

  if (x->size < y->size)
    return reverse_sort ? 1 : -1;
  else if (x->size > y->size)
    return reverse_sort ? -1 : 1;
  else
    return sorters[0][reverse_sort] (x->minisym, y->minisym);
}

/* Sort the symbols by size.  ELF provides a size but for other formats
   we have to make a guess by assuming that the difference between the
   address of a symbol and the address of the next higher symbol is the
   size.  */

static long
sort_symbols_by_size (bfd *abfd, bool is_dynamic, void *minisyms,
		      long symcount, unsigned int size,
		      struct size_sym **symsizesp)
{
  struct size_sym *symsizes;
  bfd_byte *from, *fromend;
  asymbol *sym = NULL;
  asymbol *store_sym, *store_next;

  qsort (minisyms, symcount, size, size_forward1);

  /* We are going to return a special set of symbols and sizes to
     print.  */
  symsizes = (struct size_sym *) xmalloc (symcount * sizeof (struct size_sym));
  *symsizesp = symsizes;

  /* Note that filter_symbols has already removed all absolute and
     undefined symbols.  Here we remove all symbols whose size winds
     up as zero.  */
  from = (bfd_byte *) minisyms;
  fromend = from + symcount * size;

  store_sym = sort_x;
  store_next = sort_y;

  if (from < fromend)
    {
      sym = bfd_minisymbol_to_symbol (abfd, is_dynamic, (const void *) from,
				      store_sym);
      if (sym == NULL)
	bfd_fatal (bfd_get_filename (abfd));
    }

  for (; from < fromend; from += size)
    {
      asymbol *next;
      asection *sec;
      bfd_vma sz;
      asymbol *temp;

      if (from + size < fromend)
	{
	  next = bfd_minisymbol_to_symbol (abfd,
					   is_dynamic,
					   (const void *) (from + size),
					   store_next);
	  if (next == NULL)
	    bfd_fatal (bfd_get_filename (abfd));
	}
      else
	next = NULL;

      sec = bfd_asymbol_section (sym);

      /* Synthetic symbols don't have a full type set of data available, thus
	 we can't rely on that information for the symbol size.  Ditto for
	 bfd/section.c:global_syms like *ABS*.  */
      if ((sym->flags & (BSF_SECTION_SYM | BSF_SYNTHETIC)) == 0
	  && bfd_get_flavour (abfd) == bfd_target_elf_flavour)
	sz = ((elf_symbol_type *) sym)->internal_elf_sym.st_size;
      else if ((sym->flags & (BSF_SECTION_SYM | BSF_SYNTHETIC)) == 0
	       && bfd_is_com_section (sec))
	sz = sym->value;
      else
	{
	  if (from + size < fromend
	      && sec == bfd_asymbol_section (next))
	    sz = valueof (next) - valueof (sym);
	  else
	    sz = (bfd_section_vma (sec)
		  + bfd_section_size (sec)
		  - valueof (sym));
	}

      if (sz != 0)
	{
	  symsizes->minisym = (const void *) from;
	  symsizes->size = sz;
	  ++symsizes;
	}

      sym = next;

      temp = store_sym;
      store_sym = store_next;
      store_next = temp;
    }

  symcount = symsizes - *symsizesp;

  /* We must now sort again by size.  */
  qsort ((void *) *symsizesp, symcount, sizeof (struct size_sym), size_forward2);

  return symcount;
}

/* This function is used to get the relocs for a particular section.
   It is called via bfd_map_over_sections.  */

static void
get_relocs (bfd *abfd, asection *sec, void *dataarg)
{
  struct lineno_cache *data = (struct lineno_cache *) dataarg;

  *data->secs = sec;
  *data->relocs = NULL;
  *data->relcount = 0;

  if ((sec->flags & SEC_RELOC) != 0)
    {
      long relsize = bfd_get_reloc_upper_bound (abfd, sec);
      if (relsize > 0)
	{
	  *data->relocs = (arelent **) xmalloc (relsize);
	  *data->relcount = bfd_canonicalize_reloc (abfd, sec, *data->relocs,
						    data->syms);
	}
    }

  ++data->secs;
  ++data->relocs;
  ++data->relcount;
}

static void
free_lineno_cache (bfd *abfd)
{
  struct lineno_cache *lc = bfd_usrdata (abfd);

  if (lc)
    {
      if (lc->relocs)
	for (unsigned int i = 0; i < lc->seccount; i++)
	  free (lc->relocs[i]);
      free (lc->relcount);
      free (lc->relocs);
      free (lc->secs);
      free (lc->syms);
      free (lc);
      bfd_set_usrdata (abfd, NULL);
    }
}

/* Print a single symbol.  */

static void
print_symbol (bfd *        abfd,
	      asymbol *    sym,
	      bfd_vma      ssize,
	      bfd *        archive_bfd)
{
  symbol_info syminfo;
  struct extended_symbol_info info;

  format->print_symbol_filename (archive_bfd, abfd);

  bfd_get_symbol_info (abfd, sym, &syminfo);

  /* PR 22967 - Distinguish between local and global ifunc symbols.  */
  if (syminfo.type == 'i'
      && sym->flags & BSF_GNU_INDIRECT_FUNCTION)
    {
      if (ifunc_type_chars == NULL || ifunc_type_chars[0] == 0)
	; /* Change nothing.  */
      else if (sym->flags & BSF_GLOBAL) 
	syminfo.type = ifunc_type_chars[0];
      else if (ifunc_type_chars[1] != 0)
	syminfo.type = ifunc_type_chars[1];
    }

  info.sinfo = &syminfo;
  info.ssize = ssize;
  /* Synthetic symbols do not have a full symbol type set of data available.
     Nor do bfd/section.c:global_syms like *ABS*.  */
  if ((sym->flags & (BSF_SECTION_SYM | BSF_SYNTHETIC)) != 0)
    {
      info.elfinfo = NULL;
      info.coffinfo = NULL;
    }
  else
    {
      info.elfinfo = elf_symbol_from (sym);
      info.coffinfo = coff_symbol_from (sym);
    }

  format->print_symbol_info (&info, abfd);

  if (line_numbers)
    {
      struct lineno_cache *lc = bfd_usrdata (abfd);
      const char *filename, *functionname;
      unsigned int lineno;

      /* We need to get the canonical symbols in order to call
         bfd_find_nearest_line.  This is inefficient, but, then, you
         don't have to use --line-numbers.  */
      if (lc == NULL)
	{
	  lc = xcalloc (1, sizeof (*lc));
	  bfd_set_usrdata (abfd, lc);
	}
      if (lc->syms == NULL && lc->symcount == 0)
	{
	  long symsize = bfd_get_symtab_upper_bound (abfd);
	  if (symsize <= 0)
	    lc->symcount = -1;
	  else
	    {
	      lc->syms = xmalloc (symsize);
	      lc->symcount = bfd_canonicalize_symtab (abfd, lc->syms);
	    }
	}

      if (lc->symcount <= 0)
	;
      else if (bfd_is_und_section (bfd_asymbol_section (sym)))
	{
	  unsigned int i;
	  const char *symname;

	  /* For an undefined symbol, we try to find a reloc for the
             symbol, and print the line number of the reloc.  */
	  if (lc->relocs == NULL)
	    {
	      unsigned int seccount = bfd_count_sections (abfd);
	      lc->seccount = seccount;
	      lc->secs = xmalloc (seccount * sizeof (*lc->secs));
	      lc->relocs = xmalloc (seccount * sizeof (*lc->relocs));
	      lc->relcount = xmalloc (seccount * sizeof (*lc->relcount));

	      struct lineno_cache rinfo = *lc;
	      bfd_map_over_sections (abfd, get_relocs, &rinfo);
	    }

	  symname = bfd_asymbol_name (sym);
	  for (i = 0; i < lc->seccount; i++)
	    {
	      long j;

	      for (j = 0; j < lc->relcount[i]; j++)
		{
		  arelent *r;

		  r = lc->relocs[i][j];
		  if (r->sym_ptr_ptr != NULL
		      && (*r->sym_ptr_ptr)->section == sym->section
		      && (*r->sym_ptr_ptr)->value == sym->value
		      && strcmp (symname,
				 bfd_asymbol_name (*r->sym_ptr_ptr)) == 0
		      && bfd_find_nearest_line (abfd, lc->secs[i], lc->syms,
						r->address, &filename,
						&functionname, &lineno)
		      && filename != NULL)
		    {
		      /* We only print the first one we find.  */
		      printf ("\t%s:%u", filename, lineno);
		      i = lc->seccount;
		      break;
		    }
		}
	    }
	}
      else if (bfd_asymbol_section (sym)->owner == abfd)
	{
	  if ((bfd_find_line (abfd, lc->syms, sym, &filename, &lineno)
	       || bfd_find_nearest_line (abfd, bfd_asymbol_section (sym),
					 lc->syms, sym->value, &filename,
					 &functionname, &lineno))
	      && filename != NULL
	      && lineno != 0)
	    printf ("\t%s:%u", filename, lineno);
	}
    }

  putchar ('\n');
}

/* Print the symbols when sorting by size.  */

static void
print_size_symbols (bfd *abfd,
		    bool is_dynamic,
		    struct size_sym *symsizes,
		    long symcount,
		    bfd *archive_bfd)
{
  asymbol *store;
  struct size_sym *from;
  struct size_sym *fromend;

  store = bfd_make_empty_symbol (abfd);
  if (store == NULL)
    bfd_fatal (bfd_get_filename (abfd));

  from = symsizes;
  fromend = from + symcount;

  for (; from < fromend; from++)
    {
      asymbol *sym;

      sym = bfd_minisymbol_to_symbol (abfd, is_dynamic, from->minisym, store);
      if (sym == NULL)
	bfd_fatal (bfd_get_filename (abfd));

      print_symbol (abfd, sym, from->size, archive_bfd);
    }
}


/* Print the symbols of ABFD that are held in MINISYMS.

   If ARCHIVE_BFD is non-NULL, it is the archive containing ABFD.

   SYMCOUNT is the number of symbols in MINISYMS.

   SIZE is the size of a symbol in MINISYMS.  */

static void
print_symbols (bfd *abfd,
	       bool is_dynamic,
	       void *minisyms,
	       long symcount,
	       unsigned int size,
	       bfd *archive_bfd)
{
  asymbol *store;
  bfd_byte *from;
  bfd_byte *fromend;

  store = bfd_make_empty_symbol (abfd);
  if (store == NULL)
    bfd_fatal (bfd_get_filename (abfd));

  from = (bfd_byte *) minisyms;
  fromend = from + symcount * size;

  for (; from < fromend; from += size)
    {
      asymbol *sym;

      sym = bfd_minisymbol_to_symbol (abfd, is_dynamic, from, store);
      if (sym == NULL)
	bfd_fatal (bfd_get_filename (abfd));

      print_symbol (abfd, sym, (bfd_vma) 0, archive_bfd);
    }
}

/* If ARCHIVE_BFD is non-NULL, it is the archive containing ABFD.  */

static void
display_rel_file (bfd *abfd, bfd *archive_bfd)
{
  long symcount;
  void *minisyms;
  unsigned int size;
  struct size_sym *symsizes;
  asymbol *synthsyms = NULL;

  if (! dynamic)
    {
      if (!(bfd_get_file_flags (abfd) & HAS_SYMS))
	{
	  if (!quiet)
	    non_fatal (_("%s: no symbols"), bfd_get_filename (abfd));
	  return;
	}
    }

  symcount = bfd_read_minisymbols (abfd, dynamic, &minisyms, &size);
  if (symcount <= 0)
    {
      if (!quiet)
	non_fatal (_("%s: no symbols"), bfd_get_filename (abfd));
      return;
    }

  if (show_synthetic && size == sizeof (asymbol *))
    {
      asymbol **static_syms = NULL;
      asymbol **dyn_syms = NULL;
      long static_count = 0;
      long dyn_count = 0;
      long synth_count;

      if (dynamic)
	{
	  dyn_count = symcount;
	  dyn_syms = (asymbol **) minisyms;
	}
      else
	{
	  long storage = bfd_get_dynamic_symtab_upper_bound (abfd);

	  static_count = symcount;
	  static_syms = (asymbol **) minisyms;

	  if (storage > 0)
	    {
	      dyn_syms = (asymbol **) xmalloc (storage);
	      dyn_count = bfd_canonicalize_dynamic_symtab (abfd, dyn_syms);
	      if (dyn_count < 0)
		dyn_count = 0;
	    }
	}

      synth_count = bfd_get_synthetic_symtab (abfd, static_count, static_syms,
					      dyn_count, dyn_syms, &synthsyms);
      if (synth_count > 0)
	{
	  asymbol **symp;
	  long i;

	  minisyms = xrealloc (minisyms,
			       (symcount + synth_count + 1) * sizeof (*symp));
	  symp = (asymbol **) minisyms + symcount;
	  for (i = 0; i < synth_count; i++)
	    *symp++ = synthsyms + i;
	  *symp = 0;
	  symcount += synth_count;
	}
      if (!dynamic && dyn_syms != NULL)
	free (dyn_syms);
    }

  /* lto_slim_object is set to false when a bfd is loaded with a compiler
     LTO plugin.  */
  if (abfd->lto_slim_object)
    {
      report_plugin_err = false;
      non_fatal (_("%s: plugin needed to handle lto object"),
		 bfd_get_filename (abfd));
    }

  /* Discard the symbols we don't want to print.
     It's OK to do this in place; we'll free the storage anyway
     (after printing).  */

  symcount = filter_symbols (abfd, dynamic, minisyms, symcount, size);

  symsizes = NULL;
  if (! no_sort)
    {
      sort_bfd = abfd;
      sort_dynamic = dynamic;
      sort_x = bfd_make_empty_symbol (abfd);
      sort_y = bfd_make_empty_symbol (abfd);
      if (sort_x == NULL || sort_y == NULL)
	bfd_fatal (bfd_get_filename (abfd));

      if (! sort_by_size)
	qsort (minisyms, symcount, size,
	       sorters[sort_numerically][reverse_sort]);
      else
	symcount = sort_symbols_by_size (abfd, dynamic, minisyms, symcount,
					 size, &symsizes);
    }

  if (! sort_by_size)
    print_symbols (abfd, dynamic, minisyms, symcount, size, archive_bfd);
  else
    print_size_symbols (abfd, dynamic, symsizes, symcount, archive_bfd);

  if (synthsyms)
    free (synthsyms);
  free (minisyms);
  free (symsizes);
}

/* Construct a formatting string for printing symbol values.  */

static const char *
get_print_format (void)
{
  const char * padding;
  if (print_format == FORMAT_POSIX || print_format == FORMAT_JUST_SYMBOLS)
    {
      /* POSIX compatible output does not have any padding.  */
      padding = "";
    }
  else if (print_width == 32)
    {
      padding ="08";
    }
  else /* print_width == 64 */
    {
      padding = "016";
    }

  const char * radix = NULL;
  switch (print_radix)
    {
    case 8:  radix = PRIo64; break;
    case 10: radix = PRId64; break;
    case 16: radix = PRIx64; break;
    }

  return concat ("%", padding, radix, NULL);
}

static void
set_print_width (bfd *file)
{
  print_width = bfd_get_arch_size (file);

  if (print_width == -1)
    {
      /* PR binutils/4292
	 Guess the target's bitsize based on its name.
	 We assume here than any 64-bit format will include
	 "64" somewhere in its name.  The only known exception
	 is the MMO object file format.  */
      if (strstr (bfd_get_target (file), "64") != NULL
	  || strcmp (bfd_get_target (file), "mmo") == 0)
	print_width = 64;
      else
	print_width = 32;
    }
  free ((char *) print_format_string);
  print_format_string = get_print_format ();
}

static void
display_archive (bfd *file)
{
  bfd *arfile = NULL;
  bfd *last_arfile = NULL;
  char **matching;

  format->print_archive_filename (bfd_get_filename (file));

  if (print_armap)
    print_symdef_entry (file);

  for (;;)
    {
      arfile = bfd_openr_next_archived_file (file, arfile);

      if (arfile == NULL)
	{
	  if (bfd_get_error () != bfd_error_no_more_archived_files)
	    bfd_nonfatal (bfd_get_filename (file));
	  break;
	}

      if (bfd_check_format_matches (arfile, bfd_object, &matching))
	{
	  set_print_width (arfile);
	  format->print_archive_member (bfd_get_filename (file),
					bfd_get_filename (arfile));
	  display_rel_file (arfile, file);
	}
      else
	{
	  bfd_nonfatal (bfd_get_filename (arfile));
	  if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	    list_matching_formats (matching);
	}

      if (last_arfile != NULL)
	{
	  free_lineno_cache (last_arfile);
	  bfd_close (last_arfile);
	  if (arfile == last_arfile)
	    return;
	}
      last_arfile = arfile;
    }

  if (last_arfile != NULL)
    {
      free_lineno_cache (last_arfile);
      bfd_close (last_arfile);
    }
}

static bool
display_file (char *filename)
{
  bool retval = true;
  bfd *file;
  char **matching;

  if (get_file_size (filename) < 1)
    return false;

  file = bfd_openr (filename, target ? target : plugin_target);
  if (file == NULL)
    {
      bfd_nonfatal (filename);
      return false;
    }

  /* If printing line numbers, decompress the debug sections.  */
  if (line_numbers)
    file->flags |= BFD_DECOMPRESS;

  if (bfd_check_format (file, bfd_archive))
    {
      display_archive (file);
    }
  else if (bfd_check_format_matches (file, bfd_object, &matching))
    {
      set_print_width (file);
      format->print_object_filename (filename);
      display_rel_file (file, NULL);
    }
  else
    {
      bfd_nonfatal (filename);
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	list_matching_formats (matching);
      retval = false;
    }

  free_lineno_cache (file);
  if (!bfd_close (file))
    retval = false;

  return retval;
}

/* The following 3 groups of functions are called unconditionally,
   once at the start of processing each file of the appropriate type.
   They should check `filename_per_file' and `filename_per_symbol',
   as appropriate for their output format, to determine whether to
   print anything.  */

/* Print the name of an object file given on the command line.  */

static void
print_object_filename_bsd (const char *filename)
{
  if (filename_per_file && !filename_per_symbol)
    printf ("\n%s:\n", filename);
}

static void
print_object_filename_sysv (const char *filename)
{
  if (undefined_only)
    printf (_("\n\nUndefined symbols from %s:\n\n"), filename);
  else
    printf (_("\n\nSymbols from %s:\n\n"), filename);
  if (print_width == 32)
    printf (_("\
Name                  Value   Class        Type         Size     Line  Section\n\n"));
  else
    printf (_("\
Name                  Value           Class        Type         Size             Line  Section\n\n"));
}

static void
print_object_filename_posix (const char *filename)
{
  if (filename_per_file && !filename_per_symbol)
    printf ("%s:\n", filename);
}

static void
do_not_print_object_filename (const char *filename ATTRIBUTE_UNUSED)
{
}

/* Print the name of an archive file given on the command line.  */

static void
print_archive_filename_bsd (const char *filename)
{
  if (filename_per_file)
    printf ("\n%s:\n", filename);
}

static void
print_archive_filename_sysv (const char *filename ATTRIBUTE_UNUSED)
{
}

static void
print_archive_filename_posix (const char *filename ATTRIBUTE_UNUSED)
{
}

static void
do_not_print_archive_filename (const char *filename ATTRIBUTE_UNUSED)
{
}

/* Print the name of an archive member file.  */

static void
print_archive_member_bsd (const char *archive ATTRIBUTE_UNUSED,
			  const char *filename)
{
  if (!filename_per_symbol)
    printf ("\n%s:\n", filename);
}

static void
print_archive_member_sysv (const char *archive, const char *filename)
{
  if (undefined_only)
    printf (_("\n\nUndefined symbols from %s[%s]:\n\n"), archive, filename);
  else
    printf (_("\n\nSymbols from %s[%s]:\n\n"), archive, filename);
  if (print_width == 32)
    printf (_("\
Name                  Value   Class        Type         Size     Line  Section\n\n"));
  else
    printf (_("\
Name                  Value           Class        Type         Size             Line  Section\n\n"));
}

static void
print_archive_member_posix (const char *archive, const char *filename)
{
  if (!filename_per_symbol)
    printf ("%s[%s]:\n", archive, filename);
}

static void
do_not_print_archive_member (const char *archive ATTRIBUTE_UNUSED,
			     const char *filename ATTRIBUTE_UNUSED)
{
}


/* Print the name of the file (and archive, if there is one)
   containing a symbol.  */

static void
print_symbol_filename_bsd (bfd *archive_bfd, bfd *abfd)
{
  if (filename_per_symbol)
    {
      if (archive_bfd)
	printf ("%s:", bfd_get_filename (archive_bfd));
      printf ("%s:", bfd_get_filename (abfd));
    }
}

static void
print_symbol_filename_sysv (bfd *archive_bfd, bfd *abfd)
{
  if (filename_per_symbol)
    {
      if (archive_bfd)
	printf ("%s:", bfd_get_filename (archive_bfd));
      printf ("%s:", bfd_get_filename (abfd));
    }
}

static void
print_symbol_filename_posix (bfd *archive_bfd, bfd *abfd)
{
  if (filename_per_symbol)
    {
      if (archive_bfd)
	printf ("%s[%s]: ", bfd_get_filename (archive_bfd),
		bfd_get_filename (abfd));
      else
	printf ("%s: ", bfd_get_filename (abfd));
    }
}

static void
do_not_print_symbol_filename (bfd *archive_bfd ATTRIBUTE_UNUSED,
			      bfd *abfd ATTRIBUTE_UNUSED)
{
}


/* Print a symbol value.  */

static void
print_value (bfd *abfd ATTRIBUTE_UNUSED, bfd_vma val)
{
  switch (print_width)
    {
    case 32:
    case 64:
      printf (print_format_string, (uint64_t) val);
      break;

    default:
      fatal (_("Print width has not been initialized (%d)"), print_width);
      break;
    }
}

/* Print a line of information about a symbol.  */

static void
print_symbol_info_bsd (struct extended_symbol_info *info, bfd *abfd)
{
  if (bfd_is_undefined_symclass (SYM_TYPE (info)))
    {
      if (print_width == 64)
	printf ("        ");
      printf ("        ");
    }
  else
    {
      /* Normally we print the value of the symbol.  If we are printing the
	 size or sorting by size then we print its size, except for the
	 (weird) special case where both flags are defined, in which case we
	 print both values.  This conforms to documented behaviour.  */
      if (sort_by_size && !print_size)
	print_value (abfd, SYM_SIZE (info));
      else
	print_value (abfd, SYM_VALUE (info));
      if (print_size && SYM_SIZE (info))
	{
	  printf (" ");
	  print_value (abfd, SYM_SIZE (info));
	}
    }

  printf (" %c", SYM_TYPE (info));

  if (SYM_TYPE (info) == '-')
    {
      /* A stab.  */
      printf (" ");
      printf (other_format, SYM_STAB_OTHER (info));
      printf (" ");
      printf (desc_format, SYM_STAB_DESC (info));
      printf (" %5s", SYM_STAB_NAME (info));
    }
  print_symname (" %s", info, NULL, abfd);
}

static void
print_symbol_info_sysv (struct extended_symbol_info *info, bfd *abfd)
{
  print_symname ("%-20s|", info, NULL, abfd);

  if (bfd_is_undefined_symclass (SYM_TYPE (info)))
    {
      if (print_width == 32)
	printf ("        ");
      else
	printf ("                ");
    }
  else
    print_value (abfd, SYM_VALUE (info));

  printf ("|   %c  |", SYM_TYPE (info));

  if (SYM_TYPE (info) == '-')
    {
      /* A stab.  */
      printf ("%18s|  ", SYM_STAB_NAME (info));		/* (C) Type.  */
      printf (desc_format, SYM_STAB_DESC (info));	/* Size.  */
      printf ("|     |");				/* Line, Section.  */
    }
  else
    {
      /* Type, Size, Line, Section */
      if (info->elfinfo)
	printf ("%18s|",
		get_elf_symbol_type (ELF_ST_TYPE (info->elfinfo->internal_elf_sym.st_info)));
      else if (info->coffinfo)
	printf ("%18s|",
		get_coff_symbol_type (&info->coffinfo->native->u.syment));
      else
	printf ("                  |");

      if (SYM_SIZE (info))
	print_value (abfd, SYM_SIZE (info));
      else
	{
	  if (print_width == 32)
	    printf ("        ");
	  else
	    printf ("                ");
	}

      if (info->elfinfo)
	printf("|     |%s", info->elfinfo->symbol.section->name);
      else if (info->coffinfo)
	printf("|     |%s", info->coffinfo->symbol.section->name);
      else
	printf("|     |");
    }
}

static void
print_symbol_info_posix (struct extended_symbol_info *info, bfd *abfd)
{
  print_symname ("%s ", info, NULL, abfd);
  printf ("%c ", SYM_TYPE (info));

  if (bfd_is_undefined_symclass (SYM_TYPE (info)))
    printf ("        ");
  else
    {
      print_value (abfd, SYM_VALUE (info));
      printf (" ");
      if (SYM_SIZE (info))
	print_value (abfd, SYM_SIZE (info));
    }
}

static void
just_print_symbol_name (struct extended_symbol_info *info, bfd *abfd)
{
  print_symname ("%s", info, NULL, abfd);
}

int
main (int argc, char **argv)
{
  int c;
  int retval;

#ifdef HAVE_LC_MESSAGES
  setlocale (LC_MESSAGES, "");
#endif
  setlocale (LC_CTYPE, "");
  setlocale (LC_COLLATE, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = *argv;
  xmalloc_set_program_name (program_name);
  bfd_set_error_program_name (program_name);
#if BFD_SUPPORTS_PLUGINS
  bfd_plugin_set_program_name (program_name);
#endif

  expandargv (&argc, &argv);

  if (bfd_init () != BFD_INIT_MAGIC)
    fatal (_("fatal error: libbfd ABI mismatch"));
  set_default_bfd_target ();

  while ((c = getopt_long (argc, argv, "aABCDef:gHhjJlnopPrSst:uU:vVvWX:",
			   long_options, (int *) 0)) != EOF)
    {
      switch (c)
	{
	case 'a':
	  print_debug_syms = 1;
	  break;
	case 'A':
	case 'o':
	  filename_per_symbol = 1;
	  break;
	case 'B':		/* For MIPS compatibility.  */
	  set_output_format ("bsd");
	  break;
	case 'C':
	  do_demangle = 1;
	  if (optarg != NULL)
	    {
	      enum demangling_styles style;

	      style = cplus_demangle_name_to_style (optarg);
	      if (style == unknown_demangling)
		fatal (_("unknown demangling style `%s'"),
		       optarg);

	      cplus_demangle_set_style (style);
	    }
	  break;
	case OPTION_RECURSE_LIMIT:
	  demangle_flags &= ~ DMGL_NO_RECURSE_LIMIT;
	  break;
	case OPTION_NO_RECURSE_LIMIT:
	  demangle_flags |= DMGL_NO_RECURSE_LIMIT;
	  break;
	case OPTION_QUIET:
	  quiet = 1;
	  break;
	case 'D':
	  dynamic = 1;
	  break;
	case 'e':
	  /* Ignored for HP/UX compatibility.  */
	  break;
	case 'f':
	  set_output_format (optarg);
	  break;
	case 'g':
	  external_only = 1;
	  break;
	case 'H':
	case 'h':
	  usage (stdout, 0);
	case 'l':
	  line_numbers = 1;
	  break;
	case 'n':
	case 'v':
	  no_sort = 0;
	  sort_numerically = 1;
	  sort_by_size = 0;
	  break;
	case 'p':
	  no_sort = 1;
	  sort_numerically = 0;
	  sort_by_size = 0;
	  break;
	case OPTION_SIZE_SORT:
	  no_sort = 0;
	  sort_numerically = 0;
	  sort_by_size = 1;
	  break;
	case 'P':
	  set_output_format ("posix");
	  break;
	case 'j':
	  set_output_format ("just-symbols");
	  break;
	case 'r':
	  reverse_sort = 1;
	  break;
	case 's':
	  print_armap = 1;
	  break;
	case 'S':
	  print_size = 1;
	  break;
	case 't':
	  set_print_radix (optarg);
	  break;
	case 'u':
	  undefined_only = 1;
	  defined_only = 0;
	  break;
	case 'U':
	  defined_only = 1;
	  undefined_only = 0;
	  break;

	case OPTION_UNICODE:
	  if (streq (optarg, "default") || streq (optarg, "d"))
	    unicode_display = unicode_default;
	  else if (streq (optarg, "locale") || streq (optarg, "l"))
	    unicode_display = unicode_locale;
	  else if (streq (optarg, "escape") || streq (optarg, "e"))
	    unicode_display = unicode_escape;
	  else if (streq (optarg, "invalid") || streq (optarg, "i"))
	    unicode_display = unicode_invalid;
	  else if (streq (optarg, "hex") || streq (optarg, "x"))
	    unicode_display = unicode_hex;
	  else if (streq (optarg, "highlight") || streq (optarg, "h"))
	    unicode_display = unicode_highlight;
	  else
	    fatal (_("invalid argument to -U/--unicode: %s"), optarg);
	  break;

	case 'V':
	  show_version = 1;
	  break;
	case 'W':
	  non_weak = 1;
	  break;
	case 'X':
	  /* Ignored for (partial) AIX compatibility.  On AIX, the
	     argument has values 32, 64, or 32_64, and specifies that
	     only 32-bit, only 64-bit, or both kinds of objects should
	     be examined.  The default is 32.  So plain AIX nm on a
	     library archive with both kinds of objects will ignore
	     the 64-bit ones.  For GNU nm, the default is and always
	     has been -X 32_64, and other options are not supported.  */
	  if (strcmp (optarg, "32_64") != 0)
	    fatal (_("Only -X 32_64 is supported"));
	  break;

	case OPTION_TARGET:	/* --target */
	  target = optarg;
	  break;

	case OPTION_PLUGIN:	/* --plugin */
#if BFD_SUPPORTS_PLUGINS
	  bfd_plugin_set_plugin (optarg);
#else
	  fatal (_("sorry - this program has been built without plugin support\n"));
#endif
	  break;

	case OPTION_IFUNC_CHARS:
	  ifunc_type_chars = optarg;
	  break;

	case 0:		/* A long option that just sets a flag.  */
	  break;

	default:
	  usage (stderr, 1);
	}
    }

  if (show_version)
    print_version ("nm");

  if (sort_by_size && undefined_only)
    {
      non_fatal (_("Using the --size-sort and --undefined-only options together"));
      non_fatal (_("will produce no output, since undefined symbols have no size."));
      return 0;
    }

  /* OK, all options now parsed.  If no filename specified, do a.out.  */
  if (optind == argc)
    return !display_file ("a.out");

  retval = 0;

  if (argc - optind > 1)
    filename_per_file = 1;

  /* We were given several filenames to do.  */
  while (optind < argc)
    {
      if (!display_file (argv[optind++]))
	retval++;
    }

  exit (retval);
  return retval;
}
