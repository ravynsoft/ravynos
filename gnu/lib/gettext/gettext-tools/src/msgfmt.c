/* Converts Uniforum style .po files to binary .mo files
   Copyright (C) 1995-1998, 2000-2007, 2009-2010, 2012, 2014-2016, 2018-2023 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "noreturn.h"
#include "closeout.h"
#include "str-list.h"
#include "dir-list.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "xalloc.h"
#include "msgfmt.h"
#include "write-mo.h"
#include "write-java.h"
#include "write-csharp.h"
#include "write-resources.h"
#include "write-tcl.h"
#include "write-qt.h"
#include "write-desktop.h"
#include "write-xml.h"
#include "propername.h"
#include "message.h"
#include "open-catalog.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "read-desktop.h"
#include "po-charset.h"
#include "msgl-check.h"
#include "msgl-iconv.h"
#include "concat-filename.h"
#include "its.h"
#include "locating-rule.h"
#include "search-path.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Contains exit status for case in which no premature exit occurs.  */
static int exit_status;

/* If true include even fuzzy translations in output file.  */
static bool include_fuzzies = false;

/* If true include even untranslated messages in output file.  */
static bool include_untranslated = false;

/* Specifies name of the output file.  */
static const char *output_file_name;

/* Java mode output file specification.  */
static bool java_mode;
static bool assume_java2;
static const char *java_resource_name;
static const char *java_locale_name;
static const char *java_class_directory;
static bool java_output_source;

/* C# mode output file specification.  */
static bool csharp_mode;
static const char *csharp_resource_name;
static const char *csharp_locale_name;
static const char *csharp_base_directory;

/* C# resources mode output file specification.  */
static bool csharp_resources_mode;

/* Tcl mode output file specification.  */
static bool tcl_mode;
static const char *tcl_locale_name;
static const char *tcl_base_directory;

/* Qt mode output file specification.  */
static bool qt_mode;

/* Desktop Entry mode output file specification.  */
static bool desktop_mode;
static const char *desktop_locale_name;
static const char *desktop_template_name;
static const char *desktop_base_directory;
static hash_table desktop_keywords;
static bool desktop_default_keywords = true;

/* XML mode output file specification.  */
static bool xml_mode;
static const char *xml_locale_name;
static const char *xml_template_name;
static const char *xml_base_directory;
static const char *xml_language;
static its_rule_list_ty *xml_its_rules;

/* We may have more than one input file.  Domains with same names in
   different files have to merged.  So we need a list of tables for
   each output file.  */
struct msg_domain
{
  /* List for mapping message IDs to message strings.  */
  message_list_ty *mlp;
  /* Name of domain these ID/String pairs are part of.  */
  const char *domain_name;
  /* Output file name.  */
  const char *file_name;
  /* Link to the next domain.  */
  struct msg_domain *next;
};
static struct msg_domain *domain_list;
static struct msg_domain *current_domain;

/* Be more verbose.  Use only 'fprintf' and 'multiline_warning' but not
   'error' or 'multiline_error' to emit verbosity messages, because 'error'
   and 'multiline_error' during PO file parsing cause the program to exit
   with EXIT_FAILURE.  See function lex_end().  */
int verbose = 0;

/* If true check strings according to format string rules for the
   language.  */
static bool check_format_strings = false;

/* If true check the header entry is present and complete.  */
static bool check_header = false;

/* Check that domain directives can be satisfied.  */
static bool check_domain = false;

/* Check that msgfmt's behaviour is semantically compatible with
   X/Open msgfmt or XView msgfmt.  */
static bool check_compatibility = false;

/* If true, consider that strings containing an '&' are menu items and
   the '&' designates a keyboard accelerator, and verify that the translations
   also have a keyboard accelerator.  */
static bool check_accelerators = false;
static char accelerator_char = '&';

/* Counters for statistics on translations for the processed files.  */
static int msgs_translated;
static int msgs_untranslated;
static int msgs_fuzzy;

/* If not zero print statistics about translation at the end.  */
static int do_statistics;

