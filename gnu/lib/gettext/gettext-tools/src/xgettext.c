/* Extracts strings from C source file to Uniforum style .po file.
   Copyright (C) 1995-1998, 2000-2016, 2018-2023 Free Software Foundation, Inc.
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
#include <alloca.h>

/* Specification.  */
#include "xgettext.h"

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>
#include <limits.h>

#if HAVE_ICONV
#include <iconv.h>
#endif

#include <textstyle.h>

#include "noreturn.h"
#include "rc-str-list.h"
#include "xg-encoding.h"
#include "xg-arglist-context.h"
#include "xg-message.h"
#include "closeout.h"
#include "dir-list.h"
#include "file-list.h"
#include "str-list.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "xerror.h"
#include "xvasprintf.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "verify.h"
#include "c-strstr.h"
#include "xerror.h"
#include "filename.h"
#include "concat-filename.h"
#include "c-strcase.h"
#include "open-catalog.h"
#include "read-catalog-abstract.h"
#include "read-po.h"
#include "message.h"
#include "pos.h"
#include "po-charset.h"
#include "msgl-iconv.h"
#include "msgl-ascii.h"
#include "msgl-ofn.h"
#include "msgl-check.h"
#include "po-time.h"
#include "write-catalog.h"
#include "write-po.h"
#include "write-properties.h"
#include "write-stringtable.h"
#include "format.h"
#include "propername.h"
#include "sentence.h"
#include "its.h"
#include "locating-rule.h"
#include "search-path.h"
#include "gettext.h"

/* A convenience macro.  I don't like writing gettext() every time.  */
#define _(str) gettext (str)


#include "x-po.h"
#include "x-properties.h"
#include "x-stringtable.h"
#include "x-c.h"
#include "x-python.h"
#include "x-java.h"
#include "x-csharp.h"
#include "x-javascript.h"
#include "x-scheme.h"
#include "x-lisp.h"
#include "x-elisp.h"
#include "x-librep.h"
#include "x-ruby.h"
#include "x-sh.h"
#include "x-awk.h"
#include "x-lua.h"
#include "x-smalltalk.h"
#include "x-vala.h"
#include "x-tcl.h"
#include "x-perl.h"
#include "x-php.h"
#include "x-ycp.h"
#include "x-rst.h"
#include "x-desktop.h"
#include "x-glade.h"
#include "x-gsettings.h"
#include "x-appdata.h"


#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))
#define ENDOF(a) ((a) + SIZEOF(a))


/* If true, add all comments immediately preceding one of the keywords. */
bool add_all_comments = false;

/* Tag used in comment of prevailing domain.  */
char *comment_tag;

/* Name of default domain file.  If not set defaults to messages.po.  */
static const char *default_domain;

/* If called with --debug option the output reflects whether format
   string recognition is done automatically or forced by the user.  */
static int do_debug;

/* Content of .po files with symbols to be excluded.  */
message_list_ty *exclude;

/* Force output of PO file even if empty.  */
static int force_po;

/* Copyright holder of the output file and the translations.  */
static const char *copyright_holder = "THE PACKAGE'S COPYRIGHT HOLDER";

/* Package name.  */
static const char *package_name = NULL;

/* Package version.  */
static const char *package_version = NULL;

/* Email address or URL for reports of bugs in msgids.  */
static const char *msgid_bugs_address = NULL;

/* String used as prefix for msgstr.  */
const char *msgstr_prefix;

/* String used as suffix for msgstr.  */
const char *msgstr_suffix;

/* Directory in which output files are created.  */
static char *output_dir;

/* The output syntax: .pot or .properties or .strings.  */
static catalog_output_format_ty output_syntax = &output_format_po;

/* If nonzero omit header with information about this run.  */
int xgettext_omit_header;

/* Be more verbose.  */
int verbose = 0;

/* Table of flag_context_list_ty tables.  */
static flag_context_list_table_ty flag_table_c;
static flag_context_list_table_ty flag_table_cxx_qt;
static flag_context_list_table_ty flag_table_cxx_kde;
static flag_context_list_table_ty flag_table_cxx_boost;
static flag_context_list_table_ty flag_table_objc;
static flag_context_list_table_ty flag_table_gcc_internal;
static flag_context_list_table_ty flag_table_python;
static flag_context_list_table_ty flag_table_java;
static flag_context_list_table_ty flag_table_csharp;
static flag_context_list_table_ty flag_table_javascript;
static flag_context_list_table_ty flag_table_scheme;
static flag_context_list_table_ty flag_table_lisp;
static flag_context_list_table_ty flag_table_elisp;
static flag_context_list_table_ty flag_table_librep;
static flag_context_list_table_ty flag_table_ruby;
static flag_context_list_table_ty flag_table_sh;
static flag_context_list_table_ty flag_table_awk;
static flag_context_list_table_ty flag_table_lua;
static flag_context_list_table_ty flag_table_vala;
static flag_context_list_table_ty flag_table_tcl;
static flag_context_list_table_ty flag_table_perl;
static flag_context_list_table_ty flag_table_php;
static flag_context_list_table_ty flag_table_ycp;

/* If true, recognize Qt format strings.  */
static bool recognize_format_qt;

/* If true, recognize KDE format strings.  */
static bool recognize_format_kde;

/* If true, recognize Boost format strings.  */
static bool recognize_format_boost;

/* Syntax checks enabled by default.  */
enum is_syntax_check default_syntax_check[NSYNTAXCHECKS];

static locating_rule_list_ty *its_locating_rules;

#define ITS_ROOT_UNTRANSLATABLE \
  "<its:rules xmlns:its=\"http://www.w3.org/2005/11/its\"" \
  "           version=\"2.0\">" \
  "  <its:translateRule selector=\"/*\" translate=\"no\"/>" \
  "</its:rules>"

/* If nonzero add comments used by itstool.  */
static bool add_itstool_comments = false;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-comments", optional_argument, NULL, 'c' },
  { "add-location", optional_argument, NULL, 'n' },
  { "boost", no_argument, NULL, CHAR_MAX + 11 },
  { "c++", no_argument, NULL, 'C' },
  { "check", required_argument, NULL, CHAR_MAX + 17 },
  { "color", optional_argument, NULL, CHAR_MAX + 14 },
  { "copyright-holder", required_argument, NULL, CHAR_MAX + 1 },
  { "debug", no_argument, &do_debug, 1 },
  { "default-domain", required_argument, NULL, 'd' },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "exclude-file", required_argument, NULL, 'x' },
  { "extract-all", no_argument, NULL, 'a' },
  { "files-from", required_argument, NULL, 'f' },
  { "flag", required_argument, NULL, CHAR_MAX + 8 },
  { "force-po", no_argument, &force_po, 1 },
  { "foreign-user", no_argument, NULL, CHAR_MAX + 2 },
  { "from-code", required_argument, NULL, CHAR_MAX + 3 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "its", required_argument, NULL, CHAR_MAX + 20 },
  { "itstool", no_argument, NULL, CHAR_MAX + 19 },
  { "join-existing", no_argument, NULL, 'j' },
  { "kde", no_argument, NULL, CHAR_MAX + 10 },
  { "keyword", optional_argument, NULL, 'k' },
  { "language", required_argument, NULL, 'L' },
  { "msgid-bugs-address", required_argument, NULL, CHAR_MAX + 5 },
  { "msgstr-prefix", optional_argument, NULL, 'm' },
  { "msgstr-suffix", optional_argument, NULL, 'M' },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-location", no_argument, NULL, CHAR_MAX + 16 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 4 },
  { "omit-header", no_argument, &xgettext_omit_header, 1 },
  { "output", required_argument, NULL, 'o' },
  { "output-dir", required_argument, NULL, 'p' },
  { "package-name", required_argument, NULL, CHAR_MAX + 12 },
  { "package-version", required_argument, NULL, CHAR_MAX + 13 },
  { "properties-output", no_argument, NULL, CHAR_MAX + 6 },
  { "qt", no_argument, NULL, CHAR_MAX + 9 },
  { "sentence-end", required_argument, NULL, CHAR_MAX + 18 },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "string-limit", required_argument, NULL, 'l' },
  { "stringtable-output", no_argument, NULL, CHAR_MAX + 7 },
  { "style", required_argument, NULL, CHAR_MAX + 15 },
  { "trigraphs", no_argument, NULL, 'T' },
  { "verbose", no_argument, NULL, 'v' },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w' },
  { NULL, 0, NULL, 0 }
};