/* Long options.  */
static const struct option long_options[] =
{
  { "alignment", required_argument, NULL, 'a' },
  { "check", no_argument, NULL, 'c' },
  { "check-accelerators", optional_argument, NULL, CHAR_MAX + 1 },
  { "check-compatibility", no_argument, NULL, 'C' },
  { "check-domain", no_argument, NULL, CHAR_MAX + 2 },
  { "check-format", no_argument, NULL, CHAR_MAX + 3 },
  { "check-header", no_argument, NULL, CHAR_MAX + 4 },
  { "csharp", no_argument, NULL, CHAR_MAX + 10 },
  { "csharp-resources", no_argument, NULL, CHAR_MAX + 11 },
  { "desktop", no_argument, NULL, CHAR_MAX + 15 },
  { "directory", required_argument, NULL, 'D' },
  { "endianness", required_argument, NULL, CHAR_MAX + 13 },
  { "help", no_argument, NULL, 'h' },
  { "java", no_argument, NULL, 'j' },
  { "java2", no_argument, NULL, CHAR_MAX + 5 },
  { "keyword", optional_argument, NULL, 'k' },
  { "language", required_argument, NULL, 'L' },
  { "locale", required_argument, NULL, 'l' },
  { "no-convert", no_argument, NULL, CHAR_MAX + 17 },
  { "no-hash", no_argument, NULL, CHAR_MAX + 6 },
  { "no-redundancy", no_argument, NULL, CHAR_MAX + 18 },
  { "output-file", required_argument, NULL, 'o' },
  { "properties-input", no_argument, NULL, 'P' },
  { "qt", no_argument, NULL, CHAR_MAX + 9 },
  { "resource", required_argument, NULL, 'r' },
  { "source", no_argument, NULL, CHAR_MAX + 14 },
  { "statistics", no_argument, &do_statistics, 1 },
  { "strict", no_argument, NULL, 'S' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 8 },
  { "tcl", no_argument, NULL, CHAR_MAX + 7 },
  { "template", required_argument, NULL, CHAR_MAX + 16 },
  { "use-fuzzy", no_argument, NULL, 'f' },
  { "use-untranslated", no_argument, NULL, CHAR_MAX + 12 },
  { "verbose", no_argument, NULL, 'v' },
  { "version", no_argument, NULL, 'V' },
  { "xml", no_argument, NULL, 'x' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static const char *add_mo_suffix (const char *);
static struct msg_domain *new_domain (const char *name, const char *file_name);
static bool is_nonobsolete (const message_ty *mp);
static void read_catalog_file_msgfmt (char *filename,
                                      catalog_input_format_ty input_syntax);
static int msgfmt_desktop_bulk (const char *directory,
                                const char *template_file_name,
                                hash_table *keywords,
                                const char *file_name);
static int msgfmt_xml_bulk (const char *directory,
                            const char *template_file_name,
                            its_rule_list_ty *its_rules,
                            const char *file_name);


int
main (int argc, char *argv[])
{
  int opt;
  bool do_help = false;
  bool do_version = false;
  bool strict_uniforum = false;
  catalog_input_format_ty input_syntax = &input_format_po;
  int arg_i;
  const char *canon_encoding;
  struct msg_domain *domain;

  /* Set default value for global variables.  */
  alignment = DEFAULT_OUTPUT_ALIGNMENT;
  byteswap = 0 ^ ENDIANNESS;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;
  error_one_per_line = 1;
  exit_status = EXIT_SUCCESS;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  while ((opt = getopt_long (argc, argv, "a:cCd:D:fhjk::l:L:o:Pr:vVx",
                             long_options, NULL))
         != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;
      case 'a':
        {
          char *endp;
          size_t new_align = strtoul (optarg, &endp, 0);

          if (endp != optarg)
            alignment = new_align;
        }
        break;
      case 'c':
        check_domain = true;
        check_format_strings = true;
        check_header = true;
        break;
      case 'C':
        check_compatibility = true;
        break;
      case 'd':
        java_class_directory = optarg;
        csharp_base_directory = optarg;
        tcl_base_directory = optarg;
        desktop_base_directory = optarg;
        xml_base_directory = optarg;
        break;
      case 'D':
        dir_list_append (optarg);
        break;
      case 'f':
        include_fuzzies = true;
        break;
      case 'h':
        do_help = true;
        break;
      case 'j':
        java_mode = true;
        break;
      case 'k':
        if (optarg == NULL || *optarg == '\0')
          desktop_default_keywords = false;
        else
          {
            /* Ensure that desktop_keywords is initialized.  */
            if (desktop_keywords.table == NULL)
              hash_init (&desktop_keywords, 100);
            desktop_add_keyword (&desktop_keywords, optarg, false);
          }
        break;
      case 'l':
        java_locale_name = optarg;
        csharp_locale_name = optarg;
        tcl_locale_name = optarg;
        desktop_locale_name = optarg;
        xml_locale_name = optarg;
        break;
      case 'L':
        xml_language = optarg;
        break;
      case 'o':
        output_file_name = optarg;
        break;
      case 'P':
        input_syntax = &input_format_properties;
        break;
      case 'r':
        java_resource_name = optarg;
        csharp_resource_name = optarg;
        break;
      case 'S':
        strict_uniforum = true;
        break;
      case 'v':
        verbose++;
        break;
      case 'V':
        do_version = true;
        break;
      case 'x':
        xml_mode = true;
        break;
      case CHAR_MAX + 1: /* --check-accelerators */
        check_accelerators = true;
        if (optarg != NULL)
          {
            if (optarg[0] != '\0' && ispunct ((unsigned char) optarg[0])
                && optarg[1] == '\0')
              accelerator_char = optarg[0];
            else
              error (EXIT_FAILURE, 0,
                     _("the argument to %s should be a single punctuation character"),
                     "--check-accelerators");
          }
        break;
      case CHAR_MAX + 2: /* --check-domain */
        check_domain = true;
        break;
      case CHAR_MAX + 3: /* --check-format */
        check_format_strings = true;
        break;
      case CHAR_MAX + 4: /* --check-header */
        check_header = true;
        break;
      case CHAR_MAX + 5: /* --java2 */
        java_mode = true;
        assume_java2 = true;
        break;
      case CHAR_MAX + 6: /* --no-hash */
        no_hash_table = true;
        break;
      case CHAR_MAX + 7: /* --tcl */
        tcl_mode = true;
        break;
      case CHAR_MAX + 8: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;
      case CHAR_MAX + 9: /* --qt */
        qt_mode = true;
        break;
      case CHAR_MAX + 10: /* --csharp */
        csharp_mode = true;
        break;
      case CHAR_MAX + 11: /* --csharp-resources */
        csharp_resources_mode = true;
        break;
      case CHAR_MAX + 12: /* --use-untranslated (undocumented) */
        include_untranslated = true;
        break;
      case CHAR_MAX + 13: /* --endianness={big|little} */
        {
          int endianness;

          if (strcmp (optarg, "big") == 0)
            endianness = 1;
          else if (strcmp (optarg, "little") == 0)
            endianness = 0;
          else
            error (EXIT_FAILURE, 0, _("invalid endianness: %s"), optarg);

          byteswap = endianness ^ ENDIANNESS;
        }
        break;
      case CHAR_MAX + 14: /* --source */
        java_output_source = true;
        break;
      case CHAR_MAX + 15: /* --desktop */
        desktop_mode = true;
        break;
      case CHAR_MAX + 16: /* --template=TEMPLATE */
        desktop_template_name = optarg;
        xml_template_name = optarg;
        break;
      case CHAR_MAX + 17: /* --no-convert */
        no_convert_to_utf8 = true;
        break;
      case CHAR_MAX + 18: /* --no-redundancy */
        no_redundancy = true;
        break;
      default:
        usage (EXIT_FAILURE);
        break;
      }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "1995-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Ulrich Drepper"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test whether we have a .po file name as argument.  */
  if (optind >= argc
      && !(desktop_mode && desktop_base_directory)
      && !(xml_mode && xml_base_directory))
    {
      error (EXIT_SUCCESS, 0, _("no input file given"));
      usage (EXIT_FAILURE);
    }
  if (optind < argc
      && ((desktop_mode && desktop_base_directory)
          || (xml_mode && xml_base_directory)))
    {
      error (EXIT_SUCCESS, 0,
             _("no input file should be given if %s and %s are specified"),
             desktop_mode ? "--desktop" : "--xml", "-d");
      usage (EXIT_FAILURE);
    }

  /* Check for contradicting options.  */
  {
    unsigned int modes =
      (java_mode ? 1 : 0)
      | (csharp_mode ? 2 : 0)
      | (csharp_resources_mode ? 4 : 0)
      | (tcl_mode ? 8 : 0)
      | (qt_mode ? 16 : 0)
      | (desktop_mode ? 32 : 0)
      | (xml_mode ? 64 : 0);
    static const char *mode_options[] =
      { "--java", "--csharp", "--csharp-resources", "--tcl", "--qt",
        "--desktop", "--xml" };
    /* More than one bit set?  */
    if (modes & (modes - 1))
      {
        const char *first_option;
        const char *second_option;
        unsigned int i;
        for (i = 0; ; i++)
          if (modes & (1 << i))
            break;
        first_option = mode_options[i];
        for (i = i + 1; ; i++)
          if (modes & (1 << i))
            break;
        second_option = mode_options[i];
        error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
               first_option, second_option);
      }
  }
  if (java_mode)
    {
      if (output_file_name != NULL)
        {
          error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
                 "--java", "--output-file");
        }
      if (java_class_directory == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-d directory\" specification"),
                 "--java");
          usage (EXIT_FAILURE);
        }
    }
  else if (csharp_mode)
    {
      if (output_file_name != NULL)
        {
          error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
                 "--csharp", "--output-file");
        }
      if (csharp_locale_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-l locale\" specification"),
                 "--csharp");
          usage (EXIT_FAILURE);
        }
      if (csharp_base_directory == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-d directory\" specification"),
                 "--csharp");
          usage (EXIT_FAILURE);
        }
    }
  else if (tcl_mode)
    {
      if (output_file_name != NULL)
        {
          error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
                 "--tcl", "--output-file");
        }
      if (tcl_locale_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-l locale\" specification"),
                 "--tcl");
          usage (EXIT_FAILURE);
        }
      if (tcl_base_directory == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-d directory\" specification"),
                 "--tcl");
          usage (EXIT_FAILURE);
        }
    }
  else if (desktop_mode)
    {
      if (desktop_template_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"--template template\" specification"),
                 "--desktop");
          usage (EXIT_FAILURE);
        }
      if (output_file_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-o file\" specification"),
                 "--desktop");
          usage (EXIT_FAILURE);
        }
      if (desktop_base_directory != NULL && desktop_locale_name != NULL)
        error (EXIT_FAILURE, 0,
               _("%s and %s are mutually exclusive in %s"),
               "-d", "-l", "--desktop");
      if (desktop_base_directory == NULL && desktop_locale_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-l locale\" specification"),
                 "--desktop");
          usage (EXIT_FAILURE);
        }
    }
  else if (xml_mode)
    {
      if (xml_template_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"--template template\" specification"),
                 "--xml");
          usage (EXIT_FAILURE);
        }
      if (output_file_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-o file\" specification"),
                 "--xml");
          usage (EXIT_FAILURE);
        }
      if (xml_base_directory != NULL && xml_locale_name != NULL)
        error (EXIT_FAILURE, 0,
               _("%s and %s are mutually exclusive in %s"),
               "-d", "-l", "--xml");
      if (xml_base_directory == NULL && xml_locale_name == NULL)
        {
          error (EXIT_SUCCESS, 0,
                 _("%s requires a \"-l locale\" specification"),
                 "--xml");
          usage (EXIT_FAILURE);
        }
    }
  else
    {
      if (java_resource_name != NULL)
        {
          error (EXIT_SUCCESS, 0, _("%s is only valid with %s or %s"),
                 "--resource", "--java", "--csharp");
          usage (EXIT_FAILURE);
        }
      if (java_locale_name != NULL)
        {
          error (EXIT_SUCCESS, 0, _("%s is only valid with %s, %s or %s"),
                 "--locale", "--java", "--csharp", "--tcl");
          usage (EXIT_FAILURE);
        }
      if (java_class_directory != NULL)
        {
          error (EXIT_SUCCESS, 0, _("%s is only valid with %s, %s or %s"),
                 "-d", "--java", "--csharp", "--tcl");
          usage (EXIT_FAILURE);
        }
    }

  if (desktop_mode)
    {
      /* Ensure that desktop_keywords is initialized.  */
      if (desktop_keywords.table == NULL)
        hash_init (&desktop_keywords, 100);
      if (desktop_default_keywords)
        desktop_add_default_keywords (&desktop_keywords);
    }

  /* Bulk processing mode for .desktop files.
     Process all .po files in desktop_base_directory.  */
  if (desktop_mode && desktop_base_directory)
    {
      exit_status = msgfmt_desktop_bulk (desktop_base_directory,
                                         desktop_template_name,
                                         &desktop_keywords,
                                         output_file_name);
      exit (exit_status);
    }

  if (xml_mode)
    {
      char **its_dirs;
      char **dirs;
      locating_rule_list_ty *its_locating_rules;
      const char *its_basename;

      its_dirs = get_search_path ("its");
      its_locating_rules = locating_rule_list_alloc ();
      for (dirs = its_dirs; *dirs != NULL; dirs++)
        locating_rule_list_add_from_directory (its_locating_rules, *dirs);

      its_basename = locating_rule_list_locate (its_locating_rules,
                                                xml_template_name,
                                                xml_language);

      if (its_basename != NULL)
        {
          size_t j;

          xml_its_rules = its_rule_list_alloc ();
          for (j = 0; its_dirs[j] != NULL; j++)
            {
              char *its_filename =
                xconcatenated_filename (its_dirs[j], its_basename, NULL);
              struct stat statbuf;
              bool ok = false;

              if (stat (its_filename, &statbuf) == 0)
                ok = its_rule_list_add_from_file (xml_its_rules, its_filename);
              free (its_filename);
              if (ok)
                break;
            }
          if (its_dirs[j] == NULL)
            {
              its_rule_list_free (xml_its_rules);
              xml_its_rules = NULL;
            }
        }
      locating_rule_list_free (its_locating_rules);

      for (dirs = its_dirs; *dirs != NULL; dirs++)
        free (*dirs);
      free (its_dirs);

      if (xml_its_rules == NULL)
        error (EXIT_FAILURE, 0, _("cannot locate ITS rules for %s"),
               xml_template_name);
    }

  /* Bulk processing mode for XML files.
     Process all .po files in xml_base_directory.  */
  if (xml_mode && xml_base_directory)
    {
      exit_status = msgfmt_xml_bulk (xml_base_directory,
                                     xml_template_name,
                                     xml_its_rules,
                                     output_file_name);
      exit (exit_status);
    }

  /* The -o option determines the name of the domain and therefore
     the output file.  */
  if (output_file_name != NULL)
    current_domain =
      new_domain (output_file_name,
                  strict_uniforum && !csharp_resources_mode && !qt_mode
                  ? add_mo_suffix (output_file_name)
                  : output_file_name);

  /* Process all given .po files.  */
  for (arg_i = optind; arg_i < argc; arg_i++)
    {
      /* Remember that we currently have not specified any domain.  This
         is of course not true when we saw the -o option.  */
      if (output_file_name == NULL)
        current_domain = NULL;

      /* And process the input file.  */
      read_catalog_file_msgfmt (argv[arg_i], input_syntax);
    }

  /* We know a priori that some input_syntax->parse() functions convert
     strings to UTF-8.  */
  canon_encoding = (input_syntax->produces_utf8 ? po_charset_utf8 : NULL);

  /* Remove obsolete messages.  They were only needed for duplicate
     checking.  */
  for (domain = domain_list; domain != NULL; domain = domain->next)
    message_list_remove_if_not (domain->mlp, is_nonobsolete);

  /* Perform all kinds of checks: plural expressions, format strings, ...  */
  {
    int nerrors = 0;

    for (domain = domain_list; domain != NULL; domain = domain->next)
      nerrors +=
        check_message_list (domain->mlp,
                            /* Untranslated and fuzzy messages have already
                               been dealt with during parsing, see below in
                               msgfmt_frob_new_message.  */
                            0, 0,
                            1, check_format_strings, check_header,
                            check_compatibility,
                            check_accelerators, accelerator_char);

    /* Exit with status 1 on any error.  */
    if (nerrors > 0)
      {
        error (0, 0,
               ngettext ("found %d fatal error", "found %d fatal errors",
                         nerrors),
               nerrors);
        exit_status = EXIT_FAILURE;
      }
  }

  /* Compose the input file name(s).
     This is used for statistics and error messages.  */
  char *all_input_file_names;
  {
    string_list_ty input_file_names;

    string_list_init (&input_file_names);;
    for (arg_i = optind; arg_i < argc; arg_i++)
      string_list_append (&input_file_names, argv[arg_i]);
    all_input_file_names =
      string_list_join (&input_file_names, ", ", '\0', false);
    string_list_destroy (&input_file_names);
  }

  /* Now write out all domains.  */
  for (domain = domain_list; domain != NULL; domain = domain->next)
    {
      if (java_mode)
        {
          if (msgdomain_write_java (domain->mlp, canon_encoding,
                                    java_resource_name, java_locale_name,
                                    java_class_directory, assume_java2,
                                    java_output_source))
            exit_status = EXIT_FAILURE;
        }
      else if (csharp_mode)
        {
          if (msgdomain_write_csharp (domain->mlp, canon_encoding,
                                      csharp_resource_name, csharp_locale_name,
                                      csharp_base_directory))
            exit_status = EXIT_FAILURE;
        }
      else if (csharp_resources_mode)
        {
          if (msgdomain_write_csharp_resources (domain->mlp, canon_encoding,
                                                domain->domain_name,
                                                domain->file_name))
            exit_status = EXIT_FAILURE;
        }
      else if (tcl_mode)
        {
          if (msgdomain_write_tcl (domain->mlp, canon_encoding,
                                   tcl_locale_name, tcl_base_directory))
            exit_status = EXIT_FAILURE;
        }
      else if (qt_mode)
        {
          if (msgdomain_write_qt (domain->mlp, canon_encoding,
                                  domain->domain_name, domain->file_name))
            exit_status = EXIT_FAILURE;
        }
      else if (desktop_mode)
        {
          if (msgdomain_write_desktop (domain->mlp, canon_encoding,
                                       desktop_locale_name,
                                       desktop_template_name,
                                       &desktop_keywords,
                                       domain->file_name))
            exit_status = EXIT_FAILURE;
        }
      else if (xml_mode)
        {
          if (msgdomain_write_xml (domain->mlp, canon_encoding,
                                   xml_locale_name,
                                   xml_template_name,
                                   xml_its_rules,
                                   domain->file_name))
            exit_status = EXIT_FAILURE;
        }
      else
        {
          if (msgdomain_write_mo (domain->mlp, domain->domain_name,
                                  domain->file_name, all_input_file_names))
            exit_status = EXIT_FAILURE;
        }

      /* List is not used anymore.  */
      message_list_free (domain->mlp, 0);
    }

  /* Print statistics if requested.  */
  if (verbose || do_statistics)
    {
      if (do_statistics + verbose >= 2 && optind < argc)
        {
          /* Print the input file name(s) in front of the statistics line.  */
          /* TRANSLATORS: The prefix before a statistics message.  The argument
             is a file name or a comma separated list of file names.  */
          fprintf (stderr, _("%s: "), all_input_file_names);
        }
      fprintf (stderr,
               ngettext ("%d translated message", "%d translated messages",
                         msgs_translated),
               msgs_translated);
      if (msgs_fuzzy > 0)
        fprintf (stderr,
                 ngettext (", %d fuzzy translation", ", %d fuzzy translations",
                           msgs_fuzzy),
                 msgs_fuzzy);
      if (msgs_untranslated > 0)
        fprintf (stderr,
                 ngettext (", %d untranslated message",
                           ", %d untranslated messages",
                           msgs_untranslated),
                 msgs_untranslated);
      fputs (".\n", stderr);
    }

  exit (exit_status);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION] filename.po ...\n\
"), program_name);
      printf ("\n");
      printf (_("\
Generate binary message catalog from textual translation description.\n\
"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n\
Similarly for optional arguments.\n\
"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  filename.po ...             input files\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf (_("\
If input file is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Operation mode:\n"));
      printf (_("\
  -j, --java                  Java mode: generate a Java ResourceBundle class\n"));
      printf (_("\
      --java2                 like --java, and assume Java2 (JDK 1.2 or higher)\n"));
      printf (_("\
      --csharp                C# mode: generate a .NET .dll file\n"));
      printf (_("\
      --csharp-resources      C# resources mode: generate a .NET .resources file\n"));
      printf (_("\
      --tcl                   Tcl mode: generate a tcl/msgcat .msg file\n"));
      printf (_("\
      --qt                    Qt mode: generate a Qt .qm file\n"));
      printf (_("\
      --desktop               Desktop Entry mode: generate a .desktop file\n"));
      printf (_("\
      --xml                   XML mode: generate XML file\n"));
      printf ("\n");
      printf (_("\
Output file location:\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified file\n"));
      printf (_("\
      --strict                enable strict Uniforum mode\n"));
      printf (_("\
If output file is -, output is written to standard output.\n"));
      printf ("\n");
      printf (_("\
Output file location in Java mode:\n"));
      printf (_("\
  -r, --resource=RESOURCE     resource name\n"));
      printf (_("\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n"));
      printf (_("\
      --source                produce a .java file, instead of a .class file\n"));
      printf (_("\
  -d DIRECTORY                base directory of classes directory hierarchy\n"));
      printf (_("\
The class name is determined by appending the locale name to the resource name,\n\
separated with an underscore.  The -d option is mandatory.  The class is\n\
written under the specified directory.\n\
"));
      printf ("\n");
      printf (_("\
Output file location in C# mode:\n"));
      printf (_("\
  -r, --resource=RESOURCE     resource name\n"));
      printf (_("\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n"));
      printf (_("\
  -d DIRECTORY                base directory for locale dependent .dll files\n"));
      printf (_("\
The -l and -d options are mandatory.  The .dll file is written in a\n\
subdirectory of the specified directory whose name depends on the locale.\n"));
      printf ("\n");
      printf (_("\
Output file location in Tcl mode:\n"));
      printf (_("\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n"));
      printf (_("\
  -d DIRECTORY                base directory of .msg message catalogs\n"));
      printf (_("\
The -l and -d options are mandatory.  The .msg file is written in the\n\
specified directory.\n"));
      printf ("\n");
      printf (_("\
Desktop Entry mode options:\n"));
      printf (_("\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified file\n"));
      printf (_("\
  --template=TEMPLATE         a .desktop file used as a template\n"));
      printf (_("\
  -d DIRECTORY                base directory of .po files\n"));
      printf (_("\
  -kWORD, --keyword=WORD      look for WORD as an additional keyword\n\
  -k, --keyword               do not to use default keywords\n"));
      printf (_("\
The -l, -o, and --template options are mandatory.  If -D is specified, input\n\
files are read from the directory instead of the command line arguments.\n"));
      printf ("\n");
      printf (_("\
XML mode options:\n"));
      printf (_("\
  -l, --locale=LOCALE         locale name, either language or language_COUNTRY\n"));
      printf (_("\
  -L, --language=NAME         recognise the specified XML language\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified file\n"));
      printf (_("\
  --template=TEMPLATE         an XML file used as a template\n"));
      printf (_("\
  -d DIRECTORY                base directory of .po files\n"));
      printf (_("\
The -l, -o, and --template options are mandatory.  If -D is specified, input\n\
files are read from the directory instead of the command line arguments.\n"));
      printf ("\n");
      printf (_("\
Input file syntax:\n"));
      printf (_("\
  -P, --properties-input      input files are in Java .properties syntax\n"));
      printf (_("\
      --stringtable-input     input files are in NeXTstep/GNUstep .strings\n\
                              syntax\n"));
      printf ("\n");
      printf (_("\
Input file interpretation:\n"));
      printf (_("\
  -c, --check                 perform all the checks implied by\n\
                                --check-format, --check-header, --check-domain\n"));
      printf (_("\
      --check-format          check language dependent format strings\n"));
      printf (_("\
      --check-header          verify presence and contents of the header entry\n"));
      printf (_("\
      --check-domain          check for conflicts between domain directives\n\
                                and the --output-file option\n"));
      printf (_("\
  -C, --check-compatibility   check that GNU msgfmt behaves like X/Open msgfmt\n"));
      printf (_("\
      --check-accelerators[=CHAR]  check presence of keyboard accelerators for\n\
                                menu items\n"));
      printf (_("\
  -f, --use-fuzzy             use fuzzy entries in output\n"));
      printf ("\n");
      printf (_("\
Output details:\n"));
      printf (_("\
      --no-convert            don't convert the messages to UTF-8 encoding\n"));
      printf (_("\
      --no-redundancy         don't pre-expand ISO C 99 <inttypes.h>\n\
                                format string directive macros\n"));
      printf (_("\
  -a, --alignment=NUMBER      align strings to NUMBER bytes (default: %d)\n"), DEFAULT_OUTPUT_ALIGNMENT);
      printf (_("\
      --endianness=BYTEORDER  write out 32-bit numbers in the given byte order\n\
                                (big or little, default depends on platform)\n"));
      printf (_("\
      --no-hash               binary file will not include the hash table\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
      printf (_("\
      --statistics            print statistics about translations\n"));
      printf (_("\
  -v, --verbose               increase verbosity level\n"));
      printf ("\n");
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf(_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
             "https://savannah.gnu.org/projects/gettext",
             "bug-gettext@gnu.org");
    }

  exit (status);
}


static const char *
add_mo_suffix (const char *fname)
{
  size_t len;
  char *result;

  len = strlen (fname);
  if (len > 3 && memcmp (fname + len - 3, ".mo", 3) == 0)
    return fname;
  if (len > 4 && memcmp (fname + len - 4, ".gmo", 4) == 0)
    return fname;
  result = XNMALLOC (len + 4, char);
  stpcpy (stpcpy (result, fname), ".mo");
  return result;
}


static struct msg_domain *
new_domain (const char *name, const char *file_name)
{
  struct msg_domain **p_dom = &domain_list;

  while (*p_dom != NULL && strcmp (name, (*p_dom)->domain_name) != 0)
    p_dom = &(*p_dom)->next;

  if (*p_dom == NULL)
    {
      struct msg_domain *domain;

      domain = XMALLOC (struct msg_domain);
      domain->mlp = message_list_alloc (true);
      domain->domain_name = name;
      domain->file_name = file_name;
      domain->next = NULL;
      *p_dom = domain;
    }

  return *p_dom;
}


static bool
is_nonobsolete (const message_ty *mp)
{
  return !mp->obsolete;
}


/* The rest of the file defines a subclass msgfmt_catalog_reader_ty of
   default_catalog_reader_ty.  Its particularities are:
   - The header entry check is performed on-the-fly.
   - Comments are not stored, they are discarded right away.
     (This is achieved by setting handle_comments = false.)
   - The multi-domain handling is adapted to our domain_list.
 */


/* This structure defines a derived class of the default_catalog_reader_ty
   class.  (See read-catalog-abstract.h for an explanation.)  */
typedef struct msgfmt_catalog_reader_ty msgfmt_catalog_reader_ty;
struct msgfmt_catalog_reader_ty
{
  /* inherited instance variables, etc */
  DEFAULT_CATALOG_READER_TY

  bool has_header_entry;
};


/* Prepare for first message.  */
static void
msgfmt_constructor (abstract_catalog_reader_ty *that)
{
  msgfmt_catalog_reader_ty *this = (msgfmt_catalog_reader_ty *) that;

  /* Invoke superclass constructor.  */
  default_constructor (that);

  this->has_header_entry = false;
}


/* Some checks after whole file is read.  */
static void
msgfmt_parse_debrief (abstract_catalog_reader_ty *that)
{
  msgfmt_catalog_reader_ty *this = (msgfmt_catalog_reader_ty *) that;

  /* Invoke superclass method.  */
  default_parse_debrief (that);

  /* Test whether header entry was found.  */
  if (check_header)
    {
      if (!this->has_header_entry)
        {
          multiline_error (xasprintf ("%s: ", this->file_name),
                           xasprintf (_("warning: PO file header missing or invalid\n")));
          multiline_error (NULL,
                           xasprintf (_("warning: charset conversion will not work\n")));
        }
    }
}


/* Set 'domain' directive when seen in .po file.  */
static void
msgfmt_set_domain (default_catalog_reader_ty *this, char *name)
{
  /* If no output file was given, we change it with each 'domain'
     directive.  */
  if (!java_mode && !csharp_mode && !csharp_resources_mode && !tcl_mode
      && !qt_mode && !desktop_mode && !xml_mode && output_file_name == NULL)
    {
      size_t correct;

      correct = strcspn (name, INVALID_PATH_CHAR);
      if (name[correct] != '\0')
        {
          exit_status = EXIT_FAILURE;
          if (correct == 0)
            {
              error (0, 0,
                     _("domain name \"%s\" not suitable as file name"), name);
              return;
            }
          else
            error (0, 0,
                   _("domain name \"%s\" not suitable as file name: will use prefix"),
                   name);
          name[correct] = '\0';
        }

      /* Set new domain.  */
      current_domain = new_domain (name, add_mo_suffix (name));
      this->domain = current_domain->domain_name;
      this->mlp = current_domain->mlp;
    }
  else
    {
      if (check_domain)
        po_gram_error_at_line (&gram_pos,
                               _("'domain %s' directive ignored"), name);

      /* NAME was allocated in po-gram-gen.y but is not used anywhere.  */
      free (name);
    }
}


static void
msgfmt_add_message (default_catalog_reader_ty *this,
                    char *msgctxt,
                    char *msgid,
                    lex_pos_ty *msgid_pos,
                    char *msgid_plural,
                    char *msgstr, size_t msgstr_len,
                    lex_pos_ty *msgstr_pos,
                    char *prev_msgctxt,
                    char *prev_msgid,
                    char *prev_msgid_plural,
                    bool force_fuzzy, bool obsolete)
{
  /* Check whether already a domain is specified.  If not, use default
     domain.  */
  if (current_domain == NULL)
    {
      current_domain = new_domain (MESSAGE_DOMAIN_DEFAULT,
                                   add_mo_suffix (MESSAGE_DOMAIN_DEFAULT));
      /* Keep current_domain and this->domain synchronized.  */
      this->domain = current_domain->domain_name;
      this->mlp = current_domain->mlp;
    }

  /* Invoke superclass method.  */
  default_add_message (this, msgctxt, msgid, msgid_pos, msgid_plural,
                       msgstr, msgstr_len, msgstr_pos,
                       prev_msgctxt, prev_msgid, prev_msgid_plural,
                       force_fuzzy, obsolete);
}


static void
msgfmt_frob_new_message (default_catalog_reader_ty *that, message_ty *mp,
                         const lex_pos_ty *msgid_pos,
                         const lex_pos_ty *msgstr_pos)
{
  msgfmt_catalog_reader_ty *this = (msgfmt_catalog_reader_ty *) that;

  if (!mp->obsolete)
    {
      /* Don't emit untranslated entries.
         Also don't emit fuzzy entries, unless --use-fuzzy was specified.
         But ignore fuzziness of the header entry.  */
      if ((!include_untranslated && mp->msgstr[0] == '\0')
          || (!include_fuzzies && mp->is_fuzzy && !is_header (mp)))
        {
          if (check_compatibility)
            {
              error_with_progname = false;
              error_at_line (0, 0, mp->pos.file_name, mp->pos.line_number,
                             (mp->msgstr[0] == '\0'
                              ? _("empty 'msgstr' entry ignored")
                              : _("fuzzy 'msgstr' entry ignored")));
              error_with_progname = true;
            }

          /* Increment counter for fuzzy/untranslated messages.  */
          if (mp->msgstr[0] == '\0')
            ++msgs_untranslated;
          else
            ++msgs_fuzzy;

          mp->obsolete = true;
        }
      else
        {
          /* Test for header entry.  */
          if (is_header (mp))
            {
              this->has_header_entry = true;
            }
          else
            /* We don't count the header entry in the statistic so place
               the counter incrementation here.  */
            if (mp->is_fuzzy)
              ++msgs_fuzzy;
            else
              ++msgs_translated;
        }
    }
}


/* Test for '#, fuzzy' comments and warn.  */
static void
msgfmt_comment_special (abstract_catalog_reader_ty *that, const char *s)
{
  msgfmt_catalog_reader_ty *this = (msgfmt_catalog_reader_ty *) that;

  /* Invoke superclass method.  */
  default_comment_special (that, s);

  if (this->is_fuzzy)
    {
      static bool warned = false;

      if (!include_fuzzies && check_compatibility && !warned)
        {
          warned = true;
          error (0, 0,
                 _("%s: warning: source file contains fuzzy translation"),
                 gram_pos.file_name);
        }
    }
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static default_catalog_reader_class_ty msgfmt_methods =
{
  {
    sizeof (msgfmt_catalog_reader_ty),
    msgfmt_constructor,
    default_destructor,
    default_parse_brief,
    msgfmt_parse_debrief,
    default_directive_domain,
    default_directive_message,
    default_comment,
    default_comment_dot,
    default_comment_filepos,
    msgfmt_comment_special
  },
  msgfmt_set_domain, /* set_domain */
  msgfmt_add_message, /* add_message */
  msgfmt_frob_new_message /* frob_new_message */
};


/* Read .po file FILENAME and store translation pairs.  */
static void
read_catalog_file_msgfmt (char *filename, catalog_input_format_ty input_syntax)
{
  char *real_filename;
  FILE *fp = open_catalog_file (filename, &real_filename, true);
  default_catalog_reader_ty *pop;

  pop = default_catalog_reader_alloc (&msgfmt_methods);
  pop->handle_comments = false;
  pop->allow_domain_directives = true;
  pop->allow_duplicates = false;
  pop->allow_duplicates_if_same_msgstr = false;
  pop->file_name = real_filename;
  pop->mdlp = NULL;
  pop->mlp = NULL;
  if (current_domain != NULL)
    {
      /* Keep current_domain and this->domain synchronized.  */
      pop->domain = current_domain->domain_name;
      pop->mlp = current_domain->mlp;
    }
  po_lex_pass_obsolete_entries (true);
  catalog_reader_parse ((abstract_catalog_reader_ty *) pop, fp, real_filename,
                        filename, input_syntax);
  catalog_reader_free ((abstract_catalog_reader_ty *) pop);

  if (fp != stdin)
    fclose (fp);
}

static void
add_languages (string_list_ty *languages, string_list_ty *desired_languages,
               const char *line, size_t length)
{
  const char *start;

  /* Split the line by whitespace and build the languages list.  */
  for (start = line; start - line < length; )
    {
      const char *p;

      /* Skip whitespace before the string.  */
      while (*start == ' ' || *start == '\t')
        start++;

      p = start;
      while (*p != '\0' && *p != ' ' && *p != '\t')
        p++;

      if (desired_languages == NULL
          || string_list_member_desc (desired_languages, start, p - start))
        string_list_append_unique_desc (languages, start, p - start);
      start = p + 1;
    }
}

/* Compute the languages list by reading the "LINGUAS" envvar or the
   LINGUAS file under DIRECTORY.  */
static void
get_languages (string_list_ty *languages, const char *directory)
{
  char *envval;
  string_list_ty real_desired_languages, *desired_languages = NULL;
  char *linguas_file_name = NULL;
  struct stat statbuf;
  FILE *fp;
  size_t line_len = 0;
  char *line_buf = NULL;

  envval = getenv ("LINGUAS");
  if (envval)
    {
      string_list_init (&real_desired_languages);
      add_languages (&real_desired_languages, NULL, envval, strlen (envval));
      desired_languages = &real_desired_languages;
    }

  linguas_file_name = xconcatenated_filename (directory, "LINGUAS", NULL);
  if (stat (linguas_file_name, &statbuf) < 0)
    {
      error (EXIT_SUCCESS, 0, _("%s does not exist"), linguas_file_name);
      goto out;
    }

  fp = fopen (linguas_file_name, "r");
  if (fp == NULL)
    {
      error (EXIT_SUCCESS, 0, _("%s exists but cannot read"),
             linguas_file_name);
      goto out;
    }

  while (!feof (fp))
    {
      /* Read next line from file.  */
      int len = getline (&line_buf, &line_len, fp);

      /* In case of an error leave loop.  */
      if (len < 0)
        break;

      /* Remove trailing '\n' and trailing whitespace.  */
      if (len > 0 && line_buf[len - 1] == '\n')
        line_buf[--len] = '\0';
      while (len > 0
             && (line_buf[len - 1] == ' '
                 || line_buf[len - 1] == '\t'
                 || line_buf[len - 1] == '\r'))
        line_buf[--len] = '\0';

      /* Test if we have to ignore the line.  */
      if (!(*line_buf == '\0' || *line_buf == '#'))
        /* Include the line among the languages.  */
        add_languages (languages, desired_languages, line_buf, len);
    }

  free (line_buf);
  fclose (fp);

 out:
  if (desired_languages != NULL)
    string_list_destroy (desired_languages);
  free (linguas_file_name);
}

static void
msgfmt_operand_list_init (msgfmt_operand_list_ty *operands)
{
  operands->items = NULL;
  operands->nitems = 0;
  operands->nitems_max = 0;
}

static void
msgfmt_operand_list_destroy (msgfmt_operand_list_ty *operands)
{
  size_t i;

  for (i = 0; i < operands->nitems; i++)
    {
      free (operands->items[i].language);
      message_list_free (operands->items[i].mlp, 0);
    }
  free (operands->items);
}

static void
msgfmt_operand_list_append (msgfmt_operand_list_ty *operands,
                            const char *language,
                            message_list_ty *messages)
{
  msgfmt_operand_ty *operand;

  if (operands->nitems == operands->nitems_max)
    {
      operands->nitems_max = operands->nitems_max * 2 + 1;
      operands->items = xrealloc (operands->items,
                                  sizeof (msgfmt_operand_ty)
                                  * operands->nitems_max);
    }

  operand = &operands->items[operands->nitems++];
  operand->language = xstrdup (language);
  operand->mlp = messages;
}

static int
msgfmt_operand_list_add_from_directory (msgfmt_operand_list_ty *operands,
                                        const char *directory)
{
  string_list_ty languages;
  void *saved_dir_list;
  int retval = 0;
  size_t i;

  string_list_init (&languages);
  get_languages (&languages, directory);

  if (languages.nitems == 0)
    return 0;

  /* Reset the directory search list so only .po files under DIRECTORY
     will be read.  */
  saved_dir_list = dir_list_save_reset ();
  dir_list_append (directory);

  /* Read all .po files.  */
  for (i = 0; i < languages.nitems; i++)
    {
      const char *language = languages.item[i];
      message_list_ty *mlp;
      char *input_file_name;
      int nerrors;

      current_domain = new_domain (MESSAGE_DOMAIN_DEFAULT,
                                   add_mo_suffix (MESSAGE_DOMAIN_DEFAULT));

      input_file_name = xconcatenated_filename ("", language, ".po");
      read_catalog_file_msgfmt (input_file_name, &input_format_po);
      free (input_file_name);

      /* The domain directive is not supported in the bulk execution mode.
         Thus, domain_list should always contain a single domain.  */
      assert (current_domain == domain_list && domain_list->next == NULL);
      mlp = current_domain->mlp;
      free (current_domain);
      current_domain = domain_list = NULL;

      /* Remove obsolete messages.  They were only needed for duplicate
         checking.  */
      message_list_remove_if_not (mlp, is_nonobsolete);

      /* Perform all kinds of checks: plural expressions, format
         strings, ...  */
      nerrors =
        check_message_list (mlp,
                            /* Untranslated and fuzzy messages have already
                               been dealt with during parsing, see below in
                               msgfmt_frob_new_message.  */
                            0, 0,
                            1, check_format_strings, check_header,
                            check_compatibility,
                            check_accelerators, accelerator_char);

      retval += nerrors;
      if (nerrors > 0)
        {
          error (0, 0,
                 ngettext ("found %d fatal error", "found %d fatal errors",
                           nerrors),
                 nerrors);
          continue;
        }

      /* Convert the messages to Unicode.  */
      iconv_message_list (mlp, NULL, po_charset_utf8, NULL);

      msgfmt_operand_list_append (operands, language, mlp);
    }

  string_list_destroy (&languages);
  dir_list_restore (saved_dir_list);

  return retval;
}

/* Helper function to support 'bulk' operation mode of --desktop.
   This reads all .po files in DIRECTORY and merges them into a
   .desktop file FILE_NAME.  Currently it does not support some
   options available in 'iterative' mode, such as --statistics.  */
static int
msgfmt_desktop_bulk (const char *directory,
                     const char *template_file_name,
                     hash_table *keywords,
                     const char *file_name)
{
  msgfmt_operand_list_ty operands;
  int nerrors, status;

  msgfmt_operand_list_init (&operands);

  /* Read all .po files.  */
  nerrors = msgfmt_operand_list_add_from_directory (&operands, directory);
  if (nerrors > 0)
    status = 1;
  else
    /* Write the messages into .desktop file.  */
    status = msgdomain_write_desktop_bulk (&operands,
                                           template_file_name,
                                           keywords,
                                           file_name);

  msgfmt_operand_list_destroy (&operands);

  return status;
}

/* Helper function to support 'bulk' operation mode of --xml.
   This reads all .po files in DIRECTORY and merges them into an
   XML file FILE_NAME.  Currently it does not support some
   options available in 'iterative' mode, such as --statistics.  */
static int
msgfmt_xml_bulk (const char *directory,
                 const char *template_file_name,
                 its_rule_list_ty *its_rules,
                 const char *file_name)
{
  msgfmt_operand_list_ty operands;
  int nerrors, status;

  msgfmt_operand_list_init (&operands);

  /* Read all .po files.  */
  nerrors = msgfmt_operand_list_add_from_directory (&operands, directory);
  if (nerrors > 0)
    {
      msgfmt_operand_list_destroy (&operands);
      return 1;
    }

  /* Write the messages into .xml file.  */
  status = msgdomain_write_xml_bulk (&operands,
                                     template_file_name,
                                     its_rules,
                                     file_name);

  msgfmt_operand_list_destroy (&operands);

  return status;
}