/* The extractors must all be functions returning void and taking as arguments
   - the file name or file stream,
   - the flag table,
   - a message domain list argument in which to add the messages.
   An extract_from_stream_func is preferred, because it supports extracting from
   stdin.  */
typedef void (*extract_from_stream_func) (FILE *fp, const char *real_filename,
                                          const char *logical_filename,
                                          flag_context_list_table_ty *flag_table,
                                          msgdomain_list_ty *mdlp);
typedef void (*extract_from_file_func) (const char *found_in_dir,
                                        const char *real_filename,
                                        const char *logical_filename,
                                        flag_context_list_table_ty *flag_table,
                                        msgdomain_list_ty *mdlp);

typedef struct extractor_ty extractor_ty;
struct extractor_ty
{
  extract_from_stream_func extract_from_stream;
  extract_from_file_func extract_from_file;
  flag_context_list_table_ty *flag_table;
  struct formatstring_parser *formatstring_parser1;
  struct formatstring_parser *formatstring_parser2;
  struct formatstring_parser *formatstring_parser3;
  struct formatstring_parser *formatstring_parser4;
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void read_exclusion_file (char *file_name);
static void extract_from_file (const char *file_name, extractor_ty extractor,
                               msgdomain_list_ty *mdlp);
static void extract_from_xml_file (const char *file_name,
                                   its_rule_list_ty *rules,
                                   msgdomain_list_ty *mdlp);
static message_ty *construct_header (void);
static void finalize_header (msgdomain_list_ty *mdlp);
static extractor_ty language_to_extractor (const char *name);
static const char *extension_to_language (const char *extension);


int
main (int argc, char *argv[])
{
  int optchar;
  bool do_help = false;
  bool do_version = false;
  msgdomain_list_ty *mdlp;
  bool join_existing = false;
  bool no_default_keywords = false;
  bool some_additional_keywords = false;
  bool sort_by_msgid = false;
  bool sort_by_filepos = false;
  char **dirs;
  char **its_dirs = NULL;
  char *explicit_its_filename = NULL;
  const char *file_name;
  const char *files_from = NULL;
  string_list_ty *file_list;
  char *output_file = NULL;
  const char *language = NULL;
  extractor_ty extractor = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
  int cnt;
  size_t i;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set initial value of variables.  */
  default_domain = MESSAGE_DOMAIN_DEFAULT;
  xgettext_global_source_encoding = NULL;
  init_flag_table_c ();
  init_flag_table_objc ();
  init_flag_table_kde ();
  init_flag_table_python ();
  init_flag_table_java ();
  init_flag_table_csharp ();
  init_flag_table_javascript ();
  init_flag_table_scheme ();
  init_flag_table_lisp ();
  init_flag_table_elisp ();
  init_flag_table_librep ();
  init_flag_table_ruby ();
  init_flag_table_sh ();
  init_flag_table_awk ();
  init_flag_table_lua ();
  init_flag_table_vala ();
  init_flag_table_tcl ();
  init_flag_table_perl ();
  init_flag_table_php ();
  init_flag_table_gcc_internal ();
  init_flag_table_ycp ();

  while ((optchar = getopt_long (argc, argv,
                                 "ac::Cd:D:eEf:Fhijk::l:L:m::M::no:p:sTvVw:W:x:",
                                 long_options, NULL)) != EOF)
    switch (optchar)
      {
      case '\0':                /* Long option.  */
        break;

      case 'a':
        x_c_extract_all ();
        x_sh_extract_all ();
        x_python_extract_all ();
        x_lisp_extract_all ();
        x_elisp_extract_all ();
        x_librep_extract_all ();
        x_scheme_extract_all ();
        x_java_extract_all ();
        x_csharp_extract_all ();
        x_awk_extract_all ();
        x_tcl_extract_all ();
        x_perl_extract_all ();
        x_php_extract_all ();
        x_ruby_extract_all ();
        x_lua_extract_all ();
        x_javascript_extract_all ();
        x_vala_extract_all ();
        break;

      case 'c':
        if (optarg == NULL)
          {
            add_all_comments = true;
            comment_tag = NULL;
          }
        else
          {
            add_all_comments = false;
            comment_tag = optarg;
            /* We ignore leading white space.  */
            while (isspace ((unsigned char) *comment_tag))
              ++comment_tag;
          }
        break;

      case 'C':
        language = "C++";
        break;

      case 'd':
        default_domain = optarg;
        break;

      case 'D':
        dir_list_append (optarg);
        break;

      case 'e':
        message_print_style_escape (false);
        break;

      case 'E':
        message_print_style_escape (true);
        break;

      case 'f':
        files_from = optarg;
        break;

      case 'F':
        sort_by_filepos = true;
        break;

      case 'h':
        do_help = true;
        break;

      case 'i':
        message_print_style_indent ();
        break;

      case 'j':
        join_existing = true;
        break;

      case 'k':
        if (optarg != NULL && *optarg == '\0')
          /* Make "--keyword=" work like "--keyword" and "-k".  */
          optarg = NULL;
        x_c_keyword (optarg);
        x_objc_keyword (optarg);
        x_sh_keyword (optarg);
        x_python_keyword (optarg);
        x_lisp_keyword (optarg);
        x_elisp_keyword (optarg);
        x_librep_keyword (optarg);
        x_scheme_keyword (optarg);
        x_java_keyword (optarg);
        x_csharp_keyword (optarg);
        x_awk_keyword (optarg);
        x_tcl_keyword (optarg);
        x_perl_keyword (optarg);
        x_php_keyword (optarg);
        x_ruby_keyword (optarg);
        x_lua_keyword (optarg);
        x_javascript_keyword (optarg);
        x_vala_keyword (optarg);
        x_desktop_keyword (optarg);
        if (optarg == NULL)
          no_default_keywords = true;
        else
          some_additional_keywords = true;
        break;

      case 'l':
        /* Accepted for backward compatibility with 0.10.35.  */
        break;

      case 'L':
        language = optarg;
        break;

      case 'm':
        /* -m takes an optional argument.  If none is given "" is assumed. */
        msgstr_prefix = optarg == NULL ? "" : optarg;
        break;

      case 'M':
        /* -M takes an optional argument.  If none is given "" is assumed. */
        msgstr_suffix = optarg == NULL ? "" : optarg;
        break;

      case 'n':
        if (handle_filepos_comment_option (optarg))
          usage (EXIT_FAILURE);
        break;

      case 'o':
        output_file = optarg;
        break;

      case 'p':
        {
          size_t len = strlen (optarg);

          if (output_dir != NULL)
            free (output_dir);

          if (optarg[len - 1] == '/')
            output_dir = xstrdup (optarg);
          else
            output_dir = xasprintf ("%s/", optarg);
        }
        break;

      case 's':
        sort_by_msgid = true;
        break;

      case 'S':
        message_print_style_uniforum ();
        break;

      case 'T':
        x_c_trigraphs ();
        break;

      case 'v':
        verbose++;
        break;

      case 'V':
        do_version = true;
        break;

      case 'w':
        {
          int value;
          char *endp;
          value = strtol (optarg, &endp, 10);
          if (endp != optarg)
            message_page_width_set (value);
        }
        break;

      case 'x':
        read_exclusion_file (optarg);
        break;

      case CHAR_MAX + 1:        /* --copyright-holder */
        copyright_holder = optarg;
        break;

      case CHAR_MAX + 2:        /* --foreign-user */
        copyright_holder = "";
        break;

      case CHAR_MAX + 3:        /* --from-code */
        xgettext_global_source_encoding = po_charset_canonicalize (optarg);
        if (xgettext_global_source_encoding == NULL)
          {
            multiline_warning (xasprintf (_("warning: ")),
                               xasprintf (_("'%s' is not a valid encoding name.  Using ASCII as fallback.\n"),
                                          optarg));
            xgettext_global_source_encoding = po_charset_ascii;
          }
        break;

      case CHAR_MAX + 4:        /* --no-wrap */
        message_page_width_ignore ();
        break;

      case CHAR_MAX + 5:        /* --msgid-bugs-address */
        msgid_bugs_address = optarg;
        break;

      case CHAR_MAX + 6:        /* --properties-output */
        output_syntax = &output_format_properties;
        break;

      case CHAR_MAX + 7:        /* --stringtable-output */
        output_syntax = &output_format_stringtable;
        break;

      case CHAR_MAX + 8:        /* --flag */
        xgettext_record_flag (optarg);
        break;

      case CHAR_MAX + 9:        /* --qt */
        recognize_format_qt = true;
        break;

      case CHAR_MAX + 10:       /* --kde */
        recognize_format_kde = true;
        activate_additional_keywords_kde ();
        break;

      case CHAR_MAX + 11:       /* --boost */
        recognize_format_boost = true;
        break;

      case CHAR_MAX + 12:       /* --package-name */
        package_name = optarg;
        break;

      case CHAR_MAX + 13:       /* --package-version */
        package_version = optarg;
        break;

      case CHAR_MAX + 14: /* --color */
        if (handle_color_option (optarg) || color_test_mode)
          usage (EXIT_FAILURE);
        break;

      case CHAR_MAX + 15: /* --style */
        handle_style_option (optarg);
        break;

      case CHAR_MAX + 16: /* --no-location */
        message_print_style_filepos (filepos_comment_none);
        break;

      case CHAR_MAX + 17: /* --check */
        for (i = 0; i < NSYNTAXCHECKS; i++)
          {
            if (strcmp (optarg, syntax_check_name[i]) == 0)
              {
                default_syntax_check[i] = yes;
                break;
              }
          }
        if (i == NSYNTAXCHECKS)
          error (EXIT_FAILURE, 0, _("syntax check '%s' unknown"), optarg);
        break;

      case CHAR_MAX + 18: /* --sentence-end */
        if (strcmp (optarg, "single-space") == 0)
          sentence_end_required_spaces = 1;
        else if (strcmp (optarg, "double-space") == 0)
          sentence_end_required_spaces = 2;
        else
          error (EXIT_FAILURE, 0, _("sentence end type '%s' unknown"), optarg);
        break;

      case CHAR_MAX + 20: /* --its */
        explicit_its_filename = optarg;
        break;

      case CHAR_MAX + 19: /* --itstool */
        add_itstool_comments = true;
        break;

      default:
        usage (EXIT_FAILURE);
        /* NOTREACHED */
      }

  /* Version information requested.  */
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

  /* Verify selected options.  */
  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--sort-output", "--sort-by-file");

  /* We cannot support both Qt and KDE, or Qt and Boost, or KDE and Boost
     format strings, because there are only two formatstring parsers per
     language, and formatstring_c is the first one for C++.  */
  if (recognize_format_qt && recognize_format_kde)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--qt", "--kde");
  if (recognize_format_qt && recognize_format_boost)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--qt", "--boost");
  if (recognize_format_kde && recognize_format_boost)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--kde", "--boost");

  if (join_existing && strcmp (default_domain, "-") == 0)
    error (EXIT_FAILURE, 0,
           _("--join-existing cannot be used when output is written to stdout"));

  if (no_default_keywords && !some_additional_keywords)
    {
      error (0, 0, _("\
xgettext cannot work without keywords to look for"));
      usage (EXIT_FAILURE);
    }

  /* Test whether we have some input files given.  */
  if (files_from == NULL && optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input file given"));
      usage (EXIT_FAILURE);
    }

  /* Explicit ITS file selection and language specification are
     mutually exclusive.  */
  if (explicit_its_filename != NULL && language != NULL)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--its", "--language");

  /* Warn when deprecated options are used.  */
  if (sort_by_msgid)
    error (EXIT_SUCCESS, 0, _("The option '%s' is deprecated."),
           "--sort-output");

  if (explicit_its_filename == NULL)
    {
      its_dirs = get_search_path ("its");
      its_locating_rules = locating_rule_list_alloc ();
      for (dirs = its_dirs; *dirs != NULL; dirs++)
        locating_rule_list_add_from_directory (its_locating_rules, *dirs);
    }

  /* Determine extractor from language.  */
  if (language != NULL)
    extractor = language_to_extractor (language);

  /* Canonize msgstr prefix/suffix.  */
  if (msgstr_prefix != NULL && msgstr_suffix == NULL)
    msgstr_suffix = "";
  else if (msgstr_prefix == NULL && msgstr_suffix != NULL)
    msgstr_prefix = "";

  {
    /* Default output directory is the current directory.  */
    const char *defaulted_output_dir = (output_dir != NULL ? output_dir : ".");

    /* Construct the name of the output file.  If the default domain has
       the special name "-" we write to stdout.  */
    if (output_file)
      {
        if (IS_RELATIVE_FILE_NAME (output_file) && strcmp (output_file, "-") != 0)
          /* Please do NOT add a .po suffix! */
          file_name =
            xconcatenated_filename (defaulted_output_dir, output_file, NULL);
        else
          file_name = xstrdup (output_file);
      }
    else if (strcmp (default_domain, "-") == 0)
      file_name = "-";
    else
      file_name =
        xconcatenated_filename (defaulted_output_dir, default_domain, ".po");
  }

  /* Determine list of files we have to process.  */
  if (files_from != NULL)
    file_list = read_names_from_file (files_from);
  else
    file_list = string_list_alloc ();
  /* Append names from command line.  */
  for (cnt = optind; cnt < argc; ++cnt)
    string_list_append_unique (file_list, argv[cnt]);

  /* Allocate converter from xgettext_global_source_encoding to UTF-8 (except
     from ASCII or UTF-8, when this conversion is a no-op).  */
  if (xgettext_global_source_encoding != NULL
      && xgettext_global_source_encoding != po_charset_ascii
      && xgettext_global_source_encoding != po_charset_utf8)
    {
#if HAVE_ICONV
      iconv_t cd;

      /* Avoid glibc-2.1 bug with EUC-KR.  */
# if ((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
     && !defined _LIBICONV_VERSION
      if (strcmp (xgettext_global_source_encoding, "EUC-KR") == 0)
        cd = (iconv_t)(-1);
      else
# endif
      cd = iconv_open (po_charset_utf8, xgettext_global_source_encoding);
      if (cd == (iconv_t)(-1))
        error (EXIT_FAILURE, 0,
               _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(), and iconv() does not support this conversion."),
               xgettext_global_source_encoding, po_charset_utf8,
               last_component (program_name));
      xgettext_global_source_iconv = cd;
#else
      error (EXIT_FAILURE, 0,
             _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(). This version was built without iconv()."),
             xgettext_global_source_encoding, po_charset_utf8,
             last_component (program_name));
#endif
    }

  /* Allocate a message list to remember all the messages.  */
  mdlp = msgdomain_list_alloc (true);

  /* Generate a header, so that we know how and when this PO file was
     created.  */
  if (!xgettext_omit_header)
    message_list_append (mdlp->item[0]->messages, construct_header ());

  /* Read in the old messages, so that we can add to them.  */
  if (join_existing)
    {
      /* Temporarily reset the directory list to empty, because file_name
         is an output file and therefore should not be searched for.  */
      void *saved_directory_list = dir_list_save_reset ();
      extractor_ty po_extractor =
        { extract_po, NULL, NULL, NULL, NULL, NULL, NULL };

      extract_from_file (file_name, po_extractor, mdlp);
      if (!is_ascii_msgdomain_list (mdlp))
        mdlp = iconv_msgdomain_list (mdlp, po_charset_utf8, true, file_name);

      dir_list_restore (saved_directory_list);
    }

  /* Process all input files.  */
  for (i = 0; i < file_list->nitems; i++)
    {
      const char *filename;
      extractor_ty this_file_extractor;
      its_rule_list_ty *its_rules = NULL;

      filename = file_list->item[i];

      if (extractor.extract_from_stream || extractor.extract_from_file)
        this_file_extractor = extractor;
      else if (explicit_its_filename != NULL)
        {
          its_rules = its_rule_list_alloc ();
          if (!its_rule_list_add_from_file (its_rules,
                                            explicit_its_filename))
            error (EXIT_FAILURE, 0,
                   _("warning: ITS rule file '%s' does not exist"),
                   explicit_its_filename);
        }
      else
        {
          const char *language_from_extension = NULL;
          const char *base;
          char *reduced;

          base = strrchr (filename, '/');
          if (!base)
            base = filename;

          reduced = xstrdup (base);
          /* Remove a trailing ".in" - it's a generic suffix.  */
          while (strlen (reduced) >= 3
                 && memcmp (reduced + strlen (reduced) - 3, ".in", 3) == 0)
            reduced[strlen (reduced) - 3] = '\0';

          /* If no language is specified with -L, deduce it the extension.  */
          if (language == NULL)
            {
              const char *p;

              /* Work out what the file extension is.  */
              p = reduced + strlen (reduced);
              for (; p > reduced && language_from_extension == NULL; p--)
                {
                  if (*p == '.')
                    {
                      const char *extension = p + 1;

                      /* Derive the language from the extension, and
                         the extractor function from the language.  */
                      language_from_extension =
                        extension_to_language (extension);
                    }
                }
            }

          /* If language is not determined from the file name
             extension, check ITS locating rules.  */
          if (language_from_extension == NULL
              && strcmp (filename, "-") != 0)
            {
              const char *its_basename;

              its_basename = locating_rule_list_locate (its_locating_rules,
                                                        filename,
                                                        language);

              if (its_basename != NULL)
                {
                  size_t j;

                  its_rules = its_rule_list_alloc ();

                  /* If the ITS file is identified by the name,
                     set the root element untranslatable.  */
                  if (language != NULL)
                    its_rule_list_add_from_string (its_rules,
                                                   ITS_ROOT_UNTRANSLATABLE);

                  for (j = 0; its_dirs[j] != NULL; j++)
                    {
                      char *its_filename =
                        xconcatenated_filename (its_dirs[j], its_basename,
                                                NULL);
                      struct stat statbuf;

                      if (stat (its_filename, &statbuf) == 0
                          && its_rule_list_add_from_file (its_rules,
                                                          its_filename))
                        {
                          /* The last element in its_dirs always points to
                             the fallback directory.  */
                          if (its_dirs[j + 1] == NULL)
                            error (0, 0,
                                   _("warning: a fallback ITS rule file '%s' is used; "
                                     "it may not be in sync with the upstream"),
                                   its_filename);
                          free (its_filename);
                          break;
                        }
                    }
                  if (its_dirs[j] == NULL)
                    {
                      error (0, 0,
                             _("warning: ITS rule file '%s' does not exist; check your gettext installation"),
                             its_basename);
                      its_rule_list_free (its_rules);
                      its_rules = NULL;
                    }
                }
            }

          if (its_rules == NULL)
            {
              if (language_from_extension == NULL)
                {
                  const char *extension = strrchr (reduced, '.');
                  if (extension == NULL)
                    extension = "";
                  else
                    extension++;
                  error (0, 0,
                         _("warning: file '%s' extension '%s' is unknown; will try C"),
                         filename, extension);
                  language_from_extension = "C";
                }

              this_file_extractor =
                language_to_extractor (language_from_extension);
            }

          free (reduced);
        }

      if (its_rules != NULL)
        {
          /* Extract the strings from the file, using ITS.  */
          extract_from_xml_file (filename, its_rules, mdlp);
          its_rule_list_free (its_rules);
        }
      else
        /* Extract the strings from the file.  */
        extract_from_file (filename, this_file_extractor, mdlp);
    }
  string_list_free (file_list);

  /* Finalize the constructed header.  */
  if (!xgettext_omit_header)
    finalize_header (mdlp);

  /* Free the allocated converter.  */
#if HAVE_ICONV
  if (xgettext_global_source_encoding != NULL
      && xgettext_global_source_encoding != po_charset_ascii
      && xgettext_global_source_encoding != po_charset_utf8)
    iconv_close (xgettext_global_source_iconv);
#endif

  /* Sorting the list of messages.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (mdlp);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (mdlp);

  /* Check syntax of messages.  */
  {
    int nerrors = 0;

    for (i = 0; i < mdlp->nitems; i++)
      {
        message_list_ty *mlp = mdlp->item[i]->messages;
        nerrors = syntax_check_message_list (mlp);
      }

    /* Exit with status 1 on any error.  */
    if (nerrors > 0)
      error (EXIT_FAILURE, 0,
             ngettext ("found %d fatal error", "found %d fatal errors",
                       nerrors),
             nerrors);
  }

  /* Write the PO file.  */
  msgdomain_list_print (mdlp, file_name, output_syntax, force_po, do_debug);

  if (its_locating_rules)
    locating_rule_list_free (its_locating_rules);

  if (its_dirs != NULL)
    {
      for (i = 0; its_dirs[i] != NULL; i++)
        free (its_dirs[i]);
      free (its_dirs);
    }

  exit (EXIT_SUCCESS);
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
Usage: %s [OPTION] [INPUTFILE]...\n\
"), program_name);
      printf ("\n");
      printf (_("\
Extract translatable strings from given input files.\n\
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
  INPUTFILE ...               input files\n"));
      printf (_("\
  -f, --files-from=FILE       get list of input files from FILE\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf (_("\
If input file is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Output file location:\n"));
      printf (_("\
  -d, --default-domain=NAME   use NAME.po for output (instead of messages.po)\n"));
      printf (_("\
  -o, --output=FILE           write output to specified file\n"));
      printf (_("\
  -p, --output-dir=DIR        output files will be placed in directory DIR\n"));
      printf (_("\
If output file is -, output is written to standard output.\n"));
      printf ("\n");
      printf (_("\
Choice of input file language:\n"));
      printf (_("\
  -L, --language=NAME         recognise the specified language\n\
                                (C, C++, ObjectiveC, PO, Shell, Python, Lisp,\n\
                                EmacsLisp, librep, Scheme, Smalltalk, Java,\n\
                                JavaProperties, C#, awk, YCP, Tcl, Perl, PHP,\n\
                                Ruby, GCC-source, NXStringTable, RST, RSJ,\n\
                                Glade, Lua, JavaScript, Vala, Desktop)\n"));
      printf (_("\
  -C, --c++                   shorthand for --language=C++\n"));
      printf (_("\
By default the language is guessed depending on the input file name extension.\n"));
      printf ("\n");
      printf (_("\
Input file interpretation:\n"));
      printf (_("\
      --from-code=NAME        encoding of input files\n\
                                (except for Python, Tcl, Glade)\n"));
      printf (_("\
By default the input files are assumed to be in ASCII.\n"));
      printf ("\n");
      printf (_("\
Operation mode:\n"));
      printf (_("\
  -j, --join-existing         join messages with existing file\n"));
      printf (_("\
  -x, --exclude-file=FILE.po  entries from FILE.po are not extracted\n"));
      printf (_("\
  -cTAG, --add-comments=TAG   place comment blocks starting with TAG and\n\
                                preceding keyword lines in output file\n\
  -c, --add-comments          place all comment blocks preceding keyword lines\n\
                                in output file\n"));
      printf (_("\
      --check=NAME            perform syntax check on messages\n\
                                (ellipsis-unicode, space-ellipsis,\n\
                                 quote-unicode, bullet-unicode)\n"));
      printf (_("\
      --sentence-end=TYPE     type describing the end of sentence\n\
                                (single-space, which is the default, \n\
                                 or double-space)\n"));
      printf ("\n");
      printf (_("\
Language specific options:\n"));
      printf (_("\
  -a, --extract-all           extract all strings\n"));
      printf (_("\
                                (only languages C, C++, ObjectiveC, Shell,\n\
                                Python, Lisp, EmacsLisp, librep, Scheme, Java,\n\
                                C#, awk, Tcl, Perl, PHP, GCC-source, Glade,\n\
                                Lua, JavaScript, Vala)\n"));
      printf (_("\
  -kWORD, --keyword=WORD      look for WORD as an additional keyword\n\
  -k, --keyword               do not to use default keywords\n"));
      printf (_("\
                                (only languages C, C++, ObjectiveC, Shell,\n\
                                Python, Lisp, EmacsLisp, librep, Scheme, Java,\n\
                                C#, awk, Tcl, Perl, PHP, GCC-source, Glade,\n\
                                Lua, JavaScript, Vala, Desktop)\n"));
      printf (_("\
      --flag=WORD:ARG:FLAG    additional flag for strings inside the argument\n\
                              number ARG of keyword WORD\n"));
      printf (_("\
                                (only languages C, C++, ObjectiveC, Shell,\n\
                                Python, Lisp, EmacsLisp, librep, Scheme, Java,\n\
                                C#, awk, YCP, Tcl, Perl, PHP, GCC-source,\n\
                                Lua, JavaScript, Vala)\n"));
      printf (_("\
  -T, --trigraphs             understand ANSI C trigraphs for input\n"));
      printf (_("\
                                (only languages C, C++, ObjectiveC)\n"));
      printf (_("\
      --its=FILE              apply ITS rules from FILE\n"));
      printf (_("\
                                (only XML based languages)\n"));
      printf (_("\
      --qt                    recognize Qt format strings\n"));
      printf (_("\
                                (only language C++)\n"));
      printf (_("\
      --kde                   recognize KDE 4 format strings\n"));
      printf (_("\
                                (only language C++)\n"));
      printf (_("\
      --boost                 recognize Boost format strings\n"));
      printf (_("\
                                (only language C++)\n"));
      printf (_("\
      --debug                 more detailed formatstring recognition result\n"));
      printf ("\n");
      printf (_("\
Output details:\n"));
      printf (_("\
      --color                 use colors and other text attributes always\n\
      --color=WHEN            use colors and other text attributes if WHEN.\n\
                              WHEN may be 'always', 'never', 'auto', or 'html'.\n"));
      printf (_("\
      --style=STYLEFILE       specify CSS style rule file for --color\n"));
      printf (_("\
  -e, --no-escape             do not use C escapes in output (default)\n"));
      printf (_("\
  -E, --escape                use C escapes in output, no extended chars\n"));
      printf (_("\
      --force-po              write PO file even if empty\n"));
      printf (_("\
  -i, --indent                write the .po file using indented style\n"));
      printf (_("\
      --no-location           do not write '#: filename:line' lines\n"));
      printf (_("\
  -n, --add-location          generate '#: filename:line' lines (default)\n"));
      printf (_("\
      --strict                write out strict Uniforum conforming .po file\n"));
      printf (_("\
      --properties-output     write out a Java .properties file\n"));
      printf (_("\
      --stringtable-output    write out a NeXTstep/GNUstep .strings file\n"));
      printf (_("\
      --itstool               write out itstool comments\n"));
      printf (_("\
  -w, --width=NUMBER          set output page width\n"));
      printf (_("\
      --no-wrap               do not break long message lines, longer than\n\
                              the output page width, into several lines\n"));
      printf (_("\
  -s, --sort-output           generate sorted output (deprecated)\n"));
      printf (_("\
  -F, --sort-by-file          sort output by file location\n"));
      printf (_("\
      --omit-header           don't write header with 'msgid \"\"' entry\n"));
      printf (_("\
      --copyright-holder=STRING  set copyright holder in output\n"));
      printf (_("\
      --foreign-user          omit FSF copyright in output for foreign user\n"));
      printf (_("\
      --package-name=PACKAGE  set package name in output\n"));
      printf (_("\
      --package-version=VERSION  set package version in output\n"));
      printf (_("\
      --msgid-bugs-address=EMAIL@ADDRESS  set report address for msgid bugs\n"));
      printf (_("\
  -m[STRING], --msgstr-prefix[=STRING]  use STRING or \"\" as prefix for msgstr\n\
                                values\n"));
      printf (_("\
  -M[STRING], --msgstr-suffix[=STRING]  use STRING or \"\" as suffix for msgstr\n\
                                values\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
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


static void
exclude_directive_domain (abstract_catalog_reader_ty *pop, char *name)
{
  po_gram_error_at_line (&gram_pos,
                         _("this file may not contain domain directives"));
}


static void
exclude_directive_message (abstract_catalog_reader_ty *pop,
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
  message_ty *mp;

  /* See if this message ID has been seen before.  */
  if (exclude == NULL)
    exclude = message_list_alloc (true);
  mp = message_list_search (exclude, msgctxt, msgid);
  if (mp != NULL)
    free (msgid);
  else
    {
      mp = message_alloc (msgctxt, msgid, msgid_plural, "", 1, msgstr_pos);
      /* Do not free msgid.  */
      message_list_append (exclude, mp);
    }

  /* All we care about is the msgid.  Throw the msgstr away.
     Don't even check for duplicate msgids.  */
  free (msgstr);
}


/* So that the one parser can be used for multiple programs, and also
   use good data hiding and encapsulation practices, an object
   oriented approach has been taken.  An object instance is allocated,
   and all actions resulting from the parse will be through
   invocations of method functions of that object.  */

static abstract_catalog_reader_class_ty exclude_methods =
{
  sizeof (abstract_catalog_reader_ty),
  NULL, /* constructor */
  NULL, /* destructor */
  NULL, /* parse_brief */
  NULL, /* parse_debrief */
  exclude_directive_domain,
  exclude_directive_message,
  NULL, /* comment */
  NULL, /* comment_dot */
  NULL, /* comment_filepos */
  NULL, /* comment_special */
};


static void
read_exclusion_file (char *filename)
{
  char *real_filename;
  FILE *fp = open_catalog_file (filename, &real_filename, true);
  abstract_catalog_reader_ty *pop;

  pop = catalog_reader_alloc (&exclude_methods);
  catalog_reader_parse (pop, fp, real_filename, filename, &input_format_po);
  catalog_reader_free (pop);

  if (fp != stdin)
    fclose (fp);
}


static void
flag_context_list_table_insert (flag_context_list_table_ty *table,
                                unsigned int index,
                                const char *name_start, const char *name_end,
                                int argnum, enum is_format value, bool pass)
{
  char *allocated_name = NULL;

  if (table == &flag_table_lisp)
    {
      /* Convert NAME to upper case.  */
      size_t name_len = name_end - name_start;
      char *name = allocated_name = (char *) xmalloca (name_len);
      size_t i;

      for (i = 0; i < name_len; i++)
        name[i] = (name_start[i] >= 'a' && name_start[i] <= 'z'
                   ? name_start[i] - 'a' + 'A'
                   : name_start[i]);
      name_start = name;
      name_end = name + name_len;
    }
  else if (table == &flag_table_tcl)
    {
      /* Remove redundant "::" prefix.  */
      if (name_end - name_start > 2
          && name_start[0] == ':' && name_start[1] == ':')
        name_start += 2;
    }

  flag_context_list_table_add (table, index, name_start, name_end,
                               argnum, value, pass);

  if (allocated_name != NULL)
    freea (allocated_name);
}

void
xgettext_record_flag (const char *optionstring)
{
  /* Check the string has at least two colons.  (Colons in the name are
     allowed, needed for the Lisp and the Tcl backends.)  */
  const char *colon1;
  const char *colon2;

  for (colon2 = optionstring + strlen (optionstring); ; )
    {
      if (colon2 == optionstring)
        goto err;
      colon2--;
      if (*colon2 == ':')
        break;
    }
  for (colon1 = colon2; ; )
    {
      if (colon1 == optionstring)
        goto err;
      colon1--;
      if (*colon1 == ':')
        break;
    }
  {
    const char *name_start = optionstring;
    const char *name_end = colon1;
    const char *argnum_start = colon1 + 1;
    const char *argnum_end = colon2;
    const char *flag_start = colon2 + 1;
    const char *flag_end;
    const char *backend;
    int argnum;

    /* Check the parts' syntax.  */
    if (name_end == name_start)
      goto err;
    if (argnum_end == argnum_start)
      goto err;
    {
      char *endp;
      argnum = strtol (argnum_start, &endp, 10);
      if (endp != argnum_end)
        goto err;
    }
    if (argnum <= 0)
      goto err;

    flag_end = strchr (flag_start, '!');
    if (flag_end != NULL)
      backend = flag_end + 1;
    else
      {
        flag_end = flag_start + strlen (flag_start);
        backend = NULL;
      }

    /* Analyze the flag part.  */
    {
      bool pass;

      pass = false;
      if (flag_end - flag_start >= 5 && memcmp (flag_start, "pass-", 5) == 0)
        {
          pass = true;
          flag_start += 5;
        }

      /* Unlike po_parse_comment_special(), we don't accept "fuzzy",
         "wrap", or "check" here - it has no sense.  */
      if (flag_end - flag_start >= 7
          && memcmp (flag_end - 7, "-format", 7) == 0)
        {
          const char *p;
          size_t n;
          enum is_format value;
          size_t type;

          p = flag_start;
          n = flag_end - flag_start - 7;

          if (n >= 3 && memcmp (p, "no-", 3) == 0)
            {
              p += 3;
              n -= 3;
              value = no;
            }
          else if (n >= 9 && memcmp (p, "possible-", 9) == 0)
            {
              p += 9;
              n -= 9;
              value = possible;
            }
          else if (n >= 11 && memcmp (p, "impossible-", 11) == 0)
            {
              p += 11;
              n -= 11;
              value = impossible;
            }
          else
            value = yes_according_to_context;

          for (type = 0; type < NFORMATS; type++)
            if (strlen (format_language[type]) == n
                && memcmp (format_language[type], p, n) == 0)
              {
                /* This dispatch does the reverse mapping of all the SCANNERS_*
                   macros defined in the x-*.h files.  For example,
                   SCANNERS_JAVA contains an entry
                     { ...,
                       &flag_table_java,
                       &formatstring_java, &formatstring_java_printf
                     }
                   Therefore here, we have to associate
                     format_java          with   flag_table_java at index 0,
                     format_java_printf   with   flag_table_java at index 1.  */
                switch (type)
                  {
                  case format_c:
                    if (backend == NULL || strcmp (backend, "C") == 0
                        || strcmp (backend, "C++") == 0)
                      {
                        flag_context_list_table_insert (&flag_table_c, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                      }
                    if (backend == NULL || strcmp (backend, "C++") == 0)
                      {
                        flag_context_list_table_insert (&flag_table_cxx_qt, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                        flag_context_list_table_insert (&flag_table_cxx_kde, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                        flag_context_list_table_insert (&flag_table_cxx_boost, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                      }
                    if (backend == NULL || strcmp (backend, "ObjectiveC") == 0)
                      {
                        flag_context_list_table_insert (&flag_table_objc, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                      }
                    if (backend == NULL || strcmp (backend, "Vala") == 0)
                      {
                        flag_context_list_table_insert (&flag_table_vala, 0,
                                                        name_start, name_end,
                                                        argnum, value, pass);
                      }
                    break;
                  case format_cplusplus_brace:
                    flag_context_list_table_insert (&flag_table_c, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    flag_context_list_table_insert (&flag_table_cxx_qt, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    flag_context_list_table_insert (&flag_table_cxx_kde, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    flag_context_list_table_insert (&flag_table_cxx_boost, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_objc:
                    flag_context_list_table_insert (&flag_table_objc, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_python:
                    flag_context_list_table_insert (&flag_table_python, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_python_brace:
                    flag_context_list_table_insert (&flag_table_python, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_java:
                    flag_context_list_table_insert (&flag_table_java, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_java_printf:
                    flag_context_list_table_insert (&flag_table_java, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_csharp:
                    flag_context_list_table_insert (&flag_table_csharp, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_javascript:
                    flag_context_list_table_insert (&flag_table_javascript, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_scheme:
                    flag_context_list_table_insert (&flag_table_scheme, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_lisp:
                    flag_context_list_table_insert (&flag_table_lisp, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_elisp:
                    flag_context_list_table_insert (&flag_table_elisp, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_librep:
                    flag_context_list_table_insert (&flag_table_librep, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_ruby:
                    flag_context_list_table_insert (&flag_table_ruby, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_sh:
                    flag_context_list_table_insert (&flag_table_sh, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_awk:
                    flag_context_list_table_insert (&flag_table_awk, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_lua:
                    flag_context_list_table_insert (&flag_table_lua, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_pascal:
                    break;
                  case format_smalltalk:
                    break;
                  case format_qt:
                    flag_context_list_table_insert (&flag_table_cxx_qt, 2,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_qt_plural:
                    flag_context_list_table_insert (&flag_table_cxx_qt, 3,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_kde:
                    flag_context_list_table_insert (&flag_table_cxx_kde, 2,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_kde_kuit:
                    flag_context_list_table_insert (&flag_table_cxx_kde, 3,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_boost:
                    flag_context_list_table_insert (&flag_table_cxx_boost, 2,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_tcl:
                    flag_context_list_table_insert (&flag_table_tcl, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_perl:
                    flag_context_list_table_insert (&flag_table_perl, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_perl_brace:
                    flag_context_list_table_insert (&flag_table_perl, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_php:
                    flag_context_list_table_insert (&flag_table_php, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_gcc_internal:
                    flag_context_list_table_insert (&flag_table_gcc_internal, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_gfc_internal:
                    flag_context_list_table_insert (&flag_table_gcc_internal, 1,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  case format_ycp:
                    flag_context_list_table_insert (&flag_table_ycp, 0,
                                                    name_start, name_end,
                                                    argnum, value, pass);
                    break;
                  default:
                    abort ();
                  }
                return;
              }
          /* If the flag is not among the valid values, the optionstring is
             invalid.  */
        }
    }
  }

err:
  error (EXIT_FAILURE, 0,
         _("A --flag argument doesn't have the <keyword>:<argnum>:[pass-]<flag> syntax: %s"),
         optionstring);
}


/* Comment handling: There is a list of automatic comments that may be appended
   to the next message.  Used by remember_a_message().  */

static string_list_ty *comment;

static void
xgettext_comment_add (const char *str)
{
  if (comment == NULL)
    comment = string_list_alloc ();
  string_list_append (comment, str);
}

const char *
xgettext_comment (size_t n)
{
  if (comment == NULL || n >= comment->nitems)
    return NULL;
  return comment->item[n];
}

void
xgettext_comment_reset (void)
{
  if (comment != NULL)
    {
      string_list_free (comment);
      comment = NULL;
    }
}


refcounted_string_list_ty *savable_comment;

void
savable_comment_add (const char *str)
{
  if (savable_comment == NULL)
    {
      savable_comment = XMALLOC (refcounted_string_list_ty);
      savable_comment->refcount = 1;
      string_list_init (&savable_comment->contents);
    }
  else if (savable_comment->refcount > 1)
    {
      /* Unshare the list by making copies.  */
      struct string_list_ty *oldcontents;
      size_t i;

      savable_comment->refcount--;
      oldcontents = &savable_comment->contents;

      savable_comment = XMALLOC (refcounted_string_list_ty);
      savable_comment->refcount = 1;
      string_list_init (&savable_comment->contents);
      for (i = 0; i < oldcontents->nitems; i++)
        string_list_append (&savable_comment->contents, oldcontents->item[i]);
    }
  string_list_append (&savable_comment->contents, str);
}

void
savable_comment_reset ()
{
  drop_reference (savable_comment);
  savable_comment = NULL;
}

void
savable_comment_to_xgettext_comment (refcounted_string_list_ty *rslp)
{
  xgettext_comment_reset ();
  if (rslp != NULL)
    {
      size_t i;

      for (i = 0; i < rslp->contents.nitems; i++)
        xgettext_comment_add (rslp->contents.item[i]);
    }
}


/* xgettext_find_file and xgettext_open look up a file, taking into account
   the --directory options.
   xgettext_find_file merely returns the file name and the directory in which
   it was found.  This function is useful for parsers implemented as separate
   programs.
   xgettext_open returns the open file stream.  This function is useful for
   built-in parsers.  */

static void
xgettext_find_file (const char *fn,
                    char **logical_file_name_p,
                    const char **found_in_dir_p,
                    char **real_file_name_p)
{
  char *new_name;
  const char *found_in_dir;
  char *logical_file_name;
  struct stat statbuf;

  found_in_dir = NULL;

  /* We cannot handle "-" here.  "/dev/fd/0" is not portable, and it cannot
     be opened multiple times.  */
  if (IS_RELATIVE_FILE_NAME (fn))
    {
      int j;

      for (j = 0; ; ++j)
        {
          const char *dir = dir_list_nth (j);

          if (dir == NULL)
            error (EXIT_FAILURE, ENOENT,
                   _("error while opening \"%s\" for reading"), fn);

          new_name = xconcatenated_filename (dir, fn, NULL);

          if (stat (new_name, &statbuf) == 0)
            {
              found_in_dir = dir;
              break;
            }

          if (errno != ENOENT)
            error (EXIT_FAILURE, errno,
                   _("error while opening \"%s\" for reading"), new_name);
          free (new_name);
        }

      /* Note that the NEW_NAME variable contains the actual file name
         and the logical file name is what is reported by xgettext.  In
         this case NEW_NAME is set to the file which was found along the
         directory search path, and LOGICAL_FILE_NAME is is set to the
         file name which was searched for.  */
      logical_file_name = xstrdup (fn);
    }
  else
    {
      new_name = xstrdup (fn);
      if (stat (fn, &statbuf) != 0)
        error (EXIT_FAILURE, errno,
               _("error while opening \"%s\" for reading"), fn);
      logical_file_name = xstrdup (new_name);
    }

  *logical_file_name_p = logical_file_name;
  *found_in_dir_p = found_in_dir;
  *real_file_name_p = new_name;
}

static FILE *
xgettext_open (const char *fn,
               char **logical_file_name_p, char **real_file_name_p)
{
  FILE *fp;
  char *new_name;
  char *logical_file_name;

  if (strcmp (fn, "-") == 0)
    {
      new_name = xstrdup (_("standard input"));
      logical_file_name = xstrdup (new_name);
      fp = stdin;
    }
  else if (IS_RELATIVE_FILE_NAME (fn))
    {
      int j;

      for (j = 0; ; ++j)
        {
          const char *dir = dir_list_nth (j);

          if (dir == NULL)
            error (EXIT_FAILURE, ENOENT,
                   _("error while opening \"%s\" for reading"), fn);

          new_name = xconcatenated_filename (dir, fn, NULL);

          fp = fopen (new_name, "r");
          if (fp != NULL)
            break;

          if (errno != ENOENT)
            error (EXIT_FAILURE, errno,
                   _("error while opening \"%s\" for reading"), new_name);
          free (new_name);
        }

      /* Note that the NEW_NAME variable contains the actual file name
         and the logical file name is what is reported by xgettext.  In
         this case NEW_NAME is set to the file which was found along the
         directory search path, and LOGICAL_FILE_NAME is is set to the
         file name which was searched for.  */
      logical_file_name = xstrdup (fn);
    }
  else
    {
      new_name = xstrdup (fn);
      fp = fopen (fn, "r");
      if (fp == NULL)
        error (EXIT_FAILURE, errno,
               _("error while opening \"%s\" for reading"), fn);
      logical_file_name = xstrdup (new_name);
    }

  *logical_file_name_p = logical_file_name;
  *real_file_name_p = new_name;
  return fp;
}


/* Language dependent format string parser.
   NULL if the language has no notion of format strings.  */
struct formatstring_parser *current_formatstring_parser1;
struct formatstring_parser *current_formatstring_parser2;
struct formatstring_parser *current_formatstring_parser3;
struct formatstring_parser *current_formatstring_parser4;


static void
extract_from_file (const char *file_name, extractor_ty extractor,
                   msgdomain_list_ty *mdlp)
{
  char *logical_file_name;
  char *real_file_name;

  current_formatstring_parser1 = extractor.formatstring_parser1;
  current_formatstring_parser2 = extractor.formatstring_parser2;
  current_formatstring_parser3 = extractor.formatstring_parser3;
  current_formatstring_parser4 = extractor.formatstring_parser4;

  if (extractor.extract_from_stream)
    {
      FILE *fp = xgettext_open (file_name, &logical_file_name, &real_file_name);

      /* Set the default for the source file encoding.  May be overridden by
         the extractor function.  */
      xgettext_current_source_encoding =
        (xgettext_global_source_encoding != NULL ? xgettext_global_source_encoding :
         po_charset_ascii);
#if HAVE_ICONV
      xgettext_current_source_iconv = xgettext_global_source_iconv;
#endif

      extractor.extract_from_stream (fp, real_file_name, logical_file_name,
                                     extractor.flag_table, mdlp);

      if (fp != stdin)
        fclose (fp);
    }
  else
    {
      const char *found_in_dir;
      xgettext_find_file (file_name, &logical_file_name,
                          &found_in_dir, &real_file_name);

      extractor.extract_from_file (found_in_dir, real_file_name,
                                   logical_file_name,
                                   extractor.flag_table, mdlp);
    }
  free (logical_file_name);
  free (real_file_name);

  current_formatstring_parser1 = NULL;
  current_formatstring_parser2 = NULL;
  current_formatstring_parser3 = NULL;
  current_formatstring_parser4 = NULL;
}

static message_ty *
xgettext_its_extract_callback (message_list_ty *mlp,
                               const char *msgctxt,
                               const char *msgid,
                               lex_pos_ty *pos,
                               const char *extracted_comment,
                               const char *marker,
                               enum its_whitespace_type_ty whitespace)
{
  message_ty *message;

  message = remember_a_message (mlp,
                                msgctxt == NULL ? NULL : xstrdup (msgctxt),
                                xstrdup (msgid),
                                false, false,
                                null_context, pos,
                                extracted_comment, NULL, false);

  if (add_itstool_comments)
    {
      char *dot = xasprintf ("(itstool) path: %s", marker);
      message_comment_dot_append (message, dot);
      free (dot);

      if (whitespace == ITS_WHITESPACE_PRESERVE)
        message->do_wrap = no;
    }

  return message;
}

static void
extract_from_xml_file (const char *file_name,
                       its_rule_list_ty *rules,
                       msgdomain_list_ty *mdlp)
{
  char *logical_file_name;
  char *real_file_name;
  FILE *fp = xgettext_open (file_name, &logical_file_name, &real_file_name);

  /* The default encoding for XML is UTF-8.  It can be overridden by
     an XML declaration in the XML file itself, not through the
     --from-code option.  */
  xgettext_current_source_encoding = po_charset_utf8;

#if HAVE_ICONV
  xgettext_current_source_iconv = xgettext_global_source_iconv;
#endif

  its_rule_list_extract (rules, fp, real_file_name, logical_file_name,
                         NULL,
                         mdlp,
                         xgettext_its_extract_callback);

  if (fp != stdin)
    fclose (fp);
  free (logical_file_name);
  free (real_file_name);
}


bool
recognize_qt_formatstrings (void)
{
  return recognize_format_qt
         && current_formatstring_parser4 == &formatstring_qt_plural;
}


static message_ty *
construct_header ()
{
  char *project_id_version;
  time_t now;
  char *timestring;
  message_ty *mp;
  char *msgstr;
  char *comment;
  static lex_pos_ty pos = { __FILE__, __LINE__ };

  if (package_name != NULL)
    {
      if (package_version != NULL)
        project_id_version = xasprintf ("%s %s", package_name, package_version);
      else
        project_id_version = xasprintf ("%s", package_name);
    }
  else
    project_id_version = xstrdup ("PACKAGE VERSION");

  if (msgid_bugs_address != NULL && msgid_bugs_address[0] == '\0')
    multiline_warning (xasprintf (_("warning: ")),
                       xstrdup (_("\
The option --msgid-bugs-address was not specified.\n\
If you are using a 'Makevars' file, please specify\n\
the MSGID_BUGS_ADDRESS variable there; otherwise please\n\
specify an --msgid-bugs-address command line option.\n\
")));

  time (&now);
  timestring = po_strftime (&now);

  msgstr = xasprintf ("\
Project-Id-Version: %s\n\
Report-Msgid-Bugs-To: %s\n\
POT-Creation-Date: %s\n\
PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n\
Last-Translator: FULL NAME <EMAIL@ADDRESS>\n\
Language-Team: LANGUAGE <LL@li.org>\n\
Language: \n\
MIME-Version: 1.0\n\
Content-Type: text/plain; charset=CHARSET\n\
Content-Transfer-Encoding: 8bit\n",
                      project_id_version,
                      msgid_bugs_address != NULL ? msgid_bugs_address : "",
                      timestring);
  assume (msgstr != NULL);
  free (timestring);
  free (project_id_version);

  mp = message_alloc (NULL, "", NULL, msgstr, strlen (msgstr) + 1, &pos);

  if (copyright_holder[0] != '\0')
    comment = xasprintf ("\
SOME DESCRIPTIVE TITLE.\n\
Copyright (C) YEAR %s\n\
This file is distributed under the same license as the %s package.\n\
FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n",
                           copyright_holder,
                           package_name != NULL ? package_name : "PACKAGE");
  else
    comment = xstrdup ("\
SOME DESCRIPTIVE TITLE.\n\
This file is put in the public domain.\n\
FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n");
  message_comment_append (mp, comment);
  free (comment);

  mp->is_fuzzy = true;

  return mp;
}

static void
finalize_header (msgdomain_list_ty *mdlp)
{
  /* If the generated PO file has plural forms, add a Plural-Forms template
     to the constructed header.  */
  {
    bool has_plural;
    size_t i, j;

    has_plural = false;
    for (i = 0; i < mdlp->nitems; i++)
      {
        message_list_ty *mlp = mdlp->item[i]->messages;

        for (j = 0; j < mlp->nitems; j++)
          {
            message_ty *mp = mlp->item[j];

            if (mp->msgid_plural != NULL)
              {
                has_plural = true;
                break;
              }
          }
        if (has_plural)
          break;
      }

    if (has_plural)
      {
        message_ty *header =
          message_list_search (mdlp->item[0]->messages, NULL, "");
        if (header != NULL
            && c_strstr (header->msgstr, "Plural-Forms:") == NULL)
          {
            size_t insertpos = strlen (header->msgstr);
            const char *suffix;
            size_t suffix_len;
            char *new_msgstr;

            suffix = "\nPlural-Forms: nplurals=INTEGER; plural=EXPRESSION;\n";
            if (insertpos == 0 || header->msgstr[insertpos-1] == '\n')
              suffix++;
            suffix_len = strlen (suffix);
            new_msgstr = XNMALLOC (header->msgstr_len + suffix_len, char);
            memcpy (new_msgstr, header->msgstr, insertpos);
            memcpy (new_msgstr + insertpos, suffix, suffix_len);
            memcpy (new_msgstr + insertpos + suffix_len,
                    header->msgstr + insertpos,
                    header->msgstr_len - insertpos);
            header->msgstr = new_msgstr;
            header->msgstr_len = header->msgstr_len + suffix_len;
          }
      }
  }

  /* If not all the strings were plain ASCII, or if the output syntax
     requires a charset conversion, set the charset in the header to UTF-8.
     All messages have already been converted to UTF-8 in remember_a_message
     and remember_a_message_plural.  */
  {
    bool has_nonascii = ! is_ascii_msgdomain_list (mdlp);
    bool has_filenames_with_spaces =
      msgdomain_list_has_filenames_with_spaces (mdlp);

    if (has_nonascii
        || (has_filenames_with_spaces
            && output_syntax->requires_utf8_for_filenames_with_spaces)
        || output_syntax->requires_utf8)
      {
        message_list_ty *mlp = mdlp->item[0]->messages;

        iconv_message_list (mlp, po_charset_utf8, po_charset_utf8, NULL);
      }
  }
}


static extractor_ty
language_to_extractor (const char *name)
{
  struct table_ty
  {
    const char *name;
    extract_from_stream_func extract_from_stream;
    extract_from_file_func extract_from_file;
    flag_context_list_table_ty *flag_table;
    struct formatstring_parser *formatstring_parser1;
    struct formatstring_parser *formatstring_parser2;
  };
  typedef struct table_ty table_ty;

  static table_ty table[] =
  {
    SCANNERS_PO
    SCANNERS_PROPERTIES
    SCANNERS_STRINGTABLE
    SCANNERS_C
    SCANNERS_PYTHON
    SCANNERS_JAVA
    SCANNERS_CSHARP
    SCANNERS_JAVASCRIPT
    SCANNERS_SCHEME
    SCANNERS_LISP
    SCANNERS_ELISP
    SCANNERS_LIBREP
    SCANNERS_RUBY
    SCANNERS_SH
    SCANNERS_AWK
    SCANNERS_LUA
    SCANNERS_SMALLTALK
    SCANNERS_VALA
    SCANNERS_TCL
    SCANNERS_PERL
    SCANNERS_PHP
    SCANNERS_YCP
    SCANNERS_RST
    SCANNERS_DESKTOP
    SCANNERS_GLADE
    SCANNERS_GSETTINGS
    SCANNERS_APPDATA
    /* Here may follow more languages and their scanners: pike, etc...
       Make sure new scanners honor the --exclude-file option.  */
  };

  table_ty *tp;

  for (tp = table; tp < ENDOF(table); ++tp)
    if (c_strcasecmp (name, tp->name) == 0)
      {
        extractor_ty result;

        result.extract_from_stream = tp->extract_from_stream;
        result.extract_from_file = tp->extract_from_file;
        result.flag_table = tp->flag_table;
        result.formatstring_parser1 = tp->formatstring_parser1;
        result.formatstring_parser2 = tp->formatstring_parser2;
        result.formatstring_parser3 = NULL;
        result.formatstring_parser4 = NULL;

        /* Handle --qt.  It's preferrable to handle this facility here rather
           than through an option --language=C++/Qt because the latter would
           conflict with the language "C++" regarding the file extensions.  */
        if (recognize_format_qt && strcmp (tp->name, "C++") == 0)
          {
            result.flag_table = &flag_table_cxx_qt;
            result.formatstring_parser3 = &formatstring_qt;
            result.formatstring_parser4 = &formatstring_qt_plural;
          }
        /* Likewise for --kde.  */
        if (recognize_format_kde && strcmp (tp->name, "C++") == 0)
          {
            result.flag_table = &flag_table_cxx_kde;
            result.formatstring_parser3 = &formatstring_kde;
            result.formatstring_parser4 = &formatstring_kde_kuit;
          }
        /* Likewise for --boost.  */
        if (recognize_format_boost && strcmp (tp->name, "C++") == 0)
          {
            result.flag_table = &flag_table_cxx_boost;
            result.formatstring_parser3 = &formatstring_boost;
          }

        return result;
      }

  error (EXIT_FAILURE, 0, _("language '%s' unknown"), name);
  /* NOTREACHED */
  {
    extractor_ty result = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    return result;
  }
}


static const char *
extension_to_language (const char *extension)
{
  struct table_ty
  {
    const char *extension;
    const char *language;
  };
  typedef struct table_ty table_ty;

  static table_ty table[] =
  {
    EXTENSIONS_PO
    EXTENSIONS_PROPERTIES
    EXTENSIONS_STRINGTABLE
    EXTENSIONS_C
    EXTENSIONS_PYTHON
    EXTENSIONS_JAVA
    EXTENSIONS_CSHARP
    EXTENSIONS_JAVASCRIPT
    EXTENSIONS_SCHEME
    EXTENSIONS_LISP
    EXTENSIONS_ELISP
    EXTENSIONS_LIBREP
    EXTENSIONS_RUBY
    EXTENSIONS_SH
    EXTENSIONS_AWK
    EXTENSIONS_LUA
    EXTENSIONS_SMALLTALK
    EXTENSIONS_VALA
    EXTENSIONS_TCL
    EXTENSIONS_PERL
    EXTENSIONS_PHP
    EXTENSIONS_YCP
    EXTENSIONS_RST
    EXTENSIONS_DESKTOP
    EXTENSIONS_GLADE
    EXTENSIONS_GSETTINGS
    EXTENSIONS_APPDATA
    /* Here may follow more file extensions... */
  };

  table_ty *tp;

  for (tp = table; tp < ENDOF(table); ++tp)
    if (strcmp (extension, tp->extension) == 0)
      return tp->language;
  return NULL;
}
